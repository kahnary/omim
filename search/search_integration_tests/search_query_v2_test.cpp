#include "testing/testing.hpp"

#include "search/search_tests_support/test_feature.hpp"
#include "search/search_tests_support/test_mwm_builder.hpp"
#include "search/search_tests_support/test_results_matching.hpp"
#include "search/search_tests_support/test_search_engine.hpp"
#include "search/search_tests_support/test_search_request.hpp"

#include "search/search_query_factory.hpp"
#include "search/v2/search_query_v2.hpp"

#include "indexer/classificator_loader.hpp"
#include "indexer/index.hpp"
#include "indexer/mwm_set.hpp"

#include "storage/country_decl.hpp"
#include "storage/country_info_getter.hpp"

#include "geometry/point2d.hpp"

#include "platform/country_file.hpp"
#include "platform/local_country_file.hpp"
#include "platform/local_country_file_utils.hpp"
#include "platform/platform.hpp"

#include "base/scope_guard.hpp"

#include "std/vector.hpp"

using namespace search::tests_support;

using TRules = vector<shared_ptr<MatchingRule>>;

namespace
{
class TestSearchQueryFactory : public search::SearchQueryFactory
{
  // search::SearchQueryFactory overrides:
  unique_ptr<search::Query> BuildSearchQuery(Index & index, CategoriesHolder const & categories,
                                             vector<search::Suggest> const & suggests,
                                             storage::CountryInfoGetter const & infoGetter) override
  {
    return make_unique<search::v2::SearchQueryV2>(index, categories, suggests, infoGetter);
  }
};

class SearchQueryV2Test
{
public:
  SearchQueryV2Test()
    : m_platform(GetPlatform())
    , m_scopedLog(LDEBUG)
    , m_engine("en", make_unique<storage::CountryInfoGetterForTesting>(),
               make_unique<TestSearchQueryFactory>())
  {
    classificator::Load();
  }

  ~SearchQueryV2Test()
  {
    for (auto const & file : m_files)
      Cleanup(file);
  }

  void RegisterCountry(string const & name, m2::RectD const & rect)
  {
    auto & infoGetter =
        static_cast<storage::CountryInfoGetterForTesting&>(m_engine.GetCountryInfoGetter());
    infoGetter.AddCountry(storage::CountryDef(name, rect));
  }

  template <typename TFn>
  MwmSet::MwmId RegisterMwm(string const & name, feature::DataHeader::MapType type, TFn && fn)
  {
    m_files.emplace_back(m_platform.WritableDir(), platform::CountryFile(name), 0 /* version */);
    auto & file = m_files.back();
    Cleanup(file);

    {
      TestMwmBuilder builder(file, type);
      fn(builder);
    }
    auto result = m_engine.RegisterMap(file);
    ASSERT_EQUAL(result.second, MwmSet::RegResult::Success, ());
    return result.first;
  }

  void SetViewport(m2::RectD const & viewport) { m_viewport = viewport; }

  bool ResultsMatch(string const & query, vector<shared_ptr<MatchingRule>> const & rules)
  {
    TestSearchRequest request(m_engine, query, "en", search::SearchParams::ALL, m_viewport);
    request.Wait();
    return MatchResults(m_engine, rules, request.Results());
  }

protected:
  Platform & m_platform;
  my::ScopedLogLevelChanger m_scopedLog;
  vector<platform::LocalCountryFile> m_files;
  vector<storage::CountryDef> m_countries;
  unique_ptr<storage::CountryInfoGetterForTesting> m_infoGetter;
  TestSearchEngine m_engine;
  m2::RectD m_viewport;

private:
  static void Cleanup(platform::LocalCountryFile const & map)
  {
    platform::CountryIndexes::DeleteFromDisk(map);
    map.DeleteFromDisk(MapOptions::Map);
  }
};
}  // namespace

UNIT_CLASS_TEST(SearchQueryV2Test, Smoke)
{
  auto const wonderlandCountry =
      make_shared<TestCountry>(m2::PointD(10, 10), "Wonderland", "en");

  auto const losAlamosCity =
      make_shared<TestCity>(m2::PointD(10, 10), "Los Alamos", "en", 100 /* rank */);
  auto const mskCity = make_shared<TestCity>(m2::PointD(0, 0), "Moscow", "en", 100 /* rank */);
  auto const longPondVillage =
      make_shared<TestVillage>(m2::PointD(15, 15), "Long Pond Village", "en", 10 /* rank */);

  auto const feynmanStreet = make_shared<TestStreet>(
      vector<m2::PointD>{m2::PointD(9.999, 9.999), m2::PointD(10, 10), m2::PointD(10.001, 10.001)},
      "Feynman street", "en");
  auto const bohrStreet1 = make_shared<TestStreet>(
      vector<m2::PointD>{m2::PointD(9.999, 10.001), m2::PointD(10, 10), m2::PointD(10.001, 9.999)},
      "Bohr street", "en");
  auto const bohrStreet2 = make_shared<TestStreet>(
      vector<m2::PointD>{m2::PointD(10.001, 9.999), m2::PointD(10.002, 9.998)}, "Bohr street",
      "en");
  auto const bohrStreet3 = make_shared<TestStreet>(
      vector<m2::PointD>{m2::PointD(10.002, 9.998), m2::PointD(10.003, 9.997)}, "Bohr street",
      "en");
  auto const firstAprilStreet = make_shared<TestStreet>(
      vector<m2::PointD>{m2::PointD(14.998, 15), m2::PointD(15.002, 15)}, "1st April street", "en");

  auto const feynmanHouse = make_shared<TestBuilding>(m2::PointD(10, 10), "Feynman house",
                                                      "1 unit 1", *feynmanStreet, "en");
  auto const bohrHouse =
      make_shared<TestBuilding>(m2::PointD(10, 10), "Bohr house", "1 unit 1", *bohrStreet1, "en");
  auto const hilbertHouse = make_shared<TestBuilding>(
      vector<m2::PointD>{
          {10.0005, 10.0005}, {10.0006, 10.0005}, {10.0006, 10.0006}, {10.0005, 10.0006}},
      "Hilbert house", "1 unit 2", *bohrStreet1, "en");
  auto const descartesHouse =
      make_shared<TestBuilding>(m2::PointD(10, 10), "Descartes house", "2", "en");
  auto const bornHouse =
      make_shared<TestBuilding>(m2::PointD(14.999, 15), "Born house", "8", *firstAprilStreet, "en");

  auto const busStop = make_shared<TestPOI>(m2::PointD(0, 0), "Bus stop", "en");
  auto const tramStop = make_shared<TestPOI>(m2::PointD(0.0001, 0.0001), "Tram stop", "en");
  auto const quantumTeleport1 =
      make_shared<TestPOI>(m2::PointD(0.0002, 0.0002), "Quantum teleport 1", "en");
  auto const quantumTeleport2 =
      make_shared<TestPOI>(m2::PointD(10, 10), "Quantum teleport 2", "en");
  auto const quantumCafe = make_shared<TestPOI>(m2::PointD(-0.0002, -0.0002), "Quantum cafe", "en");
  auto const lantern1 = make_shared<TestPOI>(m2::PointD(10.0005, 10.0005), "lantern 1", "en");
  auto const lantern2 = make_shared<TestPOI>(m2::PointD(10.0006, 10.0005), "lantern 2", "en");

  RegisterMwm("testWorld", feature::DataHeader::world, [&](TestMwmBuilder & builder)
              {
                builder.Add(*wonderlandCountry);
                builder.Add(*losAlamosCity);
                builder.Add(*mskCity);
              });
  auto wonderlandId =
      RegisterMwm("wonderland", feature::DataHeader::country, [&](TestMwmBuilder & builder)
                  {
                    builder.Add(*losAlamosCity);
                    builder.Add(*mskCity);
                    builder.Add(*longPondVillage);

                    builder.Add(*feynmanStreet);
                    builder.Add(*bohrStreet1);
                    builder.Add(*bohrStreet2);
                    builder.Add(*bohrStreet3);
                    builder.Add(*firstAprilStreet);

                    builder.Add(*feynmanHouse);
                    builder.Add(*bohrHouse);
                    builder.Add(*hilbertHouse);
                    builder.Add(*descartesHouse);
                    builder.Add(*bornHouse);

                    builder.Add(*busStop);
                    builder.Add(*tramStop);
                    builder.Add(*quantumTeleport1);
                    builder.Add(*quantumTeleport2);
                    builder.Add(*quantumCafe);
                    builder.Add(*lantern1);
                    builder.Add(*lantern2);
                  });

  RegisterCountry("wonderland", m2::RectD(m2::PointD(-1.0, -1.0), m2::PointD(10.1, 10.1)));

  SetViewport(m2::RectD(m2::PointD(-1.0, -1.0), m2::PointD(1.0, 1.0)));

  {
    TRules rules = {make_shared<ExactMatch>(wonderlandId, busStop)};
    TEST(ResultsMatch("Bus stop", rules), ());
  }
  {
    TRules rules = {make_shared<ExactMatch>(wonderlandId, quantumCafe),
                    make_shared<ExactMatch>(wonderlandId, quantumTeleport1),
                    make_shared<ExactMatch>(wonderlandId, quantumTeleport2)};
    TEST(ResultsMatch("quantum", rules), ());
  }
  {
    TRules rules = {make_shared<ExactMatch>(wonderlandId, quantumCafe),
                    make_shared<ExactMatch>(wonderlandId, quantumTeleport1)};
    TEST(ResultsMatch("quantum Moscow ", rules), ());
  }
  {
    TEST(ResultsMatch("     ", TRules()), ());
  }
  {
    TRules rules = {make_shared<ExactMatch>(wonderlandId, quantumTeleport2)};
    TEST(ResultsMatch("teleport feynman street", rules), ());
  }
  {
    TRules rules = {make_shared<ExactMatch>(wonderlandId, feynmanHouse),
                    make_shared<ExactMatch>(wonderlandId, lantern1)};
    TEST(ResultsMatch("feynman street 1", rules), ());
  }
  {
    TRules rules = {make_shared<ExactMatch>(wonderlandId, bohrHouse),
                    make_shared<ExactMatch>(wonderlandId, hilbertHouse),
                    make_shared<ExactMatch>(wonderlandId, lantern1)};
    TEST(ResultsMatch("bohr street 1", rules), ());
  }
  {
    TEST(ResultsMatch("bohr street 1 unit 3", TRules()), ());
  }
  {
    TRules rules = {make_shared<ExactMatch>(wonderlandId, lantern1),
                    make_shared<ExactMatch>(wonderlandId, lantern2)};
    TEST(ResultsMatch("bohr street 1 lantern ", rules), ());
  }
  {
    TRules rules = {make_shared<ExactMatch>(wonderlandId, feynmanHouse)};
    TEST(ResultsMatch("wonderland los alamos feynman 1 unit 1 ", rules), ());
  }
  {
    // It's possible to find Descartes house by name.
    TRules rules = {make_shared<ExactMatch>(wonderlandId, descartesHouse)};
    TEST(ResultsMatch("Los Alamos Descartes", rules), ());
  }
  {
    // It's not possible to find Descartes house by house number,
    // because it doesn't belong to Los Alamos streets. But it still
    // exists.
    TRules rules = {make_shared<ExactMatch>(wonderlandId, lantern2),
                    make_shared<ExactMatch>(wonderlandId, quantumTeleport2)};
    TEST(ResultsMatch("Los Alamos 2", rules), ());
  }
  {
    TRules rules = {make_shared<ExactMatch>(wonderlandId, bornHouse)};
    TEST(ResultsMatch("long pond 1st april street 8", rules), ());
  }
}

UNIT_CLASS_TEST(SearchQueryV2Test, SearchInWorld)
{
  auto const wonderland = make_shared<TestCountry>(m2::PointD(0, 0), "Wonderland", "en");
  auto const losAlamos =
      make_shared<TestCity>(m2::PointD(0, 0), "Los Alamos", "en", 100 /* rank */);

  auto testWorldId = RegisterMwm("testWorld", feature::DataHeader::world,
                                 [&](TestMwmBuilder & builder)
                                 {
                                   builder.Add(*wonderland);
                                   builder.Add(*losAlamos);
                                 });

  RegisterCountry("Wonderland", m2::RectD(m2::PointD(-1.0, -1.0), m2::PointD(1.0, 1.0)));

  SetViewport(m2::RectD(m2::PointD(-1.0, -1.0), m2::PointD(-0.5, -0.5)));
  {
    TRules rules = {make_shared<ExactMatch>(testWorldId, losAlamos)};
    TEST(ResultsMatch("Los Alamos", rules), ());
  }
  {
    TRules rules = {make_shared<ExactMatch>(testWorldId, wonderland)};
    TEST(ResultsMatch("Wonderland", rules), ());
  }
  {
    TRules rules = {make_shared<ExactMatch>(testWorldId, losAlamos)};
    TEST(ResultsMatch("Wonderland Los Alamos", rules), ());
  }
}

UNIT_CLASS_TEST(SearchQueryV2Test, SearchByName)
{
  auto const london = make_shared<TestCity>(m2::PointD(1, 1), "London", "en", 100 /* rank */);
  auto const hydePark =
      make_shared<TestPark>(vector<m2::PointD>{m2::PointD(0.5, 0.5), m2::PointD(1.5, 0.5),
                                               m2::PointD(1.5, 1.5), m2::PointD(0.5, 1.5)},
                            "Hyde Park", "en");

  RegisterMwm("testWorld", feature::DataHeader::world, [&](TestMwmBuilder & builder)
              {
                builder.Add(*london);
              });
  auto wonderlandId = RegisterMwm("wonderland", feature::DataHeader::country,
                                 [&](TestMwmBuilder & builder)
                                 {
                                   builder.Add(*hydePark);
                                 });
  RegisterCountry("Wonderland", m2::RectD(m2::PointD(0, 0), m2::PointD(2, 2)));

  SetViewport(m2::RectD(m2::PointD(-1.0, -1.0), m2::PointD(-0.9, -0.9)));

  TRules rules = {make_shared<ExactMatch>(wonderlandId, hydePark)};
  TEST(ResultsMatch("hyde park", rules), ());
  TEST(ResultsMatch("london hyde park", rules), ());
}

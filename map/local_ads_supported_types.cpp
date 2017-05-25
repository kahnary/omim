#include "map/local_ads_manager.hpp"

#include <initializer_list>

void LocalAdsManager::FillSupportedTypes()
{
  m_supportedTypes.Append<std::initializer_list<std::initializer_list<char const *>>>(
      {{"amenity", "atm"},
       {"amenity", "bank"},
       {"amenity", "bar"},
       {"amenity", "bbq"},
       {"amenity", "bicycle_parking"},
       {"amenity", "bicycle_rental"},
       {"amenity", "brothel"},
       {"amenity", "bureau_de_change"},
       {"amenity", "cafe"},
       {"amenity", "car_rental"},
       {"amenity", "car_sharing"},
       {"amenity", "car_wash"},
       {"amenity", "casino"},
       {"amenity", "charging_station"},
       {"amenity", "childcare"},
       {"amenity", "cinema"},
       {"amenity", "clinic"},
       {"amenity", "college"},
       {"amenity", "community_centre"},
       {"amenity", "courthouse"},
       {"amenity", "dentist"},
       {"amenity", "doctors"},
       {"amenity", "fast_food"},
       {"amenity", "fuel"},
       {"amenity", "grave_yard"},
       {"amenity", "hospital"},
       {"amenity", "hunting_stand"},
       {"amenity", "kindergarten"},
       {"amenity", "library"},
       {"amenity", "marketplace"},
       {"amenity", "nightclub"},
       {"amenity", "parking"},
       {"amenity", "pharmacy"},
       {"amenity", "post_office"},
       {"amenity", "pub"},
       {"amenity", "public_bookcase"},
       {"amenity", "recycling"},
       {"amenity", "restaurant"},
       {"amenity", "school"},
       {"amenity", "shelter"},
       {"amenity", "taxi"},
       {"amenity", "telephone"},
       {"amenity", "theatre"},
       {"amenity", "toilets"},
       {"amenity", "townhall"},
       {"amenity", "university"},
       {"amenity", "vending_machine"},
       {"amenity", "veterinary"},
       {"amenity", "waste_disposal"},

       {"craft"},

       {"historic", "castle"},
       {"historic", "museum"},
       {"historic", "ship"},

       {"leisure", "fitness_centre"},
       {"leisure", "sauna"},
       {"leisure", "sports_centre"},

       {"man_made", "lighthouse"},

       {"office"},

       {"shop"},

       {"sponsored", "booking"},

       {"sport"},

       {"tourism", "alpine_hut"},
       {"tourism", "apartment"},
       {"tourism", "artwork"},
       {"tourism", "attraction"},
       {"tourism", "camp_site"},
       {"tourism", "caravan_site"},
       {"tourism", "chalet"},
       {"tourism", "gallery"},
       {"tourism", "guest_house"},
       {"tourism", "hostel"},
       {"tourism", "hotel"},
       {"tourism", "information", "office"},
       {"tourism", "motel"},
       {"tourism", "museum"},
       {"tourism", "picnic_site"},
       {"tourism", "resort"},
       {"tourism", "viewpoint"},
       {"tourism", "zoo"}});
}
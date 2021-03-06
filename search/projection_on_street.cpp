#include "search/projection_on_street.hpp"

#include "geometry/mercator.hpp"

#include "geometry/robust_orientation.hpp"


namespace search
{

// ProjectionOnStreet ------------------------------------------------------------------------------
ProjectionOnStreet::ProjectionOnStreet()
  : m_proj(0, 0), m_distMeters(0), m_segIndex(0), m_projSign(false)
{
}

// ProjectionOnStreetCalculator --------------------------------------------------------------------
ProjectionOnStreetCalculator::ProjectionOnStreetCalculator(vector<m2::PointD> const & points)
{
  size_t const count = points.size();
  if (count < 2)
    return;

  m_segProjs.resize(count - 1);
  for (size_t i = 0; i + 1 != count; ++i)
    m_segProjs[i].SetBounds(points[i], points[i + 1]);
}

bool ProjectionOnStreetCalculator::GetProjection(m2::PointD const & point,
                                                 ProjectionOnStreet & proj) const
{
  size_t const kInvalidIndex = m_segProjs.size();
  proj.m_segIndex = kInvalidIndex;
  proj.m_distMeters = numeric_limits<double>::max();

  for (size_t index = 0; index < m_segProjs.size(); ++index)
  {
    m2::PointD const ptProj = m_segProjs[index](point);
    double const distMeters = MercatorBounds::DistanceOnEarth(point, ptProj);
    if (distMeters < proj.m_distMeters)
    {
      proj.m_proj = ptProj;
      proj.m_distMeters = distMeters;
      proj.m_segIndex = index;
      proj.m_projSign = m2::robust::OrientedS(m_segProjs[index].P0(),
                                              m_segProjs[index].P1(),
                                              point) <= 0.0;
    }
  }

  return (proj.m_segIndex < kInvalidIndex);
}

}  // namespace search

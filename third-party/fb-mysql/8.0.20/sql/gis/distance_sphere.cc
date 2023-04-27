// Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2.0,
// as published by the Free Software Foundation.
//
// This program is also distributed with certain software (including
// but not limited to OpenSSL) that is licensed under separate terms,
// as designated in a particular file or component or in included license
// documentation.  The authors of MySQL hereby grant you an additional
// permission to link the program and your derivative works with the
// separately licensed software that they have included with MySQL.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License, version 2.0, for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.

/// @file
///
/// Implements the distance_sphere functor and function.

#include "sql/gis/distance_sphere.h"
#include "sql/gis/distance_sphere_functor.h"

#include <boost/geometry.hpp>
#include <cmath>      // std::isinf, M_PI
#include <stdexcept>  // std::overflow_error

#include "my_dbug.h"                                // DBUG_ASSERT
#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/functor.h"     // gis::Functor, gis::not_implemented_exception
#include "sql/gis/geometries.h"  // gis::{Geometry{,_type}, Coordinate_system}
#include "sql/gis/geometries_cs.h"      // gis::{Cartesian_*, Geographic_*}
#include "sql/gis/geometries_traits.h"  // boost::geometry traits for gis types
#include "sql/sql_exception_handler.h"  // handle_gis_exception

namespace bg = boost::geometry;

namespace gis {

/// Map Cartesian geometry to geographic, mapping degrees east = x, degrees
/// north = y. Do not canonicalize coordinates of poles.
///
/// Used when a SQL function needs to accept Cartesian coordiates as a shorthand
/// for geographic with some default SRS.
static Geographic_point reinterpret_as_degrees(const Cartesian_point &g) {
  double lon_deg = g.x();
  double lat_deg = g.y();

  if (!(-180.0 < lon_deg && lon_deg <= 180.0))
    throw longitude_out_of_range_exception(lon_deg, -180.0, 180.0);

  if (!(-90.0 <= lat_deg && lat_deg <= 90.0))
    throw latitude_out_of_range_exception(lat_deg, -90.0, 90.0);

  return {lon_deg * M_PI / 180.0, lat_deg * M_PI / 180.0};
}

/// Map Cartesian geometry to geographic, mapping degrees east = x, degrees
/// north = y. Do not canonicalize coordinates of poles.
///
/// Used when a SQL function needs to accept Cartesian coordiates as a shorthand
/// for geographic with some default SRS.
static Geographic_multipoint reinterpret_as_degrees(
    const Cartesian_multipoint &g) {
  Geographic_multipoint dg{};
  for (auto const &point : g) {
    dg.push_back(reinterpret_as_degrees(point));
  }
  return dg;
}

double Distance_sphere::operator()(const Geometry *g1,
                                   const Geometry *g2) const {
  return apply(*this, g1, g2);
}

double Distance_sphere::eval(const Cartesian_point *g1,
                             const Cartesian_point *g2) const {
  // The parser interprets SRID 0 coordinates as Cartesian. This is incorrect
  // for distance_sphere that takes spherical coordinates in degrees.
  // Convert to internal representation for geographic coordinates.
  Geographic_point rg1 = reinterpret_as_degrees(*g1);
  Geographic_point rg2 = reinterpret_as_degrees(*g2);
  return eval(&rg1, &rg2);
}

double Distance_sphere::eval(const Cartesian_point *g1,
                             const Cartesian_multipoint *g2) const {
  // Distance is commutative.
  return eval(g2, g1);
}

double Distance_sphere::eval(const Cartesian_multipoint *g1,
                             const Cartesian_point *g2) const {
  Geographic_multipoint rg1 = reinterpret_as_degrees(*g1);
  Geographic_point rg2 = reinterpret_as_degrees(*g2);
  return eval(&rg1, &rg2);
}

double Distance_sphere::eval(const Cartesian_multipoint *g1,
                             const Cartesian_multipoint *g2) const {
  Geographic_multipoint rg1 = reinterpret_as_degrees(*g1);
  Geographic_multipoint rg2 = reinterpret_as_degrees(*g2);
  return eval(&rg1, &rg2);
}

double Distance_sphere::eval(const Geographic_point *g1,
                             const Geographic_point *g2) const {
  return bg::distance(*g1, *g2, m_strategy);
}

double Distance_sphere::eval(const Geographic_point *g1,
                             const Geographic_multipoint *g2) const {
  return bg::distance(*g1, *g2, m_strategy);
}

double Distance_sphere::eval(const Geographic_multipoint *g1,
                             const Geographic_point *g2) const {
  return bg::distance(*g1, *g2, m_strategy);
}

double Distance_sphere::eval(const Geographic_multipoint *g1,
                             const Geographic_multipoint *g2) const {
  // Boost does not yet implement distance between two multipoints. Find
  // minumum by iterating over multipoint-point distances.
  double minimum = eval(g1, &(*g2)[0]);
  for (size_t i = 1; i < g2->size(); i++) {
    double d = eval(g1, &(*g2)[i]);
    if (d < minimum) minimum = d;
  }
  return minimum;
}

double Distance_sphere::eval(const Geometry *g1, const Geometry *g2) const {
  throw not_implemented_exception::for_non_projected(*g1, *g2);
}

bool distance_sphere(const dd::Spatial_reference_system *srs,
                     const Geometry *g1, const Geometry *g2,
                     const char *func_name, double sphere_radius,
                     double *result, bool *result_null) noexcept {
  try {
    DBUG_ASSERT(g1->coordinate_system() == g2->coordinate_system());
    DBUG_ASSERT(!srs || srs->is_cartesian() || srs->is_geographic());
    DBUG_ASSERT(!srs || srs->is_cartesian() == (g1->coordinate_system() ==
                                                Coordinate_system::kCartesian));
    DBUG_ASSERT(!srs ||
                srs->is_geographic() == (g1->coordinate_system() ==
                                         Coordinate_system::kGeographic));

    *result_null = false;

    if (srs && srs->is_projected())
      throw not_implemented_exception::for_projected(*g1, *g2);

    *result = Distance_sphere{sphere_radius}(g1, g2);

    if (std::isinf(*result)) throw std::overflow_error("INFINITY");

    return false;
  } catch (...) {
    handle_gis_exception(func_name);
    return true;
  }
}

}  // namespace gis

// Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
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
/// Implements the is_simple functor and function.
#include "sql/gis/is_simple.h"          // gis::is_simple
#include "sql/gis/is_simple_functor.h"  // gis::Is_simple

#include <boost/geometry.hpp>

#include "my_dbug.h"                                // DBUG_ASSERT
#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/functor.h"                        // gis::null_value_exception
#include "sql/gis/geometries.h"  // gis::{Geometry{,_type}, Coordinate_system}
#include "sql/gis/geometries_cs.h"       // gis::{Cartesian_*, Geographic_*}
#include "sql/gis/geometries_traits.h"   // boost::geometry traits for gis types
#include "sql/gis/intersects_functor.h"  // gis::Intersects
#include "sql/gis/touches_functor.h"     // gis::Touches
#include "sql/sql_exception_handler.h"   // handle_gis_exception

namespace bg = boost::geometry;

namespace gis {

Is_simple::Is_simple(double semi_major, double semi_minor)
    : m_geostrat{bg::srs::spheroid<double>(semi_major, semi_minor)},
      m_semi_major{semi_major},
      m_semi_minor{semi_minor} {}

bool Is_simple::operator()(const Geometry &g) const {
  DBUG_ASSERT(!g.is_empty() || g.type() == Geometry_type::kGeometrycollection);

  return apply(*this, g);
}

bool Is_simple::eval(const Cartesian_point &g) const {
  return bg::is_simple(g);
}

bool Is_simple::eval(const Cartesian_linestring &g) const {
  return bg::is_simple(g);
}

bool Is_simple::eval(const Cartesian_polygon &g) const {
  return bg::is_simple(g);
}

bool Is_simple::eval(const Cartesian_multipoint &g) const {
  return bg::is_simple(g);
}

bool Is_simple::eval(const Cartesian_multipolygon &g) const {
  return bg::is_simple(g);
}

bool Is_simple::eval(const Cartesian_multilinestring &g) const {
  return bg::is_simple(g);
}

bool Is_simple::eval(const Geographic_point &g) const {
  return bg::is_simple(g, m_geostrat);
}

bool Is_simple::eval(const Geographic_linestring &g) const {
  return bg::is_simple(g, m_geostrat);
}

bool Is_simple::eval(const Geographic_polygon &g) const {
  return bg::is_simple(g, m_geostrat);
}

bool Is_simple::eval(const Geographic_multipoint &g) const {
  return bg::is_simple(g, m_geostrat);
}

bool Is_simple::eval(const Geographic_multipolygon &g) const {
  return bg::is_simple(g, m_geostrat);
}

bool Is_simple::eval(const Geographic_multilinestring &g) const {
  return bg::is_simple(g, m_geostrat);
}

bool Is_simple::eval(const Geometrycollection &g) const {
  // Boost does not yet implement operations on geometrycollections. This
  // function implements the SQL/MM Spatial specification.

  const Is_simple &is_simple = *this;
  Intersects intersects{m_semi_major, m_semi_minor};
  Touches touches{m_semi_major, m_semi_minor};
  auto interiors_intersect = [&](const Geometry &g1, const Geometry &g2) {
    // TODO: Express this function in terms of `boost::relate` when that gets a
    // functor.

    // [intersecting interior] = ([intersecting] and not [touching]).
    // Proof by relate mask:
    // [T*******] = ([not FF*FF***] and not [FT****** or F**T**** or F***T***])
    // However, `touches` throws when applied to two geometries with
    // dimentionality zero. Zero-dim geomtries have no boundary, so they cannot
    // 'touch'.

    // Extend `touches` to give false for zero-dim geometries.
    if (!intersects(&g1, &g2)) return false;

    try {
      return !touches(&g1, &g2);
    } catch (const null_value_exception &) {
      return true;
    }
  };

  // The two requirements for simplicity of a gemetrycollection as described in
  // SQL/MM Part 3 Section 10.1.1 "Description" 9 are..
  for (size_t i = 0; i < g.size(); i++) {
    // 1) Subgeometries must be simple.
    if (!is_simple(g[i])) return false;
    // 2) Subgeometries must have mutually non-intersecting interiors.
    for (size_t j = 0; j < i; j++)
      if (interiors_intersect(g[i], g[j])) return false;
  }

  return true;
}

bool is_simple(const dd::Spatial_reference_system *srs, const Geometry *g,
               const char *func_name, bool *result,
               bool *result_null) noexcept {
  try {
    DBUG_ASSERT(!srs || srs->is_cartesian() || srs->is_geographic());
    DBUG_ASSERT(!srs || srs->is_cartesian() == (g->coordinate_system() ==
                                                Coordinate_system::kCartesian));
    DBUG_ASSERT(!srs ||
                srs->is_geographic() ==
                    (g->coordinate_system() == Coordinate_system::kGeographic));

    *result_null = false;

    double semi_major = srs ? srs->semi_major_axis() : 0.0;
    double semi_minor = srs ? srs->semi_minor_axis() : 0.0;

    *result = Is_simple{semi_major, semi_minor}(*g);
    return false;
  } catch (...) { /* purecov: inspected */
    handle_gis_exception(func_name);
    return true;
  }
}

}  // namespace gis

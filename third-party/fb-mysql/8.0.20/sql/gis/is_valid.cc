// Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
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

///@file
///
/// This file implements the Is_valid functor and is_valid function.

#include "is_valid.h"
#include "is_valid_functor.h"

#include <algorithm>  // all_of
#include <limits>

#include <boost/geometry.hpp>  // boost::geometry::is_simple

#include "my_inttypes.h"                            // MYF
#include "my_sys.h"                                 // my_error
#include "mysqld_error.h"                           // Error codes
#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_traits.h"
#include "sql/sql_exception_handler.h"  // handle_gis_exception

namespace bg = boost::geometry;

namespace gis {
Is_valid::Is_valid(double semi_major, double semi_minor)
    : m_geographic_ll_la_aa_strategy(
          bg::srs::spheroid<double>(semi_major, semi_minor)) {}

bool Is_valid::operator()(const Geometry &g) const { return apply(*this, g); }

bool Is_valid::eval(const Cartesian_point &g) const { return bg::is_valid(g); }
bool Is_valid::eval(const Cartesian_linestring &g) const {
  return bg::is_valid(g);
}
bool Is_valid::eval(const Cartesian_polygon &g) const {
  return bg::is_valid(g);
}
bool Is_valid::eval(const Cartesian_multipoint &g) const {
  return bg::is_valid(g);
}
bool Is_valid::eval(const Cartesian_multilinestring &g) const {
  return bg::is_valid(g);
}
bool Is_valid::eval(const Cartesian_multipolygon &g) const {
  return bg::is_valid(g);
}
bool Is_valid::eval(const Cartesian_geometrycollection &g) const {
  return std::all_of(g.begin(), g.end(),
                     [this](const Geometry *a) { return apply(*this, *a); });
}

bool Is_valid::eval(const Geographic_point &g) const { return bg::is_valid(g); }
bool Is_valid::eval(const Geographic_linestring &g) const {
  return bg::is_valid(g, m_geographic_ll_la_aa_strategy);
}
bool Is_valid::eval(const Geographic_polygon &g) const {
  return bg::is_valid(g, m_geographic_ll_la_aa_strategy);
}
bool Is_valid::eval(const Geographic_multipoint &g) const {
  return bg::is_valid(g, m_geographic_ll_la_aa_strategy);
}
bool Is_valid::eval(const Geographic_multilinestring &g) const {
  return bg::is_valid(g, m_geographic_ll_la_aa_strategy);
}
bool Is_valid::eval(const Geographic_multipolygon &g) const {
  return bg::is_valid(g, m_geographic_ll_la_aa_strategy);
}
bool Is_valid::eval(const Geographic_geometrycollection &g) const {
  return std::all_of(g.begin(), g.end(),
                     [this](const Geometry *a) { return apply(*this, *a); });
}

bool is_valid(const dd::Spatial_reference_system *srs, const Geometry *g,
              const char *func_name, bool *is_valid) noexcept {
  try {
    DBUG_ASSERT(srs == nullptr ||
                ((srs->is_cartesian() &&
                  g->coordinate_system() == Coordinate_system::kCartesian) ||
                 (srs->is_geographic() &&
                  g->coordinate_system() == Coordinate_system::kGeographic)));
    if (g->is_empty()) {
      *is_valid = true;
      return false;
    }
    double smajor = 0.0;
    double sminor = 0.0;
    if (srs != nullptr) {
      smajor = srs->semi_major_axis();
      sminor = srs->semi_minor_axis();
    }
    Is_valid is_valid_functor(smajor, sminor);
    *is_valid = is_valid_functor(*g);
    return false;
  } catch (...) {
    handle_gis_exception(func_name);
    return true;
  }
}
}  // namespace gis

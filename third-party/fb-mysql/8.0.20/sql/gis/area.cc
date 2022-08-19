// Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

/// @file
///
/// Implements the area functor and function.

#include "sql/gis/area.h"
#include "sql/gis/area_functor.h"

#include <boost/geometry.hpp>  // boost::geometry
#include <cmath>               // isfinite

#include "my_dbug.h"                                // DBUG_ASSERT
#include "my_inttypes.h"                            // MYF
#include "my_sys.h"                                 // my_error
#include "mysqld_error.h"                           // ER_DATA_OUT_OF_RANGE
#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/geometries.h"     // gis::{Geometry{,_type},Coordinate_system}
#include "sql/gis/geometries_cs.h"  // gis::{Cartesian_*,Geographic_*}
#include "sql/gis/geometries_traits.h"  // boost::geometry traits for gis types
#include "sql/sql_exception_handler.h"  // handle_gis_exception

namespace bg = boost::geometry;

namespace gis {

Area::Area() = default;

Area::Area(double semi_major, double semi_minor)
    : m_semi_major(semi_major),
      m_semi_minor(semi_minor),
      m_geographic_strategy(bg::srs::spheroid<double>(semi_major, semi_minor)) {
}

double Area::operator()(const Geometry &g) const { return apply(*this, g); }

double Area::eval(const Cartesian_polygon &g) const { return bg::area(g); }

double Area::eval(const Cartesian_multipolygon &g) const { return bg::area(g); }

double Area::eval(const Geographic_polygon &g) const {
  return bg::area(g, m_geographic_strategy);
}

double Area::eval(const Geographic_multipolygon &g) const {
  return bg::area(g, m_geographic_strategy);
}

double Area::eval(const Geometry &) const {
  /* purecov: begin deadcode */
  // Not implemented
  DBUG_ASSERT(false);
  throw std::exception();
  /* purecov: end */
}

bool area(const dd::Spatial_reference_system *srs, const Geometry *g,
          const char *func_name, double *result, bool *result_null) noexcept {
  try {
    DBUG_ASSERT(((srs == nullptr || srs->is_cartesian()) &&
                 g->coordinate_system() == Coordinate_system::kCartesian) ||
                (srs != nullptr && srs->is_geographic() &&
                 g->coordinate_system() == Coordinate_system::kGeographic));
    // Calling area on empty geometry results in NULL.
    if (g->is_empty()) {
      *result_null = true;
      return false;
    }

    if (srs && srs->is_geographic())
      *result = Area(srs->semi_major_axis(), srs->semi_minor_axis())(*g);
    else
      *result = Area()(*g);

    if (!std::isfinite(*result)) {
      my_error(ER_DATA_OUT_OF_RANGE, MYF(0), "Result", func_name);
      return true;
    }

    *result_null = false;
    return false;
  } catch (...) {
    /* purecov: begin inspected */
    handle_gis_exception(func_name);
    return true;
    /* purecov: end */
  }
}

}  // namespace gis

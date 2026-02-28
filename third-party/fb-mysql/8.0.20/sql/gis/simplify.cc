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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.

/// @file
///
/// This file implements the simplify functor and function.

#include "sql/gis/simplify.h"
#include "sql/gis/simplify_functor.h"

#include <memory>  // std::unique_ptr

#include <boost/geometry.hpp>

#include "my_dbug.h"                                // DBUG_ASSERT
#include "my_inttypes.h"                            // MYF
#include "my_sys.h"                                 // my_error
#include "mysqld_error.h"                           // Error codes
#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_traits.h"
#include "sql/sql_exception_handler.h"  // handle_gis_exception

namespace bg = boost::geometry;

namespace gis {

std::unique_ptr<Geometry> Simplify::operator()(const Geometry &g) const {
  return apply(*this, g);
}

std::unique_ptr<Geometry> Simplify::eval(const Geometry &g) const {
  // All parameter type combinations have been implemented.
  DBUG_ASSERT(false);
  throw not_implemented_exception::for_non_projected(g);
}

std::unique_ptr<Geometry> Simplify::eval(const Cartesian_point &g) const {
  Cartesian_point *pt_result = new Cartesian_point();
  std::unique_ptr<Geometry> result(pt_result);
  bg::simplify(g, *pt_result, m_max_distance);
  return result;
}

std::unique_ptr<Geometry> Simplify::eval(const Cartesian_linestring &g) const {
  Cartesian_linestring *ls_result = new Cartesian_linestring();
  std::unique_ptr<Geometry> result(ls_result);
  bg::simplify(g, *ls_result, m_max_distance);
  if (ls_result->size() < 2) ls_result->clear();
  return result;
}

std::unique_ptr<Geometry> Simplify::eval(const Cartesian_polygon &g) const {
  Cartesian_polygon *py_result = new Cartesian_polygon();
  std::unique_ptr<Geometry> result(py_result);
  bg::simplify(g, *py_result, m_max_distance);
  if (py_result->exterior_ring().size() < 4)
    result.reset(new Cartesian_polygon());
  return result;
}

std::unique_ptr<Geometry> Simplify::eval(
    const Cartesian_geometrycollection &g) const {
  Cartesian_geometrycollection *gc_result = new Cartesian_geometrycollection();
  std::unique_ptr<Geometry> result(gc_result);
  for (Geometry *geom : g) {
    std::unique_ptr<Geometry> simplified_geom = (*this)(*geom);
    if (!simplified_geom->is_empty()) gc_result->push_back(*simplified_geom);
  }
  return result;
}

std::unique_ptr<Geometry> Simplify::eval(const Cartesian_multipoint &g) const {
  Cartesian_multipoint *mpt_result = new Cartesian_multipoint();
  std::unique_ptr<Geometry> result(mpt_result);
  bg::simplify(g, *mpt_result, m_max_distance);
  return result;
}

std::unique_ptr<Geometry> Simplify::eval(
    const Cartesian_multilinestring &g) const {
  std::unique_ptr<Cartesian_multilinestring> unfiltered_result(
      new Cartesian_multilinestring());
  bg::simplify(g, *unfiltered_result, m_max_distance);

  // bg::simplify may create geometries with too few points. Filter out those.
  Cartesian_multilinestring *mls_result = new Cartesian_multilinestring();
  std::unique_ptr<Geometry> result(mls_result);
  for (Cartesian_linestring &ls : *unfiltered_result) {
    if (ls.size() >= 2) mls_result->push_back(ls);
  }

  return result;
}
std::unique_ptr<Geometry> Simplify::eval(
    const Cartesian_multipolygon &g) const {
  std::unique_ptr<Cartesian_multipolygon> unfiltered_result(
      new Cartesian_multipolygon());
  bg::simplify(g, *unfiltered_result, m_max_distance);

  // bg::simplify may create geometries with too few points. Filter out those.
  Cartesian_multipolygon *mpy_result = new Cartesian_multipolygon();
  std::unique_ptr<Geometry> result(mpy_result);
  for (Cartesian_polygon &py : *unfiltered_result) {
    if (py.exterior_ring().size() >= 4) mpy_result->push_back(py);
  }

  return result;
}

bool simplify(const dd::Spatial_reference_system *srs, const Geometry &g,
              double max_distance, const char *func_name,
              std::unique_ptr<Geometry> *result) noexcept {
  try {
    DBUG_ASSERT(srs == nullptr ||
                ((srs->is_cartesian() &&
                  g.coordinate_system() == Coordinate_system::kCartesian) ||
                 (srs->is_geographic() &&
                  g.coordinate_system() == Coordinate_system::kGeographic)));

    if (srs != nullptr && !srs->is_cartesian()) {
      DBUG_ASSERT(srs->is_geographic());
      std::stringstream types;
      types << type_to_name(g.type()) << ", ...";
      my_error(ER_NOT_IMPLEMENTED_FOR_GEOGRAPHIC_SRS, MYF(0), func_name,
               types.str().c_str());
      return true;
    }

    Simplify simplify_func(max_distance);
    *result = simplify_func(g);
    if ((*result)->is_empty()) result->reset();
  } catch (...) {
    handle_gis_exception(func_name);
    return true;
  }

  return false;
}

}  // namespace gis

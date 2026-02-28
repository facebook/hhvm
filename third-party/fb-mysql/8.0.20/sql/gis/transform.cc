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
/// This file implements the transform functor and function.

// boost/geometry/srs/projections/impl/dms_parser.hpp uses "SEC" as a template
// parameter name, but SEC is a macro in sys/time.h on Solaris.
#include "my_config.h"  // HAVE_SYS_TIME_H
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#undef SEC
#endif

#include "sql/gis/transform.h"
#include "sql/gis/transform_functor.h"

#include <boost/geometry.hpp>
#include <memory>  // std::unique_ptr

#include "my_inttypes.h"                            // MYF
#include "my_sys.h"                                 // my_error
#include "mysqld_error.h"                           // Error codes
#include "sql/dd/string_type.h"                     // dd::String_type
#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_traits.h"
#include "sql/sql_exception_handler.h"  // handle_gis_exception

namespace bg = boost::geometry;

namespace gis {

Transform::Transform(const std::string &old_srs_params,
                     const std::string &new_srs_params,
                     Coordinate_system output_cs)
    : m_transformation(bg::srs::proj4(old_srs_params),
                       bg::srs::proj4(new_srs_params)),
      m_output_cs(output_cs) {}

std::unique_ptr<Geometry> Transform::operator()(const Geometry &g) const {
  return apply(*this, g);
}

std::unique_ptr<Geometry> Transform::eval(const Geometry &g) const {
  /* purecov: begin deadcode */
  DBUG_ASSERT(false);
  throw not_implemented_exception::for_non_projected(g);
  /* purecov: end */
}

std::unique_ptr<Geometry> Transform::eval(const Geographic_point &g) const {
  switch (m_output_cs) {
    case Coordinate_system::kCartesian:
      // We currently support only transformations between two geographic
      // systems.
      break;
    case Coordinate_system::kGeographic: {
      // Workaround for limitation in Developer Studio 12.5 on Solaris that
      // doesn't allow returning std::unique_ptr<subtype of Geometry>.
      Geographic_point *pt_result = new Geographic_point();
      std::unique_ptr<Geometry> result(pt_result);
      m_transformation.forward(g, *pt_result);
      return result;
    }
  }
  /* purceov: begin deadcode */
  DBUG_ASSERT(false);
  throw std::exception();
  /* purceov: end */
}

std::unique_ptr<Geometry> Transform::eval(
    const Geographic_linestring &g) const {
  switch (m_output_cs) {
    case Coordinate_system::kCartesian:
      // We currently support only transformations between two geographic
      // systems.
      break;
    case Coordinate_system::kGeographic: {
      // Workaround for limitation in Developer Studio 12.5 on Solaris that
      // doesn't allow returning std::unique_ptr<subtype of Geometry>.
      Geographic_linestring *ls_result = new Geographic_linestring();
      std::unique_ptr<Geometry> result(ls_result);
      m_transformation.forward(g, *ls_result);
      return result;
    }
  }
  /* purceov: begin deadcode */
  DBUG_ASSERT(false);
  throw std::exception();
  /* purceov: end */
}

std::unique_ptr<Geometry> Transform::eval(const Geographic_polygon &g) const {
  switch (m_output_cs) {
    case Coordinate_system::kCartesian:
      // We currently support only transformations between two geographic
      // systems.
      break;
    case Coordinate_system::kGeographic: {
      // Workaround for limitation in Developer Studio 12.5 on Solaris that
      // doesn't allow returning std::unique_ptr<subtype of Geometry>.
      Geographic_polygon *py_result = new Geographic_polygon();
      std::unique_ptr<Geometry> result(py_result);
      m_transformation.forward(g, *py_result);
      return result;
    }
  }
  /* purceov: begin deadcode */
  DBUG_ASSERT(false);
  throw std::exception();
  /* purceov: end */
}

std::unique_ptr<Geometry> Transform::eval(
    const Geographic_geometrycollection &g) const {
  switch (m_output_cs) {
    case Coordinate_system::kCartesian:
      // We currently support only transformations between two geographic
      // systems.
      break;
    case Coordinate_system::kGeographic: {
      // Workaround for limitation in Developer Studio 12.5 on Solaris that
      // doesn't allow returning std::unique_ptr<subtype of Geometry>.
      Geographic_geometrycollection *gc_result =
          new Geographic_geometrycollection();
      std::unique_ptr<Geometry> result(gc_result);
      for (std::size_t i = 0; i < g.size(); i++) {
        std::unique_ptr<Geometry> geom_res(apply(*this, g[i]));
        gc_result->push_back(*geom_res);
      }
      return result;
    }
  }
  /* purceov: begin deadcode */
  DBUG_ASSERT(false);
  throw std::exception();
  /* purceov: end */
}

std::unique_ptr<Geometry> Transform::eval(
    const Geographic_multipoint &g) const {
  switch (m_output_cs) {
    case Coordinate_system::kCartesian:
      // We currently support only transformations between two geographic
      // systems.
      break;
    case Coordinate_system::kGeographic: {
      // Workaround for limitation in Developer Studio 12.5 on Solaris that
      // doesn't allow returning std::unique_ptr<subtype of Geometry>.
      Geographic_multipoint *mpt_result = new Geographic_multipoint();
      std::unique_ptr<Geometry> result(mpt_result);
      m_transformation.forward(g, *mpt_result);
      return result;
    }
  }
  /* purceov: begin deadcode */
  DBUG_ASSERT(false);
  throw std::exception();
  /* purceov: end */
}

std::unique_ptr<Geometry> Transform::eval(
    const Geographic_multilinestring &g) const {
  switch (m_output_cs) {
    case Coordinate_system::kCartesian:
      // We currently support only transformations between two geographic
      // systems.
      break;
    case Coordinate_system::kGeographic: {
      // Workaround for limitation in Developer Studio 12.5 on Solaris that
      // doesn't allow returning std::unique_ptr<subtype of Geometry>.
      Geographic_multilinestring *mls_result = new Geographic_multilinestring();
      std::unique_ptr<Geometry> result(mls_result);
      m_transformation.forward(g, *mls_result);
      return result;
    }
  }
  /* purceov: begin deadcode */
  DBUG_ASSERT(false);
  throw std::exception();
  /* purceov: end */
}

std::unique_ptr<Geometry> Transform::eval(
    const Geographic_multipolygon &g) const {
  switch (m_output_cs) {
    case Coordinate_system::kCartesian:
      // We currently support only transformations between two geographic
      // systems.
      break;
    case Coordinate_system::kGeographic: {
      // Workaround for limitation in Developer Studio 12.5 on Solaris that
      // doesn't allow returning std::unique_ptr<subtype of Geometry>.
      Geographic_multipolygon *mpy_result = new Geographic_multipolygon();
      std::unique_ptr<Geometry> result(mpy_result);
      m_transformation.forward(g, *mpy_result);
      return result;
    }
  }
  /* purceov: begin deadcode */
  DBUG_ASSERT(false);
  throw std::exception();
  /* purceov: end */
}

bool transform(const dd::Spatial_reference_system *source_srs,
               const Geometry &in,
               const dd::Spatial_reference_system *target_srs,
               const char *func_name, std::unique_ptr<Geometry> *out) noexcept {
  try {
    dd::String_type source_proj = source_srs->proj4_parameters();
    dd::String_type target_proj = target_srs->proj4_parameters();

    if (source_srs->missing_towgs84()) {
      my_error(ER_TRANSFORM_SOURCE_SRS_MISSING_TOWGS84, MYF(0),
               source_srs->id());
      return true;
    }
    if (target_srs->missing_towgs84()) {
      my_error(ER_TRANSFORM_TARGET_SRS_MISSING_TOWGS84, MYF(0),
               target_srs->id());
      return true;
    }

    if (source_proj.size() == 0) {
      DBUG_ASSERT(source_srs->is_projected());
      my_error(ER_TRANSFORM_SOURCE_SRS_NOT_SUPPORTED, MYF(0), source_srs->id());
      return true;
    }
    if (target_proj.size() == 0) {
      DBUG_ASSERT(target_srs->is_projected());
      my_error(ER_TRANSFORM_TARGET_SRS_NOT_SUPPORTED, MYF(0), target_srs->id());
      return true;
    }

    Transform transform(source_proj.c_str(), target_proj.c_str(),
                        target_srs->cs_type());
    *out = transform(in);

    return false;
  } catch (...) {
    /* purecov: begin inspected */
    handle_gis_exception(func_name);
    return true;
    /* purecov: end */
  }
}

}  // namespace gis

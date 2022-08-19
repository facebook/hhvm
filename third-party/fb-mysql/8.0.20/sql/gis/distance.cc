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
/// This file implements the distance function.

#include "sql/gis/distance.h"
#include "sql/gis/distance_functor.h"

#include <cmath>  // std::isfinite

#include "sql/sql_exception_handler.h"  // handle_gis_exception

#include "my_inttypes.h"   // MYF
#include "my_sys.h"        // my_error
#include "mysqld_error.h"  // Error codes

namespace gis {
bool distance(const dd::Spatial_reference_system *srs, const Geometry *g1,
              const Geometry *g2, double *distance, bool *is_null) noexcept {
  try {
    DBUG_ASSERT(g1->coordinate_system() == g2->coordinate_system());
    DBUG_ASSERT(srs == nullptr ||
                ((srs->is_cartesian() &&
                  g1->coordinate_system() == Coordinate_system::kCartesian) ||
                 (srs->is_geographic() &&
                  g1->coordinate_system() == Coordinate_system::kGeographic)));

    if ((*is_null = (g1->is_empty() || g2->is_empty()))) return false;

    Distance dist(srs ? srs->semi_major_axis() : 0.0,
                  srs ? srs->semi_minor_axis() : 0.0);
    *distance = dist(g1, g2);
  } catch (...) {
    handle_gis_exception("st_distance");
    return true;
  }

  if (!std::isfinite(*distance) || *distance < 0.0) {
    my_error(ER_GIS_INVALID_DATA, MYF(0), "st_distance");
    return true;
  }

  return false;
}
}  // namespace gis

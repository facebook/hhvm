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
/// This file implements the covered_by functor and mbr_covered_by function.

#include <boost/geometry.hpp>

#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/box.h"
#include "sql/gis/box_traits.h"
#include "sql/gis/covered_by_functor.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_traits.h"
#include "sql/gis/mbr_utils.h"
#include "sql/gis/relops.h"
#include "sql/sql_exception_handler.h"  // handle_gis_exception

namespace bg = boost::geometry;

namespace gis {

Covered_by::Covered_by(double semi_major, double semi_minor)
    : m_semi_major(semi_major), m_semi_minor(semi_minor) {}

bool Covered_by::operator()(const Geometry *g1, const Geometry *g2) const {
  return apply(*this, g1, g2);
}

bool Covered_by::operator()(const Box *b1, const Box *b2) const {
  DBUG_ASSERT(b1->coordinate_system() == b2->coordinate_system());
  switch (b1->coordinate_system()) {
    case Coordinate_system::kCartesian:
      return eval(down_cast<const Cartesian_box *>(b1),
                  down_cast<const Cartesian_box *>(b2));
    case Coordinate_system::kGeographic:
      return eval(down_cast<const Geographic_box *>(b1),
                  down_cast<const Geographic_box *>(b2));
  }

  DBUG_ASSERT(false);
  return false;
}

bool Covered_by::eval(const Geometry *g1, const Geometry *g2) const {
  // Currently only implemented for boxes (MBRs).
  DBUG_ASSERT(false);
  throw not_implemented_exception::for_non_projected(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// covered_by(Box, Box)

bool Covered_by::eval(const Cartesian_box *b1, const Cartesian_box *b2) const {
  return bg::covered_by(*b1, *b2);
}

bool Covered_by::eval(const Geographic_box *b1,
                      const Geographic_box *b2) const {
  return bg::covered_by(*b1, *b2);
}

//////////////////////////////////////////////////////////////////////////////

bool mbr_covered_by(const dd::Spatial_reference_system *srs, const Geometry *g1,
                    const Geometry *g2, const char *func_name, bool *covered_by,
                    bool *null) noexcept {
  try {
    DBUG_ASSERT(g1->coordinate_system() == g2->coordinate_system());
    DBUG_ASSERT(srs == nullptr ||
                ((srs->is_cartesian() &&
                  g1->coordinate_system() == Coordinate_system::kCartesian) ||
                 (srs->is_geographic() &&
                  g1->coordinate_system() == Coordinate_system::kGeographic)));

    if ((*null = (g1->is_empty() || g2->is_empty()))) return false;

    Covered_by covered_by_func(srs ? srs->semi_major_axis() : 0.0,
                               srs ? srs->semi_minor_axis() : 0.0);

    switch (g1->coordinate_system()) {
      case Coordinate_system::kCartesian: {
        Cartesian_box mbr1;
        box_envelope(g1, srs, &mbr1);
        Cartesian_box mbr2;
        box_envelope(g2, srs, &mbr2);
        *covered_by = covered_by_func(&mbr1, &mbr2);
        break;
      }
      case Coordinate_system::kGeographic: {
        Geographic_box mbr1;
        box_envelope(g1, srs, &mbr1);
        Geographic_box mbr2;
        box_envelope(g2, srs, &mbr2);
        *covered_by = covered_by_func(&mbr1, &mbr2);
        break;
      }
    }
  } catch (...) {
    handle_gis_exception(func_name);
    return true;
  }

  return false;
}

}  // namespace gis

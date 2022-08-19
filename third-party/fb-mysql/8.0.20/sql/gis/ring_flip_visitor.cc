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

#include "sql/gis/ring_flip_visitor.h"

#include <boost/geometry.hpp>  // boost::geometry::correct

#include "sql/gis/geometries_cs.h"
#include "sql/gis/geometries_traits.h"
#include "template_utils.h"

namespace gis {

bool Ring_flip_visitor::visit_enter(Polygon *py) {
  if (!m_invalid) {
    try {
      switch (py->coordinate_system()) {
        case Coordinate_system::kCartesian:
          boost::geometry::correct(*down_cast<Cartesian_polygon *>(py));
          break;
        case Coordinate_system::kGeographic:
          boost::geometry::correct(*down_cast<Geographic_polygon *>(py),
                                   *m_geographic_strategy);
          break;
      }
    } catch (...) {
      m_invalid = true;
    }
  }
  return true;  // Don't descend into each ring.
}

bool Ring_flip_visitor::visit_enter(Multipolygon *mpy) {
  if (!m_invalid) {
    try {
      switch (mpy->coordinate_system()) {
        case Coordinate_system::kCartesian:
          boost::geometry::correct(*down_cast<Cartesian_multipolygon *>(mpy));
          break;
        case Coordinate_system::kGeographic:
          boost::geometry::correct(*down_cast<Geographic_multipolygon *>(mpy),
                                   *m_geographic_strategy);
          break;
      }
    } catch (...) {
      m_invalid = true;
    }
  }
  return true;  // Don't descend into each polygon.
}

}  // namespace gis

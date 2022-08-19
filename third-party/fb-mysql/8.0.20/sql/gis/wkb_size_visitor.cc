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

#include "wkb_size_visitor.h"

#include <cstdint>  // std::uint32_t, std::uint8_t

#include "my_dbug.h"  // DBUG_ASSERT

namespace gis {

bool Wkb_size_visitor::visit_enter(Geometry *) {
  /* purecov: begin deadcode */
  DBUG_ASSERT(false);
  return true;
  /* purecov: end */
}

bool Wkb_size_visitor::visit_enter(Linestring *ls) {
  m_size += sizeof(std::uint8_t) +            // Byte order
            sizeof(std::uint32_t) +           // WKB type
            sizeof(std::uint32_t) +           // numPoints
            ls->size() * 2 * sizeof(double);  // Point data;
  return true;  // Points included above, so don't descend into children.
}

bool Wkb_size_visitor::visit_enter(Linearring *lr) {
  m_size += sizeof(std::uint32_t) +           // numPoints
            lr->size() * 2 * sizeof(double);  // Point data;
  return true;  // Points included above, so don't descend into children.
}

bool Wkb_size_visitor::visit_enter(Polygon *) {
  m_size += sizeof(std::uint8_t) +   // Byte order
            sizeof(std::uint32_t) +  // WKB type
            sizeof(std::uint32_t);   // numRings
  return false;  // Descend into children to count rings sizes.
}

bool Wkb_size_visitor::visit_enter(Geometrycollection *) {
  m_size += sizeof(std::uint8_t) +   // Byte order
            sizeof(std::uint32_t) +  // WKB type
            sizeof(std::uint32_t);   // numGeometries
  return false;  // Descend into children to count them separately.
}

bool Wkb_size_visitor::visit(Point *) {
  m_size += sizeof(std::uint8_t) +   // Byte order
            sizeof(std::uint32_t) +  // WKB type
            2 * sizeof(double);      // Point data;
  return false;
}

}  // namespace gis

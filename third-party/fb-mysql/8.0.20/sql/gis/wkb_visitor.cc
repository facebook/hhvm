// Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.
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

#include "wkb_visitor.h"

#include <cstdint>

#include "my_byteorder.h"  // float8store, int4store

namespace gis {

bool Wkb_visitor::visit_enter(Linestring *ls) {
  *m_wkb_current_position = '\x01';  // Little-endian
  m_wkb_current_position++;
  int4store(m_wkb_current_position, static_cast<uint32>(ls->type()));
  m_wkb_current_position += sizeof(std::uint32_t);
  int4store(m_wkb_current_position, ls->size());
  m_wkb_current_position += sizeof(std::uint32_t);

  if (m_srs != nullptr && m_srs->is_geographic()) {
    for (size_t i = 0; i < ls->size(); i++) {
      float8store(m_wkb_current_position,
                  m_srs->from_normalized_longitude((*ls)[i].x()));
      m_wkb_current_position += sizeof(double);
      float8store(m_wkb_current_position,
                  m_srs->from_normalized_latitude((*ls)[i].y()));
      m_wkb_current_position += sizeof(double);
    }
  } else {
    for (size_t i = 0; i < ls->size(); i++) {
      float8store(m_wkb_current_position, (*ls)[i].x());
      m_wkb_current_position += sizeof(double);
      float8store(m_wkb_current_position, (*ls)[i].y());
      m_wkb_current_position += sizeof(double);
    }
  }
  DBUG_ASSERT(m_wkb + m_wkb_size >= m_wkb_current_position);
  return true;
}

bool Wkb_visitor::visit_enter(Polygon *py) {
  *m_wkb_current_position = '\x01';  // Little-endian
  m_wkb_current_position++;
  int4store(m_wkb_current_position, static_cast<uint32>(py->type()));
  m_wkb_current_position += sizeof(std::uint32_t);
  int4store(m_wkb_current_position, py->size());
  m_wkb_current_position += sizeof(std::uint32_t);

  for (size_t ix = 0; ix < py->size(); ix++) {
    Linearring &lr = ix == 0 ? py->exterior_ring() : py->interior_ring(ix - 1);
    int4store(m_wkb_current_position, lr.size());
    m_wkb_current_position += sizeof(std::uint32_t);
    if (m_srs != nullptr && m_srs->is_geographic()) {
      for (size_t i = 0; i < lr.size(); i++) {
        float8store(m_wkb_current_position,
                    m_srs->from_normalized_longitude(lr[i].x()));
        m_wkb_current_position += sizeof(double);
        float8store(m_wkb_current_position,
                    m_srs->from_normalized_latitude(lr[i].y()));
        m_wkb_current_position += sizeof(double);
      }
    } else {
      for (size_t i = 0; i < lr.size(); i++) {
        float8store(m_wkb_current_position, lr[i].x());
        m_wkb_current_position += sizeof(double);
        float8store(m_wkb_current_position, lr[i].y());
        m_wkb_current_position += sizeof(double);
      }
    }
  }
  DBUG_ASSERT(m_wkb + m_wkb_size >= m_wkb_current_position);
  return true;
}

bool Wkb_visitor::visit_enter(Geometrycollection *gc) {
  *m_wkb_current_position = '\x01';  // Little-endian
  m_wkb_current_position++;
  int4store(m_wkb_current_position, static_cast<uint32>(gc->type()));
  m_wkb_current_position += sizeof(std::uint32_t);
  int4store(m_wkb_current_position, gc->size());
  m_wkb_current_position += sizeof(std::uint32_t);
  DBUG_ASSERT(m_wkb + m_wkb_size >= m_wkb_current_position);
  return false;
}

bool Wkb_visitor::visit(Point *pt) {
  *m_wkb_current_position = '\x01';  // Little-endian
  m_wkb_current_position++;
  int4store(m_wkb_current_position, static_cast<uint32>(pt->type()));
  m_wkb_current_position += sizeof(std::uint32_t);
  if (m_srs != nullptr && m_srs->is_geographic()) {
    float8store(m_wkb_current_position,
                m_srs->from_normalized_longitude(pt->x()));
    m_wkb_current_position += sizeof(double);
    float8store(m_wkb_current_position,
                m_srs->from_normalized_latitude(pt->y()));
    m_wkb_current_position += sizeof(double);
  } else {
    float8store(m_wkb_current_position, pt->x());
    m_wkb_current_position += sizeof(double);
    float8store(m_wkb_current_position, pt->y());
    m_wkb_current_position += sizeof(double);
  }
  DBUG_ASSERT(m_wkb + m_wkb_size >= m_wkb_current_position);
  return false;
}

}  // namespace gis

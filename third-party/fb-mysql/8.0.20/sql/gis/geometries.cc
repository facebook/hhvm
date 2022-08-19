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

#include "sql/gis/geometries.h"

#include <utility>  // std::swap

#include "my_dbug.h"
#include "sql/gis/geometries_cs.h"
#include "sql/gis/geometry_visitor.h"

namespace gis {

bool Point::accept(Geometry_visitor *v) { return v->visit(this); }

template <>
double Point::get<0>() const {
  return m_x;
}

template <>
double Point::get<1>() const {
  return m_y;
}

double Point::x() const { return get<0>(); }

double Point::y() const { return get<1>(); }

template <>
void Point::set<0>(double d) {
  m_x = d;
}

template <>
void Point::set<1>(double d) {
  m_y = d;
}

void Point::x(double d) { set<0>(d); }

void Point::y(double d) { set<1>(d); }

bool Cartesian_linestring::accept(Geometry_visitor *v) {
  if (!v->visit_enter(this) && m_points.size() > 0) {
    if (m_points[0].accept(v)) return true;
    for (decltype(m_points)::size_type i = 1; i < m_points.size(); i++) {
      if (v->visit(this) || m_points[i].accept(v)) return true;
    }
  }
  return v->visit_leave(this);
}

void Cartesian_linestring::push_back(const Point &pt) {
  DBUG_ASSERT(pt.coordinate_system() == Coordinate_system::kCartesian);
  m_points.push_back(static_cast<const Cartesian_point &>(pt));
}

void Cartesian_linestring::push_back(Point &&pt) {
  DBUG_ASSERT(pt.coordinate_system() == Coordinate_system::kCartesian);
  m_points.push_back(static_cast<Cartesian_point &&>(pt));
}

bool Cartesian_linestring::empty() const { return m_points.empty(); }

bool Geographic_linestring::accept(Geometry_visitor *v) {
  if (!v->visit_enter(this) && m_points.size() > 0) {
    if (m_points[0].accept(v)) return true;
    for (decltype(m_points)::size_type i = 1; i < m_points.size(); i++) {
      if (v->visit(this) || m_points[i].accept(v)) return true;
    }
  }
  return v->visit_leave(this);
}

void Geographic_linestring::push_back(const Point &pt) {
  DBUG_ASSERT(pt.coordinate_system() == Coordinate_system::kGeographic);
  m_points.push_back(static_cast<const Geographic_point &>(pt));
}

void Geographic_linestring::push_back(Point &&pt) {
  DBUG_ASSERT(pt.coordinate_system() == Coordinate_system::kGeographic);
  m_points.push_back(static_cast<Geographic_point &&>(pt));
}

bool Geographic_linestring::empty() const { return m_points.empty(); }

bool Cartesian_linearring::accept(Geometry_visitor *v) {
  if (!v->visit_enter(this) && m_points.size() > 0) {
    if (m_points[0].accept(v)) return true;
    for (decltype(m_points)::size_type i = 1; i < m_points.size(); i++) {
      if (v->visit(this) || m_points[i].accept(v)) return true;
    }
  }
  return v->visit_leave(this);
}

bool Geographic_linearring::accept(Geometry_visitor *v) {
  if (!v->visit_enter(this) && m_points.size() > 0) {
    if (m_points[0].accept(v)) return true;
    for (decltype(m_points)::size_type i = 1; i < m_points.size(); i++) {
      if (v->visit(this) || m_points[i].accept(v)) return true;
    }
  }
  return v->visit_leave(this);
}

bool Cartesian_polygon::accept(Geometry_visitor *v) {
  if (!v->visit_enter(this)) {
    if (m_exterior_ring.accept(v)) return true;
    for (auto &&ring : m_interior_rings) {
      if (v->visit(this) || ring.accept(v)) return true;
    }
  }
  return v->visit_leave(this);
}

void Cartesian_polygon::push_back(const Linearring &lr) {
  DBUG_ASSERT(lr.coordinate_system() == Coordinate_system::kCartesian);
  if (m_exterior_ring.empty() && m_interior_rings.empty())
    m_exterior_ring = static_cast<const Cartesian_linearring &>(lr);
  else
    m_interior_rings.push_back(static_cast<const Cartesian_linearring &>(lr));
}

void Cartesian_polygon::push_back(Linearring &&lr) {
  DBUG_ASSERT(lr.coordinate_system() == Coordinate_system::kCartesian);
  if (m_exterior_ring.empty() && m_interior_rings.empty())
    m_exterior_ring = static_cast<Cartesian_linearring &&>(lr);
  else
    m_interior_rings.push_back(static_cast<Cartesian_linearring &&>(lr));
}

bool Cartesian_polygon::empty() const {
  return m_exterior_ring.empty() && m_interior_rings.empty();
}

std::size_t Cartesian_polygon::size() const {
  std::size_t sz = m_interior_rings.size();
  if (!m_exterior_ring.empty()) sz++;
  return sz;
}

Linearring &Cartesian_polygon::interior_ring(std::size_t n) {
  return m_interior_rings[n];
}

bool Geographic_polygon::accept(Geometry_visitor *v) {
  if (!v->visit_enter(this)) {
    if (m_exterior_ring.accept(v)) return true;
    for (auto &&ring : m_interior_rings) {
      if (v->visit(this) || ring.accept(v)) return true;
    }
  }
  return v->visit_leave(this);
}

Cartesian_linearring &Cartesian_polygon::cartesian_exterior_ring() const {
  return const_cast<Cartesian_linearring &>(m_exterior_ring);
}

// Doxygen doesn't understand decltype as it is used here.
#ifndef IN_DOXYGEN

decltype(Cartesian_polygon::m_interior_rings) &
Cartesian_polygon::interior_rings() {
  return m_interior_rings;
}

decltype(Cartesian_polygon::m_interior_rings) const &
Cartesian_polygon::const_interior_rings() const {
  return m_interior_rings;
}

#endif  // IN_DOXYGEN

void Geographic_polygon::push_back(const Linearring &lr) {
  DBUG_ASSERT(lr.coordinate_system() == Coordinate_system::kGeographic);
  if (m_exterior_ring.empty() && m_interior_rings.empty())
    m_exterior_ring = static_cast<const Geographic_linearring &>(lr);
  else
    m_interior_rings.push_back(static_cast<const Geographic_linearring &>(lr));
}

void Geographic_polygon::push_back(Linearring &&lr) {
  DBUG_ASSERT(lr.coordinate_system() == Coordinate_system::kGeographic);
  if (m_exterior_ring.empty() && m_interior_rings.empty())
    m_exterior_ring = static_cast<Geographic_linearring &&>(lr);
  else
    m_interior_rings.push_back(static_cast<Geographic_linearring &&>(lr));
}

bool Geographic_polygon::empty() const {
  return m_exterior_ring.empty() && m_interior_rings.empty();
}

std::size_t Geographic_polygon::size() const {
  std::size_t sz = m_interior_rings.size();
  if (!m_exterior_ring.empty()) sz++;
  return sz;
}

Geographic_linearring &Geographic_polygon::geographic_exterior_ring() const {
  return const_cast<Geographic_linearring &>(m_exterior_ring);
}

Linearring &Geographic_polygon::interior_ring(std::size_t n) {
  return m_interior_rings[n];
}

// Doxygen doesn't understand decltype as it is used here.
#ifndef IN_DOXYGEN

decltype(Geographic_polygon::m_interior_rings) &
Geographic_polygon::interior_rings() {
  return m_interior_rings;
}

decltype(Geographic_polygon::m_interior_rings) const &
Geographic_polygon::const_interior_rings() const {
  return m_interior_rings;
}

#endif  // IN_DOXYGEN

Cartesian_geometrycollection::Cartesian_geometrycollection(
    const Cartesian_geometrycollection &gc)
    : m_geometries(
          Malloc_allocator<Geometry *>(key_memory_Geometry_objects_data)) {
  for (Geometry *g : gc.m_geometries) {
    switch (g->type()) {
      case Geometry_type::kPoint:
        m_geometries.push_back(
            new Cartesian_point(*static_cast<Cartesian_point *>(g)));
        break;
      case Geometry_type::kLinestring:
        m_geometries.push_back(
            new Cartesian_linestring(*static_cast<Cartesian_linestring *>(g)));
        break;
      case Geometry_type::kPolygon:
        m_geometries.push_back(
            new Cartesian_polygon(*static_cast<Cartesian_polygon *>(g)));
        break;
      case Geometry_type::kGeometrycollection:
        m_geometries.push_back(new Cartesian_geometrycollection(
            *static_cast<Cartesian_geometrycollection *>(g)));
        break;
      case Geometry_type::kMultipoint:
        m_geometries.push_back(
            new Cartesian_multipoint(*static_cast<Cartesian_multipoint *>(g)));
        break;
      case Geometry_type::kMultilinestring:
        m_geometries.push_back(new Cartesian_multilinestring(
            *static_cast<Cartesian_multilinestring *>(g)));
        break;
      case Geometry_type::kMultipolygon:
        m_geometries.push_back(new Cartesian_multipolygon(
            *static_cast<Cartesian_multipolygon *>(g)));
        break;
      default:
        DBUG_ASSERT(false); /* purecov: inspected */
    }
  }
}

bool Cartesian_geometrycollection::accept(Geometry_visitor *v) {
  if (!v->visit_enter(this) && m_geometries.size() > 0) {
    if (m_geometries[0]->accept(v)) return true;
    for (decltype(m_geometries)::size_type i = 1; i < m_geometries.size();
         i++) {
      if (v->visit(this) || m_geometries[i]->accept(v)) return true;
    }
  }
  return v->visit_leave(this);
}

void Cartesian_geometrycollection::push_back(const Geometry &g) {
  switch (g.type()) {
    case Geometry_type::kPoint:
      m_geometries.push_back(
          new Cartesian_point(static_cast<const Cartesian_point &>(g)));
      break;
    case Geometry_type::kLinestring:
      m_geometries.push_back(new Cartesian_linestring(
          static_cast<const Cartesian_linestring &>(g)));
      break;
    case Geometry_type::kPolygon:
      m_geometries.push_back(
          new Cartesian_polygon(static_cast<const Cartesian_polygon &>(g)));
      break;
    case Geometry_type::kGeometrycollection:
      m_geometries.push_back(new Cartesian_geometrycollection(
          static_cast<const Cartesian_geometrycollection &>(g)));
      break;
    case Geometry_type::kMultipoint:
      m_geometries.push_back(new Cartesian_multipoint(
          static_cast<const Cartesian_multipoint &>(g)));
      break;
    case Geometry_type::kMultilinestring:
      m_geometries.push_back(new Cartesian_multilinestring(
          static_cast<const Cartesian_multilinestring &>(g)));
      break;
    case Geometry_type::kMultipolygon:
      m_geometries.push_back(new Cartesian_multipolygon(
          static_cast<const Cartesian_multipolygon &>(g)));
      break;
    default:
      DBUG_ASSERT(false); /* purecov: inspected */
  }
}

void Cartesian_geometrycollection::push_back(Geometry &&g) {
  switch (g.type()) {
    case Geometry_type::kPoint:
      m_geometries.push_back(
          new Cartesian_point(static_cast<Cartesian_point &&>(g)));
      break;
    case Geometry_type::kLinestring:
      m_geometries.push_back(
          new Cartesian_linestring(static_cast<Cartesian_linestring &&>(g)));
      break;
    case Geometry_type::kPolygon:
      m_geometries.push_back(
          new Cartesian_polygon(static_cast<Cartesian_polygon &&>(g)));
      break;
    case Geometry_type::kGeometrycollection:
      m_geometries.push_back(new Cartesian_geometrycollection(
          static_cast<Cartesian_geometrycollection &&>(g)));
      break;
    case Geometry_type::kMultipoint:
      m_geometries.push_back(
          new Cartesian_multipoint(static_cast<Cartesian_multipoint &&>(g)));
      break;
    case Geometry_type::kMultilinestring:
      m_geometries.push_back(new Cartesian_multilinestring(
          static_cast<Cartesian_multilinestring &&>(g)));
      break;
    case Geometry_type::kMultipolygon:
      m_geometries.push_back(new Cartesian_multipolygon(
          static_cast<Cartesian_multipolygon &&>(g)));
      break;
    default:
      DBUG_ASSERT(false); /* purecov: inspected */
  }
}

bool Cartesian_geometrycollection::empty() const {
  return m_geometries.empty();
}

Geographic_geometrycollection::Geographic_geometrycollection(
    const Geographic_geometrycollection &gc)
    : m_geometries(
          Malloc_allocator<Geometry *>(key_memory_Geometry_objects_data)) {
  for (Geometry *g : gc.m_geometries) {
    switch (g->type()) {
      case Geometry_type::kPoint:
        m_geometries.push_back(
            new Geographic_point(*static_cast<Geographic_point *>(g)));
        break;
      case Geometry_type::kLinestring:
        m_geometries.push_back(new Geographic_linestring(
            *static_cast<Geographic_linestring *>(g)));
        break;
      case Geometry_type::kPolygon:
        m_geometries.push_back(
            new Geographic_polygon(*static_cast<Geographic_polygon *>(g)));
        break;
      case Geometry_type::kGeometrycollection:
        m_geometries.push_back(new Geographic_geometrycollection(
            *static_cast<Geographic_geometrycollection *>(g)));
        break;
      case Geometry_type::kMultipoint:
        m_geometries.push_back(new Geographic_multipoint(
            *static_cast<Geographic_multipoint *>(g)));
        break;
      case Geometry_type::kMultilinestring:
        m_geometries.push_back(new Geographic_multilinestring(
            *static_cast<Geographic_multilinestring *>(g)));
        break;
      case Geometry_type::kMultipolygon:
        m_geometries.push_back(new Geographic_multipolygon(
            *static_cast<Geographic_multipolygon *>(g)));
        break;
      default:
        DBUG_ASSERT(false); /* purecov: inspected */
    }
  }
}

bool Geographic_geometrycollection::accept(Geometry_visitor *v) {
  if (!v->visit_enter(this) && m_geometries.size() > 0) {
    if (m_geometries[0]->accept(v)) return true;
    for (decltype(m_geometries)::size_type i = 1; i < m_geometries.size();
         i++) {
      if (v->visit(this) || m_geometries[i]->accept(v)) return true;
    }
  }
  return v->visit_leave(this);
}

void Geographic_geometrycollection::push_back(const Geometry &g) {
  switch (g.type()) {
    case Geometry_type::kPoint:
      m_geometries.push_back(
          new Geographic_point(static_cast<const Geographic_point &>(g)));
      break;
    case Geometry_type::kLinestring:
      m_geometries.push_back(new Geographic_linestring(
          static_cast<const Geographic_linestring &>(g)));
      break;
    case Geometry_type::kPolygon:
      m_geometries.push_back(
          new Geographic_polygon(static_cast<const Geographic_polygon &>(g)));
      break;
    case Geometry_type::kGeometrycollection:
      m_geometries.push_back(new Geographic_geometrycollection(
          static_cast<const Geographic_geometrycollection &>(g)));
      break;
    case Geometry_type::kMultipoint:
      m_geometries.push_back(new Geographic_multipoint(
          static_cast<const Geographic_multipoint &>(g)));
      break;
    case Geometry_type::kMultilinestring:
      m_geometries.push_back(new Geographic_multilinestring(
          static_cast<const Geographic_multilinestring &>(g)));
      break;
    case Geometry_type::kMultipolygon:
      m_geometries.push_back(new Geographic_multipolygon(
          static_cast<const Geographic_multipolygon &>(g)));
      break;
    default:
      DBUG_ASSERT(false); /* purecov: inspected */
  }
}

void Geographic_geometrycollection::push_back(Geometry &&g) {
  switch (g.type()) {
    case Geometry_type::kPoint:
      m_geometries.push_back(
          new Geographic_point(static_cast<Geographic_point &&>(g)));
      break;
    case Geometry_type::kLinestring:
      m_geometries.push_back(
          new Geographic_linestring(static_cast<Geographic_linestring &&>(g)));
      break;
    case Geometry_type::kPolygon:
      m_geometries.push_back(
          new Geographic_polygon(static_cast<Geographic_polygon &&>(g)));
      break;
    case Geometry_type::kGeometrycollection:
      m_geometries.push_back(new Geographic_geometrycollection(
          static_cast<Geographic_geometrycollection &&>(g)));
      break;
    case Geometry_type::kMultipoint:
      m_geometries.push_back(
          new Geographic_multipoint(static_cast<Geographic_multipoint &&>(g)));
      break;
    case Geometry_type::kMultilinestring:
      m_geometries.push_back(new Geographic_multilinestring(
          static_cast<Geographic_multilinestring &&>(g)));
      break;
    case Geometry_type::kMultipolygon:
      m_geometries.push_back(new Geographic_multipolygon(
          static_cast<Geographic_multipolygon &&>(g)));
      break;
    default:
      DBUG_ASSERT(false); /* purecov: inspected */
  }
}

bool Geographic_geometrycollection::empty() const {
  return m_geometries.empty();
}

bool Cartesian_multipoint::accept(Geometry_visitor *v) {
  if (!v->visit_enter(this) && m_points.size() > 0) {
    if (m_points[0].accept(v)) return true;
    for (decltype(m_points)::size_type i = 1; i < m_points.size(); i++) {
      if (v->visit(this) || m_points[i].accept(v)) return true;
    }
  }
  return v->visit_leave(this);
}

void Cartesian_multipoint::push_back(const Geometry &pt) {
  DBUG_ASSERT(pt.coordinate_system() == Coordinate_system::kCartesian);
  m_points.push_back(static_cast<const Cartesian_point &>(pt));
}

void Cartesian_multipoint::push_back(Geometry &&pt) {
  DBUG_ASSERT(pt.coordinate_system() == Coordinate_system::kCartesian);
  m_points.push_back(static_cast<Cartesian_point &&>(pt));
}

bool Cartesian_multipoint::empty() const { return m_points.empty(); }

bool Geographic_multipoint::accept(Geometry_visitor *v) {
  if (!v->visit_enter(this) && m_points.size() > 0) {
    if (m_points[0].accept(v)) return true;
    for (decltype(m_points)::size_type i = 1; i < m_points.size(); i++) {
      if (v->visit(this) || m_points[i].accept(v)) return true;
    }
  }
  return v->visit_leave(this);
}

void Geographic_multipoint::push_back(const Geometry &pt) {
  DBUG_ASSERT(pt.coordinate_system() == Coordinate_system::kGeographic);
  m_points.push_back(static_cast<const Geographic_point &>(pt));
}

void Geographic_multipoint::push_back(Geometry &&pt) {
  DBUG_ASSERT(pt.coordinate_system() == Coordinate_system::kGeographic);
  m_points.push_back(static_cast<Geographic_point &&>(pt));
}

bool Geographic_multipoint::empty() const { return m_points.empty(); }

bool Cartesian_multilinestring::accept(Geometry_visitor *v) {
  if (!v->visit_enter(this) && m_linestrings.size() > 0) {
    if (m_linestrings[0].accept(v)) return true;
    for (decltype(m_linestrings)::size_type i = 1; i < m_linestrings.size();
         i++) {
      if (v->visit(this) || m_linestrings[i].accept(v)) return true;
    }
  }
  return v->visit_leave(this);
}

void Cartesian_multilinestring::push_back(const Geometry &ls) {
  DBUG_ASSERT(ls.coordinate_system() == Coordinate_system::kCartesian);
  m_linestrings.push_back(static_cast<const Cartesian_linestring &>(ls));
}

void Cartesian_multilinestring::push_back(Geometry &&ls) {
  DBUG_ASSERT(ls.coordinate_system() == Coordinate_system::kCartesian);
  m_linestrings.push_back(static_cast<Cartesian_linestring &&>(ls));
}

bool Cartesian_multilinestring::empty() const { return m_linestrings.empty(); }

bool Geographic_multilinestring::accept(Geometry_visitor *v) {
  if (!v->visit_enter(this) && m_linestrings.size() > 0) {
    if (m_linestrings[0].accept(v)) return true;
    for (decltype(m_linestrings)::size_type i = 1; i < m_linestrings.size();
         i++) {
      if (v->visit(this) || m_linestrings[i].accept(v)) return true;
    }
  }
  return v->visit_leave(this);
}

void Geographic_multilinestring::push_back(const Geometry &ls) {
  DBUG_ASSERT(ls.coordinate_system() == Coordinate_system::kGeographic);
  m_linestrings.push_back(static_cast<const Geographic_linestring &>(ls));
}

void Geographic_multilinestring::push_back(Geometry &&ls) {
  DBUG_ASSERT(ls.coordinate_system() == Coordinate_system::kGeographic);
  m_linestrings.push_back(static_cast<Geographic_linestring &&>(ls));
}

bool Geographic_multilinestring::empty() const { return m_linestrings.empty(); }

bool Cartesian_multipolygon::accept(Geometry_visitor *v) {
  if (!v->visit_enter(this) && m_polygons.size() > 0) {
    if (m_polygons[0].accept(v)) return true;
    for (decltype(m_polygons)::size_type i = 1; i < m_polygons.size(); i++) {
      if (v->visit(this) || m_polygons[i].accept(v)) return true;
    }
  }
  return v->visit_leave(this);
}

void Cartesian_multipolygon::push_back(const Geometry &py) {
  DBUG_ASSERT(py.coordinate_system() == Coordinate_system::kCartesian);
  m_polygons.push_back(static_cast<const Cartesian_polygon &>(py));
}

void Cartesian_multipolygon::push_back(Geometry &&py) {
  DBUG_ASSERT(py.coordinate_system() == Coordinate_system::kCartesian);
  m_polygons.push_back(static_cast<Cartesian_polygon &&>(py));
}

bool Cartesian_multipolygon::empty() const { return m_polygons.empty(); }

bool Geographic_multipolygon::accept(Geometry_visitor *v) {
  if (!v->visit_enter(this) && m_polygons.size() > 0) {
    if (m_polygons[0].accept(v)) return true;
    for (decltype(m_polygons)::size_type i = 1; i < m_polygons.size(); i++) {
      if (v->visit(this) || m_polygons[i].accept(v)) return true;
    }
  }
  return v->visit_leave(this);
}

void Geographic_multipolygon::push_back(const Geometry &py) {
  DBUG_ASSERT(py.coordinate_system() == Coordinate_system::kGeographic);
  m_polygons.push_back(static_cast<const Geographic_polygon &>(py));
}

void Geographic_multipolygon::push_back(Geometry &&py) {
  DBUG_ASSERT(py.coordinate_system() == Coordinate_system::kGeographic);
  m_polygons.push_back(static_cast<Geographic_polygon &&>(py));
}

bool Geographic_multipolygon::empty() const { return m_polygons.empty(); }

const char *type_to_name(Geometry_type type) {
  switch (type) {
    case Geometry_type::kPoint:
      return "POINT";
    case Geometry_type::kLinestring:
      return "LINESTRING";
    case Geometry_type::kPolygon:
      return "POLYGON";
    case Geometry_type::kGeometrycollection:
      return "GEOMCOLLECTION";
    case Geometry_type::kMultipoint:
      return "MULTIPOINT";
    case Geometry_type::kMultilinestring:
      return "MULTILINESTRING";
    case Geometry_type::kMultipolygon:
      return "MULTIPOLYGON";
    default:
      /* purecov: begin inspected */
      DBUG_ASSERT(false);
      return "UNKNOWN";
      /* purecov: end */
  }
}

}  // namespace gis

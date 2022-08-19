#ifndef SQL_GIS_WKB_SIZE_VISITOR_H_INCLUDED
#define SQL_GIS_WKB_SIZE_VISITOR_H_INCLUDED

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

#include "sql/gis/geometry_visitor.h"

#include <cstddef>  // std::size_t

namespace gis {

/// A visitor that computes the size of a WKB representation of a geometry.
class Wkb_size_visitor : public Nop_visitor {
 private:
  /// The size of the geometry.
  std::size_t m_size;

 public:
  /// Construct a new WKB visitor.
  Wkb_size_visitor() : m_size(0) {}
  std::size_t size() const { return m_size; }

  using Nop_visitor::visit_enter;
  bool visit_enter(Geometry *g) override;
  bool visit_enter(Linestring *ls) override;
  bool visit_enter(Linearring *lr) override;
  bool visit_enter(Polygon *py) override;
  bool visit_enter(Geometrycollection *gc) override;

  using Nop_visitor::visit;
  bool visit(Point *pt) override;
};

}  // namespace gis

#endif  // SQL_GIS_WKB_SIZE_VISITOR_H_INCLUDED

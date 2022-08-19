#ifndef SQL_GIS_WKB_VISITOR_H_INCLUDED
#define SQL_GIS_WKB_VISITOR_H_INCLUDED

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

#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/geometry_visitor.h"

namespace gis {

/// A visitor that serializes the geometry to little-endian WKB and appends it
/// to a string.
class Wkb_visitor : public Nop_visitor {
 private:
  /// Spatial reference system of the geometry.
  const dd::Spatial_reference_system *m_srs;
  /// The WKB string.
  const char *m_wkb;
  /// Size of the buffer allocated for the WKB string.
  const size_t m_wkb_size;
  /// The position of the next character to be added to the string.
  char *m_wkb_current_position;

 public:
  /// Construct a new WKB visitor.
  ///
  /// @param[in] srs The spatial reference system of the geometry.
  /// @param[in] wkb The string to write the WKB to.
  /// @param[in] wkb_size The length of the WKB string buffer.
  Wkb_visitor(const dd::Spatial_reference_system *srs, char *wkb,
              size_t wkb_size)
      : m_srs(srs),
        m_wkb(wkb),
        m_wkb_size(wkb_size),
        m_wkb_current_position(wkb) {}

  using Nop_visitor::visit_enter;
  bool visit_enter(Geometry *) override {
    /* purecov: begin deadcode */
    DBUG_ASSERT(false);
    return true;
    /* purecov: end */
  }
  bool visit_enter(Linestring *ls) override;
  bool visit_enter(Polygon *py) override;
  bool visit_enter(Geometrycollection *gc) override;

  using Nop_visitor::visit;
  bool visit(Point *pt) override;
};

}  // namespace gis

#endif  // SQL_GIS_WKB_VISITOR_H_INCLUDED

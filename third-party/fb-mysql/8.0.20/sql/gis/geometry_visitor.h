#ifndef SQL_GIS_GEOMETRY_VISITOR_H_INCLUDED
#define SQL_GIS_GEOMETRY_VISITOR_H_INCLUDED

// Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
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
/// The geometries implement a hierarchical visitor pattern. This file declares
/// the interface for visitors.

#include "sql/gis/geometries.h"

namespace gis {

/// Abstract visitor class to be used on class Geometry and descendants.
///
/// A visitor will visit all elements of a compound geometry, always going down
/// to each point unless the geometry is empty. E.g., when visiting a
/// linestring, the visitor will descend into each point of the linestring.
///
/// The visitor can abort execution at any time. This is done by returning true
/// from a visit() or visit_leave() function. If these functions return false,
/// execution will continue. The accept() member function on geometries returns
/// true if the visitor aborte exectuion and false otherwise.
class Geometry_visitor {
 public:
  Geometry_visitor() {}
  virtual ~Geometry_visitor() {}

  /// Enters a compound geometry.
  ///
  /// This is called on entry to a compound geometry, i.e., all
  /// geometries except points.
  ///
  /// @param g The geometry to visit.
  ///
  /// @retval true Don't descend into children.
  /// @retval false Descend into children.
  virtual bool visit_enter(Geometry *g) = 0;
  virtual bool visit_enter(Curve *) = 0;
  virtual bool visit_enter(Linestring *) = 0;
  virtual bool visit_enter(Linearring *) = 0;
  virtual bool visit_enter(Surface *) = 0;
  virtual bool visit_enter(Polygon *) = 0;
  virtual bool visit_enter(Geometrycollection *) = 0;
  virtual bool visit_enter(Multipoint *) = 0;
  virtual bool visit_enter(Multicurve *) = 0;
  virtual bool visit_enter(Multilinestring *) = 0;
  virtual bool visit_enter(Multisurface *) = 0;
  virtual bool visit_enter(Multipolygon *) = 0;

  /// Visits a geometry.
  ///
  /// This is called on each non-compound geometry and between visiting
  /// descendants. E.g., visit(Linestring *) will be called after visiting the
  /// first point in the linestring, then after visiting the second, etc., but
  /// not after visiting the last point.
  ///
  /// @param g The geometry to visit.
  ///
  /// @retval true Abort visitor execution.
  /// @retval false Continue visitor execution.
  virtual bool visit(Geometry *g) = 0;
  virtual bool visit(Point *) = 0;
  virtual bool visit(Curve *) = 0;
  virtual bool visit(Linestring *) = 0;
  virtual bool visit(Linearring *) = 0;
  virtual bool visit(Surface *) = 0;
  virtual bool visit(Polygon *) = 0;
  virtual bool visit(Geometrycollection *) = 0;
  virtual bool visit(Multipoint *) = 0;
  virtual bool visit(Multicurve *) = 0;
  virtual bool visit(Multilinestring *) = 0;
  virtual bool visit(Multisurface *) = 0;
  virtual bool visit(Multipolygon *) = 0;

  /// Leaves a compound geometry.
  ///
  /// Called after visiting the last child of a compound geometry. The return
  /// value is returned to the accept() function.
  ///
  /// @param g The geometry to visit.
  ///
  /// @retval true Abort visitor execution.
  /// @retval false Continue visitor execution.
  virtual bool visit_leave(Geometry *g) = 0;
  virtual bool visit_leave(Curve *) = 0;
  virtual bool visit_leave(Linestring *) = 0;
  virtual bool visit_leave(Linearring *) = 0;
  virtual bool visit_leave(Surface *) = 0;
  virtual bool visit_leave(Polygon *) = 0;
  virtual bool visit_leave(Geometrycollection *) = 0;
  virtual bool visit_leave(Multipoint *) = 0;
  virtual bool visit_leave(Multicurve *) = 0;
  virtual bool visit_leave(Multilinestring *) = 0;
  virtual bool visit_leave(Multisurface *) = 0;
  virtual bool visit_leave(Multipolygon *) = 0;
};

/// A visitor that implements the entire interface and does nothing.
class Nop_visitor : public Geometry_visitor {
 public:
  virtual bool visit_enter(Geometry *) override { return false; }
  virtual bool visit_enter(Curve *c) override {
    return visit_enter(static_cast<Geometry *>(c));
  }
  virtual bool visit_enter(Linestring *ls) override {
    return visit_enter(static_cast<Curve *>(ls));
  }
  virtual bool visit_enter(Linearring *lr) override {
    return visit_enter(static_cast<Linestring *>(lr));
  }
  virtual bool visit_enter(Surface *s) override {
    return visit_enter(static_cast<Geometry *>(s));
  }
  virtual bool visit_enter(Polygon *py) override {
    return visit_enter(static_cast<Surface *>(py));
  }
  virtual bool visit_enter(Geometrycollection *gc) override {
    return visit_enter(static_cast<Geometry *>(gc));
  }
  virtual bool visit_enter(Multipoint *mpt) override {
    return visit_enter(static_cast<Geometrycollection *>(mpt));
  }
  virtual bool visit_enter(Multicurve *mc) override {
    return visit_enter(static_cast<Geometrycollection *>(mc));
  }
  virtual bool visit_enter(Multilinestring *mls) override {
    return visit_enter(static_cast<Multicurve *>(mls));
  }
  virtual bool visit_enter(Multisurface *ms) override {
    return visit_enter(static_cast<Geometrycollection *>(ms));
  }
  virtual bool visit_enter(Multipolygon *mpy) override {
    return visit_enter(static_cast<Multisurface *>(mpy));
  }

  virtual bool visit(Geometry *) override { return false; }
  virtual bool visit(Point *pt) override {
    return visit(static_cast<Geometry *>(pt));
  }
  virtual bool visit(Curve *c) override {
    return visit(static_cast<Geometry *>(c));
  }
  virtual bool visit(Linestring *ls) override {
    return visit(static_cast<Curve *>(ls));
  }
  virtual bool visit(Linearring *lr) override {
    return visit(static_cast<Linestring *>(lr));
  }
  virtual bool visit(Surface *s) override {
    return visit(static_cast<Geometry *>(s));
  }
  virtual bool visit(Polygon *py) override {
    return visit(static_cast<Surface *>(py));
  }
  virtual bool visit(Geometrycollection *gc) override {
    return visit(static_cast<Geometry *>(gc));
  }
  virtual bool visit(Multipoint *mpt) override {
    return visit(static_cast<Geometrycollection *>(mpt));
  }
  virtual bool visit(Multicurve *mc) override {
    return visit(static_cast<Geometrycollection *>(mc));
  }
  virtual bool visit(Multilinestring *mls) override {
    return visit(static_cast<Multicurve *>(mls));
  }
  virtual bool visit(Multisurface *ms) override {
    return visit(static_cast<Geometrycollection *>(ms));
  }
  virtual bool visit(Multipolygon *mpy) override {
    return visit(static_cast<Multisurface *>(mpy));
  }

  virtual bool visit_leave(Geometry *) override { return false; }
  virtual bool visit_leave(Curve *c) override {
    return visit_leave(static_cast<Geometry *>(c));
  }
  virtual bool visit_leave(Linestring *ls) override {
    return visit_leave(static_cast<Curve *>(ls));
  }
  virtual bool visit_leave(Linearring *lr) override {
    return visit_leave(static_cast<Linestring *>(lr));
  }
  virtual bool visit_leave(Surface *s) override {
    return visit_leave(static_cast<Geometry *>(s));
  }
  virtual bool visit_leave(Polygon *py) override {
    return visit_leave(static_cast<Surface *>(py));
  }
  virtual bool visit_leave(Geometrycollection *gc) override {
    return visit_leave(static_cast<Geometry *>(gc));
  }
  virtual bool visit_leave(Multipoint *mpt) override {
    return visit_leave(static_cast<Geometrycollection *>(mpt));
  }
  virtual bool visit_leave(Multicurve *mc) override {
    return visit_leave(static_cast<Geometrycollection *>(mc));
  }
  virtual bool visit_leave(Multilinestring *mls) override {
    return visit_leave(static_cast<Multicurve *>(mls));
  }
  virtual bool visit_leave(Multisurface *ms) override {
    return visit_leave(static_cast<Geometrycollection *>(ms));
  }
  virtual bool visit_leave(Multipolygon *mpy) override {
    return visit_leave(static_cast<Multisurface *>(mpy));
  }
};

}  // namespace gis

#endif  // SQL_GIS_GEOMETRY_VISITOR_H_INCLUDED

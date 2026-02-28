#ifndef SQL_GIS_CROSSES_FUNCTOR_H_INCLUDED
#define SQL_GIS_CROSSES_FUNCTOR_H_INCLUDED

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
/// This file declares the crosses functor interface.
///
/// The functor is not intended for use directly by MySQL code. It should be
/// used indirectly through the gis::crosses() function.
///
/// @see gis::crosses

#include <boost/geometry.hpp>

#include "sql/gis/functor.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_traits.h"

namespace gis {

/// Crosses functor that calls Boost.Geometry with the correct parameter
/// types.
///
/// The functor throws exceptions and is therefore only intended used to
/// implement crosses or other geographic functions. It should not be used
/// directly by other MySQL code.
class Crosses : public Functor<bool> {
 private:
  /// Semi-major axis of ellipsoid.
  double m_semi_major;
  /// Semi-minor axis of ellipsoid.
  double m_semi_minor;
  /// Strategy used for P/L and P/A.
  boost::geometry::strategy::within::geographic_winding<Geographic_point>
      m_geographic_pl_pa_strategy;
  /// Strategy used for L/L, L/A and A/A.
  boost::geometry::strategy::intersection::geographic_segments<>
      m_geographic_ll_la_aa_strategy;

 public:
  /// Creates a new Crosses functor.
  ///
  /// @param semi_major Semi-major axis of ellipsoid.
  /// @param semi_minor Semi-minor axis of ellipsoid.
  Crosses(double semi_major, double semi_minor);
  double semi_major() const { return m_semi_major; }
  double semi_minor() const { return m_semi_minor; }
  bool operator()(const Geometry *g1, const Geometry *g2) const override;
  bool eval(const Geometry *g1, const Geometry *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // crosses(Cartesian_point, *)

  bool eval(const Cartesian_point *g1, const Cartesian_point *g2) const;
  bool eval(const Cartesian_point *g1, const Cartesian_linestring *g2) const;
  bool eval(const Cartesian_point *g1, const Cartesian_polygon *g2) const;
  bool eval(const Cartesian_point *g1,
            const Cartesian_geometrycollection *g2) const;
  bool eval(const Cartesian_point *g1, const Cartesian_multipoint *g2) const;
  bool eval(const Cartesian_point *g1,
            const Cartesian_multilinestring *g2) const;
  bool eval(const Cartesian_point *g1, const Cartesian_multipolygon *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // crosses(Cartesian_linestring, *)

  bool eval(const Cartesian_linestring *g1, const Cartesian_point *g2) const;
  bool eval(const Cartesian_linestring *g1,
            const Cartesian_linestring *g2) const;
  bool eval(const Cartesian_linestring *g1, const Cartesian_polygon *g2) const;
  bool eval(const Cartesian_linestring *g1,
            const Cartesian_geometrycollection *g2) const;
  bool eval(const Cartesian_linestring *g1,
            const Cartesian_multipoint *g2) const;
  bool eval(const Cartesian_linestring *g1,
            const Cartesian_multilinestring *g2) const;
  bool eval(const Cartesian_linestring *g1,
            const Cartesian_multipolygon *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // crosses(Cartesian_polygon, *)

  bool eval(const Cartesian_polygon *g1, const Geometry *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // crosses(Cartesian_geometrycollection, *)

  bool eval(const Cartesian_geometrycollection *g1, const Geometry *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // crosses(Cartesian_multipoint, *)

  bool eval(const Cartesian_multipoint *g1, const Cartesian_point *g2) const;
  bool eval(const Cartesian_multipoint *g1,
            const Cartesian_linestring *g2) const;
  bool eval(const Cartesian_multipoint *g1, const Cartesian_polygon *g2) const;
  bool eval(const Cartesian_multipoint *g1,
            const Cartesian_geometrycollection *g2) const;
  bool eval(const Cartesian_multipoint *g1,
            const Cartesian_multipoint *g2) const;
  bool eval(const Cartesian_multipoint *g1,
            const Cartesian_multilinestring *g2) const;
  bool eval(const Cartesian_multipoint *g1,
            const Cartesian_multipolygon *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // crosses(Cartesian_multilinestring, *)

  bool eval(const Cartesian_multilinestring *g1,
            const Cartesian_point *g2) const;
  bool eval(const Cartesian_multilinestring *g1,
            const Cartesian_linestring *g2) const;
  bool eval(const Cartesian_multilinestring *g1,
            const Cartesian_polygon *g2) const;
  bool eval(const Cartesian_multilinestring *g1,
            const Cartesian_geometrycollection *g2) const;
  bool eval(const Cartesian_multilinestring *g1,
            const Cartesian_multipoint *g2) const;
  bool eval(const Cartesian_multilinestring *g1,
            const Cartesian_multilinestring *g2) const;
  bool eval(const Cartesian_multilinestring *g1,
            const Cartesian_multipolygon *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // crosses(Cartesian_multipolygon, *)

  bool eval(const Cartesian_multipolygon *g1, const Geometry *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // crosses(Geographic_point, *)

  bool eval(const Geographic_point *g1, const Geographic_point *g2) const;
  bool eval(const Geographic_point *g1, const Geographic_linestring *g2) const;
  bool eval(const Geographic_point *g1, const Geographic_polygon *g2) const;
  bool eval(const Geographic_point *g1,
            const Geographic_geometrycollection *g2) const;
  bool eval(const Geographic_point *g1, const Geographic_multipoint *g2) const;
  bool eval(const Geographic_point *g1,
            const Geographic_multilinestring *g2) const;
  bool eval(const Geographic_point *g1,
            const Geographic_multipolygon *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // crosses(Geographic_linestring, *)

  bool eval(const Geographic_linestring *g1, const Geographic_point *g2) const;
  bool eval(const Geographic_linestring *g1,
            const Geographic_linestring *g2) const;
  bool eval(const Geographic_linestring *g1,
            const Geographic_polygon *g2) const;
  bool eval(const Geographic_linestring *g1,
            const Geographic_geometrycollection *g2) const;
  bool eval(const Geographic_linestring *g1,
            const Geographic_multipoint *g2) const;
  bool eval(const Geographic_linestring *g1,
            const Geographic_multilinestring *g2) const;
  bool eval(const Geographic_linestring *g1,
            const Geographic_multipolygon *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // crosses(Geographic_polygon, *)

  bool eval(const Geographic_polygon *g1, const Geometry *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // crosses(Geographic_geometrycollection, *)

  bool eval(const Geographic_geometrycollection *g1, const Geometry *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // crosses(Geographic_multipoint, *)

  bool eval(const Geographic_multipoint *g1, const Geographic_point *g2) const;
  bool eval(const Geographic_multipoint *g1,
            const Geographic_linestring *g2) const;
  bool eval(const Geographic_multipoint *g1,
            const Geographic_polygon *g2) const;
  bool eval(const Geographic_multipoint *g1,
            const Geographic_geometrycollection *g2) const;
  bool eval(const Geographic_multipoint *g1,
            const Geographic_multipoint *g2) const;
  bool eval(const Geographic_multipoint *g1,
            const Geographic_multilinestring *g2) const;
  bool eval(const Geographic_multipoint *g1,
            const Geographic_multipolygon *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // crosses(Geographic_multilinestring, *)

  bool eval(const Geographic_multilinestring *g1,
            const Geographic_point *g2) const;
  bool eval(const Geographic_multilinestring *g1,
            const Geographic_linestring *g2) const;
  bool eval(const Geographic_multilinestring *g1,
            const Geographic_polygon *g2) const;
  bool eval(const Geographic_multilinestring *g1,
            const Geographic_geometrycollection *g2) const;
  bool eval(const Geographic_multilinestring *g1,
            const Geographic_multipoint *g2) const;
  bool eval(const Geographic_multilinestring *g1,
            const Geographic_multilinestring *g2) const;
  bool eval(const Geographic_multilinestring *g1,
            const Geographic_multipolygon *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // crosses(Geographic_multipolygon, *)

  bool eval(const Geographic_multipolygon *g1, const Geometry *g2) const;
};

}  // namespace gis

#endif  // SQL_GIS_CROSSES_FUNCTOR_H_INCLUDED

#ifndef SQL_GIS_DISTANCE_FUNCTOR_H_INCLUDED
#define SQL_GIS_DISTANCE_FUNCTOR_H_INCLUDED

// Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
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
/// This file declares the distance functor interface.
///
/// The functor is not intended for use directly by MySQL code. It should be
/// used indirectly through the gis::distance() function.
///
/// @see gis::distance

#include <memory>  // std::unique_ptr

#include <boost/geometry.hpp>

#include "sql/gis/functor.h"
#include "sql/gis/geometries.h"

namespace gis {

/// Distance functor that calls Boost.Geometry with the correct parameter types.
///
/// The functor throws exceptions and is therefore only intended used to
/// implement distance or other geographic functions. It should not be used
/// directly by other MySQL code.
class Distance : public Functor<double> {
 private:
  std::unique_ptr<boost::geometry::strategy::distance::andoyer<
      boost::geometry::srs::spheroid<double>>>
      m_geographic_strategy_pp;
  std::unique_ptr<boost::geometry::strategy::distance::geographic_cross_track<
      boost::geometry::strategy::andoyer,
      boost::geometry::srs::spheroid<double>, double>>
      m_geographic_strategy_non_pp;

 public:
  Distance(double major, double minor);
  double operator()(const Geometry *g1, const Geometry *g2) const override;
  double eval(const Geometry *g1, const Geometry *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // distance(Cartesian_point, *)

  double eval(const Cartesian_point *g1, const Cartesian_point *g2) const;
  double eval(const Cartesian_point *g1, const Cartesian_linestring *g2) const;
  double eval(const Cartesian_point *g1, const Cartesian_polygon *g2) const;
  double eval(const Cartesian_point *g1,
              const Cartesian_geometrycollection *g2) const;
  double eval(const Cartesian_point *g1, const Cartesian_multipoint *g2) const;
  double eval(const Cartesian_point *g1,
              const Cartesian_multilinestring *g2) const;
  double eval(const Cartesian_point *g1,
              const Cartesian_multipolygon *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // distance(Cartesian_linestring, *)

  double eval(const Cartesian_linestring *g1, const Cartesian_point *g2) const;
  double eval(const Cartesian_linestring *g1,
              const Cartesian_linestring *g2) const;
  double eval(const Cartesian_linestring *g1,
              const Cartesian_polygon *g2) const;
  double eval(const Cartesian_linestring *g1,
              const Cartesian_geometrycollection *g2) const;
  double eval(const Cartesian_linestring *g1,
              const Cartesian_multipoint *g2) const;
  double eval(const Cartesian_linestring *g1,
              const Cartesian_multilinestring *g2) const;
  double eval(const Cartesian_linestring *g1,
              const Cartesian_multipolygon *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // distance(Cartesian_polygon, *)

  double eval(const Cartesian_polygon *g1, const Cartesian_point *g2) const;
  double eval(const Cartesian_polygon *g1,
              const Cartesian_linestring *g2) const;
  double eval(const Cartesian_polygon *g1, const Cartesian_polygon *g2) const;
  double eval(const Cartesian_polygon *g1,
              const Cartesian_geometrycollection *g2) const;
  double eval(const Cartesian_polygon *g1,
              const Cartesian_multipoint *g2) const;
  double eval(const Cartesian_polygon *g1,
              const Cartesian_multilinestring *g2) const;
  double eval(const Cartesian_polygon *g1,
              const Cartesian_multipolygon *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // distance(Cartesian_geometrycollection, *)

  double eval(const Cartesian_geometrycollection *g1, const Geometry *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // distance(Cartesian_multipoint, *)

  double eval(const Cartesian_multipoint *g1, const Cartesian_point *g2) const;
  double eval(const Cartesian_multipoint *g1,
              const Cartesian_linestring *g2) const;
  double eval(const Cartesian_multipoint *g1,
              const Cartesian_polygon *g2) const;
  double eval(const Cartesian_multipoint *g1,
              const Cartesian_geometrycollection *g2) const;
  double eval(const Cartesian_multipoint *g1,
              const Cartesian_multipoint *g2) const;
  double eval(const Cartesian_multipoint *g1,
              const Cartesian_multilinestring *g2) const;
  double eval(const Cartesian_multipoint *g1,
              const Cartesian_multipolygon *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // distance(Cartesian_multilinestring, *)

  double eval(const Cartesian_multilinestring *g1,
              const Cartesian_point *g2) const;
  double eval(const Cartesian_multilinestring *g1,
              const Cartesian_linestring *g2) const;
  double eval(const Cartesian_multilinestring *g1,
              const Cartesian_polygon *g2) const;
  double eval(const Cartesian_multilinestring *g1,
              const Cartesian_geometrycollection *g2) const;
  double eval(const Cartesian_multilinestring *g1,
              const Cartesian_multipoint *g2) const;
  double eval(const Cartesian_multilinestring *g1,
              const Cartesian_multilinestring *g2) const;
  double eval(const Cartesian_multilinestring *g1,
              const Cartesian_multipolygon *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // distance(Cartesian_multipolygon, *)

  double eval(const Cartesian_multipolygon *g1,
              const Cartesian_point *g2) const;
  double eval(const Cartesian_multipolygon *g1,
              const Cartesian_linestring *g2) const;
  double eval(const Cartesian_multipolygon *g1,
              const Cartesian_polygon *g2) const;
  double eval(const Cartesian_multipolygon *g1,
              const Cartesian_geometrycollection *g2) const;
  double eval(const Cartesian_multipolygon *g1,
              const Cartesian_multipoint *g2) const;
  double eval(const Cartesian_multipolygon *g1,
              const Cartesian_multilinestring *g2) const;
  double eval(const Cartesian_multipolygon *g1,
              const Cartesian_multipolygon *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // distance(Geographic_point, *)

  double eval(const Geographic_point *g1, const Geographic_point *g2) const;
  double eval(const Geographic_point *g1,
              const Geographic_linestring *g2) const;
  double eval(const Geographic_point *g1, const Geographic_polygon *g2) const;
  double eval(const Geographic_point *g1,
              const Geographic_multipoint *g2) const;
  double eval(const Geographic_point *g1,
              const Geographic_multilinestring *g2) const;
  double eval(const Geographic_point *g1,
              const Geographic_multipolygon *g2) const;
  double eval(const Geographic_point *g1,
              const Geographic_geometrycollection *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // distance(Geographic_linestring, *)

  double eval(const Geographic_linestring *g1,
              const Geographic_point *g2) const;
  double eval(const Geographic_linestring *g1,
              const Geographic_linestring *g2) const;
  double eval(const Geographic_linestring *g1,
              const Geographic_polygon *g2) const;
  double eval(const Geographic_linestring *g1,
              const Geographic_multipoint *g2) const;
  double eval(const Geographic_linestring *g1,
              const Geographic_multilinestring *g2) const;
  double eval(const Geographic_linestring *g1,
              const Geographic_multipolygon *g2) const;
  double eval(const Geographic_linestring *g1,
              const Geographic_geometrycollection *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // distance(Geographic_polygon, *)

  double eval(const Geographic_polygon *g1, const Geographic_point *g2) const;
  double eval(const Geographic_polygon *g1,
              const Geographic_linestring *g2) const;
  double eval(const Geographic_polygon *g1, const Geographic_polygon *g2) const;
  double eval(const Geographic_polygon *g1,
              const Geographic_multipoint *g2) const;
  double eval(const Geographic_polygon *g1,
              const Geographic_multilinestring *g2) const;
  double eval(const Geographic_polygon *g1,
              const Geographic_multipolygon *g2) const;
  double eval(const Geographic_polygon *g1,
              const Geographic_geometrycollection *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // distance(Geographic_multipoint, *)

  double eval(const Geographic_multipoint *g1,
              const Geographic_point *g2) const;
  double eval(const Geographic_multipoint *g1,
              const Geographic_linestring *g2) const;
  double eval(const Geographic_multipoint *g1,
              const Geographic_polygon *g2) const;
  double eval(const Geographic_multipoint *g1,
              const Geographic_multipoint *g2) const;
  double eval(const Geographic_multipoint *g1,
              const Geographic_multilinestring *g2) const;
  double eval(const Geographic_multipoint *g1,
              const Geographic_multipolygon *g2) const;
  double eval(const Geographic_multipoint *g1,
              const Geographic_geometrycollection *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // distance(Geographic_multilinestring, *)

  double eval(const Geographic_multilinestring *g1,
              const Geographic_point *g2) const;
  double eval(const Geographic_multilinestring *g1,
              const Geographic_linestring *g2) const;
  double eval(const Geographic_multilinestring *g1,
              const Geographic_polygon *g2) const;
  double eval(const Geographic_multilinestring *g1,
              const Geographic_multipoint *g2) const;
  double eval(const Geographic_multilinestring *g1,
              const Geographic_multilinestring *g2) const;
  double eval(const Geographic_multilinestring *g1,
              const Geographic_multipolygon *g2) const;
  double eval(const Geographic_multilinestring *g1,
              const Geographic_geometrycollection *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // distance(Geographic_multipolygon, *)

  double eval(const Geographic_multipolygon *g1,
              const Geographic_point *g2) const;
  double eval(const Geographic_multipolygon *g1,
              const Geographic_linestring *g2) const;
  double eval(const Geographic_multipolygon *g1,
              const Geographic_polygon *g2) const;
  double eval(const Geographic_multipolygon *g1,
              const Geographic_multipoint *g2) const;
  double eval(const Geographic_multipolygon *g1,
              const Geographic_multilinestring *g2) const;
  double eval(const Geographic_multipolygon *g1,
              const Geographic_multipolygon *g2) const;
  double eval(const Geographic_multipolygon *g1,
              const Geographic_geometrycollection *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // distance(Geographic_geometrycollection, *)

  double eval(const Geographic_geometrycollection *g1,
              const Geometry *g2) const;
};

}  // namespace gis

#endif  // SQL_GIS_DISTANCE_FUNCTOR_H_INCLUDED

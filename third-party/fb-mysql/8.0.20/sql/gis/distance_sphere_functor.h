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
/// Declares the Distance_sphere functor interface.
///
/// The functor is not intended for use directly by MySQL code. It should be
/// used indirectly through the gis::distance_sphere() function.
///
/// @see gis::distance_sphere

#ifndef SQL_GIS_DISTANCE_SPHERE_FUNCTOR_H_INCLUDED
#define SQL_GIS_DISTANCE_SPHERE_FUNCTOR_H_INCLUDED

#include <boost/geometry.hpp>

#include "sql/gis/functor.h"
#include "sql/gis/geometries.h"     // gis::Geometry
#include "sql/gis/geometries_cs.h"  // gis::{Cartesian_*, Geographic_*}

namespace gis {

/// Functor that calls Boost.Geometry with the correct parameter types.
///
/// The functor throws exceptions and is therefore only intended used to
/// implement geographic functions. It should not be used directly by other
/// MySQL code.
class Distance_sphere : public Functor<double> {
  boost::geometry::strategy::distance::haversine<double> m_strategy;

 public:
  Distance_sphere(double sphere_radius) : m_strategy{sphere_radius} {}

  double operator()(const Geometry *g1, const Geometry *g2) const override;

  double eval(const Cartesian_point *g1, const Cartesian_point *g2) const;
  double eval(const Cartesian_point *g1, const Cartesian_multipoint *g2) const;
  double eval(const Cartesian_multipoint *g1, const Cartesian_point *g2) const;
  double eval(const Cartesian_multipoint *g1,
              const Cartesian_multipoint *g2) const;

  double eval(const Geographic_point *g1, const Geographic_point *g2) const;
  double eval(const Geographic_point *g1,
              const Geographic_multipoint *g2) const;
  double eval(const Geographic_multipoint *g1,
              const Geographic_point *g2) const;
  double eval(const Geographic_multipoint *g1,
              const Geographic_multipoint *g2) const;

  double eval(const Geometry *g1, const Geometry *g2) const;
};

}  // namespace gis

#endif  // SQL_GIS_DISTANCE_SPHERE_FUNCTOR_H_INCLUDED

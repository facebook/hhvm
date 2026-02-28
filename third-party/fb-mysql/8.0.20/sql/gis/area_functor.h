// Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License, version 2.0,
//  as published by the Free Software Foundation.
//
//  This program is also distributed with certain software (including
//  but not limited to OpenSSL) that is licensed under separate terms,
//  as designated in a particular file or component or in included license
//  documentation.  The authors of MySQL hereby grant you an additional
//  permission to link the program and your derivative works with the
//  separately licensed software that they have included with MySQL.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License, version 2.0, for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

/// @file
///
/// Declares the area functor interface.
///
/// The functor is not intended for use directly by MySQL code. It should be
/// used indirectly through the gis::area() function.
///
/// @see gis::area

#ifndef SQL_GIS_AREA_FUNCTOR_H_INCLUDED
#define SQL_GIS_AREA_FUNCTOR_H_INCLUDED

#include <boost/geometry.hpp>  // boost::geometry

#include "sql/gis/functor.h"        // gis::Unary_functor
#include "sql/gis/geometries.h"     // gis::Geometry
#include "sql/gis/geometries_cs.h"  // gis::{Cartesian_*,Geographic_*}

namespace gis {

/// Area functor that calls boost::geometry::area with the correct parameter
/// types.
///
/// The functor may throw exceptions. It is intended for implementing geographic
/// functions. It should not be used directly by other MySQL code.
class Area : public Unary_functor<double> {
  double m_semi_major;
  double m_semi_minor;

  boost::geometry::strategy::area::geographic<> m_geographic_strategy;

 public:
  Area();
  Area(double semi_major, double semi_minor);

  double operator()(const Geometry &g) const override;

  double eval(const Cartesian_polygon &g) const;
  double eval(const Cartesian_multipolygon &g) const;

  double eval(const Geographic_polygon &g) const;
  double eval(const Geographic_multipolygon &g) const;

  double eval(const Geometry &g) const;
};

}  // namespace gis

#endif  // SQL_GIS_AREA_FUNCTOR_H_INCLUDED

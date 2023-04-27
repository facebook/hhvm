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
/// Declares the is_simple functor interface.
///
/// The functor is not intended for use directly by MySQL code. It should be
/// used indirectly through the gis::is_simple() function.
///
/// @see gis::is_simple

#ifndef SQL_GIS_IS_SIMPLE_FUNCTOR_H_INCLUDED
#define SQL_GIS_IS_SIMPLE_FUNCTOR_H_INCLUDED

#include <boost/geometry.hpp>

#include "sql/gis/functor.h"        // gis::Unary_functor
#include "sql/gis/geometries.h"     // gis::Geometry
#include "sql/gis/geometries_cs.h"  // gis::{Cartesian_*, Geographic_*}

namespace gis {

/// Is_simple functor calls boost::geometry::is_simple with the correct
/// parameter types.
///
/// The functor may throw exceptions. It is intended for implementing geographic
/// functions. It should not be used directly by other MySQL code.
class Is_simple : public Unary_functor<bool> {
  boost::geometry::strategy::intersection::geographic_segments<> m_geostrat;

  // For instantiating gis::Intersects when evaluating a Geometrycollection.
  double m_semi_major;
  double m_semi_minor;

 public:
  Is_simple(double semi_major, double semi_minor);

  bool operator()(const Geometry &g) const override;

  bool eval(const Cartesian_point &g) const;
  bool eval(const Cartesian_linestring &g) const;
  bool eval(const Cartesian_polygon &g) const;
  bool eval(const Cartesian_multipoint &g) const;
  bool eval(const Cartesian_multipolygon &g) const;
  bool eval(const Cartesian_multilinestring &g) const;

  bool eval(const Geographic_point &g) const;
  bool eval(const Geographic_linestring &g) const;
  bool eval(const Geographic_polygon &g) const;
  bool eval(const Geographic_multipoint &g) const;
  bool eval(const Geographic_multipolygon &g) const;
  bool eval(const Geographic_multilinestring &g) const;

  bool eval(const Geometrycollection &g) const;
};

}  // namespace gis

#endif  // include guard

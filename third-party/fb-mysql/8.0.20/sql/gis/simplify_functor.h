#ifndef SQL_GIS_SIMPLIFY_FUNCTOR_H_INCLUDED
#define SQL_GIS_SIMPLIFY_FUNCTOR_H_INCLUDED

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

/// @file
///
/// This file declares the simplify functor interface.
///
/// The functor is not intended for use directly by MySQL code. It should be
/// used indirectly through the gis::simplify() function.
///
/// @see gis::simplify

#include <memory>  // std::unique_ptr

#include <boost/geometry.hpp>

#include "sql/gis/functor.h"
#include "sql/gis/geometries.h"

namespace gis {

/// Simplify functor that calls Boost.Geometry with the correct parameter types.
///
/// The functor throws exceptions and is therefore only intended used to
/// implement simplify or other geographic functions. It should not be used
/// directly by other MySQL code.
class Simplify : public Unary_functor<std::unique_ptr<Geometry>> {
 private:
  double m_max_distance;

 public:
  Simplify(double max_distance) : m_max_distance(max_distance) {}
  std::unique_ptr<Geometry> operator()(const Geometry &g) const override;
  std::unique_ptr<Geometry> eval(const Geometry &g) const;
  std::unique_ptr<Geometry> eval(const Cartesian_point &g) const;
  std::unique_ptr<Geometry> eval(const Cartesian_linestring &g) const;
  std::unique_ptr<Geometry> eval(const Cartesian_polygon &g) const;
  std::unique_ptr<Geometry> eval(const Cartesian_geometrycollection &g) const;
  std::unique_ptr<Geometry> eval(const Cartesian_multipoint &g) const;
  std::unique_ptr<Geometry> eval(const Cartesian_multilinestring &g) const;
  std::unique_ptr<Geometry> eval(const Cartesian_multipolygon &g) const;
};

}  // namespace gis

#endif  // SQL_GIS_SIMPLIFY_FUNCTOR_H_INCLUDED

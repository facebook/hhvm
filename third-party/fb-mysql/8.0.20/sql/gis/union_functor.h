#ifndef SQL_GIS_UNION_FUNCTOR_H_INCLUDED
#define SQL_GIS_UNION_FUNCTOR_H_INCLUDED

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
/// This file declares the union functor interface.
///
/// The functor is not intended for use directly by MySQL code. It should be
/// used indirectly through the gis::union() function.
///
/// @see gis::union

#include <boost/geometry.hpp>

#include "sql/gis/functor.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_traits.h"

namespace gis {

/// Union functor that calls Boost.Geometry with the correct parameter
/// types.
///
/// The functor throws exceptions and is therefore only intended used to
/// implement union or other geographic functions. It should not be used
/// directly by other MySQL code.
class Union : public Functor<Geometry *> {
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
  Union(double semi_major, double semi_minor);
  Geometry *operator()(const Geometry *g1, const Geometry *g2) const override;
  Geometry *eval(const Geometry *g1, const Geometry *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // union(Cartesian_multilinestring, *)

  Geometry *eval(const Cartesian_multilinestring *g1,
                 const Cartesian_linestring *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // union(Cartesian_multipolygon, *)

  Geometry *eval(const Cartesian_multipolygon *g1,
                 const Cartesian_polygon *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // union(Geographic_multilinestring, *)

  Geometry *eval(const Geographic_multilinestring *g1,
                 const Geographic_linestring *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // union(Geographic_multipolygon, *)

  Geometry *eval(const Geographic_multipolygon *g1,
                 const Geographic_polygon *g2) const;
};

}  // namespace gis

#endif  // SQL_GIS_UNION_FUNCTOR_H_INCLUDED

#ifndef SQL_GIS_BOX_TRAITS_H_INCLUDED
#define SQL_GIS_BOX_TRAITS_H_INCLUDED

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
/// This file contains Boost.Geometry type traits declarations for Cartesian and
/// geographic boxes.
///
/// @see box.h

#include <boost/geometry/geometries/concepts/box_concept.hpp>

#include "sql/gis/box.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_cs.h"
#include "sql/gis/geometries_traits.h"  // To get fully defined traits.

namespace boost {
namespace geometry {
namespace traits {

////////////////////////////////////////////////////////////////////////////////

// Cartesian

template <>
struct tag<gis::Cartesian_box> {
  typedef box_tag type;
};

template <>
struct point_type<gis::Cartesian_box> {
  typedef gis::Cartesian_point type;
};

template <std::size_t Dimension>
struct indexed_access<gis::Cartesian_box, min_corner, Dimension> {
  static inline double get(gis::Cartesian_box const &b) {
    return b.min_corner().get<Dimension>();
  }

  static inline void set(gis::Cartesian_box &b, double const &value) {
    b.min_corner().set<Dimension>(value);
  }
};

template <std::size_t Dimension>
struct indexed_access<gis::Cartesian_box, max_corner, Dimension> {
  static inline double get(gis::Cartesian_box const &b) {
    return b.max_corner().get<Dimension>();
  }

  static inline void set(gis::Cartesian_box &b, double const &value) {
    b.max_corner().set<Dimension>(value);
  }
};

////////////////////////////////////////////////////////////////////////////////

// Geographic

template <>
struct tag<gis::Geographic_box> {
  typedef box_tag type;
};

template <>
struct point_type<gis::Geographic_box> {
  typedef gis::Geographic_point type;
};

template <std::size_t Dimension>
struct indexed_access<gis::Geographic_box, min_corner, Dimension> {
  static inline double get(gis::Geographic_box const &b) {
    return b.min_corner().get<Dimension>();
  }

  static inline void set(gis::Geographic_box &b, double const &value) {
    b.min_corner().set<Dimension>(value);
  }
};

template <std::size_t Dimension>
struct indexed_access<gis::Geographic_box, max_corner, Dimension> {
  static inline double get(gis::Geographic_box const &b) {
    return b.max_corner().get<Dimension>();
  }

  static inline void set(gis::Geographic_box &b, double const &value) {
    b.max_corner().set<Dimension>(value);
  }
};

}  // namespace traits
}  // namespace geometry
}  // namespace boost

#endif  // SQL_GIS_BOX_H_INCLUDED

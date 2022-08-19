#ifndef SQL_GIS_GEOMETRIES_TRAITS_H_INCLUDED
#define SQL_GIS_GEOMETRIES_TRAITS_H_INCLUDED

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
/// This file contains Boost.Geometry type traits declarations for Cartesian and
/// geographic geometries.
///
/// @see geometries_cs.h

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/interior_type.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/geometries/concepts/linestring_concept.hpp>
#include <boost/geometry/geometries/concepts/point_concept.hpp>
#include <boost/geometry/geometries/concepts/polygon_concept.hpp>
#include <boost/geometry/multi/core/tags.hpp>

#include "sql/gis/geometries_cs.h"
#include "sql/malloc_allocator.h"

namespace boost {
namespace geometry {
namespace traits {

////////////////////////////////////////////////////////////////////////////////

// Cartesian

// Point

template <>
struct tag<gis::Cartesian_point> {
  typedef boost::geometry::point_tag type;
};

template <>
struct coordinate_type<gis::Cartesian_point> {
  typedef double type;
};

template <>
struct coordinate_system<gis::Cartesian_point> {
  typedef boost::geometry::cs::cartesian type;
};

template <>
struct dimension<gis::Cartesian_point> : boost::mpl::int_<2> {};

template <std::size_t Dimension>
struct access<gis::Cartesian_point, Dimension> {
  static inline double get(gis::Cartesian_point const &p) {
    return p.get<Dimension>();
  }

  static inline void set(gis::Cartesian_point &p, double const &value) {
    p.set<Dimension>(value);
  }
};

// Linestring

template <>
struct tag<gis::Cartesian_linestring> {
  typedef boost::geometry::linestring_tag type;
};

// Linearring

template <>
struct tag<gis::Cartesian_linearring> {
  typedef boost::geometry::ring_tag type;
};

template <>
struct point_order<gis::Cartesian_linearring> {
  static const order_selector value = counterclockwise;
};

template <>
struct closure<gis::Cartesian_linearring> {
  static const closure_selector value = closed;
};

// Polygon

template <>
struct tag<gis::Cartesian_polygon> {
  typedef boost::geometry::polygon_tag type;
};

template <>
struct ring_const_type<gis::Cartesian_polygon> {
  typedef gis::Cartesian_linearring const &type;
};

template <>
struct ring_mutable_type<gis::Cartesian_polygon> {
  typedef gis::Cartesian_linearring &type;
};

template <>
struct interior_const_type<gis::Cartesian_polygon> {
  typedef std::vector<gis::Cartesian_linearring,
                      Malloc_allocator<gis::Cartesian_linearring>> const &type;
};

template <>
struct interior_mutable_type<gis::Cartesian_polygon> {
  typedef std::vector<gis::Cartesian_linearring,
                      Malloc_allocator<gis::Cartesian_linearring>> &type;
};

template <>
struct exterior_ring<gis::Cartesian_polygon> {
  static inline gis::Cartesian_linearring &get(gis::Cartesian_polygon &py) {
    return py.cartesian_exterior_ring();
  }

  static inline gis::Cartesian_linearring const &get(
      gis::Cartesian_polygon const &py) {
    return py.cartesian_exterior_ring();
  }
};

template <>
struct interior_rings<gis::Cartesian_polygon> {
  static inline std::vector<gis::Cartesian_linearring,
                            Malloc_allocator<gis::Cartesian_linearring>>
      &get(gis::Cartesian_polygon &py) {
    return py.interior_rings();
  }

  static inline std::vector<gis::Cartesian_linearring,
                            Malloc_allocator<gis::Cartesian_linearring>> const &
  get(gis::Cartesian_polygon const &py) {
    return py.const_interior_rings();
  }
};

// Multipoint

template <>
struct tag<gis::Cartesian_multipoint> {
  typedef boost::geometry::multi_point_tag type;
};

// Multilinestring

template <>
struct tag<gis::Cartesian_multilinestring> {
  typedef boost::geometry::multi_linestring_tag type;
};

// Multipolygon

template <>
struct tag<gis::Cartesian_multipolygon> {
  typedef boost::geometry::multi_polygon_tag type;
};

////////////////////////////////////////////////////////////////////////////////

// Geographic

// Point

template <>
struct tag<gis::Geographic_point> {
  typedef boost::geometry::point_tag type;
};

template <>
struct coordinate_type<gis::Geographic_point> {
  typedef double type;
};

template <>
struct coordinate_system<gis::Geographic_point> {
  typedef boost::geometry::cs::geographic<radian> type;
};

template <>
struct dimension<gis::Geographic_point> : boost::mpl::int_<2> {};

template <std::size_t Dimension>
struct access<gis::Geographic_point, Dimension> {
  static inline double get(gis::Geographic_point const &p) {
    return p.get<Dimension>();
  }

  static inline void set(gis::Geographic_point &p, double const &value) {
    p.set<Dimension>(value);
  }
};

// Linestring

template <>
struct tag<gis::Geographic_linestring> {
  typedef boost::geometry::linestring_tag type;
};

// Linearring

template <>
struct tag<gis::Geographic_linearring> {
  typedef boost::geometry::ring_tag type;
};

template <>
struct point_order<gis::Geographic_linearring> {
  static const order_selector value = counterclockwise;
};

template <>
struct closure<gis::Geographic_linearring> {
  static const closure_selector value = closed;
};

// Polygon

template <>
struct tag<gis::Geographic_polygon> {
  typedef boost::geometry::polygon_tag type;
};

template <>
struct ring_const_type<gis::Geographic_polygon> {
  typedef gis::Geographic_linearring const &type;
};

template <>
struct ring_mutable_type<gis::Geographic_polygon> {
  typedef gis::Geographic_linearring &type;
};

template <>
struct interior_const_type<gis::Geographic_polygon> {
  typedef std::vector<gis::Geographic_linearring,
                      Malloc_allocator<gis::Geographic_linearring>> const &type;
};

template <>
struct interior_mutable_type<gis::Geographic_polygon> {
  typedef std::vector<gis::Geographic_linearring,
                      Malloc_allocator<gis::Geographic_linearring>> &type;
};

template <>
struct exterior_ring<gis::Geographic_polygon> {
  static inline gis::Geographic_linearring &get(gis::Geographic_polygon &py) {
    return py.geographic_exterior_ring();
  }

  static inline gis::Geographic_linearring const &get(
      gis::Geographic_polygon const &py) {
    return py.geographic_exterior_ring();
  }
};

template <>
struct interior_rings<gis::Geographic_polygon> {
  static inline std::vector<gis::Geographic_linearring,
                            Malloc_allocator<gis::Geographic_linearring>>
      &get(gis::Geographic_polygon &py) {
    return py.interior_rings();
  }

  static inline std::vector<gis::Geographic_linearring,
                            Malloc_allocator<gis::Geographic_linearring>> const
      &
      get(gis::Geographic_polygon const &py) {
    return py.const_interior_rings();
  }
};

// Multipoint

template <>
struct tag<gis::Geographic_multipoint> {
  typedef boost::geometry::multi_point_tag type;
};

// Multilinestring

template <>
struct tag<gis::Geographic_multilinestring> {
  typedef boost::geometry::multi_linestring_tag type;
};

// Multipolygon

template <>
struct tag<gis::Geographic_multipolygon> {
  typedef boost::geometry::multi_polygon_tag type;
};

}  // namespace traits
}  // namespace geometry
}  // namespace boost

#endif  // SQL_GIS_GEOMETRIES_TRAITS_H_INCLUDED

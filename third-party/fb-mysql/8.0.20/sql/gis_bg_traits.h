#ifndef GIS_BG_TRAITS_INCLUDED
#define GIS_BG_TRAITS_INCLUDED

/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

/* This file defines all boost geometry traits. */

#include <boost/concept/requires.hpp>
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
#include <boost/geometry/util/math.hpp>
#include <boost/static_assert.hpp>

#include "sql/spatial.h"

// Boost Geometry traits.
namespace boost {
namespace geometry {
namespace traits {
template <>
struct tag<Gis_point> {
  typedef boost::geometry::point_tag type;
};

template <>
struct coordinate_type<Gis_point> {
  typedef double type;
};

template <>
struct coordinate_system<Gis_point> {
  typedef boost::geometry::cs::cartesian type;
};

template <>
struct dimension<Gis_point> : boost::mpl::int_<GEOM_DIM> {};

template <std::size_t Dimension>
struct access<Gis_point, Dimension> {
  static inline double get(Gis_point const &p) { return p.get<Dimension>(); }

  static inline void set(Gis_point &p, double const &value) {
    p.set<Dimension>(value);
  }
};

////////////////////////////////// LINESTRING ////////////////////////////
template <>
struct tag<Gis_line_string> {
  typedef boost::geometry::linestring_tag type;
};

////////////////////////////////// POLYGON //////////////////////////////////

template <>
struct tag<Gis_polygon> {
  typedef boost::geometry::polygon_tag type;
};

template <>
struct ring_const_type<Gis_polygon> {
  typedef Gis_polygon::ring_type const &type;
};

template <>
struct ring_mutable_type<Gis_polygon> {
  typedef Gis_polygon::ring_type &type;
};

template <>
struct interior_const_type<Gis_polygon> {
  typedef Gis_polygon::inner_container_type const &type;
};

template <>
struct interior_mutable_type<Gis_polygon> {
  typedef Gis_polygon::inner_container_type &type;
};

template <>
struct exterior_ring<Gis_polygon> {
  typedef Gis_polygon polygon_type;

  static inline polygon_type::ring_type &get(polygon_type &p) {
    return p.outer();
  }

  static inline polygon_type::ring_type const &get(polygon_type const &p) {
    return p.outer();
  }
};

template <>
struct interior_rings<Gis_polygon> {
  typedef Gis_polygon polygon_type;

  static inline polygon_type::inner_container_type &get(polygon_type &p) {
    return p.inners();
  }

  static inline polygon_type::inner_container_type const &get(
      polygon_type const &p) {
    return p.inners();
  }
};

////////////////////////////////// RING //////////////////////////////////
template <>
struct point_order<Gis_polygon_ring> {
  static const order_selector value = counterclockwise;
};

template <>
struct closure<Gis_polygon_ring> {
  static const closure_selector value = closed;
};

template <>
struct tag<Gis_polygon_ring> {
  typedef boost::geometry::ring_tag type;
};

////////////////////////////////// MULTI GEOMETRIES /////////////////////////

/////////////////////////////////// multi linestring types /////////////////////
template <>
struct tag<Gis_multi_line_string> {
  typedef boost::geometry::multi_linestring_tag type;
};

/////////////////////////////////// multi point types /////////////////////

template <>
struct tag<Gis_multi_point> {
  typedef boost::geometry::multi_point_tag type;
};

/////////////////////////////////// multi polygon types /////////////////////
template <>
struct tag<Gis_multi_polygon> {
  typedef boost::geometry::multi_polygon_tag type;
};

}  // namespace traits

}  // namespace geometry
}  // namespace boost

#endif  // !GIS_BG_TRAITS_INCLUDED

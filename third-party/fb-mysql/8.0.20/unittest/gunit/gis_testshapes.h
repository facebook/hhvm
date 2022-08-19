#ifndef UNITTEST_GUNIT_GIS_TESTSHAPES_H_INCLUDED
#define UNITTEST_GUNIT_GIS_TESTSHAPES_H_INCLUDED

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

#include <exception>
#include <vector>
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_cs.h"

struct Cartesian_types {
  typedef gis::Cartesian_point Point;
  typedef gis::Cartesian_linestring Linestring;
  typedef gis::Cartesian_linearring Linearring;
  typedef gis::Cartesian_polygon Polygon;
  typedef gis::Cartesian_geometrycollection Geometrycollection;
  typedef gis::Cartesian_multipoint Multipoint;
  typedef gis::Cartesian_multilinestring Multilinestring;
  typedef gis::Cartesian_multipolygon Multipolygon;

  static gis::Coordinate_system coordinate_system() {
    return gis::Coordinate_system::kCartesian;
  }
};

struct Geographic_types {
  typedef gis::Geographic_point Point;
  typedef gis::Geographic_linestring Linestring;
  typedef gis::Geographic_linearring Linearring;
  typedef gis::Geographic_polygon Polygon;
  typedef gis::Geographic_geometrycollection Geometrycollection;
  typedef gis::Geographic_multipoint Multipoint;
  typedef gis::Geographic_multilinestring Multilinestring;
  typedef gis::Geographic_multipolygon Multipolygon;

  static gis::Coordinate_system coordinate_system() {
    return gis::Coordinate_system::kGeographic;
  }
};
template <typename T>
typename T::Linearring linearringFromVector(std::vector<double> data) {
  if (data.size() % 2 != 0) {
    throw std::exception(); /* purecov: dead code */
  }
  typename T::Linearring lr;
  for (size_t i = 0; i + 1 < data.size(); i += 2) {
    lr.push_back(typename T::Point(data[i], data[i + 1]));
  }
  return lr;
}

template <typename T, typename U>
T polygonFromVector(std::vector<double> data) {
  T py;
  py.push_back(linearringFromVector<U>(data));
  return py;
}

template <typename T>
typename T::Polygon selfTouchingPolygon() {
  typename T::Polygon py;
  py.push_back(linearringFromVector<T>(
      {0, 0, 1, 0, 0, 0.5, 0.5, 0.25, 0.25, 0.5, 0.5, 0.5, 0, 1, 0, 0}));
  return py;
}

template <typename U>
typename U::Polygon polygonWithTouchingHole() {
  typename U::Polygon py;
  py.push_back(linearringFromVector<U>({0, 0, 1, 0, 0, 1, 0, 0}));
  py.push_back(
      linearringFromVector<U>({0.5, 0.5, 0.5, 0.25, 0.25, 0.5, 0.5, 0.5}));
  return py;
}
template <typename U>
typename U::Polygon polygon_reverse() {
  typename U::Polygon py;
  py.push_back(linearringFromVector<U>({0, 0, 0, 1, 1, 0, 0, 0}));
  py.push_back(
      linearringFromVector<U>({0.5, 0.5, 0.25, 0.5, 0.5, 0.25, 0.5, 0.5}));
  return py;
}
template <typename U>
typename U::Polygon polygon_reverse_touching_hole() {
  typename U::Polygon py;
  py.push_back(linearringFromVector<U>({0, 0, 1, 0, 0, 1, 0, 0}));
  py.push_back(
      linearringFromVector<U>({0.5, 0.5, 0.25, 0.5, 0.5, 0.25, 0.5, 0.5}));
  return py;
}
template <typename T>
typename T::Linearring linearring_2xsquare_around_origin() {
  return linearringFromVector<T>({-1, -1, 1, -1, 1, 1, -1, 1, -1, -1});
}
template <typename U>
typename U::Polygon polygon_with_touching_hole_vertice_vertice() {
  typename U::Polygon py;
  py.push_back(linearring_2xsquare_around_origin<U>());
  py.push_back(
      linearringFromVector<U>({-1., 0.0, 0.5, 0.25, 0.5, -0.25, -1, 0.0}));
  return py;
}

template <typename T>
typename T::Polygon polygon_hourglass();

template <>
gis::Geographic_polygon polygon_hourglass<Geographic_types>() {
  gis::Geographic_polygon py;
  py.push_back(linearringFromVector<Geographic_types>(
      {-0.5, 1, 0.5, 1, -0.5 + M_PI, 1, 0.5 - M_PI, 1}));
  return py;
}
template <>
gis::Cartesian_polygon polygon_hourglass<Cartesian_types>() {
  gis::Cartesian_polygon py;
  py.push_back(linearringFromVector<Cartesian_types>(
      {-1, -1, 1, -1, -1, 1, 1, 1, -1, -1}));
  return py;
}

template <typename T>
typename T::Polygon polygonSelfTouchEdgeVertice() {
  typename T::Polygon py;
  py.push_back(linearringFromVector<T>(
      {0, 0, 0.2, -0.2, 0, -0.4, 0, 0.2, -0.4, -0.6, 0.6, 0, 0, 0}));
  return py;
}

template <typename T>
typename T::Polygon polygonDisconnectedLimit() {
  typename T::Polygon py;
  py.push_back(linearringFromVector<T>({0, 0, 0.2, 0, 0.2, 0.3, 0, 0.3, 0, 0}));
  py.push_back(linearringFromVector<T>({0, 0.4, 0.4, 0.4, 0.2, 0.2, 0, 0.4}));
  return py;
}
template <typename T>
typename T::Polygon polygon_empty_hole() {
  typename T::Polygon py;
  py.push_back(linearringFromVector<T>({0, 0, 1, 0, 0, 1, 0, 0}));
  py.push_back(linearringFromVector<T>(std::vector<double>()));
  return py;
}

template <typename T>
typename T::Polygon polygon_open() {
  typename T::Polygon py;
  py.push_back(linearringFromVector<T>({0, 0, 1, 0, 1, 0}));
  return py;
}
template <typename T>
typename T::Polygon polygon_inner_partially_outside() {
  typename T::Polygon py;
  py.push_back(linearringFromVector<T>({0, 0, 1, 0, 1, 0, 0, 0}));
  py.push_back(
      linearringFromVector<T>({0.25, 0.25, 1, .25, .25, 1, 0.25, 0.25}));
  return py;
}
template <typename T>
typename T::Polygon polygon_inner_wholly_outside() {
  typename T::Polygon py;
  py.push_back(linearringFromVector<T>({0, 0, 1, 0, 1, 0, 0, 0}));
  py.push_back(linearringFromVector<T>({1, 0.5, 0.5, 1, 1, 1, 1, 0.5}));
  return py;
}
template <typename T>
typename T::Polygon polygon_inner_intersecting() {
  typename T::Polygon py;
  py.push_back(linearringFromVector<T>({0, 0, 1, 0, 1, 1, 1, 0, 0, 0}));
  py.push_back(linearringFromVector<T>({.2, 0.2, 0.2, .8, .8, .2, .2, .2}));
  py.push_back(linearringFromVector<T>({.4, 0.4, 0.4, .8, .8, .4, .4, .4}));
  return py;
}
template <typename T>
typename T::Linearring linearring_unit_square() {
  return linearringFromVector<T>({0, 0, 1, 0, 1, 1, 0, 1, 0, 0});
}
template <typename T>
typename T::Polygon polygon_2_inner() {
  typename T::Polygon py;
  py.push_back(linearring_unit_square<T>());
  py.push_back(
      linearringFromVector<T>({0.2, 0.2, 0.2, 0.8, 0.8, 0.2, 0.2, 0.2}));
  py.push_back(
      linearringFromVector<T>({0.8, 0.4, 0.4, 0.8, 0.8, 0.8, 0.8, 0.4}));
  return py;
}
template <typename T>
typename T::Polygon polygon_2_inner_edge_to_edge0() {
  typename T::Polygon py;
  py.push_back(linearring_unit_square<T>());
  py.push_back(
      linearringFromVector<T>({0.2, 0.2, 0.2, 0.8, 0.8, 0.2, 0.2, 0.2}));
  py.push_back(
      linearringFromVector<T>({0.8, 0.2, 0.2, 0.8, 0.8, 0.8, 0.8, 0.2}));
  return py;
}
template <typename T>
typename T::Polygon polygon_2_inner_edge_to_edge1() {
  typename T::Polygon py;
  py.push_back(linearring_unit_square<T>());
  py.push_back(
      linearringFromVector<T>({0.2, 0.2, 0.2, 0.8, 0.8, 0.2, 0.2, 0.2}));
  py.push_back(
      linearringFromVector<T>({0.8, 0.2, 0.4, 0.6, 0.8, 0.6, 0.8, 0.2}));
  return py;
}
template <typename T>
typename T::Polygon polygon_2_inner_edge_to_vertice() {
  typename T::Polygon py;
  py.push_back(linearringFromVector<T>({-1, -1, 1, -1, 1, 1, -1, 1, -1, -1}));
  py.push_back(
      linearringFromVector<T>({-0.5, 0.0, 0.0, 0.5, 0.0, -0.5, -0.5, 0.0}));
  py.push_back(
      linearringFromVector<T>({0.0, 0.0, 0.5, 0.5, 0.5, -0.5, 0.0, 0.0}));
  return py;
}
template <typename T>
typename T::Polygon polygon_2_inner_vertice_to_vertice() {
  typename T::Polygon py;
  py.push_back(linearring_2xsquare_around_origin<T>());
  py.push_back(
      linearringFromVector<T>({-0.5, -0.5, -0.5, 0.5, 0.0, 0.0, -0.5, -0.5}));
  py.push_back(
      linearringFromVector<T>({0.0, 0.0, 0.5, 0.5, 0.5, -0.5, 0.0, 0.0}));
  return py;
}
template <typename T>
typename T::Polygon polygon_1_1_bug26476445() {
  typename T::Polygon py;
  py.push_back(linearringFromVector<T>(
      {0.15707963267948966, 0.19198621771937624, 2.4958208303518914,
       0.9250245035569946, 0.24434609527920614, 0.6632251157578452,
       0.15707963267948966, 0.19198621771937624}));
  return py;
}
template <typename T>
typename T::Polygon polygon_1_2_bug26476445() {
  typename T::Polygon py;
  py.push_back(linearringFromVector<T>(
      {-2.5132741228718345, -1.0122909661567112, -0.06981317007977318,
       -1.2217304763960306, -2.478367537831948, -0.10471975511965977,
       -2.5132741228718345, -1.0122909661567112}));
  return py;
}
template <typename T>
typename T::Multipolygon multipolygon_1_bug26476445() {
  typename T::Multipolygon mpy;
  mpy.push_back(polygon_1_1_bug26476445<T>());
  mpy.push_back(polygon_1_2_bug26476445<T>());
  return mpy;
}

template <typename T>
typename T::Polygon polygon_2_1_bug26476445() {
  typename T::Polygon py;
  py.push_back(linearringFromVector<T>(
      {0.15707963267948966, 0.19198621771937624, 0.7504915783575616,
       0.9424777960769379, 0.24434609527920614, 1.4660765716752369,
       0.15707963267948966, 0.19198621771937624}));
  return py;
}
template <typename T>
typename T::Polygon polygon_2_2_bug26476445() {
  typename T::Polygon py;
  py.push_back(linearringFromVector<T>(
      {-2.5132741228718345, -1.0122909661567112, -0.06981317007977318,
       -1.2217304763960306, -2.478367537831948, -0.10471975511965977,
       -2.5132741228718345, -1.0122909661567112}));
  return py;
}
template <typename T>
typename T::Multipolygon multipolygon_2_bug26476445() {
  typename T::Multipolygon mpy;
  mpy.push_back(polygon_2_1_bug26476445<T>());
  mpy.push_back(polygon_2_2_bug26476445<T>());
  return mpy;
}

template <typename T>
typename T::Linestring linestringFromVector(std::vector<double> data) {
  if (data.size() % 2 != 0) {
    throw std::exception();
  }
  typename T::Linearring ls;
  for (size_t i = 0; i + 1 < data.size(); i += 2) {
    ls.push_back(typename T::Point(data[i], data[i + 1]));
  }
  return ls;
}

template <typename T>
typename T::Linestring linestring_3pt_normal() {
  return linestringFromVector<T>({0.0, 0.0, 0.56, 0.99, -.66, 2.0});
}

template <typename T>
typename T::Linestring linestring_ring() {
  return linestringFromVector<T>({0.0, 0.0, 0.56, 0.99, 0.0, 2.0, 0.0, 0.0});
}

template <typename T>
typename T::Linestring linestring_selfcrossing() {
  return linestringFromVector<T>({0.0, 0.0, 1.0, 0.0, 0.1, 1.0, 0.0, -2.0});
}

#endif  // UNITTEST_GUNIT_GIS_TESTSHAPES_H_INCLUDED

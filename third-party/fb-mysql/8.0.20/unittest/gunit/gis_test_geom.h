/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef UNITTEST_GUNIT_GIS_TEST_GEOM_H_INCLUDED
#define UNITTEST_GUNIT_GIS_TEST_GEOM_H_INCLUDED

#include <cmath>  // M_PI, M_PI_2

#include "sql/gis/geometries_cs.h"  // gis::{Cartesian,Geographic}*

// Header-only definitions of sample geometries useful for testing
// Note that the internal point representation for geographic coordinates is
// (eastwards (-pi, pi], northwards [pi/2, pi/2]) in radians.

namespace {
namespace test_geom {

using Cpt = gis::Cartesian_point;
using Cls = gis::Cartesian_linestring;
using Cgc = gis::Cartesian_geometrycollection;
using Gpt = gis::Geographic_point;
using Gls = gis::Geographic_linestring;
using Ggc = gis::Geographic_geometrycollection;

inline Cpt point_origin() { return {0.0, 0.0}; }

inline Cls linestring_line() {
  // Slanted line
  Cls ls;
  ls.push_back(Cpt{-1, -1});
  ls.push_back(Cpt{+1, +1});
  return ls;
}

inline Cls linestring_right_angle() {
  // _|-shape
  Cls ls;
  ls.push_back(Cpt{-1, -1});
  ls.push_back(Cpt{+1, -1});
  ls.push_back(Cpt{+1, +1});
  return ls;
}

inline Cls linestring_triangle() {
  // Closed _|-shape
  Cls ls;
  ls.push_back(Cpt{-1, -1});
  ls.push_back(Cpt{+1, -1});
  ls.push_back(Cpt{+1, +1});
  ls.push_back(Cpt{-1, -1});
  return ls;
}

inline Cls linestring_selfintersecting() {
  // Open bow / fish-shape, like: ><|
  Cls ls;
  ls.push_back(Cpt{-1, +1});
  ls.push_back(Cpt{+1, -1});
  ls.push_back(Cpt{+1, +1});
  ls.push_back(Cpt{-1, -1});
  return ls;
}

inline Cls linestring_selfintersecting_closed() {
  // Bow-shape, like |><|
  Cls ls;
  ls.push_back(Cpt{-1, +1});
  ls.push_back(Cpt{+1, -1});
  ls.push_back(Cpt{+1, +1});
  ls.push_back(Cpt{-1, -1});
  ls.push_back(Cpt{-1, +1});
  return ls;
}

inline Cls linestring_selftangential() {
  Cls ls;
  ls.push_back(Cpt{-1.0, -0.75});
  ls.push_back(Cpt{+1.0, -0.75});
  ls.push_back(Cpt{+0.0, +0.00});  // Tangent point
  ls.push_back(Cpt{+0.5, -0.50});
  ls.push_back(Cpt{-0.5, -0.50});  // Tangent line
  ls.push_back(Cpt{+0.5, +0.50});  // Touch at midpoint
  ls.push_back(Cpt{-1.0, -0.75});
  return ls;
}

inline Gls linestring_pole_alias() {
  // Line between two coordinates both mapping to the north-pole
  Gls ls;
  ls.push_back(Gpt{0, M_PI_2});
  ls.push_back(Gpt{1, M_PI_2});
  return ls;
}

inline Cgc geometrycollection_empty() { return {}; }

inline Cgc geometrycollection_crossintersecting() {
  Cgc gc;
  {
    Cls ls;
    ls.push_back(Cpt{-1, -1});
    ls.push_back(Cpt{+1, +1});
    gc.push_back(ls);
  }
  {
    Cls ls;
    ls.push_back(Cpt{-1, +1});
    ls.push_back(Cpt{+1, -1});
    gc.push_back(ls);
  }
  return gc;
}

inline Cgc geometrycollection_subintersecting() {
  Cgc gc;
  gc.push_back(linestring_selfintersecting());
  return gc;
}

inline Ggc bug26495068() {
  // Regression test for Bug#26495068
  // Certain geometries crossing the anti-meridian give incorrect results.
  auto deg = [](double x) { return x * M_PI / 180; };
  Ggc gc;
  {
    Gls ls;
    ls.push_back(Gpt{deg(135), deg(0)});
    ls.push_back(Gpt{deg(-150), deg(36)});
    gc.push_back(ls);
  }
  {
    Gls ls;
    ls.push_back(Gpt{deg(-101), deg(0)});
    ls.push_back(Gpt{deg(-178), deg(30)});
    gc.push_back(ls);
  }
  return gc;
}

}  // namespace test_geom
}  // namespace

#endif  // include guard

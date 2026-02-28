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

#include <gtest/gtest.h>

#include "sql/gis/geometries.h"  // gis::Geometry, gis::Coordinate_system
#include "sql/gis/is_simple.h"

#include "unittest/gunit/gis_project_on_pole.h"
#include "unittest/gunit/gis_test.h"
#include "unittest/gunit/gis_test_geom.h"
#include "unittest/gunit/gis_typeset.h"

namespace {

template <typename T_typeset>
struct Is_simple : Gis_test<T_typeset> {
  bool is_simple(gis::Geometry const &g) {
    assert(g.coordinate_system() == T_typeset::coordinate_system());

    bool result;
    {
      bool result_null;
      gis::is_simple(this->m_srs.get(), &g, "testcase", &result, &result_null);
      assert(!result_null);
    }

    return result;
  }

  // Implictly convert to geographic coordinates, projecting `g` onto the
  // north pole if given cartesian coordinates while testing geographic.
  template <typename T>
  bool implicit_geo_is_simple(T const &g) {
    if ((T_typeset::coordinate_system() ==
         gis::Coordinate_system::kGeographic) &&
        (g.coordinate_system() == gis::Coordinate_system::kCartesian))
      return is_simple(gis_project_on_pole(g));
    else
      return is_simple(g);
  }
};

TYPED_TEST_CASE(Is_simple, gis_typeset::Test_both);
using Is_simple_car = Is_simple<gis_typeset::Cartesian>;
using Is_simple_geo = Is_simple<gis_typeset::Geographic>;

///// Tests common to Cartesian and Geographic

/// Point
TYPED_TEST(Is_simple, point_origin) {
  EXPECT_TRUE(this->implicit_geo_is_simple(test_geom::point_origin()));
}

/// Linestring
TYPED_TEST(Is_simple, linestring_line) {
  EXPECT_TRUE(this->implicit_geo_is_simple(test_geom::linestring_line()));
}
TYPED_TEST(Is_simple, linestring_right_angle) {
  EXPECT_TRUE(
      this->implicit_geo_is_simple(test_geom::linestring_right_angle()));
}
TYPED_TEST(Is_simple, linestring_triangle) {
  EXPECT_TRUE(this->implicit_geo_is_simple(test_geom::linestring_triangle()));
}
TYPED_TEST(Is_simple, linestring_selfintersecting) {
  EXPECT_FALSE(
      this->implicit_geo_is_simple(test_geom::linestring_selfintersecting()));
}
TYPED_TEST(Is_simple, linestring_selfintersecting_closed) {
  EXPECT_FALSE(this->implicit_geo_is_simple(
      test_geom::linestring_selfintersecting_closed()));
}

/// Polygon, Multipolygon
// All valid polygons are simple; considering testing not worth it.

///// Cartesian only tests

// This test was failing which put trough implicit_geo_is_simple. Most likely a
// numeric error.
TEST_F(Is_simple_car, linestring_selftangential_point_edge) {
  EXPECT_FALSE(is_simple(test_geom::linestring_selftangential()));
}

// TODO: Overload project_on_north_pole for geometrycollection and make these
// tests common for cartesian and geographic.

TEST_F(Is_simple_car, geometrycollection_empty) {
  EXPECT_TRUE(is_simple(test_geom::geometrycollection_empty()));
}
TEST_F(Is_simple_car, geometrycollection_crossintersecting) {
  EXPECT_FALSE(is_simple(test_geom::geometrycollection_crossintersecting()));
}
TEST_F(Is_simple_car, geometrycollection_subintersecting) {
  EXPECT_FALSE(is_simple(test_geom::geometrycollection_subintersecting()));
}

///// Geographic only tests

// SQL/MM is ambiguous on whether pole-aliasing is simple, but Adam Wulkiewicz,
// Boost::geometry developer, agrees that we choose to consider it not simple.
TEST_F(Is_simple_geo, linestring_pole_alias) {
  EXPECT_FALSE(is_simple(test_geom::linestring_pole_alias()));
}

// Regression test for Bug#26495068
// Certain geometries crossing the anti-meridian give an incorrect result.
TEST_F(Is_simple_geo, bug26495068) {
  EXPECT_FALSE(is_simple(test_geom::bug26495068()));
}

}  // namespace

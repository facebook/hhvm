/*
   Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <gtest/gtest.h>
#include "sql/dd/dd.h"
#include "sql/dd/impl/types/spatial_reference_system_impl.h"
#include "sql/dd/types/spatial_reference_system.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_cs.h"
#include "sql/gis/is_valid.h"

#include "unittest/gunit/gis_testshapes.h"
namespace isvalid_unittest {
std::unique_ptr<dd::Spatial_reference_system_impl> get_srs(
    gis::Coordinate_system coordinate_system) {
  switch (coordinate_system) {
    case gis::Coordinate_system::kCartesian: {
      // Use SRID 0.
      return std::unique_ptr<dd::Spatial_reference_system_impl>();
    }
    case gis::Coordinate_system::kGeographic: {
      // EPSG 4326, but with long-lat axes (E-N).
      std::unique_ptr<dd::Spatial_reference_system_impl> m_srs(
          dynamic_cast<dd::Spatial_reference_system_impl *>(
              dd::create_object<dd::Spatial_reference_system>()));
      m_srs->set_id(4326);
      m_srs->set_name("WGS 84");
      m_srs->set_created(0UL);
      m_srs->set_last_altered(0UL);
      m_srs->set_organization("EPSG");
      m_srs->set_organization_coordsys_id(4326);
      m_srs->set_definition(
          "GEOGCS[\"WGS 84\",DATUM[\"World Geodetic System "
          "1984\",SPHEROID[\"WGS "
          "84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],"
          "AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY["
          "\"EPSG\",\"8901\"]],UNIT[\"degree\",0.017453292519943278,"
          "AUTHORITY[\"EPSG\",\"9122\"]],AXIS[\"Lon\",EAST],AXIS[\"Lat\","
          "NORTH],AUTHORITY[\"EPSG\",\"4326\"]]");
      m_srs->set_description("");
      m_srs->parse_definition();
      return m_srs;
      break;
    }
  }
  throw std::exception(); /* purecov: dead code */
}

template <typename Types>
class IsvalidTest : public ::testing::Test {
 public:
  std::unique_ptr<const dd::Spatial_reference_system_impl> m_srs;
  IsvalidTest() { m_srs = get_srs(Types::coordinate_system()); }
};

typedef ::testing::Types<Cartesian_types, Geographic_types> Types;

TYPED_TEST_CASE(IsvalidTest, Types);

TYPED_TEST(IsvalidTest, Point) {
  typename TypeParam::Point pt0{0, 0};
  bool result;
  gis::is_valid(this->m_srs.get(), &pt0, "unittest", &result);
  EXPECT_TRUE(result);

  typename TypeParam::Point pt1;
  gis::is_valid(this->m_srs.get(), &pt1, "unittest", &result);
  EXPECT_TRUE(result);
}

TYPED_TEST(IsvalidTest, Linestring) {
  typename TypeParam::Linestring ls{};
  EXPECT_TRUE(ls.is_empty());
  bool result;
  gis::is_valid(this->m_srs.get(), &ls, "unittest", &result);
  EXPECT_TRUE(result);
  ls.push_back(typename TypeParam::Point(0.0, 0.0));
  gis::is_valid(this->m_srs.get(), &ls, "unittest", &result);
  EXPECT_FALSE(result);  // This should be a invalid linestring, but mysql
  // should not generate it.
  ls.push_back(typename TypeParam::Point(1.0, 1.0));
  gis::is_valid(this->m_srs.get(), &ls, "unittest", &result);
  EXPECT_TRUE(result);
}
TYPED_TEST(IsvalidTest, Polygon) {
  typename TypeParam::Linearring lr{};
  typename TypeParam::Polygon py{};
  bool result;
  gis::is_valid(this->m_srs.get(), &py, "unittest", &result);
  EXPECT_TRUE(result);
  lr.push_back(typename TypeParam::Point(0.0, 0.0));
  py.push_back(lr);
  gis::is_valid(this->m_srs.get(), &py, "unittest", &result);
  EXPECT_FALSE(result);
  auto py1 = selfTouchingPolygon<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py1, "unittest", &result);
  EXPECT_FALSE(result);
  auto py2 = polygonWithTouchingHole<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py2, "unittest", &result);
  EXPECT_TRUE(result);
  auto py3 = polygonSelfTouchEdgeVertice<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py3, "unittest", &result);
  EXPECT_FALSE(result);
  auto py4 = polygonDisconnectedLimit<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py4, "unittest", &result);
  EXPECT_FALSE(result);
  auto py5 = polygon_with_touching_hole_vertice_vertice<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py5, "unittest", &result);
  EXPECT_TRUE(result);
  auto py6 = polygon_hourglass<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py6, "unittest", &result);
  EXPECT_FALSE(result);
  auto py7 = polygon_empty_hole<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py7, "unittest", &result);
  EXPECT_FALSE(result);
  auto py8 = polygon_reverse_touching_hole<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py8, "unittest", &result);
  EXPECT_FALSE(result);
  auto py9 = polygon_reverse<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py9, "unittest", &result);
  EXPECT_FALSE(result);
  auto py10 = polygon_open<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py10, "unittest", &result);
  EXPECT_FALSE(result);
  auto py11 = polygon_inner_partially_outside<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py11, "unittest", &result);
  EXPECT_FALSE(result);
  auto py12 = polygon_inner_wholly_outside<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py12, "unittest", &result);
  EXPECT_FALSE(result);
  auto py13 = polygon_inner_intersecting<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py13, "unittest", &result);
  EXPECT_FALSE(result);
  auto py14 = polygon_2_inner<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py14, "unittest", &result);
  EXPECT_TRUE(result);
  auto py15 = polygon_2_inner_edge_to_edge0<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py15, "unittest", &result);
  EXPECT_FALSE(result);
  auto py16 = polygon_2_inner_edge_to_edge1<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py16, "unittest", &result);
  EXPECT_FALSE(result);
  auto py17 = polygon_2_inner_edge_to_vertice<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py17, "unittest", &result);
  EXPECT_TRUE(result);
  auto py18 = polygon_2_inner_vertice_to_vertice<TypeParam>();
  gis::is_valid(this->m_srs.get(), &py18, "unittest", &result);
  EXPECT_TRUE(result);
}
TYPED_TEST(IsvalidTest, Multipoint) {
  typename TypeParam::Multipoint mpt{};
  bool result;
  gis::is_valid(this->m_srs.get(), &mpt, "unittest", &result);
  EXPECT_TRUE(result);
}
TYPED_TEST(IsvalidTest, Multilinestring) {
  typename TypeParam::Multilinestring mls{};
  bool result;
  gis::is_valid(this->m_srs.get(), &mls, "unittest", &result);
  EXPECT_TRUE(result);
}
TYPED_TEST(IsvalidTest, Multipolygon) {
  typename TypeParam::Multipolygon mpy{};
  bool result;
  gis::is_valid(this->m_srs.get(), &mpy, "unittest", &result);
  EXPECT_TRUE(result);
}
TYPED_TEST(IsvalidTest, Geometrycollection) {
  typename TypeParam::Geometrycollection gc{};
  bool result;
  gis::is_valid(this->m_srs.get(), &gc, "unittest", &result);
  EXPECT_TRUE(result);
}

TEST(IsvalidTest, bug26476445_1) {
  auto mpy1 = multipolygon_1_bug26476445<Geographic_types>();
  bool result;
  auto m_srs1 = get_srs(mpy1.coordinate_system());
  gis::is_valid(m_srs1.get(), &mpy1, "unittest", &result);
  EXPECT_TRUE(result);
}

TEST(IsvalidTest, bug26476445_2) {
  auto mpy2 = multipolygon_2_bug26476445<Geographic_types>();
  bool result;
  auto m_srs2 = get_srs(mpy2.coordinate_system());
  gis::is_valid(m_srs2.get(), &mpy2, "unittest", &result);
  EXPECT_TRUE(result);
}
}  // namespace isvalid_unittest

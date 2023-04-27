/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA */

#include <initializer_list>

#include <gtest/gtest.h>
#include "mysqld_error.h"
#include "sql/dd/dd.h"
#include "sql/dd/impl/types/spatial_reference_system_impl.h"
#include "sql/dd/types/spatial_reference_system.h"
#include "sql/gis/distance.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_cs.h"
#include "sql/gis/st_units_of_measure.h"
#include "unittest/gunit/test_utils.h"

namespace distance_unittest {

class DistanceUnitTest : public ::testing::Test {
 public:
  my_testing::Server_initializer initializer;

  DistanceUnitTest() = default;
  void SetUp() override { initializer.SetUp(); }
  void TearDown() override { initializer.TearDown(); }
  THD *thd() { return initializer.thd(); }
  ~DistanceUnitTest() override = default;
};
TEST_F(DistanceUnitTest, unordered_map) {
  auto units = gis::units();
  auto find_res = units.find("metre");
  EXPECT_TRUE(find_res->first == "metre");
  find_res = units.find("metrE");
  EXPECT_TRUE(find_res->first == "metre");
  find_res = units.find("metrE");
  EXPECT_FALSE(find_res == units.end());
  find_res = units.find("metEr");
  EXPECT_TRUE(find_res == units.end());
  find_res = units.find("Clarke's foot");
  EXPECT_FALSE(find_res == units.end());
}
TEST_F(DistanceUnitTest, get_conversion_factor) {
  double conversion_factor = 0;
  EXPECT_FALSE(gis::get_conversion_factor("metre", &conversion_factor));
  EXPECT_FALSE(gis::get_conversion_factor("METRE", &conversion_factor));
  EXPECT_FALSE(gis::get_conversion_factor("British foot (Sears 1922)",
                                          &conversion_factor));
  EXPECT_FALSE(gis::get_conversion_factor("claRke'S LInk", &conversion_factor));
}
TEST_F(DistanceUnitTest, er_unit_not_found) {
  initializer.set_expected_error(ER_UNIT_NOT_FOUND);
  double conversion_factor = 0;
  EXPECT_TRUE(gis::get_conversion_factor("MITRE", &conversion_factor));
}

std::unique_ptr<dd::Spatial_reference_system_impl> GetGeographicalSrs() {
  // EPSG 4326, but with long-lat axes (E-N).
  std::unique_ptr<dd::Spatial_reference_system_impl> srs(
      dynamic_cast<dd::Spatial_reference_system_impl *>(
          dd::create_object<dd::Spatial_reference_system>()));
  srs->set_id(4326);
  srs->set_name("WGS 84");
  srs->set_created(0UL);
  srs->set_last_altered(0UL);
  srs->set_organization("EPSG");
  srs->set_organization_coordsys_id(4326);
  srs->set_definition(
      "GEOGCS[\"WGS 84\",DATUM[\"World Geodetic System "
      "1984\",SPHEROID[\"WGS "
      "84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],"
      "AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY["
      "\"EPSG\",\"8901\"]],UNIT[\"degree\",0.017453292519943278,"
      "AUTHORITY[\"EPSG\",\"9122\"]],AXIS[\"Lon\",EAST],AXIS[\"Lat\","
      "NORTH],AUTHORITY[\"EPSG\",\"4326\"]]");
  srs->set_description("");
  srs->parse_definition();
  return srs;
}

gis::Geographic_linestring MakeLinestring(
    const std::initializer_list<double> &data) {
  if (data.size() % 2 != 0) {
    throw std::exception(); /* purecov: dead code */
  }
  typename gis::Geographic_linestring linestring;
  for (std::initializer_list<double>::const_iterator it = data.begin();
       it != data.end(); std::advance(it, 2)) {
    linestring.push_back(gis::Geographic_point(*it, *(it + 1)));
  }
  return linestring;
}

gis::Geographic_linearring MakeLinearring(
    const std::initializer_list<double> &data) {
  if (data.size() % 2 != 0) {
    throw std::exception(); /* purecov: dead code */
  }
  gis::Geographic_linearring linear_ring;
  for (std::initializer_list<double>::const_iterator it = data.begin();
       it != data.end(); std::advance(it, 2)) {
    linear_ring.push_back(gis::Geographic_point(*it, *(it + 1)));
  }
  return linear_ring;
}
gis::Geographic_polygon MakePolygon(
    const std::initializer_list<std::initializer_list<double>> &data) {
  gis::Geographic_polygon polygon;
  for (auto ring : data) {
    polygon.push_back(MakeLinearring(ring));
  }
  return polygon;
}
gis::Geographic_multipoint MakeMultipoint(
    const std::initializer_list<std::initializer_list<double>> &data) {
  gis::Geographic_multipoint multipoint;
  for (auto point : data) {
    EXPECT_EQ(point.size(), 2);
    multipoint.push_back(
        gis::Geographic_point(*point.begin(), *(point.begin() + 1)));
  }
  return multipoint;
}

gis::Geographic_multilinestring MakeMultilinestring(
    const std::initializer_list<std::initializer_list<double>> &data) {
  gis ::Geographic_multilinestring multilinestring;
  for (const auto linestring : data) {
    multilinestring.push_back(MakeLinestring(linestring));
  }
  return multilinestring;
}

gis::Geographic_multipolygon MakeMultipolygon(
    const std::initializer_list<
        std::initializer_list<std::initializer_list<double>>> &data) {
  gis::Geographic_multipolygon multipolygon;
  for (auto polygon : data) {
    multipolygon.push_back(MakePolygon(polygon));
  }
  return multipolygon;
}

double TestDistanceGeographic(const gis::Geometry &g1, const gis::Geometry &g2,
                              bool *null_value = nullptr) {
  auto srs = GetGeographicalSrs();
  double distance = 0.0;
  if (null_value != nullptr) {
    bool res = gis::distance(srs.get(), &g1, &g2, &distance, null_value);
    EXPECT_FALSE(res);
  } else {
    bool is_null = false;
    bool res = gis::distance(srs.get(), &g1, &g2, &distance, &is_null);
    EXPECT_FALSE(res);
    EXPECT_FALSE(is_null);
  }
  return distance;
}

TEST(DistanceTest, Geographic_pp) {
  EXPECT_GT(TestDistanceGeographic(gis::Geographic_point{0., 0.},
                                   gis::Geographic_point{1. * M_PI / 360, 0.}),
            55659.0);
  EXPECT_LT(TestDistanceGeographic(gis::Geographic_point{0., 0.},
                                   gis::Geographic_point{1. * M_PI / 360, 0.}),
            55660.0);
}

TEST(DistanceTest, GeographicLinestringPoint) {
  gis::Geographic_linestring geographic_linestring =
      MakeLinestring({-0.2, 0.05, 0.2, 0.05});
  gis::Geographic_point geographic_point1{0, 0};
  gis::Geographic_point geographic_point2{0, 0.10};
  EXPECT_GT(TestDistanceGeographic(geographic_point1, geographic_linestring),
            TestDistanceGeographic(geographic_point2, geographic_linestring));
  EXPECT_GT(TestDistanceGeographic(geographic_linestring, geographic_point1),
            TestDistanceGeographic(geographic_linestring, geographic_point2));
}

TEST(DistanceTest, Geographic_lsls) {
  EXPECT_LT(
      TestDistanceGeographic(MakeLinestring({-1., 0., 1, 0}),
                             MakeLinestring({-1., 0.08, 1, 0.004, 1, 0.08})),
      TestDistanceGeographic(MakeLinestring({-1., 0., 1, 0}),
                             MakeLinestring({-1., 0.08, 1, 0.08})));
}

TEST(DistanceTest, GeographicPolygonPoint) {
  gis::Geographic_point geographic_point1{0.1, 0.1};
  gis::Geographic_polygon geographic_polygon =
      MakePolygon({{0, 0, 1, 0, 0, 1, 0, 0}});
  EXPECT_DOUBLE_EQ(
      TestDistanceGeographic(geographic_polygon, geographic_point1), 0);
  geographic_polygon =
      MakePolygon({{0, 0, 1, 0, 0, 1, 0, 0},
                   {0.05, 0.05, 0.05, 0.95, 0.95, 0.05, 0.05, 0.05}});
  EXPECT_GT(TestDistanceGeographic(geographic_polygon, geographic_point1), 0);
  EXPECT_DOUBLE_EQ(
      TestDistanceGeographic(geographic_polygon, geographic_point1),
      TestDistanceGeographic(geographic_point1, geographic_polygon));
}

TEST(DistanceTest, GeographicPolygonLinestring) {
  gis::Geographic_polygon geographic_polygon =
      MakePolygon({{0, 0, 1, 0, 0, 1, 0, 0}});
  gis::Geographic_linestring geographic_linestring1 =
      MakeLinestring({-1., 0.5, -.1, 0.5});
  gis::Geographic_linestring geographic_linestring2 =
      MakeLinestring({-1., 0.5, .1, 0.5});
  EXPECT_GT(TestDistanceGeographic(geographic_polygon, geographic_linestring1),
            0);
  EXPECT_DOUBLE_EQ(
      TestDistanceGeographic(geographic_linestring2, geographic_polygon), 0);
}

TEST(DistanceTest, GeographicPolygonPolygon) {
  gis::Geographic_polygon geographic_polygon1 =
      MakePolygon({{0, 0, 1, 0, 0, 1, 0, 0}});
  gis::Geographic_polygon geographic_polygon2 =
      MakePolygon({{-1, 0, -.1, 0, -1, 1, -1, 0}});
  gis::Geographic_polygon geographic_polygon3 =
      MakePolygon({{-1, 0, .1, 0, -1, 1, -1, 0}});
  EXPECT_GT(TestDistanceGeographic(geographic_polygon1, geographic_polygon2),
            0);
  EXPECT_DOUBLE_EQ(
      TestDistanceGeographic(geographic_polygon1, geographic_polygon3), 0);
}

TEST(DistanceTest, Geographic_Multipoint_Point) {
  {
    gis::Geographic_multipoint geographic_multipoint;
    gis::Geographic_multipoint geographic_point;
    geographic_multipoint.push_back(gis::Geographic_point({-1, 0}));
    geographic_multipoint.push_back(gis::Geographic_point({1, 0}));
    gis::Geographic_point geographic_point1{0.0, 0.1};
    gis::Geographic_point geographic_point2{1.0, 0.0};
    EXPECT_GT(TestDistanceGeographic(geographic_multipoint, geographic_point1),
              0);
    EXPECT_DOUBLE_EQ(
        TestDistanceGeographic(geographic_multipoint, geographic_point2), 0);
  }
  {
    gis::Geographic_point geographic_point{0, 0};
    bool is_null = false;
    TestDistanceGeographic(gis::Geographic_multipoint(), geographic_point,
                           &is_null);
    EXPECT_TRUE(is_null);
    is_null = false;
    TestDistanceGeographic(geographic_point, gis::Geographic_multipoint(),
                           &is_null);
    EXPECT_TRUE(is_null);
  }
}

TEST(DistanceTest, Geographic_Multipoint_Linestring) {
  gis::Geographic_linestring geographic_linestring =
      MakeLinestring({-1, 0, 1, 0});
  gis::Geographic_multipoint geographic_multipoint1 =
      MakeMultipoint({{-1, 0}, {1, 0}});
  gis::Geographic_point geographic_point1{0.0, 0.1};
  EXPECT_LT(
      TestDistanceGeographic(geographic_linestring, geographic_multipoint1),
      TestDistanceGeographic(geographic_multipoint1, geographic_point1));
  EXPECT_DOUBLE_EQ(
      TestDistanceGeographic(geographic_linestring, geographic_point1),
      TestDistanceGeographic(geographic_point1, geographic_linestring));
}

TEST(DistanceTest, Geographic_Multipoint_Polygon) {
  gis::Geographic_multipoint geographic_multipoint1;
  geographic_multipoint1.push_back(gis::Geographic_point({-1.1, 0}));
  geographic_multipoint1.push_back(gis::Geographic_point({1.1, 0}));
  gis::Geographic_polygon geographic_polygon =
      MakePolygon({{0, 0, 1, 0, 0, 1, 0, 0}});
  EXPECT_GT(TestDistanceGeographic(geographic_multipoint1, geographic_polygon),
            0);
  EXPECT_DOUBLE_EQ(
      TestDistanceGeographic(geographic_multipoint1, geographic_polygon),
      TestDistanceGeographic(geographic_polygon, geographic_multipoint1));
}

TEST(DistanceTest, Geographic_Multipoint_Multipoint) {
  gis::Geographic_multipoint geographic_multipoint1;
  geographic_multipoint1.push_back(gis::Geographic_point({-1, 0}));
  geographic_multipoint1.push_back(gis::Geographic_point({1, 0}));
  gis::Geographic_multipoint geographic_multipoint2;
  geographic_multipoint2.push_back(gis::Geographic_point({0, -1}));
  geographic_multipoint2.push_back(gis::Geographic_point({0, 1}));
  EXPECT_GT(
      TestDistanceGeographic(geographic_multipoint1, geographic_multipoint2),
      0);
  EXPECT_DOUBLE_EQ(
      TestDistanceGeographic(geographic_multipoint1, geographic_multipoint2),
      TestDistanceGeographic(geographic_multipoint2, geographic_multipoint1));
}

TEST(DistanceTest, Geographic_Multilinestring_point) {
  {
    gis::Geographic_multilinestring geographic_multilinestring1;
    geographic_multilinestring1.push_back(MakeLinestring({-1, 0, 1, 0}));
    gis::Geographic_point geographic_point1(0, 1);
    EXPECT_GT(
        TestDistanceGeographic(geographic_multilinestring1, geographic_point1),
        0);
  }
  {
    gis::Geographic_point geographic_point{0, 0};
    bool is_null = false;
    TestDistanceGeographic(gis::Geographic_multilinestring(), geographic_point,
                           &is_null);
    EXPECT_TRUE(is_null);
    is_null = false;
    TestDistanceGeographic(geographic_point, gis::Geographic_multilinestring(),
                           &is_null);
    EXPECT_TRUE(is_null);
  }
}

TEST(DistanceTest, Geographic_Multilinestring_Linestring) {
  gis::Geographic_multilinestring geographic_multilinestring1;
  geographic_multilinestring1.push_back(MakeLinestring({-1, 0, 1, 0}));
  gis::Geographic_linestring geographic_linestringtr1 =
      MakeLinestring({-0, -1, 0, 1});
  EXPECT_DOUBLE_EQ(TestDistanceGeographic(geographic_multilinestring1,
                                          geographic_linestringtr1),
                   0);
  EXPECT_DOUBLE_EQ(TestDistanceGeographic(geographic_linestringtr1,
                                          geographic_multilinestring1),
                   0);
}

TEST(DistanceTest, Geographic_Multilinestring_Polygon) {
  gis::Geographic_multilinestring geographic_multilinestring1;
  geographic_multilinestring1.push_back(MakeLinestring({-1, 0.5, 1, 0.5}));
  gis::Geographic_polygon geographic_polygon1 =
      MakePolygon({{0, 0, 1, 0, 0, 1, 0, 0}});
  EXPECT_DOUBLE_EQ(
      TestDistanceGeographic(geographic_multilinestring1, geographic_polygon1),
      0);
  EXPECT_DOUBLE_EQ(
      TestDistanceGeographic(geographic_polygon1, geographic_multilinestring1),
      0);
}

TEST(DistanceTest, Geographic_Multilinestring_Multipoint) {
  gis::Geographic_multilinestring geographic_multilinestring1;
  geographic_multilinestring1.push_back(MakeLinestring({-1, 0, 1, 0}));
  gis::Geographic_multipoint geographic_multipoint1;
  geographic_multipoint1.push_back(gis::Geographic_point(0, 1));
  EXPECT_GT(TestDistanceGeographic(geographic_multilinestring1,
                                   geographic_multipoint1),
            0);
  EXPECT_GT(TestDistanceGeographic(geographic_multipoint1,
                                   geographic_multilinestring1),
            0);
}

TEST(DistanceTest, Geographic_Multilinestring_Multilinestring) {
  gis::Geographic_multilinestring geographic_multilinestring1;
  geographic_multilinestring1.push_back(MakeLinestring({-1, 0, 1, 0}));
  gis::Geographic_multilinestring geographic_multilinestring2;
  geographic_multilinestring2.push_back(MakeLinestring({-0, -1, 0, 1}));
  EXPECT_DOUBLE_EQ(TestDistanceGeographic(geographic_multilinestring1,
                                          geographic_multilinestring2),
                   0);
}

TEST(DistanceTest, GeographicMultipolygon_Point) {
  {
    gis::Geographic_multipolygon geographic_multipolygon;
    gis::Geographic_polygon geographic_polygon;
    gis::Geographic_linearring linearring =
        MakeLinearring({0, 0, 1, 0, 0, 1, 0, 0});
    geographic_polygon.push_back(linearring);
    geographic_multipolygon.push_back(geographic_polygon);
    gis::Geographic_point geographic_point1{0.1, 0.1};
    EXPECT_DOUBLE_EQ(
        TestDistanceGeographic(geographic_multipolygon, geographic_point1), 0);
    EXPECT_DOUBLE_EQ(
        TestDistanceGeographic(geographic_point1, geographic_multipolygon), 0);
  }
  {
    gis::Geographic_multipolygon geographic_multipolygon;
    gis::Geographic_polygon geographic_polygon;
    gis::Geographic_linearring linearring =
        MakeLinearring({0, 0, 1, 0, 0, 1, 0, 0});
    geographic_polygon.push_back(linearring);
    linearring =
        MakeLinearring({0.05, 0.05, 0.05, 0.95, 0.95, 0.05, 0.05, 0.05});
    geographic_polygon.push_back(linearring);
    geographic_multipolygon.push_back(geographic_polygon);
    gis::Geographic_point geographic_point1{0.1, 0.1};
    EXPECT_GT(
        TestDistanceGeographic(geographic_multipolygon, geographic_point1), 0);
  }
  {
    gis::Geographic_multipolygon geographic_multipolygon;
    gis::Geographic_polygon geographic_polygon1 =
        MakePolygon({{0, 0, 1, 0, 0, 1, 0, 0},
                     {0.05, 0.05, 0.05, 0.95, 0.95, 0.05, 0.05, 0.05}});
    gis::Geographic_polygon geographic_polygon2 =
        MakePolygon({{0.5, 0, 1.5, 0, .5, 1, .5, 0},
                     {.55, 0.05, 0.55, 0.95, 0.55, 0.05, 0.55, 0.05}});
    geographic_multipolygon.push_back(geographic_polygon2);
    gis::Geographic_point geographic_point1{0.2, 0.2};
    gis::Geographic_point geographic_point2{0.525, 0.2};
    EXPECT_GT(
        TestDistanceGeographic(geographic_multipolygon, geographic_point1), 0);
    EXPECT_DOUBLE_EQ(
        TestDistanceGeographic(geographic_multipolygon, geographic_point2), 0);
  }
  {
    gis::Geographic_point geographic_point{0, 0};
    bool is_null = false;
    TestDistanceGeographic(gis::Geographic_multipolygon(), geographic_point,
                           &is_null);
    EXPECT_TRUE(is_null);
    is_null = false;
    TestDistanceGeographic(geographic_point, gis::Geographic_multipolygon(),
                           &is_null);
    EXPECT_TRUE(is_null);
  }
}
TEST(DistanceTest, Geographic_Multipolygon_Linestring) {
  gis::Geographic_polygon geographic_polygon =
      MakePolygon({{0, 0, 1, 0, 0, 1, 0, 0}});
  gis::Geographic_multipolygon geographic_multipolygon;
  geographic_multipolygon.push_back(geographic_polygon);
  gis::Geographic_linestring geographic_linestringtr1 =
      MakeLinestring({-1., 0.5, -.1, 0.5});
  EXPECT_GT(
      TestDistanceGeographic(geographic_multipolygon, geographic_linestringtr1),
      0);
  EXPECT_GT(
      TestDistanceGeographic(geographic_linestringtr1, geographic_multipolygon),
      0);
}

TEST(DistanceTest, Geographic_Multipolygon_polygon) {
  gis::Geographic_polygon geographic_polygon1 =
      MakePolygon({{0, 0, 1, 0, 0, 1, 0, 0}});
  gis::Geographic_polygon geographic_polygon2 =
      MakePolygon({{-1, 0, -.1, 0, -1, 1, -1, 0}});
  gis::Geographic_multipolygon geographic_multipolygon1;
  geographic_multipolygon1.push_back(geographic_polygon1);
  EXPECT_GT(
      TestDistanceGeographic(geographic_multipolygon1, geographic_polygon2), 0);
  EXPECT_DOUBLE_EQ(
      TestDistanceGeographic(geographic_multipolygon1, geographic_polygon2),
      TestDistanceGeographic(geographic_polygon2, geographic_multipolygon1));
}

TEST(DistanceTest, Geographic_Multipolygon_MultiPoint) {
  gis::Geographic_multipolygon geographic_multipolygon;
  gis::Geographic_polygon geographic_polygon1 =
      MakePolygon({{0, 0, 1, 0, 0, 1, 0, 0},
                   {0.05, 0.05, 0.05, 0.95, 0.95, 0.05, 0.05, 0.05}});
  geographic_multipolygon.push_back(geographic_polygon1);
  gis::Geographic_polygon geographic_polygon2 =
      MakePolygon({{0.5, 0, 1.5, 0, .5, 1, .5, 0},
                   {.55, 0.05, 0.55, 0.95, 0.55, 0.05, 0.55, 0.05}});
  geographic_multipolygon.push_back(geographic_polygon2);
  gis::Geographic_point geographic_point1{0.2, 0.2};
  gis::Geographic_point geographic_point2{0.525, 0.2};
  gis::Geographic_multipoint geographic_multipoint;
  geographic_multipoint.push_back(geographic_point1);
  geographic_multipoint.push_back(geographic_point2);
  EXPECT_DOUBLE_EQ(
      TestDistanceGeographic(geographic_multipolygon, geographic_multipoint),
      0);
  EXPECT_DOUBLE_EQ(
      TestDistanceGeographic(geographic_multipoint, geographic_multipolygon),
      0);
}

TEST(DistanceTest, Geographic_Multipolygon_Multilinestring) {
  {
    gis::Geographic_polygon geographic_polygon =
        MakePolygon({{0, 0, 1, 0, 0, 1, 0, 0}});
    gis::Geographic_multipolygon geographic_multipolygon;
    geographic_multipolygon.push_back(geographic_polygon);
    gis::Geographic_linestring geographic_linestring1 =
        MakeLinestring({-1., 0.5, -.1, 0.5});
    gis::Geographic_multilinestring geographic_multilinestring1;
    geographic_multilinestring1.push_back(geographic_linestring1);
    EXPECT_GT(TestDistanceGeographic(geographic_multipolygon,
                                     geographic_multilinestring1),
              0);
    EXPECT_DOUBLE_EQ(TestDistanceGeographic(geographic_multilinestring1,
                                            geographic_multipolygon),
                     TestDistanceGeographic(geographic_multipolygon,
                                            geographic_multilinestring1));
  }
  {
    bool is_null;
    TestDistanceGeographic(gis::Geographic_multipolygon(),
                           gis::Geographic_multilinestring(), &is_null);
    EXPECT_TRUE(is_null);
  }
  {
    gis::Geographic_multipolygon geographic_multipolygon =
        MakeMultipolygon({{{0, 0, 1, 0, 0, 1, 0, 0}}});
    bool is_null;
    TestDistanceGeographic(geographic_multipolygon,
                           gis::Geographic_multilinestring(), &is_null);
    EXPECT_TRUE(is_null);
  }
  {
    gis::Geographic_multilinestring geographic_multilinestring;
    geographic_multilinestring.push_back(MakeLinestring({-1., 0.5, -.1, 0.5}));
    bool is_null;
    TestDistanceGeographic(gis::Geographic_multipolygon(),
                           geographic_multilinestring, &is_null);
    EXPECT_TRUE(is_null);
  }
}

TEST(DistanceTest, Geographic_Multipolygon_Multipolygon) {
  gis::Geographic_polygon geographic_polygon1 =
      MakePolygon({{0, 0, 1, 0, 0, 1, 0, 0}});
  gis::Geographic_polygon geographic_polygon2 =
      MakePolygon({{-1, 0, -.1, 0, -1, 1, -1, 0}});
  gis::Geographic_multipolygon geographic_multipolygon1,
      geographic_multipolygon2;
  geographic_multipolygon1.push_back(geographic_polygon1);
  geographic_multipolygon2.push_back(geographic_polygon2);
  EXPECT_GT(TestDistanceGeographic(geographic_multipolygon1,
                                   geographic_multipolygon2),
            0);
  EXPECT_DOUBLE_EQ(TestDistanceGeographic(geographic_multipolygon1,
                                          geographic_multipolygon2),
                   TestDistanceGeographic(geographic_multipolygon2,
                                          geographic_multipolygon1));
}

TEST(DistanceTest, Geographic_GeometryCollection) {
  {
    gis::Geographic_geometrycollection geographic_geometrycollection;
    geographic_geometrycollection.push_back(
        MakeLinestring({-1., 0.5, -.1, 0.5}));
    gis::Geographic_point geographic_point{0, 0};
    EXPECT_GT(
        TestDistanceGeographic(geographic_geometrycollection, geographic_point),
        0);
  }
  {
    gis::Geographic_geometrycollection geographic_geometrycollection;
    geographic_geometrycollection.push_back(
        MakeLinestring({-1., 0.5, -.1, 0.5}));
    EXPECT_GT(TestDistanceGeographic(gis::Geographic_point{0, 0},
                                     geographic_geometrycollection),
              0);
    EXPECT_GT(TestDistanceGeographic(MakeLinestring({0, 0, 0.05, 0}),
                                     geographic_geometrycollection),
              0);
    EXPECT_GT(
        TestDistanceGeographic(MakePolygon({{0, 0, 0.5, 0, 0, 0.25, 0, 0}}),
                               geographic_geometrycollection),
        0);
    EXPECT_GT(TestDistanceGeographic(
                  MakeMultipoint({{0, 0}, {0.5, 0}, {0, 0.25}, {0, 0}}),
                  geographic_geometrycollection),
              0);
    EXPECT_GT(TestDistanceGeographic(MakeMultilinestring({{0, 0, 0.05, 0}}),
                                     geographic_geometrycollection),
              0);
    EXPECT_GT(TestDistanceGeographic(
                  MakeMultipolygon({{{0, 0, 0.5, 0, 0, 0.25, 0, 0}}}),
                  geographic_geometrycollection),
              0);
  }
}
TEST(DistanceTest, Bug29545865) {
  EXPECT_GT(
      TestDistanceGeographic(
          gis::Geographic_point{3.32696 / 180 * M_PI, -6.29345 / 180 * M_PI},
          MakeLinestring({-16.42203 / 180 * M_PI, -7.52882 / 180 * M_PI,
                          4.89998 / 180 * M_PI, -6.15568 / 180 * M_PI})),
      470.0);
  EXPECT_LT(
      TestDistanceGeographic(
          gis::Geographic_point{3.32696 / 180 * M_PI, -6.29345 / 180 * M_PI},
          MakeLinestring({-16.42203 / 180 * M_PI, -7.52882 / 180 * M_PI,
                          4.89998 / 180 * M_PI, -6.15568 / 180 * M_PI})),
      530.0);

  EXPECT_GT(
      TestDistanceGeographic(
          gis::Geographic_point{-7.35561 / 180 * M_PI, 7.2137 / 180 * M_PI},
          MakeLinestring({8.65279 / 180 * M_PI, -2.71668 / 180 * M_PI,
                          -7.13372 / 180 * M_PI, 8.35583 / 180 * M_PI,
                          -9.09998 / 180 * M_PI, -1.22625 / 180 * M_PI})),
      1500);
  EXPECT_LT(
      TestDistanceGeographic(
          gis::Geographic_point{-7.35561 / 180 * M_PI, 7.2137 / 180 * M_PI},
          MakeLinestring({8.65279 / 180 * M_PI, -2.71668 / 180 * M_PI,
                          -7.13372 / 180 * M_PI, 8.35583 / 180 * M_PI,
                          -9.09998 / 180 * M_PI, -1.22625 / 180 * M_PI})),
      2000);
}

}  // namespace distance_unittest

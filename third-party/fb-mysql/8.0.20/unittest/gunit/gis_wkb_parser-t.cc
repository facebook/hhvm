/*
  Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "my_config.h"

#include <gtest/gtest.h>
#include <memory>  // unique_ptr
#include <string>

#include "sql/dd/dd.h"
#include "sql/dd/impl/types/spatial_reference_system_impl.h"
#include "sql/dd/properties.h"
#include "sql/dd/types/spatial_reference_system.h"
#include "sql/gis/geometries.h"
#include "sql/gis/wkb.h"

namespace wkb_parser__unittest {

class WkbParserTest : public ::testing::Test {
 protected:
  dd::Spatial_reference_system_impl *m_cartesian_srs;
  dd::Spatial_reference_system_impl *m_lat_long_srs;
  dd::Spatial_reference_system_impl *m_long_lat_srs;
  dd::Spatial_reference_system_impl *m_tweaked_geo_srs;

  void SetUp() {
    m_cartesian_srs = dynamic_cast<dd::Spatial_reference_system_impl *>(
        dd::create_object<dd::Spatial_reference_system>());
    m_cartesian_srs->set_id(3857);
    m_cartesian_srs->set_name("WGS 84 / Pseudo-Mercator");
    m_cartesian_srs->set_created(0UL);
    m_cartesian_srs->set_last_altered(0UL);
    m_cartesian_srs->set_organization("EPSG");
    m_cartesian_srs->set_organization_coordsys_id(3857);
    m_cartesian_srs->set_definition(
        "PROJCS[\"WGS 84 / Pseudo-Mercator\",GEOGCS[\"WGS 84\",DATUM[\"World "
        "Geodetic System 1984\",SPHEROID[\"WGS "
        "84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY["
        "\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\","
        "\"8901\"]],UNIT[\"degree\",0.017453292519943278,AUTHORITY[\"EPSG\","
        "\"9122\"]],AXIS[\"Lat\",NORTH],AXIS[\"Lon\",EAST],AUTHORITY[\"EPSG\","
        "\"4326\"]],PROJECTION[\"Popular Visualisation Pseudo "
        "Mercator\",AUTHORITY[\"EPSG\",\"1024\"]],PARAMETER[\"Latitude of "
        "natural "
        "origin\",0,AUTHORITY[\"EPSG\",\"8801\"]],PARAMETER[\"Longitude of "
        "natural origin\",0,AUTHORITY[\"EPSG\",\"8802\"]],PARAMETER[\"False "
        "easting\",0,AUTHORITY[\"EPSG\",\"8806\"]],PARAMETER[\"False "
        "northing\",0,AUTHORITY[\"EPSG\",\"8807\"]],UNIT[\"metre\",1,AUTHORITY["
        "\"EPSG\",\"9001\"]],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH],AUTHORITY["
        "\"EPSG\",\"3857\"]]");
    m_cartesian_srs->set_description("");
    m_cartesian_srs->parse_definition();

    // Standard EPSG 4326, lat-long (N-E), Greenwich Meridian
    m_lat_long_srs = dynamic_cast<dd::Spatial_reference_system_impl *>(
        dd::create_object<dd::Spatial_reference_system>());
    m_lat_long_srs->set_id(4326);
    m_lat_long_srs->set_name("WGS 84");
    m_lat_long_srs->set_created(0UL);
    m_lat_long_srs->set_last_altered(0UL);
    m_lat_long_srs->set_organization("EPSG");
    m_lat_long_srs->set_organization_coordsys_id(4326);
    m_lat_long_srs->set_definition(
        "GEOGCS[\"WGS 84\",DATUM[\"World Geodetic System 1984\",SPHEROID[\"WGS "
        "84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY["
        "\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\","
        "\"8901\"]],UNIT[\"degree\",0.017453292519943278,AUTHORITY[\"EPSG\","
        "\"9122\"]],AXIS[\"Lat\",NORTH],AXIS[\"Lon\",EAST],AUTHORITY[\"EPSG\","
        "\"4326\"]]");
    m_lat_long_srs->set_description("");
    m_lat_long_srs->parse_definition();

    // Axis-swapped EPSG 4326, long-lat (E-N), Greenwich Meridian
    m_long_lat_srs = dynamic_cast<dd::Spatial_reference_system_impl *>(
        dd::create_object<dd::Spatial_reference_system>());
    m_long_lat_srs->set_id(4326);
    m_long_lat_srs->set_name("Long-Lat WGS 84");
    m_long_lat_srs->set_created(0UL);
    m_long_lat_srs->set_last_altered(0UL);
    m_long_lat_srs->set_organization("EPSG");
    m_long_lat_srs->set_organization_coordsys_id(4326);
    m_long_lat_srs->set_definition(
        "GEOGCS[\"Long-Lat WGS 84\",DATUM[\"World Geodetic System "
        "1984\",SPHEROID[\"WGS "
        "84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY["
        "\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\","
        "\"8901\"]],UNIT[\"degree\",0.017453292519943278,AUTHORITY[\"EPSG\","
        "\"9122\"]],AXIS[\"Lon\",EAST],AXIS[\"Lat\",NORTH],AUTHORITY[\"EPSG\","
        "\"4326\"]]");
    m_long_lat_srs->set_description("");
    m_long_lat_srs->parse_definition();

    // Lat-long (S-W) in grad, Meridian 10 grad East of Greenwich
    m_tweaked_geo_srs = dynamic_cast<dd::Spatial_reference_system_impl *>(
        dd::create_object<dd::Spatial_reference_system>());
    m_tweaked_geo_srs->set_id(1000000);
    m_tweaked_geo_srs->set_name("Tweaked");
    m_tweaked_geo_srs->set_created(0UL);
    m_tweaked_geo_srs->set_last_altered(0UL);
    m_tweaked_geo_srs->set_organization("EPSG");
    m_tweaked_geo_srs->set_organization_coordsys_id(1000000);
    m_tweaked_geo_srs->set_definition(
        "GEOGCS[\"Tweaked\",DATUM[\"World Geodetic System "
        "1984\",SPHEROID[\"WGS "
        "84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY["
        "\"EPSG\",\"6326\"]],PRIMEM[\"10 "
        "E\",10,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"grad\",0."
        "01570796326794895,AUTHORITY[\"EPSG\",\"9122\"]],AXIS[\"Lat\",SOUTH],"
        "AXIS[\"Lon\",WEST],AUTHORITY[\"EPSG\",\"4326\"]]");
    m_tweaked_geo_srs->set_description("");
    m_tweaked_geo_srs->parse_definition();
  }

  void TearDown() {
    delete m_cartesian_srs;
    delete m_lat_long_srs;
    delete m_long_lat_srs;
    delete m_tweaked_geo_srs;
  }

  WkbParserTest() {}

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(WkbParserTest);
};

TEST_F(WkbParserTest, Invalid) {
  std::unique_ptr<gis::Geometry> g;
  // Incorrect byte_order
  //              |bo||WKB type      ||x                             ||y |
  std::string bo(
      "\x03\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x3f\x00\x00\x00\x00\x00"
      "\x00\x00\x40",
      21);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, bo.c_str(), bo.length());
  EXPECT_FALSE(g);

  // Invalid WKB type
  //                        |bo||WKB type      ||x
  //                        ||y                             |
  std::string invalid_type(
      "\x01\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x3f\x00\x00\x00\x00\x00"
      "\x00\x00\x40",
      21);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, invalid_type.c_str(),
                     invalid_type.length());
  EXPECT_FALSE(g);

  // Too short WKB type
  //                      |bo||WKB type      |
  std::string short_type("\x01\xff\x00\x00", 4);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, short_type.c_str(),
                     short_type.length());
  EXPECT_FALSE(g);
}

TEST_F(WkbParserTest, Point) {
  std::unique_ptr<gis::Geometry> g;

  // Little-endian POINT(1 2)
  //              |bo||WKB type      ||x                             ||y |
  std::string le(
      "\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x3f\x00\x00\x00\x00\x00"
      "\x00\x00\x40",
      21);
  // Big-endian POINT(1 2)
  //              |bo||WKB type      ||x                             ||y |
  std::string be(
      "\x00\x00\x00\x00\x01\x3f\xf0\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00"
      "\x00\x00\x00",
      21);

  // Cartesian
  g = gis::parse_wkb(nullptr, m_cartesian_srs, le.c_str(), le.length());
  EXPECT_EQ(g->type(), gis::Geometry_type::kPoint);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).x(), 1.0);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).y(), 2.0);

  g = gis::parse_wkb(nullptr, m_cartesian_srs, be.c_str(), be.length());
  EXPECT_EQ(g->type(), gis::Geometry_type::kPoint);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).x(), 1.0);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).y(), 2.0);

  // Lat-long
  g = gis::parse_wkb(nullptr, m_lat_long_srs, le.c_str(), le.length());
  EXPECT_EQ(g->type(), gis::Geometry_type::kPoint);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).x(),
                   0.034906585039886556);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).y(),
                   0.017453292519943278);

  g = gis::parse_wkb(nullptr, m_lat_long_srs, be.c_str(), be.length());
  EXPECT_EQ(g->type(), gis::Geometry_type::kPoint);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).x(),
                   0.034906585039886556);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).y(),
                   0.017453292519943278);

  // Long-lat
  g = gis::parse_wkb(nullptr, m_long_lat_srs, le.c_str(), le.length());
  EXPECT_EQ(g->type(), gis::Geometry_type::kPoint);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).x(),
                   0.017453292519943278);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).y(),
                   0.034906585039886556);

  g = gis::parse_wkb(nullptr, m_long_lat_srs, be.c_str(), be.length());
  EXPECT_EQ(g->type(), gis::Geometry_type::kPoint);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).x(),
                   0.017453292519943278);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).y(),
                   0.034906585039886556);

  // Lat-long, grad, meridian 10 grad East of Greenwich
  g = gis::parse_wkb(nullptr, m_tweaked_geo_srs, le.c_str(), le.length());
  EXPECT_EQ(g->type(), gis::Geometry_type::kPoint);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).x(), 0.1256637061435916);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).y(),
                   -0.01570796326794895);

  g = gis::parse_wkb(nullptr, m_tweaked_geo_srs, be.c_str(), be.length());
  EXPECT_EQ(g->type(), gis::Geometry_type::kPoint);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).x(), 0.1256637061435916);
  EXPECT_DOUBLE_EQ(static_cast<gis::Point &>(*g.get()).y(),
                   -0.01570796326794895);

  // One byte shorter than it should be
  //                     |bo||WKB type      ||x                             ||y
  //                     |
  std::string too_short(
      "\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x3f\x00\x00\x00\x00\x00"
      "\x00\x00",
      20);
  // One byte longer than WKB data
  //                    |bo||WKB type      ||x                             ||y |
  std::string too_long(
      "\x00\x00\x00\x00\x01\x3f\xf0\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00"
      "\x00\x00\x00\x00",
      22);

  g = gis::parse_wkb(nullptr, m_cartesian_srs, too_short.c_str(),
                     too_short.length());
  EXPECT_FALSE(g);

  g = gis::parse_wkb(nullptr, m_cartesian_srs, too_long.c_str(),
                     too_long.length());
  EXPECT_FALSE(g);
}

TEST_F(WkbParserTest, Linestring) {
  std::unique_ptr<gis::Geometry> g;

  // Linestring with no points
  //                    |bo||WKB type      ||numPoints     |
  std::string empty_ls("\x01\x02\x00\x00\x00\x00\x00\x00\x00", 9);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, empty_ls.c_str(),
                     empty_ls.length());
  EXPECT_FALSE(g);

  // Linestring with one point
  //                  |bo||WKB type      ||numPoints     ||x
  //                  ||y                             |
  std::string one_ls(
      "\x01\x02\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00",
      25);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, one_ls.c_str(), one_ls.length());
  EXPECT_FALSE(g);

  // Linestring with two points
  //                  |bo||WKB type      ||numPoints     ||x
  //                  ||y                             ||x
  //                  ||y                             |
  std::string two_ls(
      "\x01\x02\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00",
      41);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, two_ls.c_str(), two_ls.length());
  EXPECT_EQ(g->type(), gis::Geometry_type::kLinestring);
  EXPECT_FALSE(static_cast<gis::Linestring *>(g.get())->empty());
  EXPECT_EQ(static_cast<gis::Linestring *>(g.get())->size(), 2UL);

  //                    |x                             ||y |
  std::string pt_0_0(
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
  std::string pt_10_0(
      "\x00\x00\x00\x00\x00\x00\x24\x40\x00\x00\x00\x00\x00\x00\x00\x00", 16);
  std::string pt_10_10(
      "\x00\x00\x00\x00\x00\x00\x24\x40\x00\x00\x00\x00\x00\x00\x24\x40", 16);
  std::string pt_0_10(
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x24\x40", 16);
  //              |bo||WKB type      ||numPoints     |
  std::string ls("\x01\x02\x00\x00\x00\x04\x00\x00\x00", 9);
  ls.append(pt_0_0);
  ls.append(pt_10_0);
  ls.append(pt_10_10);
  ls.append(pt_10_0);

  g = gis::parse_wkb(nullptr, m_cartesian_srs, ls.c_str(), ls.length());
  EXPECT_EQ(g->type(), gis::Geometry_type::kLinestring);
  EXPECT_FALSE(static_cast<gis::Linestring *>(g.get())->empty());
  EXPECT_EQ(static_cast<gis::Linestring *>(g.get())->size(), 4UL);
}

TEST_F(WkbParserTest, Polygon) {
  std::unique_ptr<gis::Geometry> g;

  // Polygon with no rings
  //                    |bo||WKB type      ||numRings      |
  std::string empty_py("\x01\x03\x00\x00\x00\x00\x00\x00\x00", 9);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, empty_py.c_str(),
                     empty_py.length());
  EXPECT_FALSE(g);

  // Polygon with exterior ring with no points
  //                   |bo||WKB type      ||numRings      ||numPoints     |
  std::string zero_py("\x01\x03\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00",
                      13);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, zero_py.c_str(),
                     zero_py.length());
  EXPECT_FALSE(g);

  // Polygon with exterior ring with one point
  //                  |bo||WKB type      ||numRings      ||numPoints     ||x
  //                  ||y                             |
  std::string one_py(
      "\x01\x03\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      29);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, one_py.c_str(), one_py.length());
  EXPECT_FALSE(g);

  // Polygon with exterior ring with two points
  //                  |bo||WKB type      ||numRings      ||numPoints     ||x
  //                  ||y                             ||x
  //                  ||y                             |
  std::string two_py(
      "\x01\x03\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      45);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, two_py.c_str(), two_py.length());
  EXPECT_FALSE(g);

  // Polygon with exterior ring with three points
  //                    |bo||WKB type      ||numRings      ||numPoints     ||x
  //                    ||y                             ||x
  //                    ||y                             ||x
  //                    ||y                             |
  std::string three_py(
      "\x01\x03\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00",
      61);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, three_py.c_str(),
                     three_py.length());
  EXPECT_FALSE(g);

  //                    |x                             ||y |
  std::string pt_0_0(
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
  std::string pt_10_0(
      "\x00\x00\x00\x00\x00\x00\x24\x40\x00\x00\x00\x00\x00\x00\x00\x00", 16);
  std::string pt_10_10(
      "\x00\x00\x00\x00\x00\x00\x24\x40\x00\x00\x00\x00\x00\x00\x24\x40", 16);
  std::string pt_0_10(
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x24\x40", 16);
  std::string exterior("\x05\x00\x00\x00", 4);
  exterior.append(pt_0_0);
  exterior.append(pt_10_0);
  exterior.append(pt_10_10);
  exterior.append(pt_0_10);
  exterior.append(pt_0_0);
  //                  |x                             ||y |
  std::string pt_2_2(
      "\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x40", 16);
  std::string pt_4_2(
      "\x00\x00\x00\x00\x00\x00\x10\x40\x00\x00\x00\x00\x00\x00\x00\x40", 16);
  std::string pt_4_4(
      "\x00\x00\x00\x00\x00\x00\x10\x40\x00\x00\x00\x00\x00\x00\x10\x40", 16);
  std::string pt_2_4(
      "\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x10\x40", 16);
  //                     |numPoints     |
  std::string interior1("\x05\x00\x00\x00", 4);
  interior1.append(pt_2_2);
  interior1.append(pt_2_4);
  interior1.append(pt_4_4);
  interior1.append(pt_4_2);
  interior1.append(pt_2_2);
  //                  |x                             ||y |
  std::string pt_6_6(
      "\x00\x00\x00\x00\x00\x00\x18\x40\x00\x00\x00\x00\x00\x00\x18\x40", 16);
  std::string pt_6_8(
      "\x00\x00\x00\x00\x00\x00\x18\x40\x00\x00\x00\x00\x00\x00\x20\x40", 16);
  std::string pt_8_8(
      "\x00\x00\x00\x00\x00\x00\x20\x40\x00\x00\x00\x00\x00\x00\x20\x40", 16);
  //                     |numPoints     |
  std::string interior2("\x04\x00\x00\x00", 4);
  interior2.append(pt_6_6);
  interior2.append(pt_6_8);
  interior2.append(pt_8_8);
  interior2.append(pt_6_6);
  // Little-endian POLYGON((0 0, 10 0, 10 10, 0 10, 0 0), (2 2, 2 4, 4 4, 4 2),
  // (6 6, 6 8, 8 8, 6 6))
  //              |bo||WKB type      ||numRings      |
  std::string py("\x01\x03\x00\x00\x00\x03\x00\x00\x00", 9);
  py.append(exterior);
  py.append(interior1);
  py.append(interior2);

  g = gis::parse_wkb(nullptr, m_cartesian_srs, py.c_str(), py.length());
  EXPECT_EQ(g->type(), gis::Geometry_type::kPolygon);
  EXPECT_EQ(static_cast<gis::Polygon *>(g.get())->size(), 3UL);
  EXPECT_FALSE(static_cast<gis::Polygon *>(g.get())->empty());
  EXPECT_EQ(static_cast<gis::Polygon *>(g.get())->interior_ring(1).size(), 4UL);
}

TEST_F(WkbParserTest, Multipoint) {
  std::unique_ptr<gis::Geometry> g;

  // Little-endian POINT(1 2)
  //              |bo||WKB type      ||x                             ||y |
  std::string le(
      "\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x3f\x00\x00\x00\x00\x00"
      "\x00\x00\x40",
      21);
  // Big-endian POINT(1 2)
  //              |bo||WKB type      ||x                             ||y |
  std::string be(
      "\x00\x00\x00\x00\x01\x3f\xf0\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00"
      "\x00\x00\x00",
      21);
  // Mixed-endian MULTIPOINT((1 2), (1 2))
  //               |bo||WKB type      ||numPoints     |
  std::string mpt("\x01\x04\x00\x00\x00\x02\x00\x00\x00", 9);
  mpt.append(le);
  mpt.append(be);

  g = gis::parse_wkb(nullptr, m_cartesian_srs, mpt.c_str(), mpt.length());
  EXPECT_EQ(g->type(), gis::Geometry_type::kMultipoint);
  EXPECT_FALSE(static_cast<gis::Multipoint *>(g.get())->empty());
  EXPECT_EQ(static_cast<gis::Multipoint *>(g.get())->size(), 2UL);

  // Multipoint with no points
  //                     |bo||WKB type      ||numPoints     |
  std::string empty_mpt("\x01\x04\x00\x00\x00\x00\x00\x00\x00", 9);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, empty_mpt.c_str(),
                     empty_mpt.length());
  EXPECT_FALSE(g);
}

TEST_F(WkbParserTest, Multilinestring) {
  std::unique_ptr<gis::Geometry> g;

  //                    |x                             ||y |
  std::string pt_0_0(
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
  std::string pt_10_0(
      "\x00\x00\x00\x00\x00\x00\x24\x40\x00\x00\x00\x00\x00\x00\x00\x00", 16);
  std::string pt_10_10(
      "\x00\x00\x00\x00\x00\x00\x24\x40\x00\x00\x00\x00\x00\x00\x24\x40", 16);
  std::string pt_0_10(
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x24\x40", 16);
  //              |bo||WKB type      ||numPoints     |
  std::string ls("\x01\x02\x00\x00\x00\x04\x00\x00\x00", 9);
  ls.append(pt_0_0);
  ls.append(pt_10_0);
  ls.append(pt_10_10);
  ls.append(pt_10_0);
  // Mixed-endian MULTILINESTRING((0 0, 10 0, 10 10, 0 10))
  //               |bo||WKB type      ||numLinestrings|
  std::string mls("\x00\x00\x00\x00\x05\x00\x00\x00\x01", 9);
  mls.append(ls);

  g = gis::parse_wkb(nullptr, m_cartesian_srs, mls.c_str(), mls.length());
  EXPECT_EQ(g->type(), gis::Geometry_type::kMultilinestring);
  EXPECT_FALSE(static_cast<gis::Multilinestring *>(g.get())->empty());
  EXPECT_EQ(static_cast<gis::Multilinestring *>(g.get())->size(), 1UL);

  // Multilinestrings with no linestrings
  //                     |bo||WKB type      ||numLinestrings|
  std::string empty_mls("\x01\x05\x00\x00\x00\x00\x00\x00\x00", 9);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, empty_mls.c_str(),
                     empty_mls.length());
  EXPECT_FALSE(g);
}

TEST_F(WkbParserTest, Multipolygon) {
  std::unique_ptr<gis::Geometry> g;

  //                    |x                             ||y |
  std::string pt_0_0(
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
  std::string pt_10_0(
      "\x00\x00\x00\x00\x00\x00\x24\x40\x00\x00\x00\x00\x00\x00\x00\x00", 16);
  std::string pt_10_10(
      "\x00\x00\x00\x00\x00\x00\x24\x40\x00\x00\x00\x00\x00\x00\x24\x40", 16);
  std::string pt_0_10(
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x24\x40", 16);
  std::string exterior("\x05\x00\x00\x00", 4);
  exterior.append(pt_0_0);
  exterior.append(pt_10_0);
  exterior.append(pt_10_10);
  exterior.append(pt_0_10);
  exterior.append(pt_0_0);
  //                  |x                             ||y |
  std::string pt_2_2(
      "\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x40", 16);
  std::string pt_4_2(
      "\x00\x00\x00\x00\x00\x00\x10\x40\x00\x00\x00\x00\x00\x00\x00\x40", 16);
  std::string pt_4_4(
      "\x00\x00\x00\x00\x00\x00\x10\x40\x00\x00\x00\x00\x00\x00\x10\x40", 16);
  std::string pt_2_4(
      "\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x10\x40", 16);
  //                     |numPoints     |
  std::string interior1("\x05\x00\x00\x00", 4);
  interior1.append(pt_2_2);
  interior1.append(pt_2_4);
  interior1.append(pt_4_4);
  interior1.append(pt_4_2);
  interior1.append(pt_2_2);
  //                  |x                             ||y |
  std::string pt_6_6(
      "\x00\x00\x00\x00\x00\x00\x18\x40\x00\x00\x00\x00\x00\x00\x18\x40", 16);
  std::string pt_6_8(
      "\x00\x00\x00\x00\x00\x00\x18\x40\x00\x00\x00\x00\x00\x00\x20\x40", 16);
  std::string pt_8_8(
      "\x00\x00\x00\x00\x00\x00\x20\x40\x00\x00\x00\x00\x00\x00\x20\x40", 16);
  //                     |numPoints     |
  std::string interior2("\x04\x00\x00\x00", 4);
  interior2.append(pt_6_6);
  interior2.append(pt_6_8);
  interior2.append(pt_8_8);
  interior2.append(pt_6_6);
  //              |bo||WKB type      ||numRings      |
  std::string py("\x01\x03\x00\x00\x00\x03\x00\x00\x00", 9);
  py.append(exterior);
  py.append(interior1);
  py.append(interior2);
  // Mixed-endian MULTIPOLYGON(((0 0, 10 0, 10 10, 0 10, 0 0),
  // (2 2, 2 4, 4 4, 4 2), (6 6, 6 8, 8 8, 6 6)))
  //               |bo||WKB type      ||numPolygons   |
  std::string mpy("\x00\x00\x00\x00\x06\x00\x00\x00\x01", 9);
  mpy.append(py);

  g = gis::parse_wkb(nullptr, m_cartesian_srs, mpy.c_str(), mpy.length());
  EXPECT_EQ(g->type(), gis::Geometry_type::kMultipolygon);
  EXPECT_FALSE(static_cast<gis::Multipolygon *>(g.get())->empty());
  EXPECT_EQ(static_cast<gis::Multipolygon *>(g.get())->size(), 1UL);

  // Multipolygon with no polygons
  //                     |bo||WKB type      ||numPolygons   |
  std::string empty_mpy("\x01\x06\x00\x00\x00\x00\x00\x00\x00", 9);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, empty_mpy.c_str(),
                     empty_mpy.length());
  EXPECT_FALSE(g);
}

TEST_F(WkbParserTest, Geometrycollection) {
  std::unique_ptr<gis::Geometry> g;

  // Little-endian POINT(1 2)
  //              |bo||WKB type      ||x                             ||y |
  std::string le(
      "\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x3f\x00\x00\x00\x00\x00"
      "\x00\x00\x40",
      21);
  // Big-endian POINT(1 2)
  //              |bo||WKB type      ||x                             ||y |
  std::string be(
      "\x00\x00\x00\x00\x01\x3f\xf0\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00"
      "\x00\x00\x00",
      21);
  // Mixed-endian MULTIPOINT((1 2), (1 2))
  //              |bo||WKB type      ||numPoints     |
  std::string gc("\x01\x07\x00\x00\x00\x02\x00\x00\x00", 9);
  gc.append(le);
  gc.append(be);

  g = gis::parse_wkb(nullptr, m_cartesian_srs, gc.c_str(), gc.length());
  EXPECT_EQ(g->type(), gis::Geometry_type::kGeometrycollection);
  EXPECT_FALSE(static_cast<gis::Geometrycollection *>(g.get())->empty());
  EXPECT_EQ(static_cast<gis::Geometrycollection *>(g.get())->size(), 2UL);

  // Empty geometrycollection
  //                    |bo||WKB type      ||numGeometries |
  std::string empty_gc("\x01\x07\x00\x00\x00\x00\x00\x00\x00", 9);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, empty_gc.c_str(),
                     empty_gc.length());
  EXPECT_TRUE(g);

  // Geometry collection with empty geometrycollection
  //                      |bo||WKB type      ||numGeometries ||bo||WKB type
  //                      ||numGeometries |
  std::string gc_empty_gc(
      "\x01\x07\x00\x00\x00\x01\x00\x00\x00\x01\x07\x00\x00\x00\x00\x00\x00"
      "\x00",
      18);
  g = gis::parse_wkb(nullptr, m_cartesian_srs, gc_empty_gc.c_str(),
                     gc_empty_gc.length());
  EXPECT_TRUE(g);
  EXPECT_FALSE(static_cast<gis::Geometrycollection *>(g.get())->empty());
  EXPECT_TRUE(static_cast<gis::Geometrycollection *>(g.get())->is_empty());
}

}  // namespace wkb_parser__unittest

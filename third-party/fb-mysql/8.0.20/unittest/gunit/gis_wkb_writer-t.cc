// Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
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
/// This file test gis::write_geometry and the gis::Wkb_visitor class.

#include "my_config.h"

#include <gtest/gtest.h>
#include <memory>  // unique_ptr
#include <string>

#include "sql/dd/dd.h"
#include "sql/dd/impl/types/spatial_reference_system_impl.h"
#include "sql/dd/properties.h"
#include "sql/dd/types/spatial_reference_system.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_cs.h"
#include "sql/gis/wkb.h"

namespace wkb_writer_unittest {

class WkbWriterTest : public ::testing::Test {
 protected:
  dd::Spatial_reference_system_impl *m_cartesian_srs;
  dd::Spatial_reference_system_impl *m_radian_srs;
  dd::Spatial_reference_system_impl *m_grad_srs;

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

    // EPSG 4326, lat-long (N-E), Greenwich Meridian, but in radians
    m_radian_srs = dynamic_cast<dd::Spatial_reference_system_impl *>(
        dd::create_object<dd::Spatial_reference_system>());
    m_radian_srs->set_id(1000000);
    m_radian_srs->set_name("WGS 84");
    m_radian_srs->set_created(0UL);
    m_radian_srs->set_last_altered(0UL);
    m_radian_srs->set_organization("EPSG");
    m_radian_srs->set_organization_coordsys_id(4326);
    m_radian_srs->set_definition(
        "GEOGCS[\"WGS 84\",DATUM[\"World Geodetic System 1984\",SPHEROID[\"WGS "
        "84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY["
        "\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\","
        "\"8901\"]],UNIT[\"radian\",1.0],AXIS[\"Lat\",NORTH],AXIS[\"Lon\",EAST]"
        ",AUTHORITY[\"EPSG\",\"4326\"]]");
    m_radian_srs->set_description("");
    m_radian_srs->parse_definition();

    // Lat-long (S-W) in grad, Meridian 10 grad East of Greenwich
    m_grad_srs = dynamic_cast<dd::Spatial_reference_system_impl *>(
        dd::create_object<dd::Spatial_reference_system>());
    m_grad_srs->set_id(1000001);
    m_grad_srs->set_name("Tweaked");
    m_grad_srs->set_created(0UL);
    m_grad_srs->set_last_altered(0UL);
    m_grad_srs->set_organization("EPSG");
    m_grad_srs->set_organization_coordsys_id(1000000);
    m_grad_srs->set_definition(
        "GEOGCS[\"Tweaked\",DATUM[\"World Geodetic System "
        "1984\",SPHEROID[\"WGS "
        "84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY["
        "\"EPSG\",\"6326\"]],PRIMEM[\"10 "
        "E\",10,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"grad\",0."
        "01570796326794895,AUTHORITY[\"EPSG\",\"9122\"]],AXIS[\"Lat\",SOUTH],"
        "AXIS[\"Lon\",WEST],AUTHORITY[\"EPSG\",\"4326\"]]");
    m_grad_srs->set_description("");
    m_grad_srs->parse_definition();
  }

  void TearDown() {
    delete m_cartesian_srs;
    delete m_radian_srs;
    delete m_grad_srs;
  }

  WkbWriterTest() {}

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(WkbWriterTest);
};

TEST_F(WkbWriterTest, Point) {
  gis::Cartesian_point cpt(1, 2);
  gis::Geographic_point gpt(1, 2);

  // Little-endian POINT(1 2)
  std::string point(
      "\x00\x00\x00\x00"                   // SRID
      "\x01"                               // Byte order
      "\x01\x00\x00\x00"                   // WKB type
      "\x00\x00\x00\x00\x00\x00\xf0\x3f"   // X
      "\x00\x00\x00\x00\x00\x00\x00\x40",  // Y
      25);

  // Cartesian
  String srid0_point;
  gis::write_geometry(nullptr, cpt, &srid0_point);
  EXPECT_EQ(point.size(), srid0_point.length());
  EXPECT_FALSE(memcmp(point.c_str(), srid0_point.c_ptr(), point.size()));

  String cartesian_point;
  gis::write_geometry(m_cartesian_srs, cpt, &cartesian_point);
  EXPECT_EQ(point.size(), cartesian_point.length());
  EXPECT_FALSE(memcmp("\x11\x0f\x00\x00", cartesian_point.c_ptr(), 4));
  EXPECT_FALSE(
      memcmp(point.c_str() + 4, cartesian_point.c_ptr() + 4, point.size() - 4));

  // Radians
  String radian_point;
  gis::write_geometry(m_radian_srs, gpt, &radian_point);
  EXPECT_EQ(point.size(), radian_point.length());
  EXPECT_FALSE(memcmp("\x40\x42\x0f\x00", radian_point.c_ptr(), 4));
  EXPECT_FALSE(
      memcmp(point.c_str() + 4, radian_point.c_ptr() + 4, point.size() - 4));

  // Lat-long (S-W), grad, meridian 10 grad East of Greenwich. Point is 1 grad
  // West (=9 grad East of Greenwich), 2 grad South.
  gis::Geographic_point grad_pt(0.14137166941154056, -0.031415926535897899);
  String grad_point;
  gis::write_geometry(m_grad_srs, grad_pt, &grad_point);
  EXPECT_EQ(point.size(), grad_point.length());
  EXPECT_FALSE(memcmp("\x41\x42\x0f\x00", grad_point.c_ptr(), 4));
  EXPECT_FALSE(
      memcmp(point.c_str() + 4, grad_point.c_ptr() + 4, point.size() - 4));
}

TEST_F(WkbWriterTest, Linestring) {
  gis::Cartesian_linestring cls;
  cls.push_back(gis::Cartesian_point(0, 0));
  cls.push_back(gis::Cartesian_point(0, 0));
  gis::Geographic_linestring gls;
  gls.push_back(gis::Geographic_point(0, 0));
  gls.push_back(gis::Geographic_point(0, 0));

  // Linestring with two points
  std::string linestring(
      "\x00\x00\x00\x00"                   // SRID
      "\x01"                               // Byte order
      "\x02\x00\x00\x00"                   // WKB type
      "\x02\x00\x00\x00"                   // numPoints
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // X
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // Y
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // X
      "\x00\x00\x00\x00\x00\x00\x00\x00",  // Y
      45);

  // Cartesian
  String srid0_linestring;
  gis::write_geometry(nullptr, cls, &srid0_linestring);
  EXPECT_EQ(linestring.size(), srid0_linestring.length());
  EXPECT_FALSE(
      memcmp(linestring.c_str(), srid0_linestring.c_ptr(), linestring.size()));

  String cartesian_linestring;
  gis::write_geometry(m_cartesian_srs, cls, &cartesian_linestring);
  EXPECT_EQ(linestring.size(), cartesian_linestring.length());
  EXPECT_FALSE(memcmp("\x11\x0f\x00\x00", cartesian_linestring.c_ptr(), 4));
  EXPECT_FALSE(memcmp(linestring.c_str() + 4, cartesian_linestring.c_ptr() + 4,
                      linestring.size() - 4));

  // Radians
  String radian_linestring;
  gis::write_geometry(m_radian_srs, gls, &radian_linestring);
  EXPECT_EQ(linestring.size(), radian_linestring.length());
  EXPECT_FALSE(memcmp("\x40\x42\x0f\x00", radian_linestring.c_ptr(), 4));
  EXPECT_FALSE(memcmp(linestring.c_str() + 4, radian_linestring.c_ptr() + 4,
                      linestring.size() - 4));
}

TEST_F(WkbWriterTest, Polygon) {
  gis::Cartesian_polygon cpy;
  gis::Cartesian_linearring cexterior;
  cexterior.push_back(gis::Cartesian_point(0, 0));
  cexterior.push_back(gis::Cartesian_point(0, 0));
  cexterior.push_back(gis::Cartesian_point(0, 0));
  cexterior.push_back(gis::Cartesian_point(0, 0));
  cpy.push_back(cexterior);
  gis::Cartesian_linearring cinterior;
  cinterior.push_back(gis::Cartesian_point(0, 0));
  cinterior.push_back(gis::Cartesian_point(0, 0));
  cinterior.push_back(gis::Cartesian_point(0, 0));
  cinterior.push_back(gis::Cartesian_point(0, 0));
  cpy.push_back(cinterior);

  gis::Geographic_polygon gpy;
  gis::Geographic_linearring gexterior;
  gexterior.push_back(gis::Geographic_point(0, 0));
  gexterior.push_back(gis::Geographic_point(0, 0));
  gexterior.push_back(gis::Geographic_point(0, 0));
  gexterior.push_back(gis::Geographic_point(0, 0));
  gpy.push_back(gexterior);
  gis::Geographic_linearring ginterior;
  ginterior.push_back(gis::Geographic_point(0, 0));
  ginterior.push_back(gis::Geographic_point(0, 0));
  ginterior.push_back(gis::Geographic_point(0, 0));
  ginterior.push_back(gis::Geographic_point(0, 0));
  gpy.push_back(ginterior);

  // Polygon with two rings, each with four points
  std::string polygon(
      "\x00\x00\x00\x00"                   // SRID
      "\x01"                               // Byte order
      "\x03\x00\x00\x00"                   // WKB type
      "\x02\x00\x00\x00"                   // numRings
      "\x04\x00\x00\x00"                   // numPoints
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // X
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // Y
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // X
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // Y
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // X
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // Y
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // X
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // Y
      "\x04\x00\x00\x00"                   // numPoints
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // X
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // Y
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // X
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // Y
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // X
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // Y
      "\x00\x00\x00\x00\x00\x00\x00\x00"   // X
      "\x00\x00\x00\x00\x00\x00\x00\x00",  // Y
      149);

  // Cartesian
  String srid0_polygon;
  gis::write_geometry(nullptr, cpy, &srid0_polygon);
  EXPECT_EQ(polygon.size(), srid0_polygon.length());
  EXPECT_FALSE(memcmp(polygon.c_str(), srid0_polygon.c_ptr(), polygon.size()));

  String cartesian_polygon;
  gis::write_geometry(m_cartesian_srs, cpy, &cartesian_polygon);
  EXPECT_EQ(polygon.size(), cartesian_polygon.length());
  EXPECT_FALSE(memcmp("\x11\x0f\x00\x00", cartesian_polygon.c_ptr(), 4));
  EXPECT_FALSE(memcmp(polygon.c_str() + 4, cartesian_polygon.c_ptr() + 4,
                      polygon.size() - 4));

  // Radians
  String radian_polygon;
  gis::write_geometry(m_radian_srs, gpy, &radian_polygon);
  EXPECT_EQ(polygon.size(), radian_polygon.length());
  EXPECT_FALSE(memcmp("\x40\x42\x0f\x00", radian_polygon.c_ptr(), 4));
  EXPECT_FALSE(memcmp(polygon.c_str() + 4, radian_polygon.c_ptr() + 4,
                      polygon.size() - 4));
}

TEST_F(WkbWriterTest, Geometrycollection) {
  gis::Cartesian_geometrycollection cgc;
  cgc.push_back(gis::Cartesian_point(1, 2));
  gis::Geographic_geometrycollection ggc;
  ggc.push_back(gis::Geographic_point(1, 2));

  // Little-endian GEOMETRYCOLLECTION(POINT(1 2));
  std::string geometrycollection(
      "\x00\x00\x00\x00"                   // SRID
      "\x01"                               // Byte order
      "\x07\x00\x00\x00"                   // WKB type
      "\x01\x00\x00\x00"                   // numGeometries
      "\x01"                               // Byte order
      "\x01\x00\x00\x00"                   // WKB type
      "\x00\x00\x00\x00\x00\x00\xf0\x3f"   // X
      "\x00\x00\x00\x00\x00\x00\x00\x40",  // Y
      34);

  // Cartesian
  String srid0_geometrycollection;
  gis::write_geometry(nullptr, cgc, &srid0_geometrycollection);
  EXPECT_EQ(geometrycollection.size(), srid0_geometrycollection.length());
  EXPECT_FALSE(memcmp(geometrycollection.c_str(),
                      srid0_geometrycollection.c_ptr(),
                      geometrycollection.size()));

  String cartesian_geometrycollection;
  gis::write_geometry(m_cartesian_srs, cgc, &cartesian_geometrycollection);
  EXPECT_EQ(geometrycollection.size(), cartesian_geometrycollection.length());
  EXPECT_FALSE(
      memcmp("\x11\x0f\x00\x00", cartesian_geometrycollection.c_ptr(), 4));
  EXPECT_FALSE(memcmp(geometrycollection.c_str() + 4,
                      cartesian_geometrycollection.c_ptr() + 4,
                      geometrycollection.size() - 4));

  // Radians
  String radian_geometrycollection;
  gis::write_geometry(m_radian_srs, ggc, &radian_geometrycollection);
  EXPECT_EQ(geometrycollection.size(), radian_geometrycollection.length());
  EXPECT_FALSE(
      memcmp("\x40\x42\x0f\x00", radian_geometrycollection.c_ptr(), 4));
  EXPECT_FALSE(memcmp(geometrycollection.c_str() + 4,
                      radian_geometrycollection.c_ptr() + 4,
                      geometrycollection.size() - 4));
}

}  // namespace wkb_writer_unittest

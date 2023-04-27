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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

#include "my_config.h"

#include <gtest/gtest.h>
#include <memory>  // unique_ptr

#include "sql/dd/dd.h"
#include "sql/dd/impl/types/spatial_reference_system_impl.h"
#include "sql/dd/properties.h"
#include "sql/dd/types/spatial_reference_system.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_cs.h"
#include "sql/gis/relops.h"

namespace gis_srs_unittest {

TEST(SrsTest, GeogcsProj4Parameters) {
  dd::Spatial_reference_system_impl *srs;
  srs = dynamic_cast<dd::Spatial_reference_system_impl *>(
      dd::create_object<dd::Spatial_reference_system>());
  srs->set_id(1000000);
  srs->set_name("Test");
  srs->set_created(0UL);
  srs->set_last_altered(0UL);

  // Ellipsoid without TOWGS84.
  srs->set_definition(
      "GEOGCS[\"WGS 84\",DATUM[\"World Geodetic System 1984\",SPHEROID[\"WGS "
      "84\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0."
      "017453292519943278],AXIS[\"Lat\",NORTH],AXIS[\"Lon\",EAST]]");
  srs->parse_definition();
  EXPECT_STREQ(srs->proj4_parameters().c_str(), "");

  // Ellipsoid recognized as WGS 84 (authority clause on GEOGCS).
  srs->set_definition(
      "GEOGCS[\"WGS 84\",DATUM[\"World Geodetic System 1984\",SPHEROID[\"WGS "
      "84\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0."
      "017453292519943278],AXIS[\"Lat\",NORTH],AXIS[\"Lon\",EAST],AUTHORITY["
      "\"EPSG\",\"4326\"]]");
  srs->parse_definition();
  EXPECT_STREQ(srs->proj4_parameters().c_str(),
               "+proj=lonlat +a=6378137 +rf=298.257223563 "
               "+towgs84=0,0,0,0,0,0,0 +no_defs");

  // Ellipsoid with TOWGS84.
  srs->set_definition(
      "GEOGCS[\"WGS 84\",DATUM[\"World Geodetic System 1984\",SPHEROID[\"WGS "
      "84\",6378137,298.257223563],TOWGS84[1,2,3,4,5,6,7]],PRIMEM["
      "\"Greenwich\",0],UNIT[\"degree\",0.017453292519943278],AXIS[\"Lat\","
      "NORTH],AXIS[\"Lon\",EAST]]");
  srs->parse_definition();
  EXPECT_STREQ(srs->proj4_parameters().c_str(),
               "+proj=lonlat +a=6378137 +rf=298.257223563 "
               "+towgs84=1,2,3,4,5,6,7 +no_defs");

  // Sphere without TOWGS84.
  srs->set_definition(
      "GEOGCS[\"Sphere\",DATUM[\"Sphere\",SPHEROID[\"Sphere\",6378137,0]],"
      "PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.017453292519943278],AXIS["
      "\"Lat\",NORTH],AXIS[\"Lon\",EAST]]");
  srs->parse_definition();
  EXPECT_STREQ(srs->proj4_parameters().c_str(), "");

  // Sphere with TOWGS84.
  srs->set_definition(
      "GEOGCS[\"Sphere\",DATUM[\"Sphere\",SPHEROID[\"Sphere\",6378137,0],"
      "TOWGS84[1,2,3,4,5,6,7]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0."
      "017453292519943278],AXIS[\"Lat\",NORTH],AXIS[\"Lon\",EAST]]");
  srs->parse_definition();
  EXPECT_STREQ(
      srs->proj4_parameters().c_str(),
      "+proj=lonlat +a=6378137 +b=6378137 +towgs84=1,2,3,4,5,6,7 +no_defs");

  delete srs;
}

TEST(SrsTest, ProjcsProj4Parameters) {
  dd::Spatial_reference_system_impl *srs;
  srs = dynamic_cast<dd::Spatial_reference_system_impl *>(
      dd::create_object<dd::Spatial_reference_system>());
  srs->set_id(1000000);
  srs->set_name("Test");
  srs->set_created(0UL);
  srs->set_last_altered(0UL);

  // Projection.
  srs->set_definition(
      "PROJCS[\"WGS 84 / Pseudo-Mercator\",GEOGCS[\"WGS 84\",DATUM[\"World "
      "Geodetic System 1984\",SPHEROID[\"WGS "
      "84\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0."
      "017453292519943278],AXIS[\"Lat\",NORTH],AXIS[\"Lon\",EAST]],PROJECTION["
      "\"Popular Visualisation Pseudo "
      "Mercator\",AUTHORITY[\"EPSG\",\"1024\"]],PARAMETER[\"Latitude of "
      "natural origin\",0,AUTHORITY[\"EPSG\",\"8801\"]],PARAMETER[\"Longitude "
      "of natural origin\",0,AUTHORITY[\"EPSG\",\"8802\"]],PARAMETER[\"False "
      "easting\",0,AUTHORITY[\"EPSG\",\"8806\"]],PARAMETER[\"False "
      "northing\",0,AUTHORITY[\"EPSG\",\"8807\"]],UNIT[\"metre\",1],AXIS[\"X\","
      "EAST],AXIS[\"Y\",NORTH]]");
  srs->parse_definition();
  EXPECT_STREQ(srs->proj4_parameters().c_str(), "");

  delete srs;
}

}  // namespace gis_srs_unittest

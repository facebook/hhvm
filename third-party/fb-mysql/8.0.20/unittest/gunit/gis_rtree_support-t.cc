
/*
   Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.

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
#include "sql/gis/mbr_utils.h"

namespace mbr_utils_unittest {
gis::Geographic_linearring linearringFromVector(std::vector<double> data) {
  if (data.size() % 2 != 0) {
    throw std::exception(); /* purecov: dead code */
  }
  gis::Geographic_linearring lr;
  for (size_t i = 0; i + 1 < data.size(); i += 2) {
    lr.push_back(gis::Geographic_point(data[i], data[i + 1]));
  }
  return lr;
}

gis::Geographic_polygon polygonFromVector(std::vector<double> data) {
  gis::Geographic_polygon py;
  py.push_back(linearringFromVector(data));
  return py;
}

std::unique_ptr<dd::Spatial_reference_system_impl> get_srs() {
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
}

TEST(MBR_Utils_test, test) {
  auto polygon = polygonFromVector({0.26377745335935482, 0.84009247535954357,
                                    0.26387320519564444, 0.84008767903244352,
                                    0.26387320679785603, 0.84008767099694759,
                                    0.26377745335935482, 0.84009247535954357});
  gis::Geographic_box box;
  std::unique_ptr<dd::Spatial_reference_system> srs = get_srs();
  gis::box_envelope(&polygon, srs.get(), &box);
  EXPECT_GT(box.max_corner().x(), 0.26);
  EXPECT_GT(box.max_corner().y(), 0.84);
  EXPECT_LT(box.max_corner().x(), 0.27);
  EXPECT_LT(box.max_corner().y(), 0.85);
}
}  // namespace mbr_utils_unittest

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

#ifndef UNITTEST_GUNIT_GIS_SRS_H_INCLUDED
#define UNITTEST_GUNIT_GIS_SRS_H_INCLUDED

#include <memory>  // std::unique_ptr

#include "sql/dd/dd.h"  // dd:create_object
#include "sql/dd/impl/types/spatial_reference_system_impl.h"

namespace {
namespace gis_srs {

// EPSG 4326, but with long-lat axes (E-N).
std::unique_ptr<dd::Spatial_reference_system_impl> swapped_epsg4326() {
  auto srs = std::unique_ptr<dd::Spatial_reference_system_impl>{
      dynamic_cast<dd::Spatial_reference_system_impl *>(
          dd::create_object<dd::Spatial_reference_system>())};

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

}  // namespace gis_srs
}  // namespace

#endif  // include guard

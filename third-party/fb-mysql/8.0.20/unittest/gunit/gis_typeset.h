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

#ifndef UNITTEST_GUNIT_GIS_TYPESET_H_INCLUDED
#define UNITTEST_GUNIT_GIS_TYPESET_H_INCLUDED

#include "sql/gis/geometries_cs.h"  // gis::{Cartesian_*,Geographic_*}

namespace {
namespace gis_typeset {

struct Cartesian {
  using Point = gis::Cartesian_point;
  using Linestring = gis::Cartesian_linestring;
  using Linearring = gis::Cartesian_linearring;
  using Polygon = gis::Cartesian_polygon;
  using Geometrycollection = gis::Cartesian_geometrycollection;
  using Multipoint = gis::Cartesian_multipoint;
  using Multilinestring = gis::Cartesian_multilinestring;
  using Multipolygon = gis::Cartesian_multipolygon;

  static gis::Coordinate_system coordinate_system() {
    return gis::Coordinate_system::kCartesian;
  }
};

struct Geographic {
  using Point = gis::Geographic_point;
  using Linestring = gis::Geographic_linestring;
  using Linearring = gis::Geographic_linearring;
  using Polygon = gis::Geographic_polygon;
  using Geometrycollection = gis::Geographic_geometrycollection;
  using Multipoint = gis::Geographic_multipoint;
  using Multilinestring = gis::Geographic_multilinestring;
  using Multipolygon = gis::Geographic_multipolygon;

  static gis::Coordinate_system coordinate_system() {
    return gis::Coordinate_system::kGeographic;
  }
};

typedef ::testing::Types<Cartesian, Geographic> Test_both;

}  // namespace gis_typeset
}  // namespace

#endif  // include guard

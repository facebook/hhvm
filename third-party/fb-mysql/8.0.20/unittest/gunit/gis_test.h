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

#ifndef UNITTEST_GUNIT_GIS_TEST_H_INCLUDED
#define UNITTEST_GUNIT_GIS_TEST_H_INCLUDED

#include <memory>  // std::unique_ptr

#include <gtest/gtest.h>

#include "sql/dd/types/spatial_reference_system.h"
#include "sql/gis/geometries.h"  // gis::Coordinate_system

#include "unittest/gunit/gis_srs.h"

namespace {

template <typename T_typeset>
struct Gis_test : ::testing::Test {
  std::unique_ptr<dd::Spatial_reference_system_impl> m_srs;

  Gis_test() {
    switch (T_typeset::coordinate_system()) {
      case gis::Coordinate_system::kCartesian:
        // Use SRID 0, leave m_srs empty
        break;
      case gis::Coordinate_system::kGeographic:
        m_srs = gis_srs::swapped_epsg4326();
        break;
    }
  }
};

}  // namespace

#endif  // include guard

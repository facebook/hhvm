/*
  Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/dd.h"
#include "sql/dd/impl/types/spatial_reference_system_impl.h"
#include "sql/dd/properties.h"
#include "sql/dd/types/spatial_reference_system.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_cs.h"
#include "sql/gis/relops.h"

namespace gis_relops_unittest {

struct Cartesian_types {
  typedef gis::Cartesian_point Point;
  typedef gis::Cartesian_linestring Linestring;
  typedef gis::Cartesian_linearring Linearring;
  typedef gis::Cartesian_polygon Polygon;
  typedef gis::Cartesian_geometrycollection Geometrycollection;
  typedef gis::Cartesian_multipoint Multipoint;
  typedef gis::Cartesian_multilinestring Multilinestring;
  typedef gis::Cartesian_multipolygon Multipolygon;

  static gis::Coordinate_system coordinate_system() {
    return gis::Coordinate_system::kCartesian;
  }
};

struct Geographic_types {
  typedef gis::Geographic_point Point;
  typedef gis::Geographic_linestring Linestring;
  typedef gis::Geographic_linearring Linearring;
  typedef gis::Geographic_polygon Polygon;
  typedef gis::Geographic_geometrycollection Geometrycollection;
  typedef gis::Geographic_multipoint Multipoint;
  typedef gis::Geographic_multilinestring Multilinestring;
  typedef gis::Geographic_multipolygon Multipolygon;

  static gis::Coordinate_system coordinate_system() {
    return gis::Coordinate_system::kGeographic;
  }
};

template <typename Types>
class RelopsTest : public ::testing::Test {
 protected:
  dd::Spatial_reference_system_impl *m_srs;

 public:
  RelopsTest() {
    switch (Types::coordinate_system()) {
      case gis::Coordinate_system::kCartesian:
        // Use SRID 0.
        m_srs = nullptr;
        break;
      case gis::Coordinate_system::kGeographic:
        // EPSG 4326, but with long-lat axes (E-N).
        m_srs = dynamic_cast<dd::Spatial_reference_system_impl *>(
            dd::create_object<dd::Spatial_reference_system>());
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
        break;
    }
  }

  ~RelopsTest() { delete m_srs; }
};

typedef ::testing::Types<Cartesian_types, Geographic_types> Types;
TYPED_TEST_CASE(RelopsTest, Types);

// The purpose of this test is to cover all type combinations, not to check if
// the results are correct.
TYPED_TEST(RelopsTest, CodeCoverage) {
  typename TypeParam::Geometrycollection gc;
  gc.push_back(typename TypeParam::Point(0.0, 0.0));

  typename TypeParam::Linestring ls;
  ls.push_back(typename TypeParam::Point(0.0, 0.0));
  ls.push_back(typename TypeParam::Point(1.0, 1.0));
  gc.push_back(ls);

  typename TypeParam::Polygon py;
  typename TypeParam::Linearring lr;
  lr.push_back(typename TypeParam::Point(0.0, 0.0));
  lr.push_back(typename TypeParam::Point(1.0, 0.0));
  lr.push_back(typename TypeParam::Point(1.0, 1.0));
  lr.push_back(typename TypeParam::Point(0.0, 1.0));
  lr.push_back(typename TypeParam::Point(0.0, 0.0));
  py.push_back(lr);
  gc.push_back(py);

  typename TypeParam::Geometrycollection gc_empty;
  gc.push_back(gc_empty);

  typename TypeParam::Geometrycollection gc_inner;
  gc_inner.push_back(typename TypeParam::Point(0.0, 0.0));
  gc.push_back(gc_inner);

  typename TypeParam::Multipoint mpt;
  mpt.push_back(typename TypeParam::Point(0.0, 0.0));
  gc.push_back(mpt);

  typename TypeParam::Multilinestring mls;
  mls.push_back(ls);
  gc.push_back(mls);

  typename TypeParam::Multipolygon mpy;
  mpy.push_back(py);
  gc.push_back(mpy);

  for (auto g1 : gc) {
    for (auto g2 : gc) {
      bool result = false;
      bool is_null = false;
      gis::crosses(this->m_srs, g1, g2, "unittest", &result, &is_null);
      gis::disjoint(this->m_srs, g1, g2, "unittest", &result, &is_null);
      gis::equals(this->m_srs, g1, g2, "unittest", &result, &is_null);
      gis::intersects(this->m_srs, g1, g2, "unittest", &result, &is_null);
      gis::mbr_covered_by(this->m_srs, g1, g2, "unittest", &result, &is_null);
      gis::mbr_disjoint(this->m_srs, g1, g2, "unittest", &result, &is_null);
      gis::mbr_equals(this->m_srs, g1, g2, "unittest", &result, &is_null);
      gis::mbr_intersects(this->m_srs, g1, g2, "unittest", &result, &is_null);
      gis::mbr_overlaps(this->m_srs, g1, g2, "unittest", &result, &is_null);
      gis::mbr_touches(this->m_srs, g1, g2, "unittest", &result, &is_null);
      gis::mbr_within(this->m_srs, g1, g2, "unittest", &result, &is_null);
      gis::overlaps(this->m_srs, g1, g2, "unittest", &result, &is_null);
      gis::touches(this->m_srs, g1, g2, "unittest", &result, &is_null);
      gis::within(this->m_srs, g1, g2, "unittest", &result, &is_null);
    }
  }
}

}  // namespace gis_relops_unittest

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

#include "sql/gis/geometries.h"
#include "sql/gis/geometries_cs.h"
#include "sql/gis/geometry_visitor.h"

namespace geometries_unittest {

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
class GeometriesTest : public ::testing::Test {
 public:
  GeometriesTest() {}

  ~GeometriesTest() {}
};

typedef ::testing::Types<Cartesian_types, Geographic_types> Types;
TYPED_TEST_CASE(GeometriesTest, Types);

TYPED_TEST(GeometriesTest, Point) {
  typename TypeParam::Point pt;
  EXPECT_EQ(gis::Geometry_type::kPoint, pt.type());
  EXPECT_EQ(TypeParam::coordinate_system(), pt.coordinate_system());
  EXPECT_TRUE(std::isnan(pt.x()));
  EXPECT_TRUE(std::isnan(pt.y()));
  EXPECT_TRUE(pt.is_empty());

  gis::Nop_visitor visitor;
  EXPECT_FALSE(pt.accept(&visitor));

  typename TypeParam::Point pt2(0.0, 0.0);
  EXPECT_EQ(0.0, pt2.x());
  EXPECT_EQ(0.0, pt2.y());

  pt.x(-1.0);
  pt.y(-1.0);
  EXPECT_EQ(-1.0, pt.x());
  EXPECT_EQ(-1.0, pt.y());
  EXPECT_FALSE(pt.is_empty());

  pt.x(1.0);
  pt.y(1.0);
  EXPECT_EQ(1.0, pt.x());
  EXPECT_EQ(1.0, pt.y());

  pt.x(1.7976931348623157e308);
  pt.y(-1.7976931348623157e308);
  EXPECT_EQ(1.7976931348623157e308, pt.x());
  EXPECT_EQ(-1.7976931348623157e308, pt.y());
}

TYPED_TEST(GeometriesTest, Curve) {}

TYPED_TEST(GeometriesTest, Linestring) {
  typename TypeParam::Linestring ls;
  EXPECT_EQ(gis::Geometry_type::kLinestring, ls.type());
  EXPECT_EQ(TypeParam::coordinate_system(), ls.coordinate_system());
  EXPECT_EQ(0U, ls.size());
  EXPECT_TRUE(ls.empty());
  EXPECT_TRUE(ls.is_empty());

  ls.push_back(typename TypeParam::Point(0.0, 0.0));
  ls.push_back(typename TypeParam::Point(10.0, 10.0));
  ls.push_back(typename TypeParam::Point(20.0, 0.0));
  ls.push_back(typename TypeParam::Point(30.0, 10.0));
  EXPECT_EQ(4U, ls.size());
  EXPECT_FALSE(ls.empty());
  EXPECT_FALSE(ls.is_empty());

  gis::Nop_visitor visitor;
  EXPECT_FALSE(ls.accept(&visitor));

  EXPECT_EQ(0.0, ls[0].x());
  EXPECT_EQ(0.0, ls[0].y());
  EXPECT_EQ(30.0, ls[3].x());
  EXPECT_EQ(10.0, ls[3].y());
}

TYPED_TEST(GeometriesTest, Linearring) {
  typename TypeParam::Linearring lr;
  EXPECT_EQ(gis::Geometry_type::kLinestring, lr.type());
  EXPECT_EQ(TypeParam::coordinate_system(), lr.coordinate_system());
  EXPECT_EQ(0U, lr.size());
  EXPECT_TRUE(lr.empty());
  EXPECT_TRUE(lr.is_empty());

  lr.push_back(typename TypeParam::Point(0.0, 0.0));
  lr.push_back(typename TypeParam::Point(10.0, 10.0));
  lr.push_back(typename TypeParam::Point(20.0, 0.0));
  lr.push_back(typename TypeParam::Point(0.0, 10.0));
  EXPECT_EQ(4U, lr.size());
  EXPECT_FALSE(lr.empty());
  EXPECT_FALSE(lr.is_empty());

  gis::Nop_visitor visitor;
  EXPECT_FALSE(lr.accept(&visitor));

  EXPECT_EQ(10.0, lr[1].x());
  EXPECT_EQ(10.0, lr[1].y());
  EXPECT_EQ(20.0, lr[2].x());
  EXPECT_EQ(0.0, lr[2].y());
}

TYPED_TEST(GeometriesTest, Surface) {}

TYPED_TEST(GeometriesTest, Polygon) {
  typename TypeParam::Polygon py;
  EXPECT_EQ(gis::Geometry_type::kPolygon, py.type());
  EXPECT_EQ(TypeParam::coordinate_system(), py.coordinate_system());
  EXPECT_EQ(0U, py.size());
  EXPECT_TRUE(py.empty());
  EXPECT_TRUE(py.is_empty());

  typename TypeParam::Linearring exterior;
  exterior.push_back(typename TypeParam::Point(0.0, 0.0));
  exterior.push_back(typename TypeParam::Point(10.0, 0.0));
  exterior.push_back(typename TypeParam::Point(10.0, 10.0));
  exterior.push_back(typename TypeParam::Point(0.0, 10.0));
  exterior.push_back(typename TypeParam::Point(0.0, 0.0));
  py.push_back(exterior);
  EXPECT_FALSE(py.empty());
  EXPECT_FALSE(py.is_empty());

  typename TypeParam::Linearring interior;
  interior.push_back(typename TypeParam::Point(2.0, 2.0));
  interior.push_back(typename TypeParam::Point(2.0, 8.0));
  interior.push_back(typename TypeParam::Point(8.0, 8.0));
  interior.push_back(typename TypeParam::Point(8.0, 2.0));
  interior.push_back(typename TypeParam::Point(2.0, 2.0));
  py.push_back(std::move(interior));

  EXPECT_EQ(2U, py.size());
  EXPECT_FALSE(py.empty());
  EXPECT_EQ(1U, py.interior_rings().size());
  EXPECT_FALSE(py.interior_ring(0).empty());

  gis::Nop_visitor visitor;
  EXPECT_FALSE(py.accept(&visitor));
}

TYPED_TEST(GeometriesTest, Geometrycollection) {
  typename TypeParam::Geometrycollection gc;
  EXPECT_EQ(gis::Geometry_type::kGeometrycollection, gc.type());
  EXPECT_EQ(TypeParam::coordinate_system(), gc.coordinate_system());
  EXPECT_TRUE(gc.empty());
  EXPECT_TRUE(gc.is_empty());

  gc.push_back(typename TypeParam::Geometrycollection());
  EXPECT_FALSE(gc.empty());
  EXPECT_TRUE(gc.is_empty());

  typename TypeParam::Geometrycollection gc2;
  gc2.push_back(typename TypeParam::Geometrycollection());
  gc.push_back(std::move(gc2));
  EXPECT_TRUE(gc.is_empty());

  gc.push_back(typename TypeParam::Point(0.0, 0.0));
  gc.push_back(typename TypeParam::Point(10.0, 0.0));
  gc.push_back(typename TypeParam::Point(10.0, 10.0));
  gc.push_back(typename TypeParam::Point(0.0, 10.0));
  gc.push_back(typename TypeParam::Point(0.0, 0.0));

  typename TypeParam::Linestring ls;
  ls.push_back(typename TypeParam::Point(0.0, 0.0));
  ls.push_back(typename TypeParam::Point(10.0, 0.0));
  ls.push_back(typename TypeParam::Point(10.0, 10.0));
  ls.push_back(typename TypeParam::Point(0.0, 10.0));
  ls.push_back(typename TypeParam::Point(0.0, 0.0));
  gc.push_back(std::move(ls));

  typename TypeParam::Linearring exterior;
  exterior.push_back(typename TypeParam::Point(0.0, 0.0));
  exterior.push_back(typename TypeParam::Point(10.0, 0.0));
  exterior.push_back(typename TypeParam::Point(10.0, 10.0));
  exterior.push_back(typename TypeParam::Point(0.0, 10.0));
  exterior.push_back(typename TypeParam::Point(0.0, 0.0));
  typename TypeParam::Polygon py;
  py.push_back(std::move(exterior));
  gc.push_back(std::move(py));

  typename TypeParam::Multipoint mpt;
  mpt.push_back(typename TypeParam::Point(0.0, 0.0));
  gc.push_back(std::move(mpt));

  typename TypeParam::Linestring ls2;
  ls2.push_back(typename TypeParam::Point(0.0, 0.0));
  ls2.push_back(typename TypeParam::Point(1.0, 1.0));
  typename TypeParam::Multilinestring mls;
  mls.push_back(std::move(ls2));
  gc.push_back(std::move(mls));

  typename TypeParam::Multipolygon mpy;
  gc.push_back(std::move(mpy));

  typename TypeParam::Geometrycollection inner_gc;
  gc.push_back(std::move(inner_gc));

  EXPECT_EQ(13U, gc.size());
  EXPECT_FALSE(gc.empty());
  EXPECT_FALSE(gc.is_empty());

  gis::Nop_visitor visitor;
  EXPECT_FALSE(gc.accept(&visitor));

  typename TypeParam::Geometrycollection gc_copy = gc;
  EXPECT_EQ(13U, gc_copy.size());
  EXPECT_FALSE(gc.empty());
  EXPECT_FALSE(gc.is_empty());
}

TYPED_TEST(GeometriesTest, Multipoint) {
  typename TypeParam::Multipoint mpt;
  EXPECT_EQ(gis::Geometry_type::kMultipoint, mpt.type());
  EXPECT_EQ(TypeParam::coordinate_system(), mpt.coordinate_system());
  EXPECT_TRUE(mpt.empty());
  EXPECT_TRUE(mpt.is_empty());

  mpt.push_back(typename TypeParam::Point(0.0, 0.0));
  EXPECT_EQ(1U, mpt.size());
  EXPECT_FALSE(mpt.empty());
  EXPECT_FALSE(mpt.is_empty());

  mpt.push_back(typename TypeParam::Point(1.0, 1.0));
  EXPECT_EQ(2U, mpt.size());

  gis::Nop_visitor visitor;
  EXPECT_FALSE(mpt.accept(&visitor));
}

TYPED_TEST(GeometriesTest, Multicurve) {
  std::unique_ptr<gis::Multicurve> mc(new
                                      typename TypeParam::Multilinestring());
  EXPECT_EQ(0U, mc->size());
  EXPECT_TRUE(mc->empty());
  EXPECT_TRUE(mc->is_empty());

  gis::Nop_visitor visitor;
  EXPECT_FALSE(mc->accept(&visitor));
}

TYPED_TEST(GeometriesTest, Multilinestring) {
  typename TypeParam::Multilinestring mls;
  EXPECT_EQ(gis::Geometry_type::kMultilinestring, mls.type());
  EXPECT_EQ(TypeParam::coordinate_system(), mls.coordinate_system());
  EXPECT_TRUE(mls.empty());
  EXPECT_TRUE(mls.is_empty());

  typename TypeParam::Linestring ls;
  ls.push_back(typename TypeParam::Point(0.0, 0.0));
  ls.push_back(typename TypeParam::Point(10.0, 0.0));
  ls.push_back(typename TypeParam::Point(10.0, 10.0));
  ls.push_back(typename TypeParam::Point(0.0, 10.0));
  ls.push_back(typename TypeParam::Point(0.0, 0.0));
  mls.push_back(std::move(ls));
  EXPECT_EQ(1U, mls.size());
  EXPECT_FALSE(mls.empty());
  EXPECT_FALSE(mls.is_empty());

  typename TypeParam::Linestring ls2;
  ls.push_back(typename TypeParam::Point(0.0, 0.0));
  ls.push_back(typename TypeParam::Point(20.0, 20.0));
  mls.push_back(std::move(ls));
  EXPECT_EQ(2U, mls.size());

  gis::Nop_visitor visitor;
  EXPECT_FALSE(mls.accept(&visitor));
}

TYPED_TEST(GeometriesTest, Multisurface) {
  std::unique_ptr<gis::Multisurface> ms(new typename TypeParam::Multipolygon());
  EXPECT_EQ(0U, ms->size());
  EXPECT_TRUE(ms->empty());
  EXPECT_TRUE(ms->is_empty());

  gis::Nop_visitor visitor;
  EXPECT_FALSE(ms->accept(&visitor));
}

TYPED_TEST(GeometriesTest, Multipolygon) {
  typename TypeParam::Multipolygon mpy;
  EXPECT_EQ(gis::Geometry_type::kMultipolygon, mpy.type());
  EXPECT_EQ(TypeParam::coordinate_system(), mpy.coordinate_system());
  EXPECT_TRUE(mpy.empty());
  EXPECT_TRUE(mpy.is_empty());

  typename TypeParam::Linearring exterior;
  exterior.push_back(typename TypeParam::Point(0.0, 0.0));
  exterior.push_back(typename TypeParam::Point(10.0, 0.0));
  exterior.push_back(typename TypeParam::Point(10.0, 10.0));
  exterior.push_back(typename TypeParam::Point(0.0, 10.0));
  exterior.push_back(typename TypeParam::Point(0.0, 0.0));

  typename TypeParam::Linearring interior;
  interior.push_back(typename TypeParam::Point(2.0, 2.0));
  interior.push_back(typename TypeParam::Point(2.0, 8.0));
  interior.push_back(typename TypeParam::Point(8.0, 8.0));
  interior.push_back(typename TypeParam::Point(8.0, 2.0));
  interior.push_back(typename TypeParam::Point(2.0, 2.0));

  typename TypeParam::Polygon py;
  py.push_back(std::move(exterior));
  py.push_back(std::move(interior));
  mpy.push_back(std::move(py));
  EXPECT_EQ(1U, mpy.size());
  EXPECT_FALSE(mpy.empty());
  EXPECT_FALSE(mpy.is_empty());

  mpy.push_back(typename TypeParam::Polygon());
  EXPECT_EQ(2U, mpy.size());

  gis::Nop_visitor visitor;
  EXPECT_FALSE(mpy.accept(&visitor));
}

}  // namespace geometries_unittest

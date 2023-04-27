/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <gtest/gtest.h>
#include <stddef.h>
#include <cmath>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/gis/srid.h"
#include "sql/gstream.h"
#include "sql/spatial.h"
#include "unittest/gunit/test_utils.h"  // Server_initializer

namespace gis_algo_unittest {

/*
  Testing Gis_polygon_ring::set_ring_order function.
 */
class SetRingOrderTest : public ::testing::Test {
 public:
  SetRingOrderTest() : my_flags(Geometry::wkb_linestring, 0) {
    latincc = &my_charset_latin1;
  }

  void SetUp() { m_init.SetUp(); }

  void TearDown() { m_init.TearDown(); }

  Geometry *geometry_from_text(const String &wkt, String *wkb,
                               Geometry_buffer *geobuf);
  void set_order_and_compare(const std::string &str, const std::string &str2,
                             bool want_ccw = true);

  const static gis::srid_t srid = 0;
  CHARSET_INFO *latincc;
  String str, str2, wkt, wkt2;
  Geometry_buffer buffer, buffer2;
  Geometry::Flags_t my_flags;
  my_testing::Server_initializer m_init;
};

Geometry *SetRingOrderTest::geometry_from_text(const String &wkt, String *wkb,
                                               Geometry_buffer *geobuf) {
  Gis_read_stream trs(m_init.thd(), wkt.charset(), wkt.ptr(), wkt.length());

  wkb->set_charset(&my_charset_bin);
  wkb->length(0);
  return Geometry::create_from_wkt(geobuf, &trs, wkb, true);
}

void SetRingOrderTest::set_order_and_compare(const std::string &s1,
                                             const std::string &s2,
                                             bool want_ccw) {
  wkt.set(s1.c_str(), s1.length(), latincc);
  wkt2.set(s2.c_str(), s2.length(), latincc);

  Gis_line_string *ringp =
      static_cast<Gis_line_string *>(geometry_from_text(wkt, &str, &buffer));
  DBUG_ASSERT(ringp->get_geotype() == Geometry::wkb_linestring);
  Gis_polygon_ring ring(ringp->get_ptr(), ringp->get_nbytes(), my_flags, 0U);
  EXPECT_EQ(ring.set_ring_order(want_ccw), false);

  ringp =
      static_cast<Gis_line_string *>(geometry_from_text(wkt2, &str2, &buffer2));
  DBUG_ASSERT(ringp->get_geotype() == Geometry::wkb_linestring);
  Gis_polygon_ring ring2(ringp->get_ptr(), ringp->get_nbytes(), my_flags, 0U);
  EXPECT_EQ(ring2.set_ring_order(want_ccw), false);

  EXPECT_EQ(str.length(), str2.length());
  EXPECT_EQ(memcmp(str.ptr(), str2.ptr(), str.length()), 0);
}

TEST_F(SetRingOrderTest, SetRingOrderCCW) {
  SCOPED_TRACE("SetRingOrderCCW");
  std::string geom1("linestring(0 0, 0 1, 1 1, 1 0, 0 0)");
  std::string geom2("linestring(0 0, 1 0, 1 1, 0 1, 0 0)");
  set_order_and_compare(geom1, geom2);
}

TEST_F(SetRingOrderTest, SetRingOrderCW) {
  SCOPED_TRACE("SetRingOrderCW");
  std::string geom1("linestring(0 0, 0 1, 1 1, 1 0, 0 0)");
  std::string geom2("linestring(0 0, 1 0, 1 1, 0 1, 0 0)");
  set_order_and_compare(geom1, geom2, false);
}

TEST_F(SetRingOrderTest, SetRingOrder2CCW) {
  SCOPED_TRACE("SetRingOrder2CCW");
  std::string geom3("linestring(0 0, 0 1, 1 0, 0 0)");
  std::string geom4("linestring(0 0, 1 0, 0 1, 0 0)");
  set_order_and_compare(geom3, geom4);
}

TEST_F(SetRingOrderTest, SetRingOrder2CW) {
  SCOPED_TRACE("SetRingOrder2CW");
  std::string geom3("linestring(0 0, 0 1, 1 0, 0 0)");
  std::string geom4("linestring(0 0, 1 0, 0 1, 0 0)");
  set_order_and_compare(geom3, geom4, false);
}

TEST_F(SetRingOrderTest, DuplicateMinPointBeforeCCW) {
  SCOPED_TRACE("DuplicateMinPointBeforeCCW");
  std::string geom1("linestring(0 0, 0 1, 1 1, 1 0, 0 0, 0 0, 0 0)");
  std::string geom2("linestring(0 0, 0 0, 0 0, 1 0, 1 1, 0 1, 0 0)");
  set_order_and_compare(geom1, geom2);
}

TEST_F(SetRingOrderTest, DuplicateMinPointBeforeCW) {
  SCOPED_TRACE("DuplicateMinPointBeforeCW");
  std::string geom1("linestring(0 0, 0 1, 1 1, 1 0, 0 0, 0 0, 0 0)");
  std::string geom2("linestring(0 0, 0 0, 0 0, 1 0, 1 1, 0 1, 0 0)");
  set_order_and_compare(geom1, geom2, false);
}

TEST_F(SetRingOrderTest, DuplicateMinPointAfterCCW) {
  SCOPED_TRACE("DuplicateMinPointAfterCCW");
  std::string geom1("linestring(0 0, 0 0, 0 0, 0 1, 1 1, 1 0, 0 0)");
  std::string geom2("linestring(0 0, 1 0, 1 1, 0 1, 0 0, 0 0, 0 0)");
  set_order_and_compare(geom1, geom2);
}

TEST_F(SetRingOrderTest, DuplicateMinPointAfterCW) {
  SCOPED_TRACE("DuplicateMinPointAfterCW");
  std::string geom1("linestring(0 0, 0 0, 0 0, 0 1, 1 1, 1 0, 0 0)");
  std::string geom2("linestring(0 0, 1 0, 1 1, 0 1, 0 0, 0 0, 0 0)");
  set_order_and_compare(geom1, geom2, false);
}

TEST_F(SetRingOrderTest, RingDegradedToPointTest) {
  SCOPED_TRACE("RingDegradedToPointTest");
  std::string s1("linestring(0 0, 0 0, 0 0, 0 0, 0 0)");
  wkt.set(s1.c_str(), s1.length(), latincc);

  Gis_line_string *ringp =
      static_cast<Gis_line_string *>(geometry_from_text(wkt, &str, &buffer));
  DBUG_ASSERT(ringp->get_geotype() == Geometry::wkb_linestring);
  Gis_polygon_ring ring(ringp->get_ptr(), ringp->get_nbytes(), my_flags, 0U);
  EXPECT_EQ(ring.set_ring_order(true /*CCW*/), true);
}

/*
  Testing functions in Geometry and its children classes that are not covered
  by current BG functionalities.
 */
class GeometryManipulationTest : public SetRingOrderTest {
 public:
  void assign_multipolygon_back(Gis_multi_polygon &mpl2,
                                const Gis_polygon &pl) {
    for (size_t i = 0; i < pl.outer().size(); i++)
      mpl2.back().outer().push_back(pl.outer()[i]);
    for (size_t i = 0; i < pl.inners().size(); i += 2) {
      mpl2.back().inners().push_back(pl.inners()[i]);
      if (i + 1 < pl.inners().size()) {
        mpl2.back().inners().resize(mpl2.back().inners().size() + 1);
        for (size_t j = 0; j < pl.inners()[i + 1].size(); j++)
          mpl2.back().inners().back().push_back(pl.inners()[i + 1][j]);
      }
    }
  }
};

TEST_F(GeometryManipulationTest, PolygonCopyTest) {
  SCOPED_TRACE("PolygonCopyTest");
  std::string s1("polygon((0 0, 1 0, 1 1, 0 1, 0 0))");
  wkt.set(s1.c_str(), s1.length(), latincc);

  Gis_polygon *plgn =
      static_cast<Gis_polygon *>(geometry_from_text(wkt, &str, &buffer));
  Gis_polygon plgn1(plgn->get_data_ptr(), plgn->get_data_size(),
                    plgn->get_flags(), plgn->get_srid());
  Gis_polygon plgn2(plgn1);
  Gis_polygon plgn3;

  plgn3 = plgn2;

  String wkb3, wkb4, wkb5;
  plgn3.as_wkb(&wkb3, false);
  plgn3.to_wkb_unparsed();
  plgn3.as_wkb(&wkb5, true);
  EXPECT_EQ(wkb3.length(), wkb5.length());
  EXPECT_EQ(memcmp(wkb3.ptr() + WKB_HEADER_SIZE, wkb5.ptr() + WKB_HEADER_SIZE,
                   wkb5.length() - WKB_HEADER_SIZE),
            0);

  plgn2.as_geometry(&wkb4, false);
  EXPECT_EQ(wkb3.length() + 4, wkb4.length());
  EXPECT_EQ(memcmp(GEOM_HEADER_SIZE + wkb4.ptr(), wkb3.ptr() + WKB_HEADER_SIZE,
                   wkb3.length() - WKB_HEADER_SIZE),
            0);

  // Check they have identical data. Can only do so in wkb form.
  plgn1.to_wkb_unparsed();
  plgn2.to_wkb_unparsed();
  EXPECT_EQ(plgn1.get_data_size(), plgn2.get_data_size());
  EXPECT_EQ(
      memcmp(plgn->get_data_ptr(), plgn2.get_data_ptr(), plgn2.get_data_size()),
      0);

  EXPECT_EQ(plgn3.get_data_size(), plgn2.get_data_size());
  EXPECT_EQ(
      memcmp(plgn3.get_data_ptr(), plgn2.get_data_ptr(), plgn2.get_data_size()),
      0);
}

TEST_F(GeometryManipulationTest, PolygonManipulationTest) {
  SCOPED_TRACE("PolygonManipulationTest");
  std::string s1("polygon((0 0, 1 0, 1 1, 0 1, 0 0))");
  std::string s2("multipolygon(((0 0, 1 0, 1 1, 0 1, 0 0)))");
  std::string s3("linestring(0.5 0.25, 0.5 0.75, 0.75 0.75, 0.5 0.25)");
  std::string s4(
      "multipolygon(((0 0, 1 0, 1 1, 0 1, 0 0)),     \
    ((0 0, 1 0, 1 1, 0 1, 0 0), (0.5 0.25, 0.5 0.75, 0.75  0.75, 0.5 0.25)),\
    ((0 0, 1 0, 1 1, 0 1, 0 0), (0.5 0.25, 0.5 0.75, 0.75  0.75, 0.5 0.25)))");
  std::string s5(
      "polygon((0 0, 1 0, 1 1, 0 1, 0 0),\
    (0.5 0.25, 0.5 0.75, 0.75  0.75, 0.5 0.25))");
  wkt.set(s1.c_str(), s1.length(), latincc);
  wkt2.set(s3.c_str(), s3.length(), latincc);

  Gis_polygon *plgn0 =
      static_cast<Gis_polygon *>(geometry_from_text(wkt, &str, &buffer));
  Gis_line_string *ls0 =
      static_cast<Gis_line_string *>(geometry_from_text(wkt2, &str2, &buffer2));
  Gis_polygon plgn(plgn0->get_data_ptr(), plgn0->get_data_size(),
                   plgn0->get_flags(), plgn0->get_srid());
  Gis_line_string ls(ls0->get_data_ptr(), ls0->get_data_size(),
                     ls0->get_flags(), ls0->get_srid());
  Gis_line_string ls00(*ls0);

  Geometry_buffer buffer3;
  String wkt3, str3;

  wkt3.set(s2.c_str(), s2.length(), latincc);
  Gis_multi_polygon *pmplgn = (static_cast<Gis_multi_polygon *>(
      geometry_from_text(wkt3, &str3, &buffer3)));
  Gis_multi_polygon mplgn0(pmplgn->get_data_ptr(), pmplgn->get_data_size(),
                           pmplgn->get_flags(), pmplgn->get_srid());
  EXPECT_EQ(mplgn0.size(), 1U);
  Gis_multi_polygon mplgn = mplgn0;

  plgn.inners().resize(1);
  for (int i = 0; i < 4; i++) (plgn.inners())[0].push_back(ls[i]);
  mplgn.push_back(plgn);
  plgn.to_wkb_unparsed();

  Geometry_buffer buffer5;
  String wkt5, str5;
  wkt5.set(s5.c_str(), s5.length(), latincc);

  Gis_polygon *plgn20 =
      static_cast<Gis_polygon *>(geometry_from_text(wkt5, &str5, &buffer5));
  Gis_polygon plgn2(plgn20->get_data_ptr(), plgn20->get_data_size(),
                    plgn20->get_flags(), plgn20->get_srid());
  EXPECT_EQ(plgn.get_data_size(), plgn2.get_nbytes());

  mplgn.push_back(plgn2);

  plgn2.to_wkb_unparsed();
  EXPECT_EQ(
      memcmp(plgn.get_data_ptr(), plgn2.get_data_ptr(), plgn2.get_data_size()),
      0);

  Geometry_buffer buffer4;
  String wkt4, str4;
  wkt4.set(s4.c_str(), s4.length(), latincc);

  Gis_multi_polygon *mplgn2 = static_cast<Gis_multi_polygon *>(
      geometry_from_text(wkt4, &str4, &buffer4));

  EXPECT_EQ(mplgn.get_data_size(), mplgn2->get_data_size());
  EXPECT_EQ(memcmp(mplgn.get_data_ptr(), mplgn2->get_data_ptr(),
                   mplgn2->get_data_size()),
            0);
}

TEST_F(GeometryManipulationTest, ResizeAssignmentTest) {
  Gis_polygon_ring ring1, ring6, ring7, ring8;
  Gis_line_string ls4, ls5, ls6, ls7, ls8;
  for (int i = 0; i < 5; i++) {
    Gis_point pt;
    pt.set<0>(i);
    pt.set<1>(i);
    ring1.push_back(pt);
    ls4.push_back(pt);
    ls6.push_back(pt);
    ls7.push_back(pt);
    ring6.push_back(pt);
    ring7.push_back(pt);
    if (i != 4) {
      ls8.push_back(pt);
      ring8.push_back(pt);
    }
  }
  Gis_point pt;
  pt.set<0>(0);
  pt.set<1>(0);
  ls7.push_back(pt);

  Gis_polygon plgn3, plgn4, plgn5;
  plgn3.outer() = ring1;
  plgn3.inners().push_back(ring1);
  plgn3.inners().resize(plgn3.inners().size());

  plgn5.outer() = ring8;
  plgn5.inners().push_back(ring7);
  plgn5.inners().push_back(ring6);

  Gis_multi_polygon mplgn3, mplgn4;
  mplgn3.push_back(plgn3);
  mplgn3.resize(2);
  assign_multipolygon_back(mplgn3, plgn5);
  mplgn3.push_back(plgn3);
  mplgn3.resize(4);
  assign_multipolygon_back(mplgn3, plgn5);

  mplgn4.resize(1);
  assign_multipolygon_back(mplgn4, plgn3);
  mplgn4.push_back(plgn5);
  mplgn4.resize(3);
  assign_multipolygon_back(mplgn4, plgn3);
  mplgn4.push_back(plgn5);
  mplgn3.reassemble();
  mplgn4.reassemble();
  EXPECT_EQ(
      mplgn3.get_ptr() != mplgn4.get_ptr() &&
          mplgn3.get_nbytes() == mplgn4.get_nbytes() &&
          memcmp(mplgn3.get_ptr(), mplgn4.get_ptr(), mplgn3.get_nbytes()) == 0,
      true);

  Gis_multi_line_string mls1, mls2;
  mls1.resize(1);
  for (size_t i = 0; i < ls4.size(); i++) mls1.back().push_back(ls4[i]);
  mls1.push_back(ls6);
  mls1.resize(3);
  for (size_t i = 0; i < ls7.size(); i++) mls1.back().push_back(ls7[i]);
  mls1.push_back(ls8);

  mls2.push_back(ls4);
  mls2.resize(2);
  for (size_t i = 0; i < ls6.size(); i++) mls2.back().push_back(ls6[i]);

  mls2.push_back(ls7);
  mls2.resize(4);
  for (size_t i = 0; i < ls8.size(); i++) mls2.back().push_back(ls8[i]);

  mls1.reassemble();
  mls2.reassemble();
  EXPECT_EQ(mls1.get_ptr() != mls2.get_ptr() &&
                mls1.get_nbytes() == mls2.get_nbytes() &&
                memcmp(mls1.get_ptr(), mls2.get_ptr(), mls1.get_nbytes()) == 0,
            true);
  String str1, str2;
  str1.append(mls1.get_cptr(), mls1.get_nbytes(), &my_charset_bin);
  Gis_geometry_collection geocol(0, Geometry::wkb_multipolygon, &str1, &str2);

  ls4 = ls5;
  EXPECT_EQ(ls4.get_ptr() == nullptr && ls4.get_nbytes() == 0, true);
  EXPECT_EQ(ls5.get_ptr() == nullptr && ls5.get_nbytes() == 0, true);
  plgn3 = plgn4;
  plgn3.to_wkb_unparsed();
  EXPECT_EQ(plgn3.get_ptr() == nullptr && plgn3.get_nbytes() == 0, true);

  ls4 = ls6;
  EXPECT_EQ(ls4.get_ptr() != ls6.get_ptr() &&
                ls4.get_nbytes() == ls6.get_nbytes() &&
                memcmp(ls4.get_ptr(), ls6.get_ptr(), ls6.get_nbytes()) == 0,
            true);

  ls4 = ls7;
  EXPECT_EQ(ls4.get_ptr() != ls7.get_ptr() &&
                ls4.get_nbytes() == ls7.get_nbytes() &&
                memcmp(ls4.get_ptr(), ls7.get_ptr(), ls7.get_nbytes()) == 0,
            true);

  ls4 = ls6;
  EXPECT_EQ(ls4.get_ptr() != ls6.get_ptr() &&
                ls4.get_nbytes() == ls6.get_nbytes() &&
                memcmp(ls4.get_ptr(), ls6.get_ptr(), ls6.get_nbytes()) == 0,
            true);

  void *buf = gis_wkb_alloc(ls8.get_nbytes() + 32);
  memcpy(buf, ls8.get_ptr(), ls8.get_nbytes());
  char *cbuf = static_cast<char *>(buf);
  memset(cbuf + ls8.get_nbytes(), 0xff, 32);
  cbuf[ls8.get_nbytes() + 31] = '\0';

  ls4.set_ptr(buf, ls8.get_nbytes());
  EXPECT_EQ(ls4.get_ptr() != ls8.get_ptr() &&
                ls4.get_nbytes() == ls8.get_nbytes() &&
                memcmp(ls4.get_ptr(), ls8.get_ptr(), ls8.get_nbytes()) == 0,
            true);

  for (size_t i = ls4.size() + 1; i < 64; i++) {
    ls4.resize(i);
    ls4.back().set<0>(i);
    ls4.back().set<1>(i);
  }
}

/*
  Tests of miscellineous GIS functionalities.
*/
class GisMiscTests : public ::testing::Test {
 public:
};

TEST_F(GisMiscTests, PointxyDistanceTest) {
  const point_xy pt1(1.0, 1.0);
  const point_xy pt2(1e300, -1e300);
  const point_xy pt3(1e300, 1);
  const point_xy pt4(1, 1e300);
  const point_xy pt5(pt2);

  EXPECT_FALSE(std::isfinite(pt1.distance(pt2)));
  EXPECT_FALSE(std::isfinite(pt1.distance(pt3)));
  EXPECT_FALSE(std::isfinite(pt1.distance(pt4)));
  EXPECT_FALSE(std::isfinite(pt2.distance(pt3)));
  EXPECT_FALSE(std::isfinite(pt2.distance(pt4)));
  EXPECT_FALSE(std::isfinite(pt3.distance(pt4)));
  EXPECT_FLOAT_EQ(0.0, pt2.distance(pt5));
}

}  // namespace gis_algo_unittest

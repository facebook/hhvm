/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "gcs_base_test.h"

namespace xcom_view_ids_unittest {

class XcomVIewIds : public GcsBaseTest {
 protected:
  XcomVIewIds() {}
  ~XcomVIewIds() {}
};

TEST_F(XcomVIewIds, CompareViewsTest) {
  Gcs_xcom_view_identifier view_id1(0, 0);
  Gcs_xcom_view_identifier view_id2(0, 0);
  ASSERT_EQ(view_id1, view_id2);

  view_id2.increment_by_one();
  ASSERT_NE(view_id1, view_id2);
  ASSERT_TRUE(view_id1 < view_id2);

  view_id1.increment_by_one();
  ASSERT_EQ(view_id1, view_id2);
  ASSERT_FALSE(view_id1 < view_id2);

  Gcs_xcom_view_identifier view_id3(1, 23);
  Gcs_xcom_view_identifier view_id4(1, 24);
  ASSERT_TRUE(view_id3 < view_id4);
  ASSERT_NE(view_id1, view_id3);
  ASSERT_NE(view_id1, view_id4);
  ASSERT_NE(view_id3, view_id4);

  view_id3.increment_by_one();
  ASSERT_FALSE(view_id3 < view_id4);
  ASSERT_NE(view_id1, view_id3);
  ASSERT_NE(view_id1, view_id4);
  ASSERT_EQ(view_id3, view_id4);

  Gcs_xcom_view_identifier view_id5(0, 30);
  ASSERT_TRUE(view_id5 < view_id4);
  ASSERT_TRUE(view_id5 < view_id3);
}

TEST_F(XcomVIewIds, SetOrderingTest) {
  Gcs_xcom_view_identifier view_id1(2, 0);
  Gcs_xcom_view_identifier view_id2(0, 0);
  Gcs_xcom_view_identifier view_id3(1, 25);
  Gcs_xcom_view_identifier view_id4(1, 24);
  Gcs_xcom_view_identifier view_id5(0, 30);
  Gcs_xcom_view_identifier view_id6(1, 25);

  std::set<Gcs_xcom_view_identifier> view_set;
  view_set.insert(view_id1);
  view_set.insert(view_id2);
  view_set.insert(view_id3);
  view_set.insert(view_id4);
  view_set.insert(view_id5);
  view_set.insert(view_id6);

  ASSERT_EQ(view_set.size(), 5);

  ASSERT_EQ(*view_set.begin(), view_id2);
  view_set.erase(view_set.begin());
  ASSERT_EQ(*view_set.begin(), view_id5);
  view_set.erase(view_set.begin());
  ASSERT_EQ(*view_set.begin(), view_id4);
  view_set.erase(view_set.begin());
  ASSERT_EQ(*view_set.begin(), view_id3);
  view_set.erase(view_set.begin());
  ASSERT_EQ(*view_set.begin(), view_id1);
  view_set.erase(view_set.begin());
  ASSERT_TRUE(view_set.empty());
}

}  // namespace xcom_view_ids_unittest

/* Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.

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
#include "unittest/gunit/libmysqlgcs/include/gcs_base_test.h"

#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/gcs_xcom_expels_in_progress.h"

namespace gcs_xcom_expels_in_progress_unittest {

class GcsXcomExpelsInProgressTest : public GcsBaseTest {
 protected:
  GcsXcomExpelsInProgressTest() = default;

  ~GcsXcomExpelsInProgressTest() override = default;

  Gcs_xcom_expels_in_progress m_expels_in_progress;
};

/* This test validates the memory of expels in progress that the GCS suspicion
   manager keeps track of.

   It checks that we keep track of expels and the configuration ID that lead to
   them.
   It checks that we count members that were expelled but not yet removed as
   suspects for the purposes of majority calculation. It checks that we forget
   about expels once they take effect in a
   configuration after the one in which the expels were issued. */
TEST_F(GcsXcomExpelsInProgressTest, ExpelTracking) {
  std::string m1_endpoint{"127.0.0.1:12345"};
  auto m1_gcs = std::make_unique<Gcs_member_identifier>(m1_endpoint);
  Gcs_xcom_node_information m1_xcom{m1_endpoint};
  std::string m2_endpoint{"127.0.0.1:12346"};
  auto m2_gcs = std::make_unique<Gcs_member_identifier>(m2_endpoint);
  Gcs_xcom_node_information m2_xcom{m2_endpoint};
  std::string m3_endpoint{"127.0.0.1:12347"};
  auto m3_gcs = std::make_unique<Gcs_member_identifier>(m3_endpoint);
  Gcs_xcom_node_information m3_xcom{m3_endpoint};

  Gcs_xcom_nodes expels_issued;
  synode_no synode_0{0, 0, 0};
  synode_no synode_1{0, 1, 0};
  synode_no synode_2{0, 2, 0};
  synode_no synode_3{0, 3, 0};
  std::vector<Gcs_member_identifier *> suspected_members;
  std::vector<Gcs_member_identifier *> suspected_nonmembers;
  std::vector<Gcs_member_identifier *> members_that_left;

  /* Expel m1 in synode_1.

     expels={m1} */
  expels_issued.add_node(m1_xcom);
  m_expels_in_progress.remember_expels_issued(synode_1, expels_issued);
  ASSERT_EQ(m_expels_in_progress.size(), 1);
  ASSERT_TRUE(m_expels_in_progress.contains(*m1_gcs.get(), synode_1));
  expels_issued.clear_nodes();

  /* Expel m2 in synode_2.

     expels={m1,m2} */
  expels_issued.add_node(m2_xcom);
  m_expels_in_progress.remember_expels_issued(synode_2, expels_issued);
  ASSERT_EQ(m_expels_in_progress.size(), 2);
  ASSERT_TRUE(m_expels_in_progress.contains(*m2_gcs.get(), synode_2));
  expels_issued.clear_nodes();

  /* Count m2 as "alive and expelled but not yet removed."

     expels={m1,m2}
     suspects={m1} */
  suspected_members.push_back(m1_gcs.get());
  ASSERT_EQ(m_expels_in_progress.number_of_expels_not_about_suspects(
                suspected_members, suspected_nonmembers),
            1);
  suspected_members.clear();

  /* Count m1 as "alive and expelled but not yet removed."

     expels={m1,m2}
     suspects={m2} */
  suspected_nonmembers.push_back(m2_gcs.get());
  ASSERT_EQ(m_expels_in_progress.number_of_expels_not_about_suspects(
                suspected_members, suspected_nonmembers),
            1);
  suspected_nonmembers.clear();

  /* Count no one as "alive and expelled but not yet removed."

     expels={m1,m2}
     suspects={m1,m2} */
  suspected_members.push_back(m2_gcs.get());
  suspected_nonmembers.push_back(m1_gcs.get());
  ASSERT_EQ(m_expels_in_progress.number_of_expels_not_about_suspects(
                suspected_members, suspected_nonmembers),
            0);
  suspected_members.clear();
  suspected_nonmembers.clear();

  /* Count m1 and m2 as "alive and expelled but not yet removed."

     expels={m1,m2}
     suspects={} */
  ASSERT_EQ(m_expels_in_progress.number_of_expels_not_about_suspects(
                suspected_members, suspected_nonmembers),
            2);

  /* Count m1 and m2 as "alive and expelled but not yet removed."

     expels={m1,m2}
     suspects={m3} */
  suspected_members.push_back(m3_gcs.get());
  ASSERT_EQ(m_expels_in_progress.number_of_expels_not_about_suspects(
                suspected_members, suspected_nonmembers),
            2);
  suspected_members.clear();

  suspected_nonmembers.push_back(m3_gcs.get());
  ASSERT_EQ(m_expels_in_progress.number_of_expels_not_about_suspects(
                suspected_members, suspected_nonmembers),
            2);
  suspected_nonmembers.clear();

  /* Do not forget that we expelled m1 on synode_1 due to view from the past
     (synode_0).

     expels={(m1,1)}
     left={m1} synode=0 */
  members_that_left.push_back(m1_gcs.get());
  m_expels_in_progress.forget_expels_that_have_taken_effect(synode_0,
                                                            members_that_left);
  ASSERT_EQ(m_expels_in_progress.size(), 2);
  members_that_left.clear();

  /* Do not forget that we expelled m2 on synode_2 due to view from the past
     (synode_1).

     expels={(m2,2)}
     left={m2} synode=1 */
  members_that_left.push_back(m2_gcs.get());
  m_expels_in_progress.forget_expels_that_have_taken_effect(synode_1,
                                                            members_that_left);
  ASSERT_EQ(m_expels_in_progress.size(), 2);
  members_that_left.clear();

  /* Do not forget that we expelled m1 and m2.

     expels={(m1,1); (m2,2)}
     left={} synode=3 */
  m_expels_in_progress.forget_expels_that_have_taken_effect(synode_3,
                                                            members_that_left);
  ASSERT_EQ(m_expels_in_progress.size(), 2);

  /* Do not forget that we expelled m1 and m2.

     expels={(m1,1); (m2,2)}
     left={m3} synode=3 */
  members_that_left.push_back(m3_gcs.get());
  m_expels_in_progress.forget_expels_that_have_taken_effect(synode_3,
                                                            members_that_left);
  ASSERT_EQ(m_expels_in_progress.size(), 2);
  members_that_left.clear();

  /* Forget that we expelled m1 and m2.

     expels={(m1,1); (m2,2)}
     left={m1,m2} synode=3 */
  members_that_left.push_back(m1_gcs.get());
  members_that_left.push_back(m2_gcs.get());
  members_that_left.push_back(m3_gcs.get());
  m_expels_in_progress.forget_expels_that_have_taken_effect(synode_3,
                                                            members_that_left);
  ASSERT_EQ(m_expels_in_progress.size(), 0);
  members_that_left.clear();
}
}  // namespace gcs_xcom_expels_in_progress_unittest

/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <string>
#include <vector>

#include "gcs_base_test.h"

#include "gcs_message_stage_lz4.h"
#include "gcs_message_stage_split.h"

using std::vector;

namespace gcs_parameters_unittest {

class GcsParametersTest : public GcsBaseTest {
 protected:
  GcsParametersTest() : m_gcs(nullptr) {}

  virtual void SetUp() {
    m_gcs = Gcs_xcom_interface::get_interface();

    // convenience alias to specialized version of Gcs_interface.
    m_xcs = static_cast<Gcs_xcom_interface *>(m_gcs);

    // These are all parameters and are all valid
    m_params.add_parameter("group_name", "ola");
    m_params.add_parameter("local_node", "127.0.0.1:24844");
    m_params.add_parameter("peer_nodes", "127.0.0.1:24844,127.0.0.1:24845");
    m_params.add_parameter("bootstrap_group", "true");
    m_params.add_parameter("poll_spin_loops", "100");
    m_params.add_parameter("compression", "on");
    m_params.add_parameter("compression_threshold", "1024");
    m_params.add_parameter("ip_whitelist", "127.0.0.1,192.168.1.0/24");
    m_params.add_parameter("non_member_expel_timeout", "5");
    m_params.add_parameter("suspicions_processing_period", "25");
    m_params.add_parameter("member_expel_timeout", "120");
    m_params.add_parameter("join_attempts", "3");
    m_params.add_parameter("join_sleep_time", "5");
    m_params.add_parameter("fragmentation", "on");
    m_params.add_parameter("fragmentation_threshold", "1024");
  }

  virtual void TearDown() {
    // fake factory cleanup member function
    static_cast<Gcs_xcom_interface *>(m_gcs)->cleanup();
  }

  Gcs_interface *m_gcs;
  Gcs_xcom_interface *m_xcs;
  Gcs_interface_parameters m_params;

  void do_check_params() {
    enum_gcs_error err = m_gcs->initialize(m_params);
    ASSERT_EQ(err, GCS_NOK);
    err = m_gcs->finalize();
    // initialization failed, and thus so will finalization
    ASSERT_EQ(err, GCS_NOK);
  }

  void do_check_ok_params() {
    enum_gcs_error err = m_gcs->initialize(m_params);
    ASSERT_EQ(err, GCS_OK);
    err = m_gcs->finalize();
    // initialization succeeded, and thus so will finalization
    ASSERT_EQ(err, GCS_OK);
  }
};

/*
 This file contains a bunch of test for BUG#22901408.

 Checks default values for compression, does sanity checks, etc.
 */
TEST_F(GcsParametersTest, ParametersCompression) {
  enum_gcs_error err;

  // --------------------------------------------------------
  // Compression default values
  // --------------------------------------------------------
  Gcs_interface_parameters implicit_values;
  implicit_values.add_parameter("group_name", "ola");
  implicit_values.add_parameter("peer_nodes",
                                "127.0.0.1:24844,127.0.0.1:24845");
  implicit_values.add_parameter("local_node", "127.0.0.1:24844");
  implicit_values.add_parameter("bootstrap_group", "true");
  implicit_values.add_parameter("poll_spin_loops", "100");

  err = m_gcs->initialize(implicit_values);

  ASSERT_EQ(err, GCS_OK);

  const Gcs_interface_parameters &init_params =
      m_xcs->get_initialization_parameters();

  // compression is ON by default
  ASSERT_TRUE(init_params.get_parameter("compression")->compare("on") == 0);

  // compression_threshold is set to the default
  std::stringstream ss;
  ss << Gcs_message_stage_lz4::DEFAULT_THRESHOLD;
  ASSERT_TRUE(
      init_params.get_parameter("compression_threshold")->compare(ss.str()) ==
      0);

  // finalize the interface
  err = m_gcs->finalize();

  ASSERT_EQ(err, GCS_OK);

  // --------------------------------------------------------
  // Compression explicit values
  // --------------------------------------------------------
  std::string compression = "off";
  std::string compression_threshold = "1";

  Gcs_interface_parameters explicit_values;
  explicit_values.add_parameter("group_name", "ola");
  explicit_values.add_parameter("peer_nodes",
                                "127.0.0.1:24844,127.0.0.1:24845");
  explicit_values.add_parameter("local_node", "127.0.0.1:24844");
  explicit_values.add_parameter("bootstrap_group", "true");
  explicit_values.add_parameter("poll_spin_loops", "100");
  explicit_values.add_parameter("compression", compression);
  explicit_values.add_parameter("compression_threshold", compression_threshold);

  err = m_gcs->initialize(explicit_values);

  const Gcs_interface_parameters &init_params2 =
      m_xcs->get_initialization_parameters();

  ASSERT_EQ(err, GCS_OK);

  // compression is ON by default
  ASSERT_TRUE(init_params2.get_parameter("compression")->compare(compression) ==
              0);

  // compression is set to the value we explicitly configured
  ASSERT_TRUE(init_params2.get_parameter("compression_threshold")
                  ->compare(compression_threshold) == 0);

  err = m_gcs->finalize();

  ASSERT_EQ(err, GCS_OK);
}

/*
 Checks default values for fragmentation, does sanity checks, etc.
 */
TEST_F(GcsParametersTest, ParametersFragmentation) {
  enum_gcs_error err;

  // --------------------------------------------------------
  // Fragmentation default values
  // --------------------------------------------------------
  Gcs_interface_parameters implicit_values;
  implicit_values.add_parameter("group_name", "ola");
  implicit_values.add_parameter("peer_nodes",
                                "127.0.0.1:24844,127.0.0.1:24845");
  implicit_values.add_parameter("local_node", "127.0.0.1:24844");
  implicit_values.add_parameter("bootstrap_group", "true");
  implicit_values.add_parameter("poll_spin_loops", "100");

  err = m_gcs->initialize(implicit_values);

  ASSERT_EQ(err, GCS_OK);

  const Gcs_interface_parameters &init_params =
      m_xcs->get_initialization_parameters();

  // fragmentation is ON by default
  ASSERT_TRUE(init_params.get_parameter("fragmentation")->compare("on") == 0);

  // fragmentation_threshold is set to the default
  std::stringstream ss;
  ss << Gcs_message_stage_split_v2::DEFAULT_THRESHOLD;
  ASSERT_TRUE(
      init_params.get_parameter("fragmentation_threshold")->compare(ss.str()) ==
      0);

  // finalize the interface
  err = m_gcs->finalize();

  ASSERT_EQ(err, GCS_OK);

  // --------------------------------------------------------
  // Fragmentation explicit values
  // --------------------------------------------------------
  std::string fragmentation = "off";
  std::string fragmentation_threshold = "1";

  Gcs_interface_parameters explicit_values;
  explicit_values.add_parameter("group_name", "ola");
  explicit_values.add_parameter("peer_nodes",
                                "127.0.0.1:24844,127.0.0.1:24845");
  explicit_values.add_parameter("local_node", "127.0.0.1:24844");
  explicit_values.add_parameter("bootstrap_group", "true");
  explicit_values.add_parameter("poll_spin_loops", "100");
  explicit_values.add_parameter("fragmentation", fragmentation);
  explicit_values.add_parameter("fragmentation_threshold",
                                fragmentation_threshold);

  err = m_gcs->initialize(explicit_values);

  const Gcs_interface_parameters &init_params2 =
      m_xcs->get_initialization_parameters();

  ASSERT_EQ(err, GCS_OK);

  // fragmentation is ON by default
  ASSERT_TRUE(
      init_params2.get_parameter("fragmentation")->compare(fragmentation) == 0);

  // fragmentation is set to the value we explicitly configured
  ASSERT_TRUE(init_params2.get_parameter("fragmentation_threshold")
                  ->compare(fragmentation_threshold) == 0);

  err = m_gcs->finalize();

  ASSERT_EQ(err, GCS_OK);
}

TEST_F(GcsParametersTest, SanityParameters) {
  // initialize the interface
  enum_gcs_error err = m_gcs->initialize(m_params);

  ASSERT_EQ(err, GCS_OK);

  // finalize the interface
  err = m_gcs->finalize();

  ASSERT_EQ(err, GCS_OK);
}

TEST_F(GcsParametersTest, AbsentGroupName) {
  Gcs_interface_parameters params;
  params.add_parameter("peer_nodes", "127.0.0.1:24844,127.0.0.1:24845");
  params.add_parameter("local_node", "127.0.0.1:24844");
  params.add_parameter("bootstrap_group", "true");
  params.add_parameter("poll_spin_loops", "100");

  enum_gcs_error err = m_gcs->initialize(params);
  ASSERT_EQ(err, GCS_NOK);
  err = m_gcs->finalize();
  // initialization failed, and thus so will finalization
  ASSERT_EQ(err, GCS_NOK);
}

TEST_F(GcsParametersTest, AbsentPeerNodes) {
  Gcs_interface_parameters params;
  params.add_parameter("group_name", "ola");
  params.add_parameter("local_node", "127.0.0.1:24844");
  params.add_parameter("bootstrap_group", "true");
  params.add_parameter("poll_spin_loops", "100");

  enum_gcs_error err = m_gcs->initialize(params);
  ASSERT_EQ(err, GCS_NOK);
  err = m_gcs->finalize();
  // initialization failed, and thus so will finalization
  ASSERT_EQ(err, GCS_NOK);
}

TEST_F(GcsParametersTest, AbsentLocalNode) {
  Gcs_interface_parameters params;
  params.add_parameter("group_name", "ola");
  params.add_parameter("peer_nodes", "127.0.0.1:24844,127.0.0.1:24845");
  params.add_parameter("bootstrap_group", "true");
  params.add_parameter("poll_spin_loops", "100");

  enum_gcs_error err = m_gcs->initialize(params);
  ASSERT_EQ(err, GCS_NOK);
  err = m_gcs->finalize();
  // initialization failed, and thus so will finalization
  ASSERT_EQ(err, GCS_NOK);
}

TEST_F(GcsParametersTest, InvalidPeerNodes) {
  std::string *p =
      const_cast<std::string *>(m_params.get_parameter("peer_nodes"));
  std::string save = *p;

  // invalid peer
  *p = "127.0.0.1 24844,127.0.0.1 24845";

  enum_gcs_error err = m_gcs->initialize(m_params);
  ASSERT_EQ(err, GCS_NOK);
  err = m_gcs->finalize();
  // initialization failed, and thus so will finalization
  ASSERT_EQ(err, GCS_NOK);

  *p = save;
}

TEST_F(GcsParametersTest, InvalidLocalNode) {
  std::string *p =
      const_cast<std::string *>(m_params.get_parameter("local_node"));
  std::string save = *p;

  // invalid peer
  *p = "127.0.0.1 24844";
  do_check_params();
  *p = save;
}

TEST_F(GcsParametersTest, InvalidPollSpinLoops) {
  std::string *p =
      const_cast<std::string *>(m_params.get_parameter("poll_spin_loops"));
  std::string save = *p;

  *p = "Invalid";
  do_check_params();
  *p = save;
}

TEST_F(GcsParametersTest, InvalidCompressionThreshold) {
  std::string *p = const_cast<std::string *>(
      m_params.get_parameter("compression_threshold"));
  std::string save = *p;

  *p = "Invalid";
  do_check_params();
  *p = save;
}

TEST_F(GcsParametersTest, InvalidFragmentationThreshold) {
  std::string *p = const_cast<std::string *>(
      m_params.get_parameter("fragmentation_threshold"));
  std::string save = *p;

  *p = "Invalid";
  do_check_params();
  *p = save;
}

TEST_F(GcsParametersTest, InvalidLocalNodeAddress) {
  std::string *p =
      const_cast<std::string *>(m_params.get_parameter("local_node"));
  std::string save = *p;

  *p = "127.0";
  do_check_params();
  *p = save;
}

TEST_F(GcsParametersTest, InvalidWhitelistIPMask) {
  std::string *p =
      const_cast<std::string *>(m_params.get_parameter("ip_whitelist"));
  std::string save = *p;

  *p = "192.168.1.1/33";
  do_check_params();
  *p = save;
}

TEST_F(GcsParametersTest, InvalidWhitelistIP) {
  std::string *p =
      const_cast<std::string *>(m_params.get_parameter("ip_whitelist"));
  std::string save = *p;

  *p = "192.168.1.256/24";
  do_check_params();
  *p = save;
}

TEST_F(GcsParametersTest, InvalidWhitelistIPs) {
  std::string *p =
      const_cast<std::string *>(m_params.get_parameter("ip_whitelist"));
  std::string save = *p;

  *p = "192.168.1.222/24,255.257.256.255";
  do_check_params();
  *p = save;
}

TEST_F(GcsParametersTest, HalfBakedIP) {
  std::string *p =
      const_cast<std::string *>(m_params.get_parameter("ip_whitelist"));
  std::string save = *p;

  *p = "192.168.";
  do_check_params();
  *p = save;
}

TEST_F(GcsParametersTest, InvalidLocalNode_IP_not_found) {
  std::string *p =
      const_cast<std::string *>(m_params.get_parameter("local_node"));
  std::string save = *p;

  *p = "8.8.8.8:24844";
  do_check_params();

  *p = "128.0.3.4:12345";
  do_check_params();

  *p = "localhost:12345";
  do_check_ok_params();

  *p = save;
}

TEST_F(GcsParametersTest, InvalidNonMemberExpelTimeout) {
  std::string *p = const_cast<std::string *>(
      m_params.get_parameter("non_member_expel_timeout"));
  std::string save = *p;

  *p = "Invalid";
  do_check_params();

  *p = "-1";
  do_check_params();

  *p = "3.5";
  do_check_params();

  *p = save;
}

TEST_F(GcsParametersTest, InvalidSuspicionsProcessingPeriod) {
  std::string *p = const_cast<std::string *>(
      m_params.get_parameter("suspicions_processing_period"));
  std::string save = *p;

  *p = "Invalid";
  do_check_params();

  *p = "-1";
  do_check_params();

  *p = "3.5";
  do_check_params();

  *p = save;
}

TEST_F(GcsParametersTest, InvalidMemberExpelTimeout) {
  std::string *p =
      const_cast<std::string *>(m_params.get_parameter("member_expel_timeout"));
  std::string save = *p;

  *p = "Invalid";
  do_check_params();

  *p = "-1";
  do_check_params();

  *p = "3.5";
  do_check_params();

  *p = save;
}

TEST_F(GcsParametersTest, InvalidJoinAttempts) {
  std::string *p =
      const_cast<std::string *>(m_params.get_parameter("join_attempts"));
  std::string save = *p;

  *p = "Invalid";
  do_check_params();
  *p = save;
}

TEST_F(GcsParametersTest, InvalidJoinSleepTime) {
  std::string *p =
      const_cast<std::string *>(m_params.get_parameter("join_sleep_time"));
  std::string save = *p;

  *p = "Invalid";
  do_check_params();
  *p = save;
}

}  // namespace gcs_parameters_unittest

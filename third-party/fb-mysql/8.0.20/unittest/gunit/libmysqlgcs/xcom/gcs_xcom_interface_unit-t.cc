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

#include "synode_no.h"

using std::vector;

extern void do_cb_xcom_receive_data(synode_no message_id,
                                    Gcs_xcom_nodes *xcom_nodes,
                                    synode_no last_removed, u_int size,
                                    char *data);

namespace gcs_interface_unittest {

class GcsInterfaceTest : public GcsBaseTest {};

/**
  This test is primarily to run valgrind to make sure
  that the interface can be initialized and finalized
  multiple times without any leak.
 */
TEST_F(GcsInterfaceTest, DoubleInitFinalizeTest) {
  Gcs_interface *gcs = Gcs_xcom_interface::get_interface();

  Gcs_interface_parameters if_params;
  if_params.add_parameter("group_name", "ola");
  if_params.add_parameter("peer_nodes", "127.0.0.1:24844");
  if_params.add_parameter("local_node", "127.0.0.1:24844");
  if_params.add_parameter("bootstrap_group", "true");

  // initialize the interface
  gcs->initialize(if_params);

  // finalize the interface
  gcs->finalize();

  // initialize the interface again
  gcs->initialize(if_params);

  // finalize it again interface
  gcs->finalize();

  // fake factory cleanup member function
  static_cast<Gcs_xcom_interface *>(gcs)->cleanup();
}

TEST_F(GcsInterfaceTest, ReceiveEmptyMessageTest) {
  Gcs_interface *gcs = Gcs_xcom_interface::get_interface();

  Gcs_interface_parameters if_params;
  if_params.add_parameter("group_name", "ola");
  if_params.add_parameter("peer_nodes", "127.0.0.1:24844");
  if_params.add_parameter("local_node", "127.0.0.1:24844");
  if_params.add_parameter("bootstrap_group", "true");

  // initialize the interface
  gcs->initialize(if_params);

  // invoke the callback with a message with size zero
  do_cb_xcom_receive_data(null_synode, nullptr, null_synode, 0, nullptr);

  // finalize the interface
  gcs->finalize();

  // fake factory cleanup member function
  static_cast<Gcs_xcom_interface *>(gcs)->cleanup();
}

TEST_F(GcsInterfaceTest, InvalidCacheSize) {
  Gcs_interface *gcs = Gcs_xcom_interface::get_interface();

  Gcs_interface_parameters if_params;
  if_params.add_parameter("group_name", "ola");
  if_params.add_parameter("peer_nodes", "127.0.0.1:12345");
  if_params.add_parameter("local_node", "127.0.0.1:12345");
  if_params.add_parameter("bootstrap_group", "true");
  // Check for wrong value.
  if_params.add_parameter("xcom_cache_size", "100");

  enum_gcs_error initialized = gcs->initialize(if_params);
  ASSERT_EQ(GCS_NOK, initialized);

  Gcs_interface_parameters cfg_params;
  cfg_params.add_parameter("group_name", "ola");
  cfg_params.add_parameter("peer_nodes", "127.0.0.1:12345");
  cfg_params.add_parameter("local_node", "127.0.0.1:12345");
  cfg_params.add_parameter("bootstrap_group", "true");
  // Check for out of range value.
  cfg_params.add_parameter("xcom_cache_size", "100000000000000000000");

  initialized = gcs->initialize(cfg_params);
  ASSERT_EQ(GCS_NOK, initialized);

  Gcs_interface_parameters cfg_params_valid;
  cfg_params_valid.add_parameter("group_name", "ola");
  cfg_params_valid.add_parameter("peer_nodes", "127.0.0.1:12345");
  cfg_params_valid.add_parameter("local_node", "127.0.0.1:12345");
  cfg_params_valid.add_parameter("bootstrap_group", "true");

  // Finally, set a valid one.
  cfg_params_valid.add_parameter("xcom_cache_size", "4000000000");
  // And verify that GCS initializes correctly.
  initialized = gcs->initialize(cfg_params_valid);
  ASSERT_EQ(GCS_OK, initialized);

  // Finalize the interface.
  gcs->finalize();

  // Fake factory cleanup member function.
  static_cast<Gcs_xcom_interface *>(gcs)->cleanup();
}

}  // namespace gcs_interface_unittest

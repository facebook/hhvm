/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "gcs_xcom_group_member_information.h"
#include "gcs_xcom_utils.h"

void homemade_free_site_def(unsigned int n, site_def *s,
                            node_address *node_addrs) {
  // TODO: replace the following with free_site_def(site_config) once
  //       the header file in site_def.h is fixed
  for (unsigned int i = 0; i < n; i++) free(node_addrs[i].uuid.data.data_val);
  free_node_set(&s->global_node_set);
  free_node_set(&s->local_node_set);
  remove_node_list(n, node_addrs, &s->nodes);
  free(s->nodes.node_list_val);
  free(s);
}

namespace gcs_parameters_unittest {

class GcsNodeAddressTest : public GcsBaseTest {
 protected:
  GcsNodeAddressTest() {}

  static void SetUpTestCase() { My_xp_util::init_time(); }
};

TEST_F(GcsNodeAddressTest, TestNodeAddress) {
  /*
    Check address "localhost:1030".
  */
  std::string hostname = "localhost";
  xcom_port port = 1030;
  std::string address = "localhost:1030";

  Gcs_xcom_node_address local_addr_1(address);
  ASSERT_EQ(local_addr_1.get_member_address(), address);
  ASSERT_EQ(local_addr_1.get_member_ip(), hostname);
  ASSERT_EQ(local_addr_1.get_member_port(), port);
  std::string *rep = local_addr_1.get_member_representation();
  ASSERT_EQ(*rep, address);
  delete rep;

  /*
    Check address "127.0.0.1:1030".
  */
  hostname = "127.0.0.1";
  port = 1030;
  address = "127.0.0.1:1030";
  Gcs_xcom_node_address local_addr_2(address);

  ASSERT_EQ(local_addr_2.get_member_address(), address);
  ASSERT_EQ(local_addr_2.get_member_ip(), hostname);
  ASSERT_EQ(local_addr_2.get_member_port(), port);
  rep = local_addr_2.get_member_representation();
  ASSERT_EQ(*rep, address);
  delete rep;

  /*
    Check address "127.0.0.1".
  */
  hostname = "";
  port = 0;
  address = "127.0.0.1";
  Gcs_xcom_node_address invalid_addr_1(address);

  ASSERT_EQ(invalid_addr_1.get_member_address(), address);
  ASSERT_EQ(invalid_addr_1.get_member_ip(), hostname);
  ASSERT_EQ(invalid_addr_1.get_member_port(), port);
  ASSERT_FALSE(invalid_addr_1.is_valid());
  rep = invalid_addr_1.get_member_representation();
  ASSERT_EQ(*rep, address);
  delete rep;

  /*
    Check address "127.0.0.1:".
  */
  hostname = "";
  port = 0;
  address = "127.0.0.1:";
  Gcs_xcom_node_address invalid_addr_2(address);

  ASSERT_EQ(invalid_addr_2.get_member_address(), address);
  ASSERT_EQ(invalid_addr_2.get_member_ip(), hostname);
  ASSERT_EQ(invalid_addr_2.get_member_port(), port);
  ASSERT_FALSE(invalid_addr_2.is_valid());
  rep = invalid_addr_2.get_member_representation();
  ASSERT_EQ(*rep, address);
  delete rep;

  /*
    Check address "127.0.0.1:invalid".
  */
  hostname = "";
  port = 0;
  address = "127.0.0.1:invalid";
  Gcs_xcom_node_address invalid_addr_3(address);

  ASSERT_EQ(invalid_addr_3.get_member_address(), address);
  ASSERT_EQ(invalid_addr_3.get_member_ip(), hostname);
  ASSERT_EQ(invalid_addr_3.get_member_port(), port);
  ASSERT_FALSE(invalid_addr_3.is_valid());
  rep = invalid_addr_3.get_member_representation();
  ASSERT_EQ(*rep, address);
  delete rep;
}

TEST_F(GcsNodeAddressTest, TestNodeAddressV6) {
  /*
    Check address "[::1]:1030".
  */
  std::string hostname = "::1";
  xcom_port port = 1030;
  std::string address = "[::1]:1030";
  Gcs_xcom_node_address local_addr_2(address);

  ASSERT_EQ(local_addr_2.get_member_address(), address);
  ASSERT_EQ(local_addr_2.get_member_ip(), hostname);
  ASSERT_EQ(local_addr_2.get_member_port(), port);
  std::string *rep = local_addr_2.get_member_representation();
  ASSERT_EQ(*rep, address);
  delete rep;

  /*
    Check address "[::1]".
  */
  hostname = "";
  port = 0;
  address = "[::1]";
  Gcs_xcom_node_address invalid_addr_1(address);

  ASSERT_EQ(invalid_addr_1.get_member_address(), address);
  ASSERT_EQ(invalid_addr_1.get_member_ip(), hostname);
  ASSERT_EQ(invalid_addr_1.get_member_port(), port);
  ASSERT_FALSE(invalid_addr_1.is_valid());
  rep = invalid_addr_1.get_member_representation();
  ASSERT_EQ(*rep, address);
  delete rep;

  /*
    Check address "[::1]:".
  */
  hostname = "";
  port = 0;
  address = "[::1]:";
  Gcs_xcom_node_address invalid_addr_2(address);

  ASSERT_EQ(invalid_addr_2.get_member_address(), address);
  ASSERT_EQ(invalid_addr_2.get_member_ip(), hostname);
  ASSERT_EQ(invalid_addr_2.get_member_port(), port);
  ASSERT_FALSE(invalid_addr_2.is_valid());
  rep = invalid_addr_2.get_member_representation();
  ASSERT_EQ(*rep, address);
  delete rep;

  /*
    Check address "[::1:1]:".
  */
  hostname = "";
  port = 0;
  address = "[::1:1]:";
  Gcs_xcom_node_address invalid_addr_3(address);

  ASSERT_EQ(invalid_addr_3.get_member_address(), address);
  ASSERT_EQ(invalid_addr_3.get_member_ip(), hostname);
  ASSERT_EQ(invalid_addr_3.get_member_port(), port);
  ASSERT_FALSE(invalid_addr_3.is_valid());
  rep = invalid_addr_3.get_member_representation();
  ASSERT_EQ(*rep, address);
  delete rep;

  /*
    Check address "::1".
  */
  hostname = "";
  port = 0;
  address = "::1";
  Gcs_xcom_node_address invalid_addr_4(address);

  ASSERT_EQ(invalid_addr_4.get_member_address(), address);
  ASSERT_EQ(invalid_addr_4.get_member_ip(), hostname);
  ASSERT_EQ(invalid_addr_4.get_member_port(), port);
  ASSERT_FALSE(invalid_addr_4.is_valid());
  rep = invalid_addr_4.get_member_representation();
  ASSERT_EQ(*rep, address);
  delete rep;

  /*
    Check address "[::1".
  */
  hostname = "";
  port = 0;
  address = "[::1";
  Gcs_xcom_node_address invalid_addr_5(address);

  ASSERT_EQ(invalid_addr_5.get_member_address(), address);
  ASSERT_EQ(invalid_addr_5.get_member_ip(), hostname);
  ASSERT_EQ(invalid_addr_5.get_member_port(), port);
  ASSERT_FALSE(invalid_addr_5.is_valid());
  rep = invalid_addr_5.get_member_representation();
  ASSERT_EQ(*rep, address);
  delete rep;

  /*
  Check address "::1]".
  */
  hostname = "";
  port = 0;
  address = "2606:b400:85c:1048:221:f6ff:fe2e:972b]:10301";
  Gcs_xcom_node_address invalid_addr_6(address);

  ASSERT_EQ(invalid_addr_6.get_member_address(), address);
  ASSERT_EQ(invalid_addr_6.get_member_ip(), hostname);
  ASSERT_EQ(invalid_addr_6.get_member_port(), port);
  ASSERT_FALSE(invalid_addr_6.is_valid());
  rep = invalid_addr_6.get_member_representation();
  ASSERT_EQ(*rep, address);
  delete rep;

  /*
  Check address "[hostname]".
  */
  hostname = "";
  port = 0;
  address = "[hostname]:10301";
  Gcs_xcom_node_address invalid_addr_7(address);

  ASSERT_EQ(invalid_addr_7.get_member_address(), address);
  ASSERT_EQ(invalid_addr_7.get_member_ip(), hostname);
  ASSERT_EQ(invalid_addr_7.get_member_port(), port);
  ASSERT_FALSE(invalid_addr_7.is_valid());
  rep = invalid_addr_7.get_member_representation();
  ASSERT_EQ(*rep, address);
  delete rep;
}

class GcsUUIDTest : public GcsBaseTest {
 protected:
  GcsUUIDTest() {}

  static void SetUpTestCase() { My_xp_util::init_time(); }
};

TEST_F(GcsUUIDTest, TestGcsUUID) {
  /*
    Check that the UUIDs are different as expected.
  */
  uchar *buffer = nullptr;
  u_int size = 0;
  Gcs_xcom_uuid uuid_1 = Gcs_xcom_uuid::create_uuid();

  /*
    Check that we cannot encode the UUID if the buffer is NULL.
  */
  ASSERT_FALSE(uuid_1.encode(&buffer, &size));

  /*
    Check that we can encode the UUID if the buffer is not NULL.
  */
  buffer = static_cast<uchar *>(malloc(uuid_1.actual_value.size()));
  ASSERT_TRUE(uuid_1.encode(&buffer, &size));

  /*
    Try to check that if we decode the buffer objects will have the
    same value. Note, howerver, that the parameters will make the
    operation fail.


    We are using the machine clock as UUID and nodes are created
    one after the other and the UUIDs may be equal. So we let the
    time pass by for just one second.
  */
  My_xp_util::sleep_seconds(1);
  Gcs_xcom_uuid uuid_2 = Gcs_xcom_uuid::create_uuid();
  ASSERT_FALSE(uuid_2.decode(nullptr, size));

  /*
    Check that if we decode the buffer objects will have the same value.
  */
  ASSERT_TRUE(uuid_1.actual_value != uuid_2.actual_value);
  uuid_2.decode(buffer, size);
  ASSERT_TRUE(uuid_1.actual_value == uuid_2.actual_value);

  free(buffer);
}

class GcsNodeInformationTest : public GcsBaseTest {
 protected:
  GcsNodeInformationTest() {}

  static void SetUpTestCase() { My_xp_util::init_time(); }
};

TEST_F(GcsNodeInformationTest, TestGcsNodeInformation) {
  std::string id_1("localhost:13001");
  std::string id_2("invalid");

  Gcs_xcom_node_information node_1(id_1);
  Gcs_xcom_node_information node_2(id_1, true);
  Gcs_xcom_node_information node_3(id_1, false);
  Gcs_xcom_node_information node_4(id_2, false);
  Gcs_xcom_uuid uuid_5 = Gcs_xcom_uuid::create_uuid();
  Gcs_xcom_node_information node_5(id_1, uuid_5, 1, false);

  /*
    Check whether the constructor:
      Gcs_xcom_node_information(const std::string &member_id,
                                const bool alive
                               );
    is working fine. Note that the first parameter defines the
    regular identifier.
  */
  ASSERT_EQ(node_1.get_member_id().get_member_id(), id_1);
  ASSERT_EQ(node_1.is_alive(), true);

  ASSERT_EQ(node_2.get_member_id().get_member_id(), id_1);
  ASSERT_EQ(node_2.is_alive(), true);

  ASSERT_EQ(node_3.get_member_id().get_member_id(), id_1);
  ASSERT_EQ(node_3.is_alive(), false);

  ASSERT_EQ(node_4.get_member_id().get_member_id(), id_2);
  ASSERT_EQ(node_4.is_alive(), false);

  /*
    Check information on node_no which must be initially invalid.
  */
  ASSERT_EQ(node_1.get_node_no(), VOID_NODE_NO);
  node_1.set_node_no(1);
  ASSERT_EQ(node_1.get_node_no(), 1);

  ASSERT_EQ(node_2.get_node_no(), VOID_NODE_NO);
  node_2.set_node_no(2);
  ASSERT_EQ(node_2.get_node_no(), 2);

  ASSERT_EQ(node_3.get_node_no(), VOID_NODE_NO);
  node_3.set_node_no(3);
  ASSERT_EQ(node_3.get_node_no(), 3);

  ASSERT_EQ(node_4.get_node_no(), VOID_NODE_NO);
  node_4.set_node_no(4);
  ASSERT_EQ(node_4.get_node_no(), 4);

  /*
    Set the timestamp and check whether it has timed out:
    (m_suspicion_creation_timestamp + suspicion_timeout) < now_ts
  */
  node_1.set_suspicion_creation_timestamp(1 /* Set the m_timestamp */);
  ASSERT_TRUE(node_1.has_timed_out(3 /* now_ts */, 1 /* timeout */));
  ASSERT_EQ(node_1.get_suspicion_creation_timestamp(), 1);

  /*
    Check whether the constructor:
      Gcs_xcom_node_information(const std::string &member_id,
                                const Gcs_xcom_uuid &uuid,
                                const unsigned int node_no,
                                const bool alive
                               );
    is working fine. Note that the first parameter defines the
    regular identifier.
  */
  ASSERT_EQ(node_5.get_node_no(), 1);
  ASSERT_TRUE(node_5.get_member_uuid().actual_value == uuid_5.actual_value);
  /*
    We are using the machine clock as UUID and nodes are created
    one after the other and the UUIDs may be equal. So we let the
    time pass by for just one second.
  */
  My_xp_util::sleep_seconds(1);
  node_5.regenerate_member_uuid();
  ASSERT_TRUE(node_5.get_member_uuid().actual_value != uuid_5.actual_value);
}

class GcsNodesTest : public GcsBaseTest {
 protected:
  GcsNodesTest() {}

  static void SetUpTestCase() { My_xp_util::init_time(); }
};

TEST_F(GcsNodesTest, TestGcsNodesBasicProperties) {
  Gcs_xcom_nodes nodes;

  Gcs_xcom_node_information node_1("localhost:1031");
  node_1.set_node_no(1);

  Gcs_xcom_node_information node_2("localhost:1032");
  node_2.set_node_no(2);

  Gcs_xcom_node_information node_3("localhost:1033");
  node_3.set_node_no(3);

  const Gcs_xcom_node_information *ret = nullptr;

  /*
    Initially the node's number is void and check whether the
    set_node_no() function is working properly.
  */
  ASSERT_FALSE(nodes.is_valid());
  ASSERT_EQ(nodes.get_node_no(), VOID_NODE_NO);
  nodes.set_node_no(0);
  ASSERT_EQ(nodes.get_node_no(), 0);
  ASSERT_TRUE(nodes.is_valid());

  /*
    Initially the number of nodes is zero.
  */
  ASSERT_EQ(nodes.get_size(), 0);
  ASSERT_TRUE(nodes.empty());

  /*
    Adding and removing nodes. Note that it allows duplicated nodes.
  */
  nodes.add_node(node_1);
  ASSERT_EQ(nodes.get_size(), 1);
  ASSERT_FALSE(nodes.empty());

  nodes.add_node(node_1);
  ASSERT_EQ(nodes.get_size(), 2);

  nodes.remove_node(node_1);
  ASSERT_EQ(nodes.get_size(), 1);

  nodes.add_node(node_2);
  ASSERT_EQ(nodes.get_size(), 2);

  /*
    Trying to get a node using get_node(const Gcs_member_identifier &member_id).
  */
  ret = nodes.get_node(node_1.get_member_id());
  ASSERT_EQ(ret->get_member_id(), node_1.get_member_id());

  ret = nodes.get_node(node_2.get_member_id());
  ASSERT_EQ(ret->get_member_id(), node_2.get_member_id());

  ret = nodes.get_node(node_3.get_member_id());
  ASSERT_TRUE(ret == nullptr);

  /*
    Trying to get a node using get_node(const std::string &member_id).
  */
  ret = nodes.get_node(node_1.get_member_id().get_member_id());
  ASSERT_EQ(ret->get_member_id(), node_1.get_member_id());

  ret = nodes.get_node(node_2.get_member_id().get_member_id());
  ASSERT_EQ(ret->get_member_id(), node_2.get_member_id());

  ret = nodes.get_node(node_3.get_member_id().get_member_id());
  ASSERT_TRUE(ret == nullptr);

  /*
    Trying to get a node using get_node(unsigned int node_no).
  */
  ret = nodes.get_node(node_1.get_member_id().get_member_id());
  ASSERT_EQ(ret->get_member_id(), node_1.get_member_id());

  ret = nodes.get_node(node_2.get_member_id().get_member_id());
  ASSERT_EQ(ret->get_member_id(), node_2.get_member_id());

  ret = nodes.get_node(node_3.get_member_id().get_member_id());
  ASSERT_TRUE(ret == nullptr);

  /*
    Trying to get a node using get_node(const Gcs_xcom_uuid &uuid).

    We are only looking for node_1 because we are using the machine
    clock as UUID and nodes are created one after the other and the
    UUIDs may be equal.
  */
  ret = nodes.get_node(node_1.get_member_uuid());
  ASSERT_EQ(ret->get_member_id(), node_1.get_member_id());
}

TEST_F(GcsNodesTest, TestGcsNodesCopyingNodes) {
  Gcs_xcom_nodes nodes;
  Gcs_xcom_nodes copied_nodes;

  Gcs_xcom_node_information node_1("localhost:1031");
  node_1.set_node_no(1);
  nodes.add_node(node_1);

  Gcs_xcom_node_information node_2("localhost:1032");
  node_2.set_node_no(2);
  nodes.add_node(node_2);

  const Gcs_xcom_node_information *ret = nullptr;
  Gcs_xcom_node_information *ret_1 = nullptr;
  Gcs_xcom_node_information *ret_2 = nullptr;

  /*
    Copying nodes through the add_nodes method which cleans up
    the previous data and copies the set of nodes provided as
    parameter.
  */
  copied_nodes.add_nodes(nodes);
  const std::vector<Gcs_xcom_node_information> &node_list =
      copied_nodes.get_nodes();
  std::vector<Gcs_xcom_node_information>::const_iterator nodes_it;
  for (nodes_it = node_list.begin(); nodes_it != node_list.end(); ++nodes_it) {
    ret = nodes.get_node((*nodes_it).get_member_id());
    ASSERT_EQ(ret->get_member_id(), (*nodes_it).get_member_id());
  }
  ASSERT_EQ(copied_nodes.get_size(), nodes.get_size());

  /*
    Check if the information in the different sets are copies and
    not references or pointers.
  */
  ret_1 = const_cast<Gcs_xcom_node_information *>(
      nodes.get_node(node_1.get_member_id()));
  ASSERT_TRUE(ret_1->get_member_id() == node_1.get_member_id());
  ASSERT_TRUE(ret_1->get_member_uuid().actual_value ==
              node_1.get_member_uuid().actual_value);
  ASSERT_TRUE(ret_1->get_node_no() == node_1.get_node_no());

  ret_2 = const_cast<Gcs_xcom_node_information *>(
      copied_nodes.get_node(node_1.get_member_id()));
  ASSERT_TRUE(ret_2->get_member_id() == node_1.get_member_id());
  ASSERT_TRUE(ret_2->get_member_uuid().actual_value ==
              node_1.get_member_uuid().actual_value);
  ASSERT_TRUE(ret_2->get_node_no() == node_1.get_node_no());

  ret_1->set_node_no(10);
  ret_2->set_node_no(20);
  ret_1 = const_cast<Gcs_xcom_node_information *>(
      nodes.get_node(node_1.get_member_id()));
  ret_2 = const_cast<Gcs_xcom_node_information *>(
      copied_nodes.get_node(node_1.get_member_id()));
  ASSERT_FALSE(ret_1->get_node_no() == node_1.get_node_no());
  ASSERT_FALSE(ret_2->get_node_no() == node_1.get_node_no());
  ASSERT_FALSE(ret_1->get_node_no() == ret_2->get_node_no());
  ASSERT_EQ(node_1.get_node_no(), 1);
  ASSERT_EQ(ret_1->get_node_no(), 10);
  ASSERT_EQ(ret_2->get_node_no(), 20);
}

TEST_F(GcsNodesTest, TestGcsNodesEncoding) {
  Gcs_xcom_nodes nodes;

  Gcs_xcom_node_information node_1("localhost:1031");
  node_1.set_node_no(1);
  nodes.add_node(node_1);

  /*
    We are using the machine clock as UUID and nodes are created
    one after the other and the UUIDs may be equal. So we let the
    time pass by for just one second.
  */
  My_xp_util::sleep_seconds(1);
  Gcs_xcom_node_information node_2("localhost:1032");
  node_2.set_node_no(2);
  nodes.add_node(node_2);

  /*
    Enconding the list of nodes in a format that can be used
    by XCOM and checking if the encode method is producing the
    expected result:

    length   -> Has the number of addresses and uuids.
    addrs[n] -> Pointer to the member identifier as string.
    uuids[n] -> data.data_len and data.data_val.
  */
  const Gcs_xcom_node_information *ret_1 = nullptr;
  const Gcs_xcom_node_information *ret_2 = nullptr;

  Gcs_xcom_uuid uuid_1;
  Gcs_xcom_uuid uuid_2;

  unsigned int length = 0;
  char **addrs = nullptr;
  blob *uuids = nullptr;

  nodes.encode(&length, &addrs, &uuids);

  ASSERT_EQ(length, 2);
  ret_1 = nodes.get_node(addrs[0]);
  ret_2 = nodes.get_node(addrs[1]);
  ASSERT_FALSE(ret_1->get_member_id() == ret_2->get_member_id());

  uuid_1.decode(reinterpret_cast<uchar *>(uuids[0].data.data_val),
                uuids[0].data.data_len);
  uuid_2.decode(reinterpret_cast<uchar *>(uuids[1].data.data_val),
                uuids[1].data.data_len);
  ret_1 = nodes.get_node(uuid_1);
  ret_2 = nodes.get_node(uuid_2);
  ASSERT_EQ(uuids[0].data.data_len, uuid_1.actual_value.size());
  ASSERT_EQ(uuids[1].data.data_len, uuid_2.actual_value.size());
  ASSERT_FALSE(ret_1->get_member_id() == ret_2->get_member_id());

  /*
    There is no problem if encoded is called twice. The nodes
    object will be responsible for deallocating any previously
    allocated memory without causing any leak.
  */
  nodes.encode(&length, &addrs, &uuids);
}

TEST_F(GcsNodesTest, TestGcsNodesConstructor) {
  const Gcs_xcom_node_information *ret = nullptr;

  Gcs_xcom_uuid uuid_1 = Gcs_xcom_uuid::create_uuid();
  blob blob_1 = {{0, static_cast<char *>(malloc(uuid_1.actual_value.size()))}};
  uuid_1.encode(reinterpret_cast<uchar **>(&blob_1.data.data_val),
                &blob_1.data.data_len);

  Gcs_xcom_uuid uuid_2 = Gcs_xcom_uuid::create_uuid();
  blob blob_2 = {{0, static_cast<char *>(malloc(uuid_2.actual_value.size()))}};
  uuid_2.encode(reinterpret_cast<uchar **>(&blob_2.data.data_val),
                &blob_2.data.data_len);

  node_address node_addrs[2] = {
      {const_cast<char *>("127.0.0.1:12345"), blob_1, {x_1_0, x_1_2}},
      {const_cast<char *>("127.0.0.1:12343"), blob_2, {x_1_0, x_1_2}}};

  site_def *site_config = new_site_def();
  init_site_def(2, node_addrs, site_config);
  site_config->nodeno = 0;
  site_config->x_proto = static_cast<xcom_proto>(1);

  node_set nodes;
  alloc_node_set(&nodes, 2);
  set_node_set(&nodes);
  nodes.node_set_val[0] = 0;

  /*
    Check if Gcs_xcom_nodes(const site_def *site, node_set &nodes) is
    working properly.
  */
  Gcs_xcom_nodes xcom_nodes(site_config, nodes);

  ret = xcom_nodes.get_node("127.0.0.1:12345");
  ASSERT_EQ(ret->get_member_id().get_member_id(), "127.0.0.1:12345");
  ASSERT_EQ(ret->get_node_no(), 0);
  ASSERT_FALSE(ret->is_alive());
  ASSERT_EQ(ret->get_member_uuid().actual_value, uuid_1.actual_value);

  ret = xcom_nodes.get_node("127.0.0.1:12343");
  ASSERT_EQ(ret->get_member_id().get_member_id(), "127.0.0.1:12343");
  ASSERT_EQ(ret->get_node_no(), 1);
  ASSERT_TRUE(ret->is_alive());
  ASSERT_EQ(ret->get_member_uuid().actual_value, uuid_2.actual_value);

  homemade_free_site_def(2, site_config, node_addrs);
  free_node_set(&nodes);
}

}  // namespace gcs_parameters_unittest

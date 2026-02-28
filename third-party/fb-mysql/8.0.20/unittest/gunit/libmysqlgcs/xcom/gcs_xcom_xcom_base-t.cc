/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <cstring>
#include <string>
#include <vector>
#include "app_data.h"
#include "get_synode_app_data.h"
#include "pax_msg.h"
#include "xcom_base.h"
#include "xcom_cache.h"
#include "xcom_memory.h"
#include "xcom_transport.h"

namespace xcom_base_unittest {

class XcomBase : public GcsBaseTest {
 protected:
  XcomBase() { ::init_cache(); }
  ~XcomBase() { ::deinit_cache(); }
};

TEST_F(XcomBase, XcomSendClientAppDataUpgradeScenario) {
  app_data a;
  std::string address("127.0.0.1:12345");

  char *names[] = {const_cast<char *>(address.c_str())};
  node_address *na = new_node_address(1, names);

  a.body.c_t = add_node_type;
  a.body.app_u_u.nodes.node_list_len = 1;
  a.body.app_u_u.nodes.node_list_val = na;

  int result = 0;
  result = are_we_allowed_to_upgrade_to_v6(&a);

  ASSERT_EQ(result, 1);
  delete_node_address(1, na);
}

TEST_F(XcomBase, XcomSendClientAppDataUpgradeScenarioV6) {
  app_data a;
  std::string address("[::1]:12345");

  char *names[] = {const_cast<char *>(address.c_str())};
  node_address *na = new_node_address(1, names);

  a.body.c_t = add_node_type;
  a.body.app_u_u.nodes.node_list_len = 1;
  a.body.app_u_u.nodes.node_list_val = na;

  int result = 0;
  result = are_we_allowed_to_upgrade_to_v6(&a);

  ASSERT_EQ(result, 0);

  delete_node_address(1, na);
}

TEST_F(XcomBase, XcomSendClientAppDataUpgradeScenarioMalformed) {
  app_data a;
  std::string address("::1]:12345");

  char *names[] = {const_cast<char *>(address.c_str())};
  node_address *na = new_node_address(1, names);

  a.body.c_t = add_node_type;
  a.body.app_u_u.nodes.node_list_len = 1;
  a.body.app_u_u.nodes.node_list_val = na;

  int result = 0;
  result = are_we_allowed_to_upgrade_to_v6(&a);

  ASSERT_EQ(result, 0);

  delete_node_address(1, na);
}

TEST_F(XcomBase, XcomNewClientEligibleDowngradeScenario) {
  std::string address("127.0.0.1:12345");

  char *names[] = {const_cast<char *>(address.c_str())};
  node_address *na = new_node_address(1, names);

  site_def *sd = new_site_def();
  init_site_def(1, na, sd);

  xcom_proto incoming = x_1_4;
  int result = is_new_node_eligible_for_ipv6(incoming, sd);

  ASSERT_EQ(result, 0);

  free_site_def(sd);
  delete_node_address(1, na);
}

TEST_F(XcomBase, XcomNewClientEligibleDowngradeScenarioFail) {
  std::string address("[::1]:12345");

  char *names[] = {const_cast<char *>(address.c_str())};
  node_address *na = new_node_address(1, names);

  site_def *sd = new_site_def();
  init_site_def(1, na, sd);

  xcom_proto incoming = x_1_4;
  int result = is_new_node_eligible_for_ipv6(incoming, sd);

  ASSERT_EQ(result, 1);

  free_site_def(sd);
  delete_node_address(1, na);
}

TEST_F(XcomBase, XcomNewClientEligibleDowngradeScenarioNullSiteDef) {
  xcom_proto incoming = x_1_4;
  int result = is_new_node_eligible_for_ipv6(incoming, nullptr);

  ASSERT_EQ(result, 0);
}

TEST_F(XcomBase, XcomNewClientEligibleDowngradeScenarioVersionSame) {
  xcom_proto incoming = my_xcom_version;
  int result = is_new_node_eligible_for_ipv6(incoming, nullptr);

  ASSERT_EQ(result, 0);
}

TEST_F(XcomBase, GetSynodeAppDataNotCached) {
  synode_no synode;
  synode.group_id = 12345;
  synode.msgno = 0;
  synode.node = 0;

  synode_no_array synodes;
  synodes.synode_no_array_len = 1;
  synodes.synode_no_array_val = &synode;

  synode_app_data_array result;
  result.synode_app_data_array_len = 0;
  result.synode_app_data_array_val = nullptr;

  auto error_code = ::xcom_get_synode_app_data(&synodes, &result);
  ASSERT_EQ(error_code, XCOM_GET_SYNODE_APP_DATA_NOT_CACHED);
}

TEST_F(XcomBase, GetSynodeAppDataNotDecided) {
  synode_no synode;
  synode.group_id = 12345;
  synode.msgno = 0;
  synode.node = 0;

  synode_no_array synodes;
  synodes.synode_no_array_len = 1;
  synodes.synode_no_array_val = &synode;

  synode_app_data_array result;
  result.synode_app_data_array_len = 0;
  result.synode_app_data_array_val = nullptr;

  /* Add the synode to the cache, but it is undecided. */
  get_cache(synode);

  auto error_code = ::xcom_get_synode_app_data(&synodes, &result);
  ASSERT_EQ(error_code, XCOM_GET_SYNODE_APP_DATA_NOT_DECIDED);
}

TEST_F(XcomBase, GetSynodeAppDataSuccessful) {
  synode_no synode;
  synode.group_id = 12345;
  synode.msgno = 0;
  synode.node = 0;

  synode_no_array synodes;
  synodes.synode_no_array_len = 1;
  synodes.synode_no_array_val = &synode;

  synode_app_data_array result;
  result.synode_app_data_array_len = 0;
  result.synode_app_data_array_val = nullptr;

  /* Add the synode to the cache, and set it as decided. */
  char const *const payload = "Message in a bottle";
  app_data_ptr a = new_app_data();
  a->body.c_t = app_type;
  a->body.app_u_u.data.data_len = std::strlen(payload) + 1;
  a->body.app_u_u.data.data_val = const_cast<char *>(payload);

  pax_msg *p = pax_msg_new_0(synode);
  p->op = learn_op;
  p->a = a;
  p->refcnt = 1;

  pax_machine *paxos = get_cache(synode);
  paxos->learner.msg = p;

  auto error_code = ::xcom_get_synode_app_data(&synodes, &result);
  ASSERT_EQ(error_code, XCOM_GET_SYNODE_APP_DATA_OK);

  ASSERT_EQ(result.synode_app_data_array_len, 1);
  ASSERT_EQ(synode_eq(result.synode_app_data_array_val[0].synode, synode), 1);
  ASSERT_EQ(result.synode_app_data_array_val[0].data.data_len,
            p->a->body.app_u_u.data.data_len);
  ASSERT_EQ(std::strcmp(result.synode_app_data_array_val[0].data.data_val,
                        p->a->body.app_u_u.data.data_val),
            0);

  /* Cleanup */
  a->body.app_u_u.data.data_len = 0;
  a->body.app_u_u.data.data_val = nullptr;

  unchecked_replace_pax_msg(&paxos->learner.msg, nullptr);

  my_xdr_free(reinterpret_cast<xdrproc_t>(xdr_synode_app_data_array),
              reinterpret_cast<char *>(&result));
}

TEST_F(XcomBase, GetSynodeAppDataTooManySynodes) {
  /*
   Bypass protocol negotiation because we are not actually connected to
   anything.
  */
  connection_descriptor con;
  con.connected_ = CON_PROTO;
  con.x_proto = get_latest_common_proto();

  constexpr uint32_t group_id = 1;
  /* Unserializable message. Exceeds array size. */
  u_int constexpr nr_synodes = MAX_SYNODE_ARRAY + 1;

  synode_no_array synodes;
  synodes.synode_no_array_len = nr_synodes;
  synodes.synode_no_array_val =
      static_cast<synode_no *>(std::calloc(nr_synodes, sizeof(synode_no)));
  ASSERT_NE(synodes.synode_no_array_val, nullptr);

  synode_app_data_array reply;
  ASSERT_EQ(xcom_client_get_synode_app_data(&con, group_id, &synodes, &reply),
            0);

  std::free(synodes.synode_no_array_val);
}

/* Disable on Windows. The test outcome varies wildly on our test environment,
 * likely due to different configurations of the stack size. */
#if !defined(_WIN32)
TEST_F(XcomBase, ProposerBatchDeserialization) {
  pax_msg *p = nullptr;
  unchecked_replace_pax_msg(&p, pax_msg_new_0(null_synode));

  for (auto i = 0; i < MAX_BATCH_APP_DATA; i++) {
    app_data_ptr a = new_app_data();
    a->body.c_t = app_type;
    a->next = p->a;

    p->a = a;
  }

  char *buffer = nullptr;
  uint32_t buffer_len = 0;

  int const serialized =
      serialize_msg(p, get_latest_common_proto(), &buffer_len, &buffer);
  ASSERT_EQ(serialized, 1);

  unchecked_replace_pax_msg(&p, pax_msg_new_0(null_synode));

  int const deserialized =
      deserialize_msg(p, get_latest_common_proto(), buffer, buffer_len);
  ASSERT_EQ(deserialized, 1);

  std::free(p);
  std::free(buffer);
}
#endif  // !defined(_WIN32)

// clang-format off
/*
This test validates the fix of
Bug #28966455 APPLIER LOG MISSES A TRANSACTION IN GR.

Situation
---------
S0 is the leader, trying to get consensus on a transaction T for its own slot.
S1 is the follower, trying to take over the slot and get consensus on a no_op.

Legend
------
SX: Server X
PX: Proposer of SX
ALX: Acceptor/learner of SX
O: Event on the respective server
X->: Message sent from the server on X to the server on ->
###: Comment/observation
E: The part where P1 is deviating from the Paxos protocol

Diagram
-------
  S0           S1         S2
P0  AL0      P1  AL1      AL2
|    O       |    O        O  AL{0,1,2}.promise = (0,0)
|    |       |    |        |
############################  P0 starts trying consensus for T
|    |       |    |        |
O    |       |    |        |  P0.ballot = (0,0); P0.value = T
X--->|-------|--->|------->|  accept_op[ballot=(0,0),
|    |       |    |        |            value=T (P0.value)]
|    |       |    |        |
|    O       |    O        O  AL{0,1,2}.value = T
|<---X-------|----X--------X  ack_accept_op[ballot=(0,0)]
|    |       |    |        |
############################  P0 got majority of accepts for (0,0) T
|    |       |    |        |
############################  P1 starts trying consensus for no_op
|    |       |    |        |
|    |       O    |        |  P1.ballot = (1,1); P1.value = no_op
|    |<------X--->|------->|  prepare_op[ballot=(1,1)]
|    |       |    |        |
|    O       |    O        O  AL{0,1,2}.promise = (1,1)
|    X------>|<---X--------X  ack_prepare_op[ballot=(1,1),
|    |       |    |        |                 accepted={(0,0) T}]
|    |       |    |        |
############################  P1 got a majority of prepares for (1,1)
|    |       |    |        |
|    |       E    |        |  P1.value should be set to T here.
|    |       E    |        |  According to the Paxos protocol, if any
|    |       E    |        |  acceptor replies with a previously
|    |       E    |        |  accepted value, one must use it. But
|    |       E    |        |  handle_ack_prepare did not do it because
|    |       E    |        |  handle_ack_prepare has the following code:
|    |       E    |        |
|    |       E    |        |  if (gt_ballot(m->proposal,
|    |       E    |        |                p->proposer.msg->proposal))
|    |       E    |        |  {
|    |       E    |        |    replace_pax_msg(&p->proposer.msg, m);
|    |       E    |        |    ...
|    |       E    |        |  }
|    |       E    |        |
|    |       E    |        |  And p->proposer.msg->proposal was
|    |       E    |        |  initialized to (0,1) on P1, meaning that:
|    |       E    |        |
|    |       E    |        |  if (0,0) > (0,1): P1.value = T
|    |       E    |        |
|    |       E    |        |  Therefore, P1.value = no_op.
|    |       E    |        |  (see handle_ack_prepare)
|    |       |    |        |
|    |  ...--X--->|------->|  accept_op[ballot=(1,1),
|    |       |    |        |            value=no_op (P1.value)]
|    |       |    |        |
|    |       |    O        O  AL{1,2}.value = no_op
|    |       |<---X--------X  ack_accept_op[ballot=(1,1)]
|    |       |    |        |
############################  P1 got majority of accepts for (1,1) no_op
############################  Values accepted for P{0,1} don't agree
|    |       |    |        |
|    |  ...--X--->|------->|  tiny_learn_op[ballot=(1,1), no_op]
|    |       |    |        |
|    |       |    O        O  AL{1,2} learn no_op
|    |       |    O        O  Executor task of S{1,2} delivers no_op
|    |       |    |        |
X--->|--...  |    |        |  tiny_learn_op[ballot=(0,0)]
|    |       |    |        |
|    O       |    |        |  AL0 learns T
|    O       |    |        |  Executor task of S0 delivers T
|    |       |    |        |
############################  S0 delivered T, S{1,2} delivered no_op
|    |       |    |        |
*/
// clang-format on
TEST_F(XcomBase, PaxosLearnSameValue) {
  // Synod (42, 0).
  synode_no synod;
  synod.group_id = 1;
  synod.msgno = 42;
  synod.node = 0;

  // pax_machine for each "server."
  auto *s0_paxos =
      static_cast<pax_machine *>(std::calloc(1, sizeof(pax_machine)));
  init_pax_machine(s0_paxos, nullptr, synod);

  auto *s1_paxos =
      static_cast<pax_machine *>(std::calloc(1, sizeof(pax_machine)));
  init_pax_machine(s1_paxos, nullptr, synod);

  auto *s2_paxos =
      static_cast<pax_machine *>(std::calloc(1, sizeof(pax_machine)));
  init_pax_machine(s2_paxos, nullptr, synod);

  // site_def for each "server."
  auto *s0_config = new_site_def();
  s0_config->nodeno = 0;
  s0_config->nodes.node_list_len = 3;
  s0_config->global_node_set.node_set_len = 3;

  auto *s1_config = new_site_def();
  s1_config->nodeno = 1;
  s1_config->nodes.node_list_len = 3;
  s1_config->global_node_set.node_set_len = 3;

  auto *s2_config = new_site_def();
  s2_config->nodeno = 2;
  s2_config->nodes.node_list_len = 3;
  s2_config->global_node_set.node_set_len = 3;

  // clang-format off
  /****************************************************************************
     S0           S1         S2
   P0  AL0      P1  AL1      AL2
   |    |       |    |        |
   O    |       |    |        |  P0.ballot = (0,0); P0.value = T
   X--->|-------|--->|------->|  accept_op[ballot=(0,0),
   |    |       |    |        |            value=T (P0.value)]
   |    |       |    |        |
   |    O       |    O        O  AL{0,1,2}.value = T
   |<---X-------|----X--------X  ack_accept_op[ballot=(0,0)]
   |    |       |    |        |
   ****************************************************************************/
  // clang-format on
  pax_msg *tx = pax_msg_new(synod, nullptr);
  tx->a = new_app_data();
  tx->a->body.c_t = app_type;
  tx->a->body.app_u_u.data.data_len = 1;
  tx->a->body.app_u_u.data.data_val = static_cast<char *>(malloc(sizeof(char)));
  replace_pax_msg(&s0_paxos->proposer.msg, tx);
  prepare_push_2p(s0_config, s0_paxos);
  pax_msg *s0_accept_tx = s0_paxos->proposer.msg;
  init_propose_msg(s0_accept_tx);
  s0_accept_tx->from = 0;

  ballot ballot_tx;
  ballot_tx.cnt = 0;
  ballot_tx.node = 0;
  ASSERT_TRUE(eq_ballot(s0_paxos->proposer.msg->proposal, ballot_tx));

  // S0 sends s0_accept_tx to AL{0,1,2}

  // AL{0,1,2} receive s0_accept_tx

  pax_msg *s0_accept_tx_s0 = clone_pax_msg(s0_accept_tx);
  pax_msg *s0_ack_accept_tx =
      handle_simple_accept(s0_paxos, s0_accept_tx_s0, s0_accept_tx_s0->synode);
  ASSERT_NE(nullptr, s0_ack_accept_tx);
  s0_ack_accept_tx->from = 0;

  pax_msg *s0_accept_tx_s1 = clone_pax_msg(s0_accept_tx);
  pax_msg *s1_ack_accept_tx =
      handle_simple_accept(s1_paxos, s0_accept_tx_s1, s0_accept_tx_s1->synode);
  ASSERT_NE(nullptr, s1_ack_accept_tx);
  s1_ack_accept_tx->from = 1;

  pax_msg *s0_accept_tx_s2 = clone_pax_msg(s0_accept_tx);
  pax_msg *s2_ack_accept_tx =
      handle_simple_accept(s2_paxos, s0_accept_tx_s2, s0_accept_tx_s2->synode);
  ASSERT_NE(nullptr, s2_ack_accept_tx);
  s2_ack_accept_tx->from = 2;

  // AL{0,1,2} send s{0,1,2}_ack_accept_tx to P0

  // P0 receives s{0,1,2}_ack_accept_tx

  pax_msg *s0_ack_accept_tx_s0 = clone_pax_msg(s0_ack_accept_tx);
  ASSERT_EQ(nullptr,
            handle_simple_ack_accept(s0_config, s0_paxos, s0_ack_accept_tx_s0));

  pax_msg *s1_ack_accept_tx_s0 = clone_pax_msg(s1_ack_accept_tx);
  pax_msg *s0_learn_tx =
      handle_simple_ack_accept(s0_config, s0_paxos, s1_ack_accept_tx_s0);
  ASSERT_NE(nullptr, s0_learn_tx);
  ASSERT_EQ(tiny_learn_op, s0_learn_tx->op);
  s0_learn_tx->from = 0;

  pax_msg *s2_ack_accept_tx_s0 = clone_pax_msg(s2_ack_accept_tx);
  ASSERT_EQ(nullptr,
            handle_simple_ack_accept(s0_config, s0_paxos, s2_ack_accept_tx_s0));

  // clang-format off
  /****************************************************************************
     S0           S1         S2
   P0  AL0      P1  AL1      AL2
   |    |       |    |        |
   |    |       O    |        |  P1.ballot = (1,1); P1.value = no_op
   |    |<------X--->|------->|  prepare_op[ballot=(1,1)]
   |    |       |    |        |
   |    |       |    |        |
   |    O       |    O        O  AL{0,1,2}.promise = (1,1)
   |    X------>|<---X--------X  ack_prepare_op[ballot=(1,1),
   |    |       |    |        |                 accepted={(0,0) T}]
   |    |       |    |        |
   ****************************************************************************/
  // clang-format on
  replace_pax_msg(&s1_paxos->proposer.msg, pax_msg_new(synod, s1_config));
  create_noop(s1_paxos->proposer.msg);
  pax_msg *s1_prepare_noop = clone_pax_msg(s1_paxos->proposer.msg);
  prepare_push_3p(s1_config, s1_paxos, s1_prepare_noop, synod, no_op);
  init_prepare_msg(s1_prepare_noop);
  s1_prepare_noop->from = 1;

  ballot ballot_noop;
  ballot_noop.cnt = 1;
  ballot_noop.node = 1;
  ASSERT_TRUE(eq_ballot(s1_prepare_noop->proposal, ballot_noop));

  // P1 sends s1_prepare_noop to AL{0,1,2}

  // AL{0,1,2} receive s1_prepare_noop

  pax_msg *s1_prepare_noop_s0 = clone_pax_msg(s1_prepare_noop);
  pax_msg *s0_ack_prepare_noop =
      handle_simple_prepare(s0_paxos, s1_prepare_noop_s0, synod);
  ASSERT_NE(nullptr, s0_ack_prepare_noop);
  ASSERT_EQ(ack_prepare_op, s0_ack_prepare_noop->op);
  ASSERT_TRUE(eq_ballot(s0_ack_prepare_noop->proposal, ballot_tx));
  ASSERT_EQ(normal, s0_ack_prepare_noop->msg_type);
  s0_ack_prepare_noop->from = 0;

  pax_msg *s1_prepare_noop_s1 = clone_pax_msg(s1_prepare_noop);
  pax_msg *s1_ack_prepare_noop =
      handle_simple_prepare(s1_paxos, s1_prepare_noop_s1, synod);
  ASSERT_NE(nullptr, s1_ack_prepare_noop);
  ASSERT_EQ(ack_prepare_op, s1_ack_prepare_noop->op);
  ASSERT_TRUE(eq_ballot(s1_ack_prepare_noop->proposal, ballot_tx));
  ASSERT_EQ(normal, s1_ack_prepare_noop->msg_type);
  s1_ack_prepare_noop->from = 1;

  pax_msg *s1_prepare_noop_s2 = clone_pax_msg(s1_prepare_noop);
  pax_msg *s2_ack_prepare_noop =
      handle_simple_prepare(s2_paxos, s1_prepare_noop_s2, synod);
  ASSERT_NE(nullptr, s2_ack_prepare_noop);
  ASSERT_EQ(ack_prepare_op, s2_ack_prepare_noop->op);
  ASSERT_TRUE(eq_ballot(s2_ack_prepare_noop->proposal, ballot_tx));
  ASSERT_EQ(normal, s2_ack_prepare_noop->msg_type);
  s2_ack_prepare_noop->from = 2;

  // AL{0,1,2} send s{0,1,2}_ack_prepare_noop to P1

  // P1 receives s{0,1,2}_ack_prepare_noop

  pax_msg *s0_ack_prepare_noop_s1 = clone_pax_msg(s0_ack_prepare_noop);
  ASSERT_FALSE(
      handle_simple_ack_prepare(s1_config, s1_paxos, s0_ack_prepare_noop_s1));

  pax_msg *s1_ack_prepare_noop_s1 = clone_pax_msg(s1_ack_prepare_noop);
  bool can_send_accept =
      handle_simple_ack_prepare(s1_config, s1_paxos, s1_ack_prepare_noop_s1);
  ASSERT_TRUE(can_send_accept);
  pax_msg *s1_accept_noop = s1_paxos->proposer.msg;
  ASSERT_NE(nullptr, s1_accept_noop);
  ASSERT_EQ(accept_op, s1_accept_noop->op);
  s1_accept_noop->from = 1;

  pax_msg *s2_ack_prepare_noop_s1 = clone_pax_msg(s2_ack_prepare_noop);
  ASSERT_FALSE(
      handle_simple_ack_prepare(s1_config, s1_paxos, s2_ack_prepare_noop_s1));

  // clang-format off
  /****************************************************************************
     S0           S1         S2
   P0  AL0      P1  AL1      AL2
   |    |       |    |        |
   |    |       E    |        |  P1.value should be set to T here.
   |    |       E    |        |  According to the Paxos protocol, if any
   |    |       E    |        |  acceptor replies with a previously
   |    |       E    |        |  accepted value, one must use it. But
   |    |       E    |        |  handle_ack_prepare will not do it because
   |    |       E    |        |  handle_ack_prepare has the following code:
   |    |       E    |        |
   |    |       E    |        |  if (gt_ballot(m->proposal,
   |    |       E    |        |                p->proposer.msg->proposal))
   |    |       E    |        |  {
   |    |       E    |        |    replace_pax_msg(&p->proposer.msg, m);
   |    |       E    |        |    ...
   |    |       E    |        |  }
   |    |       E    |        |
   |    |       E    |        |  However, p->proposer.msg->proposal is initialized
   |    |       E    |        |  to (0,1) on P1, meaning that:
   |    |       E    |        |
   |    |       E    |        |  if (0,0) > (0,1): P1.value = no_op
   |    |       E    |        |
   |    |       E    |        |  Therefore, P1.value = no_op.
   |    |       E    |        |  (see handle_ack_prepare)
   |    |       |    |        |
   |    |  ...--X--->|------->|  accept_op[ballot=(1,1),
   |    |       |    |        |            value=no_op (P1.value)]
   |    |       |    |        |
   |    |       |    O        O  AL{0,1,2}.value = no_op
   |    |       |<---X--------X  ack_accept_op[ballot=(1,1)]
   |    |       |    |        |
   |    |       |    |        |
   |    |  ...--X--->|------->|  tiny_learn_op[ballot=(1,1), no_op]
   |    |       |    |        |
   |    |       |    O        O  AL{1,2} learn no_op
   |    |       |    O        O  Executor task of S{1,2} delivers no_op
   |    |       |    |        |
   ****************************************************************************/
  // clang-format on
  /*
   Here was the problem. P1 should have inherited T from one of AL{0,1,2}. But
   it did not because, s1_paxos->proposer.msg->proposal was initialized to
   (0,1).
   This lead to gt_ballot(m->proposal, p->proposer.msg->proposal) being false:

     gt_ballot(m->proposal, p->proposer.msg->proposal) <=>
     gt_ballot((0,0), (0,1)) <=>
     false

   The assert below would fire on mysql-trunk before the fix for
   Bug #28966455 APPLIER LOG MISSES A TRANSACTION IN GR.
   */
  // assert(s1_paxos->proposer.msg->msg_type != no_op);

  // P1 sends s1_accept_noop to AL{1,2}
  // P1 alsos sends s1_accept_noop to AL0, but it gets delayed

  // AL{1,2} receive s1_accept_noop
  pax_msg *s1_ack_accept_noop =
      handle_simple_accept(s1_paxos, clone_pax_msg(s1_accept_noop), synod);
  s1_ack_accept_noop->from = 1;

  pax_msg *s2_ack_accept_noop =
      handle_simple_accept(s2_paxos, clone_pax_msg(s1_accept_noop), synod);
  s2_ack_accept_noop->from = 2;

  // AL{1,2} send s{1,2}_ack_accept_noop to P1

  // P1 receives s{1,2}_ack_accept_noop
  pax_msg *s1_ack_accept_noop_s1 = clone_pax_msg(s1_ack_accept_noop);
  ASSERT_EQ(nullptr, handle_simple_ack_accept(s1_config, s1_paxos,
                                              s1_ack_accept_noop_s1));

  pax_msg *s2_ack_accept_noop_s1 = clone_pax_msg(s2_ack_accept_noop);
  pax_msg *s1_learn_noop =
      handle_simple_ack_accept(s1_config, s1_paxos, s2_ack_accept_noop_s1);
  ASSERT_NE(nullptr, s1_learn_noop);
  ASSERT_EQ(tiny_learn_op, s1_learn_noop->op);
  s1_learn_noop->from = 1;

  // P1 sends s1_learn_noop to AL{1,2}
  // P1 alsos sends s1_learn_noop to AL0, but it gets delayed

  // AL{1,2} receive s1_learn_noop
  pax_msg *s1_learn_noop_s1 = clone_pax_msg(s1_learn_noop);
  handle_learn(s1_config, s1_paxos, s1_learn_noop_s1);

  pax_msg *s1_learn_noop_s2 = clone_pax_msg(s1_learn_noop);
  handle_learn(s2_config, s2_paxos, s1_learn_noop_s2);

  ASSERT_TRUE(pm_finished(s1_paxos));

  ASSERT_TRUE(pm_finished(s2_paxos));

  // S1 and S2 would deliver no_op... (but deliver tx after the fix)

  // clang-format off
  /****************************************************************************
     S0           S1         S2
   P0  AL0      P1  AL1      AL2
   |    |       |    |        |
   X--->|--...  |    |        |  tiny_learn_op[ballot=(0,0)]
   |    |       |    |        |
   |    O       |    |        |  AL0 learns T
   |    O       |    |        |  Executor task of S0 delivers T
   |    |       |    |        |
   ****************************************************************************/
  // clang-format on

  // P0 sends s0_learn_tx to AL0
  // P1 alsos sends s0_learn_tx to AL{1,2}, but it doesn't matter

  // AL0 receive s0_learn_tx
  pax_msg *s0_learn_tx_s0 = clone_pax_msg(s0_learn_tx);
  handle_tiny_learn(s0_config, s0_paxos, s0_learn_tx_s0);

  ASSERT_TRUE(pm_finished(s0_paxos));

  // ...and S0 delivers tx

  bool const every_executor_delivered_same_value =
      (s0_paxos->learner.msg->msg_type == s1_paxos->learner.msg->msg_type &&
       s1_paxos->learner.msg->msg_type == s2_paxos->learner.msg->msg_type);
  ASSERT_TRUE(every_executor_delivered_same_value);

  // Cleanup.
  auto free_pax_msg = [](pax_msg *p) {
    p->refcnt = 1;
    replace_pax_msg(&p, nullptr);
  };

  init_pax_machine(s0_paxos, nullptr, synod);
  std::free(s0_paxos->proposer.prep_nodeset->bits.bits_val);
  std::free(s0_paxos->proposer.prep_nodeset);
  std::free(s0_paxos->proposer.prop_nodeset->bits.bits_val);
  std::free(s0_paxos->proposer.prop_nodeset);
  std::free(s0_paxos);
  std::free(s0_config);
  free_pax_msg(s0_ack_accept_tx);
  free_pax_msg(s0_ack_accept_tx_s0);
  free_pax_msg(s0_learn_tx);
  free_pax_msg(s1_prepare_noop_s0);
  free_pax_msg(s0_ack_prepare_noop);
  free_pax_msg(s0_learn_tx_s0);

  init_pax_machine(s1_paxos, nullptr, synod);
  std::free(s1_paxos->proposer.prep_nodeset->bits.bits_val);
  std::free(s1_paxos->proposer.prep_nodeset);
  std::free(s1_paxos->proposer.prop_nodeset->bits.bits_val);
  std::free(s1_paxos->proposer.prop_nodeset);
  std::free(s1_paxos);
  std::free(s1_config);
  free_pax_msg(s1_ack_accept_tx);
  free_pax_msg(s1_ack_accept_tx_s0);
  free_pax_msg(s1_prepare_noop);
  free_pax_msg(s1_prepare_noop_s1);
  free_pax_msg(s1_ack_prepare_noop);
  free_pax_msg(s1_ack_prepare_noop_s1);
  free_pax_msg(s2_ack_prepare_noop_s1);
  free_pax_msg(s1_ack_accept_noop);
  free_pax_msg(s1_ack_accept_noop_s1);
  free_pax_msg(s2_ack_accept_noop_s1);
  free_pax_msg(s1_learn_noop);

  init_pax_machine(s2_paxos, nullptr, synod);
  std::free(s2_paxos->proposer.prep_nodeset->bits.bits_val);
  std::free(s2_paxos->proposer.prep_nodeset);
  std::free(s2_paxos->proposer.prop_nodeset->bits.bits_val);
  std::free(s2_paxos->proposer.prop_nodeset);
  std::free(s2_paxos);
  std::free(s2_config);
  free_pax_msg(s2_ack_accept_tx);
  free_pax_msg(s2_ack_accept_tx_s0);
  free_pax_msg(s1_prepare_noop_s2);
  free_pax_msg(s2_ack_prepare_noop);
  free_pax_msg(s2_ack_accept_noop);
}

TEST_F(XcomBase, HandleBootWithoutIdentity) {
  // Synod (42, 0).
  synode_no synod;
  synod.group_id = 1;
  synod.msgno = 42;
  synod.node = 0;

  // Fake node information.
  char *name = const_cast<char *>("127.0.0.1:10001");
  char *names[] = {name};
  blob uuid;
  uuid.data.data_len = 1;
  uuid.data.data_val = const_cast<char *>("1");
  blob uuids[] = {uuid};

  // site_def for the "server."
  auto *config = new_site_def();
  config->nodeno = 0;
  config->nodes.node_list_len = 1;
  config->nodes.node_list_val = ::new_node_address_uuid(1, names, uuids);

  pax_msg *need_boot = pax_msg_new(synod, nullptr);
  // need_boot_op without an identity.
  init_need_boot_op(need_boot, nullptr);
  ASSERT_TRUE(should_handle_boot(config, need_boot));

  // Cleanup.
  need_boot->refcnt = 1;
  replace_pax_msg(&need_boot, nullptr);

  ::delete_node_address(1, config->nodes.node_list_val);

  std::free(config);
}

TEST_F(XcomBase, HandleBootWithIdentityOfExistingMember) {
  // Synod (42, 0).
  synode_no synod;
  synod.group_id = 1;
  synod.msgno = 42;
  synod.node = 0;

  // Fake node information.
  char *name = const_cast<char *>("127.0.0.1:10001");
  char *names[] = {name};
  blob uuid;
  uuid.data.data_len = 1;
  uuid.data.data_val = const_cast<char *>("1");
  blob uuids[] = {uuid};

  // site_def for the "server."
  auto *config = new_site_def();
  config->nodeno = 0;
  config->nodes.node_list_len = 1;
  config->nodes.node_list_val = ::new_node_address_uuid(1, names, uuids);

  pax_msg *need_boot = pax_msg_new(synod, nullptr);
  // need_boot_op with an identity.
  node_address *identity = ::new_node_address_uuid(1, names, uuids);
  init_need_boot_op(need_boot, identity);
  ASSERT_TRUE(should_handle_boot(config, need_boot));

  // Cleanup.
  need_boot->refcnt = 1;
  replace_pax_msg(&need_boot, nullptr);

  ::delete_node_address(1, config->nodes.node_list_val);
  ::delete_node_address(1, identity);

  std::free(config);
}

TEST_F(XcomBase, HandleBootWithIdentityOfNonExistingMember) {
  // Synod (42, 0).
  synode_no synod;
  synod.group_id = 1;
  synod.msgno = 42;
  synod.node = 0;

  // Fake node information.
  char *name = const_cast<char *>("127.0.0.1:10001");
  char *names[] = {name};
  blob uuid;
  uuid.data.data_len = 1;
  uuid.data.data_val = const_cast<char *>("1");
  blob uuids[] = {uuid};

  // site_def for the "server."
  auto *config = new_site_def();
  config->nodeno = 0;
  config->nodes.node_list_len = 1;
  config->nodes.node_list_val = ::new_node_address_uuid(1, names, uuids);

  pax_msg *need_boot = pax_msg_new(synod, nullptr);
  // need_boot_op with an identity.
  blob unknown_uuid;
  unknown_uuid.data.data_len = 1;
  unknown_uuid.data.data_val = const_cast<char *>("2");
  blob unknown_uuids[] = {unknown_uuid};
  node_address *identity = ::new_node_address_uuid(1, names, unknown_uuids);
  init_need_boot_op(need_boot, identity);
  ASSERT_FALSE(should_handle_boot(config, need_boot));

  // Cleanup.
  need_boot->refcnt = 1;
  replace_pax_msg(&need_boot, nullptr);

  ::delete_node_address(1, config->nodes.node_list_val);
  ::delete_node_address(1, identity);

  std::free(config);
}

TEST_F(XcomBase, HandleBootWithMoreThanOneIdentity) {
  // Synod (42, 0).
  synode_no synod;
  synod.group_id = 1;
  synod.msgno = 42;
  synod.node = 0;

  // Fake node information.
  char *name = const_cast<char *>("127.0.0.1:10001");
  char *names[] = {name};
  blob uuid;
  uuid.data.data_len = 1;
  uuid.data.data_val = const_cast<char *>("1");
  blob uuids[] = {uuid};

  // site_def for the "server."
  auto *config = new_site_def();
  config->nodeno = 0;
  config->nodes.node_list_len = 1;
  config->nodes.node_list_val = ::new_node_address_uuid(1, names, uuids);

  pax_msg *need_boot = pax_msg_new(synod, nullptr);
  // need_boot_op with two identities.
  char *other_name = const_cast<char *>("127.0.0.1:10002");
  char *two_names[] = {name, other_name};
  blob two_uuids[] = {uuid, uuid};
  node_address *identity = ::new_node_address_uuid(2, two_names, two_uuids);
  need_boot->op = need_boot_op;
  if (identity != nullptr) {
    need_boot->a = new_app_data();
    need_boot->a->body.c_t = xcom_boot_type;
    init_node_list(2, identity, &need_boot->a->body.app_u_u.nodes);
  }
  ASSERT_FALSE(should_handle_boot(config, need_boot));

  // Cleanup.
  need_boot->refcnt = 1;
  replace_pax_msg(&need_boot, nullptr);

  ::delete_node_address(1, config->nodes.node_list_val);
  ::delete_node_address(2, identity);

  std::free(config);
}

}  // namespace xcom_base_unittest

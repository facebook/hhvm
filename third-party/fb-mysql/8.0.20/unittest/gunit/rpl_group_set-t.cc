/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <string.h>

#define FRIEND_OF_GTID_SET class GroupTest_Group_containers_Test
#define FRIEND_OF_GROUP_CACHE class GroupTest_Group_containers_Test
#define FRIEND_OF_GROUP_LOG_STATE class GroupTest_Group_containers_Test
#define NON_DISABLED_UNITTEST_GTID

#include "binlog.h"
#include "my_thread.h"
#include "rpl_gtid.h"
#include "sql_class.h"

#define N_SIDS 16

#define ASSERT_OK(X) ASSERT_EQ(RETURN_STATUS_OK, X)
#define EXPECT_OK(X) EXPECT_EQ(RETURN_STATUS_OK, X)
#define EXPECT_NOK(X) EXPECT_NE(RETURN_STATUS_OK, X)

class GroupTest : public ::testing::Test {
 public:
  static const char *uuids[16];
  rpl_sid sids[16];
  unsigned int seed;

  void SetUp() {
    seed = (unsigned int)time(nullptr);
    printf("# seed = %u\n", seed);
    srand(seed);
    for (int i = 0; i < 16; i++) sids[i].parse(uuids[i]);

    verbose = false;
    errtext_stack_pos = 0;
    errtext_stack[0] = 0;
    append_errtext(__LINE__, "seed=%d", seed);
    my_delete("sid-map-0", MYF(0));
    my_delete("sid-map-1", MYF(0));
    my_delete("sid-map-2", MYF(0));
  }

  void TearDown() {
    my_delete("sid-map-0", MYF(0));
    my_delete("sid-map-1", MYF(0));
    my_delete("sid-map-2", MYF(0));
  }

  /*
    Test that different, equivalent ways to construct a Gtid_set give
    the same resulting Gtid_set.  This is used to test Gtid_set,
    Sid_map, Group_cache, Group_log_state, and Owned_groups.

    We will generate sets of groups in *stages*.  Each stage is
    divided into a number of *sub-stages* (the number of substages is
    taken uniformly at random from the set 1, 2, ..., 200).  In each
    sub-stage, we randomly sample one sub-group from a fixed set of
    groups.  The fixed set of groups consists of groups from 16
    different SIDs.  For the Nth SID (1 <= N <= 16), the fixed set of
    groups contains all GNOS from the closed interval [N, N - 1 + N *
    N].  The stage consists of the set of groups from all the
    sub-stages.
  */

#define BEGIN_SUBSTAGE_LOOP(group_test, stage, do_errtext)                    \
  (group_test)->push_errtext();                                               \
  for (int substage_i = 0; substage_i < (stage)->n_substages; substage_i++) { \
    Substage &substage = (stage)->substages[substage_i];                      \
    if (do_errtext)                                                           \
      (group_test)                                                            \
          ->append_errtext(__LINE__, "sidno=%d group=%s substage_i=%d",       \
                           substage.sidno, substage.gtid_str, substage_i);
#define END_SUBSTAGE_LOOP(group_test) \
  }                                   \
  group_test->pop_errtext()

  /**
    A substage, i.e., one of the randomly generated groups.
  */
  struct Substage {
    rpl_sidno sidno;
    rpl_gno gno;
    const rpl_sid *sid;
    char sid_str[binary_log::Uuid::TEXT_LENGTH + 1];
    char gtid_str[binary_log::Uuid::TEXT_LENGTH + 1 + MAX_GNO_TEXT_LENGTH + 1];
    bool is_first, is_last, is_auto;
#ifndef NO_DBUG
    void print() const {
      printf("%d/%s [first=%d last=%d auto=%d]", sidno, gtid_str, is_first,
             is_last, is_auto);
    }
#endif
  };

  /**
    A stage, i.e., the sequence of randomly generated groups.
  */
  struct Stage {
    class GroupTest *group_test;
    Sid_map *sid_map;

    // List of groups added in the present stage.
    static const int MAX_SUBSTAGES = 200;
    Substage substages[MAX_SUBSTAGES];
    int n_substages;

    // Set of groups added in the present stage.
    Gtid_set set;
    int str_len;
    char *str;

    // The subset of groups that can be added as automatic groups.
    Gtid_set automatic_groups;
    // The subset of groups that cannot be added as automatic groups.
    Gtid_set non_automatic_groups;

    Stage(class GroupTest *gt, Sid_map *sm)
        : group_test(gt),
          sid_map(sm),
          set(sm),
          str_len(0),
          str(nullptr),
          automatic_groups(sm),
          non_automatic_groups(sm) {
      init(sm);
    }

    void init(Sid_map *sm) {
      rpl_sidno max_sidno = sm->get_max_sidno();
      ASSERT_OK(set.ensure_sidno(max_sidno));
      ASSERT_OK(automatic_groups.ensure_sidno(max_sidno));
      ASSERT_OK(non_automatic_groups.ensure_sidno(max_sidno));
    }

    ~Stage() { free(str); }

    void print() const {
      printf("%d substages = {\n", n_substages);
      for (int i = 0; i < n_substages; i++) {
        printf("  substage[%d]: ", i);
        substages[i].print();
        printf("\n");
      }
      printf("\n");
    }

    /**
      Generate the the random groups that constitute a stage.

      @param done_groups The set of all groups added in previous
      stages.
      @param other_sm Sid_map to which groups should be added.
    */
    void new_stage(const Gtid_set *done_groups, Sid_map *other_sm) {
      set.clear();
      automatic_groups.clear();
      non_automatic_groups.clear();

      n_substages = 1 + (rand() % MAX_SUBSTAGES);
      BEGIN_SUBSTAGE_LOOP(group_test, this, false) {
        // generate random GTID
        substage.sidno = 1 + (rand() % N_SIDS);
        substage.gno = 1 + (rand() % (substage.sidno * substage.sidno));
        // compute alternative forms
        substage.sid = sid_map->sidno_to_sid(substage.sidno);
        ASSERT_NE((rpl_sid *)nullptr, substage.sid) << group_test->errtext;
        substage.sid->to_string(substage.sid_str);
        substage.sid->to_string(substage.gtid_str);
        substage.gtid_str[rpl_sid.TEXT_LENGTH] = ':';
        format_gno(substage.gtid_str + rpl_sid.TEXT_LENGTH + 1, substage.gno);

        ASSERT_LE(1, other_sm->add_permanent(substage.sid))
            << group_test->errtext;

        // check if this group could be added as an 'automatic' group
        Gtid_set::Const_interval_iterator ivit(done_groups, substage.sidno);
        const Gtid_set::Interval *iv = ivit.get();
        substage.is_auto =
            !set.contains_group(substage.sidno, substage.gno) &&
            ((iv == nullptr || iv->start > 1) ? substage.gno == 1
                                              : substage.gno == iv->end);

        // check if this sub-group is the first in its group in this
        // stage, and add it to the set
        substage.is_first = !set.contains_group(substage.sidno, substage.gno);
        if (substage.is_first) ASSERT_OK(set.add(substage.gtid_str));
      }
      END_SUBSTAGE_LOOP(group_test);

      // Iterate backwards so that we can detect when a subgroup is
      // the last subgroup of its group.
      set.clear();
      for (int substage_i = n_substages - 1; substage_i >= 0; substage_i--) {
        Substage &substage = substages[substage_i];
        substage.is_last = !set.contains_group(substage.sidno, substage.gno);
        if (substage.is_last) ASSERT_OK(set.add(substage.gtid_str));
      }

      str_len = set.get_string_length();
      str = (char *)realloc(str, str_len + 1);
      set.to_string(str);
    }
  };

  /*
    We maintain a text that contains the state of the test.  We print
    this text when a test assertion fails.  The text is updated each
    iteration of each loop, so that we can easier track the exact
    point in time when an error occurs.  Since loops may be nested, we
    maintain a stack of offsets in the error text: before a new loop
    is entered, the position of the end of the string is pushed to the
    stack and the text appended in each iteration is added to that
    position.
  */
  char errtext[1000];
  int errtext_stack[100];
  int errtext_stack_pos;
  bool verbose;

  void append_errtext(int line, const char *fmt, ...)
      MY_ATTRIBUTE((format(printf, 3, 4))) {
    va_list argp;
    va_start(argp, fmt);
    vsprintf(errtext + errtext_stack[errtext_stack_pos], fmt, argp);
    if (verbose) printf("@line %d: %s\n", line, errtext);
    va_end(argp);
  }

  void push_errtext() {
    int old_len = errtext_stack[errtext_stack_pos];
    int len = old_len + strlen(errtext + old_len);
    strcpy(errtext + len, " | ");
    errtext_stack[++errtext_stack_pos] = len + 3;
  }

  void pop_errtext() { errtext[errtext_stack[errtext_stack_pos--] - 3] = 0; }

  void group_subset(Gtid_set *sub, Gtid_set *super, bool outcome, int line,
                    const char *desc) {
    append_errtext(line, "%s", desc);
    // check using is_subset
    EXPECT_EQ(outcome, sub->is_subset(super)) << errtext;
    // check using set subtraction
    enum_return_status status;
    Gtid_set sub_minus_super(sub, &status);
    ASSERT_OK(status) << errtext;
    ASSERT_OK(sub_minus_super.remove(super)) << errtext;
    ASSERT_EQ(outcome, sub_minus_super.is_empty()) << errtext;
  }
};

const char *GroupTest::uuids[16] = {
    "00000000-0000-0000-0000-000000000000",
    "11111111-1111-1111-1111-111111111111",
    "22222222-2222-2222-2222-222222222222",
    "33333333-3333-3333-3333-333333333333",
    "44444444-4444-4444-4444-444444444444",
    "55555555-5555-5555-5555-555555555555",
    "66666666-6666-6666-6666-666666666666",
    "77777777-7777-7777-7777-777777777777",
    "88888888-8888-8888-8888-888888888888",
    "99999999-9999-9999-9999-999999999999",
    "aaaaAAAA-aaaa-AAAA-aaaa-aAaAaAaAaaaa",
    "bbbbBBBB-bbbb-BBBB-bbbb-bBbBbBbBbbbb",
    "ccccCCCC-cccc-CCCC-cccc-cCcCcCcCcccc",
    "ddddDDDD-dddd-DDDD-dddd-dDdDdDdDdddd",
    "eeeeEEEE-eeee-EEEE-eeee-eEeEeEeEeeee",
    "ffffFFFF-ffff-FFFF-ffff-fFfFfFfFffff",
};

TEST_F(GroupTest, Uuid) {
  Uuid u;
  char buf[100];

  // check that we get back the same UUID after parse + print
  for (int i = 0; i < N_SIDS; i++) {
    EXPECT_OK(u.parse(uuids[i])) << "i=" << i;
    u.to_string(buf);
    EXPECT_STRCASEEQ(uuids[i], buf) << "i=" << i;
  }
  // check error cases
  EXPECT_OK(u.parse("ffffFFFF-ffff-FFFF-ffff-ffffffffFFFFf"));
  EXPECT_NOK(u.parse("ffffFFFF-ffff-FFFF-ffff-ffffffffFFFg"));
  EXPECT_NOK(u.parse("ffffFFFF-ffff-FFFF-ffff-ffffffffFFF"));
  EXPECT_NOK(u.parse("ffffFFFF-ffff-FFFF-fff-fffffffffFFFF"));
  EXPECT_NOK(u.parse("ffffFFFF-ffff-FFFF-ffff-ffffffffFFF-"));
  EXPECT_NOK(u.parse(" ffffFFFF-ffff-FFFF-ffff-ffffffffFFFF"));
  EXPECT_NOK(u.parse("ffffFFFFfffff-FFFF-ffff-ffffffffFFFF"));
}

TEST_F(GroupTest, Sid_map) {
  Checkable_rwlock lock;
  Sid_map sm(&lock);

  lock.rdlock();
  ASSERT_OK(sm.open("sid-map-0"));

  // Add a random SID until we have N_SID SIDs in the map.
  while (sm.get_max_sidno() < N_SIDS)
    ASSERT_LE(1, sm.add_permanent(&sids[rand() % N_SIDS])) << errtext;

  // Check that all N_SID SIDs are in the map, and that
  // get_sorted_sidno() has the correct order.  This implies that no
  // SID was added twice.
  for (int i = 0; i < N_SIDS; i++) {
    rpl_sidno sidno = sm.get_sorted_sidno(i);
    const rpl_sid *sid;
    char buf[100];
    EXPECT_NE((rpl_sid *)nullptr, sid = sm.sidno_to_sid(sidno)) << errtext;
    const int max_len = binary_log::Uuid::TEXT_LENGTH;
    EXPECT_EQ(max_len, sid->to_string(buf)) << errtext;
    EXPECT_STRCASEEQ(uuids[i], buf) << errtext;
    EXPECT_EQ(sidno, sm.sid_to_sidno(sid)) << errtext;
  }
  lock.unlock();
  lock.assert_no_lock();
}

TEST_F(GroupTest, Group_containers) {
  /*
    In this test, we maintain 298 Gtid_sets.  We add groups to these
    Gtid_sets in stages, as described above.  We add the groups to
    each of the 298 Gtid_sets in different ways, as described below.
    At the end of each stage, we check that all the 298 resulting
    Gtid_sets are mutually equal.

    We add groups in the two ways:

    A. Test Gtid_sets and Sid_maps.  We vary two parameters:

       Parameter 1: vary the way that groups are added:
        0. Add one group at a time, using add(sidno, gno).
        1. Add one group at a time, using add(text).
        2. Add all new groups at once, using add(gs_new).
        3. add all new groups at once, using add(gs_new.to_string()).
        4. Maintain a string that contains the concatenation of all
           gs_new.to_string(). in each stage, we set gs[4] to a new
           Gtid_set created from this string.

       Parameter 2: vary the Sid_map object:
        0. Use a Sid_map that has all the SIDs in order.
        1. Use a Sid_map where SIDs are added in the order they appear.

       We vary these parameters in all combinations; thus we construct
       10 Gtid_sets.
  */
  enum enum_sets_method {
    METHOD_SIDNO_GNO = 0,
    METHOD_GROUP_TEXT,
    METHOD_GTID_SET,
    METHOD_GTID_SET_TEXT,
    METHOD_ALL_TEXTS_CONCATENATED,
    MAX_METHOD
  };
  enum enum_sets_sid_map { SID_MAP_0 = 0, SID_MAP_1, MAX_SID_MAP };
  const int N_COMBINATIONS_SETS = MAX_METHOD * MAX_SID_MAP;
  /*
    B. Test Group_cache, Group_log_state, and Owned_groups.  All
       sub-groups for the stage are added to the Group_cache, the
       Group_cache is flushed to the Group_log_state, and the
       Gtid_set is extracted from the Group_log_state.  We vary the
       following parameters.

       Parameter 1: type of statement:
        0. Transactional replayed statement: add all groups to the
           transaction group cache (which is flushed to a
           Group_log_state at the end of the stage).  Set
           GTID_NEXT_LIST to the list of all groups in the stage.
        1. Non-transactional replayed statement: add all groups to the
           stmt group cache (which is flushed to the Group_log_state
           at the end of each sub-stage).  Set GTID_NEXT_LIST = NULL.
        2. Randomize: for each sub-stage, choose 0 or 1 with 50%
           chance.  Set GTID_NEXT_LIST to the list of all groups in
           the stage.
        3. Automatic groups: add all groups to the stmt group cache,
           but make the group automatic if possible, i.e., if the SID
           and GNO are unlogged and there is no smaller unlogged GNO
           for this SID.  Set GTID_NEXT_LIST = NULL.

       Parameter 2: ended or non-ended sub-groups:
        0. All sub-groups are unended (except automatic sub-groups).
        1. For each group, the last sub-group of the group in the
           stage is ended.  Don't add groups that are already ended in the
           Group_log_state.
        2. For each group in the stage, choose 0 or 1 with 50% chance.

       Parameter 3: empty or normal sub-group:
        0. Generate only normal (and possibly automatic) sub-groups.
        1. Generate only empty (and possibly automatic) sub-groups.
        2. Generate only empty (and possibly automatic) sub-groups.
           Add the sub-groups implicitly: do not call
           add_empty_subgroup(); instead rely on
           gtid_before_flush_trx_cache() to add empty subgroups.
        3. Choose 0 or 1 with 33% chance.

       Parameter 4: insert anonymous sub-groups or not:
        0. Do not generate anonymous sub-groups.
        1. Generate an anomous sub-group before each sub-group with
           50% chance and an anonymous group after each sub-group with
           50% chance.

       We vary these parameters in all combinations; thus we construct
       4*3*4*2=96 Gtid_sets.
  */
  enum enum_caches_type {
    TYPE_TRX = 0,
    TYPE_NONTRX,
    TYPE_RANDOMIZE,
    TYPE_AUTO,
    MAX_TYPE
  };
  enum enum_caches_end { END_OFF = 0, END_ON, END_RANDOMIZE, MAX_END };
  enum enum_caches_empty {
    EMPTY_OFF = 0,
    EMPTY_ON,
    EMPTY_IMPLICIT,
    EMPTY_RANDOMIZE,
    MAX_EMPTY
  };
  enum enum_caches_anon { ANON_OFF = 0, ANON_ON, MAX_ANON };
  const int N_COMBINATIONS_CACHES = MAX_TYPE * MAX_END * MAX_EMPTY * MAX_ANON;
  const int N_COMBINATIONS = N_COMBINATIONS_SETS + N_COMBINATIONS_CACHES;

  // Auxiliary macros to loop through all combinations of parameters.
#define BEGIN_LOOP_A                                                        \
  push_errtext();                                                           \
  for (int method_i = 0, combination_i = 0; method_i < MAX_METHOD;          \
       method_i++) {                                                        \
    for (int sid_map_i = 0; sid_map_i < MAX_SID_MAP;                        \
         sid_map_i++, combination_i++) {                                    \
      Gtid_set &gtid_set MY_ATTRIBUTE((unused)) =                           \
          containers[combination_i]->gtid_set;                              \
      Sid_map *&sid_map MY_ATTRIBUTE((unused)) = sid_maps[sid_map_i];       \
      append_errtext(__LINE__, "sid_map_i=%d method_i=%d combination_i=%d", \
                     sid_map_i, method_i, combination_i);

#define END_LOOP_A \
  }                \
  }                \
  pop_errtext()

#define BEGIN_LOOP_B                                                           \
  push_errtext();                                                              \
  for (int type_i = 0, combination_i = N_COMBINATIONS_SETS; type_i < MAX_TYPE; \
       type_i++) {                                                             \
    for (int end_i = 0; end_i < MAX_END; end_i++) {                            \
      for (int empty_i = 0; empty_i < MAX_EMPTY; empty_i++) {                  \
        for (int anon_i = 0; anon_i < MAX_ANON; anon_i++, combination_i++) {   \
          Gtid_set &gtid_set MY_ATTRIBUTE((unused)) =                          \
              containers[combination_i]->gtid_set;                             \
          Group_cache &stmt_cache MY_ATTRIBUTE((unused)) =                     \
              containers[combination_i]->stmt_cache;                           \
          Group_cache &trx_cache MY_ATTRIBUTE((unused)) =                      \
              containers[combination_i]->trx_cache;                            \
          Group_log_state &group_log_state MY_ATTRIBUTE((unused)) =            \
              containers[combination_i]->group_log_state;                      \
          append_errtext(__LINE__,                                             \
                         "type_i=%d end_i=%d empty_i=%d "                      \
                         "anon_i=%d combination_i=%d",                         \
                         type_i, end_i, empty_i, anon_i, combination_i);       \
  // verbose= (combination_i == 108); /*todo*/

#define END_LOOP_B \
  }                \
  }                \
  }                \
  }                \
  pop_errtext()

  // Do not generate warnings (because that causes segfault when done
  // from a unittest).
  global_system_variables.log_error_verbosity = 1;

  mysql_bin_log.server_uuid_sidno = 1;

  // Create Sid_maps.
  Checkable_rwlock &lock = mysql_bin_log.sid_lock;
  Sid_map **sid_maps = new Sid_map *[2];
  sid_maps[0] = &mysql_bin_log.sid_map;
  sid_maps[1] = new Sid_map(&lock);

  lock.rdlock();
  ASSERT_OK(sid_maps[0]->open("sid-map-1"));
  ASSERT_OK(sid_maps[1]->open("sid-map-2"));
  /*
    Make sid_maps[0] and sid_maps[1] different: sid_maps[0] is
    generated in order; sid_maps[1] is generated in the order that
    SIDS are inserted in the Gtid_set.
  */
  for (int i = 0; i < N_SIDS; i++)
    ASSERT_LE(1, sid_maps[0]->add_permanent(&sids[i])) << errtext;

  // Create list of container objects.  These are the objects that we
  // test.
  struct Containers {
    Gtid_set gtid_set;
    Group_cache stmt_cache;
    Group_cache trx_cache;
    Group_log_state group_log_state;
    Containers(Checkable_rwlock *lock, Sid_map *sm)
        : gtid_set(sm), group_log_state(lock, sm) {
      init();
    }
    void init() { ASSERT_OK(group_log_state.ensure_sidno()); };
  };
  Containers **containers = new Containers *[N_COMBINATIONS];
  BEGIN_LOOP_A { containers[combination_i] = new Containers(&lock, sid_map); }
  END_LOOP_A;
  BEGIN_LOOP_B {
    containers[combination_i] = new Containers(&lock, sid_maps[0]);
  }
  END_LOOP_B;

  /*
    Construct a Gtid_set that contains the set of all groups from
    which we sample.
  */
  static char all_groups_str[100 * 100];
  char *s = all_groups_str;
  s += sprintf(s, "%s:1", uuids[0]);
  for (rpl_sidno sidno = 2; sidno <= N_SIDS; sidno++)
    s += sprintf(s, ",\n%s:1-%d", uuids[sidno - 1], sidno * sidno);
  enum_return_status status;
  Gtid_set all_groups(sid_maps[0], all_groups_str, &status);
  ASSERT_OK(status) << errtext;

  // The set of groups that were added in some previous stage.
  Gtid_set done_groups(sid_maps[0]);
  ASSERT_OK(done_groups.ensure_sidno(sid_maps[0]->get_max_sidno()));

  /*
    Iterate through stages. In each stage, create the "stage group
    set" by generating up to 200 subgroups.  Add this stage group set
    to each of the group sets in different ways.  Stop when the union
    of all stage group sets is equal to the full set from which we
    took the samples.
  */
  char *done_str = nullptr;
  int done_str_len = 0;
  Stage stage(this, sid_maps[0]);
  int stage_i = 0;

  /*
    We need a THD object only to read THD::variables.gtid_next,
    THD::variables.gtid_end, THD::variables.gtid_next_list,
    THD::thread_id, THD::server_status.  We don't want to invoke the
    THD constructor because that would require setting up mutexes,
    etc.  Hence we use malloc instead of new.
  */
  THD *thd = (THD *)malloc(sizeof(THD));
  ASSERT_NE((THD *)nullptr, thd) << errtext;
  Gtid_specification *gtid_next = &thd->variables.gtid_next;
  thd->set_new_thread_id();
  gtid_next->type = Gtid_specification::AUTOMATIC;
  bool &gtid_end = thd->variables.gtid_end;
  bool &gtid_commit = thd->variables.gtid_commit;
  thd->server_status = 0;
  thd->system_thread = NON_SYSTEM_THREAD;
  thd->variables.gtid_next_list.gtid_set = &stage.set;

  push_errtext();
  while (!all_groups.equals(&done_groups)) {
    stage_i++;
    append_errtext(__LINE__, "stage_i=%d", stage_i);
    stage.new_stage(&done_groups, sid_maps[1]);

    if (verbose) {
      printf("======== stage %d ========\n", stage_i);
      stage.print();
    }

    // Create a string that contains all previous stage.str,
    // concatenated.
    done_str = (char *)realloc(done_str, done_str_len + 1 + stage.str_len + 1);
    ASSERT_NE((char *)nullptr, done_str) << errtext;
    done_str_len += sprintf(done_str + done_str_len, ",%s", stage.str);

    // Add groups to Gtid_sets.
    BEGIN_LOOP_A {
      switch (method_i) {
        case METHOD_SIDNO_GNO:
          BEGIN_SUBSTAGE_LOOP(this, &stage, true) {
            rpl_sidno sidno_1 = sid_map->sid_to_sidno(substage.sid);
            ASSERT_LE(1, sidno_1) << errtext;
            ASSERT_OK(gtid_set.ensure_sidno(sidno_1));
            ASSERT_OK(gtid_set._add(sidno_1, substage.gno));
          }
          END_SUBSTAGE_LOOP(this);
          break;
        case METHOD_GROUP_TEXT:
          BEGIN_SUBSTAGE_LOOP(this, &stage, true) {
            ASSERT_OK(gtid_set.add(substage.gtid_str));
          }
          END_SUBSTAGE_LOOP(this);
          break;
        case METHOD_GTID_SET:
          ASSERT_OK(gtid_set.add(&stage.set)) << errtext;
          break;
        case METHOD_GTID_SET_TEXT:
          ASSERT_OK(gtid_set.add(stage.str)) << errtext;
          break;
        case METHOD_ALL_TEXTS_CONCATENATED:
          gtid_set.clear();
          ASSERT_OK(gtid_set.add(done_str)) << errtext;
        case MAX_METHOD:
          break;
      }
    }
    END_LOOP_A;

    // Add groups to Group_caches.
    BEGIN_LOOP_B {
      if (verbose) {
        printf("======== stage=%d combination=%d ========\n", stage_i,
               combination_i);
#ifndef DBUG_OFF
        printf("group log state:\n");
        group_log_state.print();
        printf("trx cache:\n");
        trx_cache.print(sid_maps[0]);
        printf("stmt cache:\n");
        stmt_cache.print(sid_maps[0]);
#endif  // ifdef DBUG_OFF
      }

      Gtid_set ended_groups(sid_maps[0]);
      bool trx_contains_logged_subgroup = false;
      bool stmt_contains_logged_subgroup = false;
      BEGIN_SUBSTAGE_LOOP(this, &stage, true) {
        int type_j;
        if (type_i == TYPE_RANDOMIZE)
          type_j = rand() % 2;
        else if (type_i == TYPE_AUTO && !substage.is_auto)
          type_j = TYPE_NONTRX;
        else
          type_j = type_i;
        int end_j;
        if (substage.is_first &&
            ((end_i == END_RANDOMIZE && (rand() % 2)) || end_i == END_ON)) {
          ASSERT_OK(ended_groups.ensure_sidno(substage.sidno));
          ASSERT_OK(ended_groups._add(substage.sidno, substage.gno));
        }
        end_j = substage.is_last &&
                ended_groups.contains_group(substage.sidno, substage.gno);

        /*
          In EMPTY_RANDOMIZE mode, we have to determine once *per
          group* (not substage) if we use EMPTY_END or not. So we
          determine this for the first subgroup of the group, and then
          we memoize which groups use EMPTY_END using the Gtid_set
          empty_end.
        */
        int empty_j;
        if (empty_i == EMPTY_RANDOMIZE)
          empty_j = rand() % 3;
        else
          empty_j = empty_i;
        int anon_j1, anon_j2;
        if (type_j != TYPE_TRX || anon_i == ANON_OFF)
          anon_j1 = anon_j2 = ANON_OFF;
        else {
          anon_j1 = rand() % 2;
          anon_j2 = rand() % 2;
        }
        if (verbose)
          printf("type_j=%d end_j=%d empty_j=%d anon_j1=%d anon_j2=%d\n",
                 type_j, end_j, empty_j, anon_j1, anon_j2);

        thd->variables.gtid_next_list.is_non_null =
            (type_i == TYPE_NONTRX || type_i == TYPE_AUTO) ? 0 : 1;
        gtid_commit = (substage_i == stage.n_substages - 1) ||
                      !thd->variables.gtid_next_list.is_non_null;

        if (type_j == TYPE_AUTO) {
          gtid_next->type = Gtid_specification::AUTOMATIC;
          gtid_next->group.sidno = substage.sidno;
          gtid_next->group.gno = 0;
          gtid_end = false;
          lock.unlock();
          lock.assert_no_lock();
          gtid_before_statement(thd, &lock, &group_log_state, &stmt_cache,
                                &trx_cache);
          lock.rdlock();
          stmt_cache.add_logged_subgroup(thd, 20 + rand() % 100 /*binlog_len*/);
          stmt_contains_logged_subgroup = true;
        } else {
          Group_cache &cache = type_j == TYPE_TRX ? trx_cache : stmt_cache;

          if (anon_j1) {
            gtid_next->type = Gtid_specification::ANONYMOUS;
            gtid_next->group.sidno = 0;
            gtid_next->group.gno = 0;
            gtid_end = false;
            lock.unlock();
            lock.assert_no_lock();
            gtid_before_statement(thd, &lock, &group_log_state, &stmt_cache,
                                  &trx_cache);
            lock.rdlock();
            cache.add_logged_subgroup(thd, 20 + rand() % 100 /*binlog_len*/);
            trx_contains_logged_subgroup = true;
          }

          gtid_next->type = Gtid_specification::GTID;
          gtid_next->group.sidno = substage.sidno;
          gtid_next->group.gno = substage.gno;
          gtid_end = (end_j == END_ON) ? true : false;
          lock.unlock();
          lock.assert_no_lock();
          gtid_before_statement(thd, &lock, &group_log_state, &stmt_cache,
                                &trx_cache);
          lock.rdlock();
          if (!group_log_state.is_ended(substage.sidno, substage.gno)) {
            switch (empty_j) {
              case EMPTY_OFF:
                cache.add_logged_subgroup(thd,
                                          20 + rand() % 100 /*binlog_len*/);
                if (type_j == TYPE_TRX)
                  trx_contains_logged_subgroup = true;
                else
                  stmt_contains_logged_subgroup = true;
                break;
              case EMPTY_ON:
                cache.add_empty_subgroup(substage.sidno, substage.gno,
                                         end_j ? true : false);
                break;
              case EMPTY_IMPLICIT:
                break;  // do nothing
              default:
                assert(0);
            }
          }

          if (anon_j2) {
            gtid_next->type = Gtid_specification::ANONYMOUS;
            gtid_next->group.sidno = 0;
            gtid_next->group.gno = 0;
            gtid_end = false;
            lock.unlock();
            lock.assert_no_lock();
            gtid_before_statement(thd, &lock, &group_log_state, &stmt_cache,
                                  &trx_cache);
            lock.rdlock();
            cache.add_logged_subgroup(thd, 20 + rand() % 100 /*binlog_len*/);
            trx_contains_logged_subgroup = true;
          }
        }

#ifndef DBUG_OFF
        if (verbose) {
          printf("stmt_cache:\n");
          stmt_cache.print(sid_maps[0]);
        }
#endif  // ifndef DBUG_OFF
        if (!stmt_cache.is_empty())
          gtid_flush_group_cache(
              thd, &lock, &group_log_state, nullptr /*group log*/, &stmt_cache,
              &trx_cache, 1 /*binlog_no*/, 1 /*binlog_pos*/,
              stmt_contains_logged_subgroup ? 20 + rand() % 99 : -1
              /*offset_after_last_statement*/);
        stmt_contains_logged_subgroup = false;
        gtid_before_flush_trx_cache(thd, &lock, &group_log_state, &trx_cache);
        if (gtid_commit) {
          // simulate gtid_after_flush_trx_cache() but don't
          // execute a COMMIT statement
          thd->variables.gtid_has_ongoing_super_group = 0;

#ifndef DBUG_OFF
          if (verbose) {
            printf("trx_cache:\n");
            trx_cache.print(sid_maps[0]);
            printf(
                "trx_cache.is_empty=%d n_subgroups=%d "
                "trx_contains_logged_subgroup=%d\n",
                trx_cache.is_empty(), trx_cache.get_n_subgroups(),
                trx_contains_logged_subgroup);
          }
#endif  // ifndef DBUG_OFF

          if (!trx_cache.is_empty())
            gtid_flush_group_cache(
                thd, &lock, &group_log_state, nullptr /*group log*/, &trx_cache,
                &trx_cache, 1 /*binlog_no*/, 1 /*binlog_pos*/,
                trx_contains_logged_subgroup ? 20 + rand() % 99 : -1
                /*offset_after_last_statement*/);
          trx_contains_logged_subgroup = false;
        }
      }
      END_SUBSTAGE_LOOP(this);

      gtid_set.clear();
      ASSERT_OK(group_log_state.owned_groups.get_partial_groups(&gtid_set));
      ASSERT_OK(gtid_set.add(&group_log_state.ended_groups));
    }
    END_LOOP_B;

    // add stage.set to done_groups
    Gtid_set old_done_groups(&done_groups, &status);
    ASSERT_OK(status);
    ASSERT_OK(done_groups.add(&stage.set));

    // check the Gtid_set::remove and Gtid_set::is_subset functions
    Gtid_set diff(&done_groups, &status);
    ASSERT_OK(status);
    ASSERT_OK(diff.remove(&old_done_groups));
    Gtid_set not_new(&stage.set, &status);
    ASSERT_OK(status);
    ASSERT_OK(not_new.remove(&diff));

#define GROUP_SUBSET(gs1, gs2, outcome) \
  group_subset(&gs1, &gs2, outcome, __LINE__, #gs1 " <= " #gs2);
    push_errtext();
    GROUP_SUBSET(not_new, not_new, true);
    GROUP_SUBSET(not_new, diff, not_new.is_empty());
    GROUP_SUBSET(not_new, stage.set, true);
    GROUP_SUBSET(not_new, done_groups, true);
    GROUP_SUBSET(not_new, old_done_groups, true);

    GROUP_SUBSET(diff, not_new, diff.is_empty());
    GROUP_SUBSET(diff, diff, true);
    GROUP_SUBSET(diff, stage.set, true);
    GROUP_SUBSET(diff, done_groups, true);
    GROUP_SUBSET(diff, old_done_groups, diff.is_empty());

    GROUP_SUBSET(stage.set, not_new, diff.is_empty());
    GROUP_SUBSET(stage.set, diff, not_new.is_empty());
    GROUP_SUBSET(stage.set, stage.set, true);
    GROUP_SUBSET(stage.set, done_groups, true);
    GROUP_SUBSET(stage.set, old_done_groups, diff.is_empty());

    // GROUP_SUBSET(done_groups, not_new, ???);
    GROUP_SUBSET(done_groups, diff, old_done_groups.is_empty());
    GROUP_SUBSET(done_groups, stage.set, done_groups.equals(&stage.set));
    GROUP_SUBSET(done_groups, done_groups, true);
    GROUP_SUBSET(done_groups, old_done_groups, diff.is_empty());

    GROUP_SUBSET(old_done_groups, not_new, old_done_groups.equals(&not_new));
    GROUP_SUBSET(old_done_groups, diff, old_done_groups.is_empty());
    // GROUP_SUBSET(old_done_groups, stage.set, ???);
    GROUP_SUBSET(old_done_groups, done_groups, true);
    GROUP_SUBSET(old_done_groups, old_done_groups, true);
    pop_errtext();

    /*
      Verify that all group sets are equal.  We test both a.equals(b)
      and b.equals(a) and a.equals(a), because we want to verify that
      Gtid_set::equals is correct too.  We compare both the sets
      using Gtid_set::equals, and the output of to_string() using
      EXPECT_STREQ.
    */
    BEGIN_LOOP_A {
      char *buf1 = new char[gtid_set.get_string_length() + 1];
      gtid_set.to_string(buf1);
      for (int i = 0; i < N_COMBINATIONS_SETS; i++) {
        Gtid_set &gtid_set_2 = containers[i]->gtid_set;
        if (combination_i < i) {
          char *buf2 = new char[gtid_set_2.get_string_length() + 1];
          gtid_set_2.to_string(buf2);
          EXPECT_STREQ(buf1, buf2) << errtext << " i=" << i;
          delete buf2;
        }
        EXPECT_EQ(true, gtid_set.equals(&gtid_set_2)) << errtext << " i=" << i;
      }
      delete buf1;
    }
    END_LOOP_A;
    BEGIN_LOOP_B {
      EXPECT_EQ(true, containers[combination_i]->gtid_set.equals(&done_groups))
          << errtext;
    }
    END_LOOP_B;
  }
  pop_errtext();

  // Finally, verify that the string representations of
  // done_groups is as expected
  static char buf[100 * 100];
  done_groups.to_string(buf);
  EXPECT_STRCASEEQ(all_groups_str, buf) << errtext;
  lock.unlock();
  lock.assert_no_lock();

  // Clean up.
  free(done_str);
  for (int i = 0; i < N_COMBINATIONS; i++) delete containers[i];
  delete containers;
  delete sid_maps[1];
  delete sid_maps;
  free(thd);

  mysql_bin_log.sid_lock.assert_no_lock();
}

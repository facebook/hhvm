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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA  */

/**
   @file
   Unit test of the Optimizer trace API (WL#5257)
*/

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>
#include <sys/types.h>

#include "my_inttypes.h"
#include "my_macros.h"

#include "m_string.h"    // llstr
#include "mysys_err.h"   // for testing of OOM
#include "sql/mysqld.h"  // system_charset_info
#include "sql/opt_trace.h"
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>  // for WEXITSTATUS
#endif

namespace opt_trace_unittest {

const ulonglong all_features = Opt_trace_context::default_features;

/**
   @note It is a macro, for proper reporting of line numbers in case of
   assertion failure. SCOPED_TRACE will report line number at the
   macro expansion site.
*/
#define check_json_compliance(str, length) \
  {                                        \
    SCOPED_TRACE("");                      \
    do_check_json_compliance(str, length); \
  }

/**
   Checks compliance of a trace with JSON syntax rules.
   This is a helper which has interest only when developing the test; once you
   know that the produced trace is compliant and has expected content, just
   set "expected" to it, add a comparison with "expected", and don't use this
   function.
   @param  str     pointer to trace
   @param  length  trace's length
*/
static void do_check_json_compliance(const char *str, size_t length) {
  return;
  /*
    Read from stdin, eliminate comments, parse as JSON. If invalid, an
    exception is thrown by Python, uncaught, which produces a non-zero error
    code.
  */
#ifndef _WIN32
  const char python_cmd[] =
      "python -c \""
      "import json, re, sys;"
      "s= sys.stdin.read();"
      "s= re.sub('/\\\\*[ A-Za-z_]* \\\\*/', '', s);"
      "json.loads(s, 'utf-8')\"";
  // Send the trace to this new process' stdin:
  FILE *fd = popen(python_cmd, "w");
  ASSERT_TRUE(nullptr != fd);
  ASSERT_NE(0U, length);  // empty is not compliant
  ASSERT_EQ(1U, fwrite(str, length, 1, fd));
  int rc = pclose(fd);
  rc = WEXITSTATUS(rc);
  EXPECT_EQ(0, rc);
#endif
}

extern "C" void my_error_handler(uint error, const char *str, myf MyFlags);

class TraceContentTest : public ::testing::Test {
 public:
  Opt_trace_context trace;
  static bool oom;  ///< whether we got an OOM error from opt trace
 protected:
  static void SetUpTestCase() {
    system_charset_info = &my_charset_utf8_general_ci;
  }
  virtual void SetUp() {
    /* Save original and install our custom error hook. */
    m_old_error_handler_hook = error_handler_hook;
    error_handler_hook = my_error_handler;
    oom = false;
    // Setting debug flags triggers enter/exit trace, so redirect to /dev/null
    DBUG_SET("o," IF_WIN("NUL", "/dev/null"));
  }
  virtual void TearDown() { error_handler_hook = m_old_error_handler_hook; }

  static void (*m_old_error_handler_hook)(uint, const char *, myf);
};
bool TraceContentTest::oom;
void (*TraceContentTest::m_old_error_handler_hook)(uint, const char *, myf);

void my_error_handler(uint error, const char *, myf) {
  const uint EE = static_cast<uint>(EE_OUTOFMEMORY);
  EXPECT_EQ(EE, error);
  if (error == EE) TraceContentTest::oom = true;
}

TEST_F(TraceContentTest, ConstructAndDestruct) {}

/** Test empty trace */
TEST_F(TraceContentTest, Empty) {
  ASSERT_FALSE(
      trace.start(true, false, false, false, -1, 1, ULONG_MAX, all_features));
  EXPECT_TRUE(trace.is_started());
  EXPECT_TRUE(trace.support_I_S());
  /*
    Add at least an object to it. A really empty trace ("") is not
    JSON-compliant, at least Python's JSON module raises an exception.
  */
  { Opt_trace_object oto(&trace); }
  /* End trace */
  trace.end();
  /* And verify trace's content */
  Opt_trace_iterator it(&trace);
  /*
    ASSERT here, because a failing EXPECT_FALSE would continue into
    it.get_value() and segfault.
  */
  ASSERT_FALSE(it.at_end());
  Opt_trace_info info;
  it.get_value(&info);
  const char expected[] = "{\n}";
  EXPECT_STREQ(expected, info.trace_ptr);
  EXPECT_EQ(sizeof(expected) - 1, info.trace_length);
  check_json_compliance(info.trace_ptr, info.trace_length);
  EXPECT_EQ(0U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  EXPECT_FALSE(oom);
  /* Should be no more traces */
  it.next();
  ASSERT_TRUE(it.at_end());
}

/** Test normal usage */
TEST_F(TraceContentTest, NormalUsage) {
  ASSERT_FALSE(
      trace.start(true, false, true, false, -1, 1, ULONG_MAX, all_features));
  {
    Opt_trace_object oto(&trace);
    oto.add_select_number(123456);
    {
      Opt_trace_array ota(&trace, "one array");
      ota.add(200.4);
      {
        Opt_trace_object oto1(&trace);
        oto1.add_alnum("one key", "one value").add("another key", 100U);
      }
      ota.add_alnum("one string element");
      ota.add(true);
      ota.add_hex(12318421343459ULL);
    }
    oto.add("yet another key", -1000LL);
    {
      Opt_trace_array ota(&trace, "another array");
      ota.add(1LL).add(2).add(3LL).add(4LL);
    }
  }
  trace.end();
  Opt_trace_iterator it(&trace);
  ASSERT_FALSE(it.at_end());
  Opt_trace_info info;
  it.get_value(&info);
  const char expected[] =
      "{\n"
      "  \"select#\": 123456,\n"
      "  \"one array\": [\n"
      "    200.4,\n"
      "    {\n"
      "      \"one key\": \"one value\",\n"
      "      \"another key\": 100\n"
      "    },\n"
      "    \"one string element\",\n"
      "    true,\n"
      "    0x0b341b20dce3\n"
      "  ] /* one array */,\n"
      "  \"yet another key\": -1000,\n"
      "  \"another array\": [\n"
      "    1,\n"
      "    2,\n"
      "    3,\n"
      "    4\n"
      "  ] /* another array */\n"
      "}";
  EXPECT_STREQ(expected, info.trace_ptr);
  EXPECT_EQ(sizeof(expected) - 1, info.trace_length);
  check_json_compliance(info.trace_ptr, info.trace_length);
  EXPECT_EQ(0U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  EXPECT_FALSE(oom);
  it.next();
  ASSERT_TRUE(it.at_end());
}

/** Test reaction to malformed JSON (object with value without key) */
TEST_F(TraceContentTest, BuggyObject) {
  ASSERT_FALSE(
      trace.start(true, false, true, false, -1, 1, ULONG_MAX, all_features));
  {
    Opt_trace_object oto(&trace);
    {
      Opt_trace_array ota(&trace, "one array");
      ota.add(200.4);
      {
        Opt_trace_object oto1(&trace);
        oto1.add_alnum("one value");    // no key, which is wrong
        oto1.add(326);                  // same
        Opt_trace_object oto2(&trace);  // same
      }
      ota.add_alnum("one string element");
      ota.add(true);
    }
    oto.add("yet another key", -1000LL);
    {
      Opt_trace_array ota(&trace, "another array");
      ota.add(1LL).add(2LL).add(3LL).add(4LL);
    }
  }
  trace.end();
  Opt_trace_iterator it(&trace);
  ASSERT_FALSE(it.at_end());
  Opt_trace_info info;
  it.get_value(&info);
  const char expected[] =
      "{\n"
      "  \"one array\": [\n"
      "    200.4,\n"
      "    {\n"
      "      \"unknown_key_1\": \"one value\",\n"
      "      \"unknown_key_2\": 326,\n"
      "      \"unknown_key_3\": {\n"
      "      }\n"
      "    },\n"
      "    \"one string element\",\n"
      "    true\n"
      "  ] /* one array */,\n"
      "  \"yet another key\": -1000,\n"
      "  \"another array\": [\n"
      "    1,\n"
      "    2,\n"
      "    3,\n"
      "    4\n"
      "  ] /* another array */\n"
      "}";
  EXPECT_STREQ(expected, info.trace_ptr);
  EXPECT_EQ(sizeof(expected) - 1, info.trace_length);
  check_json_compliance(info.trace_ptr, info.trace_length);
  EXPECT_EQ(0U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  EXPECT_FALSE(oom);
  it.next();
  ASSERT_TRUE(it.at_end());
}

/** Test reaction to malformed JSON (array with value with key) */
TEST_F(TraceContentTest, BuggyArray) {
  ASSERT_FALSE(
      trace.start(true, false, true, false, -1, 1, ULONG_MAX, all_features));
  {
    Opt_trace_object oto(&trace);
    {
      Opt_trace_array ota(&trace, "one array");
      ota.add("superfluous key", 200.4);            // key, which is wrong
      ota.add("not necessary", 326);                // same
      Opt_trace_object oto2(&trace, "not needed");  // same
    }
    oto.add("yet another key", -1000LL);
    {
      Opt_trace_array ota(&trace, "another array");
      ota.add(1LL).add(2LL).add(3LL).add(4LL);
    }
  }
  trace.end();
  Opt_trace_iterator it(&trace);
  ASSERT_FALSE(it.at_end());
  Opt_trace_info info;
  it.get_value(&info);
  const char expected[] =
      "{\n"
      "  \"one array\": [\n"
      "    200.4,\n"
      "    326,\n"
      "    {\n"
      "    } /* not needed */\n"
      "  ] /* one array */,\n"
      "  \"yet another key\": -1000,\n"
      "  \"another array\": [\n"
      "    1,\n"
      "    2,\n"
      "    3,\n"
      "    4\n"
      "  ] /* another array */\n"
      "}";
  EXPECT_STREQ(expected, info.trace_ptr);
  EXPECT_EQ(sizeof(expected) - 1, info.trace_length);
  check_json_compliance(info.trace_ptr, info.trace_length);
  EXPECT_EQ(0U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  EXPECT_FALSE(oom);
  it.next();
  ASSERT_TRUE(it.at_end());
}

/** Test Opt_trace_disable_I_S */
TEST_F(TraceContentTest, DisableISWithObject) {
  ASSERT_FALSE(
      trace.start(true, false, true, false, -1, 1, ULONG_MAX, all_features));
  {
    Opt_trace_object oto(&trace);
    {
      Opt_trace_array ota(&trace, "one array");
      ota.add(200.4);
      {
        Opt_trace_object oto1(&trace);
        oto1.add_alnum("one key", "one value").add("another key", 100LL);
        Opt_trace_disable_I_S otd(&trace, true);
        oto1.add("a third key", false);
        Opt_trace_object oto2(&trace, "a fourth key");
        oto2.add("key inside", 1LL);
        /* don't disable... but above layer is stronger */
        Opt_trace_disable_I_S otd2(&trace, false);
        oto2.add("another key inside", 5LL);
        // disabling should apply to substatements too:
        ASSERT_FALSE(trace.start(true, false, true, false, -1, 1, ULONG_MAX,
                                 all_features));
        { Opt_trace_object oto3(&trace); }
        trace.end();
      }
      ota.add_alnum("one string element");
      ota.add(true);
    }
    Opt_trace_disable_I_S otd2(&trace, false);  // don't disable
    oto.add("yet another key", -1000LL);
    {
      Opt_trace_array ota(&trace, "another array");
      ota.add(1LL).add(2LL).add(3LL).add(4LL);
    }
  }
  trace.end();
  Opt_trace_iterator it(&trace);
  ASSERT_FALSE(it.at_end());
  Opt_trace_info info;
  it.get_value(&info);
  const char expected[] =
      "{\n"
      "  \"one array\": [\n"
      "    200.4,\n"
      "    {\n"
      "      \"one key\": \"one value\",\n"
      "      \"another key\": 100\n"
      "    },\n"
      "    \"one string element\",\n"
      "    true\n"
      "  ] /* one array */,\n"
      "  \"yet another key\": -1000,\n"
      "  \"another array\": [\n"
      "    1,\n"
      "    2,\n"
      "    3,\n"
      "    4\n"
      "  ] /* another array */\n"
      "}";
  EXPECT_STREQ(expected, info.trace_ptr);
  EXPECT_EQ(sizeof(expected) - 1, info.trace_length);
  check_json_compliance(info.trace_ptr, info.trace_length);
  EXPECT_EQ(0U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  EXPECT_FALSE(oom);
  it.next();
  ASSERT_TRUE(it.at_end());
}

/** Test Opt_trace_context::disable_I_S_for_this_and_children */
TEST_F(TraceContentTest, DisableISWithCall) {
  // Test that it disables even before any start()
  trace.disable_I_S_for_this_and_children();
  ASSERT_FALSE(
      trace.start(true, false, true, false, -1, 1, ULONG_MAX, all_features));
  {
    Opt_trace_object oto(&trace);
    {
      Opt_trace_array ota(&trace, "one array");
      ota.add(200.4);
      {
        Opt_trace_object oto1(&trace);
        oto1.add_alnum("one key", "one value").add("another key", 100LL);
        oto1.add("a third key", false);
        Opt_trace_object oto2(&trace, "a fourth key");
        oto2.add("key inside", 1LL);
        // disabling should apply to substatements too:
        ASSERT_FALSE(trace.start(true, false, true, false, -1, 1, ULONG_MAX,
                                 all_features));
        { Opt_trace_object oto3(&trace); }
        trace.end();
        /* don't disable... but above layer is stronger */
        Opt_trace_disable_I_S otd2(&trace, false);
        oto2.add("another key inside", 5LL);
        // disabling should apply to substatements too:
        ASSERT_FALSE(trace.start(true, false, true, false, -1, 1, ULONG_MAX,
                                 all_features));
        { Opt_trace_object oto4(&trace); }
        trace.end();
      }
      ota.add_alnum("one string element");
      ota.add(true);
    }
    oto.add("yet another key", -1000LL);
    {
      Opt_trace_array ota(&trace, "another array");
      ota.add(1LL).add(2LL).add(3LL).add(4LL);
    }
  }
  trace.end();
  trace.restore_I_S();
  Opt_trace_iterator it(&trace);
  ASSERT_TRUE(it.at_end());
}

/** Helper for Trace_settings_test.offset */
void make_one_trace(Opt_trace_context *trace, const char *name, long offset,
                    long limit) {
  ASSERT_FALSE(trace->start(true, false, true, false, offset, limit, ULONG_MAX,
                            all_features));
  {
    Opt_trace_object oto(trace);
    oto.add(name, 0LL);
  }
  trace->end();
}

/**
   Helper for Trace_settings_test.offset

   @param  trace  The trace context.
   @param  names  A NULL-terminated array of "names".

   Checks that the list of traces is as expected.
   This macro checks that the first trace contains names[0], that the second
   trace contains names[1], etc. That the number of traces is the same as
   the number of elements in "names".

   @note It is a macro, for proper reporting of line numbers in case of
   assertion failure. SCOPED_TRACE will report line number at the
   macro expansion site.
*/
#define check(trace, names)  \
  {                          \
    SCOPED_TRACE("");        \
    do_check(&trace, names); \
  }

void do_check(Opt_trace_context *trace, const char **names) {
  Opt_trace_iterator it(trace);
  Opt_trace_info info;
  for (const char **name = names; *name != nullptr; name++) {
    ASSERT_FALSE(it.at_end());
    it.get_value(&info);
    const size_t name_len = strlen(*name);
    EXPECT_EQ(name_len + 11, info.trace_length);
    EXPECT_EQ(0, strncmp(info.trace_ptr + 5, *name, name_len));
    EXPECT_EQ(0U, info.missing_bytes);
    it.next();
  }
  ASSERT_TRUE(it.at_end());
}

/** Test offset/limit variables */
TEST_F(TraceContentTest, Offset) {
  make_one_trace(&trace, "100", -1 /* offset */, 1 /* limit */);
  const char *expected_traces0[] = {"100", nullptr};
  check(trace, expected_traces0);
  make_one_trace(&trace, "101", -1, 1);
  /* 101 should have overwritten 100 */
  const char *expected_traces1[] = {"101", nullptr};
  check(trace, expected_traces1);
  make_one_trace(&trace, "102", -1, 1);
  const char *expected_traces2[] = {"102", nullptr};
  check(trace, expected_traces2);
  trace.reset();
  const char *expected_traces_empty[] = {nullptr};
  check(trace, expected_traces_empty);
  make_one_trace(&trace, "103", -3, 2);
  make_one_trace(&trace, "104", -3, 2);
  make_one_trace(&trace, "105", -3, 2);
  make_one_trace(&trace, "106", -3, 2);
  make_one_trace(&trace, "107", -3, 2);
  make_one_trace(&trace, "108", -3, 2);
  make_one_trace(&trace, "109", -3, 2);
  const char *expected_traces3[] = {"107", "108", nullptr};
  check(trace, expected_traces3);
  trace.reset();
  check(trace, expected_traces_empty);
  make_one_trace(&trace, "110", 3, 2);
  make_one_trace(&trace, "111", 3, 2);
  make_one_trace(&trace, "112", 3, 2);
  make_one_trace(&trace, "113", 3, 2);
  make_one_trace(&trace, "114", 3, 2);
  make_one_trace(&trace, "115", 3, 2);
  make_one_trace(&trace, "116", 3, 2);
  const char *expected_traces10[] = {"113", "114", nullptr};
  check(trace, expected_traces10);
  trace.reset();
  check(trace, expected_traces_empty);
  make_one_trace(&trace, "117", 0, 1);
  make_one_trace(&trace, "118", 0, 1);
  make_one_trace(&trace, "119", 0, 1);
  const char *expected_traces17[] = {"117", nullptr};
  check(trace, expected_traces17);
  trace.reset();
  make_one_trace(&trace, "120", 0, 0);
  make_one_trace(&trace, "121", 0, 0);
  make_one_trace(&trace, "122", 0, 0);
  const char *expected_traces20[] = {nullptr};
  check(trace, expected_traces20);
  EXPECT_FALSE(oom);
}

/** Test truncation by max_mem_size */
TEST_F(TraceContentTest, MaxMemSize) {
  ASSERT_FALSE(trace.start(true, false, false, false, -1, 1,
                           1000 /* max_mem_size */, all_features));
  /* make a "long" trace */
  {
    Opt_trace_object oto(&trace);
    Opt_trace_array ota(&trace, "one array");
    for (int i = 0; i < 100; i++) {
      ota.add_alnum("make it long");
    }
  }
  trace.end();
  Opt_trace_iterator it(&trace);
  ASSERT_FALSE(it.at_end());
  Opt_trace_info info;
  it.get_value(&info);
  const char expected[] =
      "{\n"
      "  \"one array\": [\n"
      "    \"make it long\",\n"
      "    \"make it long\",\n";
  /*
    Without truncation the trace would take:
    2+17+3+1+20*100 = 2023
  */
  EXPECT_EQ(996U, info.trace_length);
  EXPECT_EQ(1027U, info.missing_bytes);  // 996+1027=2023
  EXPECT_FALSE(info.missing_priv);
  EXPECT_FALSE(oom);
  EXPECT_EQ(0, strncmp(expected, info.trace_ptr, sizeof(expected) - 1));
  it.next();
  ASSERT_TRUE(it.at_end());
}

/** Test how truncation by max_mem_size affects next traces */
TEST_F(TraceContentTest, MaxMemSize2) {
  ASSERT_FALSE(trace.start(true, false, false, false, -2, 2,
                           21 /* max_mem_size */, all_features));
  /* make a "long" trace */
  {
    Opt_trace_object oto(&trace);
    oto.add_alnum("some key1", "make it long");
  }
  trace.end();
  /* A second similar trace */
  ASSERT_FALSE(trace.start(true, false, false, false, -2, 2, 21, all_features));
  {
    Opt_trace_object oto(&trace);
    oto.add_alnum("some key2", "make it long");
  }
  trace.end();
  Opt_trace_iterator it(&trace);
  ASSERT_FALSE(it.at_end());
  Opt_trace_info info;
  it.get_value(&info);
  EXPECT_EQ(17U, info.trace_length);
  EXPECT_EQ(16U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  EXPECT_FALSE(oom);
  it.next();
  ASSERT_FALSE(it.at_end());
  it.get_value(&info);
  /* 2nd trace completely empty as first trace left no room */
  EXPECT_EQ(0U, info.trace_length);
  EXPECT_EQ(33U, info.missing_bytes);
  it.next();
  ASSERT_TRUE(it.at_end());
  /*
    3rd trace; the first one should automatically be purged, thus the 3rd
    should have a bit of room.
  */
  ASSERT_FALSE(trace.start(true, false, false, false, -2, 2, 21, all_features));
  {
    Opt_trace_object oto(&trace);
    oto.add_alnum("some key3", "make it long");
  }
  trace.end();
  Opt_trace_iterator it2(&trace);
  ASSERT_FALSE(it2.at_end());
  it2.get_value(&info);
  EXPECT_EQ(0U, info.trace_length);
  EXPECT_EQ(33U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  EXPECT_FALSE(oom);
  it2.next();
  it2.get_value(&info);
  /*
    3rd one had room. A bit less than first, because just reading the second
    with the iterator has reallocated the second from 0 to 8 bytes...
  */
  EXPECT_EQ(14U, info.trace_length);
  EXPECT_EQ(19U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  it2.next();
  ASSERT_TRUE(it2.at_end());
}

void open_object(uint count, Opt_trace_context *trace, bool simulate_oom) {
  if (count == 0) return;
  count--;
  char key[4];
  /*
    Add 100 to always have a key of length 3, this matters to
    TraceContentTest.Indent.
    We could use just a fixed string, but it would cause an assertion failure
    (due to invalid JSON, which itself is conceivable in case of OOM).
  */
  llstr(100 + count, key);
  if (simulate_oom) {
    if (count == 90) DBUG_SET("+d,opt_trace_oom_in_open_struct");
    /*
      Now we let 80 objects be created, so that one of them surely hits
      re-allocation and OOM failure.
    */
    if (count == 10) DBUG_SET("-d,opt_trace_oom_in_open_struct");
  }
  Opt_trace_object oto(trace, key);
  open_object(count, trace, simulate_oom);
}

#ifndef DBUG_OFF

/// Test reaction to out-of-memory condition in trace buffer
TEST_F(TraceContentTest, OOMinBuffer) {
  ASSERT_FALSE(
      trace.start(true, false, false, false, -1, 1, ULONG_MAX, all_features));
  {
    Opt_trace_object oto(&trace);
    {
      Opt_trace_array ota(&trace, "one array");
      DBUG_SET("+d,opt_trace_oom_in_buffers");
      for (int i = 0; i < 30; i++)
        ota.add_alnum("_______________________________________________");
      DBUG_SET("-d,opt_trace_oom_in_buffers");
    }
  }
  trace.end();
  Opt_trace_iterator it(&trace);
  ASSERT_FALSE(it.at_end());
  Opt_trace_info info;
  it.get_value(&info);
  EXPECT_EQ(0U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  it.next();
  ASSERT_TRUE(it.at_end());
  EXPECT_TRUE(oom);
}

/// Test reaction to out-of-memory condition in book-keeping data structures
TEST_F(TraceContentTest, OOMinBookKeeping) {
  ASSERT_FALSE(
      trace.start(true, false, false, false, -1, 1, ULONG_MAX, all_features));
  {
    Opt_trace_object oto(&trace);
    open_object(100, &trace, true);
  }
  trace.end();
  Opt_trace_iterator it(&trace);
  ASSERT_FALSE(it.at_end());
  Opt_trace_info info;
  it.get_value(&info);
  EXPECT_EQ(0U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  it.next();
  ASSERT_TRUE(it.at_end());
  EXPECT_TRUE(oom);
}

/// Test reaction to OOM when purging traces
TEST_F(TraceContentTest, OOMinPurge) {
  make_one_trace(&trace, "103", -3, 2);
  make_one_trace(&trace, "104", -3, 2);
  DBUG_SET("+d,opt_trace_oom_in_purge");
  make_one_trace(&trace, "105", -3, 2);
  make_one_trace(&trace, "106", -3, 2);
  make_one_trace(&trace, "107", -3, 2);
  make_one_trace(&trace, "108", -3, 2);
  make_one_trace(&trace, "109", -3, 2);
  make_one_trace(&trace, "110", -3, 2);
  make_one_trace(&trace, "111", -3, 2);
  make_one_trace(&trace, "112", -3, 2);
  make_one_trace(&trace, "113", -3, 2);
  make_one_trace(&trace, "114", -3, 2);
  make_one_trace(&trace, "115", -3, 2);
  make_one_trace(&trace, "116", -3, 2);
  make_one_trace(&trace, "117", -3, 2);
  make_one_trace(&trace, "118", -3, 2);
  make_one_trace(&trace, "119", -3, 2);
  make_one_trace(&trace, "120", -3, 2);
  make_one_trace(&trace, "121", -3, 2);
  make_one_trace(&trace, "122", -3, 2);  // purge first fails here

  DBUG_SET("-d,opt_trace_oom_in_purge");
  // 122 could not purge 119, so we should see 119 and 120
  const char *expected_traces3[] = {"119", "120", nullptr};
  check(trace, expected_traces3);
  EXPECT_TRUE(oom);

  // Back to normal:
  oom = false;
  make_one_trace(&trace, "123", -3, 2);  // purge succeeds
  const char *expected_traces4[] = {"121", "122", nullptr};
  check(trace, expected_traces4);
  EXPECT_FALSE(oom);
}

#endif  // !DBUG_OFF

/** Test filtering by feature */
TEST_F(TraceContentTest, FilteringByFeature) {
  ASSERT_FALSE(trace.start(true, false, false, false, -1, 1, ULONG_MAX,
                           Opt_trace_context::MISC));
  {
    Opt_trace_object oto(&trace);
    {
      Opt_trace_array ota(&trace, "one array");
      ota.add(200.4);
      {
        Opt_trace_object oto1(&trace, Opt_trace_context::GREEDY_SEARCH);
        oto1.add_alnum("one key", "one value").add("another key", 100LL);
        Opt_trace_object oto2(&trace, "a fourth key", Opt_trace_context::MISC);
        oto2.add("another key inside", 5LL);
      }
      ota.add(true);
    }
    {
      Opt_trace_object oto3(&trace, "key for oto3",
                            Opt_trace_context::GREEDY_SEARCH);
      oto3.add("etc", 25);
    }
    oto.add("yet another key", -1000LL);
    {
      Opt_trace_array ota(&trace, "another array");
      ota.add(1LL).add(2LL).add(3LL).add(4LL);
    }
  }
  trace.end();
  Opt_trace_iterator it(&trace);
  ASSERT_FALSE(it.at_end());
  Opt_trace_info info;
  it.get_value(&info);
  const char expected[] =
      "{\n"
      "  \"one array\": [\n"
      "    200.4,\n"
      "    \"...\",\n"
      "    true\n"
      "  ],\n"
      "  \"key for oto3\": \"...\",\n"
      "  \"yet another key\": -1000,\n"
      "  \"another array\": [\n"
      "    1,\n"
      "    2,\n"
      "    3,\n"
      "    4\n"
      "  ]\n"
      "}";
  EXPECT_STREQ(expected, info.trace_ptr);
  EXPECT_EQ(sizeof(expected) - 1, info.trace_length);
  check_json_compliance(info.trace_ptr, info.trace_length);
  EXPECT_EQ(0U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  EXPECT_FALSE(oom);
  it.next();
  ASSERT_TRUE(it.at_end());
}

/** Test escaping of characters */
TEST_F(TraceContentTest, Escaping) {
  ASSERT_FALSE(
      trace.start(true, false, true, false, -1, 1, ULONG_MAX, all_features));
  // All ASCII 0-127 chars are valid UTF8 encodings
  char all_chars[130];
  for (uint c = 0; c < sizeof(all_chars) - 2; c++) all_chars[c] = c;
  // Now a character with a two-byte code in utf8: ä
  all_chars[128] = static_cast<char>(0xc3);
  all_chars[129] = static_cast<char>(0xa4);
  // all_chars is used both as query...
  trace.set_query(all_chars, sizeof(all_chars), system_charset_info);
  {
    Opt_trace_object oto(&trace);
    // ... and inside the trace:
    oto.add_utf8("somekey", all_chars, sizeof(all_chars));
  }
  trace.end();
  Opt_trace_iterator it(&trace);
  ASSERT_FALSE(it.at_end());
  Opt_trace_info info;
  it.get_value(&info);
  // we get the trace escaped, JSON-compliant:
  const char expected[] =
      "{\n"
      "  \"somekey\": "
      "\"\\u0000\\u0001\\u0002\\u0003\\u0004\\u0005\\u0006\\u0007\\u0008\\t\\n"
      "\\u000b\\u000c\\r\\u000e\\u000f\\u0010\\u0011\\u0012\\u0013\\u0014\\u001"
      "5\\u0016\\u0017\\u0018\\u0019\\u001a\\u001b\\u001c\\u001d\\u001e\\u001f "
      "!\\\"#$%&'()*+,-./"
      "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`"
      "abcdefghijklmnopqrstuvwxyz{|}~ä\"\n"
      "}";
  EXPECT_STREQ(expected, info.trace_ptr);
  EXPECT_EQ(sizeof(expected) - 1, info.trace_length);
  check_json_compliance(info.trace_ptr, info.trace_length);
  EXPECT_EQ(0U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  EXPECT_FALSE(oom);
  EXPECT_EQ(sizeof(all_chars), info.query_length);
  // we get the query unescaped, verbatim, not 0-terminated:
  EXPECT_EQ(0, memcmp(all_chars, info.query_ptr, sizeof(all_chars)));
  EXPECT_EQ(system_charset_info, info.query_charset);
  it.next();
  ASSERT_TRUE(it.at_end());
}

/** Test how the system handles non-UTF8 characters, a violation of its API */
TEST_F(TraceContentTest, NonUtf8) {
  ASSERT_FALSE(
      trace.start(true, false, true, false, -1, 1, ULONG_MAX, all_features));
  /*
    A string which starts with invalid utf8 (the four first bytes are éèÄà in
    latin1).
    In utf8, the following holds
    - E0->EF can only be the start of a 3-byte sequence
    - C2->DF                            2-byte
    - ASCII              a single-byte sequence
  */
  const char all_chars[] =
      "\xe9\xe8\xc4\xe0"
      "ABC";
  // We declare a query in latin1
  trace.set_query(all_chars, sizeof(all_chars), &my_charset_latin1);
  {
    Opt_trace_object oto(&trace);
    /*
      We pass the non-utf8-compliant string to add_utf8() (violating the
      API). We get it back unchanged. The trace system could try to be robust,
      detecting and sanitizing wrong characters (replacing them with '?'); but
      it does not bother, as the MySQL Server normally does not violate the
      API.
    */
    oto.add_utf8("somekey", all_chars, sizeof(all_chars) - 1);
  }
  trace.end();
  Opt_trace_iterator it(&trace);
  ASSERT_FALSE(it.at_end());
  Opt_trace_info info;
  it.get_value(&info);
  // This is not UTF8-compliant and thus not JSON-compliant.
  const char expected[] =
      "{\n"
      "  \"somekey\": \""
      "\xe9\xe8\xc4\xe0"
      "ABC\"\n"
      "}";
  EXPECT_STREQ(expected, info.trace_ptr);
  EXPECT_EQ(sizeof(expected) - 1, info.trace_length);
  EXPECT_EQ(0U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  EXPECT_FALSE(oom);
  EXPECT_EQ(sizeof(all_chars), info.query_length);
  // we get the query unescaped, verbatim, not 0-terminated:
  EXPECT_EQ(0, memcmp(all_chars, info.query_ptr, sizeof(all_chars)));
  it.next();
  ASSERT_TRUE(it.at_end());
}

/**
   Test indentation by many blanks.
   By creating a 100-level deep structure, we force an indentation which
   enters the while() block in Opt_trace_stmt::next_line().
*/
TEST_F(TraceContentTest, Indent) {
  ASSERT_FALSE(
      trace.start(true, false, false, false, -1, 1, ULONG_MAX, all_features));
  {
    Opt_trace_object oto(&trace);
    open_object(100, &trace, false);
  }
  trace.end();
  Opt_trace_iterator it(&trace);
  ASSERT_FALSE(it.at_end());
  Opt_trace_info info;
  it.get_value(&info);
  /*
    Formula for the expected size.
    Before the Nth call to open_object(), indentation inside the innermost
    empty object is noted I(N); so the relationship between the size before
    Nth call and the size after Nth call is:
    S(N+1) = S(N)
             + I(N)   (indentation before added '"xxx": {\n' )
             + 9      (length of added '"xxx": {\n' )
             + I(N)   (indentation before added '}\n' )
             + 2      (length of added '}\n' )
    and the indentation is increased by two as we are one level deeper:
    I(N+1) = I(N) + 2
    With S(1) = 3 (length of '{\n}') and I(1) = 2.
    So I(N) = 2 * N and
    S(N+1) - S(N) = 11 + 4 * N
    So S(N) = 3 + 11 * (N - 1) + 2 * N * (N - 1).
    For 100 calls, the final size is S(101) = 21303.
    Each call adds 10 non-space characters, so there should be
    21303
    - 10 * 100 (added non-spaces characters)
    - 3 (non-spaces of initial object before first function call)
    = 20300 spaces.
  */
  EXPECT_EQ(21303U, info.trace_length);
  uint spaces = 0;
  for (uint i = 0; i < info.trace_length; i++)
    if (info.trace_ptr[i] == ' ') spaces++;
  EXPECT_EQ(20300U, spaces);
  check_json_compliance(info.trace_ptr, info.trace_length);
  EXPECT_EQ(0U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  EXPECT_FALSE(oom);
  it.next();
  ASSERT_TRUE(it.at_end());
}

/** Test Opt_trace_context::missing_privilege() */
TEST_F(TraceContentTest, MissingPrivilege) {
  ASSERT_FALSE(
      trace.start(true, false, true, false, 0, 100, ULONG_MAX, all_features));
  {
    Opt_trace_object oto(&trace);
    {
      Opt_trace_array ota(&trace, "one array");
      ota.add(200.4);
      {
        Opt_trace_object oto1(&trace);
        oto1.add_alnum("one key", "one value").add("another key", 100LL);
        oto1.add("a third key", false);
        Opt_trace_object oto2(&trace, "a fourth key");
        oto2.add("key inside", 1LL);
        ASSERT_FALSE(trace.start(true, false, true, false, 0, 100, ULONG_MAX,
                                 all_features));
        {
          Opt_trace_object oto3(&trace);
          trace.missing_privilege();
          ASSERT_FALSE(trace.start(true, false, true, false, 0, 100, ULONG_MAX,
                                   all_features));
          {
            Opt_trace_object oto4(&trace);
            oto4.add_alnum("in4", "key4");
          }
          trace.end();
        }
        trace.end();  // this should restore I_S support
        // so this should be visible
        oto2.add("another key inside", 5LL);
        // and this new sub statement too:
        ASSERT_FALSE(trace.start(true, false, true, false, 0, 100, ULONG_MAX,
                                 all_features));
        {
          Opt_trace_object oto5(&trace);
          oto5.add("in5", true);
        }
        trace.end();
      }
      ota.add_alnum("one string element");
      ota.add(true);
    }
    oto.add("yet another key", -1000LL);
    {
      Opt_trace_array ota(&trace, "another array");
      ota.add(1LL).add(2LL).add(3LL).add(4LL);
    }
  }
  trace.end();
  Opt_trace_iterator it(&trace);
  ASSERT_FALSE(it.at_end());
  Opt_trace_info info;
  it.get_value(&info);
  const char expected[] =
      "{\n"
      "  \"one array\": [\n"
      "    200.4,\n"
      "    {\n"
      "      \"one key\": \"one value\",\n"
      "      \"another key\": 100,\n"
      "      \"a third key\": false,\n"
      "      \"a fourth key\": {\n"
      "        \"key inside\": 1,\n"
      "        \"another key inside\": 5\n"
      "      } /* a fourth key */\n"
      "    },\n"
      "    \"one string element\",\n"
      "    true\n"
      "  ] /* one array */,\n"
      "  \"yet another key\": -1000,\n"
      "  \"another array\": [\n"
      "    1,\n"
      "    2,\n"
      "    3,\n"
      "    4\n"
      "  ] /* another array */\n"
      "}";
  EXPECT_STREQ(expected, info.trace_ptr);
  EXPECT_EQ(sizeof(expected) - 1, info.trace_length);
  check_json_compliance(info.trace_ptr, info.trace_length);
  EXPECT_EQ(0U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  EXPECT_FALSE(oom);
  it.next();
  ASSERT_FALSE(it.at_end());
  // Now the substatement with a missing privilege
  it.get_value(&info);
  const char expected2[] = "";  // because of missing privilege...
  EXPECT_STREQ(expected2, info.trace_ptr);
  EXPECT_EQ(sizeof(expected2) - 1, info.trace_length);
  EXPECT_EQ(0U, info.missing_bytes);
  EXPECT_TRUE(info.missing_priv);  // ... tested here.
  it.next();
  ASSERT_FALSE(it.at_end());
  // And now the last substatement, visible
  it.get_value(&info);
  const char expected3[] =
      "{\n"
      "  \"in5\": true\n"
      "}";
  EXPECT_STREQ(expected3, info.trace_ptr);
  EXPECT_EQ(sizeof(expected3) - 1, info.trace_length);
  check_json_compliance(info.trace_ptr, info.trace_length);
  EXPECT_EQ(0U, info.missing_bytes);
  EXPECT_FALSE(info.missing_priv);
  it.next();
  ASSERT_TRUE(it.at_end());
}

/** Test Opt_trace_context::missing_privilege() on absent trace */
TEST_F(TraceContentTest, MissingPrivilege2) {
  /*
    Ask for neither I_S not debug output, and no
    missing_privilege() support
  */
  ASSERT_FALSE(
      trace.start(false, false, true, false, 0, 100, ULONG_MAX, all_features));
  EXPECT_FALSE(trace.is_started());
  trace.end();
  /*
    Ask for neither I_S not debug output, but ask that
    missing_privilege() is supported.
  */
  ASSERT_FALSE(
      trace.start(false, true, true, false, 0, 100, ULONG_MAX, all_features));
  EXPECT_TRUE(trace.is_started());
  trace.missing_privilege();
  // This above should make the substatement below not be traced:
  ASSERT_FALSE(
      trace.start(true, false, true, false, 0, 100, ULONG_MAX, all_features));
  {
    Opt_trace_object oto5(&trace);
    oto5.add("in5", true);
  }
  trace.end();
  trace.end();
  Opt_trace_iterator it(&trace);
  ASSERT_TRUE(it.at_end());
}

/**
   Test an optimization: that no Opt_trace_stmt is created in common case
   where all statements and substatements ask neither for I_S nor for DBUG,
   nor for support of missing_privilege() function.
*/
TEST_F(TraceContentTest, NoOptTraceStmt) {
  ASSERT_FALSE(
      trace.start(false, false, false, false, -1, 1, ULONG_MAX, all_features));
  EXPECT_FALSE(trace.is_started());
  // one substatement:
  ASSERT_FALSE(
      trace.start(false, false, false, false, -1, 1, ULONG_MAX, all_features));
  EXPECT_FALSE(trace.is_started());
  // another one deeper nested:
  ASSERT_FALSE(
      trace.start(false, false, false, false, -1, 1, ULONG_MAX, all_features));
  EXPECT_FALSE(trace.is_started());
  trace.end();
  trace.end();
  trace.end();
}

}  // namespace opt_trace_unittest

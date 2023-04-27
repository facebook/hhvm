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

#include "unittest/gunit/test_utils.h"

#include <gtest/gtest.h>
#include <atomic>
#include <new>
#include <ostream>
#include <string>

#include "gtest/gtest-message.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"  // DBUG_ASSERT
#include "my_inttypes.h"
#include "mysql_com.h"
#include "sql/binlog.h"
#include "sql/client_settings.h"
#include "sql/conn_handler/connection_handler_manager.h"
#include "sql/dd/dd.h"
#include "sql/dd/impl/dictionary_impl.h"  // dd::Dictionary_impl
#include "sql/dd/impl/tables/column_type_elements.h"
#include "sql/dd/impl/tables/schemata.h"
#include "sql/dd/impl/tables/tables.h"
#include "sql/derror.h"
#include "sql/item_func.h"
#include "sql/keycaches.h"
#include "sql/log.h"  // query_logger
#include "sql/my_decimal.h"
#include "sql/mysqld.h"  // set_remaining_args
#include "sql/mysqld_thd_manager.h"
#include "sql/opt_costconstantcache.h"  // optimizer cost constant cache
#include "sql/opt_range.h"
#include "sql/rpl_filter.h"
#include "sql/rpl_handler.h"  // delegates_init()
#include "sql/set_var.h"
#include "sql/sql_class.h"
#include "sql/sql_lex.h"
#include "sql/sql_locale.h"
#include "sql/xa.h"
#include "unicode/uclean.h"

namespace my_testing {

int chars_2_decimal(const char *chars, my_decimal *to) {
  const char *end = strend(chars);
  return string2decimal(chars, to, &end);
}

/*
  A mock error handler for error_handler_hook.
*/
std::atomic<uint> expected_error{0};
extern "C" void test_error_handler_hook(uint err, const char *str, myf) {
  EXPECT_EQ(expected_error.load(), err) << str;
}

void setup_server_for_unit_tests() {
  std::string my_name(my_progname);
  char *argv[] = {const_cast<char *>(my_name.c_str()),
                  const_cast<char *>("--secure-file-priv=NULL"),
                  const_cast<char *>("--log_syslog=0"),
                  const_cast<char *>("--explicit_defaults_for_timestamp"),
                  const_cast<char *>("--datadir=" DATA_DIR),
                  const_cast<char *>("--lc-messages-dir=" ERRMSG_DIR),
                  nullptr};
  set_remaining_args(6, argv);
  system_charset_info = &my_charset_utf8_general_ci;
  sys_var_init();
  init_common_variables();
  test_flags |= TEST_SIGINT;
  opt_core_file = false;
  my_init_signals();
  randominit(&sql_rand, 0, 0);
  transaction_cache_init();
  delegates_init();
  gtid_server_init();
  error_handler_hook = test_error_handler_hook;
  // Initialize Query_logger last, to avoid spurious warnings to stderr.
  query_logger.init();
  init_optimizer_cost_module(false);
  DD_initializer::SetUp();
}

void teardown_server_for_unit_tests() {
  mysql_bin_log.cleanup();
  range_optimizer_free();
  sys_var_end();
  delegates_destroy();
  transaction_cache_free();
  gtid_server_cleanup();
  query_logger.cleanup();
  item_func_sleep_free();
  item_create_cleanup();
  delete_optimizer_cost_module();
  DD_initializer::TearDown();
  delete &dd::tables::Column_type_elements::instance();
  delete &dd::tables::Schemata::instance();
  delete &dd::tables::Tables::instance();
  key_caches.delete_elements();
  free_tmpdir(&mysql_tmpdir_list);
  delete binlog_filter;
  u_cleanup();
  cleanup_errmsgs();
  Connection_handler_manager::destroy_instance();
  mysql_client_plugin_deinit();
  deinit_errmessage();  // finish server errs
  Global_THD_manager::destroy_instance();
  my_end(0);
  clean_up_mysqld_mutexes();
}

void Server_initializer::set_expected_error(uint val) { expected_error = val; }

void Server_initializer::SetUp() {
  expected_error = 0;
  m_thd = new THD(false);
  THD *stack_thd = m_thd;

  m_thd->set_new_thread_id();

  m_thd->thread_stack = (char *)&stack_thd;
  m_thd->store_globals();
  lex_start(m_thd);
}

void Server_initializer::TearDown() {
  m_thd->cleanup_after_query();
  delete m_thd;
}

Mock_error_handler::Mock_error_handler(THD *thd, uint expected_error)
    : m_thd(thd), m_expected_error(expected_error), m_handle_called(0) {
  thd->push_internal_handler(this);
}

Mock_error_handler::~Mock_error_handler() {
  // Strange Visual Studio bug: have to store 'this' in local variable.
  Internal_error_handler *me = this;
  EXPECT_EQ(me, m_thd->pop_internal_handler());
  if (m_expected_error == 0) {
    EXPECT_EQ(0, m_handle_called);
  } else {
    EXPECT_GT(m_handle_called, 0)
        << "Error " << m_expected_error << " expected.";
  }
}

bool Mock_error_handler::handle_condition(THD *, uint sql_errno, const char *,
                                          Sql_condition::enum_severity_level *,
                                          const char *) {
  EXPECT_EQ(m_expected_error, sql_errno);
  ++m_handle_called;
  return true;
}

void DD_initializer::SetUp() {
  /*
    With WL#6599, SELECT_LEX::add_table_to_list() will invoke
    dd::Dictionary::is_system_view_name() method. E.g., the unit
    test InsertDelayed would invoke above API. This requires us
    to have a instance of dictionary_impl. We do not really need
    to initialize dd::System_views for this test. Also, there can
    be future test cases that need the same.
  */
  dd::Dictionary_impl::s_instance = new (std::nothrow) dd::Dictionary_impl();
  DBUG_ASSERT(dd::Dictionary_impl::s_instance != nullptr);
}

void DD_initializer::TearDown() {
  DBUG_ASSERT(dd::Dictionary_impl::s_instance != nullptr);
  delete dd::Dictionary_impl::s_instance;
}

}  // namespace my_testing

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <test/test_ext_oracle.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtOracle::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_oci_connect);
  RUN_TEST(test_oci_new_connect);
  RUN_TEST(test_oci_pconnect);
  RUN_TEST(test_oci_server_version);
  RUN_TEST(test_oci_password_change);
  RUN_TEST(test_oci_new_cursor);
  RUN_TEST(test_oci_new_descriptor);
  RUN_TEST(test_oci_close);
  RUN_TEST(test_oci_commit);
  RUN_TEST(test_oci_rollback);
  RUN_TEST(test_oci_error);
  RUN_TEST(test_oci_internal_debug);
  RUN_TEST(test_oci_parse);
  RUN_TEST(test_oci_statement_type);
  RUN_TEST(test_oci_free_statement);
  RUN_TEST(test_oci_free_descriptor);
  RUN_TEST(test_oci_bind_array_by_name);
  RUN_TEST(test_oci_bind_by_name);
  RUN_TEST(test_oci_cancel);
  RUN_TEST(test_oci_define_by_name);
  RUN_TEST(test_oci_execute);
  RUN_TEST(test_oci_num_fields);
  RUN_TEST(test_oci_num_rows);
  RUN_TEST(test_oci_result);
  RUN_TEST(test_oci_set_prefetch);
  RUN_TEST(test_oci_fetch_all);
  RUN_TEST(test_oci_fetch_array);
  RUN_TEST(test_oci_fetch_assoc);
  RUN_TEST(test_oci_fetch_object);
  RUN_TEST(test_oci_fetch_row);
  RUN_TEST(test_oci_fetch);
  RUN_TEST(test_oci_field_is_null);
  RUN_TEST(test_oci_field_name);
  RUN_TEST(test_oci_field_precision);
  RUN_TEST(test_oci_field_scale);
  RUN_TEST(test_oci_field_size);
  RUN_TEST(test_oci_field_type_raw);
  RUN_TEST(test_oci_field_type);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtOracle::test_oci_connect() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_new_connect() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_pconnect() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_server_version() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_password_change() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_new_cursor() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_new_descriptor() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_close() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_commit() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_rollback() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_error() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_internal_debug() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_parse() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_statement_type() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_free_statement() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_free_descriptor() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_bind_array_by_name() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_bind_by_name() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_cancel() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_define_by_name() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_execute() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_num_fields() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_num_rows() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_result() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_set_prefetch() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_fetch_all() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_fetch_array() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_fetch_assoc() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_fetch_object() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_fetch_row() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_fetch() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_field_is_null() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_field_name() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_field_precision() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_field_scale() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_field_size() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_field_type_raw() {
  //VCB("<?php ");
  return true;
}

bool TestExtOracle::test_oci_field_type() {
  //VCB("<?php ");
  return true;
}

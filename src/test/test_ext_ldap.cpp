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

#include <test/test_ext_ldap.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtLdap::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_ldap_connect);
  RUN_TEST(test_ldap_explode_dn);
  RUN_TEST(test_ldap_dn2ufn);
  RUN_TEST(test_ldap_err2str);
  RUN_TEST(test_ldap_8859_to_t61);
  RUN_TEST(test_ldap_t61_to_8859);
  RUN_TEST(test_ldap_add);
  RUN_TEST(test_ldap_mod_add);
  RUN_TEST(test_ldap_mod_del);
  RUN_TEST(test_ldap_mod_replace);
  RUN_TEST(test_ldap_modify);
  RUN_TEST(test_ldap_bind);
  RUN_TEST(test_ldap_sasl_bind);
  RUN_TEST(test_ldap_set_rebind_proc);
  RUN_TEST(test_ldap_sort);
  RUN_TEST(test_ldap_start_tls);
  RUN_TEST(test_ldap_unbind);
  RUN_TEST(test_ldap_get_option);
  RUN_TEST(test_ldap_set_option);
  RUN_TEST(test_ldap_close);
  RUN_TEST(test_ldap_list);
  RUN_TEST(test_ldap_read);
  RUN_TEST(test_ldap_search);
  RUN_TEST(test_ldap_rename);
  RUN_TEST(test_ldap_delete);
  RUN_TEST(test_ldap_compare);
  RUN_TEST(test_ldap_errno);
  RUN_TEST(test_ldap_error);
  RUN_TEST(test_ldap_get_dn);
  RUN_TEST(test_ldap_count_entries);
  RUN_TEST(test_ldap_get_entries);
  RUN_TEST(test_ldap_first_entry);
  RUN_TEST(test_ldap_next_entry);
  RUN_TEST(test_ldap_get_attributes);
  RUN_TEST(test_ldap_first_attribute);
  RUN_TEST(test_ldap_next_attribute);
  RUN_TEST(test_ldap_first_reference);
  RUN_TEST(test_ldap_next_reference);
  RUN_TEST(test_ldap_parse_reference);
  RUN_TEST(test_ldap_parse_result);
  RUN_TEST(test_ldap_free_result);
  RUN_TEST(test_ldap_get_values_len);
  RUN_TEST(test_ldap_get_values);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtLdap::test_ldap_connect() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_explode_dn() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_dn2ufn() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_err2str() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_8859_to_t61() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_t61_to_8859() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_add() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_mod_add() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_mod_del() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_mod_replace() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_modify() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_bind() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_sasl_bind() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_set_rebind_proc() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_sort() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_start_tls() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_unbind() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_get_option() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_set_option() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_close() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_list() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_read() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_search() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_rename() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_delete() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_compare() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_errno() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_error() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_get_dn() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_count_entries() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_get_entries() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_first_entry() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_next_entry() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_get_attributes() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_first_attribute() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_next_attribute() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_first_reference() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_next_reference() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_parse_reference() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_parse_result() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_free_result() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_get_values_len() {
  //VCB("<?php ");
  return true;
}

bool TestExtLdap::test_ldap_get_values() {
  //VCB("<?php ");
  return true;
}

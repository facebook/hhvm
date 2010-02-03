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

#include <test/test_ext_reflection.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtReflection::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_hphp_get_extension_info);
  RUN_TEST(test_hphp_get_class_info);
  RUN_TEST(test_hphp_get_function_info);
  RUN_TEST(test_hphp_invoke);
  RUN_TEST(test_hphp_invoke_method);
  RUN_TEST(test_hphp_instanceof);
  RUN_TEST(test_hphp_create_object);
  RUN_TEST(test_hphp_get_property);
  RUN_TEST(test_hphp_set_property);
  RUN_TEST(test_hphp_get_static_property);
  RUN_TEST(test_hphp_set_static_property);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtReflection::test_hphp_get_extension_info() {
  //VCB("<?php ");
  return true;
}

bool TestExtReflection::test_hphp_get_class_info() {
  //VCB("<?php ");
  return true;
}

bool TestExtReflection::test_hphp_get_function_info() {
  //VCB("<?php ");
  return true;
}

bool TestExtReflection::test_hphp_invoke() {
  //VCB("<?php ");
  return true;
}

bool TestExtReflection::test_hphp_invoke_method() {
  //VCB("<?php ");
  return true;
}

bool TestExtReflection::test_hphp_instanceof() {
  //VCB("<?php ");
  return true;
}

bool TestExtReflection::test_hphp_create_object() {
  //VCB("<?php ");
  return true;
}

bool TestExtReflection::test_hphp_get_property() {
  //VCB("<?php ");
  return true;
}

bool TestExtReflection::test_hphp_set_property() {
  //VCB("<?php ");
  return true;
}

bool TestExtReflection::test_hphp_get_static_property() {
  //VCB("<?php ");
  return true;
}

bool TestExtReflection::test_hphp_set_static_property() {
  //VCB("<?php ");
  return true;
}

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

#include <test/test_ext_class.h>
#include <runtime/ext/ext_class.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtClass::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_get_declared_classes);
  RUN_TEST(test_get_declared_interfaces);
  RUN_TEST(test_class_exists);
  RUN_TEST(test_interface_exists);
  RUN_TEST(test_get_class_methods);
  RUN_TEST(test_get_class_vars);
  RUN_TEST(test_get_class);
  RUN_TEST(test_get_parent_class);
  RUN_TEST(test_is_a);
  RUN_TEST(test_is_subclass_of);
  RUN_TEST(test_method_exists);
  RUN_TEST(test_property_exists);
  RUN_TEST(test_get_object_vars);
  RUN_TEST(test_call_user_method_array);
  RUN_TEST(test_call_user_method);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// test classes are all in g_class_map defined in test_ext_function.cpp

bool TestExtClass::test_get_declared_classes() {
  Array classes = f_get_declared_classes();
  VS(classes[0], "test");
  return Count(true);
}

bool TestExtClass::test_get_declared_interfaces() {
  Array classes = f_get_declared_interfaces();
  VS(classes[0], "itestable");
  return Count(true);
}

bool TestExtClass::test_class_exists() {
  VERIFY(f_class_exists("TEst"));
  return Count(true);
}

bool TestExtClass::test_interface_exists() {
  VERIFY(f_interface_exists("iTESTable"));
  return Count(true);
}

bool TestExtClass::test_get_class_methods() {
  Array methods = f_get_class_methods("TEst");
  VS(methods[0], "foo");
  VS(methods[1], "func");
  VS(methods[2], "bar");
  return Count(true);
}

bool TestExtClass::test_get_class_vars() {
  Array properties = f_get_class_vars("TEst");
  VS(properties, CREATE_MAP3("foo", null, "prop", null, "bar", null));
  return Count(true);
}

bool TestExtClass::test_get_class() {
  // TestCodeRun covers this
  return Count(true);
}

bool TestExtClass::test_get_parent_class() {
  // TestCodeRun covers this
  return Count(true);
}

bool TestExtClass::test_is_a() {
  // TestCodeRun covers this
  return Count(true);
}

bool TestExtClass::test_is_subclass_of() {
  // TestCodeRun covers this
  return Count(true);
}

bool TestExtClass::test_method_exists() {
  // TestCodeRun covers this
  return Count(true);
}

bool TestExtClass::test_property_exists() {
  VERIFY(f_property_exists("TEst", "prop"));
  return Count(true);
}

bool TestExtClass::test_get_object_vars() {
  // TestCodeRun covers this
  return Count(true);
}

bool TestExtClass::test_call_user_method_array() {
  // TestCodeRun covers this
  return Count(true);
}

bool TestExtClass::test_call_user_method() {
  // TestCodeRun covers this
  return Count(true);
}

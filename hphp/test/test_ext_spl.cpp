/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <test/test_ext_spl.h>
#include <runtime/ext/ext_spl.h>

IMPLEMENT_SEP_EXTENSION_TEST(Spl);
///////////////////////////////////////////////////////////////////////////////

bool TestExtSpl::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_spl_classes);
  RUN_TEST(test_spl_object_hash);
  RUN_TEST(test_class_implements);
  RUN_TEST(test_class_parents);
  RUN_TEST(test_iterator_apply);
  RUN_TEST(test_iterator_count);
  RUN_TEST(test_iterator_to_array);
  RUN_TEST(test_spl_autoload_call);
  RUN_TEST(test_spl_autoload_extensions);
  RUN_TEST(test_spl_autoload_functions);
  RUN_TEST(test_spl_autoload_register);
  RUN_TEST(test_spl_autoload_unregister);
  RUN_TEST(test_spl_autoload);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtSpl::test_spl_classes() {
  return Count(true);
}

bool TestExtSpl::test_spl_object_hash() {
  return Count(true);
}

bool TestExtSpl::test_class_implements() {
  return Count(true);
}

bool TestExtSpl::test_class_parents() {
  return Count(true);
}

bool TestExtSpl::test_iterator_apply() {
  return Count(true);
}

bool TestExtSpl::test_iterator_count() {
  return Count(true);
}

bool TestExtSpl::test_iterator_to_array() {
  return Count(true);
}

bool TestExtSpl::test_spl_autoload_call() {
  return Count(true);
}

bool TestExtSpl::test_spl_autoload_extensions() {
  return Count(true);
}

bool TestExtSpl::test_spl_autoload_functions() {
  return Count(true);
}

bool TestExtSpl::test_spl_autoload_register() {
  return Count(true);
}

bool TestExtSpl::test_spl_autoload_unregister() {
  return Count(true);
}

bool TestExtSpl::test_spl_autoload() {
  return Count(true);
}

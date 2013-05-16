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

#include "hphp/test/test_ext_function.h"
#include "hphp/runtime/ext/ext_function.h"

///////////////////////////////////////////////////////////////////////////////

bool TestExtFunction::RunTests(const std::string &which) {
  bool ret = true;

  DECLARE_TEST_FUNCTIONS("function test($s1) {"
                         " return $s1;"
                         "}");

  RUN_TEST(test_get_defined_functions);
  RUN_TEST(test_function_exists);
  RUN_TEST(test_is_callable);
  RUN_TEST(test_call_user_func_array);
  RUN_TEST(test_call_user_func);
  RUN_TEST(test_forward_static_call_array);
  RUN_TEST(test_forward_static_call);
  RUN_TEST(test_create_function);
  RUN_TEST(test_func_get_arg);
  RUN_TEST(test_func_get_args);
  RUN_TEST(test_func_num_args);
  RUN_TEST(test_register_postsend_function);
  RUN_TEST(test_register_shutdown_function);
  RUN_TEST(test_register_cleanup_function);
  RUN_TEST(test_register_tick_function);
  RUN_TEST(test_unregister_tick_function);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtFunction::test_get_defined_functions() {
  static const StaticString s_internal("internal");
  Array funcs = f_get_defined_functions();
  VERIFY(!funcs[s_internal][0].toString().empty());
  return Count(true);
}

bool TestExtFunction::test_function_exists() {
  VERIFY(f_function_exists("test"));
  VERIFY(f_function_exists("TEst"));
  VERIFY(!f_function_exists("fake"));
  return Count(true);
}

bool TestExtFunction::test_is_callable() {
  VERIFY(f_is_callable("TEst"));
  // lots of testing in TestCodeRun::TestObjectMethod()
  return Count(true);
}

bool TestExtFunction::test_call_user_func_array() {
  Variant ret = vm_call_user_func("TEst", CREATE_VECTOR1("param"));
  VS(ret, "param");
  return Count(true);
}

bool TestExtFunction::test_call_user_func() {
  Variant ret = f_call_user_func(1, "TEst", CREATE_VECTOR1("param"));
  VS(ret, "param");
  return Count(true);
}

bool TestExtFunction::test_forward_static_call_array() {
  // tested in TestCodeRun::TestLateStaticBinding
  return true;
}

bool TestExtFunction::test_forward_static_call() {
  // tested in TestCodeRun::TestLateStaticBinding
  return true;
}

bool TestExtFunction::test_create_function() {
  try {
    f_create_function("$a", "");
  } catch (const NotSupportedException& e) {
    return Count(false);
  }
  return Count(true);
}

bool TestExtFunction::test_func_get_arg() {
  // TestCodeRun covers this
  return Count(true);
}

bool TestExtFunction::test_func_get_args() {
  // TestCodeRun covers this
  return Count(true);
}

bool TestExtFunction::test_func_num_args() {
  // TestCodeRun covers this
  return Count(true);
}

bool TestExtFunction::test_register_postsend_function() {
  // TestCodeRun covers this
  return Count(true);
}

bool TestExtFunction::test_register_shutdown_function() {
  // TestCodeRun covers this
  return Count(true);
}

bool TestExtFunction::test_register_cleanup_function() {
  // TestCodeRun covers this
  return Count(true);
}

bool TestExtFunction::test_register_tick_function() {
  try {
    f_register_tick_function(0, "test", Array::Create());
  } catch (const NotImplementedException& e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtFunction::test_unregister_tick_function() {
  try {
    f_unregister_tick_function("test");
  } catch (const NotImplementedException& e) {
    return Count(true);
  }
  return Count(false);
}

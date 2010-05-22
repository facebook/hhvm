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

#include <test/test_ext_fb.h>
#include <runtime/ext/ext_fb.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtFb::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_fb_thrift_serialize);
  RUN_TEST(test_fb_thrift_unserialize);
  RUN_TEST(test_fb_rename_function);
  RUN_TEST(test_fb_utf8ize);
  RUN_TEST(test_fb_call_user_func_safe);
  RUN_TEST(test_fb_call_user_func_safe_return);
  RUN_TEST(test_fb_call_user_func_array_safe);
  RUN_TEST(test_fb_load_local_databases);
  RUN_TEST(test_fb_parallel_query);
  RUN_TEST(test_fb_crossall_query);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtFb::test_fb_thrift_serialize() {
  Variant ret;
  VS(f_fb_thrift_unserialize(f_fb_thrift_serialize("test"), ref(ret)), "test");
  VERIFY(same(ret, true));

  ret = null;
  VS(f_fb_thrift_unserialize(f_fb_thrift_serialize(CREATE_VECTOR1("test")),
                             ref(ret)),
     CREATE_VECTOR1("test"));
  VERIFY(same(ret, true));
  return Count(true);
}

bool TestExtFb::test_fb_thrift_unserialize() {
  // tested above
  return Count(true);
}

bool TestExtFb::test_fb_rename_function() {
  // tested in TestCodeRun
  return Count(true);
}

bool TestExtFb::test_fb_utf8ize() {
  {
    Variant s = "hon\xE7k";
    VERIFY(f_fb_utf8ize(ref(s)));
    VS(s, "honk");
  }
  {
    Variant s = "test\xE0\xB0\xB1\xE0";
    VERIFY(f_fb_utf8ize(ref(s)));
    VS(s, "test\xE0\xB0\xB1");
  }
  {
    Variant s = "test\xE0\xB0\xB1\xE0\xE0";
    VERIFY(f_fb_utf8ize(ref(s)));
    VS(s, "test\xE0\xB0\xB1");
  }
  return Count(true);
}

bool TestExtFb::test_fb_call_user_func_safe() {
  {
    Variant ret = f_fb_call_user_func_safe
      (1, "TEst", CREATE_VECTOR1("param"));
    VS(ret, CREATE_VECTOR2(true, "param"));
  }
  {
    Variant ret = f_fb_call_user_func_safe
      (1, "NonTEst", CREATE_VECTOR1("param"));
    VS(ret, CREATE_VECTOR2(false, null));
  }
  return Count(true);
}

bool TestExtFb::test_fb_call_user_func_safe_return() {
  {
    Variant ret = f_fb_call_user_func_safe_return
      (1, "TEst", "ok", CREATE_VECTOR1("param"));
    VS(ret, "param");
  }
  {
    Variant ret = f_fb_call_user_func_safe_return
      (1, "NonTEst", "ok", CREATE_VECTOR1("param"));
    VS(ret, "ok");
  }
  return Count(true);
}

bool TestExtFb::test_fb_call_user_func_array_safe() {
  {
    Variant ret = f_fb_call_user_func_array_safe
      ("TEst", CREATE_VECTOR1("param"));
    VS(ret, CREATE_VECTOR2(true, "param"));
  }
  {
    Variant ret = f_fb_call_user_func_array_safe
      ("NonT", CREATE_VECTOR1("param"));
    VS(ret, CREATE_VECTOR2(false, null));
  }
  return Count(true);
}

bool TestExtFb::test_fb_load_local_databases() {
  // tested with PHP unit tests
  return Count(true);
}

bool TestExtFb::test_fb_parallel_query() {
  // tested with PHP unit tests
  return Count(true);
}

bool TestExtFb::test_fb_crossall_query() {
  // tested with PHP unit tests
  return Count(true);
}

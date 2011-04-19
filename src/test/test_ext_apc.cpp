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

#include <test/test_ext_apc.h>
#include <runtime/ext/ext_apc.h>
#include <runtime/base/shared/shared_store_base.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/program_functions.h>

///////////////////////////////////////////////////////////////////////////////

namespace HPHP {
  extern SharedStores s_apc_store;
}

bool TestExtApc::RunTests(const std::string &which) {
  bool ret = true;

  RuntimeOption::ApcTableType = RuntimeOption::ApcHashTable;
  s_apc_store.reset();
  printf("\nNon shared-memory version:\n");
  RUN_TEST(test_apc_add);
  RUN_TEST(test_apc_store);
  RUN_TEST(test_apc_fetch);
  RUN_TEST(test_apc_delete);
  RUN_TEST(test_apc_compile_file);
  RUN_TEST(test_apc_cache_info);
  RUN_TEST(test_apc_clear_cache);
  RUN_TEST(test_apc_define_constants);
  RUN_TEST(test_apc_load_constants);
  RUN_TEST(test_apc_sma_info);
  RUN_TEST(test_apc_filehits);
  RUN_TEST(test_apc_delete_file);
  RUN_TEST(test_apc_inc);
  RUN_TEST(test_apc_dec);
  RUN_TEST(test_apc_cas);
  RUN_TEST(test_apc_bin_dump);
  RUN_TEST(test_apc_bin_load);
  RUN_TEST(test_apc_bin_dumpfile);
  RUN_TEST(test_apc_bin_loadfile);
  RUN_TEST(test_apc_exists);

  RuntimeOption::ApcTableType = RuntimeOption::ApcConcurrentTable;
  s_apc_store.reset();
  printf("\nNon shared-memory concurrent version:\n");
  RUN_TEST(test_apc_add);
  RUN_TEST(test_apc_store);
  RUN_TEST(test_apc_fetch);
  RUN_TEST(test_apc_delete);
  RUN_TEST(test_apc_compile_file);
  RUN_TEST(test_apc_cache_info);
  RUN_TEST(test_apc_clear_cache);
  RUN_TEST(test_apc_define_constants);
  RUN_TEST(test_apc_load_constants);
  RUN_TEST(test_apc_sma_info);
  RUN_TEST(test_apc_filehits);
  RUN_TEST(test_apc_delete_file);
  RUN_TEST(test_apc_inc);
  RUN_TEST(test_apc_dec);
  RUN_TEST(test_apc_cas);
  RUN_TEST(test_apc_bin_dump);
  RUN_TEST(test_apc_bin_load);
  RUN_TEST(test_apc_bin_dumpfile);
  RUN_TEST(test_apc_bin_loadfile);
  RUN_TEST(test_apc_exists);

  s_apc_store.clear();
  RuntimeOption::ApcTableType = RuntimeOption::ApcHashTable;
  s_apc_store.create();
  printf("\nNon shared-memory version:\n");
  RUN_TEST(test_apc_add);
  RUN_TEST(test_apc_store);
  RUN_TEST(test_apc_fetch);
  RUN_TEST(test_apc_delete);
  RUN_TEST(test_apc_compile_file);
  RUN_TEST(test_apc_cache_info);
  RUN_TEST(test_apc_clear_cache);
  RUN_TEST(test_apc_define_constants);
  RUN_TEST(test_apc_load_constants);
  RUN_TEST(test_apc_sma_info);
  RUN_TEST(test_apc_filehits);
  RUN_TEST(test_apc_delete_file);
  RUN_TEST(test_apc_inc);
  RUN_TEST(test_apc_dec);
  RUN_TEST(test_apc_cas);
  RUN_TEST(test_apc_bin_dump);
  RUN_TEST(test_apc_bin_load);
  RUN_TEST(test_apc_bin_dumpfile);
  RUN_TEST(test_apc_bin_loadfile);
  RUN_TEST(test_apc_exists);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtApc::test_apc_add() {
  f_apc_add("ts", "TestString");
  f_apc_add("ta", CREATE_MAP2("a", 1, "b", 2));
  f_apc_add("ts", "NewValue");
  f_apc_add("ta", CREATE_VECTOR1("newelement"));
  VS(f_apc_fetch("ts"), "TestString");
  VS(f_apc_fetch("ta"), CREATE_MAP2("a", 1, "b", 2));

  VS(f_apc_fetch("ts"), "TestString");
  VS(f_apc_fetch("ta"), CREATE_MAP2("a", 1, "b", 2));

  f_apc_add("texp", "TestString", 1);
  sleep(1);
  VS(f_apc_fetch("texp"), false);

  Variant ret = f_apc_store("foo", false);
  VS(ret, true);
  ret = f_apc_add("foo", false);
  VS(ret, false);
  Variant success;
  ret = f_apc_fetch("foo", ref(success));
  VS(ret, false);
  VS(success, true);
  ret = f_apc_fetch("bar", ref(success));
  VS(ret, false);
  VS(success, false);
  Variant map1= CREATE_MAP1("foo", false);
  ret = f_apc_fetch(CREATE_VECTOR1("foo"), ref(success));
  VS(ret, map1);
  ret = f_apc_fetch(CREATE_VECTOR1("bar"), ref(success));
  VS(ret, Array::Create());
  VS(success, false);
  ret = f_apc_fetch(CREATE_VECTOR2("foo", "bar"), ref(success));
  VS(ret, map1);
  VS(success, true);
  ret = f_apc_fetch(CREATE_VECTOR4("foo", "bar", "foo", "bar"), ref(success));
  VS(ret, map1);
  VS(success, true);
  return Count(true);
}

bool TestExtApc::test_apc_store() {
  Array complexMap = CREATE_MAP2("a",
                                 CREATE_MAP2("b", 1, "c",
                                             CREATE_VECTOR2("d", "e")),
                                 "f", CREATE_VECTOR3(1,2,3));
  f_apc_store("complexMap", complexMap);
  f_apc_store("ts", "TestString");
  f_apc_store("ta", CREATE_MAP2("a", 1, "b", 2));
  f_apc_store("ts", "NewValue");
  f_apc_store("ta", CREATE_VECTOR1("newelement"));
  VS(f_apc_fetch("ts"), "NewValue");
  VS(f_apc_fetch("ta"), CREATE_VECTOR1("newelement"));
  VS(f_apc_fetch("complexMap"), complexMap);

  VS(f_apc_fetch("ts"), "NewValue");
  VS(f_apc_fetch("ta"), CREATE_VECTOR1("newelement"));
  VS(f_apc_fetch("complexMap"), complexMap);

  // Make sure it doesn't change the shared value.
  Array complexMapFetched = f_apc_fetch("complexMap");
  VERIFY(complexMapFetched.exists("a"));
  complexMapFetched.set("q",0);
  VERIFY(complexMapFetched.exists("q"));
  VS(f_apc_fetch("complexMap"), complexMap);

  String tsFetched = f_apc_fetch("ts");
  VS(tsFetched, "NewValue");
  String sharedString = tsFetched;
  tsFetched.lvalAt(0) = "M";
  VS(tsFetched, "MewValue");
  VS(sharedString, "NewValue");
  VERIFY(tsFetched.get() != sharedString.get());
  VS(f_apc_fetch("ts"), "NewValue");

  return Count(true);
}

bool TestExtApc::test_apc_fetch() {
  // reproducing a memory leak (3/26/09)
  f_apc_add("apcdata", CREATE_MAP2("a", "test", "b", 1)); // MapVariant
  {
    Variant apcdata = f_apc_fetch("apcdata");
    Variant c = apcdata; // bump up ref count to make a MapVariant copy
    apcdata.set("b", 3); // problem
    VS(apcdata, CREATE_MAP2("a", "test", "b", 3));
  }
  {
    Variant apcdata = f_apc_fetch("apcdata");
    apcdata += CREATE_MAP1("b", 4); // problem
    VS(apcdata, CREATE_MAP2("a", "test", "b", 1));
  }
  {
    Variant apcdata = f_apc_fetch(CREATE_VECTOR2("apcdata", "nah"));
    VS(apcdata, CREATE_MAP1("apcdata", CREATE_MAP2("a", "test", "b", 1)));
  }
  return Count(true);
}

bool TestExtApc::test_apc_delete() {
  f_apc_store("ts", "TestString");
  f_apc_store("ta", CREATE_MAP2("a", 1, "b", 2));
  f_apc_delete("ts");
  f_apc_delete("ta");
  VS(f_apc_fetch("ts"), false);
  VS(f_apc_fetch("ta"), false);

  VS(f_apc_fetch("ts"), false);
  VS(f_apc_fetch("ta"), false);

  f_apc_store("ts", "TestString");
  f_apc_store("ta", CREATE_MAP2("a", 1, "b", 2));
  VS(f_apc_delete(CREATE_VECTOR2("ts", "ta")), Array::Create());
  VS(f_apc_fetch("ts"), false);
  VS(f_apc_fetch("ta"), false);

  return Count(true);
}

bool TestExtApc::test_apc_compile_file() {
  try {
    f_apc_compile_file("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApc::test_apc_cache_info() {
  Array ci = f_apc_cache_info();
  VS(ci.rvalAt("start_time"), start_time());
  return Count(true);
}

bool TestExtApc::test_apc_clear_cache() {
  f_apc_store("ts", "TestString");
  f_apc_store("ta", CREATE_MAP2("a", 1, "b", 2));

  f_apc_clear_cache();
  VS(f_apc_fetch("ts"), false);
  VS(f_apc_fetch("ta"), false);

  VS(f_apc_fetch("ts"), false);
  VS(f_apc_fetch("ta"), false);
  return Count(true);
}

bool TestExtApc::test_apc_define_constants() {
  try {
    f_apc_define_constants("", "");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApc::test_apc_load_constants() {
  try {
    f_apc_load_constants("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApc::test_apc_sma_info() {
  VS(f_apc_sma_info(), Array::Create());
  return Count(true);
}

bool TestExtApc::test_apc_filehits() {
  try {
    f_apc_filehits();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApc::test_apc_delete_file() {
  try {
    f_apc_delete_file("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApc::test_apc_inc() {
  f_apc_store("ts", 12);
  VS(f_apc_inc("ts"), 13);
  VS(f_apc_inc("ts", 5), 18);
  VS(f_apc_inc("ts", -3), 15);
  return Count(true);
}

bool TestExtApc::test_apc_dec() {
  f_apc_store("ts", 12);
  VS(f_apc_dec("ts"), 11);
  VS(f_apc_dec("ts", 5), 6);
  VS(f_apc_dec("ts", -3), 9);
  return Count(true);
}

bool TestExtApc::test_apc_cas() {
  f_apc_store("ts", 12);
  f_apc_cas("ts", 12, 15);
  VS(f_apc_fetch("ts"), 15);
  f_apc_cas("ts", 12, 18);
  VS(f_apc_fetch("ts"), 15);
  return Count(true);
}

bool TestExtApc::test_apc_bin_dump() {
  try {
    f_apc_bin_dump();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApc::test_apc_bin_load() {
  try {
    f_apc_bin_load("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApc::test_apc_bin_dumpfile() {
  try {
    f_apc_bin_dumpfile(0, null, "");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApc::test_apc_bin_loadfile() {
  try {
    f_apc_bin_loadfile("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApc::test_apc_exists() {
  f_apc_store("ts", "TestString");
  VS(f_apc_exists("ts"), true);
  VS(f_apc_exists("TestString"), false);
  VS(f_apc_exists(CREATE_VECTOR2("ts", "TestString")), CREATE_VECTOR1("ts"));
  return Count(true);
}

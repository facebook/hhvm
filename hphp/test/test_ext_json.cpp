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

#include "hphp/test/test_ext_json.h"
#include "hphp/runtime/ext/ext_json.h"
#include "hphp/system/lib/systemlib.h"

///////////////////////////////////////////////////////////////////////////////

bool TestExtJson::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_json_encode);
  RUN_TEST(test_json_decode);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtJson::test_json_encode() {
  VS(f_json_encode(CREATE_MAP3("a", 1, "b", 2.3, 3, "test")),
     "{\"a\":1,\"b\":2.3,\"3\":\"test\"}");
  VS(f_json_encode(CREATE_VECTOR5("a", 1, true, false, uninit_null())),
     "[\"a\",1,true,false,null]");

  VS(f_json_encode("a\xE0"), "null");
  VS(f_json_encode("a\xE0", k_JSON_FB_LOOSE), "\"a?\"");

  VS(f_json_encode(CREATE_MAP2("0", "apple", "1", "banana")),
     "[\"apple\",\"banana\"]");

  VS(f_json_encode(CREATE_VECTOR1(CREATE_MAP1("a", "apple"))),
     "[{\"a\":\"apple\"}]");

  VS(f_json_encode(CREATE_VECTOR1(CREATE_MAP1("a", "apple")),
                   k_JSON_PRETTY_PRINT),
    "[\n    {\n        \"a\": \"apple\"\n    }\n]");

  VS(f_json_encode(CREATE_VECTOR4(1, 2, 3, CREATE_VECTOR3(1, 2, 3)),
                   k_JSON_PRETTY_PRINT),
    "[\n"
    "    1,\n"
    "    2,\n"
    "    3,\n"
    "    [\n"
    "        1,\n"
    "        2,\n"
    "        3\n"
    "    ]\n"
    "]");

  Array arr = CREATE_MAP3(
    "a", 1,
    "b", CREATE_VECTOR2(1, 2),
    "c", CREATE_MAP1("d", 42)
  );
  VS(f_json_encode(arr, k_JSON_PRETTY_PRINT),
    "{\n"
    "    \"a\": 1,\n"
    "    \"b\": [\n"
    "        1,\n"
    "        2\n"
    "    ],\n"
    "    \"c\": {\n"
    "        \"d\": 42\n"
    "    }\n"
    "}");

  return Count(true);
}

bool TestExtJson::test_json_decode() {
  Array arr = CREATE_MAP1("fbid", 101501853510151001LL);
  VS(f_json_decode(f_json_encode(arr), true), arr);

  VS(f_json_decode("{\"0\":{\"00\":0}}", true),
     CREATE_MAP1("0", CREATE_MAP1("00", 0)));

  VS(f_json_decode("{\"a\":1,\"b\":2.3,\"3\":\"test\"}", true),
     CREATE_MAP3("a", 1, "b", 2.3, 3, "test"));
  VS(f_json_decode("[\"a\",1,true,false,null]", true),
     CREATE_VECTOR5("a", 1, true, false, uninit_null()));

  Object obj = f_json_decode("{\"a\":1,\"b\":2.3,\"3\":\"test\"}");
  Object obj2(SystemLib::AllocStdClassObject());
  obj2->o_set("a", 1);
  obj2->o_set("b", 2.3);
  obj2->o_set("3", "test");
  VS(obj.toArray(), obj2.toArray());

  obj = f_json_decode("[\"a\",1,true,false,null]");
  VS(obj.toArray(), CREATE_VECTOR5("a", 1, true, false, uninit_null()));

  VS(f_json_decode("{z:1}",     true),       uninit_null());
  VS(f_json_decode("{z:1}",     true, k_JSON_FB_LOOSE), CREATE_MAP1("z", 1));
  VS(f_json_decode("{z:\"z\"}", true),       uninit_null());
  VS(f_json_decode("{z:\"z\"}", true, k_JSON_FB_LOOSE), CREATE_MAP1("z", "z"));
  VS(f_json_decode("{'x':1}",   true),       uninit_null());
  VS(f_json_decode("{'x':1}",   true, k_JSON_FB_LOOSE), CREATE_MAP1("x", 1));
  VS(f_json_decode("{y:1,}",    true),       uninit_null());
  VS(f_json_decode("{y:1,}",    true, k_JSON_FB_LOOSE), CREATE_MAP1("y", 1));
  VS(f_json_decode("{,}",       true),       uninit_null());
  VS(f_json_decode("{,}",       true, k_JSON_FB_LOOSE), uninit_null());
  VS(f_json_decode("[1,2,3,]",  true),       uninit_null());
  VS(f_json_decode("[1,2,3,]",  true, k_JSON_FB_LOOSE), CREATE_VECTOR3(1,2,3));
  VS(f_json_decode("[,]",       true),       uninit_null());
  VS(f_json_decode("[,]",       true, k_JSON_FB_LOOSE), uninit_null());
  VS(f_json_decode("[]",        true),       Array::Create());
  VS(f_json_decode("[]",        true, k_JSON_FB_LOOSE), Array::Create());
  VS(f_json_decode("{}",        true),       Array::Create());
  VS(f_json_decode("{}",        true, k_JSON_FB_LOOSE), Array::Create());

  VS(f_json_decode("[{\"a\":\"apple\"},{\"b\":\"banana\"}]", true),
     CREATE_VECTOR2(CREATE_MAP1("a", "apple"), CREATE_MAP1("b", "banana")));

  Variant a = "[{\"a\":[{\"n\":\"1st\"}]},{\"b\":[{\"n\":\"2nd\"}]}]";
  VS(f_json_decode(a, true),
     CREATE_VECTOR2
     (CREATE_MAP1("a", CREATE_VECTOR1(CREATE_MAP1("n", "1st"))),
      CREATE_MAP1("b", CREATE_VECTOR1(CREATE_MAP1("n", "2nd")))));

  return Count(true);
}

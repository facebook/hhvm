/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/test/ext/test_cpp_base.h"
#include "hphp/runtime/base/base-includes.h"
#include "hphp/util/logger.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/ext/ext_variable.h"
#include "hphp/runtime/ext/ext_apc.h"
#include "hphp/runtime/ext/ext_mysql.h"
#include "hphp/runtime/ext/ext_curl.h"
#include "hphp/runtime/base/shared-store-base.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/ip-block-map.h"
#include "hphp/test/ext/test_mysql_info.h"
#include "hphp/system/systemlib.h"
#include "hphp/runtime/ext/ext_string.h"

///////////////////////////////////////////////////////////////////////////////

TestCppBase::TestCppBase() {
}

///////////////////////////////////////////////////////////////////////////////

bool TestCppBase::RunTests(const std::string &which) {
  bool ret = true;
  RUN_TEST(TestString);
  RUN_TEST(TestArray);
  RUN_TEST(TestObject);
  RUN_TEST(TestVariant);
  RUN_TEST(TestIpBlockMap);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// building blocks

class Timer {
public:
  explicit Timer(const char *name = nullptr) {
    if (name) m_name = name;
    gettimeofday(&m_start, 0);
  }

  int64_t getMicroSeconds() {
    struct timeval end;
    gettimeofday(&end, 0);
    return (end.tv_sec - m_start.tv_sec) * 1000000 +
      (end.tv_usec - m_start.tv_usec);
  }

private:
  std::string m_name;
  struct timeval m_start;
};

class SomeClass {
public:
  SomeClass() : m_data(0) {}
  void dump() { printf("data: %d\n", m_data);}
  int m_data;
};

///////////////////////////////////////////////////////////////////////////////
// data types

bool TestCppBase::TestString() {
  // constructors
  {
    VS(String(15).c_str(), "15");
    VS(String(-15).c_str(), "-15");
    VS(String(int64_t(12345678912345678LL)).c_str(), "12345678912345678");
    VS(String(int64_t(-12345678912345678LL)).c_str(), "-12345678912345678");
    VS(String(5.603).c_str(), "5.603");
    VS(String("test").c_str(), "test");
    VS(String(String("test")).c_str(), "test");
  }

  // informational
  {
    VERIFY(String().isNull());
    VERIFY(!String("").isNull());
    VERIFY(String().empty());
    VERIFY(String("").empty());
    VERIFY(!String("test").empty());
    VERIFY(String().size() == 0);
    VERIFY(String().length() == 0);
    VERIFY(String("").size() == 0);
    VERIFY(String("").length() == 0);
    VERIFY(String("test").size() == 4);
    VERIFY(String("test").length() == 4);
    VERIFY(!String("2test").isNumeric());
    VERIFY(!String("2test").isInteger());
    VERIFY(!String("test").isNumeric());
    VERIFY(!String("test").isInteger());
    VERIFY(String("23").isNumeric());
    VERIFY(String("23").isInteger());
    VERIFY(String("23.3").isNumeric());
    VERIFY(!String("23.3").isInteger());
  }

  // operators
  {
    String s;
    s = "test1";                   VS(s.c_str(), "test1");
    s = String("test2");           VS(s.c_str(), "test2");
    s = Variant("test3");          VS(s.c_str(), "test3");
    s = String("a") + "b";         VS(s.c_str(), "ab");
    s = String("c") + String("d"); VS(s.c_str(), "cd");
    s += "efg";                    VS(s.c_str(), "cdefg");
    s += String("hij");            VS(s.c_str(), "cdefghij");
  }

  // manipulations
  {
    String s = f_strtolower("Test");
    VS(s.c_str(), "test");
  }

  // conversions
  {
    VERIFY(!String().toBoolean());
    VERIFY(String("123").toBoolean());
    VERIFY(String("123").toByte() == 123);
    VERIFY(String("32767").toInt16() == 32767);
    VERIFY(String("1234567890").toInt32() == 1234567890);
    VERIFY(String("123456789012345678").toInt64() == 123456789012345678LL);
    VERIFY(String("123.45").toDouble() == 123.45);
  }

  return Count(true);
}

static const StaticString s_n0("n0");
static const StaticString s_n1("n1");
static const StaticString s_n2("n2");
static const StaticString s_A("A");
static const StaticString s_name("name");
static const StaticString s_1("1");

bool TestCppBase::TestArray() {
  // Array::Create(), Array constructors and informational
  {
    Array arr;
    VERIFY(arr.empty()); VERIFY(arr.size() == 0); VERIFY(arr.length() == 0);
    VERIFY(arr.isNull());

    arr = Array::Create();
    VERIFY(arr.empty()); VERIFY(arr.size() == 0); VERIFY(arr.length() == 0);
    VERIFY(!arr.isNull());

    arr = Array::Create(0);
    VERIFY(!arr.empty()); VERIFY(arr.size() == 1); VERIFY(arr.length() == 1);
    VERIFY(!arr.isNull());
    VERIFY(arr[0].toInt32() == 0);
    VS(arr, Array(ArrayInit(1).set(0).create()));

    arr = Array::Create("test");
    VERIFY(!arr.empty()); VERIFY(arr.size() == 1); VERIFY(arr.length() == 1);
    VERIFY(!arr.isNull());
    VERIFY(equal(arr[0], String("test")));
    VS(arr, Array(ArrayInit(1).set("test").create()));

    Array arrCopy = arr;
    arr = Array::Create(arr);
    VERIFY(!arr.empty()); VERIFY(arr.size() == 1); VERIFY(arr.length() == 1);
    VERIFY(!arr.isNull());
    VERIFY(arr[0].toArray().size() == 1);
    VS(arr[0], arrCopy);
    VS(arr, Array(ArrayInit(1).set(arrCopy).create()));

    arr = Array::Create("name", 1);
    VERIFY(!arr.empty()); VERIFY(arr.size() == 1); VERIFY(arr.length() == 1);
    VERIFY(!arr.isNull());
    VERIFY(arr[s_name].toInt32() == 1);
    VS(arr, Array(ArrayInit(1).set(s_name, 1).create()));

    arr = Array::Create(s_name, "test");
    VERIFY(!arr.empty()); VERIFY(arr.size() == 1); VERIFY(arr.length() == 1);
    VERIFY(!arr.isNull());
    VERIFY(equal(arr[s_name], String("test")));
    VS(arr, Array(ArrayInit(1).set(s_name, "test").create()));

    arrCopy = arr;
    arr = Array::Create(s_name, arr);
    VERIFY(!arr.empty()); VERIFY(arr.size() == 1); VERIFY(arr.length() == 1);
    VERIFY(!arr.isNull());
    VS(arr[s_name], arrCopy);
    VERIFY(arr[s_name].toArray().size() == 1);
    VS(arr, Array(ArrayInit(1).set(s_name, arrCopy).create()));
  }

  // iteration
  {
    Array arr = make_map_array("n1", "v1", "n2", "v2");
    int i = 0;
    for (ArrayIter iter = arr.begin(); iter; ++iter, ++i) {
      if (i == 0) {
        VERIFY(equal(iter.first(), String("n1")));
        VERIFY(equal(iter.second(), String("v1")));
      } else {
        VERIFY(equal(iter.first(), String("n2")));
        VERIFY(equal(iter.second(), String("v2")));
      }
    }
    VERIFY(i == 2);
  }
  /* TODO: fix this
  {
    Variant arr = make_map_array("n1", "v1", "n2", "v2");
    arr.escalate();
    for (ArrayIter iter = arr.begin(arr, true); !iter->end(); iter->next()){
      arr.lvalAt(iter->first()).reset();
    }
    VS(arr, Array::Create());
  }
  */

  static const StaticString s_Array("Array");

  // conversions
  {
    Array arr0;
    VERIFY(arr0.toBoolean() == false);
    VERIFY(arr0.toByte() == 0);
    VERIFY(arr0.toInt16() == 0);
    VERIFY(arr0.toInt32() == 0);
    VERIFY(arr0.toInt64() == 0);
    VERIFY(arr0.toDouble() == 0.0);
    VERIFY(arr0.toString()->empty());

    Array arr1 = Array::Create("test");
    VERIFY(arr1.toBoolean() == true);
    VERIFY(arr1.toByte() == 1);
    VERIFY(arr1.toInt16() == 1);
    VERIFY(arr1.toInt32() == 1);
    VERIFY(arr1.toInt64() == 1);
    VERIFY(arr1.toDouble() == 1.0);
    VERIFY(arr1.toString() == s_Array);
  }

  // offset
  {
    Array arr;
    arr.set(0, "v1");
    arr.set(1, "v2");
    VS(arr, make_packed_array("v1", "v2"));
  }
  {
    Array arr;
    arr.set(s_n1, "v1");
    arr.set(s_n2, "v2");
    VS(arr, make_map_array("n1", "v1", "n2", "v2"));
  }
  {
    Array arr;
    arr.lvalAt(0) = String("v1");
    arr.lvalAt(1) = String("v2");
    VS(arr, make_packed_array("v1", "v2"));
  }
  {
    Array arr;
    arr.lvalAt(s_n1) = String("v1");
    arr.lvalAt(s_n2) = String("v2");
    VS(arr, make_map_array("n1", "v1", "n2", "v2"));
  }
  {
    Array arr;
    Variant name = "name";
    arr.lvalAt(name) = String("value");
    VS(arr, make_map_array("name", "value"));
  }

  {
    Array arr;
    arr.lvalAt(1) = 10;
    VS(arr[1], 10);
    VS(arr[Variant(1.5)], 10);
    VS(arr[s_1], 10);
    VS(arr[Variant("1")], 10);
  }
  {
    Array arr;
    arr.lvalAt(Variant(1.5)) = 10;
    VS(arr[1], 10);
    VS(arr[Variant(1.5)], 10);
    VS(arr[s_1], 10);
    VS(arr[Variant("1")], 10);
  }
  {
    Array arr;
    arr.lvalAt(s_1) = 10;
    VS(arr[1], 10);
    VS(arr[Variant(1.5)], 10);
    VS(arr[s_1], 10);
    VS(arr[Variant("1")], 10);
  }
  {
    Array arr;
    arr.lvalAt(Variant("1")) = 10;
    VS(arr[1], 10);
    VS(arr[Variant(1.5)], 10);
    VS(arr[s_1], 10);
    VS(arr[Variant("1")], 10);
  }

  // membership
  {
    Array arr;
    arr.lvalAt(0) = String("v1");
    arr.lvalAt(1) = String("v2");
    VERIFY(arr.exists(0));
    arr.remove(0);
    VERIFY(!arr.exists(0));
    VS(arr, Array::Create(1, "v2"));
    arr.append("v3");
    VS(arr, make_map_array(1, "v2", 2, "v3"));
  }
  {
    static const StaticString s_0("0");
    Array arr;
    arr.lvalAt(0) = String("v1");
    VERIFY(arr.exists(0));
    arr.remove(String(s_0));
    VERIFY(!arr.exists(0));
  }
  {
    Array arr;
    arr.lvalAt(0) = String("v1");
    VERIFY(arr.exists(0));
    arr.remove(Variant("0"));
    VERIFY(!arr.exists(0));
  }
  {
    Array arr;
    arr.lvalAt(0) = String("v1");
    VERIFY(arr.exists(0));
    arr.remove(Variant(Variant("0")));
    VERIFY(!arr.exists(0));
  }
  {
    Array arr;
    arr.lvalAt(0) = String("v1");
    VERIFY(arr.exists(0));
    arr.remove(Variant(Variant(0.5)));
    VERIFY(!arr.exists(0));
  }
  {
    Array arr;
    arr.lvalAt(Variant()) = 123;
    VERIFY(arr.exists(empty_string));
    arr.remove(Variant());
    VERIFY(!arr.exists(empty_string));
  }
  {
    Array arr;
    arr.lvalAt(s_n1) = String("v1");
    arr.lvalAt(s_n2) = String("v2");
    VERIFY(arr.exists(s_n1));
    arr.remove(s_n1);
    VERIFY(!arr.exists(s_n1));
    VS(arr, Array::Create(s_n2, "v2"));
    arr.append("v3");
    VS(arr, make_map_array("n2", "v2", 0, "v3"));
  }
  {
    Array arr;
    arr.lvalAt() = String("test");
    VS(arr, make_packed_array("test"));
  }
  {
    Array arr;
    arr.lvalAt(s_name) = String("value");
    VERIFY(arr.exists(s_name));
  }
  {
    Array arr;
    arr.lvalAt(1) = String("value");
    VERIFY(arr.exists(1));
    VERIFY(arr.exists(s_1));
    VERIFY(arr.exists(Variant("1")));
    VERIFY(arr.exists(Variant(1)));
    VERIFY(arr.exists(Variant(1.5)));
  }
  {
    Array arr;
    arr.lvalAt(s_1) = String("value");
    VERIFY(arr.exists(1));
    VERIFY(arr.exists(s_1));
    VERIFY(arr.exists(Variant("1")));
    VERIFY(arr.exists(Variant(1)));
    VERIFY(arr.exists(Variant(1.5)));
  }
  {
    Array arr;
    arr.lvalAt(Variant(1.5)) = String("value");
    VERIFY(arr.exists(1));
    VERIFY(arr.exists(s_1));
    VERIFY(arr.exists(Variant("1")));
    VERIFY(arr.exists(Variant(1)));
    VERIFY(arr.exists(Variant(1.5)));
  }
  {
    Array arr;
    arr.lvalAt(Variant("1")) = String("value");
    VERIFY(arr.exists(1));
    VERIFY(arr.exists(s_1));
    VERIFY(arr.exists(Variant("1")));
    VERIFY(arr.exists(Variant(1)));
    VERIFY(arr.exists(Variant(1.5)));
  }

  // merge
  {
    Array arr = Array::Create(0) + Array::Create(1);
    VS(arr, Array::Create(0));
    arr += make_packed_array(0, 1);
    VS(arr, make_packed_array(0, 1));

    arr = Array::Create(0).merge(Array::Create(1));
    VS(arr, make_packed_array(0, 1));
    arr = arr.merge(make_packed_array(0, 1));
    VS(arr, make_packed_array(0, 1, 0, 1));

    arr = Array::Create("s0").merge(Array::Create("s1"));
    VS(arr, make_packed_array("s0", "s1"));

    arr = Array::Create("n0", "s0") + Array::Create("n1", "s1");
    VS(arr, make_map_array("n0", "s0", "n1", "s1"));
    arr += make_map_array("n0", "s0", "n1", "s1");
    VS(arr, make_map_array("n0", "s0", "n1", "s1"));

    arr = Array::Create("n0", "s0").merge(Array::Create("n1", "s1"));
    VS(arr, make_map_array("n0", "s0", "n1", "s1"));
    Array arrX = make_map_array("n0", "s2", "n1", "s3");
    arr = arr.merge(arrX);
    VS(arr, make_map_array("n0", "s2", "n1", "s3"));
  }

  // slice
  {
    Array arr = make_packed_array("test1", "test2");
    Array sub = arr.slice(1, 1, true);
    VS(sub, make_map_array(1, "test2"));
  }
  {
    Array arr = make_packed_array("test1", "test2");
    Array sub = arr.slice(1, 1, false);
    VS(sub, make_packed_array("test2"));
  }
  {
    Array arr = make_map_array("n1", "test1", "n2", "test2");
    Array sub = arr.slice(1, 1, true);
    VS(sub, make_map_array("n2", "test2"));
  }
  {
    Array arr = make_map_array("n1", "test1", "n2", "test2");
    Array sub = arr.slice(1, 1, false);
    VS(sub, make_map_array("n2", "test2"));
  }

  // escalation
  {
    Array arr;
    arr.lvalAt(0).lvalAt(0) = 1.2;
    VS(arr, make_packed_array(make_packed_array(1.2)));
  }
  {
    Array arr;
    arr.lvalAt(s_name).lvalAt(0) = 1.2;
    VS(arr, make_map_array(s_name, make_packed_array(1.2)));
  }
  {
    Array arr = Array::Create();
    arr.lvalAt(0).lvalAt(0) = 1.2;
    VS(arr, make_packed_array(make_packed_array(1.2)));
  }
  {
    Array arr = Array::Create();
    arr.lvalAt(s_name).lvalAt(0) = 1.2;
    VS(arr, make_map_array(s_name, make_packed_array(1.2)));
  }
  {
    Array arr = Array::Create("test");
    arr.lvalAt(0) = make_packed_array(1.2);
    VS(arr, make_packed_array(make_packed_array(1.2)));
  }
  {
    Array arr = Array::Create("test");
    arr.lvalAt(s_name).lvalAt(0) = 1.2;
    VS(arr, make_map_array(0, "test", s_name, make_packed_array(1.2)));
  }
  {
    Array arr = Array::Create();
    arr.append("apple");
    arr.set(2, "pear");
    VS(arr[2], "pear");
  }

  {
    Array arr = make_map_array(0, "a", 1, "b");
    VERIFY(arr->isVectorData());
  }
  {
    Array arr = make_map_array(1, "a", 0, "b");
    VERIFY(!arr->isVectorData());
  }
  {
    Array arr = make_map_array(1, "a", 2, "b");
    VERIFY(!arr->isVectorData());
  }
  {
    Array arr = make_map_array(1, "a");
    arr.set(0, "b");
    VERIFY(!arr->isVectorData());
  }

  return Count(true);
}

bool TestCppBase::TestObject() {
  {
    String s = "O:1:\"B\":1:{s:3:\"obj\";O:1:\"A\":1:{s:1:\"a\";i:10;}}";
    Variant v = unserialize_from_string(s);
    VERIFY(v.isObject());
    auto o = v.toObject();
    VS(o->o_getClassName(), "__PHP_Incomplete_Class");
    auto os = f_serialize(o);
    VS(os, "O:1:\"B\":1:{s:3:\"obj\";O:1:\"A\":1:{s:1:\"a\";i:10;}}");
  }
  return Count(true);
}

bool TestCppBase::TestVariant() {
  // conversions
  {
    Variant v("123");
    VERIFY(v.toInt32() == 123);
  }

  // offset
  {
    Variant v = "test";
    VS(v.rvalAt(0), "t");
  }
  {
    Variant v;
    v.lvalAt(0) = String("v0");
    v.lvalAt(1) = String("v1");
    VERIFY(equal(v[0], String("v0")));
    VERIFY(equal(v[1], String("v1")));
  }
  {
    Variant v;
    v.lvalAt() = String("test");
    VS(v, make_packed_array("test"));
  }
  {
    Variant v;
    v.lvalAt(1) = String("test");
    VS(v[1], "test");
    VS(v[Variant(1.5)], "test");
    VS(v[s_1], "test");
    VS(v[Variant("1")], "test");
  }
  {
    Variant v;
    v.lvalAt(Variant(1.5)) = String("test");
    VS(v[1], "test");
    VS(v[Variant(1.5)], "test");
    VS(v[s_1], "test");
    VS(v[Variant("1")], "test");
  }
  {
    Variant v;
    v.lvalAt(s_1) = String("test");
    VS(v[1], "test");
    VS(v[Variant(1.5)], "test");
    VS(v[s_1], "test");
    VS(v[Variant("1")], "test");
  }
  {
    Variant v;
    v.lvalAt(Variant("1")) = String("test");
    VS(v[1], "test");
    VS(v[Variant(1.5)], "test");
    VS(v[s_1], "test");
    VS(v[Variant("1")], "test");
  }
  {
    Variant v = "asd";
    v.set(0, "b");
    VS(v, "bsd");
    v.set(6, "c");
    VS(v, "bsd   c");
  }

  // membership
  {
    Variant v;
    v.lvalAt(s_n0) = String("v0");
    v.lvalAt(s_n1) = String("v1");
    v.remove(s_n1);
    VS(v, make_map_array(s_n0, "v0"));
    v.append("v2");
    VS(v, make_map_array(s_n0, "v0", 0, "v2"));
  }
  {
    Variant v;
    v.lvalAt(s_n0) = String("v0");
    v.lvalAt(1) = String("v1");
    v.remove(Variant(1.5));
    VS(v, make_map_array("n0", "v0"));
  }
  {
    Variant v;
    v.lvalAt(s_n0) = String("v0");
    v.lvalAt(1) = String("v1");
    v.remove(Variant("1"));
    VS(v, make_map_array("n0", "v0"));
  }
  {
    Variant v;
    v.lvalAt(s_n0) = String("v0");
    v.lvalAt(1) = String("v1");
    v.remove(String("1"));
    VS(v, make_map_array("n0", "v0"));
  }
  {
    Variant v;
    v.lvalAt(s_n0) = String("v0");
    v.lvalAt(empty_string) = String("v1");
    v.remove(Variant());
    VS(v, make_map_array("n0", "v0"));
  }

  // references
  {
    Variant v1("original");
    Variant v2 = v1;
    v2 = String("changed");
    VERIFY(equal(v1, String("original")));
  }
  {
    Variant v1("original");
    Variant v2 = strongBind(v1);
    v2 = String("changed");
    VERIFY(equal(v1, String("changed")));
  }
  {
    Variant v1 = 10;
    Variant v2 = Array(ArrayInit(1).setRef(v1).create());
    v1 = 20;
    VS(v2[0], 20);
  }
  {
    Variant v1 = 10;
    Variant v2;
    v2.lvalAt() = ref(v1);
    v1 = 20;
    VS(v2[0], 20);
  }
  {
    Variant v1 = 10;
    Variant v2 = make_packed_array(5);
    v2.lvalAt() = ref(v1);
    v1 = 20;
    VS(v2[1], 20);
  }

  // array escalation
  {
    Variant arr;
    arr.lvalAt(0).lvalAt(0) = 1.2;
    VS(arr, make_packed_array(make_packed_array(1.2)));
  }
  {
    Variant arr;
    arr.lvalAt(s_name).lvalAt(0) = 1.2;
    VS(arr, make_map_array(s_name, make_packed_array(1.2)));
  }
  {
    Variant arr = Array::Create();
    arr.lvalAt(0).lvalAt(0) = 1.2;
    VS(arr, make_packed_array(make_packed_array(1.2)));
  }
  {
    Variant arr = Array::Create();
    arr.lvalAt(s_name).lvalAt(0) = 1.2;
    VS(arr, make_map_array(s_name, make_packed_array(1.2)));
  }
  {
    Variant arr = Array::Create("test");
    arr.lvalAt(0) = make_packed_array(1.2);
    VS(arr, make_packed_array(make_packed_array(1.2)));
  }
  {
    Variant arr = Array::Create("test");
    arr.lvalAt(s_name).lvalAt(0) = 1.2;
    VS(arr, make_map_array(0, "test", s_name, make_packed_array(1.2)));
  }

  return Count(true);
}

///////////////////////////////////////////////////////////////////////////////

/* Pull 32bit Big Endian words from an in6_addr */
static inline long in6addrWord(struct in6_addr addr, char wordNo) {
  return ((addr.s6_addr[(wordNo*4)+0] << 24) |
          (addr.s6_addr[(wordNo*4)+1] << 16) |
          (addr.s6_addr[(wordNo*4)+2] <<  8) |
          (addr.s6_addr[(wordNo*4)+3] <<  0)) & 0xFFFFFFFF;
}

bool TestCppBase::TestIpBlockMap() {
  struct in6_addr addr;
  int bits;

  VERIFY(IpBlockMap::ReadIPv6Address("204.15.21.0/22", &addr, bits));
  VS(bits, 118);
  VS(in6addrWord(addr, 0), 0x00000000L);
  VS(in6addrWord(addr, 1), 0x00000000L);
  VS(in6addrWord(addr, 2), 0x0000FFFFL);
  VS(in6addrWord(addr, 3), 0xCC0F1500L);

  VERIFY(IpBlockMap::ReadIPv6Address("127.0.0.1", &addr, bits));
  VS(bits, 128);
  VS(in6addrWord(addr, 0), 0x00000000L);
  VS(in6addrWord(addr, 1), 0x00000000L);
  VS(in6addrWord(addr, 2), 0x0000FFFFL);
  VS(in6addrWord(addr, 3), 0x7F000001L);

  VERIFY(IpBlockMap::ReadIPv6Address(
    "1111:2222:3333:4444:5555:6666:789a:bcde", &addr, bits));
  VS(bits, 128);
  VS(in6addrWord(addr, 0), 0x11112222L);
  VS(in6addrWord(addr, 1), 0x33334444L);
  VS(in6addrWord(addr, 2), 0x55556666L);
  VS(in6addrWord(addr, 3), 0x789abcdeL);

  VERIFY(IpBlockMap::ReadIPv6Address(
    "1111:2222:3333:4444:5555:6666:789a:bcde/68", &addr, bits));
  VS(bits, 68);
  VS(in6addrWord(addr, 0), 0x11112222L);
  VS(in6addrWord(addr, 1), 0x33334444L);
  VS(in6addrWord(addr, 2), 0x55556666L);
  VS(in6addrWord(addr, 3), 0x789abcdeL);

  IpBlockMap::BinaryPrefixTrie root(true);
  unsigned char value[16];

  // Default value with no additional nodes
  memset(value, 0, 16);
  VERIFY(root.isAllowed(value, 1));
  value[0] = 0x80;
  VERIFY(root.isAllowed(value));

  // Inheritance of parent allow value through multiple levels of new nodes
  IpBlockMap::BinaryPrefixTrie::InsertNewPrefix(&root, value, 1, false);
  value[0] = 0xf0;
  IpBlockMap::BinaryPrefixTrie::InsertNewPrefix(&root, value, 4, true);
  VERIFY(root.isAllowed(value));
  value[0] = 0xe0;
  VERIFY(!root.isAllowed(value));
  value[0] = 0xc0;
  VERIFY(!root.isAllowed(value));
  value[0] = 0x80;
  VERIFY(!root.isAllowed(value));
  value[0] = 0;
  VERIFY(root.isAllowed(value));

  // > 1 byte in address
  value[2] = 0xff;
  IpBlockMap::BinaryPrefixTrie::InsertNewPrefix(&root, value, 24, false);
  VERIFY(!root.isAllowed(value));
  value[3] = 0xff;
  VERIFY(!root.isAllowed(value));
  value[2] = 0xfe;
  VERIFY(root.isAllowed(value));

  // Exact address match
  value[2]  = 0xff;
  value[15] = 1;
  IpBlockMap::BinaryPrefixTrie::InsertNewPrefix(&root, value, 128, true);
  VERIFY(root.isAllowed(value));

  Hdf hdf;
  hdf.fromString(
    "  0 {\n"
    "    Location = /test\n"
    "    AllowFirst = true\n"
    "    Ip {\n"
    "      Allow {\n"
    "       * = 127.0.0.1\n"
    "     }\n"
    "     Deny {\n"
    "       * = 8.32.0.0/24\n"
    "       * = aaaa:bbbb:cccc:dddd:eeee:ffff:1111::/80\n"
    "     }\n"
    "    }\n"
    "  }\n"
  );

  IpBlockMap ibm(hdf);
  VERIFY(!ibm.isBlocking("test/blah.php", "127.0.0.1"));
  VERIFY(ibm.isBlocking("test/blah.php", "8.32.0.104"));
  VERIFY(ibm.isBlocking("test/blah.php",
                        "aaaa:bbbb:cccc:dddd:eeee:9999:8888:7777"));
  VERIFY(!ibm.isBlocking("test/blah.php",
                         "aaaa:bbbb:cccc:dddd:eee3:4444:3333:2222"));

  return Count(true);
}

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

#include <test/test_cpp_base.h>
#include <runtime/base/base_includes.h>
#include <util/logger.h>
#include <runtime/base/memory/memory_manager.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/ext/ext_apc.h>
#include <runtime/ext/ext_mysql.h>
#include <runtime/ext/ext_curl.h>
#include <runtime/base/shared/shared_store_base.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/ip_block_map.h>
#include <test/test_mysql_info.inc>
#include <system/lib/systemlib.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

TestCppBase::TestCppBase() {
}

///////////////////////////////////////////////////////////////////////////////

bool TestCppBase::RunTests(const std::string &which) {
  bool ret = true;
  RUN_TEST(TestSmartAllocator);
  RUN_TEST(TestString);
  RUN_TEST(TestArray);
  RUN_TEST(TestObject);
  RUN_TEST(TestVariant);
#ifndef DEBUGGING_SMART_ALLOCATOR
  RUN_TEST(TestMemoryManager);
#endif
  RUN_TEST(TestIpBlockMap);
  RUN_TEST(TestEqualAsStr);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// building blocks

class Timer {
public:
  Timer(const char *name = NULL) {
    if (name) m_name = name;
    gettimeofday(&m_start, 0);
  }

  int64 getMicroSeconds() {
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
  bool calculate(int &size) { return false;}
  void backup(LinearAllocator &allocator) {}
  void restore(const char *&data) {}
  void sweep() {}
  void dump() { printf("data: %d\n", m_data);}
  int m_data;
};

static StaticString s_TestResource("TestResource");

class TestResource : public ResourceData {
public:
  // overriding ResourceData
  CStrRef o_getClassName() const { return s_TestResource; }
};

typedef SmartAllocator<SomeClass, -1, SmartAllocatorImpl::NoCallbacks>
        SomeClassAlloc;

bool TestCppBase::TestSmartAllocator() {
  int iMax = 1000000;
  int64 time1, time2;
  {
    static IMPLEMENT_THREAD_LOCAL(SomeClassAlloc, allocator);

    Timer t;
    for (int i = 0; i < iMax; i++) {
      SomeClass *obj = new (allocator.get()) SomeClass();
      allocator.get()->dealloc(obj);
    }
    time1 = t.getMicroSeconds();
    if (!Test::s_quiet) {
      printf("SmartAlloctor: %lld us\n", time1);
    }
  }
  {
    Timer t;
    for (int i = 0; i < iMax; i++) {
      SomeClass *obj = new SomeClass();
      delete obj;
    }
    time2 = t.getMicroSeconds();
    if (!Test::s_quiet) {
      printf("malloc/free: %lld us\n", time2);
    }
  }
  return Count(true);
}

///////////////////////////////////////////////////////////////////////////////
// data types

bool TestCppBase::TestString() {
  // constructors
  {
    VS((const char *)String(15), "15");
    VS((const char *)String(-15), "-15");
    VS((const char *)String(12345678912345678LL), "12345678912345678");
    VS((const char *)String(-12345678912345678LL), "-12345678912345678");
    VS((const char *)String(5.603), "5.603");
    VS((const char *)String("test"), "test");
    VS((const char *)String(String("test")), "test");
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
    VERIFY(!String("2test").isValidVariableName());
    VERIFY(!String("test").isNumeric());
    VERIFY(!String("test").isInteger());
    VERIFY(String("test").isValidVariableName());
    VERIFY(String("23").isNumeric());
    VERIFY(String("23").isInteger());
    VERIFY(String("23.3").isNumeric());
    VERIFY(!String("23.3").isInteger());
  }

  // operators
  {
    String s;
    s = "test1";                   VS((const char *)s, "test1");
    s = String("test2");           VS((const char *)s, "test2");
    s = Variant("test3");          VS((const char *)s, "test3");
    s = String("a") + "b";         VS((const char *)s, "ab");
    s = String("c") + String("d"); VS((const char *)s, "cd");
    s += "efg";                    VS((const char *)s, "cdefg");
    s += String("hij");            VS((const char *)s, "cdefghij");

    s = String("\x50\x51") | "\x51\x51"; VS((const char *)s, "\x51\x51");
    s = String("\x50\x51") & "\x51\x51"; VS((const char *)s, "\x50\x51");
    s = String("\x50\x51") ^ "\x51\x51"; VS((const char *)s, "\x01");
    s = "\x50\x51"; s |= "\x51\x51";     VS((const char *)s, "\x51\x51");
    s = "\x50\x51"; s &= "\x51\x51";     VS((const char *)s, "\x50\x51");
    s = "\x50\x51"; s ^= "\x51\x51";     VS((const char *)s, "\x01");
    s = "\x50\x51"; s = ~s;              VS((const char *)s, "\xAF\xAE");
  }

  // manipulations
  {
    String s = StringUtil::ToLower("Test");
    VS((const char *)s, "test");
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

  // offset
  {
    VS((const char *)String("test").rvalAt(2), "s");
    String s = "test";
    s.lvalAt(2) = "";
    VS((const char *)s, "tet");
    s.lvalAt(2) = "zz";
    VS((const char *)s, "tez");
    s.lvalAt(4) = "q";
    VS((const char *)s, "tez q");
  }

  return Count(true);
}

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
    VERIFY((int)arr[0] == 0);
    VS(arr, Array(ArrayInit(1, true).set(0).create()));

    arr = Array::Create("test");
    VERIFY(!arr.empty()); VERIFY(arr.size() == 1); VERIFY(arr.length() == 1);
    VERIFY(!arr.isNull());
    VERIFY(arr[0] == "test");
    VS(arr, Array(ArrayInit(1, true).set("test").create()));

    Array arrCopy = arr;
    arr = Array::Create(arr);
    VERIFY(!arr.empty()); VERIFY(arr.size() == 1); VERIFY(arr.length() == 1);
    VERIFY(!arr.isNull());
    VERIFY(arr[0].toArray().size() == 1);
    VS(arr[0], arrCopy);
    VS(arr, Array(ArrayInit(1, true).set(arrCopy).create()));

    arr = Array::Create("name", 1);
    VERIFY(!arr.empty()); VERIFY(arr.size() == 1); VERIFY(arr.length() == 1);
    VERIFY(!arr.isNull());
    VERIFY((int)arr["name"] == 1);
    VS(arr, Array(ArrayInit(1, false).set("name", 1).create()));

    arr = Array::Create("name", "test");
    VERIFY(!arr.empty()); VERIFY(arr.size() == 1); VERIFY(arr.length() == 1);
    VERIFY(!arr.isNull());
    VERIFY(arr["name"] == "test");
    VS(arr, Array(ArrayInit(1, false).set("name", "test").create()));

    arrCopy = arr;
    arr = Array::Create("name", arr);
    VERIFY(!arr.empty()); VERIFY(arr.size() == 1); VERIFY(arr.length() == 1);
    VERIFY(!arr.isNull());
    VS(arr["name"], arrCopy);
    VERIFY(arr["name"].toArray().size() == 1);
    VS(arr, Array(ArrayInit(1, false).set("name", arrCopy).create()));
  }

  // iteration
  {
    Array arr = CREATE_MAP2("n1", "v1", "n2", "v2");
    int i = 0;
    for (ArrayIter iter = arr.begin(); iter; ++iter, ++i) {
      if (i == 0) {
        VERIFY(iter.first() == "n1");
        VERIFY(iter.second() == "v1");
      } else {
        VERIFY(iter.first() == "n2");
        VERIFY(iter.second() == "v2");
      }
    }
    VERIFY(i == 2);
  }
  {
    Variant arr = CREATE_MAP1("n1", "v1");
    arr.escalate(true);
    Variant k, v;
    for (MutableArrayIter iter = arr.begin(&k, v); iter.advance();) {
      arr.weakRemove(k);
    }
    VS(arr, Array::Create());
  }
  /* TODO: fix this
  {
    Variant arr = CREATE_MAP2("n1", "v1", "n2", "v2");
    arr.escalate();
    for (ArrayIterPtr iter = arr.begin(arr, true); !iter->end(); iter->next()){
      unset(arr.lvalAt(iter->first()));
    }
    VS(arr, Array::Create());
  }
  */

  // conversions
  {
    Array arr0;
    VERIFY(arr0.toBoolean() == false);
    VERIFY(arr0.toByte() == 0);
    VERIFY(arr0.toInt16() == 0);
    VERIFY(arr0.toInt32() == 0);
    VERIFY(arr0.toInt64() == 0);
    VERIFY(arr0.toDouble() == 0.0);
    VERIFY(arr0.toString() == "");

    Array arr1 = Array::Create("test");
    VERIFY(arr1.toBoolean() == true);
    VERIFY(arr1.toByte() == 1);
    VERIFY(arr1.toInt16() == 1);
    VERIFY(arr1.toInt32() == 1);
    VERIFY(arr1.toInt64() == 1);
    VERIFY(arr1.toDouble() == 1.0);
    VERIFY(arr1.toString() == "Array");
  }

  // offset
  {
    Array arr;
    arr.set(0, "v1");
    arr.set(1, "v2");
    VS(arr, CREATE_VECTOR2("v1", "v2"));
  }
  {
    Array arr;
    arr.set("n1", "v1");
    arr.set("n2", "v2");
    VS(arr, CREATE_MAP2("n1", "v1", "n2", "v2"));
  }
  {
    Array arr;
    arr.lvalAt(0) = "v1";
    arr.lvalAt(1) = "v2";
    VS(arr, CREATE_VECTOR2("v1", "v2"));
  }
  {
    Array arr;
    arr.lvalAt("n1") = "v1";
    arr.lvalAt("n2") = "v2";
    VS(arr, CREATE_MAP2("n1", "v1", "n2", "v2"));
  }
  {
    Array arr;
    Variant name = "name";
    arr.lvalAt(name) = "value";
    VS(arr, CREATE_MAP1("name", "value"));
  }
  {
    Array arr;
    arr.lvalAt("A") = 10;
    arr.lvalAt("A")++;
    VS(arr["A"], 11);
  }
  {
    Array arr;
    arr.lvalAt(1) = 10;
    VS(arr[1], 10);
    VS(arr[1.5], 10);
    VS(arr[Variant(1.5)], 10);
    VS(arr["1"], 10);
    VS(arr[Variant("1")], 10);
  }
  {
    Array arr;
    arr.lvalAt(Variant(1.5)) = 10;
    VS(arr[1], 10);
    VS(arr[1.5], 10);
    VS(arr[Variant(1.5)], 10);
    VS(arr["1"], 10);
    VS(arr[Variant("1")], 10);
  }
  {
    Array arr;
    arr.lvalAt("1") = 10;
    VS(arr[1], 10);
    VS(arr[1.5], 10);
    VS(arr[Variant(1.5)], 10);
    VS(arr["1"], 10);
    VS(arr[Variant("1")], 10);
  }
  {
    Array arr;
    arr.lvalAt(Variant("1")) = 10;
    VS(arr[1], 10);
    VS(arr[1.5], 10);
    VS(arr[Variant(1.5)], 10);
    VS(arr["1"], 10);
    VS(arr[Variant("1")], 10);
  }

  // membership
  {
    Array arr;
    arr.lvalAt(0) = "v1";
    arr.lvalAt(1) = "v2";
    VERIFY(arr.exists(0));
    arr.remove(0);
    VERIFY(!arr.exists(0));
    VS(arr, Array::Create(1, "v2"));
    arr.append("v3");
    VS(arr, CREATE_MAP2(1, "v2", 2, "v3"));
  }
  {
    Array arr;
    arr.lvalAt(0) = "v1";
    VERIFY(arr.exists(0));
    arr.remove("0");
    VERIFY(!arr.exists(0));
  }
  {
    Array arr;
    arr.lvalAt(0) = "v1";
    VERIFY(arr.exists(0));
    arr.remove(Variant("0"));
    VERIFY(!arr.exists(0));
  }
  {
    Array arr;
    arr.lvalAt(0) = "v1";
    VERIFY(arr.exists(0));
    arr.remove(Variant(Variant("0")));
    VERIFY(!arr.exists(0));
  }
  {
    Array arr;
    arr.lvalAt(0) = "v1";
    VERIFY(arr.exists(0));
    arr.remove(Variant(Variant(0.5)));
    VERIFY(!arr.exists(0));
  }
  {
    Array arr;
    arr.lvalAt(Variant()) = 123;
    VERIFY(arr.exists(""));
    arr.remove(Variant());
    VERIFY(!arr.exists(""));
  }
  {
    Array arr;
    arr.lvalAt("n1") = "v1";
    arr.lvalAt("n2") = "v2";
    VERIFY(arr.exists("n1"));
    arr.remove("n1");
    VERIFY(!arr.exists("n1"));
    VS(arr, Array::Create("n2", "v2"));
    arr.append("v3");
    VS(arr, CREATE_MAP2("n2", "v2", 0, "v3"));
  }
  {
    Array arr;
    arr.lvalAt() = "test";
    VS(arr, CREATE_VECTOR1("test"));
  }
  {
    Array arr;
    arr.lvalAt("name") = "value";
    VERIFY(arr.exists("name"));
  }
  {
    Array arr;
    arr.lvalAt(1) = "value";
    VERIFY(arr.exists(1));
    VERIFY(arr.exists(1.5));
    VERIFY(arr.exists("1"));
    VERIFY(arr.exists(Variant("1")));
    VERIFY(arr.exists(Variant(1)));
    VERIFY(arr.exists(Variant(1.5)));
  }
  {
    Array arr;
    arr.lvalAt("1") = "value";
    VERIFY(arr.exists(1));
    VERIFY(arr.exists(1.5));
    VERIFY(arr.exists("1"));
    VERIFY(arr.exists(Variant("1")));
    VERIFY(arr.exists(Variant(1)));
    VERIFY(arr.exists(Variant(1.5)));
  }
  {
    Array arr;
    arr.lvalAt(1.5) = "value";
    VERIFY(arr.exists(1));
    VERIFY(arr.exists(1.5));
    VERIFY(arr.exists("1"));
    VERIFY(arr.exists(Variant("1")));
    VERIFY(arr.exists(Variant(1)));
    VERIFY(arr.exists(Variant(1.5)));
  }
  {
    Array arr;
    arr.lvalAt(Variant(1.5)) = "value";
    VERIFY(arr.exists(1));
    VERIFY(arr.exists(1.5));
    VERIFY(arr.exists("1"));
    VERIFY(arr.exists(Variant("1")));
    VERIFY(arr.exists(Variant(1)));
    VERIFY(arr.exists(Variant(1.5)));
  }
  {
    Array arr;
    arr.lvalAt(Variant("1")) = "value";
    VERIFY(arr.exists(1));
    VERIFY(arr.exists(1.5));
    VERIFY(arr.exists("1"));
    VERIFY(arr.exists(Variant("1")));
    VERIFY(arr.exists(Variant(1)));
    VERIFY(arr.exists(Variant(1.5)));
  }

  // merge
  {
    Array arr = Array::Create(0) + Array::Create(1);
    VS(arr, Array::Create(0));
    arr += CREATE_VECTOR2(0, 1);
    VS(arr, CREATE_VECTOR2(0, 1));

    arr = Array::Create(0).merge(Array::Create(1));
    VS(arr, CREATE_VECTOR2(0, 1));
    arr = arr.merge(CREATE_VECTOR2(0, 1));
    VS(arr, CREATE_VECTOR4(0, 1, 0, 1));

    arr = Array::Create("s0").merge(Array::Create("s1"));
    VS(arr, CREATE_VECTOR2("s0", "s1"));

    arr = Array::Create("n0", "s0") + Array::Create("n1", "s1");
    VS(arr, CREATE_MAP2("n0", "s0", "n1", "s1"));
    arr += CREATE_MAP2("n0", "s0", "n1", "s1");
    VS(arr, CREATE_MAP2("n0", "s0", "n1", "s1"));

    arr = Array::Create("n0", "s0").merge(Array::Create("n1", "s1"));
    VS(arr, CREATE_MAP2("n0", "s0", "n1", "s1"));
    Array arrX = CREATE_MAP2("n0", "s2", "n1", "s3");
    arr = arr.merge(arrX);
    VS(arr, CREATE_MAP2("n0", "s2", "n1", "s3"));
  }

  // slice
  {
    Array arr = CREATE_VECTOR2("test1", "test2");
    Array sub = arr.slice(1, 1, true);
    VS(sub, CREATE_MAP1(1, "test2"));
  }
  {
    Array arr = CREATE_VECTOR2("test1", "test2");
    Array sub = arr.slice(1, 1, false);
    VS(sub, CREATE_VECTOR1("test2"));
  }
  {
    Array arr = CREATE_MAP2("n1", "test1", "n2", "test2");
    Array sub = arr.slice(1, 1, true);
    VS(sub, CREATE_MAP1("n2", "test2"));
  }
  {
    Array arr = CREATE_MAP2("n1", "test1", "n2", "test2");
    Array sub = arr.slice(1, 1, false);
    VS(sub, CREATE_MAP1("n2", "test2"));
  }

  // escalation
  {
    Array arr;
    lval(arr.lvalAt(0)).lvalAt(0) = 1.2;
    VS(arr, CREATE_VECTOR1(CREATE_VECTOR1(1.2)));
  }
  {
    Array arr;
    lval(arr.lvalAt("name")).lvalAt(0) = 1.2;
    VS(arr, CREATE_MAP1("name", CREATE_VECTOR1(1.2)));
  }
  {
    Array arr = Array::Create();
    lval(arr.lvalAt(0)).lvalAt(0) = 1.2;
    VS(arr, CREATE_VECTOR1(CREATE_VECTOR1(1.2)));
  }
  {
    Array arr = Array::Create();
    lval(arr.lvalAt("name")).lvalAt(0) = 1.2;
    VS(arr, CREATE_MAP1("name", CREATE_VECTOR1(1.2)));
  }
  {
    Array arr = Array::Create("test");
    arr.lvalAt(0) = CREATE_VECTOR1(1.2);
    VS(arr, CREATE_VECTOR1(CREATE_VECTOR1(1.2)));
  }
  {
    Array arr = Array::Create("test");
    lval(arr.lvalAt("name")).lvalAt(0) = 1.2;
    VS(arr, CREATE_MAP2(0, "test", "name", CREATE_VECTOR1(1.2)));
  }
  {
    Array arr = Array::Create();
    arr.append("apple");
    arr.set(2, "pear");
    VS(arr[2], "pear");
  }

  {
    Array arr = CREATE_MAP2(0, "a", 1, "b");
    VERIFY(arr->isVectorData());
  }
  {
    Array arr = CREATE_MAP2(1, "a", 0, "b");
    VERIFY(!arr->isVectorData());
  }
  {
    Array arr = CREATE_MAP2(1, "a", 2, "b");
    VERIFY(!arr->isVectorData());
  }
  {
    Array arr = CREATE_MAP1(1, "a");
    arr.set(0, "b");
    VERIFY(!arr->isVectorData());
  }

  return Count(true);
}

bool TestCppBase::TestObject() {
  {
    String s = "O:1:\"B\":1:{s:3:\"obj\";O:1:\"A\":1:{s:1:\"a\";i:10;}}";
    VS(f_serialize(f_unserialize(s)), "O:22:\"__PHP_Incomplete_Class\":2:{s:27:\"__PHP_Incomplete_Class_Name\";s:1:\"B\";s:3:\"obj\";O:22:\"__PHP_Incomplete_Class\":2:{s:27:\"__PHP_Incomplete_Class_Name\";s:1:\"A\";s:1:\"a\";i:10;}}");
  }
  VERIFY(!equal(Object(new TestResource()), Object(new TestResource()) ));
  return Count(true);
}

bool TestCppBase::TestVariant() {
  // operators
  {
    Variant v(15);
    v += 20;
    VERIFY(v.isNumeric());
    VERIFY(v.is(KindOfInt64));
    VERIFY(v == Variant(35));
  }

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
    v.lvalAt(0) = "v0";
    v.lvalAt(1) = "v1";
    VERIFY(v[0] == "v0");
    VERIFY(v[1] == "v1");
  }
  {
    Variant v;
    v.lvalAt() = "test";
    VS(v, CREATE_VECTOR1("test"));
  }
  {
    Variant v;
    v.lvalAt(1) = "test";
    VS(v[1], "test");
    VS(v[1.5], "test");
    VS(v[Variant(1.5)], "test");
    VS(v["1"], "test");
    VS(v[Variant("1")], "test");
  }
  {
    Variant v;
    v.lvalAt(Variant(1.5)) = "test";
    VS(v[1], "test");
    VS(v[1.5], "test");
    VS(v[Variant(1.5)], "test");
    VS(v["1"], "test");
    VS(v[Variant("1")], "test");
  }
  {
    Variant v;
    v.lvalAt("1") = "test";
    VS(v[1], "test");
    VS(v[1.5], "test");
    VS(v[Variant(1.5)], "test");
    VS(v["1"], "test");
    VS(v[Variant("1")], "test");
  }
  {
    Variant v;
    v.lvalAt(Variant("1")) = "test";
    VS(v[1], "test");
    VS(v[1.5], "test");
    VS(v[Variant(1.5)], "test");
    VS(v["1"], "test");
    VS(v[Variant("1")], "test");
  }

  // membership
  {
    Variant v;
    v.lvalAt("n0") = "v0";
    v.lvalAt("n1") = "v1";
    v.remove("n1");
    VS(v, CREATE_MAP1("n0", "v0"));
    v.append("v2");
    VS(v, CREATE_MAP2("n0", "v0", 0, "v2"));
  }
  {
    Variant v;
    v.lvalAt("n0") = "v0";
    v.lvalAt(1) = "v1";
    v.remove(Variant(1.5));
    VS(v, CREATE_MAP1("n0", "v0"));
  }
  {
    Variant v;
    v.lvalAt("n0") = "v0";
    v.lvalAt(1) = "v1";
    v.remove(Variant("1"));
    VS(v, CREATE_MAP1("n0", "v0"));
  }
  {
    Variant v;
    v.lvalAt("n0") = "v0";
    v.lvalAt(1) = "v1";
    v.remove("1");
    VS(v, CREATE_MAP1("n0", "v0"));
  }
  {
    Variant v;
    v.lvalAt("n0") = "v0";
    v.lvalAt("") = "v1";
    v.remove(Variant());
    VS(v, CREATE_MAP1("n0", "v0"));
  }

  // references
  {
    Variant v1("original");
    Variant v2 = v1;
    v2 = "changed";
    VERIFY(v1 == "original");
  }
  {
    Variant v1("original");
    Variant v2 = strongBind(v1);
    v2 = "changed";
    VERIFY(v1 == "changed");
  }
  {
    Variant v1 = 10;
    Variant v2 = Array(ArrayInit(1, true).setRef(v1).create());
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
    Variant v2 = CREATE_VECTOR1(5);
    v2.lvalAt() = ref(v1);
    v1 = 20;
    VS(v2[1], 20);
  }
  {
    Variant v1 = 10;
    Variant v2 = strongBind(v1);
    v2++;
    VS(v2, 11);
    VS(v1, 11);
  }
  {
    Variant arr = CREATE_VECTOR2(1, 2);
    arr.escalate(true);
    Variant v;
    for (MutableArrayIter iter = arr.begin(NULL, v); iter.advance();) {
      v++;
    }
    VS(arr, CREATE_VECTOR2(2, 3));
  }

  // array escalation
  {
    Variant arr;
    lval(arr.lvalAt(0)).lvalAt(0) = 1.2;
    VS(arr, CREATE_VECTOR1(CREATE_VECTOR1(1.2)));
  }
  {
    Variant arr;
    lval(arr.lvalAt("name")).lvalAt(0) = 1.2;
    VS(arr, CREATE_MAP1("name", CREATE_VECTOR1(1.2)));
  }
  {
    Variant arr = Array::Create();
    lval(arr.lvalAt(0)).lvalAt(0) = 1.2;
    VS(arr, CREATE_VECTOR1(CREATE_VECTOR1(1.2)));
  }
  {
    Variant arr = Array::Create();
    lval(arr.lvalAt("name")).lvalAt(0) = 1.2;
    VS(arr, CREATE_MAP1("name", CREATE_VECTOR1(1.2)));
  }
  {
    Variant arr = Array::Create("test");
    arr.lvalAt(0) = CREATE_VECTOR1(1.2);
    VS(arr, CREATE_VECTOR1(CREATE_VECTOR1(1.2)));
  }
  {
    Variant arr = Array::Create("test");
    lval(arr.lvalAt("name")).lvalAt(0) = 1.2;
    VS(arr, CREATE_MAP2(0, "test", "name", CREATE_VECTOR1(1.2)));
  }

  return Count(true);
}

///////////////////////////////////////////////////////////////////////////////

class TestGlobals {
public:
  TestGlobals() {
    String a = "apple";
    m_string = a + "orange"; // so mallocing m_data internally

    m_array = CREATE_MAP2("a", "apple", "b", "orange");
  }

  Variant m_string;
  Array m_array;
  Variant m_string2;
  Array m_array2;
  Variant m_conn;
  Variant m_curlconn;
  Variant m_curlMultiConn;
  String key;
  String value;
  DECLARE_SMART_ALLOCATION_NOCALLBACKS(TestGlobals);
  void dump() {}
};
IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS(TestGlobals);

bool TestCppBase::TestMemoryManager() {
  s_apc_store.reset();
  MemoryManager::TheMemoryManager()->enable();

  TestGlobals *globals = NEW(TestGlobals)();
  f_apc_store("key", CREATE_VECTOR2("value", "s"));
  f_apc_store("key2", "apple");
  f_apc_store("key3", CREATE_MAP1("foo", "foo"));
  globals->m_array2 = f_apc_fetch("key");
  String apple = f_apc_fetch("key2"); // a shared string data
  Array arr = CREATE_MAP1(apple, "jobs");
  VS(arr[apple], "jobs");
  globals->m_string2 = f_apc_fetch("key2");
  globals->m_conn = f_mysql_connect(TEST_HOSTNAME, TEST_DATABASE,
                                    TEST_PASSWORD, false, 0);
  MemoryManager::TheMemoryManager()->checkpoint();
  globals->m_curlconn = f_curl_init("http://localhost:8080/request");
  f_curl_setopt(globals->m_curlconn, CURLOPT_WRITEFUNCTION,
                String("foo", CopyString));
  globals->m_curlMultiConn = f_curl_multi_init();
  Variant c1 = f_curl_init("http://localhost:8080/request");
  Variant c2 = f_curl_init("http://localhost:8080/request");
  f_curl_multi_add_handle(globals->m_curlMultiConn, c1);
  f_curl_multi_add_handle(globals->m_curlMultiConn, c2);
  globals->m_conn = null;

  // we do it twice, so to verify MemoryManager's rollback() is valid
  // we do it 3rd time, so to verify LinearAllocator works under rollback.
  // we do it 4th time, so to verify MySQL connection works under rollback.
  for (int i = 0; i < 4; i++) {

    // Circular reference between two arrays. Without sweeping, these memory
    // will still be reachable after exit.
    {
      Variant arr = Array::Create();
      arr.append(arr);
    }
    {
      Variant arr = Array::Create();
      arr.append(ref(arr));
    }
    {
      Variant arr1 = Array::Create();
      Variant arr2 = Array::Create();
      arr1.append(ref(arr2));
      arr2.append(ref(arr1));
    }

    // Circular reference between two objects.
    {
      Object obj(SystemLib::AllocStdClassObject());
      obj->o_set("a", obj);
      obj->o_set("f", Object(NEWOBJ(PlainFile)()));
    }
    {
      Object obj1(SystemLib::AllocStdClassObject());
      Object obj2(SystemLib::AllocStdClassObject());
      obj1->o_set("a", obj2);
      obj2->o_set("a", obj1);
      obj1->o_set("f", Object(NEWOBJ(PlainFile)()));
    }

    // dangling APC variables inside circular arrays
    {
      Variant arr1 = Array::Create();
      Variant arr2 = Array::Create();
      arr1.append(ref(arr2));
      arr2.append(ref(arr1));
      f_apc_store("name", CREATE_VECTOR2("value", "s"));
      Variant v = f_apc_fetch("name");
      arr1.append(v);
      f_apc_delete("name");
    }

    globals->m_string++; // mutating m_data internally
    VS(globals->m_string, "appleorangf");

    globals->m_array.set("a", "pear");
    globals->m_array.set("c", "banana");
    VS(globals->m_array["a"], "pear");
    VS(globals->m_array["c"], "banana");

    globals->m_conn = null;
    MemoryManager::TheMemoryManager()->sweepAll();
    MemoryManager::TheMemoryManager()->rollback();
    VS(globals->m_array2["0"], "value");
    VS(globals->m_array2["1"], "s");
    VS(globals->m_string2, "apple");
    Logger::Verbose("%s", SharedStores::ReportStats(0).c_str());

    VS(globals->m_string, "appleorange");

    VS(globals->m_array["a"], "apple");
    VERIFY(!globals->m_array.exists("c"));

    VS(arr[apple], "jobs");
  }
  DELETE(TestGlobals)(globals);
  return Count(true);
}

bool TestCppBase::TestIpBlockMap() {
  struct in6_addr addr;
  int bits;

  VERIFY(IpBlockMap::ReadIPv6Address("204.15.21.0/22", &addr, bits));
  VS(bits, 118);
  VS((long)addr.s6_addr32[0], (long)htonl(0x00000000));
  VS((long)addr.s6_addr32[1], (long)htonl(0x00000000));
  VS((long)addr.s6_addr32[2], (long)htonl(0x0000FFFF));
  VS((long)addr.s6_addr32[3], (long)htonl(0xCC0F1500));

  VERIFY(IpBlockMap::ReadIPv6Address("127.0.0.1", &addr, bits));
  VS(bits, 128);
  VS((long)addr.s6_addr32[0], (long)htonl(0x00000000));
  VS((long)addr.s6_addr32[1], (long)htonl(0x00000000));
  VS((long)addr.s6_addr32[2], (long)htonl(0x0000FFFF));
  VS((long)addr.s6_addr32[3], (long)htonl(0x7F000001));

  VERIFY(IpBlockMap::ReadIPv6Address(
    "1111:2222:3333:4444:5555:6666:789a:bcde", &addr, bits));
  VS(bits, 128);
  VS((long)addr.s6_addr32[0], (long)htonl(0x11112222));
  VS((long)addr.s6_addr32[1], (long)htonl(0x33334444));
  VS((long)addr.s6_addr32[2], (long)htonl(0x55556666));
  VS((long)addr.s6_addr32[3], (long)htonl(0x789abcde));

  VERIFY(IpBlockMap::ReadIPv6Address(
    "1111:2222:3333:4444:5555:6666:789a:bcde/68", &addr, bits));
  VS(bits, 68);
  VS((long)addr.s6_addr32[0], (long)htonl(0x11112222));
  VS((long)addr.s6_addr32[1], (long)htonl(0x33334444));
  VS((long)addr.s6_addr32[2], (long)htonl(0x55556666));
  VS((long)addr.s6_addr32[3], (long)htonl(0x789abcde));

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

bool TestCppBase::TestEqualAsStr() {

  const int arr_len = 18;
  Variant var_array[arr_len];
  var_array[0] = false;
  var_array[1] = true;
  var_array[2] = 0;
  var_array[3] = 1;
  var_array[4] = 42;
  var_array[5] = 0.0;
  var_array[6] = 1.0;
  var_array[7] = 42.2;
  var_array[8] = "0";
  var_array[9] = "1";
  var_array[10] = "42";
  var_array[11] = "x";
  var_array[12] = Array::Create();
  Variant v1("original");
  var_array[13] = v1;
  Variant v2("changed");
  var_array[14] = v2;
  var_array[15] = "";
  var_array[16] = "Array";
  var_array[17] = "ARRAY";
  for (int i = 0; i < arr_len; i++) {
    for (int j = 0; j < arr_len; j++) {
      bool eqAsStr = equalAsStr(var_array[i], var_array[j]);
      bool sm = same(toString(var_array[i]), toString(var_array[j]));
      VERIFY(eqAsStr == sm);
    }
  }
  return Count(true);
}

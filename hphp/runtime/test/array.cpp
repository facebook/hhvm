/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <gtest/gtest.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/repo-auth-type-array.h"

namespace HPHP {

TEST(ARRAY, Capacity) {
  EXPECT_TRUE(CapCode::Threshold == 0x7ff);

#define MP(a, b) std::make_pair(a, b)
  // Update the numbers if we change CapCode::Threshold
  std::pair<uint32_t, uint32_t> caps [] = {
    MP(3, 0),
    MP(4, 0),
    MP(5, 0),
    MP(6, 0),
    MP(7, 0),
    MP(8, 9),
    MP(12, 13),
    MP(127, 0),
    MP(128, 159),
    MP(0xFFFF, 0),
    MP(0x10000, 0),
    MP(0x10001, 0)
  };
#undef MP

  for (size_t i = 0; i != sizeof(caps) / sizeof(caps[0]); ++i) {
    EXPECT_TRUE(PackedArray::getMaxCapInPlaceFast(caps[i].first) ==
                caps[i].second);
  }
}

TEST(ARRAY, Constructors) {
  const String s_name("name");

  Array arr;
  EXPECT_TRUE(arr.empty());
  EXPECT_TRUE(arr.size() == 0);
  EXPECT_TRUE(arr.length() == 0);
  EXPECT_TRUE(arr.isNull());

  arr = Array::Create();
  EXPECT_TRUE(arr.empty());
  EXPECT_TRUE(arr.size() == 0);
  EXPECT_TRUE(arr.length() == 0);
  EXPECT_TRUE(!arr.isNull());

  arr = Array::Create(0);
  EXPECT_TRUE(!arr.empty());
  EXPECT_TRUE(arr.size() == 1);
  EXPECT_TRUE(arr.length() == 1);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(arr[0].toInt32() == 0);

  arr = Array::Create("test");
  EXPECT_TRUE(!arr.empty());
  EXPECT_TRUE(arr.size() == 1);
  EXPECT_TRUE(arr.length() == 1);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(equal(arr[0], String("test")));

  Array arrCopy = arr;
  arr = Array::Create(arr);
  EXPECT_TRUE(!arr.empty());
  EXPECT_TRUE(arr.size() == 1);
  EXPECT_TRUE(arr.length() == 1);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(arr[0].toArray().size() == 1);
  EXPECT_TRUE(equal(arr[0], arrCopy));

  arr = Array::Create("name", 1);
  EXPECT_TRUE(!arr.empty());
  EXPECT_TRUE(arr.size() == 1);
  EXPECT_TRUE(arr.length() == 1);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(arr[s_name].toInt32() == 1);

  arr = Array::Create(s_name, "test");
  EXPECT_TRUE(!arr.empty());
  EXPECT_TRUE(arr.size() == 1);
  EXPECT_TRUE(arr.length() == 1);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(equal(arr[s_name], String("test")));

  arrCopy = arr;
  arr = Array::Create(s_name, arr);
  EXPECT_TRUE(!arr.empty());
  EXPECT_TRUE(arr.size() == 1);
  EXPECT_TRUE(arr.length() == 1);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(equal(arr[s_name], arrCopy));
  EXPECT_TRUE(arr[s_name].toArray().size() == 1);
}

TEST(ARRAY, Iteration) {
  Array arr = make_map_array("n1", "v1", "n2", "v2");
  int i = 0;
  for (ArrayIter iter = arr.begin(); iter; ++iter, ++i) {
    if (i == 0) {
      EXPECT_TRUE(equal(iter.first(), String("n1")));
      EXPECT_TRUE(equal(iter.second(), String("v1")));
    } else {
      EXPECT_TRUE(equal(iter.first(), String("n2")));
      EXPECT_TRUE(equal(iter.second(), String("v2")));
    }
  }
  EXPECT_TRUE(i == 2);
}

TEST(ARRAY, Conversions) {
  const String s_Array("Array");

  Array arr0;
  EXPECT_TRUE(arr0.toBoolean() == false);
  EXPECT_TRUE(arr0.toByte() == 0);
  EXPECT_TRUE(arr0.toInt16() == 0);
  EXPECT_TRUE(arr0.toInt32() == 0);
  EXPECT_TRUE(arr0.toInt64() == 0);
  EXPECT_TRUE(arr0.toDouble() == 0.0);
  EXPECT_TRUE(arr0.toString().empty());

  Array arr1 = Array::Create("test");
  EXPECT_TRUE(arr1.toBoolean() == true);
  EXPECT_TRUE(arr1.toByte() == 1);
  EXPECT_TRUE(arr1.toInt16() == 1);
  EXPECT_TRUE(arr1.toInt32() == 1);
  EXPECT_TRUE(arr1.toInt64() == 1);
  EXPECT_TRUE(arr1.toDouble() == 1.0);
  EXPECT_TRUE(arr1.toString() == s_Array);
}

TEST(Array, Offsets) {
  const String s_n1("n1");
  const String s_n2("n2");
  const String s_1("1");

  {
    Array arr;
    arr.set(0, "v1");
    arr.set(1, "v2");
    EXPECT_TRUE(equal(arr, make_packed_array("v1", "v2")));
  }
  {
    Array arr;
    arr.set(s_n1, "v1");
    arr.set(s_n2, "v2");
    EXPECT_TRUE(equal(arr, make_map_array("n1", "v1", "n2", "v2")));
  }
  {
    Array arr;
    arr.lvalAt(0) = String("v1");
    arr.lvalAt(1) = String("v2");
    EXPECT_TRUE(equal(arr, make_packed_array("v1", "v2")));
  }
  {
    Array arr;
    arr.lvalAt(s_n1) = String("v1");
    arr.lvalAt(s_n2) = String("v2");
    EXPECT_TRUE(equal(arr, make_map_array("n1", "v1", "n2", "v2")));
  }
  {
    Array arr;
    Variant name = "name";
    arr.lvalAt(name) = String("value");
    EXPECT_TRUE(equal(arr, make_map_array("name", "value")));
  }

  {
    Array arr;
    arr.lvalAt(1) = 10;
    EXPECT_TRUE(equal(arr[1], 10));
    EXPECT_TRUE(equal(arr[Variant(1.5)], 10));
    EXPECT_TRUE(equal(arr[s_1], 10));
    EXPECT_TRUE(equal(arr[Variant("1")], 10));
  }
  {
    Array arr;
    arr.lvalAt(Variant(1.5)) = 10;
    EXPECT_TRUE(equal(arr[1], 10));
    EXPECT_TRUE(equal(arr[Variant(1.5)], 10));
    EXPECT_TRUE(equal(arr[s_1], 10));
    EXPECT_TRUE(equal(arr[Variant("1")], 10));
  }
  {
    Array arr;
    arr.lvalAt(s_1) = 10;
    EXPECT_TRUE(equal(arr[1], 10));
    EXPECT_TRUE(equal(arr[Variant(1.5)], 10));
    EXPECT_TRUE(equal(arr[s_1], 10));
    EXPECT_TRUE(equal(arr[Variant("1")], 10));
  }
  {
    Array arr;
    arr.lvalAt(Variant("1")) = 10;
    EXPECT_TRUE(equal(arr[1], 10));
    EXPECT_TRUE(equal(arr[Variant(1.5)], 10));
    EXPECT_TRUE(equal(arr[s_1], 10));
    EXPECT_TRUE(equal(arr[Variant("1")], 10));
  }
}

TEST(ARRAY, Membership) {
  const String s_n1("n1");
  const String s_n2("n2");
  const String s_name("name");
  const String s_1("1");

  {
    Array arr;
    arr.lvalAt(0) = String("v1");
    arr.lvalAt(1) = String("v2");
    EXPECT_TRUE(arr.exists(0));
    arr.remove(0);
    EXPECT_TRUE(!arr.exists(0));
    EXPECT_TRUE(equal(arr, Array::Create(1, "v2")));
    arr.append("v3");
    EXPECT_TRUE(equal(arr, make_map_array(1, "v2", 2, "v3")));
  }
  {
    const String s_0("0");
    Array arr;
    arr.lvalAt(0) = String("v1");
    EXPECT_TRUE(arr.exists(0));
    arr.remove(String(s_0));
    EXPECT_TRUE(!arr.exists(0));
  }
  {
    Array arr;
    arr.lvalAt(0) = String("v1");
    EXPECT_TRUE(arr.exists(0));
    arr.remove(Variant("0"));
    EXPECT_TRUE(!arr.exists(0));
  }
  {
    Array arr;
    arr.lvalAt(0) = String("v1");
    EXPECT_TRUE(arr.exists(0));
    arr.remove(Variant(Variant("0")));
    EXPECT_TRUE(!arr.exists(0));
  }
  {
    Array arr;
    arr.lvalAt(0) = String("v1");
    EXPECT_TRUE(arr.exists(0));
    arr.remove(Variant(Variant(0.5)));
    EXPECT_TRUE(!arr.exists(0));
  }
  {
    Array arr;
    arr.lvalAt(Variant()) = 123;
    EXPECT_TRUE(arr.exists(empty_string_ref));
    arr.remove(Variant());
    EXPECT_TRUE(!arr.exists(empty_string_ref));
  }
  {
    Array arr;
    arr.lvalAt(s_n1) = String("v1");
    arr.lvalAt(s_n2) = String("v2");
    EXPECT_TRUE(arr.exists(s_n1));
    arr.remove(s_n1);
    EXPECT_TRUE(!arr.exists(s_n1));
    EXPECT_TRUE(equal(arr, Array::Create(s_n2, "v2")));
    arr.append("v3");
    EXPECT_TRUE(equal(arr, make_map_array("n2", "v2", 0, "v3")));
  }
  {
    Array arr;
    arr.lvalAt() = String("test");
    EXPECT_TRUE(equal(arr, make_packed_array("test")));
  }
  {
    Array arr;
    arr.lvalAt(s_name) = String("value");
    EXPECT_TRUE(arr.exists(s_name));
  }
  {
    Array arr;
    arr.lvalAt(1) = String("value");
    EXPECT_TRUE(arr.exists(1));
    EXPECT_TRUE(arr.exists(s_1));
    EXPECT_TRUE(arr.exists(Variant("1")));
    EXPECT_TRUE(arr.exists(Variant(1)));
    EXPECT_TRUE(arr.exists(Variant(1.5)));
  }
  {
    Array arr;
    arr.lvalAt(s_1) = String("value");
    EXPECT_TRUE(arr.exists(1));
    EXPECT_TRUE(arr.exists(s_1));
    EXPECT_TRUE(arr.exists(Variant("1")));
    EXPECT_TRUE(arr.exists(Variant(1)));
    EXPECT_TRUE(arr.exists(Variant(1.5)));
  }
  {
    Array arr;
    arr.lvalAt(Variant(1.5)) = String("value");
    EXPECT_TRUE(arr.exists(1));
    EXPECT_TRUE(arr.exists(s_1));
    EXPECT_TRUE(arr.exists(Variant("1")));
    EXPECT_TRUE(arr.exists(Variant(1)));
    EXPECT_TRUE(arr.exists(Variant(1.5)));
  }
  {
    Array arr;
    arr.lvalAt(Variant("1")) = String("value");
    EXPECT_TRUE(arr.exists(1));
    EXPECT_TRUE(arr.exists(s_1));
    EXPECT_TRUE(arr.exists(Variant("1")));
    EXPECT_TRUE(arr.exists(Variant(1)));
    EXPECT_TRUE(arr.exists(Variant(1.5)));
  }
}


TEST(ARRAY, Merge) {
  {
    Array arr = Array::Create(0) + Array::Create(1);
    EXPECT_TRUE(equal(arr, Array::Create(0)));
    arr += make_packed_array(0, 1);
    EXPECT_TRUE(equal(arr, make_packed_array(0, 1)));

    arr = Array::Create(0).merge(Array::Create(1));
    EXPECT_TRUE(equal(arr, make_packed_array(0, 1)));
    arr = arr.merge(make_packed_array(0, 1));
    EXPECT_TRUE(equal(arr, make_packed_array(0, 1, 0, 1)));

    arr = Array::Create("s0").merge(Array::Create("s1"));
    EXPECT_TRUE(equal(arr, make_packed_array("s0", "s1")));

    arr = Array::Create("n0", "s0") + Array::Create("n1", "s1");
    EXPECT_TRUE(equal(arr, make_map_array("n0", "s0", "n1", "s1")));
    arr += make_map_array("n0", "s0", "n1", "s1");
    EXPECT_TRUE(equal(arr, make_map_array("n0", "s0", "n1", "s1")));

    arr = Array::Create("n0", "s0").merge(Array::Create("n1", "s1"));
    EXPECT_TRUE(equal(arr, make_map_array("n0", "s0", "n1", "s1")));
    Array arrX = make_map_array("n0", "s2", "n1", "s3");
    arr = arr.merge(arrX);
    EXPECT_TRUE(equal(arr, make_map_array("n0", "s2", "n1", "s3")));
  }

  {
    Array arr = make_map_array(0, "a", 1, "b");
    EXPECT_TRUE(arr->isVectorData());
  }
  {
    Array arr = make_map_array(1, "a", 0, "b");
    EXPECT_TRUE(!arr->isVectorData());
  }
  {
    Array arr = make_map_array(1, "a", 2, "b");
    EXPECT_TRUE(!arr->isVectorData());
  }
  {
    Array arr = make_map_array(1, "a");
    arr.set(0, "b");
    EXPECT_TRUE(!arr->isVectorData());
  }
}

}

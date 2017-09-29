/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

TEST(ARRAY, Constructors) {
  const String s_name("name");

  Array arr;
  EXPECT_TRUE(arr.empty());
  EXPECT_TRUE(arr.size() == 0);
  EXPECT_TRUE(arr.length() == 0);
  EXPECT_TRUE(arr.isNull());
  EXPECT_TRUE(arr.isPHPArray());
  EXPECT_FALSE(arr.isHackArray());

  arr = Array::Create();
  EXPECT_TRUE(arr.empty());
  EXPECT_TRUE(arr.size() == 0);
  EXPECT_TRUE(arr.length() == 0);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(arr.isPHPArray());
  EXPECT_FALSE(arr.isHackArray());

  arr = Array::Create(0);
  EXPECT_TRUE(!arr.empty());
  EXPECT_TRUE(arr.size() == 1);
  EXPECT_TRUE(arr.length() == 1);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(arr[0].toInt32() == 0);
  EXPECT_TRUE(arr.isPHPArray());
  EXPECT_FALSE(arr.isHackArray());

  arr = Array::Create("test");
  EXPECT_TRUE(!arr.empty());
  EXPECT_TRUE(arr.size() == 1);
  EXPECT_TRUE(arr.length() == 1);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(equal(arr[0], String("test")));
  EXPECT_TRUE(arr.isPHPArray());
  EXPECT_FALSE(arr.isHackArray());

  Array arrCopy = arr;
  arr = Array::Create(arr);
  EXPECT_TRUE(!arr.empty());
  EXPECT_TRUE(arr.size() == 1);
  EXPECT_TRUE(arr.length() == 1);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(arr[0].toArray().size() == 1);
  EXPECT_TRUE(equal(arr[0], arrCopy));
  EXPECT_TRUE(arr.isPHPArray());
  EXPECT_FALSE(arr.isHackArray());

  arr = Array::Create("name", 1);
  EXPECT_TRUE(!arr.empty());
  EXPECT_TRUE(arr.size() == 1);
  EXPECT_TRUE(arr.length() == 1);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(arr[s_name].toInt32() == 1);
  EXPECT_TRUE(arr.isPHPArray());
  EXPECT_FALSE(arr.isHackArray());

  arr = Array::Create(s_name, "test");
  EXPECT_TRUE(!arr.empty());
  EXPECT_TRUE(arr.size() == 1);
  EXPECT_TRUE(arr.length() == 1);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(equal(arr[s_name], String("test")));
  EXPECT_TRUE(arr.isPHPArray());
  EXPECT_FALSE(arr.isHackArray());

  arrCopy = arr;
  arr = Array::Create(s_name, arr);
  EXPECT_TRUE(!arr.empty());
  EXPECT_TRUE(arr.size() == 1);
  EXPECT_TRUE(arr.length() == 1);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(equal(arr[s_name], arrCopy));
  EXPECT_TRUE(arr[s_name].toArray().size() == 1);
  EXPECT_TRUE(arr.isPHPArray());
  EXPECT_FALSE(arr.isHackArray());

  arr = Array::CreateVec();
  EXPECT_TRUE(arr.empty());
  EXPECT_TRUE(arr.size() == 0);
  EXPECT_TRUE(arr.length() == 0);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(arr.isVecArray());
  EXPECT_TRUE(arr.isHackArray());
  EXPECT_FALSE(arr.isDict());
  EXPECT_FALSE(arr.isKeyset());
  EXPECT_FALSE(arr.isPHPArray());

  arr = Array::CreateDict();
  EXPECT_TRUE(arr.empty());
  EXPECT_TRUE(arr.size() == 0);
  EXPECT_TRUE(arr.length() == 0);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(arr.isDict());
  EXPECT_TRUE(arr.isHackArray());
  EXPECT_FALSE(arr.isVecArray());
  EXPECT_FALSE(arr.isKeyset());
  EXPECT_FALSE(arr.isPHPArray());

  arr = Array::CreateKeyset();
  EXPECT_TRUE(arr.empty());
  EXPECT_TRUE(arr.size() == 0);
  EXPECT_TRUE(arr.length() == 0);
  EXPECT_TRUE(!arr.isNull());
  EXPECT_TRUE(arr.isKeyset());
  EXPECT_TRUE(arr.isHackArray());
  EXPECT_FALSE(arr.isVecArray());
  EXPECT_FALSE(arr.isDict());
  EXPECT_FALSE(arr.isPHPArray());
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

  arr = make_vec_array("v1", "v2");
  i = 0;
  for (ArrayIter iter = arr.begin(); iter; ++iter, ++i) {
    if (i == 0) {
      EXPECT_TRUE(equal(iter.first(), 0));
      EXPECT_TRUE(equal(iter.second(), String("v1")));
    } else {
      EXPECT_TRUE(equal(iter.first(), 1));
      EXPECT_TRUE(equal(iter.second(), String("v2")));
    }
  }
  EXPECT_TRUE(i == 2);

  arr = make_dict_array("k1", "v1", "k2", "v2");
  i = 0;
  for (ArrayIter iter = arr.begin(); iter; ++iter, ++i) {
    if (i == 0) {
      EXPECT_TRUE(equal(iter.first(), String("k1")));
      EXPECT_TRUE(equal(iter.second(), String("v1")));
    } else {
      EXPECT_TRUE(equal(iter.first(), String("k2")));
      EXPECT_TRUE(equal(iter.second(), String("v2")));
    }
  }
  EXPECT_TRUE(i == 2);

  arr = make_keyset_array("v1", "v2");
  i = 0;
  for (ArrayIter iter = arr.begin(); iter; ++iter, ++i) {
    if (i == 0) {
      EXPECT_TRUE(equal(iter.first(), String("v1")));
      EXPECT_TRUE(equal(iter.second(), String("v1")));
    } else {
      EXPECT_TRUE(equal(iter.first(), String("v2")));
      EXPECT_TRUE(equal(iter.second(), String("v2")));
    }
  }
  EXPECT_TRUE(i == 2);
}

TEST(ARRAY, Conversions) {
  const String s_Array("Array");
  const String s_Vec("Vec");
  const String s_Dict("Dict");
  const String s_Keyset("Keyset");

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

  Array vec0 = Array::CreateVec();
  EXPECT_TRUE(vec0.toBoolean() == false);
  EXPECT_TRUE(vec0.toByte() == 0);
  EXPECT_TRUE(vec0.toInt16() == 0);
  EXPECT_TRUE(vec0.toInt32() == 0);
  EXPECT_TRUE(vec0.toInt64() == 0);
  EXPECT_TRUE(vec0.toDouble() == 0.0);
  EXPECT_TRUE(vec0.toString() == s_Vec);

  Array vec1 = Array::CreateVec();
  vec1.append("test");
  EXPECT_TRUE(vec1.toBoolean() == true);
  EXPECT_TRUE(vec1.toByte() == 1);
  EXPECT_TRUE(vec1.toInt16() == 1);
  EXPECT_TRUE(vec1.toInt32() == 1);
  EXPECT_TRUE(vec1.toInt64() == 1);
  EXPECT_TRUE(vec1.toDouble() == 1.0);
  EXPECT_TRUE(vec1.toString() == s_Vec);

  Array dict0 = Array::CreateDict();
  EXPECT_TRUE(dict0.toBoolean() == false);
  EXPECT_TRUE(dict0.toByte() == 0);
  EXPECT_TRUE(dict0.toInt16() == 0);
  EXPECT_TRUE(dict0.toInt32() == 0);
  EXPECT_TRUE(dict0.toInt64() == 0);
  EXPECT_TRUE(dict0.toDouble() == 0.0);
  EXPECT_TRUE(dict0.toString() == s_Dict);

  Array dict1 = Array::CreateDict();
  dict1.set(Variant{"key"}, Variant{"value"});
  EXPECT_TRUE(dict1.toBoolean() == true);
  EXPECT_TRUE(dict1.toByte() == 1);
  EXPECT_TRUE(dict1.toInt16() == 1);
  EXPECT_TRUE(dict1.toInt32() == 1);
  EXPECT_TRUE(dict1.toInt64() == 1);
  EXPECT_TRUE(dict1.toDouble() == 1.0);
  EXPECT_TRUE(dict1.toString() == s_Dict);

  Array keyset0 = Array::CreateKeyset();
  EXPECT_TRUE(keyset0.toBoolean() == false);
  EXPECT_TRUE(keyset0.toByte() == 0);
  EXPECT_TRUE(keyset0.toInt16() == 0);
  EXPECT_TRUE(keyset0.toInt32() == 0);
  EXPECT_TRUE(keyset0.toInt64() == 0);
  EXPECT_TRUE(keyset0.toDouble() == 0.0);
  EXPECT_TRUE(keyset0.toString() == s_Keyset);

  Array keyset1 = Array::CreateKeyset();
  keyset1.append("test");
  EXPECT_TRUE(keyset1.toBoolean() == true);
  EXPECT_TRUE(keyset1.toByte() == 1);
  EXPECT_TRUE(keyset1.toInt16() == 1);
  EXPECT_TRUE(keyset1.toInt32() == 1);
  EXPECT_TRUE(keyset1.toInt64() == 1);
  EXPECT_TRUE(keyset1.toDouble() == 1.0);
  EXPECT_TRUE(keyset1.toString() == s_Keyset);
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
    Array arr = Array::CreateVec();
    arr.append("v1");
    arr.append("v2");
    EXPECT_TRUE(equal(arr, make_vec_array("v1", "v2")));
  }
  {
    Array arr = Array::CreateDict();
    arr.set(Variant{"k1"}, Variant{"v1"});
    arr.set(Variant{"k2"}, Variant{"v2"});
    EXPECT_TRUE(equal(arr, make_dict_array("k1", "v1", "k2", "v2")));
  }
  {
    Array arr = Array::CreateKeyset();
    arr.append("v1");
    arr.append("v2");
    EXPECT_TRUE(equal(arr, make_keyset_array("v1", "v2")));
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
  {
    Array arr = Array::CreateVec();
    arr.append(Variant("value1"));
    arr.append(Variant("value2"));
    EXPECT_TRUE(equal(arr[0], Variant("value1")));
    EXPECT_TRUE(equal(arr[1], Variant("value2")));
    EXPECT_TRUE(arr[Variant("0")].isNull());
    EXPECT_TRUE(arr[2].isNull());
    EXPECT_TRUE(arr[Variant("key")].isNull());
  }
  {
    Array arr = Array::CreateDict();
    arr.set(Variant("key1"), Variant("value1"));
    arr.set(Variant("key2"), Variant("value2"));
    arr.set(Variant("1"), Variant("value3"));
    EXPECT_TRUE(equal(arr[Variant("key1")], Variant("value1")));
    EXPECT_TRUE(equal(arr[Variant("key2")], Variant("value2")));
    EXPECT_TRUE(equal(arr[Variant("1")], Variant("value3")));
    EXPECT_TRUE(arr[1].isNull());
    EXPECT_TRUE(arr[Variant("key")].isNull());
  }
  {
    Array arr = Array::CreateKeyset();
    arr.append(Variant("value1"));
    arr.append(Variant("value2"));
    arr.append(Variant("1"));
    EXPECT_TRUE(equal(arr[Variant("value1")], Variant("value1")));
    EXPECT_TRUE(equal(arr[Variant("value2")], Variant("value2")));
    EXPECT_TRUE(equal(arr[Variant("1")], Variant("1")));
    EXPECT_TRUE(arr[1].isNull());
    EXPECT_TRUE(arr[Variant("key")].isNull());
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
    auto const lval = arr.lvalAt();
    lval.type() = KindOfString;
    lval.val().pstr = StringData::Make("test", CopyString);
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
  {
    Array arr = Array::CreateVec();
    arr.append(Variant("value1"));
    arr.append(Variant("value2"));
    EXPECT_TRUE(arr.exists(0));
    EXPECT_TRUE(arr.exists(1));
    EXPECT_FALSE(arr.exists(Variant("0")));
    EXPECT_FALSE(arr.exists(2));
    EXPECT_FALSE(arr.exists(Variant("key")));
  }
  {
    Array arr = Array::CreateDict();
    arr.set(Variant("key1"), Variant("value1"));
    arr.set(Variant("key2"), Variant("value2"));
    arr.set(Variant("1"), Variant("value3"));
    EXPECT_TRUE(arr.exists(Variant("key1")));
    EXPECT_TRUE(arr.exists(Variant("key2")));
    EXPECT_TRUE(arr.exists(Variant("1")));
    EXPECT_FALSE(arr.exists(1));
    EXPECT_FALSE(arr.exists(Variant("key")));
  }
  {
    Array arr = Array::CreateKeyset();
    arr.append(Variant("value1"));
    arr.append(Variant("value2"));
    arr.append(Variant("1"));
    EXPECT_TRUE(arr.exists(Variant("value1")));
    EXPECT_TRUE(arr.exists(Variant("value2")));
    EXPECT_TRUE(arr.exists(Variant("1")));
    EXPECT_FALSE(arr.exists(1));
    EXPECT_FALSE(arr.exists(Variant("key")));
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

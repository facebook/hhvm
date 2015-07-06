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

#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"

namespace HPHP {

TEST(Object, Serialization) {
  String s = "O:1:\"B\":1:{s:3:\"obj\";O:1:\"A\":1:{s:1:\"a\";i:10;}}";
  Variant v = unserialize_from_string(s);
  EXPECT_TRUE(v.isObject());
  auto o = v.toObject();
  EXPECT_TRUE(
    !o->getClassName().asString().compare("__PHP_Incomplete_Class")
  );
  auto os = HHVM_FN(serialize)(o);
  EXPECT_TRUE(
    !os.compare( "O:1:\"B\":1:{s:3:\"obj\";O:1:\"A\":1:{s:1:\"a\";i:10;}}")
  );
}

TEST(Object, Casts) {
  // Test cast operations
  {
    EXPECT_FALSE(isa<c_Vector>(Object()));
    EXPECT_TRUE(isa_or_null<c_Vector>(Object()));

    auto dummy = req::make<c_Vector>();
    Object res(dummy);
    Object empty;
    EXPECT_TRUE(isa<c_Vector>(res));
    EXPECT_TRUE(isa_or_null<c_Vector>(res));

    EXPECT_FALSE(isa<c_Map>(res));
    EXPECT_FALSE(isa_or_null<c_Map>(res));

    // cast tests
    // Bad types and null pointers should throw.
    EXPECT_EQ(cast<c_Vector>(res), dummy);
    EXPECT_EQ(cast<ObjectData>(res), dummy);
    try {
      cast<c_Map>(res);
      EXPECT_FALSE(true);
    } catch(...) {
      EXPECT_TRUE(true);
    }
    try {
      cast<c_Vector>(empty);
      EXPECT_FALSE(true);
    } catch(...) {
      EXPECT_TRUE(true);
    }

    // cast_or_null tests
    // Bad types should throw, null pointers are ok.
    EXPECT_EQ(cast_or_null<ObjectData>(empty), nullptr);
    EXPECT_EQ(cast_or_null<ObjectData>(res), dummy);

    try {
      cast_or_null<c_Map>(res);
      EXPECT_FALSE(true);
    } catch(...) {
      EXPECT_TRUE(true);
    }

    // dyn_cast tests
    // Bad types are ok, null pointers should throw.
    EXPECT_EQ(dyn_cast<c_Vector>(res), dummy);
    EXPECT_EQ(dyn_cast<ObjectData>(res), dummy);
    EXPECT_EQ(dyn_cast<c_Map>(res), nullptr);

    try {
      dyn_cast<c_Vector>(empty);
      EXPECT_FALSE(true);
    } catch(...) {
      EXPECT_TRUE(true);
    }

    // dyn_cast_or_null
    // Bad types and null pointers are ok.  Should never throw.
    EXPECT_EQ(dyn_cast_or_null<c_Map>(res), nullptr);
    EXPECT_EQ(dyn_cast_or_null<ObjectData>(res), dummy);
    EXPECT_EQ(dyn_cast_or_null<ObjectData>(empty), nullptr);
  }
}

}

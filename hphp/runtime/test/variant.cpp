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
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/ext/ext_collections.h"

namespace HPHP {

TEST(Variant, Conversions) {
  Variant v("123");
  EXPECT_TRUE(v.toInt32() == 123);
}

TEST(Variant, References) {
  {
    Variant v1("original");
    Variant v2 = v1;
    v2 = String("changed");
    EXPECT_TRUE(equal(v1, String("original")));
  }
  {
    Variant v1("original");
    Variant v2(Variant::StrongBind{}, v1);
    v2 = String("changed");
    EXPECT_TRUE(equal(v1, String("changed")));
  }
}

TEST(Variant, Refcounts) {
  {
    auto ptr = makeSmartPtr<DummyResource>();
    EXPECT_TRUE(ptr->hasExactlyOneRef());
    Variant v(std::move(ptr));
    EXPECT_TRUE(v.getRefCount() == 1);
  }

  {
    auto ptr = makeSmartPtr<DummyResource>();
    EXPECT_TRUE(ptr->hasExactlyOneRef());
    {
      Variant v(ptr);
      EXPECT_TRUE(ptr->getCount() == 2);
      EXPECT_TRUE(v.getRefCount() == 2);
    }
    EXPECT_TRUE(ptr->hasExactlyOneRef());
  }
}

TEST(Variant, Casts) {
  // Test Resource cast operations
  {
    EXPECT_FALSE(isa<DummyResource>(Variant()));
    EXPECT_TRUE(isa_or_null<DummyResource>(Variant()));
    EXPECT_FALSE(isa<DummyResource>(Variant(true)));
    EXPECT_FALSE(isa_or_null<DummyResource>(Variant(true)));

    auto dummy = makeSmartPtr<DummyResource>();
    Variant var(dummy);
    Variant empty;
    EXPECT_TRUE(isa<DummyResource>(var));
    EXPECT_TRUE(isa_or_null<DummyResource>(var));

    EXPECT_FALSE(isa<File>(var));
    EXPECT_FALSE(isa_or_null<File>(var));

    // cast tests
    // Bad types and null pointers should throw.
    EXPECT_EQ(cast<DummyResource>(var), dummy);
    EXPECT_EQ(cast<ResourceData>(var), dummy);
    try {
      cast<File>(var);
      EXPECT_FALSE(true);
    } catch(...) {
      EXPECT_TRUE(true);
    }
    try {
      cast<c_Map>(var);
      EXPECT_FALSE(true);
    } catch(...) {
      EXPECT_TRUE(true);
    }
    try {
      cast<DummyResource>(empty);
      EXPECT_FALSE(true);
    } catch(...) {
      EXPECT_TRUE(true);
    }

    // cast_or_null tests
    // Bad types should throw, null pointers are ok.
    EXPECT_EQ(cast_or_null<ResourceData>(empty), nullptr);
    EXPECT_EQ(cast_or_null<ResourceData>(var), dummy);

    try {
      cast_or_null<File>(var);
      EXPECT_FALSE(true);
    } catch(...) {
      EXPECT_TRUE(true);
    }
    try {
      cast_or_null<c_Map>(var);
      EXPECT_FALSE(true);
    } catch(...) {
      EXPECT_TRUE(true);
    }

    // dyn_cast tests
    // Bad types are ok, null pointers should throw.
    EXPECT_EQ(dyn_cast<DummyResource>(var), dummy);
    EXPECT_EQ(dyn_cast<ResourceData>(var), dummy);
    EXPECT_EQ(dyn_cast<File>(var), nullptr);
    EXPECT_EQ(dyn_cast<c_Map>(var), nullptr);
    try {
      dyn_cast<DummyResource>(Variant());
      EXPECT_FALSE(true);
    } catch(...) {
      EXPECT_TRUE(true);
    }

    // dyn_cast_or_null
    // Bad types and null pointers are ok.  Should never throw.
    EXPECT_EQ(dyn_cast_or_null<ResourceData>(empty), nullptr);
    EXPECT_EQ(dyn_cast_or_null<ResourceData>(var), dummy);
    EXPECT_EQ(dyn_cast_or_null<c_Map>(var), nullptr);
  }

  // Test Object cast operations
  {
    EXPECT_FALSE(isa<c_Vector>(Variant()));
    EXPECT_TRUE(isa_or_null<c_Vector>(Variant()));
    EXPECT_FALSE(isa<c_Vector>(Variant(true)));
    EXPECT_FALSE(isa_or_null<c_Vector>(Variant(true)));

    auto dummy = makeSmartPtr<c_Vector>();
    Variant var(dummy);
    Variant empty;
    EXPECT_TRUE(isa<c_Vector>(var));
    EXPECT_TRUE(isa_or_null<c_Vector>(var));

    EXPECT_FALSE(isa<c_Map>(var));
    EXPECT_FALSE(isa_or_null<c_Map>(var));

    // cast tests
    // Bad types and null pointers should throw.
    EXPECT_EQ(cast<c_Vector>(var), dummy);
    EXPECT_EQ(cast<ObjectData>(var), dummy);
    try {
      cast<c_Map>(var);
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
    try {
      cast<File>(empty);
      EXPECT_FALSE(true);
    } catch(...) {
      EXPECT_TRUE(true);
    }

    // cast_or_null tests
    // Bad types should throw, null pointers are ok.
    EXPECT_EQ(cast_or_null<c_Vector>(empty), nullptr);
    EXPECT_EQ(cast_or_null<c_Vector>(var), dummy);

    try {
      cast_or_null<File>(var);
      EXPECT_FALSE(true);
    } catch(...) {
      EXPECT_TRUE(true);
    }
    try {
      cast_or_null<c_Map>(var);
      EXPECT_FALSE(true);
    } catch(...) {
      EXPECT_TRUE(true);
    }

    // dyn_cast tests
    // Bad types are ok, null pointers should throw.
    EXPECT_EQ(dyn_cast<c_Vector>(var), dummy);
    EXPECT_EQ(dyn_cast<ObjectData>(var), dummy);
    EXPECT_EQ(dyn_cast<c_Map>(var), nullptr);
    EXPECT_EQ(dyn_cast<File>(var), nullptr);
    try {
      dyn_cast<c_Vector>(empty);
      EXPECT_FALSE(true);
    } catch(...) {
      EXPECT_TRUE(true);
    }

    // dyn_cast_or_null
    // Bad types and null pointers are ok.  Should never throw.
    EXPECT_EQ(dyn_cast_or_null<c_Map>(empty), nullptr);
    EXPECT_EQ(dyn_cast_or_null<c_Vector>(var), dummy);
    EXPECT_EQ(dyn_cast_or_null<c_Map>(var), nullptr);
  }
}

}

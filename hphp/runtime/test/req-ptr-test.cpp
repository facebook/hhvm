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

#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/base/file.h"

namespace HPHP {

struct DummyResource2 : DummyResource {
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(DummyResource2);
  CLASSNAME_IS("Unknown");
  DummyResource2() {}
  String m_class_name;
  const String& o_getClassNameHook() const override {
    if (m_class_name.empty()) {
      return classnameof();
    }
    return m_class_name;
  }
  bool isInvalid() const override { return m_class_name.empty(); }
};

TEST(ReqPtr, Refcounts) {
  {
    auto ptr = req::make<DummyResource>();
    EXPECT_TRUE(ptr->hasExactlyOneRef());
    req::ptr<ResourceData> r(std::move(ptr));
    EXPECT_TRUE(r.get()->hasExactlyOneRef());
  }

  {
    auto ptr = req::make<DummyResource>();
    EXPECT_TRUE(ptr->hasExactlyOneRef());
    {
      req::ptr<ResourceData> r(ptr);
      EXPECT_TRUE(ptr->hasMultipleRefs()); // count==2
      EXPECT_TRUE(r.get()->hasMultipleRefs());
    }
    EXPECT_TRUE(ptr->hasExactlyOneRef());
  }
}

TEST(ReqPtr, Assignment) {
  auto ptr = req::make<DummyResource>();
  auto* tmp = ptr.get();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
  ptr = ptr;
#pragma clang diagnostic pop
  EXPECT_TRUE(ptr->hasExactlyOneRef());
  EXPECT_TRUE(ptr.get() == tmp);
}

TEST(ReqPtr, Operators) {
  {
    req::ptr<DummyResource> p1;
    req::ptr<DummyResource2> p2;
    EXPECT_TRUE(p1 == p1);
    EXPECT_TRUE(p1 == p2);
    EXPECT_TRUE(p1 == nullptr);
    EXPECT_TRUE(nullptr == p1);
    EXPECT_FALSE(p1 != p1);
    EXPECT_FALSE(p1 != p2);
    EXPECT_FALSE(p1 != nullptr);
    EXPECT_FALSE(nullptr != p1);
  }
  {
    auto p1 = req::make<DummyResource>();
    auto p2 = req::make<DummyResource>();
    auto p3 = p1;
    EXPECT_FALSE(p1 == p2);
    EXPECT_TRUE(p1 == p3);
    EXPECT_FALSE(p1 == nullptr);
    EXPECT_FALSE(nullptr == p1);
    EXPECT_TRUE(p1 != p2);
    EXPECT_FALSE(p1 != p3);
    EXPECT_TRUE(p1 != nullptr);
    EXPECT_TRUE(nullptr != p1);
  }
  {
    auto p1 = req::make<DummyResource>();
    auto p2 = req::make<DummyResource2>();
    EXPECT_FALSE(p1 == p2);
    EXPECT_TRUE(p1 != p2);
  }
  {
    auto p1 = req::make<DummyResource>();
    req::ptr<ResourceData> p2(p1);
    EXPECT_TRUE(p1 == p2);
    EXPECT_FALSE(p1 != p2);
  }
}

TEST(ReqPtr, Casts) {
  // Test cast operations
  {
    EXPECT_FALSE(isa<DummyResource>(req::ptr<DummyResource>(nullptr)));
    EXPECT_TRUE(isa_or_null<DummyResource>(req::ptr<DummyResource>(nullptr)));

    auto dummy = req::make<DummyResource>();
    req::ptr<ResourceData> res(dummy);
    req::ptr<ResourceData> empty;
    req::ptr<File> emptyFile;

    EXPECT_TRUE(isa<DummyResource>(res));
    EXPECT_TRUE(isa_or_null<DummyResource>(res));

    EXPECT_FALSE(isa<File>(res));
    EXPECT_FALSE(isa_or_null<File>(res));

    // cast tests
    // Bad types and null pointers should throw.
    EXPECT_EQ(cast<DummyResource>(res), dummy);
    EXPECT_EQ(cast<ResourceData>(res), dummy);

    try {
      cast<File>(res);
      ADD_FAILURE();
    } catch(...) {
      SUCCEED();
    }

    try {
      cast<DummyResource>(empty);
      ADD_FAILURE();
    } catch(...) {
      SUCCEED();
    }

    // cast_or_null tests
    // Bad types should throw, null pointers are ok.
    EXPECT_EQ(cast_or_null<ResourceData>(empty),  nullptr);
    EXPECT_EQ(cast_or_null<ResourceData>(dummy), dummy);

    try {
      cast_or_null<File>(res);
      ADD_FAILURE();
    } catch(...) {
      SUCCEED();
    }

    // dyn_cast tests
    // Bad types are ok, null pointers should throw.
    EXPECT_EQ(dyn_cast<DummyResource>(res), dummy);
    EXPECT_EQ(dyn_cast<ResourceData>(res), dummy);
    EXPECT_EQ(dyn_cast<File>(res), nullptr);
    try {
      dyn_cast<File>(emptyFile);
      ADD_FAILURE();
    } catch(...) {
      SUCCEED();
    }

    // dyn_cast_or_null
    // Bad types and null pointers are ok.  Should never throw.
    EXPECT_EQ(dyn_cast_or_null<DummyResource>(res), res);
    EXPECT_EQ(dyn_cast_or_null<File>(res), nullptr);
    EXPECT_EQ(dyn_cast_or_null<ResourceData>(empty), nullptr);
    EXPECT_EQ(dyn_cast_or_null<ResourceData>(emptyFile), nullptr);
  }
}

TEST(ReqPtr, MoveCasts) {
  auto res = unsafe_cast_or_null<DummyResource>(req::make<DummyResource>());
  EXPECT_NE(res, nullptr);
  auto res2 = dyn_cast<DummyResource>(req::make<DummyResource>());
  EXPECT_NE(res2, nullptr);
  auto res3 = dyn_cast<DummyResource2>(req::make<DummyResource>());
  EXPECT_EQ(res3, nullptr);
}

}

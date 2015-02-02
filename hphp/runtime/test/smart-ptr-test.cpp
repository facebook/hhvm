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

#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/base/file.h"

namespace HPHP {

struct DummyResource2 : public DummyResource {
public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(DummyResource2);
  CLASSNAME_IS("Unknown");
  DummyResource2() {}
  String m_class_name;
  virtual const String& o_getClassNameHook() const {
    if (m_class_name.empty()) {
      return classnameof();
    }
    return m_class_name;
  }
  virtual bool isInvalid() const { return m_class_name.empty(); }
  void o_setResourceId(int64_t id) { o_id = id; }
};

TEST(SmartPtr, Refcounts) {
  {
    auto ptr = makeSmartPtr<DummyResource>();
    EXPECT_TRUE(ptr->hasExactlyOneRef());
    SmartPtr<ResourceData> r(std::move(ptr));
    EXPECT_TRUE(r.get()->getCount() == 1);
  }

  {
    auto ptr = makeSmartPtr<DummyResource>();
    EXPECT_TRUE(ptr->hasExactlyOneRef());
    {
      SmartPtr<ResourceData> r(ptr);
      EXPECT_TRUE(ptr->getCount() == 2);
      EXPECT_TRUE(r.get()->getCount() == 2);
    }
    EXPECT_TRUE(ptr->hasExactlyOneRef());
  }
}

TEST(SmartPtr, Operators) {
  {
    SmartPtr<DummyResource> p1;
    SmartPtr<DummyResource2> p2;
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
    auto p1 = makeSmartPtr<DummyResource>();
    auto p2 = makeSmartPtr<DummyResource>();
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
    auto p1 = makeSmartPtr<DummyResource>();
    auto p2 = makeSmartPtr<DummyResource2>();
    EXPECT_FALSE(p1 == p2);
    EXPECT_TRUE(p1 != p2);
  }
  {
    auto p1 = makeSmartPtr<DummyResource>();
    SmartPtr<ResourceData> p2(p1);
    EXPECT_TRUE(p1 == p2);
    EXPECT_FALSE(p1 != p2);
  }
}

}

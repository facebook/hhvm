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

TEST(Resource, Refcounts) {
  {
    auto ptr = req::make<DummyResource>();
    EXPECT_TRUE(ptr->hasExactlyOneRef());
    OptResource r(std::move(ptr));
    EXPECT_TRUE(r->hasExactlyOneRef());
  }

  {
    auto ptr = req::make<DummyResource>();
    EXPECT_TRUE(ptr->hasExactlyOneRef());
    {
      OptResource r(ptr);
      EXPECT_TRUE(ptr->hasMultipleRefs()); // count==2
      EXPECT_TRUE(r->hasMultipleRefs());
    }
    EXPECT_TRUE(ptr->hasExactlyOneRef());
  }
}

TEST(Resource, Casts) {
  // Test cast operations
  {
    EXPECT_FALSE(isa<DummyResource>(OptResource()));
    EXPECT_TRUE(isa_or_null<DummyResource>(OptResource()));

    auto dummy = req::make<DummyResource>();
    OptResource res(dummy);
    OptResource empty;
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
    EXPECT_EQ(cast_or_null<ResourceData>(empty), nullptr);
    EXPECT_EQ(cast_or_null<ResourceData>(res), dummy);

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
      dyn_cast<DummyResource>(empty);
      ADD_FAILURE();
    } catch(...) {
      SUCCEED();
    }

    // dyn_cast_or_null
    // Bad types and null pointers are ok.  Should never throw.
    EXPECT_EQ(dyn_cast_or_null<File>(res), nullptr);
    EXPECT_EQ(dyn_cast_or_null<ResourceData>(res), dummy);
    EXPECT_EQ(dyn_cast_or_null<ResourceData>(empty), nullptr);
  }
}

}

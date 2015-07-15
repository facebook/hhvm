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

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"

namespace HPHP {

TEST(MemoryManager, RootMaps) {
  {
    ASSERT_EQ(MM().lookupRoot<DummyResource>(0), nullptr);
    auto dummy = req::make<DummyResource>();
    auto id = MM().addRoot(dummy);
    ASSERT_EQ(MM().lookupRoot<DummyResource>(id), dummy);
    auto removed = MM().removeRoot<DummyResource>(id);
    ASSERT_EQ(removed, dummy);
    ASSERT_EQ(MM().lookupRoot<DummyResource>(id), nullptr);
    ASSERT_FALSE(MM().removeRoot<DummyResource>(id));

    MM().addRoot(dummy);
    ASSERT_TRUE(MM().removeRoot(dummy));
  }
  {
    ASSERT_EQ(MM().lookupRoot<c_Vector>(0), nullptr);
    auto vec = req::make<c_Vector>();
    auto id = MM().addRoot(vec);
    ASSERT_EQ(MM().lookupRoot<c_Vector>(id), vec);
    auto removed = MM().removeRoot<c_Vector>(id);
    ASSERT_EQ(removed, vec);
    ASSERT_EQ(MM().lookupRoot<c_Vector>(id), nullptr);
    ASSERT_FALSE(MM().removeRoot<c_Vector>(id));

    MM().addRoot(vec);
    ASSERT_TRUE(MM().removeRoot(vec));
  }
}

}

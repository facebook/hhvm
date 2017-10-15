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

#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/root-map.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/util/thread-local.h"

namespace HPHP {

struct RootMapTestData {
  RootMap<DummyResource> resources;
  RootMap<c_Vector> objects;
};
THREAD_LOCAL(RootMapTestData, s_root);

TEST(RootMaps, Resources) {
  {
    using RootId = RootMap<DummyResource>::RootId;
    ASSERT_EQ(s_root->resources.lookupRoot(RootId{0}), nullptr);
    auto dummy = req::make<DummyResource>();
    auto id = s_root->resources.addRoot(dummy);
    ASSERT_EQ(s_root->resources.lookupRoot(id), dummy);
    auto removed = s_root->resources.removeRoot(id);
    ASSERT_EQ(removed, dummy);
    ASSERT_EQ(s_root->resources.lookupRoot(id), nullptr);
    ASSERT_FALSE(s_root->resources.removeRoot(id));

    s_root->resources.addRoot(dummy);
    ASSERT_TRUE(s_root->resources.removeRoot(dummy));
  }
}

TEST(RootMaps, Objects) {
  {
    using RootId = RootMap<c_Vector>::RootId;
    ASSERT_EQ(s_root->objects.lookupRoot(RootId{0}), nullptr);
    auto vec = req::make<c_Vector>();
    auto id = s_root->objects.addRoot(vec);
    ASSERT_EQ(s_root->objects.lookupRoot(id), vec);
    auto removed = s_root->objects.removeRoot(id);
    ASSERT_EQ(removed, vec);
    ASSERT_EQ(s_root->objects.lookupRoot(id), nullptr);
    ASSERT_FALSE(s_root->objects.removeRoot(id));

    s_root->objects.addRoot(vec);
    ASSERT_TRUE(s_root->objects.removeRoot(vec));
  }
}

}

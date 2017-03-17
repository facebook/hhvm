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

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"

namespace HPHP {

static void allocAndJoin(size_t size, bool free) {
  std::thread thread([&]() {
      MemoryManager::TlsWrapper::getCheck();
      if (free) {
        String str(size, ReserveString);
      } else {
        StringData::Make(size); // Leak.
      }
    });
  thread.join();
}

TEST(MemoryManager, OnThreadExit) {
  allocAndJoin(42, true);
  allocAndJoin(kMaxSmallSize + 1, true);
#ifdef DEBUG
  EXPECT_DEATH(allocAndJoin(42, false), "");
  EXPECT_DEATH(allocAndJoin(kMaxSmallSize + 1, false), "");
#endif
}

}

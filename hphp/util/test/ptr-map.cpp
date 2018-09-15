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

#include <folly/portability/GTest.h>

#include "hphp/util/ptr-map.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

TEST(PTRMAP, InsertAndQuery) {

  PtrMap<const void*> ptr;

  ptr.insert((void*)100, 100);  //  [100,200)
  ptr.insert((void*)250, 100);  //  [250,350)
  ptr.prepare();

  EXPECT_EQ(ptr.size(),2);
  EXPECT_EQ(ptr.isStart((void*)100), true);  //  the header of [100,200)
  EXPECT_EQ(ptr.isStart((void*)150), false); //  inside [100,200), not header
  EXPECT_EQ(ptr.isStart((void*)50), false);  //  outside [100,350) range
  EXPECT_EQ(ptr.isStart((void*)220), false); //  outside both intervals
  EXPECT_EQ(ptr.isStart((void*)500), false); //  outside [100,350) range

  EXPECT_EQ(ptr.index((const void*)120), 0);  //  inside [100,200)
  EXPECT_EQ(ptr.index((const void*)260), 1);  //  inside [250,350)

  auto r = ptr.region((void*)150);
  EXPECT_EQ((void*)r->first, (void*)100);  //  [100,200)
  EXPECT_EQ(r->second, 100);
  EXPECT_EQ(ptr.region((void*)220), nullptr);  //  outside any intervals

  //  mask test
  auto bound = 0xffffffffffffULL; //  48 bit address space mask
  ptr.insert((void*)(bound-100), 1000);  //  [bound-100,bound+900)
  ptr.prepare();

  EXPECT_EQ(uintptr_t(ptr.span().second), bound-100+1000-100);
  EXPECT_NE(ptr.region((void*)(bound-100+50)), nullptr);

  // if PtrMap masked pointers, address would become 150, inside [100,200)
  r = ptr.region((void*)(bound+1+150));
  EXPECT_EQ((void*)r->first, (void*)(bound-100));
  EXPECT_EQ(r->second, 1000);

  // if PtrMap masked pointers, address would becomes 220, outside any interval
  EXPECT_NE(ptr.region((void*)(bound+1+220)), nullptr);
}
///////////////////////////////////////////////////////////////////////////////

}

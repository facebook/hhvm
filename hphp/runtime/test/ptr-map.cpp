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

#include "hphp/runtime/base/memory-manager-defs.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

TEST(PTRMAP, InsertAndQuery) {

  auto ptr = PtrMap();

  ptr.insert((Header*)100, 100);  //  [100,200)
  ptr.insert((Header*)250, 100);  //  [250,350)
  ptr.prepare();

  EXPECT_EQ(ptr.size(),2);
  EXPECT_EQ(ptr.isHeader((void*)100), true);  //  the header of [100,200)
  EXPECT_EQ(ptr.isHeader((void*)150), false); //  inside [100,200), not header
  EXPECT_EQ(ptr.isHeader((void*)50), false);  //  outside [100,350) range
  EXPECT_EQ(ptr.isHeader((void*)220), false); //  outside both intervals
  EXPECT_EQ(ptr.isHeader((void*)500), false); //  outside [100,350) range

  EXPECT_EQ(ptr.index((const HPHP::Header *)120),0);  //  inside [100,200)
  EXPECT_EQ(ptr.index((const HPHP::Header *)250),1);  //  inside [200,350)

  const PtrMap::Region* r ;
  r = ptr.region((void*)150);
  EXPECT_EQ((void*)r->first,(void*)100);  //  [100,200)
  EXPECT_EQ(r->second,100);
  EXPECT_EQ(ptr.region((void*)220),nullptr);  //  outside any intervals

  //  mask test
  auto bound = 0xffffffffffffULL; //  48 bit address space mask
  ptr.insert((Header *)(bound-100), 1000);  //  [bound-100,bound+900)
  ptr.prepare();

  EXPECT_EQ( uintptr_t( ptr.span().second) , bound-100+1000-100);
  EXPECT_NE(ptr.region((void*)(bound-100+50)),nullptr); // [bound-100,bound+900)
  // address becomes 150, inside [100,200)
  r = ptr.region((void*)(bound+1+150));
  EXPECT_EQ((void*)r->first,(void*)100);
  EXPECT_EQ(r->second,100);
   // address becomes 220, outside any intervals
  EXPECT_EQ(ptr.region((void*)(bound+1+220)),nullptr);
}
///////////////////////////////////////////////////////////////////////////////

}

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/stack-trace.h"

#include <gtest/gtest.h>

namespace HPHP {

static int rangeCmp(StackTrace::PerfMap::Range a,
                    StackTrace::PerfMap::Range b) {
  StackTrace::PerfMap::Range::Cmp cmp;
  return (cmp(b, a) ? 1 : 0) - (cmp(a, b) ? 1 : 0);
}

static bool rangeLt(StackTrace::PerfMap::Range a,
                    StackTrace::PerfMap::Range b) {
  return rangeCmp(a, b) < 0;
}

static bool rangeEq(StackTrace::PerfMap::Range a,
                    StackTrace::PerfMap::Range b) {
  return rangeCmp(a, b) == 0;
}

static bool rangeGt(StackTrace::PerfMap::Range a,
                    StackTrace::PerfMap::Range b) {
  return rangeCmp(a, b) > 0;
}

TEST(StackTraceTest, Cmp) {
  //                          1  1
  //    0  1  234  5  6  789  0  1
  // a:        [----------)
  // b: |
  EXPECT_TRUE(rangeGt({3,8}, {0,0}));
  // a:        [----------)
  // b: [--)
  EXPECT_TRUE(rangeGt({3,8}, {0,1}));
  // a:        [----------)
  // b: [-----)
  EXPECT_TRUE(rangeGt({3,8}, {0,2}));
  // a:        [----------)
  // b: [------)
  EXPECT_TRUE(rangeGt({3,8}, {0,3}));
  // a:        [----------)
  // b: [-------)
  EXPECT_TRUE(rangeEq({3,8}, {0,4}));
  // a:        [----------)
  // b: [-------)
  EXPECT_TRUE(rangeEq({3,8}, {0,5}));
  // a:        [----------)
  // b: [----------------)
  EXPECT_TRUE(rangeEq({3,8}, {0,7}));
  // a:        [----------)
  // b: [-----------------)
  EXPECT_TRUE(rangeEq({3,8}, {0,8}));
  // a:        [----------)
  // b: [------------------)
  EXPECT_TRUE(rangeEq({3,8}, {0,9}));
  // a:        [----------)
  // b: [---------------------)
  EXPECT_TRUE(rangeEq({3,8}, {0,10}));

  //                          1  1
  //    0  1  234  5  6  789  0  1
  // a:        [----------)
  // b:       |
  EXPECT_TRUE(rangeGt({3,8}, {2,2}));
  // a:        [----------)
  // b:       [)
  EXPECT_TRUE(rangeGt({3,8}, {2,3}));
  // a:        [----------)
  // b:       [-)
  EXPECT_TRUE(rangeEq({3,8}, {2,4}));
  // a:        [----------)
  // b:       [----)
  EXPECT_TRUE(rangeEq({3,8}, {2,5}));
  // a:        [----------)
  // b:       [----------)
  EXPECT_TRUE(rangeEq({3,8}, {2,7}));
  // a:        [----------)
  // b:       [-----------)
  EXPECT_TRUE(rangeEq({3,8}, {2,8}));
  // a:        [----------)
  // b:       [------------)
  EXPECT_TRUE(rangeEq({3,8}, {2,9}));
  // a:        [----------)
  // b:       [---------------)
  EXPECT_TRUE(rangeEq({3,8}, {2,10}));

  //                          1  1
  //    0  1  234  5  6  789  0  1
  // a:        [----------)
  // b:        |
  EXPECT_TRUE(rangeGt({3,8}, {3,3}));
  // a:        [----------)
  // b:        [)
  EXPECT_TRUE(rangeEq({3,8}, {3,4}));
  // a:        [----------)
  // b:        [---)
  EXPECT_TRUE(rangeEq({3,8}, {3,5}));
  // a:        [----------)
  // b:        [---------)
  EXPECT_TRUE(rangeEq({3,8}, {3,7}));
  // a:        [----------)
  // b:        [----------)
  EXPECT_TRUE(rangeEq({3,8}, {3,8}));
  // a:        [----------)
  // b:        [-----------)
  EXPECT_TRUE(rangeEq({3,8}, {3,9}));
  // a:        [----------)
  // b:        [--------------)
  EXPECT_TRUE(rangeEq({3,8}, {3,10}));

  //                          1  1
  //    0  1  234  5  6  789  0  1
  // a:        [----------)
  // b:         |
  EXPECT_TRUE(rangeEq({3,8}, {4,4}));
  // a:        [----------)
  // b:         [--)
  EXPECT_TRUE(rangeEq({3,8}, {4,5}));
  // a:        [----------)
  // b:         [--------)
  EXPECT_TRUE(rangeEq({3,8}, {4,7}));
  // a:        [----------)
  // b:         [---------)
  EXPECT_TRUE(rangeEq({3,8}, {4,8}));
  // a:        [----------)
  // b:         [----------)
  EXPECT_TRUE(rangeEq({3,8}, {4,9}));
  // a:        [----------)
  // b:         [-------------)
  EXPECT_TRUE(rangeEq({3,8}, {4,10}));

  //                          1  1
  //    0  1  234  5  6  789  0  1
  // a:        [----------)
  // b:            |
  EXPECT_TRUE(rangeEq({3,8}, {5,5}));
  // a:        [----------)
  // b:            [--)
  EXPECT_TRUE(rangeEq({3,8}, {5,6}));
  // a:        [----------)
  // b:            [-----)
  EXPECT_TRUE(rangeEq({3,8}, {5,7}));
  // a:        [----------)
  // b:            [------)
  EXPECT_TRUE(rangeEq({3,8}, {5,8}));
  // a:        [----------)
  // b:            [-------)
  EXPECT_TRUE(rangeEq({3,8}, {5,9}));
  // a:        [----------)
  // b:            [----------)
  EXPECT_TRUE(rangeEq({3,8}, {5,10}));

  //                          1  1
  //    0  1  234  5  6  789  0  1
  // a:        [----------)
  // b:                  |
  EXPECT_TRUE(rangeEq({3,8}, {7,7}));
  // a:        [----------)
  // b:                  [)
  EXPECT_TRUE(rangeEq({3,8}, {7,8}));
  // a:        [----------)
  // b:                  [-)
  EXPECT_TRUE(rangeEq({3,8}, {7,9}));
  // a:        [----------)
  // b:                  [----)
  EXPECT_TRUE(rangeEq({3,8}, {7,10}));

  //                          1  1
  //    0  1  234  5  6  789  0  1
  // a:        [----------)
  // b:                   |
  EXPECT_TRUE(rangeLt({3,8}, {8,8}));
  // a:        [----------)
  // b:                   [)
  EXPECT_TRUE(rangeLt({3,8}, {8,9}));
  // a:        [----------)
  // b:                   [---)
  EXPECT_TRUE(rangeLt({3,8}, {8,10}));

  //                          1  1
  //    0  1  234  5  6  789  0  1
  // a:        [----------)
  // b:                    |
  EXPECT_TRUE(rangeLt({3,8}, {9,9}));
  // a:        [----------)
  // b:                    [--)
  EXPECT_TRUE(rangeLt({3,8}, {9,10}));

  //                          1  1
  //    0  1  234  5  6  789  0  1
  // a:        [----------)
  // b:                       |
  EXPECT_TRUE(rangeLt({3,8}, {10,10}));
  // a:        [----------)
  // b:                       [--)
  EXPECT_TRUE(rangeLt({3,8}, {10,11}));

  //    0  1  2
  // a:    |
  // b: |
  EXPECT_TRUE(rangeGt({1,1}, {0,0}));
  // a:    |
  // b:    |
  EXPECT_TRUE(rangeEq({1,1}, {1,1}));
  // a:    |
  // b:       |
  EXPECT_TRUE(rangeLt({1,1}, {2,2}));
}

}

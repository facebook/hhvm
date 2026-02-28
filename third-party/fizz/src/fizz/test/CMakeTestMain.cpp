/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/init/Init.h>
#include <folly/portability/GTest.h>

/*
 * This is the recommended main function for all tests.
 * The Makefile links it into all of the test programs so that tests do not need
 * to - and indeed should typically not - define their own main() functions
 */
#ifndef _MSC_VER
int main(int argc, char** argv) __attribute__((__weak__));
#endif

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  folly::Init follyInit(&argc, &argv);
  return RUN_ALL_TESTS();
}

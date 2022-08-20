/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/scm/Mercurial.h"
#include <folly/portability/GTest.h>

using namespace std::chrono;

TEST(Mercurial, convertCommitDate) {
  auto date = watchman::Mercurial::convertCommitDate("1529420960.025200");
  auto result = duration_cast<seconds>(date.time_since_epoch()).count();
  auto expected = 1529420960;
  EXPECT_EQ(result, expected);
}

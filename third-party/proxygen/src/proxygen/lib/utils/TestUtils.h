/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Random.h>
#include <folly/Range.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/portability/SysResource.h>

#ifndef NDEBUG
#define EXPECT_DEATH_NO_CORE(token, regex) \
  {                                        \
    rlimit oldLim;                         \
    getrlimit(RLIMIT_CORE, &oldLim);       \
    rlimit newLim{0, oldLim.rlim_max};     \
    setrlimit(RLIMIT_CORE, &newLim);       \
    EXPECT_DEATH(token, regex);            \
    setrlimit(RLIMIT_CORE, &oldLim);       \
  }
#else
#define EXPECT_DEATH_NO_CORE(tken, regex) \
  {}
#endif

inline folly::StringPiece getContainingDirectory(folly::StringPiece input) {
  auto pos = folly::rfind(input, '/');
  if (pos == std::string::npos) {
    pos = 0;
  } else {
    pos += 1;
  }
  return input.subpiece(0, pos);
}

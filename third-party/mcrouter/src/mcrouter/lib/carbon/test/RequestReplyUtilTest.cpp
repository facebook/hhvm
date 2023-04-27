/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstddef>

#include "mcrouter/lib/carbon/RequestReplyUtil.h"

using namespace carbon;

namespace {

struct RequestOne {
  static constexpr size_t typeId = 1;
};

struct RequestTwo {
  static constexpr size_t typeId = 2;
};

struct RequestThree {
  static constexpr size_t typeId = 3;
};

template <class List, int expectedMin, int expectedMax, int expectedSize>
struct ListLimitChecker {
  static constexpr bool valid =
      RequestListLimits<List>::minTypeId == expectedMin &&
      RequestListLimits<List>::maxTypeId == expectedMax &&
      RequestListLimits<List>::typeIdRangeSize == expectedSize;
};

static_assert(
    ListLimitChecker<
        carbon::List<RequestOne>,
        /* expMin */ 1,
        /* expMax */ 1,
        /* expSiz */ 1>::valid,
    "Invalid limits");

static_assert(
    ListLimitChecker<
        carbon::List<RequestTwo>,
        /* expMin */ 2,
        /* expMax */ 2,
        /* expSiz */ 1>::valid,
    "Invalid limits");

static_assert(
    ListLimitChecker<
        carbon::List<RequestOne, RequestTwo>,
        /* expMin */ 1,
        /* expMax */ 2,
        /* expSiz */ 2>::valid,
    "Invalid limits");

static_assert(
    ListLimitChecker<
        carbon::List<RequestOne, RequestTwo, RequestThree>,
        /* expMin */ 1,
        /* expMax */ 3,
        /* expSiz */ 3>::valid,
    "Invalid limits");

static_assert(
    ListLimitChecker<
        carbon::List<RequestOne, RequestThree>,
        /* expMin */ 1,
        /* expMax */ 3,
        /* expSiz */ 3>::valid,
    "Invalid limits");

static_assert(
    ListLimitChecker<
        carbon::List<RequestOne, RequestThree, RequestTwo>,
        /* expMin */ 1,
        /* expMax */ 3,
        /* expSiz */ 3>::valid,
    "Invalid limits");

static_assert(
    ListLimitChecker<
        carbon::List<RequestThree, RequestOne>,
        /* expMin */ 1,
        /* expMax */ 3,
        /* expSiz */ 3>::valid,
    "Invalid limits");

static_assert(
    ListLimitChecker<
        carbon::List<RequestThree, RequestTwo>,
        /* expMin */ 2,
        /* expMax */ 3,
        /* expSiz */ 2>::valid,
    "Invalid limits");

} // anonymous namespace

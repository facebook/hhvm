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
#include <folly/String.h>
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
  {                                       \
  }
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

inline bool queryStringsAreEquivalent(const std::string& query,
                                      const std::string& expectedQuery) {
  std::unordered_set<std::string> urlQueryParams;
  std::unordered_set<std::string> expectedUrlQueryParams;
  folly::splitTo<std::string>(
      '&', query, std::inserter(urlQueryParams, urlQueryParams.begin()));
  folly::splitTo<std::string>(
      '&',
      expectedQuery,
      std::inserter(expectedUrlQueryParams, expectedUrlQueryParams.begin()));

  if (urlQueryParams.size() != expectedUrlQueryParams.size()) {
    return false;
  }
  for (const auto& element : urlQueryParams) {
    if (expectedUrlQueryParams.find(element) == expectedUrlQueryParams.end()) {
      return false;
    }
  }
  return true;
}

inline bool urlsAreEquivalent(const std::string& url,
                              const std::string& expectedUrl) {
  std::vector<std::string> urlFragments;
  std::vector<std::string> expectedUrlFragments;

  folly::split('#', url, urlFragments, true);
  folly::split('#', expectedUrl, expectedUrlFragments, true);

  if (urlFragments.size() != expectedUrlFragments.size()) {
    return false;
  }

  if (urlFragments.size() == 2 &&
      (urlFragments[1] != expectedUrlFragments[1])) {
    return false;
  }

  std::vector<std::string> urlPreludeAndQuery;
  std::vector<std::string> expectedUrlPreludeAndQuery;

  folly::split('?', urlFragments[0], urlPreludeAndQuery, true);
  folly::split('?', expectedUrlFragments[0], expectedUrlPreludeAndQuery, true);

  if (urlPreludeAndQuery.size() != expectedUrlPreludeAndQuery.size()) {
    return false;
  }

  if (urlPreludeAndQuery[0] != expectedUrlPreludeAndQuery[0]) {
    return false;
  }

  if (urlPreludeAndQuery.size() == 2) {
    return queryStringsAreEquivalent(urlPreludeAndQuery[1],
                                     expectedUrlPreludeAndQuery[1]);
  } else {
    return true;
  }
}

/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WatchmanConnection.h"

namespace watchman {

using namespace folly;

static const dynamic kError("error");

WatchmanResponseError::WatchmanResponseError(const folly::dynamic& response)
    : WatchmanError(response[kError].c_str()), response_(response) {}

const folly::dynamic& WatchmanResponseError::getResponse() const {
  return response_;
}
} // namespace watchman

/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/CPortability.h>
#include <stdexcept>

namespace facebook::db {

class FOLLY_EXPORT Exception : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

class FOLLY_EXPORT OperationStateException : public Exception {
 public:
  using Exception::Exception;
};

class FOLLY_EXPORT InvalidConnectionException : public Exception {
 public:
  using Exception::Exception;
};

} // namespace facebook::db

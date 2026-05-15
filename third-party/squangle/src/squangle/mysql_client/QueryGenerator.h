/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/container/F14Set.h>

#include "squangle/mysql_client/Query.h"

namespace facebook::common::mysql_client {

class QueryGenerator {
 public:
  virtual ~QueryGenerator() = default;
  virtual Query query() = 0;

  virtual folly::F14FastSet<std::string> tables() {
    return {};
  }

  virtual bool isReadOnly() const {
    return false;
  }
};

} // namespace facebook::common::mysql_client

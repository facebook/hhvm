/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/Query.h"

namespace facebook::common::mysql_client {

class QueryGenerator {
 public:
  virtual ~QueryGenerator() = default;
  virtual Query query() = 0;
};

} // namespace facebook::common::mysql_client

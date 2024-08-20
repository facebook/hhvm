/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>

#include "squangle/mysql_client/AsyncMysqlClient.h"
#include "squangle/mysql_client/Operation.h"

namespace facebook::common::mysql_client {

// This will allow callback users to be able to read all rows in the end of
// operation (Success or Failure).
// The Appender callbacks will only be called for `Success` and `Failure`
// and pass the whole result of the query or multi query.
typedef std::function<void(QueryOperation&, QueryResult, QueryCallbackReason)>
    QueryAppenderCallback;
typedef std::function<
    void(MultiQueryOperation&, std::vector<QueryResult>, QueryCallbackReason)>
    MultiQueryAppenderCallback;

QueryCallback resultAppender(QueryAppenderCallback&& callback);

MultiQueryCallback resultAppender(MultiQueryAppenderCallback&& callback);

} // namespace facebook::common::mysql_client

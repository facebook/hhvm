/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

/*
 * toFuture is our interface for database operation using folly::Future.
 * It's completely compatible with the `Operation` interface, so to use
 * futures all you need is to pass the `Operation`.
 *
 */

#pragma once

#include "squangle/mysql_client/DbResult.h"

#include <folly/futures/Future.h>

namespace facebook::common::mysql_client {

class ConnectOperation;
class QueryOperation;
class MultiQueryOperation;

using ConnectOperation_ptr = std::shared_ptr<ConnectOperation>;
using QueryOperation_ptr = std::shared_ptr<QueryOperation>;
using MultiQueryOperation_ptr = std::shared_ptr<MultiQueryOperation>;

// SemiFuture for ConnectOperation
FOLLY_NODISCARD folly::SemiFuture<ConnectResult> toSemiFuture(
    const ConnectOperation_ptr& conn_op);

// SemiFuture for QueryOperation
FOLLY_NODISCARD folly::SemiFuture<DbQueryResult> toSemiFuture(
    QueryOperation_ptr query_op);

// SemiFuture for MultiQueryOperation
FOLLY_NODISCARD folly::SemiFuture<DbMultiQueryResult> toSemiFuture(
    MultiQueryOperation_ptr mquery_op);

} // namespace facebook::common::mysql_client

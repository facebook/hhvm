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

#ifndef COMMON_ASYNC_MYSQL_FUTURE_ADAPTER_H
#define COMMON_ASYNC_MYSQL_FUTURE_ADAPTER_H

#include "squangle/mysql_client/DbResult.h"
#include "squangle/mysql_client/Operation.h"

#include <folly/futures/Future.h>

namespace facebook {
namespace common {
namespace mysql_client {

typedef std::shared_ptr<ConnectOperation> ConnectOperation_ptr;
typedef std::shared_ptr<QueryOperation> QueryOperation_ptr;
typedef std::shared_ptr<MultiQueryOperation> MultiQueryOperation_ptr;

// SemiFuture for ConnectOperation
FOLLY_NODISCARD folly::SemiFuture<ConnectResult> toSemiFuture(
    ConnectOperation_ptr conn_op);

// SemiFuture for QueryOperation
FOLLY_NODISCARD folly::SemiFuture<DbQueryResult> toSemiFuture(
    QueryOperation_ptr query_op);

// SemiFuture for MultiQueryOperation
FOLLY_NODISCARD folly::SemiFuture<DbMultiQueryResult> toSemiFuture(
    MultiQueryOperation_ptr mquery_op);

} // namespace mysql_client
} // namespace common
} // namespace facebook

#endif // COMMON_ASYNC_MYSQL_FUTURE_ADAPTER_H

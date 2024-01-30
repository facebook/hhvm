/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/AsyncHelpers.h"

namespace facebook {
namespace common {
namespace mysql_client {

QueryCallback resultAppender(QueryAppenderCallback&& callback) {
  return [callback = std::move(callback)](
             QueryOperation& op, QueryResult* res, QueryCallbackReason reason) {
    if (reason != QueryCallbackReason::RowsFetched) {
      QueryResult result{0};
      if (op.ok()) {
        // Can't access query result on error
        result = op.stealQueryResult();
      }
      callback(op, std::move(result), reason);
    }
  };
}

MultiQueryCallback resultAppender(MultiQueryAppenderCallback&& callback) {
  return [callback = std::move(callback)](
             MultiQueryOperation& op,
             QueryResult* res,
             QueryCallbackReason reason) {
    if (reason != QueryCallbackReason::RowsFetched &&
        reason != QueryCallbackReason::QueryBoundary) {
      // stealQueryResults is always allowed since in multiQuery
      // the first few queries might succeed and then be followed by a failure
      callback(op, op.stealQueryResults(), reason);
    }
  };
}
} // namespace mysql_client
} // namespace common
} // namespace facebook

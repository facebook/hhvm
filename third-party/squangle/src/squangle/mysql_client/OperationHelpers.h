/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdexcept>

#include "squangle/mysql_client/FetchOperation.h"

namespace facebook::common::mysql_client {

// Thrown by makeRowBlockFromStream() when the rows coming off the wire do not
// match the column metadata the server declared -- e.g. more values in a row
// than there are fields.
//
// This exists as a distinct type so the fetch loop can catch *only* row
// building. That loop runs inside a noexcept libevent callback and also
// invokes consumer callbacks (notifyInitQuery, notifyQuerySuccess, and the
// buffered/stream callbacks reached through notifyRowsReady); catching
// std::exception there would silently swallow consumer exceptions too, which
// is a behavior change for every squangle client. Consumer exceptions keep
// their existing behavior of terminating the process.
class MalformedResultError : public std::runtime_error {
 public:
  explicit MalformedResultError(const std::string& what)
      : std::runtime_error(what) {}
};

// Drains `row_stream` into a RowBlock. Throws MalformedResultError if the
// server's rows disagree with its column metadata.
RowBlock makeRowBlockFromStream(
    std::shared_ptr<RowFields> row_fields,
    RowStream* row_stream);

} // namespace facebook::common::mysql_client

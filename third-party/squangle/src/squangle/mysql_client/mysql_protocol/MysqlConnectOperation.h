/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/ConnectOperation.h"

namespace facebook::common::mysql_client::mysql_protocol {

// MysqlConnectOperation is a UNIFIED class that combines ConnectOperation logic
// with MySQL protocol implementation using COMPOSITION.
//
// Key difference from legacy pattern:
// - Legacy: Caller creates MysqlConnectOperationImpl, passes to
// ConnectOperation
// - Unified: MysqlConnectOperation creates its own impl internally
//
class MysqlConnectOperation : public ConnectOperation {
 public:
  // Factory method that takes client and connection key, handles wrapping
  // internally
  static std::shared_ptr<MysqlConnectOperation> create(
      MysqlClientBase* client,
      std::shared_ptr<const ConnectionKey> conn_key);

 private:
  explicit MysqlConnectOperation(std::unique_ptr<ConnectOperationImpl> impl);
};

} // namespace facebook::common::mysql_client::mysql_protocol

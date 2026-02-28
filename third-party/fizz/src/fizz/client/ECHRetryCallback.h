/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/Actions.h>

namespace fizz {
namespace client {

/**
 * ECHRetryCallback is used to convey ECHRetryConfigs that are
 * received by the client.
 */
class ECHRetryCallback {
 public:
  virtual ~ECHRetryCallback() = default;

  /**
   * retryAvailable may be invoked whenever the client receives a list of
   * ECHRetryConfigs from the server.
   *
   * There is no guarantee that this callback will be invoked on a connection.
   */
  virtual void retryAvailable(ECHRetryAvailable retry) = 0;
};

} // namespace client
} // namespace fizz

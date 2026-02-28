/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/State.h>

namespace fizz {
namespace client {

enum class EarlyDataRejectionPolicy {
  /**
   * Treat early data rejection as a fatal error. An EARLY_DATA_REJECTED
   * AsyncSocketException will be delivered to the read callback.
   */
  FatalConnectionError,

  /**
   * Automatically resend early data as normal data. Will only be done if the
   * following connection parameters match what was used for early data:
   *  - Application Protocol
   *  - TLS Protocol Version
   *  - TLS Cipher Suite
   *  - Client Identity
   *  - Server Identity
   */
  AutomaticResend,
};

bool earlyParametersMatch(const State&);

} // namespace client
} // namespace fizz

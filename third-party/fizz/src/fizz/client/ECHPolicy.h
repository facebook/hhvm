/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/ech/Types.h>
#include <folly/Optional.h>

namespace fizz::client {

class ECHPolicy {
 public:
  virtual ~ECHPolicy() = default;

  /**
   *  Finds corresponding ech config from policies for the corresponding SNI.
   *  @returns folly::none if config is not found, the ECH config for SNI
   *  otherwise.
   */
  virtual folly::Optional<std::vector<fizz::ech::ECHConfig>> getConfig(
      const std::string& hostname) const = 0;
};

} // namespace fizz::client

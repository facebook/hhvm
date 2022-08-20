/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>

namespace facebook {
namespace memcache {

/**
 * Represents the load on a given server.
 */
class ServerLoad {
 public:
  /**
   * Create server load from it's raw value.
   *
   * @param load  Raw value of server load (a number between 0 and 1,000,000)
   *              that represents the current load on the server.
   *              0 means no load, and 1000000 means max load.
   */
  explicit ServerLoad(uint32_t rawLoad) noexcept;

  /**
   * Create a ServerLoad instance given a load percentage
   *
   * @param percentLoad   Load on server represented by a number
   *                      between 0.0 and 100.0.
   */
  static ServerLoad fromPercentLoad(double percentLoad) noexcept;

  /**
   * Represents empty server load (i.e. no load at the server).
   */
  static const ServerLoad zero() noexcept;

  /**
   * Complement of load on server.
   *
   * @return  Complement of load on server
   */
  ServerLoad complement() const noexcept;

  /**
   * Percent of load on server.
   *
   * @return  Load percent on server (number between 0 and 100).
   */
  double percentLoad() const noexcept;

  /**
   * Returns the raw value of server load (i.e. number between 0 and 10e6).
   */
  uint32_t raw() const noexcept {
    return load_;
  }

  /**
   * Tells whether or not server load is 0.
   */
  bool isZero() const noexcept {
    return load_ == 0;
  }

 private:
  // Number between 0 and 10^6 (1000000) that represents the load on the server.
  uint32_t load_{0};
};

} // namespace memcache
} // namespace facebook

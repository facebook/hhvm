/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace proxygen {

/**
 * Base class for all the Configuration objects
 */
class ServiceConfiguration {
 public:
  ServiceConfiguration() : takeoverEnabled_(false) {
  }

  virtual ~ServiceConfiguration() {
  }

  /**
   * Set/get whether or not we should enable socket takeover
   */
  void setTakeoverEnabled(bool enabled) {
    takeoverEnabled_ = enabled;
  }
  bool takeoverEnabled() const {
    return takeoverEnabled_;
  }

 private:
  bool takeoverEnabled_;
};

} // namespace proxygen

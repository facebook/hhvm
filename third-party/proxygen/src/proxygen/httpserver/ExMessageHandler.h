/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/httpserver/RequestHandler.h>

namespace proxygen {

/**
 * Interface to be implemented to handle EX messages from client.
 */
class ExMessageHandler : public RequestHandler {
 public:
  virtual void onUpgrade(proxygen::UpgradeProtocol /*prot*/) noexcept override {
    LOG(FATAL) << "ExMessageHandler doesn't support upgrade";
  }

  virtual ExMessageHandler* getExHandler() noexcept override {
    LOG(FATAL) << "getExHandler can't be called on ExMessageHandler";
  }
};

} // namespace proxygen

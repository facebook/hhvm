/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/session/HTTPUpstreamSession.h>

namespace ProxyService {
class SessionWrapper : public proxygen::HTTPSession::InfoCallback {
 private:
  proxygen::HTTPUpstreamSession* session_{nullptr};

 public:
  explicit SessionWrapper(proxygen::HTTPUpstreamSession* session)
      : session_(session) {
    session_->setInfoCallback(this);
  }

  ~SessionWrapper() override {
    if (session_) {
      session_->drain();
    }
  }

  proxygen::HTTPUpstreamSession* operator->() const {
    return session_;
  }

  // Note: you must not start any asynchronous work from onDestroy()
  void onDestroy(const proxygen::HTTPSessionBase&) override {
    session_ = nullptr;
  }
};

} // namespace ProxyService

/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/HTTPSettings.h>
#include <quic/state/EarlyDataAppParamsHandler.h>

namespace proxygen {

/**
 * EarlyDataAppParamsHandler for HTTP/3.
 *
 * Serializes/validates HTTP/3 SETTINGS for 0-RTT early data support
 * per RFC 9114 Section 7.2.4.2.
 *
 * Server mode: setCurrentSettings() is called at server startup with the
 * server's own egress settings. validate() checks that the client's cached
 * settings are compatible with current settings (limits have not decreased,
 * enabled features have not been disabled). A single handler instance is
 * shared across all connections.
 *
 * Client mode: The handler is created without settings (setCurrentSettings()
 * not called before the connection). validate() on reconnect just verifies
 * the cached blob is parseable and allows 0-RTT — the server does the real
 * compatibility check. After connecting, setCurrentSettings() is called with
 * the server's SETTINGS received via onSettings(), and get() serializes them
 * for caching in the PSK when the NewSessionTicket arrives. Each connection
 * has its own handler instance (owned by the session).
 */
class H3EarlyDataHandler : public quic::EarlyDataAppParamsHandler {
 public:
  void setCurrentSettings(HTTPSettings settings);
  void setCurrentSettings(const SettingsList& settings);

  HTTPSettings& getSettings() noexcept {
    return settings_;
  }

  [[nodiscard]] bool hasSettings() const noexcept {
    return settingsInitialized_;
  }

  bool validate(const quic::Optional<std::string>& alpn,
                const quic::BufPtr& appParams) override;

  quic::BufPtr get() override;

 private:
  HTTPSettings settings_;
  bool settingsInitialized_{false};
};

} // namespace proxygen

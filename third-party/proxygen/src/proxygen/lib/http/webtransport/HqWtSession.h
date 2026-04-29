/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/webtransport/QuicWtSession.h>
#include <proxygen/lib/http/webtransport/WebTransportSession.h>

namespace proxygen::detail {

class HqWtSession : public WtHttpSession {
 public:
  using Ptr = std::shared_ptr<HqWtSession>;

  static Ptr make(folly::EventBase* evb,
                  detail::WtStreamManager::WtConfig wtConfig,
                  WebTransportHandler::Ptr wtHandler,
                  std::shared_ptr<quic::QuicSocket> quicSocket,
                  uint64_t connectStreamId);

  ~HqWtSession() noexcept override = default;

  /**
   * note: Only ::onHeadersComplete & ::onError will be invoked on observer (if
   * set). This is so client code (HTTPUpstreamSession) can determine if
   * the WebTransport was successful and return the HTTPMessage/error to the
   * application.
   */
  void init(Ptr self,
            HttpWtClientCallbackPtr wtClientCallback = nullptr) noexcept;

  // invoked by HttpTransaction on http error
  void onHttpError(const HTTPException& err) noexcept override;

  // invoked when both reads&writes are done; derived classes can clean up
  void onDone() noexcept override;

  H3WtSession& getH3WtSession() noexcept {
    return wtSess_;
  }

 private:
  HqWtSession(WtLooper& readLooper,
              WtLooper& writeLooper,
              std::shared_ptr<QuicSocket> quicSocket,
              WebTransportHandler::Ptr wtHandler,
              WtStreamManager::WtConfig wtConfig,
              uint64_t connectStreamId,
              H3ConnectStreamCallback& cb) noexcept;
  H3WtSession wtSess_;
};

} // namespace proxygen::detail

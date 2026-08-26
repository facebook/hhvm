/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <glog/logging.h>
#include <folly/ExceptionWrapper.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/common/TLSStats.h>

namespace apache::thrift::fast_thrift::connection::security::handler {

HANDLER_TAG(tls_metrics_handler);

/**
 * Inner-pipeline handler that counts security outcomes. Completely
 * pass-through — it reads the resolved message but never modifies it.
 *
 * Sits at the tail end of the TLS pipeline, which is the one position that
 * sees every outcome. The work path runs tail→head and reaches here before
 * any stage has run, so it is not what this counts. Both the things worth
 * counting travel the other way:
 *
 *   success — TLSFinalizer turns the resolved connection around onto the read
 *             path, which runs head→tail and still carries what the handshake
 *             recorded about the peer. That record cannot be re-read later: a
 *             StopTLS downgrade replaces the transport that could report it.
 *   failure — a stage that gives up fires an exception, and exceptions also
 *             propagate head→tail, so every stage's failures pass through
 *             here on their way out.
 *
 * A plaintext connection under SSLPolicy::PERMITTED is read here too, with no
 * peerSecurity, and is deliberately counted as nothing: it completed no
 * handshake.
 */
class TLSMetricsHandler {
 public:
  // Borrowed for the handler's lifetime. The shard belongs to the server,
  // which outlives every per-EventBase pipeline.
  explicit TLSMetricsHandler(TLSStatsShard* stats) noexcept : stats_(stats) {
    DCHECK(stats_ != nullptr);
  }

  // === Outbound (work path, passthrough) ===
  //
  // Nothing has happened to the connection yet at this point — no stage has
  // run — so there is nothing here to count.

  template <typename Context>
  [[nodiscard]] channel_pipeline::Result onWrite(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }

  // === Inbound (resolved connections) ===

  template <typename Context>
  [[nodiscard]] channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    DCHECK(stats_ != nullptr);
    const auto& peerSecurity = msg.get<TLSResponseMessage>().peerSecurity;
    if (peerSecurity != nullptr) {
      stats_->tlsComplete.incrementValue(1);
      if (peerSecurity->sessionResumed) {
        stats_->tlsResumption.incrementValue(1);
      }
      if (peerSecurity->peerCertificate != nullptr) {
        stats_->tlsWithClientCert.incrementValue(1);
      }
    }
    return ctx.fireRead(std::move(msg));
  }

  // Every connection a stage gave up on passes through here — see
  // StageFailure.h. One number for "accepted but never served", whichever
  // stage decided that.
  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    DCHECK(stats_ != nullptr);
    stats_->tlsError.incrementValue(1);
    ctx.fireException(std::move(e));
  }

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

 private:
  TLSStatsShard* stats_;
};

} // namespace apache::thrift::fast_thrift::connection::security::handler

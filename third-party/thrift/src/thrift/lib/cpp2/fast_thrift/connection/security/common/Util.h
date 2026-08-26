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

#include <string>
#include <string_view>

#include <folly/ExceptionWrapper.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSocketException.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>

namespace apache::thrift::fast_thrift::connection::security {

/**
 * Report that a parked TLS stage gave up on a connection.
 *
 * A stage takes ownership of the transport while its helper runs, so the work
 * path *ends* there — the message is not forwarded, and on failure the helper
 * is destroyed and takes the socket with it. Nothing downstream would ever
 * learn the connection existed, let alone that it failed.
 *
 * Firing an exception is what closes that gap. It travels the inner pipeline
 * to its tail adapter, which hands it to ConnectionTLSHandler, which re-fires
 * it onto the acceptance pipeline — so a single observer there sees every
 * stage's failures instead of each stage reporting for itself.
 *
 * This does not close anything. The connection is already gone by the time a
 * stage calls this; the exception carries the news, not the teardown.
 *
 * A null `ctx` means the handler has been removed from the pipeline, which is
 * the normal shutdown race — there is nowhere to report to, and the caller is
 * expected to have logged it.
 */
inline void fireConnectionFailure(
    channel_pipeline::detail::ContextImpl* ctx,
    const folly::exception_wrapper& ex,
    std::string_view what,
    const folly::SocketAddress& clientAddr) noexcept {
  if (ctx == nullptr) {
    return;
  }
  // Always rebuilt rather than forwarding `ex` as-is, even though that costs
  // the original exception's type. What reaches an observer is a message, and
  // the underlying exception carries only what the TLS library saw — not
  // which stage was running or who the peer was. Forwarding verbatim leaves
  // that context in the DBG3 line at the call site, which is off in
  // production, so the one report that does get out says the least.
  //
  // Nothing downstream dispatches on the type today; if something ever needs
  // to, this wants a dedicated exception carrying `ex` rather than folding it
  // into text.
  std::string message{what};
  message += " for ";
  message += clientAddr.describe();
  if (ex) {
    message += ": ";
    message += ex.what().toStdString();
  }
  ctx->fireException(
      folly::make_exception_wrapper<folly::AsyncSocketException>(
          folly::AsyncSocketException::AsyncSocketExceptionType::INTERNAL_ERROR,
          std::move(message)));
}

} // namespace apache::thrift::fast_thrift::connection::security

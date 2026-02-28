/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPError.h"
#include "proxygen/lib/http/coro/HTTPSourceFilter.h"
#include "proxygen/lib/http/coro/HTTPSourceFilterChain.h"
#include "proxygen/lib/http/coro/HTTPSourceHolder.h"
#include "proxygen/lib/http/coro/util/TimedBaton.h"
#include <folly/logging/xlog.h>

namespace proxygen::coro {

/**
 * HTTPSourceReader is a convenience class for reading from an HTTPSource.
 *
 * The caller can set callbacks to run as various parts of the message are read.
 * The callbacks can be synchronous (onHeaders, onBody) or coroutines
 * (onHeadersAsync), and can be mixed and matched on the same reader.
 *
 * Some events are terminal - onHeaders or onBody with eom specified,
 * onTrailers, and onError.
 *
 * For non-terminal headers or body, the callback returns a value indicating
 * how to proceed - Continue to continue processing or Stop to cancel the read.
 * * Stop will call stopReading on source if it was incomplete.
 *
 * For example:
 *
 * HTTPSourceReader reader(source);
 *
 * reader
 *   .onHeaders([] (std::unique_ptr<HTTPMessage> headers,
 *                  bool isFinal,
 *                  bool eom) {
 *       // handle headers.  isFinal is false if headers is an HTTP 1xx response
 *       if (eom) {
 *         // handle eom
 *       }
 *       // Return code is ignored when eom is true
 *       return HTTPSourceReader::Continue;
 *     })
 *   .onBody([] (BufQueue body, bool eom) {
 *       // body may be empty when eom is true
 *       if (!body.empty()) {
 *         // handle body
 *       }
 *       if (eom) {
 *         // handle EOM
 *       }
 *       // Return code is ignored when eom is true
 *       return HTTPSourceReader::Continue;
 *     })
 *   .onError([] (ErrorContext errContext, HTTPError error) {
 *       // An error occurred.  errContext specifies if the error happened
 *       // while awaiting headers or body.
 *       // handle error
 *     });
 *
 * co_await reader.read();
 */

class HTTPSourceReader {
 public:
  HTTPSourceReader() = default;

  explicit HTTPSourceReader(HTTPSourceHolder source) {
    filterChain_.setSource(source.release());
  }

  HTTPSourceReader& setSource(HTTPSource* source) {
    filterChain_.setSource(source);
    return *this;
  }

  HTTPSourceReader& setSource(HTTPSourceHolder source) {
    return setSource(source.release());
  }

  HTTPSourceReader& insertFilter(HTTPSourceFilter* filter) {
    filterChain_.insertEnd(filter);
    return *this;
  }

  constexpr static bool Continue = false;
  constexpr static bool Cancel = true;

  // Pre-read
  using PreReadFn = folly::Function<bool()>;
  using AsyncPreReadFn = folly::Function<folly::coro::Task<bool>()>;
  HTTPSourceReader& preRead(PreReadFn preReadFn);
  HTTPSourceReader& preReadAsync(AsyncPreReadFn preReadFn);

  // Headers, final, EOM
  using HeaderFn =
      folly::Function<bool(std::unique_ptr<HTTPMessage>, bool, bool)>;
  using AsyncHeaderFn = folly::Function<folly::coro::Task<bool>(
      std::unique_ptr<HTTPMessage>, bool, bool)>;
  HTTPSourceReader& onHeaders(HeaderFn headerFn);
  HTTPSourceReader& onHeadersAsync(AsyncHeaderFn headerFn);

  // Body, EOM
  using BodyFn = folly::Function<bool(BufQueue, bool)>;
  using AsyncBodyFn = folly::Function<folly::coro::Task<bool>(BufQueue, bool)>;
  HTTPSourceReader& onBody(BodyFn bodyFn);
  HTTPSourceReader& onBodyAsync(AsyncBodyFn bodyFn);

  // Datagram, EOM
  using DatagramFn = folly::Function<bool(std::unique_ptr<folly::IOBuf>)>;
  using AsyncDatagramFn =
      folly::Function<folly::coro::Task<bool>(std::unique_ptr<folly::IOBuf>)>;
  HTTPSourceReader& onDatagram(DatagramFn datagramFn);
  HTTPSourceReader& onDatagramAsync(AsyncDatagramFn datagramFn);

  // Promise, source for push, EOM
  using PushPromiseFn = folly::Function<bool(
      std::unique_ptr<HTTPMessage>, HTTPSourceHolder, bool)>;
  using AsyncPushPromiseFn = folly::Function<folly::coro::Task<bool>(
      std::unique_ptr<HTTPMessage>, HTTPSourceHolder, bool)>;
  HTTPSourceReader& onPushPromise(PushPromiseFn promiseFn);
  HTTPSourceReader& onPushPromiseAsync(AsyncPushPromiseFn promiseFn);

  // Trailers (EOM implicit)
  using TrailerFn = folly::Function<void(std::unique_ptr<HTTPHeaders>)>;
  using AsyncTrailerFn =
      folly::Function<folly::coro::Task<void>(std::unique_ptr<HTTPHeaders>)>;
  HTTPSourceReader& onTrailers(TrailerFn trailerFn);
  HTTPSourceReader& onTrailersAsync(AsyncTrailerFn trailerFn);

  enum class ErrorContext { HEADERS, BODY };
  using ErrorFn = folly::Function<void(ErrorContext, HTTPError)>;
  using AsyncErrorFn =
      folly::Function<folly::coro::Task<void>(ErrorContext, HTTPError)>;
  HTTPSourceReader& onError(ErrorFn errorFn);
  HTTPSourceReader& onErrorAsync(AsyncErrorFn errorFn);

  // maxBodySize is passed into ::readBodyEvent(maxBodySize)
  folly::coro::Task<void> read(
      uint32_t maxBodySize = std::numeric_limits<uint32_t>::max());

 private:
  FilterChain filterChain_;

  PreReadFn preReadFn_;
  HeaderFn headerFn_;
  BodyFn bodyFn_;
  DatagramFn datagramFn_;
  PushPromiseFn promiseFn_;
  TrailerFn trailerFn_;
  ErrorFn errorFn_;

  AsyncPreReadFn asyncPreReadFn_;
  AsyncHeaderFn asyncHeaderFn_;
  AsyncBodyFn asyncBodyFn_;
  AsyncDatagramFn asyncDatagramFn_;
  AsyncPushPromiseFn asyncPromiseFn_;
  AsyncTrailerFn asyncTrailerFn_;
  AsyncErrorFn asyncErrorFn_;
};

} // namespace proxygen::coro

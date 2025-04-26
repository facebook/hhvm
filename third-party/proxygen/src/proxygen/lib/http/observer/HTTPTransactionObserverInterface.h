/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>
#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/utils/Time.h>

namespace proxygen {

class HTTPTransactionObserverAccessor;

/**
 * Observer of HTTP transaction events.
 */
class HTTPTransactionObserverInterface {
 public:
  virtual ~HTTPTransactionObserverInterface() = default;

  enum class Events {
    TxnBytes = 1,
  };

  struct TxnBytesEvent {
    enum class Type : uint8_t {
      REQUEST_HEADERS_COMPLETE,
      FIRST_HEADER_BYTE_WRITE,
      FIRST_BODY_BYTE_WRITE,
      FIRST_BODY_BYTE_ACK,
      LAST_BODY_BYTE_WRITE,
      LAST_BODY_BYTE_ACK,
      BODY_BYTES_GENERATED,
      HEADER_BYTES_GENERATED,
    };

    const proxygen::TimePoint timestamp;
    const Type type;
    const folly::Optional<size_t> maybeNumBytes;
    const folly::Optional<std::reference_wrapper<const HTTPMessage>>
        maybeHeadersRef;

    TxnBytesEvent(TxnBytesEvent&&) = delete;
    TxnBytesEvent& operator=(const TxnBytesEvent&) = delete;
    TxnBytesEvent& operator=(TxnBytesEvent&& rhs) = delete;
    TxnBytesEvent(const TxnBytesEvent&) = delete;

    struct BuilderFields {
      folly::Optional<std::reference_wrapper<const proxygen::TimePoint>>
          maybeTimestampRef;
      Type type;
      size_t numBytes;
      folly::Optional<std::reference_wrapper<const HTTPMessage>>
          maybeHeadersRef;
      explicit BuilderFields() = default;
    };

    struct Builder : public BuilderFields {
      Builder&& setTimestamp(const proxygen::TimePoint& timestampIn);
      Builder&& setType(Type typeIn);
      Builder&& setNumBytes(const size_t numBytes);
      Builder&& setHeaders(const proxygen::HTTPMessage& headersIn);
      TxnBytesEvent build() &&;
      explicit Builder() = default;
    };

    // Use builder to construct
    explicit TxnBytesEvent(const BuilderFields& builderFields);
  };

  virtual void onBytesEvent(HTTPTransactionObserverAccessor* /* txn */,
                            const TxnBytesEvent& /* event */) noexcept {
  }
};

} // namespace proxygen

/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <chrono>
#include <cstdint>
#include <glog/logging.h>
#include <proxygen/lib/http/HTTPHeaders.h>
#include <utility>

namespace proxygen {

/**
 * Accessor object "observed" by HTTPSessionObservers.
 *
 * Instead of passing SessionObservers a pointer to HTTPSession or
 * HTTPSessionBase, we pass a pointer to this SessionAccessor object which
 * exposes functionality that can be used by the observers to get information
 * from the HTTP session and/or use public functions to manipulate the HTTP
 * session.
 *
 * This layer of indirection clearly defines the interface and expectations that
 * observers can expect from the HTTP session, which in turn enables the same
 * observer implementation to be used with different session implementations,
 * including future implementations which may not inherit from HTTPSessionBase.
 *
 * The lifetime of the HTTPSessionAccessor is tied to that of the underlying
 * HTTP session: when the session is destroyed, the accessor is destroyed as
 * well.
 */
class HTTPSessionObserverAccessor {
 public:
  virtual std::size_t sendPing(uint64_t data) = 0;
  virtual ~HTTPSessionObserverAccessor() = default;
};

/**
 * Observer of Http session events.
 */
class HTTPSessionObserverInterface {
 public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;
  enum class Events { requestStarted = 1, preWrite = 2, pingReply = 3 };

  virtual ~HTTPSessionObserverInterface() = default;

  /**
   * Event structures.
   */

  struct RequestStartedEvent {
    const TimePoint timestamp;
    const HTTPHeaders& requestHeaders;

    // Do not support copy or move given that requestHeaders is a ref.
    RequestStartedEvent(RequestStartedEvent&&) = delete;
    RequestStartedEvent& operator=(const RequestStartedEvent&) = delete;
    RequestStartedEvent& operator=(RequestStartedEvent&& rhs) = delete;
    RequestStartedEvent(const RequestStartedEvent&) = delete;

    struct BuilderFields {
      folly::Optional<std::reference_wrapper<const TimePoint>>
          maybeTimestampRef;
      folly::Optional<std::reference_wrapper<const HTTPHeaders>>
          maybeHTTPHeadersRef;
      explicit BuilderFields() = default;
    };

    struct Builder : public BuilderFields {
      Builder&& setTimestamp(const TimePoint& timestamp);
      Builder&& setHeaders(const proxygen::HTTPHeaders& headers);
      RequestStartedEvent build() &&;
      explicit Builder() = default;
    };

    // Use builder to construct.
    explicit RequestStartedEvent(const BuilderFields& builderFields);
  };

  struct PreWriteEvent {
    const uint64_t pendingEgressBytes;
    const TimePoint timestamp;

    PreWriteEvent(PreWriteEvent&&) = delete;
    PreWriteEvent& operator=(const PreWriteEvent&) = delete;
    PreWriteEvent& operator=(PreWriteEvent&& rhs) = delete;
    PreWriteEvent(const PreWriteEvent&) = delete;

    struct BuilderFields {
      folly::Optional<std::reference_wrapper<const uint64_t>>
          maybePendingEgressBytesRef;
      folly::Optional<std::reference_wrapper<const TimePoint>>
          maybeTimestampRef;

      explicit BuilderFields() = default;
    };

    struct Builder : public BuilderFields {
      Builder&& setPendingEgressBytes(const uint64_t& pendingEgressBytes);
      Builder&& setTimestamp(const TimePoint& timestamp);

      PreWriteEvent build() &&;
      explicit Builder() = default;
    };

    // Use builder to construct.
    explicit PreWriteEvent(BuilderFields& builderFields);
  };

  struct PingReplyEvent {
    const uint64_t id;
    const TimePoint timestamp;

    PingReplyEvent(PingReplyEvent&&) = delete;
    PingReplyEvent& operator=(const PingReplyEvent&) = delete;
    PingReplyEvent& operator=(PingReplyEvent&& rhs) = delete;
    PingReplyEvent(const PingReplyEvent&) = delete;

    struct BuilderFields {
      folly::Optional<std::reference_wrapper<const uint64_t>> maybeId;
      folly::Optional<std::reference_wrapper<const TimePoint>>
          maybeTimestampRef;
      explicit BuilderFields() = default;
    };

    struct Builder : public BuilderFields {
      Builder&& setId(const uint64_t& Id);
      Builder&& setTimestamp(const TimePoint& Timestamp);
      PingReplyEvent build() &&;
      explicit Builder() = default;
    };

    // Use builder to construct.
    explicit PingReplyEvent(BuilderFields& builderFields);
  };

  /**
   * Events.
   */

  /**
   * headersComplete() is invoked when all HTTP request headers are received.
   *
   * @param session  Http session.
   * @param event    RequestStartedEvent with details.
   */
  virtual void requestStarted(HTTPSessionObserverAccessor* /* session */,
                              const RequestStartedEvent& /* event */) noexcept {
  }

  /**
   * preWrite() is invoked just before transactions are about to write to
   * transport.
   *
   * @param session  Http session.
   * @param event    PreWriteEvent with details.
   */
  virtual void preWrite(HTTPSessionObserverAccessor* /* session */,
                        const PreWriteEvent& /* event */) noexcept {
  }

  virtual void pingReply(HTTPSessionObserverAccessor* /* session */,
                         const PingReplyEvent& /* event */) noexcept {
  }
};

} // namespace proxygen

/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPByteEvents.h"
#include "proxygen/lib/http/coro/util/TimedBaton.h"
#include <folly/coro/Task.h>
#include <folly/io/IOBuf.h>
#include <folly/logging/xlog.h>
#include <proxygen/lib/http/HTTPHeaderSize.h>
#include <proxygen/lib/http/HTTPMessage.h>
#include <quic/common/BufUtil.h>

namespace proxygen::coro {

class HTTPSource;

using BufQueue = quic::BufQueue;

/**
 * An Event containing HTTP headers.
 *
 * Request headers are always final.
 * Response headers can be non-final if they have a 1xx status code
 */
struct HTTPHeaderEvent {
  std::unique_ptr<HTTPMessage> headers;
  bool eom{false};
  std::vector<HTTPByteEventRegistration> byteEventRegistrations;

  // Placeholder until real ByteEvent callbacks are implemented
  folly::Function<void(HTTPHeaderSize headerSize) noexcept> egressHeadersFn{
      nullptr};

  HTTPHeaderEvent(std::unique_ptr<HTTPMessage> inHeaders, bool inEOM)
      : headers(std::move(inHeaders)), eom(inEOM) {
    XCHECK(headers->isFinal() || !eom);
  }

  HTTPHeaderEvent(HTTPHeaderEvent&& goner) = default;
  HTTPHeaderEvent& operator=(HTTPHeaderEvent&& goner) = default;

  bool isFinal() const noexcept {
    return headers->isFinal();
  }

  void describe(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const HTTPHeaderEvent& headerEvent);

struct HTTPPushEvent {
  std::unique_ptr<HTTPMessage> promise;

  HTTPPushEvent(std::unique_ptr<HTTPMessage> p, HTTPSource* pSource)
      : promise(std::move(p)), pushSource_(pSource) {
  }

  HTTPPushEvent(HTTPPushEvent&& goner)
      : promise(std::move(goner.promise)), pushSource_(goner.pushSource_) {
    goner.pushSource_ = nullptr;
  }

  HTTPPushEvent& operator=(HTTPPushEvent&& goner) {
    promise = std::move(goner.promise);
    pushSource_ = goner.pushSource_;
    goner.pushSource_ = nullptr;
    return *this;
  }

  ~HTTPPushEvent();

  // This object owns the push source and will automatically abort it on delete.
  // Call this method to take ownership of the push source, at which point the
  // caller must read to EOM or call stopReading.
  HTTPSource* movePushSource() {
    auto pushSource = pushSource_;
    pushSource_ = nullptr;
    return pushSource;
  }

  void describe(std::ostream& os) const;

 private:
  HTTPSource* pushSource_;
};

std::ostream& operator<<(std::ostream& os, const HTTPPushEvent& pushEvent);

/**
 * An Event containing part of an HTTP stream after the headers.  It can include
 *
 * BODY - some data
 * TRAILERS - metadata block. The EOM flag is always true for trailers.
 * PUSH_PROMISE - The beginning of a server push
 *                Only response streams can contain PUSH_PROMISE
 *                This needs work.  Probably needs to include an HTTPSource
 *                For the push response?
 * UPGRADE - TODO - this is meant to handle HTTP/1.x or CONNECT style upgrades
 *
 */
struct HTTPBodyEvent {
  enum EventType : uint8_t {
    UPGRADE,
    BODY,
    DATAGRAM,
    PUSH_PROMISE,
    TRAILERS,
    SUSPEND,
    PADDING,
  } eventType;
  bool eom{false};

  union EventData {
    BufQueue body;
    std::unique_ptr<folly::IOBuf> datagram;
    HTTPPushEvent push;
    std::unique_ptr<HTTPHeaders> trailers;
    folly::coro::Task<TimedBaton::Status> resume;
    // @lint-ignore CLANGTIDY cppcoreguidelines-pro-type-member-init
    uint16_t paddingSize;
    EventData() {
    }
    ~EventData() {
    }
  } event;

  std::vector<HTTPByteEventRegistration> byteEventRegistrations;

  HTTPBodyEvent(HTTPBodyEvent&& goner) {
    eventType = goner.eventType;
    eom = goner.eom;
    byteEventRegistrations = std::move(goner.byteEventRegistrations);
    switch (eventType) {
      case BODY:
        new (&event.body) BufQueue(std::move(goner.event.body));
        break;
      case DATAGRAM:
        new (&event.datagram)
            std::unique_ptr<folly::IOBuf>(std::move(goner.event.datagram));
        break;
      case PUSH_PROMISE:
        new (&event.push) HTTPPushEvent(std::move(goner.event.push));
        break;
      case TRAILERS:
        new (&event.trailers)
            std::unique_ptr<HTTPHeaders>(std::move(goner.event.trailers));
        break;
      case SUSPEND:
        new (&event.resume) folly::coro::Task<TimedBaton::Status>(
            std::move(goner.event.resume));
        break;
      case PADDING:
        event.paddingSize = goner.event.paddingSize;
        break;
      case UPGRADE:
        [[fallthrough]];
      default:
        // no-op
        break;
    }
  }

  HTTPBodyEvent& operator=(HTTPBodyEvent&& goner) {
    // Destroy this object first
    this->~HTTPBodyEvent();

    eventType = goner.eventType;
    eom = goner.eom;
    byteEventRegistrations = std::move(goner.byteEventRegistrations);
    switch (eventType) {
      case BODY:
        new (&event.body) BufQueue(std::move(goner.event.body));
        break;
      case DATAGRAM:
        new (&event.datagram)
            std::unique_ptr<folly::IOBuf>(std::move(goner.event.datagram));
        break;
      case PUSH_PROMISE:
        new (&event.push) HTTPPushEvent(std::move(goner.event.push));
        break;
      case TRAILERS:
        new (&event.trailers)
            std::unique_ptr<HTTPHeaders>(std::move(goner.event.trailers));
        break;
      case SUSPEND:
        new (&event.resume) folly::coro::Task<TimedBaton::Status>(
            std::move(goner.event.resume));
        break;
      case PADDING:
        event.paddingSize = goner.event.paddingSize;
        break;
      case UPGRADE:
        [[fallthrough]];
      default:
        // no-op
        break;
    }
    return *this;
  }

  ~HTTPBodyEvent() {
    switch (eventType) {
      case BODY:
        event.body.~BufQueue();
        break;
      case DATAGRAM:
        event.datagram.reset();
        break;
      case PUSH_PROMISE:
        event.push.~HTTPPushEvent();
        break;
      case TRAILERS:
        event.trailers.reset();
        break;
      case SUSPEND:
        event.resume.~Task<TimedBaton::Status>();
        break;
      case PADDING:
        // Nothing special to clean up a uint16_t
        [[fallthrough]];
      case UPGRADE:
        [[fallthrough]];
      default:
        // no-op
        break;
    }
  }

  // Default ctor makes an empty body event with no data and no EOM
  HTTPBodyEvent()
      : HTTPBodyEvent(std::unique_ptr<folly::IOBuf>(nullptr), false) {
  }

  HTTPBodyEvent(std::unique_ptr<folly::IOBuf> body, bool inEOM)
      : eventType(EventType::BODY), eom(inEOM) {
    new (&event.body) BufQueue(std::move(body));
  }

  HTTPBodyEvent(BufQueue body, bool inEOM)
      : eventType(EventType::BODY), eom(inEOM) {
    new (&event.body) BufQueue(std::move(body));
  }

  // Datagram can't have EOM
  enum class Datagram {};
  HTTPBodyEvent(Datagram, std::unique_ptr<folly::IOBuf> datagram)
      : eventType(EventType::DATAGRAM), eom(false) {
    new (&event.datagram) std::unique_ptr<folly::IOBuf>(std::move(datagram));
  }

  enum class Padding {};
  HTTPBodyEvent(Padding, uint16_t padding)
      : eventType(EventType::PADDING), eom(false) {
    event.paddingSize = padding;
  }

  HTTPBodyEvent(std::unique_ptr<HTTPMessage> promise,
                HTTPSource* source,
                bool inEOM)
      : eventType(EventType::PUSH_PROMISE), eom(inEOM) {
    new (&event.push) HTTPPushEvent(std::move(promise), source);
  }

  explicit HTTPBodyEvent(std::unique_ptr<HTTPHeaders> trailers)
      : eventType(EventType::TRAILERS), eom(true) {
    new (&event.trailers) std::unique_ptr<HTTPHeaders>(std::move(trailers));
  }

  explicit HTTPBodyEvent(folly::coro::Task<TimedBaton::Status> resume)
      : eventType(EventType::SUSPEND), eom(false) {
    new (&event.resume)
        folly::coro::Task<TimedBaton::Status>(std::move(resume));
  }

  void describe(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const HTTPBodyEvent& bodyEvent);

} // namespace proxygen::coro

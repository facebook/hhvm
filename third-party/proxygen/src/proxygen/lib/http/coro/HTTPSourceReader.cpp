/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPSourceReader.h"
#include <folly/logging/xlog.h>

namespace proxygen::coro {

HTTPSourceReader& HTTPSourceReader::preRead(PreReadFn preReadFn) {
  if (asyncPreReadFn_) {
    throw std::logic_error("The async callback has been already set");
  }

  preReadFn_ = std::move(preReadFn);
  return *this;
}

HTTPSourceReader& HTTPSourceReader::preReadAsync(AsyncPreReadFn preReadFn) {
  if (preReadFn_) {
    throw std::logic_error("The plain callback has been already set");
  }

  asyncPreReadFn_ = std::move(preReadFn);
  return *this;
}

HTTPSourceReader& HTTPSourceReader::onHeaders(HeaderFn headerFn) {
  if (asyncHeaderFn_) {
    throw std::logic_error("The async callback has been already set");
  }

  headerFn_ = std::move(headerFn);
  return *this;
}
HTTPSourceReader& HTTPSourceReader::onHeadersAsync(AsyncHeaderFn headerFn) {
  if (headerFn_) {
    throw std::logic_error("The plain callback has been already set");
  }

  asyncHeaderFn_ = std::move(headerFn);
  return *this;
}

HTTPSourceReader& HTTPSourceReader::onBody(BodyFn bodyFn) {
  if (asyncBodyFn_) {
    throw std::logic_error("The async callback has been already set");
  }

  bodyFn_ = std::move(bodyFn);
  return *this;
}
HTTPSourceReader& HTTPSourceReader::onBodyAsync(AsyncBodyFn bodyFn) {
  if (bodyFn_) {
    throw std::logic_error("The plain callback has been already set");
  }

  asyncBodyFn_ = std::move(bodyFn);
  return *this;
}

HTTPSourceReader& HTTPSourceReader::onDatagram(DatagramFn datagramFn) {
  if (asyncDatagramFn_) {
    throw std::logic_error("The async callback has been already set");
  }

  datagramFn_ = std::move(datagramFn);
  return *this;
}
HTTPSourceReader& HTTPSourceReader::onDatagramAsync(
    AsyncDatagramFn datagramFn) {
  if (datagramFn_) {
    throw std::logic_error("The plain callback has been already set");
  }

  asyncDatagramFn_ = std::move(datagramFn);
  return *this;
}

HTTPSourceReader& HTTPSourceReader::onPushPromise(PushPromiseFn promiseFn) {
  if (asyncPromiseFn_) {
    throw std::logic_error("The async callback has been already set");
  }

  promiseFn_ = std::move(promiseFn);
  return *this;
}
HTTPSourceReader& HTTPSourceReader::onPushPromiseAsync(
    AsyncPushPromiseFn promiseFn) {
  if (promiseFn_) {
    throw std::logic_error("The plain callback has been already set");
  }

  asyncPromiseFn_ = std::move(promiseFn);
  return *this;
}

HTTPSourceReader& HTTPSourceReader::onTrailers(TrailerFn trailerFn) {
  if (asyncTrailerFn_) {
    throw std::logic_error("The async callback has been already set");
  }

  trailerFn_ = std::move(trailerFn);
  return *this;
}
HTTPSourceReader& HTTPSourceReader::onTrailersAsync(AsyncTrailerFn trailerFn) {
  if (trailerFn_) {
    throw std::logic_error("The plain callback has been already set");
  }

  asyncTrailerFn_ = std::move(trailerFn);
  return *this;
}

HTTPSourceReader& HTTPSourceReader::onError(ErrorFn errorFn) {
  if (asyncErrorFn_) {
    throw std::logic_error("The async callback has been already set");
  }

  errorFn_ = std::move(errorFn);
  return *this;
}
HTTPSourceReader& HTTPSourceReader::onErrorAsync(AsyncErrorFn errorFn) {
  if (errorFn_) {
    throw std::logic_error("The plain callback has been already set");
  }

  asyncErrorFn_ = std::move(errorFn);
  return *this;
}

folly::coro::Task<void> HTTPSourceReader::read(uint32_t maxBodySize) {
  auto head = filterChain_.release();
  XCHECK(head);
  bool readCompleted = false;
  SCOPE_EXIT {
    if (!readCompleted) {
      head->stopReading();
    }
  };
  bool stop = false;
  folly::CancellationCallback cancellationCallback{
      co_await folly::coro::co_current_cancellation_token,
      [&stop] { stop = true; }};
  while (!stop) {
    if (preReadFn_) {
      stop |= preReadFn_();
    } else if (asyncPreReadFn_) {
      stop |= co_await asyncPreReadFn_();
    }
    if (stop) {
      break;
    }

    auto headerEvent = co_await co_awaitTry(head->readHeaderEvent());
    if (headerEvent.hasException()) {
      readCompleted = true;
      if (errorFn_) {
        errorFn_(ErrorContext::HEADERS, getHTTPError(headerEvent));
      } else if (asyncErrorFn_) {
        co_await asyncErrorFn_(ErrorContext::HEADERS,
                               getHTTPError(headerEvent));
      } else {
        co_yield folly::coro::co_error(headerEvent.exception());
      }
      co_return;
    }
    auto finalHeaders = headerEvent->isFinal();
    readCompleted = headerEvent->eom;
    if (headerFn_) {
      stop |= headerFn_(
          std::move(headerEvent->headers), finalHeaders, headerEvent->eom);
    } else if (asyncHeaderFn_) {
      stop |= co_await asyncHeaderFn_(
          std::move(headerEvent->headers), finalHeaders, headerEvent->eom);
    }
    if (headerEvent->eom) {
      co_return;
    }
    if (finalHeaders) {
      break;
    }
  }
  while (!stop) {
    if (preReadFn_) {
      stop |= preReadFn_();
    } else if (asyncPreReadFn_) {
      stop |= co_await asyncPreReadFn_();
    }
    if (stop) {
      break;
    }

    // TODO: support max arg to readBodyEvent, or change preRead to return
    // buffer size?
    auto bodyEvent = co_await co_awaitTry(head->readBodyEvent(maxBodySize));
    if (bodyEvent.hasException()) {
      readCompleted = true;
      if (errorFn_) {
        errorFn_(ErrorContext::BODY, getHTTPError(bodyEvent));
      } else if (asyncErrorFn_) {
        co_await asyncErrorFn_(ErrorContext::BODY, getHTTPError(bodyEvent));
      } else {
        co_yield folly::coro::co_error(bodyEvent.exception());
      }
      co_return;
    }
    readCompleted = bodyEvent->eom;
    switch (bodyEvent->eventType) {
      case HTTPBodyEvent::BODY:
        XCHECK(!bodyEvent->event.body.empty() || bodyEvent->eom);
        if (bodyFn_) {
          stop |= bodyFn_(std::move(bodyEvent->event.body), bodyEvent->eom);
        } else if (asyncBodyFn_) {
          stop |= co_await asyncBodyFn_(std::move(bodyEvent->event.body),
                                        bodyEvent->eom);
        }
        break;
      case HTTPBodyEvent::DATAGRAM:
        CHECK(bodyEvent->event.datagram);
        CHECK(!bodyEvent->eom);
        if (datagramFn_) {
          stop |= datagramFn_(std::move(bodyEvent->event.datagram));
        } else if (asyncDatagramFn_) {
          stop |=
              co_await asyncDatagramFn_(std::move(bodyEvent->event.datagram));
        }
        break;
      case HTTPBodyEvent::PUSH_PROMISE:
        if (promiseFn_) {
          stop |= promiseFn_(std::move(bodyEvent->event.push.promise),
                             bodyEvent->event.push.movePushSource(),
                             bodyEvent->eom);
        } else if (asyncPromiseFn_) {
          stop |=
              co_await asyncPromiseFn_(std::move(bodyEvent->event.push.promise),
                                       bodyEvent->event.push.movePushSource(),
                                       bodyEvent->eom);
        }
        break;
      case HTTPBodyEvent::TRAILERS:
        XCHECK(bodyEvent->eom) << "Trailers implies EOM";
        if (trailerFn_) {
          trailerFn_(std::move(bodyEvent->event.trailers));
        } else if (asyncTrailerFn_) {
          co_await asyncTrailerFn_(std::move(bodyEvent->event.trailers));
        } else if (bodyFn_) {
          bodyFn_(quic::BufQueue(nullptr), /*eom=*/true);
        } else if (asyncBodyFn_) {
          co_await asyncBodyFn_(quic::BufQueue(nullptr), /*eom=*/true);
        }
        break;
      case HTTPBodyEvent::UPGRADE: // delete
        break;
      case HTTPBodyEvent::SUSPEND: {
        auto res = co_await std::move(bodyEvent->event.resume);
        if (res == TimedBaton::Status::cancelled) {
          co_yield folly::coro::co_error(
              HTTPError(HTTPErrorCode::CORO_CANCELLED, "Read cancelled"));
        }
        break;
      }
      case HTTPBodyEvent::PADDING: // no read API for padding
        break;
    }
    if (bodyEvent->eom) {
      co_return;
    }
  }
}

} // namespace proxygen::coro

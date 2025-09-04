/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPByteEventHelpers.h"

namespace {
constexpr uint32_t kMaxTxAckEvents = 96;
} // namespace

namespace proxygen::coro {

size_t PendingByteEvent::fireEvents(std::list<PendingByteEvent>& events,
                                    uint64_t offset) {
  size_t nEvents = 0;
  while (!events.empty() && events.front().sessionOffset <= offset) {
    auto& event = events.front();
    auto cb = std::move(event.callback);
    if (cb) {
      cb->onByteEvent(std::move(event.byteEvent));
    }
    events.pop_front();
    nEvents++;
  }
  return nEvents;
}

void PendingByteEvent::cancelEvents(std::list<PendingByteEvent>& events,
                                    const HTTPError& error) {
  while (!events.empty()) {
    auto& event = events.front();
    auto cb = std::move(event.callback);
    if (cb) {
      cb->onByteEventCanceled(std::move(event.byteEvent), error);
    }
    events.pop_front();
  }
}

folly::WriteFlags AsyncSocketByteEventObserver::TxAckEvent::writeFlags() {
  folly::WriteFlags writeFlags = folly::WriteFlags::NONE;
  for (auto& regAndEvent : regAndEvents) {
    for (auto eventType : HTTPByteEvent::kByteEventTypes) {
      if (regAndEvent.registration.events & uint8_t(eventType)) {
        switch (eventType) {
          case HTTPByteEvent::Type::NIC_TX:
            writeFlags |= folly::WriteFlags::TIMESTAMP_TX;
            break;
          case HTTPByteEvent::Type::CUMULATIVE_ACK:
            writeFlags |= folly::WriteFlags::TIMESTAMP_ACK;
            break;
          default:
            break;
        }
      }
    }
  }
  if (writeFlags != folly::WriteFlags::NONE) {
    // Observer needs TIMESTAMP_WRITE to register for TIMESTAMP_TX/ACK
    writeFlags |= folly::WriteFlags::TIMESTAMP_WRITE;
  }
  return writeFlags;
}

bool AsyncSocketByteEventObserver::canRegister(uint8_t nEvents) const {
  return txAckByteEventsEnabled_ && nEvents > 0 &&
         numPendingTxAckEvents_ + txEvents_.size() + ackEvents_.size() +
                 nEvents <=
             kMaxTxAckEvents;
}

/* We register for TIMESTAMP_WRITE events as well as TX/ACK so we can track
 * the state of the raw transport.  When HTTPCoroSession finishes its write(),
 * maxTransportWriteOffset_ will contain the raw transport offset of the last
 * byte of the write.
 */
void AsyncSocketByteEventObserver::byteEvent(
    folly::AsyncSocket* /* socket */,
    const folly::AsyncSocketObserverInterface::ByteEvent& event) noexcept {
  size_t nEvents = 0;
  // Note: if there are other observers on this socket that register for TX or
  // ACK events, this observer will also fire, even if it did _not_ register.
  XLOG(DBG5) << "byteEvent type=" << uint32_t(event.type)
             << " off=" << event.offset;
  if (event.type ==
      folly::AsyncSocketObserverInterface::ByteEvent::Type::WRITE) {
    maxTransportWriteOffset_ = event.offset;
  } else if (event.type ==
             folly::AsyncSocketObserverInterface::ByteEvent::Type::TX) {
    maxTransportTxOffset_ = event.offset;
    nEvents = PendingByteEvent::fireEvents(txEvents_, event.offset);
  } else if (event.type ==
             folly::AsyncSocketObserverInterface::ByteEvent::Type::ACK) {
    maxTransportAckOffset_ = event.offset;
    nEvents = PendingByteEvent::fireEvents(ackEvents_, event.offset);
  }
  decRef(nEvents);
}

/* scheduleOrFireTxAckEvent will emplace the TX/ACK event, and fire it if
 * somehow we've already seen the TX/ACK of that offset in the observer.
 * Otherwise it bumps the refcount of outstanding byte events and schedules a
 * timeout.
 */
void AsyncSocketByteEventObserver::scheduleOrFireTxAckEvent(
    RegAndEvent regAndEvent) {
  if (regAndEvent.registration.events & uint8_t(HTTPByteEvent::Type::NIC_TX)) {
    XLOG(DBG5) << "Scheduling TX event off=" << maxTransportWriteOffset_;
    // copy the ByteEvent, we are setting the type and moving it to txEvents_
    auto event = regAndEvent.byteEvent;
    event.type = HTTPByteEvent::Type::NIC_TX;
    txEvents_.emplace_back(maxTransportWriteOffset_,
                           std::move(event),
                           regAndEvent.registration.callback);
    if (maxTransportTxOffset_ >= maxTransportWriteOffset_) {
      auto nEvents =
          PendingByteEvent::fireEvents(txEvents_, maxTransportTxOffset_);
      XCHECK_EQ(nEvents, 1u);
    } else {
      XCHECK(timer_);
      refCount_.incRef();
      txEvents_.back().refcount = &refCount_;
      timer_->scheduleTimeout(&txEvents_.back(), byteEventTimeout_);
    }
  }
  if (regAndEvent.registration.events &
      uint8_t(HTTPByteEvent::Type::CUMULATIVE_ACK)) {
    XLOG(DBG5) << "Scheduling ACK event off=" << maxTransportWriteOffset_;
    regAndEvent.byteEvent.type = HTTPByteEvent::Type::CUMULATIVE_ACK;
    ackEvents_.emplace_back(maxTransportWriteOffset_,
                            std::move(regAndEvent.byteEvent),
                            regAndEvent.registration.callback);
    if (maxTransportAckOffset_ >= maxTransportWriteOffset_) {
      auto nEvents =
          PendingByteEvent::fireEvents(ackEvents_, maxTransportAckOffset_);
      XCHECK_EQ(nEvents, 1u);
    } else {
      refCount_.incRef();
      XCHECK(timer_);
      ackEvents_.back().refcount = &refCount_;
      timer_->scheduleTimeout(&ackEvents_.back(), byteEventTimeout_);
    }
  }
  regAndEvent.registration.callback.reset();
}

void AsyncSocketByteEventObserver::registerByteEvents(
    uint64_t streamID,
    uint64_t sessionByteOffset,
    const folly::Optional<HTTPByteEvent::FieldSectionInfo>& fsInfo,
    uint64_t bodyOffset,
    std::vector<HTTPByteEventRegistration>&& registrations,
    bool eom) {
  auto localRegistrations = std::move(registrations);
  for (auto& reg : localRegistrations) {
    if (reg.events == 0 || !reg.callback) {
      continue; // nothing to do
    }
    HTTPByteEvent ev;
    ev.fieldSectionInfo = fsInfo;
    ev.streamID = reg.streamID ? *reg.streamID : streamID;
    ev.bodyOffset = bodyOffset;
    ev.transportOffset = sessionByteOffset;
    ev.eom = eom;
    if (reg.events & uint8_t(HTTPByteEvent::Type::TRANSPORT_WRITE)) {
      ev.type = HTTPByteEvent::Type::TRANSPORT_WRITE;
      transportWriteEvents_.emplace_back(sessionByteOffset, ev, reg.callback);
      reg.events &= ~uint8_t(HTTPByteEvent::Type::TRANSPORT_WRITE);
    }
    if (reg.events & uint8_t(HTTPByteEvent::Type::KERNEL_WRITE)) {
      ev.type = HTTPByteEvent::Type::KERNEL_WRITE;
      kernelWriteEvents_.emplace_back(sessionByteOffset, ev, reg.callback);
      reg.events &= ~uint8_t(HTTPByteEvent::Type::KERNEL_WRITE);
    }
    auto numEvents = numTxAckEventFlags(reg.events);
    if (numEvents > 0) {
      if (canRegister(numEvents)) {
        if (txAckEvents_.empty() ||
            txAckEvents_.back().sessionByteOffset != sessionByteOffset) {
          txAckEvents_.emplace_back(sessionByteOffset);
        }
        txAckEvents_.back().regAndEvents.emplace_back(
            RegAndEvent({std::move(reg), ev}));
        numPendingTxAckEvents_ += numEvents;
      } else {
        // Transport byte events not enabled, or at max TX/ACK events
        std::string errorMessage = !txAckByteEventsEnabled_
                                       ? "TX/ACK Events disabled"
                                       : "Too many TX/ACK Events registered";
        reg.cancel(HTTPError(HTTPErrorCode::CANCEL, std::move(errorMessage)),
                   std::move(ev));
      }
    } // implicit cancellation but there's no events left
  }
}

} // namespace proxygen::coro

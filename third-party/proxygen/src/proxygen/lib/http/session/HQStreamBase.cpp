/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HQStreamBase.h>

#include <proxygen/lib/http/session/HQSession.h>

namespace proxygen {

HQStreamBase::HQStreamBase(
    HQSession& session,
    HTTPCodecFilterChain& codecFilterChain,
    folly::Optional<hq::UnidirectionalStreamType> streamType)
    : codecFilterChain(codecFilterChain),
      createdTime(std::chrono::steady_clock::now()),
      type_(streamType),
      session_(session) {
}

const HTTPCodec& HQStreamBase::getCodec() const noexcept {
  if (realCodec_) {
    // it's not in the codec stack
    return *realCodec_;
  }
  if (*realCodecPtr_) {
    // it's in the codec stack, but not current
    return *realCodecPtr_->get();
  }
  // must be the current codec
  return *CHECK_NOTNULL(&codecFilterChain.getChainEnd());
}

HQSession& HQStreamBase::getSession() const noexcept {
  return session_;
}

/**
 * The session maintains a codecStack which contains the history of
 * setActiveCodec calls.  The stack has a pointer to where the active codec
 * needs to be restored, and the actual unique_ptr to the codec if a newer one
 * gets pushed on.  The stack will always have at least depth 1.
 *
 * So if setActiveCodec is called 3 times recursively with {c1, c2, c1}, the
 * state will look like this:
 *
 *  codecFilterChain->chainEnd: codec1
 *
 *  [ &codecStack[1].second, nullptr      txn1    ]
 *  [ &txn2->realCodec_,     codec2,      txn2    ]
 *  [ &txn1->realCodec_,     nullptr,     txn1    ]
 *  [ nullptr,               dummy codec, nullptr ]
 *
 * That said, I can't imagine how depth could be greater than 3:
 *  dummy codec, ingress codec, egress codec.
 */
folly::Function<void()> HQStreamBase::setActiveCodec(const std::string& where) {
  if (!realCodecPtr_->get()) {
    // already the active codec, no-op
    CHECK(!realCodec_);
    return [] {};
  }
  VLOG(5) << "Pushing active codec from " << where;
  CHECK_LT(session_.codecStack_.size(), HQSession::kMaxCodecStackDepth);
  CHECK(!session_.codecStack_.back().codec);
  // Set the requested codec as the chain destination, and store the previous
  // codec to its spot in the codecStack
  session_.codecStack_.back().codec =
      codecFilterChain.setDestination(std::move(*realCodecPtr_));
  // push a new entry for the current codec
  session_.codecStack_.emplace_back(realCodecPtr_, nullptr, this);
  // update a ptr to where to find the codec if needed
  realCodecPtr_ = &session_.codecStack_.back().codec;
  codecFilterChain.setCallback(this);
  return [this, where] {
    // put this codec back where it belongs
    VLOG(5) << "Popping active codec from " << where;
    auto codecPtr = session_.codecStack_.back().codecPtr;
    CHECK(!session_.codecStack_.back().codec);
    // pop the stack
    session_.codecStack_.pop_back();
    // move previous codec from the stack to active
    *codecPtr = codecFilterChain.setDestination(
        std::move(session_.codecStack_.back().codec));
    codecFilterChain.setCallback(session_.codecStack_.back().callback);
    realCodecPtr_ = codecPtr;
  };
}

size_t HQStreamBase::generateStreamPreface() {
  // Request (aka HQStreamTransport) streams do not type set.
  // If "generateStreamPreface" is invoked on those, its a bug
  CHECK(type_.has_value())
      << "Can not generate preface on streams without a type";
  VLOG(4) << "generating stream preface for " << type_.value()
          << " stream streamID=" << getEgressStreamId() << " sess=" << session_;
  auto res = hq::writeStreamPreface(
      writeBuf_, static_cast<hq::StreamTypeType>(type_.value()));
  CHECK(!res.hasError());
  return res.value();
}
} // namespace proxygen

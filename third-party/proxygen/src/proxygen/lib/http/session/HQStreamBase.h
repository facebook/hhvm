/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>
#include <folly/lang/Assume.h>
#include <proxygen/lib/http/HTTPException.h>
#include <proxygen/lib/http/codec/HQUnidirectionalCodec.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTPChecks.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/codec/HTTPCodecFilter.h>
#include <proxygen/lib/http/codec/HTTPSettings.h>
#include <quic/codec/Types.h>

/**
 *
 *
 * Building blocks for the different types of HTTP/3 streams.
 *
 * The streams in HQSession.h can be classified by several
 * axes:
 *   - Kind: Can carry requests / can carry control
 *   - Direction: Unidirectional / bidirectional
 *   - Transport Cardinality:
 *       Use a single transport stream / use 2 transport streams
 *
 * The above is achieved via the base/trait inheritance scheme:
 *    - Codec management is encapsulated in HQStreamBase class
 *    - Direction and cardinality are encapsulated in
 *      HQStreamMapping hierarchy
 *    - Kind is represented by
 *        Requests - HQStreamTransportBase (HQSession.h)
 *        Control  - HQControlStream (HQSession.h)
 *
 * Different streams use different combinations of the above:
 *
 *  - Request streams (HQStreamTransport):
 *     - Kind: requests
 *     - Direction: bidir
 *     - Cardinality: single bidirectional transport stream
 *
 *  - Control streams (HQControlStream)
 *     - Kind: control
 *     - Direction: unidir
 *     - Cardinality: 2 unidirectional transport streams
 *
 *  - Incoming push streams (HQIngressPushStream)
 *    - Kind: requests
 *    - Direction: unidir
 *    - Cardinality: single unidirectional ingress transport stream
 *
 *  - Incoming push streams (HQEgressPushStream)
 *    - Kind: requests
 *    - Direction: unidir
 *    - Cardinality: single unidirectional egress transport stream
 *
 *
 *  The above puts a challenge: The codec management code needs to access
 *    different streams, but should not be aware of the cardinailty.
 *
 *  This challenge is resolved via a diamond inheritance:
 *
 *
 *
 *     +---------------------+
 *     |   HQStreamMapping   |
 *     |    virtual base     |
 *     +---------------------+
 *                |
 *                |
 *                |
 *                |
 *                |
 *                +-------------------------------+
 *                V                               |
 *     +---------------------+                    |
 *     |    HQStreamBase     |                    |
 *     |  codec management   |                    |
 *     +---------------------+                    V
 *                |                    +---------------------+
 *                |                    |  Concrete mapping   |
 *                V                    |   (see below)       |
 *  +----------------------------+     +---------------------+
 *  |    Base implementation     |                |
 *  |          of Kind           |                |
 *  |     (requests/control)     |                |
 *  | Requests:                  |                |
 *  |   HQStreamTransportBase    |                |
 *  | Control                    |                |
 *  |   HQControlStream          |                |
 *  | (in HQSession.h)           |                |
 *  +----------------------------+                |
 *                |                               |
 *                +-------------------------------+
 *                |
 *                |
 *                |
 *                |
 *                V
 *  +----------------------------+
 *  |      Concrete stream       |   See HQStreamTransport
 *  |       implementation       |       HQControlStream
 *  |   combines the Kind base   |       HQIngressPushStream
 *  +----------------------------+       HQEgressPushStream
 *
 *
 */

namespace proxygen {

class HTTPSessionController;
class HQSession;
class VersionUtils;

/**
 * the HQStreamMapping trait defines how logical stream directions are
 * mapped to physical transport streams, and which of the directions are not
 * implemented.
 *
 *
 *                            +-----------------+
 *                            | HQStreamMapping |
 *              +-------------|  -------------  |----------+
 *              V             | L4 - L5 mapping |          |
 *    +------------------+    +-----------------+          |
 *    |     SSBidir      |             |                   |
 *    |  --------------  |             |                   |
 * +--|  egress/ingress  |             |                   |
 * |  | 1 bidirectional  |             |                   |
 * |  |    L4 stream     |             |                   |
 * |  +------------------+             |                   |
 * |                                   |                   |
 * +------------+                      |                   |
 * |            |                      |                   |
 * |            V                      |                   |
 * |  +------------------+             V                   |
 * |  |     SSEgress     |  +---------------------+        |
 * |  |  --------------  |  |    HQStreamBase     |        |
 * |  |   egress only    |  |     ----------      |        |
 * |  | 1 unidirectional |  |  Codec stack mgmt   |        |
 * |  |    L4 stream     |  +---------------------+        |
 * |  +------------------+                                 |
 * |                                                       V
 * |                                             +------------------+
 * |  +------------------+                       |     CSBidir      |
 * |  |    SSIngress     |                       |  --------------  |
 * |  |  --------------  |                       |  egress/ingress  |
 * +->|   ingress only   |                       | 2 unidirectional |
 *    | 1 unidirectional |                       |    L4 streams    |
 *    |    L4 stream     |                       +------------------+
 *    +------------------+
 *
 */
class HQStreamMapping {
 public:
  // NOTE: do not put any non-trivial logic in
  // the destructor of HQStreamMapping.
  virtual ~HQStreamMapping() = default;

  // Accessors for ingress/egress transport streams
  virtual quic::StreamId getEgressStreamId() const = 0;

  virtual quic::StreamId getIngressStreamId() const = 0;

  virtual bool hasIngressStreamId() const = 0;

  virtual bool hasEgressStreamId() const = 0;

  bool hasStreamId() const {
    return hasIngressStreamId() || hasEgressStreamId();
  }

  // Safe way to check whether this HQStream is using that quicStream
  // see usage in HQSession::findControlStream
  virtual bool isUsing(quic::StreamId /* streamId */) const = 0;

  virtual quic::StreamId getStreamId() const = 0;

  virtual void setIngressStreamId(quic::StreamId /* streamId */) = 0;

  virtual void setEgressStreamId(quic::StreamId /* streamId */) = 0;

  virtual HTTPException::Direction getStreamDirection() const = 0;
};

/**
 * The lowest common denominator of HQ streams.
 * This class defines codec stacking and holds the session object.
 */
class HQStreamBase
    : public virtual HQStreamMapping
    , public HTTPCodec::Callback {
 public:
  HQStreamBase() = delete;
  HQStreamBase(
      HQSession& /* session */,
      HTTPCodecFilterChain& /* codecFilterChain */,
      folly::Optional<hq::UnidirectionalStreamType> streamType = folly::none);

  virtual ~HQStreamBase() = default;

  const HTTPCodec& getCodec() const noexcept;

  HQSession& getSession() const noexcept;

  folly::Function<void()> setActiveCodec(const std::string& /* where */);

  HTTPCodecFilterChain& codecFilterChain;

  const std::chrono::steady_clock::time_point createdTime;

  size_t generateStreamPreface();

  /* Stream type (for streams with prefaces only) */
  folly::Optional<hq::UnidirectionalStreamType> type_;

  /** Chain of ingress IOBufs */
  folly::IOBufQueue readBuf_{folly::IOBufQueue::cacheChainLength()};

  /** Queue of egress IOBufs */
  folly::IOBufQueue writeBuf_{folly::IOBufQueue::cacheChainLength()};

  uint64_t bytesWritten_{0}; // Accounts for egress bytes written on the
                             // stream.

 protected:
  /** parent session **/
  HQSession& session_;

  std::unique_ptr<HTTPCodec> realCodec_;
  // realCodecPtr points to wherever this txn's codec is UNLESS it is the
  // active codec, in which case it is nullptr, and the codec is at the end
  // of the filter chain
  std::unique_ptr<HTTPCodec>* realCodecPtr_{&realCodec_};
};

namespace detail {

/**
 * Transport configurations that use a single transport stream.
 * Intended usage: Request streams (bidirectional), Push streams (single
 * direction)
 */
namespace singlestream {

/**
 * Bidirectional single transport stream
 */
class SSBidir : public virtual HQStreamMapping {
 public:
  virtual ~SSBidir() override = default;

  explicit SSBidir(folly::Optional<quic::StreamId> streamId)
      : streamId_(streamId) {
  }

  quic::StreamId getEgressStreamId() const override {
    return getStreamId();
  }

  quic::StreamId getIngressStreamId() const override {
    return getStreamId();
  }

  bool isUsing(quic::StreamId streamId) const override {
    if (streamId_) {
      return streamId_.value() == streamId;
    }
    return false;
  }

  bool hasIngressStreamId() const override {
    return streamId_.has_value();
  }

  bool hasEgressStreamId() const override {
    return streamId_.has_value();
  }

  quic::StreamId getStreamId() const override {
    CHECK(streamId_) << "Stream MUST be assigned before being accessed";
    return streamId_.value();
  }

  void setIngressStreamId(quic::StreamId streamId) override {
    streamId_ = streamId;
  }

  void setEgressStreamId(quic::StreamId streamId) override {
    streamId_ = streamId;
  }

  virtual HTTPException::Direction getStreamDirection() const override {
    return HTTPException::Direction::INGRESS_AND_EGRESS;
  }

 protected:
  folly::Optional<quic::StreamId> streamId_;
}; // proxygen::detail::singlestream::Bidir

/**
 * Egress only transport streams
 */
class SSEgress : public SSBidir {
 public:
  virtual ~SSEgress() override = default;

  explicit SSEgress(folly::Optional<quic::StreamId> streamId)
      : SSBidir(streamId) {
  }

  quic::StreamId getIngressStreamId() const override {
    LOG(FATAL) << "Egress only stream can not be used for ingress";
    folly::assume_unreachable();
  }

  void setIngressStreamId(quic::StreamId /* streamId */) override {
    LOG(FATAL) << "Egress only stream can not be used for ingress";
  }

  bool hasIngressStreamId() const override {
    return false;
  }

  virtual HTTPException::Direction getStreamDirection() const override {
    return HTTPException::Direction::EGRESS;
  }

}; // proxygen::detail::singlestream::Egress

class SSIngress : public SSBidir {
 public:
  virtual ~SSIngress() override = default;

  explicit SSIngress(folly::Optional<quic::StreamId> streamId)
      : SSBidir(streamId) {
  }

  quic::StreamId getEgressStreamId() const override {
    LOG(FATAL) << "Ingress only stream can not be used for egress";
    folly::assume_unreachable();
  }

  void setEgressStreamId(quic::StreamId /* streamId */) override {
    LOG(FATAL) << "Ingress only stream can not be used for egress";
  }

  bool hasEgressStreamId() const override {
    return false;
  }

  virtual HTTPException::Direction getStreamDirection() const override {
    return HTTPException::Direction::INGRESS;
  }

}; // proxygen::detail::singlestream::Ingres
} // namespace singlestream

/**
 * Composite stream bases.
 */
namespace composite {
/**
 * Bidirectional composite base
 */
class CSBidir : public virtual HQStreamMapping {
 public:
  virtual ~CSBidir() override = default;

  explicit CSBidir(folly::Optional<quic::StreamId> egressStreamId,
                   folly::Optional<quic::StreamId> ingressStreamId)
      : egressStreamId_(egressStreamId), ingressStreamId_(ingressStreamId) {
  }

  quic::StreamId getEgressStreamId() const override {
    CHECK(egressStreamId_)
        << "Egress stream MUST be assigned before being accessed";
    return egressStreamId_.value();
  }

  quic::StreamId getIngressStreamId() const override {
    CHECK(ingressStreamId_)
        << "Ingress stream MUST be assigned before being accessed";
    return ingressStreamId_.value();
  }

  bool isUsing(quic::StreamId streamId) const override {
    if (ingressStreamId_) {
      if (ingressStreamId_.value() == streamId) {
        return true;
      }
    }
    if (egressStreamId_) {
      if (egressStreamId_.value() == streamId) {
        return true;
      }
    }
    return false;
  }

  bool hasIngressStreamId() const override {
    return ingressStreamId_.has_value();
  }

  bool hasEgressStreamId() const override {
    return egressStreamId_.has_value();
  }

  quic::StreamId getStreamId() const override {
    LOG(FATAL) << "Ambiguous call 'getStreamId' on a composite stream";
    folly::assume_unreachable();
  }

  void setIngressStreamId(quic::StreamId streamId) override {
    ingressStreamId_ = streamId;
  }

  void setEgressStreamId(quic::StreamId streamId) override {
    egressStreamId_ = streamId;
  }

  virtual HTTPException::Direction getStreamDirection() const override {
    return HTTPException::Direction::INGRESS_AND_EGRESS;
  }

 private:
  folly::Optional<quic::StreamId> egressStreamId_;
  folly::Optional<quic::StreamId> ingressStreamId_;
}; // proxygen::detail::composite::Bidir

} // namespace composite
} // namespace detail
} // namespace proxygen

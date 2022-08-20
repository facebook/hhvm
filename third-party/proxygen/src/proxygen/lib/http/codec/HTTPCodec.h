/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Portability.h>
#include <folly/io/IOBufQueue.h>
#include <proxygen/lib/http/HTTPException.h>
#include <proxygen/lib/http/HTTPHeaderSize.h>
#include <proxygen/lib/http/codec/CodecProtocol.h>
#include <proxygen/lib/http/codec/ErrorCode.h>
#include <proxygen/lib/http/codec/HTTPSettings.h>
#include <proxygen/lib/http/codec/TransportDirection.h>
#include <proxygen/lib/http/codec/compress/HPACKCodec.h>
#include <proxygen/lib/http/codec/compress/HeaderCodec.h>

namespace proxygen {

class HTTPHeaders;
class HTTPMessage;
class HTTPTransactionHandler;
class HTTPErrorPage;

/**
 * Interface for a parser&generator that can translate between an internal
 * representation of an HTTP request and a wire format.  The details of the
 * wire format (e.g., HTTP/1.x encoding vs. SPDY encoding) are left for
 * subclasses to implement.
 */
class HTTPCodec {
 public:
  /**
   * Key that uniquely identifies a request/response pair within
   * (and only within) the scope of the codec.  Code outside the
   * codec should regard the StreamID as an opaque data
   * structure; different subclasses of HTTPCodec are likely to
   * use different conventions for generating StreamID values.
   *
   * A value of zero indicates an uninitialized/unknown/unspecified
   * StreamID.
   */
  using StreamID = uint64_t;

  static const folly::Optional<StreamID> NoStream;

  static const folly::Optional<uint8_t> NoPadding;

  static constexpr StreamID MaxStreamID = std::numeric_limits<StreamID>::max();

  struct ExAttributes {
    ExAttributes() {
    }
    ExAttributes(StreamID controlStreamId, bool isUnidirectional)
        : controlStream(controlStreamId), unidirectional(isUnidirectional) {
    }

    StreamID controlStream;
    bool unidirectional;
  };

  static const folly::Optional<ExAttributes> NoExAttributes;

  class PriorityQueue {
   public:
    virtual ~PriorityQueue() {
    }

    virtual void addPriorityNode(StreamID id, StreamID parent) = 0;
  };

  /**
   * Callback interface that users of HTTPCodec must implement
   */
  class Callback {
   public:
    /**
     * Called when a new message is seen while parsing the ingress
     * @param stream   The stream ID
     * @param msg      A newly allocated HTTPMessage
     */
    virtual void onMessageBegin(StreamID stream, HTTPMessage* msg) = 0;

    /**
     * Called when a new push message is seen while parsing the ingress.
     *
     * @param stream   The stream ID
     * @param assocStream The stream ID of the associated stream,
     *                 which can never be 0
     * @param msg      A newly allocated HTTPMessage
     */
    virtual void onPushMessageBegin(StreamID /* stream */,
                                    StreamID /* assocStream */,
                                    HTTPMessage* /* msg */) {
    }

    /**
     * Called when a new extended message is seen while parsing the ingress.
     *
     * @param stream   The stream ID
     * @param controlStream The stream ID of the associated stream,
     *                 which can never be 0
     * @param msg      A newly allocated HTTPMessage
     */
    virtual void onExMessageBegin(StreamID /* stream */,
                                  StreamID /* controlStream */,
                                  bool /* unidirectional */,
                                  HTTPMessage* /* msg */) {
    }

    /**
     * Called when all the headers of an ingress message have been parsed
     * @param stream   The stream ID
     * @param msg      The message
     * @param size     Size of the ingress header
     */
    virtual void onHeadersComplete(StreamID stream,
                                   std::unique_ptr<HTTPMessage> msg) = 0;

    /**
     * Called for each block of message body data
     * @param stream  The stream ID
     * @param chain   One or more buffers of body data. The codec will
     *                remove any protocol framing, such as HTTP/1.1 chunk
     *                headers, from the buffers before calling this function.
     * @param padding Number of pad bytes that came with the data segment
     */
    virtual void onBody(StreamID stream,
                        std::unique_ptr<folly::IOBuf> chain,
                        uint16_t padding) = 0;

    /**
     * Called for each HTTP chunk header.
     *
     * onChunkHeader() will be called when the chunk header is received.  As
     * the chunk data arrives, it will be passed to the callback normally with
     * onBody() calls.  Note that the chunk data may arrive in multiple
     * onBody() calls: it is not guaranteed to arrive in a single onBody()
     * call.
     *
     * After the chunk data has been received and the terminating CRLF has been
     * received, onChunkComplete() will be called.
     *
     * @param stream    The stream ID
     * @param length    The chunk length.
     */
    virtual void onChunkHeader(StreamID /* stream */, size_t /* length */) {
    }

    /**
     * Called when the terminating CRLF is received to end a chunk of HTTP body
     * data.
     *
     * @param stream    The stream ID
     */
    virtual void onChunkComplete(StreamID /* stream */) {
    }

    /**
     * Called when all the trailers of an ingress message have been
     * parsed, but only if the number of trailers is nonzero.
     * @param stream   The stream ID
     * @param trailers  The message trailers
     */
    virtual void onTrailersComplete(StreamID stream,
                                    std::unique_ptr<HTTPHeaders> trailers) = 0;

    /**
     * Called at end of a message (including body and trailers, if applicable)
     * @param stream   The stream ID
     * @param upgrade  Whether the connection has been upgraded to another
     *                 protocol.
     */
    virtual void onMessageComplete(StreamID stream, bool upgrade) = 0;

    /**
     * Called when a parsing or protocol error has occurred
     * @param stream   The stream ID
     * @param error    Description of the error
     * @param newTxn   true if onMessageBegin has not been called for txn
     */
    virtual void onError(StreamID stream,
                         const HTTPException& error,
                         bool newTxn = false) = 0;

    /**
     * Called when the peer has asked to shut down a stream
     * immediately.
     * @param stream   The stream ID
     * @param code     The code the stream was aborted with
     * @note  Not applicable to all protocols.
     */
    virtual void onAbort(StreamID /* stream */, ErrorCode /* code */) {
    }

    /**
     * Called upon receipt of a frame header.
     * @param stream_id The stream ID
     * @param flags     The flags field of frame header
     * @param length    The length field of frame header
     * @param type      The type field of frame header
     * @param version   The version of frame (SPDY only)
     * @note Not all protocols have frames. SPDY and HTTP/2 do,
     *       but HTTP/1.1 doesn't.
     */
    virtual void onFrameHeader(StreamID /* stream_id */,
                               uint8_t /* flags */,
                               uint64_t /* length */,
                               uint64_t /* type */,
                               uint16_t /* version */ = 0) {
    }

    /**
     * Called upon receipt of a goaway.
     * @param lastGoodStreamID  Last successful stream created by the receiver
     * @param code              The code the connection was aborted with
     * @param debugData         The additional debug data for diagnostic purpose
     * @note Not all protocols have goaways. SPDY does, but HTTP/1.1 doesn't.
     */
    virtual void onGoaway(
        uint64_t /* lastGoodStreamID */,
        ErrorCode /* code */,
        std::unique_ptr<folly::IOBuf> /* debugData */ = nullptr) {
    }

    /**
     * Called upon receipt of an unknown frame.
     * @param streamID          stream ID
     * @param frameType         frame type
     * @note only used for quic testing
     */
    virtual void onUnknownFrame(uint64_t /* streamID */,
                                uint64_t /* frameType */) {
    }

    /**
     * Called upon receipt of a ping request
     * @param data attached to the ping request
     * @note Not all protocols have pings.HTTP/2 does, but HTTP/1.1 doesn't.
     */
    virtual void onPingRequest(uint64_t /* data */) {
    }

    /**
     * Called upon receipt of a ping reply
     * @param data data attached to the ping reply
     * @note Not all protocols have pings. HTTP/2 does, but HTTP/1.1 doesn't.
     */
    virtual void onPingReply(uint64_t /* data */) {
    }

    /**
     * Called upon receipt of a window update, for protocols that support
     * flow control. For instance spdy/3 and higher.
     */
    virtual void onWindowUpdate(StreamID /* stream */, uint32_t /* amount */) {
    }

    /**
     * Called upon receipt of a settings frame, for protocols that support
     * settings.
     *
     * @param settings a list of settings that were sent in the settings frame
     */
    virtual void onSettings(const SettingsList& /* settings */) {
    }

    /**
     * Called upon receipt of a settings frame with ACK set, for
     * protocols that support settings ack.
     */
    virtual void onSettingsAck() {
    }

    /**
     * Called upon receipt of a priority frame, for protocols that support
     * dynamic priority
     */
    virtual void onPriority(StreamID /* stream */,
                            const HTTPMessage::HTTP2Priority& /* pri */) {
    }

    /**
     * Experimental: this is the new HTTP Priority draft format of priority
     * update. This is called when a PRIORITY_UPDATE frame is received.
     */
    virtual void onPriority(StreamID, const HTTPPriority& /* pri */) {
    }

    /**
     * Experimental: this is the new HTTP Priority draft format of priority
     * update. This is called when a PUSH_PRIORITY_UPDATE frame is received.
     */
    virtual void onPushPriority(StreamID, const HTTPPriority& /* pri */) {
    }

    /**
     * Called upon receipt of a valid protocol switch.  Return false if
     * protocol switch could not be completed.
     */
    virtual bool onNativeProtocolUpgrade(
        StreamID /* stream */,
        CodecProtocol /* protocol */,
        const std::string& /* protocolString */,
        HTTPMessage& /* msg */) {
      return false;
    }

    /**
     * Called after a header frame is generated.
     * This only applies to framed codecs.
     */
    virtual void onGenerateFrameHeader(StreamID /* stream_id */,
                                       uint8_t /* type */,
                                       uint64_t /* length */,
                                       uint16_t /* version */ = 0) {
    }

    /**
     * Called upon receipt of a certificate request frame, for protocols that
     * support secondary certificate authentication.
     * @param requestId The Request-ID identifying the certificate request
     * @param authRequest The authenticator request
     * @note Not all protocols support secondary certificate authentication.
     * HTTP/2 does, but HTTP/1.1 doesn't.
     */
    virtual void onCertificateRequest(
        uint16_t /* requestId */,
        std::unique_ptr<folly::IOBuf> /* authRequest */) {
    }

    /**
     * Called upon receipt of an authenticator, for protocols that
     * support secondary certificate authentication.
     * @param certId The Cert-ID identifying this authenticator
     * @param authenticator The authenticator request
     * @note Not all protocols support secondary certificate authentication.
     * HTTP/2 does, but HTTP/1.1 doesn't.
     */
    virtual void onCertificate(
        uint16_t /* certId */,
        std::unique_ptr<folly::IOBuf> /* authenticator */) {
    }

    /**
     * Return the number of open streams started by this codec callback.
     * Parallel codecs with a maximum number of streams will invoke this
     * to determine if a new stream exceeds the limit.
     */
    virtual uint32_t numOutgoingStreams() const {
      return 0;
    }

    /**
     * Return the number of open streams started by the remote side.
     * Parallel codecs with a maximum number of streams will invoke this
     * to determine if a new stream exceeds the limit.
     */
    virtual uint32_t numIncomingStreams() const {
      return 0;
    }

    virtual ~Callback() {
    }
  };

  virtual ~HTTPCodec() {
  }

  /**
   * Maps a stream id to its sequence number using the underlying protocol as
   * context.
   */
  static size_t streamIDToSeqNo(CodecProtocol protocol,
                                HTTPCodec::StreamID id) {
    switch (protocol) {
      case CodecProtocol::HTTP_1_1:
        DCHECK_NE(id, 0);
        return id - 1;
      case CodecProtocol::HTTP_2:
        return id / 2;
      case CodecProtocol::HQ:
      case CodecProtocol::HTTP_3:
        // This doesn't factor out of order stream arrival...
        return id / 4;
      default:
        LOG(FATAL) << "Unreachable";
        return std::numeric_limits<size_t>::max();
    }
  }

  /**
   * Gets both the egress and ingress header table size, bytes stored in header
   * table, and the number of headers stored in the header table
   **/
  virtual CompressionInfo getCompressionInfo() const {
    static CompressionInfo defaultCompressionInfo;
    return defaultCompressionInfo;
  }

  /**
   * Gets the session protocol currently used by the codec. This can be
   * mapped to a string for logging and diagnostic use.
   */
  virtual CodecProtocol getProtocol() const = 0;

  /**
   * Gets the user agent string of the client. Thus, it is only meaningful for a
   * DOWNSTREAM session. Note that the value is available after
   * onHeadersComplete().  It can help in diagnosing the interactions between
   * different codec implementation.
   */
  virtual const std::string& getUserAgent() const = 0;

  /**
   * Get the transport direction of this codec:
   * DOWNSTREAM if the codec receives requests from clients or
   * UPSTREAM if the codec sends requests to servers.
   */
  virtual TransportDirection getTransportDirection() const = 0;

  /**
   * Returns true iff this codec supports per stream flow control
   */
  virtual bool supportsStreamFlowControl() const {
    return false;
  }

  /**
   * Returns true iff this codec supports session level flow control
   */
  virtual bool supportsSessionFlowControl() const {
    return false;
  }

  /**
   * Reserve a stream ID.
   * @return           A stream ID on success, or zero on error.
   */
  virtual StreamID createStream() = 0;

  /**
   * Set the callback to notify on ingress events
   * @param callback  The callback object
   */
  virtual void setCallback(Callback* callback) = 0;

  /**
   * Check whether the codec still has at least one HTTP
   * stream to parse.
   */
  virtual bool isBusy() const = 0;

  /**
   * Pause or resume the ingress parser
   * @param paused  Whether the caller wants the parser to be paused
   */
  virtual void setParserPaused(bool paused) = 0;

  /**
   * Return true if the parser is paused
   */
  virtual bool isParserPaused() const = 0;

  /**
   * Parse ingress data.
   * @param  buf   A single IOBuf of data to parse
   * @return Number of bytes consumed.
   */
  virtual size_t onIngress(const folly::IOBuf& buf) = 0;

  /**
   * Finish parsing when the ingress stream has ended.
   */
  virtual void onIngressEOF() = 0;

  /**
   * Invoked on a codec that has been upgraded to via an HTTPMessage on
   * a different codec.  The codec may return false to halt the upgrade.
   */
  virtual bool onIngressUpgradeMessage(const HTTPMessage& /* msg */) {
    return true;
  }

  /**
   * Check whether the codec can process new streams. Typically,
   * an implementing subclass will return true when a new codec is
   * created and false once it encounters a situation that would
   * prevent reuse of the underlying transport (e.g., a "Connection: close"
   * in HTTP/1.x).
   * @note A return value of true means that the codec can process new
   *       connections at some reasonable point in the future; that may
   *       mean "immediately," for codecs that support pipelined or
   *       interleaved requests, or "upon completion of the current
   *       stream" for codecs that do not.
   */
  virtual bool isReusable() const = 0;

  /**
   * Returns true if this codec is in a state where it accepting new
   * requests but will soon begin to reject new requests. For SPDY and
   * HTTP/2, this is true when the first GOAWAY NO_ERROR is sent during
   * graceful shutdown.
   */
  virtual bool isWaitingToDrain() const = 0;

  /**
   * Checks whether the socket needs to be closed when EOM is sent. This is used
   * during CONNECT when EOF needs to be sent after upgrade to notify the server
   */
  virtual bool closeOnEgressComplete() const = 0;

  /**
   * Check whether the codec supports the processing of multiple
   * requests in parallel.
   */
  virtual bool supportsParallelRequests() const = 0;

  /**
   * Check whether the codec supports pushing resources from server to
   * client.
   */
  virtual bool supportsPushTransactions() const = 0;

  /**
   * Check whether the codec supports bidirectional communications between
   * server and client.
   */
  virtual bool supportsExTransactions() const {
    return false;
  }

  /**
   * Generate a connection preface, if there is any for this protocol.
   *
   * @return size of the generated message
   */
  virtual size_t generateConnectionPreface(folly::IOBufQueue& /* writeBuf */) {
    return 0;
  }

  /**
   * Write an egress message header.  For pushed streams, you must specify
   * the assocStream.
   * @param extraHeaders Optional extra headers to be generated togetger with
   *                     the msg.
   * @retval size the size of the generated message, both the actual size
   *              and the size of the uncompressed data.
   * @return None
   */
  virtual void generateHeader(
      folly::IOBufQueue& writeBuf,
      StreamID stream,
      const HTTPMessage& msg,
      bool eom = false,
      HTTPHeaderSize* size = nullptr,
      const folly::Optional<HTTPHeaders>& extraHeaders = folly::none) = 0;

  virtual void generatePushPromise(folly::IOBufQueue& /* writeBuf */,
                                   StreamID /* stream */,
                                   const HTTPMessage& /* msg */,
                                   StreamID /* assocStream */,
                                   bool /* eom = false */,
                                   HTTPHeaderSize* /* size = nullptr */) {
  }

  virtual void generateExHeader(folly::IOBufQueue& /* writeBuf */,
                                StreamID /* stream */,
                                const HTTPMessage& /* msg */,
                                const HTTPCodec::ExAttributes& /*exAttributes*/,
                                bool /* eom = false */,
                                HTTPHeaderSize* /* size = nullptr */) {
  }

  /**
   * Write part of an egress message body.
   *
   * This will automatically generate a chunk header and footer around the data
   * if necessary (e.g. you haven't manually sent a chunk header and the
   * message should be chunked).
   *
   * @param padding Optionally add padding bytes to the body if possible
   * @param eom implicitly generate the EOM marker with this body frame
   *
   * @return number of bytes written
   */
  virtual size_t generateBody(folly::IOBufQueue& writeBuf,
                              StreamID stream,
                              std::unique_ptr<folly::IOBuf> chain,
                              folly::Optional<uint8_t> padding,
                              bool eom) = 0;

  /**
   * Write a body chunk header, if relevant.
   */
  virtual size_t generateChunkHeader(folly::IOBufQueue& writeBuf,
                                     StreamID stream,
                                     size_t length) = 0;

  /**
   * Write a body chunk terminator, if relevant.
   */
  virtual size_t generateChunkTerminator(folly::IOBufQueue& writeBuf,
                                         StreamID stream) = 0;

  /**
   * Write the message trailers
   * @return number of bytes written
   */
  virtual size_t generateTrailers(folly::IOBufQueue& writeBuf,
                                  StreamID stream,
                                  const HTTPHeaders& trailers) = 0;

  /**
   * Generate any protocol framing needed to finalize an egress
   * message. This method must be called to complete a stream.
   *
   * @return number of bytes written
   */
  virtual size_t generateEOM(folly::IOBufQueue& writeBuf, StreamID stream) = 0;

  /**
   * Generate any protocol framing needed to abort a stream.
   * @return number of bytes written
   */
  virtual size_t generateRstStream(folly::IOBufQueue& writeBuf,
                                   StreamID stream,
                                   ErrorCode code) = 0;

  /**
   * Generate any protocol framing needed to gracefully drain or abort a
   * connection.
   *
   * Calling with lastStream = MaxStreamID and code = NO_ERROR will leave it to
   * the codec to properly fill in the last stream ID.
   *
   * @return number of bytes written
   */
  virtual size_t generateGoaway(
      folly::IOBufQueue& writeBuf,
      StreamID lastStream = MaxStreamID,
      ErrorCode code = ErrorCode::NO_ERROR,
      std::unique_ptr<folly::IOBuf> debugData = nullptr) = 0;

  // Generate an immediate goaway
  virtual size_t generateImmediateGoaway(
      folly::IOBufQueue& writeBuf,
      ErrorCode code = ErrorCode::NO_ERROR,
      std::unique_ptr<folly::IOBuf> debugData = nullptr) {
    return generateGoaway(
        writeBuf, getLastIncomingStreamID(), code, std::move(debugData));
  }

  /**
   * If the protocol supports it, generate a ping message that the other
   * side should respond to.
   */
  virtual size_t generatePingRequest(
      folly::IOBufQueue& /* writeBuf */,
      folly::Optional<uint64_t> /* data */ = folly::none) {
    return 0;
  }

  /**
   * Generate a reply to a ping message, if supported in the
   * protocol implemented by the codec.
   */
  virtual size_t generatePingReply(folly::IOBufQueue& /* writeBuf */,
                                   uint64_t /* data */) {
    return 0;
  }

  /**
   * Generate a settings message, if supported in the
   * protocol implemented by the codec.
   */
  virtual size_t generateSettings(folly::IOBufQueue& /* writeBuf */) {
    return 0;
  }

  /**
   * Generate a settings ack message, if supported in the
   * protocol implemented by the codec.
   */
  virtual size_t generateSettingsAck(folly::IOBufQueue& /* writeBuf */) {
    return 0;
  }

  /*
   * Generate a WINDOW_UPDATE message, if supported. The delta is the amount
   * of ingress bytes we processed and freed from the current receive window.
   * Returns the number of bytes written on the wire as a result of invoking
   * this function.
   */
  virtual size_t generateWindowUpdate(folly::IOBufQueue& /* writeBuf */,
                                      StreamID /* stream */,
                                      uint32_t /* delta */) {
    return 0;
  }

  /*
   * Generate a PRIORITY message, if supported
   */
  virtual size_t generatePriority(folly::IOBufQueue& /* writeBuf */,
                                  StreamID /* stream */,
                                  const HTTPMessage::HTTP2Priority& /* pri */) {
    return 0;
  }

  /**
   * Generate a PRIORITY_UPDATE frame, according to the new HTTP priority
   * draft, if supported.
   */
  virtual size_t generatePriority(folly::IOBufQueue& /* writeBuf */,
                                  StreamID /* stream */,
                                  HTTPPriority /* priority */) {
    return 0;
  }

  /**
   * Generate a PUSH_PRIORITY_UPDATE frame for non push stream, according to
   * the new HTTP priority draft, if supported.
   */
  virtual size_t generatePushPriority(folly::IOBufQueue& /* writeBuf */,
                                      StreamID /* stream */,
                                      HTTPPriority /* priority */) {
    return 0;
  }

  /*
   * Generate a CERTIFICATE_REQUEST message, if supported in the protocol
   * implemented by the codec.
   */
  virtual size_t generateCertificateRequest(
      folly::IOBufQueue& /* writeBuf */,
      uint16_t /* requestId */,
      std::unique_ptr<folly::IOBuf> /* chain */) {
    return 0;
  }

  /*
   * Generate a CERTIFICATE message, if supported in the protocol
   * implemented by the codec.
   */
  virtual size_t generateCertificate(
      folly::IOBufQueue& /* writeBuf */,
      uint16_t /* certId */,
      std::unique_ptr<folly::IOBuf> /* certData */) {
    return 0;
  }

  /*
   * The below interfaces need only be implemented if the codec supports
   * settings
   */
  virtual HTTPSettings* getEgressSettings() {
    return nullptr;
  }

  virtual const HTTPSettings* getIngressSettings() const {
    return nullptr;
  }

  /**
   * This enables HTTP/2 style behavior during graceful shutdown that allows
   * 2 GOAWAYs to be sent during shutdown.
   */
  virtual void enableDoubleGoawayDrain() {
  }

  /**
   * set stats for the header codec, if the protocol supports header compression
   */
  virtual void setHeaderCodecStats(HeaderCodec::Stats* /* stats */) {
  }

  /**
   * Get the identifier of the last stream started by the remote.
   */
  virtual StreamID getLastIncomingStreamID() const {
    return 0;
  }

  /**
   * Get the default size of flow control windows for this protocol
   */
  virtual uint32_t getDefaultWindowSize() const {
    return 0;
  }

  /**
   * Create virtual nodes in HTTP/2 priority tree. Some protocols (SPDY) have a
   * linear priority structure which must be simulated in the HTTP/2 tree
   * structure with "virtual" nodes representing different priority bands.
   * There are other cases we simply want a "plain" linear priority structure
   * even with HTTP/2. In that case a Priority frame will also be sent out for
   * each virtual node created so that peer will have the same linear structure.
   *
   * @param queue     the priority queue to add nodes
   * @param writeBuf  IOBufQueue to append priority frames to send. For SPDY,
   *                    the writeBuf will be ignored.
   * @param maxLavel  the max level of virtual priority nodes to create. For
   *                    SPDY, this value will be ignored.
   */
  virtual size_t addPriorityNodes(PriorityQueue& /* queue */,
                                  folly::IOBufQueue& /* writeBuf */,
                                  uint8_t /* maxLevel */) {
    return 0;
  }

  /**
   * Map the given linear priority to the correct parent node dependency
   */
  virtual StreamID mapPriorityToDependency(uint8_t /* priority */) const {
    return 0;
  }

  /**
   * Map the parent back to the priority, -1 if this doesn't make sense.
   */
  virtual int8_t mapDependencyToPriority(StreamID /* parent */) const {
    return -1;
  }
};

} // namespace proxygen

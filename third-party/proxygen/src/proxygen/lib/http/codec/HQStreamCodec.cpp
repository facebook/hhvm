/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HQStreamCodec.h>

#include <folly/Format.h>
#include <folly/ScopeGuard.h>
#include <folly/SingletonThreadLocal.h>
#include <folly/io/Cursor.h>
#include <proxygen/lib/http/HTTP3ErrorCode.h>
#include <proxygen/lib/http/codec/HQUtils.h>
#include <proxygen/lib/http/codec/compress/QPACKCodec.h>

namespace {

using namespace proxygen;

void logIfFieldSectionExceedsPeerMax(const HTTPHeaderSize& encodedSize,
                                     uint32_t maxHeaderListSize,
                                     const HTTPHeaders& fields) {
  if (encodedSize.uncompressed > maxHeaderListSize) {
    // The remote side told us they don't want headers this large, but try
    // anyways
    std::string serializedFields;
    fields.forEach(
        [&serializedFields](const std::string& name, const std::string& value) {
          serializedFields =
              folly::to<std::string>(serializedFields, "\\n", name, ":", value);
        });
    LOG(ERROR) << "generating HEADERS frame larger than peer maximum nHeaders="
               << fields.size() << " all headers=" << serializedFields;
  }
}

} // namespace

namespace proxygen { namespace hq {

using namespace folly;
using namespace folly::io;

HQStreamCodec::HQStreamCodec(StreamID streamId,
                             TransportDirection direction,
                             QPACKCodec& headerCodec,
                             folly::IOBufQueue& encoderWriteBuf,
                             folly::IOBufQueue& decoderWriteBuf,
                             folly::Function<uint64_t()> qpackEncoderMaxData,
                             HTTPSettings& ingressSettings)
    : HQFramedCodec(streamId, direction),
      headerCodec_(headerCodec),
      qpackEncoderWriteBuf_(encoderWriteBuf),
      qpackDecoderWriteBuf_(decoderWriteBuf),
      qpackEncoderMaxDataFn_(std::move(qpackEncoderMaxData)),
      ingressSettings_(ingressSettings) {
  VLOG(4) << "creating " << getTransportDirectionString(direction)
          << " HQ stream codec for stream " << streamId_;
}

HQStreamCodec::~HQStreamCodec() {
}

ParseResult HQStreamCodec::checkFrameAllowed(FrameType type) {
  if (isConnect_ && type != hq::FrameType::DATA) {
    return HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED;
  }
  switch (type) {
    case hq::FrameType::SETTINGS:
    case hq::FrameType::GOAWAY:
    case hq::FrameType::MAX_PUSH_ID:
    case hq::FrameType::CANCEL_PUSH:
    case hq::FrameType::PRIORITY_UPDATE:
    case hq::FrameType::PUSH_PRIORITY_UPDATE:
    case hq::FrameType::FB_PRIORITY_UPDATE:
    case hq::FrameType::FB_PUSH_PRIORITY_UPDATE:
    case hq::FrameType::WEBTRANSPORT_BIDI:
      return HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED;
    case hq::FrameType::PUSH_PROMISE:
      if (transportDirection_ == TransportDirection::DOWNSTREAM) {
        return HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED;
      }
      break;
    default:
      break;
  }
  return folly::none;
}

ParseResult HQStreamCodec::parseData(Cursor& cursor,
                                     const FrameHeader& header) {
  // NOTE: If an error path is added to this method, it needs to setParserPaused

  // It's possible the data is in the wrong place per HTTP semantics, but it
  // will be caught by HTTPTransaction
  std::unique_ptr<IOBuf> outData;
  VLOG(10) << "parsing all frame DATA bytes for stream=" << streamId_
           << " length=" << header.length;
  auto res = hq::parseData(cursor, header, outData);
  CHECK(!res);

  // no need to do deliverCallbackIfAllowed
  // the HQSession can trap this and stop reading.
  // i.e we can immediately reset in onNewStream if we get a stream id
  // higher than MAXID advertised in the goaway
  if (callback_ && (outData && !outData->empty())) {
    callback_->onBody(streamId_, std::move(outData), 0);
  }
  return res;
}

ParseResult HQStreamCodec::parseHeaders(Cursor& cursor,
                                        const FrameHeader& header) {
  setParserPaused(true);
  if (finalIngressHeadersSeen_) {
    if (parsingTrailers_) {
      VLOG(4) << "Unexpected HEADERS frame for stream=" << streamId_;
      if (callback_) {
        HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                         "Invalid HEADERS frame");
        ex.setHttp3ErrorCode(HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED);
        callback_->onError(streamId_, ex, false);
      }
      return folly::none;
    } else {
      parsingTrailers_ = true;
    }
  }
  std::unique_ptr<IOBuf> outHeaderData;
  auto res = hq::parseHeaders(cursor, header, outHeaderData);
  if (res) {
    VLOG(4) << "Invalid HEADERS frame for stream=" << streamId_;
    return res;
  }
  VLOG(4) << "Parsing HEADERS frame for stream=" << streamId_
          << " length=" << outHeaderData->computeChainDataLength();
  if (callback_ && !parsingTrailers_) {
    // H2 performs the decompression/semantic validation first.  Also, this
    // should really only be called once per this whole codec, not per header
    // block -- think info status. This behavior mirrors HTTP2Codec at present.
    callback_->onMessageBegin(streamId_, nullptr);
  }
  decodeInfo_.init(transportDirection_ == TransportDirection::DOWNSTREAM,
                   parsingTrailers_,
                   /*validate=*/true,
                   strictValidation_,
                   /*allowEmptyPath=*/false);
  headerCodec_.decodeStreaming(
      streamId_, std::move(outHeaderData), header.length, this);
  // decodeInfo_.msg gets moved in onHeadersComplete.  If it is still around,
  // parsing is incomplete, leave the parser paused.
  if (!decodeInfo_.msg) {
    setParserPaused(false);
  }
  return res;
}

ParseResult HQStreamCodec::parsePushPromise(Cursor& cursor,
                                            const FrameHeader& header) {
  setParserPaused(true);
  PushId outPushId;
  std::unique_ptr<IOBuf> outHeaderData;
  auto res = hq::parsePushPromise(cursor, header, outPushId, outHeaderData);
  if (res) {
    return res;
  }

  // Notify the callback on beginning of a push promise.
  // The callback will be further notified when the header block
  // is fully parsed, via a call to `onHeadersComplete`.
  // It is up to the callback to match the push promise
  // with the headers block, via using same stream id
  if (callback_) {
    callback_->onPushMessageBegin(outPushId, streamId_, nullptr);
  }

  decodeInfo_.init(true /* isReq */,
                   false /* isRequestTrailers */,
                   /*validate=*/true,
                   strictValidation_,
                   /*allowEmptyPath=*/false);
  auto headerDataLength = outHeaderData->computeChainDataLength();
  headerCodec_.decodeStreaming(
      streamId_, std::move(outHeaderData), headerDataLength, this);
  if (!decodeInfo_.msg) {
    setParserPaused(false);
  } // else parsing incomplete, see comment in parseHeaders

  return res;
}

void HQStreamCodec::onHeader(const HPACKHeaderName& name,
                             const folly::fbstring& value) {
  if (decodeInfo_.onHeader(name, value)) {
    if (userAgent_.empty() && name.getHeaderCode() == HTTP_HEADER_USER_AGENT) {
      userAgent_ = value.toStdString();
    }
  } else {
    VLOG(4) << "dir=" << uint32_t(transportDirection_)
            << decodeInfo_.parsingError << " codec=" << headerCodec_;
  }
}

void HQStreamCodec::onHeadersComplete(HTTPHeaderSize decodedSize,
                                      bool acknowledge) {
  CHECK(parserPaused_);
  decodeInfo_.onHeadersComplete(decodedSize);
  auto resumeParser = folly::makeGuard([this] { setParserPaused(false); });
  auto g2 = folly::makeGuard(activationHook_());

  // Check parsing error
  DCHECK_EQ(decodeInfo_.decodeError, HPACK::DecodeError::NONE);
  // Leave msg in decodeInfo_ for now, to keep the parser paused
  if (!decodeInfo_.parsingError.empty()) {
    LOG(ERROR) << "Failed parsing header list for stream=" << streamId_
               << ", error=" << decodeInfo_.parsingError;
    if (!decodeInfo_.headerErrorValue.empty()) {
      std::cerr << " value=" << decodeInfo_.headerErrorValue << std::endl;
    }
    HTTPException err(
        HTTPException::Direction::INGRESS,
        fmt::format("HQStreamCodec stream error: stream={} status={} error:{}",
                    streamId_,
                    400,
                    decodeInfo_.parsingError));
    if (parsingTrailers_) {
      err.setHttp3ErrorCode(HTTP3::ErrorCode::HTTP_MESSAGE_ERROR);
    } else {
      err.setHttpStatusCode(400);
    }
    err.setProxygenError(kErrorParseHeader);
    // Have to clone it
    err.setPartialMsg(std::make_unique<HTTPMessage>(*decodeInfo_.msg));
    callback_->onError(streamId_, err, true);
    resumeParser.dismiss();
    return;
  }
  std::unique_ptr<HTTPMessage> msg = std::move(decodeInfo_.msg);
  msg->setAdvancedProtocolString(getCodecProtocolString(CodecProtocol::HQ));

  if (curHeader_.type == hq::FrameType::HEADERS) {
    if (!finalIngressHeadersSeen_ && msg->isFinal()) {
      finalIngressHeadersSeen_ = true;
    }
  }

  if (transportDirection_ == TransportDirection::DOWNSTREAM &&
      msg->getMethod() == HTTPMethod::CONNECT) {
    isConnect_ = true;
  }

  if (acknowledge) {
    qpackDecoderWriteBuf_.append(headerCodec_.encodeHeaderAck(streamId_));
  }
  // Report back what we've parsed
  if (callback_) {
    if (parsingTrailers_) {
      auto trailerHeaders =
          std::make_unique<HTTPHeaders>(msg->extractHeaders());
      callback_->onTrailersComplete(streamId_, std::move(trailerHeaders));
    } else {
      // TODO: should we treat msg as chunked like H2?
      callback_->onHeadersComplete(streamId_, std::move(msg));
    }
  }
}

void HQStreamCodec::onDecodeError(HPACK::DecodeError decodeError) {
  // the parser may be paused, but this codec is dead.
  CHECK(parserPaused_);
  decodeInfo_.decodeError = decodeError;
  DCHECK_NE(decodeInfo_.decodeError, HPACK::DecodeError::NONE);
  LOG(ERROR) << "Failed decoding header block for stream=" << streamId_
             << " decodeError=" << uint32_t(decodeError);

  if (decodeInfo_.msg) {
    // print the partial message
    decodeInfo_.msg->dumpMessage(3);
  }

  if (callback_) {
    auto g = folly::makeGuard(activationHook_());
    HTTPException ex(
        HTTPException::Direction::INGRESS,
        folly::to<std::string>("Stream headers decompression error=",
                               uint32_t(decodeError)));
    ex.setHttp3ErrorCode(HTTP3::ErrorCode::HTTP_QPACK_DECOMPRESSION_FAILED);
    // HEADERS_TOO_LARGE is a stream error, everything else is a session error
    callback_->onError(decodeError == HPACK::DecodeError::HEADERS_TOO_LARGE
                           ? streamId_
                           : kSessionStreamId,
                       ex,
                       false);
  }
  // leave the partial msg in decodeInfo, it keeps the parser paused
}

void HQStreamCodec::generateHeader(
    folly::IOBufQueue& writeBuf,
    StreamID stream,
    const HTTPMessage& msg,
    bool /*eom*/,
    HTTPHeaderSize* size,
    const folly::Optional<HTTPHeaders>& extraHeaders) {
  DCHECK_EQ(stream, streamId_);
  generateHeaderImpl(writeBuf, msg, folly::none, size, extraHeaders);

  // For requests, set final header seen flag right away.
  // For responses, header is final only if response code is >= 200.
  if (msg.isRequest() || (msg.isResponse() && msg.getStatusCode() >= 200)) {
    finalEgressHeadersSeen_ = true;
  }
}

void HQStreamCodec::generatePushPromise(folly::IOBufQueue& writeBuf,
                                        StreamID stream,
                                        const HTTPMessage& msg,
                                        StreamID pushId,
                                        bool /*eom*/,
                                        HTTPHeaderSize* size) {
  DCHECK_EQ(stream, streamId_);
  DCHECK(transportDirection_ == TransportDirection::DOWNSTREAM);
  generateHeaderImpl(
      writeBuf, msg, pushId, size, folly::none /* extraHeaders */);
}

void HQStreamCodec::generateHeaderImpl(
    folly::IOBufQueue& writeBuf,
    const HTTPMessage& msg,
    folly::Optional<StreamID> pushId,
    HTTPHeaderSize* size,
    const folly::Optional<HTTPHeaders>& extraHeaders) {
  auto result = headerCodec_.encodeHTTP(qpackEncoderWriteBuf_,
                                        msg,
                                        true,
                                        streamId_,
                                        maxEncoderStreamData(),
                                        extraHeaders);
  if (size) {
    *size = headerCodec_.getEncodedSize();
  }

  logIfFieldSectionExceedsPeerMax(
      headerCodec_.getEncodedSize(),
      ingressSettings_.getSetting(SettingsId::MAX_HEADER_LIST_SIZE,
                                  std::numeric_limits<uint32_t>::max()),
      msg.getHeaders());

  // HTTP/2 serializes priority here, but HQ priorities need to go on the
  // control stream

  WriteResult res;
  if (pushId) {
    res = hq::writePushPromise(writeBuf, *pushId, std::move(result));
  } else {
    res = hq::writeHeaders(writeBuf, std::move(result));
  }

  if (res.hasError()) {
    LOG(ERROR) << __func__ << ": failed to write "
               << ((pushId) ? "push promise: " : "headers: ") << res.error();
  }
}

size_t HQStreamCodec::generateBodyImpl(folly::IOBufQueue& writeBuf,
                                       std::unique_ptr<folly::IOBuf> chain) {
  auto result = hq::writeData(writeBuf, std::move(chain));
  if (result) {
    return *result;
  }
  LOG(FATAL) << "frame exceeded 2^62-1 limit";
  return 0;
}

size_t HQStreamCodec::generateBody(folly::IOBufQueue& writeBuf,
                                   StreamID stream,
                                   std::unique_ptr<folly::IOBuf> chain,
                                   folly::Optional<uint8_t> /*padding*/,
                                   bool /*eom*/) {
  DCHECK_EQ(stream, streamId_);

  size_t bytesWritten = generateBodyImpl(writeBuf, std::move(chain));

  return bytesWritten;
}

size_t HQStreamCodec::generateBodyDSR(StreamID stream,
                                      size_t length,
                                      folly::Optional<uint8_t> /*padding*/,
                                      bool /*eom*/) {
  DCHECK_EQ(stream, streamId_);

  // Assuming we have generated a single DATA frame.
  return length;
}

size_t HQStreamCodec::generateTrailers(folly::IOBufQueue& writeBuf,
                                       StreamID stream,
                                       const HTTPHeaders& trailers) {
  DCHECK_EQ(stream, streamId_);
  std::vector<compress::Header> allTrailers;
  CodecUtil::appendHeaders(trailers, allTrailers, HTTP_HEADER_NONE);
  auto encodeRes =
      headerCodec_.encode(allTrailers, streamId_, maxEncoderStreamData());
  qpackEncoderWriteBuf_.append(std::move(encodeRes.control));

  logIfFieldSectionExceedsPeerMax(
      headerCodec_.getEncodedSize(),
      ingressSettings_.getSetting(SettingsId::MAX_HEADER_LIST_SIZE,
                                  std::numeric_limits<uint32_t>::max()),
      trailers);
  WriteResult res;
  res = hq::writeHeaders(writeBuf, std::move(encodeRes.stream));

  if (res.hasError()) {
    LOG(ERROR) << __func__ << ": failed to write trailers: " << res.error();
    return 0;
  }
  return *res;
}

size_t HQStreamCodec::generateEOM(folly::IOBufQueue& /*writeBuf*/,
                                  StreamID stream) {
  // Generate EOM is a no-op
  DCHECK_EQ(stream, streamId_);
  return 0;
}

CompressionInfo HQStreamCodec::getCompressionInfo() const {
  return headerCodec_.getCompressionInfo();
}

}} // namespace proxygen::hq

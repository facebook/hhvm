/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HTTP1xCodec.h>

#include <folly/Memory.h>
#include <folly/Random.h>
#include <folly/base64.h>
#include <folly/ssl/OpenSSLHash.h>
#include <proxygen/lib/http/HTTPHeaderSize.h>
#include <proxygen/lib/http/RFC2616.h>
#include <proxygen/lib/http/codec/CodecProtocol.h>
#include <proxygen/lib/http/codec/CodecUtil.h>

using folly::IOBuf;
using folly::IOBufQueue;
using folly::StringPiece;
using std::string;
using std::unique_ptr;

namespace {

static const std::string kChunked = "chunked";
const char CRLF[] = "\r\n";

/**
 * Write an ASCII decimal representation of an integer value
 * @note This function does -not- append a trailing null byte.
 * @param value  Integer value to write.
 * @param dst    Location to which the value will be written.
 * @return Number of bytes written.
 */
unsigned u64toa(uint64_t value, void* dst) {
  // Write backwards.
  char* next = (char*)dst;
  char* start = next;
  do {
    *next++ = '0' + (value % 10);
    value /= 10;
  } while (value != 0);
  unsigned length = next - start;

  // Reverse in-place.
  next--;
  while (next > start) {
    char swap = *next;
    *next = *start;
    *start = swap;
    next--;
    start++;
  }
  return length;
}

void appendUint(IOBufQueue& queue, size_t& len, uint64_t value) {
  char buf[32];
  size_t encodedLen = u64toa(value, buf);
  queue.append(buf, encodedLen);
  len += encodedLen;
}

#define appendLiteral(queue, len, str) \
  (len) += (sizeof(str) - 1);          \
  (queue).append(str, sizeof(str) - 1)

void appendString(IOBufQueue& queue, size_t& len, StringPiece str) {
  queue.append(str.data(), str.size());
  len += str.size();
}

} // anonymous namespace

namespace proxygen {

HTTP1xCodec::HTTP1xCodec(TransportDirection direction,
                         bool force1_1,
                         bool strictValidation)
    : callback_(nullptr),
      ingressTxnID_(0),
      egressTxnID_(0),
      currentIngressBuf_(nullptr),
      headerParseState_(HeaderParseState::kParsingHeaderIdle),
      transportDirection_(direction),
      keepaliveRequested_(KeepaliveRequested::UNSET),
      force1_1_(force1_1),
      strictValidation_(strictValidation),
      parserActive_(false),
      pendingEOF_(false),
      parserPaused_(false),
      parserError_(false),
      requestPending_(false),
      responsePending_(false),
      egressChunked_(false),
      inChunk_(false),
      lastChunkWritten_(false),
      keepalive_(true),
      disableKeepalivePending_(false),
      connectRequest_(false),
      headRequest_(false),
      expectNoResponseBody_(false),
      mayChunkEgress_(false),
      is1xxResponse_(false),
      inRecvLastChunk_(false),
      ingressUpgrade_(false),
      ingressUpgradeComplete_(false),
      egressUpgrade_(false),
      nativeUpgrade_(false),
      headersComplete_(false),
      releaseEgressAfterRequest_(false) {
  switch (direction) {
    case TransportDirection::DOWNSTREAM:
      http_parser_init(&parser_, HTTP_REQUEST);
      break;
    case TransportDirection::UPSTREAM:
      http_parser_init(&parser_, HTTP_RESPONSE);
      break;
    default:
      LOG(FATAL) << "Unknown transport direction.";
  }
  parser_.data = this;
}

HTTP1xCodec::~HTTP1xCodec() {
  // This code used to throw a parse error there were unterminated headers
  // being parsed.  None of the cases where this can happen relied on the parse
  // error.
}

HTTPCodec::StreamID HTTP1xCodec::createStream() {
  if (transportDirection_ == TransportDirection::DOWNSTREAM) {
    return ++ingressTxnID_;
  } else {
    return ++egressTxnID_;
  }
}

void HTTP1xCodec::setParserPaused(bool paused) {
  if ((paused == parserPaused_) || parserError_) {
    // If we're bailing early, we better be paused already
    DCHECK(parserError_ ||
           (HTTP_PARSER_ERRNO(&parser_) == HPE_PAUSED) == paused);
    return;
  }
  if (paused) {
    if (HTTP_PARSER_ERRNO(&parser_) == HPE_OK) {
      http_parser_pause(&parser_, 1);
    }
  } else {
    http_parser_pause(&parser_, 0);
  }
  parserPaused_ = paused;
}

const http_parser_settings* HTTP1xCodec::getParserSettings() {
  static http_parser_settings parserSettings = [] {
    http_parser_settings st;
    st.on_message_begin = HTTP1xCodec::onMessageBeginCB;
    st.on_url = HTTP1xCodec::onUrlCB;
    st.on_header_field = HTTP1xCodec::onHeaderFieldCB;
    st.on_header_value = HTTP1xCodec::onHeaderValueCB;
    st.on_headers_complete = HTTP1xCodec::onHeadersCompleteCB;
    st.on_body = HTTP1xCodec::onBodyCB;
    st.on_message_complete = HTTP1xCodec::onMessageCompleteCB;
    st.on_reason = HTTP1xCodec::onReasonCB;
    st.on_chunk_header = HTTP1xCodec::onChunkHeaderCB;
    st.on_chunk_complete = HTTP1xCodec::onChunkCompleteCB;
    return st;
  }();
  return &parserSettings;
}

size_t HTTP1xCodec::onIngress(const IOBuf& buf) {
  folly::io::Cursor cursor(&buf);

  size_t totalBytesParsed = 0;
  while (!cursor.isAtEnd()) {
    folly::IOBuf currentReadBuf;
    auto bufSize = cursor.peekBytes().size();
    cursor.cloneAtMost(currentReadBuf, bufSize);
    size_t bytesParsed = onIngressImpl(currentReadBuf);
    if (bytesParsed == 0) {
      // If the codec didn't make any progress with current input, we
      // wait for more data until this is called again
      break;
    }
    totalBytesParsed += bytesParsed;
    if (bufSize > bytesParsed) {
      cursor.retreat(bufSize - bytesParsed);
    }
    if (isParserPaused()) {
      break;
    }
  }
  return totalBytesParsed;
}

size_t HTTP1xCodec::onIngressImpl(const IOBuf& buf) {
  if (parserError_) {
    return 0;
  } else if (ingressUpgradeComplete_) {
    callback_->onBody(ingressTxnID_, buf.clone(), 0);
    return buf.computeChainDataLength();
  } else {
    // Callers responsibility to prevent calling onIngress from a callback
    CHECK(!parserActive_);
    parserActive_ = true;
    currentIngressBuf_ = &buf;
    if (transportDirection_ == TransportDirection::UPSTREAM &&
        parser_.http_major == 0 && parser_.http_minor == 9) {
      // HTTP/0.9 responses have no header block, so create a fake 200 response
      // and put the codec in upgrade mode
      onMessageBegin();
      parser_.status_code = 200;
      msg_->setStatusCode(200);
      onHeadersComplete(0);
      parserActive_ = false;
      ingressUpgradeComplete_ = true;
      return onIngressImpl(buf);
    }
    size_t bytesParsed = http_parser_execute_options(
        &parser_,
        getParserSettings(),
        strictValidation_ ? F_HTTP_PARSER_OPTIONS_URL_STRICT : 0,
        (const char*)buf.data(),
        buf.length());
    // in case we parsed a section of the headers but we're not done parsing
    // the headers we need to keep accounting of it for total header size
    if (!headersComplete_) {
      headerSize_.uncompressed += bytesParsed;
      headerSize_.compressed += bytesParsed;
    }
    parserActive_ = false;
    parserError_ = (HTTP_PARSER_ERRNO(&parser_) != HPE_OK) &&
                   (HTTP_PARSER_ERRNO(&parser_) != HPE_PAUSED);
    if (parserError_) {
      onParserError();
    }
    if (currentHeaderName_.empty() && !currentHeaderNameStringPiece_.empty()) {
      // we currently are storing a chunk of header name via pointers in
      // currentHeaderNameStringPiece_, but the currentIngressBuf_ is about to
      // vanish and so we need to copy over that data to currentHeaderName_
      currentHeaderName_.assign(currentHeaderNameStringPiece_.begin(),
                                currentHeaderNameStringPiece_.size());
    }
    currentIngressBuf_ = nullptr;
    if (pendingEOF_) {
      onIngressEOF();
      pendingEOF_ = false;
    }
    return bytesParsed;
  }
}

void HTTP1xCodec::onIngressEOF() {
  if (parserError_) {
    return;
  }
  if (parserActive_) {
    pendingEOF_ = true;
    return;
  }
  if (ingressUpgradeComplete_) {
    callback_->onMessageComplete(ingressTxnID_, false);
    return;
  }
  parserActive_ = true;
  if (http_parser_execute(&parser_, getParserSettings(), nullptr, 0) != 0) {
    parserError_ = true;
  } else {
    parserError_ = (HTTP_PARSER_ERRNO(&parser_) != HPE_OK) &&
                   (HTTP_PARSER_ERRNO(&parser_) != HPE_PAUSED);
  }
  parserActive_ = false;
  if (parserError_) {
    onParserError();
  }
}

void HTTP1xCodec::onParserError(const char* what) {
  inRecvLastChunk_ = false;
  http_errno parser_errno = HTTP_PARSER_ERRNO(&parser_);
  HTTPException error(
      HTTPException::Direction::INGRESS,
      what ? what
           : folly::to<std::string>("Error parsing message: ",
                                    http_errno_description(parser_errno)));
  // generate a string of parsed headers so that we can pass it to callback
  if (msg_) {
    error.setPartialMsg(std::move(msg_));
  }
  // store the ingress buffer
  if (currentIngressBuf_) {
    error.setCurrentIngressBuf(currentIngressBuf_->cloneOne());
  }
  if (transportDirection_ == TransportDirection::DOWNSTREAM &&
      egressTxnID_ < ingressTxnID_) {
    error.setHttpStatusCode(400);
  } // else we've already egressed a response for this txn, don't attempt a 400
  // See http_parser.h for what these error codes mean
  if (parser_errno == HPE_INVALID_EOF_STATE) {
    error.setProxygenError(kErrorEOF);
  } else if (parser_errno == HPE_HEADER_OVERFLOW ||
             parser_errno == HPE_INVALID_CONSTANT ||
             (parser_errno >= HPE_INVALID_VERSION &&
              parser_errno <= HPE_HUGE_CONTENT_LENGTH) ||
             parser_errno == HPE_CB_header_field ||
             parser_errno == HPE_CB_header_value ||
             parser_errno == HPE_CB_headers_complete) {
    error.setProxygenError(kErrorParseHeader);
  } else if (parser_errno == HPE_INVALID_CHUNK_SIZE ||
             parser_errno == HPE_HUGE_CHUNK_SIZE) {
    error.setProxygenError(kErrorParseBody);
  } else {
    error.setProxygenError(kErrorUnknown);
  }
  callback_->onError(ingressTxnID_, error);
}

bool HTTP1xCodec::isReusable() const {
  return keepalive_ && !egressUpgrade_ && !ingressUpgrade_ && !parserError_ &&
         websockAcceptKey_.empty();
}

bool HTTP1xCodec::isBusy() const {
  return requestPending_ || responsePending_;
}

void HTTP1xCodec::addDateHeader(IOBufQueue& writeBuf, size_t& len) {
  appendLiteral(writeBuf, len, "Date: ");
  appendString(writeBuf, len, HTTPMessage::formatDateHeader());
  appendLiteral(writeBuf, len, CRLF);
}

constexpr folly::StringPiece kUpgradeToken = "websocket";
constexpr folly::StringPiece kUpgradeConnectionToken = "Upgrade";
// websocket/http1.1 draft.
constexpr folly::StringPiece kWSMagicString =
    "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

std::string HTTP1xCodec::generateWebsocketKey() const {
  std::array<unsigned char, 16> arr;
  folly::Random::secureRandom(arr.data(), arr.size());
  return folly::base64Encode(std::string_view((char*)arr.data(), arr.size()));
}

std::string HTTP1xCodec::generateWebsocketAccept(const std::string& key) const {
  folly::ssl::OpenSSLHash::Digest digest;
  digest.hash_init(EVP_sha1());
  digest.hash_update(folly::StringPiece(key));
  digest.hash_update(kWSMagicString);
  std::array<unsigned char, 20> arr;
  folly::MutableByteRange accept(arr.data(), arr.size());
  digest.hash_final(accept);
  return folly::base64Encode(
      std::string_view((char*)accept.data(), accept.size()));
}

void HTTP1xCodec::serializeWebsocketHeader(IOBufQueue& writeBuf,
                                           size_t& len,
                                           bool upstream) {
  if (upstream) {
    appendLiteral(writeBuf, len, "Upgrade: ");
    appendString(writeBuf, len, kUpgradeToken.str());
    appendLiteral(writeBuf, len, CRLF);
    upgradeHeader_ = kUpgradeToken.str();

    auto key = generateWebsocketKey();
    appendLiteral(writeBuf, len, "Sec-WebSocket-Key: ");
    appendString(writeBuf, len, key);
    appendLiteral(writeBuf, len, CRLF);
    DCHECK(websockAcceptKey_.empty());
    websockAcceptKey_ = generateWebsocketAccept(key);
  } else {
    appendLiteral(writeBuf, len, "Upgrade: ");
    appendString(writeBuf, len, kUpgradeToken.str());
    appendLiteral(writeBuf, len, CRLF);

    appendLiteral(writeBuf, len, "Sec-WebSocket-Accept: ");
    appendString(writeBuf, len, websockAcceptKey_);
    appendLiteral(writeBuf, len, CRLF);
  }
}

void HTTP1xCodec::generateHeader(
    IOBufQueue& writeBuf,
    StreamID txn,
    const HTTPMessage& msg,
    bool eom,
    HTTPHeaderSize* size,
    const folly::Optional<HTTPHeaders>& extraHeaders) {
  if (keepalive_ && disableKeepalivePending_) {
    keepalive_ = false;
  }
  const bool upstream = (transportDirection_ == TransportDirection::UPSTREAM);
  const bool downstream = !upstream;
  if (upstream) {
    DCHECK_EQ(txn, egressTxnID_);
    requestPending_ = true;
    responsePending_ = true;
    connectRequest_ = (msg.getMethod() == HTTPMethod::CONNECT);
    headRequest_ = (msg.getMethod() == HTTPMethod::HEAD);
    expectNoResponseBody_ = connectRequest_ || headRequest_;
  } else {
    // In HTTP, transactions must be egressed sequentially -- no out of order
    // responses.  So txn must be egressTxnID_ + 1.  Furthermore, we shouldn't
    // ever egress a response before we see a request, so txn can't
    // be > ingressTxnID_
    if ((txn != egressTxnID_ + 1 && !(txn == egressTxnID_ && is1xxResponse_)) ||
        (txn > ingressTxnID_)) {
      LOG(DFATAL) << "Out of order, duplicate or premature HTTP response";
    }
    if (!is1xxResponse_) {
      ++egressTxnID_;
    }
    is1xxResponse_ = msg.is1xxResponse() || msg.isEgressWebsocketUpgrade();

    expectNoResponseBody_ =
        connectRequest_ || headRequest_ ||
        RFC2616::responseBodyMustBeEmpty(msg.getStatusCode());
  }

  int statusCode = 0;
  StringPiece statusMessage;
  if (downstream) {
    statusCode = msg.getStatusCode();
    statusMessage = msg.getStatusMessage();
    // If a response to a websocket upgrade is being sent out, it must be 101.
    // This is required since the application may not have changed the status,
    // particularly when proxying between a H2 hop and a H1 hop.
    if (msg.isEgressWebsocketUpgrade()) {
      statusCode = 101;
      statusMessage = HTTPMessage::getDefaultReason(101);
    }
    if (connectRequest_ && (statusCode >= 200 && statusCode < 300)) {
      // Set egress upgrade flag if we are sending a 200 response
      // to a CONNECT request we received earlier.
      egressUpgrade_ = true;
    } else if (statusCode == 101) {
      // Set the upgrade flags if we upgraded after the request from client.
      ingressUpgrade_ = true;
      egressUpgrade_ = true;
    } else if (connectRequest_ && ingressUpgrade_) {
      // Disable upgrade when rejecting CONNECT request
      ingressUpgrade_ = false;

      // This codec/session is no longer useful as we might have
      // forwarded some data before receiving the 200.
      keepalive_ = false;
    }
  } else {
    if (connectRequest_ || msg.isEgressWebsocketUpgrade()) {
      // Sending a CONNECT request or a websocket upgrade request to an upstream
      // server. This is used to determine the chunked setting below.
      egressUpgrade_ = true;
    }
  }

  egressChunked_ = msg.getIsChunked() && !egressUpgrade_;
  lastChunkWritten_ = false;
  std::pair<uint8_t, uint8_t> version = msg.getHTTPVersion();
  if (version > HTTPMessage::kHTTPVersion11) {
    version = HTTPMessage::kHTTPVersion11;
  }

  size_t len = 0;
  switch (transportDirection_) {
    case TransportDirection::DOWNSTREAM:
      DCHECK_NE(statusCode, 0);
      if (version == HTTPMessage::kHTTPVersion09) {
        return;
      }
      if (force1_1_ && version < HTTPMessage::kHTTPVersion11) {
        version = HTTPMessage::kHTTPVersion11;
      }
      appendLiteral(writeBuf, len, "HTTP/");
      appendUint(writeBuf, len, version.first);
      appendLiteral(writeBuf, len, ".");
      appendUint(writeBuf, len, version.second);
      appendLiteral(writeBuf, len, " ");
      appendUint(writeBuf, len, statusCode);
      appendLiteral(writeBuf, len, " ");
      appendString(writeBuf, len, statusMessage);
      break;
    case TransportDirection::UPSTREAM:
      if (force1_1_ && version < HTTPMessage::kHTTPVersion11) {
        version = HTTPMessage::kHTTPVersion11;
      }
      if (msg.isEgressWebsocketUpgrade()) {
        appendString(writeBuf, len, methodToString(HTTPMethod::GET));
      } else {
        appendString(writeBuf, len, msg.getMethodString());
      }
      appendLiteral(writeBuf, len, " ");
      if (connectRequest_ && msg.getURL().empty()) {
        appendString(
            writeBuf, len, msg.getHeaders().getSingleOrEmpty(HTTP_HEADER_HOST));
      } else {
        appendString(writeBuf, len, msg.getURL());
      }
      if (version != HTTPMessage::kHTTPVersion09) {
        appendLiteral(writeBuf, len, " HTTP/");
        appendUint(writeBuf, len, version.first);
        appendLiteral(writeBuf, len, ".");
        appendUint(writeBuf, len, version.second);
      }
      mayChunkEgress_ = (version.first == 1) && (version.second >= 1);
      if (!upgradeHeader_.empty()) {
        LOG(DFATAL)
            << "Attempted to pipeline HTTP request with pending upgrade";
        upgradeHeader_.clear();
      }
      break;
  }
  appendLiteral(writeBuf, len, CRLF);

  if (keepalive_ && (!msg.wantsKeepalive() || version.first < 1 ||
                     (downstream && version == HTTPMessage::kHTTPVersion10 &&
                      keepaliveRequested_ != KeepaliveRequested::ENABLED))) {
    // Disable keepalive if
    //  - the message asked to turn it off
    //  - it's HTTP/0.9
    //  - this is a response to a 1.0 request that didn't say keep-alive
    keepalive_ = false;
  }
  egressChunked_ &= mayChunkEgress_;
  if (version == HTTPMessage::kHTTPVersion09) {
    parser_.http_major = 0;
    parser_.http_minor = 9;
    return;
  }
  folly::StringPiece deferredContentLength;
  bool hasTransferEncodingChunked = false;
  bool hasDateHeader = false;
  bool hasUpgradeHeader = false;
  std::vector<StringPiece> connectionTokens;
  size_t lastConnectionToken = 0;
  bool egressWebsocketUpgrade = msg.isEgressWebsocketUpgrade();
  bool hasUpgradeTokeninConnection = false;
  auto headerEncoder = [&](HTTPHeaderCode code,
                           folly::StringPiece header,
                           folly::StringPiece value) {
    if (code == HTTP_HEADER_CONTENT_LENGTH) {
      // Write the Content-Length last (t1071703)
      deferredContentLength = value;
      return; // continue
    } else if (code == HTTP_HEADER_CONNECTION &&
               (!is1xxResponse_ || egressWebsocketUpgrade)) {
      static const string kClose = "close";
      static const string kKeepAlive = "keep-alive";
      folly::split(',', value, connectionTokens);
      for (auto curConnectionToken = lastConnectionToken;
           curConnectionToken < connectionTokens.size();
           curConnectionToken++) {
        auto token = trimWhitespace(connectionTokens[curConnectionToken]);
        if (caseInsensitiveEqual(token, "upgrade")) {
          hasUpgradeTokeninConnection = true;
        }
        if (caseInsensitiveEqual(token, kClose)) {
          keepalive_ = false;
        } else if (!caseInsensitiveEqual(token, kKeepAlive)) {
          connectionTokens[lastConnectionToken++] = token;
        } // else eat the keep-alive token
      }
      connectionTokens.resize(lastConnectionToken);
      // We'll generate a new Connection header based on the keepalive_
      // state
      return;
    } else if (code == HTTP_HEADER_UPGRADE && txn == 1) {
      hasUpgradeHeader = true;
      if (upstream) {
        // save in case we get a 101 Switching Protocols
        upgradeHeader_ = value.str();
      }
    } else if (!hasTransferEncodingChunked &&
               code == HTTP_HEADER_TRANSFER_ENCODING) {
      if (!caseInsensitiveEqual(value, kChunked)) {
        return;
      }
      hasTransferEncodingChunked = true;
      if (!mayChunkEgress_) {
        return;
      }
    } else if (!hasDateHeader && code == HTTP_HEADER_DATE) {
      hasDateHeader = true;
    } else if (egressWebsocketUpgrade &&
               code == HTTP_HEADER_SEC_WEBSOCKET_KEY) {
      // will generate our own key per hop, not client's.
      return;
    } else if (egressWebsocketUpgrade &&
               code == HTTP_HEADER_SEC_WEBSOCKET_ACCEPT) {
      // will generate our own accept per hop, not client's.
      return;
    }
    if (value.find_first_of("\r\n") != std::string::npos) {
      return;
    }
    size_t lineLen = header.size() + value.size() + 4; // 4 for ": " + CRLF
    auto writable =
        writeBuf.preallocate(lineLen, std::max(lineLen, size_t(2000)));
    char* dst = (char*)writable.first;
    memcpy(dst, header.data(), header.size());
    dst += header.size();
    *dst++ = ':';
    *dst++ = ' ';
    memcpy(dst, value.data(), value.size());
    dst += value.size();
    *dst++ = '\r';
    *dst = '\n';
    DCHECK_EQ(size_t(++dst - (char*)writable.first), lineLen);
    writeBuf.postallocate(lineLen);
    len += lineLen;
  };
  msg.getHeaders().forEachWithCode(headerEncoder);
  if (extraHeaders) {
    extraHeaders->forEachWithCode(headerEncoder);
  }

  bool bodyCheck =
      (downstream && keepalive_ && !expectNoResponseBody_ && !egressUpgrade_) ||
      // auto chunk POSTs and any request that came to us chunked
      (upstream && ((msg.getMethod() == HTTPMethod::POST) || egressChunked_));
  // TODO: 400 a 1.0 POST with no content-length
  // clear egressChunked_ if the header wasn't actually set
  egressChunked_ &= hasTransferEncodingChunked;
  if (bodyCheck && !egressChunked_ && deferredContentLength.empty()) {
    // On a connection that would otherwise be eligible for keep-alive,
    // we're being asked to send a response message with no Content-Length,
    // no chunked encoding, and no special circumstances that would eliminate
    // the need for a response body. If the client supports chunking, turn
    // on chunked encoding now.  Otherwise, turn off keepalives on this
    // connection.
    if (!hasTransferEncodingChunked && mayChunkEgress_) {
      appendLiteral(writeBuf, len, "Transfer-Encoding: chunked\r\n");
      egressChunked_ = true;
    } else {
      keepalive_ = false;
    }
  }
  if (downstream && !hasDateHeader) {
    addDateHeader(writeBuf, len);
  }

  // websocket headers
  if (msg.isEgressWebsocketUpgrade()) {
    if (!hasUpgradeHeader && txn == 1) {
      // upgradeHeader_ is set in serializeWwebsocketHeader for requests.
      serializeWebsocketHeader(writeBuf, len, upstream);
      if (!hasUpgradeTokeninConnection) {
        connectionTokens.push_back(kUpgradeConnectionToken);
        lastConnectionToken++;
      }
    } else {
      LOG(ERROR) << folly::to<string>(
          "Not serializing headers. "
          "Upgrade headers present/txn: ",
          hasUpgradeHeader,
          txn);
    }
  }

  if (!is1xxResponse_ || upstream || !connectionTokens.empty()) {
    // We don't seem to add keep-alive/close and let the application add any
    // for 1xx responses.
    appendLiteral(writeBuf, len, "Connection: ");
    if (connectionTokens.size() > 0) {
      appendString(writeBuf, len, folly::join(", ", connectionTokens));
    }
    if (!is1xxResponse_) {
      if (connectionTokens.size() > 0) {
        appendString(writeBuf, len, ", ");
      }
      if (keepalive_) {
        appendLiteral(writeBuf, len, "keep-alive");
      } else {
        appendLiteral(writeBuf, len, "close");
      }
    }
    appendLiteral(writeBuf, len, "\r\n");
  }

  if (!deferredContentLength.empty()) {
    appendLiteral(writeBuf, len, "Content-Length: ");
    appendString(writeBuf, len, deferredContentLength);
    appendLiteral(writeBuf, len, CRLF);
  }
  appendLiteral(writeBuf, len, CRLF);
  if (eom) {
    len += generateEOM(writeBuf, txn);
  }

  if (size) {
    size->compressed = len;
    size->uncompressed = len;
  }
}

size_t HTTP1xCodec::generateBody(IOBufQueue& writeBuf,
                                 StreamID txn,
                                 unique_ptr<IOBuf> chain,
                                 folly::Optional<uint8_t> /*padding*/,
                                 bool eom) {
  DCHECK_EQ(txn, egressTxnID_);
  size_t buflen = 0;
  size_t totLen = 0;
  if (chain) {
    buflen = chain->computeChainDataLength();
    totLen = buflen;
  }
  if (totLen == 0) {
    if (eom) {
      totLen += generateEOM(writeBuf, txn);
    }
    return totLen;
  }

  if (egressChunked_ && !inChunk_) {
    char chunkLenBuf[32];
    int rc = snprintf(chunkLenBuf, sizeof(chunkLenBuf), "%zx\r\n", buflen);
    CHECK_GT(rc, 0);
    CHECK_LT(size_t(rc), sizeof(chunkLenBuf));

    writeBuf.append(chunkLenBuf, rc);
    totLen += rc;

    writeBuf.append(std::move(chain));
    writeBuf.append("\r\n", 2);
    totLen += 2;
  } else {
    writeBuf.append(std::move(chain));
  }
  if (eom) {
    totLen += generateEOM(writeBuf, txn);
  }

  return totLen;
}

size_t HTTP1xCodec::generateChunkHeader(IOBufQueue& writeBuf,
                                        StreamID /*txn*/,
                                        size_t length) {
  // TODO: Format directly into the IOBuf, rather than copying after the fact.
  // IOBufQueue::append() currently forces us to copy.

  CHECK(length) << "use sendEOM to terminate the message using the "
                << "standard zero-length chunk. Don't "
                << "send zero-length chunks using this API.";
  if (egressChunked_) {
    CHECK(!inChunk_);
    inChunk_ = true;
    char chunkLenBuf[32];
    int rc = snprintf(chunkLenBuf, sizeof(chunkLenBuf), "%zx\r\n", length);
    CHECK_GT(rc, 0);
    CHECK_LT(size_t(rc), sizeof(chunkLenBuf));

    writeBuf.append(chunkLenBuf, rc);
    return rc;
  }

  return 0;
}

size_t HTTP1xCodec::generateChunkTerminator(IOBufQueue& writeBuf,
                                            StreamID /*txn*/) {
  if (egressChunked_ && inChunk_) {
    inChunk_ = false;
    writeBuf.append("\r\n", 2);
    return 2;
  }

  return 0;
}

size_t HTTP1xCodec::generateTrailers(IOBufQueue& writeBuf,
                                     StreamID txn,
                                     const HTTPHeaders& trailers) {
  DCHECK_EQ(txn, egressTxnID_);
  size_t len = 0;
  if (egressChunked_) {
    CHECK(!inChunk_);
    appendLiteral(writeBuf, len, "0\r\n");
    lastChunkWritten_ = true;
    trailers.forEach([&](const string& trailer, const string& value) {
      appendString(writeBuf, len, trailer);
      appendLiteral(writeBuf, len, ": ");
      appendString(writeBuf, len, value);
      appendLiteral(writeBuf, len, CRLF);
    });
  }
  return len;
}

size_t HTTP1xCodec::generateEOM(IOBufQueue& writeBuf, StreamID txn) {
  DCHECK_EQ(txn, egressTxnID_);
  size_t len = 0;
  if (egressChunked_) {
    CHECK(!inChunk_);
    if (headRequest_ && transportDirection_ == TransportDirection::DOWNSTREAM) {
      lastChunkWritten_ = true;
    } else {
      // appending a 0\r\n only if it's not a HEAD and downstream request
      if (!lastChunkWritten_) {
        lastChunkWritten_ = true;
        if (!(headRequest_ &&
              transportDirection_ == TransportDirection::DOWNSTREAM)) {
          appendLiteral(writeBuf, len, "0\r\n");
        }
      }
      appendLiteral(writeBuf, len, CRLF);
    }
  }
  switch (transportDirection_) {
    case TransportDirection::DOWNSTREAM:
      responsePending_ = false;
      break;
    case TransportDirection::UPSTREAM:
      requestPending_ = false;
      break;
  }
  return len;
}

size_t HTTP1xCodec::generateRstStream(IOBufQueue& /*writeBuf*/,
                                      StreamID /*txn*/,
                                      ErrorCode /*statusCode*/) {
  // statusCode ignored for HTTP/1.1
  // We won't be able to send anything else on the transport after this.
  keepalive_ = false;
  return 0;
}

size_t HTTP1xCodec::generateGoaway(IOBufQueue&,
                                   StreamID lastStream,
                                   ErrorCode error,
                                   std::unique_ptr<folly::IOBuf>) {
  // statusCode ignored for HTTP/1.1
  // We won't be able to send anything else on the transport after this.
  // For clients, immediately mark keepalive_ false.  For servers, wait until
  // we've flushed the next headers.
  if (transportDirection_ == TransportDirection::UPSTREAM ||
      disableKeepalivePending_ || lastStream != HTTPCodec::MaxStreamID ||
      error != ErrorCode::NO_ERROR) {
    keepalive_ = false;
  } else {
    disableKeepalivePending_ = true;
  }
  return 0;
}

void HTTP1xCodec::setAllowedUpgradeProtocols(std::list<std::string> protocols) {
  CHECK(transportDirection_ == TransportDirection::DOWNSTREAM);
  for (const auto& proto : protocols) {
    allowedNativeUpgrades_ += folly::to<string>(proto, ",");
  }
  if (!allowedNativeUpgrades_.empty()) {
    allowedNativeUpgrades_.erase(allowedNativeUpgrades_.size() - 1);
  }
}

const std::string& HTTP1xCodec::getAllowedUpgradeProtocols() {
  return allowedNativeUpgrades_;
}

int HTTP1xCodec::onMessageBegin() {
  headersComplete_ = false;
  headerSize_.uncompressed = 0;
  headerSize_.compressed = 0;
  headerParseState_ = HeaderParseState::kParsingHeaderStart;
  msg_.reset(new HTTPMessage());
  trailers_.reset();
  if (transportDirection_ == TransportDirection::DOWNSTREAM) {
    requestPending_ = true;
    responsePending_ = true;
  }
  // If there was a 1xx on this connection, don't increment the ingress txn id
  if (transportDirection_ == TransportDirection::DOWNSTREAM ||
      !is1xxResponse_) {
    ++ingressTxnID_;
  }
  if (transportDirection_ == TransportDirection::UPSTREAM) {
    is1xxResponse_ = false;
  }
  callback_->onMessageBegin(ingressTxnID_, msg_.get());
  return 0;
}

int HTTP1xCodec::onURL(const char* buf, size_t len) {
  url_.append(buf, len);
  return 0;
}

int HTTP1xCodec::onReason(const char* buf, size_t len) {
  reason_.append(buf, len);
  return 0;
}

bool HTTP1xCodec::pushHeaderNameAndValue(HTTPHeaders& hdrs) {
  // Header names are strictly validated by http_parser, however, it allows
  // quoted+escaped CTLs so run our stricter check here.
  if (strictValidation_) {
    folly::StringPiece headerName(currentHeaderName_.empty()
                                      ? currentHeaderNameStringPiece_
                                      : currentHeaderName_);
    bool compatValidate = false;
    if (!CodecUtil::validateHeaderValue(
            folly::StringPiece(currentHeaderValue_),
            compatValidate ? CodecUtil::CtlEscapeMode::STRICT_COMPAT
                           : CodecUtil::CtlEscapeMode::STRICT)) {
      LOG(ERROR) << "Invalid header name=" << headerName;
      std::cerr << " value=" << currentHeaderValue_ << std::endl;
      return false;
    }
  }
  if (LIKELY(currentHeaderName_.empty())) {
    hdrs.addFromCodec(currentHeaderNameStringPiece_.begin(),
                      currentHeaderNameStringPiece_.size(),
                      std::move(currentHeaderValue_));
  } else {
    hdrs.add(currentHeaderName_, std::move(currentHeaderValue_));
    currentHeaderName_.clear();
  }
  currentHeaderNameStringPiece_.clear();
  currentHeaderValue_.clear();
  return true;
}

int HTTP1xCodec::onHeaderField(const char* buf, size_t len) {
  bool valid = true;
  if (headerParseState_ == HeaderParseState::kParsingHeaderValue) {
    valid = pushHeaderNameAndValue(msg_->getHeaders());
  } else if (headerParseState_ == HeaderParseState::kParsingTrailerValue) {
    if (!trailers_) {
      trailers_.reset(new HTTPHeaders());
    }
    valid = pushHeaderNameAndValue(*trailers_);
  }
  if (!valid) {
    return -1;
  }

  if (isParsingHeaderOrTrailerName()) {

    // we're already parsing a header name
    if (currentHeaderName_.empty()) {
      // but we've been keeping it in currentHeaderNameStringPiece_ until now
      if (currentHeaderNameStringPiece_.end() == buf) {
        // the header name we are currently reading is contiguous in memory,
        // and so we just adjust the right end of our StringPiece;
        // this is likely because onIngress() hasn't been called since we got
        // the previous chunk (otherwise currentHeaderName_ would be nonempty)
        currentHeaderNameStringPiece_.advance(len);
      } else {
        // this is just for safety - if for any reason there is a discontinuity
        // even though we are during the same onIngress() call,
        // we fall back to currentHeaderName_
        currentHeaderName_.assign(currentHeaderNameStringPiece_.begin(),
                                  currentHeaderNameStringPiece_.size());
        currentHeaderName_.append(buf, len);
      }
    } else {
      // we had already fallen back to currentHeaderName_ before
      currentHeaderName_.append(buf, len);
    }

  } else {
    // we're not yet parsing a header name - this is the first chunk
    // (typically, there is only one)
    currentHeaderNameStringPiece_.reset(buf, len);

    if (headerParseState_ >= HeaderParseState::kParsingHeadersComplete) {
      headerParseState_ = HeaderParseState::kParsingTrailerName;
    } else {
      headerParseState_ = HeaderParseState::kParsingHeaderName;
    }
  }
  return 0;
}

int HTTP1xCodec::onHeaderValue(const char* buf, size_t len) {
  if (isParsingHeaders()) {
    headerParseState_ = HeaderParseState::kParsingHeaderValue;
  } else {
    headerParseState_ = HeaderParseState::kParsingTrailerValue;
  }
  currentHeaderValue_.append(buf, len);
  return 0;
}

int HTTP1xCodec::onHeadersComplete(size_t len) {
  if (headerParseState_ == HeaderParseState::kParsingHeaderValue) {
    if (!pushHeaderNameAndValue(msg_->getHeaders())) {
      return -1;
    }
  }

  // discard messages with folded or multiple valued Transfer-Encoding headers
  // ex : "chunked , zorg\r\n" or "\r\n chunked \r\n" (t12767790)
  HTTPHeaders& hdrs = msg_->getHeaders();
  const std::string& headerVal =
      hdrs.getSingleOrEmpty(HTTP_HEADER_TRANSFER_ENCODING);
  if (!headerVal.empty() && !caseInsensitiveEqual(headerVal, kChunked)) {
    LOG(ERROR) << "Invalid Transfer-Encoding header. Value =" << headerVal;
    return -1;
  }

  // discard messages with multiple content-length headers (t12767790)
  if (hdrs.getNumberOfValues(HTTP_HEADER_CONTENT_LENGTH) > 1) {
    // Only reject the message if the Content-Length headers have different
    // values
    folly::Optional<folly::StringPiece> contentLen;
    bool error = hdrs.forEachValueOfHeader(
        HTTP_HEADER_CONTENT_LENGTH, [&](folly::StringPiece value) -> bool {
          if (!contentLen.has_value()) {
            contentLen = value;
            return false;
          }
          return (contentLen.value() != value);
        });

    if (error) {
      LOG(ERROR) << "Invalid message, multiple Content-Length headers";
      return -1;
    }
  }

  // Update the HTTPMessage with the values parsed from the header
  msg_->setHTTPVersion(parser_.http_major, parser_.http_minor);
  msg_->setIsChunked((parser_.flags & F_CHUNKED));

  if (transportDirection_ == TransportDirection::DOWNSTREAM) {
    // Set the method type
    msg_->setMethod(http_method_str(static_cast<http_method>(parser_.method)));

    connectRequest_ = (msg_->getMethod() == HTTPMethod::CONNECT);

    // If this is a headers-only request, we shouldn't send
    // an entity-body in the response.
    headRequest_ = (msg_->getMethod() == HTTPMethod::HEAD);

    ParseURL parseUrl = msg_->setURL(std::move(url_), strictValidation_);
    if (strictValidation_ && !parseUrl.valid()) {
      LOG(ERROR) << "Invalid URL: " << msg_->getURL();
      return -1;
    }
    url_.clear();

    if (parseUrl.hasHost()) {
      // RFC 2616 5.2.1 states "If Request-URI is an absoluteURI, the host
      // is part of the Request-URI. Any Host header field value in the
      // request MUST be ignored."
      auto hostAndPort = parseUrl.hostAndPort();
      VLOG(4) << "Adding inferred host header: " << hostAndPort;
      msg_->getHeaders().set(HTTP_HEADER_HOST, hostAndPort);
    }

    // If the client sent us an HTTP/1.x with x >= 1, we may send
    // chunked responses.
    mayChunkEgress_ = ((parser_.http_major == 1) && (parser_.http_minor >= 1));
  } else {
    msg_->setStatusCode(parser_.status_code);
    msg_->setStatusMessage(std::move(reason_));
    reason_.clear();
  }

  auto g = folly::makeGuard([this] {
    // Always clear the outbound upgrade header after we receive a response
    if (transportDirection_ == TransportDirection::UPSTREAM &&
        parser_.status_code != 100) {
      upgradeHeader_.clear();
    }
  });
  headerParseState_ = HeaderParseState::kParsingHeadersComplete;
  if (transportDirection_ == TransportDirection::UPSTREAM) {
    if (connectRequest_ &&
        (parser_.status_code >= 200 && parser_.status_code < 300)) {
      // Enable upgrade if this is a 200 response to a CONNECT
      // request we sent earlier
      ingressUpgrade_ = true;
    } else if (parser_.status_code == 101) {
      // Set the upgrade flags if the server has upgraded.
      const std::string& serverUpgrade =
          msg_->getHeaders().getSingleOrEmpty(HTTP_HEADER_UPGRADE);
      if (serverUpgrade.empty() || upgradeHeader_.empty()) {
        LOG(ERROR) << "Invalid 101 response, empty upgrade headers";
        return -1;
      }
      auto result = checkForProtocolUpgrade(
          upgradeHeader_, serverUpgrade, false /* client mode */);
      if (result) {
        ingressUpgrade_ = true;
        egressUpgrade_ = true;
        if (result->first != CodecProtocol::HTTP_1_1) {
          bool success = callback_->onNativeProtocolUpgrade(
              ingressTxnID_, result->first, result->second, *msg_);
          if (success) {
            nativeUpgrade_ = true;
            msg_->setIsUpgraded(ingressUpgrade_);
            return 1; // no message body if successful
          }
        } else if (result->second == getCodecProtocolString(result->first)) {
          // someone upgraded to http/1.1?  Reset upgrade flags
          ingressUpgrade_ = false;
          egressUpgrade_ = false;
        }
        // else, there's some non-native upgrade
      } else {
        LOG(ERROR) << "Invalid 101 response, client/server upgrade mismatch "
                      "client="
                   << upgradeHeader_ << " server=" << serverUpgrade;
        return -1;
      }
    } else if (parser_.upgrade || parser_.flags & F_UPGRADE) {
      // Ignore upgrade header for upstream response messages with status code
      // different from 101 in case if it was not a response to CONNECT.
      parser_.upgrade = false;
      parser_.flags &= ~F_UPGRADE;
    }
  } else {
    if (connectRequest_) {
      // Enable upgrade by default for the CONNECT requests.
      // If we locally reject CONNECT, we will disable this flag while
      // sending the reject response. If we forward the req to upstream proxy,
      // we will start forwarding data to the proxy without waiting for
      // the response from the proxy server.
      ingressUpgrade_ = true;
    } else if (!allowedNativeUpgrades_.empty() && ingressTxnID_ == 1) {
      upgradeHeader_ = msg_->getHeaders().getSingleOrEmpty(HTTP_HEADER_UPGRADE);
      if (!upgradeHeader_.empty() && !allowedNativeUpgrades_.empty()) {
        auto result = checkForProtocolUpgrade(
            upgradeHeader_, allowedNativeUpgrades_, true /* server mode */);
        if (result && result->first != CodecProtocol::HTTP_1_1) {
          nativeUpgrade_ = true;
          upgradeResult_ = *result;
          // unfortunately have to copy because msg_ is passed to
          // onHeadersComplete
          upgradeRequest_ = std::make_unique<HTTPMessage>(*msg_);
        }
      }
    }
  }
  msg_->setIsUpgraded(ingressUpgrade_);

  const std::string& upgrade = hdrs.getSingleOrEmpty(HTTP_HEADER_UPGRADE);
  if (kUpgradeToken.equals(upgrade, folly::AsciiCaseInsensitive())) {
    msg_->setIngressWebsocketUpgrade();
    if (transportDirection_ == TransportDirection::UPSTREAM) {
      // response.
      const std::string& accept =
          hdrs.getSingleOrEmpty(HTTP_HEADER_SEC_WEBSOCKET_ACCEPT);
      if (accept != websockAcceptKey_) {
        LOG(ERROR) << "Mismatch in expected ws accept key: "
                   << "upstream: " << accept
                   << " expected: " << websockAcceptKey_;
        return -1;
      }
    } else {
      // request.
      // If the websockAcceptKey is already set, we error out.
      // Currently, websockAcceptKey is never cleared, which means
      // that only one Websocket upgrade attempt can be made on the
      // connection. If that upgrade request is not successful for any
      // reason, the connection is no longer usable. At some point, we
      // may want to change this to clear the websockAcceptKey if
      // the request doesn't succeed keeping the connection usable.
      if (!websockAcceptKey_.empty()) {
        LOG(ERROR) << "ws accept key already set: '" << websockAcceptKey_
                   << "'";
        return -1;
      }
      auto key = hdrs.getSingleOrEmpty(HTTP_HEADER_SEC_WEBSOCKET_KEY);
      websockAcceptKey_ = generateWebsocketAccept(key);
    }
  }

  bool msgKeepalive = msg_->computeKeepalive();
  if (!msgKeepalive) {
    keepalive_ = false;
  }
  if (transportDirection_ == TransportDirection::DOWNSTREAM) {
    // Remember whether this was an HTTP 1.0 request with keepalive enabled
    if (msgKeepalive && msg_->isHTTP1_0() &&
        (keepaliveRequested_ == KeepaliveRequested::UNSET ||
         keepaliveRequested_ == KeepaliveRequested::ENABLED)) {
      keepaliveRequested_ = KeepaliveRequested::ENABLED;
    } else {
      keepaliveRequested_ = KeepaliveRequested::DISABLED;
    }
  }

  // Determine whether the HTTP parser should ignore any headers
  // that indicate the presence of a message body.  This is needed,
  // for example, if the message is a response to a request with
  // method==HEAD.
  bool ignoreBody;
  if (transportDirection_ == TransportDirection::DOWNSTREAM) {
    ignoreBody = false;
  } else {
    is1xxResponse_ = msg_->is1xxResponse();
    if (expectNoResponseBody_) {
      ignoreBody = true;
    } else {
      ignoreBody = RFC2616::responseBodyMustBeEmpty(msg_->getStatusCode());
    }
  }

  headersComplete_ = true;
  headerSize_.uncompressed += len;
  headerSize_.compressed += len;
  msg_->setIngressHeaderSize(headerSize_);

  if (userAgent_.empty()) {
    userAgent_ = msg_->getHeaders().getSingleOrEmpty(HTTP_HEADER_USER_AGENT);
  }
  callback_->onHeadersComplete(ingressTxnID_, std::move(msg_));

  // 1 is a magic value that tells the http_parser not to expect a
  // message body even if the message header implied the presence
  // of one (e.g., via a Content-Length)
  return (ignoreBody) ? 1 : 0;
}

int HTTP1xCodec::onBody(const char* buf, size_t len) {
  DCHECK(!isParsingHeaders());
  DCHECK(!inRecvLastChunk_);
  CHECK_NOTNULL(currentIngressBuf_);
  const char* dataStart = (const char*)currentIngressBuf_->data();
  const char* dataEnd = dataStart + currentIngressBuf_->length();
  DCHECK_GE(buf, dataStart);
  DCHECK_LE(buf + len, dataEnd);
  unique_ptr<IOBuf> clone(currentIngressBuf_->cloneOne());
  clone->trimStart(buf - dataStart);
  clone->trimEnd(dataEnd - (buf + len));
  DCHECK_EQ(len, clone->computeChainDataLength());
  callback_->onBody(ingressTxnID_, std::move(clone), 0);
  return 0;
}

int HTTP1xCodec::onChunkHeader(size_t len) {
  if (len > 0) {
    callback_->onChunkHeader(ingressTxnID_, len);
  } else {
    VLOG(5) << "Suppressed onChunkHeader callback for final zero length "
            << "chunk";
    inRecvLastChunk_ = true;
  }
  return 0;
}

int HTTP1xCodec::onChunkComplete() {
  if (inRecvLastChunk_) {
    inRecvLastChunk_ = false;
  } else {
    callback_->onChunkComplete(ingressTxnID_);
  }
  return 0;
}

int HTTP1xCodec::onMessageComplete() {
  DCHECK(!isParsingHeaders());
  DCHECK(!inRecvLastChunk_);
  if (headerParseState_ == HeaderParseState::kParsingTrailerValue) {
    if (!trailers_) {
      trailers_.reset(new HTTPHeaders());
    }
    if (!pushHeaderNameAndValue(*trailers_)) {
      return -1;
    }
  }

  headerParseState_ = HeaderParseState::kParsingHeaderIdle;
  if (trailers_) {
    callback_->onTrailersComplete(ingressTxnID_, std::move(trailers_));
  }

  switch (transportDirection_) {
    case TransportDirection::DOWNSTREAM: {
      requestPending_ = false;
      if (upgradeRequest_) {
        ingressUpgrade_ =
            callback_->onNativeProtocolUpgrade(ingressTxnID_,
                                               upgradeResult_.first,
                                               upgradeResult_.second,
                                               *upgradeRequest_);
        upgradeRequest_.reset();
      }
      // else there was no match, OR we upgraded to http/1.1 OR someone
      // specified a non-native protocol in the setAllowedUpgradeProtocols.
      // No-ops
      break;
    }
    case TransportDirection::UPSTREAM:
      responsePending_ = is1xxResponse_;
      if (is1xxResponse_ && !nativeUpgrade_ && !ingressUpgrade_) {
        // Some other 1xx status code, which doesn't terminate the message
        return 0;
      }
  }

  // For downstream, always call onMessageComplete. If native upgrade,
  // pass upgrade=false. Else pass ingressUpgrade_.
  // For upstream, call onMessagComplete if not native upgrade.
  if (!nativeUpgrade_) {
    callback_->onMessageComplete(ingressTxnID_, ingressUpgrade_);
  } else if (transportDirection_ == TransportDirection::DOWNSTREAM) {
    // native upgrade and downstream.
    callback_->onMessageComplete(ingressTxnID_, false);
  }
  // else we suppressed onHeadersComplete, suppress onMessageComplete also.
  // The new codec will handle these callbacks with the real message

  if (ingressUpgrade_) {
    ingressUpgradeComplete_ = true;
    // If upgrade is complete, any pending data should not be parsed.
    // It must be forwarded directly to the handler.
    setParserPaused(true);
  }

  return 0;
}

int HTTP1xCodec::onMessageBeginCB(http_parser* parser) {
  HTTP1xCodec* codec = static_cast<HTTP1xCodec*>(parser->data);
  DCHECK(codec != nullptr);
  DCHECK_EQ(&codec->parser_, parser);

  try {
    return codec->onMessageBegin();
  } catch (const std::exception& ex) {
    codec->onParserError(ex.what());
    return 1;
  }
}

int HTTP1xCodec::onUrlCB(http_parser* parser, const char* buf, size_t len) {
  HTTP1xCodec* codec = static_cast<HTTP1xCodec*>(parser->data);
  DCHECK(codec != nullptr);
  DCHECK_EQ(&codec->parser_, parser);

  try {
    return codec->onURL(buf, len);
  } catch (const std::exception& ex) {
    codec->onParserError(ex.what());
    return 1;
  }
}

int HTTP1xCodec::onReasonCB(http_parser* parser, const char* buf, size_t len) {
  HTTP1xCodec* codec = static_cast<HTTP1xCodec*>(parser->data);
  DCHECK(codec != nullptr);
  DCHECK_EQ(&codec->parser_, parser);

  try {
    return codec->onReason(buf, len);
  } catch (const std::exception& ex) {
    codec->onParserError(ex.what());
    return 1;
  }
}

int HTTP1xCodec::onHeaderFieldCB(http_parser* parser,
                                 const char* buf,
                                 size_t len) {
  HTTP1xCodec* codec = static_cast<HTTP1xCodec*>(parser->data);
  DCHECK(codec != nullptr);
  DCHECK_EQ(&codec->parser_, parser);

  try {
    return codec->onHeaderField(buf, len);
  } catch (const std::exception& ex) {
    codec->onParserError(ex.what());
    return 1;
  }
}

int HTTP1xCodec::onHeaderValueCB(http_parser* parser,
                                 const char* buf,
                                 size_t len) {
  HTTP1xCodec* codec = static_cast<HTTP1xCodec*>(parser->data);
  DCHECK(codec != nullptr);
  DCHECK_EQ(&codec->parser_, parser);

  try {
    return codec->onHeaderValue(buf, len);
  } catch (const std::exception& ex) {
    codec->onParserError(ex.what());
    return 1;
  }
}

int HTTP1xCodec::onHeadersCompleteCB(http_parser* parser,
                                     const char* /*buf*/,
                                     size_t len) {
  HTTP1xCodec* codec = static_cast<HTTP1xCodec*>(parser->data);
  DCHECK(codec != nullptr);
  DCHECK_EQ(&codec->parser_, parser);

  try {
    return codec->onHeadersComplete(len);
  } catch (const std::exception& ex) {
    codec->onParserError(ex.what());
    return 3;
  }
}

int HTTP1xCodec::onBodyCB(http_parser* parser, const char* buf, size_t len) {
  HTTP1xCodec* codec = static_cast<HTTP1xCodec*>(parser->data);
  DCHECK(codec != nullptr);
  DCHECK_EQ(&codec->parser_, parser);

  try {
    return codec->onBody(buf, len);
  } catch (const std::exception& ex) {
    // Note: http_parser appears to completely ignore the return value from the
    // on_body() callback.  There seems to be no way to abort parsing after an
    // error in on_body().
    //
    // We handle this by checking if error_ is set after each call to
    // http_parser_execute().
    codec->onParserError(ex.what());
    return 1;
  }
}

int HTTP1xCodec::onChunkHeaderCB(http_parser* parser) {
  HTTP1xCodec* codec = static_cast<HTTP1xCodec*>(parser->data);
  DCHECK(codec != nullptr);
  DCHECK_EQ(&codec->parser_, parser);

  try {
    return codec->onChunkHeader(parser->content_length);
  } catch (const std::exception& ex) {
    codec->onParserError(ex.what());
    return 1;
  }
}

int HTTP1xCodec::onChunkCompleteCB(http_parser* parser) {
  HTTP1xCodec* codec = static_cast<HTTP1xCodec*>(parser->data);
  DCHECK(codec != nullptr);
  DCHECK_EQ(&codec->parser_, parser);

  try {
    return codec->onChunkComplete();
  } catch (const std::exception& ex) {
    codec->onParserError(ex.what());
    return 1;
  }
}

int HTTP1xCodec::onMessageCompleteCB(http_parser* parser) {
  HTTP1xCodec* codec = static_cast<HTTP1xCodec*>(parser->data);
  DCHECK(codec != nullptr);
  DCHECK_EQ(&codec->parser_, parser);

  try {
    return codec->onMessageComplete();
  } catch (const std::exception& ex) {
    codec->onParserError(ex.what());
    return 1;
  }
}

bool HTTP1xCodec::supportsNextProtocol(const std::string& npn) {
  return npn.length() == 8 && (npn == "http/1.0" || npn == "http/1.1");
}

HTTP1xCodec HTTP1xCodec::makeResponseCodec(bool mayChunkEgress) {
  HTTP1xCodec codec(TransportDirection::DOWNSTREAM);
  codec.mayChunkEgress_ = mayChunkEgress;
  return codec;
}

} // namespace proxygen

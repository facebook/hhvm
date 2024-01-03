/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <array>
#include <boost/variant.hpp>
#include <folly/Conv.h>
#include <folly/Optional.h>
#include <folly/SocketAddress.h>
#include <folly/io/IOBufQueue.h>
#include <glog/logging.h>
#include <map>
#include <mutex>
#include <proxygen/lib/http/HTTPHeaderSize.h>
#include <proxygen/lib/http/HTTPHeaders.h>
#include <proxygen/lib/http/HTTPMethod.h>
#include <proxygen/lib/http/HeaderConstants.h>
#include <proxygen/lib/utils/ParseURL.h>
#include <proxygen/lib/utils/Time.h>
#include <string>

namespace proxygen {

// Default urgency = 3 is from the draft. It leaves space for both higher and
// lower urgency level which is good. We default to Incremental = True as
// opposed to False. This is because our transport layer has been behaving
// like that before the HTTP priority support is introduced.
constexpr uint8_t kDefaultHttpPriorityUrgency = 3;
// We default incremental to True, different from the draft
constexpr bool kDefaultHttpPriorityIncremental = true;
constexpr uint64_t kDefaultOrderId = 0;
constexpr int8_t kMinPriority = 0;
constexpr int8_t kMaxPriority = 7;

struct HTTPPriority {
  uint8_t urgency : 3;
  bool incremental : 1;
  uint64_t orderId : 58;

  HTTPPriority()
      : urgency(kDefaultHttpPriorityUrgency),
        incremental(kDefaultHttpPriorityIncremental),
        orderId(kDefaultOrderId) {
  }

  HTTPPriority(uint8_t urgencyIn,
               bool incrementalIn,
               uint64_t orderIdIn = kDefaultOrderId)
      : urgency(std::min(urgencyIn, static_cast<uint8_t>(kMaxPriority))),
        incremental(incrementalIn),
        orderId(orderIdIn) {
  }
};

inline bool operator==(const HTTPPriority& a, const HTTPPriority& b) {
  return a.urgency == b.urgency && a.incremental == b.incremental &&
         a.orderId == b.orderId;
}

// Convert Priority to a string representation in the form of "u=urgency[,i]"
std::string httpPriorityToString(const HTTPPriority& priority);

class HTTPMessage;

folly::Optional<HTTPPriority> httpPriorityFromHTTPMessage(
    const HTTPMessage& message);

/**
 * An HTTP request or response minus the body.
 *
 * Some of the methods on this class will assert if called from the wrong
 * context since they only make sense for a request or response. Make sure
 * you know what type of HTTPMessage this is before calling such methods.
 *
 * All header names stored in this class are case-insensitive.
 */
class HTTPMessage {
 public:
  enum WebSocketUpgrade {
    NONE,
    INGRESS,
    EGRESS,
  };

  enum class Scheme {
    HTTP,
    HTTPS,
    MASQUE,
  };

  HTTPMessage();
  ~HTTPMessage();
  HTTPMessage(HTTPMessage&& message) noexcept;
  HTTPMessage(const HTTPMessage& message);
  HTTPMessage& operator=(const HTTPMessage& message);
  HTTPMessage& operator=(HTTPMessage&& message);

  // upgradeWebsocket_ can have three states, WebSocketUpgrade::NONE by
  // default. WebSocketUpgrade::INGRESS is used by the codec to indicate a
  // websocket upgrade request was received from downstream or a successful
  // upgrade finished on an upstream stream.
  // WebSocketUpgrade::EGRESS is used by the application handler to indicate
  // websocket upgrade headers should be sent with the outgoing
  // request/response. Based on upstream/downstream, the codec serializes the
  // appropriate headers.
  void setIngressWebsocketUpgrade() {
    upgradeWebsocket_ = WebSocketUpgrade::INGRESS;
  }
  void setEgressWebsocketUpgrade() {
    upgradeWebsocket_ = WebSocketUpgrade::EGRESS;
  }
  bool isIngressWebsocketUpgrade() const {
    return upgradeWebsocket_ == WebSocketUpgrade::INGRESS;
  }
  bool isEgressWebsocketUpgrade() const {
    return upgradeWebsocket_ == WebSocketUpgrade::EGRESS;
  }

  /**
   * Is this a chunked message? (fpreq, fpresp)
   */
  void setIsChunked(bool chunked) {
    chunked_ = chunked;
  }
  bool getIsChunked() const {
    return chunked_;
  }

  /**
   * Is this an upgraded message? (fpreq, fpresp)
   */
  void setIsUpgraded(bool upgraded) {
    upgraded_ = upgraded;
  }
  bool getIsUpgraded() const {
    return upgraded_;
  }

  /**
   * Set/Get client address
   */
  void setClientAddress(const folly::SocketAddress& addr,
                        std::string ipStr = empty_string,
                        std::string portStr = empty_string) {
    auto& req = request();
    req.clientAddress_ = addr;
    if (!ipStr.empty() && !portStr.empty()) {
      req.clientIPPort_.emplace(std::move(ipStr), std::move(portStr));
    } else {
      if (req.clientIPPort_) {
        req.clientIPPort_.reset();
      }
    }
  }

  const folly::SocketAddress& getClientAddress() const {
    return request().clientAddress_;
  }

  const std::string& getClientIP() const {
    auto& req = request();
    if (!req.clientIPPort_ || req.clientIPPort_->ip.empty()) {
      if (req.clientAddress_.isInitialized()) {
        req.clientIPPort_.emplace(
            req.clientAddress_.getAddressStr(),
            folly::to<std::string>(req.clientAddress_.getPort()));
      } else {
        return empty_string;
      }
    }
    return req.clientIPPort_->ip;
  }

  const std::string& getClientPort() const {
    auto& req = request();
    if (!req.clientIPPort_ || req.clientIPPort_->port.empty()) {
      if (req.clientAddress_.isInitialized()) {
        req.clientIPPort_.emplace(
            req.clientAddress_.getAddressStr(),
            folly::to<std::string>(req.clientAddress_.getPort()));
      } else {
        return empty_string;
      }
    }
    return req.clientIPPort_->port;
  }

  /**
   * Set/Get destination (vip) address
   */
  void setDstAddress(const folly::SocketAddress& addr,
                     std::string addressStr = empty_string,
                     std::string portStr = empty_string) {
    dstAddress_ = addr;
    if (!addressStr.empty() && !portStr.empty()) {
      dstIP_ = std::move(addressStr);
      dstPort_ = std::move(portStr);
    } else {
      dstIP_.clear();
      dstPort_.clear();
    }
  }

  const folly::SocketAddress& getDstAddress() const {
    return dstAddress_;
  }

  const std::string& getDstIP() const {
    if (dstIP_.empty() && dstAddress_.isInitialized()) {
      dstIP_ = dstAddress_.getAddressStr();
    }
    return dstIP_;
  }

  const std::string& getDstPort() const {
    if (dstPort_.empty() && dstAddress_.isInitialized()) {
      dstPort_ = folly::to<std::string>(dstAddress_.getPort());
    }
    return dstPort_;
  }

  /**
   * Set/Get the local IP address
   */
  template <typename T> // T = string
  void setLocalIp(T&& ip) {
    localIP_ = std::forward<T>(ip);
  }
  const std::string& getLocalIp() const {
    return localIP_;
  }

  /**
   * Access the method (fpreq)
   */
  void setMethod(HTTPMethod method);
  void setMethod(folly::StringPiece method);
  void rawSetMethod(const std::string& method) {
    setMethod(method);
  }

  /**
   * @Returns an HTTPMethod enum value representing the method if it is a
   * standard request method, or else "none" if it is an extension method
   * (fpreq)
   */
  folly::Optional<HTTPMethod> getMethod() const;

  /**
   * @Returns a string representation of the request method (fpreq)
   */
  const std::string& getMethodString() const;

  /**
   * Access the URL component (fpreq)
   *
   * The <url> component from the initial "METHOD <url> HTTP/..." line. When
   * valid, this is a full URL, not just a path.
   */
  template <typename T> // T = string
  ParseURL setURL(T&& url, bool strict = false) {
    return setURLImpl(std::forward<T>(url), true, strict);
  }

  // The template function above doesn't work with char*,
  // so explicitly convert to a string first.
  ParseURL setURL(const char* url, bool strict = false) {
    return setURL(std::string(url), strict);
  }
  const std::string& getURL() const {
    return request().url_;
  }
  void rawSetURL(const std::string& url) {
    setURL(url);
  }

  /**
   * Access the path component (fpreq)
   *
   * getPath will lazily allocate a string object, which is generally
   * more expensive.  Prefer getPathAsStringPiece.
   */
  const std::string& getPath() const {
    auto& req = request();
    if (!req.pathStr_) {
      req.pathStr_ =
          std::make_unique<std::string>(req.path_.data(), req.path_.size());
    }
    return *req.pathStr_;
  }

  folly::StringPiece getPathAsStringPiece() const {
    return request().path_;
  }

  /**
   * Access the query component (fpreq)
   *
   * getQueryString will lazily allocate a string object, which is generally
   * more expensive.  Prefer getQueryStringAsStringPiece.
   */
  const std::string& getQueryString() const {
    auto& req = request();
    if (!req.queryStr_) {
      req.queryStr_ =
          std::make_unique<std::string>(req.query_.data(), req.query_.size());
    }
    return *req.queryStr_;
  }

  folly::StringPiece getQueryStringAsStringPiece() const {
    return request().query_;
  }

  /**
   * Version constants
   */
  static const std::pair<uint8_t, uint8_t> kHTTPVersion09;
  static const std::pair<uint8_t, uint8_t> kHTTPVersion10;
  static const std::pair<uint8_t, uint8_t> kHTTPVersion11;

  /**
   * Access the HTTP version number (fpreq, fpres)
   */
  void setHTTPVersion(uint8_t major, uint8_t minor);
  const std::pair<uint8_t, uint8_t>& getHTTPVersion() const;

  /**
   * Access the HTTP status message string (res)
   */
  template <typename T> // T = string
  void setStatusMessage(T&& msg) {
    response().statusMsg_ = std::forward<T>(msg);
  }
  const std::string& getStatusMessage() const {
    return response().statusMsg_;
  }
  void rawSetStatusMessage(std::string msg) {
    setStatusMessage(msg);
  }

  /**
   * Get/Set the HTTP version string (like "1.1").
   * XXX: Note we only support X.Y format while setting version.
   */
  const std::string& getVersionString() const {
    return versionStr_;
  }
  void setVersionString(const std::string& ver) {
    if (ver.size() != 3 || ver[1] != '.' || !isdigit(ver[0]) ||
        !isdigit(ver[2])) {
      return;
    }

    setHTTPVersion(ver[0] - '0', ver[2] - '0');
  }

  /**
   * Access the headers (fpreq, fpres)
   */
  HTTPHeaders& getHeaders() {
    return headers_;
  }
  const HTTPHeaders& getHeaders() const {
    return headers_;
  }

  /**
   * Move headers out of current message (returns rvalue ref)
   */
  HTTPHeaders&& extractHeaders() {
    return std::move(headers_);
  }

  /**
   * Access the trailers
   */
  HTTPHeaders* getTrailers() {
    return trailers_.get();
  }
  const HTTPHeaders* getTrailers() const {
    return trailers_.get();
  }

  /**
   * Set the trailers, replacing any that might already be present
   */
  void setTrailers(std::unique_ptr<HTTPHeaders>&& trailers) {
    trailers_ = std::move(trailers);
  }

  /**
   * Move trailers out of current message
   */
  std::unique_ptr<HTTPHeaders> extractTrailers() {
    return std::move(trailers_);
  }

  /**
   * Decrements Max-Forwards header, when present on OPTIONS or TRACE methods.
   *
   * Returns HTTP status code.
   */
  int processMaxForwards();

  /**
   * Returns true if the version of this message is HTTP/1.0
   */
  bool isHTTP1_0() const;

  /**
   * Returns true if the version of this message is HTTP/1.1
   */
  bool isHTTP1_1() const;

  /**
   * Returns true if these are final headers.
   */
  bool isFinal() const {
    return (isRequest() || !is1xxResponse() || getStatusCode() == 101);
  }

  /**
   * Returns true if this is a 1xx response.
   */
  bool is1xxResponse() const {
    return (getStatusCode() / 100) == 1;
  }

  /**
   * Returns true if this is a 4xx response.
   */
  bool is4xxResponse() const {
    return (getStatusCode() / 100) == 4;
  }

  /**
   * Returns true if this is a 5xx response.
   */
  bool is5xxResponse() const {
    return (getStatusCode() / 100) == 5;
  }

  /**
   * Formats the current time appropriately for a Date header
   */
  static std::string formatDateHeader();

  /**
   * Ensures this HTTPMessage contains a host header, adding a default one
   * with the destination address if necessary.
   */
  void ensureHostHeader();

  /**
   * Indicates if this request wants the connection to be kept-alive
   * (default true).  Not all codecs respect this option.
   */
  void setWantsKeepalive(bool wantsKeepaliveVal) {
    wantsKeepalive_ = wantsKeepaliveVal;
  }
  bool wantsKeepalive() const {
    return wantsKeepalive_;
  }

  /**
   * Returns true if trailers are allowed on this message.  Trailers
   * are not allowed on responses unless the client is expecting them.
   */
  bool trailersAllowed() const {
    return trailersAllowed_;
  }
  /**
   * Accessor to set whether trailers are allowed in the response
   */
  void setTrailersAllowed(bool trailersAllowedVal) {
    trailersAllowed_ = trailersAllowedVal;
  }

  /**
   * Returns true if this message has trailers that need to be serialized
   */
  bool hasTrailers() const {
    return trailersAllowed_ && trailers_ && trailers_->size() > 0;
  }

  /**
   * Access the status code (fpres)
   */
  void setStatusCode(uint16_t status);
  uint16_t getStatusCode() const;

  void setUpgradeProtocol(std::string protocol) {
    upgradeProtocol_ = std::make_unique<std::string>(std::move(protocol));
  }
  const std::string* getUpgradeProtocol() const {
    return upgradeProtocol_.get();
  }

  /**
   * Access the push status code
   */
  void setPushStatusCode(const uint16_t status);
  std::string getPushStatusStr() const;
  uint16_t getPushStatusCode() const;

  /**
   * Fill in the fields for a response message header that the server will
   * send directly to the client.
   *
   * @param version           HTTP version (major, minor)
   * @param statusCode        HTTP status code to respond with
   * @param msg               textual message to embed in "message" status field
   * @param contentLength     the length of the data to be written out through
   *                          this message
   */
  void constructDirectResponse(const std::pair<uint8_t, uint8_t>& version,
                               const int statusCode,
                               const std::string& statusMsg,
                               int contentLength = 0);

  /**
   * Fill in the fields for a response message header that the server will
   * send directly to the client. This function assumes the status code and
   * status message have already been set on this HTTPMessage object
   *
   * @param version           HTTP version (major, minor)
   * @param contentLength     the length of the data to be written out through
   *                          this message
   */
  void constructDirectResponse(const std::pair<uint8_t, uint8_t>& version,
                               int contentLength = 0);

  /**
   * Check if query parameter with the specified name exists.
   */
  bool hasQueryParam(const std::string& name) const;

  /**
   * Get the query parameter with the specified name.
   *
   * Returns a pointer to the query parameter value, or nullptr if there is no
   * parameter with the specified name.  The returned value is only valid as
   * long as this HTTPMessage object.
   */
  const std::string* getQueryParamPtr(const std::string& name) const;

  /**
   * Get the query parameter with the specified name.
   *
   * Returns a reference to the query parameter value, or
   * proxygen::empty_string if there is no parameter with the
   * specified name.  The returned value is only valid as long as this
   * HTTPMessage object.
   */
  const std::string& getQueryParam(const std::string& name) const;

  /**
   * Get the query parameter with the specified name as int.
   *
   * If the conversion fails, throws exception.
   */
  int getIntQueryParam(const std::string& name) const;

  /**
   * Get the query parameter with the specified name as int.
   *
   * Returns the query parameter if it can be parsed as int otherwise the
   * default value.
   */
  int getIntQueryParam(const std::string& name, int defval) const;

  /**
   * Get the query parameter with the specified name after percent decoding.
   *
   * Returns empty string if parameter is missing or folly::uriUnescape
   * query param
   */
  std::string getDecodedQueryParam(const std::string& name) const;

  /**
   * Get all the query parameters.
   *
   * Returns a reference to the query parameters map.  The returned
   * value is only valid as long as this
   * HTTPMessage object.
   */
  const std::map<std::string, std::string>& getQueryParams() const;

  /**
   * Set the query string to the specified value, and recreate the url_.
   *
   * Returns true if the query string was changed successfully.
   */
  bool setQueryString(const std::string& query, bool strict = false);

  /**
   * Remove the query parameter with the specified name.
   *
   * Returns true if the query parameter was present and deleted.
   */
  bool removeQueryParam(const std::string& name);

  /**
   * Sets the query parameter with the specified name to the specified value.
   *
   * Returns true if the query parameter was successfully set.
   */
  bool setQueryParam(const std::string& name,
                     const std::string& value,
                     bool strict = false);

  /**
   * Get the cookie with the specified name.
   *
   * Returns a StringPiece to the cookie value, or an empty StringPiece if
   * there is no cookie with the specified name.  The returned cookie is
   * only valid as long as the Cookie Header in HTTPMessage object exists.
   * Applications should make sure they call unparseCookies() when editing
   * the Cookie Header, so that the StringPiece references are cleared.
   */
  const folly::StringPiece getCookie(const std::string& name) const;

  /**
   * Print the message out.
   */
  void dumpMessage(int verbosity) const;

  void describe(std::ostream& os) const;

  /**
   * Print the message out, serializes through mutex
   * Used in shutdown path
   */
  void atomicDumpMessage(int verbosity) const;

  /**
   * Print the message out to logSink.
   */
  void dumpMessageToSink(google::LogSink* logSink) const;

  /**
   * Interact with headers that are defined to be per-hop.
   *
   * It is expected that during request processing, stripPerHopHeaders() will
   * be called before the message is proxied to the other connection.
   */
  void stripPerHopHeaders(bool stripPriority = false,
                          const HTTPHeaders* customHeaders = nullptr);

  const HTTPHeaders& getStrippedPerHopHeaders() const {
    CHECK(strippedPerHopHeaders_) << "call stripPerHopHeaders first";
    return *strippedPerHopHeaders_;
  }

  void setSecure(bool secure) {
    if (secure && scheme_ != Scheme::MASQUE) {
      scheme_ = Scheme::HTTPS;
    } else if (!secure) {
      scheme_ = Scheme::HTTP;
    }
  }

  bool isSecure() const {
    return (scheme_ == Scheme::HTTPS || scheme_ == Scheme::MASQUE);
  }

  void setMasque() {
    scheme_ = Scheme::MASQUE;
  }

  bool isMasque() const {
    return scheme_ == Scheme::MASQUE;
  }

  const std::string& getScheme() const {
    switch (scheme_) {
      case HTTPMessage::Scheme::HTTP:
        return headers::kHttp;
      case HTTPMessage::Scheme::HTTPS:
        return headers::kHttps;
      case HTTPMessage::Scheme::MASQUE:
        return headers::kMasque;
    }
    return headers::kHttp;
  }

  int getSecureVersion() const {
    return sslVersion_;
  }

  const char* getSecureCipher() const {
    return sslCipher_;
  }

  void setSecureInfo(int ver, const char* cipher) {
    // cipher is a static const char* provided and managed by openssl lib
    sslVersion_ = ver;
    sslCipher_ = cipher;
  }

  void setAdvancedProtocolString(const std::string& protocol) {
    protoStr_ = &protocol;
  }

  bool isAdvancedProto() const {
    return protoStr_ != nullptr;
  }

  const std::string* getAdvancedProtocolString() const {
    return protoStr_;
  }

  /**
   * Return the protocol string used by this HTTPMessage.
   *
   * If this HTTP message is using an advanced protocol, the protocol string
   * will be the advanced protocol. If not, it will simply be the HTTP version.
   */
  const std::string& getProtocolString() const {
    if (isAdvancedProto()) {
      return *protoStr_;
    }

    return versionStr_;
  }

  /* Setter and getter for the SPDY priority value (0 - 7).  When serialized
   * to SPDY/2, Codecs will collpase 0,1 -> 0, 2,3 -> 1, etc.
   *
   * Negative values of pri are interpreted much like negative array
   * indexes in python, so -1 will be the largest numerical priority
   * value for this SPDY version (i.e. 3 for SPDY/2 or 7 for SPDY/3),
   * -2 the second largest (i.e. 2 for SPDY/2 or 6 for SPDY/3).
   */
  static uint8_t normalizePriority(int8_t pri) {
    if (pri > kMaxPriority || pri < -kMaxPriority) {
      // outside [-7, 7] => highest priority
      return kMaxPriority;
    } else if (pri < 0) {
      return pri + kMaxPriority + 1;
    }
    return pri;
  }

  void setPriority(int8_t pri) {
    pri_ = normalizePriority(pri);
    h2Pri_ = folly::none;
  }
  uint8_t getPriority() const {
    return pri_;
  }

  /**
   * Set a Priority header on the HTTPMessage by urgency and incremental.
   */
  void setHTTPPriority(uint8_t urgency, bool incremental);

  folly::Optional<HTTPPriority> getHTTPPriority() const noexcept {
    return httpPriorityFromHTTPMessage(*this);
  }

  /**
   * Set a Priority header on the HTTPMessage by httpPriority.
   *
   * There is no getter of HTTPPriority in a HTTPMessage.
   * Use httpPriorityFromHTTPMessage for that.
   */
  void setHTTPPriority(HTTPPriority httpPriority);

  using HTTP2Priority = std::tuple<uint64_t, bool, uint8_t>;

  folly::Optional<HTTP2Priority> getHTTP2Priority() const {
    return h2Pri_;
  }

  void setHTTP2Priority(const HTTP2Priority& h2Pri) {
    h2Pri_ = h2Pri;
  }

  /**
   * get and setter for transaction sequence number
   */
  void setSeqNo(int32_t seqNo) {
    seqNo_ = seqNo;
  }
  int32_t getSeqNo() const {
    return seqNo_;
  }

  /**
   * getter and setter for size in serialized form
   */
  void setIngressHeaderSize(const HTTPHeaderSize& size) {
    size_ = size;
  }
  const HTTPHeaderSize& getIngressHeaderSize() const {
    return size_;
  }

  /**
   * Getter and setter for the time when the first byte of the message arrived
   */
  TimePoint getStartTime() const {
    return startTime_;
  }
  void setStartTime(const TimePoint& startTime) {
    startTime_ = startTime;
  }

  /**
   * Check if a particular token value is present in a header that consists of
   * a list of comma separated tokens.  (e.g., a header with a #rule
   * body as specified in the RFC 2616 BNF notation.)
   */
  bool checkForHeaderToken(const HTTPHeaderCode headerCode,
                           char const* token,
                           bool caseSensitive) const;

  /**
   * Forget about the parsed cookies.
   *
   * Ideally HTTPMessage should automatically forget about the current parsed
   * cookie state whenever a Cookie header is changed.  However, at the moment
   * callers have to explicitly call unparseCookies() after modifying the
   * cookie headers.
   */
  void unparseCookies() const;

  /**
   * Get the default reason string for a status code.
   *
   * This returns the reason string for the specified status code as specified
   * in RFC 2616.  For unknown status codes, the string "-" is returned.
   */
  static const char* getDefaultReason(uint16_t status);

  /**
   * Computes whether the state of this message is compatible with keepalive.
   * Changing the headers, version, etc can change the result.
   */
  bool computeKeepalive() const;

  /**
   * @returns true if this HTTPMessage represents an HTTP request
   */
  bool isRequest() const {
    return fields_.which_ == MessageType::REQUEST;
  }

  /**
   * @returns true if this HTTPMessage represents an HTTP response
   */
  bool isResponse() const {
    return fields_.which_ == MessageType::RESPONSE;
  }

  /**
   * Assuming input contains
   * <name><valueDelim><value>(<pairDelim><name><valueDelim><value>),
   * invoke callback once with each name,value pair.
   */
  static void splitNameValuePieces(
      folly::StringPiece input,
      char pairDelim,
      char valueDelim,
      std::function<void(folly::StringPiece, folly::StringPiece)> callback);

  static void splitNameValue(
      folly::StringPiece input,
      char pairDelim,
      char valueDelim,
      std::function<void(std::string&&, std::string&&)> callback);

  /**
   * Form the URL from the individual components.
   * url -> {scheme}://{authority}{path}?{query}#{fragment}
   */
  static std::string createUrl(const folly::StringPiece scheme,
                               const folly::StringPiece authority,
                               const folly::StringPiece path,
                               const folly::StringPiece query,
                               const folly::StringPiece fragment);

  /**
   * Create a query string from the query parameters map
   * containing the name-value pairs.
   */
  static std::string createQueryString(
      const std::map<std::string, std::string>& params, uint32_t maxSize);

 protected:
  // Message start time, in msec since the epoch.
  TimePoint startTime_;

 private:
  void parseCookies() const;

  template <typename T> // T = string
  ParseURL setURLImpl(T&& url, bool unparse, bool strict) {
    DVLOG(9) << "setURL: " << std::forward<T>(url);

    // Set the URL, path, and query string parameters
    request().url_ = std::forward<T>(url);
    return setURLImplInternal(unparse, strict);
  }

  ParseURL setURLImplInternal(bool unparse, bool strict);

  bool setQueryStringImpl(const std::string& queryString,
                          bool unparse,
                          bool strict);
  void parseQueryParams() const;
  void unparseQueryParams();

  bool doHeaderTokenCheck(const HTTPHeaders& headers_,
                          const HTTPHeaderCode headerCode,
                          char const* token,
                          bool caseSensitive) const;

  /**
   * Trims whitespace from the beggining and end of the StringPiece.
   */
  static folly::StringPiece trim(folly::StringPiece sp);

  /** The 12 standard fields for HTTP messages. Use accessors.
   * An HTTPMessage is either a Request or Response.
   * Once an accessor for either is used, that fixes the type of HTTPMessage.
   * If an access is then used for the other type, a DCHECK will fail.
   */
  struct IPPort {
    std::string ip;
    std::string port;
    IPPort(std::string inIp, std::string inPort)
        : ip(std::move(inIp)), port(std::move(inPort)) {
    }
  };
  struct Request {
    folly::SocketAddress clientAddress_;
    mutable folly::Optional<IPPort> clientIPPort_;
    mutable boost::
        variant<boost::blank, std::unique_ptr<std::string>, HTTPMethod>
            method_;
    folly::StringPiece path_;
    folly::StringPiece query_;
    mutable std::unique_ptr<std::string> pathStr_;
    mutable std::unique_ptr<std::string> queryStr_;
    std::string url_;

    uint16_t pushStatus_;

    Request() = default;

    Request(const Request& req)
        : clientIPPort_(req.clientIPPort_),
          path_(req.path_),
          query_(req.query_),
          pathStr_(nullptr),
          queryStr_(nullptr),
          url_(req.url_),
          pushStatus_(req.pushStatus_) {
      if (req.method_.which() == 1) {
        method_ = std::make_unique<std::string>(
            *boost::get<std::unique_ptr<std::string>>(req.method_));
      } else if (req.method_.which() == 2) {
        method_ = boost::get<HTTPMethod>(req.method_);
      }
    }
  };

  struct Response {
    uint16_t status_;
    std::string statusStr_;
    std::string statusMsg_;
  };

  folly::SocketAddress dstAddress_;
  mutable std::string dstIP_;
  mutable std::string dstPort_;

  std::string localIP_;
  std::string versionStr_;

  enum class MessageType : uint8_t { NONE = 0, REQUEST = 1, RESPONSE = 2 };
  struct Fields {
    Fields() = default;
    Fields(const Fields& other) {
      copyFrom(other);
    }

    Fields& operator=(const Fields& other) {
      clear();
      copyFrom(other);
      return *this;
    }

    ~Fields() {
      clear();
    }

    void clear() {
      switch (which_) {
        case MessageType::REQUEST:
          data_.request.~Request();
          break;
        case MessageType::RESPONSE:
          data_.response.~Response();
          break;
        case MessageType::NONE:
          break;
      }
      which_ = MessageType::NONE;
    }

    void copyFrom(const Fields& other) {
      which_ = other.which_;
      switch (which_) {
        case MessageType::REQUEST:
          new (&data_.request) Request(other.data_.request);
          break;
        case MessageType::RESPONSE:
          new (&data_.response) Response(other.data_.response);
          break;
        case MessageType::NONE:
          break;
      }
    }

    Fields(Fields&& other) {
      moveFrom(std::move(other));
    }

    Fields& operator=(Fields&& other) {
      clear();
      moveFrom(std::move(other));
      return *this;
    }

    void moveFrom(Fields&& other) {
      which_ = other.which_;
      switch (which_) {
        case MessageType::REQUEST:
          new (&data_.request) Request(std::move(other.data_.request));
          break;
        case MessageType::RESPONSE:
          new (&data_.response) Response(std::move(other.data_.response));
          break;
        case MessageType::NONE:
          break;
      }
    }

    mutable MessageType which_{MessageType::NONE};
    mutable union Data {
      Data() {
      }
      ~Data() {
      }
      Request request;
      Response response;
    } data_;
  } fields_;

  // mutable boost::variant<boost::blank, Request, Response> fields_;

  Request& request() {
    DCHECK(fields_.which_ == MessageType::NONE ||
           fields_.which_ == MessageType::REQUEST)
        << int(fields_.which_);
    if (fields_.which_ == MessageType::NONE) {
      fields_.which_ = MessageType::REQUEST;
      new (&fields_.data_.request) Request();
    } else if (fields_.which_ == MessageType::RESPONSE) {
      throw std::runtime_error("Invoked Request API on HTTP Response");
    }

    return fields_.data_.request;
  }

  const Request& request() const {
    auto msg = const_cast<HTTPMessage*>(this);
    return msg->request();
  }

  Response& response() {
    DCHECK(fields_.which_ == MessageType::NONE ||
           fields_.which_ == MessageType::RESPONSE)
        << int(fields_.which_);
    if (fields_.which_ == MessageType::NONE) {
      fields_.which_ = MessageType::RESPONSE;
      new (&fields_.data_.response) Response();
    } else if (fields_.which_ == MessageType::REQUEST) {
      throw std::runtime_error("Invoked Response API on HTTP Request");
    }

    return fields_.data_.response;
  }

  const Response& response() const {
    auto msg = const_cast<HTTPMessage*>(this);
    return msg->response();
  }

  /*
   * Cookies and query parameters
   * These are mutable since we parse them lazily in getCookie() and
   * getQueryParam()
   */
  mutable std::map<folly::StringPiece, folly::StringPiece> cookies_;
  // TODO: use StringPiece for queryParams_ and delete splitNameValue()
  mutable std::map<std::string, std::string> queryParams_;

  HTTPHeaders headers_;
  std::unique_ptr<HTTPHeaders> strippedPerHopHeaders_;
  HTTPHeaderSize size_;
  WebSocketUpgrade upgradeWebsocket_;
  std::unique_ptr<HTTPHeaders> trailers_;

  int32_t seqNo_;
  int sslVersion_;
  const char* sslCipher_;
  const std::string* protoStr_;
  std::unique_ptr<std::string> upgradeProtocol_;
  uint8_t pri_;
  folly::Optional<HTTP2Priority> h2Pri_;

  std::pair<uint8_t, uint8_t> version_;
  mutable bool parsedCookies_ : 1;
  mutable bool parsedQueryParams_ : 1;
  bool chunked_ : 1;
  bool upgraded_ : 1;
  bool wantsKeepalive_ : 1;
  bool trailersAllowed_ : 1;

  Scheme scheme_{Scheme::HTTP};

  // used by atomicDumpMessage
  static std::mutex mutexDump_;
};

std::ostream& operator<<(std::ostream& os, const HTTPMessage& msg);

/**
 * Returns a std::string that has the control characters removed from the
 * input string.
 */
template <typename Str>
std::string stripCntrlChars(const Str& str) {
  std::string res;
  res.reserve(str.size());
  for (size_t i = 0; i < str.size(); ++i) {
    if (!(str[i] <= 0x1F || str[i] == 0x7F)) {
      res += str[i];
    }
  }
  return res;
}
} // namespace proxygen

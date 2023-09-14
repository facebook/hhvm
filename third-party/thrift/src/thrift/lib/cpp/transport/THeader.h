/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef THRIFT_TRANSPORT_THEADER_H_
#define THRIFT_TRANSPORT_THEADER_H_ 1

#include <functional>
#include <map>
#include <optional>
#include <string_view>
#include <vector>

#include <folly/Optional.h>
#include <folly/String.h>
#include <folly/Utility.h>
#include <folly/io/async/fdsock/SocketFds.h>
#include <folly/portability/Unistd.h>
#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <bitset>
#include <chrono>

// These are local to this build and never appear on the wire.
enum CLIENT_TYPE {
  THRIFT_HEADER_CLIENT_TYPE = 0,
  THRIFT_FRAMED_DEPRECATED = 1,
  THRIFT_UNFRAMED_DEPRECATED = 2,
  THRIFT_HTTP_SERVER_TYPE = 3,
  THRIFT_HTTP_CLIENT_TYPE = 4,
  THRIFT_FRAMED_COMPACT = 5,
  THRIFT_ROCKET_CLIENT_TYPE = 6,
  THRIFT_HTTP_GET_CLIENT_TYPE = 7,
  THRIFT_UNFRAMED_COMPACT_DEPRECATED = 8,
  THRIFT_HTTP2_CLIENT_TYPE = 9,
  // This MUST always be last and have the largest value!
  THRIFT_UNKNOWN_CLIENT_TYPE = 10,
};

#define CLIENT_TYPES_LEN THRIFT_UNKNOWN_CLIENT_TYPE

// These appear on the wire.
enum HEADER_FLAGS {
  HEADER_FLAG_SUPPORT_OUT_OF_ORDER = 0x01,
  // Set for reverse messages (server->client requests, client->server replies)
  HEADER_FLAG_DUPLEX_REVERSE = 0x08,
};

namespace folly {
class IOBuf;
class IOBufQueue;
} // namespace folly

namespace apache {
namespace thrift {
namespace util {
class THttpClientParser;
}
} // namespace thrift
} // namespace apache

namespace apache {
namespace thrift {
namespace transport {

namespace detail {
/**
 * This is a helper class to facilitate transport upgrade from header to rocket
 * for non-TLS services. The socket stored in header channel is a shared_ptr
 * while the socket in rocket is a unique_ptr. The goal is to transfer the
 * socket from header to rocket, by managing the lifetime using this custom
 * deleter which makes it possible for the unique_ptr stolen by stealPtr()
 * outlives the shared_ptr.
 */
template <typename T, typename Deleter>
class ReleaseDeleter {
 public:
  explicit ReleaseDeleter(std::unique_ptr<T, Deleter> uPtr)
      : ptr_(uPtr.release()), deleter_(uPtr.get_deleter()) {}

  void operator()(T* obj) {
    (void)obj;
    if (ptr_) {
      DCHECK(obj == ptr_);
      deleter_(ptr_);
    }
  }

  /**
   * Steal the unique_ptr stored in this deleter.
   */
  std::unique_ptr<T, Deleter> stealPtr() {
    DCHECK(ptr_);
    auto uPtr = std::unique_ptr<T, Deleter>(ptr_, deleter_);
    ptr_ = nullptr;
    return uPtr;
  }

 private:
  T* ptr_;
  Deleter deleter_;
};

template <typename T, typename Deleter>
std::shared_ptr<T> convertToShared(std::unique_ptr<T, Deleter> uPtr) {
  auto ptr = uPtr.get();
  auto deleter = ReleaseDeleter<T, Deleter>(std::move(uPtr));
  return std::shared_ptr<T>(ptr, deleter);
}
} // namespace detail

using apache::thrift::protocol::T_BINARY_PROTOCOL;
using apache::thrift::protocol::T_COMPACT_PROTOCOL;

/**
 * Class that will take an IOBuf and wrap it in some thrift headers.
 * see thrift/doc/HeaderFormat.txt for details.
 *
 * Supports transforms: zlib snappy zstd
 * Supports headers: http-style key/value per request and per connection
 * other: Protocol Id and seq ID in header.
 *
 * Backwards compatible with TFramed format, and unframed format, assuming
 * your server transport is compatible (some server types require 4-byte size
 * at the start).
 */
class THeader final {
 public:
  // Non-copiable, but movable. Reasons to avoid copies:
  //   - Normal request flows should never copy THeader, since they're
  //     highly performance-sensitive.
  //   - Additionally, any copy must take special care with thread-safety on
  //     the objects pointed to by all the copies (e.g.  `httpClientParser_`
  //     and `routingData_`.
  //   - When `fds` are **received** into THeader, we require unique ownership
  //     semantics -- otherwise, any confusion about which copy is allowed
  //     to close the FD can result in operations on invalid FDs, which
  //     can trivially cause data loss and worse due to FD reuse.
  // If your application requires you to copy THeaders that are being sent,
  // use `copyOrDfatalIfReceived` with due care.  That said, I've only seen
  // one legitimate use for copying THeader -- creating shadow requests.
  THeader(const THeader&) = delete;
  THeader operator=(const THeader&) = delete;
  THeader(THeader&&) = default;
  THeader& operator=(THeader&&) = default;

  // The docblock on constructors explains why copying "received" `THeaders`
  // is not allowed.  Since the same type is used for "send" and "receive",
  // we need a runtime check for when FDs are being received.  While it
  // would be more usable to disallow all "received" copies, there is no way
  // to distinguish sent/received THeaders unless they bear FDs.
  THeader copyOrDfatalIfReceived() const;

  enum {
    ALLOW_BIG_FRAMES = 1 << 0,
  };

  explicit THeader(int options = 0);

  void setClientType(CLIENT_TYPE ct) { c_.clientType_ = ct; }
  // Force using specified client type when using legacy client types
  // i.e. sniffing out client type is disabled.
  void forceClientType(bool enable) { c_.forceClientType_ = enable; }
  CLIENT_TYPE getClientType() const { return c_.clientType_; }

  uint16_t getProtocolId() const { return c_.protoId_; }
  void setProtocolId(uint16_t protoId) { c_.protoId_ = protoId; }

  int8_t getProtocolVersion() const;
  void setProtocolVersion(uint8_t ver) { c_.protoVersion_ = ver; }

  void resetProtocol();

  uint16_t getFlags() const { return c_.flags_; }
  void setFlags(uint16_t flags) { c_.flags_ = flags; }

  // Info headers
  // NB: we're using F14NodeMap here because it's a very≥ general map, which
  // sometimes requires reference stability guarantees.
  typedef folly::F14NodeMap<std::string, std::string> StringToStringMap;

  /**
   * We know we got a packet in header format here, try to parse the header
   *
   * @param IObuf of the header + data.  Untransforms the data as appropriate.
   * @return Just the data section in an IOBuf
   */
  std::unique_ptr<folly::IOBuf> readHeaderFormat(
      std::unique_ptr<folly::IOBuf>, StringToStringMap& persistentReadHeaders);

  /**
   * Untransform the data based on the received header flags
   * On conclusion of function, setReadBuffer is called with the
   * untransformed data.
   *
   * @param IOBuf input data section
   * @return IOBuf output data section
   */
  static std::unique_ptr<folly::IOBuf> untransform(
      std::unique_ptr<folly::IOBuf>, std::vector<uint16_t>& readTrans);

  /**
   * Transform the data based on our write transform flags
   * At conclusion of function the write buffer is set to the
   * transformed data.
   *
   * @param IOBuf to transform.  Returns transformed IOBuf (or chain)
   * @return transformed data IOBuf
   */
  static std::unique_ptr<folly::IOBuf> transform(
      std::unique_ptr<folly::IOBuf>,
      std::vector<uint16_t>& writeTrans,
      size_t minCompressBytes = 0);

  /**
   * Copy metadata, but not headers.
   */
  void copyMetadataFrom(const THeader& src);

  static uint16_t getNumTransforms(const std::vector<uint16_t>& transforms) {
    return folly::to_narrow(transforms.size());
  }

  void setTransform(uint16_t transId);
  void setReadTransform(uint16_t transId);
  void setTransforms(const std::vector<uint16_t>& trans) {
    c_.writeTrans_ = trans;
  }
  const std::vector<uint16_t>& getTransforms() const { return c_.readTrans_; }
  std::vector<uint16_t>& getWriteTransforms() { return c_.writeTrans_; }

  void setClientMetadata(const ClientMetadata& clientMetadata);
  std::optional<ClientMetadata> extractClientMetadata();

  // these work with write headers
  void setHeader(std::string_view key, const std::string& value);
  void setHeader(std::string_view key, std::string&& value);
  void setHeader(
      const char* key, size_t keyLength, const char* value, size_t valueLength);
  void setHeaders(StringToStringMap&&);
  void clearHeaders();
  bool isWriteHeadersEmpty() const;
  StringToStringMap& mutableWriteHeaders();
  StringToStringMap releaseWriteHeaders();
  StringToStringMap extractAllWriteHeaders();
  const StringToStringMap& getWriteHeaders() const;

  // these work with read headers
  void setReadHeaders(StringToStringMap&&);
  void setReadHeader(std::string_view key, std::string&& value);
  void eraseReadHeader(const std::string& key);
  const StringToStringMap& getHeaders() const;
  StringToStringMap releaseHeaders();

  void setExtraWriteHeaders(StringToStringMap* extraWriteHeaders) {
    c_.extraWriteHeaders_ = extraWriteHeaders;
  }
  StringToStringMap* getExtraWriteHeaders() const {
    return c_.extraWriteHeaders_;
  }

  std::string getPeerIdentity() const;
  void setIdentity(const std::string& identity);

  // accessors for seqId
  uint32_t getSequenceNumber() const { return c_.seqId_; }
  void setSequenceNumber(uint32_t sid) { c_.seqId_ = sid; }

  enum TRANSFORMS {
    NONE = 0x00,
    ZLIB_TRANSFORM = 0x01,
    // HMAC_TRANSFORM = 0x02, Deprecated and no longer supported
    // SNAPPY_TRANSFORM = 0x03, Deprecated and no longer supported
    // QLZ_TRANSFORM = 0x04, Deprecated and no longer supported
    ZSTD_TRANSFORM = 0x05,

    // DO NOT USE. Sentinel value for enum count. Always keep as last value.
    TRANSFORM_LAST_FIELD = 0x06,
  };

  /* IOBuf interface */

  /**
   * Adds the header based on the type of transport:
   * unframed - does nothing.
   * framed - prepends frame size
   * header - prepends header, optionally appends mac
   * http - only supported for sync case, prepends http header.
   *
   * @return IOBuf chain with header _and_ data.  Data is not copied
   */
  std::unique_ptr<folly::IOBuf> addHeader(
      std::unique_ptr<folly::IOBuf>, bool transform = true);
  /**
   * Given an IOBuf Chain, remove the header.  Supports unframed (sync
   * only), framed, header, and http (sync case only).  This doesn't
   * check if the client type implied by the header is valid.
   * isSupportedClient() or checkSupportedClient() should be called
   * after.
   *
   * @param IOBufQueue - queue to try to read message from.
   *
   * @param needed - if the return is nullptr (i.e. we didn't read a full
   *                 message), needed is set to the number of bytes needed
   *                 before you should call removeHeader again.
   *
   * @return IOBuf - the message chain.  May be shared, may be chained.
   *                 If nullptr, we didn't get enough data for a whole message,
   *                 call removeHeader again after reading needed more bytes.
   */
  std::unique_ptr<folly::IOBuf> removeHeader(
      folly::IOBufQueue*,
      size_t& needed,
      StringToStringMap& persistentReadHeaders);

  void setDesiredCompressionConfig(CompressionConfig compressionConfig) {
    c_.compressionConfig_ = compressionConfig;
  }

  folly::Optional<CompressionConfig> getDesiredCompressionConfig() const {
    return c_.compressionConfig_;
  }

  void setProxiedPayloadMetadata(
      ProxiedPayloadMetadata proxiedPayloadMetadata) {
    c_.proxiedPayloadMetadata_ = proxiedPayloadMetadata;
  }

  std::optional<ProxiedPayloadMetadata> extractProxiedPayloadMetadata() {
    return std::exchange(c_.proxiedPayloadMetadata_, {});
  }

  void setCrc32c(folly::Optional<uint32_t> crc32c) { c_.crc32c_ = crc32c; }

  folly::Optional<uint32_t> getCrc32c() const { return c_.crc32c_; }

  void setServerLoad(folly::Optional<int64_t> load) { c_.serverLoad_ = load; }

  folly::Optional<int64_t> getServerLoad() const { return c_.serverLoad_; }

  apache::thrift::concurrency::PRIORITY getCallPriority() const;

  std::chrono::milliseconds getTimeoutFromHeader(std::string_view header) const;

  std::chrono::milliseconds getClientTimeout() const;

  std::chrono::milliseconds getClientQueueTimeout() const;

  // Overall queue timeout (either set by client or server override)
  // This is set on the server side in responses.
  folly::Optional<std::chrono::milliseconds> getServerQueueTimeout() const;

  // This is populated by the server and reflects the time the request spent
  // in the queue prior to processing.
  folly::Optional<std::chrono::milliseconds> getProcessDelay() const;

  const folly::Optional<std::string>& clientId() const;
  const folly::Optional<std::string>& serviceTraceMeta() const;

  void setHttpClientParser(
      std::shared_ptr<apache::thrift::util::THttpClientParser>);

  void setClientTimeout(std::chrono::milliseconds timeout);
  void setClientQueueTimeout(std::chrono::milliseconds timeout);
  void setServerQueueTimeout(std::chrono::milliseconds timeout);
  void setProcessDelay(std::chrono::milliseconds timeQueued);
  void setCallPriority(apache::thrift::concurrency::PRIORITY priority);
  void setClientId(const std::string& clientId);
  void setServiceTraceMeta(const std::string& serviceTraceMeta);

  // Utility method for converting TRANSFORMS enum to string
  static const folly::StringPiece getStringTransform(
      const TRANSFORMS transform);

  static CLIENT_TYPE tryGetClientType(const folly::IOBuf& data);

  void setRoutingData(std::shared_ptr<void> data) {
    c_.routingData_ = std::move(data);
  }
  std::shared_ptr<void> releaseRoutingData() {
    return std::move(c_.routingData_);
  }

  const std::optional<LoggingContext>& loggingContext() const {
    return c_.loggingContext_;
  }
  std::optional<LoggingContext>& loggingContext() { return c_.loggingContext_; }

  folly::SocketFds fds; // No accessor, since this type **is** the API.

  // 0 and 16th bits must be 0 to differentiate from framed & unframed
  static const uint32_t HEADER_MAGIC = 0x0FFF0000;
  static const uint32_t HEADER_MASK = 0xFFFF0000;
  static const uint32_t FLAGS_MASK = 0x0000FFFF;
  static const uint32_t HTTP_SERVER_MAGIC = 0x504F5354; // POST
  static const uint32_t HTTP_CLIENT_MAGIC = 0x48545450; // HTTP
  static const uint32_t HTTP_GET_CLIENT_MAGIC = 0x47455420; // GET
  static const uint32_t HTTP_HEAD_CLIENT_MAGIC = 0x48454144; // HEAD
  static const uint32_t BIG_FRAME_MAGIC = 0x42494746; // BIGF

  static const uint32_t MAX_FRAME_SIZE = 0x3FFFFFFF;
  static constexpr std::string_view PRIORITY_HEADER = "thrift_priority";
  static constexpr std::string_view CLIENT_TIMEOUT_HEADER = "client_timeout";
  static constexpr std::string_view QUEUE_TIMEOUT_HEADER = "queue_timeout";
  static constexpr std::string_view QUERY_LOAD_HEADER = "load";
  static constexpr std::string_view kClientId = "client_id";
  static constexpr std::string_view kServiceTraceMeta = "service_trace_meta";
  static constexpr std::string_view CLIENT_METADATA_HEADER = "client_metadata";

 private:
  static bool isFramed(CLIENT_TYPE clientType);

  // Use first 64 bits to determine client protocol
  static folly::Optional<CLIENT_TYPE> analyzeFirst32bit(uint32_t w);
  static CLIENT_TYPE analyzeSecond32bit(uint32_t w);

  // Calls appropriate method based on client type
  // returns nullptr if Header of Unknown type
  std::unique_ptr<folly::IOBuf> removeNonHeader(
      folly::IOBufQueue* queue,
      size_t& needed,
      CLIENT_TYPE clientType,
      uint32_t sz);

  template <
      template <class BaseProt>
      class ProtocolClass,
      protocol::PROTOCOL_TYPES ProtocolID>
  std::unique_ptr<folly::IOBuf> removeUnframed(
      folly::IOBufQueue* queue, size_t& needed);
  std::unique_ptr<folly::IOBuf> removeHttpServer(folly::IOBufQueue* queue);
  std::unique_ptr<folly::IOBuf> removeHttpClient(
      folly::IOBufQueue* queue, size_t& needed);
  std::unique_ptr<folly::IOBuf> removeFramed(
      uint32_t sz, folly::IOBufQueue* queue);

  /**
   * Returns the maximum number of bytes that write k/v headers can take
   */
  size_t getMaxWriteHeadersSize() const;

  /**
   * Returns whether the 1st byte of the protocol payload should be hadled
   * as compact framed.
   */
  static bool compactFramed(uint32_t magic);

  std::optional<std::string> extractHeader(std::string_view key);
  StringToStringMap& ensureReadHeaders();
  StringToStringMap& ensureWriteHeaders();

  static constexpr std::string_view IDENTITY_HEADER = "identity";
  static constexpr std::string_view ID_VERSION_HEADER = "id_version";
  static constexpr std::string_view ID_VERSION = "1";

  // IMPORTANT: Anything not in this sub-struct must be copied explicitly by
  // `copyOrDfatalIfReceived`.
  struct TriviallyCopiable {
    explicit TriviallyCopiable(int options);

    // Http client parser
    std::shared_ptr<apache::thrift::util::THttpClientParser> httpClientParser_;

    int16_t protoId_;
    int8_t protoVersion_;
    CLIENT_TYPE clientType_;
    bool forceClientType_;
    uint32_t seqId_;
    uint16_t flags_;
    std::string identity_;

    std::vector<uint16_t> readTrans_;
    std::vector<uint16_t> writeTrans_;

    // Map to use for headers
    std::optional<StringToStringMap> readHeaders_;
    std::optional<StringToStringMap> writeHeaders_;

    // Won't be cleared when flushing
    StringToStringMap* extraWriteHeaders_{nullptr};

    // If these values are set, they are used instead of looking inside
    // the header map.
    folly::Optional<std::chrono::milliseconds> clientTimeout_;
    folly::Optional<std::chrono::milliseconds> queueTimeout_;
    folly::Optional<std::chrono::milliseconds> processDelay_;
    folly::Optional<std::chrono::milliseconds> serverQueueTimeout_;
    folly::Optional<apache::thrift::concurrency::PRIORITY> priority_;
    folly::Optional<std::string> clientId_;
    folly::Optional<std::string> serviceTraceMeta_;

    bool allowBigFrames_;
    folly::Optional<CompressionConfig> compressionConfig_;

    std::shared_ptr<void> routingData_;

    // CRC32C of message payload for checksum.
    folly::Optional<uint32_t> crc32c_;
    folly::Optional<int64_t> serverLoad_;

    std::optional<ProxiedPayloadMetadata> proxiedPayloadMetadata_;

    // We make this field optional to differentiate SR calls v.s raw thrift
    // calls. This field will always be set by SR.
    std::optional<LoggingContext> loggingContext_;
  };

  // Supports `copyOrDfatalIfReceived`.
  explicit THeader(TriviallyCopiable c, folly::SocketFds f)
      : fds(std::move(f)), c_(c) {}

  TriviallyCopiable c_;
};

} // namespace transport
} // namespace thrift
} // namespace apache

#endif // #ifndef THRIFT_TRANSPORT_THEADER_H_

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

#include <thrift/lib/cpp/transport/THeader.h>

#include <fmt/core.h>
#include <folly/Conv.h>
#include <folly/ExceptionString.h>
#include <folly/MapUtil.h>
#include <folly/String.h>
#include <folly/compression/Compression.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/lang/Bits.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp/protocol/TBinaryProtocol.h>
#include <thrift/lib/cpp/protocol/TCompactProtocol.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp/transport/TBufferTransports.h>
#include <thrift/lib/cpp/util/THttpParser.h>
#include <thrift/lib/cpp/util/VarintUtils.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Util.h>

#include <algorithm>
#include <cassert>
#include <string>

using std::map;
using std::pair;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

namespace apache {
namespace thrift {
namespace transport {
namespace {
const THeader::StringToStringMap& kEmptyMap() {
  static const THeader::StringToStringMap& map =
      *(new transport::THeader::StringToStringMap);
  return map;
}

struct infoIdType {
  enum idType {
    // start at 1 to avoid confusing header padding for an infoId
    KEYVALUE = 1,
    // for persistent header
    PKEYVALUE = 2,
    END // signal the end of infoIds we can handle
  };
};
} // namespace

using namespace apache::thrift::protocol;
using namespace apache::thrift::util;
using namespace folly;
using namespace folly::io;
using apache::thrift::protocol::TBinaryProtocol;

std::string getReadableChars(Cursor c, size_t limit) {
  size_t size = 0;
  return c.readWhile([&size, limit](char b) {
    const auto isPrintable = 0x20 <= b && b < 0x7F;
    size++;
    return size <= limit && isPrintable;
  });
}

THeader::THeader(int options) : c_(options) {}

THeader::THeader::TriviallyCopiable::TriviallyCopiable(int options)
    : protoId_(T_COMPACT_PROTOCOL),
      protoVersion_(-1),
      clientType_(THRIFT_HEADER_CLIENT_TYPE),
      forceClientType_(false),
      seqId_(0),
      flags_(0),
      allowBigFrames_(options & ALLOW_BIG_FRAMES) {}

THeader THeader::copyOrDfatalIfReceived() const {
  SocketFds newFds;
  newFds.cloneToSendFromOrDfatal(fds);
  return THeader(c_, std::move(newFds));
}

int8_t THeader::getProtocolVersion() const {
  return c_.protoVersion_;
}

bool THeader::compactFramed(uint32_t magic) {
  int8_t protocolId = (magic >> 24);
  int8_t protocolVersion =
      (magic >> 16) & (uint32_t)TCompactProtocol::VERSION_MASK;
  return (
      (protocolId == TCompactProtocol::PROTOCOL_ID) &&
      (protocolVersion <= TCompactProtocol::VERSION_N) &&
      (protocolVersion >= TCompactProtocol::VERSION_LOW));
}

template <
    template <class BaseProt>
    class ProtocolClass,
    PROTOCOL_TYPES ProtocolID>
unique_ptr<IOBuf> THeader::removeUnframed(IOBufQueue* queue, size_t& needed) {
  auto buf = queue->move();
  auto range = buf->coalesce();
  queue->append(std::move(buf));

  // Test skip using the protocol to detect the end of the message
  TMemoryBuffer memBuffer(
      const_cast<uint8_t*>(range.begin()),
      range.size(),
      TMemoryBuffer::OBSERVE);
  c_.protoId_ = ProtocolID;
  ProtocolClass<TBufferBase> proto(&memBuffer);
  uint32_t msgSize = 0;
  try {
    std::string name;
    protocol::TMessageType messageType;
    int32_t seqid;
    msgSize += proto.readMessageBegin(name, messageType, seqid);
    msgSize += protocol::skip(proto, protocol::T_STRUCT);
    msgSize += proto.readMessageEnd();
  } catch (const TTransportException& ex) {
    if (ex.getType() == TTransportException::END_OF_FILE) {
      // We don't have the full data yet.  We can't tell exactly
      // how many bytes we need, but it is at least one.
      needed = 1;
      return nullptr;
    }
  }

  return queue->split(msgSize);
}

unique_ptr<IOBuf> THeader::removeHttpServer(IOBufQueue* queue) {
  c_.protoId_ = T_BINARY_PROTOCOL;
  // Users must explicitly support this.
  return queue->move();
}

unique_ptr<IOBuf> THeader::removeHttpClient(IOBufQueue* queue, size_t& needed) {
  c_.protoId_ = T_BINARY_PROTOCOL;
  TMemoryBuffer memBuffer;
  THttpClientParser parser;
  parser.setDataBuffer(&memBuffer);
  const IOBuf* headBuf = queue->front();
  const IOBuf* nextBuf = headBuf;
  uint32_t bytesParsed = 0;
  do {
    auto remainingDataLen = nextBuf->length();
    size_t offset = 0;
    auto ioBufData = nextBuf->data();
    do {
      void* parserBuf;
      size_t parserBufLen;
      parser.getReadBuffer(&parserBuf, &parserBufLen);
      size_t toCopyLen = std::min(parserBufLen, size_t(remainingDataLen));
      memcpy(parserBuf, ioBufData + offset, toCopyLen);
      bytesParsed += toCopyLen;
      if (parser.readDataAvailable(toCopyLen)) {
        queue->trimStart(bytesParsed - parser.getUnparsedDataLen());
        c_.readHeaders_ = parser.moveReadHeaders();
        return memBuffer.cloneBufferAsIOBuf();
      }
      remainingDataLen -= toCopyLen;
      offset += toCopyLen;
    } while (remainingDataLen > 0);
    nextBuf = nextBuf->next();
  } while (nextBuf != headBuf);
  // We don't have full data yet and we don't know how many bytes we need,
  // but it is at least 1.
  needed = parser.getMinBytesRequired();
  return nullptr;
}

unique_ptr<IOBuf> THeader::removeFramed(uint32_t sz, IOBufQueue* queue) {
  // Trim off the frame size.
  queue->trimStart(4);
  return queue->split(sz);
}

folly::Optional<CLIENT_TYPE> THeader::analyzeFirst32bit(uint32_t w) {
  if ((w & TBinaryProtocol::VERSION_MASK) ==
      static_cast<uint32_t>(TBinaryProtocol::VERSION_1)) {
    return THRIFT_UNFRAMED_DEPRECATED;
  } else if (compactFramed(w)) {
    return THRIFT_UNFRAMED_COMPACT_DEPRECATED;
  } else if (
      w == HTTP_SERVER_MAGIC || w == HTTP_GET_CLIENT_MAGIC ||
      w == HTTP_HEAD_CLIENT_MAGIC) {
    return THRIFT_HTTP_SERVER_TYPE;
  } else if (w == HTTP_CLIENT_MAGIC) {
    return THRIFT_HTTP_CLIENT_TYPE;
  }
  return folly::none;
}

CLIENT_TYPE THeader::analyzeSecond32bit(uint32_t w) {
  if ((w & TBinaryProtocol::VERSION_MASK) ==
      static_cast<uint32_t>(TBinaryProtocol::VERSION_1)) {
    return THRIFT_FRAMED_DEPRECATED;
  }
  if (compactFramed(w)) {
    return THRIFT_FRAMED_COMPACT;
  }
  if ((w & HEADER_MASK) == HEADER_MAGIC) {
    return THRIFT_HEADER_CLIENT_TYPE;
  }
  return THRIFT_UNKNOWN_CLIENT_TYPE;
}

CLIENT_TYPE THeader::tryGetClientType(const folly::IOBuf& data) {
  bool isRocket = apache::thrift::rocket::isMaybeRocketFrame(data);

  folly::io::Cursor cursor(&data);
  uint32_t word;
  if (cursor.tryReadBE(word)) {
    auto res = analyzeFirst32bit(word);
    if (res) {
      if (isRocket) {
        if (*res == CLIENT_TYPE::THRIFT_HTTP_CLIENT_TYPE ||
            *res == CLIENT_TYPE::THRIFT_HTTP_SERVER_TYPE) {
          return CLIENT_TYPE::THRIFT_UNKNOWN_CLIENT_TYPE;
        }
        return CLIENT_TYPE::THRIFT_ROCKET_CLIENT_TYPE;
      }
      return *res;
    }
  }
  if (cursor.tryReadBE(word)) {
    auto res = analyzeSecond32bit(word);
    if (isRocket) {
      if (res == CLIENT_TYPE::THRIFT_HEADER_CLIENT_TYPE) {
        return CLIENT_TYPE::THRIFT_UNKNOWN_CLIENT_TYPE;
      }
      return CLIENT_TYPE::THRIFT_ROCKET_CLIENT_TYPE;
    }
    return res;
  }

  return CLIENT_TYPE::THRIFT_UNKNOWN_CLIENT_TYPE;
}

bool THeader::isFramed(CLIENT_TYPE type) {
  switch (type) {
    case THRIFT_FRAMED_DEPRECATED:
    case THRIFT_FRAMED_COMPACT:
      return true;
    default:
      return false;
  };
}

unique_ptr<IOBuf> THeader::removeNonHeader(
    IOBufQueue* queue, size_t& needed, CLIENT_TYPE type, uint32_t sz) {
  switch (type) {
    case THRIFT_FRAMED_DEPRECATED:
      c_.protoId_ = T_BINARY_PROTOCOL;
      return removeFramed(sz, queue);
    case THRIFT_FRAMED_COMPACT:
      c_.protoId_ = T_COMPACT_PROTOCOL;
      return removeFramed(sz, queue);
    case THRIFT_UNFRAMED_DEPRECATED:
      return removeUnframed<TBinaryProtocolT, T_BINARY_PROTOCOL>(queue, needed);
    case THRIFT_HTTP_SERVER_TYPE:
      return removeHttpServer(queue);
    case THRIFT_HTTP_CLIENT_TYPE:
      return removeHttpClient(queue, needed);
    case THRIFT_UNFRAMED_COMPACT_DEPRECATED:
      return removeUnframed<TCompactProtocolT, T_COMPACT_PROTOCOL>(
          queue, needed);
    default:
      // Fallback to sniffing out the magic for Header
      return nullptr;
  };
}

unique_ptr<IOBuf> THeader::removeHeader(
    IOBufQueue* queue,
    size_t& needed,
    StringToStringMap& persistentReadHeaders) {
  if (!queue || queue->empty()) {
    needed = 4;
    return nullptr;
  }
  Cursor c(queue->front());
  size_t remaining;
  if (queue->options().cacheChainLength) {
    remaining = queue->chainLength();
  } else {
    remaining = queue->front()->computeChainDataLength();
  }
  size_t frameSizeBytes = 4;
  needed = 0;

  if (remaining < 4) {
    needed = 4 - remaining;
    return nullptr;
  }

  // Use first word to check type.
  uint32_t sz32 = c.readBE<uint32_t>();
  remaining -= 4;

  if (c_.forceClientType_) {
    // Make sure we have read the whole frame in.
    if (isFramed(c_.clientType_) && (sz32 > remaining)) {
      needed = sz32 - remaining;
      return nullptr;
    }
    unique_ptr<IOBuf> buf =
        THeader::removeNonHeader(queue, needed, c_.clientType_, sz32);
    if (buf) {
      return buf;
    }
  }

  auto clientT = THeader::analyzeFirst32bit(sz32);
  if (clientT) {
    c_.clientType_ = *clientT;
    return THeader::removeNonHeader(queue, needed, *clientT, sz32);
  }

  size_t sz;
  if (sz32 > MAX_FRAME_SIZE) {
    if (sz32 == BIG_FRAME_MAGIC) {
      if (!c_.allowBigFrames_) {
        throw TTransportException(
            TTransportException::INVALID_FRAME_SIZE, "Big frames not allowed");
      }
      if (8 > remaining) {
        needed = 8 - remaining;
        return nullptr;
      }
      sz = c.readBE<uint64_t>();
      remaining -= 8;
      frameSizeBytes += 8;
    } else if (
        sz32 == *reinterpret_cast<const uint32_t*>("coll") ||
        sz32 == *reinterpret_cast<const uint32_t*>("H pa")) {
      // special case for the most common question in user-group
      // this will probably saves hours of engineering effort.
      c.retreat(4);
      std::string err = "The Thrift server received an ASCII request '" +
          getReadableChars(c, 32) +
          "' and safely ignored it. "
          "In all likelihood, this isn't the reason of your problem "
          "(probably a local daemon sending HTTP content to all listening ports).";
      throw TTransportException(TTransportException::INVALID_FRAME_SIZE, err);
    } else {
      std::string err = folly::stringPrintf(
          "Header transport frame is too large: %u (hex 0x%08x", sz32, sz32);
      // does it look like ascii?
      if ( // all bytes 0x7f or less; all are > 0x1F
          ((sz32 & 0x7f7f7f7f) == sz32) && ((sz32 >> 24) & 0xff) >= 0x20 &&
          ((sz32 >> 16) & 0xff) >= 0x20 && ((sz32 >> 8) & 0xff) >= 0x20 &&
          ((sz32 >> 0) & 0xff) >= 0x20) {
        char buffer[5];
        uint32_t* asUint32 = reinterpret_cast<uint32_t*>(buffer);
        *asUint32 = folly::Endian::big(sz32);
        buffer[4] = 0;
        folly::stringAppendf(&err, ", ascii '%s'", buffer);
      }
      folly::stringAppendf(&err, ")");
      throw TTransportException(TTransportException::INVALID_FRAME_SIZE, err);
    }
  } else {
    sz = sz32;
  }

  // Make sure we have read the whole frame in.
  if (sz > remaining) {
    needed = sz - remaining;
    return nullptr;
  }

  // Could be header format or framed. Check next uint32
  uint32_t magic = c.readBE<uint32_t>();
  c_.clientType_ = analyzeSecond32bit(magic);
  unique_ptr<IOBuf> buf =
      THeader::removeNonHeader(queue, needed, c_.clientType_, sz);
  if (buf) {
    return buf;
  }

  if (c_.clientType_ == THRIFT_UNKNOWN_CLIENT_TYPE) {
    throw TTransportException(
        TTransportException::BAD_ARGS,
        folly::stringPrintf(
            "Could not detect client transport type: magic 0x%08x", magic));
  }

  if (sz < 10) {
    throw TTransportException(
        TTransportException::INVALID_FRAME_SIZE,
        folly::stringPrintf("Header transport frame is too small: %zu", sz));
  }
  c_.flags_ = magic & FLAGS_MASK;
  c_.seqId_ = c.readBE<uint32_t>();

  // Trim off the frame size.
  queue->trimStart(frameSizeBytes);
  buf = readHeaderFormat(queue->split(sz), persistentReadHeaders);

  return buf;
}

static string getString(RWPrivateCursor& c, size_t sz) {
  if (!c.canAdvance(sz)) {
    throw TTransportException(
        TTransportException::CORRUPTED_DATA,
        folly::stringPrintf(
            "String size %zu is larger than available %zu bytes",
            sz,
            c.totalLength()));
  }
  string str(sz, '\0');
  c.pull(&str[0], sz);
  return str;
}

/**
 * Reads a string from ptr, taking care not to reach headerBoundary
 * Advances ptr on success
 *
 * @param RWPrivateCursor        cursor to read from
 */
static string readString(RWPrivateCursor& c) {
  return getString(c, readVarint<uint32_t>(c));
}

static void readInfoHeaders(
    RWPrivateCursor& c, THeader::StringToStringMap& headers_) {
  // Process key-value headers
  uint32_t numKVHeaders = readVarint<int32_t>(c);
  // continue until we reach (paded) end of packet
  while (numKVHeaders--) {
    // format: key; value
    // both: length (varint32); value (string)
    string key = readString(c);
    string value = readString(c);
    // save to headers
    headers_[key] = value;
  }
}

unique_ptr<IOBuf> THeader::readHeaderFormat(
    unique_ptr<IOBuf> buf, StringToStringMap& persistentReadHeaders) {
  c_.readTrans_.clear(); // Clear out any previous transforms.
  c_.readHeaders_.reset(); // Clear out any previous headers.

  // magic(4), seqId(2), flags(2), headerSize(2)
  const uint8_t commonHeaderSize = 10;

  RWPrivateCursor c(buf.get());

  // skip over already processed magic(4), seqId(4), headerSize(2)
  c += commonHeaderSize - 2; // advance to headerSize field
  // On the wire, headerSize is in 4 byte words.  See HeaderFormat.txt
  uint32_t headerSize = 4 * c.readBE<uint16_t>() + commonHeaderSize;
  if (headerSize > buf->computeChainDataLength()) {
    throw TTransportException(
        TTransportException::INVALID_FRAME_SIZE,
        "Header size is larger than frame");
  }
  Cursor data(buf.get());
  data += headerSize;
  c_.protoId_ = readVarint<uint16_t>(c);
  uint16_t numTransforms = readVarint<uint16_t>(c);
  c_.readTrans_.reserve(numTransforms);

  uint16_t macSz = 0;

  // For now all transforms consist of only the ID, not data.
  for (int i = 0; i < numTransforms; i++) {
    int32_t transId = readVarint<int32_t>(c);
    c_.readTrans_.push_back(transId);
    setTransform(transId);
  }

  // Info headers
  while (data.data() != c.data()) {
    uint32_t infoId = readVarint<int32_t>(c);

    if (infoId == 0) {
      // header padding
      break;
    }
    if (infoId >= infoIdType::END) {
      // cannot handle infoId
      break;
    }
    switch (infoId) {
      case infoIdType::KEYVALUE:
        readInfoHeaders(c, ensureReadHeaders());
        break;
      case infoIdType::PKEYVALUE:
        readInfoHeaders(c, persistentReadHeaders);
        break;
    }
  }

  // if persistent headers are not empty, merge together.
  if (!persistentReadHeaders.empty()) {
    ensureReadHeaders().insert(
        persistentReadHeaders.begin(), persistentReadHeaders.end());
  }

  // Get just the data section using trim on a queue
  unique_ptr<IOBufQueue> msg(new IOBufQueue);
  msg->append(std::move(buf));
  msg->trimStart(headerSize);
  msg->trimEnd(macSz);

  buf = msg->move();
  // msg->move() can return an empty pointer if all the data is
  // trimmed out.  Turn it back into an empty buf.
  if (!buf) {
    buf = IOBuf::create(0);
  }

  // Untransform data section
  buf = untransform(std::move(buf), c_.readTrans_);

  if (c_.protoId_ == T_JSON_PROTOCOL &&
      c_.clientType_ != THRIFT_HTTP_SERVER_TYPE) {
    throw TApplicationException(
        TApplicationException::UNSUPPORTED_CLIENT_TYPE,
        "Client is trying to send JSON without HTTP");
  }

  return buf;
}

static unique_ptr<IOBuf> decompressCodec(
    const IOBuf& buf, folly::io::CodecType codec) {
  try {
    return folly::io::getCodec(codec)->uncompress(&buf);
  } catch (const std::exception& e) {
    throw TApplicationException(
        TApplicationException::MISSING_RESULT,
        folly::exceptionStr(e).toStdString());
  }
}

unique_ptr<IOBuf> THeader::untransform(
    unique_ptr<IOBuf> buf, std::vector<uint16_t>& readTrans) {
  for (vector<uint16_t>::const_reverse_iterator it = readTrans.rbegin();
       it != readTrans.rend();
       ++it) {
    using folly::io::CodecType;
    const uint16_t transId = *it;

    switch (transId) {
      case ZLIB_TRANSFORM:
        buf = decompressCodec(*buf, CodecType::ZLIB);
        break;
      case ZSTD_TRANSFORM:
        buf = decompressCodec(*buf, CodecType::ZSTD);
        break;
      default:
        throw TApplicationException(
            TApplicationException::MISSING_RESULT,
            fmt::format("Unknown transform: {}", transId));
    }
  }

  return buf;
}

static unique_ptr<IOBuf> compressCodec(
    const IOBuf& buf,
    folly::io::CodecType codec,
    int level = folly::io::COMPRESSION_LEVEL_DEFAULT) {
  try {
    return folly::io::getCodec(codec, level)->compress(&buf);
  } catch (const std::exception& e) {
    throw TTransportException(
        TTransportException::CORRUPTED_DATA,
        folly::exceptionStr(e).toStdString());
  }
}

unique_ptr<IOBuf> THeader::transform(
    unique_ptr<IOBuf> buf,
    std::vector<uint16_t>& writeTrans,
    size_t minCompressBytes) {
  size_t dataSize = buf->computeChainDataLength();

  for (vector<uint16_t>::iterator it = writeTrans.begin();
       it != writeTrans.end();) {
    using folly::io::CodecType;
    const uint16_t transId = *it;

    switch (transId) {
      case ZLIB_TRANSFORM:
        if (dataSize < minCompressBytes) {
          it = writeTrans.erase(it);
          continue;
        }
        buf = compressCodec(*buf, CodecType::ZLIB);
        break;
      case ZSTD_TRANSFORM:
        if (dataSize < minCompressBytes) {
          it = writeTrans.erase(it);
          continue;
        }
        buf = compressCodec(*buf, CodecType::ZSTD, 1);
        break;
      default:
        throw TTransportException(
            TTransportException::CORRUPTED_DATA,
            fmt::format("Unknown transform: {}", transId));
    }
    ++it;
  }

  return buf;
}

void THeader::setTransform(uint16_t transId) {
  for (auto& trans : c_.writeTrans_) {
    if (trans == transId) {
      return;
    }
  }
  c_.writeTrans_.push_back(transId);
}

void THeader::setReadTransform(uint16_t transId) {
  for (auto& trans : c_.readTrans_) {
    if (trans == transId) {
      return;
    }
  }
  c_.readTrans_.push_back(transId);
}

void THeader::copyMetadataFrom(const THeader& src) {
  setProtocolId(src.c_.protoId_);
  setTransforms(src.c_.writeTrans_);
  setSequenceNumber(src.c_.seqId_);
  setClientType(src.c_.clientType_);
  setFlags(src.c_.flags_);
  forceClientType(src.c_.forceClientType_);
}

void THeader::resetProtocol() {
  // Set to anything except HTTP type so we don't flush again
  c_.clientType_ = THRIFT_HEADER_CLIENT_TYPE;
}

/**
 * Writes a string to a byte buffer, as size (varint32) + string (non-null
 * terminated)
 * Automatically advances ptr to after the written portion
 */
static void writeString(uint8_t*& ptr, const string& str) {
  DCHECK_LT(str.length(), std::numeric_limits<uint32_t>::max());
  uint32_t strLen = str.length();
  ptr += writeVarint32(strLen, ptr);
  memcpy(ptr, str.c_str(), strLen); // no need to write \0
  ptr += strLen;
}

/**
 * Writes headers to a byte buffer and clear the header map
 */
static void flushInfoHeaders(
    uint8_t*& pkt,
    THeader::StringToStringMap& headers,
    uint32_t infoIdType,
    bool clearAfterFlush = true) {
  uint32_t headerCount = headers.size();
  if (headerCount > 0) {
    pkt += writeVarint32(infoIdType, pkt);
    // Write key-value headers count
    pkt += writeVarint32(headerCount, pkt);
    // Write info headers
    for (auto it = headers.begin(); it != headers.end(); ++it) {
      writeString(pkt, it->first); // key
      writeString(pkt, it->second); // value
    }
    if (clearAfterFlush) {
      headers.clear();
    }
  }
}

void THeader::setHeader(std::string_view key, const std::string& value) {
  ensureWriteHeaders()[key] = value;
}

void THeader::setHeader(std::string_view key, std::string&& value) {
  ensureWriteHeaders()[key] = std::move(value);
}

void THeader::setHeader(
    const char* key, size_t keyLength, const char* value, size_t valueLength) {
  ensureWriteHeaders().emplace(std::make_pair(
      std::string(key, keyLength), std::string(value, valueLength)));
}

void THeader::setHeaders(THeader::StringToStringMap&& headers) {
  c_.writeHeaders_ = std::move(headers);
}

void THeader::setReadHeaders(THeader::StringToStringMap&& headers) {
  c_.readHeaders_ = std::move(headers);
}

void THeader::eraseReadHeader(const std::string& key) {
  if (c_.readHeaders_) {
    c_.readHeaders_->erase(key);
  }
}

static size_t getInfoHeaderSize(const THeader::StringToStringMap& headers) {
  if (headers.empty()) {
    return 0;
  }
  size_t maxWriteHeadersSize = 5 + 5; // type and count (2 varints32)
  for (const auto& it : headers) {
    // add sizes of key and value to maxWriteHeadersSize
    // 2 varints32 + the strings themselves
    maxWriteHeadersSize += 5 + 5 + (it.first).length() + (it.second).length();
  }
  return maxWriteHeadersSize;
}

size_t THeader::getMaxWriteHeadersSize() const {
  size_t maxWriteHeadersSize = 0;
  maxWriteHeadersSize +=
      c_.writeHeaders_ ? getInfoHeaderSize(*c_.writeHeaders_) : 0;
  if (c_.extraWriteHeaders_) {
    maxWriteHeadersSize += getInfoHeaderSize(*c_.extraWriteHeaders_);
  }
  return maxWriteHeadersSize;
}

void THeader::clearHeaders() {
  c_.writeHeaders_.reset();
}

THeader::StringToStringMap& THeader::ensureReadHeaders() {
  if (!c_.readHeaders_) {
    c_.readHeaders_.emplace();
  }
  return *c_.readHeaders_;
}
THeader::StringToStringMap& THeader::ensureWriteHeaders() {
  if (!c_.writeHeaders_) {
    c_.writeHeaders_.emplace();
  }
  return *c_.writeHeaders_;
}

bool THeader::isWriteHeadersEmpty() const {
  return !c_.writeHeaders_ || c_.writeHeaders_->empty();
}

THeader::StringToStringMap& THeader::mutableWriteHeaders() {
  return ensureWriteHeaders();
}

THeader::StringToStringMap THeader::releaseWriteHeaders() {
  return c_.writeHeaders_ ? *std::exchange(c_.writeHeaders_, std::nullopt)
                          : THeader::StringToStringMap{};
}

THeader::StringToStringMap THeader::extractAllWriteHeaders() {
  auto headers = releaseWriteHeaders();
  if (c_.extraWriteHeaders_ != nullptr) {
    headers.insert(
        c_.extraWriteHeaders_->begin(), c_.extraWriteHeaders_->end());
  }
  return headers;
}

const THeader::StringToStringMap& THeader::getWriteHeaders() const {
  return c_.writeHeaders_ ? *c_.writeHeaders_ : kEmptyMap();
}

void THeader::setReadHeader(std::string_view key, std::string&& value) {
  ensureReadHeaders()[key] = std::move(value);
}

const THeader::StringToStringMap& THeader::getHeaders() const {
  return c_.readHeaders_ ? *c_.readHeaders_ : kEmptyMap();
}

THeader::StringToStringMap THeader::releaseHeaders() {
  return c_.readHeaders_ ? *std::exchange(c_.readHeaders_, std::nullopt)
                         : THeader::StringToStringMap{};
}

string THeader::getPeerIdentity() const {
  if (!c_.readHeaders_) {
    return "";
  }
  if (auto* id = folly::get_ptr(*c_.readHeaders_, IDENTITY_HEADER)) {
    if (auto* version = folly::get_ptr(*c_.readHeaders_, ID_VERSION_HEADER);
        version && *version == ID_VERSION) {
      return *id;
    }
  }
  return "";
}

void THeader::setIdentity(const string& identity) {
  this->c_.identity_ = identity;
}

unique_ptr<IOBuf> THeader::addHeader(unique_ptr<IOBuf> buf, bool transform) {
  // We may need to modify some transforms before send.  Make a copy here
  std::vector<uint16_t> writeTrans = c_.writeTrans_;

  if (c_.clientType_ == THRIFT_HEADER_CLIENT_TYPE) {
    if (transform) {
      buf = THeader::transform(std::move(buf), writeTrans);
    }
  }
  size_t chainSize = buf->computeChainDataLength();

  if (c_.protoId_ == T_JSON_PROTOCOL &&
      c_.clientType_ != THRIFT_HTTP_SERVER_TYPE) {
    throw TTransportException(
        TTransportException::BAD_ARGS, "Trying to send JSON without HTTP");
  }

  if (chainSize > MAX_FRAME_SIZE &&
      c_.clientType_ != THRIFT_HEADER_CLIENT_TYPE) {
    throw TTransportException(
        TTransportException::INVALID_FRAME_SIZE,
        "Attempting to send non-header frame that is too large");
  }

  // Add in special flags
  // All flags must be added before any calls to getMaxWriteHeadersSize
  if (c_.identity_.length() > 0) {
    ensureWriteHeaders();
    (*c_.writeHeaders_)[IDENTITY_HEADER] = c_.identity_;
    (*c_.writeHeaders_)[ID_VERSION_HEADER] = ID_VERSION;
  }

  if (c_.clientType_ == THRIFT_HEADER_CLIENT_TYPE) {
    // header size will need to be updated at the end because of varints.
    // Make it big enough here for max varint size, plus 4 for padding.
    int headerSize =
        (2 + getNumTransforms(writeTrans) * 2 /* transform data */) * 5 + 4;
    // add approximate size of info headers
    headerSize += getMaxWriteHeadersSize();

    // Pkt size
    unique_ptr<IOBuf> header = IOBuf::create(22 + headerSize);
    // 8 bytes of headroom, we'll use them if we go over MAX_FRAME_SIZE
    header->advance(8);

    uint8_t* pkt = header->writableData();
    uint8_t* headerStart;
    uint8_t* headerSizePtr;
    uint8_t* pktStart = pkt;

    size_t szHbo;
    uint32_t szNbo;
    uint16_t headerSizeN;

    // Fixup szNbo later
    pkt += sizeof(szNbo);
    uint16_t magicN = folly::Endian::big<uint16_t>(HEADER_MAGIC >> 16);
    memcpy(pkt, &magicN, sizeof(magicN));
    pkt += sizeof(magicN);
    uint16_t flagsN = folly::Endian::big(c_.flags_);
    memcpy(pkt, &flagsN, sizeof(flagsN));
    pkt += sizeof(flagsN);
    uint32_t seqIdN = folly::Endian::big(c_.seqId_);
    memcpy(pkt, &seqIdN, sizeof(seqIdN));
    pkt += sizeof(seqIdN);
    headerSizePtr = pkt;
    // Fixup headerSizeN later
    pkt += sizeof(headerSizeN);
    headerStart = pkt;

    pkt += writeVarint32(c_.protoId_, pkt);
    pkt += writeVarint32(getNumTransforms(writeTrans), pkt);

    for (auto& transId : writeTrans) {
      pkt += writeVarint32(transId, pkt);
    }

    // write info headers

    // write non-persistent kv-headers
    if (c_.writeHeaders_) {
      flushInfoHeaders(pkt, *c_.writeHeaders_, infoIdType::KEYVALUE);
    }
    if (c_.extraWriteHeaders_) {
      flushInfoHeaders(
          pkt, *c_.extraWriteHeaders_, infoIdType::KEYVALUE, false);
    }

    // TODO(davejwatson) optimize this for writing twice/memcopy to pkt buffer.
    // See code in TBufferTransports

    // Fixups after varint size calculations
    headerSize = (pkt - headerStart);
    uint8_t padding = 4 - (headerSize % 4);
    headerSize += padding;

    // Pad out pkt with 0x00
    for (int i = 0; i < padding; i++) {
      *(pkt++) = 0x00;
    }
    assert(pkt - pktStart <= static_cast<ptrdiff_t>(header->capacity()));

    // Pkt size
    szHbo = headerSize + chainSize // thrift header + payload
        + (headerStart - pktStart - 4); // common header section
    headerSizeN = folly::Endian::big<uint16_t>(headerSize / 4);
    memcpy(headerSizePtr, &headerSizeN, sizeof(headerSizeN));

    // Set framing size.
    if (szHbo > MAX_FRAME_SIZE) {
      if (!c_.allowBigFrames_) {
        throw TTransportException(
            TTransportException::INVALID_FRAME_SIZE, "Big frames not allowed");
      }
      header->prepend(8);
      pktStart -= 8;
      szNbo = folly::Endian::big(BIG_FRAME_MAGIC);
      memcpy(pktStart, &szNbo, sizeof(szNbo));
      uint64_t s = folly::Endian::big<uint64_t>(szHbo);
      memcpy(pktStart + 4, &s, sizeof(s));
    } else {
      szNbo = folly::Endian::big<uint32_t>(szHbo);
      memcpy(pktStart, &szNbo, sizeof(szNbo));
    }

    header->append(szHbo - chainSize + 4);
    header->prependChain(std::move(buf));
    buf = std::move(header);
  } else if (
      (c_.clientType_ == THRIFT_FRAMED_DEPRECATED) ||
      (c_.clientType_ == THRIFT_FRAMED_COMPACT)) {
    uint32_t szHbo = (uint32_t)chainSize;
    uint32_t szNbo = folly::Endian::big<uint32_t>(szHbo);

    unique_ptr<IOBuf> header = IOBuf::create(4);
    header->append(4);
    memcpy(header->writableData(), &szNbo, 4);
    header->prependChain(std::move(buf));
    buf = std::move(header);
  } else if (
      c_.clientType_ == THRIFT_UNFRAMED_DEPRECATED ||
      c_.clientType_ == THRIFT_UNFRAMED_COMPACT_DEPRECATED ||
      c_.clientType_ == THRIFT_HTTP_SERVER_TYPE) {
    // We just return buf
    // TODO: IOBufize httpTransport.
  } else if (c_.clientType_ == THRIFT_HTTP_CLIENT_TYPE) {
    CHECK(c_.httpClientParser_.get() != nullptr);
    buf = c_.httpClientParser_->constructHeader(
        std::move(buf),
        c_.writeHeaders_ ? *c_.writeHeaders_ : kEmptyMap(),
        c_.extraWriteHeaders_);
    c_.writeHeaders_.reset();
  } else {
    throw TTransportException(
        TTransportException::BAD_ARGS, "Unknown client type");
  }

  return buf;
}

apache::thrift::concurrency::PRIORITY THeader::getCallPriority() const {
  if (c_.priority_) {
    return *c_.priority_;
  }
  const auto& map = getHeaders();
  auto iter = map.find(PRIORITY_HEADER);
  if (iter != map.end()) {
    try {
      unsigned prio = folly::to<unsigned>(iter->second);
      if (prio < apache::thrift::concurrency::N_PRIORITIES) {
        return static_cast<apache::thrift::concurrency::PRIORITY>(prio);
      }
    } catch (const std::range_error&) {
    }
    LOG(INFO) << "Bad method priority " << iter->second << ", using default";
  }
  // no priority
  return apache::thrift::concurrency::N_PRIORITIES;
}

std::chrono::milliseconds THeader::getTimeoutFromHeader(
    std::string_view header) const {
  const auto& map = getHeaders();
  auto iter = map.find(header);
  if (iter != map.end()) {
    try {
      int64_t timeout = folly::to<int64_t>(iter->second);
      return std::chrono::milliseconds(timeout);
    } catch (const std::range_error&) {
    }
    LOG(INFO) << "Bad client timeout " << iter->second << ", using default";
  }

  return std::chrono::milliseconds(0);
}

std::chrono::milliseconds THeader::getClientTimeout() const {
  if (c_.clientTimeout_) {
    return *c_.clientTimeout_;
  } else {
    return getTimeoutFromHeader(CLIENT_TIMEOUT_HEADER);
  }
}

std::chrono::milliseconds THeader::getClientQueueTimeout() const {
  if (c_.queueTimeout_) {
    return *c_.queueTimeout_;
  } else {
    return getTimeoutFromHeader(QUEUE_TIMEOUT_HEADER);
  }
}

folly::Optional<std::chrono::milliseconds> THeader::getServerQueueTimeout()
    const {
  return c_.serverQueueTimeout_;
}

folly::Optional<std::chrono::milliseconds> THeader::getProcessDelay() const {
  return c_.processDelay_;
}

const folly::Optional<std::string>& THeader::clientId() const {
  return c_.clientId_;
}

const folly::Optional<std::string>& THeader::tenantId() const {
  return c_.tenantId_;
}

const folly::Optional<std::string>& THeader::serviceTraceMeta() const {
  return c_.serviceTraceMeta_;
}

void THeader::setHttpClientParser(shared_ptr<THttpClientParser> parser) {
  CHECK(c_.clientType_ == THRIFT_HTTP_CLIENT_TYPE);
  c_.httpClientParser_ = parser;
}

void THeader::setClientTimeout(std::chrono::milliseconds timeout) {
  c_.clientTimeout_ = timeout;
}

void THeader::setClientQueueTimeout(std::chrono::milliseconds timeout) {
  c_.queueTimeout_ = timeout;
}

void THeader::setServerQueueTimeout(std::chrono::milliseconds timeout) {
  c_.serverQueueTimeout_ = timeout;
}

void THeader::setProcessDelay(std::chrono::milliseconds timeQueued) {
  c_.processDelay_ = timeQueued;
}

void THeader::setCallPriority(apache::thrift::concurrency::PRIORITY priority) {
  c_.priority_ = priority;
}

void THeader::setClientId(const std::string& clientId) {
  c_.clientId_ = clientId;
}

void THeader::setTenantId(const std::string& tenantId) {
  c_.tenantId_ = tenantId;
}

void THeader::setServiceTraceMeta(const std::string& serviceTraceMeta) {
  c_.serviceTraceMeta_ = serviceTraceMeta;
}

static constexpr folly::StringPiece TRANSFORMS_STRING_LIST[] = {
    folly::StringPiece("none"),
    folly::StringPiece("zlib"),
    folly::StringPiece("hmac"),
    folly::StringPiece("snappy"),
    folly::StringPiece("qlz"),
    folly::StringPiece("zstd"),
};

const folly::StringPiece THeader::getStringTransform(
    const TRANSFORMS transform) {
  constexpr const std::size_t num_string_transforms =
      sizeof(TRANSFORMS_STRING_LIST) / sizeof(folly::StringPiece);
  static_assert(
      num_string_transforms == THeader::TRANSFORMS::TRANSFORM_LAST_FIELD,
      "TRANSFORMS enum and TRANSFORMS_STRING_LIST mismatch");
  return TRANSFORMS_STRING_LIST[transform];
}

void THeader::setClientMetadata(const ClientMetadata& clientMetadata) {
  ensureWriteHeaders()[std::string{CLIENT_METADATA_HEADER}] =
      apache::thrift::SimpleJSONSerializer::serialize<std::string>(
          clientMetadata);
}

std::optional<ClientMetadata> THeader::extractClientMetadata() {
  if (auto mdString = extractHeader(CLIENT_METADATA_HEADER)) {
    return apache::thrift::SimpleJSONSerializer::deserialize<ClientMetadata>(
        *mdString);
  }

  return {};
}

std::optional<std::string> THeader::extractHeader(std::string_view key) {
  if (!c_.readHeaders_) {
    return std::nullopt;
  }
  std::optional<std::string> res;
  auto itr = c_.readHeaders_->find(std::string{key});
  if (itr != c_.readHeaders_->end()) {
    res = std::move(itr->second);
    c_.readHeaders_->erase(itr);
  }
  return res;
}
} // namespace transport
} // namespace thrift
} // namespace apache

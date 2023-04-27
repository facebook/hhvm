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

#ifndef CPP2_PROTOCOL_PROTOCOL_H_
#define CPP2_PROTOCOL_PROTOCOL_H_ 1

#include <sys/types.h>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <glog/logging.h>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp/protocol/TProtocol.h>
#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/CloneableIOBuf.h>
#include <thrift/lib/cpp2/protocol/ProtocolReaderWireTypeInfo.h>

/**
 * Protocol Readers and Writers are ducktyped in cpp2.
 * This means they have no base class or virtual methods,
 * and you must explicitly choose the correct class to use.
 *
 * This results in a 2x performance increase over a traditional
 * object oriented hierarchy. If you need a base class, you can
 * use the VirtualReader / VitualWriter in VirtualProtocol.h
 */

namespace apache {
namespace thrift {
namespace detail {

class ProtocolBase {
 public:
  // height is maximum permitted remaining levels of nesting, applying to map,
  // set, list, and struct types
  void setHeight(size_t height) { height_ = height + 1; }
  size_t getHeight() const { return height_ - 1; }
  ProtocolBase();

 protected:
  void descend() {
    if (!--height_) {
      protocol::TProtocolException::throwExceededDepthLimit();
    }
  }

  void ascend() { ++height_; }

 private:
  size_t height_;
};

} // namespace detail

/**
 * Certain serialization / deserialization operations allow sharing
 * external (user-owned) buffers. This means that the external buffers must
 * remain allocated (and unchanged) until the serialization / deserialization
 * is complete.
 *
 * This is often counter-intuitive; for example, deserializing from a string
 * wouldn't work with a temporary string if sharing is allowed. So sharing
 * external buffers must be requested explicitly.
 *
 * Note that we always share memory that is under IOBuf's control (that is,
 * IOBuf chains for which isManaged() is true). To prevent that, call unshare()
 * on the IOBuf chain as appropriate.
 */
enum ExternalBufferSharing {
  COPY_EXTERNAL_BUFFER,
  SHARE_EXTERNAL_BUFFER,
};

using apache::thrift::protocol::TProtocolException;
using apache::thrift::protocol::TType;
typedef apache::thrift::protocol::PROTOCOL_TYPES ProtocolType;

/*
 * Enumerated definition of the message types that the Thrift protocol
 * supports.
 */
enum class MessageType {
  T_CALL = 1,
  T_REPLY = 2,
  T_EXCEPTION = 3,
  T_ONEWAY = 4,
};

namespace detail {
struct SkipNoopString {
  void append(const char*, size_t) {}
  void clear() {}
  void reserve(size_t) {}
};

// Checks if bool hold a valid value (true or false) and throws exception
// otherwise. Without the check we may produce undeserializable data.
inline bool validate_bool(uint8_t value) {
#if defined(__x86_64__) && defined(__GNUC__) && \
    (!defined(__clang_major__) || __clang_major__ >= 9)
  // An optimized version that avoid extra load/store.
  asm volatile goto(
#if !defined(__clang_major__) || __clang_major__ >= 14
      // Provide AT&T and Intel syntax when included in an -masm=intel context.
      // See also https://reviews.llvm.org/D113707
      "cmp{b $1, %b0| %b0, $1}\n"
#else
      "cmpb $1, %b0\n"
#endif
      "ja %l1"
      : // no outputs.
      : "r"(value)
      : "cc"
      : invalid);
  return value;
invalid:
  LOG(FATAL) << "invalid bool value";
  return false;
#else
  // Store in a volatile variable to prevent the compiler from optimizing the
  // check away.
  volatile uint8_t volatileByte = value;
  uint8_t byte = volatileByte;
  if (!(byte == 0 || byte == 1)) {
    LOG(FATAL) << "invalid bool value";
  }
  return byte != 0;
#endif
}
} // namespace detail

/* forward declaration */
template <class Protocol_, class WireType>
void skip_n(Protocol_& prot, uint32_t n, std::initializer_list<WireType> types);

/**
 * Helper template for implementing Protocol::skip().
 *
 * Templatized to avoid having to make virtual function calls. Protocols with
 * their own opinions about skipping implementation can specialize, although
 * currently none do.
 */
template <class Protocol_, class WireType>
void skip(Protocol_& prot, WireType arg_type) {
  switch (arg_type) {
    case TType::T_BOOL: {
      bool boolv;
      prot.readBool(boolv);
      return;
    }
    // case TType::T_I08: // same numeric value as T_BYTE
    case TType::T_BYTE: {
      int8_t bytev;
      prot.readByte(bytev);
      return;
    }
    case TType::T_I16: {
      int16_t i16;
      prot.readI16(i16);
      return;
    }
    case TType::T_I32: {
      int32_t i32;
      prot.readI32(i32);
      return;
    }
    case TType::T_U64:
    case TType::T_I64: {
      int64_t i64;
      prot.readI64(i64);
      return;
    }
    case TType::T_DOUBLE: {
      double dub;
      prot.readDouble(dub);
      return;
    }
    case TType::T_FLOAT: {
      float flt;
      prot.readFloat(flt);
      return;
    }
    // case TType::T_UTF7: // same numeric value as T_STRING
    case TType::T_UTF8:
    case TType::T_UTF16:
    case TType::T_STRING: {
      apache::thrift::detail::SkipNoopString str;
      prot.readBinary(str);
      return;
    }
    case TType::T_STRUCT: {
      std::string name;
      int16_t fid;
      TType ftype;
      prot.readStructBegin(name);
      while (true) {
        prot.readFieldBegin(name, ftype, fid);
        if (ftype == TType::T_STOP) {
          break;
        }
        apache::thrift::skip(prot, ftype);
        prot.readFieldEnd();
      }
      prot.readStructEnd();
      return;
    }
    case TType::T_MAP: {
      TType keyType;
      TType valType;
      uint32_t size;
      prot.readMapBegin(keyType, valType, size);
      skip_n(prot, size, {keyType, valType});
      prot.readMapEnd();
      return;
    }
    case TType::T_SET: {
      TType elemType;
      uint32_t size;
      prot.readSetBegin(elemType, size);
      skip_n(prot, size, {elemType});
      prot.readSetEnd();
      return;
    }
    case TType::T_LIST: {
      TType elemType;
      uint32_t size;
      prot.readListBegin(elemType, size);
      skip_n(prot, size, {elemType});
      prot.readListEnd();
      return;
    }
    case TType::T_STOP:
    case TType::T_VOID:
    case TType::T_STREAM:
      // Unimplemented, fallback to default
    default: {
      TProtocolException::throwInvalidSkipType(arg_type);
    }
  }
}

/**
 * Check if the remaining part of buffers contain least necessary amount of
 * bytes to encode N elements of given type.
 *
 * Note: this is a lightweight lower bound check, it doesn't necessary mean
 *       that we would actually succeed at reading N items.
 */
template <class Protocol_>
inline bool canReadNElements(
    Protocol_& prot,
    uint32_t n,
    std::initializer_list<
        typename apache::thrift::detail::ProtocolReaderWireTypeInfo<
            Protocol_>::WireType> types) {
  return prot.getCursor().canAdvance(n * types.size());
}

/*
 * Skip n tuples - used for skipping lists, sets, maps.
 *
 * As with skip(), protocols can specialize.
 */
template <class Protocol_, class WireType>
void skip_n(
    Protocol_& prot, uint32_t n, std::initializer_list<WireType> types) {
  size_t sum = 0;
  bool allFixedSizes = true;
  for (auto type : types) {
    auto size = prot.fixedSizeInContainer(type);
    sum += size;
    allFixedSizes = allFixedSizes && size;
  }
  if (allFixedSizes) {
    prot.skipBytes(sum * n);
    return;
  }

  for (uint32_t i = 0; i < n; i++) {
    for (auto type : types) {
      apache::thrift::skip(prot, type);
    }
  }
}

template <class StrType>
struct StringTraits {
  static StrType fromStringLiteral(const char* str) { return StrType(str); }

  static bool isEmpty(const StrType& str) { return str.empty(); }

  static bool isEqual(const StrType& lhs, const StrType& rhs) {
    return lhs == rhs;
  }

  static bool isLess(const StrType& lhs, const StrType& rhs) {
    return lhs < rhs;
  }
};

template <>
struct StringTraits<folly::IOBuf> {
  // Use with string literals only!
  static folly::IOBuf fromStringLiteral(const char* str) {
    return folly::IOBuf::wrapBufferAsValue(str, strlen(str));
  }

  static bool isEmpty(const folly::IOBuf& str) { return str.empty(); }

  static bool isEqual(const folly::IOBuf& lhs, const folly::IOBuf& rhs) {
    return folly::IOBufEqualTo{}(lhs, rhs);
  }

  static bool isLess(const folly::IOBuf& lhs, const folly::IOBuf& rhs) {
    return folly::IOBufLess{}(lhs, rhs);
  }
};

template <>
struct StringTraits<std::unique_ptr<folly::IOBuf>> {
  // Use with string literals only!
  static std::unique_ptr<folly::IOBuf> fromStringLiteral(const char* str) {
    return (
        str[0] != '\0' ? folly::IOBuf::wrapBuffer(str, strlen(str)) : nullptr);
  }

  static bool isEmpty(const std::unique_ptr<folly::IOBuf>& str) {
    return !str || str->empty();
  }

  static bool isEqual(
      const std::unique_ptr<folly::IOBuf>& lhs,
      const std::unique_ptr<folly::IOBuf>& rhs) {
    return folly::IOBufEqualTo{}(lhs, rhs);
  }

  static bool isLess(
      const std::unique_ptr<folly::IOBuf>& lhs,
      const std::unique_ptr<folly::IOBuf>& rhs) {
    return folly::IOBufLess{}(lhs, rhs);
  }
};

} // namespace thrift
} // namespace apache

#endif

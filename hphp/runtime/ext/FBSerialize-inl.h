/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_FBSERIALIZE_INL_H_
#define incl_HPHP_FBSERIALIZE_INL_H_

#include <folly/Bits.h>
#include <folly/Conv.h>

namespace HPHP { namespace serialize {

struct SerializeError : std::runtime_error {
  explicit SerializeError(const std::string& msg)
      : std::runtime_error(msg) {
  }
};

struct UnserializeError : std::runtime_error {
  explicit UnserializeError(const std::string& msg)
      : std::runtime_error(msg) {
  }
};

template <class V>
template <typename Variant>
void FBSerializer<V>::serialize(const Variant& thing,
                                char* out) {
  FBSerializer<V> serializer(out);
  serializer.doSerialize(thing);
}

template <class V>
FBSerializer<V>::FBSerializer(char* out) : out_(out) {
}

template <class V>
template <typename Variant>
void FBSerializer<V>::doSerialize(const Variant& thing) {
  serializeThing(thing, 0);
}

template <class V>
inline void FBSerializer<V>::write(const char* src, size_t size) {
  memcpy(out_, src, size);
  out_ += size;
}

template <class V>
void FBSerializer<V>::writeCode(Code code) {
  uint8_t v = code;
  write(reinterpret_cast<char*>(&v), CODE_SIZE);
}

template <class V>
void FBSerializer<V>::serializeBoolean(bool val) {
  writeCode(FB_SERIALIZE_BOOLEAN);
  int8_t v = val;
  write(reinterpret_cast<char*>(&v), BOOLEAN_SIZE);
}

template <class V>
void FBSerializer<V>::serializeInt64(int64_t val) {
  if (val == (int64_t)(int8_t)val) {
    writeCode(FB_SERIALIZE_BYTE);
    int8_t v = val;
    write(reinterpret_cast<char*>(&v), INT8_SIZE);
  } else if (val == (int64_t)(int16_t)val) {
    writeCode(FB_SERIALIZE_I16);
    uint16_t nval = folly::Endian::big16(static_cast<uint16_t>(val));
    write(reinterpret_cast<char*>(&nval), INT16_SIZE);
  } else if (val == (int64_t)(int32_t)val) {
    writeCode(FB_SERIALIZE_I32);
    uint32_t nval = folly::Endian::big32(static_cast<uint32_t>(val));
    write(reinterpret_cast<char*>(&nval), INT32_SIZE);
  } else {
    writeCode(FB_SERIALIZE_I64);
    uint64_t nval = folly::Endian::big64(static_cast<uint64_t>(val));
    write(reinterpret_cast<char*>(&nval), INT64_SIZE);
  }
}

template <class V>
void FBSerializer<V>::serializeDouble(double val) {
  writeCode(FB_SERIALIZE_DOUBLE);
  write(reinterpret_cast<char*>(&val), DOUBLE_SIZE);
}

template <class V>
template <typename String>
void FBSerializer<V>::serializeString(const String& str) {
  size_t len = V::stringLen(str);

  if (len == (size_t)(uint8_t)len) {
    writeCode(FB_SERIALIZE_VARCHAR);
    uint8_t nval = len;
    write(reinterpret_cast<char*>(&nval), INT8_SIZE);
  } else {
    writeCode(FB_SERIALIZE_STRING);
    uint32_t nval = folly::Endian::big32(static_cast<uint32_t>(len));
    write(reinterpret_cast<char*>(&nval), INT32_SIZE);
  }
  write(V::stringData(str), V::stringLen(str));
}

template <class V>
template <typename Map>
void FBSerializer<V>::serializeMap(const Map& map, size_t depth) {
  writeCode(FB_SERIALIZE_STRUCT);
  for (auto it = V::mapIterator(map); V::mapNotEnd(map, it); V::mapNext(it)) {
    auto key = V::mapKey(it);
    switch (V::mapKeyType(key)) {
      case Type::INT64:
        serializeInt64(V::mapKeyAsInt64(key));
        break;
      case Type::STRING:
        serializeString(V::mapKeyAsString(key));
        break;
      default:
        throw SerializeError("map key must be int64 or string");
    }
    serializeThing(V::mapValue(it), depth + 1);
  }
  writeCode(FB_SERIALIZE_STOP);
}

template <class V>
template <typename Vector>
void FBSerializer<V>::serializeVector(const Vector& vec, size_t depth) {
  writeCode(FB_SERIALIZE_STRUCT);

  size_t index = 0;
  for (auto it = V::vectorIterator(vec);
       V::vectorNotEnd(vec, it);
       V::vectorNext(it), ++index) {
    serializeInt64(index);
    serializeThing(V::vectorValue(it), depth + 1);
  }
  writeCode(FB_SERIALIZE_STOP);
}

template <class V>
template <typename Variant>
void FBSerializer<V>::serializeThing(const Variant& thing, size_t depth) {
  if (depth > 256) {
    throw SerializeError("link depth > 256");
  }

  switch (V::type(thing)) {
    case Type::NULLT:
      writeCode(FB_SERIALIZE_NULL);
      break;

    case Type::BOOL:
      serializeBoolean(V::asBool(thing));
      break;

    case Type::INT64:
      serializeInt64(V::asInt64(thing));
      break;

    case Type::DOUBLE:
      serializeDouble(V::asDouble(thing));
      break;

    case Type::STRING:
      serializeString(V::asString(thing));
      break;

    case Type::MAP:
      serializeMap(V::asMap(thing), depth);
      break;

    case Type::VECTOR:
      serializeVector(V::asVector(thing), depth);
      break;

    default:
      throw SerializeError("Unknown type");
  }
}

template <class V>
template <typename Variant>
size_t FBSerializer<V>::serializedSize(const Variant& thing) {
  return serializedSizeThing(thing, 0);
}

template <class V>
template <typename Variant>
size_t FBSerializer<V>::serializedSizeThing(const Variant& thing,
                                            size_t depth) {
  if (depth > 256) {
    throw SerializeError("link depth > 256");
  }

  switch (V::type(thing)) {
    case Type::NULLT:
      return CODE_SIZE;
      break;

    case Type::BOOL:
      return CODE_SIZE + BOOLEAN_SIZE;
      break;

    case Type::INT64:
      return serializedSizeInt64(V::asInt64(thing));
      break;

    case Type::DOUBLE:
      return CODE_SIZE + DOUBLE_SIZE;
      break;

    case Type::STRING:
      return serializedSizeString(V::asString(thing));
      break;

    case Type::MAP:
      return serializedSizeMap(V::asMap(thing), depth);
      break;

    case Type::VECTOR:
      return serializedSizeVector(V::asVector(thing), depth);
      break;

    default:
      throw SerializeError("Unknown type");
  }
}

template <class V>
size_t FBSerializer<V>::serializedSizeInt64(int64_t val) {
  if (val == (int64_t)(int8_t)val) {
    return CODE_SIZE + INT8_SIZE;
  } else if (val == (int64_t)(int16_t)val) {
    return CODE_SIZE + INT16_SIZE;
  } else if (val == (int64_t)(int32_t)val) {
    return CODE_SIZE + INT32_SIZE;
  } else {
    return CODE_SIZE + INT64_SIZE;
  }
}

template <class V>
template <typename String>
size_t FBSerializer<V>::serializedSizeString(const String& str) {
  size_t len = V::stringLen(str);

  if (len == (size_t)(uint8_t)len) {
    return CODE_SIZE + INT8_SIZE + len;
  } else {
    return CODE_SIZE + INT32_SIZE + len;
  }
}

template <class V>
template <typename Map>
size_t FBSerializer<V>::serializedSizeMap(const Map& map, size_t depth) {
  // Map code + stop code
  size_t ret = CODE_SIZE + CODE_SIZE;

  for (auto it = V::mapIterator(map); V::mapNotEnd(map, it); V::mapNext(it)) {
    auto key = V::mapKey(it);
    switch (V::mapKeyType(key)) {
      case Type::INT64:
        ret += serializedSizeInt64(V::mapKeyAsInt64(key));
        break;
      case Type::STRING:
        ret += serializedSizeString(V::mapKeyAsString(key));
        break;
      default:
        throw SerializeError("map key must be int64 or string");
    }
    ret += serializedSizeThing(V::mapValue(it), depth + 1);
  }

  return ret;
}

template <class V>
template <typename Vector>
size_t FBSerializer<V>::serializedSizeVector(const Vector& vec, size_t depth) {
  // Vector code + stop code
  size_t ret = CODE_SIZE + CODE_SIZE;

  size_t index = 0;
  for (auto it = V::vectorIterator(vec);
       V::vectorNotEnd(vec, it);
       V::vectorNext(it), ++index) {
    ret += serializedSizeInt64(index);
    ret += serializedSizeThing(V::vectorValue(it), depth + 1);
  }

  return ret;
}

template <class V>
inline typename V::VariantType FBUnserializer<V>::unserialize(
  folly::StringPiece serialized) {

  FBUnserializer<V> unserializer(serialized);
  return unserializer.unserializeThing();
}

template <class V>
FBUnserializer<V>::FBUnserializer(folly::StringPiece serialized) :
    p_(serialized.data()), end_(p_ + serialized.size()) {
}

template <class V>
inline void FBUnserializer<V>::need(size_t n) const {
  if (UNLIKELY(p_ + n > end_)) {
    throw UnserializeError("Unexpected end");
  }
}

template <class V>
inline bool FBUnserializer<V>::unserializeBoolean() {
  p_ += CODE_SIZE;

  need(BOOLEAN_SIZE);
  bool ret = *p_;
  p_ += BOOLEAN_SIZE;
  return ret;
}

template <class V>
inline int64_t FBUnserializer<V>::unserializeInt64() {
  size_t code = *p_;
  p_ += CODE_SIZE;

  switch (code) {
    case FB_SERIALIZE_BYTE:
    {
      need(INT8_SIZE);
      int8_t ret = static_cast<int8_t>(*(p_));
      p_ += INT8_SIZE;
      return ret;
    }
    case FB_SERIALIZE_I16:
    {
      need(INT16_SIZE);
      int16_t ret = folly::Endian::big16(
        *reinterpret_cast<const uint16_t*>(p_));
      p_ += INT16_SIZE;
      return ret;
    }
    case FB_SERIALIZE_I32:
    {
      need(INT32_SIZE);
      int32_t ret = folly::Endian::big32(
        *reinterpret_cast<const uint32_t*>(p_));
      p_ += INT32_SIZE;
      return ret;
    }
    case FB_SERIALIZE_I64:
    {
      need(INT64_SIZE);
      int64_t ret = folly::Endian::big64(
          *reinterpret_cast<const uint64_t*>(p_));
      p_ += INT64_SIZE;
      return ret;
    }
    default:
      throw UnserializeError("Invalid integer code.");
  }
}

template <class V>
inline double FBUnserializer<V>::unserializeDouble() {
  p_ += CODE_SIZE;

  need(DOUBLE_SIZE);
  double ret = *reinterpret_cast<const double*>(p_);
  p_ += DOUBLE_SIZE;
  return ret;
}

template <class V>
inline typename V::StringType FBUnserializer<V>::unserializeString() {
  auto s = unserializeStringPiece();
  return V::stringFromData(s.data(), s.size());
}

template <class V>
inline folly::StringPiece FBUnserializer<V>::unserializeStringPiece() {
  size_t code = *p_;
  p_ += CODE_SIZE;
  size_t data_size = 0;

  switch (code) {
    case FB_SERIALIZE_VARCHAR:
      need(INT8_SIZE);
      data_size = (uint8_t)*(p_);
      p_ += INT8_SIZE;
      break;
    case FB_SERIALIZE_STRING:
      need(INT32_SIZE);
      data_size = folly::Endian::big32(
        *reinterpret_cast<const int32_t*>(p_));
      p_ += INT32_SIZE;
      break;
    default:
      throw UnserializeError("Invalid string code.");
  }

  need(data_size);
  folly::StringPiece ret(p_, data_size);
  p_ += data_size;
  return ret;
}

template <class V>
inline FBSerializeBase::Code FBUnserializer<V>::nextCode() const {
  need(CODE_SIZE);
  return static_cast<Code>(*p_);
}

template <class V>
inline typename V::MapType FBUnserializer<V>::unserializeMap() {
  p_ += CODE_SIZE;

  typename V::MapType ret = V::createMap();

  size_t code = nextCode();
  while (code != FB_SERIALIZE_STOP) {
    switch (code) {
      case FB_SERIALIZE_VARCHAR:
      case FB_SERIALIZE_STRING:
        {
          auto key = unserializeString();
          auto value = unserializeThing();
          V::mapSet(ret, std::move(key), std::move(value));
        }
        break;
      default:
        {
          auto key = unserializeInt64();
          auto value = unserializeThing();
          V::mapSet(ret, std::move(key), std::move(value));
        }
    }

    code = nextCode();
  }
  p_ += CODE_SIZE;

  return ret;
}

template <class V>
inline folly::StringPiece FBUnserializer<V>::getSerializedMap() {
  const char* head = p_;
  p_ += CODE_SIZE;

  size_t code = nextCode();
  while (code != FB_SERIALIZE_STOP) {
    switch (code) {
      case FB_SERIALIZE_BYTE:
        p_ += (CODE_SIZE + INT8_SIZE);
        break;
      case FB_SERIALIZE_I16:
        p_ += (CODE_SIZE + INT16_SIZE);
        break;
      case FB_SERIALIZE_I32:
        p_ += (CODE_SIZE + INT32_SIZE);
        break;
      case FB_SERIALIZE_I64:
        p_ += (CODE_SIZE + INT64_SIZE);
        break;
      case FB_SERIALIZE_VARCHAR:
      case FB_SERIALIZE_STRING:
        unserializeStringPiece();
        break;
      case FB_SERIALIZE_STRUCT:
        {
          getSerializedMap();
          break;
        }
      case FB_SERIALIZE_NULL:
        {
           p_ += CODE_SIZE;
           break;
        }
      case FB_SERIALIZE_DOUBLE:
        {
          p_ += (CODE_SIZE + DOUBLE_SIZE);
          break;
        }
      case FB_SERIALIZE_BOOLEAN:
        {
          p_ += (CODE_SIZE + BOOLEAN_SIZE);
          break;
        }
      default:
        throw UnserializeError("Invalid code: " + folly::to<std::string>(code)
                               + " at location " + folly::to<std::string>(p_));
    }
    code = nextCode();
  }
  p_ += CODE_SIZE;
  const char* tail = p_;
  return folly::StringPiece(head, tail);
}

template <class V>
inline typename V::VariantType FBUnserializer<V>::unserializeThing() {
  size_t code = nextCode();

  switch (code) {
    case FB_SERIALIZE_BYTE:
    case FB_SERIALIZE_I16:
    case FB_SERIALIZE_I32:
    case FB_SERIALIZE_I64:
      return V::fromInt64(unserializeInt64());
    case FB_SERIALIZE_VARCHAR:
    case FB_SERIALIZE_STRING:
      return unserializeString();
    case FB_SERIALIZE_STRUCT:
      return V::fromMap(unserializeMap());
    case FB_SERIALIZE_NULL:
      ++p_;
      return V::createNull();
    case FB_SERIALIZE_DOUBLE:
      return V::fromDouble(unserializeDouble());
    case FB_SERIALIZE_BOOLEAN:
      return V::fromBool(unserializeBoolean());
    default:
      throw UnserializeError("Invalid code: " + folly::to<std::string>(code)
                             + " at location " + folly::to<std::string>(p_));
  }
}

template <class V>
inline void FBUnserializer<V>::advance(size_t delta) {
  p_ += delta;
}

}}

#endif // incl_HPHP_FBSERIALIZE_INL_H_

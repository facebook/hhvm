/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <type_traits>
#include <unordered_set>
#include <vector>

#include <folly/Optional.h>
#include <folly/Varint.h>

#include "hphp/compiler/option.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/vm/coeffects.h"
#include "hphp/runtime/vm/litstr-table.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"

#include "hphp/util/compact-vector.h"

/*
 * This module contains helpers for serializing and deserializing
 * metadata into blobs suitable for insertion into the hhbc repo.
 *
 * Types may provide their own serialization logic by implementing a
 * single-argument member function template called `serde' (as in
 * "serialization/deserialization"), and pushing data it wants to
 * serialize into the parameter.  The function may take a set of
 * optional arguments but if your function require those arguments
 * they also have to be passed in when calling sd. Those members may
 * in turn have customized serialization behavior, or they may "bottom
 * out" to these helpers if they are basic-enough types.
 *
 * For example:
 *
 *    struct MyBlobableStuff {
 *      int m_foo;
 *      std::vector<const StringData*> m_strList;
 *      OtherBlobbableData m_other;
 *
 *      template<class SerDe> void serde(SerDe& sd) {
 *        sd(m_foo)(m_strList)(m_other);
 *      }
 *    };
 *
 * The argument to SerDe will have a static const member called
 * `deserializing' which can be used to do custom behavior depending
 * on whether you're deserializing.  But generally the goal here
 * should be to eliminate a class of bugs by having the same (textual)
 * code implement both serialization and deserialization.
 */

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Warning: some language abuse below.
 *
 * This defines a trait that determines whether or not a type T has a
 * member function template called `serde' that specializes for the
 * type `SerDe'.  The point here is that calling one of the blob
 * helpers on an object that knows how to do its own serialization
 * will just reflect back to it.
 *
 * If you are unlucky enough to need to know how this works, look up
 * "SFINAE".
 */

template<class T, class SerDe, class... F>
struct IsNontrivialSerializable {
private:
  template<class U, void (U::*)(SerDe&, F...)> struct Checker;
  template<class U> static char test(
    Checker<U,&U::template serde<SerDe, F...> >*);
  template<class>   static long test(...);
public:
  enum { value = sizeof(test<T>(0)) == 1 };
};

//////////////////////////////////////////////////////////////////////

struct BlobEncoder {
  static const bool deserializing = false;
  explicit BlobEncoder(bool l) : m_useGlobalIds{l} {}

  void writeRaw(const char* ptr, size_t size) {
    auto const start = m_blob.size();
    m_blob.resize(start + size);
    std::copy(ptr, ptr + size, &m_blob[start]);
  }

  /*
   * Currently the most basic encoder/decode only works for integral
   * types.  (We don't want this to accidentally get used for things
   * like pointers or aggregates.)
   *
   * Floating point support could be added later if we need it ...
   */
   template<class T>
   typename std::enable_if<
     std::is_integral<T>::value && std::is_signed<T>::value
   >::type
   encode(const T& t) {
     encode(folly::encodeZigZag(static_cast<int64_t>(t)));
   }

  template<class T>
  typename std::enable_if<
    (std::is_integral<T>::value && !std::is_signed<T>::value) ||
    std::is_enum<T>::value
  >::type
  encode(const T& t) {
    const size_t start = m_blob.size();
    unsigned char buf[folly::kMaxVarintLength64];
    uint64_t value = uint64_t{
      static_cast<typename std::make_unsigned<T>::type>(t)};
    size_t buf_size = folly::encodeVarint(value, buf);

    m_blob.resize(start + buf_size);

    std::copy(buf, buf + buf_size, &m_blob[start]);
  }

  template<class T, class... F>
  typename std::enable_if<
    IsNontrivialSerializable<T,BlobEncoder, F...>::value
  >::type encode(const T& t, F... lambdas) {
    const_cast<T&>(t).serde(*this, lambdas...);
  }

  void encode(bool b) {
    encode(b ? 1 : 0);
  }

  void encode(const SHA1& sha1) {
    for (auto w : sha1.q) encode(w);
  }

  void encode(Location::Range loc) {
    encode(loc.line0);
    encode(loc.char0);
    encode(loc.line1);
    encode(loc.char1);
  }

  void encode(DataType t) {
    // always encode DataType as int8 even if it's a bigger size.
    assertx(DataType(int8_t(t)) == t);
    encode(int8_t(t));
  }

  void encode(const LowStringPtr& s) {
    const StringData* sd = s;
    if (m_useGlobalIds) {
      Id id = LitstrTable::get().mergeLitstr(sd);
      encode(id);
      return;
    }

    encode(sd);
  }

  void encode(const StringData* sd) {
    if (!sd) { return encode(uint32_t(0)); }
    uint32_t sz = sd->size();
    encode(sz + 1);
    if (!sz) { return; }

    const size_t start = m_blob.size();
    m_blob.resize(start + sz);
    std::copy(sd->data(), sd->data() + sz, &m_blob[start]);
  }

  void encode(const std::string& s) {
    uint32_t sz = s.size();
    encode(sz);
    if (!sz) return;
    const size_t start = m_blob.size();
    m_blob.resize(start + sz);
    std::copy(s.data(), s.data() + sz, &m_blob[start]);
  }

  void encode(const TypedValue& tv) {
    if (tv.m_type == KindOfUninit) {
      return encode(staticEmptyString());
    }
    auto s = internal_serialize(tvAsCVarRef(&tv));
    encode(s.get());
  }

  void encode(const TypedValueAux& tv) = delete;

  template<class T>
  void encode(const folly::Optional<T>& opt) {
    const bool some = opt.hasValue();
    encode(some);
    if (some) encode(*opt);
  }

  template <size_t I = 0, typename... Ts>
  typename std::enable_if<I == sizeof...(Ts), void>::type
  encode(const std::tuple<Ts...>& /*val*/) {}

  template<size_t I = 0, typename ...Ts>
  typename std::enable_if<I < sizeof...(Ts), void>::type
  encode(const std::tuple<Ts...>& val) {
    encode(std::get<I>(val));
    encode<I + 1, Ts...>(val);
  }

  template<class K, class V>
  void encode(const std::pair<K,V>& kv) {
    encode(kv.first);
    encode(kv.second);
  }

  template<class T, typename FDeltaEncode>
  void encode(const std::vector<T>& cont, FDeltaEncode deltaEncode) {
    if (cont.size() >= 0xffffffffu) {
      throw std::runtime_error("maximum vector size exceeded in BlobEncoder");
    }

    auto prev = T{};
    encode(uint32_t(cont.size()));
    for (auto it = cont.begin(); it != cont.end(); ++it) {
      encode(deltaEncode(prev, *it));
      prev = *it;
    }
  }

  template<class T>
  void encode(const std::vector<T>& vec) {
    encodeContainer(vec, "vector");
  }

  template<class T>
  void encode(const CompactVector<T>& vec) {
    encodeContainer(vec, "CompactVector");
  }

  template<class T>
  typename std::enable_if<
    (std::is_same<typename T::value_type,
                  std::pair<typename T::key_type const,
                            typename T::mapped_type>>::value ||
     std::is_same<typename T::value_type,
                  std::pair<typename T::key_type,
                            typename T::mapped_type>>::value) &&
    !IsNontrivialSerializable<T,BlobEncoder>::value
  >::type encode(const T& map) {
    encodeContainer(map, "map");
  }

  template<class T>
  typename std::enable_if<
    std::is_same<typename T::key_type,
                 typename T::value_type>::value &&
    !IsNontrivialSerializable<T,BlobEncoder>::value
  >::type encode(const T& set) {
    encodeContainer(set, "set");
  }

  template<class T, class... F>
  BlobEncoder& operator()(const T& t, F... lambdas) {
    encode(t, lambdas...);
    return *this;
  }

  /*
   * Record the size of the data emitted during f(), which
   * BlobDecoder::skipSize or BlobDecoder::peekSize can later read.
   */
  template <typename F>
  BlobEncoder& withSize(F f) {
    uint64_t start = m_blob.size();
    m_blob.resize(start + sizeof(uint64_t));
    // The size is stored before the data, but we don't know it yet,
    // so store 0 and then patch it afterwards (this means we have to
    // used a fixed size encoding).
    std::memset(&m_blob[start], 0, sizeof(uint64_t));
    f();
    uint64_t size = m_blob.size() - start;
    assertx(size >= sizeof(uint64_t));
    size -= sizeof(uint64_t);
    std::copy((char*)&size, (char*)&size + sizeof(uint64_t), &m_blob[start]);
    return *this;
  }

  size_t size() const { return m_blob.size(); }
  const void* data() const { return &m_blob[0]; }

  std::vector<char>&& take() { return std::move(m_blob); }

private:
  template<class Cont>
  void encodeContainer(Cont& cont, const char* desc) {
    if (cont.size() >= 0xffffffffu) {
      throw std::runtime_error("maximum " + std::string(desc) +
        " size exceeded in BlobEncoder");
    }
    encode(uint32_t(cont.size()));
    typedef typename Cont::const_iterator iter;
    for (iter it = cont.begin(); it != cont.end(); ++it) {
      encode(*it);
    }
  }

private:
  std::vector<char> m_blob;
  bool m_useGlobalIds;
};

//////////////////////////////////////////////////////////////////////

struct BlobDecoder {
  static const bool deserializing = true;

  BlobDecoder(const void* vp, size_t sz, bool l)
    : m_start{static_cast<const unsigned char*>(vp)}
    , m_p{m_start}
    , m_last{m_p + sz}
    , m_useGlobalIds{l}
  {}

  void assertDone() {
    assertx(m_p >= m_last);
  }

  const unsigned char* data() const { return m_p; }
  void advance(size_t s) { m_p += s; }

  size_t remaining() const {
    return m_p <= m_last ? (m_last - m_p) : 0;
  }

  size_t advanced() const { return m_p - m_start; }

  // See encode() in BlobEncoder for why this only allows integral
  // types.
  template<class T>
  typename std::enable_if<
    std::is_integral<T>::value && std::is_signed<T>::value
  >::type
  decode(T& t) {
    uint64_t value;
    decode(value);
    t = static_cast<T>(folly::decodeZigZag(value));
  }

  template<class T>
  typename std::enable_if<
    (std::is_integral<T>::value && !std::is_signed<T>::value) ||
    std::is_enum<T>::value
  >::type
  decode(T& t) {
    folly::ByteRange range(m_p, m_last);
    t = static_cast<T>(folly::decodeVarint(range));
    m_p = range.begin();
  }

  template<class T, class... F>
  typename std::enable_if<
    IsNontrivialSerializable<T,BlobDecoder, F...>::value
  >::type decode(T& t, F... lambdas) {
    t.serde(*this, lambdas...);
  }

  void decode(SHA1& sha1) {
    for (auto& w : sha1.q) decode(w);
  }

  void decode(Location::Range& loc) {
    int line0, char0, line1, char1;
    decode(line0);
    decode(char0);
    decode(line1);
    decode(char1);
    loc = {line0, char0, line1, char1};
  }

  void decode(DataType& t) {
    // always decode DataType as int8 even if it's a bigger size.
    int8_t t2;
    decode(t2);
    t = DataType(t2);
  }

  void decode(const StringData*& sd) {
    String s(decodeString());
    sd = s.get() ? makeStaticString(s) : 0;
  }

  void decode(LowStringPtr& s) {
    if (m_useGlobalIds) {
      Id id;
      decode(id);
      s = LitstrTable::get().lookupLitstrId(id);
      return;
    }

    String st(decodeString());
    s = st.get() ? makeStaticString(st) : 0;
  }

  void decode(std::string& s) {
    uint32_t sz;
    decode(sz);
    assertx(m_last - m_p >= sz);
    s = std::string{m_p, m_p + sz};
    m_p += sz;
  }

  size_t peekStdStringSize() {
    auto const before = advanced();
    uint32_t sz;
    decode(sz);
    auto const sizeBytes = advanced() - before;
    m_p -= sizeBytes;
    return sz + sizeBytes;
  }

  void decode(TypedValue& tv) {
    tvWriteUninit(tv);

    String s = decodeString();
    assertx(!!s);
    if (s.empty()) return;

    tvAsVariant(&tv) =
      unserialize_from_string(s, VariableUnserializer::Type::Internal);
    tvAsVariant(&tv).setEvalScalar();
  }

  void decode(TypedValueAux& tv) = delete;

  template<class T>
  void decode(folly::Optional<T>& opt) {
    bool some;
    decode(some);

    if (!some) {
      opt = folly::none;
    } else {
      T value;
      decode(value);
      opt = value;
    }
  }

  template <size_t I = 0, typename... Ts>
  typename std::enable_if<I == sizeof...(Ts), void>::type
  decode(std::tuple<Ts...>& /*val*/) {}

  template<size_t I = 0, typename ...Ts>
  typename std::enable_if<I < sizeof...(Ts), void>::type
  decode(std::tuple<Ts...>& val) {
    decode(std::get<I>(val));
    decode<I + 1, Ts...>(val);
  }

  template<class K, class V>
  void decode(std::pair<K,V>& val) {
    decode(val.first);
    decode(val.second);
  }

  template<typename Cont>
  auto decode(Cont& vec) -> decltype(vec.emplace_back(), void()) {
    uint32_t size;
    decode(size);
    if (size) vec.reserve(vec.size() + size);
    for (uint32_t i = 0; i < size; ++i) {
      vec.emplace_back();
      decode(vec.back());
    }
  }

  template<class T, typename FDeltaDecode>
  void decode(std::vector<T>& vec, FDeltaDecode deltaDecode) {
    uint32_t size;
    decode(size);
    vec.reserve(size);

    auto prev = T{};
    auto delta = T{};
    for (uint32_t i = 0; i < size; ++i) {
      decode(delta);
      prev = deltaDecode(prev, delta);
      vec.push_back(prev);
    }
  }

  template<class T>
  typename std::enable_if<
    (std::is_same<typename T::value_type,
                  std::pair<typename T::key_type const,
                            typename T::mapped_type>>::value ||
     std::is_same<typename T::value_type,
                  std::pair<typename T::key_type,
                            typename T::mapped_type>>::value) &&
    !IsNontrivialSerializable<T,BlobDecoder>::value
  >::type decode(T& map) {
    uint32_t size;
    decode(size);
    for (uint32_t i = 0; i < size; ++i) {
      typename T::key_type key;
      decode(key);
      typename T::mapped_type val;
      decode(val);
      map.emplace(key, val);
    }
  }

  template<class T>
  typename std::enable_if<
    std::is_same<typename T::key_type,
                 typename T::value_type>::value &&
    !IsNontrivialSerializable<T,BlobDecoder>::value
  >::type decode(T& set) {
    uint32_t size;
    decode(size);
    for (uint32_t i = 0; i < size; ++i) {
      typename T::value_type val;
      decode(val);
      set.insert(val);
    }
  }

  template<class T, class... F>
  BlobDecoder& operator()(T& t, F... lambdas) {
    decode(t, lambdas...);
    return *this;
  }

  /*
   * Read a block of data (using f()) which is previously encoded with
   * BlobEncoder::withSize.
   */
  template <typename F>
  BlobDecoder& withSize(F f) {
    // Since we're going to read the data anyways, we don't actually
    // need the size, but we'll assert if it doesn't match what we
    // decode.
    assertx(remaining() >= sizeof(uint64_t));
    uint64_t size;
    std::copy(m_p, m_p + sizeof(uint64_t), (unsigned char*)&size);
    advance(sizeof(uint64_t));
    auto const DEBUG_ONLY before = remaining();
    assertx(before >= size);
    f();
    assertx(before - remaining() == size);
    return *this;
  }

  /*
   * Skip over a block of data encoded with BlobEncoder::withSize.
   */
  BlobDecoder& skipWithSize() {
    assertx(remaining() >= sizeof(uint64_t));
    uint64_t size;
    std::copy(m_p, m_p + sizeof(uint64_t), (unsigned char*)&size);
    assertx(remaining() >= size + sizeof(uint64_t));
    advance(sizeof(uint64_t) + size);
    return *this;
  }

  /*
   * Read the size encoded by BlobEncoder::withSize, without advancing
   * the decoder state.
   */
  size_t peekSize() const {
    assertx(remaining() >= sizeof(uint64_t));
    uint64_t size;
    std::copy(m_p, m_p + sizeof(uint64_t), (unsigned char*)&size);
    return size + sizeof(uint64_t);
  }

  /*
   * Skip over an encoded StringData*
   */
  BlobDecoder& skipString() {
    uint32_t sz;
    decode(sz);
    if (sz < 2) return *this;
    --sz;
    assertx(remaining() >= sz);
    advance(sz);
    return *this;
  }

  /*
   * Skip over an encoded std::string
   */
  BlobDecoder& skipStdString() {
    uint32_t sz;
    decode(sz);
    if (sz < 1) return *this;
    assertx(remaining() >= sz);
    advance(sz);
    return *this;
  }

  void setUseGlobalIds(bool b) { m_useGlobalIds = b; }

private:
  String decodeString() {
    uint32_t sz;
    decode(sz);
    if (sz == 0) return String();
    sz--;
    if (sz == 0) return empty_string();

    String s = String(sz, ReserveString);
    char* pch = s.mutableData();
    assertx(m_last - m_p >= sz);
    std::copy(m_p, m_p + sz, pch);
    m_p += sz;
    s.setSize(sz);
    return s;
  }

private:
  const unsigned char* m_start;
  const unsigned char* m_p;
  const unsigned char* const m_last;
  bool m_useGlobalIds;
};

//////////////////////////////////////////////////////////////////////

}

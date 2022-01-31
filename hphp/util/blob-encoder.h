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

#include "hphp/util/compact-vector.h"
#include "hphp/util/optional.h"

#include <folly/sorted_vector_types.h>
#include <folly/Varint.h>

#include <set>
#include <type_traits>
#include <unordered_map>
#include <vector>

/*
 * This module contains helpers for serializing and deserializing
 * metadata into binary blobs.
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
 * If you cannot add a serde member function to the type (for example,
 * for pointers), you can get the same effect by defining a
 * specialization of BlobEncoderHelper with the appropriate member(s).
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
 * This defines a trait that determines whether or not a type T has a
 * member function template called `serde' that specializes for the
 * type `SerDe'.  The point here is that calling one of the blob
 * helpers on an object that knows how to do its own serialization
 * will just reflect back to it.
 */

template<typename T, typename SerDe, typename... F>
struct IsNontrivialSerializable {
private:
  using yes = std::true_type;
  using no = std::false_type;
  template<typename U> static auto test(int) ->
    decltype(
      std::declval<U>().serde(
        std::declval<SerDe&>(),
        std::declval<F>()...),
      yes()
    );
  template<typename> static no test(...);
public:
  static constexpr bool value = std::is_same<decltype(test<T>(0)),yes>::value;
};

//////////////////////////////////////////////////////////////////////

/*
 * Like IsNontrivialSerializable, but checks for the presense of a
 * static makeForSerde function (with a matching signature) in the
 * type T.
 */
template<typename T, typename SerDe>
struct HasMakeForSerde {
private:
  using yes = std::true_type;
  using no = std::false_type;
  template<typename U> static auto test(int) ->
    decltype(U::makeForSerde(std::declval<SerDe&>()), yes());
  template<typename> static no test(...);
public:
  static constexpr bool value = std::is_same<decltype(test<T>(0)),yes>::value;
};

//////////////////////////////////////////////////////////////////////

template <typename T> struct BlobEncoderHelper {};

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

  template<typename T, typename... F>
  typename std::enable_if<
    IsNontrivialSerializable<BlobEncoderHelper<T>, BlobEncoder, T&, F...>::value
  >::type encode(const T& t, F... lambdas) {
    BlobEncoderHelper<T>::serde(*this, const_cast<T&>(t), lambdas...);
  }

  // Avoid pointer to integer conversions
  template<typename T, typename... F>
  typename std::enable_if<
    !IsNontrivialSerializable<BlobEncoderHelper<T>, BlobEncoder,
                              T&, F...>::value &&
    std::is_pointer<T>::value
  >::type encode(const T&, F...) = delete;

  void encode(bool b) {
    encode(b ? 1 : 0);
  }

  void encode(const std::string& s) {
    uint32_t sz = s.size();
    encode(sz);
    if (!sz) return;
    const size_t start = m_blob.size();
    m_blob.resize(start + sz);
    std::copy(s.data(), s.data() + sz, &m_blob[start]);
  }

  template<class T>
  void encode(const Optional<T>& opt) {
    const bool some = opt.has_value();
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

  template<typename T, typename A>
  void encode(const std::vector<T, A>& vec) {
    encodeOrderedContainer(vec);
  }

  template<typename T, typename A>
  void encode(const CompactVector<T, A>& vec) {
    encodeOrderedContainer(vec);
  }

  template<typename K, typename C, typename A>
  void encode(const std::set<K, C, A>& set) {
    encodeOrderedContainer(set);
  }

  template<typename K, typename V,
           typename C, typename A,
           typename G, typename C2>
  void encode(const folly::sorted_vector_map<K, V, C, A, G, C2>& map) {
    encodeOrderedContainer(map);
  }

  template<typename K, typename V, typename C, typename A>
  void encode(const std::map<K, V, C, A>& map) {
    encodeOrderedContainer(map);
  }

  template<typename K, typename V, typename H, typename E, typename A>
  void encode(const std::unordered_map<K, V, H, E, A>& map) {
    encodeUnorderedContainer(map);
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

  bool usesGlobalIds() const { return m_useGlobalIds; }

  size_t size() const { return m_blob.size(); }
  const void* data() const { return m_blob.data(); }

  std::vector<char>&& take() { return std::move(m_blob); }

private:
  template<typename Cont>
  void encodeOrderedContainer(const Cont& cont) {
    if (cont.size() >= 0xffffffffu) {
      throw std::runtime_error("maximum size exceeded in BlobEncoder");
    }
    encode(uint32_t(cont.size()));
    for (auto const& e : cont) encode(e);
  }

  // Unordered containers need to be sorted first, to ensure
  // deterministic output.
  template<typename Cont>
  void encodeUnorderedContainer(const Cont& cont) {
    if (cont.size() >= 0xffffffffu) {
      throw std::runtime_error("maximum size exceeded in BlobEncoder");
    }

    std::vector<typename Cont::key_type> keys;
    keys.reserve(cont.size());
    for (auto const& e : cont) keys.emplace_back(e.first);
    std::sort(keys.begin(), keys.end());

    encode(uint32_t(keys.size()));
    for (auto const& k : keys) {
      auto const it = cont.find(k);
      assertx(it != cont.end());
      encode(*it);
    }
  }

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
  void advance(size_t s) {
    assertx(remaining() >= s);
    m_p += s;
  }

  size_t remaining() const {
    return m_p <= m_last ? (m_last - m_p) : 0;
  }

  size_t advanced() const { return m_p - m_start; }

  // Produce a value of type T from the decoder. Uses a specialized
  // creation function if available, otherwise just default constructs
  // the value and calls the decoder on it.
  template <typename T> T make() {
    if constexpr (HasMakeForSerde<T, BlobDecoder>::value) {
      return T::makeForSerde(*this);
    } else {
      T t;
      (*this)(t);
      return t;
    }
  }

  // Like make(), except asserts the decoder is done afterwards.
  template <typename T> T makeWhole() {
    if constexpr (HasMakeForSerde<T, BlobDecoder>::value) {
      auto t = T::makeForSerde(*this);
      assertDone();
      return t;
    } else {
      T t;
      (*this)(t);
      assertDone();
      return t;
    }
  }

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

  template<typename T, typename... F>
  typename std::enable_if<
    IsNontrivialSerializable<BlobEncoderHelper<T>, BlobDecoder, T&, F...>::value
  >::type decode(T& t, F... lambdas) {
    BlobEncoderHelper<T>::serde(*this, t, lambdas...);
  }

  // Avoid pointer to integer conversions
  template<typename T, typename... F>
  typename std::enable_if<
    !IsNontrivialSerializable<BlobEncoderHelper<T>, BlobDecoder,
                              T&, F...>::value &&
    std::is_pointer<T>::value
  >::type decode(T&, F...) = delete;

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

  template<class T>
  void decode(Optional<T>& opt) {
    bool some;
    decode(some);

    if (!some) {
      opt = std::nullopt;
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

  template<typename T, typename A>
  void decode(std::vector<T, A>& vec) {
    decodeVecContainer(vec);
  }

  template<typename T, typename A>
  void decode(CompactVector<T, A>& vec) {
    decodeVecContainer(vec);
  }

  template<typename K, typename C, typename A>
  void decode(std::set<K, C, A>& set) {
    decodeSetContainer(set);
  }

  template<typename K, typename V,
           typename C, typename A,
           typename G, typename C2>
  void decode(folly::sorted_vector_map<K, V, C, A, G, C2>& map) {
    decodeMapContainer(map);
  }

  template<typename K, typename V, typename C, typename A>
  void decode(std::map<K, V, C, A>& map) {
    decodeMapContainer(map);
  }

  template<typename K, typename V, typename H, typename E, typename A>
  void decode(std::unordered_map<K, V, H, E, A>& map) {
    decodeMapContainer(map);
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

  bool usesGlobalIds() const { return m_useGlobalIds; }
  void setUseGlobalIds(bool b) { m_useGlobalIds = b; }

private:
  template<typename Cont>
  void decodeMapContainer(Cont& cont) {
    cont.clear();
    uint32_t size;
    decode(size);
    for (uint32_t i = 0; i < size; ++i) {
      // Cont::value_type typically has a const key, so we cannot use
      // it directly
      std::pair<typename Cont::key_type, typename Cont::mapped_type> val;
      decode(val);
      cont.emplace(std::move(val));
    }
  }

  template<typename Cont>
  void decodeSetContainer(Cont& cont) {
    cont.clear();
    uint32_t size;
    decode(size);
    for (uint32_t i = 0; i < size; ++i) {
      typename Cont::value_type val;
      decode(val);
      cont.emplace(std::move(val));
    }
  }

  template<typename Cont>
  void decodeVecContainer(Cont& cont) {
    cont.clear();
    uint32_t size;
    decode(size);
    for (uint32_t i = 0; i < size; ++i) {
      typename Cont::value_type val;
      decode(val);
      cont.emplace_back(std::move(val));
    }
  }

  const unsigned char* m_start;
  const unsigned char* m_p;
  const unsigned char* const m_last;
  bool m_useGlobalIds;
};

//////////////////////////////////////////////////////////////////////

}

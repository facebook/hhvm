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
#include "hphp/util/copy-ptr.h"
#include "hphp/util/insertion-ordered-map.h"
#include "hphp/util/optional.h"
#include "hphp/util/tiny-vector.h"

#include <folly/sorted_vector_types.h>
#include <folly/Varint.h>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>

#include <filesystem>
#include <memory>
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
 *    struct MyBlobbableStuff {
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
 * Like IsNontrivialSerializable, but checks for the presence of a
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

template <typename T>
auto maybe_reserve(T* t, size_t s) -> std::void_t<decltype(t->reserve(s))> {
  t->reserve(s);
}
inline void maybe_reserve(...) {}

template <typename T>
auto maybe_shrink(T* t) -> std::void_t<decltype(t->shrink_to_fit())> {
  t->shrink_to_fit();
}
inline void maybe_shrink(...) {}

//////////////////////////////////////////////////////////////////////

template <typename T> struct BlobEncoderHelper {};

//////////////////////////////////////////////////////////////////////

struct BlobEncoder {
  static const bool deserializing = false;
  BlobEncoder() = default;

  void writeRaw(const char* ptr, size_t size) {
    auto const start = m_blob.size();
    m_blob.resize(start + size);
    std::copy(ptr, ptr + size, m_blob.data() + start);
  }

  /*
   * Currently the most basic encoder/decode only works for integral
   * types.  (We don't want this to accidentally get used for things
   * like pointers or aggregates.)
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

  void encode(double d) {
    writeRaw(reinterpret_cast<const char*>(&d), sizeof(double));
  }

  void encode(const std::string& s) {
    uint32_t sz = s.size();
    encode(sz);
    if (!sz) return;
    const size_t start = m_blob.size();
    m_blob.resize(start + sz);
    std::copy(s.data(), s.data() + sz, &m_blob[start]);
  }

  void encode(const std::filesystem::path& p) {
    encode(p.string());
  }

  template<unsigned long N>
  void encode(const std::bitset<N>& b) {
    static_assert(N <= std::numeric_limits<unsigned long long>::digits,
                  "SerDe of bitsets longer than 64-bits not yet supported");
    encode(b.to_ullong());
  }

  template<class T, typename... Extra>
  void encode(const Optional<T>& opt, const Extra&... extra) {
    auto const some = opt.has_value();
    encode(some);
    if (some) encode(*opt, extra...);
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

  template<typename T, typename A, typename... Extra>
  void encode(const std::vector<T, A>& vec, const Extra&... extra) {
    encodeOrderedContainer(vec, extra...);
  }

  template<typename T, typename A, typename... Extra>
  void encode(const CompactVector<T, A>& vec, const Extra&... extra) {
    encodeOrderedContainer(vec, extra...);
  }

  template<typename T, size_t S, size_t M, typename A, typename... Extra>
  void encode(const TinyVector<T, S, M, A>& vec, const Extra&... extra) {
    encodeOrderedContainer(vec, extra...);
  }

  template<typename K, typename C, typename A>
  void encode(const std::set<K, C, A>& set) {
    encodeOrderedContainer(set);
  }

  template<typename T, typename H, typename E, typename A, typename C,
           typename... Extra>
  void encode(const folly::F14NodeSet<T, H, E, A>& set, const C& c,
              const Extra&... extra) {
    encodeUnorderedSet(set, c, extra...);
  }

  template<typename T, typename H, typename E, typename A, typename C,
           typename... Extra>
  void encode(const folly::F14VectorSet<T, H, E, A>& set, const C& c,
              const Extra&... extra) {
    encodeUnorderedSet(set, c, extra...);
  }

  template<typename T, typename H, typename E, typename A, typename C,
           typename... Extra>
  void encode(const folly::F14ValueSet<T, H, E, A>& set, const C& c,
              const Extra&... extra) {
    encodeUnorderedSet(set, c, extra...);
  }

  template<typename T, typename H, typename E, typename A, typename C,
           typename... Extra>
  void encode(const folly::F14FastSet<T, H, E, A>& set, const C& c,
              const Extra&... extra) {
    encodeUnorderedSet(set, c, extra...);
  }

  template<typename K, typename V,
           typename C, typename A,
           typename G, typename C2>
  void encode(const folly::sorted_vector_map<K, V, C, A, G, C2>& map) {
    encodeOrderedContainer(map);
  }

  template <typename K, typename V, typename H, typename E>
  void encode(const InsertionOrderedMap<K, V, H, E>& map) {
    encodeOrderedContainer(map);
  }

  template<typename K, typename V, typename C, typename A>
  void encode(const std::map<K, V, C, A>& map) {
    encodeOrderedContainer(map);
  }

  template<typename K, typename V, typename H, typename E, typename A,
           typename C, typename... Extra>
  void encode(const std::unordered_map<K, V, H, E, A>& map, const C& c,
              const Extra&... extra) {
    encodeUnorderedMap(map, c, extra...);
  }

  template<typename K, typename V, typename H, typename E, typename A,
           typename C, typename... Extra>
  void encode(const folly::F14NodeMap<K, V, H, E, A>& map, const C& c,
              const Extra&... extra) {
    encodeUnorderedMap(map, c, extra...);
  }

  template<typename K, typename V, typename H, typename E, typename A,
           typename C, typename... Extra>
  void encode(const folly::F14VectorMap<K, V, H, E, A>& map, const C& c,
              const Extra&... extra) {
    encodeUnorderedMap(map, c, extra...);
  }

  template<typename K, typename V, typename H, typename E, typename A,
           typename C, typename... Extra>
  void encode(const folly::F14ValueMap<K, V, H, E, A>& map, const C& c,
              const Extra&... extra) {
    encodeUnorderedMap(map, c, extra...);
  }

  template<typename K, typename V, typename H, typename E, typename A,
           typename C, typename... Extra>
  void encode(const folly::F14FastMap<K, V, H, E, A>& map, const C& c,
              const Extra&... extra) {
    encodeUnorderedMap(map, c, extra...);
  }

  template<class T, class... F>
  BlobEncoder& operator()(const T& t, F... lambdas) {
    encode(t, lambdas...);
    return *this;
  }

  template<typename T, typename FDeltaEncode>
  BlobEncoder& delta(const std::vector<T>& cont, FDeltaEncode deltaEncode) {
    if (cont.size() >= 0xffffffffu) {
      throw std::runtime_error("maximum vector size exceeded in BlobEncoder");
    }

    auto prev = T{};
    encode(uint32_t(cont.size()));
    for (auto it = cont.begin(); it != cont.end(); ++it) {
      encode(deltaEncode(prev, *it));
      prev = *it;
    }
    return *this;
  }

  // Encode a type which has a conversion to nullptr, skipping the
  // encoding if it is nullptr. This is mainly meant for smart pointer
  // types where the pointer may not be set.
  template <typename T>
  BlobEncoder& nullable(const T& t) {
    if (t) {
      (*this)(true);
      (*this)(t);
    } else {
      (*this)(false);
    }
    return *this;
  }

  template <typename T>
  BlobEncoder& sharedPtr(const std::shared_ptr<T>& p) {
    if (p) {
      (*this)(true);
      (*this)(*p);
    } else {
      (*this)(false);
    }
    return *this;
  }

  /*
   * Record the size of the data emitted during f(), which
   * BlobDecoder::skipSize or BlobDecoder::peekSize can later read.
   */
  template <typename F>
  BlobEncoder& withSize(const F& f) {
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

  /* Like withSize, but uses 32-bit size */
  template <typename F>
  BlobEncoder& withSize32(const F& f) {
    uint64_t start = m_blob.size();
    m_blob.resize(start + sizeof(uint32_t));
    // The size is stored before the data, but we don't know it yet,
    // so store 0 and then patch it afterwards (this means we have to
    // used a fixed size encoding).
    std::memset(&m_blob[start], 0, sizeof(uint32_t));
    f();
    uint64_t size = m_blob.size() - start;
    assertx(size <= std::numeric_limits<uint32_t>::max());
    assertx(size >= sizeof(uint32_t));
    size -= sizeof(uint32_t);
    uint32_t s = size;
    std::copy((char*)&s, (char*)&s + sizeof(uint32_t), &m_blob[start]);
    return *this;
  }

  template <typename T>
  BlobEncoder& fixedWidth(T offset) {
    uint64_t start = m_blob.size();
    m_blob.resize(start + sizeof(T));
    std::copy((char*)&offset, (char*)&offset + sizeof(T), &m_blob[start]);
    return *this;
  }

  /*
   * Run f() to encode N items. The number of encoded items is
   * returned from f() and encoded before the items. This can be used
   * to encode a list without knowing the size of the list before
   * hand.
   *
   * The list encoded with lazyCount can be read by
   * BlobDecoder::readWithLazyCount.
   */
  template <typename F>
  BlobEncoder& lazyCount(const F& f) {
    uint64_t start = m_blob.size();
    m_blob.resize(start + sizeof(uint32_t));
    std::memset(&m_blob[start], 0, sizeof(uint32_t));
    uint32_t count = f();
    std::copy((char*)&count, (char*)&count + sizeof(uint32_t), &m_blob[start]);
    return *this;
  }

  // Run f1 to encode data, then run f2 to encode more data. The data
  // encoded by f1 is encoded with a size prefix (using withSize), so
  // that it can be skipped. This is meant to be paired with
  // BlobDecoder::alternate, where the data will be decoded in
  // opposite order.
  template <typename F1, typename F2>
  BlobEncoder& alternate(const F1& f1, const F2& f2) {
    withSize(f1);
    f2();
    return *this;
  }

  size_t size() const { return m_blob.size(); }
  const void* data() const { return m_blob.data(); }

  std::vector<char>&& take() { return std::move(m_blob); }

private:
  template<typename Cont, typename... Extra>
  void encodeOrderedContainer(const Cont& cont, Extra... extra) {
    if (cont.size() >= 0xffffffffu) {
      throw std::runtime_error("maximum size exceeded in BlobEncoder");
    }
    encode(uint32_t(cont.size()));
    for (auto const& e : cont) encode(e, extra...);
  }

  // Unordered containers need to be sorted first, to ensure
  // deterministic output.
  template<typename Cont, typename Cmp, typename... Extra>
  void encodeUnorderedMap(const Cont& cont, const Cmp& cmp,
                         const Extra&... extra) {
    if (cont.size() >= 0xffffffffu) {
      throw std::runtime_error("maximum size exceeded in BlobEncoder");
    }

    std::vector<typename Cont::key_type> keys;
    keys.reserve(cont.size());
    for (auto const& e : cont) keys.emplace_back(e.first);
    std::sort(keys.begin(), keys.end(), cmp);

    encode(uint32_t(keys.size()));
    for (auto const& k : keys) {
      auto const it = cont.find(k);
      assertx(it != cont.end());
      encode(it->first);
      encode(it->second, extra...);
    }
  }

  template<typename Cont, typename Cmp, typename... Extra>
  void encodeUnorderedSet(const Cont& cont, const Cmp& cmp,
                          const Extra&... extra) {
    if (cont.size() >= 0xffffffffu) {
      throw std::runtime_error("maximum size exceeded in BlobEncoder");
    }

    std::vector<typename Cont::key_type> keys;
    keys.reserve(cont.size());
    for (auto const& e : cont) keys.emplace_back(e);
    std::sort(keys.begin(), keys.end(), cmp);

    encode(uint32_t(keys.size()));
    for (auto const& k : keys) encode(k, extra...);
  }

  std::vector<char> m_blob;
};

//////////////////////////////////////////////////////////////////////

struct BlobDecoder {
  static const bool deserializing = true;

  BlobDecoder(const void* vp, size_t sz)
    : m_start{static_cast<const unsigned char*>(vp)}
    , m_p{m_start}
    , m_last{m_p + sz}
  {}

  void assertDone() {
    assertx(m_p >= m_last);
  }

  const unsigned char* start() const { return m_start; }
  const unsigned char* end() const { return m_last; }

  const unsigned char* data() const { return m_p; }

  void advance(size_t s) {
    assertx(remaining() >= s);
    m_p += s;
  }

  void retreat(size_t s) {
    assertx(advanced() >= s);
    m_p -= s;
  }

  size_t remaining() const {
    return m_p <= m_last ? (m_last - m_p) : 0;
  }

  size_t advanced() const { return m_p - m_start; }

  // Produce a value of type T from the decoder. Uses a specialized
  // creation function if available, otherwise just default constructs
  // the value and calls the decoder on it.
  template <typename T, typename... Extra> T make(const Extra&... extra) {
    if constexpr (HasMakeForSerde<T, BlobDecoder>::value) {
      return T::makeForSerde(*this, extra...);
    } else {
      T t;
      (*this)(t, extra...);
      return t;
    }
  }

  // Like make(), except asserts the decoder is done afterwards.
  template <typename T, typename... Extra> T makeWhole(const Extra&... extra) {
    if constexpr (HasMakeForSerde<T, BlobDecoder>::value) {
      auto t = T::makeForSerde(*this, extra...);
      assertDone();
      return t;
    } else {
      T t;
      (*this)(t, extra...);
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

  void decode(double& d) {
    assertx(remaining() >= sizeof(double));
    std::memcpy(&d, data(), sizeof(double));
    advance(sizeof(double));
  }

  void decode(std::string& s) {
    uint32_t sz;
    decode(sz);
    assertx(m_last - m_p >= sz);
    s = std::string{m_p, m_p + sz};
    m_p += sz;
  }

  void decode(std::filesystem::path& p) {
    std::string s;
    decode(s);
    p = std::filesystem::path(std::move(s));
  }

  template<unsigned long N>
  void decode(std::bitset<N>& val) {
    static_assert(N <= std::numeric_limits<unsigned long long>::digits,
                  "SerDe of bitsets longer than 64-bits not yet supported");
    unsigned long long v;
    decode(v);
    val = v;
  }

  size_t peekStdStringSize() {
    auto const before = advanced();
    uint32_t sz;
    decode(sz);
    auto const sizeBytes = advanced() - before;
    m_p -= sizeBytes;
    return sz + sizeBytes;
  }

  template<class T, typename... Extra>
  void decode(Optional<T>& opt, Extra... extra) {
    bool some;
    decode(some);

    if (!some) {
      opt = std::nullopt;
    } else {
      opt = make<T>(extra...);
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

  template<typename T, typename A, typename... Extra>
  void decode(std::vector<T, A>& vec, Extra... extra) {
    decodeVecContainer(vec, extra...);
  }

  template<typename T, typename A, typename... Extra>
  void decode(CompactVector<T, A>& vec, Extra... extra) {
    decodeVecContainer(vec, extra...);
  }

  template<typename T, size_t S, size_t M, typename A, typename... Extra>
  void decode(TinyVector<T, S, M, A>& vec, Extra... extra) {
    decodeVecContainer(vec, extra...);
  }

  template<typename K, typename C, typename A>
  void decode(std::set<K, C, A>& set) {
    decodeOrderedSetContainer(set);
  }

  template<typename T, typename H, typename E, typename A, typename C,
           typename... Extra>
  void decode(folly::F14NodeSet<T, H, E, A>& set, const C&,
              const Extra&... extra) {
    decodeSetContainer(set, extra...);
  }

  template<typename T, typename H, typename E, typename A, typename C,
           typename... Extra>
  void decode(folly::F14VectorSet<T, H, E, A>& set, const C&,
              const Extra&... extra) {
    decodeSetContainer(set, extra...);
  }

  template<typename T, typename H, typename E, typename A, typename C,
           typename... Extra>
  void decode(folly::F14ValueSet<T, H, E, A>& set, const C&,
              const Extra&... extra) {
    decodeSetContainer(set, extra...);
  }

  template<typename T, typename H, typename E, typename A, typename C,
           typename... Extra>
  void decode(folly::F14FastSet<T, H, E, A>& set, const C&,
              const Extra&... extra) {
    decodeSetContainer(set, extra...);
  }

  template<typename K, typename V,
           typename C, typename A,
           typename G, typename C2>
  void decode(folly::sorted_vector_map<K, V, C, A, G, C2>& map) {
    decodeMapContainer(map);
  }

  template <typename K, typename V, typename H, typename E>
  void decode(InsertionOrderedMap<K, V, H, E>& map) {
    decodeMapContainer(map);
  }

  template<typename K, typename V, typename C, typename A>
  void decode(std::map<K, V, C, A>& map) {
    decodeMapContainer(map);
  }

  template<typename K, typename V, typename H, typename E, typename A,
           typename C, typename... Extra>
  void decode(std::unordered_map<K, V, H, E, A>& map, const C&,
              const Extra&... extra) {
    decodeMapContainer(map, extra...);
  }

  template<typename K, typename V, typename H, typename E, typename A,
           typename C, typename... Extra>
  void decode(folly::F14NodeMap<K, V, H, E, A>& map, const C&,
              const Extra&... extra) {
    decodeMapContainer(map, extra...);
  }

  template<typename K, typename V, typename H, typename E, typename A,
           typename C, typename... Extra>
  void decode(folly::F14VectorMap<K, V, H, E, A>& map, const C&,
              const Extra&... extra) {
    decodeMapContainer(map, extra...);
  }

  template<typename K, typename V, typename H, typename E, typename A,
           typename C, typename... Extra>
  void decode(folly::F14ValueMap<K, V, H, E, A>& map, const C&,
              const Extra&... extra) {
    decodeMapContainer(map, extra...);
  }

  template<typename K, typename V, typename H, typename E, typename A,
           typename C, typename... Extra>
  void decode(folly::F14FastMap<K, V, H, E, A>& map, const C&,
              const Extra&... extra) {
    decodeMapContainer(map, extra...);
  }

  template<class T, class... F>
  BlobDecoder& operator()(T& t, F... lambdas) {
    decode(t, lambdas...);
    return *this;
  }

  template<class T, typename FDeltaDecode>
  BlobDecoder& delta(std::vector<T>& vec, FDeltaDecode deltaDecode) {
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
    vec.shrink_to_fit();
    return *this;
  }

  // Decode a type encoded by BlobEncoder::nullable. If the value was
  // not encoded, it will be set to nullptr.
  template <typename T>
  BlobDecoder& nullable(T& t) {
    bool present;
    (*this)(present);
    if (present) {
      (*this)(t);
    } else {
      t = nullptr;
    }
    return *this;
  }

  template <typename T>
  BlobDecoder& sharedPtr(std::shared_ptr<T>& p) {
    bool present;
    (*this)(present);
    if (present) {
      p = std::make_shared<T>(make<T>());
    } else {
      p.reset();
    }
    return *this;
  }

  /*
   * Read a block of data (using f()) which is previously encoded with
   * BlobEncoder::withSize.
   */
  template <typename F>
  BlobDecoder& withSize(const F& f) {
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

  template <typename F>
  BlobDecoder& withSize32(const F& f) {
    // Since we're going to read the data anyways, we don't actually
    // need the size, but we'll assert if it doesn't match what we
    // decode.
    assertx(remaining() >= sizeof(uint32_t));
    uint32_t size;
    std::copy(m_p, m_p + sizeof(uint32_t), (unsigned char*)&size);
    advance(sizeof(uint32_t));
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

  /* Like skipWithSize, but for BlobEncoder::withSize32 */
  BlobDecoder& skipWithSize32() {
    assertx(remaining() >= sizeof(uint32_t));
    uint32_t size;
    std::copy(m_p, m_p + sizeof(uint32_t), (unsigned char*)&size);
    assertx(remaining() >= size + sizeof(uint32_t));
    advance(sizeof(uint32_t) + size);
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

  /* Like peekSize, but for BlobEncoder::withSize32 */
  size_t peekSize32() const {
    assertx(remaining() >= sizeof(uint32_t));
    uint32_t size;
    std::copy(m_p, m_p + sizeof(uint32_t), (unsigned char*)&size);
    return size + sizeof(uint32_t);
  }

  template <typename T>
  void fixedWidth(T& offset) {
    assertx(remaining() >= sizeof(T));
    std::memcpy(&offset, data(), sizeof(T));
    advance(sizeof(T));
  }

  /*
   * Read a list encoded with BlobEncoder::lazyCount. f() will be
   * called N times, where N is the encoded list size.
   */
  template <typename F>
  BlobDecoder& readWithLazyCount(const F& f) {
    assertx(remaining() >= sizeof(uint32_t));
    uint32_t count;
    std::copy(m_p, m_p + sizeof(uint32_t), (unsigned char*)&count);
    advance(sizeof(uint32_t));
    for (size_t i = 0; i < count; ++i) f();
    return *this;
  }

  // Decode data encoded by BlobEncoder::alternate. First the data
  // encoded by f2 is decoded (this involves skipping over the f1
  // data), then the data cursor is rewound to point at the data
  // encoded by f1. Once decoded, the data cursor is then set to point
  // to after f2. The end result is that the data blocks are decoded
  // in the opposite order as they were encoded.
  template <typename F1, typename F2>
  BlobDecoder& alternate(const F1& f1, const F2& f2) {
    auto const start = advanced();
    // Skip over f1
    skipWithSize();
    // Decode f2
    f2();
    auto const end = advanced();
    assertx(end >= start);
    // Move back to f1
    retreat(end - start);
    // Decode f1
    withSize(f1);
    auto const middle = advanced();
    assertx(end >= middle);
    // Advance past f2
    advance(end - middle);
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

private:
  template<typename Cont, typename... Extra>
  void decodeMapContainer(Cont& cont, const Extra&... extra) {
    cont.clear();
    uint32_t size;
    decode(size);
    maybe_reserve(&cont, size);
    for (uint32_t i = 0; i < size; ++i) {
      auto key = make<typename Cont::key_type>();
      auto val = make<typename Cont::mapped_type>(extra...);
      cont.emplace(std::move(key), std::move(val));
    }
    maybe_shrink(&cont);
  }

  template<typename Cont, typename... Extra>
  void decodeSetContainer(Cont& cont, const Extra&... extra) {
    cont.clear();
    uint32_t size;
    decode(size);
    maybe_reserve(&cont, size);
    for (uint32_t i = 0; i < size; ++i) {
      auto val = make<typename Cont::value_type>(extra...);
      cont.emplace(std::move(val));
    }
    maybe_shrink(&cont);
  }

  template<typename Cont, typename... Extra>
  void decodeOrderedSetContainer(Cont& cont, const Extra&... extra) {
    cont.clear();
    uint32_t size;
    decode(size);
    maybe_reserve(&cont, size);
    for (uint32_t i = 0; i < size; ++i) {
      auto val = make<typename Cont::value_type>(extra...);
      cont.emplace(std::move(val));
    }
    maybe_shrink(&cont);
  }

  template<typename Cont, typename... Extra>
  void decodeVecContainer(Cont& cont, const Extra&... extra) {
    cont.clear();
    uint32_t size;
    decode(size);
    maybe_reserve(&cont, size);
    for (uint32_t i = 0; i < size; ++i) {
      auto val = make<typename Cont::value_type>(extra...);
      cont.emplace_back(std::move(val));
    }
    maybe_shrink(&cont);
  }

  const unsigned char* m_start;
  const unsigned char* m_p;
  const unsigned char* const m_last;
};

//////////////////////////////////////////////////////////////////////

// Can't form non-const refs to bitfields, so use this to serde them
// instead.
#define SERDE_BITFIELD(X, SD)                   \
  if constexpr (std::remove_reference<decltype(SD)>::type::deserializing) { \
    decltype(X) v;                              \
    SD(v);                                      \
    X = v;                                      \
  } else {                                      \
    SD(X);                                      \
  }

//////////////////////////////////////////////////////////////////////

namespace detail {

// Helpers to stamp out BlobEncoderHelpers for unique_ptr and
// copy_ptrs wrapping the above types.

template <typename T, typename D> struct UPBlobImpl {
  template <typename SerDe, typename... Extra>
  static void serde(SerDe& sd, std::unique_ptr<T, D>& p, Extra... extra) {
    if constexpr (SerDe::deserializing) {
      p = std::unique_ptr<T, D>(new T);
    } else {
      assertx(p);
    }
    sd(*p, extra...);
  }
};

template <typename T> struct CPBlobImpl {
  template <typename SerDe, typename... Extra>
  static void serde(SerDe& sd, copy_ptr<T>& p, Extra... extra) {
    if constexpr (SerDe::deserializing) {
      sd(*p.emplace(), extra...);
    } else {
      assertx(p);
      sd(*p, extra...);
    }
  }
};

}

//////////////////////////////////////////////////////////////////////

#define MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(T)                    \
  template<typename D>                                          \
  struct BlobEncoderHelper<std::unique_ptr<T, D>>               \
    : public detail::UPBlobImpl<T, D> {};

#define MAKE_COPY_PTR_BLOB_SERDE_HELPER(T)                      \
  template<>                                                    \
  struct BlobEncoderHelper<copy_ptr<T>>                         \
    : public detail::CPBlobImpl<T> {};

//////////////////////////////////////////////////////////////////////

}

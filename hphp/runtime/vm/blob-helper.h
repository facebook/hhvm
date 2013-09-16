/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_RUNTIME_VM_BLOB_HELPER_H_
#define incl_HPHP_RUNTIME_VM_BLOB_HELPER_H_

#include <algorithm>
#include <vector>
#include <cstdlib>
#include <type_traits>

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/complex-types.h"

#include "folly/Varint.h"

/*
 * This module contains helpers for serializing and deserializing
 * metadata into blobs suitable for insertion into the hhbc repo.
 *
 * Types may provide their own serialization logic by implementing a
 * single-argument member function template called `serde' (as in
 * "serialization/deserialization"), and pushing data it wants to
 * serialize into the parameter.  Those members may in turn have
 * customized serialization behavior, or they may "bottom out" to
 * these helpers if they are basic-enough types.
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

template<class T, class SerDe>
class IsNontrivialSerializable {
  template<class U, void (U::*)(SerDe&)> struct Checker;
  template<class U> static char test(Checker<U,&U::template serde<SerDe> >*);
  template<class>   static long test(...);
public:
  enum { value = sizeof(test<T>(0)) == 1 };
};

//////////////////////////////////////////////////////////////////////

class RepoQuery;

//////////////////////////////////////////////////////////////////////

struct BlobEncoder {
  static const bool deserializing = false;

  /*
   * Currently the most basic encoder/decode only works for integral
   * types.  (We don't want this to accidently get used for things
   * like pointers or aggregates.)
   *
   * Floating point support could be added later if we need it ...
   */
  template<class T>
  typename std::enable_if<
    std::is_integral<T>::value ||
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

  template<class T>
  typename std::enable_if<
    IsNontrivialSerializable<T,BlobEncoder>::value
  >::type encode(const T& t) {
    const_cast<T&>(t).serde(*this);
  }

  void encode(bool b) {
    encode(b ? 1 : 0);
  }

  void encode(DataType t) {
    // always encode DataType as int8 even if it's a bigger size.
    assert(DataType(int8_t(t)) == t);
    encode(int8_t(t));
  }

  void encode(const StringData* sd) {
    if (!sd) return encode(uint32_t(0));
    uint32_t sz = sd->size();
    encode(sz + 1);

    const size_t start = m_blob.size();
    m_blob.resize(start + sz);
    std::copy(sd->data(), sd->data() + sz, &m_blob[start]);
  }

  void encode(const TypedValue& tv) {
    if (tv.m_type == KindOfUninit) {
      // This represents an empty string
      return encode(uint32_t(1));
    }
    String s = f_serialize(tvAsCVarRef(&tv));
    encode(s.get());
  }

  template<class K, class V>
  void encode(const std::pair<K,V>& kv) {
    encode(kv.first);
    encode(kv.second);
  }

  template<class T>
  void encode(const std::vector<T>& vec) {
    encodeContainer(vec, "vector");
  }

  template<class K, class V, class H, class C>
  void encode(const hphp_hash_map<K,V,H,C>& map) {
    encodeContainer(map, "map");
  }

  template<class V, class H, class C>
  void encode(const hphp_hash_set<V,H,C>& set) {
    encodeContainer(set, "set");
  }

  template<class T>
  BlobEncoder& operator()(const T& t) {
    encode(t);
    return *this;
  }

  size_t size() const { return m_blob.size(); }
  const void* data() const { return &m_blob[0]; }

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
};

//////////////////////////////////////////////////////////////////////

struct BlobDecoder {
  static const bool deserializing = true;

  explicit BlobDecoder(const void* vp, size_t sz)
    : m_p(static_cast<const unsigned char*>(vp))
    , m_last(m_p + sz)
  {}

  // See encode() in BlobEncoder for why this only allows integral
  // types.
  template<class T>
  typename std::enable_if<
    std::is_integral<T>::value ||
    std::is_enum<T>::value
  >::type
  decode(T& t) {
    folly::ByteRange range(m_p, m_last);
    t = static_cast<T>(folly::decodeVarint(range));
    m_p = range.begin();
  }

  template<class T>
  typename std::enable_if<
    IsNontrivialSerializable<T,BlobEncoder>::value
  >::type decode(T& t) {
    t.serde(*this);
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

  void decode(TypedValue& tv) {
    tvWriteUninit(&tv);

    String s = decodeString();
    assert(!!s);
    if (s->empty()) return;

    tvAsVariant(&tv) = unserialize_from_string(s);
    tvAsVariant(&tv).setEvalScalar();
  }

  template<class K, class V>
  void decode(std::pair<K,V>& val) {
    decode(val.first);
    decode(val.second);
  }

  template<class T>
  void decode(std::vector<T>& vec) {
    uint32_t size;
    decode(size);
    for (uint32_t i = 0; i < size; ++i) {
      vec.push_back(T());
      decode(vec.back());
    }
  }

  template<class K, class V, class H, class C>
  void decode(hphp_hash_map<K,V,H,C>& map) {
    uint32_t size;
    decode(size);
    for (uint32_t i = 0; i < size; ++i) {
      std::pair<K,V> val;
      decode(val);
      map.insert(val);
    }
  }

  template<class V, class H, class C>
  void decode(hphp_hash_set<V,H,C>& set) {
    uint32_t size;
    decode(size);
    for (uint32_t i = 0; i < size; ++i) {
      V val;
      decode(val);
      set.insert(val);
    }
  }

  template<class T>
  BlobDecoder& operator()(T& t) {
    decode(t);
    return *this;
  }

private:
  String decodeString() {
    uint32_t sz;
    decode(sz);
    if (sz == 0) return String();
    sz--;

    String s = String(sz, ReserveString);
    char* pch = s.bufferSlice().ptr;
    assert(m_last - m_p >= sz);
    std::copy(m_p, m_p + sz, pch);
    m_p += sz;
    return s.setSize(sz);
  }

private:
  const unsigned char* m_p;
  const unsigned char* const m_last;
};

//////////////////////////////////////////////////////////////////////

}

#endif

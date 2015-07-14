/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_STRING_H_
#define incl_HPHP_STRING_H_

#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/types.h"
#include "hphp/util/assertions.h"
#include "hphp/util/hash-map-typedefs.h"
#include "hphp/util/functional.h"

#include <algorithm>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

class Array;
class String;
class VarNR;

// reserve space for buffer that will be filled in by client.
enum ReserveStringMode { ReserveString };

//////////////////////////////////////////////////////////////////////

namespace {

/*
 * Don't actually perform a shrink unless the savings meets this
 * threshold.
 */
constexpr int kMinShrinkThreshold = 1024;

}

//////////////////////////////////////////////////////////////////////

// Built strings will have their reference counts pre-initialized to 1.
StringData* buildStringData(int     n);
StringData* buildStringData(int64_t n);
StringData* buildStringData(double  n);

std::string convDblToStrWithPhpFormat(double n);

//////////////////////////////////////////////////////////////////////

/**
 * String type wrapping around StringData to implement copy-on-write and
 * literal string handling (to avoid string copying).
 */
class String {
  req::ptr<StringData> m_str;

protected:
  using IsUnowned = req::ptr<StringData>::IsUnowned;
  using NoIncRef = req::ptr<StringData>::NoIncRef;

  String(StringData* sd, NoIncRef) : m_str(sd, NoIncRef{}) {}

public:
  typedef hphp_hash_map<int64_t, const StringData *, int64_hash>
    IntegerStringDataMap;
  static const int MinPrecomputedInteger = SCHAR_MIN;
  static const int MaxPrecomputedInteger = 4095 + SCHAR_MIN;
  static StringData const **converted_integers_raw;
  static StringData const **converted_integers;
  static IntegerStringDataMap integer_string_data_map;

  static bool HasConverted(int64_t n) {
    return MinPrecomputedInteger <= n && n <= MaxPrecomputedInteger;
  }
  static bool HasConverted(int n) {
    return HasConverted((int64_t)n);
  }
  static void PreConvertInteger(int64_t n);

  // create a string from a character
  static String FromChar(char ch) {
    return makeStaticString(ch);
  }
  static String FromCStr(const char* str) {
    return makeStaticString(str);
  }

  static const StringData *ConvertInteger(int64_t n);
  static const StringData *GetIntegerStringData(int64_t n) {
    if (HasConverted(n)) {
      const StringData *sd = *(converted_integers + n);
      if (UNLIKELY(sd == nullptr)) {
        return ConvertInteger(n);
      }
      return sd;
    }
    IntegerStringDataMap::const_iterator it =
      integer_string_data_map.find(n);
    if (it != integer_string_data_map.end()) return it->second;
    return nullptr;
  }

public:
  String() {}
  ~String();

  StringData* get() const { return m_str.get(); }
  void reset() { m_str.reset(); }

  // Transfer ownership of our reference to this StringData.
  StringData* detach() { return m_str.detach(); }

  /**
   * Constructors
   */
  /* implicit */ String(StringData *data) : m_str(data) { }
  /* implicit */ String(int     n);
  /* implicit */ String(int64_t n);
  /* implicit */ String(double  n);
  /* implicit */ String(const char* s)
  : m_str(LIKELY((bool)s) ? StringData::Make(s, CopyString)
                           : nullptr, NoIncRef{}) { }

  String(const String& str) : m_str(str.m_str) { }
  /* implicit */ String(const StaticString& str);
  /* implicit */ String(char) = delete; // prevent unintentional promotion

  // Disable this---otherwise this would generally implicitly create a
  // Variant(bool) and then call String(Variant&&) ...
  /* implicit */ String(const StringData*) = delete;

  // Move ctor
  /* implicit */ String(String&& str) noexcept : m_str(std::move(str.m_str)) {}
  /* implicit */ String(Variant&& src);
  // Move assign
  String& operator=(String&& src) {
    m_str = std::move(src.m_str);
    return *this;
  }

  // Move assign from Variant
  String& operator=(Variant&& src);

  /* implicit */ String(const std::string &s)
  : m_str(StringData::Make(s.data(), s.size(), CopyString), NoIncRef{}) { }

  // attach to null terminated malloc'ed string, maybe free it now.
  String(char* s, AttachStringMode mode)
  : m_str(LIKELY((bool)s) ? StringData::Make(s, mode) : nullptr, NoIncRef{}) {}

  // copy a null terminated string
  String(const char *s, CopyStringMode mode)
  : m_str(LIKELY((bool)s) ? StringData::Make(s, mode) : nullptr, NoIncRef{}) {}

  // attach to binary malloc'ed string
  String(char* s, size_t length, AttachStringMode mode)
  : m_str(LIKELY((bool)s) ? StringData::Make(s, length, mode)
                          : nullptr, NoIncRef{}) { }

  // make copy of binary binary string
  String(const char *s, size_t length, CopyStringMode mode)
  : m_str(LIKELY((bool)s) ? StringData::Make(s, length, mode)
                          : nullptr, NoIncRef{}) { }

  // force a copy of a String
  String(const String& s, CopyStringMode mode)
  : m_str(LIKELY((bool)s) ? StringData::Make(s.c_str(), s.size(), mode)
                          : nullptr, NoIncRef{}) {}

  // make an empty string with cap reserve bytes, plus 1 for '\0'
  String(size_t cap, ReserveStringMode mode)
  : m_str(StringData::Make(cap), NoIncRef{}) { }

  static String attach(StringData* sd) {
    return String(sd, NoIncRef{});
  }

  void clear() { reset();}
  /**
   * Informational
   */
  const char *data() const {
    return m_str ? m_str->data() : "";
  }

  char *mutableData() const {
    return m_str->mutableData();
  }

public:
  const String& setSize(int len) {
    assert(m_str);
    m_str->setSize(len);
    return *this;
  }
  const String& shrink(size_t len) {
    assert(m_str);
    if (m_str->capacity() - len > kMinShrinkThreshold) {
      m_str = req::ptr<StringData>::attach(m_str->shrinkImpl(len));
    } else {
      assert(len < StringData::MaxSize);
      m_str->setSize(len);
    }
    return *this;
  }
  MutableSlice reserve(size_t size) {
    if (!m_str) return MutableSlice("", 0);
    auto const tmp = m_str->reserve(size);
    if (UNLIKELY(tmp != m_str)) {
      m_str = req::ptr<StringData>::attach(tmp);
    }
    return m_str->bufferSlice();
  }
  const char *c_str() const {
    return m_str ? m_str->data() : "";
  }
  bool empty() const {
    return m_str ? m_str->empty() : true;
  }
  int size() const {
    return m_str ? m_str->size() : 0;
  }
  int length() const {
    return m_str ? m_str->size() : 0;
  }
  uint32_t capacity() const {
    return m_str->capacity(); // intentionally skip nullptr check
  }
  StringSlice slice() const {
    return m_str ? m_str->slice() : StringSlice("", 0);
  }
  MutableSlice bufferSlice() {
    return m_str ? m_str->bufferSlice() : MutableSlice("", 0);
  }
  bool isNull() const { return !m_str; }
  bool isNumeric() const {
    return m_str ? m_str->isNumeric() : false;
  }
  bool isInteger() const {
    return m_str ? m_str->isInteger() : false;
  }
  bool isZero() const {
    return m_str ? m_str->isZero() : false;
  }

  /**
   * Take a sub-string from start with specified length. Note, read
   * http://www.php.net/substr about meanings of negative start or length.
   */
  String substr(int start, int length = 0x7FFFFFFF,
                bool nullable = false) const;

  /**
   * Find a character or a substring and return its position. "pos" has to be
   * within current string and it's the start point for searching.
   */
  static const int npos = -1;
  int find(char ch, int pos = 0, bool caseSensitive = true) const;
  int find(const char *s, int pos = 0, bool caseSensitive = true) const;
  int find(const String& s, int pos = 0, bool caseSensitive = true) const;
  int rfind(char ch, int pos = 0, bool caseSensitive = true) const;
  int rfind(const char *s, int pos = 0, bool caseSensitive = true) const;
  int rfind(const String& s, int pos = 0, bool caseSensitive = true) const;

  /**
   * Operators
   */
  String& operator=(StringData *data) {
    m_str = data;
    return *this;
  }
  String& operator=(const String& v) {
    m_str = v.m_str;
    return *this;
  }
  String& operator=(const StaticString& v);
  String& operator=(const char* v);
  String& operator=(const Variant& v);
  String& operator=(const std::string &s);
  // These should be members, but g++ doesn't yet support the rvalue
  // reference notation on lhs (http://goo.gl/LuCTo).
  friend String&& operator+(String&& lhs, const char* rhs);
  friend String&& operator+(String&& lhs, String&& rhs);
  friend String operator+(String&& lhs, const String & rhs);
  friend String operator+(const String & lhs, String&& rhs);
  friend String operator+(const String& lhs, const char* rhs);
  friend String operator+(const String & lhs, const String & rhs);
  String &operator += (const char* v);
  String &operator += (const String& v);
  String &operator += (const StringSlice& slice);
  String &operator += (const MutableSlice& slice);
  String  operator |  (const String& v) const = delete;
  String  operator &  (const String& v) const = delete;
  String  operator ^  (const String& v) const = delete;
  String &operator |= (const String& v) = delete;
  String &operator &= (const String& v) = delete;
  String &operator ^= (const String& v) = delete;
  String  operator ~  () const = delete;
  explicit operator std::string () const { return toCppString(); }
  explicit operator bool() const { return (bool)m_str; }

  /**
   * These are convenient functions for writing extensions, since code
   * generation always uses explicit functions like same(), less() etc. that
   * are type specialized and unambiguous.
   */
  bool operator == (const char* v) const = delete;
  bool operator != (const char* v) const = delete;
  bool operator >= (const char* v) const = delete;
  bool operator <= (const char* v) const = delete;
  bool operator >  (const char* v) const = delete;
  bool operator <  (const char* v) const = delete;
  bool operator == (const String& v) const;
  bool operator != (const String& v) const;
  bool operator >= (const String& v) const = delete;
  bool operator <= (const String& v) const = delete;
  bool operator >  (const String& v) const;
  bool operator <  (const String& v) const;
  bool operator == (const Variant& v) const;
  bool operator != (const Variant& v) const;
  bool operator >= (const Variant& v) const = delete;
  bool operator <= (const Variant& v) const = delete;
  bool operator >  (const Variant& v) const;
  bool operator <  (const Variant& v) const;

  /**
   * Type conversions
   */
  bool   toBoolean() const { return m_str ? m_str->toBoolean() : false;}
  char   toByte   () const { return m_str ? m_str->toByte   () : 0;}
  short  toInt16  () const { return m_str ? m_str->toInt16  () : 0;}
  int    toInt32  () const { return m_str ? m_str->toInt32  () : 0;}
  int64_t toInt64 () const { return m_str ? m_str->toInt64  () : 0;}
  double toDouble () const { return m_str ? m_str->toDouble () : 0;}
  VarNR  toKey   () const;
  std::string toCppString() const { return std::string(c_str(), size()); }

  /**
   * Comparisons
   */
  bool same (const char* v2) const = delete;
  bool same (const StringData *v2) const;
  bool same (const String& v2) const;
  bool same (const Array& v2) const;
  bool same (const Object& v2) const;
  bool same (const Resource& v2) const;
  bool equal(const char* v2) const = delete;
  bool equal(const StringData *v2) const;
  bool equal(const String& v2) const;
  bool equal(const Array& v2) const;
  bool equal(const Object& v2) const;
  bool equal(const Resource& v2) const;
  bool less (const char* v2) const = delete;
  bool less (const StringData *v2) const;
  bool less (const String& v2) const;
  bool less (const Array& v2) const;
  bool less (const Object& v2) const;
  bool less (const Resource& v2) const;
  bool more (const char* v2) const = delete;
  bool more (const StringData *v2) const;
  bool more (const String& v2) const;
  bool more (const Array& v2) const;
  bool more (const Object& v2) const;
  bool more (const Resource& v2) const;

  int compare(const char* v2) const;
  int compare(const String& v2) const;

  /**
   * Offset
   */
  String rvalAt(bool    key) const { return rvalAtImpl(key ? 1 : 0);}
  String rvalAt(char    key) const { return rvalAtImpl(key);}
  String rvalAt(short   key) const { return rvalAtImpl(key);}
  String rvalAt(int     key) const { return rvalAtImpl(key);}
  String rvalAt(int64_t key) const { return rvalAtImpl(key);}
  String rvalAt(double  key) const { return rvalAtImpl((int64_t)key);}
  String rvalAt(const char* key) const {
    return rvalAtImpl(String(key).toInt32());
  }
  String rvalAt(const StringData *key) const {
    not_reached();
    return rvalAtImpl(key ? key->toInt32() : 0);
  }
  String rvalAt(const String& key) const { return rvalAtImpl(key.toInt32());}
  String rvalAt(const Array& key) const;
  String rvalAt(const Object& key) const;
  String rvalAt(const Variant& key) const;

  /**
   * Returns one character at specified position.
   */
  char charAt(int pos) const;
  char operator[](int pos) const { return charAt(pos);}

  /**
   * Input/Output
   */
  void serialize(VariableSerializer *serializer) const;
  void unserialize(VariableUnserializer *uns, char delimiter0 = '"',
                   char delimiter1 = '"');

  /**
   * Debugging
   */
  void dump() const;

 private:
  static req::ptr<StringData> buildString(int n);
  static req::ptr<StringData> buildString(int64_t n);

  String rvalAtImpl(int key) const {
    if (m_str) {
      return m_str->getChar(key);
    }
    return String();
  }

  static void compileTimeAssertions() {
    static_assert(sizeof(String) == sizeof(req::ptr<StringData>), "");
  }
};

extern const String null_string;

///////////////////////////////////////////////////////////////////////////////

struct string_data_lt {
  bool operator()(const StringData *s1, const StringData *s2) const {
    int len1 = s1->size();
    int len2 = s2->size();
    if (len1 < len2) {
      return (len1 == 0) || (memcmp(s1->data(), s2->data(), len1) <= 0);
    } else if (len1 == len2) {
      return (len1 != 0) && (memcmp(s1->data(), s2->data(), len1) < 0);
    } else /* len1 > len2 */ {
      return ((len2 != 0) && (memcmp(s1->data(), s2->data(), len2) < 0));
    }
  }
};

template <class T> using ConstStringDataMap = hphp_hash_map<
  const StringData*,
  T,
  string_data_hash,
  string_data_same
>;

using ConstStringDataSet = hphp_hash_set<
  const StringData*,
  string_data_hash,
  string_data_same
>;

struct hphp_string_hash {
  size_t operator()(const String& s) const {
    return s.get()->hash();
  }
};

struct hphp_string_same {
  bool operator()(const String& s1, const String& s2) const {
    return s1.get()->same(s2.get());
  }
};

struct hphp_string_isame {
  bool operator()(const String& s1, const String& s2) const {
    return s1.get()->isame(s2.get());
  }
};

struct StringDataHashCompare {
  bool equal(const StringData *s1, const StringData *s2) const {
    assert(s1 && s2);
    return s1->same(s2);
  }
  size_t hash(const StringData *s) const {
    assert(s);
    return s->hash();
  }
};

struct StringDataHashICompare {
  bool equal(const StringData *s1, const StringData *s2) const {
    assert(s1 && s2);
    return s1->isame(s2);
  }
  size_t hash(const StringData *s) const {
    assert(s);
    return s->hash();
  }
};

typedef hphp_hash_set<String, hphp_string_hash, hphp_string_isame> StringISet;

template<typename T>
class StringIMap :
  public hphp_hash_map<String, T, hphp_string_hash, hphp_string_isame> { };

typedef hphp_hash_set<String, hphp_string_hash, hphp_string_same> StringSet;

template<typename T>
class StringMap :
  public hphp_hash_map<String, T, hphp_string_hash, hphp_string_same> { };

///////////////////////////////////////////////////////////////////////////////
// StrNR

class StrNR {
  StringData *m_px;

public:
  StrNR() : m_px(nullptr) {}
  explicit StrNR(StringData *sd) : m_px(sd) {}
  explicit StrNR(const StringData *sd) : m_px(const_cast<StringData*>(sd)) {}
  explicit StrNR(const String &s) : m_px(s.get()) {} // XXX

  ~StrNR() {
    if (debug) {
      m_px = reinterpret_cast<StringData*>(0xdeadbeeffaceb004);
    }
  }

  /* implicit */ operator const String&() const { return asString(); }
  const char* data() const { return m_px ? m_px->data() : ""; }
  const char* c_str() const { return data(); }
  int size() const { return m_px ? m_px->size() : 0; }
  bool empty() const { return size() == 0; }

  uint32_t capacity() const {
    return m_px->capacity(); // intentionally skip nullptr check
  }

  String& asString() {
    return *reinterpret_cast<String*>(this);
  }
  const String& asString() const {
    return const_cast<StrNR*>(this)->asString();
  }

  StringData* get() const { return m_px; }

private:
  static void compileTimeAssertions() {
    static_assert(offsetof(StrNR, m_px) == kExpectedMPxOffset, "");
  }
};

///////////////////////////////////////////////////////////////////////////////

/**
 * A StaticString can be co-accessed by multiple threads, therefore they are
 * not thread local, and they have to be allocated BEFORE any thread starts,
 * so that they won't be garbage collected by MemoryManager. This is used by
 * constant strings, so they can be pre-allocated before request handling.
 */
class StaticString : public String {
public:
  friend class StringUtil;

  explicit StaticString(const char* s);
  StaticString(const char* s, int length); // binary string
  explicit StaticString(std::string s);
  ~StaticString() {
    // prevent ~req::ptr from destroying contents.
    detach();
  }
  StaticString& operator=(const StaticString &str);

private:
  void insert();
};

#define LITSTR_INIT(str)    (true ? (str) : ("" str "")), (sizeof(str)-1)

StaticString getDataTypeString(DataType t);

//////////////////////////////////////////////////////////////////////

inline String::String(const StaticString& str) :
  m_str(str.m_str.get(), NoIncRef{}) {
  assert(str.m_str->isStatic());
}

inline String& String::operator=(const StaticString& v) {
  m_str = req::ptr<StringData>::attach(v.m_str.get());
  return *this;
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE String empty_string() {
  return String::attach(staticEmptyString());
}

//////////////////////////////////////////////////////////////////////

}

namespace folly {
template<> struct FormatValue<HPHP::String> {
  explicit FormatValue(const HPHP::String& str) : m_val(str) {}

  template<typename Callback>
  void format(FormatArg& arg, Callback& cb) const {
    FormatValue<HPHP::StringData*>(m_val.get()).format(arg, cb);
  }

 private:
  const HPHP::String& m_val;
};
}

#endif

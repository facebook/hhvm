/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef incl_HPHP_INSIDE_HPHP_COMPLEX_TYPES_H_
#error Directly including 'type_string.h' is prohibited. \
       Include 'complex_types.h' instead.
#endif

#include <util/assertions.h>
#include <runtime/base/util/smart_ptr.h>
#include <runtime/base/string_data.h>
#include <runtime/base/string_offset.h>
#include <runtime/base/types.h>
#include <runtime/base/hphp_value.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
class Array;
class String;
class VarNR;

// helpers
StringData* buildStringData(int     n);
StringData* buildStringData(int64_t   n);
StringData* buildStringData(double  n);
StringData* buildStringData(litstr  s);

/**
 * String type wrapping around StringData to implement copy-on-write and
 * literal string handling (to avoid string copying).
 */
class String : protected SmartPtr<StringData> {
  typedef SmartPtr<StringData> StringBase;

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
  static void PreConvertInteger(int64_t n) ATTRIBUTE_COLD;

  // create a string from a character
  static String FromChar(char ch) {
    return StringData::GetStaticString(ch);
  }

  static const StringData *ConvertInteger(int64_t n) ATTRIBUTE_COLD;
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
  static const StringData *GetIntegerStringData(int n) {
    return GetIntegerStringData((int64_t)n);
  }
  static const char *GetIntegerString(int64_t n) {
    const StringData *sd = GetIntegerStringData(n);
    if (sd) return sd->data();
    return nullptr;
  }
  static const char *GetIntegerString(int n) {
    return GetIntegerString((int64_t)n);
  }

public:
  String() {}
  ~String();

  StringData* get() const { return m_px; }
  void reset() { StringBase::reset(); }

  // Deliberately doesn't throw_null_pointer_exception as a perf
  // optimization.
  StringData* operator->() const {
    return m_px;
  }

  // Transfer ownership of our reference to this StringData.
  StringData* detach() {
    StringData* ret = m_px;
    m_px = 0;
    return ret;
  }

  /**
   * Constructors
   */
  /* implicit */ String(StringData *data) : StringBase(data) { }
  /* implicit */ String(int     n);
  /* implicit */ String(int64_t   n);
  /* implicit */ String(double  n);
  /* implicit */ String(litstr  s) {
    if (s) {
      m_px = buildStringData(s);
      m_px->setRefCount(1);
    }
  }
  String(const String& str) : StringBase(str.m_px) { }

  // Move ctor
  /* implicit */ String(String&& str) : StringBase(std::move(str)) {}
  /* implicit */ String(Variant&& src);
  // Move assign
  String& operator=(String&& src) {
    static_assert(sizeof(String) == sizeof(StringBase),"Fix this.");
    StringBase::operator=(std::move(src));
    return *this;
  }

  // Move assign from Variant
  String& operator=(Variant&& src);

  /* implicit */ String(const std::string &s) { // always make a copy
    m_px = NEW(StringData)(s.data(), s.size(), CopyString);
    m_px->setRefCount(1);
  }
  // attach to null terminated malloc'ed string, maybe free it now.
  String(const char *s, AttachStringMode mode) {
    if (s) {
      m_px = NEW(StringData)(s, mode);
      m_px->setRefCount(1);
    }
  }
  // copy a null terminated string
  String(const char *s, CopyStringMode mode) {
    if (s) {
      m_px = NEW(StringData)(s, mode);
      m_px->setRefCount(1);
    }
  }
  // attach to binary malloc'ed string
  String(const char *s, int length, AttachStringMode mode) {
    if (s) {
      m_px = NEW(StringData)(s, length, mode);
      m_px->setRefCount(1);
    }
  }
  // make copy of binary binary string
  String(const char *s, int length, CopyStringMode mode) {
    if (s) {
      m_px = NEW(StringData)(s, length, mode);
      m_px->setRefCount(1);
    }
  }
  // force a copy of a String
  String(const String& s, CopyStringMode mode) {
    if (s.m_px) {
      m_px = NEW(StringData)(s.c_str(), s.size(), mode);
      m_px->setRefCount(1);
    }
  }
  // make an empty string with cap reserve bytes, plus 1 for '\0'
  String(int cap, ReserveStringMode mode) {
    m_px = NEW(StringData)(cap);
    m_px->setRefCount(1);
  }

  void checkStaticHelper();
  void checkStatic() {}

  void clear() { reset();}
  /**
   * Informational
   */
  const char *data() const {
    return m_px ? m_px->data() : "";
  }
public:
  CStrRef setSize(int len) {
    assert(m_px);
    m_px->setSize(len);
    return *this;
  }
  CStrRef shrink(int len) {
    assert(m_px);
    m_px->shrink(len);
    return *this;
  }
  MutableSlice reserve(int size) {
    return m_px ? m_px->reserve(size) : MutableSlice("", 0);
  }
  const char *c_str() const {
    return m_px ? m_px->data() : "";
  }
  bool empty() const {
    return m_px ? m_px->empty() : true;
  }
  int size() const {
    return m_px ? m_px->size() : 0;
  }
  int length() const {
    return m_px ? m_px->size() : 0;
  }
  StringSlice slice() const {
    return m_px ? m_px->slice() : StringSlice("", 0);
  }
  MutableSlice mutableSlice() {
    return m_px ? m_px->mutableSlice() : MutableSlice("", 0);
  }
  bool isNull() const {
    return m_px == nullptr;
  }
  bool isNumeric() const {
    return m_px ? m_px->isNumeric() : false;
  }
  bool isInteger() const {
    return m_px ? m_px->isInteger() : false;
  }
  bool isZero() const {
    return m_px ? m_px->isZero() : false;
  }

  bool isValidVariableName() const {
    return m_px ? m_px->isValidVariableName() : false;
  }

  /**
   * Take a sub-string from start with specified length. Note, read
   * http://www.php.net/substr about meanings of negative start or length.
   */
  String substr(int start, int length = 0x7FFFFFFF,
                bool nullable = false) const;

  /**
   * Returns the last token if string is delimited by the specified.
   */
  String lastToken(char delimiter);

  /**
   * Find a character or a substring and return its position. "pos" has to be
   * within current string and it's the start point for searching.
   */
  static const int npos = -1;
  int find(char ch, int pos = 0, bool caseSensitive = true) const;
  int find(const char *s, int pos = 0, bool caseSensitive = true) const;
  int find(CStrRef s, int pos = 0, bool caseSensitive = true) const;
  int rfind(char ch, int pos = 0, bool caseSensitive = true) const;
  int rfind(const char *s, int pos = 0, bool caseSensitive = true) const;
  int rfind(CStrRef s, int pos = 0, bool caseSensitive = true) const;

  /**
   * Replace a substr with another and return replaced one. Note, read
   * http://www.php.net/substr about meanings of negative start or length.
   *
   * The form that takes a "count" reference will still replace all occurrences
   * and return total replaced count in the out parameter. It does NOT mean
   * it will replace at most that many occurrences, so count's input value
   * is never checked.
   */
  String replace(int start, int length, CStrRef replacement) const;
  String replace(CStrRef search, CStrRef replacement) const;
  String replace(CStrRef search, CStrRef replacement, int &count,
                 bool caseSensitive) const;

  /**
   * Operators
   */
  String &operator =  (StringData *data);
  String &operator =  (litstr  v);
  String &operator =  (const String& v);
  String &operator =  (CVarRef v);
  String &operator =  (const std::string &s);
  // These should be members, but g++ doesn't yet support the rvalue
  // reference notation on lhs (http://goo.gl/LuCTo).
  friend String&& operator+(String&& lhs, litstr rhs);
  friend String&& operator+(String&& lhs, String&& rhs);
  friend String operator+(String&& lhs, const String & rhs);
  friend String operator+(const String & lhs, String&& rhs);
  friend String operator+(const String& lhs, litstr rhs);
  friend String operator+(const String & lhs, const String & rhs);
  String &operator += (litstr  v);
  String &operator += (CStrRef v);
  String &operator += (const StringSlice& slice);
  String &operator += (const MutableSlice& slice);
  String  operator |  (CStrRef v) const;
  String  operator &  (CStrRef v) const;
  String  operator ^  (CStrRef v) const;
  String &operator |= (CStrRef v);
  String &operator &= (CStrRef v);
  String &operator ^= (CStrRef v);
  String  operator ~  () const;
  explicit operator std::string () const {
    return std::string(c_str(), size());
  }
  explicit operator bool() const {
    return m_px != nullptr;
  }

  /**
   * These are convenient functions for writing extensions, since code
   * generation always uses explicit functions like same(), less() etc. that
   * are type specialized and unambiguous.
   */
  bool operator == (litstr  v) const;
  bool operator != (litstr  v) const;
  bool operator >= (litstr  v) const;
  bool operator <= (litstr  v) const;
  bool operator >  (litstr  v) const;
  bool operator <  (litstr  v) const;
  bool operator == (CStrRef v) const;
  bool operator != (CStrRef v) const;
  bool operator >= (CStrRef v) const;
  bool operator <= (CStrRef v) const;
  bool operator >  (CStrRef v) const;
  bool operator <  (CStrRef v) const;
  bool operator == (CVarRef v) const;
  bool operator != (CVarRef v) const;
  bool operator >= (CVarRef v) const;
  bool operator <= (CVarRef v) const;
  bool operator >  (CVarRef v) const;
  bool operator <  (CVarRef v) const;

  /**
   * Type conversions
   */
  bool   toBoolean() const { return m_px ? m_px->toBoolean() : false;}
  char   toByte   () const { return m_px ? m_px->toByte   () : 0;}
  short  toInt16  () const { return m_px ? m_px->toInt16  () : 0;}
  int    toInt32  () const { return m_px ? m_px->toInt32  () : 0;}
  int64_t  toInt64  () const { return m_px ? m_px->toInt64  () : 0;}
  double toDouble () const { return m_px ? m_px->toDouble () : 0;}
  VarNR  toKey   () const;

  /**
   * Comparisons
   */
  bool same (litstr  v2) const;
  bool same (const StringData *v2) const;
  bool same (CStrRef v2) const;
  bool same (CArrRef v2) const;
  bool same (CObjRef v2) const;
  bool equal(litstr  v2) const;
  bool equal(const StringData *v2) const;
  bool equal(CStrRef v2) const;
  bool equal(CArrRef v2) const;
  bool equal(CObjRef v2) const;
  bool less (litstr  v2) const;
  bool less (const StringData *v2) const;
  bool less (CStrRef v2) const;
  bool less (CArrRef v2) const;
  bool less (CObjRef v2) const;
  bool more (litstr  v2) const;
  bool more (const StringData *v2) const;
  bool more (CStrRef v2) const;
  bool more (CArrRef v2) const;
  bool more (CObjRef v2) const;

  /**
   * Offset
   */
  String rvalAt(bool    key) const { return rvalAtImpl(key ? 1 : 0);}
  String rvalAt(char    key) const { return rvalAtImpl(key);}
  String rvalAt(short   key) const { return rvalAtImpl(key);}
  String rvalAt(int     key) const { return rvalAtImpl(key);}
  String rvalAt(int64_t   key) const { return rvalAtImpl(key);}
  String rvalAt(double  key) const { return rvalAtImpl((int64_t)key);}
  String rvalAt(litstr  key) const { return rvalAtImpl(String(key).toInt32());}
  String rvalAt(const StringData *key) const {
    not_reached();
    return rvalAtImpl(key ? key->toInt32() : 0);
  }
  String rvalAt(CStrRef key) const { return rvalAtImpl(key.toInt32());}
  String rvalAt(CArrRef key) const;
  String rvalAt(CObjRef key) const;
  String rvalAt(CVarRef key) const;

  StringOffset lvalAt(bool    key) { return lvalAtImpl(key ? 1 : 0);}
  StringOffset lvalAt(char    key) { return lvalAtImpl(key);}
  StringOffset lvalAt(short   key) { return lvalAtImpl(key);}
  StringOffset lvalAt(int     key) { return lvalAtImpl(key);}
  StringOffset lvalAt(int64_t   key) { return lvalAtImpl(key);}
  StringOffset lvalAt(double  key) { return lvalAtImpl((int64_t)key);}
  StringOffset lvalAt(litstr  key) { return lvalAtImpl(String(key).toInt32());}
  StringOffset lvalAt(const StringData *key) {
    not_reached();
    return lvalAtImpl(key ? key->toInt32() : 0);
  }
  StringOffset lvalAt(CStrRef key) { return lvalAtImpl(key.toInt32());}
  StringOffset lvalAt(CArrRef key);
  StringOffset lvalAt(CObjRef key);
  StringOffset lvalAt(CVarRef key);

  template <class K, class V>
  inline const V &set(K key, const V &value);

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

  StringOffset lvalAtImpl(int key) {
    StringData *s = StringData::Escalate(m_px);
    StringBase::operator=(s);
    return StringOffset(m_px, key);
  }

  String rvalAtImpl(int key) const {
    if (m_px) {
      return m_px->getChar(key);
    }
    return String();
  }

  static void compileTimeAssertions() {
    BOOST_STATIC_ASSERT((offsetof(String, m_px) == kExpectedMPxOffset));
  }
};

extern const String null_string;

///////////////////////////////////////////////////////////////////////////////

struct string_data_hash {
  size_t operator()(const StringData *s) const {
    return s->hash();
  }
};

struct string_data_same {
  bool operator()(const StringData *s1, const StringData *s2) const {
    assert(s1 && s2);
    return s1->same(s2);
  }
};

struct string_data_isame {
  bool operator()(const StringData *s1, const StringData *s2) const {
    assert(s1 && s2);
    return s1->isame(s2);
  }
};

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

typedef hphp_hash_set<const StringData*, string_data_hash, string_data_same>
  ConstStringDataSet;

struct hphp_string_hash {
  size_t operator()(CStrRef s) const {
    return s->hash();
  }
};

struct hphp_string_same {
  bool operator()(CStrRef s1, CStrRef s2) const {
    return s1->same(s2.get());
  }
};

struct hphp_string_isame {
  bool operator()(CStrRef s1, CStrRef s2) const {
    return s1->isame(s2.get());
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
public:
  explicit StrNR(StringData *data) {
    m_px = data;
  }
  explicit StrNR(const StringData *data) {
    m_px = const_cast<StringData*>(data);
  }
  explicit StrNR(const String &s) { // XXX
    m_px = s.get();
  }

  operator CStrRef() const { return asString(); }
  const char *data() const { return m_px ? m_px->data() : ""; }

  String& asString() {
    return *reinterpret_cast<String*>(this);
  }

  const String& asString() const {
    return const_cast<StrNR*>(this)->asString();
  }

protected:
  StringData *m_px;
  static void compileTimeAssertions() {
    BOOST_STATIC_ASSERT((offsetof(StrNR, m_px) == kExpectedMPxOffset));
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

  explicit StaticString(litstr s);
  StaticString(litstr s, int length); // binary string
  explicit StaticString(std::string s);
  StaticString(const StaticString &str);
  ~StaticString() {
    // prevent ~SmartPtr from calling decRefCount after data is released
    m_px = nullptr;
  }
  StaticString& operator=(const StaticString &str);

private:
  void insert();
};

extern const StaticString empty_string;

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_STRING_H_

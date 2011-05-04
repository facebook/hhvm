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

#ifndef __INSIDE_HPHP_COMPLEX_TYPES_H__
#error Directly including 'type_string.h' is prohibited. \
       Include 'complex_types.h' instead.
#endif

#ifndef __HPHP_STRING_H__
#define __HPHP_STRING_H__

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
class AtomicString;

/**
 * String type wrapping around StringData to implement copy-on-write and
 * literal string handling (to avoid string copying).
 */
class String : public SmartPtr<StringData> {
public:
  // create a string from a character
  static String FromChar(char ch);

public:
  String() {}

  /**
   * Constructors
   */
  String(StringData *data) : SmartPtr<StringData>(data) { }
  String(int     n);
  String(int64   n);
  String(double  n);
  String(litstr  s) {
    if (s) {
      m_px = NEW(StringData)(s, AttachLiteral);
      m_px->setRefCount(1);
    }
  }
  String(CStrRef str)
    : SmartPtr<StringData>(str.m_px) { }
  String(const std::string &s) { // always make a copy
    m_px = NEW(StringData)(s.data(), s.size(), CopyString);
    m_px->setRefCount(1);
  }
  String(const char *s, StringDataMode mode) { // null-terminated string
    if (s) {
      m_px = NEW(StringData)(s, mode);
      m_px->setRefCount(1);
    }
  }
  String(const char *s, int length, StringDataMode mode) { // binary string
    if (s) {
      m_px = NEW(StringData)(s, length, mode);
      m_px->setRefCount(1);
    }
  }
  String(const AtomicString &s);

  void clear() { reset();}
  /**
   * Informational
   */
  operator const char *() const {
    return m_px ? m_px->data() : "";
  }
  const char *data() const {
    return m_px ? m_px->data() : "";
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
  bool isNull() const {
    return m_px == NULL;
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
  bool isLiteral() const {
    return m_px ? m_px->isLiteral() : true;
  }

  int64 hashForIntSwitch(int64 firstNonZero, int64 noMatch) const {
    return m_px ? m_px->hashForIntSwitch(firstNonZero, noMatch) : 0;
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
  int rfind(char ch, int pos = -1, bool caseSensitive = true) const;
  int rfind(const char *s, int pos = -1, bool caseSensitive = true) const;
  int rfind(CStrRef s, int pos = -1, bool caseSensitive = true) const;

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
  String &operator =  (CStrRef v);
  String &operator =  (CVarRef v);
  String &operator =  (const std::string &s);
  String &operator =  (const AtomicString &s);
  String  operator +  (litstr  v) const;
  String  operator +  (CStrRef v) const;
  String &operator += (litstr  v);
  String &operator += (CStrRef v);
  String  operator |  (CStrRef v) const;
  String  operator &  (CStrRef v) const;
  String  operator ^  (CStrRef v) const;
  String &operator |= (CStrRef v);
  String &operator &= (CStrRef v);
  String &operator ^= (CStrRef v);
  String  operator ~  () const;

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
  int64  toInt64  () const { return m_px ? m_px->toInt64  () : 0;}
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
  String rvalAt(int64   key) const { return rvalAtImpl(key);}
  String rvalAt(double  key) const { return rvalAtImpl((int64)key);}
  String rvalAt(litstr  key) const { return rvalAtImpl(String(key).toInt32());}
  String rvalAt(const StringData *key) const {
    assert(false);
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
  StringOffset lvalAt(int64   key) { return lvalAtImpl(key);}
  StringOffset lvalAt(double  key) { return lvalAtImpl((int64)key);}
  StringOffset lvalAt(litstr  key) { return lvalAtImpl(String(key).toInt32());}
  StringOffset lvalAt(const StringData *key) {
    assert(false);
    return lvalAtImpl(key ? key->toInt32() : 0);
  }
  StringOffset lvalAt(CStrRef key) { return lvalAtImpl(key.toInt32());}
  StringOffset lvalAt(CArrRef key);
  StringOffset lvalAt(CObjRef key);
  StringOffset lvalAt(CVarRef key);

  template <class K, class V>
  inline const V &set(K key, const V &value);

  template<class T>
  String refvalAt(T key) {
    return rvalAt(key);
  }

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
   * Check TheStaticStringSet, and upgrade itself to an existing StaticString.
   */
  bool checkStatic();

  /**
   * Marshaling/Unmarshaling between request thread and fiber thread.
   */
  String fiberCopy() const;

  /**
   * Debugging
   */
  void dump() const;

 private:
  StringOffset lvalAtImpl(int key) {
    StringData *s = StringData::Escalate(m_px);
    SmartPtr<StringData>::operator=(s);
    return StringOffset(m_px, key);
  }

  String rvalAtImpl(int key) const {
    if (m_px) {
      return m_px->getChar(key);
    }
    return String();
  }

  static void compileTimeAssertions() {
    CT_ASSERT(offsetof(String, m_px) == offsetof(Value, m_data));
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
    ASSERT(s1 && s2);
    return s1->same(s2);
  }
};

struct string_data_isame {
  bool operator()(const StringData *s1, const StringData *s2) const {
    ASSERT(s1 && s2);
    return s1->isame(s2);
  }
};

typedef hphp_hash_set<StringData *, string_data_hash, string_data_same>
  StringDataSet;

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

typedef hphp_hash_set<String, hphp_string_hash, hphp_string_isame> StringISet;

template<typename T>
class StringIMap :
  public hphp_hash_map<String, T, hphp_string_hash, hphp_string_isame> { };

typedef hphp_hash_set<String, hphp_string_hash, hphp_string_same> StringSet;

template<typename T>
class StringMap :
  public hphp_hash_map<String, T, hphp_string_hash, hphp_string_same> { };

///////////////////////////////////////////////////////////////////////////////

/**
 * A StaticString can be co-accessed by multiple threads, therefore they are
 * not thread local, and they have to be allocated BEFORE any thread starts,
 * so that they won't be garbage collected by MemoryManager. This is used by
 * constant strings, so they can be pre-allocated before request handling.
 */
class StaticString : public String {
public:
  static StringDataSet &TheStaticStringSet();
  static void FinishInit();
  static void ResetAll(); // only supposed to be called during program shutdown

public:
  friend class StringUtil;
  friend class LiteralStringInitializer;

  StaticString(litstr s);
  StaticString(litstr s, int length); // binary string
  StaticString(std::string s);
  StaticString(const StaticString &str);
  ~StaticString() {
    // prevent ~SmartPtr from calling decRefCount after data is released
    m_px = NULL;
  }
  StaticString& operator=(const StaticString &str);

private:
  void init(litstr s, int length);
  void insert();

  StringData m_data;
  static StringDataSet *s_stringSet;
};

extern const StaticString empty_string;

///////////////////////////////////////////////////////////////////////////////
// StrNR

class StrNR : public String {
public:
  StrNR(StringData *data) {
    m_px = data;
  }
  StrNR(const StrNR &s) {
    m_px = s.m_px;
  }
  ~StrNR() {
    m_px = NULL;
  }
};

///////////////////////////////////////////////////////////////////////////////
// AtomicString

class AtomicString : public AtomicSmartPtr<StringData> {
public:
  AtomicString() { }
  AtomicString(const char *s, StringDataMode mode = AttachLiteral)
    : AtomicSmartPtr<StringData>(s ? new StringData(s, mode) : NULL) { }
  AtomicString(const std::string &s)
    : AtomicSmartPtr<StringData>(new StringData(s.data(), s.size(),
                                                CopyString)) { }
  AtomicString(StringData *str);
  AtomicString(const AtomicString &s) : AtomicSmartPtr<StringData>(s.m_px) { }

  AtomicString &operator=(const AtomicString &s);
  AtomicString &operator=(const std::string &s);

  const char *c_str() const {
    return m_px ? m_px->data() : "";
  }
  bool empty() const {
    return m_px ? m_px->empty() : true;
  }
  int size() const {
    return m_px ? m_px->size() : 0;
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_STRING_H__

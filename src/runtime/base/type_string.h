/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#define null_string String::s_nullString

/**
 * String type wrapping around StringData to implement copy-on-write and
 * literal string handling (to avoid string copying).
 */
class String : protected Value {
 public:
  friend class Variant;

  String() {
    m_data.pstr = NULL;
  }
  ~String() {
    if (m_data.pstr && m_data.pstr->decRefCount() == 0) {
      m_data.pstr->release();
    }
  }

  static const String s_nullString;

  /**
   * Constructors
   */
  String(StringData *data) {
    m_data.pstr = data;
    if (m_data.pstr) {
      m_data.pstr->incRefCount();
    }
  }
  String(int     n);
  String(int64   n);
  String(double  n);
  String(litstr  s) {
    m_data.pstr = NEW(StringData)(s, AttachLiteral);
    m_data.pstr->incRefCount();
  }
  String(CStrRef str) {
    m_data.pstr = str.m_data.pstr;
    if (m_data.pstr) {
      m_data.pstr->incRefCount();
    }
  }

  String(const std::string &s) {
    // always make a copy
    m_data.pstr = NEW(StringData)(s.data(), s.size(), CopyString);
    m_data.pstr->incRefCount();
  }
  String(const char *s, StringDataMode mode) {
    // null-terminated string
    m_data.pstr = s ? NEW(StringData)(s, mode) : NULL;
    if (m_data.pstr) {
      m_data.pstr->incRefCount();
    }
  }
  String(const char *s, int length, StringDataMode mode) {
    // binary string
    m_data.pstr = s ? NEW(StringData)(s, length, mode) : NULL;
    if (m_data.pstr) {
      m_data.pstr->incRefCount();
    }
  }

  // create a string from a character
  static String FromChar(char ch);

  bool isNull() const { return m_data.pstr == NULL; }

  String& set(StringData *px) {
    if (m_data.pstr != px) {
      if (m_data.pstr && m_data.pstr->decRefCount() == 0) {
        m_data.pstr->release();
      }
      m_data.pstr = px;
      if (m_data.pstr) {
        m_data.pstr->incRefCount();
      }
    }
    return *this;
  }

 private:
  String& setPtr(StringData *px) {
    ASSERT(m_data.pstr != px);
    ASSERT(px != NULL);
    if (m_data.pstr && m_data.pstr->decRefCount() == 0) {
      m_data.pstr->release();
    }
    m_data.pstr = px;
    m_data.pstr->incRefCount();
    return *this;
  }

 public:
  String& set(const String& src) {
    return set(src.m_data.pstr);
  }

  void reset() {
    if (m_data.pstr && m_data.pstr->decRefCount() == 0) {
      m_data.pstr->release();
    }
    m_data.pstr = NULL;
  }

  /**
   * Magic delegation.
   */
  StringData *operator->() const {
    if (!m_data.pstr) throw NullPointerException();
    return m_data.pstr;
  }

  /**
   * Get the raw pointer.
   */
  StringData *get() const {
    return m_data.pstr;
  }


  void assign(const char *data, StringDataMode mode);
  void assign(const char *data, int len, StringDataMode mode);
  void assignLiteral(litstr data) {
    assign(data, AttachLiteral);
  }

  void clear() { reset();}
  /**
   * Informational
   */
  operator const char *() const {
    return m_data.pstr ? m_data.pstr->data() : "";
  }
  const char *data() const {
    return m_data.pstr ? m_data.pstr->data() : "";
  }
  const char *c_str() const {
    return m_data.pstr ? m_data.pstr->data() : "";
  }
  bool empty() const {
    return m_data.pstr ? m_data.pstr->empty() : true;
  }
  int size() const {
    return m_data.pstr ? m_data.pstr->size() : 0;
  }
  int length() const {
    return m_data.pstr ? m_data.pstr->size() : 0;
  }
  bool isNumeric() const {
    return m_data.pstr ? m_data.pstr->isNumeric() : false;
  }
  bool isInteger() const {
    return m_data.pstr ? m_data.pstr->isInteger() : false;
  }
  bool isZero() const {
    return m_data.pstr ? m_data.pstr->isZero() : false;
  }

  bool isValidVariableName() const {
    return m_data.pstr ? m_data.pstr->isValidVariableName() : false;
  }
  bool isLiteral() const {
    return m_data.pstr ? m_data.pstr->isLiteral() : true;
  }

  /**
   * Take a sub-string from start with specified length. Note, read
   * http://www.php.net/substr about meanings of negative start or length.
   */
  String substr(int start, int length = 0x7FFFFFFF) const;

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
  bool   toBoolean() const {
    return m_data.pstr ? m_data.pstr->toBoolean() : false;
  }
  char   toByte   () const {
    return m_data.pstr ? m_data.pstr->toByte() : false;
  }
  short  toInt16  () const { return m_data.pstr ? m_data.pstr->toInt16  () : 0;}
  int    toInt32  () const { return m_data.pstr ? m_data.pstr->toInt32  () : 0;}
  int64  toInt64  () const { return m_data.pstr ? m_data.pstr->toInt64  () : 0;}
  double toDouble () const { return m_data.pstr ? m_data.pstr->toDouble () : 0;}
  Variant toKey   () const;

  /**
   * Comparisons
   */
  bool same (litstr  v2) const;
  bool same (CStrRef v2) const;
  bool same (CArrRef v2) const;
  bool same (CObjRef v2) const;
  bool equal(litstr  v2) const;
  bool equal(CStrRef v2) const;
  bool equal(CArrRef v2) const;
  bool equal(CObjRef v2) const;
  bool less (litstr  v2) const;
  bool less (CStrRef v2) const;
  bool less (CArrRef v2) const;
  bool less (CObjRef v2) const;
  bool more (litstr  v2) const;
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
  StringOffset lvalAt(CStrRef key) { return lvalAtImpl(key.toInt32());}
  StringOffset lvalAt(CArrRef key);
  StringOffset lvalAt(CObjRef key);
  StringOffset lvalAt(CVarRef key);

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
  void unserialize(std::istream &in, char delimiter0 = '"',
                   char delimiter1 = '"');

  /**
   * Debugging
   */
  void dump();

 private:
  StringOffset lvalAtImpl(int key) {
    setPtr(NEW(StringData)(data(), size(), CopyString));
    return StringOffset(m_data.pstr, key);
  }

  String rvalAtImpl(int key) const {
    if (m_data.pstr) {
      return m_data.pstr->getChar(key);
    }
    return String();
  }
};

struct zend_hash {
  size_t operator()(CStrRef s) const {
    return hash_string(s.data(), s.size());
  }
};

struct zend_eqstr {
  bool operator()(CStrRef s1, CStrRef s2) const {
    int len = s1.size();
    if (s2.size() != len)
      return false;
    return !memcmp(s1.data(), s2.data(), len);
  }
};

/**
 * Two useful typedefs
 */
typedef hphp_hash_set<String, zend_hash, zend_eqstr> StringSet;
typedef hphp_hash_map<String, int, zend_hash, zend_eqstr> MapStringToInt;

///////////////////////////////////////////////////////////////////////////////

/**
 * A StaticString can be co-accessed by multiple threads, therefore they are
 * not thread local, and they have to be allocated BEFORE any thread starts,
 * so that they won't be garbage collected by MemoryManager. This is used by
 * constant strings, so they can be pre-allocated before request handling.
 */
class StaticString : public String {
public:
  StaticString() {}
  StaticString(litstr s);
  StaticString(litstr s, int length); // binary string
  StaticString(std::string s);
  StaticString(const StaticString &str);
  ~StaticString() {
    // prevent ~String from calling decRefCount after data is released
    String::m_data.pstr = NULL;
  }
  StaticString &operator =  (litstr  v);
private:
  StringData m_data;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_STRING_H__

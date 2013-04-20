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

#ifndef incl_HPHP_STRING_BUFFER_H_
#define incl_HPHP_STRING_BUFFER_H_

#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class File;

class StringBufferLimitException : public FatalErrorException {
public:
  StringBufferLimitException(int size, CStrRef partialResult)
    : FatalErrorException(0, "StringBuffer exceeded %d bytes of memory", size),
      m_result(partialResult) {}
  virtual ~StringBufferLimitException() throw() {}

  String m_result;
};

/**
 * Efficient string concatenation.
 */
DECLARE_BOOST_TYPES(StringBuffer);
class StringBuffer {
public:
  /**
   * Constructing a string buffer with some initial size, subsequent allocation
   * will double existing size every round.
   */
  explicit StringBuffer(int initialSize = StringData::MaxSmallSize);
  ~StringBuffer();

  static const int kDefaultOutputLimit = StringData::MaxSize;
  void setOutputLimit(int maxBytes) {
    m_maxBytes = maxBytes > 0 ? maxBytes : kDefaultOutputLimit;
  }

  bool valid() const { return m_buffer != nullptr;}
  bool empty() const { return m_len == 0;}
  int size() const { return m_len;}
  int length() const { return m_len;}
  const char *data() const;
public:
  char charAt(int pos) const;

  /**
   * Detach buffer and yield a String. After this, do not use this StringBuffer
   * object any more.
   */
  String detach() { return detachImpl(); }
  String detachImpl();
  String copy();
  void reset();
  void clear() { reset();}
  void resize(int size);
  void release();

  /**
   * Increase internal buffer to at least "size" longer, and return the write
   * position to append more chars.
   */
  char *reserve(int size);

  /**
   * Append strings.
   */
  void append(int n);
  void append(int64_t n);
  void append(char c) {
    if (m_buffer && m_len < m_cap) {
      m_buffer[m_len++] = c;
      return;
    }
    appendHelper(c);
  }
  void appendHelper(char c);
  void append(unsigned char c) { append((char)c);}
  void append(litstr  s) { assert(s); append(s, strlen(s));}
  void append(CStrRef s);
  void append(CVarRef s);
  void append(const StringData *s) { append(s->data(), s->size()); }
  void append(const char *s, int len) {
    assert(len >= 0);
    if (m_buffer && len <= m_cap - m_len) {
      memcpy(m_buffer + m_len, s, len);
      m_len += len;
      return;
    }
    appendHelper(s, len);
  }
  void appendHelper(const char *s, int len);
  void append(const std::string &s) { append(s.data(), s.size());}
  /**
   * Json-escape the string and then append it.
   */
  void appendJsonEscape(const char *s, int len, int options);

  StringBuffer &operator+=(int n)     { append(n); return *this;}
  StringBuffer &operator+=(char c)    { append(c); return *this;}
  StringBuffer &operator+=(litstr  s) { append(s); return *this;}
  StringBuffer &operator+=(CStrRef s) { append(s); return *this;}

  StringBuffer &add(CStrRef s) { append(s); return *this; }
  StringBuffer &add(const char *s, int len) { append(s, len); return *this; }

  /**
   * Append what buf has, and reset buf. Internally, if this StringBuffer
   * is empty, it will swap with buf, so to avoid one string copying.
   */
  void absorb(StringBuffer &buf);

  /**
   * Write data.
   */
  void printf(const char *format, ...);

  /**
   * Read a file into this buffer. Use a larger page size to read more bytes
   * a time for large files.
   */
  void read(FILE *in, int page_size = 1024);
  void read(File *in, int page_size = 1024);

private:
  // disabling copy constructor and assignment
  StringBuffer(const StringBuffer &sb) { assert(false); }
  StringBuffer &operator=(const StringBuffer &sb) {
    assert(false);
    return *this;
  }

  StringData* m_str;
  char *m_buffer;
  int m_initialCap;
  int m_maxBytes;
  int m_cap;
  int m_len;

  void growBy(int spaceRequired);
};

/**
 * StringBuffer-like wrapper for a malloc'd null-terminated C-String
 */
DECLARE_BOOST_TYPES(CstrBuffer);
class CstrBuffer {
 public:
  static const unsigned kMaxCap = INT_MAX;
  CstrBuffer(int len); // reserve space
  CstrBuffer(char* data, int len); // attach a malloc'd buffer
  explicit CstrBuffer(const char *filename); // read in a file
  ~CstrBuffer();
  const char* data() const;
  unsigned size() const { return m_len; }
  bool valid() const { return m_buffer != nullptr; }
  void append(const char* s, int len);
  String detach();

 private:
  char* m_buffer;
  unsigned m_len;
  unsigned m_cap;
};

inline const char* CstrBuffer::data() const {
  assert(m_len <= m_cap);
  m_buffer[m_len] = 0;
  return m_buffer;
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_STRING_BUFFER_H_

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

#ifndef __HPHP_STRING_BUFFER_H__
#define __HPHP_STRING_BUFFER_H__

#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/taint/taint_data.h>

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
  StringBuffer(int initialSize = 1024);
  StringBuffer(const char *filename);
  StringBuffer(char *data, int len); // attaching
  ~StringBuffer();

  void setOutputLimit(int maxBytes) { m_maxBytes = maxBytes;}

  bool valid() const { return m_buffer != NULL;}
  bool empty() const { return m_pos == 0;}
  int size() const { return m_pos;}
  int length() const { return m_pos;}
  const char *data();
  char charAt(int pos) const;

  /**
   * Detach buffer and yield a String. After this, do not use this StringBuffer
   * object any more.
   */
  char *detach(int &size);
  String detach();
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
  void append(int64 n);
  void append(char c) {
    if (m_buffer && m_pos + 1 <= m_size) {
      m_buffer[m_pos++] = c;
      return;
    }
    appendHelper(c);
  }
  void appendHelper(char c);
  void append(unsigned char c) { append((char)c);}
  void append(litstr  s) { ASSERT(s); append(s, strlen(s));}
  void append(CStrRef s);
  void append(const char *s, int len) {
    TAINT_OBSERVER_REGISTER_MUTATED(this);
    ASSERT(len >= 0);
    if (m_buffer && m_pos + len <= m_size) {
      memcpy(m_buffer + m_pos, s, len);
      m_pos += len;
      return;
    }
    appendHelper(s, len);
  }
  void appendHelper(const char *s, int len);
  void append(const std::string &s) { append(s.data(), s.size());}
  /**
   * Json-escape the string and then append it.
   */
  void appendJsonEscape(const char *s, int len, bool loose);

  StringBuffer &operator+=(int n)     { append(n); return *this;}
  StringBuffer &operator+=(char c)    { append(c); return *this;}
  StringBuffer &operator+=(litstr  s) { append(s); return *this;}
  StringBuffer &operator+=(CStrRef s) { append(s); return *this;}

  StringBuffer &add(const char *s, int len) { append(s, len); return *this; }
  StringBuffer &add(CStrRef s)        { append(s); return *this; }

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

#ifdef TAINTED
  TaintData* getTaintData() { return &m_taint_data; }
  const TaintData& getTaintDataRef() const { return m_taint_data; }
#endif

private:
  // disabling copy constructor and assignment
  StringBuffer(const StringBuffer &sb) { ASSERT(false);}
  StringBuffer &operator=(const StringBuffer &sb) {ASSERT(false);return *this;}

  char *m_buffer;
  int m_initialSize;
  int m_maxBytes;
  int m_size;
  int m_pos;
#ifdef TAINTED
  TaintData m_taint_data;
#endif

  void grow(int minSize);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_STRING_BUFFER_H__

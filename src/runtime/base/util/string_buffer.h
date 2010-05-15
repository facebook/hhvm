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

#ifndef __HPHP_STRING_BUFFER_H__
#define __HPHP_STRING_BUFFER_H__

#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class File;

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
  StringBuffer(int initialSize = 256);
  StringBuffer(const char *filename);
  StringBuffer(char *data, int len); // attaching
  ~StringBuffer();

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
  operator String() { return detach();}
  void reset();
  void clear() { reset();}
  void resize(int size);

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
  void append(char c);
  void append(unsigned char c) { append((char)c);}
  void append(litstr  s) { ASSERT(s); append(s, strlen(s));}
  void append(CStrRef s) { append(s.data(), s.size());}
  void append(const char *s, int len);
  void append(const std::string &s) { append(s.data(), s.size());}
  StringBuffer &operator+=(int n)     { append(n); return *this;}
  StringBuffer &operator+=(char c)    { append(c); return *this;}
  StringBuffer &operator+=(litstr  s) { append(s); return *this;}
  StringBuffer &operator+=(CStrRef s) { append(s); return *this;}

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
  StringBuffer(const StringBuffer &sb) { ASSERT(false);}
  StringBuffer &operator=(const StringBuffer &sb) {ASSERT(false);return *this;}

  char *m_buffer;
  int m_size;
  int m_pos;

  void grow(int minSize);
};

#if !defined(HPHP_AUTO_SBUFFER) || HPHP_AUTO_SBUFFER
#define StringBufferName(prefix,name) prefix##_sbuf_##name
#define DeclareStringBuffer(prefix,name,size) \
  StringBuffer StringBufferName(prefix,name)(size)
#define StringBufferAppend(prefix,name,value) \
  StringBufferName(prefix,name).append(value)
#define StringBufferDetach(prefix,name) \
  concat_assign(name, StringBufferName(prefix,name).detach())

#else

#define DeclareStringBuffer(prefix,name,size)
#define StringBufferAppend(prefix,name,value) concat_assign(name, value)
#define StringBufferDetach(prefix,name)

#endif

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_STRING_BUFFER_H__

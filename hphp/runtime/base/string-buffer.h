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

#ifndef incl_HPHP_STRING_BUFFER_H_
#define incl_HPHP_STRING_BUFFER_H_

#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/type-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class File;

struct StringBufferLimitException : FatalErrorException {
  StringBufferLimitException(int size, const String& partialResult)
    : FatalErrorException(0, "StringBuffer exceeded %d bytes of memory", size),
      m_result(partialResult) {}

  String m_result;
};

/*
 * Efficient string concatenation.
 *
 * StringBuffer is designed not to contain any malloc()d memory (only
 * per-request allocated memory) based on sweeping-related assumptions.
 */
struct StringBuffer {
  static constexpr uint32_t kDefaultOutputLimit = StringData::MaxSize;

  /*
   * Construct a string buffer with some initial size, subsequent allocation
   * will geometrically grow the size when needed.
   */
  explicit StringBuffer(uint32_t initialSize = SmallStringReserve);
  ~StringBuffer();

  StringBuffer(const StringBuffer& sb) = delete;
  StringBuffer& operator=(const StringBuffer& sb) = delete;

  /*
   * Set an "output limit" for this string buffer.  If any append goes
   * over this size, the StringBuffer will throw an exception.
   *
   * Pre: size() < maxBytes
   */
  void setOutputLimit(int maxBytes) {
    m_maxBytes = maxBytes > 0 ? maxBytes : kDefaultOutputLimit;
  }

  /*
   * Access the current state of the string.
   *
   * data() has the following semantics:
   *
   *   if size() > 0:
   *
   *     the pointer returned by to data() is a string of length
   *     size(), and is guaranteed to be null terminated.
   *
   *   if size() == 0:
   *
   *     the return value of data() may *either* be nullptr or a
   *     pointer to a zero-length string.  Calling code may assume it
   *     will not be null if it can guarantee detach() or release()
   *     have never been called.
   *
   * The pointer and size should be considered invalidated after any
   * call to a non-const member function on this class.
   */
  uint32_t size() const { return m_len; }
  const char* data() const;

  /*
   * Returns whether this string has length zero.
   */
  bool empty() const { return m_len == 0; }

  /*
   * Detach buffer and yield a String.
   *
   * Post: empty()
   */
  String detach();

  /*
   * Copy this buffer into a String.  The contents of this buffer
   * object are unchanged.
   */
  String copy() const;

  /*
   * Set the length of this string to zero.
   *
   * Post: empty()
   */
  void clear();

  /*
   * Set the size of this string to `size'.
   *
   * This function may only be used to reduce the size of the string,
   * or increase the size of the string to a value in range after a
   * call to appendCursor().
   *
   * Post: size() == size
   */
  void resize(uint32_t size);

  /*
   * Release all memory associated with this string buffer.
   *
   * Post: empty()
   */
  void release();

  /*
   * Return a pointer for writing up to `additionalBytes' more bytes.
   * The caller may write fewer bytes than this, and should call
   * resize() as appropriate or the new bytes will not be considered
   * part of the string.
   *
   * The caller *may* write one more byte than `additionalBytes', but
   * only if it is a null terminator.
   *
   * The returned pointer is invalidated after any call to a non-const
   * member function.
   */
  char* appendCursor(int additionalBytes);

  /*
   * Mutate a character in existing buffer.
   */
  void set(uint32_t offset, char c) {
    assert(offset < m_len);
    m_buffer[offset] = c;
  }

  /*
   * Append various types of things to this string.
   */
  void append(char c) {
    if (m_buffer && m_len < m_cap) {
      m_buffer[m_len++] = c;
      return;
    }
    appendHelper(c);
  }
  void append(unsigned char c) { append((char)c);}
  void append(const char* s) { assert(s); append(s, strlen(s)); }
  void append(const String& s) { append(s.data(), s.size()); }
  void append(const std::string& s) { append(s.data(), s.size()); }
  void append(const StringData* s) { append(s->data(), s->size()); }
  void append(StringSlice s) { append(s.ptr, s.len); }
  void append(const char* s, int len) {
    assert(len >= 0);
    if (m_buffer && len <= m_cap - m_len) {
      memcpy(m_buffer + m_len, s, len);
      m_len += len;
      return;
    }
    appendHelper(s, len);
  }
  void append(const Variant& s);
  void append(int n);
  void append(int64_t n);

  /*
   * Take ownership of the string being built by buf.
   *
   * Post: buf.size() == 0
   */
  void absorb(StringBuffer& buf);

  /*
   * Append to this string using a printf-style format specification.
   */
  void printf(const char* format, ...) ATTRIBUTE_PRINTF(2,3);

  /*
   * Read a file into this buffer. Use a larger page size to read more bytes
   * a time for large files.
   */
  void read(FILE *in, int page_size = 1024);
  void read(File *in, int page_size = 1024);

  template<class F> void scan(F& mark) const {
    mark(m_str);
  }
  template <typename F> friend void scan(const StringBuffer&, F&);

private:
  void appendHelper(const char* s, int len);
  void appendHelper(char c);
  void growBy(int spaceRequired);
  void makeValid(uint32_t minCap);
  bool valid() const { return m_buffer != nullptr; }

private:
  StringData* m_str;
  char *m_buffer;
  uint32_t m_initialCap;
  uint32_t m_maxBytes;
  uint32_t m_cap;                    // doesn't include null terminator
  uint32_t m_len;
};

/*
 * StringBuffer-like wrapper for a malloc'd, null-terminated C-style
 * string.
 */
struct CstrBuffer {
  static const unsigned kMaxCap = INT_MAX;

  /*
   * Create a buffer with enough space for a string of length `len'.
   * (I.e. an allocation of size len + 1.)
   *
   * Pre: len <= kMaxCap
   */
  explicit CstrBuffer(int len);

  /*
   * Take ownership of an existing malloc'd buffer containing a
   * null-terminated string of length `len'.  It is assumed the
   * capacity is also len.
   *
   * Pre: len < kMaxCap
   */
  CstrBuffer(char* data, int len);

  /*
   * Create a CstrBuffer, attempting to read the contents of a given
   * file.
   *
   * I/O errors are not reported.  size() will just be zero in that
   * case.
   *
   * Post: valid()
   */
  explicit CstrBuffer(const char* filename);

  CstrBuffer(const CstrBuffer&) = delete;
  CstrBuffer& operator=(const CstrBuffer&) = delete;
  ~CstrBuffer();

  /*
   * Read-only access to the data this buffer contains.  Guaranteed to
   * be null-terminated.
   *
   * Pre: valid()
   */
  const char* data() const;
  unsigned size() const { return m_len; }

  /*
   * Returns whether this CstrBuffer contains a buffer.  This can only
   * return false if detach() has been called.
   */
  bool valid() const { return m_buffer != nullptr; }

  /*
   * Append the supplied data to this string.
   *
   * Pre: valid()
   */
  void append(StringSlice slice);

  /*
   * Create a request-local string from this buffer.
   *
   * Pre: valid()
   * Post: !valid()
   */
  String detach();

private:
  char* m_buffer;
  unsigned m_len;
  unsigned m_cap; // doesn't include the space for the \0
};

inline const char* CstrBuffer::data() const {
  assert(m_len <= m_cap);
  m_buffer[m_len] = 0;
  return m_buffer;
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_STRING_BUFFER_H_

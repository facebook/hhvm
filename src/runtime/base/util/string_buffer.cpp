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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <runtime/base/util/string_buffer.h>
#include <util/alloc.h>
#include <runtime/base/file/file.h>
#include <runtime/base/zend/zend_functions.h>
#ifdef TAINTED
#include <runtime/base/tainting.h>
#include <runtime/base/propagate_tainting.h>
#include <runtime/base/tainted_metadata.h>
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

StringBuffer::StringBuffer(int initialSize /* = 1024 */)
    : m_initialSize(initialSize), m_size(initialSize), m_pos(0) {
  ASSERT(initialSize > 0);
  m_buffer = (char *)Util::safe_malloc(initialSize + 1);
  #ifdef TAINTED
  m_tainting = default_tainting;
  m_tainted_metadata = NULL;
  #endif
}

StringBuffer::StringBuffer(const char *filename)
    : m_buffer(NULL), m_initialSize(1024), m_size(0), m_pos(0) {
  struct stat sb;
  if (stat(filename, &sb) == 0) {
    m_size = sb.st_size;
    m_buffer = (char *)Util::safe_malloc(m_size + 1);

    int fd = ::open(filename, O_RDONLY);
    if (fd != -1) {
      while (m_pos < m_size) {
        int buffer_size = m_size - m_pos;
        int len = ::read(fd, m_buffer + m_pos, buffer_size);
        if (len == -1 && errno == EINTR) continue;
        if (len <= 0) break;
        m_pos += len;
      }
      ::close(fd);
    }
  }
  #ifdef TAINTED
  m_tainting = default_tainting;
  m_tainted_metadata = NULL;
  #endif
}

StringBuffer::StringBuffer(char *data, int len)
  : m_buffer(data), m_size(len), m_pos(len) {
  #ifdef TAINTED
  m_tainting = default_tainting;
  m_tainted_metadata = NULL;
  #endif
}

StringBuffer::~StringBuffer() {
  if (m_buffer) {
    free(m_buffer);
  }
}

const char *StringBuffer::data() {
  if (m_buffer && m_pos) {
    m_buffer[m_pos] = '\0'; // fixup
    return m_buffer;
  }
  return NULL;
}

char StringBuffer::charAt(int pos) const {
  ASSERT(pos >= 0 && pos < m_pos);
  if (m_buffer && pos >= 0 && pos < m_pos) {
    return m_buffer[pos];
  }
  return '\0';
}

char *StringBuffer::detach(int &size) {
  if (m_buffer) {
    if (m_pos) {
      m_buffer[m_pos] = '\0'; // fixup
      size = m_pos;
      char *ret = m_buffer;
      m_buffer = NULL;
      m_pos = 0;
      return ret;
    }
    size = 0;
  }
  return NULL;
}

String StringBuffer::detach() {
  if (m_buffer && m_pos) {
    m_buffer[m_pos] = '\0'; // fixup
    String ret(m_buffer, m_pos, AttachString);
    m_buffer = NULL;
    m_pos = 0;
#ifdef TAINTED
    propagate_tainting1_buf(*this, ret);
#endif
    return ret;
  }
  return String("");
}

String StringBuffer::copy() {
  return String(data(), size(), CopyString);
}

void StringBuffer::absorb(StringBuffer &buf) {
  if (empty()) {
    char *buffer = m_buffer;
    int size = m_size;

    m_buffer = buf.m_buffer;
    m_size = buf.m_size;
    m_pos = buf.m_pos;
#ifdef TAINTED
    propagate_tainting1_bufbuf( buf, *this );
#endif

    buf.m_buffer = buffer;
    buf.m_size = size;
    buf.reset();
  } else {
    append(buf.detach());
  }
}

void StringBuffer::reset() {
  m_pos = 0;
#ifdef TAINTED
  m_tainting = default_tainting;
  m_tainted_metadata = NULL;
#endif
}

void StringBuffer::release() {
  if (m_buffer) {
    free(m_buffer);
    m_buffer = NULL;
  }
}

void StringBuffer::resize(int size) {
  ASSERT(size >= 0 && size < m_size);
  if (size >= 0 && size < m_size) {
    m_pos = size;
  }
}

char *StringBuffer::reserve(int size) {
  if (m_size < m_pos + size) {
    m_size = m_pos + size;
    m_buffer = (char *)Util::safe_realloc(m_buffer, m_size + 1);
  } else if (m_buffer == NULL) {
    m_size = m_initialSize;
    m_buffer = (char *)Util::safe_malloc(m_size + 1);
  }
  return m_buffer + m_pos;
}

void StringBuffer::append(int n) {
  char buf[12];
  int is_negative;
  int len;
  char *p = conv_10(n, &is_negative, buf + 12, &len);
  append(p, len);
}

void StringBuffer::append(int64 n) {
  char buf[21];
  int is_negative;
  int len;
  char *p = conv_10(n, &is_negative, buf + 21, &len);
  append(p, len);
}

void StringBuffer::append(char ch) {
  if (m_buffer == NULL) {
    m_size = m_initialSize;
    m_buffer = (char *)Util::safe_malloc(m_size + 1);
  }

  if (m_pos + 1 > m_size) {
    grow(m_pos + 1);
  }
  m_buffer[m_pos++] = ch;
}


void StringBuffer::append(CStrRef s) {
  append(s.data(), s.size());
  #ifdef TAINTED
  propagate_tainting2_buf(s, *this, *this);
  #endif
}

void StringBuffer::append(const char *s, int len) {
  if (m_buffer == NULL) {
    m_size = m_initialSize;
    if (len > m_size) {
      m_size = len;
    }
    m_buffer = (char *)Util::safe_malloc(m_size + 1);
  }

  ASSERT(s);
  ASSERT(len >= 0);
  if (len <= 0) return;

  if (m_pos + len > m_size) {
    grow(m_pos + len);
  }
  memcpy(m_buffer + m_pos, s, len);
  m_pos += len;
}

void StringBuffer::printf(const char *format, ...) {
  va_list ap;
  va_start(ap, format);

  bool printed = false;
  for (int len = 1024; !printed; len <<= 1) {
    va_list v;
    va_copy(v, ap);

    char *buf = (char*)Util::safe_malloc(len);
    if (vsnprintf(buf, len, format, v) < len) {
      append(buf);
      printed = true;
    }
    free(buf);

    va_end(v);
  }

  va_end(ap);
}

void StringBuffer::read(FILE* in, int page_size /* = 1024 */) {
  ASSERT(in);
  ASSERT(page_size > 0);

  if (m_buffer == NULL) {
    m_size = m_initialSize;
    m_buffer = (char *)Util::safe_malloc(m_size + 1);
  }

  while (true) {
    int buffer_size = m_size - m_pos;
    if (buffer_size < page_size) {
      grow(m_pos + page_size);
      buffer_size = m_size - m_pos;
    }
    int len = fread(m_buffer + m_pos, 1, buffer_size, in);
    if (len == 0) break;
    m_pos += len;
  }
}

void StringBuffer::read(File* in, int page_size /* = 1024 */) {
  ASSERT(in);
  ASSERT(page_size > 0);

  if (m_buffer == NULL) {
    m_size = m_initialSize;
    m_buffer = (char *)Util::safe_malloc(m_size + 1);
  }

  while (true) {
    int buffer_size = m_size - m_pos;
    if (buffer_size < page_size) {
      grow(m_pos + page_size);
      buffer_size = m_size - m_pos;
    }
    int len = in->readImpl(m_buffer + m_pos, buffer_size);
    if (len == 0) break;
    m_pos += len;
  }
}

#ifdef TAINTED
void StringBuffer::setTaint(bitstring b){
  m_tainting = m_tainting | b;
  if(is_tainting_metadata(b)){
    // resetting the metadata
    if(m_tainted_metadata != NULL){
      delete m_tainted_metadata;
      m_tainted_metadata = NULL;
    }
    m_tainted_metadata = new TaintedMetadata();
  }
}
void StringBuffer::unsetTaint(bitstring b){
  m_tainting = m_tainting & (~b);
  if(is_tainting_metadata(b)){
    // erasing the metadata
    if(m_tainted_metadata != NULL){
      delete m_tainted_metadata;
      m_tainted_metadata = NULL;
    }
  }
}
TaintedMetadata* StringBuffer::getTaintedMetadata() const {
  return m_tainted_metadata;
}
#endif

void StringBuffer::grow(int minSize) {
  int new_size = m_size;
  new_size <<= 1;
  if (new_size < minSize) {
    new_size = minSize;
  }

  char *new_buffer;
  new_buffer = (char *)Util::safe_realloc(m_buffer, new_size + 1);

  m_size = new_size;
  m_buffer = new_buffer;
}

///////////////////////////////////////////////////////////////////////////////
}

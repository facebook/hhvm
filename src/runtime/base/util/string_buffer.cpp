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

#include <runtime/base/util/string_buffer.h>
#include <runtime/base/file/file.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

StringBuffer::StringBuffer(int initialSize /* = 256 */)
  : m_size(initialSize), m_pos(0) {
  ASSERT(initialSize > 0);
  m_buffer = (char *)malloc(initialSize + 1);
}

StringBuffer::StringBuffer(const char *filename)
  : m_buffer(NULL), m_size(0), m_pos(0) {
  struct stat sb;
  if (stat(filename, &sb) == 0) {
    m_size = sb.st_size;
    m_buffer = (char *)malloc(m_size + 1);

    FILE *f = fopen(filename, "r");
    if (f) {
      while (m_pos < m_size) {
        int buffer_size = m_size - m_pos;
        int len = fread(m_buffer + m_pos, 1, buffer_size, f);
        if (len == 0) break;
        m_pos += len;
      }
      fclose(f);
    }
  }
}

StringBuffer::StringBuffer(char *data, int len)
  : m_buffer(data), m_size(len), m_pos(len) {
}

StringBuffer::~StringBuffer() {
  if (m_buffer) {
    free(m_buffer);
  }
}

const char *StringBuffer::data() {
  ASSERT(m_buffer);

  if (m_pos) {
    m_buffer[m_pos] = '\0'; // fixup
    return m_buffer;
  }
  return NULL;
}

char StringBuffer::charAt(int pos) const {
  ASSERT(pos >= 0 && pos < m_pos);
  return m_buffer[pos];
}

char *StringBuffer::detach(int &size) {
  ASSERT(m_buffer);

  if (m_pos) {
    m_buffer[m_pos] = '\0'; // fixup
    size = m_pos;
    char *ret = m_buffer;
    m_buffer = NULL;
    m_pos = 0;
    return ret;
  }

  size = 0;
  return NULL;
}

String StringBuffer::detach() {
  ASSERT(m_buffer);

  if (m_pos) {
    m_buffer[m_pos] = '\0'; // fixup
    String ret(m_buffer, m_pos, AttachString);
    m_buffer = NULL;
    m_pos = 0;
    return ret;
  }
  return String("");
}

void StringBuffer::reset() {
  if (m_buffer == NULL) {
    m_buffer = (char *)malloc(m_size + 1);
  }
  m_pos = 0;
}

void StringBuffer::resize(int size) {
  ASSERT(size >= 0 && size < m_size);
  m_pos = size;
}

char *StringBuffer::reserve(int size) {
  if (m_size < m_pos + size) {
    m_size = m_pos + size;
    if (m_buffer == NULL) {
      m_buffer = (char *)malloc(m_size + 1);
    } else {
      m_buffer = (char *)realloc(m_buffer, m_size + 1);
    }
  }
  return m_buffer + m_pos;
}

void StringBuffer::append(int n) {
  char buf[12];
  snprintf(buf, sizeof(buf), "%d", n);
  append(buf);
}

void StringBuffer::append(int64 n) {
  char buf[24];
  snprintf(buf, sizeof(buf), "%lld", n);
  append(buf);
}

void StringBuffer::append(char ch) {
  ASSERT(m_buffer);

  if (m_pos + 1 > m_size) {
    grow(m_pos + 1);
  }
  m_buffer[m_pos++] = ch;
}

void StringBuffer::append(const char *s, int len) {
  ASSERT(m_buffer);

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

    char *buf = (char*)malloc(len);
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

void StringBuffer::grow(int minSize) {
  m_size <<= 1;
  if (m_size < minSize) m_size = minSize;
  m_buffer = (char *)realloc(m_buffer, m_size + 1);
}

///////////////////////////////////////////////////////////////////////////////
}

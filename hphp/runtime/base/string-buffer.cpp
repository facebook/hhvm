/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/string-buffer.h"

#include <algorithm>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "folly/Conv.h"

#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/ext/ext_json.h"

#include "hphp/util/alloc.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

StringBuffer::StringBuffer(int initialSize /* = SmallStringReserve */)
  : m_initialCap(initialSize)
  , m_maxBytes(kDefaultOutputLimit)
  , m_len(0)
{
  assert(initialSize > 0);
  m_str = StringData::Make(initialSize);
  auto const s = m_str->bufferSlice();
  m_buffer = s.ptr;
  m_cap = s.len;
}

StringBuffer::~StringBuffer() {
  if (m_str) {
    assert((m_str->setSize(0), true)); // appease StringData::checkSane()
    m_str->release();
  }
}

const char *StringBuffer::data() const {
  if (m_buffer && m_len) {
    m_buffer[m_len] = '\0'; // fixup
    return m_buffer;
  }
  return nullptr;
}

String StringBuffer::detach() {
  if (m_buffer && m_len) {
    assert(m_str && m_str->getCount() == 0);
    m_buffer[m_len] = '\0'; // fixup
    StringData* str = m_str;
    str->setSize(m_len);
    m_str = 0;
    m_buffer = 0;
    m_len = 0;
    m_cap = 0;
    return String(str); // causes incref
  }
  return empty_string;
}

String StringBuffer::copy() const {
  return String(data(), size(), CopyString);
}

void StringBuffer::absorb(StringBuffer& buf) {
  if (empty()) {
    StringData* str = m_str;

    m_str = buf.m_str;
    m_buffer = buf.m_buffer;
    m_len = buf.m_len;
    m_cap = buf.m_cap;

    buf.m_str = str;
    if (str) {
      buf.m_buffer = (char*)str->data();
      buf.m_len = str->size();
      buf.m_cap = str->capacity() - 1;
    } else {
      buf.m_buffer = 0;
      buf.m_len = 0;
      buf.m_cap = 0;
    }
    buf.clear();
    return;
  }

  append(buf.detach());
}

void StringBuffer::clear() {
  m_len = 0;
}

void StringBuffer::release() {
  if (m_str) {
    assert(m_str->getCount() == 0);
    m_buffer[m_len] = 0; // appease StringData::checkSane()
    m_str->release();
  }
  m_str = 0;
  m_buffer = 0;
  m_len = m_cap = 0;
}

void StringBuffer::resize(int size) {
  assert(size >= 0 && size <= m_cap);
  if (size >= 0 && size <= m_cap) {
    m_len = size;
  }
}

char* StringBuffer::appendCursor(int size) {
  if (!m_buffer) {
    makeValid(size);
  } else if (m_cap - m_len < size) {
    m_buffer[m_len] = 0;
    m_str->setSize(m_len);
    auto const tmp = m_str->reserve(m_len + size);
    if (UNLIKELY(tmp != m_str)) {
      assert(m_str->getCount() == 0);
      m_str->release();
      m_str = tmp;
    }
    auto const s = m_str->bufferSlice();
    m_buffer = s.ptr;
    m_cap = s.len;
  }
  return m_buffer + m_len;
}

void StringBuffer::append(int n) {
  char buf[12];
  int len;
  const StringData *sd = String::GetIntegerStringData(n);
  char *p;
  if (!sd) {
    auto sl = conv_10(n, buf + 12);
    p = const_cast<char*>(sl.ptr);
    len = sl.len;
  } else {
    p = (char *)sd->data();
    len = sd->size();
  }
  append(p, len);
}

void StringBuffer::append(int64_t n) {
  char buf[21];
  int len;
  const StringData *sd = String::GetIntegerStringData(n);
  char *p;
  if (!sd) {
    auto sl = conv_10(n, buf + 21);
    p = const_cast<char*>(sl.ptr);
    len = sl.len;
  } else {
    p = (char *)sd->data();
    len = sd->size();
  }
  append(p, len);
}

void StringBuffer::append(CVarRef v) {
  auto const cell = v.asCell();
  switch (cell->m_type) {
  case KindOfStaticString:
  case KindOfString:
    append(cell->m_data.pstr);
    break;
  case KindOfInt64:
    append(cell->m_data.num);
    break;
  default:
    append(v.toString());
  }
}

void StringBuffer::appendHelper(char ch) {
  if (!valid()) makeValid(1);
  if (m_len == m_cap) {
    growBy(1);
  }
  m_buffer[m_len++] = ch;
}

void StringBuffer::makeValid(int minCap) {
  assert(!valid());
  assert(!m_len);
  m_str = StringData::Make(std::max(m_initialCap, minCap));
  m_buffer = (char*)m_str->data();
  m_cap = m_str->capacity() - 1;
}

void StringBuffer::appendHelper(const char *s, int len) {
  if (!valid()) makeValid(len);

  assert(s);
  assert(len >= 0);
  if (len <= 0) return;

  if (len > m_cap - m_len) {
    growBy(len);
  }
  memcpy(m_buffer + m_len, s, len);
  m_len += len;
}

void StringBuffer::printf(const char *format, ...) {
  va_list ap;
  va_start(ap, format);

  bool printed = false;
  for (int len = 1024; !printed; len <<= 1) {
    va_list v;
    va_copy(v, ap);

    char *buf = (char*)smart_malloc(len);
    if (vsnprintf(buf, len, format, v) < len) {
      append(buf);
      printed = true;
    }
    smart_free(buf);

    va_end(v);
  }

  va_end(ap);
}

void StringBuffer::read(FILE* in, int page_size /* = 1024 */) {
  assert(in);
  assert(page_size > 0);

  if (!valid()) makeValid(page_size);
  while (true) {
    int buffer_size = m_cap - m_len;
    if (buffer_size < page_size) {
      growBy(page_size);
      buffer_size = m_cap - m_len;
    }
    size_t len = fread(m_buffer + m_len, 1, buffer_size, in);
    if (len == 0) break;
    m_len += len;
  }
}

void StringBuffer::read(File* in, int page_size /* = 1024 */) {
  assert(in);
  assert(page_size > 0);

  if (!valid()) makeValid(page_size);
  while (true) {
    int buffer_size = m_cap - m_len;
    if (buffer_size < page_size) {
      growBy(page_size);
      buffer_size = m_cap - m_len;
    }
    int64_t len = in->readImpl(m_buffer + m_len, buffer_size);
    assert(len >= 0);
    if (len == 0) break;
    m_len += len;
  }
}

void StringBuffer::growBy(int spaceRequired) {
  /*
   * The default initial size is a power-of-two minus 1.
   * This doubling scheme keeps the total block size a
   * power of two, which should be good for memory allocators.
   * But note there is no guarantee either that the initial size
   * is power-of-two minus 1, or that it stays that way
   * (new_size < minSize below).
   */
  long new_size = m_cap * 2L + 1;
  long minSize = m_cap + (long)spaceRequired;
  if (new_size < minSize) {
    new_size = minSize;
  }

  if (m_maxBytes > 0 && new_size > m_maxBytes) {
    if (minSize > m_maxBytes) {
      throw StringBufferLimitException(m_maxBytes, detach());
    } else {
      new_size = m_maxBytes;
    }
  }

  m_buffer[m_len] = 0;
  m_str->setSize(m_len);
  auto const tmp = m_str->reserve(new_size);
  if (UNLIKELY(tmp != m_str)) {
    assert(m_str->getCount() == 0);
    m_str->release();
    m_str = tmp;
  }
  auto const s = m_str->bufferSlice();
  m_buffer = s.ptr;
  m_cap = s.len;
}

//////////////////////////////////////////////////////////////////////

CstrBuffer::CstrBuffer(int cap)
  : m_buffer((char*)Util::safe_malloc(cap + 1)), m_len(0), m_cap(cap) {
  assert(unsigned(cap) <= kMaxCap);
}

CstrBuffer::CstrBuffer(const char *filename)
  : m_buffer(nullptr), m_len(0) {
  struct stat sb;
  if (stat(filename, &sb) == 0) {
    if (sb.st_size > kMaxCap - 1) {
      auto const str = folly::to<std::string>(
        "file ", filename, " is too large"
      );
      throw StringBufferLimitException(kMaxCap, String(str.c_str()));
    }
    m_cap = sb.st_size;
    m_buffer = (char *)Util::safe_malloc(m_cap + 1);

    int fd = ::open(filename, O_RDONLY);
    if (fd != -1) {
      while (m_len < m_cap) {
        int buffer_size = m_cap - m_len;
        int len = ::read(fd, m_buffer + m_len, buffer_size);
        if (len == -1 && errno == EINTR) continue;
        if (len <= 0) break;
        m_len += len;
      }
      ::close(fd);
    }
  }
}

CstrBuffer::CstrBuffer(char* data, int len)
  : m_buffer(data), m_len(len), m_cap(len) {
  assert(unsigned(len) < kMaxCap);
}

CstrBuffer::~CstrBuffer() {
  free(m_buffer);
}

void CstrBuffer::append(StringSlice slice) {
  auto const data = slice.ptr;
  auto const len = slice.len;

  assert(m_buffer && len >= 0);

  unsigned newlen = m_len + len;
  if (newlen + 1 > m_cap) {
    if (newlen + 1 > kMaxCap) {
      throw StringBufferLimitException(kMaxCap, detach());
    }
    unsigned newcap = Util::nextPower2(newlen + 1);
    m_buffer = (char*)Util::safe_realloc(m_buffer, newcap);
    m_cap = newcap - 1;
    assert(newlen + 1 <= m_cap);
  }
  memcpy(m_buffer + m_len, data, len);
  m_buffer[m_len = newlen] = 0;
}

String CstrBuffer::detach() {
  assert(m_len <= m_cap);
  m_buffer[m_len] = 0;
  String s(m_buffer, m_len, AttachString);
  m_buffer = 0;
  m_len = m_cap = 0;
  return s;
}

///////////////////////////////////////////////////////////////////////////////
}

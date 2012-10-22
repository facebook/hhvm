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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>
#include <runtime/base/util/string_buffer.h>
#include <util/alloc.h>
#include <runtime/base/file/file.h>
#include <runtime/base/zend/zend_functions.h>
#include <runtime/base/zend/utf8_decode.h>
#include <runtime/base/taint/taint_observer.h>
#include <runtime/ext/ext_json.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

StringBuffer::StringBuffer(int initialSize /* = 63 */)
  : m_initialCap(initialSize), m_maxBytes(kDefaultOutputLimit),
    m_len(0) {
  ASSERT(initialSize > 0);
  m_str = NEW(StringData)(initialSize);
  MutableSlice s = m_str->mutableSlice();
  m_buffer = s.ptr;
  m_cap = s.len;
  TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, dataIgnoreTaint());
}

StringBuffer::~StringBuffer() {
  if (m_str) {
    ASSERT((m_str->setSize(0), true)); // appease StringData::checkSane()
    DELETE(StringData)(m_str);
  }
}

const char *StringBuffer::data() const {
  TAINT_OBSERVER_REGISTER_ACCESSED(m_taint_data);
  if (m_buffer && m_len) {
    m_buffer[m_len] = '\0'; // fixup
    return m_buffer;
  }
  return NULL;
}

const char *StringBuffer::dataIgnoreTaint() const {
  if (m_buffer && m_len) {
    m_buffer[m_len] = '\0'; // fixup
    return m_buffer;
  }
  return NULL;
}

char StringBuffer::charAt(int pos) const {
  ASSERT(pos >= 0 && pos < m_len);
  if (m_buffer && pos >= 0 && pos < m_len) {
    return m_buffer[pos];
  }
  return '\0';
}

String StringBuffer::detachImpl() {
  TAINT_OBSERVER_REGISTER_ACCESSED(m_taint_data);
#ifdef TAINTED
  m_taint_data.unsetTaint(TAINT_BIT_ALL);
#endif

  if (m_buffer && m_len) {
    ASSERT(m_str && m_str->getCount() == 0);
    m_buffer[m_len] = '\0'; // fixup
    StringData* str = m_str;
    str->setSize(m_len);
    m_str = 0;
    m_buffer = 0;
    m_len = 0;
    m_cap = 0;
    return String(str); // causes incref
  }
  return String("");
}

String StringBuffer::copy() {
  // REGISTER_ACCESSED() is called by data()
  return String(data(), size(), CopyString);
}

String StringBuffer::copyWithTaint() {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
  return String(data(), size(), CopyString);
}

void StringBuffer::absorb(StringBuffer &buf) {
  if (empty()) {
    TAINT_OBSERVER_REGISTER_ACCESSED(buf.getTaintDataRefConst());
    TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, dataIgnoreTaint());

    StringData* str = m_str;

    m_str = buf.m_str;
    m_buffer = buf.m_buffer;
    m_len = buf.m_len;
    m_cap = buf.m_cap;

    buf.m_str = str;
    if (str) {
      buf.m_buffer = (char*)str->data();
      buf.m_len = str->size();
      buf.m_cap = str->capacity();
    } else {
      buf.m_buffer = 0;
      buf.m_len = 0;
      buf.m_cap = 0;
    }
    buf.reset();
  } else {
    // REGISTER_ACCESSED()/REGISTER_MUTATED() are called by append()/detach()
    append(buf.detach());
  }
}

void StringBuffer::reset() {
  m_len = 0;
#ifdef TAINTED
  m_taint_data.unsetTaint(TAINT_BIT_ALL);
#endif
}

void StringBuffer::release() {
  if (m_str) {
    m_buffer[m_len] = 0; // appease StringData::checkSane()
    DELETE(StringData)(m_str);
  }
  m_str = 0;
  m_buffer = 0;
  m_len = m_cap = 0;
}

void StringBuffer::resize(int size) {
  ASSERT(size >= 0 && size < m_cap);
  if (size >= 0 && size < m_cap) {
    m_len = size;
  }
}

char *StringBuffer::reserve(int size) {
  if (!m_buffer) {
    m_str = NEW(StringData)(std::max(m_initialCap, m_len + size));
    m_buffer = (char*)m_str->data();
    m_cap = m_str->capacity();
  } else if (m_cap - m_len < size) {
    m_buffer[m_len] = 0;
    m_str->setSize(m_len);
    MutableSlice s = m_str->reserve(m_len + size);
    m_buffer = s.ptr;
    m_cap = s.len;
  }
  return m_buffer + m_len;
}

void StringBuffer::append(int n) {
  char buf[12];
  int is_negative;
  int len;
  const StringData *sd = String::GetIntegerStringData(n);
  char *p;
  if (!sd) {
    p = conv_10(n, &is_negative, buf + 12, &len);
  } else {
    p = (char *)sd->data();
    len = sd->size();
  }
  append(p, len);
}

void StringBuffer::append(int64 n) {
  char buf[21];
  int is_negative;
  int len;
  const StringData *sd = String::GetIntegerStringData(n);
  char *p;
  if (!sd) {
    p = conv_10(n, &is_negative, buf + 21, &len);
  } else {
    p = (char *)sd->data();
    len = sd->size();
  }
  append(p, len);
}

void StringBuffer::append(CVarRef v) {
  Variant::TypedValueAccessor tva = v.getTypedAccessor();
  if (Variant::IsString(tva)) {
    append(Variant::GetAsString(tva));
  } else if (IS_INT_TYPE(Variant::GetAccessorType(tva))) {
    append(Variant::GetInt64(tva));
  } else {
    append(v.toString());
  }
}

void StringBuffer::appendHelper(char ch) {
  if (!m_buffer) reserve(1);
  if (m_len == m_cap) {
    growBy(1);
  }
  m_buffer[m_len++] = ch;
}


void StringBuffer::append(CStrRef s) {
  // REGISTER_MUTATED() is called by data()
  append(s.data(), s.size());
}

void StringBuffer::appendHelper(const char *s, int len) {
  if (!m_buffer) reserve(len);

  ASSERT(s);
  ASSERT(len >= 0);
  if (len <= 0) return;

  if (len > m_cap - m_len) {
    growBy(len);
  }
  memcpy(m_buffer + m_len, s, len);
  m_len += len;
}

#define REVERSE16(us)                                     \
  (((us & 0xf) << 12)      | (((us >> 4) & 0xf) << 8) |   \
  (((us >> 8) & 0xf) << 4) | ((us >> 12) & 0xf))          \

void StringBuffer::appendJsonEscape(const char *s, int len, int options) {
  if (len == 0) {
    append("\"\"", 2);
  } else {
    static const char digits[] = "0123456789abcdef";

    int start = size();
    append('"');

    UTF8To16Decoder decoder(s, len, options & k_JSON_FB_LOOSE);
    for (;;) {
      int c = decoder.decode();
      if (c == UTF8_END) {
        append('"');
        break;
      }
      if (c == UTF8_ERROR) {
        // discard the part that has been already decoded.
        resize(start);
        append("null", 4);
        break;
      }
      ASSERT(c >= 0);
      unsigned short us = (unsigned short)c;
      switch (us) {
      case '"':
        if (options & k_JSON_HEX_QUOT) {
          append("\\u0022", 6);
        } else {
          append("\\\"", 2);
        }
        break;
      case '\\': append("\\\\", 2); break;
      case '/':
        if (options & k_JSON_UNESCAPED_SLASHES) {
          append('/');
        } else {
          append("\\/", 2);
        }
        break;
      case '\b': append("\\b", 2);  break;
      case '\f': append("\\f", 2);  break;
      case '\n': append("\\n", 2);  break;
      case '\r': append("\\r", 2);  break;
      case '\t': append("\\t", 2);  break;
      case '<':
        if (options & k_JSON_HEX_TAG || options & k_JSON_FB_EXTRA_ESCAPES) {
          append("\\u003C", 6);
        } else {
          append('<');
        }
        break;
      case '>':
        if (options & k_JSON_HEX_TAG) {
          append("\\u003E", 6);
        } else {
          append('>');
        }
        break;
      case '&':
        if (options & k_JSON_HEX_AMP) {
          append("\\u0026", 6);
        } else {
          append('&');
        }
        break;
      case '\'':
        if (options & k_JSON_HEX_APOS) {
          append("\\u0027", 6);
        } else {
          append('\'');
        }
        break;
      case '@':
        if (options & k_JSON_FB_EXTRA_ESCAPES) {
          append("\\u0040", 6);
        } else {
          append('@');
        }
        break;
      case '%':
       	if (options & k_JSON_FB_EXTRA_ESCAPES) {
          append("\\u0025", 6);
       	} else {
          append('%');
        }
        break;
      default:
        if (us >= ' ' && (us & 127) == us) {
          append((char)us);
        } else {
          append("\\u", 2);
          us = REVERSE16(us);
          append(digits[us & ((1 << 4) - 1)]); us >>= 4;
          append(digits[us & ((1 << 4) - 1)]); us >>= 4;
          append(digits[us & ((1 << 4) - 1)]); us >>= 4;
          append(digits[us & ((1 << 4) - 1)]);
        }
        break;
      }
    }
  }
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
  ASSERT(in);
  ASSERT(page_size > 0);

  if (!m_buffer) reserve(page_size);
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
  ASSERT(in);
  ASSERT(page_size > 0);

  if (!m_buffer) reserve(page_size);
  while (true) {
    int buffer_size = m_cap - m_len;
    if (buffer_size < page_size) {
      growBy(page_size);
      buffer_size = m_cap - m_len;
    }
    int64 len = in->readImpl(m_buffer + m_len, buffer_size);
    ASSERT(len >= 0);
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
  MutableSlice s = m_str->reserve(new_size);
  m_buffer = s.ptr;
  m_cap = s.len;
}

CstrBuffer::CstrBuffer(int cap)
  : m_buffer((char*)Util::safe_malloc(cap + 1)), m_len(0), m_cap(cap) {
  ASSERT(unsigned(cap) <= kMaxCap);
  TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, dataIgnoreTaint());
}

CstrBuffer::CstrBuffer(const char *filename)
  : m_buffer(NULL), m_len(0) {
  struct stat sb;
  if (stat(filename, &sb) == 0) {
    if (sb.st_size > kMaxCap - 1) {
      std::ostringstream out;
      out << "file " << filename << " is too large";
      throw StringBufferLimitException(kMaxCap,
                                       String(out.str().c_str()));
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
  TAINT_OBSERVER_REGISTER_MUTATED(m_taint_data, dataIgnoreTaint());
}

CstrBuffer::CstrBuffer(char* data, int len)
  : m_buffer(data), m_len(len), m_cap(len) {
  ASSERT(unsigned(len) < kMaxCap);
}

CstrBuffer::~CstrBuffer() {
  free(m_buffer);
}

void CstrBuffer::append(const char* data, int len) {
  ASSERT(m_buffer && len >= 0);
  unsigned newlen = m_len + len;
  if (newlen + 1 > m_cap) {
    if (newlen + 1 > kMaxCap) {
      throw StringBufferLimitException(kMaxCap, detach());
    }
    unsigned newcap = Util::nextPower2(newlen + 1);
    m_buffer = (char*)Util::safe_realloc(m_buffer, newcap);
    m_cap = newcap - 1;
    ASSERT(newlen + 1 <= m_cap);
  }
  memcpy(m_buffer + m_len, data, len);
  m_buffer[m_len = newlen] = 0;
}

String CstrBuffer::detach() {
  ASSERT(m_len <= m_cap);
  TAINT_OBSERVER_REGISTER_ACCESSED(m_taint_data);
  m_buffer[m_len] = 0;
  String s(m_buffer, m_len, AttachString);
  m_buffer = 0;
  m_len = m_cap = 0;
  return s;
}


///////////////////////////////////////////////////////////////////////////////
}

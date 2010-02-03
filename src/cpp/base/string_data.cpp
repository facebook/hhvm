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

#include <cpp/base/string_data.h>
#include <cpp/base/shared/shared_variant.h>
#include <cpp/base/zend/zend_functions.h>
#include <cpp/base/util/exceptions.h>
#include <math.h>
#include <cpp/base/zend/zend_string.h>
#include <cpp/base/zend/zend_strtod.h>
#include <cpp/base/type_string.h>
#include <cpp/base/runtime_option.h>

namespace HPHP {

IMPLEMENT_SMART_ALLOCATION(StringData, SmartAllocatorImpl::NeedRestoreOnce);
///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

StringData::StringData(const char *data,
                       StringDataMode mode /* = AttachLiteral */)
  : m_len(0), m_data(NULL), m_shared(NULL) {
  assign(data, mode);
}

StringData::StringData(SharedVariant *shared)
  : m_len(0), m_data(NULL), m_shared(NULL) {
  assign(shared);
}

StringData::StringData(const char *data, int len, StringDataMode mode)
  : m_len(0), m_data(NULL), m_shared(NULL) {
  assign(data, len, mode);
}

StringData::~StringData() {
  releaseData();
}

void StringData::releaseData() {
  if ((m_len & (IsLinear | IsLiteral)) == 0) {
    if (isShared()) {
      m_shared->decRef();
    } else if (m_data) {
      free((void*)m_data);
    }
  }
}

void StringData::assign(const char *data, StringDataMode mode) {
  ASSERT(data);
  assign(data, strlen(data), mode);
}

void StringData::assign(const char *data, int len, StringDataMode mode) {
  ASSERT(data);
  ASSERT(len >= 0);
  ASSERT(mode >= 0 && mode < StringDataModeCount);

  if (len < 0 || (len & IsMask)) {
    throw InvalidArgumentException("len", len);
  }

  releaseData();
  m_shared = NULL;
  m_len = len;
  if (m_len) {
    switch (mode) {
    case CopyString:
      {
        char *buf = (char*)malloc(len + 1);
        buf[len] = '\0';
        memcpy(buf, data, len);
        m_data = buf;
      }
      break;
    case AttachLiteral:
      m_len |= IsLiteral;
      m_data = data;
      ASSERT(m_data[len] == '\0');// all PHP strings need NULL termination
      break;
    case AttachString:
      m_data = data;
      ASSERT(m_data[len] == '\0');// all PHP strings need NULL termination
      break;
    default:
      ASSERT(false);
      break;
    }
  } else {
    if (mode == AttachString) {
      free((void*)data); // we don't really need a malloc-ed empty string
    }
    m_len |= IsLiteral;
    m_data = "";
  }
  ASSERT(m_data);
}

void StringData::assign(SharedVariant *shared) {
  ASSERT(shared);
  releaseData();
  shared->incRef();
  m_shared = shared;
  m_data = m_shared->stringData();
  m_len = m_shared->stringLength() | IsShared;
  ASSERT(m_data);
}

void StringData::append(const char *s, int len) {
  if (len == 0) return;

  if (len < 0 || (len & IsMask)) {
    throw InvalidArgumentException("len", len);
  }

  if (!isMalloced()) {
    int newlen;
    m_data = string_concat(data(), size(), s, len, newlen);
    if (isShared()) {
      m_shared->decRef();
    }
    m_len = newlen;
  } else if (m_data == s) {
    int newlen;
    char *newdata = string_concat(data(), size(), s, len, newlen);
    releaseData();
    m_data = newdata;
    m_len = newlen;
  } else {
    int dataLen = size();
    ASSERT((m_data > s && m_data - s > len) ||
           (m_data < s && s - m_data > dataLen)); // no overlapping
    m_len = len + dataLen;
    m_data = (const char*)realloc((void*)m_data, m_len + 1);
    memcpy((void*)(m_data + dataLen), s, len);
    ((char*)m_data)[m_len] = '\0';
  }
}

StringData *StringData::copy(bool sharedMemory /* = false */) const {
  if (sharedMemory) {
    if (isLiteral()) {
      return new StringData(m_data, size(), AttachLiteral);
    }
    return new StringData(m_data, size(), CopyString);
  } else {
    if (isLiteral()) {
      return NEW(StringData)(m_data, size(), AttachLiteral);
    }
    return NEW(StringData)(m_data, size(), CopyString);
  }
}

void StringData::escalate() {
  ASSERT(isImmutable());

  int len = size();
  ASSERT(len);

  char *buf = (char*)malloc(len+1);
  memcpy(buf, data(), len);
  buf[len] = '\0';
  m_len = len;
  m_data = buf;
}

void StringData::dump() {
  const char *p = data();
  int len = size();

  printf("StringData(%d) (%s%s%s%d): [", _count,
         isLiteral() ? "literal " : "",
         isShared() ? "shared " : "",
         isLinear() ? "linear " : "",
         len);
  for (int i = 0; i < len; i++) {
    char ch = p[i];
    if (isprint(ch)) {
      std::cout << ch;
    } else {
      printf("\\%02x", ch);
    }
  }
  printf("]\n");
}

///////////////////////////////////////////////////////////////////////////////
// mutations

StringData *StringData::getChar(int offset) const {
  if (offset >= 0 && offset < size()) {
    char *buf = (char *)malloc(2);
    buf[0] = m_data[offset];
    buf[1] = 0;
    return NEW(StringData)(buf, 1, AttachString);
  }

  if (RuntimeOption::ThrowNotices) {
    throw UninitializedOffsetException(offset);
  }
  return NULL;
}

void StringData::setChar(int offset, CStrRef substring) {
  if (offset >= 0) {
    int len = size();
    if (len == 0) {
      // PHP will treat data as an array and we don't want to follow that.
      throw OffsetOutOfRangeException();
    }

    if (offset < len) {
      if (!substring.empty()) {
        setChar(offset, substring.data()[0]);
      } else {
        removeChar(offset);
      }
    } else {
      int newlen = offset + 1;
      char *buf = (char *)malloc(newlen + 1);
      memset(buf, ' ', newlen);
      buf[newlen] = 0;
      memcpy(buf, data(), len);
      if (!substring.empty()) buf[offset] = substring.data()[0];
      assign(buf, newlen, AttachString);
    }
  }
}

void StringData::setChar(int offset, char ch) {
  ASSERT(offset >= 0 && offset < size());
  if (isImmutable()) {
    escalate();
  }
  ((char*)m_data)[offset] = ch;
}

void StringData::removeChar(int offset) {
  ASSERT(offset >= 0 && offset < size());
  int len = size();
  if (isImmutable()) {
    char *data = (char*)malloc(len);
    if (offset) {
      memcpy(data, this->data(), offset);
    }
    if (offset < len - 1) {
      memcpy(data + offset, this->data() + offset + 1, len - offset - 1);
    }
    data[len] = 0;
    m_len = len;
    releaseData();
    m_data = data;
  } else {
    m_len = ((m_len & IsMask) | (len - 1));
    memmove((void*)(m_data + offset), m_data + offset + 1, len - offset);
  }
}

void StringData::inc() {
  if (empty()) {
    m_len = (IsLiteral | 1);
    m_data = "1";
    return;
  }
  if (isImmutable()) {
    escalate();
  }
  char *overflowed = increment_string((char *)m_data, size());
  if (overflowed) {
    assign(overflowed, AttachString);
  }
}

void StringData::negate() {
  if (empty()) return;
  ASSERT(!isImmutable());
  char *buf = (char*)m_data;
  int len = size();
  for (int i = 0; i < len; i++) {
    buf[i] = ~(buf[i]);
  }
}

///////////////////////////////////////////////////////////////////////////////
// type conversions

bool StringData::isNumeric() const {
  int len = size();
  if (len) {
    int64 lval; double dval;
    DataType ret = is_numeric_string(data(), len, &lval, &dval, 0);
    switch (ret) {
    case KindOfNull:   return false;
    case KindOfInt64:
    case KindOfDouble: return true;
    default:
      ASSERT(false);
      break;
    }
  }
  return false;
}

bool StringData::isInteger() const {
  int len = size();
  if (len) {
    int64 lval; double dval;
    DataType ret = is_numeric_string(data(), len, &lval, &dval, 0);
    switch (ret) {
    case KindOfNull:   return false;
    case KindOfInt64:  return true;
    case KindOfDouble: return false;
    default:
      ASSERT(false);
      break;
    }
  }
  return false;
}

bool StringData::isValidVariableName() const {
  return is_valid_var_name(data(), size());
}

bool StringData::toBoolean() const {
  return !empty() && !isZero();
}

int64 StringData::toInt64(int base /* = 10 */) const {
  return strtoll(data(), NULL, base);
}

double StringData::toDouble() const {
  int len = size();
  if (len) return zend_strtod(data(), NULL);
  return 0;
}

bool StringData::isStrictlyInteger(int64 &res) {
  int nKeyLength = size();
  const char* arKey = data();
  return is_strictly_integer(arKey, nKeyLength, res);
}

///////////////////////////////////////////////////////////////////////////////
// comparisons

int StringData::numericCompare(const StringData *v2) const {
  ASSERT(v2);

  int64 lval1, lval2;
  double dval1, dval2;
  DataType ret1, ret2;
  if ((ret1 = is_numeric_string(data(), size(),
                                &lval1, &dval1, 0)) == KindOfNull ||
      (ret1 == KindOfDouble && !finite(dval1)) ||
      (ret2 = is_numeric_string(v2->data(), v2->size(),
                                &lval2, &dval2, 0)) == KindOfNull ||
      (ret2 == KindOfDouble && !finite(dval2))) {
    return -2;
  }
  if (ret1 == KindOfInt64 && ret2 == KindOfInt64) {
    if (lval1 > lval2) return 1;
    if (lval1 == lval2) return 0;
    return -1;
  }
  if (ret1 == KindOfDouble && ret2 == KindOfDouble) {
    if (dval1 > dval2) return 1;
    if (dval1 == dval2) return 0;
    return -1;
  }
  if (ret1 == KindOfDouble) {
    ASSERT(ret2 == KindOfInt64);
    dval2 = (double)lval2;
  } else {
    ASSERT(ret1 == KindOfInt64);
    ASSERT(ret2 == KindOfDouble);
    dval1 = (double)lval1;
  }

  if (dval1 > dval2) return 1;
  if (dval1 == dval2) return 0;
  return -1;
}

int StringData::compare(const StringData *v2) const {
  ASSERT(v2);

  int ret = numericCompare(v2);
  if (ret < -1) {
    int len1 = size();
    int len2 = v2->size();
    int len = len1 < len2 ? len1 : len2;
    ret = memcmp(data(), v2->data(), len);
    if (ret) return ret;
    if (len1 == len2) return 0;
    return len < len1 ? 1 : -1;
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool StringData::calculate(int &totalSize) {
  if (m_data && !isLiteral()) {
    totalSize += (size() + 1); // ending NULL
    return true;
  }
  return false;
}

void StringData::backup(LinearAllocator &allocator) {
  allocator.backup(m_data, size() + 1);
}

void StringData::restore(const char *&data) {
  ASSERT(!isLiteral());
  m_data = data;
  m_len &= LenMask;
  m_len |= IsLinear;
}

void StringData::sweep() {
  releaseData();
}

///////////////////////////////////////////////////////////////////////////////
}

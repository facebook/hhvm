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

#include <runtime/base/complex_types.h>
#include <runtime/base/string_offset.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/comparisons.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/zend/zend_functions.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/zend/zend_printf.h>

namespace HPHP {

const String null_string = String();
const StaticString empty_string("");

///////////////////////////////////////////////////////////////////////////////
// statics

#define NUM_CONVERTED_INTEGERS \
  (String::MaxPrecomputedInteger - String::MinPrecomputedInteger + 1)

StringData const **String::converted_integers_raw;
StringData const **String::converted_integers;

String::IntegerStringDataMap String::integer_string_data_map;

void String::PreConvertInteger(int64_t n) {
  IntegerStringDataMap::const_iterator it =
    integer_string_data_map.find(n);
  if (it != integer_string_data_map.end()) return;
  integer_string_data_map[n] = StringData::convert_integer_helper(n);
}

const StringData *String::ConvertInteger(int64_t n) {
  StringData const **psd = converted_integers + n;
  const StringData *sd = StringData::convert_integer_helper(n);
  *psd = sd;
  return sd;
}

static int precompute_integers() ATTRIBUTE_COLD;
static int precompute_integers() {
  String::converted_integers_raw =
    (StringData const **)calloc(NUM_CONVERTED_INTEGERS, sizeof(StringData*));
  String::converted_integers = String::converted_integers_raw
    - String::MinPrecomputedInteger;
  if (RuntimeOption::serverExecutionMode()) {
    // Proactively populate, in order to increase cache locality for sequential
    // access patterns.
    for (int n = String::MinPrecomputedInteger;
         n <= String::MaxPrecomputedInteger; n++) {
      String::ConvertInteger(n);
    }
  }
  return NUM_CONVERTED_INTEGERS;
}

static int ATTRIBUTE_UNUSED initIntegers = precompute_integers();

///////////////////////////////////////////////////////////////////////////////
// constructors

String::~String() {}

StringData* buildStringData(int n) {
  char tmpbuf[12];
  char* p;
  int is_negative;
  int len;

  tmpbuf[11] = '\0';
  p = conv_10(n, &is_negative, &tmpbuf[11], &len);
  return NEW(StringData)(p, len, CopyString);
}

String::String(int n) {
  const StringData *sd = GetIntegerStringData(n);
  if (sd) {
    assert(sd->isStatic());
    m_px = (StringData *)sd;
    return;
  }
  m_px = buildStringData(n);
  m_px->setRefCount(1);
}

StringData* buildStringData(int64_t n) {
  char tmpbuf[21];
  char* p;
  int is_negative;
  int len;

  tmpbuf[20] = '\0';
  p = conv_10(n, &is_negative, &tmpbuf[20], &len);
  return NEW(StringData)(p, len, CopyString);
}

HOT_FUNC
String::String(int64_t n) {
  const StringData *sd = GetIntegerStringData(n);
  if (sd) {
    assert(sd->isStatic());
    m_px = (StringData *)sd;
    return;
  }
  m_px = buildStringData(n);
  m_px->setRefCount(1);
}

StringData* buildStringData(double n) {
  char *buf;

  if (n == 0.0) n = 0.0; // so to avoid "-0" output
  vspprintf(&buf, 0, "%.*G", 14, n);
  return NEW(StringData)(buf, AttachString);
}

String::String(double n) {
  m_px = buildStringData(n);
  m_px->setRefCount(1);
}

StringData* buildStringData(litstr s) {
  return NEW(StringData)(s, AttachLiteral);
}

///////////////////////////////////////////////////////////////////////////////
// informational

String String::substr(int start, int length /* = 0x7FFFFFFF */,
                      bool nullable /* = false */) const {
  StringSlice r = slice();
  // string_substr_check() will update start & length to a legal range.
  if (string_substr_check(r.len, start, length)) {
    return String(r.ptr + start, length, CopyString);
  }
  return nullable ? String() : String("", 0, CopyString);
}

String String::lastToken(char delimiter) {
  int pos = rfind(delimiter);
  if (pos >= 0) {
    return substr(pos + 1);
  }
  return *this;
}

int String::find(char ch, int pos /* = 0 */,
                 bool caseSensitive /* = true */) const {
  if (empty()) return -1;
  return string_find(m_px->data(), m_px->size(), ch, pos,
                     caseSensitive);
}

int String::find(const char *s, int pos /* = 0 */,
                 bool caseSensitive /* = true */) const {
  assert(s);
  if (empty()) return -1;
  if (*s && *(s+1) == 0) {
    return find(*s, pos, caseSensitive);
  }
  return string_find(m_px->data(), m_px->size(), s, strlen(s),
                     pos, caseSensitive);
}

HOT_FUNC
int String::find(CStrRef s, int pos /* = 0 */,
                 bool caseSensitive /* = true */) const {
  if (empty()) return -1;
  if (s.size() == 1) {
    return find(*s.data(), pos, caseSensitive);
  }
  return string_find(m_px->data(), m_px->size(),
                     s.data(), s.size(), pos, caseSensitive);
}

int String::rfind(char ch, int pos /* = 0 */,
                  bool caseSensitive /* = true */) const {
  if (empty()) return -1;
  return string_rfind(m_px->data(), m_px->size(), ch,
                      pos, caseSensitive);
}

int String::rfind(const char *s, int pos /* = 0 */,
                  bool caseSensitive /* = true */) const {
  assert(s);
  if (empty()) return -1;
  if (*s && *(s+1) == 0) {
    return rfind(*s, pos, caseSensitive);
  }
  return string_rfind(m_px->data(), m_px->size(), s, strlen(s),
                      pos, caseSensitive);
}

int String::rfind(CStrRef s, int pos /* = 0 */,
                  bool caseSensitive /* = true */) const {
  if (empty()) return -1;
  if (s.size() == 1) {
    return rfind(*s.data(), pos, caseSensitive);
  }
  return string_rfind(m_px->data(), m_px->size(),
                      s.data(), s.size(), pos, caseSensitive);
}

String String::replace(int start, int length, CStrRef replacement) const {
  int len = size();
  char *ret = string_replace(data(), len, start, length, replacement.data(),
                             replacement.size());
  return String(ret, len, AttachString);
}

String String::replace(CStrRef search, CStrRef replacement) const {
  int count;
  return replace(search, replacement, count, true);
}

String String::replace(CStrRef search, CStrRef replacement, int &count,
                       bool caseSensitive) const {
  count = 0;
  if (!search.empty() && !empty()) {
    int len = m_px->size();
    char *ret = string_replace(m_px->data(), len, search.data(), search.size(),
                               replacement.data(), replacement.size(), count,
                               caseSensitive);
    if (ret) {
      return String(ret, len, AttachString);
    }
  }
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// offset functions: cannot inline these due to dependencies

String String::rvalAt(CArrRef key) const {
  return rvalAtImpl(key.toInt32());
}

String String::rvalAt(CObjRef key) const {
  return rvalAtImpl(key.toInt32());
}

String String::rvalAt(CVarRef key) const {
  return rvalAtImpl(key.toInt32());
}

StringOffset String::lvalAt(CArrRef key) {
  return lvalAtImpl(key.toInt32());
}

StringOffset String::lvalAt(CObjRef key) {
  return lvalAtImpl(key.toInt32());
}

StringOffset String::lvalAt(CVarRef key) {
  return lvalAtImpl(key.toInt32());
}

char String::charAt(int pos) const {
  assert(pos >= 0 && pos <= size());
  const char *s = data();
  return s[pos];
}

///////////////////////////////////////////////////////////////////////////////
// assignments

String &String::operator=(litstr s) {
  if (m_px) decRefStr(m_px);
  if (s) {
    m_px = NEW(StringData)(s, AttachLiteral);
    m_px->setRefCount(1);
  } else {
    m_px = nullptr;
  }
  return *this;
}

String &String::operator=(StringData *data) {
  StringBase::operator=(data);
  return *this;
}

String &String::operator=(const std::string & s) {
  if (m_px) decRefStr(m_px);
  m_px = NEW(StringData)(s.c_str(), s.size(), CopyString);
  m_px->setRefCount(1);
  return *this;
}

HOT_FUNC
String &String::operator=(CStrRef str) {
  StringBase::operator=(str.m_px);
  return *this;
}

String &String::operator=(CVarRef var) {
  return operator=(var.toString());
}

///////////////////////////////////////////////////////////////////////////////
// concatenation and increments

String &String::operator+=(litstr s) {
  if (s && *s) {
    if (empty()) {
      m_px = NEW(StringData)(s, AttachLiteral);
      m_px->setRefCount(1);
    } else if (m_px->getCount() == 1) {
      m_px->append(s, strlen(s));
    } else {
      StringData* px = NEW(StringData)(m_px, s);
      px->setRefCount(1);
      decRefStr(m_px);
      m_px = px;
    }
  }
  return *this;
}

String &String::operator+=(CStrRef str) {
  if (!str.empty()) {
    if (empty()) {
      StringBase::operator=(str.m_px);
    } else if (m_px->getCount() == 1) {
      m_px->append(str.slice());
    } else {
      StringData* px = NEW(StringData)(m_px, str.slice());
      decRefStr(m_px);
      px->setRefCount(1);
      m_px = px;
    }
  }
  return *this;
}

String& String::operator+=(const StringSlice& slice) {
  if (slice.size() == 0) {
    return *this;
  }
  if (m_px && m_px->getCount() == 1) {
    m_px->append(slice);
    return *this;
  }
  if (empty()) {
    if (m_px) decRefStr(m_px);
    m_px = NEW(StringData)(slice.begin(), slice.size(), CopyString);
    m_px->setRefCount(1);
    return *this;
  }
  StringData* px = NEW(StringData)(m_px, slice);
  px->setRefCount(1);
  decRefStr(m_px);
  m_px = px;
  return *this;
}

String& String::operator+=(const MutableSlice& slice) {
  return (*this += StringSlice(slice.begin(), slice.size()));
}

String&& operator+(String&& lhs, litstr rhs) {
  lhs += rhs;
  return std::move(lhs);
}

String operator+(const String & lhs, litstr rhs) {
  if (lhs.empty()) return rhs;
  if (!rhs || !*rhs) return lhs;
  return NEW(StringData)(lhs.slice(), rhs);
}

HOT_FUNC
String&& operator+(String&& lhs, String&& rhs) {
  return std::move(lhs += rhs);
}

HOT_FUNC
String operator+(String&& lhs, const String & rhs) {
  return std::move(lhs += rhs);
}

HOT_FUNC
String operator+(const String & lhs, String&& rhs) {
  return NEW(StringData)(lhs.slice(), rhs.slice());
}

HOT_FUNC
String operator+(const String & lhs, const String & rhs) {
  if (lhs.empty()) return rhs;
  if (rhs.empty()) return lhs;
  return NEW(StringData)(lhs.slice(), rhs.slice());
}

String String::operator~() const {
  String ret(NEW(StringData)(slice(), CopyString));
  ret->negate();
  return ret;
}

String String::operator|(CStrRef v) const {
  return String(m_px).operator|=(v);
}

String String::operator&(CStrRef v) const {
  return String(m_px).operator&=(v);
}

String String::operator^(CStrRef v) const {
  return String(m_px).operator^=(v);
}

String &String::operator|=(CStrRef v) {
  const char *s1 = data();
  const char *s2 = v.data();
  int len1 = size();
  int len2 = v.size();
  int len;
  char *copy = nullptr;
  if (len2 > len1) {
    len = len2;
    copy = string_duplicate(s2, len2);
    for (int i = 0; i < len1; i++) copy[i] |= s1[i];
  } else {
    len = len1;
    copy = string_duplicate(s1, len1);
    for (int i = 0; i < len2; i++) copy[i] |= s2[i];
  }
  if (m_px) decRefStr(m_px);
  m_px = NEW(StringData)(copy, len, AttachString);
  m_px->setRefCount(1);
  return *this;
}

String &String::operator&=(CStrRef v) {
  const char *s1 = data();
  const char *s2 = v.data();
  int len1 = size();
  int len2 = v.size();
  int len;
  char *copy = nullptr;
  if (len2 < len1) {
    len = len2;
    copy = string_duplicate(s2, len2);
    for (int i = 0; i < len2; i++) copy[i] &= s1[i];
  } else {
    len = len1;
    copy = string_duplicate(s1, len1);
    for (int i = 0; i < len1; i++) copy[i] &= s2[i];
  }
  if (m_px) decRefStr(m_px);
  m_px = NEW(StringData)(copy, len, AttachString);
  m_px->setRefCount(1);
  return *this;
}

String &String::operator^=(CStrRef v) {
  const char *s1 = data();
  const char *s2 = v.data();
  int len1 = size();
  int len2 = v.size();
  int len;
  char *copy = nullptr;
  if (len2 < len1) {
    len = len2;
    copy = string_duplicate(s2, len2);
    for (int i = 0; i < len2; i++) copy[i] ^= s1[i];
  } else {
    len = len1;
    copy = string_duplicate(s1, len1);
    for (int i = 0; i < len1; i++) copy[i] ^= s2[i];
  }
  if (m_px) decRefStr(m_px);
  m_px = NEW(StringData)(copy, len, AttachString);
  m_px->setRefCount(1);
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// conversions

HOT_FUNC
VarNR String::toKey() const {
  if (!m_px) return VarNR(empty_string);
  int64_t n = 0;
  if (m_px->isStrictlyInteger(n)) {
    return VarNR(n);
  } else {
    return VarNR(m_px);
  }
}

///////////////////////////////////////////////////////////////////////////////
// comparisons

bool String::same(litstr v2) const {
  return HPHP::same(m_px, v2);
}

bool String::same(const StringData *v2) const {
  return HPHP::same(m_px, v2);
}

bool String::same(CStrRef v2) const {
  return HPHP::same(m_px, v2);
}

bool String::same(CArrRef v2) const {
  return HPHP::same(m_px, v2);
}

bool String::same(CObjRef v2) const {
  return HPHP::same(m_px, v2);
}

bool String::equal(litstr v2) const {
  return HPHP::equal(m_px, v2);
}

bool String::equal(const StringData *v2) const {
  return HPHP::equal(m_px, v2);
}

bool String::equal(CStrRef v2) const {
  return HPHP::equal(m_px, v2);
}

bool String::equal(CArrRef v2) const {
  return HPHP::equal(m_px, v2);
}

bool String::equal(CObjRef v2) const {
  return HPHP::equal(m_px, v2);
}

bool String::less(litstr v2) const {
  return HPHP::less(m_px, v2);
}

bool String::less(const StringData *v2) const {
  return HPHP::less(m_px, v2);
}

bool String::less(CStrRef v2) const {
  return HPHP::less(m_px, v2);
}

bool String::less(CArrRef v2) const {
  return HPHP::less(m_px, v2);
}

bool String::less(CObjRef v2) const {
  return HPHP::less(m_px, v2);
}

bool String::more(litstr v2) const {
  return HPHP::more(m_px, v2);
}

bool String::more(const StringData *v2) const {
  return HPHP::more(m_px, v2);
}

bool String::more(CStrRef v2) const {
  return HPHP::more(m_px, v2);
}

bool String::more(CArrRef v2) const {
  return HPHP::more(m_px, v2);
}

bool String::more(CObjRef v2) const {
  return HPHP::more(m_px, v2);
}

///////////////////////////////////////////////////////////////////////////////
// comparison operators

bool String::operator==(litstr v) const {
  return HPHP::equal(m_px, v);
}

bool String::operator!=(litstr v) const {
  return !HPHP::equal(m_px, v);
}

bool String::operator>=(litstr v) const {
  return more_or_equal(m_px, v);
}

bool String::operator<=(litstr v) const {
  return less_or_equal(m_px, v);
}

bool String::operator>(litstr v) const {
  return HPHP::more(m_px, v);
}

bool String::operator<(litstr v) const {
  return HPHP::less(m_px, v);
}

bool String::operator==(CStrRef v) const {
  return HPHP::equal(m_px, v);
}

bool String::operator!=(CStrRef v) const {
  return !HPHP::equal(m_px, v);
}

bool String::operator>=(CStrRef v) const {
  return more_or_equal(m_px, v);
}

bool String::operator<=(CStrRef v) const {
  return less_or_equal(m_px, v);
}

bool String::operator>(CStrRef v) const {
  return HPHP::more(m_px, v);
}

bool String::operator<(CStrRef v) const {
  return HPHP::less(m_px, v);
}

bool String::operator==(CVarRef v) const {
  return HPHP::equal(m_px, v);
}

bool String::operator!=(CVarRef v) const {
  return !HPHP::equal(m_px, v);
}

bool String::operator>=(CVarRef v) const {
  return more_or_equal(m_px, v);
}

bool String::operator<=(CVarRef v) const {
  return less_or_equal(m_px, v);
}

bool String::operator>(CVarRef v) const {
  return HPHP::more(m_px, v);
}

bool String::operator<(CVarRef v) const {
  return HPHP::less(m_px, v);
}

///////////////////////////////////////////////////////////////////////////////
// input/output

void String::serialize(VariableSerializer *serializer) const {
  if (m_px) {
    serializer->write(m_px->data(), m_px->size());
  } else {
    serializer->writeNull();
  }
}

void String::unserialize(VariableUnserializer *uns,
                         char delimiter0 /* = '"' */,
                         char delimiter1 /* = '"' */) {
  int64_t size = uns->readInt();
  if (size >= RuntimeOption::MaxSerializedStringSize) {
    throw Exception("Size of serialized string (%d) exceeds max", int(size));
  }
  if (size < 0) {
    throw Exception("Size of serialized string (%d) must not be negative",
                    int(size));
  }

  char ch = uns->readChar();
  if (ch != ':') {
    throw Exception("Expected ':' but got '%c'", ch);
  }
  ch = uns->readChar();
  if (ch != delimiter0) {
    throw Exception("Expected '%c' but got '%c'", delimiter0, ch);
  }
  StringData *px = NEW(StringData)(int(size));
  MutableSlice buf = px->mutableSlice();
  assert(size <= buf.len);
  uns->read(buf.ptr, size);
  px->setSize(size);
  if (m_px) decRefStr(m_px);
  m_px = px;
  px->setRefCount(1);

  ch = uns->readChar();
  if (ch != delimiter1) {
    throw Exception("Expected '%c' but got '%c'", delimiter1, ch);
  }
}

///////////////////////////////////////////////////////////////////////////////
// debugging

void String::dump() const {
  if (m_px) {
    m_px->dump();
  } else {
    printf("(null)\n");
  }
}

///////////////////////////////////////////////////////////////////////////////
// StaticString

StaticString::StaticString(litstr s) {
  StackStringData sd(s);
  m_px = StringData::GetStaticString(&sd);
}

StaticString::StaticString(litstr s, int length) {
  StackStringData sd(s, length, AttachLiteral);
  m_px = StringData::GetStaticString(&sd);
}

StaticString::StaticString(std::string s) {
  StackStringData sd(s.c_str(), s.size(), CopyString);
  m_px = StringData::GetStaticString(&sd);
}

StaticString::StaticString(const StaticString &str) {
  assert(m_px->isStatic());
  m_px = str.m_px;
}

StaticString& StaticString::operator=(const StaticString &str) {
  // Assignment to a StaticString is ignored. Generated code
  // should never use a StaticString on the left-hand side of
  // assignment. A StaticString can only be initialized by a
  // StaticString constructor or StaticString::init().
  always_assert(false);
  return *this;
}

String::String(Variant&& src) : StringBase(src.toString()) {
}

String& String::operator=(Variant&& src) {
  return *this = src.toString();
}

void String::checkStaticHelper() {
  if (StringData* t = StringData::FindStaticString(m_px)) {
    decRefStr(m_px);
    m_px = t;
  }
}

//////////////////////////////////////////////////////////////////////////////
}

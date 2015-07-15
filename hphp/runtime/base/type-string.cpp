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

#include "hphp/runtime/base/type-string.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-printf.h"

#include <algorithm>

namespace HPHP {

const String null_string = String();
const StaticString empty_string_ref("");

///////////////////////////////////////////////////////////////////////////////
// statics

#define NUM_CONVERTED_INTEGERS \
  (String::MaxPrecomputedInteger - String::MinPrecomputedInteger + 1)

StringData const **String::converted_integers_raw;
StringData const **String::converted_integers;

String::IntegerStringDataMap String::integer_string_data_map;

static const StringData* convert_integer_helper(int64_t n) {
  char tmpbuf[21];
  tmpbuf[20] = '\0';
  auto sl = conv_10(n, &tmpbuf[20]);
  return makeStaticString(sl.ptr, sl.len);
}

void String::PreConvertInteger(int64_t n) {
  IntegerStringDataMap::const_iterator it =
    integer_string_data_map.find(n);
  if (it != integer_string_data_map.end()) return;
  integer_string_data_map[n] = convert_integer_helper(n);
}

const StringData *String::ConvertInteger(int64_t n) {
  StringData const **psd = converted_integers + n;
  const StringData *sd = convert_integer_helper(n);
  *psd = sd;
  return sd;
}

static int precompute_integers();
static int precompute_integers() {
  String::converted_integers_raw =
    (StringData const **)calloc(NUM_CONVERTED_INTEGERS, sizeof(StringData*));
  String::converted_integers = String::converted_integers_raw
    - String::MinPrecomputedInteger;
  if (RuntimeOption::ServerExecutionMode()) {
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

  tmpbuf[11] = '\0';
  auto sl = conv_10(n, &tmpbuf[11]);
  return StringData::Make(sl, CopyString);
}

req::ptr<StringData> String::buildString(int n) {
  const StringData* sd = GetIntegerStringData(n);
  if (sd) {
    assert(sd->isStatic());
    return req::ptr<StringData>::attach(const_cast<StringData*>(sd));
  }
  return req::ptr<StringData>::attach(buildStringData(n));
}

String::String(int n) : m_str(buildString(n)) { }

StringData* buildStringData(int64_t n) {
  char tmpbuf[21];

  tmpbuf[20] = '\0';
  auto const sl = conv_10(n, &tmpbuf[20]);
  return StringData::Make(sl, CopyString);
}

req::ptr<StringData> String::buildString(int64_t n) {
  const StringData* sd = GetIntegerStringData(n);
  if (sd) {
    assert(sd->isStatic());
    return req::ptr<StringData>::attach(const_cast<StringData*>(sd));
  }
  return req::ptr<StringData>::attach(buildStringData(n));
}

String::String(int64_t n) : m_str(buildString(n)) { }

void formatPhpDblStr(char **pbuf, double n) {
  if (n == 0.0) n = 0.0; // so to avoid "-0" output
  vspprintf(pbuf, 0, "%.*G", 14, n);
}

StringData* buildStringData(double n) {
  char *buf = nullptr;
  formatPhpDblStr(&buf, n);
  return StringData::Make(buf, AttachString);
}

std::string convDblToStrWithPhpFormat(double n) {
  char *buf = nullptr;
  formatPhpDblStr(&buf, n);
  std::string retVal(buf);
  free(buf);
  return retVal;
}

String::String(double n) : m_str(buildStringData(n), NoIncRef{}) { }

String::String(Variant&& src) : String(src.toString()) { }

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

int String::find(char ch, int pos /* = 0 */,
                 bool caseSensitive /* = true */) const {
  if (empty()) return -1;
  return string_find(m_str->data(), m_str->size(), ch, pos,
                     caseSensitive);
}

int String::find(const char *s, int pos /* = 0 */,
                 bool caseSensitive /* = true */) const {
  assert(s);
  if (empty()) return -1;
  if (*s && *(s+1) == 0) {
    return find(*s, pos, caseSensitive);
  }
  return string_find(m_str->data(), m_str->size(), s, strlen(s),
                     pos, caseSensitive);
}

int String::find(const String& s, int pos /* = 0 */,
                 bool caseSensitive /* = true */) const {
  if (empty()) return -1;
  if (s.size() == 1) {
    return find(*s.data(), pos, caseSensitive);
  }
  return string_find(m_str->data(), m_str->size(),
                     s.data(), s.size(), pos, caseSensitive);
}

int String::rfind(char ch, int pos /* = 0 */,
                  bool caseSensitive /* = true */) const {
  if (empty()) return -1;
  return string_rfind(m_str->data(), m_str->size(), ch,
                      pos, caseSensitive);
}

int String::rfind(const char *s, int pos /* = 0 */,
                  bool caseSensitive /* = true */) const {
  assert(s);
  if (empty()) return -1;
  if (*s && *(s+1) == 0) {
    return rfind(*s, pos, caseSensitive);
  }
  return string_rfind(m_str->data(), m_str->size(), s, strlen(s),
                      pos, caseSensitive);
}

int String::rfind(const String& s, int pos /* = 0 */,
                  bool caseSensitive /* = true */) const {
  if (empty()) return -1;
  if (s.size() == 1) {
    return rfind(*s.data(), pos, caseSensitive);
  }
  return string_rfind(m_str->data(), m_str->size(),
                      s.data(), s.size(), pos, caseSensitive);
}

///////////////////////////////////////////////////////////////////////////////
// offset functions: cannot inline these due to dependencies

String String::rvalAt(const Array& key) const {
  return rvalAtImpl(key.toInt32());
}

String String::rvalAt(const Object& key) const {
  return rvalAtImpl(key.toInt32());
}

String String::rvalAt(const Variant& key) const {
  return rvalAtImpl(key.toInt32());
}

char String::charAt(int pos) const {
  assert(pos >= 0 && pos <= size());
  const char *s = data();
  return s[pos];
}

///////////////////////////////////////////////////////////////////////////////
// assignments

String& String::operator=(const char* s) {
  m_str = req::ptr<StringData>::attach(
    s ? StringData::Make(s, CopyString) : nullptr);
  return *this;
}

String& String::operator=(const std::string& s) {
  m_str = req::ptr<StringData>::attach(
    StringData::Make(s.c_str(), s.size(), CopyString));
  return *this;
}

String& String::operator=(const Variant& var) {
  return operator=(var.toString());
}

String& String::operator=(Variant&& var) {
  return operator=(var.toString());
}

///////////////////////////////////////////////////////////////////////////////
// concatenation and increments

String &String::operator+=(const char* s) {
  if (s && *s) {
    if (empty()) {
      m_str = req::ptr<StringData>::attach(StringData::Make(s, CopyString));
    } else if (m_str->hasExactlyOneRef()) {
      auto const tmp = m_str->append(StringSlice(s, strlen(s)));
      if (UNLIKELY(tmp != m_str)) {
        m_str = req::ptr<StringData>::attach(tmp);
      }
    } else {
      m_str =
        req::ptr<StringData>::attach(StringData::Make(m_str.get(), s));
    }
  }
  return *this;
}

String &String::operator+=(const String& str) {
  if (!str.empty()) {
    if (empty()) {
      m_str = str.m_str;
    } else if (m_str->hasExactlyOneRef()) {
      auto tmp = m_str->append(str.slice());
      if (UNLIKELY(tmp != m_str)) {
        m_str = req::ptr<StringData>::attach(tmp);
      }
    } else {
      m_str = req::ptr<StringData>::attach(
        StringData::Make(m_str.get(), str.slice())
      );
    }
  }
  return *this;
}

String& String::operator+=(const StringSlice& slice) {
  if (slice.size() == 0) {
    return *this;
  }
  if (m_str && m_str->hasExactlyOneRef()) {
    auto const tmp = m_str->append(slice);
    if (UNLIKELY(tmp != m_str)) {
      m_str = req::ptr<StringData>::attach(tmp);
    }
    return *this;
  }
  if (empty()) {
    m_str = req::ptr<StringData>::attach(
      StringData::Make(slice.begin(), slice.size(), CopyString));
    return *this;
  }
  m_str = req::ptr<StringData>::attach(
    StringData::Make(m_str.get(), slice)
  );
  return *this;
}

String& String::operator+=(const MutableSlice& slice) {
  return (*this += StringSlice(slice.begin(), slice.size()));
}

String&& operator+(String&& lhs, const char* rhs) {
  lhs += rhs;
  return std::move(lhs);
}

String operator+(const String & lhs, const char* rhs) {
  if (lhs.empty()) return rhs;
  if (!rhs || !*rhs) return lhs;
  return String::attach(StringData::Make(lhs.slice(), rhs));
}

String&& operator+(String&& lhs, String&& rhs) {
  return std::move(lhs += rhs);
}

String operator+(String&& lhs, const String & rhs) {
  return std::move(lhs += rhs);
}

String operator+(const String & lhs, String&& rhs) {
  return String::attach(StringData::Make(lhs.slice(), rhs.slice()));
}

String operator+(const String & lhs, const String & rhs) {
  if (lhs.empty()) return rhs;
  if (rhs.empty()) return lhs;
  return String::attach(StringData::Make(lhs.slice(), rhs.slice()));
}

///////////////////////////////////////////////////////////////////////////////
// conversions

VarNR String::toKey() const {
  if (!m_str) return VarNR(staticEmptyString());
  int64_t n = 0;
  if (m_str->isStrictlyInteger(n)) {
    return VarNR(n);
  } else {
    return VarNR(m_str.get());
  }
}

///////////////////////////////////////////////////////////////////////////////
// comparisons

bool String::same(const StringData *v2) const {
  return HPHP::same(get(), v2);
}

bool String::same(const String& v2) const {
  return HPHP::same(get(), v2);
}

bool String::same(const Array& v2) const {
  return HPHP::same(get(), v2);
}

bool String::same(const Object& v2) const {
  return HPHP::same(get(), v2);
}

bool String::same(const Resource& v2) const {
  return HPHP::same(get(), v2);
}

bool String::equal(const StringData *v2) const {
  return HPHP::equal(get(), v2);
}

bool String::equal(const String& v2) const {
  return HPHP::equal(get(), v2);
}

bool String::equal(const Array& v2) const {
  return HPHP::equal(get(), v2);
}

bool String::equal(const Object& v2) const {
  return HPHP::equal(get(), v2);
}

bool String::equal(const Resource& v2) const {
  return HPHP::equal(get(), v2);
}

bool String::less(const StringData *v2) const {
  return HPHP::less(get(), v2);
}

bool String::less(const String& v2) const {
  return HPHP::less(get(), v2);
}

bool String::less(const Array& v2) const {
  return HPHP::less(get(), v2);
}

bool String::less(const Object& v2) const {
  return HPHP::less(get(), v2);
}

bool String::less(const Resource& v2) const {
  return HPHP::less(get(), v2);
}

bool String::more(const StringData *v2) const {
  return HPHP::more(get(), v2);
}

bool String::more(const String& v2) const {
  return HPHP::more(get(), v2);
}

bool String::more(const Array& v2) const {
  return HPHP::more(get(), v2);
}

bool String::more(const Object& v2) const {
  return HPHP::more(get(), v2);
}

bool String::more(const Resource& v2) const {
  return HPHP::more(get(), v2);
}

int String::compare(const char* v2) const {
  int lengthDiff = length() - strlen(v2);
  if(lengthDiff == 0)
    return memcmp(data(), v2, length());
  else
    return lengthDiff;
}

int String::compare(const String& v2) const {
  int lengthDiff = length() - v2.length();
  if(lengthDiff == 0)
    return memcmp(data(), v2.data(), length());
  else
    return lengthDiff;
}

///////////////////////////////////////////////////////////////////////////////
// comparison operators

bool String::operator==(const String& v) const {
  return HPHP::equal(get(), v);
}

bool String::operator!=(const String& v) const {
  return !HPHP::equal(get(), v);
}

bool String::operator>(const String& v) const {
  return HPHP::more(get(), v);
}

bool String::operator<(const String& v) const {
  return HPHP::less(get(), v);
}

bool String::operator==(const Variant& v) const {
  return HPHP::equal(get(), v);
}

bool String::operator!=(const Variant& v) const {
  return !HPHP::equal(get(), v);
}

bool String::operator>(const Variant& v) const {
  return HPHP::more(get(), v);
}

bool String::operator<(const Variant& v) const {
  return HPHP::less(get(), v);
}

///////////////////////////////////////////////////////////////////////////////
// input/output

void String::serialize(VariableSerializer *serializer) const {
  if (m_str) {
    serializer->write(m_str->data(), m_str->size());
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

  uns->expectChar(':');
  uns->expectChar(delimiter0);

  auto px = req::ptr<StringData>::attach(StringData::Make(int(size)));
  auto const buf = px->bufferSlice();
  assert(size <= buf.len);
  uns->read(buf.ptr, size);
  px->setSize(size);
  m_str = std::move(px);
  uns->expectChar(delimiter1);
}

///////////////////////////////////////////////////////////////////////////////
// debugging

void String::dump() const {
  if (m_str) {
    m_str->dump();
  } else {
    printf("(null)\n");
  }
}

///////////////////////////////////////////////////////////////////////////////
// StaticString

StaticString::StaticString(const char* s)
: String(makeStaticString(s), NoIncRef{}) { }

StaticString::StaticString(const char* s, int length)
: String(makeStaticString(s, length), NoIncRef{}) { }

StaticString::StaticString(std::string s)
: String(makeStaticString(s.c_str(), s.size()), NoIncRef{}) { }

StaticString& StaticString::operator=(const StaticString &str) {
  // Assignment to a StaticString is ignored. Generated code
  // should never use a StaticString on the left-hand side of
  // assignment. A StaticString can only be initialized by a
  // StaticString constructor or StaticString::init().
  always_assert(false);
  return *this;
}

const StaticString
  s_NULL("NULL"),
  s_boolean("boolean"),
  s_integer("integer"),
  s_double("double"),
  s_string("string"),
  s_array("array"),
  s_object("object"),
  s_resource("resource"),
  s_ref("reference");

StaticString getDataTypeString(DataType t) {
  switch (t) {
    case KindOfUninit:
    case KindOfNull:       return s_NULL;
    case KindOfBoolean:    return s_boolean;
    case KindOfInt64:      return s_integer;
    case KindOfDouble:     return s_double;
    case KindOfStaticString:
    case KindOfString:     return s_string;
    case KindOfArray:      return s_array;
    case KindOfObject:     return s_object;
    case KindOfResource:   return s_resource;
    case KindOfRef:        return s_ref;

    case KindOfClass:
      break;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////////////
}

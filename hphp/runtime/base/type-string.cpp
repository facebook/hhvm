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

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-printf.h"

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

  tmpbuf[20] = '\0';
  auto const sl = conv_10(n, &tmpbuf[20]);
  return StringData::Make(sl, CopyString);
}

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
  return StringData::Make(buf, AttachString);
}

String::String(double n) {
  m_px = buildStringData(n);
  m_px->setRefCount(1);
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

int String::find(const String& s, int pos /* = 0 */,
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

int String::rfind(const String& s, int pos /* = 0 */,
                  bool caseSensitive /* = true */) const {
  if (empty()) return -1;
  if (s.size() == 1) {
    return rfind(*s.data(), pos, caseSensitive);
  }
  return string_rfind(m_px->data(), m_px->size(),
                      s.data(), s.size(), pos, caseSensitive);
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
    m_px = StringData::Make(s, CopyString);
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
  m_px = StringData::Make(s.c_str(), s.size(), CopyString);
  m_px->setRefCount(1);
  return *this;
}

String &String::operator=(const String& str) {
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
      m_px = StringData::Make(s, CopyString);
      m_px->setRefCount(1);
    } else if (m_px->getCount() == 1) {
      auto const tmp = m_px->append(StringSlice(s, strlen(s)));
      if (UNLIKELY(tmp != m_px)) StringBase::operator=(tmp);
    } else {
      StringData* px = StringData::Make(m_px, s);
      px->setRefCount(1);
      decRefStr(m_px);
      m_px = px;
    }
  }
  return *this;
}

String &String::operator+=(const String& str) {
  if (!str.empty()) {
    if (empty()) {
      StringBase::operator=(str.m_px);
    } else if (m_px->getCount() == 1) {
      auto tmp = m_px->append(str.slice());
      if (UNLIKELY(tmp != m_px)) StringBase::operator=(tmp);
    } else {
      StringData* px = StringData::Make(m_px, str.slice());
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
    auto const tmp = m_px->append(slice);
    if (UNLIKELY(tmp != m_px)) StringBase::operator=(tmp);
    return *this;
  }
  if (empty()) {
    if (m_px) decRefStr(m_px);
    m_px = StringData::Make(slice.begin(), slice.size(), CopyString);
    m_px->setRefCount(1);
    return *this;
  }
  StringData* px = StringData::Make(m_px, slice);
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
  return StringData::Make(lhs.slice(), rhs);
}

String&& operator+(String&& lhs, String&& rhs) {
  return std::move(lhs += rhs);
}

String operator+(String&& lhs, const String & rhs) {
  return std::move(lhs += rhs);
}

String operator+(const String & lhs, String&& rhs) {
  return StringData::Make(lhs.slice(), rhs.slice());
}

String operator+(const String & lhs, const String & rhs) {
  if (lhs.empty()) return rhs;
  if (rhs.empty()) return lhs;
  return StringData::Make(lhs.slice(), rhs.slice());
}

///////////////////////////////////////////////////////////////////////////////
// conversions

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

bool String::same(const StringData *v2) const {
  return HPHP::same(m_px, v2);
}

bool String::same(const String& v2) const {
  return HPHP::same(m_px, v2);
}

bool String::same(CArrRef v2) const {
  return HPHP::same(m_px, v2);
}

bool String::same(CObjRef v2) const {
  return HPHP::same(m_px, v2);
}

bool String::same(CResRef v2) const {
  return HPHP::same(m_px, v2);
}

bool String::equal(const StringData *v2) const {
  return HPHP::equal(m_px, v2);
}

bool String::equal(const String& v2) const {
  return HPHP::equal(m_px, v2);
}

bool String::equal(CArrRef v2) const {
  return HPHP::equal(m_px, v2);
}

bool String::equal(CObjRef v2) const {
  return HPHP::equal(m_px, v2);
}

bool String::equal(CResRef v2) const {
  return HPHP::equal(m_px, v2);
}

bool String::less(const StringData *v2) const {
  return HPHP::less(m_px, v2);
}

bool String::less(const String& v2) const {
  return HPHP::less(m_px, v2);
}

bool String::less(CArrRef v2) const {
  return HPHP::less(m_px, v2);
}

bool String::less(CObjRef v2) const {
  return HPHP::less(m_px, v2);
}

bool String::less(CResRef v2) const {
  return HPHP::less(m_px, v2);
}

bool String::more(const StringData *v2) const {
  return HPHP::more(m_px, v2);
}

bool String::more(const String& v2) const {
  return HPHP::more(m_px, v2);
}

bool String::more(CArrRef v2) const {
  return HPHP::more(m_px, v2);
}

bool String::more(CObjRef v2) const {
  return HPHP::more(m_px, v2);
}

bool String::more(CResRef v2) const {
  return HPHP::more(m_px, v2);
}

///////////////////////////////////////////////////////////////////////////////
// comparison operators

bool String::operator==(const String& v) const {
  return HPHP::equal(m_px, v);
}

bool String::operator!=(const String& v) const {
  return !HPHP::equal(m_px, v);
}

bool String::operator>(const String& v) const {
  return HPHP::more(m_px, v);
}

bool String::operator<(const String& v) const {
  return HPHP::less(m_px, v);
}

bool String::operator==(CVarRef v) const {
  return HPHP::equal(m_px, v);
}

bool String::operator!=(CVarRef v) const {
  return !HPHP::equal(m_px, v);
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
  StringData *px = StringData::Make(int(size));
  auto const buf = px->bufferSlice();
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
  m_px = makeStaticString(s);
}

StaticString::StaticString(litstr s, int length) {
  m_px = makeStaticString(s, length);
}

StaticString::StaticString(std::string s) {
  m_px = makeStaticString(s.c_str(), s.size());
}

StaticString::StaticString(const StaticString &str) {
  assert(str.m_px->isStatic());
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

const StaticString
  s_NULL("NULL"),
  s_boolean("boolean"),
  s_integer("integer"),
  s_double("double"),
  s_string("string"),
  s_array("array"),
  s_object("object"),
  s_resource("resource"),
  s_indirect("indirect"),
  s_ref("reference");

String getDataTypeString(DataType t) {
  switch (t) {
    case KindOfUninit:
    case KindOfNull:     return s_NULL;
    case KindOfBoolean:  return s_boolean;
    case KindOfInt64:    return s_integer;
    case KindOfDouble:   return s_double;
    case KindOfStaticString:
    case KindOfString:   return s_string;
    case KindOfArray:    return s_array;
    case KindOfObject:   return s_object;
    case KindOfResource: return s_resource;
    case KindOfRef:      return s_ref;
    case KindOfIndirect: return s_indirect;

    default:
      assert(false);
      break;
  }
  return empty_string;
}

//////////////////////////////////////////////////////////////////////////////
}

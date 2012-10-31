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

#include <runtime/base/complex_types.h>
#include <runtime/base/string_offset.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/comparisons.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/zend/zend_functions.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/zend/zend_printf.h>
#include <runtime/base/taint/taint_observer.h>

namespace HPHP {

extern bool has_eval_support;

const String null_string = String();
const StaticString empty_string("");
StringData* empty_string_data(empty_string.get());

///////////////////////////////////////////////////////////////////////////////
// statics

#define NUM_CONVERTED_INTEGERS \
  (String::MaxPrecomputedInteger - String::MinPrecomputedInteger + 1)

StringData *String::converted_integers_raw;
StringData *String::converted_integers;

String::IntegerStringDataMap String::integer_string_data_map;

const StringData *convert_integer_helper(int64 n, StringData *sd) {
  char tmpbuf[21];
  char *p;
  int is_negative;
  int len;

  tmpbuf[20] = '\0';
  p = conv_10(n, &is_negative, &tmpbuf[20], &len);
  if (sd) {
    new (sd) StringData(p, len, CopyMalloc);
  } else {
    sd = new StringData(p, len, CopyMalloc);
  }
  sd->setStatic();
  if (!String(sd).checkStatic()) {
    StaticString::TheStaticStringSet().insert(sd);
  }
  return sd;
}

void String::PreConvertInteger(int64 n) {
  IntegerStringDataMap::const_iterator it =
    integer_string_data_map.find(n);
  if (it != integer_string_data_map.end()) return;
  integer_string_data_map[n] = convert_integer_helper(n, NULL);
}

static int precompute_integers() ATTRIBUTE_COLD;
static int precompute_integers() {
  String::converted_integers_raw =
    (StringData *)malloc(NUM_CONVERTED_INTEGERS * sizeof(StringData));
  String::converted_integers = String::converted_integers_raw - SCHAR_MIN;
  for (int n = SCHAR_MIN; n < 65536; n++) {
    StringData *sd = String::converted_integers + n;
    convert_integer_helper(n, sd);
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

  TAINT_OBSERVER(TAINT_BIT_MUTATED, TAINT_BIT_NONE);

  tmpbuf[11] = '\0';
  p = conv_10(n, &is_negative, &tmpbuf[11], &len);
  return NEW(StringData)(p, len, CopyString);
}

String::String(int n) {
  const StringData *sd = GetIntegerStringData(n);
  if (sd) {
    ASSERT(sd->isStatic());
    m_px = (StringData *)sd;
    return;
  }
  m_px = buildStringData(n);
  m_px->setRefCount(1);
}

StringData* buildStringData(int64 n) {
  char tmpbuf[21];
  char* p;
  int is_negative;
  int len;

  TAINT_OBSERVER(TAINT_BIT_MUTATED, TAINT_BIT_NONE);

  tmpbuf[20] = '\0';
  p = conv_10(n, &is_negative, &tmpbuf[20], &len);
  return NEW(StringData)(p, len, CopyString);
}

HOT_FUNC
String::String(int64 n) {
  const StringData *sd = GetIntegerStringData(n);
  if (sd) {
    ASSERT(sd->isStatic());
    m_px = (StringData *)sd;
    return;
  }
  m_px = buildStringData(n);
  m_px->setRefCount(1);
}

StringData* buildStringData(double n) {
  char *buf;

  TAINT_OBSERVER(TAINT_BIT_MUTATED, TAINT_BIT_NONE);

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
  // Ignore taint in comparison functions.
  return string_find(m_px->dataIgnoreTaint(), m_px->size(), ch, pos,
                     caseSensitive);
}

int String::find(const char *s, int pos /* = 0 */,
                 bool caseSensitive /* = true */) const {
  ASSERT(s);
  if (empty()) return -1;
  if (*s && *(s+1) == 0) {
    return find(*s, pos, caseSensitive);
  }
  // Ignore taint in comparison functions.
  return string_find(m_px->dataIgnoreTaint(), m_px->size(), s, strlen(s),
                     pos, caseSensitive);
}

HOT_FUNC
int String::find(CStrRef s, int pos /* = 0 */,
                 bool caseSensitive /* = true */) const {
  if (empty()) return -1;
  if (s.size() == 1) {
    return find(*s.dataIgnoreTaint(), pos, caseSensitive);
  }
  // Ignore taint in comparison functions.
  return string_find(m_px->dataIgnoreTaint(), m_px->size(),
                     s.dataIgnoreTaint(), s.size(), pos, caseSensitive);
}

int String::rfind(char ch, int pos /* = 0 */,
                  bool caseSensitive /* = true */) const {
  if (empty()) return -1;
  // Ignore taint in comparison functions.
  return string_rfind(m_px->dataIgnoreTaint(), m_px->size(), ch,
                      pos, caseSensitive);
}

int String::rfind(const char *s, int pos /* = 0 */,
                  bool caseSensitive /* = true */) const {
  ASSERT(s);
  if (empty()) return -1;
  if (*s && *(s+1) == 0) {
    return rfind(*s, pos, caseSensitive);
  }
  // Ignore taint in comparison functions.
  return string_rfind(m_px->dataIgnoreTaint(), m_px->size(), s, strlen(s),
                      pos, caseSensitive);
}

int String::rfind(CStrRef s, int pos /* = 0 */,
                  bool caseSensitive /* = true */) const {
  if (empty()) return -1;
  if (s.size() == 1) {
    return rfind(*s.dataIgnoreTaint(), pos, caseSensitive);
  }
  // Ignore taint in comparison functions.
  return string_rfind(m_px->dataIgnoreTaint(), m_px->size(),
                      s.dataIgnoreTaint(), s.size(), pos, caseSensitive);
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
  ASSERT(pos >= 0 && pos <= size());
  const char *s = data();
  return s[pos];
}

///////////////////////////////////////////////////////////////////////////////
// assignments

String &String::operator=(litstr s) {
  if (m_px && m_px->decRefCount() == 0) {
    m_px->release();
  }
  if (s) {
    m_px = NEW(StringData)(s, AttachLiteral);
    m_px->setRefCount(1);
  } else {
    m_px = NULL;
  }
  return *this;
}

String &String::operator=(StringData *data) {
  StringBase::operator=(data);
  return *this;
}

String &String::operator=(const std::string & s) {
  if (m_px && m_px->decRefCount() == 0) {
    m_px->release();
  }
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
      if (m_px->decRefCount() == 0) m_px->release();
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
      if (m_px->decRefCount() == 0) m_px->release();
      px->setRefCount(1);
      m_px = px;
    }
  }
  return *this;
}

String String::operator+(litstr str) const {
  if (empty()) return str;
  if (!str || !*str) return *this;
  return NEW(StringData)(slice(), str);
}

HOT_FUNC
String String::operator+(CStrRef str) const {
  if (empty()) return str;
  if (str.empty()) return *this;
  return NEW(StringData)(slice(), str.slice());
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
  char *copy = NULL;
  if (len2 > len1) {
    len = len2;
    copy = string_duplicate(s2, len2);
    for (int i = 0; i < len1; i++) copy[i] |= s1[i];
  } else {
    len = len1;
    copy = string_duplicate(s1, len1);
    for (int i = 0; i < len2; i++) copy[i] |= s2[i];
  }
  if (m_px && m_px->decRefCount() == 0) {
    m_px->release();
  }
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
  char *copy = NULL;
  if (len2 < len1) {
    len = len2;
    copy = string_duplicate(s2, len2);
    for (int i = 0; i < len2; i++) copy[i] &= s1[i];
  } else {
    len = len1;
    copy = string_duplicate(s1, len1);
    for (int i = 0; i < len1; i++) copy[i] &= s2[i];
  }
  if (m_px && m_px->decRefCount() == 0) {
    m_px->release();
  }
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
  char *copy = NULL;
  if (len2 < len1) {
    len = len2;
    copy = string_duplicate(s2, len2);
    for (int i = 0; i < len2; i++) copy[i] ^= s1[i];
  } else {
    len = len1;
    copy = string_duplicate(s1, len1);
    for (int i = 0; i < len1; i++) copy[i] ^= s2[i];
  }
  if (m_px && m_px->decRefCount() == 0) {
    m_px->release();
  }
  m_px = NEW(StringData)(copy, len, AttachString);
  m_px->setRefCount(1);
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// conversions

HOT_FUNC
VarNR String::toKey() const {
  if (!m_px) return VarNR(empty_string);
  int64 n = 0;
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
  return not_less(m_px, v);
}

bool String::operator<=(litstr v) const {
  return not_more(m_px, v);
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
  return not_less(m_px, v);
}

bool String::operator<=(CStrRef v) const {
  return not_more(m_px, v);
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
  return not_less(m_px, v);
}

bool String::operator<=(CVarRef v) const {
  return not_more(m_px, v);
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
  int64 size = uns->readInt();
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
  ASSERT(size <= buf.len);
  uns->read(buf.ptr, size);
  px->setSize(size);
  if (m_px && m_px->decRefCount() == 0) {
    m_px->release();
  }
  m_px = px;
  px->setRefCount(1);

  ch = uns->readChar();
  if (ch != delimiter1) {
    throw Exception("Expected '%c' but got '%c'", delimiter1, ch);
  }

  checkStatic();
}

bool String::checkStatic() {
  ASSERT(m_px);
  StringDataSet &set = StaticString::TheStaticStringSet();
  if (!set.empty()) {
    // no need to upgrade when the initialization is done.
    StringDataSet::iterator it = set.find(m_px);
    if (it != set.end()) {
      StringBase::operator=(*it);
      return true;
    }
  }
  return false;
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

StaticString::StaticString(litstr s) : m_data(s) {
  m_px = &m_data;
  if (has_eval_support) {
    m_px = StringData::GetStaticString(m_px);
    return;
  }
  m_px->setStatic();
  if (!checkStatic()) {
    s_stringSet->insert(m_px);
  }
}

StaticString::StaticString(litstr s, int length)
  : m_data(s, length, AttachLiteral) {
  m_px = &m_data;
  if (has_eval_support) {
    m_px = StringData::GetStaticString(m_px);
    return;
  }
  m_px->setStatic();
  if (!checkStatic()) {
    s_stringSet->insert(m_px);
  }
}

StaticString::StaticString(std::string s)
  : m_data(s.c_str(), s.size(), CopyString) {
  String::operator=(&m_data);
  if (has_eval_support) {
    m_px = StringData::GetStaticString(m_px);
    return;
  }
  m_px->setStatic();
  if (!checkStatic()) {
    s_stringSet->insert(m_px);
  }
}

StaticString::StaticString(const StaticString &str)
  : m_data(str.m_data.data(), str.m_data.size(), AttachLiteral) {
  String::operator=(&m_data);
  if (has_eval_support) {
    m_px = StringData::GetStaticString(m_px);
    return;
  }
  m_px->setStatic();
  if (!checkStatic()) {
    s_stringSet->insert(m_px);
  }
}

StaticString& StaticString::operator=(const StaticString &str) {
  // Assignment to a StaticString is ignored. Generated code
  // should never use a StaticString on the left-hand side of
  // assignment. A StaticString can only be initialized by a
  // StaticString constructor or StaticString::init().
  ASSERT(false);
  return *this;
}

void StaticString::init(litstr s, int length) {
  new(&m_data) StringData(s, length, AttachLiteral);
  ASSERT(!m_px);
  String::operator=(&m_data);
  m_px->setStatic();
  if (has_eval_support) {
    m_px = StringData::GetStaticString(m_px);
    return;
  }
  if (!checkStatic()) {
    s_stringSet->insert(m_px);
  }
}

StringDataSet &StaticString::TheStaticStringSet() {
  if (s_stringSet == NULL) {
    s_stringSet = new StringDataSet();
  }
  return *s_stringSet;
}

void StaticString::FinishInit() {
  if (has_eval_support) {
    ASSERT(s_stringSet->size() == NUM_CONVERTED_INTEGERS);
  }
  // release the memory
  StringDataSet empty;
  s_stringSet->swap(empty);
}

void StaticString::ResetAll() {
  delete s_stringSet;
}

StringDataSet *StaticString::s_stringSet = NULL;

class StaticStringUninitializer {
public:
  ~StaticStringUninitializer() {
    StaticString::ResetAll();
  }
};
static StaticStringUninitializer s_static_string_uninitializer;

//////////////////////////////////////////////////////////////////////////////
}

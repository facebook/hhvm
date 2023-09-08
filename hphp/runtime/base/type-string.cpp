/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/util/conv-10.h"

#include <folly/tracing/StaticTracepoint.h>

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

static const StringData* convert_integer_helper(int64_t n) {
  char tmpbuf[21];
  tmpbuf[20] = '\0';
  auto sl = conv_10(n, &tmpbuf[20]);
  return makeStaticString(sl);
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
  return NUM_CONVERTED_INTEGERS;
}

static int ATTRIBUTE_UNUSED initIntegers = precompute_integers();
static InitFiniNode prepopulate_integers([] {
  // Proactively populate, in order to increase cache locality for sequential
  // access patterns.
  for (int n = String::MinPrecomputedInteger;
       n <= String::MaxPrecomputedInteger; n++) {
    String::ConvertInteger(n);
  }
}, InitFiniNode::When::PostRuntimeOptions);

///////////////////////////////////////////////////////////////////////////////
// constructors

String::~String() {}

StringData* buildStringData(int n) {
  return buildStringData(static_cast<int64_t>(n));
}

StringData* buildStringData(int64_t n) {
  if (auto const sd = String::GetIntegerStringData(n)) {
    assertx(sd->isStatic());
    return const_cast<StringData*>(sd);
  }

  char tmpbuf[21];
  tmpbuf[20] = '\0';
  auto const sl = conv_10(n, &tmpbuf[20]);
  return StringData::Make(sl, CopyString);
}

String::String(int n) : String(static_cast<int64_t>(n)) {}
String::String(int64_t n) : m_str(buildStringData(n), NoIncRef{}) {}

void formatPhpDblStr(char **pbuf, double n) {
  if (n == 0.0) {
    n = 0.0; // so to avoid "-0" output
  }
  vspprintf(pbuf, 0, "%.*G", 14, n);
}

StringData* buildStringData(double n) {
  char *buf = nullptr;
  formatPhpDblStr(&buf, n);
  return StringData::Make(buf, AttachString);
}

String::String(double n) : m_str(buildStringData(n), NoIncRef{}) { }

///////////////////////////////////////////////////////////////////////////////
// informational

int String::find(char ch, int pos /* = 0 */,
                 bool caseSensitive /* = true */) const {
  if (empty()) return -1;
  return string_find(m_str->data(), m_str->size(), ch, pos,
                     caseSensitive);
}

int String::find(const char *s, int pos /* = 0 */,
                 bool caseSensitive /* = true */) const {
  assertx(s);
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
  assertx(s);
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

char String::charAt(int pos) const {
  assertx(pos >= 0 && pos <= size());
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

///////////////////////////////////////////////////////////////////////////////
// concatenation and increments

String& String::operator+=(const char* s) {
  if (!s) return *this;
  return operator+=(folly::StringPiece{s, strlen(s)});
}

String& String::operator+=(const std::string& str) {
  return operator+=(folly::StringPiece{str});
}

String& String::operator+=(const String& str) {
  if (str.empty()) return *this;
  if (empty()) {
    // lhs is empty, just return str. No attempt to append in place even
    // if lhs is private & reserved.
    m_str = str.m_str;
    return *this;
  }
  return operator+=(str.slice());
}

String& String::operator+=(folly::StringPiece slice) {
  if (slice.empty()) {
    return *this;
  }
  if (!m_str) {
    m_str = req::ptr<StringData>::attach(
      StringData::Make(slice.begin(), slice.size(), CopyString));
    return *this;
  }
  if (!m_str->cowCheck()) {
    UNUSED auto const lsize = m_str->size();
    FOLLY_SDT(hhvm, hhvm_mut_concat, lsize, slice.size());
    auto const tmp = m_str->append(slice);
    if (UNLIKELY(tmp != m_str)) {
      // had to realloc even though count==1
      m_str = req::ptr<StringData>::attach(tmp);
    }
    return *this;
  }
  FOLLY_SDT(hhvm, hhvm_cow_concat, m_str->size(), slice.size());
  m_str = req::ptr<StringData>::attach(
    StringData::Make(m_str.get(), slice)
  );
  return *this;
}

String& String::operator+=(folly::MutableStringPiece slice) {
  return operator+=(folly::StringPiece{slice.begin(), slice.size()});
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

String operator+(const String & lhs, const String & rhs) {
  if (lhs.empty()) return rhs;
  if (rhs.empty()) return lhs;
  return String::attach(StringData::Make(lhs.slice(), rhs.slice()));
}

///////////////////////////////////////////////////////////////////////////////
// comparisons

bool String::same(const StringData *v2) const {
  return HPHP::same(get(), v2);
}

bool String::same(const String& v2) const {
  return HPHP::same(get(), v2.get());
}

bool String::equal(const StringData *v2) const {
  return HPHP::equal(get(), v2);
}

bool String::equal(const String& v2) const {
  return HPHP::equal(get(), v2.get());
}

bool String::less(const StringData *v2) const {
  return HPHP::less(get(), v2);
}

bool String::less(const String& v2) const {
  return HPHP::less(get(), v2.get());
}

bool String::more(const StringData *v2) const {
  return HPHP::more(get(), v2);
}

bool String::more(const String& v2) const {
  return HPHP::more(get(), v2.get());
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
  return HPHP::equal(get(), v.get());
}

bool String::operator!=(const String& v) const {
  return !HPHP::equal(get(), v.get());
}

bool String::operator>(const String& v) const {
  return HPHP::more(get(), v.get());
}

bool String::operator<(const String& v) const {
  return HPHP::less(get(), v.get());
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

bool StaticString::s_globalInit{false};
std::vector<std::tuple<const char*, size_t, StaticString*>>*
  StaticString::s_registered;

void StaticString::construct(const char* s, size_t len) {
  if (s_globalInit) {
    init(s, len);
  } else {
    if (!s_registered) {
      s_registered =
        new std::vector<std::tuple<const char*, size_t, StaticString*>>;
    }
    s_registered->emplace_back(s, len, this);
  }
}

void StaticString::init(const char* s, size_t len) {
  reset(makeStaticString(s, len));
}

uint32_t StaticString::CreateAll() {
  if (s_globalInit) return 0;
  s_globalInit = true;
  create_string_data_map();
  for (auto& p : *s_registered) {
    std::get<2>(p)->init(std::get<0>(p), std::get<1>(p));
  }
  auto const ret = s_registered->size();
  s_registered->clear();
  delete s_registered;
  return ret;
}

const StaticString
  s_null("null"),
  s_boolean("boolean"),
  s_integer("integer"),
  s_double("double"),
  s_string("string"),
  s_varray("varray"),
  s_darray("darray"),
  s_vec("vec"),
  s_dict("dict"),
  s_keyset("keyset"),
  s_object("object"),
  s_resource("resource"),
  s_rfunc("reified function"),
  s_func("function"),
  s_class("class"),
  s_clsmeth("clsmeth"),
  s_rclsmeth("rclsmeth"),
  s_enumclasslabel("enumclasslabel");

StaticString getDataTypeString(DataType t, bool isLegacy) {
  switch (t) {
    case KindOfUninit:
    case KindOfNull:       return s_null;
    case KindOfBoolean:    return s_boolean;
    case KindOfInt64:      return s_integer;
    case KindOfDouble:     return s_double;
    case KindOfPersistentString:
    case KindOfString:     return s_string;
    case KindOfPersistentVec:
    case KindOfVec:        return isLegacy ? s_varray : s_vec;
    case KindOfPersistentDict:
    case KindOfDict:       return isLegacy ? s_darray : s_dict;
    case KindOfPersistentKeyset:
    case KindOfKeyset:     return s_keyset;
    case KindOfObject:     return s_object;
    case KindOfResource:   return s_resource;
    case KindOfRFunc:      return s_rfunc;
    case KindOfFunc:       return s_func;
    case KindOfClass:      return s_class;
    case KindOfClsMeth:    return s_clsmeth;
    case KindOfRClsMeth:   return s_rclsmeth;
    case KindOfLazyClass:  return s_class;
    case KindOfEnumClassLabel: return s_enumclasslabel;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////////////
}

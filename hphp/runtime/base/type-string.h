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

#pragma once

#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/util/assertions.h"

#include <algorithm>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct VarNR;
struct VariableSerializer;
struct VariableUnserializer;

// reserve space for buffer that will be filled in by client.
enum ReserveStringMode { ReserveString };

//////////////////////////////////////////////////////////////////////

namespace {

/*
 * Don't actually perform a shrink unless the savings meets this
 * threshold.
 */
constexpr int kMinShrinkThreshold = 1024;

}

//////////////////////////////////////////////////////////////////////

// Built strings will be uncounted, or will have their reference counts
// pre-initialized to 1.
StringData* buildStringData(int     n);
StringData* buildStringData(int64_t n);
StringData* buildStringData(double  n);

//////////////////////////////////////////////////////////////////////

/**
 * String type wrapping around StringData to implement copy-on-write and
 * literal string handling (to avoid string copying).
 */
struct String {
protected:
  req::ptr<StringData> m_str;

  using NoIncRef = req::ptr<StringData>::NoIncRef;
  String(StringData* sd, NoIncRef) : m_str(sd, NoIncRef{}) {}

public:
  static const int MinPrecomputedInteger = SCHAR_MIN;
  static const int MaxPrecomputedInteger = 4095 + SCHAR_MIN;
  static StringData const **converted_integers_raw;
  static StringData const **converted_integers;

  static bool HasConverted(int64_t n) {
    return MinPrecomputedInteger <= n && n <= MaxPrecomputedInteger;
  }
  static bool HasConverted(int n) {
    return HasConverted((int64_t)n);
  }

  // create a string from a character
  static String FromChar(char ch) {
    return String{makeStaticString(ch)};
  }

  static const StringData *ConvertInteger(int64_t n);
  static const StringData *GetIntegerStringData(int64_t n) {
    if (HasConverted(n)) {
      return *(converted_integers + n);
    }
    return nullptr;
  }

public:
  String() {}
  ~String();

  StringData* get() const { return m_str.get(); }
  void reset(StringData* str = nullptr) { m_str.reset(str); }

  // Transfer ownership of our reference to this StringData.
  StringData* detach() { return m_str.detach(); }

  /**
   * Constructors
   */
  explicit String(StringData *data) : m_str(data) { }
  /* implicit */ String(int     n);
  /* implicit */ String(int64_t n);
  /* implicit */ String(double  n);
  /* implicit */ String(const char* s)
  : m_str(LIKELY((bool)s) ? StringData::Make(s, CopyString)
                           : nullptr, NoIncRef{}) { }

  String(const String& str) : m_str(str.m_str) { }
  /* implicit */ String(const StaticString& str);

  /* Prevent unintentional promotion. */
  /* implicit */ String(char) = delete;
  /* implicit */ String(Variant&&) = delete;

  // Disable this---otherwise this would generally implicitly create a
  // Variant(bool) and then call String(Variant&&) ...
  /* implicit */ String(const StringData*) = delete;

  // Move ctor
  /* implicit */ String(String&& str) noexcept : m_str(std::move(str.m_str)) {}

  // Move assign
  String& operator=(String&& src) {
    m_str = std::move(src.m_str);
    return *this;
  }
  String& operator=(req::ptr<StringData>&& src) {
    m_str = std::move(src);
    return *this;
  }

  String& operator=(const Variant&) = delete;
  String& operator=(Variant&&) = delete;

  /* implicit */ String(const std::string &s)
  : m_str(StringData::Make(s.data(), s.size(), CopyString), NoIncRef{}) { }

  /* implicit */ String(folly::StringPiece s)
  : m_str(StringData::Make(s), NoIncRef{}) {}

  // attach to null terminated malloc'ed string, maybe free it now.
  String(char* s, AttachStringMode mode)
  : m_str(LIKELY((bool)s) ? StringData::Make(s, mode) : nullptr, NoIncRef{}) {}

  // copy a null terminated string
  String(const char *s, CopyStringMode mode)
  : m_str(LIKELY((bool)s) ? StringData::Make(s, mode) : nullptr, NoIncRef{}) {}

  // attach to binary malloc'ed string
  String(char* s, size_t length, AttachStringMode mode)
  : m_str(LIKELY((bool)s) ? StringData::Make(s, length, mode)
                          : nullptr, NoIncRef{}) { }

  // make copy of binary binary string
  String(const char *s, size_t length, CopyStringMode mode)
  : m_str(LIKELY((bool)s) ? StringData::Make(s, length, mode)
                          : nullptr, NoIncRef{}) { }

  // force a copy of a String
  String(const String& s, CopyStringMode mode)
  : m_str(LIKELY((bool)s) ? StringData::Make(s.c_str(), s.size(), mode)
                          : nullptr, NoIncRef{}) {}

  // make an empty string with cap reserve bytes, plus 1 for '\0'
  String(size_t cap, ReserveStringMode /*mode*/)
      : m_str(StringData::Make(cap), NoIncRef{}) {}

  static String attach(StringData* sd) {
    return String(sd, NoIncRef{});
  }

  void clear() { reset();}
  /**
   * Informational
   */
  const char *data() const {
    return m_str ? m_str->data() : "";
  }

  char *mutableData() const {
    return m_str->mutableData();
  }

public:
  const String& setSize(int64_t len) {
    assertx(m_str);
    m_str->setSize(len);
    return *this;
  }
  const String& shrink(size_t len) {
    assertx(m_str && !m_str->hasMultipleRefs());
    if (m_str->capacity() - len > kMinShrinkThreshold) {
      m_str = req::ptr<StringData>::attach(m_str->shrinkImpl(len));
    } else {
      assertx(len < StringData::MaxSize);
      m_str->setSize(len);
    }
    return *this;
  }
  folly::MutableStringPiece reserve(size_t size) {
    if (!m_str) return folly::MutableStringPiece();
    auto const tmp = m_str->reserve(size);
    if (UNLIKELY(tmp != m_str)) {
      m_str = req::ptr<StringData>::attach(tmp);
    }
    return m_str->bufferSlice();
  }
  const char *c_str() const {
    return m_str ? m_str->data() : "";
  }
  bool empty() const {
    return m_str ? m_str->empty() : true;
  }
  int64_t size() const {
    return m_str ? m_str->size() : 0;
  }
  int64_t length() const {
    return m_str ? m_str->size() : 0;
  }
  uint32_t capacity() const {
    return m_str->capacity(); // intentionally skip nullptr check
  }
  folly::StringPiece slice() const {
    return m_str ? m_str->slice() : folly::StringPiece();
  }
  folly::MutableStringPiece bufferSlice() {
    return m_str ? m_str->bufferSlice() : folly::MutableStringPiece();
  }
  bool isNull() const { return !m_str; }
  bool isNumeric() const {
    return m_str ? m_str->isNumeric() : false;
  }
  bool isZero() const {
    return m_str ? m_str->isZero() : false;
  }

  String substr(int start, int length = StringData::MaxSize) const {
    return String::attach(
      m_str ? m_str->substr(start, length) : staticEmptyString());
  }

  /**
   * Find a character or a substring and return its position. "pos" has to be
   * within current string and it's the start point for searching.
   */
  static const int npos = -1;
  int find(char ch, int pos = 0, bool caseSensitive = true) const;
  int find(const char *s, int pos = 0, bool caseSensitive = true) const;
  int find(const String& s, int pos = 0, bool caseSensitive = true) const;
  int rfind(char ch, int pos = 0, bool caseSensitive = true) const;
  int rfind(const char *s, int pos = 0, bool caseSensitive = true) const;
  int rfind(const String& s, int pos = 0, bool caseSensitive = true) const;

  /**
   * Operators
   */
  String& operator=(StringData *data) {
    m_str = data;
    return *this;
  }
  String& operator=(const String& v) {
    m_str = v.m_str;
    return *this;
  }
  String& operator=(const StaticString& v);
  String& operator=(const char* v);
  String& operator=(const std::string &s);
  // These should be members, but g++ doesn't yet support the rvalue
  // reference notation on lhs (http://goo.gl/LuCTo).
  friend String&& operator+(String&& lhs, const char* rhs);
  friend String&& operator+(String&& lhs, String&& rhs);
  friend String operator+(String&& lhs, const String & rhs);
  friend String operator+(const String& lhs, const char* rhs);
  friend String operator+(const String & lhs, const String & rhs);
  String& operator += (const char* v);
  String& operator += (const String& v);
  String& operator += (const std::string& v);
  String& operator += (folly::StringPiece slice);
  String& operator += (folly::MutableStringPiece slice);
  String  operator |  (const String& v) const = delete;
  String  operator &  (const String& v) const = delete;
  String  operator ^  (const String& v) const = delete;
  String& operator |= (const String& v) = delete;
  String& operator &= (const String& v) = delete;
  String& operator ^= (const String& v) = delete;
  String  operator ~  () const = delete;
  explicit operator std::string () const { return toCppString(); }
  explicit operator bool() const { return (bool)m_str; }

  /**
   * These are convenient functions for writing extensions, since code
   * generation always uses explicit functions like same(), less() etc. that
   * are type specialized and unambiguous.
   */
  bool operator == (const char* v) const = delete;
  bool operator != (const char* v) const = delete;
  bool operator >= (const char* v) const = delete;
  bool operator <= (const char* v) const = delete;
  bool operator >  (const char* v) const = delete;
  bool operator <  (const char* v) const = delete;

  bool operator == (const String& v) const;
  bool operator != (const String& v) const;
  bool operator >= (const String& v) const = delete;
  bool operator <= (const String& v) const = delete;
  bool operator >  (const String& v) const;
  bool operator <  (const String& v) const;

  bool operator == (const Variant& v) const = delete;
  bool operator != (const Variant& v) const = delete;
  bool operator >= (const Variant& v) const = delete;
  bool operator <= (const Variant& v) const = delete;
  bool operator >  (const Variant& v) const = delete;
  bool operator <  (const Variant& v) const = delete;

  /**
   * Type conversions
   */
  bool   toBoolean() const { return m_str ? m_str->toBoolean() : false;}
  int64_t toInt64 () const { return m_str ? m_str->toInt64  () : 0;}
  double toDouble () const { return m_str ? m_str->toDouble () : 0;}
  std::string toCppString() const { return std::string(c_str(), size()); }
  TypedValue asTypedValue() const {
    assertx(m_str);
    return make_tv<KindOfString>(m_str.get());
  }

  /**
   * Comparisons
   */
  bool same (const char* v2) const = delete;
  bool same (const StringData *v2) const;
  bool same (const String& v2) const;
  bool same (const Array& v2) const = delete;
  bool same (const Object& v2) const = delete;
  bool same (const OptResource& v2) const = delete;

  bool equal(const char* v2) const = delete;
  bool equal(const StringData *v2) const;
  bool equal(const String& v2) const;
  bool equal(const Array& v2) const = delete;
  bool equal(const Object& v2) const = delete;
  bool equal(const OptResource& v2) const = delete;

  bool less (const char* v2) const = delete;
  bool less (const StringData *v2) const;
  bool less (const String& v2) const;
  bool less (const Array& v2) const = delete;
  bool less (const Object& v2) const = delete;
  bool less (const OptResource& v2) const = delete;

  bool more (const char* v2) const = delete;
  bool more (const StringData *v2) const;
  bool more (const String& v2) const;
  bool more (const Array& v2) const = delete;
  bool more (const Object& v2) const = delete;
  bool more (const OptResource& v2) const = delete;

  int compare(const char* v2) const;
  int compare(const String& v2) const;

  /**
   * Returns one character at specified position.
   */
  char charAt(int pos) const;
  char operator[](int pos) const { return charAt(pos);}

  /**
   * Debugging
   */
  void dump() const;

  template <class Op> ALWAYS_INLINE
  String forEachByte(Op action) const {
    String ret = String(size(), ReserveString);

    auto srcSlice = slice();

    const char* src = srcSlice.begin();
    const char* end = srcSlice.end();

    char* dst = ret.mutableData();

    for (; src != end; ++src, ++dst) {
      *dst = action(*src);
    }

    ret.setSize(size());
    return ret;
  }

  template <class Op> ALWAYS_INLINE
  String forEachByteFast(Op action) const {
    if (this->empty()) {
      return *this;
    }

    return forEachByte(action);
  }

 private:
  static void compileTimeAssertions() {
    static_assert(sizeof(String) == sizeof(req::ptr<StringData>), "");
  }
};

extern const String null_string;

///////////////////////////////////////////////////////////////////////////////
// StrNR

struct StrNR {
private:
  StringData *m_px;

public:
  StrNR() : m_px(nullptr) {}
  explicit StrNR(StringData *sd) : m_px(sd) {}
  explicit StrNR(const StringData *sd) : m_px(const_cast<StringData*>(sd)) {}
  explicit StrNR(const String &s) : m_px(s.get()) {} // XXX
  explicit StrNR(const char*) = delete;

  ~StrNR() {
    if (debug) {
      m_px = reinterpret_cast<StringData*>(0xdeadbeeffaceb004);
    }
  }

  /* implicit */ operator const String&() const { return asString(); }
  const char* data() const { return m_px ? m_px->data() : ""; }
  const char* c_str() const { return data(); }
  int size() const { return m_px ? m_px->size() : 0; }
  bool empty() const { return size() == 0; }

  uint32_t capacity() const {
    return m_px->capacity(); // intentionally skip nullptr check
  }

  String& asString() {
    return *reinterpret_cast<String*>(this);
  }
  const String& asString() const {
    return const_cast<StrNR*>(this)->asString();
  }

  StringData* get() const { return m_px; }

private:
  static void compileTimeAssertions() {
    static_assert(offsetof(StrNR, m_px) == kExpectedMPxOffset, "");
  }
};

///////////////////////////////////////////////////////////////////////////////

/**
 * A StaticString can be co-accessed by multiple threads, therefore they are
 * not thread local.
 */
struct StaticString : String {
  TYPE_SCAN_IGNORE_BASES(String);

  template<size_t N> explicit StaticString(const char(&s)[N]) {
    construct(s, N - 1);
  }
  template<size_t N> explicit StaticString(char(&s)[N]) = delete;

  explicit StaticString(std::string_view s) {
    construct(s.data(), s.size());
  }

  ~StaticString() {
    // prevent ~req::ptr from destroying contents.
    detach();
  }
  StaticString(const StaticString& other) {
    m_str = other.get();
  }
  StaticString& operator=(const StaticString& other) = delete;

  StringData* get() const {
    assertx(valid());
    return m_str.get();
  }

  bool valid() const {
    if (s_globalInit) {
      assertx(m_str);
      return true;
    }
    return false;
  }

  // Return number of lazily created StaticStrings, must run before creating
  // VM threads.
  static uint32_t CreateAll();

  // StaticStrings created before s_globalInit is set will be created lazily,
  // upon create(). Those are expected to happen during process initialization
  // before VM threads are created, so no need to worry about thread safety.
  static bool s_globalInit;
 private:
  void construct(const char* s, size_t len);
  void init(const char* s, size_t len);

  static std::vector<std::tuple<const char*, size_t, StaticString*>>*
    s_registered;
};

StaticString getDataTypeString(DataType t, bool isLegacy = false);

//////////////////////////////////////////////////////////////////////

inline String::String(const StaticString& str) :
  m_str(str.m_str.get(), NoIncRef{}) {
  assertx(str.m_str->isStatic());
}

inline String& String::operator=(const StaticString& v) {
  m_str = req::ptr<StringData>::attach(v.m_str.get());
  return *this;
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE String empty_string() {
  return String::attach(staticEmptyString());
}

ALWAYS_INLINE TypedValue empty_string_tv() {
  return make_tv<KindOfPersistentString>(staticEmptyString());
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE String& asStrRef(tv_lval tv) {
  assertx(tvIsPlausible(*tv));
  assertx(isStringType(type(tv)));
  type(tv) = KindOfString;
  return reinterpret_cast<String&>(val(tv).pstr);
}

ALWAYS_INLINE const String& asCStrRef(tv_rval tv) {
  assertx(tvIsPlausible(*tv));
  assertx(isStringType(type(tv)));
  return reinterpret_cast<const String&>(val(tv).pstr);
}

}

namespace folly {
template<> class FormatValue<HPHP::String> {
 public:
  explicit FormatValue(const HPHP::String& str) : m_val(str) {}

  template<typename Callback>
  void format(FormatArg& arg, Callback& cb) const {
    FormatValue<HPHP::StringData*>(m_val.get()).format(arg, cb);
  }

 private:
  const HPHP::String& m_val;
};

template<> class FormatValue<HPHP::StaticString> {
 public:
  explicit FormatValue(const HPHP::StaticString& str) : m_val(str) {}

  template<typename Callback>
  void format(FormatArg& arg, Callback& cb) const {
    FormatValue<HPHP::StringData*>(m_val.get()).format(arg, cb);
  }

 private:
  const HPHP::StaticString& m_val;
};
}

/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <stdarg.h>
#include <new>
#include <ostream>
#include <stdexcept>
#include "watchman/thirdparty/jansson/utf.h"
#include "watchman/watchman_hash.h"
#include "watchman/watchman_string.h"

// Filename mapping and handling strategy
// We'll track the utf-8 rendition of the underlying filesystem names
// in the watchman datastructures.  We'll convert to Wide Char at the
// boundary with the Windows API.  All paths that we observe from the
// Windows API will be converted to UTF-8.
// TODO: we should use wmain to guarantee that we only ever see UTF-8
// for our program arguments and environment

#define LEN_ESCAPE L"\\\\?\\"
#define LEN_ESCAPE_LEN 4
#define UNC_PREFIX L"UNC"
#define UNC_PREFIX_LEN 3
// We see this escape come back from reparse points when reading
// junctions/symlinks
#define SYMLINK_ESCAPE L"\\??\\"
#define SYMLINK_ESCAPE_LEN 4

static void w_string_addref(w_string_t* str);
static void w_string_delref(w_string_t* str);
static w_string_t*
w_string_new_len_typed(const char* str, uint32_t len, w_string_type_t type);

// string piece

w_string_piece::w_string_piece() : s_(nullptr), e_(nullptr) {}
w_string_piece::w_string_piece(std::nullptr_t) : s_(nullptr), e_(nullptr) {}

w_string_piece::w_string_piece(w_string_piece&& other) noexcept
    : s_(other.s_), e_(other.e_) {
  other.s_ = nullptr;
  other.e_ = nullptr;
}

w_string w_string_piece::asWString(w_string_type_t stringType) const {
  return w_string(data(), size(), stringType);
}

w_string w_string_piece::asLowerCase(w_string_type_t stringType) const {
  char* buf;
  w_string_t* s;

  /* need to make a lowercase version */
  s = (w_string_t*)(new char[sizeof(*s) + size() + 1]);
  new (s) w_string_t;

  s->refcnt = 1;
  s->len = size();
  buf = const_cast<char*>(s->buf);
  s->type = stringType;

  auto cursor = s_;
  while (cursor < e_) {
    *buf = (char)tolower((uint8_t)*cursor);
    ++cursor;
    ++buf;
  }
  *buf = 0;

  return w_string(s, false);
}

w_string w_string_piece::asLowerCaseSuffix(w_string_type_t stringType) const {
  char* buf;
  w_string_t* s;

  w_string_piece suffixPiece = this->suffix();
  if (suffixPiece == nullptr) {
    return nullptr;
  }

  /* need to make a lowercase version */
  s = (w_string_t*)(new char[sizeof(*s) + suffixPiece.size() + 1]);
  new (s) w_string_t;

  s->refcnt = 1;
  s->len = suffixPiece.size();
  buf = const_cast<char*>(s->buf);
  s->type = stringType;

  auto cursor = suffixPiece.s_;
  while (cursor < suffixPiece.e_) {
    *buf = (char)tolower((uint8_t)*cursor);
    ++cursor;
    ++buf;
  }
  *buf = 0;

  return w_string(s, false);
}

w_string w_string_piece::asUTF8Clean() const {
  w_string s(s_, e_ - s_, W_STRING_UNICODE);
  utf8_fix_string(const_cast<char*>(s.data()), s.size());
  return s;
}

bool w_string_piece::pathIsAbsolute() const {
  return w_is_path_absolute_cstr_len(data(), size());
}

/** Compares two path strings.
 * They are equal if the case of each character matches.
 * Directory separator slashes are normalized such that
 * \ and / are considered equal. */
bool w_string_piece::pathIsEqual(w_string_piece other) const {
#ifdef _WIN32
  if (size() != other.size()) {
    return false;
  }

  auto A = data();
  auto B = other.data();

  auto end = A + size();
  for (; A < end; ++A, ++B) {
    if (*A == *B) {
      continue;
    }
    if (A == data()) {
      // This is a bit awful, but msys and other similar software
      // can set the cwd to a lowercase drive letter.  Since we
      // can't ever watch at a level higher than drive letters,
      // we really shouldn't care about a case difference there
      // so we relax the strictness of the check here.
      // This case only triggers for the first character of the
      // path.  Paths evaluated with this method are always
      // absolute.  In theory, we should also do something
      // reasonable for UNC paths, but folks shouldn't be
      // watching those with watchman anyway.
      if (tolower(*A) == tolower(*B)) {
        continue;
      }
    }

    if (is_slash(*A) && is_slash(*B)) {
      continue;
    }
    return false;
  }
  return true;
#else
  return *this == other;
#endif
}

w_string_piece w_string_piece::dirName() const {
  if (e_ == s_) {
    return nullptr;
  }
  for (auto end = e_ - 1; end >= s_; --end) {
    if (is_slash(*end)) {
      /* found the end of the parent dir */
#ifdef _WIN32
      if (end > s_ && end[-1] == ':') {
        // Special case for "C:\"; we want to keep the
        // trailing slash for this case so that we continue
        // to consider it an absolute path
        return w_string_piece(s_, 1 + end - s_);
      }
#endif
      return w_string_piece(s_, end - s_);
    }
  }
  return nullptr;
}

w_string_piece w_string_piece::baseName() const {
  if (e_ == s_) {
    return *this;
  }
  for (auto end = e_ - 1; end >= s_; --end) {
    if (is_slash(*end)) {
      /* found the end of the parent dir */
#ifdef _WIN32
      if (end == e_ && end > s_ && end[-1] == ':') {
        // Special case for "C:\"; we want the baseName to
        // be this same component so that we continue
        // to consider it an absolute path
        return *this;
      }
#endif
      return w_string_piece(end + 1, e_ - (end + 1));
    }
  }

  return *this;
}

w_string_piece w_string_piece::suffix() const {
  if (e_ == s_) {
    return nullptr;
  }
  for (auto end = e_ - 1; end >= s_; --end) {
    if (is_slash(*end)) {
      return nullptr;
    }
    if (*end == '.') {
      return w_string_piece(end + 1, e_ - (end + 1));
    }
  }
  return nullptr;
}

bool w_string_piece::operator<(w_string_piece other) const {
  int res;
  if (size() < other.size()) {
    res = memcmp(data(), other.data(), size());
    return (res == 0 ? -1 : res) < 0;
  } else if (size() > other.size()) {
    res = memcmp(data(), other.data(), other.size());
    return (res == 0 ? +1 : res) < 0;
  }
  return memcmp(data(), other.data(), size()) < 0;
}

bool w_string_piece::operator==(w_string_piece other) const {
  if (s_ == other.s_ && e_ == other.e_) {
    return true;
  }
  if (size() != other.size()) {
    return false;
  } else if (size() == 0) {
    return true;
  }
  return memcmp(data(), other.data(), size()) == 0;
}

bool w_string_piece::operator!=(w_string_piece other) const {
  return !operator==(other);
}

bool w_string_piece::startsWith(w_string_piece prefix) const {
  if (prefix.size() > size()) {
    return false;
  }
  return memcmp(data(), prefix.data(), prefix.size()) == 0;
}

bool w_string_piece::startsWithCaseInsensitive(w_string_piece prefix) const {
  if (prefix.size() > size()) {
    return false;
  }

  auto me = s_;
  auto pref = prefix.s_;

  while (pref < prefix.e_) {
    if (tolower((uint8_t)*me) != tolower((uint8_t)*pref)) {
      return false;
    }
    ++pref;
    ++me;
  }
  return true;
}

// string

w_string::w_string(std::nullptr_t) {}

w_string::~w_string() {
  if (str_) {
    w_string_delref(str_);
  }
}

w_string::w_string(w_string_t* str, bool addRef) : str_(str) {
  if (str_ && addRef) {
    w_string_addref(str_);
  }
}

w_string::w_string(const w_string& other) : str_(other.str_) {
  if (str_) {
    w_string_addref(str_);
  }
}

#ifdef _WIN32

w_string::w_string(const WCHAR* wpath, size_t pathlen) {
  int len, res;
  bool is_unc = false;

  if (!wcsncmp(wpath, SYMLINK_ESCAPE, SYMLINK_ESCAPE_LEN)) {
    wpath += SYMLINK_ESCAPE_LEN;
    pathlen -= SYMLINK_ESCAPE_LEN;
  }

  if (!wcsncmp(wpath, LEN_ESCAPE, LEN_ESCAPE_LEN)) {
    wpath += LEN_ESCAPE_LEN;
    pathlen -= LEN_ESCAPE_LEN;

    if (pathlen >= UNC_PREFIX_LEN + 1 &&
        !wcsncmp(wpath, UNC_PREFIX, UNC_PREFIX_LEN) &&
        wpath[UNC_PREFIX_LEN] == '\\') {
      // Need to convert "\\?\UNC\server\share" to "\\server\share"
      // We'll pass "C\server\share" and then poke a "\" in at the
      // start
      wpath += UNC_PREFIX_LEN - 1;
      pathlen -= (UNC_PREFIX_LEN - 1);
      is_unc = true;
    }
  }

  len = WideCharToMultiByte(
      CP_UTF8, 0, wpath, pathlen, nullptr, 0, nullptr, nullptr);
  if (len <= 0) {
    throw std::system_error(
        GetLastError(), std::system_category(), "WideCharToMultiByte");
  }

  str_ = (w_string_t*)(new char[sizeof(w_string_t) + len + 1]);
  new (str_) w_string_t;

  str_->refcnt = 1;
  str_->len = len;
  auto buf = const_cast<char*>(str_->buf);
  str_->type = W_STRING_UNICODE;

  res = WideCharToMultiByte(
      CP_UTF8, 0, wpath, pathlen, buf, len, nullptr, nullptr);
  if (res != len) {
    // Weird!
    throw std::system_error(
        GetLastError(), std::system_category(), "WideCharToMultiByte");
  }

  if (is_unc) {
    // Replace the "C" from UNC with a leading slash
    buf[0] = '\\';
  }

  // Normalize directory separators for our internal UTF-8
  // strings to be forward slashes.  These will get transformed
  // to backslashes when converting to WCHAR in our counterpart
  // w_string_piece::asWideUNC()
  std::transform(
      buf, buf + len, buf, [](wchar_t c) { return c == '\\' ? '/' : c; });

  buf[res] = 0;
}

#endif

w_string& w_string::operator=(const w_string& other) {
  if (&other == this) {
    return *this;
  }

  reset();
  if (str_) {
    w_string_delref(str_);
  }
  str_ = other.str_;
  if (str_) {
    w_string_addref(str_);
  }

  return *this;
}

w_string::w_string(w_string&& other) noexcept : str_(other.str_) {
  other.str_ = nullptr;
}

w_string& w_string::operator=(w_string&& other) noexcept {
  if (&other == this) {
    return *this;
  }
  reset();
  str_ = other.str_;
  other.str_ = nullptr;
  return *this;
}

void w_string::reset() noexcept {
  if (str_) {
    w_string_delref(str_);
    str_ = nullptr;
  }
}

static inline uint32_t checked_len(size_t len) {
  if (len > UINT32_MAX) {
    throw std::range_error("string length exceeds UINT32_MAX");
  }
  return len;
}

w_string::w_string(const char* buf, size_t len, w_string_type_t stringType)
    : w_string(
          w_string_new_len_typed(buf, checked_len(len), stringType),
          false) {}

w_string::w_string(const char* buf, w_string_type_t stringType)
    : w_string(
          w_string_new_len_typed(buf, strlen_uint32(buf), stringType),
          false) {}

w_string w_string::dirName() const {
  return w_string_piece(*this).dirName().asWString();
}

w_string w_string::baseName() const {
  return w_string_piece(*this).baseName().asWString();
}

w_string w_string::asLowerCaseSuffix() const {
  ensureNotNull();
  return w_string_piece(*this).asLowerCaseSuffix();
}

w_string w_string::normalizeSeparators(char targetSeparator) const {
  w_string_t* s;
  char* buf;
  uint32_t i, len;

  len = str_->len;

  if (len == 0) {
    return *this;
  }

  // This doesn't do any special UNC or path len escape prefix handling
  // on windows.  We don't currently use it in a way that would require it.

  // Trim any trailing dir seps
  while (len > 0) {
    if (str_->buf[len - 1] == '/' || str_->buf[len - 1] == '\\') {
      --len;
    } else {
      break;
    }
  }

  s = (w_string_t*)(new char[sizeof(*s) + len + 1]);
  new (s) w_string_t;

  s->refcnt = 1;
  s->len = len;
  buf = const_cast<char*>(s->buf);

  for (i = 0; i < len; i++) {
    if (str_->buf[i] == '/' || str_->buf[i] == '\\') {
      buf[i] = targetSeparator;
    } else {
      buf[i] = str_->buf[i];
    }
  }
  buf[len] = 0;

  return w_string{s};
}

bool w_string::operator<(const w_string& other) const {
  return w_string_compare(str_, other.str_) < 0;
}

bool w_string::operator==(const w_string& other) const {
  return w_string_equal(str_, other.str_);
}

bool w_string::operator!=(const w_string& other) const {
  return !(*this == other);
}

w_string w_string::pathCat(std::initializer_list<w_string_piece> elems) {
  uint32_t length = 0;
  w_string_t* s;
  char* buf;

  for (auto& p : elems) {
    length += p.size() + 1;
  }

  s = (w_string_t*)(new char[sizeof(*s) + length]);
  new (s) w_string_t;

  s->refcnt = 1;
  buf = const_cast<char*>(s->buf);

  for (auto& p : elems) {
    if (p.size() == 0) {
      // Skip empty strings
      continue;
    }
    if (buf != s->buf) {
      *buf = '/';
      ++buf;
    }
    memcpy(buf, p.data(), p.size());
    buf += p.size();
  }
  *buf = 0;
  s->len = buf - s->buf;

  return w_string(s, false);
}

uint32_t w_string_compute_hval(w_string_t* str) {
  str->_hval = w_hash_bytes(str->buf, str->len, 0);
  str->hval_computed = 1;
  return str->_hval;
}

uint32_t w_string_piece::hashValue() const {
  return w_hash_bytes(data(), size(), 0);
}

uint32_t strlen_uint32(const char* str) {
  size_t slen = strlen(str);
  if (slen > UINT32_MAX) {
    throw std::range_error("string length exceeds UINT32_MAX");
  }

  return (uint32_t)slen;
}

#ifdef _WIN32

std::wstring w_string_piece::asWideUNC() const {
  int len, res, i, prefix_len;
  bool use_escape = false;
  bool is_unc = false;

  // Make a copy of ourselves, as we may mutate
  w_string_piece path = *this;

  if (path.size() == 0) {
    return std::wstring(L"");
  }

  // We don't want to use the length escape for special filenames like NUL:
  // (which we use as the equivalent to /dev/null).
  // We only strictly need to use the escape when pathlen >= MAX_PATH,
  // but since such paths are rare, we want to ensure that we hit any
  // problems with the escape approach on common paths (not all windows
  // API functions apparently support this very well)
  if (path.size() > 3 && path.size() < MAX_PATH &&
      path[path.size() - 1] == ':') {
    use_escape = false;
    prefix_len = 0;
  } else {
    use_escape = true;

    prefix_len = LEN_ESCAPE_LEN;

    // More func with escaped names when UNC notation is used
    if (path[0] == '\\' && path[1] == '\\') {
      is_unc = true;
      prefix_len += UNC_PREFIX_LEN;
    }
  }

  // Step 1, measure up
  len = MultiByteToWideChar(CP_UTF8, 0, path.data(), path.size(), nullptr, 0);
  if (len <= 0) {
    throw std::system_error(
        GetLastError(), std::system_category(), "MultiByteToWideChar");
  }

  // Step 2, allocate and prepend UNC prefix
  std::wstring result;
  result.reserve(prefix_len + len + 1);

  if (use_escape) {
    result.append(LEN_ESCAPE);

    // UNC paths need further mangling when using length escapes
    if (is_unc) {
      result.append(UNC_PREFIX);
      // "\\server\path" -> "\\?\UNC\server\path"
      path = w_string_piece(
          path.data() + 1,
          path.size() - 1); // Skip the first of these two slashes
    }
  }

  // Step 3, convert into the new space
  result.resize(prefix_len + len);
  res = MultiByteToWideChar(
      CP_UTF8, 0, path.data(), path.size(), &result[prefix_len], len);

  if (res != len) {
    // Something crazy happened
    throw std::system_error(
        GetLastError(), std::system_category(), "MultiByteToWideChar");
  }

  // Replace all forward slashes with backslashes.  This makes things easier
  // for clients that are just jamming paths together using /, but is also
  // required when we are using the long filename escape prefix
  for (auto& c : result) {
    if (c == '/') {
      c = '\\';
    }
  }
  return result;
}

#endif

static w_string_t*
w_string_new_len_typed(const char* str, uint32_t len, w_string_type_t type) {
  w_string_t* s;
  char* buf;

  s = (w_string_t*)(new char[sizeof(*s) + len + 1]);
  new (s) w_string_t;

  s->refcnt = 1;
  s->len = len;
  buf = const_cast<char*>(s->buf);
  if (str) {
    memcpy(buf, str, len);
  }
  buf[len] = 0;
  s->type = type;

  return s;
}

void w_string_addref(w_string_t* str) {
  ++str->refcnt;
}

void w_string_delref(w_string_t* str) {
  if (--str->refcnt != 0) {
    return;
  }
  // Call the destructor.  We can't use regular delete because
  // we allocated using operator new[], and we can't use delete[]
  // directly either because the type doesn't match what we allocated.
  str->~w_string_t();
  // Release the raw memory.
  delete[](char*) str;
}

int w_string_compare(const w_string_t* a, const w_string_t* b) {
  int res;
  if (a == b)
    return 0;
  if (a->len < b->len) {
    res = memcmp(a->buf, b->buf, a->len);
    return res == 0 ? -1 : res;
  } else if (a->len > b->len) {
    res = memcmp(a->buf, b->buf, b->len);
    return res == 0 ? +1 : res;
  }
  return memcmp(a->buf, b->buf, a->len);
}

bool w_string_equal_cstring(const w_string_t* a, const char* b) {
  uint32_t blen = strlen_uint32(b);
  if (a->len != blen)
    return false;
  return memcmp(a->buf, b, a->len) == 0 ? true : false;
}

bool w_string_equal(const w_string_t* a, const w_string_t* b) {
  if (a == b)
    return true;
  if (a == nullptr || b == nullptr)
    return false;
  if (a->len != b->len)
    return false;
  if (a->hval_computed && b->hval_computed && a->_hval != b->_hval) {
    return false;
  }
  return memcmp(a->buf, b->buf, a->len) == 0 ? true : false;
}

bool w_string_equal_caseless(w_string_piece a, w_string_piece b) {
  uint32_t i;

  if (a.size() != b.size()) {
    return false;
  }
  for (i = 0; i < a.size(); i++) {
    if (tolower((uint8_t)a[i]) != tolower((uint8_t)b[i])) {
      return false;
    }
  }
  return true;
}

bool w_string_piece::hasSuffix(w_string_piece suffix) const {
  unsigned int base, i;

  if (size() < suffix.size() + 1) {
    return false;
  }

  base = size() - suffix.size();

  if (s_[base - 1] != '.') {
    return false;
  }

  for (i = 0; i < suffix.size(); i++) {
    if (tolower((uint8_t)s_[base + i]) != suffix[i]) {
      return false;
    }
  }

  return true;
}

bool w_string_startswith(w_string_t* str, w_string_t* prefix) {
  if (prefix->len > str->len) {
    return false;
  }
  return memcmp(str->buf, prefix->buf, prefix->len) == 0;
}

bool w_string_startswith_caseless(w_string_t* str, w_string_t* prefix) {
  size_t i;

  if (prefix->len > str->len) {
    return false;
  }
  for (i = 0; i < prefix->len; i++) {
    if (tolower((uint8_t)str->buf[i]) != tolower((uint8_t)prefix->buf[i])) {
      return false;
    }
  }
  return true;
}

bool w_string_piece::contains(w_string_piece needle) const {
#if HAVE_MEMMEM
  if (needle.empty()) {
    return true;
  }
  return memmem(data(), size(), needle.data(), needle.size()) != nullptr;
#else
  return view().find(needle.view()) != std::string_view::npos;
#endif
}

w_string_piece w_string_canon_path(w_string_t* str) {
  int end;
  int trim = 0;

  for (end = str->len - 1; end >= 0 && is_slash(str->buf[end]); end--) {
    trim++;
  }
  if (trim) {
    return w_string_piece(str->buf, str->len - trim);
  }
  return w_string_piece(str);
}

bool w_string_is_known_unicode(w_string_t* str) {
  return str->type == W_STRING_UNICODE;
}

bool w_string_path_is_absolute(const w_string_t* str) {
  return w_is_path_absolute_cstr_len(str->buf, str->len);
}

bool w_is_path_absolute_cstr(const char* path) {
  return w_is_path_absolute_cstr_len(path, strlen_uint32(path));
}

bool w_is_path_absolute_cstr_len(const char* path, uint32_t len) {
#ifdef _WIN32
  char drive_letter;

  if (len <= 2) {
    return false;
  }

  // "\something"
  if (is_slash(path[0])) {
    // "\\something" is absolute, "\something" is relative to the current
    // dir of the current drive, whatever that may be, for a given process
    return is_slash(path[1]);
  }

  drive_letter = (char)tolower(path[0]);
  // "C:something"
  if (drive_letter >= 'a' && drive_letter <= 'z' && path[1] == ':') {
    // "C:\something" is absolute, but "C:something" is relative to
    // the current dir on the C drive(!)
    return is_slash(path[2]);
  }
  // we could check for things like NUL:, COM: and so on here.
  // While those are technically absolute names, we can't watch them, so
  // we don't consider them absolute for the purposes of checking whether
  // the path is a valid watchable root
  return false;
#else
  return len > 0 && path[0] == '/';
#endif
}

std::ostream& operator<<(std::ostream& stream, const w_string& a) {
  stream << a.view();
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const w_string_piece& a) {
  stream << a.view();
  return stream;
}

/* vim:ts=2:sw=2:et:
 */

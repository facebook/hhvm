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

namespace watchman {
struct StringHeader;
}

using namespace watchman;

static void w_string_delref(StringHeader* str);
static StringHeader*
w_string_new_len_typed(const char* str, uint32_t len, w_string_type_t type);

// string piece

w_string_piece::w_string_piece(w_string_piece&& other) noexcept
    : str_(other.str_), len_(other.len_) {
  other.str_ = nullptr;
  other.len_ = 0;
}

w_string w_string_piece::asWString(w_string_type_t stringType) const {
  return w_string(data(), size(), stringType);
}

w_string w_string_piece::asLowerCase(w_string_type_t stringType) const {
  // TODO: a fast islower check would avoid the unconditional
  // allocation.

  uint32_t len = size();
  return w_string::generate(len, stringType, [&](char* buf) {
    for (uint32_t i = 0; i < len; ++i) {
      // TODO: `tolower` depends on locale.
      buf[i] = static_cast<char>(tolower(static_cast<unsigned char>(str_[i])));
    }
  });
}

std::optional<w_string> w_string_piece::asLowerCaseSuffix(
    w_string_type_t stringType) const {
  w_string_piece suffixPiece = this->suffix();
  if (suffixPiece.empty()) {
    return std::nullopt;
  }

  return suffixPiece.asLowerCase(stringType);
}

w_string w_string_piece::asUTF8Clean() const {
  w_string s(str_, len_, W_STRING_UNICODE);
  utf8_fix_string(const_cast<char*>(s.data()), s.size());
  return s;
}

bool w_string_piece::pathIsAbsolute() const {
  return w_string_path_is_absolute(*this);
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
  if (len_ == 0) {
    return {};
  }
  const char* const e = str_ + len_;
  for (auto end = e - 1; end >= str_; --end) {
    if (is_slash(*end)) {
      /* found the end of the parent dir */
#ifdef _WIN32
      if (end > str_ && end[-1] == ':') {
        // Special case for "C:\"; we want to keep the
        // trailing slash for this case so that we continue
        // to consider it an absolute path
        return w_string_piece(str_, 1 + end - str_);
      }
#endif
      return w_string_piece(str_, end - str_);
    }
  }
  return {};
}

w_string_piece w_string_piece::baseName() const {
  if (len_ == 0) {
    return *this;
  }
  const char* const e = str_ + len_;
  for (auto end = e - 1; end >= str_; --end) {
    if (is_slash(*end)) {
      /* found the end of the parent dir */
#ifdef _WIN32
      if (end == e && end > str_ && end[-1] == ':') {
        // Special case for "C:\"; we want the baseName to
        // be this same component so that we continue
        // to consider it an absolute path
        return *this;
      }
#endif
      return w_string_piece(end + 1, e - (end + 1));
    }
  }

  return *this;
}

w_string_piece w_string_piece::suffix() const {
  if (len_ == 0) {
    return {};
  }
  const char* const e = str_ + len_;
  for (auto end = e - 1; end >= str_; --end) {
    if (is_slash(*end)) {
      return {};
    }
    if (*end == '.') {
      return w_string_piece(end + 1, e - (end + 1));
    }
  }
  return {};
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

  auto me = str_;
  auto pref = prefix.str_;

  const char* const end = prefix.str_ + prefix.len_;
  while (pref < end) {
    // TODO: `tolower` depends on locale.
    if (tolower((uint8_t)*me) != tolower((uint8_t)*pref)) {
      return false;
    }
    ++pref;
    ++me;
  }
  return true;
}

// string

w_string::~w_string() {
  if (str_) {
    w_string_delref(str_);
  }
}

w_string::w_string(const w_string& other) : str_(other.str_) {
  if (str_) {
    str_->addref();
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

  str_ = StringHeader::alloc(len, W_STRING_UNICODE);
  char* buf = str_->buf();

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
    str_->addref();
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

inline uint32_t hash_string(const char* str, size_t len) {
  // Watchman used to use Bob Jenkins's lookup3. Many good hash functions exist,
  // but, empirically, the standard library's are faster than lookup3 and
  // convenient.
  size_t hash = std::hash<std::string_view>{}(std::string_view(str, len));
  // Supporting 32-bit is easy: we can skip the mix.
  static_assert(
      sizeof(size_t) == sizeof(uint64_t), "32-bit platforms are not supported");

  // We could use do fancy mixing like with twang_32from64 but a simple xor
  // should be sufficient for hash tables.
  return (hash >> 32) ^ hash;
}

StringHash w_string::computeAndStoreHash() const noexcept {
  StringHash hash = hash_string(str_->buf(), str_->len);

  str_->_hval.store(hash, std::memory_order_release);
  str_->set_hval_computed();
  return hash;
}

static inline uint32_t checked_len(size_t len) {
  if (len > UINT32_MAX) {
    throw std::range_error("string length exceeds UINT32_MAX");
  }
  return len;
}

w_string::w_string(const char* buf, size_t len, w_string_type_t stringType)
    : w_string{w_string_new_len_typed(buf, checked_len(len), stringType)} {}

w_string::w_string(const char* buf, w_string_type_t stringType)
    : w_string{w_string_new_len_typed(buf, strlen_uint32(buf), stringType)} {}

w_string w_string::dirName() const {
  return w_string_piece(*this).dirName().asWString();
}

w_string w_string::baseName() const {
  return w_string_piece(*this).baseName().asWString();
}

std::optional<w_string> w_string::asLowerCaseSuffix() const {
  return piece().asLowerCaseSuffix();
}

w_string w_string::normalizeSeparators(char targetSeparator) const {
  uint32_t len = str_->len;

  if (len == 0) {
    return *this;
  }

  const char* thisbuf = str_->buf();

  // This doesn't do any special UNC or path len escape prefix handling
  // on windows.  We don't currently use it in a way that would require it.

  // Trim any trailing dir seps
  while (len > 0) {
    if (thisbuf[len - 1] == '/' || thisbuf[len - 1] == '\\') {
      --len;
    } else {
      break;
    }
  }

  return generate(len, W_STRING_BYTE, [&](char* buf) {
    for (uint32_t i = 0; i < len; ++i) {
      char c = thisbuf[i];
      buf[i] = (c == '/' || c == '\\') ? targetSeparator : c;
    }
  });
}

bool w_string::operator<(const w_string& other) const {
  if (str_ == other.str_) {
    // identity fast path
    return false;
  } else {
    return view() < other.view();
  }
}

bool w_string::operator==(const w_string& other) const {
  if (str_ == other.str_) {
    // identity fast path
    return true;
  } else if (
      str_->has_hval() && other.str_->has_hval() &&
      str_->_hval != other.str_->_hval) {
    return false;
  } else {
    return view() == other.view();
  }
}

bool w_string::operator!=(const w_string& other) const {
  return !(*this == other);
}

w_string w_string::pathCat(std::initializer_list<w_string_piece> elems) {
  uint32_t length = 0;

  for (auto& p : elems) {
    length += p.size() + 1;
  }

  auto* s = StringHeader::alloc(length, W_STRING_BYTE);
  char* buf = s->buf();

  for (auto& p : elems) {
    if (p.size() == 0) {
      // Skip empty strings
      continue;
    }
    if (buf != s->buf()) {
      *buf = '/';
      ++buf;
    }
    memcpy(buf, p.data(), p.size());
    buf += p.size();
  }
  *buf = 0;
  // Post-hoc length adjustment.
  // TODO: correctly calculate destination length up front.
  s->len = buf - s->buf();

  return w_string{s};
}

StringHash w_string_piece::hashValue() const noexcept {
  return hash_string(data(), size());
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
  int len, res, prefix_len;
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

static StringHeader*
w_string_new_len_typed(const char* str, uint32_t len, w_string_type_t type) {
  auto* s = StringHeader::alloc(len, type);
  char* buf = s->buf();
  if (len) {
    memcpy(buf, str, len);
  }
  buf[len] = 0;
  return s;
}

void w_string_delref(StringHeader* str) {
  if (str->decref()) {
    // Call the destructor.  We can't use regular delete because
    // we allocated with malloc().
    str->~StringHeader();
    free(str);
  }
}

bool w_string_equal_caseless(w_string_piece a, w_string_piece b) {
  uint32_t i;

  if (a.size() != b.size()) {
    return false;
  }
  for (i = 0; i < a.size(); i++) {
    // TODO: `tolower` depends on locale.
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

  if (str_[base - 1] != '.') {
    return false;
  }

  for (i = 0; i < suffix.size(); i++) {
    // TODO: `tolower` depends on locale.
    if (tolower((uint8_t)str_[base + i]) != suffix[i]) {
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

w_string_piece w_string_canon_path(w_string_piece str) {
  const char* begin = str.data();
  const char* end = str.data() + str.size();
  while (end != begin && is_slash(end[-1])) {
    --end;
  }
  return w_string_piece{begin, end};
}

bool w_string_path_is_absolute(w_string_piece path) {
  // TODO: To avoid needing a strlen in call sites that have a null-terminated
  // `const char*` path, this function could have a `const char*` overload
  // because it only ever looks at the first few characters of the string.

#ifdef _WIN32
  char drive_letter;

  if (path.size() <= 2) {
    return false;
  }

  // "\something"
  if (is_slash(path[0])) {
    // "\\something" is absolute, "\something" is relative to the current
    // dir of the current drive, whatever that may be, for a given process
    return is_slash(path[1]);
  }

  // TODO: `tolower` depends on locale.
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
  return path.size() > 0 && path[0] == '/';
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

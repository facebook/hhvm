/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "watchman/watchman_system.h"

#include <fmt/core.h>
#include <folly/FBString.h>

#include <stdint.h>
#include <string.h>

#include <atomic>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

class w_string_piece;

enum w_string_type_t : uint8_t {
  W_STRING_BYTE,
  W_STRING_UNICODE,
  W_STRING_MIXED
};

// Assume 64-bit platforms for now. 32-bit platforms should have a separate
// 32-bit flag next to the reference count.
static_assert(sizeof(size_t) == 8);

// Required for fmt 10
inline uint8_t format_as(w_string_type_t type) {
  return static_cast<uint8_t>(type);
}

namespace watchman {

/**
 * To pack StringHeader into 16 bytes, Watchman uses a 32-bit hash scheme. It's
 * important to hold the invariant that std::hash<w_string> ==
 * std::hash<w_string_piece>, so if we replace w_string_piece with
 * std::string_view, StringHeader will need to become 20 or 24 bytes.
 */
using StringHash = uint32_t;

/**
 * w_string is a heap-allocated string buffer with a fixed-size header and
 * variable-size contents.
 */
struct StringHeader {
  // Bottom 2 bits are w_string_type_t.
  // Third bit is whether _hval is computed.
  // Remaining 61 bits are the reference count. At 10 nanoseconds per increment,
  // overflow would take over 700 years.
  std::atomic<size_t> refcnt;
  uint32_t len;
  std::atomic<StringHash> _hval;

  StringHeader(w_string_type_t type, uint32_t len)
      : refcnt{static_cast<size_t>(type) | kRefIncrement}, len{len} {}

  bool has_hval() const {
    return refcnt.load(std::memory_order_acquire) & kHasHval;
  }

  void set_hval_computed() {
    refcnt.fetch_or(kHasHval, std::memory_order_acq_rel);
  }

  w_string_type_t get_type() const {
    return static_cast<w_string_type_t>(
        refcnt.load(std::memory_order_relaxed) & kTypeMask);
  }

  void addref() {
    refcnt.fetch_add(kRefIncrement, std::memory_order_relaxed);
  }

  // Returns true if this was the final reference, indicating the string
  // should be destroyed.
  bool decref() {
    // In the common case that the reference count is 1, we can avoid an
    // expensive atomic RMW (e.g. lock xadd) with a single load. This saves a
    // dozen or two cycles.
    //
    // This is safe because, if the reference count is 1, we have exclusive
    // ownership of the object. Therefore, the reference count cannot climb
    // to 2. (It is UB to destroy an object while another thread copies it.)
    //
    // This technique is surprisingly uncommon, but it shows up in abseil:
    // https://github.com/abseil/abseil-cpp/blob/4a1ccf16ed98c876bf1e1985afb080baeff5101f/absl/strings/internal/cord_internal.h#L114
    return (
        kRefIncrement == (refcnt.load(std::memory_order_acquire) & kRefMask) ||
        kRefIncrement ==
            (refcnt.fetch_sub(kRefIncrement, std::memory_order_acq_rel) &
             kRefMask));
  }

  /**
   * Allocates storage for length + 1 bytes.
   */
  static StringHeader* alloc(uint32_t length, w_string_type_t type) {
    void* s = malloc(sizeof(StringHeader) + length + 1);
    if (!s) {
      throw std::bad_alloc{};
    }
    new (s) StringHeader(type, length);
    return static_cast<StringHeader*>(s);
  }

  char* buf() {
    return reinterpret_cast<char*>(this + 1);
  }

  const char* buf() const {
    return reinterpret_cast<const char*>(this + 1);
  }

  static constexpr uint8_t kTypeMask = 3ull;
  static constexpr size_t kHasHval = 1ull << 2ull;
  static constexpr size_t kRefShift = 3ull;
  static constexpr size_t kRefIncrement = 1ull << kRefShift;
  static constexpr size_t kRefMask = ~(kRefIncrement - 1);
};

} // namespace watchman

/**
 * Trims all trailing slashes from a path.
 */
w_string_piece w_string_canon_path(w_string_piece str);

/**
 * Returns true if the specified string is an absolute path.
 * On Windows, this accounts for UNC paths (which are absolute) and
 * drive-relative paths (which are not).
 */
bool w_string_path_is_absolute(w_string_piece path);

uint32_t strlen_uint32(const char* str);

inline bool is_slash(char c) {
  return c == '/'
#ifdef _WIN32
      || c == '\\'
#endif
      ;
}

class w_string;

/** Represents a view over some externally managed string storage.
 * It is simply a pair of pointers that define the start and end
 * of the valid region. */
class w_string_piece {
  using StringHash = watchman::StringHash;

  const char* str_;
  size_t len_;

 public:
  w_string_piece() : str_{nullptr}, len_{0} {}

  /* implicit */ w_string_piece(const std::string& str)
      : str_{str.data()}, len_{str.size()} {}

  /* implicit */ w_string_piece(const folly::fbstring& str)
      : str_{str.data()}, len_{str.size()} {}

  /* implicit */ w_string_piece(std::nullptr_t) = delete;

  /* implicit */ w_string_piece(const char* cstr)
      : str_(cstr), len_(strlen(cstr)) {}

  w_string_piece(const char* cstr, size_t len) : str_(cstr), len_(len) {}

  w_string_piece(const char* begin, const char* end)
      : str_{begin}, len_{static_cast<size_t>(end - begin)} {
    assert(end >= begin);
  }

  /* implicit */ w_string_piece(std::string_view sv)
      : w_string_piece{sv.data(), sv.size()} {}

  w_string_piece(const w_string_piece& other) = default;
  w_string_piece& operator=(const w_string_piece& other) = default;
  w_string_piece(w_string_piece&& other) noexcept;

  const char* data() const noexcept {
    return str_;
  }

  bool empty() const noexcept {
    return len_ == 0;
  }

  size_t size() const noexcept {
    return len_;
  }

  const char& operator[](size_t i) const noexcept {
    return str_[i];
  }

  /** move the start of the string by n characters, stripping off that prefix */
  void advance(size_t n) {
    if (n > len_) {
      throw std::range_error("index out of range");
    }
    str_ += n;
    len_ -= n;
  }

  /** Return a copy of the string as a w_string */
  w_string asWString(w_string_type_t stringType = W_STRING_BYTE) const;

  /** Return a lowercased copy of the string */
  w_string asLowerCase(w_string_type_t stringType = W_STRING_BYTE) const;

  /** Return a lowercased copy of the suffix */
  std::optional<w_string> asLowerCaseSuffix(
      w_string_type_t stringType = W_STRING_BYTE) const;

  /** Return a UTF-8-clean copy of the string */
  w_string asUTF8Clean() const;

  /** Returns true if the filename suffix of this string matches
   * the provided suffix, which must be lower cased.
   * This string piece lower cased and matched against suffix */
  bool hasSuffix(w_string_piece suffix) const;

  bool pathIsAbsolute() const;
  bool pathIsEqual(w_string_piece other) const;
  w_string_piece dirName() const;
  w_string_piece baseName() const;
  w_string_piece suffix() const;

  /** Split the string by delimiter and emit to the provided vector */
  template <typename Vector>
  void split(Vector& result, char delim) const {
    const char* begin = str_;
    const char* const end = str_ + len_;
    const char* it = begin;
    while (it != end) {
      if (*it == delim) {
        result.emplace_back(begin, it - begin);
        begin = ++it;
        continue;
      }
      ++it;
    }

    if (begin != end) {
      result.emplace_back(begin, end - begin);
    }
  }

  std::string string() const {
    return std::string{view()};
  }

  std::string_view view() const {
    return std::string_view{str_, len_};
  }

  friend bool operator==(w_string_piece lhs, w_string_piece rhs) {
    return lhs.view() == rhs.view();
  }

  friend bool operator!=(w_string_piece lhs, w_string_piece rhs) {
    return lhs.view() != rhs.view();
  }

  friend bool operator<(w_string_piece lhs, w_string_piece rhs) {
    return lhs.view() < rhs.view();
  }

  bool contains(w_string_piece needle) const;
  bool startsWith(w_string_piece prefix) const;
  bool startsWithCaseInsensitive(w_string_piece prefix) const;

  // Compute a hash value for this piece
  StringHash hashValue() const noexcept;

#ifdef _WIN32
  // Returns a wide character representation of the piece
  std::wstring asWideUNC() const;
#endif
};

bool w_string_equal_caseless(w_string_piece a, w_string_piece b);

/**
 * w_string is a reference-counted, immutable, 8-bit string type.
 * It can hold known-unicode text, known-binary data, or a mixture of both.
 * The default-initialized and moved-from w_string values are empty.
 */
class w_string {
  using StringHash = watchman::StringHash;
  using StringHeader = watchman::StringHeader;

 public:
  /**
   * Constructs an empty string.
   */
  w_string() = default;

  /* implicit */ w_string(std::nullptr_t) = delete;

  /**
   * Make a new string from some bytes and a type.
   */
  w_string(
      const char* buf,
      size_t len,
      w_string_type_t stringType = W_STRING_BYTE);

  /**
   * Make a new string from a null-terminated array.
   */
  /* implicit */ w_string(
      const char* buf,
      w_string_type_t stringType = W_STRING_BYTE);

  explicit w_string(std::string_view sv) : w_string{sv.data(), sv.size()} {}

  /**
   * Copy a std::string into a w_string.
   */
  explicit w_string(const std::string& str)
      : w_string{str.data(), str.size()} {}

#ifdef _WIN32
  /** Convert a wide character path to utf-8 and return it as a w_string.
   * This constructor is intended only for path names and not as a general
   * WCHAR -> w_string constructor; it will always apply path mapping
   * logic to the result and may mangle non-pathname strings if they
   * are passed to it. */
  w_string(const WCHAR* wpath, size_t len);
#endif

  /** Private constructor. Takes ownership of StringHeader*. */
  explicit w_string(StringHeader* str) : str_{str} {}

  ~w_string();

  /** Copying adds a ref */
  w_string(const w_string& other);
  w_string& operator=(const w_string& other);

  /** Moving steals a ref */
  w_string(w_string&& other) noexcept;
  w_string& operator=(w_string&& other) noexcept;

  /**
   * Stop tracking the underlying string object, decrementing the reference
   * count.
   */
  void reset() noexcept;

  StringHash hashValue() const noexcept {
    if (str_) {
      if (str_->has_hval()) {
        return str_->_hval.load(std::memory_order_acquire);
      } else {
        // It's okay to race has computation: hashing is cheaper than blocking.
        return computeAndStoreHash();
      }
    } else {
      return w_string_piece{}.hashValue();
    }
  }

  /**
   * Returns a copy of this w_string's data as a std::string.
   */
  std::string string() const {
    return std::string{view()};
  }

  /**
   * Returns a std::string_view covering this w_string's data. Returns the empty
   * string_view if null.
   */
  std::string_view view() const noexcept {
    if (str_ == nullptr) {
      return {};
    }
    return std::string_view(data(), size());
  }

  /**
   * Returns a w_string_piece covering this w_string's data. Returns the empty
   * w_string_piece if null.
   */
  w_string_piece piece() const noexcept {
    if (str_ == nullptr) {
      return w_string_piece();
    }
    return w_string_piece(data(), size());
  }

  operator w_string_piece() const noexcept {
    return piece();
  }

  bool operator==(const w_string& other) const;
  bool operator!=(const w_string& other) const;
  bool operator<(const w_string& other) const;

  friend bool operator==(const w_string& lhs, const char* rhs) {
    return lhs.view() == rhs;
  }

  friend bool operator!=(const w_string& lhs, const char* rhs) {
    return lhs.view() != rhs;
  }

  friend bool operator==(const char* lhs, const w_string& rhs) {
    return lhs == rhs.view();
  }

  friend bool operator!=(const char* lhs, const w_string& rhs) {
    return lhs != rhs.view();
  }

  friend bool operator==(const w_string& lhs, std::nullptr_t) {
    return lhs.str_ == nullptr;
  }

  friend bool operator!=(const w_string& lhs, std::nullptr_t) {
    return lhs.str_ != nullptr;
  }

  friend bool operator==(std::nullptr_t, const w_string& rhs) {
    return nullptr == rhs.str_;
  }

  friend bool operator!=(std::nullptr_t, const w_string& rhs) {
    return nullptr != rhs.str_;
  }

  /**
   * Generates a w_string with a known size by calling a function to
   * produce the contents.
   */
  template <typename Fn>
  static w_string generate(uint32_t size, w_string_type_t type, Fn&& fn) {
    auto* s = StringHeader::alloc(size, type);
    w_string rv{s}; // Will deallocate if fn fails.
    char* buf = s->buf();
    fn(buf);
    buf[size] = 0;
    return rv;
  }

  /** path concatenation
   * Pass in a list of w_string_pieces to join them all similarly to
   * the python os.path.join() function. */
  static w_string pathCat(std::initializer_list<w_string_piece> elems);

  /** build a w_string by concatenating the string formatted representation
   * of each of the supplied arguments */
  template <typename... Args>
  static w_string build(Args&&... args) {
    static const char format_str[] = "{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}";
    static_assert(
        sizeof...(args) <= sizeof(format_str) / 2,
        "too many args passed to w_string::build");
    return w_string::format(
        fmt::string_view(format_str, sizeof...(args) * 2),
        std::forward<Args>(args)...);
  }

  /** Construct a new string using the `fmt` formatting library.
   * Syntax: https://fmt.dev/latest/syntax.html
   */
  template <typename... Args>
  static w_string format(fmt::string_view format_str, Args&&... args) {
    uint32_t size = fmt::formatted_size(fmt::runtime(format_str), args...);
    return w_string::generate(size, W_STRING_BYTE, [&](char* buf) {
      fmt::format_to(buf, fmt::runtime(format_str), args...);
    });
  }

  /** Return a possibly new version of this string that has its separators
   * normalized to unix slashes */
  w_string normalizeSeparators(char targetSeparator = '/') const;

  /** Returns a pointer to a null terminated c-string. */
  const char* c_str() const {
    return data();
  }
  const char* data() const {
    return str_ ? str_->buf() : nullptr;
  }

  bool empty() const {
    return str_ ? (str_->len == 0) : true;
  }

  size_t size() const {
    return str_ ? str_->len : 0;
  }

  w_string_type_t type() const {
    // Empty strings are known unicode.
    return str_ ? str_->get_type() : W_STRING_UNICODE;
  }

  /** Returns the directory component of the string, assuming a path string */
  w_string dirName() const;
  /** Returns the file name component of the string, assuming a path string */
  w_string baseName() const;
  /** Returns the filename suffix of a path string */
  std::optional<w_string> asLowerCaseSuffix() const;

 private:
  StringHash computeAndStoreHash() const noexcept;

  StringHeader* str_ = nullptr;
};

/** Allow w_string to act as a key in unordered_(map|set) */
namespace std {
template <>
struct hash<w_string> {
  std::size_t operator()(w_string const& str) const noexcept {
    return str.hashValue();
  }
};
template <>
struct hash<w_string_piece> {
  std::size_t operator()(w_string_piece const& str) const noexcept {
    return str.hashValue();
  }
};
} // namespace std

// Streaming operators for logging and printing
std::ostream& operator<<(std::ostream& stream, const w_string& a);
std::ostream& operator<<(std::ostream& stream, const w_string_piece& a);

// Allow formatting w_string and w_string_piece
namespace fmt {
template <>
struct formatter<w_string> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) const {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const w_string& s, FormatContext& ctx) const {
    return format_to(ctx.out(), "{}", s.view());
  }
};

template <>
struct formatter<w_string_piece> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) const {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const w_string_piece& s, FormatContext& ctx) const {
    return format_to(ctx.out(), "{}", s.view());
  }
};
} // namespace fmt

/**
 * Write as many characters from the beginning of `piece` into `array` as will
 * fit. If the string is too long, the truncated characters are replaced with
 * "...".
 *
 * This function primarily exists for fixed-size, in-memory logging.
 *
 * The resulting array may not be null-terminated. Use strnlen to compute its
 * length.
 */
template <size_t N>
void storeTruncatedHead(char (&array)[N], w_string_piece piece) {
  if (piece.size() > N) {
    memcpy(array, piece.data(), N - 3);
    array[N - 3] = '.';
    array[N - 2] = '.';
    array[N - 1] = '.';
  } else {
    memcpy(array, piece.data(), piece.size());
    if (piece.size() < N) {
      array[piece.size()] = 0;
    }
  }
}

/**
 * Write as many characters from the end of `piece` into `array` as will
 * fit. If the string is too long, the truncated characters are replaced with
 * "...".
 *
 * This function primarily exists for fixed-size, in-memory logging.
 *
 * The resulting array may not be null-terminated. Use strnlen to compute its
 * length.
 */
template <size_t N>
void storeTruncatedTail(char (&array)[N], w_string_piece piece) {
  if (piece.size() > N) {
    array[0] = '.';
    array[1] = '.';
    array[2] = '.';
    memcpy(array + 3, piece.data() + piece.size() - (N - 3), N - 3);
  } else {
    memcpy(array, piece.data(), piece.size());
    if (piece.size() < N) {
      array[piece.size()] = 0;
    }
  }
}

/* vim:ts=2:sw=2:et:
 */

/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>
#include <folly/portability/Windows.h> // for windows compatibility: STRICT maybe defined by some win headers

#ifdef STRICT
#undef STRICT
#endif

#ifdef STRICT_COMPAT
#undef STRICT_COMPAT
#endif

namespace proxygen {

// Case-insensitive string comparison
inline bool caseInsensitiveEqual(folly::StringPiece s, folly::StringPiece t) {
  return s.equals(t, folly::AsciiCaseInsensitive{});
}

struct AsciiCaseUnderscoreInsensitive {
  bool operator()(char lhs, char rhs) const {
    if (lhs == '_') {
      lhs = '-';
    }
    if (rhs == '_') {
      rhs = '-';
    }
    return folly::AsciiCaseInsensitive()(lhs, rhs);
  }
};

// Case-insensitive string comparison
inline bool caseUnderscoreInsensitiveEqual(folly::StringPiece s,
                                           folly::StringPiece t) {
  return s.equals(t, AsciiCaseUnderscoreInsensitive{});
}

enum class URLValidateMode { STRICT_COMPAT, STRICT };
inline bool validateURL(std::string_view url,
                        URLValidateMode mode = URLValidateMode::STRICT) {
  for (uint8_t p : url) {
    if (p <= 0x20 || p == 0x7f ||
        (p > 0x7f && mode != URLValidateMode::STRICT_COMPAT)) {
      // no controls or unescaped spaces
      return false;
    }
  }
  return true;
}

inline size_t findLastOf(folly::StringPiece sp, char c) {
  size_t pos = sp.size();
  while (--pos != std::string::npos && sp[pos] != c) {
    // pass
  }
  return pos;
}

template <typename Tout, typename Tin>
Tout clamped_downcast(Tin value) {
  return static_cast<Tout>(
      std::min(static_cast<uint64_t>(value),
               static_cast<uint64_t>(std::numeric_limits<Tout>::max())));
}

// Like std::isalpha but independent of the current locale.
inline bool isAlpha(uint8_t c) {
  return ((unsigned int)(c | 0x20) - 'a') < 26U;
}

} // namespace proxygen

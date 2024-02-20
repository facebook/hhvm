/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include <string>
#include <string_view>

#include "hphp/util/assertions.h"
#include "hphp/util/hash.h"

namespace HPHP {

struct StringData;

namespace Facts {

/**
 * Stripped-down interface to HPHP::StringData.
 */
struct StringPtr {
  explicit StringPtr(const StringData* impl) noexcept : m_impl{impl} {}

  StringPtr() = default;

  bool operator==(const StringPtr& o) const noexcept {
    return same(o);
  }

  bool operator!=(const StringPtr& o) const noexcept {
    return !operator==(o);
  }

  bool operator==(const StringData* p) const noexcept {
    return m_impl == p;
  }

  bool operator!=(const StringData* p) const noexcept {
    return !operator==(p);
  }

  bool operator==(const std::string_view s) const noexcept {
    if (m_impl == nullptr) {
      return false;
    }
    return slice() == s;
  }

  bool operator!=(const std::string_view s) const noexcept {
    return !operator==(s);
  }

  /*
   * Returns a slice with extents sized to the *string* that this
   * StringData wraps.  This range does not include a null terminator.
   *
   * Note: please do not add new code that assumes the range does
   * include a null-terminator if possible.  (We would like to make
   * this unnecessary eventually.)
   */
  std::string_view slice() const noexcept;

  /*
   * Accessor for the length of a string.
   *
   * Note: size() returns a signed int for historical reasons.  It is
   * guaranteed to be in the range (0 <= size() <= MaxSize)
   */
  int size() const noexcept;

  /*
   * Returns: size() == 0
   */
  bool empty() const noexcept;

  /*
   * Returns: case insensitive hash value for this string.
   */
  strhash_t hash() const noexcept;

  /*
   * Exact comparison, in the sense of php's string === operator.
   * (Exact, case-sensitive comparison.)
   */
  bool same(const StringPtr& s) const noexcept;

  /*
   * Case-insensitive exact string comparison.  (Numeric strings are
   * not treated specially.)
   */
  bool tsame(const StringPtr& s) const noexcept;
  bool fsame(const StringPtr& s) const noexcept;
  static bool tsame_slice(std::string_view, std::string_view) noexcept;
  static bool fsame_slice(std::string_view, std::string_view) noexcept;

  const StringData* get() const noexcept {
    return m_impl;
  }

  const StringData* operator->() const noexcept {
    return get();
  }

  const StringData& operator*() const noexcept {
    return *get();
  }

 private:
  friend std::ostream& operator<<(std::ostream& os, const StringPtr& s);
  const StringData* m_impl = nullptr;
};

StringPtr makeStringPtr(std::string_view s);
StringPtr makeStringPtr(const StringData& s);

std::ostream& operator<<(std::ostream& os, const StringPtr& s);

} // namespace Facts
} // namespace HPHP

template <>
struct std::hash<HPHP::Facts::StringPtr> {
  std::size_t operator()(const HPHP::Facts::StringPtr& s) const noexcept {
    return s.hash();
  }
};

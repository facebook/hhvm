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

#include "hphp/util/hash.h"

namespace HPHP {

/**
 * Implements our StringPtr class in terms of std::string.
 *
 * For unit-testing purposes only. By contrast, see
 * string_data_ptr.cpp, which implements this class in terms of
 * HPHP::StringData for production use.
 */
struct StringData {
 public:
  /* implicit */ StringData(const char* s) : m_impl{s} {}
  /* implicit */ StringData(std::string&& s) : m_impl{std::move(s)} {}
  explicit StringData(std::string_view s) : m_impl{s} {}
  std::string* impl() const;
  std::string_view slice() const noexcept;
  bool empty() const;
  size_t size() const;
  strhash_t hash() const noexcept;
  bool same(const StringData& o) const noexcept;
  bool tsame(const StringData& o) const noexcept;
  bool fsame(const StringData& o) const noexcept;

 private:
  mutable std::string m_impl;
};

} // namespace HPHP

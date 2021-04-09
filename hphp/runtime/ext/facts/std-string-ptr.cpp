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

#include <memory>
#include <string>

#include <folly/concurrency/ConcurrentHashMap.h>

#include "hphp/runtime/ext/facts/string-ptr.h"
#include "hphp/util/bstring.h"
#include "hphp/util/hash.h"

/**
 * Implements our StringPtr class in terms of std::string.
 *
 * For unit-testing purposes only. By contrast, see
 * string_data_ptr.cpp, which implements this class in terms of
 * HPHP::StringData for production use.
 */

namespace HPHP {
namespace Facts {

namespace {

/**
 * Insert-only store of static pointers
 */
folly::ConcurrentHashMap<StringPtr<std::string>, std::unique_ptr<std::string>>
    s_stringTable;

} // namespace

template <>
StringPtr<std::string>::StringPtr(const std::string* impl) noexcept
    : m_impl{impl} {
}

template <> std::string_view StringPtr<std::string>::slice() const noexcept {
  assertx(m_impl != nullptr);
  return std::string_view{*m_impl};
}

template <> int StringPtr<std::string>::size() const noexcept {
  assertx(m_impl != nullptr);
  return m_impl->size();
}

template <> bool StringPtr<std::string>::empty() const noexcept {
  assertx(m_impl != nullptr);
  return m_impl->empty();
}

template <> strhash_t StringPtr<std::string>::hash() const noexcept {
  assertx(m_impl != nullptr);
  return hash_string_i(m_impl->c_str(), m_impl->size());
}

template <>
bool StringPtr<std::string>::same(
    const StringPtr<std::string>& s) const noexcept {
  if (m_impl == s.m_impl) {
    return true;
  }
  if (m_impl == nullptr || s.m_impl == nullptr) {
    return false;
  }
  return *m_impl == *s.m_impl;
}

template <>
bool StringPtr<std::string>::isame(
    const StringPtr<std::string>& s) const noexcept {
  if (m_impl == s.m_impl) {
    return true;
  }
  if (m_impl == nullptr || s.m_impl == nullptr) {
    return false;
  }
  if (m_impl->size() != s.m_impl->size()) {
    return false;
  }
  return bstrcaseeq(m_impl->c_str(), s.m_impl->c_str(), m_impl->size());
}

template <>
StringPtr<std::string> makeStringPtr<std::string>(const std::string& s) {
  auto it = s_stringTable.find(StringPtr{&s});
  if (it != s_stringTable.end()) {
    return it->first;
  }

  auto staticStr = std::make_unique<std::string>(s);
  auto strKey = StringPtr{staticStr.get()};
  return StringPtr{
      s_stringTable.insert(strKey, std::move(staticStr)).first->first};
}

template <>
StringPtr<std::string> makeStringPtr<std::string>(const std::string_view sv) {
  return makeStringPtr<std::string>(std::string{sv});
}

template struct StringPtr<std::string>;

} // namespace Facts
} // namespace HPHP

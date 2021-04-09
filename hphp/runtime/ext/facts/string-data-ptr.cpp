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

#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/ext/facts/string-ptr.h"

/**
 * Implements our StringPtr class in terms of HPHP::StringData.
 *
 * By contrast, see std_string_ptr.cpp, which implements this class in
 * terms of std::string so our tests don't need to link against
 * //hphp/runtime/base:runtime_base.
 */

namespace HPHP {
namespace Facts {

template <>
StringPtr<StringData>::StringPtr(const StringData* impl) noexcept
    : m_impl{impl} {
}

template <> std::string_view StringPtr<StringData>::slice() const noexcept {
  if (m_impl == nullptr) {
    return std::string_view{};
  }
  return m_impl->slice();
}

template <> int StringPtr<StringData>::size() const noexcept {
  if (m_impl == nullptr) {
    return 0;
  }
  return m_impl->size();
}

template <> bool StringPtr<StringData>::empty() const noexcept {
  return m_impl == nullptr || m_impl->empty();
}

template <> strhash_t StringPtr<StringData>::hash() const noexcept {
  return m_impl->hash();
}

template <>
bool StringPtr<StringData>::same(
    const StringPtr<StringData>& s) const noexcept {
  if (m_impl == s.m_impl) {
    return true;
  }
  if (m_impl == nullptr || s.m_impl == nullptr) {
    return false;
  }
  return m_impl->same(s.m_impl);
}

template <>
bool StringPtr<StringData>::isame(
    const StringPtr<StringData>& s) const noexcept {
  if (m_impl == s.m_impl) {
    return true;
  }
  if (m_impl == nullptr || s.m_impl == nullptr) {
    return false;
  }
  return m_impl->isame(s.m_impl);
}

template <> StringPtr<StringData> makeStringPtr(const StringData& s) {
  return StringPtr<StringData>{makeStaticString(&s)};
}

template <> StringPtr<StringData> makeStringPtr(std::string_view s) {
  return StringPtr<StringData>{makeStaticString(s)};
}

// Use `extern template struct StringPtr<StringData>` in cpp files
// linked against this to avoid instantiating spurious copies of the
// same code
template struct StringPtr<StringData>;

} // namespace Facts
} // namespace HPHP

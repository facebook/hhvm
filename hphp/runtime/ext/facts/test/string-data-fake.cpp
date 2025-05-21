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

#include <algorithm>

#include "hphp/runtime/ext/facts/string-ptr.h"
#include "hphp/runtime/ext/facts/test/string-data-fake.h"
#include "hphp/util/assertions.h"
#include "hphp/util/bstring.h"

/**
 * Implements our StringPtr class in terms of std::string.
 *
 * For unit-testing purposes only. By contrast, see
 * string_data_ptr.cpp, which implements this class in terms of
 * HPHP::StringData for production use.
 */
namespace HPHP {

std::string* StringData::impl() const {
  return &m_impl;
}
std::string_view StringData::slice() const noexcept {
  return std::string_view{m_impl};
}
bool StringData::empty() const {
  return m_impl.empty();
}
size_t StringData::size() const {
  return m_impl.size();
}
strhash_t StringData::hash() const noexcept {
  strhash_t h = hash_string_i_unsafe(m_impl.c_str(), m_impl.size());
  assertx(h >= 0);
  return h;
}
bool StringData::same(const StringData& o) const noexcept {
  return m_impl == o.m_impl;
}
bool StringData::tsame(const StringData& o) const noexcept {
  auto lower = [](const std::string& _s) {
    std::string s = _s;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
  };
  return lower(m_impl) == lower(o.m_impl);
}
bool StringData::fsame(const StringData& o) const noexcept {
  return same(o);
}

namespace Facts {

std::string_view StringPtr::StringPtr::slice() const noexcept {
  assertx(m_impl != nullptr);
  return std::string_view{*m_impl->impl()};
}

int StringPtr::StringPtr::size() const noexcept {
  assertx(m_impl != nullptr);
  return m_impl->impl()->size();
}

bool StringPtr::StringPtr::empty() const noexcept {
  assertx(m_impl != nullptr);
  return m_impl->impl()->empty();
}

strhash_t StringPtr::StringPtr::hash() const noexcept {
  assertx(m_impl != nullptr);
  return hash_string_i(m_impl->impl()->c_str(), m_impl->impl()->size());
}

bool StringPtr::StringPtr::same(const StringPtr& s) const noexcept {
  if (m_impl->impl() == s.m_impl->impl()) {
    return true;
  }
  if (m_impl->impl() == nullptr || s.m_impl->impl() == nullptr) {
    return false;
  }
  return *m_impl->impl() == *s.m_impl->impl();
}

bool StringPtr::StringPtr::tsame(const StringPtr& s) const noexcept {
  if (m_impl->impl() == s.m_impl->impl()) {
    return true;
  }
  if (m_impl->impl() == nullptr || s.m_impl->impl() == nullptr) {
    return false;
  }
  if (m_impl->impl()->size() != s.m_impl->impl()->size()) {
    return false;
  }
  return bstrcaseeq(
      m_impl->impl()->c_str(),
      s.m_impl->impl()->c_str(),
      m_impl->impl()->size());
}

bool StringPtr::StringPtr::fsame(const StringPtr& s) const noexcept {
  return same(s);
}

bool StringPtr::tsame_slice(std::string_view a, std::string_view b) noexcept {
  auto lower = [](std::string_view t) {
    std::string s{t};
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
  };
  return lower(a) == lower(b);
}

bool StringPtr::fsame_slice(std::string_view a, std::string_view b) noexcept {
  auto lower = [](std::string_view t) {
    std::string s{t};
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
  };
  return lower(a) == lower(b);
}

StringPtr makeStringPtr(const StringData& s) {
  return TestStringTable::getInstance()->get(s);
}

StringPtr makeStringPtr(std::string_view sv) {
  return makeStringPtr(StringData{sv});
}

std::ostream& operator<<(std::ostream& os, const StringPtr& s) {
  if (s.get()->impl() == nullptr) {
    return os << "<nullptr>";
  }
  return os << *s.get()->impl();
}

} // namespace Facts
} // namespace HPHP

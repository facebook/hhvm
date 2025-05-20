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

#include <memory>
#include <string>
#include <string_view>

#include <folly/concurrency/ConcurrentHashMap.h>

#include "hphp/runtime/ext/facts/string-ptr.h"
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

namespace Facts {

struct TestStringTable {
 private:
  TestStringTable(){};

 public:
  static TestStringTable* getInstance() {
    static TestStringTable* const instance = new TestStringTable();
    return instance;
  }

  StringPtr get(const StringData& s) {
    auto it = stringTable_.find(StringPtr{&s});
    if (it != stringTable_.end()) {
      return it->first;
    }

    auto staticStr = std::make_unique<StringData>(s);
    auto strKey = StringPtr{staticStr.get()};
    return StringPtr{
        stringTable_.insert(strKey, std::move(staticStr)).first->first};
  }

  void clear() {
    stringTable_.clear();
  }

 private:
  folly::ConcurrentHashMap<StringPtr, std::unique_ptr<StringData>> stringTable_;
};

/**
 * Insert-only store of static pointers
 */

StringPtr makeStringPtr(const StringData& s);
StringPtr makeStringPtr(std::string_view sv);
std::ostream& operator<<(std::ostream& os, const StringPtr& s);

} // namespace Facts
} // namespace HPHP

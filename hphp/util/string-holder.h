/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_STRING_HOLDER_H_
#define incl_HPHP_STRING_HOLDER_H_

#include <stdint.h>
#include <cstddef>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * For now, use this for StringHolder, instead of something like
 * std::default_delete.
 */
enum struct FreeType {
  NoFree,
  Free,
  LocalFree,
};

/*
 * String holder class for storing a reference to a string that is not
 * owned by us. The type indicates whether/how to deallocate the string upon
 * destruction.
 */
struct StringHolder {
  /* implicit */
  StringHolder(std::nullptr_t = nullptr)
    : m_data(nullptr), m_len(0), m_type(FreeType::NoFree) {}
  StringHolder(const char* data, uint32_t len, FreeType t)
    : m_data(data), m_len(len), m_type(t) {}
  StringHolder(StringHolder&& o) noexcept
    : m_data(o.m_data), m_len(o.m_len), m_type(o.m_type) {
    o.m_data = nullptr;
  }
  StringHolder(const StringHolder&) = delete;

  ~StringHolder();

  StringHolder& operator=(StringHolder&&) noexcept;
  StringHolder& operator=(const StringHolder&) = delete;

  uint32_t size() const { return m_len; }

  // The length is not the size of the buffer, but the useful size of it. For
  // example, we could allocate a big buffer, but only uses part of it. The
  // capacity of the buffer isn't stored here.
  void shrinkTo(uint32_t newLen);

  const char* data() const { return m_data; }

  operator bool() const { return data() != nullptr; }

private:
  const char* m_data;
  uint32_t m_len;
  FreeType m_type;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_STRING_HOLDER_H_

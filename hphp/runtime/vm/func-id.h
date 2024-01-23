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

#pragma once

#include <cstdint>
#include <folly/Format.h>

#include "hphp/util/hash.h"
#include "hphp/util/low-ptr.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Func;

///////////////////////////////////////////////////////////////////////////////

/*
 * Unique identifier for a Func*.
 */
struct FuncId {
  static FuncId Invalid;
  static FuncId Dummy;
  using Int = uint32_t;

#ifdef USE_LOWPTR
  using Id = LowPtr<const Func>;
  Int toInt() const {
    return static_cast<uint32_t>(
      reinterpret_cast<uintptr_t>(
        m_id.get()));
  }
  const Func* getFunc() const {
    assertx(!isInvalid() && !isDummy());
    return m_id;
  }
  static FuncId fromInt(Int num) {
    return FuncId{Id(reinterpret_cast<const Func*>(num))};
  }
#else
  using Id = uint32_t;
  Int toInt() const { return m_id; }
  static FuncId fromInt(Int num) {
    return FuncId{num};
  }
#endif

  bool isInvalid() const { return m_id == Invalid.m_id; }
  bool isDummy()   const { return m_id == Dummy.m_id; }

  size_t hash()    const { return hash_int64(toInt()); }

  bool operator==(const FuncId& id) const {
    return m_id == id.m_id;
  }
  bool operator<(const FuncId& id) const {
    return toInt() < id.toInt();
  }

  Id m_id;
};

static_assert(sizeof(FuncId) == sizeof(uint32_t), "");

struct FuncIdHashCompare {
  static size_t hash(const FuncId& id) { return id.hash(); }
  static bool equal(const FuncId& a, const FuncId& b) { return a == b; }
};

///////////////////////////////////////////////////////////////////////////////
}

namespace std {
  template<> struct hash<HPHP::FuncId> {
    size_t operator()(HPHP::FuncId id) const { return id.hash(); }
  };
}

namespace folly {
template<> class FormatValue<HPHP::FuncId> {
  public:
    explicit FormatValue(HPHP::FuncId id) noexcept : m_id(id) {}

    template<typename C>
    void format(FormatArg& arg, C& cb) const {
      format_value::formatString(folly::to<std::string>(m_id.toInt()), arg, cb);
    }

  private:
    HPHP::FuncId m_id;
};
}

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

#include <folly/json/dynamic.h>

#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"

namespace HPHP {

struct Class;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Profiles the target functions for a given call site to determine the most
 * common callee along with the expected probability of calling it.
 */
struct CallTargetProfile {

  struct Choice {
    const Func* func;
    double probability;
  };

  void report(const Func* ar);

  static void reduce(CallTargetProfile& profile,
                     const CallTargetProfile& other);

  jit::vector<Choice> choose() const;

  std::string toString() const;
  folly::dynamic toDynamic() const;

  template<class Serializer>
  void serialize(Serializer& ser) const {
    for (size_t i = 0; i < kMaxEntries; i++) {
      auto const funcId = m_entries[i].funcId;
      const Func* func = funcId.isInvalid() ? nullptr
                                          : Func::fromFuncId(funcId);
      write_func(ser, func);
      write_raw(ser, m_entries[i].count);
    }
    write_raw(ser, m_untracked);
    write_raw(ser, m_init);
  }

  template<class Deserializer>
  void deserialize(Deserializer& ser) {
    for (size_t i = 0; i < kMaxEntries; i++) {
      auto const func = read_func(ser);
      m_entries[i].funcId =
        func != nullptr ? func->getFuncId() : FuncId::Invalid;
      read_raw(ser, m_entries[i].count);
    }
    read_raw(ser, m_untracked);
    read_raw(ser, m_init);
  }

 private:
  struct Entry {
    FuncId   funcId{FuncId::Invalid};
    uint32_t count{0};
  };

  void init();

  static const size_t kMaxEntries = 6;

  Entry    m_entries[kMaxEntries];
  uint32_t m_untracked{0};
  bool     m_init{false};
};

///////////////////////////////////////////////////////////////////////////////

inline const StringData* callTargetProfileKey() {
  return makeStaticString("CallTargetProfile");
}

///////////////////////////////////////////////////////////////////////////////


}}

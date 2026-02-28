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

namespace HPHP {

struct Class;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct ProfDataSerializer;
struct ProfDataDeserializer;

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

  void serialize(ProfDataSerializer&) const;

  void deserialize(ProfDataDeserializer&);

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

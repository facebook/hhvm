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

#include "hphp/runtime/vm/jit/call-target-profile.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"

#include "hphp/util/trace.h"

#include <cstring>
#include <sstream>

namespace HPHP { namespace jit {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void CallTargetProfile::report(const ActRec* ar) {
  assertx(ar);
  auto const func = ar->func();
  auto const funcId = func->getFuncId();
  FTRACE(5, "CallTargetProfile::report: funcId {} ({})\n", funcId,
         func->fullName());
  m_profile.update(funcId, 1);
}

void CallTargetProfile::reduce(CallTargetProfile& profile,
                               const CallTargetProfile& other) {
  return profile.m_profile.reduce(other.m_profile);
}

const Func* CallTargetProfile::choose(double& probability) const {
  auto const entry = m_profile.choose();
  if (entry == nullptr) {
    probability = 0;
    return nullptr;
  }
  auto const func = Func::fromFuncId(entry->value);
  probability = (double)entry->count / m_profile.total();
  FTRACE(2, "CallTargetProfile::choose(): func {} ({}), probability = {:.2}\n",
         entry->value, func->fullName(), probability);
  return func;
}

std::string CallTargetProfile::toString() const {
  // At some point we might want to include function names here.
  return m_profile.toString();
}

void CallTargetProfile::serialize(ProfDataSerializer& ser) const {
  for (auto const& entry : m_profile.m_entries) {
    auto const valid = entry.value != InvalidFuncId;
    auto const func = valid ? Func::fromFuncId(entry.value) : nullptr;
    write_func(ser, func);
    write_raw(ser, entry.count);
  }
  write_raw(ser, m_profile.total());
}

void CallTargetProfile::deserialize(ProfDataDeserializer& ser) {
  for (auto& entry : m_profile.m_entries) {
    auto const func = read_func(ser);
    entry.value = func ? func->getFuncId() : InvalidFuncId;
    read_raw(ser, entry.count);
  }
  read_raw(ser, m_profile.m_total);
}

///////////////////////////////////////////////////////////////////////////////

}}

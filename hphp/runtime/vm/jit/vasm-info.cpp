/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/vasm-info.h"

#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include <utility>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

/*
 * Provide a default template that matches every instruction, but which
 * is only a near match for a non-const Inst&.
 */
template <typename Inst>
std::pair<ConditionCode*,Vreg> ccUsageHelper(const Inst&) {
  return { nullptr, Vreg{} };
}

/*
 * Provide a template that only matches instructions with cc and sf
 * members, and is an exact match for a non-const Inst&.
 */
template <typename Inst>
auto ccUsageHelper(Inst& i) ->
  decltype(std::make_pair(&i.cc, Vreg(size_t(i.sf)))) {
  return { &i.cc, Vreg(size_t(i.sf)) };
}

std::pair<ConditionCode*,Vreg> ccUsage(Vinstr& inst) {
#define O(name,...) case Vinstr::name: return ccUsageHelper(inst.name##_);
  switch (inst.op) {
    VASM_OPCODES
  }
#undef O
  not_reached();
}

template<typename M>
constexpr bool useMemory() { return false; }
template<>
constexpr bool useMemory<Vptr>() { return true; }

///////////////////////////////////////////////////////////////////////////////

}

ConditionCode& getConditionCode(Vinstr& inst) {
  auto usage = ccUsage(inst);
  assertx(usage.first);
  return *usage.first;
}

Vreg getSFUseReg(const Vinstr& inst) {
  return ccUsage(const_cast<Vinstr&>(inst)).second;
}

bool touchesMemory(Vinstr::Opcode op) {
  if (op == Vinstr::lea) return false;
  if (isCall(op)) return true;
#define O(name, imms, uses, defs) \
  case Vinstr::name: { using T = name; return uses false; }
#define U(s)    useMemory<decltype(T::s)>() ||
#define UA(s)   (assertx(!useMemory<decltype(T::s)>()),false) ||
#define UH(s,h) (assertx(!useMemory<decltype(T::s)>()),false) ||
#define UW(s)   (assertx(useMemory<decltype(T::s)>()),true) ||
#define UM(s)   (assertx(useMemory<decltype(T::s)>()),true) ||
#define Un      false ||

  switch (op) {
    VASM_OPCODES
  }
  return false;

#undef Un
#undef UH
#undef UA
#undef UW
#undef UM
#undef U

#undef O
}

bool writesMemory(Vinstr::Opcode op) {
  if (isCall(op)) return true;
#define O(name, imms, uses, defs)               \
  case Vinstr::name: { using T = name; return uses false; }

#define U(s)
#define UA(s)
#define UH(s,h)
#define UW(s)    (assertx(useMemory<decltype(T::s)>()),true) ||
#define UM(s)    (assertx(useMemory<decltype(T::s)>()),true) ||
#define Un

  switch (op) {
    VASM_OPCODES
  }
  return false;

#undef Un
#undef UH
#undef UA
#undef UW
#undef UM
#undef U

#undef O
}

bool touchesMemory(const Vinstr& inst) {
  return touchesMemory(inst.op);
}

bool writesMemory(const Vinstr& inst) {
  return writesMemory(inst.op);
}


///////////////////////////////////////////////////////////////////////////////

}}

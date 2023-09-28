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

#include "hphp/runtime/vm/jit/vasm-info.h"

#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include <utility>

namespace HPHP::jit {

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
template<>
constexpr bool useMemory<Vptr8>() { return true; }
template<>
constexpr bool useMemory<Vptr16>() { return true; }
template<>
constexpr bool useMemory<Vptr32>() { return true; }
template<>
constexpr bool useMemory<Vptr64>() { return true; }
template<>
constexpr bool useMemory<Vptr128>() { return true; }

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
  if (op == Vinstr::killeffects) return true;
  if (op == Vinstr::lea) return false;
  if (isCall(op)) return true;
#define O(name, imms, uses, defs) \
  case Vinstr::name: { using T = name; return uses false; }
#define U(s)    useMemory<decltype(T::s)>() ||
#define UA(s)   useMemory<decltype(T::s)>() ||
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
  if (op == Vinstr::killeffects) return true;
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

namespace {

bool effectsImpl(const Vinstr& inst, bool pure) {
  switch (inst.op) {
    // Pure:
    case Vinstr::absdbl:
    case Vinstr::addl:
    case Vinstr::addli:
    case Vinstr::addq:
    case Vinstr::addqi:
    case Vinstr::addsd:
    case Vinstr::andb:
    case Vinstr::andbi:
    case Vinstr::andw:
    case Vinstr::andwi:
    case Vinstr::andl:
    case Vinstr::andli:
    case Vinstr::andq:
    case Vinstr::andqi64:
    case Vinstr::andqi:
    case Vinstr::btrq:
    case Vinstr::cmovb:
    case Vinstr::cmovl:
    case Vinstr::cmovq:
    case Vinstr::cmovw:
    case Vinstr::cmpb:
    case Vinstr::cmpbi:
    case Vinstr::cmpl:
    case Vinstr::cmpli:
    case Vinstr::cmpq:
    case Vinstr::cmpqi:
    case Vinstr::cmpsd:
    case Vinstr::cmpw:
    case Vinstr::cmpwi:
    case Vinstr::copy2:
    case Vinstr::copy:
    case Vinstr::copyargs:
    case Vinstr::crc32q:
    case Vinstr::csincb:
    case Vinstr::csincl:
    case Vinstr::csincq:
    case Vinstr::csincw:
    case Vinstr::cvtsi2sd:
    case Vinstr::cvttsd2siq:
    case Vinstr::decl:
    case Vinstr::decq:
    case Vinstr::divint:
    case Vinstr::divsd:
    case Vinstr::fcvtzs:
    case Vinstr::imul:
    case Vinstr::incl:
    case Vinstr::incq:
    case Vinstr::incw:
    case Vinstr::ldimmb:
    case Vinstr::ldimml:
    case Vinstr::ldimmq:
    case Vinstr::ldimmw:
    case Vinstr::ldundefq:
    case Vinstr::lea:
    case Vinstr::lead:
    case Vinstr::movb:
    case Vinstr::movl:
    case Vinstr::movsbl:
    case Vinstr::movsbq:
    case Vinstr::movslq:
    case Vinstr::movswl:
    case Vinstr::movswq:
    case Vinstr::movtdb:
    case Vinstr::movtdq:
    case Vinstr::movtqb:
    case Vinstr::movtql:
    case Vinstr::movtqw:
    case Vinstr::movw:
    case Vinstr::movzbl:
    case Vinstr::movzbq:
    case Vinstr::movzbw:
    case Vinstr::movzlq:
    case Vinstr::movzwl:
    case Vinstr::movzwq:
    case Vinstr::mulsd:
    case Vinstr::neg:
    case Vinstr::nop:
    case Vinstr::not:
    case Vinstr::notb:
    case Vinstr::orbi:
    case Vinstr::orwi:
    case Vinstr::orli:
    case Vinstr::orq:
    case Vinstr::orqi:
    case Vinstr::reload:
    case Vinstr::roundsd:
    case Vinstr::sar:
    case Vinstr::sarq:
    case Vinstr::sarqi:
    case Vinstr::setcc:
    case Vinstr::shl:
    case Vinstr::shr:
    case Vinstr::shlli:
    case Vinstr::shlq:
    case Vinstr::shlqi:
    case Vinstr::shrq:
    case Vinstr::shrli:
    case Vinstr::shrqi:
    case Vinstr::spill:
    case Vinstr::spillbi:
    case Vinstr::spillli:
    case Vinstr::spillqi:
    case Vinstr::spillundefq:
    case Vinstr::sqrtsd:
    case Vinstr::srem:
    case Vinstr::ssaalias:
    case Vinstr::subl:
    case Vinstr::subli:
    case Vinstr::subq:
    case Vinstr::subqi:
    case Vinstr::subsd:
    case Vinstr::testb:
    case Vinstr::testbi:
    case Vinstr::testl:
    case Vinstr::testli:
    case Vinstr::testq:
    case Vinstr::testqi:
    case Vinstr::testw:
    case Vinstr::testwi:
    case Vinstr::ubfmli:
    case Vinstr::ucomisd:
    case Vinstr::unpcklpd:
    case Vinstr::xorb:
    case Vinstr::xorbi:
    case Vinstr::xorw:
    case Vinstr::xorwi:
    case Vinstr::xorl:
    case Vinstr::xorq:
    case Vinstr::xorqi:
      assertx(!touchesMemory(inst));
      return pure;

    // Non-effectful but non-pure:
    case Vinstr::cloadq:
    case Vinstr::cmpbim:
    case Vinstr::cmpbm:
    case Vinstr::cmplim:
    case Vinstr::cmplm:
    case Vinstr::cmpqim:
    case Vinstr::cmpqm:
    case Vinstr::cmpwim:
    case Vinstr::cmpwm:
    case Vinstr::cvtsi2sdm:
    case Vinstr::defvmretdata:
    case Vinstr::defvmrettype:
    case Vinstr::defvmsp:
    case Vinstr::defvmfp:
    case Vinstr::leap:
    case Vinstr::load:
    case Vinstr::loadb:
    case Vinstr::loadl:
    case Vinstr::loadqd:
    case Vinstr::loadqp:
    case Vinstr::loadsbl:
    case Vinstr::loadsbq:
    case Vinstr::loadsd:
    case Vinstr::loadtqb:
    case Vinstr::loadtql:
    case Vinstr::loadups:
    case Vinstr::loadw:
    case Vinstr::loadzbl:
    case Vinstr::loadzbq:
    case Vinstr::loadzwq:
    case Vinstr::loadzlq:
    case Vinstr::mrs:
    case Vinstr::testbim:
    case Vinstr::testbm:
    case Vinstr::testlim:
    case Vinstr::testlm:
    case Vinstr::testqim:
    case Vinstr::testqm:
    case Vinstr::testwim:
    case Vinstr::testwm:
      assertx(!writesMemory(inst));
      assertx(!isCall(inst));
      return false;

    // Effectful:
    case Vinstr::addlim:
    case Vinstr::addlm:
    case Vinstr::addqim:
    case Vinstr::addqmr:
    case Vinstr::addqrm:
    case Vinstr::addwm:
    case Vinstr::andbim:
    case Vinstr::bindaddr:
    case Vinstr::ldbindaddr:
    case Vinstr::ldbindretaddr:
    case Vinstr::bindjcc:
    case Vinstr::bindjmp:
    case Vinstr::call:
    case Vinstr::callfaststub:
    case Vinstr::callm:
    case Vinstr::callphp:
    case Vinstr::callphpfe:
    case Vinstr::callphpr:
    case Vinstr::callphps:
    case Vinstr::callr:
    case Vinstr::calls:
    case Vinstr::callstub:
    case Vinstr::conjure:
    case Vinstr::conjureuse:
    case Vinstr::contenter:
    case Vinstr::cqo:
    case Vinstr::debugtrap:
    case Vinstr::declm:
    case Vinstr::decqm:
    case Vinstr::decqmlock:
    case Vinstr::decqmlocknosf:
    case Vinstr::fallback:
    case Vinstr::fallbackcc:
    case Vinstr::fallthru:
    case Vinstr::idiv:
    case Vinstr::inclm:
    case Vinstr::incqm:
    case Vinstr::incwm:
    case Vinstr::inittc:
    case Vinstr::inlineend:
    case Vinstr::inlinesideexit:
    case Vinstr::inlinestart:
    case Vinstr::jcc:
    case Vinstr::jcci:
    case Vinstr::jmp:
    case Vinstr::jmpi:
    case Vinstr::jmpm:
    case Vinstr::jmpr:
    case Vinstr::killeffects:
    case Vinstr::landingpad:
    case Vinstr::leavetc:
    case Vinstr::loadstubret:
    case Vinstr::mcprep:
    case Vinstr::msr:
    case Vinstr::nothrow:
    case Vinstr::orbim:
    case Vinstr::orlim:
    case Vinstr::orqim:
    case Vinstr::orwim:
    case Vinstr::phidef:
    case Vinstr::phijmp:
    case Vinstr::phplogue:
    case Vinstr::phpret:
    case Vinstr::pop:
    case Vinstr::popf:
    case Vinstr::popm:
    case Vinstr::popp:
    case Vinstr::poppm:
    case Vinstr::prefetch:
    case Vinstr::push:
    case Vinstr::pushf:
    case Vinstr::pushframe:
    case Vinstr::pushm:
    case Vinstr::pushp:
    case Vinstr::pushpm:
    case Vinstr::pushvmfp:
    case Vinstr::recordbasenativesp:
    case Vinstr::unrecordbasenativesp:
    case Vinstr::recordstack:
    case Vinstr::restoreripm:
    case Vinstr::restorerips:
    case Vinstr::resumetc:
    case Vinstr::ret:
    case Vinstr::saverips:
    case Vinstr::store:
    case Vinstr::storeb:
    case Vinstr::storebi:
    case Vinstr::storel:
    case Vinstr::storeli:
    case Vinstr::storeqi:
    case Vinstr::storesd:
    case Vinstr::storeups:
    case Vinstr::storew:
    case Vinstr::storewi:
    case Vinstr::stublogue:
    case Vinstr::stubret:
    case Vinstr::stubtophp:
    case Vinstr::stubunwind:
    case Vinstr::subqim:
    case Vinstr::syncpoint:
    case Vinstr::syncvmret:
    case Vinstr::syncvmrettype:
    case Vinstr::syncvmsp:
    case Vinstr::tailcallstub:
    case Vinstr::tailcallstubr:
    case Vinstr::trap:
    case Vinstr::unstublogue:
    case Vinstr::unwind:
    case Vinstr::vcall:
    case Vinstr::vinvoke:
    case Vinstr::vregrestrict:
    case Vinstr::vregunrestrict:
      return !pure;
  }
  always_assert(false);
}

}

bool effectful(const Vinstr& inst) {
  return effectsImpl(inst, false);
}

bool isPure(const Vinstr& inst) {
  return effectsImpl(inst, true);
}

}

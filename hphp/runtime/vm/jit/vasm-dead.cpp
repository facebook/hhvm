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

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include <boost/dynamic_bitset.hpp>

#include <algorithm>

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

namespace {
typedef boost::dynamic_bitset<> LiveSet;
bool effectful(Vinstr& inst) {
  switch (inst.op) {
    case Vinstr::absdbl:
    case Vinstr::addl:
    case Vinstr::addli:
    case Vinstr::addq:
    case Vinstr::addqi:
    case Vinstr::addsd:
    case Vinstr::andb:
    case Vinstr::andbi:
    case Vinstr::andl:
    case Vinstr::andli:
    case Vinstr::andq:
    case Vinstr::andqi:
    case Vinstr::andqi64:
    case Vinstr::cloadq:
    case Vinstr::cmovb:
    case Vinstr::cmovw:
    case Vinstr::cmovl:
    case Vinstr::cmovq:
    case Vinstr::cmpb:
    case Vinstr::cmpbi:
    case Vinstr::cmpbim:
    case Vinstr::cmpbm:
    case Vinstr::cmpw:
    case Vinstr::cmpwi:
    case Vinstr::cmpl:
    case Vinstr::cmpli:
    case Vinstr::cmplim:
    case Vinstr::cmplm:
    case Vinstr::cmpq:
    case Vinstr::cmpqi:
    case Vinstr::cmpqim:
    case Vinstr::cmpqm:
    case Vinstr::cmpsd:
    case Vinstr::cmpwim:
    case Vinstr::cmpwm:
    case Vinstr::copy:
    case Vinstr::copy2:
    case Vinstr::copyargs:
    case Vinstr::csincb:
    case Vinstr::csincw:
    case Vinstr::csincl:
    case Vinstr::csincq:
    case Vinstr::cvtsi2sd:
    case Vinstr::cvtsi2sdm:
    case Vinstr::cvttsd2siq:
    case Vinstr::decl:
    case Vinstr::decq:
    case Vinstr::defvmretdata:
    case Vinstr::defvmrettype:
    case Vinstr::defvmsp:
    case Vinstr::divint:
    case Vinstr::divsd:
    case Vinstr::extsb:
    case Vinstr::extsw:
    case Vinstr::extsl:
    case Vinstr::fcmpo:
    case Vinstr::fcmpu:
    case Vinstr::fctidz:
    case Vinstr::fcvtzs:
    case Vinstr::imul:
    case Vinstr::incl:
    case Vinstr::incq:
    case Vinstr::incw:
    case Vinstr::ldimmb:
    case Vinstr::ldimml:
    case Vinstr::ldimmq:
    case Vinstr::ldimmw:
    case Vinstr::lea:
    case Vinstr::lead:
    case Vinstr::leap:
    case Vinstr::load:
    case Vinstr::loadb:
    case Vinstr::loadl:
    case Vinstr::loadqd:
    case Vinstr::loadqp:
    case Vinstr::loadsd:
    case Vinstr::loadtqb:
    case Vinstr::loadtql:
    case Vinstr::loadups:
    case Vinstr::loadw:
    case Vinstr::loadzbl:
    case Vinstr::loadzbq:
    case Vinstr::loadzlq:
    case Vinstr::mflr:
    case Vinstr::movb:
    case Vinstr::movl:
    case Vinstr::movtqb:
    case Vinstr::movtdb:
    case Vinstr::movtdq:
    case Vinstr::movtql:
    case Vinstr::movtqw:
    case Vinstr::movw:
    case Vinstr::movzbw:
    case Vinstr::movzbl:
    case Vinstr::movzbq:
    case Vinstr::movzwl:
    case Vinstr::movzwq:
    case Vinstr::movzlq:
    case Vinstr::mrs:
    case Vinstr::mulsd:
    case Vinstr::neg:
    case Vinstr::nop:
    case Vinstr::not:
    case Vinstr::notb:
    case Vinstr::orq:
    case Vinstr::orqi:
    case Vinstr::roundsd:
    case Vinstr::sar:
    case Vinstr::sarq:
    case Vinstr::sarqi:
    case Vinstr::setcc:
    case Vinstr::shl:
    case Vinstr::shlli:
    case Vinstr::shlq:
    case Vinstr::shlqi:
    case Vinstr::shrli:
    case Vinstr::shrqi:
    case Vinstr::sqrtsd:
    case Vinstr::srem:
    case Vinstr::subl:
    case Vinstr::subli:
    case Vinstr::subq:
    case Vinstr::subqi:
    case Vinstr::subsd:
    case Vinstr::testb:
    case Vinstr::testbi:
    case Vinstr::testbim:
    case Vinstr::testw:
    case Vinstr::testwi:
    case Vinstr::testl:
    case Vinstr::testli:
    case Vinstr::testlim:
    case Vinstr::testq:
    case Vinstr::testqi:
    case Vinstr::testqim:
    case Vinstr::testqm:
    case Vinstr::testwim:
    case Vinstr::ucomisd:
    case Vinstr::unpcklpd:
    case Vinstr::ubfmli:
    case Vinstr::xorb:
    case Vinstr::xorbi:
    case Vinstr::xorl:
    case Vinstr::xorq:
    case Vinstr::xorqi:
      return false;

    case Vinstr::addlm:
    case Vinstr::addlim:
    case Vinstr::addqim:
    case Vinstr::andbim:
    case Vinstr::bindaddr:
    case Vinstr::bindjcc:
    case Vinstr::bindjmp:
    case Vinstr::funcguard:
    case Vinstr::call:
    case Vinstr::callarray:
    case Vinstr::callfaststub:
    case Vinstr::callm:
    case Vinstr::callphp:
    case Vinstr::callr:
    case Vinstr::calls:
    case Vinstr::callstub:
    case Vinstr::calltc:
    case Vinstr::contenter:
    case Vinstr::cqo:
    case Vinstr::debugtrap:
    case Vinstr::declm:
    case Vinstr::decqm:
    case Vinstr::decqmlock:
    case Vinstr::fallback:
    case Vinstr::fallbackcc:
    case Vinstr::fallthru:
    case Vinstr::idiv:
    case Vinstr::inclm:
    case Vinstr::incqm:
    case Vinstr::incwm:
    case Vinstr::inittc:
    case Vinstr::jcc:
    case Vinstr::jcci:
    case Vinstr::jmp:
    case Vinstr::jmpi:
    case Vinstr::jmpm:
    case Vinstr::jmpr:
    case Vinstr::landingpad:
    case Vinstr::leavetc:
    case Vinstr::loadstubret:
    case Vinstr::mcprep:
    case Vinstr::msr:
    case Vinstr::mtlr:
    case Vinstr::nothrow:
    case Vinstr::orbim:
    case Vinstr::orqim:
    case Vinstr::orwim:
    case Vinstr::phidef:
    case Vinstr::phijcc:
    case Vinstr::phijmp:
    case Vinstr::phplogue:
    case Vinstr::phpret:
    case Vinstr::pop:
    case Vinstr::popm:
    case Vinstr::popf:
    case Vinstr::popp:
    case Vinstr::poppm:
    case Vinstr::push:
    case Vinstr::pushm:
    case Vinstr::pushf:
    case Vinstr::pushp:
    case Vinstr::pushpm:
    case Vinstr::resumetc:
    case Vinstr::ret:
    case Vinstr::retransopt:
    case Vinstr::store:
    case Vinstr::storeb:
    case Vinstr::storebi:
    case Vinstr::storeups:
    case Vinstr::storel:
    case Vinstr::storeli:
    case Vinstr::storeqi:
    case Vinstr::storesd:
    case Vinstr::storew:
    case Vinstr::storewi:
    case Vinstr::stublogue:
    case Vinstr::stubret:
    case Vinstr::stubtophp:
    case Vinstr::stubunwind:
    case Vinstr::syncpoint:
    case Vinstr::syncvmret:
    case Vinstr::syncvmsp:
    case Vinstr::tailcallphp:
    case Vinstr::tailcallstub:
    case Vinstr::ud2:
    case Vinstr::unwind:
    case Vinstr::vcall:
    case Vinstr::vcallarray:
    case Vinstr::vinvoke:
    case Vinstr::vregrestrict:
    case Vinstr::vregunrestrict:
    case Vinstr::conjure:
    case Vinstr::conjureuse:
      return true;
  }
  always_assert(false);
}
}

// Remove dead instructions by doing a traditional liveness analysis.
// instructions that mutate memory, physical registers, or status flags
// are considered useful. All branches are considered useful.
//
// Given SSA, there's a faster sparse version of this algorithm that marks
// useful instructions in one pass, then transitively marks pure instructions
// that define inputs to useful instructions. However it requires a mapping
// from vreg numbers to the instruction that defines them, and a way to address
// individual instructions.
//
// We could remove useless branches by computing the post-dominator tree and
// RDF(b) for each block; then a branch is only useful if it controls whether
// or not a useful block executes, and useless branches can be forwarded to
// the nearest useful post-dominator.
void removeDeadCode(Vunit& unit) {
  Timer timer(Timer::vasm_dce);
  auto blocks = sortBlocks(unit);
  jit::vector<LiveSet> livein(unit.blocks.size());
  LiveSet live(unit.next_vr);

  auto pass = [&](bool mutate) {
    bool changed = false;
    for (auto blockIt = blocks.end(); blockIt != blocks.begin();) {
      auto b = *--blockIt;
      auto& block = unit.blocks[b];
      live.reset();
      for (auto s : succs(block)) {
        if (!livein[s].empty()) {
          live |= livein[s];
        }
      }
      for (auto i = block.code.end(); i != block.code.begin();) {
        auto& inst = *--i;
        auto useful = effectful(inst);
        visitDefs(unit, inst, [&](Vreg r) {
          if (r.isPhys() || live.test(r)) {
            useful = true;
            live.reset(r);
          }
        });
        if (useful) {
          visitUses(unit, inst, [&](Vreg r) {
            live.set(r);
          });
        } else if (mutate) {
          inst = nop{};
          changed = true;
        }
      }
      if (mutate) {
        assertx(live == livein[b]);
      } else {
        if (live != livein[b]) {
          livein[b] = live;
          changed = true;
        }
      }
    }
    return changed;
  };

  // analyze until livein reaches a fixed point
  while (pass(false)) {}
  auto const changed = pass(true);
  removeTrivialNops(unit);
  if (changed) {
    printUnit(kVasmDCELevel, "after vasm-dead", unit);
  }
}

/*
 * A very simple dead code elimination pass that just removes trivial nop
 * instructions.  We run this before any other passes because it allows
 * code-gen to create things like self-copies or self-lea's without affect on
 * optimizations downstream.  (In particular early passes like optimizeExits
 * that are looking for specific vasm sequences inside of a block.)
 */
void removeTrivialNops(Vunit& unit) {
  for (auto& b : unit.blocks) {
    b.code.erase(
      std::remove_if(
        begin(b.code), end(b.code),
        is_trivial_nop
      ),
      end(b.code)
    );
  }
}

}}

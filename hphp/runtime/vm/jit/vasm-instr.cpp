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

#include "hphp/runtime/vm/jit/vasm-instr.h"

namespace HPHP::jit {
///////////////////////////////////////////////////////////////////////////////

#define O(name, ...)  \
  static_assert(sizeof(name) <= 48, "vasm struct " #name " is too big");
VASM_OPCODES
#undef O
static_assert(sizeof(Vinstr) <= 64, "Vinstr should be <= 64 bytes");

const char* vinst_names[] = {
#define O(name, imms, uses, defs) #name,
  VASM_OPCODES
#undef O
};

///////////////////////////////////////////////////////////////////////////////

bool isBlockEnd(const Vinstr& inst) {
  switch (inst.op) {
    // service request-y things
    case Vinstr::bindjmp:
    case Vinstr::fallback:
    // control flow
    case Vinstr::jcc:
    case Vinstr::jmp:
    case Vinstr::jmpr:
    case Vinstr::jmpm:
    case Vinstr::jmpi:
    case Vinstr::phijmp:
    case Vinstr::interceptjcc:
    // terminal calls
    case Vinstr::tailcallstub:
    case Vinstr::tailcallstubr:
    case Vinstr::resumetc:
    // exception edges
    case Vinstr::unwind:
    case Vinstr::vinvoke:
    case Vinstr::contenter:
    // terminal
    case Vinstr::trap:
    case Vinstr::ret:
    case Vinstr::stubret:
    case Vinstr::phpret:
    case Vinstr::leavetc:
    case Vinstr::fallthru:
      return true;
    default:
      return false;
  }
}

bool isCall(Vinstr::Opcode op) {
  switch (op) {
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
    case Vinstr::inlinesideexit:
    case Vinstr::tailcallstub:
    case Vinstr::tailcallstubr:
    case Vinstr::vcall:
    case Vinstr::vinvoke:
      return true;
    default:
      return false;
  }
}

Width width(Vinstr::Opcode op) {
  switch (op) {
    // service requests
    case Vinstr::bindjmp:
    case Vinstr::bindjcc:
    case Vinstr::bindaddr:
    case Vinstr::ldbindaddr:
    case Vinstr::ldbindretaddr:
    case Vinstr::fallback:
    case Vinstr::fallbackcc:
    // vasm intrinsics
    case Vinstr::conjure:
    case Vinstr::conjureuse:
    case Vinstr::copy:
    case Vinstr::copy2:
    case Vinstr::copyargs:
    case Vinstr::debugtrap:
    case Vinstr::fallthru:
    case Vinstr::killeffects:
    case Vinstr::ldimmb:
    case Vinstr::ldimmw:
    case Vinstr::ldimml:
    case Vinstr::ldimmq:
    case Vinstr::ldundefq:
    case Vinstr::load:
    case Vinstr::store:
    case Vinstr::mcprep:
    case Vinstr::phidef:
    case Vinstr::phijmp:
    case Vinstr::inlinestart:
    case Vinstr::inlineend:
    case Vinstr::pushframe:
    case Vinstr::recordstack:
    case Vinstr::recordbasenativesp:
    case Vinstr::unrecordbasenativesp:
    case Vinstr::spill:
    case Vinstr::spillbi:
    case Vinstr::spillli:
    case Vinstr::spillqi:
    case Vinstr::spillundefq:
    case Vinstr::reload:
    case Vinstr::ssaalias:
    // native function abi
    case Vinstr::vcall:
    case Vinstr::vinvoke:
    case Vinstr::call:
    case Vinstr::callm:
    case Vinstr::callr:
    case Vinstr::calls:
    case Vinstr::ret:
    // stub function abi
    case Vinstr::stublogue:
    case Vinstr::unstublogue:
    case Vinstr::stubret:
    case Vinstr::callstub:
    case Vinstr::callfaststub:
    case Vinstr::tailcallstub:
    case Vinstr::tailcallstubr:
    case Vinstr::stubunwind:
    // php function abi
    case Vinstr::defvmsp:
    case Vinstr::defvmfp:
    case Vinstr::pushvmfp:
    case Vinstr::syncvmsp:
    case Vinstr::defvmretdata:
    case Vinstr::defvmrettype:
    case Vinstr::syncvmret:
    case Vinstr::syncvmrettype:
    case Vinstr::phplogue:
    case Vinstr::stubtophp:
    case Vinstr::loadstubret:
    case Vinstr::restoreripm:
    case Vinstr::restorerips:
    case Vinstr::saverips:
    case Vinstr::phpret:
    case Vinstr::callphp:
    case Vinstr::callphpfe:
    case Vinstr::callphpr:
    case Vinstr::callphps:
    case Vinstr::contenter:
    case Vinstr::inlinesideexit:
    // vm entry abi
    case Vinstr::resumetc:
    case Vinstr::inittc:
    case Vinstr::leavetc:
    // exception intrinsics
    case Vinstr::landingpad:
    case Vinstr::nothrow:
    case Vinstr::syncpoint:
    case Vinstr::unwind:
    // nop and trap
    case Vinstr::nop:
    case Vinstr::trap:
    // restrict/unrestrict new virtuals
    case Vinstr::vregrestrict:
    case Vinstr::vregunrestrict:
    // sign/zero-extending/truncating copies
    case Vinstr::movsbl:
    case Vinstr::movswl:
    case Vinstr::movsbq:
    case Vinstr::movswq:
    case Vinstr::movslq:
    case Vinstr::movzbw:
    case Vinstr::movzbl:
    case Vinstr::movzbq:
    case Vinstr::movzwl:
    case Vinstr::movzwq:
    case Vinstr::movzlq:
    case Vinstr::movtqb:
    case Vinstr::movtdb:
    case Vinstr::movtdq:
    case Vinstr::movtql:
    case Vinstr::movtqw:
    // branches
    case Vinstr::jcc:
    case Vinstr::jcci:
    case Vinstr::jmp:
    case Vinstr::jmpr:
    case Vinstr::jmpm:
    case Vinstr::jmpi:
    case Vinstr::interceptjcc:
    // push/pop
    case Vinstr::pop:
    case Vinstr::popf:
    case Vinstr::popm:
    case Vinstr::popp:
    case Vinstr::poppm:
    case Vinstr::push:
    case Vinstr::pushf:
    case Vinstr::pushm:
    case Vinstr::pushp:
    case Vinstr::pushpm:
    // floating-point conversions
    case Vinstr::cvttsd2siq:
    case Vinstr::cvtsi2sd:
    case Vinstr::cvtsi2sdm:
    case Vinstr::unpcklpd:
    // x64 instructions
    case Vinstr::cqo:
    case Vinstr::idiv:
    case Vinstr::sarq:
    case Vinstr::shlq:
    case Vinstr::shrq:
    // arm instructions
    case Vinstr::fcvtzs:
    case Vinstr::mrs:
    case Vinstr::msr:
      return Width::AnyNF;

    case Vinstr::andb:
    case Vinstr::andbi:
    case Vinstr::andbim:
    case Vinstr::notb:
    case Vinstr::orbi:
    case Vinstr::orbim:
    case Vinstr::xorb:
    case Vinstr::xorbi:
    case Vinstr::cmpb:
    case Vinstr::cmpbi:
    case Vinstr::cmpbim:
    case Vinstr::cmpbm:
    case Vinstr::testb:
    case Vinstr::testbi:
    case Vinstr::testbim:
    case Vinstr::testbm:
    case Vinstr::cmovb:
    case Vinstr::csincb:
    case Vinstr::setcc:
    case Vinstr::movb:
    case Vinstr::loadb:
    case Vinstr::loadtqb:
    case Vinstr::storeb:
    case Vinstr::storebi:
      return Width::Byte;

    case Vinstr::addwm:
    case Vinstr::andw:
    case Vinstr::andwi:
    case Vinstr::incw:
    case Vinstr::incwm:
    case Vinstr::orwim:
    case Vinstr::cmovw:
    case Vinstr::csincw:
    case Vinstr::cmpw:
    case Vinstr::cmpwi:
    case Vinstr::cmpwim:
    case Vinstr::cmpwm:
    case Vinstr::testw:
    case Vinstr::testwi:
    case Vinstr::testwim:
    case Vinstr::testwm:
    case Vinstr::loadw:
    case Vinstr::movw:
    case Vinstr::storew:
    case Vinstr::storewi:
    case Vinstr::xorw:
    case Vinstr::xorwi:
      return Width::Word;

    case Vinstr::addl:
    case Vinstr::addli:
    case Vinstr::addlm:
    case Vinstr::addlim:
    case Vinstr::andl:
    case Vinstr::andli:
    case Vinstr::decl:
    case Vinstr::declm:
    case Vinstr::incl:
    case Vinstr::inclm:
    case Vinstr::shlli:
    case Vinstr::shrli:
    case Vinstr::subl:
    case Vinstr::subli:
    case Vinstr::xorl:
    case Vinstr::orlim:
    case Vinstr::cmovl:
    case Vinstr::csincl:
    case Vinstr::cmpl:
    case Vinstr::cmpli:
    case Vinstr::cmplm:
    case Vinstr::cmplim:
    case Vinstr::testl:
    case Vinstr::testli:
    case Vinstr::testlim:
    case Vinstr::testlm:
    case Vinstr::movl:
    case Vinstr::loadl:
    case Vinstr::loadzbl:
    case Vinstr::loadsbl:
    case Vinstr::loadtql:
    case Vinstr::storel:
    case Vinstr::storeli:
    case Vinstr::ubfmli:
      return Width::Long;

    case Vinstr::addq:
    case Vinstr::addqi:
    case Vinstr::addqmr:
    case Vinstr::addqrm:
    case Vinstr::addqim:
    case Vinstr::andq:
    case Vinstr::andqi:
    case Vinstr::andqi64:
    case Vinstr::btrq:
    case Vinstr::decq:
    case Vinstr::decqm:
    case Vinstr::decqmlock:
    case Vinstr::decqmlocknosf:
    case Vinstr::incq:
    case Vinstr::incqm:
    case Vinstr::imul:
    case Vinstr::divint:
    case Vinstr::srem:
    case Vinstr::neg:
    case Vinstr::not:
    case Vinstr::orwi:
    case Vinstr::orli:
    case Vinstr::orq:
    case Vinstr::orqi:
    case Vinstr::orqim:
    case Vinstr::sar:
    case Vinstr::shl:
    case Vinstr::shr:
    case Vinstr::sarqi:
    case Vinstr::shlqi:
    case Vinstr::shrqi:
    case Vinstr::subq:
    case Vinstr::subqi:
    case Vinstr::subqim:
    case Vinstr::xorq:
    case Vinstr::xorqi:
    case Vinstr::cmpq:
    case Vinstr::cmpqi:
    case Vinstr::cmpqm:
    case Vinstr::cmpqim:
    case Vinstr::testq:
    case Vinstr::testqi:
    case Vinstr::testqm:
    case Vinstr::testqim:
    case Vinstr::cloadq:
    case Vinstr::cmovq:
    case Vinstr::csincq:
    case Vinstr::lea:
    case Vinstr::leap:
    case Vinstr::lead:
    case Vinstr::loadqp:
    case Vinstr::loadqd:
    case Vinstr::loadzbq:
    case Vinstr::loadzwq:
    case Vinstr::loadzlq:
    case Vinstr::loadsbq:
    case Vinstr::storeqi:
    case Vinstr::addsd:
    case Vinstr::subsd:
    case Vinstr::cmpsd:
    case Vinstr::ucomisd:
    case Vinstr::loadsd:
    case Vinstr::storesd:
    case Vinstr::absdbl:
    case Vinstr::divsd:
    case Vinstr::mulsd:
    case Vinstr::roundsd:
    case Vinstr::sqrtsd:
    case Vinstr::crc32q:
    case Vinstr::prefetch:
      return Width::Quad;

    case Vinstr::loadups:
    case Vinstr::storeups:
      return Width::Octa;
  }
  not_reached();
}

bool instrHasIndirectFixup(const Vinstr& inst) {
  switch (inst.op) {
    case Vinstr::vcall:
      return inst.vcall_.fixup.isIndirect();
    case Vinstr::vinvoke:
      return inst.vinvoke_.fixup.isIndirect();
    case Vinstr::syncpoint:
      return inst.syncpoint_.fix.isIndirect();
    default:
      return false;
  }
  not_reached();
}

void updateIndirectFixupBySpill(Vinstr& inst, size_t spillSize) {
  assertx(instrHasIndirectFixup(inst));
  auto const update = [&](Fixup& f) {
    assertx(f.isIndirect());
    f.adjustRipOffset(spillSize);
  };
  switch (inst.op) {
    case Vinstr::vcall:
      update(inst.vcall_.fixup);
      return;
    case Vinstr::vinvoke:
      update(inst.vinvoke_.fixup);
      return;
    case Vinstr::syncpoint:
      update(inst.syncpoint_.fix);
      return;
    default:
      always_assert(false);
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
}

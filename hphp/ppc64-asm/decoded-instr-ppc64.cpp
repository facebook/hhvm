/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | (c) Copyright IBM Corporation 2015-2016                              |
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

#include "hphp/ppc64-asm/decoded-instr-ppc64.h"

#include <folly/Format.h>

#include "hphp/ppc64-asm/decoder-ppc64.h"
#include "hphp/ppc64-asm/asm-ppc64.h"

#include "hphp/util/safe-cast.h"

namespace ppc64_asm {

size_t DecodedInstruction::size() const {
  not_implemented();
  return size_t{0};
}

int32_t DecodedInstruction::offset() const {
  return Decoder::GetDecoder().decode(m_ip)->offset();
}

bool DecodedInstruction::isNop() const {
  return Decoder::GetDecoder().decode(m_ip)->isNop();
}

bool DecodedInstruction::isBranch(bool allowCond /* = true */) const {
  // skip the preparation instructions that are not actually the branch.
  auto branch_instr = m_ip + Assembler::kJccLen - instr_size_in_bytes;
  return Decoder::GetDecoder().decode(branch_instr)->isBranch(allowCond);
}

bool DecodedInstruction::isCall() const {
  // skip the preparation instructions that are not actually the branch.
  auto branch_instr = m_ip + Assembler::kCallLen - instr_size_in_bytes;
  return Decoder::GetDecoder().decode(branch_instr)->isBranch(false);
}

bool DecodedInstruction::isJmp() const {
  // if it's conditional branch, it's not a jmp
  return isBranch(false);
}

bool DecodedInstruction::isSpOffsetInstr() const {
  return Decoder::GetDecoder().decode(m_ip)->isSpOffsetInstr();
}

bool DecodedInstruction::isClearSignBit() const {
  return Decoder::GetDecoder().decode(m_ip)->isClearSignBit();
}

HPHP::jit::ConditionCode DecodedInstruction::jccCondCode() const {
  not_implemented();
  return HPHP::jit::CC_None;
}

}

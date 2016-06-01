/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | (c) Copyright IBM Corporation 2016                                   |
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

#ifndef incl_HPHP_PPC64_ASM_DECODED_INSTR_PPC64_H_
#define incl_HPHP_PPC64_ASM_DECODED_INSTR_PPC64_H_

#include <folly/Format.h>

#include "hphp/util/asm-x64.h"

namespace ppc64_asm {

struct DecodedInstruction {
  explicit DecodedInstruction(uint8_t* ip) : m_ip(ip) { }

  size_t size() const;
  int32_t offset() const;
  bool isNop() const;
  bool isBranch(bool allowCond = true) const;
  bool isCall() const;
  bool isJmp() const;
  bool isSpOffsetInstr() const;
  bool isClearSignBit() const;
  HPHP::jit::ConditionCode jccCondCode() const;

private:
  uint8_t* m_ip;
};

}

#endif

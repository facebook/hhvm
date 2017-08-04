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

#include "hphp/ppc64-asm/decoder-ppc64.h"
#include "hphp/ppc64-asm/isa-ppc64.h"

namespace ppc64_asm {

struct DecInfoOffset {
  DecInfoOffset()
    : m_di(Decoder::GetDecoder().getInvalid())
    , m_offset(-1) {}

  DecoderInfo m_di;
  int8_t m_offset;
};

struct DecodedInstruction {
  explicit DecodedInstruction(PPC64Instr* ip, uint8_t max_size = 0)
    : m_ip(reinterpret_cast<uint8_t*>(ip))
    , m_imm(0)
    , m_dinfo(Decoder::GetDecoder().getInvalid())
    , m_size(instr_size_in_bytes)
    , m_max_size(max_size)
  {
    decode();
  }
  // 0 as @max_size means unlimited size
  explicit DecodedInstruction(uint8_t* ip, uint8_t max_size = 0)
    : m_ip(ip)
    , m_imm(0)
    , m_dinfo(Decoder::GetDecoder().getInvalid())
    , m_size(instr_size_in_bytes)
    , m_max_size(max_size)
  {
    decode();
  }

  DecodedInstruction() = delete;

  size_t size() const           { return size_t{m_size}; }

  // True if a Far branch can be changed to a Near branch
  bool couldBeNearBranch();

  // Conditional branch: up to 16bits.
  // Unconditional branch: up to 26bits (b instruction)
  // (aka Near branch)
  bool isNearBranch(AllowCond ac = AllowCond::Any) const;
  uint8_t* nearBranchTarget() const;
  bool setNearBranchTarget(uint8_t* target);

  bool isImmediate() const;
  int64_t immediate() const     { return m_imm; }
  bool setImmediate(int64_t value);

  // The following function transform branches to:
  // shrink : branch by offset up to 16bits. Return true if success.
  // wide   : branch by absolute address.
  bool shrinkBranch();
  void widenBranch(uint8_t* target);
  int32_t offsetDS() const      { return m_dinfo.offsetDS(); }
  int16_t offsetD() const       { return m_dinfo.offsetD(); }
  bool isLd(bool toc) const     { return m_dinfo.isLd(toc); }
  bool isLwz(bool toc) const    { return m_dinfo.isLwz(toc); }
  bool isAddis(bool toc) const  { return m_dinfo.isAddis(toc); }
  // if it's conditional branch, it's not a jmp
  uint8_t* ip() const           { return m_ip; }
  int32_t offset() const        { return m_dinfo.offset(); }
  bool isException() const      { return m_dinfo.isException(); }
  bool isNop() const            { return m_dinfo.isNop(); }
  // if it's conditional branch, it's not a jmp
  bool isJmp() const            { return isBranch(AllowCond::OnlyUncond); }
  bool isSpOffsetInstr() const  { return m_dinfo.isSpOffsetInstr(); }
  bool isClearSignBit() const   { return m_dinfo.isClearSignBit(); }

  // True if it's Near or Far type.
  bool isBranch(AllowCond ac = AllowCond::Any) const;

  bool isFarBranch(AllowCond ac = AllowCond::Any) const;
  DecoderInfo getFarBranch(AllowCond ac = AllowCond::Any) const;
  DecInfoOffset getFarBranchLength(AllowCond ac = AllowCond::Any) const;
  uint8_t* farBranchTarget() const { return (uint8_t*)m_imm; }
  bool setFarBranchTarget(uint8_t* target, bool smashable);

  bool isCall() const;

  // Retrieve the register used by li64 instruction
  HPHP::jit::Reg64 getLi64Reg() const;

  // Retrieve the register used by limmediate instruction
  HPHP::jit::Reg64 getLimmediateReg() const;

  // Retrieve the register used by li32 instruction
  HPHP::jit::Reg64 getLi32Reg() const { return getLi64Reg(); }

  // Check if is loading data from TOC
  bool isLoadingTOC() const;

  // Return the TOC addres of the immediate
  uint64_t* decodeTOCAddress() const;

  // Return the TOC offset of the immediate
  int64_t decodeTOCOffset() const;

  // Check if immediate is smashable (must not have reference).
  bool isSmashable(uint64_t imm) const;

private:
  // Initialize m_dinfo and m_size up to m_max_size
  void decode();
  uint8_t decodeImm();

  // True @diff fits to a Near branch from m_ip
  bool fitsOnNearBranch(ptrdiff_t diff, bool uncond) const;

  // Check if m_ip points to the beginning of a li64 instruction (relaxed
  // constraint, only checks a part of it)
  bool isLi64Possible() const;
  // Check if m_ip points to the beginning of a limmediate instruction (relaxed
  // constraint, only checks a part of it)
  bool isLimmediatePossible() const;

  // Calculate the TOC index
  int64_t calcIndex (int16_t indexBigTOC, int16_t indexTOC) const;

  uint8_t* m_ip;
  int64_t m_imm;
  DecoderInfo m_dinfo;
  uint8_t m_size;
  uint8_t m_max_size;
};

}

#endif

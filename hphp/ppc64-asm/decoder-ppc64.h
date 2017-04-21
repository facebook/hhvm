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

#ifndef incl_PPC64_ASM_DECODER_H_
#define incl_PPC64_ASM_DECODER_H_

#include <cstdint>
#include <string>
#include <deque>
#include <boost/noncopyable.hpp>

#include "hphp/ppc64-asm/isa-ppc64.h"

#include "hphp/util/assertions.h"
#include "hphp/util/portability.h"

namespace ppc64_asm {

// Mask to extract the instruction opcode
const PPC64Instr kOpcodeMask = 0xfc000000;

// Mask to remove the instruction opcode
const PPC64Instr kOperandMask = 0x03ffffff;

// decoder size is the next prime number from the table size
// decoder table is a hash table and this avoids some collisions, it's not
// the better way to do this but it's simple.
const size_t kDecoderSize = 211;

// Instruction type decoder masks
// The masks are sorted by number of bits needed to decode the instruction
// from more to less bits. Notice that some of those masks overlap instruction
// type so the table must be ordered in a way that return the
// correct instruction because the decoder will return the first exact match.

// TODO(rcardoso): Check those mask and create a better format so we can
// change those names to something more readable like: kXOFormMask, kXFXForm...

// EVX-FORM or EVS-FORM or VX-FORM [OP:6 XO:11]
const PPC64Instr kDecoderMask1  = (0x3F << 26) | (0x7FF);
// XFX-FORM [OP:6 [0x0 | 0x1] XO:10]
const PPC64Instr kDecoderMask2  = (0x3F << 26) | (0x3FF << 1) | (0x1 << 20);
// X-FORM or XL-FORM or XX1-FORM or X22-FORM or XFL-FORM
// [OP:6 XO:10 [EH:1|Rc:1|LK:1]]
const PPC64Instr kDecoderMask3  = (0x3F << 26) | (0x3FF << 1) | (0x1);
// VC-FORM [OP:6 Rc:1 XO:10]
const PPC64Instr kDecoderMask4  = (0x3F << 26) | (0x3FF)      | (0x1 << 10);
// XX1-FORM or XFX-FORM [OP:6 XO:10]
const PPC64Instr kDecoderMask5  = (0x3F << 26) | (0x3FF << 1);
// Z22-FORM or XS-FORM [OP:6 XO:10 Rc:1]
const PPC64Instr kDecoderMask6  = (0x3F << 26) | (0x1FF << 1) | (0x1);
// XX2-FORM [OP:6 XO:9]
const PPC64Instr kDecoderMask7  = (0x3F << 26) | (0x1FF << 2);
// Z23-FORM or XO-FORM [OP:6 XO:9 Rc:1]
const PPC64Instr kDecoderMask8  = (0x3F << 26) | (0xFF << 1)  | (0x1);
// TODO(rcardoso): check if this mask is correct
// XX3-FORM [OP:6 XO:8]
const PPC64Instr kDecoderMask9  = (0x3F << 26) | (0xFF << 3);
// VA-FORM or A-FORM [OP:6 [XO:8 |XO:8 Rc:1]]
const PPC64Instr kDecoderMask10 = (0x3F << 26) | (0x3F);
// M-FORM [OP:6 ME:5 Rc:1]
const PPC64Instr kDecoderMask11 = (0x3F << 26) | (0x1F << 2)  | (0x1);
// MDS-FORM [OP:6 XO:5 Rc:1 ]
const PPC64Instr kDecoderMask12 = (0x3F << 26) | (0x1F << 1);
// XX3-FORM [OP:6 XO1:1 XO2:4]
const PPC64Instr kDecoderMask13 = (0x3F << 26) | (0xF << 3)   | (0x1 << 10);
// MDS FORM [OP:6 XO:4 Rc:1]
const PPC64Instr kDecoderMask14 = (0x3F << 26) | (0xF << 1)   | (0x1);
// MD FORM [OP:6 XO:2 Rc:1]
const PPC64Instr kDecoderMask15 = (0x3F << 26) | (0x7 << 2)   | (0x1);
// XX4-FORM [OP:6 XO:2]
const PPC64Instr kDecoderMask16 = (0x3F << 26) | (0x3 << 4);
// I-FORM or B-FORM or DS-FORM [OP:6 [AA:1 LK:1 | XO:2]]
const PPC64Instr kDecoderMask17 = (0x3F << 26) | (0x3);
// TODO(rcardoso): maybe useless
// (?????) [OP:6 XO:2]
const PPC64Instr kDecoderMask18 = (0x3F << 26) | (0x1 << 2)   | (0x1 << 1);
 // SC-FORM [OP:6 0x1:1]
const PPC64Instr kDecoderMask19 = (0x3F << 26) | (0x1 << 1);
// M-FORM [OP:6 Rc:1]
const PPC64Instr kDecoderMask20 = (0x3F << 26) | (0x1);
// DQ-FORM or D-FORM [OP:6]
const PPC64Instr kDecoderMask21 = (0x3F << 26);

// Decoder List
const PPC64Instr DecoderList[] = {
  kDecoderMask1, kDecoderMask2, kDecoderMask3, kDecoderMask4,
  kDecoderMask5, kDecoderMask6, kDecoderMask7, kDecoderMask8,
  kDecoderMask9, kDecoderMask10, kDecoderMask11, kDecoderMask12,
  kDecoderMask13, kDecoderMask14, kDecoderMask15, kDecoderMask16,
  kDecoderMask17, kDecoderMask18, kDecoderMask19, kDecoderMask20,
  kDecoderMask21
};

/*
 * Defines operands mask and type for a instruction. The flags can be
 * used to encode information about a operand like kind of register or
 * if a immediate will be print as a signed or unsigned value.
*/
struct Operands {
  PPC64Instr m_mask;
  PPC64Instr m_flags;

  Operands(PPC64Instr mask, PPC64Instr flags)
  : m_mask(mask)
  , m_flags(flags)
  {}
  Operands()
  : m_mask(0)
  , m_flags(0)
  {}

  // calculates operand mask shift factor
  int operandShift() {
    int s = 32;
    if (m_mask) {
      PPC64Instr n = m_mask;
      n = (n ^ (n - 1)) >> 1;
      for (s = 0; n; s++) {
         n >>=1;
      }
    }
    return s;
  }
};

// Instructions information.
// First element is the fallback to invalid opcodes and they are sorted based
// on its opcode.
//
// All the DecoderInfo identifiers are macros defined in decoder-ppc64.cpp.
#define PPC64_OPCODES                                                         \
DE(invalid,       0x0,    Form::kInvalid, invalid,     { UN })                \
DE(cmpli,         0x28000000, Form::kD,   cmpli,       { BF, L, RA, UI })     \
DE(cmpi,          0x2C000000, Form::kD,   cmpi,        { BF, L, RA, SI })     \
DE(addicdot,      0x34000000, Form::kD,   addic.,      { RT, RA, SI })        \
DE(addi,          0x38000000, Form::kD,   addi,        { RT, RA0, SI })       \
DE(addis,         0x3C000000, Form::kD,   addis,     { RT, RA0, SISIGNOPT })  \
DE(bc,            0x40000000, Form::kB,   bc,          { BO, BI, BD })        \
DE(bcl,           0x40000001, Form::kB,   bcl,         { BO, BI, BD })        \
DE(bca,           0x40000002, Form::kB,   bca,         { BO, BI, BDA })       \
DE(bcla,          0x40000003, Form::kB,   bcla,        { BO, BI, BDA })       \
DE(b,             0x48000000, Form::kI,   b,           { LI })                \
DE(bl,            0x48000001, Form::kI,   bl,          { LI })                \
DE(ba,            0x48000002, Form::kI,   ba,          { LIA })               \
DE(bla,           0x48000003, Form::kI,   bla,         { LIA })               \
DE(bclr,          0x4C000020, Form::kXL,  bclr,        { BO, BI, BH })        \
DE(bclrl,         0x4C000021, Form::kXL,  bclrl,       { BO, BI, BH })        \
DE(bcctr,         0x4C000420, Form::kXL,  bcctr,       { BO, BI, BH })        \
DE(bcctrl,        0x4C000421, Form::kXL,  bcctrl,      { BO, BI, BH })        \
DE(bctar,         0x4C000460, Form::kX,   bctar,       { BO, BI, BH })        \
DE(bctarl,        0x4C000461, Form::kX,   bctarl,      { BO, BI, BH })        \
DE(rlwimidot,     0x50000001, Form::kM,   rlwimi.,     { RA,RS,SH,MBE,ME })   \
DE(rlwinm,        0x54000000, Form::kM,   rlwinm,      { RA,RS,SH,MBE,ME })   \
DE(rlwinmdot,     0x54000001, Form::kM,   rlwinm.,     { RA,RS,SH,MBE,ME })   \
DE(rlwnmdot,      0x5C000001, Form::kM,   rlwnm.,      { RA,RS,RB,MBE,ME })   \
DE(ori,           0x60000000, Form::kD,   ori,         { RA, RS, UI })        \
DE(oris,          0x64000000, Form::kD,   oris,        { RA, RS, UI })        \
DE(andidot,       0x70000000, Form::kD,   andi.,       { RA, RS, UI })        \
DE(andisdot,      0x74000000, Form::kD,   andis.,      { RA, RS, UI })        \
DE(rldicl,        0x78000000, Form::kMD,  rldicl,      { RA, RS, SH6, MB6 })  \
DE(rldicldot,     0x78000001, Form::kMD,  rldicl.,     { RA, RS, SH6, MB6 })  \
DE(rldicr,        0x78000004, Form::kMD,  rldicr,      { RA, RS, SH6, ME6 })  \
DE(rldicrdot,     0x78000005, Form::kMD,  rldicr.,     { RA, RS, SH6, ME6 })  \
DE(rldicdot,      0x78000009, Form::kMD,  rldic.,      { RA, RS, SH6, MB6 })  \
DE(rldimidot,     0x7800000D, Form::kMD,  rldimi.,     { RA, RS, SH6, MB6 })  \
DE(rldcldot,      0x78000011, Form::kMDS, rldcl.,      { RA, RS, RB, MB6 })   \
DE(rldcrdot,      0x78000013, Form::kMDS, rldcr.,      { RA, RS, RB, ME6 })   \
DE(cmp,           0x7C000000, Form::kX,   cmp,         { BF, L, RA, RB })     \
DE(tw,            0x7C000008, Form::kX,   tw,          { TO, RA, RB })        \
DE(subfcdot,      0x7C000011, Form::kXO,  subfc.,      { RT, RA, RB })        \
DE(mulhdudot,     0x7C000013, Form::kXO,  mulhdu.,     { RT, RA, RB })        \
DE(addcdot,       0x7C000015, Form::kXO,  addc.,       { RT, RA, RB })        \
DE(mulhwudot,     0x7C000017, Form::kXO,  mulhwu.,     { RT, RA, RB })        \
DE(isel,          0x7C00001E, Form::kA,   isel,        { RT, RA, RB, CRB })   \
DE(mfcr,          0x7C000026, Form::kXFX, mfcr,        { RT })                \
DE(ldx,           0x7C00002A, Form::kX,   ldx,         { RT, RA0, RB })       \
DE(lwzx,          0x7C00002E, Form::kX,   lwzx,        { RT, RA0, RB })       \
DE(slwdot,        0x7C000031, Form::kX,   slw.,        { RA, RS, RB })        \
DE(cntlzwdot,     0x7C000035, Form::kX,   cntlzw.,     { RA, RS })            \
DE(sld,           0x7C000036, Form::kX,   sld,         { RA, RS, RB })        \
DE(slddot,        0x7C000037, Form::kX,   sld.,        { RA, RS, RB })        \
DE(and,           0x7C000038, Form::kX,   and,         { RA, RS, RB })        \
DE(anddot,        0x7C000039, Form::kX,   and.,        { RA, RS, RB })        \
DE(cmpl,          0x7C000040, Form::kX,   cmpl,        { BF, L, RA, RB })     \
DE(subf,          0x7C000050, Form::kXO,  subf,        { RT, RA, RB })        \
DE(subfdot,       0x7C000051, Form::kXO,  subf.,       { RT, RA, RB })        \
DE(mfvsrd,        0x7C000066, Form::kXX1, mfvsrd,      { RA, XS })            \
DE(cntlzddot,     0x7C000075, Form::kX,   cntlzd.,     { RA, RS })            \
DE(andcdot,       0x7C000079, Form::kX,   andc.,       { RA, RS, RB })        \
DE(td,            0x7C000088, Form::kX,   td,          { TO, RA, RB })        \
DE(mulhddot,      0x7C000093, Form::kXO,  mulhd.,      { RT, RA, RB })        \
DE(mulhwdot,      0x7C000097, Form::kXO,  mulhw.,      { RT, RA, RB })        \
DE(mfmsr,         0x7C0000A6, Form::kX,   mfmsr,       { RT })                \
DE(ldarx,         0x7C0000A8, Form::kX,   ldarx,       { RT, RA0, RB, EH })   \
DE(lbzx,          0x7C0000AE, Form::kX,   lbzx,        { RT, RA0, RB })       \
DE(neg,           0x7C0000D0, Form::kXO,  neg,         { RT, RA })            \
DE(negdot,        0x7C0000D1, Form::kXO,  neg.,        { RT, RA })            \
DE(nor,           0x7C0000F8, Form::kX,   nor,         { RA, RS, RB })        \
DE(nordot,        0x7C0000F9, Form::kX,   nor.,        { RA, RS, RB })        \
DE(subfedot,      0x7C000111, Form::kXO,  subfe.,      { RT, RA, RB })        \
DE(addedot,       0x7C000115, Form::kXO,  adde.,       { RT, RA, RB })        \
DE(mtcrf,         0x7C000120, Form::kXFX, mtcrf,       { FXM, RS })           \
DE(mtmsr,         0x7C000124, Form::kX,   mtmsr,       { RS })                \
DE(stdx,          0x7C00012A, Form::kX,   stdx,        { RS, RA0, RB })       \
DE(stwx,          0x7C00012E, Form::kX,   stwx,        { RS, RA0, RB })       \
DE(mtmsrd,        0x7C000164, Form::kX,   mtmsrd,      { RS, A_L })           \
DE(mtvsrd,        0x7C000166, Form::kXX1, mtvsrd,      { XT, RA })            \
DE(subfzedot,     0x7C000191, Form::kXO,  subfze.,     { RT, RA })            \
DE(addzedot,      0x7C000195, Form::kXO,  addze.,      { RT, RA })            \
DE(stdcxdot,      0x7C0001AD, Form::kX,   stdcx.,      { RS, RA0, RB })       \
DE(stbx,          0x7C0001AE, Form::kX,   stbx,        { RS, RA0, RB })       \
DE(subfmedot,     0x7C0001D1, Form::kXO,  subfme.,     { RT, RA })            \
DE(mullddot,      0x7C0001D3, Form::kXO,  mulld.,      { RT, RA, RB })        \
DE(addmedot,      0x7C0001D5, Form::kXO,  addme.,      { RT, RA })            \
DE(mullwdot,      0x7C0001D7, Form::kXO,  mullw.,      { RT, RA, RB })        \
DE(add,           0x7C000214, Form::kXO,  add,         { RT, RA, RB })        \
DE(adddot,        0x7C000215, Form::kXO,  add.,        { RT, RA, RB })        \
DE(lhzx,          0x7C00022E, Form::kX,   lhzx,        { RT, RA0, RB })       \
DE(eqvdot,        0x7C000239, Form::kX,   eqv.,        { RA, RS, RB })        \
DE(xor,           0x7C000278, Form::kX,   xor,         { RA, RS, RB })        \
DE(xordot,        0x7C000279, Form::kX,   xor.,        { RA, RS, RB })        \
DE(mfspr,         0x7C0002A6, Form::kXFX, mfspr,       { RT, SPR })           \
DE(mftb,          0x7C0002E6, Form::kXFX, mftb,        { RT, TBR })           \
DE(divdeudot,     0x7C000313, Form::kXO,  divdeu.,     { RT, RA, RB })        \
DE(divweudot,     0x7C000317, Form::kXO,  divweu.,     { RT, RA, RB })        \
DE(sthx,          0x7C00032E, Form::kX,   sthx,        { RS, RA0, RB })       \
DE(orcdot,        0x7C000339, Form::kX,   orc.,        { RA, RS, RB })        \
DE(divdedot,      0x7C000353, Form::kXO,  divde.,      { RT, RA, RB })        \
DE(divwedot,      0x7C000357, Form::kXO,  divwe.,      { RT, RA, RB })        \
DE(or,            0x7C000378, Form::kX,   or,          { RA, RS, RB })        \
DE(ordot,         0x7C000379, Form::kX,   or.,         { RA, RS, RB })        \
DE(divdudot,      0x7C000393, Form::kXO,  divdu.,      { RT, RA, RB })        \
DE(divwudot,      0x7C000397, Form::kXO,  divwu.,      { RT, RA, RB })        \
DE(mtpmr,         0x7C00039C, Form::kXFX, mtpmr,       { PMR, RS })           \
DE(mtspr,         0x7C0003A6, Form::kXFX, mtspr,       { SPR, RS })           \
DE(nanddot,       0x7C0003B9, Form::kX,   nand.,       { RA, RS, RB })        \
DE(dcread2,       0x7C0003CC, Form::kX,   dcread,      { RT, RA, RB })        \
DE(divd,          0x7C0003D2, Form::kXO,  divd,        { RT, RA, RB })        \
DE(divddot,       0x7C0003D3, Form::kXO,  divd.,       { RT, RA, RB })        \
DE(divwdot,       0x7C0003D7, Form::kXO,  divw.,       { RT, RA, RB })        \
DE(cmpb,          0x7C0003F8, Form::kX,   cmpb,        { RA, RS, RB })        \
DE(subfcodot,     0x7C000411, Form::kXO,  subfco.,     { RT, RA, RB })        \
DE(addcodot,      0x7C000415, Form::kXO,  addco.,      { RT, RA, RB })        \
DE(srwdot,        0x7C000431, Form::kX,   srw.,        { RA, RS, RB })        \
DE(srddot,        0x7C000437, Form::kX,   srd.,        { RA, RS, RB })        \
DE(subfo,         0x7C000450, Form::kXO,  subfo,       { RT, RA, RB })        \
DE(subfodot,      0x7C000451, Form::kXO,  subfo.,      { RT, RA, RB })        \
DE(lfdx,          0x7C0004AE, Form::kX,   lfdx,        { FRT, RA0, RB })      \
DE(negodot,       0x7C0004D1, Form::kXO,  nego.,       { RT, RA })            \
DE(subfeodot,     0x7C000511, Form::kXO,  subfeo.,     { RT, RA, RB })        \
DE(addeodot,      0x7C000515, Form::kXO,  addeo.,      { RT, RA, RB })        \
DE(subfzeodot,    0x7C000591, Form::kXO,  subfzeo.,    { RT, RA })            \
DE(addzeodot,     0x7C000595, Form::kXO,  addzeo.,     { RT, RA })            \
DE(stfdx,         0x7C0005AE, Form::kX,   stfdx,       { FRS, RA0, RB })      \
DE(subfmeodot,    0x7C0005D1, Form::kXO,  subfmeo.,    { RT, RA })            \
DE(mulldo,        0x7C0005D2, Form::kXO,  mulldo,      { RT, RA, RB })        \
DE(mulldodot,     0x7C0005D3, Form::kXO,  mulldo.,     { RT, RA, RB })        \
DE(addmeodot,     0x7C0005D5, Form::kXO,  addmeo.,     { RT, RA })            \
DE(mullwodot,     0x7C0005D7, Form::kXO,  mullwo.,     { RT, RA, RB })        \
DE(addo,          0x7C000614, Form::kXO,  addo,        { RT, RA, RB })        \
DE(addodot,       0x7C000615, Form::kXO,  addo.,       { RT, RA, RB })        \
DE(lxvw4x,        0x7C000618, Form::kXX1, lxvw4x,      { XT, RA, RB })        \
DE(srawdot,       0x7C000631, Form::kX,   sraw.,       { RA, RS, RB })        \
DE(srad,          0x7C000634, Form::kX,   srad,        { RA, RS, RB })        \
DE(sraddot,       0x7C000635, Form::kX,   srad.,       { RA, RS, RB })        \
DE(srawidot,      0x7C000671, Form::kX,   srawi.,      { RA, RS, SH })        \
DE(sradi,         0x7C000674, Form::kXS,  sradi,       { RA, RS, SH6 })       \
DE(sradidot,      0x7C000675, Form::kXS,  sradi.,      { RA, RS, SH6 })       \
DE(lxvd2x,        0x7C000698, Form::kXX1, lxvd2x,      { XT, RA, RB })        \
DE(divdeuodot,    0x7C000713, Form::kXO,  divdeuo.,    { RT, RA, RB })        \
DE(divweuodot,    0x7C000717, Form::kXO,  divweuo.,    { RT, RA, RB })        \
DE(stxvw4x,       0x7C000718, Form::kXX1, stxvw4x,     { XS, RA, RB })        \
DE(extsh,         0x7C000734, Form::kX,   extsh,       { RA, RS })            \
DE(extshdot,      0x7C000735, Form::kX,   extsh.,      { RA, RS })            \
DE(divdeodot,     0x7C000753, Form::kXO,  divdeo.,     { RT, RA, RB })        \
DE(divweodot,     0x7C000757, Form::kXO,  divweo.,     { RT, RA, RB })        \
DE(extsb,         0x7C000774, Form::kX,   extsb,       { RA, RS})             \
DE(extsbdot,      0x7C000775, Form::kX,   extsb.,      { RA, RS})             \
DE(divduodot,     0x7C000793, Form::kXO,  divduo.,     { RT, RA, RB })        \
DE(divwuodot,     0x7C000797, Form::kXO,  divwuo.,     { RT, RA, RB })        \
DE(extsw,         0x7C0007B4, Form::kX,   extsw,       { RA, RS })            \
DE(extswdot,      0x7C0007B5, Form::kX,   extsw.,      { RA, RS })            \
DE(divdodot,      0x7C0007D3, Form::kXO,  divdo.,      { RT, RA, RB })        \
DE(divwodot,      0x7C0007D7, Form::kXO,  divwo.,      { RT, RA, RB })        \
DE(mtocrf,        0x7C100120, Form::kXFX, mtocrf,      { FXM, RS })           \
DE(lwz,           0x80000000, Form::kD,   lwz,         { RT, D, RA0 })        \
DE(lbz,           0x88000000, Form::kD,   lbz,         { RT, D, RA0 })        \
DE(stw,           0x90000000, Form::kD,   stw,         { RS, D, RA0 })        \
DE(stb,           0x98000000, Form::kD,   stb,         { RS, D, RA0 })        \
DE(lhz,           0xA0000000, Form::kD,   lhz,         { RT, D, RA0 })        \
DE(sth,           0xB0000000, Form::kD,   sth,         { RS, D, RA0 })        \
DE(lfs,           0xC0000000, Form::kD,   lfs,         { FRT, D, RA0 })       \
DE(lfd,           0xC8000000, Form::kD,   lfd,         { FRT, D, RA0 })       \
DE(stfs,          0xD0000000, Form::kD,   stfs,        { FRS, D, RA0 })       \
DE(stfd,          0xD8000000, Form::kD,   stfd,        { FRS, D, RA0 })       \
DE(ld,            0xE8000000, Form::kDS,  ld,          { RT, DS, RA0 })       \
DE(frsqrtes,      0xEC000034, Form::kA,   frsqrtes,    { FRT, FRB, A_L })     \
DE(frsqrtesdot,   0xEC000035, Form::kA,   frsqrtes.,   { FRT, FRB, A_L })     \
DE(dcmpu,         0xEC000504, Form::kX,   dcmpu,       { BF,  FRA, FRB })     \
DE(fcfids,        0xEC00069C, Form::kX,   fcfids,      { FRT, FRB })          \
DE(fcfidsdot,     0xEC00069D, Form::kX,   fcfids.,     { FRT, FRB })          \
DE(xxpermdi,      0xF0000050, Form::kXX3, xxpermdi,    { XT, XA, XB, DM })    \
DE(xsrdpi,        0xF0000124, Form::kXX2, xsrdpi,      { XT, XB })            \
DE(xssqrtdp,      0xF000012C, Form::kXX2, xssqrtdp,    { XT, XB })            \
DE(xvcvspsxws,    0xF0000260, Form::kXX2, xvcvspsxws,  { XT, XB })            \
DE(xvdivsp,       0xF00002C0, Form::kXX3, xvdivsp,     { XT, XA, XB })        \
DE(xvdivdp,       0xF00003C0, Form::kXX3, xvdivdp,     { XT, XA, XB })        \
DE(xxlxor,        0xF00004D0, Form::kXX3, xxlxor,      { XT, XA, XB })        \
DE(xscvdpuxds,    0xF0000520, Form::kXX2, xscvdpuxds,  { XT, XB })            \
DE(xscvdpsxds,    0xF0000560, Form::kXX2, xscvdpsxds,  { XT, XB })            \
DE(xscvsxddp,     0xF00005E0, Form::kXX2, xscvsxddp,   { XT, XB })            \
DE(xvcvspsxds,    0xF0000660, Form::kXX2, xvcvspsxds,  { XT, XB })            \
DE(std,           0xF8000000, Form::kDS,  std,         { RS, DS, RA0 })       \
DE(stdu,          0xF8000001, Form::kDS,  stdu,        { RS, DS, RAS })       \
DE(fcmpu,         0xFC000000, Form::kX,   fcmpu,       { BF, FRA, FRB })      \
DE(fadd,          0xFC00002A, Form::kA,   fadd,        { FRT, FRA, FRB })     \
DE(fadddot,       0xFC00002B, Form::kA,   fadd.,       { FRT, FRA, FRB })     \
DE(fcmpo,         0xFC000040, Form::kX,   fcmpo,       { BF, FRA, FRB })      \
DE(mcrfs,         0xFC000080, Form::kX,   mcrfs,       { BF, BFA })           \
DE(mtfsb0,        0xFC00008C, Form::kX,   mtfsb0,      { BT })                \
DE(mtfsb0dot,     0xFC00008D, Form::kX,   mtfsb0.,     { BT })                \
DE(fmr,           0xFC000090, Form::kX,   fmr,         { FRT, FRB })          \
DE(fmrdot,        0xFC000091, Form::kX,   fmr.,        { FRT, FRB })          \
DE(fabs,          0xFC000210, Form::kX,   fabs,        { FRT, FRB })          \
DE(fabsdot,       0xFC000211, Form::kX,   fabs.,       { FRT, FRB })          \
DE(fctid,         0xFC00065C, Form::kX,   fctid,       { FRT, FRB })          \
DE(fctiddot,      0xFC00065D, Form::kX,   fctid.,      { FRT, FRB })          \
DE(fctidz,        0xFC00065E, Form::kX,   fctidz,      { FRT, FRB })          \
DE(fctidzdot,     0xFC00065F, Form::kX,   fctidz.,     { FRT, FRB })          \
DE(fcfid,         0xFC00069C, Form::kX,   fcfid,       { FRT, FRB })          \
DE(fcfiddot,      0xFC00069D, Form::kX,   fcfid.,      { FRT, FRB })          \
/* */

enum class OpcodeNames {
  #define DE(name, op, type, mnemonic, ... )                \
    op_##name,

  PPC64_OPCODES

  #undef DE

  op_last
};

// appropriate cast for array manipulation
constexpr size_t kTotalOpcodes = static_cast<size_t>(OpcodeNames::op_last);

enum class AllowCond {
  OnlyUncond,
  Any,
  OnlyCond
};

struct DecoderInfo {
  DecoderInfo(OpcodeNames opn, PPC64Instr op, Form form,
      std::string mnemonic, std::initializer_list<Operands> oper)
  : m_opn(opn)
  , m_ip(nullptr)
  , m_op(op)
  , m_form(form)
  , m_mnemonic(mnemonic)
  , m_image(0x0) {
    // pushing it in reverse order
    for (auto it = oper.begin(); it != oper.end(); it++) {
      m_operands.push_back(*it);
    }
  }

  ~DecoderInfo() {}

  DecoderInfo() = delete;

  inline const uint8_t* ip() const            { return m_ip; }
  inline Form form() const                    { return m_form; }
  inline OpcodeNames opcode_name() const      { return m_opn; }
  inline PPC64Instr opcode() const            { return m_op; }
  inline std::string mnemonic() const         { return m_mnemonic; }
  std::string toString() const;

  const PPC64Instr instruction_image() const  { return m_image; }
  void instruction_image(const PPC64Instr i)  { m_image = i; }

  inline bool operator==(const DecoderInfo& i) {
    return (i.form() == m_form &&
            i.opcode() == m_op &&
            i.mnemonic() == m_mnemonic);
  }

  inline bool operator!=(const DecoderInfo& i) {
    return (i.form() != m_form ||
            i.opcode() != m_op ||
            i.mnemonic() != m_mnemonic);
  }

  bool isInvalid() const {
    return m_opn == OpcodeNames::op_invalid;
  }
  bool isException() const;
  bool isNop() const;
  bool isOffsetBranch(AllowCond ac = AllowCond::Any) const;
  bool isRegisterBranch(AllowCond ac = AllowCond::Any) const;
  bool isBranchWithLR() const;
  bool isClearSignBit() const;
  bool isSpOffsetInstr() const;
  int32_t offset() const;
  int32_t branchOffset() const;
  PPC64Instr setBranchOffset(int32_t offset) const;
  void setIp(const uint8_t* const ip)     { m_ip = ip; }
  void setIp(const PPC64Instr* const ip) {
    setIp(reinterpret_cast<const uint8_t* const>(ip));
  }
  bool isAddis(bool toc = false) const;
  bool isLd(bool toc = false) const;
  bool isLwz(bool toc = false) const;
  int16_t offsetDS() const;
  int16_t offsetD() const;

private:
  // Auxiliary function for isLd and isLwz
  bool isDformOp(OpcodeNames opn, bool toc) const;

  // opcode enumeration identifier
  OpcodeNames m_opn;
  // pointer to the decoded instruction in the memory
  const uint8_t* m_ip;
  // the opcode part of the instruction
  PPC64Instr m_op;
  // points out which -Form the instruction is
  Form m_form;
  // the mnemonic string of the instruction
  std::string m_mnemonic;
  // the complete instruction, as used to decode
  PPC64Instr m_image;
  // operands list
  std::deque<Operands> m_operands;
};

class Decoder : private boost::noncopyable {
  DecoderInfo **m_decoder_table;
  DecoderInfo **m_opcode_index_table;
  static Decoder* s_decoder;
  std::map<int32_t, int32_t> opcode_index_map;
  std::map<int32_t, int32_t> opcode_size_map;
  int32_t m_decoder_table_size;

  void setInstruction(DecoderInfo dinfo) {

    m_decoder_table[m_decoder_table_size++] = new DecoderInfo(dinfo);

   m_opcode_index_table[static_cast<PPC64Instr>(dinfo.opcode_name())] =
      m_decoder_table[m_decoder_table_size];
  }

  /*
   * Create index based on opcode to help when searching the instruction.
   */
  void createOpcodeIndex () {
    PPC64Instr current_opcode = 0x0;
    int32_t current_index = 0;

    opcode_index_map[current_opcode] = current_index;
    for (int i = 0; i < m_decoder_table_size; i++) {
      PPC64Instr tmp_opcode = m_decoder_table[i]->opcode() & kOpcodeMask;
      if (current_opcode != tmp_opcode) {
        opcode_size_map[current_opcode] = i - current_index; // set size
        current_opcode = tmp_opcode;
        current_index = i;
        opcode_index_map[current_opcode] = current_index;
      }
    }
  }

  Decoder();

  ~Decoder() {
    for(int i = 0; i < kDecoderSize; i++)
       if(m_decoder_table[i] != nullptr) {
          delete m_decoder_table[i];
       }
    delete[] m_decoder_table;
  }

public:
  const DecoderInfo getInvalid() {
    return *m_decoder_table[static_cast<size_t>(OpcodeNames::op_invalid)];
  }

  static Decoder& GetDecoder() {
    static Decoder dec;
    s_decoder = &dec;
    return *s_decoder;
  }

  const DecoderInfo decode(const PPC64Instr* const ip);

  int32_t searchInstr(
      int32_t opc_index,
      int32_t opc_size,
      PPC64Instr instr) const;

  inline const DecoderInfo decode(const uint8_t* const ip) {
    return decode(reinterpret_cast<const PPC64Instr* const>(ip));
  }
};

} // namespace ppc64_asm

#endif

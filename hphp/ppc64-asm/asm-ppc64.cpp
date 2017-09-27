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

#include "hphp/ppc64-asm/asm-ppc64.h"
#include "hphp/ppc64-asm/decoded-instr-ppc64.h"
#include "hphp/ppc64-asm/decoder-ppc64.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/trace.h"
#include <folly/MicroSpinLock.h>

#include <type_traits>

TRACE_SET_MOD(asmppc64);

namespace ppc64_asm {

// Lock to protect TOC when writing.
static folly::MicroSpinLock s_TOC;

//////////////////////////////////////////////////////////////////////

VMTOC::~VMTOC() {
  FTRACE(1, "Number of values if 64bits stored in TOC: {}\n",
    std::to_string(m_last_elem_pos));
}

int64_t VMTOC::pushElem(int64_t elem, bool elemMayChange) {
  int64_t offset;
  if (elemMayChange) {
    offset = allocTOC(elem);
  }
  else {
    auto& map_elem = m_map[elem];
    if (map_elem) return map_elem;
    offset = allocTOC(elem);
    map_elem = offset;
  }

  m_last_elem_pos += 1;
  return offset;
}

VMTOC& VMTOC::getInstance() {
  static VMTOC instance;
  return instance;
}

intptr_t VMTOC::getPtrVector() {
  always_assert(m_tocvector != nullptr);
  return reinterpret_cast<intptr_t>(m_tocvector->base() + INT16_MAX + 1);
}

int64_t VMTOC::getValue(int64_t index, bool qword) {
  HPHP::Address addr = reinterpret_cast<HPHP::Address>(
      static_cast<intptr_t>(index) + getPtrVector());
  int64_t ret_val = 0;
  int max_elem = qword ? 8 : 4;
  for (int i = max_elem-1; i >= 0; i--) {
    ret_val = addr[i] + (ret_val << 8);
  }
  return ret_val;
}

uint64_t* VMTOC::getAddr(int64_t index) {
  return reinterpret_cast<uint64_t*>(
      static_cast<intptr_t>(index) + getPtrVector());
}

int64_t VMTOC::allocTOC(int64_t target) {
  folly::MSLGuard g{s_TOC};
  HPHP::Address addr = m_tocvector->frontier();
  m_tocvector->qword(target);
  return addr - (m_tocvector->base() + INT16_MAX + 1);
}

void VMTOC::setTOCDataBlock(HPHP::DataBlock *db) {
  if(m_tocvector == nullptr) {
    m_tocvector = db;
    HPHP::Address addr = m_tocvector->frontier();
    forceAlignment(addr);
  }
  return;
}

void VMTOC::forceAlignment(HPHP::Address& addr) {
  folly::MSLGuard g{s_TOC};
  // keep 8-byte alignment
  while (reinterpret_cast<uintptr_t>(addr) % 8 != 0) {
    uint8_t fill_byte = 0xf0;
    m_tocvector->assertCanEmit(sizeof(uint8_t));
    m_tocvector->byte(fill_byte);
    addr = m_tocvector->frontier();
  }
}

int64_t VMTOC::getIndex(uint64_t elem) {
  auto pos = m_map.find(elem);
  if (pos != m_map.end()) {
    return pos->second;
  }
  return LLONG_MIN;
}

void BranchParams::decodeInstr(const PPC64Instr* const pinstr) {
  const DecoderInfo dinfo = Decoder::GetDecoder().decode(pinstr);
  switch (dinfo.opcode_name()) {
    case OpcodeNames::op_b:
    case OpcodeNames::op_bl:
      assert(dinfo.form() == Form::kI);
      defineBoBi(BranchConditions::Always);
      break;
    case OpcodeNames::op_bc:
      assert(dinfo.form() == Form::kB);
      B_form_t bform;
      bform.instruction = dinfo.instruction_image();
      m_bo = BranchParams::BO(bform.BO);
      m_bi = BranchParams::BI(bform.BI);
      break;
    case OpcodeNames::op_bcctr:
    case OpcodeNames::op_bcctrl:
      assert(dinfo.form() == Form::kXL);
      XL_form_t xlform;
      xlform.instruction = dinfo.instruction_image();
      m_bo = BranchParams::BO(xlform.BT);
      m_bi = BranchParams::BI(xlform.BA);
      break;
    default:
      assert(false && "Not a valid conditional branch instruction");
      // also possible: defineBoBi(BranchConditions::Always);
      break;
  }

  // Set m_lr accordingly for all 'call' flavors used
  switch (dinfo.opcode_name()) {
    case OpcodeNames::op_bl:
    case OpcodeNames::op_bcctrl:
      m_lr = true;
      break;
    default:
      m_lr = false;
      break;
  }
}

/*
 * Macro definition for EmitXOForm functions
 * Format:
 *  X(name,   arg3,  oe,  xop)
 *    name: function name
 *    arg3: ARG if needed, otherwise NONE to skip
 *    oe:   parameter value
 *    xop:  parameter value
 */

#define ADDS\
  X(add,     ARG,   0,  266)  \
  X(addo,    ARG,   1,  266)  \
  X(divd,    ARG,   0,  489)  \
  X(mulldo,  ARG,   1,  233)  \
  X(neg,     NONE,  0,  104)  \
  X(subf,    ARG,   0,  40)   \
  X(subfo,   ARG,   1,  40)   \

/* Function header: XO1 */
#define HEADER_ARG  const Reg64& rb,
#define HEADER_NONE

#define XO1(name, arg3, oe, xop)                                          \
void Assembler::name(const Reg64& rt, const Reg64& ra, arg3 bool rc) {

/* Function body: XO2 */
#define BODY_ARG  rb
#define BODY_NONE 0

#define XO2(name, arg3, oe, xop)                                          \
    EmitXOForm(31, rn(rt), rn(ra), rn(arg3), oe, xop, rc);                \
}

/* Macro expansion for function parts */
#define X(name, arg3, oe, xop)  \
      XO1(name, HEADER_##arg3,  oe, xop)  \
      XO2(name, BODY_##arg3,    oe, xop)
ADDS
#undef X

#undef HEADER_ARG
#undef HEADER_NONE

#undef BODY_ARG
#undef BODY_NONE

#undef ADDS

void Assembler::addi(const Reg64& rt, const Reg64& ra, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(14, rn(rt), rn(ra), imm.w());
}

void Assembler::addis(const Reg64& rt, const Reg64& ra, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(15, rn(rt), rn(ra), imm.w());
}

void Assembler::and(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                     bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 28, rc);
}

void Assembler::andi(const Reg64& ra, const Reg64& rs, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(28, rn(rs), rn(ra), imm.w());
}

void Assembler::b(int32_t offset) {
  EmitIForm(18, uint32_t(offset));
}

void Assembler::bl(int32_t offset) {
  EmitIForm(18, uint32_t(offset), 0, 1);
}

void Assembler::bc(uint8_t bo, uint8_t bi, int16_t offset) {
  EmitBForm(16, bo, bi, uint32_t(offset), 0, 0);
}

void Assembler::bcctr(uint8_t bo, uint8_t bi, uint16_t bh) {
  EmitXLForm(19, bo, bi, (bh & 0x3), 528);
}

void Assembler::bctrl() {
  // The concept of a conditional call is not existent for upper layers.
  // Therefore no bcctrl is defined despite being possible.
  // Only bctrl is defined.
  BranchParams bp(BranchConditions::Always);
  EmitXLForm(19, bp.bo(), bp.bi(), (0 /*bh*/ & 0x3), 528, 1);
}

void Assembler::blr() {
  // The concept of a conditional return is not existent for upper layers.
  // Therefore no bclr is defined despite being possible.
  // Only blr is defined.
  BranchParams bp(BranchConditions::Always);
  EmitXLForm(19, bp.bo(), bp.bi(), (0 /*bh*/ & 0x3), 16, 0);
}

void Assembler::cmp(uint16_t bf, bool l, const Reg64& ra, const Reg64& rb) {
  EmitXForm(31, rn((bf << 2) | (uint16_t)l), rn(ra), rn(rb), 0);
}

void Assembler::cmpi(uint16_t bf, bool l, const Reg64& ra, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(11, rn((bf << 2) | (uint16_t)l), rn(ra), imm.w());
}

void Assembler::cmpb(const Reg64& rs, const Reg64& ra, const Reg64& rb) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 508);
}

void Assembler::cmpl(uint16_t bf, bool l, const Reg64& ra, const Reg64& rb) {
  EmitXForm(31, rn((bf << 2) | (uint16_t)l), rn(ra), rn(rb), 32);
}

void Assembler::cmpli(uint16_t bf, bool l, const Reg64& ra, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(10, rn((bf << 2) | (uint16_t)l), rn(ra), imm.w());
}

void Assembler::extsb(const Reg64& ra, const Reg64& rs, bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(0), 954, rc);
}

void Assembler::extsh(const Reg64& ra, const Reg64& rs, bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(0), 922, rc);
}

void Assembler::extsw(const Reg64& ra, const Reg64& rs, bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(0), 986, rc);
}

void Assembler::isel(const Reg64& rt, const Reg64& ra, const Reg64& rb,
                     uint8_t bc) {
  EmitAForm(31, rn(rt), rn(ra), rn(rb), rn(bc), 15);
}

void Assembler::lbz(const Reg64& rt, MemoryRef m) {
  assertx(Reg64(-1) == m.r.index);  // doesn't support base+index
  EmitDForm(34, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::lbzx(const Reg64& rt, MemoryRef m) {
  assertx(!m.r.disp);  // doesn't support immediate displacement
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 87);
}

void Assembler::ld(const Reg64& rt, MemoryRef m) {
  assertx(Reg64(-1) == m.r.index);  // doesn't support base+index
  EmitDSForm(58, rn(rt), rn(m.r.base), m.r.disp, 0);
}

void Assembler::ldx(const Reg64& rt, MemoryRef m) {
  assertx(!m.r.disp);  // doesn't support immediate displacement
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 21);
}

void Assembler::lhz(const Reg64& rt, MemoryRef m) {
  assertx(Reg64(-1) == m.r.index);  // doesn't support base+index
  EmitDForm(40, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::lhzx(const Reg64& rt, MemoryRef m) {
  assertx(!m.r.disp);  // doesn't support immediate displacement
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 279);
}

void Assembler::lwz(const Reg64& rt, MemoryRef m) {
  assertx(Reg64(-1) == m.r.index);  // doesn't support base+index
  EmitDForm(32, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::lwzx(const Reg64& rt, MemoryRef m) {
  assertx(!m.r.disp);  // doesn't support immediate displacement
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 23);
}

void Assembler::mfspr(const SpecialReg spr, const Reg64& rs) {
  EmitXFXForm(31, rn(rs), spr, 339);
}

void Assembler::mtspr(const SpecialReg spr, const Reg64& rs) {
  EmitXFXForm(31, rn(rs), spr, 467);
}

void Assembler::nor(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                    bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 124, rc);
}

void Assembler::or(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                    bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 444, rc);
}

void Assembler::ori(const Reg64& ra, const Reg64& rs, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(24, rn(rs), rn(ra), imm.w());
}

void Assembler::oris(const Reg64& ra, const Reg64& rs, Immed imm) {
  assert(imm.fits(HPHP::sz::word) && "Immediate is too big");
  EmitDForm(25, rn(rs), rn(ra), imm.w());
}

void Assembler::rldicl(const Reg64& ra, const Reg64& rs, uint8_t sh,
                       uint8_t mb, bool rc) {
  EmitMDForm(30, rn(rs), rn(ra), sh, mb, 0, rc);
}

void Assembler::rldicr(const Reg64& ra, const Reg64& rs, uint8_t sh,
                       uint8_t mb, bool rc) {
  EmitMDForm(30, rn(rs), rn(ra), sh, mb, 1, rc);
}

void Assembler::rlwinm(const Reg64& ra, const Reg64& rs, uint8_t sh, uint8_t mb,
                       uint16_t me, bool rc) {
  EmitMForm(21, rn(rs), rn(ra), rn(sh), mb, me, rc);
}

void Assembler::sld(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                    bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 27, rc);
}

void Assembler::srad(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                     bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 794, rc);
}

void Assembler::sradi(const Reg64& ra, const Reg64& rs, uint8_t sh, bool rc) {
  EmitXSForm(31, rn(rs), rn(ra), sh, 413, rc);
}

void Assembler::stb(const Reg64& rt, MemoryRef m) {
  assertx(Reg64(-1) == m.r.index);  // doesn't support base+index
  EmitDForm(38, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::stbx(const Reg64& rt, MemoryRef m) {
  assertx(!m.r.disp);  // doesn't support immediate displacement
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 215);
}

void Assembler::sth(const Reg64& rt, MemoryRef m) {
  assertx(Reg64(-1) == m.r.index);  // doesn't support base+index
  EmitDForm(44, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::sthx(const Reg64& rt, MemoryRef m) {
  assertx(!m.r.disp);  // doesn't support immediate displacement
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 407);
}

void Assembler::stw(const Reg64& rt, MemoryRef m) {
  assertx(Reg64(-1) == m.r.index);  // doesn't support base+index
  EmitDForm(36, rn(rt), rn(m.r.base), m.r.disp);
}

void Assembler::stwx(const Reg64& rt, MemoryRef m) {
  assertx(!m.r.disp);  // doesn't support immediate displacement
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 151);
}

void Assembler::std(const Reg64& rt, MemoryRef m) {
  assertx(Reg64(-1) == m.r.index);  // doesn't support base+index
  EmitDSForm(62, rn(rt), rn(m.r.base), m.r.disp, 0);
}

void Assembler::stdu(const Reg64& rt, MemoryRef m) {
  assertx(Reg64(-1) == m.r.index);  // doesn't support base+index
  EmitDSForm(62, rn(rt), rn(m.r.base), m.r.disp, 1);
}

void Assembler::stdx(const Reg64& rt, MemoryRef m) {
  assertx(!m.r.disp);  // doesn't support immediate displacement
  EmitXForm(31, rn(rt), rn(m.r.base), rn(m.r.index), 149);
}

void Assembler::td(uint16_t to, const Reg64& ra, const Reg64& rb) {
  EmitXForm(31, rn(to), rn(ra), rn(rb), 68);
}

void Assembler::tw(uint16_t to, const Reg64& ra, const Reg64& rb) {
  EmitXForm(31, rn(to), rn(ra), rn(rb), 4);
}

void Assembler::xor(const Reg64& ra, const Reg64& rs, const Reg64& rb,
                     bool rc) {
  EmitXForm(31, rn(rs), rn(ra), rn(rb), 316, rc);
}

/* Floating point operations */
void Assembler::fadd(const RegXMM& frt, const RegXMM& fra, const RegXMM& frb,
                     bool rc) {
  EmitAForm(63, rn(frt), rn(fra), rn(frb), rn(0), 21, rc);
}

void Assembler::fsub(const RegXMM& frt, const RegXMM& fra, const RegXMM& frb,
                     bool rc) {
  EmitAForm(63, rn(frt), rn(fra), rn(frb), rn(0), 20, rc);
}

void Assembler::fmul(const RegXMM& frt, const RegXMM& fra, const RegXMM& frc,
                     bool rc) {
  EmitAForm(63, rn(frt), rn(fra), rn(0), rn(frc), 25, rc);
}

void Assembler::fdiv(const RegXMM& frt, const RegXMM& fra, const RegXMM& frb,
                     bool rc) {
  EmitAForm(63, rn(frt), rn(fra), rn(frb), rn(0), 18, rc);
}

void Assembler::unimplemented(){
  //Emit a instruction with invalid opcode 0x0
  EmitDForm(0, rn(0), rn(0), 0);
}

//////////////////////////////////////////////////////////////////////

void Assembler::patchAbsolute(CodeAddress jmp, CodeAddress dest) {
  // Initialize code block cb pointing to li64
  HPHP::CodeBlock cb;
  cb.init(jmp, Assembler::kLimmLen, "patched bctr");
  Assembler a{ cb };
  a.limmediate(reg::r12, ssize_t(dest), ImmType::TocOnly, true);
}

void Assembler::patchBranch(CodeAddress jmp, CodeAddress dest) {
  auto di = DecodedInstruction(jmp);

  // Detect Far branch
  if (di.isFarBranch()) {
    patchAbsolute(jmp, dest);
    return;
  }

  // Regular patch for branch by offset type
  if (!di.setNearBranchTarget(dest))
    assert(false && "Can't patch a branch with such a big offset");
}

//////////////////////////////////////////////////////////////////////

void Assembler::li64 (const Reg64& rt, int64_t imm64, bool fixedSize) {
  // li64 always emits 5 instructions i.e. 20 bytes of instructions.
  // Assumes that 0 bytes will be missing in the end.
  uint8_t missing = 0;

  // for assert purposes
  DEBUG_ONLY CodeAddress li64StartPos = frontier();

  if (HPHP::jit::deltaFits(imm64, HPHP::sz::word)) {
    // immediate has only low 16 bits set, use simple load immediate
    li(rt, static_cast<int16_t>(imm64));
    if (imm64 & (1ULL << 15) && !(imm64 & (1ULL << 16))) {
      // clear extended sign that should not be set
      // (32bits number. Sets the 16th bit but not the 17th, it's not negative!)
      clrldi(rt, rt, 48);
      missing = kLi64Len - 2 * instr_size_in_bytes;
    } else {
      missing = kLi64Len - 1 * instr_size_in_bytes;
    }
  } else if (HPHP::jit::deltaFits(imm64, HPHP::sz::dword)) {
    // immediate has only low 32 bits set
    lis(rt, static_cast<int16_t>(imm64 >> 16));
    ori(rt, rt, static_cast<int16_t>(imm64 & UINT16_MAX));
    if (imm64 & (1ULL << 31) && !(imm64 & (1ULL << 32))) {
      // clear extended sign
      // (64bits number. Sets the 32th bit but not the 33th, it's not negative!)
      clrldi(rt, rt, 32);
      missing = kLi64Len - 3 * instr_size_in_bytes;
    } else {
      missing = kLi64Len - 2 * instr_size_in_bytes;
    }
  } else if (imm64 >> 48 == 0) {
    // immediate has only low 48 bits set
    lis(rt, static_cast<int16_t>(imm64 >> 32));
    ori(rt, rt, static_cast<int16_t>((imm64 >> 16) & UINT16_MAX));
    sldi(rt,rt,16);
    ori(rt, rt, static_cast<int16_t>(imm64 & UINT16_MAX));
    if (imm64 & (1ULL << 47)) {
      // clear extended sign
      clrldi(rt, rt, 16);
    } else {
      missing = kLi64Len - 4 * instr_size_in_bytes;
    }
  } else {
    // load all 64 bits
    lis(rt, static_cast<int16_t>(imm64 >> 48));
    ori(rt, rt, static_cast<int16_t>((imm64 >> 32) & UINT16_MAX));
    sldi(rt,rt,32);
    oris(rt, rt, static_cast<int16_t>((imm64 >> 16) & UINT16_MAX));
    ori(rt, rt, static_cast<int16_t>(imm64 & UINT16_MAX));
  }

  if (fixedSize) {
    emitNop(missing);
    // guarantee our math with kLi64Len is working
    assert(kLi64Len == frontier() - li64StartPos);
  }
}

void Assembler::li32 (const Reg64& rt, int32_t imm32) {

  if (HPHP::jit::deltaFits(imm32, HPHP::sz::word)) {
    // immediate has only low 16 bits set, use simple load immediate
    li(rt, static_cast<int16_t>(imm32));
    if (imm32 & (1ULL << 15) && !(imm32 & (1ULL << 16))) {
      // clear extended sign that should not be set
      // (32bits number. Sets the 16th bit but not the 17th, it's not negative!)
      clrldi(rt, rt, 48);
    } else {
      emitNop(instr_size_in_bytes); // emit nop for a balanced li32 with 2 instr
    }
  } else {
    // immediate has 32 bits set
    lis(rt, static_cast<int16_t>(imm32 >> 16));
    ori(rt, rt, static_cast<int16_t>(imm32 & UINT16_MAX));
  }
}

void Assembler::li64TOC (const Reg64& rt, int64_t imm64,
                         ImmType immt, bool immMayChange) {
  int64_t TOCoffset;
  TOCoffset = VMTOC::getInstance().pushElem(imm64, immMayChange);

  if (TOCoffset > INT16_MAX) {
    int16_t complement = 0;
    // If last four bytes is still bigger than a signed 16bits, uses as two
    // complement.
    if ((TOCoffset & UINT16_MAX) > INT16_MAX) complement = 1;
    addis(rt, reg::r2, static_cast<int16_t>((TOCoffset >> 16) + complement));
    ld (rt, rt[TOCoffset & UINT16_MAX]);
  } else {
    ld (rt, reg::r2[TOCoffset]);
    emitNop(instr_size_in_bytes);
  }
}

void Assembler::limmediate(const Reg64& rt, int64_t imm64,
                           ImmType immt, bool immMayChange) {
  static_assert(
      std::is_unsigned<
        decltype(HPHP::RuntimeOption::EvalPPC64MinTOCImmSize)>::value,
      "RuntimeOption::EvalPPC64MinTOCImmSize is expected to be unsigned.");
  always_assert(HPHP::RuntimeOption::EvalPPC64MinTOCImmSize <= 64);

  if (immt != ImmType::TocOnly) li64(rt, imm64, immt != ImmType::AnyCompact);
  else li64TOC (rt, imm64, immt, immMayChange);
}

//////////////////////////////////////////////////////////////////////
// Label
//////////////////////////////////////////////////////////////////////

Label::~Label() {
  if (!m_toPatch.empty()) {
    assert(m_a && m_address && "Label had jumps but was never set");
  }
  for (auto& ji : m_toPatch) {
    assert(ji.a->contains(ji.addr));
    ji.a->patchBranch(ji.a->toDestAddress(ji.addr), m_address);
  }
}

void Label::branch(Assembler& a, BranchConditions bc,
                   LinkReg lr, bool addrMayChange) {
  // Only optimize jump if it'll unlikely going to be patched.
  if (m_address) {
    // if diff is 0, then this is for sure going to be patched.
    ssize_t diff = ssize_t(m_address - a.frontier());
    if (diff) {
      // check if an unconditional branch with b can be used
      if (BranchConditions::Always == bc) {
        // unconditional branch
        if (HPHP::jit::deltaFitsBits(diff, 26)) {
          addJump(&a);
          if (LinkReg::Save == lr) a.bl(diff);
          else                     a.b (diff);
          return;
        }
      } else {
        // conditional branch
        if (HPHP::jit::deltaFits(diff, HPHP::sz::word)) {
          BranchParams bp(bc);
          addJump(&a);
          assert(LinkReg::DoNotTouch == lr &&
              "Conditional call is NOT supported.");

          // Special code for overflow handling
          if (bc == BranchConditions::Overflow ||
              bc == BranchConditions::NoOverflow) {
            a.xor(reg::r0, reg::r0, reg::r0,false);
            a.mtspr(Assembler::SpecialReg::XER, reg::r0);
          }
          a.bc (bp.bo(), bp.bi(), diff);
          return;
        }
      }
    }
  }
  // fallback: use CTR to perform absolute branch up to 64 bits
  branchFar(a, bc, lr, ImmType::TocOnly, addrMayChange);
}

void Label::branchFar(Assembler& a,
                  BranchConditions bc,
                  LinkReg lr,
                  ImmType immt,
                  bool immMayChange) {
  // Marking current address for patchAbsolute
  addJump(&a);

  // Use reserved function linkage register
  const ssize_t address = ssize_t(m_address);
  a.limmediate(reg::r12, address, immt, immMayChange);

  // When branching to another context, r12 need to keep the target address
  // to correctly set r2 (TOC reference).
  a.mtctr(reg::r12);

  // Special code for overflow handling
  bool cond = (BranchConditions::Always != bc);
  if (bc == BranchConditions::Overflow || bc == BranchConditions::NoOverflow) {
    a.xor(reg::r0, reg::r0, reg::r0,false);
    a.mtspr(Assembler::SpecialReg::XER, reg::r0);
  } else if (cond && immt != ImmType::AnyCompact) {
    // Unconditional branch (jmp or call) doesn't need this reserve bytes
    a.emitNop(2 * instr_size_in_bytes);
  }

  BranchParams bp(bc);
  if (LinkReg::Save == lr) {
    // call
    a.bctrl();
  } else {
    // jcc
    a.bcctr(bp.bo(), bp.bi(), 0);
  }
}

void Label::asm_label(Assembler& a) {
  assert(!m_address && !m_a && "Label was already set");
  m_a = &a;
  m_address = a.frontier();
}

void Label::addJump(Assembler* a) {
  if (m_address) return;
  JumpInfo info;
  info.a = a;
  info.addr = a->codeBlock.frontier();
  m_toPatch.push_back(info);
}

} // namespace ppc64_asm

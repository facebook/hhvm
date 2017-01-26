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

#include "hphp/util/eh-frame.h"

#include "hphp/util/assertions.h"
#include "hphp/util/trace.h"

#include <folly/ScopeGuard.h>

#include <cstring>
#include <limits>
#include <memory>
#include <vector>

#ifdef _MSC_VER
void __register_frame(const void*) {}
void __deregister_frame(const void*) {}
#else
/*
 * libgcc exports these for registering EH information for dynamically-loaded
 * objects.  The argument is a pointer to data in the format you find in an
 * .eh_frame section.
 */
extern "C" void __register_frame(const void*);
extern "C" void __deregister_frame(const void*);
#endif

namespace HPHP {

TRACE_SET_MOD(ehframe);

///////////////////////////////////////////////////////////////////////////////

using exprlen_t = uint8_t;

void EHFrameWriter::dw_cfa(uint8_t op) {
  // Only allow CFA opcodes when we're writing an entry.
  assertx(m_cie.idx == kInvalidIdx ||
          m_fde != kInvalidIdx);
  write<uint8_t>(op);
}

void EHFrameWriter::dw_op(uint8_t op) {
  // Only allow OP opcodes when we're writing an expression.
  assertx(m_expr != kInvalidIdx);
  FTRACE(1, "         [{:4}] ",
         m_buf->size() - (m_expr + sizeof(exprlen_t)));
  write<uint8_t>(op);
}

void EHFrameWriter::write_uleb(uint64_t value) {
  do {
    uint8_t byte = value & 0x7f;
    value >>= 7;

    if (value != 0) byte |= 0x80;
    write<uint8_t>(byte);
  } while (value != 0);
}

void EHFrameWriter::write_sleb(int64_t value) {
  bool more;
  do {
    uint8_t byte = value & 0x7f;
    value >>= 7;

    more = !(value == 0  && (byte & 0x40) == 0) &&
           !(value == -1 && (byte & 0x40) != 0);

    if (more) byte |= 0x80;
    write<uint8_t>(byte);
  } while (more);
}

///////////////////////////////////////////////////////////////////////////////

EHFrameDesc::~EHFrameDesc() {
  if (m_buf == nullptr) return;

  for_each_fde([] (const FDE& fde) {
    __deregister_frame(fde.get());
  });
}

EHFrameDesc EHFrameWriter::register_and_release() {
  FTRACE(1, "Summary:\n");

  EHFrameDesc eh;
  eh.m_buf = std::move(m_buf);

  assertx(m_cie.idx == 0);
  FTRACE(1, "  [{}] CIE length={}\n", eh.cie().get(), eh.cie().length());

  // Register all the FDEs.
  eh.for_each_fde([&] (const EHFrameDesc::FDE& fde) {
    DEBUG_ONLY auto const cie_off = (fde.get() + 4) - fde.cie_off();

    assertx(eh.cie().get() == cie_off);
    FTRACE(1, "  [{}] FDE length={} cie=[{}]\n",
           fde.get(), fde.length(), cie_off);

    __register_frame(fde.get());
  });

  FTRACE(1, "\n");
  return eh;
}

///////////////////////////////////////////////////////////////////////////////

void EHFrameWriter::begin_cie(uint8_t rip, const void* personality) {
  assertx(m_buf != nullptr && m_buf->size() == 0);
  assertx(m_cie.idx == kInvalidIdx);

  write<uint32_t>(0); // length; to be rewritten later
  write<uint32_t>(0); // CIE_id
  write<uint8_t>(1);  // version

  // Null-terminated augmentation string.
  write<char>('z');
  write<char>('R');
  if (personality) write<char>('P');
  write<char>('\0');

  // Code and data alignment factors.
  write_uleb(1);
  write_sleb(-8);
  m_cie.data_align = -8;

  // Return address register.  (One byte; this appears to be undocumented.)
  write<uint8_t>(rip);

  // Augmentation data length.  We compute this ahead-of-time because it has to
  // be encoded as a ULEB128, which is variable size.
  auto const ad_len = personality
    ? sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uintptr_t)
    : sizeof(uint8_t);
  write_uleb(ad_len);

  DEBUG_ONLY auto const ad_start = m_buf->size();

  // FDE address format.
  write<uint8_t>(DW_EH_PE_absptr);

  // Personality routine address format and address.
  if (personality != nullptr) {
    write<uint8_t>(DW_EH_PE_absptr);
    write<uintptr_t>(uintptr_t(personality));
  }
  assertx(ad_len == m_buf->size() - ad_start);

  FTRACE(1,
    "[   v  ] CIE\n"
    "  CIE_id:                   0\n"
    "  version:                  1\n"
    "  augmentation:             \"{}\"\n"
    "  code_alignment_factor:    1\n"
    "  data_alignment_factor:    -8\n"
    "  return_address_register:  {}\n"
    "  Augmentation data:        0x{:0x} (FDE address encoding: absptr)\n"
    "\n"
    "  Program:\n",
    personality ? "zRP" : "zR",
    rip,
    DW_EH_PE_absptr
  );
}

void EHFrameWriter::end_cie() {
  assertx(m_buf != nullptr && m_buf->size() != 0);
  auto& vec = *m_buf;

  // Patch the length field.  Note that it doesn't count the space for itself.
  auto vp = reinterpret_cast<uint32_t*>(&vec[0]);
  *vp = vec.size() - sizeof(uint32_t);

  m_cie.idx = 0;
  FTRACE(1, "\n");
}

void EHFrameWriter::begin_fde(CodeAddress start) {
  assertx(m_buf != nullptr);
  assertx(m_cie.idx != kInvalidIdx);
  assertx(m_fde == kInvalidIdx);
  auto& vec = *m_buf;

  m_fde = vec.size();

  // Reserve space for the FDE length.
  write<uint32_t>(0);

  // Negative offset to the CIE, relative to this field.
  //
  // It's not completely clear whether all the FDEs need to follow a CIE
  // contiguously, or merely be contiguous with one another, but we use an
  // unsigned value because that's what the docs say (even though libgcc seems
  // to use a signed value).
  auto const cie_off = uint32_t(vec.size() - m_cie.idx);
  write<uint32_t>(cie_off);

  // 8-byte pointer and 8-byte size describing the code range for this FDE.
  write<CodeAddress>(start);
  write<size_t>(0); // patched later

  // Augmentation data length.
  write_uleb(0);

  // Set the CFI table row address to the start address, and reset the CFA
  // offset to what it was at the end of the CIE.
  m_loc = start;
  m_off = m_cie.offset;

  FTRACE(1,
    "[   v  ] FDE\n"
    "  CIE_pointer:              {}\n"
    "  initial_location:         {}\n"
    "\n"
    "  Program:\n",
    cie_off,
    start
  );
}

void EHFrameWriter::end_fde(size_t size) {
  assertx(m_buf != nullptr && m_buf->size() != 0);
  assertx(m_fde != kInvalidIdx && m_fde < m_buf->size());
  auto& vec = *m_buf;

  // Patch the length field.  Note that it doesn't count the space for itself.
  auto vp_len = reinterpret_cast<uint32_t*>(&vec[m_fde]);
  *vp_len = vec.size() - m_fde - sizeof(uint32_t);

  // Patch the PC range field.
  auto const idx = m_fde
    + sizeof(uint32_t)      // FDE length
    + sizeof(int32_t)       // CIE_pointer
    + sizeof(CodeAddress);  // PC start
  auto vp_range = reinterpret_cast<size_t*>(&vec[idx]);
  *vp_range = size;

  // Register that we're no longer writing.
  m_fde = kInvalidIdx;

  FTRACE(1,
    "\n",
    "  address_range:            0x{:0x}\n"
    "\n",
    size
  );
}

void EHFrameWriter::null_fde() {
  assertx(m_fde == kInvalidIdx);
  write<uint32_t>(0);
}

///////////////////////////////////////////////////////////////////////////////

static bool operand_fits(uint64_t operand) {
  return operand == (operand & 0x3f);
}

void EHFrameWriter::advance_loc_to(CodeAddress addr) {
  if (m_loc == nullptr) {
    FTRACE(1, "    set_loc to {}\n", m_loc);
    m_loc = addr;
    dw_cfa(DW_CFA_set_loc);
    write<CodeAddress>(m_loc);
    return;
  }
  assertx(addr > m_loc);

  auto const off = addr - m_loc;
  FTRACE(1, "    advance_loc {} to {}\n", off, addr);

  if (operand_fits(off)) {
    dw_cfa(DW_CFA_advance_loc | off);
  } else if (off == uint8_t(off)) {
    dw_cfa(DW_CFA_advance_loc1);
    write<uint8_t>(off);
  } else if (off == uint16_t(off)) {
    dw_cfa(DW_CFA_advance_loc2);
    write<uint16_t>(off);
  } else if (off == uint32_t(off)) {
    dw_cfa(DW_CFA_advance_loc4);
    write<uint32_t>(off);
  } else {
    dw_cfa(DW_CFA_set_loc);
    write<CodeAddress>(m_loc);
  }
  m_loc = addr;
}

void EHFrameWriter::def_cfa(uint8_t reg, uint64_t off) {
  FTRACE(1, "    def_cfa r{} at offset {}\n", reg, off);

  dw_cfa(DW_CFA_def_cfa);
  write_uleb(reg);
  write_uleb(off);

  if (m_fde == kInvalidIdx) {
    m_cie.offset = off;
  }
  m_off = off;
}

void EHFrameWriter::def_cfa_register(uint8_t reg) {
  FTRACE(1, "    def_cfa_register r{}\n", reg);
  dw_cfa(DW_CFA_def_cfa_register);
  write_uleb(reg);
}

void EHFrameWriter::def_cfa_offset(uint64_t off) {
  FTRACE(1, "    def_cfa_offset {}\n", off);
  dw_cfa(DW_CFA_def_cfa_offset);
  write_uleb(off);
}

void EHFrameWriter::advance_cfa_offset(int64_t off) {
  m_off += off;
  def_cfa_offset(m_off);
}

void EHFrameWriter::same_value(uint8_t reg) {
  FTRACE(1, "    same_value r{}\n", reg);
  dw_cfa(DW_CFA_same_value);
  write_uleb(reg);
}

void EHFrameWriter::offset(uint8_t reg, uint64_t off) {
  FTRACE(1, "    offset r{} at cfa{}\n", reg, off * m_cie.data_align);

  if (operand_fits(reg)) {
    dw_cfa(DW_CFA_offset | reg);
  } else {
    dw_cfa(DW_CFA_offset_extended);
    write_uleb(reg);
  }
  write_uleb(off);
}

void EHFrameWriter::offset_extended_sf(uint8_t reg, int64_t off) {
  FTRACE(1, "    offset r{} at cfa{}\n", reg, off * m_cie.data_align);
  dw_cfa(DW_CFA_offset_extended_sf);
  write_uleb(reg);
  write_sleb(off);
}

void EHFrameWriter::restore(uint8_t reg) {
  FTRACE(1, "    restore r{}\n", reg);
  if (operand_fits(reg)) {
    dw_cfa(DW_CFA_restore | reg);
  } else {
    dw_cfa(DW_CFA_restore_extended);
    write_uleb(reg);
  }
}

void EHFrameWriter::nop() {
  FTRACE(1, "    nop\n");
  dw_cfa(DW_CFA_nop);
}

///////////////////////////////////////////////////////////////////////////////

void EHFrameWriter::begin_expr_impl(uint8_t op, uint8_t reg) {
  assertx(m_buf != nullptr);
  assertx(m_expr == kInvalidIdx);

  dw_cfa(op);
  write_uleb(reg);

  // Reserve space for the expression length and mark the beginning of the
  // expression, so that the size can be patched on end_expression().
  m_expr = m_buf->size();
  write<exprlen_t>(0);
}

void EHFrameWriter::begin_expression(uint8_t reg) {
  FTRACE(1, "    expression r{}\n", reg);
  begin_expr_impl(DW_CFA_expression, reg);
}

void EHFrameWriter::begin_val_expression(uint8_t reg) {
  FTRACE(1, "    val_expression r{}\n", reg);
  begin_expr_impl(DW_CFA_val_expression, reg);
}

void EHFrameWriter::end_expression() {
  assertx(m_buf != nullptr && m_buf->size() != 0);
  assertx(m_expr != kInvalidIdx && m_expr < m_buf->size());
  auto& vec = *m_buf;

  // Patch the length field.  Note that it doesn't count the space for itself.
  auto vp_len = reinterpret_cast<exprlen_t*>(&vec[m_expr]);
  auto const expr_len = vec.size() - m_expr - sizeof(exprlen_t);
  assertx(expr_len < std::numeric_limits<exprlen_t>::max());
  *vp_len = expr_len;

  // Register that we are no longer writing.
  m_expr = kInvalidIdx;
}

///////////////////////////////////////////////////////////////////////////////

void EHFrameWriter::op_consts(int64_t cns) {
  dw_op(DW_OP_consts);
  FTRACE(1, "consts {}\n", cns);
  write_sleb(cns);
}

void EHFrameWriter::op_breg(uint8_t reg, int64_t off) {
  dw_op(DW_OP_bregx);
  FTRACE(1, "bregx r{} {}\n", reg, off);
  write_uleb(reg);
  write_sleb(off);
}

#define IMPL_STACK_OP(name)         \
  void EHFrameWriter::op_##name() { \
    dw_op(DW_OP_##name);            \
    FTRACE(1, #name "\n");          \
  }

IMPL_STACK_OP(dup)
IMPL_STACK_OP(drop)
IMPL_STACK_OP(swap)
IMPL_STACK_OP(deref)

IMPL_STACK_OP(abs)
IMPL_STACK_OP(and)
IMPL_STACK_OP(div)
IMPL_STACK_OP(minus)
IMPL_STACK_OP(mod)
IMPL_STACK_OP(mul)
IMPL_STACK_OP(neg)
IMPL_STACK_OP(not)
IMPL_STACK_OP(or)
IMPL_STACK_OP(plus)

#undef IMPL_STACK_OP

///////////////////////////////////////////////////////////////////////////////

}

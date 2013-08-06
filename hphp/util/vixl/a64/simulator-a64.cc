// Copyright 2013, ARM Limited
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of ARM Limited nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "hphp/util/vixl/a64/simulator-a64.h"

#include <cmath>

namespace vixl {

const Instruction* Simulator::kEndOfSimAddress = nullptr;

Simulator::Simulator(Decoder* decoder, FILE* stream) {
  // Ensure shift operations act as the simulator expects.
  assert((static_cast<int32_t>(-1) >> 1) == -1);
  assert((static_cast<uint32_t>(-1) >> 1) == 0x7FFFFFFF);

  // Setup the decoder.
  decoder_ = decoder;
  decoder_->AppendVisitor(this);

  ResetState();

  // Allocate and setup the simulator stack.
  stack_ = reinterpret_cast<byte*>(malloc(stack_size_));
  stack_limit_ = stack_ + stack_protection_size_;
  byte* tos = stack_ + stack_size_ - stack_protection_size_;
  // The stack pointer must be 16 bytes aligned.
  set_sp(reinterpret_cast<int64_t>(tos) & ~0xfUL);

  stream_ = stream;
  print_disasm_ = new PrintDisassembler(stream_);
  coloured_trace_ = false;
  disasm_trace_ = false;
}


void Simulator::ResetState() {
  // Reset the processor state.
  psr_ = 0;

  // Reset registers to 0.
  pc_ = nullptr;
  pc_modified_ = false;
  for (unsigned i = 0; i < kNumberOfRegisters; i++) {
    set_xreg(i, 0xbadbeef);
  }
  for (unsigned i = 0; i < kNumberOfFPRegisters; i++) {
    // Set FP registers to a value that is NaN in both 32-bit and 64-bit FP.
    set_dreg_bits(i, 0x7ff000007f800001UL);
  }
  // Returning to address 0 exits the Simulator.
  set_lr(reinterpret_cast<int64_t>(kEndOfSimAddress));
}


Simulator::~Simulator() {
  free(stack_);
  // The decoder may outlive the simulator.
  decoder_->RemoveVisitor(print_disasm_);
  delete print_disasm_;
}


void Simulator::Run() {
  while (pc_ != kEndOfSimAddress) {
    ExecuteInstruction();
  }
}


void Simulator::RunFrom(Instruction* first) {
  pc_ = first;
  pc_modified_ = false;
  Run();
}


void Simulator::SetFlags(uint32_t new_flags) {
  assert((new_flags & ~kConditionFlagsMask) == 0);
  psr_ &= ~kConditionFlagsMask;
  // Set the new flags.
  psr_ |= new_flags;
}


const char* Simulator::xreg_names[] = {
"x0",  "x1",  "x2",  "x3",  "x4",  "x5",  "x6",  "x7",
"x8",  "x9",  "x10", "x11", "x12", "x13", "x14", "x15",
"x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
"x24", "x25", "x26", "x27", "x28", "x29", "lr",  "xzr", "sp"};


const char* Simulator::wreg_names[] = {
"w0",  "w1",  "w2",  "w3",  "w4",  "w5",  "w6",  "w7",
"w8",  "w9",  "w10", "w11", "w12", "w13", "w14", "w15",
"w16", "w17", "w18", "w19", "w20", "w21", "w22", "w23",
"w24", "w25", "w26", "w27", "w28", "w29", "w30", "wzr", "wsp"};

const char* Simulator::sreg_names[] = {
"s0",  "s1",  "s2",  "s3",  "s4",  "s5",  "s6",  "s7",
"s8",  "s9",  "s10", "s11", "s12", "s13", "s14", "s15",
"s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",
"s24", "s25", "s26", "s27", "s28", "s29", "s30", "s31"};

const char* Simulator::dreg_names[] = {
"d0",  "d1",  "d2",  "d3",  "d4",  "d5",  "d6",  "d7",
"d8",  "d9",  "d10", "d11", "d12", "d13", "d14", "d15",
"d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
"d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31"};

const char* Simulator::vreg_names[] = {
"v0",  "v1",  "v2",  "v3",  "v4",  "v5",  "v6",  "v7",
"v8",  "v9",  "v10", "v11", "v12", "v13", "v14", "v15",
"v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
"v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"};



const char* Simulator::WRegNameForCode(unsigned code, Reg31Mode mode) {
  assert(code < kNumberOfRegisters);
  // If the code represents the stack pointer, index the name after zr.
  if ((code == kZeroRegCode) && (mode == Reg31IsStackPointer)) {
    code = kZeroRegCode + 1;
  }
  return wreg_names[code];
}


const char* Simulator::XRegNameForCode(unsigned code, Reg31Mode mode) {
  assert(code < kNumberOfRegisters);
  // If the code represents the stack pointer, index the name after zr.
  if ((code == kZeroRegCode) && (mode == Reg31IsStackPointer)) {
    code = kZeroRegCode + 1;
  }
  return xreg_names[code];
}


const char* Simulator::SRegNameForCode(unsigned code) {
  assert(code < kNumberOfFPRegisters);
  return sreg_names[code];
}


const char* Simulator::DRegNameForCode(unsigned code) {
  assert(code < kNumberOfFPRegisters);
  return dreg_names[code];
}


const char* Simulator::VRegNameForCode(unsigned code) {
  assert(code < kNumberOfFPRegisters);
  return vreg_names[code];
}


// Helpers ---------------------------------------------------------------------
int64_t Simulator::AddWithCarry(unsigned reg_size,
                                bool set_flags,
                                int64_t src1,
                                int64_t src2,
                                int64_t carry_in) {
  assert((carry_in == 0) || (carry_in == 1));
  assert((reg_size == kXRegSize) || (reg_size == kWRegSize));

  uint64_t u1, u2;
  int64_t result;
  int64_t signed_sum = src1 + src2 + carry_in;

  uint32_t N, Z, C, V;

  if (reg_size == kWRegSize) {
    u1 = static_cast<uint64_t>(src1) & kWRegMask;
    u2 = static_cast<uint64_t>(src2) & kWRegMask;

    result = signed_sum & kWRegMask;
    // Compute the C flag by comparing the sum to the max unsigned integer.
    C = CFlag * (((kWMaxUInt - u1) < (u2 + carry_in)) ||
                 ((kWMaxUInt - u1 - carry_in) < u2));
    // Overflow iff the sign bit is the same for the two inputs and different
    // for the result.
    int64_t s_src1 = src1 << (kXRegSize - kWRegSize);
    int64_t s_src2 = src2 << (kXRegSize - kWRegSize);
    int64_t s_result = result << (kXRegSize - kWRegSize);
    V = VFlag * (((s_src1 ^ s_src2) >= 0) && ((s_src1 ^ s_result) < 0));

  } else {
    u1 = static_cast<uint64_t>(src1);
    u2 = static_cast<uint64_t>(src2);

    result = signed_sum;
    // Compute the C flag by comparing the sum to the max unsigned integer.
    C = CFlag * (((kXMaxUInt - u1) < (u2 + carry_in)) ||
                 ((kXMaxUInt - u1 - carry_in) < u2));
    // Overflow iff the sign bit is the same for the two inputs and different
    // for the result.
    V = VFlag * (((src1 ^ src2) >= 0) && ((src1 ^ result) < 0));
  }

  N = CalcNFlag(result, reg_size);
  Z = CalcZFlag(result);

  if (set_flags) SetFlags(N | Z | C | V);
  return result;
}


int64_t Simulator::ShiftOperand(unsigned reg_size,
                                int64_t value,
                                Shift shift_type,
                                unsigned amount) {
  if (amount == 0) {
    return value;
  }
  int64_t mask = reg_size == kXRegSize ? kXRegMask : kWRegMask;
  switch (shift_type) {
    case LSL:
      return (value << amount) & mask;
    case LSR:
      return static_cast<uint64_t>(value) >> amount;
    case ASR: {
      // Shift used to restore the sign.
      unsigned s_shift = kXRegSize - reg_size;
      // Value with its sign restored.
      int64_t s_value = (value << s_shift) >> s_shift;
      return (s_value >> amount) & mask;
    }
    case ROR: {
      if (reg_size == kWRegSize) {
        value &= kWRegMask;
      }
      return (static_cast<uint64_t>(value) >> amount) |
             ((value & ((1L << amount) - 1L)) << (reg_size - amount));
    }
    default:
      not_implemented();
      return 0;
  }
}


int64_t Simulator::ExtendValue(unsigned reg_size,
                               int64_t value,
                               Extend extend_type,
                               unsigned left_shift) {
  switch (extend_type) {
    case UXTB:
      value &= kByteMask;
      break;
    case UXTH:
      value &= kHalfWordMask;
      break;
    case UXTW:
      value &= kWordMask;
      break;
    case SXTB:
      value = (value << 56) >> 56;
      break;
    case SXTH:
      value = (value << 48) >> 48;
      break;
    case SXTW:
      value = (value << 32) >> 32;
      break;
    case UXTX:
    case SXTX:
      break;
    default:
      not_reached();
  }
  int64_t mask = (reg_size == kXRegSize) ? kXRegMask : kWRegMask;
  return (value << left_shift) & mask;
}


void Simulator::FPCompare(double val0, double val1) {
  unsigned new_flags;

  if ((isnan(val0) != 0) || (isnan(val1) != 0)) {
    new_flags = CVFlag;
  } else {
    if (val0 < val1) {
      new_flags = NFlag;
    } else {
      new_flags = CFlag;
      if (val0 == val1) {
        new_flags |= ZFlag;
      }
    }
  }
  SetFlags(new_flags);
}


void Simulator::PrintFlags(bool print_all) {
  static bool first_run = true;
  static uint32_t last_flags;

  // Define some colour codes to use for the register dump.
  // TODO: Find a more elegant way of defining these.
  char const * const clr_normal     = (coloured_trace_) ? ("\033[m") : ("");
  char const * const clr_flag_name  = (coloured_trace_) ? ("\033[1;30m") : ("");
  char const * const clr_flag_value = (coloured_trace_) ? ("\033[1;37m") : ("");

  if (print_all || first_run || (last_flags != nzcv())) {
    fprintf(stream_, "# %sFLAGS: %sN:%1d Z:%1d C:%1d V:%1d%s\n",
            clr_flag_name,
            clr_flag_value,
            N(), Z(), C(), V(),
            clr_normal);
  }
  last_flags = nzcv();
  first_run = false;
}


void Simulator::PrintRegisters(bool print_all_regs) {
  static bool first_run = true;
  static int64_t last_regs[kNumberOfRegisters];

  // Define some colour codes to use for the register dump.
  // TODO: Find a more elegant way of defining these.
  char const * const clr_normal    = (coloured_trace_) ? ("\033[m") : ("");
  char const * const clr_reg_name  = (coloured_trace_) ? ("\033[1;34m") : ("");
  char const * const clr_reg_value = (coloured_trace_) ? ("\033[1;36m") : ("");

  for (unsigned i = 0; i < kNumberOfRegisters; i++) {
    if (print_all_regs || first_run || (last_regs[i] != registers_[i].x)) {
      fprintf(stream_,
              "# %s%4s:%s 0x%016" PRIx64 "%s\n",
              clr_reg_name,
              XRegNameForCode(i, Reg31IsStackPointer),
              clr_reg_value,
              registers_[i].x,
              clr_normal);
    }
    // Cache the new register value so the next run can detect any changes.
    last_regs[i] = registers_[i].x;
  }
  first_run = false;
}


void Simulator::PrintFPRegisters(bool print_all_regs) {
  static bool first_run = true;
  static uint64_t last_regs[kNumberOfFPRegisters];

  // Define some colour codes to use for the register dump.
  // TODO: Find a more elegant way of defining these.
  char const * const clr_normal    = (coloured_trace_) ? ("\033[m") : ("");
  char const * const clr_reg_name  = (coloured_trace_) ? ("\033[1;33m") : ("");
  char const * const clr_reg_value = (coloured_trace_) ? ("\033[1;35m") : ("");

  // Print as many rows of registers as necessary, keeping each individual
  // register in the same column each time (to make it easy to visually scan
  // for changes).
  for (unsigned i = 0; i < kNumberOfFPRegisters; i++) {
    if (print_all_regs || first_run ||
        (last_regs[i] != double_to_rawbits(fpregisters_[i].d))) {
      fprintf(stream_,
              "# %s %4s:%s 0x%016" PRIx64 "%s (%s%s:%s %g%s %s:%s %g%s)\n",
              clr_reg_name,
              VRegNameForCode(i),
              clr_reg_value,
              double_to_rawbits(fpregisters_[i].d),
              clr_normal,
              clr_reg_name,
              DRegNameForCode(i),
              clr_reg_value,
              fpregisters_[i].d,
              clr_reg_name,
              SRegNameForCode(i),
              clr_reg_value,
              fpregisters_[i].s,
              clr_normal);
    }
    // Cache the new register value so the next run can detect any changes.
    last_regs[i] = double_to_rawbits(fpregisters_[i].d);
  }
  first_run = false;
}


void Simulator::PrintProcessorState() {
  PrintFlags();
  PrintRegisters();
  PrintFPRegisters();
}


// Visitors---------------------------------------------------------------------

void Simulator::VisitUnknown(Instruction* instr) {
  printf("Unknown instruction at 0x%p: 0x%08" PRIx32 "\n",
         reinterpret_cast<void*>(instr), instr->InstructionBits());
  not_implemented();
}


void Simulator::VisitPCRelAddressing(Instruction* instr) {
  switch (instr->Mask(PCRelAddressingMask)) {
    case ADR:
      set_reg(kXRegSize,
              instr->Rd(),
              reinterpret_cast<int64_t>(instr->ImmPCOffsetTarget()));
      break;
    case ADRP:  // Not implemented in the assembler.
      not_implemented();
      break;
    default:
      not_reached();
  }
}


void Simulator::VisitUnconditionalBranch(Instruction* instr) {
  switch (instr->Mask(UnconditionalBranchMask)) {
    case BL:
      set_lr(reinterpret_cast<int64_t>(instr->NextInstruction()));
      // Fall through.
    case B:
      set_pc(instr->ImmPCOffsetTarget());
      break;
    default: not_reached();
  }
}


void Simulator::VisitConditionalBranch(Instruction* instr) {
  assert(instr->Mask(ConditionalBranchMask) == B_cond);
  if (ConditionPassed(static_cast<Condition>(instr->ConditionBranch()))) {
    set_pc(instr->ImmPCOffsetTarget());
  }
}


void Simulator::VisitUnconditionalBranchToRegister(Instruction* instr) {
  Instruction* target = Instruction::Cast(xreg(instr->Rn()));

  switch (instr->Mask(UnconditionalBranchToRegisterMask)) {
    case BLR:
      set_lr(reinterpret_cast<int64_t>(instr->NextInstruction()));
      // Fall through.
    case BR:
    case RET: set_pc(target); break;
    default: not_reached();
  }
}


void Simulator::VisitTestBranch(Instruction* instr) {
  unsigned bit_pos = (instr->ImmTestBranchBit5() << 5) |
                     instr->ImmTestBranchBit40();
  bool take_branch = ((xreg(instr->Rt()) & (1UL << bit_pos)) == 0);
  switch (instr->Mask(TestBranchMask)) {
    case TBZ: break;
    case TBNZ: take_branch = !take_branch; break;
    default: not_implemented();
  }
  if (take_branch) {
    set_pc(instr->ImmPCOffsetTarget());
  }
}


void Simulator::VisitCompareBranch(Instruction* instr) {
  unsigned rt = instr->Rt();
  bool take_branch = false;
  switch (instr->Mask(CompareBranchMask)) {
    case CBZ_w: take_branch = (wreg(rt) == 0); break;
    case CBZ_x: take_branch = (xreg(rt) == 0); break;
    case CBNZ_w: take_branch = (wreg(rt) != 0); break;
    case CBNZ_x: take_branch = (xreg(rt) != 0); break;
    default: not_implemented();
  }
  if (take_branch) {
    set_pc(instr->ImmPCOffsetTarget());
  }
}


void Simulator::AddSubHelper(Instruction* instr, int64_t op2) {
  unsigned reg_size = instr->SixtyFourBits() ? kXRegSize : kWRegSize;
  bool set_flags = instr->FlagsUpdate();
  int64_t new_val = 0;
  Instr operation = instr->Mask(AddSubOpMask);

  switch (operation) {
    case ADD:
    case ADDS: {
      new_val = AddWithCarry(reg_size,
                             set_flags,
                             reg(reg_size, instr->Rn(), instr->RnMode()),
                             op2);
      break;
    }
    case SUB:
    case SUBS: {
      new_val = AddWithCarry(reg_size,
                             set_flags,
                             reg(reg_size, instr->Rn(), instr->RnMode()),
                             ~op2,
                             1);
      break;
    }
    default: not_reached();
  }

  set_reg(reg_size, instr->Rd(), new_val, instr->RdMode());
}


void Simulator::VisitAddSubShifted(Instruction* instr) {
  unsigned reg_size = instr->SixtyFourBits() ? kXRegSize : kWRegSize;
  int64_t op2 = ShiftOperand(reg_size,
                             reg(reg_size, instr->Rm()),
                             static_cast<Shift>(instr->ShiftDP()),
                             instr->ImmDPShift());
  AddSubHelper(instr, op2);
}


void Simulator::VisitAddSubImmediate(Instruction* instr) {
  int64_t op2 = instr->ImmAddSub() << ((instr->ShiftAddSub() == 1) ? 12 : 0);
  AddSubHelper(instr, op2);
}


void Simulator::VisitAddSubExtended(Instruction* instr) {
  unsigned reg_size = instr->SixtyFourBits() ? kXRegSize : kWRegSize;
  int64_t op2 = ExtendValue(reg_size,
                            reg(reg_size, instr->Rm()),
                            static_cast<Extend>(instr->ExtendMode()),
                            instr->ImmExtendShift());
  AddSubHelper(instr, op2);
}


void Simulator::VisitAddSubWithCarry(Instruction* instr) {
  unsigned reg_size = instr->SixtyFourBits() ? kXRegSize : kWRegSize;
  int64_t op2 = reg(reg_size, instr->Rm());
  int64_t new_val;

  if ((instr->Mask(AddSubOpMask) == SUB) || instr->Mask(AddSubOpMask) == SUBS) {
    op2 = ~op2;
  }

  new_val = AddWithCarry(reg_size,
                         instr->FlagsUpdate(),
                         reg(reg_size, instr->Rn()),
                         op2,
                         C());

  set_reg(reg_size, instr->Rd(), new_val);
}


void Simulator::VisitLogicalShifted(Instruction* instr) {
  unsigned reg_size = instr->SixtyFourBits() ? kXRegSize : kWRegSize;
  Shift shift_type = static_cast<Shift>(instr->ShiftDP());
  unsigned shift_amount = instr->ImmDPShift();
  int64_t op2 = ShiftOperand(reg_size, reg(reg_size, instr->Rm()), shift_type,
                             shift_amount);
  if (instr->Mask(NOT) == NOT) {
    op2 = ~op2;
  }
  LogicalHelper(instr, op2);
}


void Simulator::VisitLogicalImmediate(Instruction* instr) {
  LogicalHelper(instr, instr->ImmLogical());
}


void Simulator::LogicalHelper(Instruction* instr, int64_t op2) {
  unsigned reg_size = instr->SixtyFourBits() ? kXRegSize : kWRegSize;
  int64_t op1 = reg(reg_size, instr->Rn());
  int64_t result = 0;
  bool update_flags = false;

  // Switch on the logical operation, stripping out the NOT bit, as it has a
  // different meaning for logical immediate instructions.
  switch (instr->Mask(LogicalOpMask & ~NOT)) {
    case ANDS: update_flags = true;  // Fall through.
    case AND: result = op1 & op2; break;
    case ORR: result = op1 | op2; break;
    case EOR: result = op1 ^ op2; break;
    default:
      not_implemented();
  }

  if (update_flags) {
    SetFlags(CalcNFlag(result, reg_size) | CalcZFlag(result));
  }

  set_reg(reg_size, instr->Rd(), result, instr->RdMode());
}


void Simulator::VisitConditionalCompareRegister(Instruction* instr) {
  unsigned reg_size = instr->SixtyFourBits() ? kXRegSize : kWRegSize;
  ConditionalCompareHelper(instr, reg(reg_size, instr->Rm()));
}


void Simulator::VisitConditionalCompareImmediate(Instruction* instr) {
  ConditionalCompareHelper(instr, instr->ImmCondCmp());
}


void Simulator::ConditionalCompareHelper(Instruction* instr, int64_t op2) {
  unsigned reg_size = instr->SixtyFourBits() ? kXRegSize : kWRegSize;
  int64_t op1 = reg(reg_size, instr->Rn());

  if (ConditionPassed(static_cast<Condition>(instr->Condition()))) {
    // If the condition passes, set the status flags to the result of comparing
    // the operands.
    if (instr->Mask(ConditionalCompareMask) == CCMP) {
      AddWithCarry(reg_size, true, op1, ~op2, 1);
    } else {
      assert(instr->Mask(ConditionalCompareMask) == CCMN);
      AddWithCarry(reg_size, true, op1, op2, 0);
    }
  } else {
    // If the condition fails, set the status flags to the nzcv immediate.
    SetFlags(instr->Nzcv() << Flags_offset);
  }
}


void Simulator::VisitLoadStoreUnsignedOffset(Instruction* instr) {
  int offset = instr->ImmLSUnsigned() << instr->SizeLS();
  LoadStoreHelper(instr, offset, Offset);
}


void Simulator::VisitLoadStoreUnscaledOffset(Instruction* instr) {
  LoadStoreHelper(instr, instr->ImmLS(), Offset);
}


void Simulator::VisitLoadStorePreIndex(Instruction* instr) {
  LoadStoreHelper(instr, instr->ImmLS(), PreIndex);
}


void Simulator::VisitLoadStorePostIndex(Instruction* instr) {
  LoadStoreHelper(instr, instr->ImmLS(), PostIndex);
}


void Simulator::VisitLoadStoreRegisterOffset(Instruction* instr) {
  Extend ext = static_cast<Extend>(instr->ExtendMode());
  assert((ext == UXTW) || (ext == UXTX) || (ext == SXTW) || (ext == SXTX));
  unsigned shift_amount = instr->ImmShiftLS() * instr->SizeLS();

  int64_t offset = ExtendValue(kXRegSize, xreg(instr->Rm()), ext,
                               shift_amount);
  LoadStoreHelper(instr, offset, Offset);
}


void Simulator::LoadStoreHelper(Instruction* instr,
                                int64_t offset,
                                AddrMode addrmode) {
  unsigned srcdst = instr->Rt();
  uint8_t* address = AddressModeHelper(instr->Rn(), offset, addrmode);
  int num_bytes = 1 << instr->SizeLS();

  LoadStoreOp op = static_cast<LoadStoreOp>(instr->Mask(LoadStoreOpMask));
  switch (op) {
    case LDRB_w:
    case LDRH_w:
    case LDR_w:
    case LDR_x: set_xreg(srcdst, MemoryRead(address, num_bytes)); break;
    case STRB_w:
    case STRH_w:
    case STR_w:
    case STR_x: MemoryWrite(address, xreg(srcdst), num_bytes); break;
    case LDRSB_w: {
      set_wreg(srcdst, ExtendValue(kWRegSize, MemoryRead8(address), SXTB));
      break;
    }
    case LDRSB_x: {
      set_xreg(srcdst, ExtendValue(kXRegSize, MemoryRead8(address), SXTB));
      break;
    }
    case LDRSH_w: {
      set_wreg(srcdst, ExtendValue(kWRegSize, MemoryRead16(address), SXTH));
      break;
    }
    case LDRSH_x: {
      set_xreg(srcdst, ExtendValue(kXRegSize, MemoryRead16(address), SXTH));
      break;
    }
    case LDRSW_x: {
      set_xreg(srcdst, ExtendValue(kXRegSize, MemoryRead32(address), SXTW));
      break;
    }
    case LDR_s: set_sreg(srcdst, MemoryReadFP32(address)); break;
    case LDR_d: set_dreg(srcdst, MemoryReadFP64(address)); break;
    case STR_s: MemoryWriteFP32(address, sreg(srcdst)); break;
    case STR_d: MemoryWriteFP64(address, dreg(srcdst)); break;
    default: not_implemented();
  }
}


void Simulator::VisitLoadStorePairOffset(Instruction* instr) {
  LoadStorePairHelper(instr, Offset);
}


void Simulator::VisitLoadStorePairPreIndex(Instruction* instr) {
  LoadStorePairHelper(instr, PreIndex);
}


void Simulator::VisitLoadStorePairPostIndex(Instruction* instr) {
  LoadStorePairHelper(instr, PostIndex);
}


void Simulator::VisitLoadStorePairNonTemporal(Instruction* instr) {
  LoadStorePairHelper(instr, Offset);
}


void Simulator::LoadStorePairHelper(Instruction* instr,
                                    AddrMode addrmode) {
  unsigned rt = instr->Rt();
  unsigned rt2 = instr->Rt2();
  int offset = instr->ImmLSPair() << instr->SizeLSPair();
  uint8_t* address = AddressModeHelper(instr->Rn(), offset, addrmode);

  LoadStorePairOp op =
    static_cast<LoadStorePairOp>(instr->Mask(LoadStorePairMask));

  // 'rt' and 'rt2' can only be aliased for stores.
  assert(((op & LoadStorePairLBit) == 0) || (rt != rt2));

  switch (op) {
    case LDP_w: {
      set_wreg(rt, MemoryRead32(address));
      set_wreg(rt2, MemoryRead32(address + kWRegSizeInBytes));
      break;
    }
    case LDP_s: {
      set_sreg(rt, MemoryReadFP32(address));
      set_sreg(rt2, MemoryReadFP32(address + kSRegSizeInBytes));
      break;
    }
    case LDP_x: {
      set_xreg(rt, MemoryRead64(address));
      set_xreg(rt2, MemoryRead64(address + kXRegSizeInBytes));
      break;
    }
    case LDP_d: {
      set_dreg(rt, MemoryReadFP64(address));
      set_dreg(rt2, MemoryReadFP64(address + kDRegSizeInBytes));
      break;
    }
    case LDPSW_x: {
      set_xreg(rt, ExtendValue(kXRegSize, MemoryRead32(address), SXTW));
      set_xreg(rt2, ExtendValue(kXRegSize,
               MemoryRead32(address + kWRegSizeInBytes), SXTW));
      break;
    }
    case STP_w: {
      MemoryWrite32(address, wreg(rt));
      MemoryWrite32(address + kWRegSizeInBytes, wreg(rt2));
      break;
    }
    case STP_s: {
      MemoryWriteFP32(address, sreg(rt));
      MemoryWriteFP32(address + kSRegSizeInBytes, sreg(rt2));
      break;
    }
    case STP_x: {
      MemoryWrite64(address, xreg(rt));
      MemoryWrite64(address + kXRegSizeInBytes, xreg(rt2));
      break;
    }
    case STP_d: {
      MemoryWriteFP64(address, dreg(rt));
      MemoryWriteFP64(address + kDRegSizeInBytes, dreg(rt2));
      break;
    }
    default: not_reached();
  }
}


void Simulator::VisitLoadLiteral(Instruction* instr) {
  uint8_t* address = instr->LiteralAddress();
  unsigned rt = instr->Rt();

  switch (instr->Mask(LoadLiteralMask)) {
    case LDR_w_lit: set_wreg(rt, MemoryRead32(address));  break;
    case LDR_x_lit: set_xreg(rt, MemoryRead64(address));  break;
    case LDR_s_lit: set_sreg(rt, MemoryReadFP32(address));  break;
    case LDR_d_lit: set_dreg(rt, MemoryReadFP64(address));  break;
    default: not_reached();
  }
}


uint8_t* Simulator::AddressModeHelper(unsigned addr_reg,
                                      int64_t offset,
                                      AddrMode addrmode) {
  uint64_t address = xreg(addr_reg, Reg31IsStackPointer);
  assert((sizeof(uintptr_t) == kXRegSizeInBytes) ||
         (address < 0x100000000UL));
  if ((addr_reg == 31) && ((address % 16) != 0)) {
    // When the base register is SP the stack pointer is required to be
    // quadword aligned prior to the address calculation and write-backs.
    // Misalignment will cause a stack alignment fault.
    ALIGNMENT_EXCEPTION();
  }
  if ((addrmode == PreIndex) || (addrmode == PostIndex)) {
    assert(offset != 0);
    set_xreg(addr_reg, address + offset, Reg31IsStackPointer);
  }

  if ((addrmode == Offset) || (addrmode == PreIndex)) {
    address += offset;
  }

  return reinterpret_cast<uint8_t*>(address);
}


uint64_t Simulator::MemoryRead(const uint8_t* address, unsigned num_bytes) {
  assert(address != nullptr);
  assert((num_bytes > 0) && (num_bytes <= sizeof(uint64_t)));
  uint64_t read = 0;
  memcpy(&read, address, num_bytes);
  return read;
}


uint8_t Simulator::MemoryRead8(uint8_t* address) {
  return MemoryRead(address, sizeof(uint8_t));
}


uint16_t Simulator::MemoryRead16(uint8_t* address) {
  return MemoryRead(address, sizeof(uint16_t));
}


uint32_t Simulator::MemoryRead32(uint8_t* address) {
  return MemoryRead(address, sizeof(uint32_t));
}


float Simulator::MemoryReadFP32(uint8_t* address) {
  return rawbits_to_float(MemoryRead32(address));
}


uint64_t Simulator::MemoryRead64(uint8_t* address) {
  return MemoryRead(address, sizeof(uint64_t));
}


double Simulator::MemoryReadFP64(uint8_t* address) {
  return rawbits_to_double(MemoryRead64(address));
}


void Simulator::MemoryWrite(uint8_t* address,
                            uint64_t value,
                            unsigned num_bytes) {
  assert(address != nullptr);
  assert((num_bytes > 0) && (num_bytes <= sizeof(uint64_t)));
  memcpy(address, &value, num_bytes);
}


void Simulator::MemoryWrite32(uint8_t* address, uint32_t value) {
  MemoryWrite(address, value, sizeof(uint32_t));
}


void Simulator::MemoryWriteFP32(uint8_t* address, float value) {
  MemoryWrite32(address, float_to_rawbits(value));
}


void Simulator::MemoryWrite64(uint8_t* address, uint64_t value) {
  MemoryWrite(address, value, sizeof(uint64_t));
}


void Simulator::MemoryWriteFP64(uint8_t* address, double value) {
  MemoryWrite64(address, double_to_rawbits(value));
}


void Simulator::VisitMoveWideImmediate(Instruction* instr) {
  MoveWideImmediateOp mov_op =
    static_cast<MoveWideImmediateOp>(instr->Mask(MoveWideImmediateMask));
  int64_t new_xn_val = 0;

  bool is_64_bits = instr->SixtyFourBits() == 1;
  // Shift is limited for W operations.
  assert(is_64_bits || (instr->ShiftMoveWide() < 2));

  // Get the shifted immediate.
  int64_t shift = instr->ShiftMoveWide() * 16;
  int64_t shifted_imm16 = instr->ImmMoveWide() << shift;

  // Compute the new value.
  switch (mov_op) {
    case MOVN_w:
    case MOVN_x: {
        new_xn_val = ~shifted_imm16;
        if (!is_64_bits) new_xn_val &= kWRegMask;
      break;
    }
    case MOVK_w:
    case MOVK_x: {
        unsigned reg_code = instr->Rd();
        int64_t prev_xn_val = is_64_bits ? xreg(reg_code)
                                         : wreg(reg_code);
        new_xn_val = (prev_xn_val & ~(0xffffL << shift)) | shifted_imm16;
      break;
    }
    case MOVZ_w:
    case MOVZ_x: {
        new_xn_val = shifted_imm16;
      break;
    }
    default:
      not_reached();
  }

  // Update the destination register.
  set_xreg(instr->Rd(), new_xn_val);
}


void Simulator::VisitConditionalSelect(Instruction* instr) {
  uint64_t new_val = xreg(instr->Rn());

  if (ConditionFailed(static_cast<Condition>(instr->Condition()))) {
    new_val = xreg(instr->Rm());
    switch (instr->Mask(ConditionalSelectMask)) {
      case CSEL_w:
      case CSEL_x: break;
      case CSINC_w:
      case CSINC_x: new_val++; break;
      case CSINV_w:
      case CSINV_x: new_val = ~new_val; break;
      case CSNEG_w:
      case CSNEG_x: new_val = -new_val; break;
      default: not_implemented();
    }
  }
  unsigned reg_size = instr->SixtyFourBits() ? kXRegSize : kWRegSize;
  set_reg(reg_size, instr->Rd(), new_val);
}


void Simulator::VisitDataProcessing1Source(Instruction* instr) {
  unsigned dst = instr->Rd();
  unsigned src = instr->Rn();

  switch (instr->Mask(DataProcessing1SourceMask)) {
    case RBIT_w: set_wreg(dst, ReverseBits(wreg(src), kWRegSize)); break;
    case RBIT_x: set_xreg(dst, ReverseBits(xreg(src), kXRegSize)); break;
    case REV16_w: set_wreg(dst, ReverseBytes(wreg(src), Reverse16)); break;
    case REV16_x: set_xreg(dst, ReverseBytes(xreg(src), Reverse16)); break;
    case REV_w: set_wreg(dst, ReverseBytes(wreg(src), Reverse32)); break;
    case REV32_x: set_xreg(dst, ReverseBytes(xreg(src), Reverse32)); break;
    case REV_x: set_xreg(dst, ReverseBytes(xreg(src), Reverse64)); break;
    case CLZ_w: set_wreg(dst, CountLeadingZeros(wreg(src), kWRegSize)); break;
    case CLZ_x: set_xreg(dst, CountLeadingZeros(xreg(src), kXRegSize)); break;
    case CLS_w: {
      set_wreg(dst, CountLeadingSignBits(wreg(src), kWRegSize));
      break;
    }
    case CLS_x: {
      set_xreg(dst, CountLeadingSignBits(xreg(src), kXRegSize));
      break;
    }
    default: not_implemented();
  }
}


uint64_t Simulator::ReverseBits(uint64_t value, unsigned num_bits) {
  assert((num_bits == kWRegSize) || (num_bits == kXRegSize));
  uint64_t result = 0;
  for (unsigned i = 0; i < num_bits; i++) {
    result = (result << 1) | (value & 1);
    value >>= 1;
  }
  return result;
}


uint64_t Simulator::ReverseBytes(uint64_t value, ReverseByteMode mode) {
  // Split the 64-bit value into an 8-bit array, where b[0] is the least
  // significant byte, and b[7] is the most significant.
  uint8_t bytes[8];
  uint64_t mask = 0xff00000000000000UL;
  for (int i = 7; i >= 0; i--) {
    bytes[i] = (value & mask) >> (i * 8);
    mask >>= 8;
  }

  // Permutation tables for REV instructions.
  //  permute_table[Reverse16] is used by REV16_x, REV16_w
  //  permute_table[Reverse32] is used by REV32_x, REV_w
  //  permute_table[Reverse64] is used by REV_x
  assert((Reverse16 == 0) && (Reverse32 == 1) && (Reverse64 == 2));
  static const uint8_t permute_table[3][8] = { {6, 7, 4, 5, 2, 3, 0, 1},
                                               {4, 5, 6, 7, 0, 1, 2, 3},
                                               {0, 1, 2, 3, 4, 5, 6, 7} };
  uint64_t result = 0;
  for (int i = 0; i < 8; i++) {
    result <<= 8;
    result |= bytes[permute_table[mode][i]];
  }
  return result;
}


void Simulator::VisitDataProcessing2Source(Instruction* instr) {
  Shift shift_op = NO_SHIFT;
  int64_t result = 0;
  switch (instr->Mask(DataProcessing2SourceMask)) {
    case SDIV_w: result = wreg(instr->Rn()) / wreg(instr->Rm()); break;
    case SDIV_x: result = xreg(instr->Rn()) / xreg(instr->Rm()); break;
    case UDIV_w: {
      uint32_t rn = static_cast<uint32_t>(wreg(instr->Rn()));
      uint32_t rm = static_cast<uint32_t>(wreg(instr->Rm()));
      result = rn / rm;
      break;
    }
    case UDIV_x: {
      uint64_t rn = static_cast<uint64_t>(xreg(instr->Rn()));
      uint64_t rm = static_cast<uint64_t>(xreg(instr->Rm()));
      result = rn / rm;
      break;
    }
    case LSLV_w:
    case LSLV_x: shift_op = LSL; break;
    case LSRV_w:
    case LSRV_x: shift_op = LSR; break;
    case ASRV_w:
    case ASRV_x: shift_op = ASR; break;
    case RORV_w:
    case RORV_x: shift_op = ROR; break;
    default: not_implemented();
  }

  unsigned reg_size = instr->SixtyFourBits() ? kXRegSize : kWRegSize;
  if (shift_op != NO_SHIFT) {
    // Shift distance encoded in the least-significant five/six bits of the
    // register.
    int mask = (instr->SixtyFourBits() == 1) ? 0x3f : 0x1f;
    unsigned shift = wreg(instr->Rm()) & mask;
    result = ShiftOperand(reg_size, reg(reg_size, instr->Rn()), shift_op,
                          shift);
  }
  set_reg(reg_size, instr->Rd(), result);
}


// The algorithm used is adapted from the one described in section 8.2 of
//   Hacker's Delight, by Henry S. Warren, Jr.
// It assumes that a right shift on a signed integer is an arithmetic shift.
static int64_t MultiplyHighSigned(int64_t u, int64_t v) {
  uint64_t u0, v0, w0;
  int64_t u1, v1, w1, w2, t;

  u0 = u & 0xffffffffL;
  u1 = u >> 32;
  v0 = v & 0xffffffffL;
  v1 = v >> 32;

  w0 = u0 * v0;
  t = u1 * v0 + (w0 >> 32);
  w1 = t & 0xffffffffL;
  w2 = t >> 32;
  w1 = u0 * v1 + w1;

  return u1 * v1 + w2 + (w1 >> 32);
}


void Simulator::VisitDataProcessing3Source(Instruction* instr) {
  unsigned reg_size = instr->SixtyFourBits() ? kXRegSize : kWRegSize;

  int64_t result = 0;
  uint64_t rn;
  uint64_t rm;
  switch (instr->Mask(DataProcessing3SourceMask)) {
    case MADD_w:
    case MADD_x:
      result = xreg(instr->Ra()) + (xreg(instr->Rn()) * xreg(instr->Rm()));
      break;
    case MSUB_w:
    case MSUB_x:
      result = xreg(instr->Ra()) - (xreg(instr->Rn()) * xreg(instr->Rm()));
      break;
    case SMADDL_x:
      result = xreg(instr->Ra()) + (wreg(instr->Rn()) * wreg(instr->Rm()));
      break;
    case SMSUBL_x:
      result = xreg(instr->Ra()) - (wreg(instr->Rn()) * wreg(instr->Rm()));
      break;
    case UMADDL_x:
      rn = static_cast<uint32_t>(wreg(instr->Rn()));
      rm = static_cast<uint32_t>(wreg(instr->Rm()));
      result = xreg(instr->Ra()) + (rn * rm);
      break;
    case UMSUBL_x:
      rn = static_cast<uint32_t>(wreg(instr->Rn()));
      rm = static_cast<uint32_t>(wreg(instr->Rm()));
      result = xreg(instr->Ra()) - (rn * rm);
      break;
    case SMULH_x:
      result = MultiplyHighSigned(xreg(instr->Rn()), xreg(instr->Rm()));
      break;
    default: not_implemented();
  }
  set_reg(reg_size, instr->Rd(), result);
}


void Simulator::VisitBitfield(Instruction* instr) {
  unsigned reg_size = instr->SixtyFourBits() ? kXRegSize : kWRegSize;
  int64_t reg_mask = instr->SixtyFourBits() ? kXRegMask : kWRegMask;
  int64_t R = instr->ImmR();
  int64_t S = instr->ImmS();
  int64_t diff = S - R;
  int64_t mask;
  if (diff >= 0) {
    mask = diff < reg_size - 1 ? (1L << (diff + 1)) - 1
                               : reg_mask;
  } else {
    mask = ((1L << (S + 1)) - 1);
    mask = (static_cast<uint64_t>(mask) >> R) | (mask << (reg_size - R));
    diff += reg_size;
  }

  // inzero indicates if the extracted bitfield is inserted into the
  // destination register value or in zero.
  // If extend is true, extend the sign of the extracted bitfield.
  bool inzero = false;
  bool extend = false;
  switch (instr->Mask(BitfieldMask)) {
    case BFM_x:
    case BFM_w:
      break;
    case SBFM_x:
    case SBFM_w:
      inzero = true;
      extend = true;
      break;
    case UBFM_x:
    case UBFM_w:
      inzero = true;
      break;
    default:
      not_implemented();
  }

  int64_t dst = inzero ? 0 : reg(reg_size, instr->Rd());
  int64_t src = reg(reg_size, instr->Rn());
  // Rotate source bitfield into place.
  int64_t result = (static_cast<uint64_t>(src) >> R) | (src << (reg_size - R));
  // Determine the sign extension.
  int64_t topbits = ((1L << (reg_size - diff - 1)) - 1) << (diff + 1);
  int64_t signbits = extend && ((src >> S) & 1) ? topbits : 0;

  // Merge sign extension, dest/zero and bitfield.
  result = signbits | (result & mask) | (dst & ~mask);

  set_reg(reg_size, instr->Rd(), result);
}


void Simulator::VisitExtract(Instruction* instr) {
  unsigned lsb = instr->ImmS();
  unsigned reg_size = (instr->SixtyFourBits() == 1) ? kXRegSize
                                                    : kWRegSize;
  set_reg(reg_size,
          instr->Rd(),
          (static_cast<uint64_t>(reg(reg_size, instr->Rm())) >> lsb) |
          (reg(reg_size, instr->Rn()) << (reg_size - lsb)));
}


void Simulator::VisitFPImmediate(Instruction* instr) {
  unsigned dest = instr->Rd();
  switch (instr->Mask(FPImmediateMask)) {
    case FMOV_s_imm: set_sreg(dest, instr->ImmFP32()); break;
    case FMOV_d_imm: set_dreg(dest, instr->ImmFP64()); break;
    default: not_reached();
  }
}


void Simulator::VisitFPIntegerConvert(Instruction* instr) {
  unsigned dst = instr->Rd();
  unsigned src = instr->Rn();

  switch (instr->Mask(FPIntegerConvertMask)) {
    case FCVTMS_ws:
      set_wreg(dst, FPToInt32(sreg(src), FPNegativeInfinity));
      break;
    case FCVTMS_xs:
      set_xreg(dst, FPToInt64(sreg(src), FPNegativeInfinity));
      break;
    case FCVTMS_wd:
      set_wreg(dst, FPToInt32(dreg(src), FPNegativeInfinity));
      break;
    case FCVTMS_xd:
      set_xreg(dst, FPToInt64(dreg(src), FPNegativeInfinity));
      break;
    case FCVTMU_ws:
      set_wreg(dst, FPToUInt32(sreg(src), FPNegativeInfinity));
      break;
    case FCVTMU_xs:
      set_xreg(dst, FPToUInt64(sreg(src), FPNegativeInfinity));
      break;
    case FCVTMU_wd:
      set_wreg(dst, FPToUInt32(dreg(src), FPNegativeInfinity));
      break;
    case FCVTMU_xd:
      set_xreg(dst, FPToUInt64(dreg(src), FPNegativeInfinity));
      break;
    case FCVTNS_ws: set_wreg(dst, FPToInt32(sreg(src), FPTieEven)); break;
    case FCVTNS_xs: set_xreg(dst, FPToInt64(sreg(src), FPTieEven)); break;
    case FCVTNS_wd: set_wreg(dst, FPToInt32(dreg(src), FPTieEven)); break;
    case FCVTNS_xd: set_xreg(dst, FPToInt64(dreg(src), FPTieEven)); break;
    case FCVTNU_ws: set_wreg(dst, FPToUInt32(sreg(src), FPTieEven)); break;
    case FCVTNU_xs: set_xreg(dst, FPToUInt64(sreg(src), FPTieEven)); break;
    case FCVTNU_wd: set_wreg(dst, FPToUInt32(dreg(src), FPTieEven)); break;
    case FCVTNU_xd: set_xreg(dst, FPToUInt64(dreg(src), FPTieEven)); break;
    case FCVTZS_ws: set_wreg(dst, FPToInt32(sreg(src), FPZero)); break;
    case FCVTZS_xs: set_xreg(dst, FPToInt64(sreg(src), FPZero)); break;
    case FCVTZS_wd: set_wreg(dst, FPToInt32(dreg(src), FPZero)); break;
    case FCVTZS_xd: set_xreg(dst, FPToInt64(dreg(src), FPZero)); break;
    case FCVTZU_ws: set_wreg(dst, FPToUInt32(sreg(src), FPZero)); break;
    case FCVTZU_xs: set_xreg(dst, FPToUInt64(sreg(src), FPZero)); break;
    case FCVTZU_wd: set_wreg(dst, FPToUInt32(dreg(src), FPZero)); break;
    case FCVTZU_xd: set_xreg(dst, FPToUInt64(dreg(src), FPZero)); break;
    case FMOV_ws: set_wreg(dst, sreg_bits(src)); break;
    case FMOV_xd: set_xreg(dst, dreg_bits(src)); break;
    case FMOV_sw: set_sreg_bits(dst, wreg(src)); break;
    case FMOV_dx: set_dreg_bits(dst, xreg(src)); break;

    // We only support conversions to double, and only when that double can
    // exactly represent a given integer. This means all 32-bit integers, and a
    // subset of 64-bit integers.
    case SCVTF_dw: {
      set_dreg(dst, wreg(src));
      break;
    }
    case SCVTF_dx: {
      double value = static_cast<double>(xreg(src));
      assert(static_cast<int64_t>(value) == xreg(src));
      set_dreg(dst, static_cast<int64_t>(value));
      break;
    }
    case UCVTF_dw: {
      set_dreg(dst, static_cast<uint32_t>(wreg(src)));
      break;
    }
    case UCVTF_dx: {
      double value = static_cast<double>(static_cast<uint64_t>(xreg(src)));
      assert(static_cast<uint64_t>(value) == static_cast<uint64_t>(xreg(src)));
      set_dreg(dst, static_cast<uint64_t>(value));
      break;
    }
    default: not_implemented();
  }
}


void Simulator::VisitFPFixedPointConvert(Instruction* instr) {
  unsigned dst = instr->Rd();
  unsigned src = instr->Rn();
  int fbits = 64 - instr->FPScale();

  // We only support two cases: unsigned and signed conversion from fixed point
  // values in X registers to floating point values in D registers. We rely on
  // casting to convert from integer to floating point, and assert that the
  // fractional part of the number is zero.
  switch (instr->Mask(FPFixedPointConvertMask)) {
    case UCVTF_dx_fixed: {
      uint64_t value = static_cast<uint64_t>(xreg(src));
      assert((value & ((1UL << fbits) - 1)) == 0);
      set_dreg(dst, static_cast<double>(value >> fbits));
      break;
    }
    case SCVTF_dx_fixed: {
      int64_t value = xreg(src);
      assert((value & ((1UL << fbits) - 1)) == 0);
      set_dreg(dst, static_cast<double>(value >> fbits));
      break;
    }
    default: not_implemented();
  }
}


int32_t Simulator::FPToInt32(double value, FPRounding rmode) {
  value = FPRoundInt(value, rmode);
  if (value >= kWMaxInt) {
    return kWMaxInt;
  } else if (value < kWMinInt) {
    return kWMinInt;
  }
  return isnan(value) ? 0 : static_cast<int32_t>(value);
}


int64_t Simulator::FPToInt64(double value, FPRounding rmode) {
  value = FPRoundInt(value, rmode);
  if (value >= kXMaxInt) {
    return kXMaxInt;
  } else if (value < kXMinInt) {
    return kXMinInt;
  }
  return isnan(value) ? 0 : static_cast<int64_t>(value);
}


uint32_t Simulator::FPToUInt32(double value, FPRounding rmode) {
  value = FPRoundInt(value, rmode);
  if (value >= kWMaxUInt) {
    return kWMaxUInt;
  } else if (value < 0.0) {
    return 0;
  }
  return isnan(value) ? 0 : static_cast<uint32_t>(value);
}


uint64_t Simulator::FPToUInt64(double value, FPRounding rmode) {
  value = FPRoundInt(value, rmode);
  if (value >= kXMaxUInt) {
    return kXMaxUInt;
  } else if (value < 0.0) {
    return 0;
  }
  return isnan(value) ? 0 : static_cast<uint64_t>(value);
}


void Simulator::VisitFPCompare(Instruction* instr) {
  unsigned reg_size = instr->FPType() == FP32 ? kSRegSize : kDRegSize;
  double fn_val = fpreg(reg_size, instr->Rn());

  switch (instr->Mask(FPCompareMask)) {
    case FCMP_s:
    case FCMP_d: FPCompare(fn_val, fpreg(reg_size, instr->Rm())); break;
    case FCMP_s_zero:
    case FCMP_d_zero: FPCompare(fn_val, 0.0); break;
    default: not_implemented();
  }
}


void Simulator::VisitFPConditionalCompare(Instruction* instr) {
  switch (instr->Mask(FPConditionalCompareMask)) {
    case FCCMP_s:
    case FCCMP_d: {
      if (ConditionPassed(static_cast<Condition>(instr->Condition()))) {
        // If the condition passes, set the status flags to the result of
        // comparing the operands.
        unsigned reg_size = instr->FPType() == FP32 ? kSRegSize : kDRegSize;
        FPCompare(fpreg(reg_size, instr->Rn()), fpreg(reg_size, instr->Rm()));
      } else {
        // If the condition fails, set the status flags to the nzcv immediate.
        SetFlags(instr->Nzcv() << Flags_offset);
      }
      break;
    }
    default: not_implemented();
  }
}


void Simulator::VisitFPConditionalSelect(Instruction* instr) {
  unsigned reg_size = instr->FPType() == FP32 ? kSRegSize : kDRegSize;

  double selected_val;
  if (ConditionPassed(static_cast<Condition>(instr->Condition()))) {
    selected_val = fpreg(reg_size, instr->Rn());
  } else {
    selected_val = fpreg(reg_size, instr->Rm());
  }

  switch (instr->Mask(FPConditionalSelectMask)) {
    case FCSEL_s:
    case FCSEL_d: set_fpreg(reg_size, instr->Rd(), selected_val); break;
    default: not_implemented();
  }
}


void Simulator::VisitFPDataProcessing1Source(Instruction* instr) {
  unsigned fd = instr->Rd();
  unsigned fn = instr->Rn();

  switch (instr->Mask(FPDataProcessing1SourceMask)) {
    case FMOV_s: set_sreg(fd, sreg(fn)); break;
    case FMOV_d: set_dreg(fd, dreg(fn)); break;
    case FABS_s: set_sreg(fd, fabs(sreg(fn))); break;
    case FABS_d: set_dreg(fd, fabs(dreg(fn))); break;
    case FNEG_s: set_sreg(fd, -sreg(fn)); break;
    case FNEG_d: set_dreg(fd, -dreg(fn)); break;
    case FSQRT_s: set_sreg(fd, sqrt(sreg(fn))); break;
    case FSQRT_d: set_dreg(fd, sqrt(dreg(fn))); break;
    case FRINTN_s: set_sreg(fd, FPRoundInt(sreg(fn), FPTieEven)); break;
    case FRINTN_d: set_dreg(fd, FPRoundInt(dreg(fn), FPTieEven)); break;
    case FRINTZ_s: set_sreg(fd, FPRoundInt(sreg(fn), FPZero)); break;
    case FRINTZ_d: set_dreg(fd, FPRoundInt(dreg(fn), FPZero)); break;
    case FCVT_ds: set_dreg(fd, sreg(fn)); break;
    default: not_implemented();
  }
}


double Simulator::FPRoundInt(double value, FPRounding round_mode) {
  if ((value == 0.0) || (value == kFP64PositiveInfinity) ||
      (value == kFP64NegativeInfinity) || isnan(value)) {
    return value;
  }

  double int_result = floor(value);
  double error = value - int_result;
  switch (round_mode) {
    case FPTieEven: {
      // If the error is greater than 0.5, or is equal to 0.5 and the integer
      // result is odd, round up.
      if ((error > 0.5) ||
          ((error == 0.5) && (fmod(int_result, 2) != 0))) {
        int_result++;
      }
      break;
    }
    case FPZero: {
      // If value>0 then we take floor(value)
      // otherwise, ceil(value).
      if (value < 0) {
         int_result = ceil(value);
      }
      break;
    }
    case FPNegativeInfinity: {
      // We always use floor(value).
      break;
    }
    default: not_implemented();
  }
  return int_result;
}


void Simulator::VisitFPDataProcessing2Source(Instruction* instr) {
  unsigned fd = instr->Rd();
  unsigned fn = instr->Rn();
  unsigned fm = instr->Rm();

  switch (instr->Mask(FPDataProcessing2SourceMask)) {
    case FADD_s: set_sreg(fd, sreg(fn) + sreg(fm)); break;
    case FADD_d: set_dreg(fd, dreg(fn) + dreg(fm)); break;
    case FSUB_s: set_sreg(fd, sreg(fn) - sreg(fm)); break;
    case FSUB_d: set_dreg(fd, dreg(fn) - dreg(fm)); break;
    case FMUL_s: set_sreg(fd, sreg(fn) * sreg(fm)); break;
    case FMUL_d: set_dreg(fd, dreg(fn) * dreg(fm)); break;
    case FDIV_s: set_sreg(fd, sreg(fn) / sreg(fm)); break;
    case FDIV_d: set_dreg(fd, dreg(fn) / dreg(fm)); break;
    case FMAX_s: set_sreg(fd, FPMax(sreg(fn), sreg(fm))); break;
    case FMAX_d: set_dreg(fd, FPMax(dreg(fn), dreg(fm))); break;
    case FMIN_s: set_sreg(fd, FPMin(sreg(fn), sreg(fm))); break;
    case FMIN_d: set_dreg(fd, FPMin(dreg(fn), dreg(fm))); break;
    default: not_implemented();
  }
}


void Simulator::VisitFPDataProcessing3Source(Instruction* instr) {
  unsigned fd = instr->Rd();
  unsigned fn = instr->Rn();
  unsigned fm = instr->Rm();
  unsigned fa = instr->Ra();

  // Note: The FMSUB implementation here is not precisely the same as the
  // instruction definition. In full implementation rounding of results would
  // occur once at the end, here rounding will occur after the first multiply
  // and then after the subsequent addition.  A full implementation here would
  // be possible but would require an effort isn't immediately justified given
  // the small differences we expect to see in most cases.

  switch (instr->Mask(FPDataProcessing3SourceMask)) {
    case FMSUB_s: set_sreg(fd, sreg(fa) + (-sreg(fn))*sreg(fm)); break;
    case FMSUB_d: set_dreg(fd, dreg(fa) + (-dreg(fn))*dreg(fm)); break;
    default: not_implemented();
  }
}


double Simulator::FPMax(double a, double b) {
  if (isnan(a)) {
    return a;
  } else if (isnan(b)) {
    return b;
  }

  if ((a == 0.0) && (b == 0.0) &&
      (copysign(1.0, a) != copysign(1.0, b))) {
    // a and b are zero, and the sign differs: return +0.0.
    return 0.0;
  } else {
    return (a > b) ? a : b;
  }
}


double Simulator::FPMin(double a, double b) {
  if (isnan(a)) {
    return a;
  } else if (isnan(b)) {
    return b;
  }

  if ((a == 0.0) && (b == 0.0) &&
      (copysign(1.0, a) != copysign(1.0, b))) {
    // a and b are zero, and the sign differs: return -0.0.
    return -0.0;
  } else {
    return (a < b) ? a : b;
  }
}


void Simulator::VisitSystem(Instruction* instr) {
  // Some system instructions hijack their Op and Cp fields to represent a
  // range of immediates instead of indicating a different instruction. This
  // makes the decoding tricky.
  if (instr->Mask(SystemSysRegFMask) == SystemSysRegFixed) {
    switch (instr->Mask(SystemSysRegMask)) {
      case MRS: {
        switch (instr->ImmSystemRegister()) {
          case NZCV: set_xreg(instr->Rt(), nzcv()); break;
          default: not_implemented();
        }
        break;
      }
      case MSR: {
        switch (instr->ImmSystemRegister()) {
          case NZCV:
            SetFlags(xreg(instr->Rt()) & kConditionFlagsMask);
            break;
          default: not_implemented();
        }
        break;
      }
    }
  } else if (instr->Mask(SystemHintFMask) == SystemHintFixed) {
    assert(instr->Mask(SystemHintMask) == HINT);
    switch (instr->ImmHint()) {
      case NOP: break;
      default: not_implemented();
    }
  } else {
    not_implemented();
  }
}


void Simulator::VisitException(Instruction* instr) {
  switch (instr->Mask(ExceptionMask)) {
    case BRK: HostBreakpoint(); break;
    case HLT:
      // The Printf pseudo instruction is so useful, we include it in the
      // default simulator.
      if (instr->ImmException() == kPrintfOpcode) {
        DoPrintf(instr);
      } else if (instr->ImmException() == kHostCallOpcode) {
        DoHostCall(instr);
      } else {
        HostBreakpoint();
      }
      break;
    default:
      not_implemented();
  }
}


void Simulator::DoPrintf(Instruction* instr) {
  assert((instr->Mask(ExceptionMask) == HLT) &&
         (instr->ImmException() == kPrintfOpcode));

  // Read the argument encoded inline in the instruction stream.
  uint32_t type;
  assert(sizeof(*instr) == 1);
  memcpy(&type, instr + kPrintfTypeOffset, sizeof(type));

  const char * format = reinterpret_cast<const char *>(x0());
  assert(format != nullptr);

  // Pass all of the relevant PCS registers onto printf. It doesn't matter
  // if we pass too many as the extra ones won't be read.
  int result = 0;
  if (type == CPURegister::kRegister) {
    result = printf(format, x1(), x2(), x3(), x4(), x5(), x6(), x7());
  } else if (type == CPURegister::kFPRegister) {
    result = printf(format, d0(), d1(), d2(), d3(), d4(), d5(), d6(), d7());
  } else {
    assert(type == CPURegister::kNoRegister);
    result = printf("%s", format);
  }
  set_x0(result);

  // TODO: Clobber all caller-saved registers here, to ensure no assumptions
  // are made about preserved state.

  // The printf parameters are inlined in the code, so skip them.
  set_pc(instr->InstructionAtOffset(kPrintfLength));

  // Set LR as if we'd just called a native printf function.
  set_lr(reinterpret_cast<uint64_t>(pc()));
}

void Simulator::DoHostCall(Instruction* instr) {
  // Read the number of arguments out of the instruction stream
  uint32_t argc;
  assert(sizeof(*instr) == 1);
  memcpy(&argc, instr + kHostCallCountOffset, sizeof(argc));
  assert(argc < 6);

  typedef intptr_t(*Native0Ptr)(void);
  typedef intptr_t(*Native1Ptr)(intptr_t);
  typedef intptr_t(*Native2Ptr)(intptr_t, intptr_t);
  typedef intptr_t(*Native3Ptr)(intptr_t, intptr_t, intptr_t);
  typedef intptr_t(*Native4Ptr)(intptr_t, intptr_t, intptr_t, intptr_t);
  typedef intptr_t(*Native5Ptr)(intptr_t, intptr_t, intptr_t, intptr_t,
                                intptr_t);

  intptr_t result;

  switch (argc) {
    case 0:
      result = reinterpret_cast<Native0Ptr>(xreg(16))();
      break;
    case 1:
      result = reinterpret_cast<Native1Ptr>(xreg(16))(xreg(0));
      break;
    case 2:
      result = reinterpret_cast<Native2Ptr>(xreg(16))(xreg(0), xreg(1));
      break;
    case 3:
      result = reinterpret_cast<Native3Ptr>(xreg(16))(
        xreg(0), xreg(1), xreg(2));
      break;
    case 4:
      result = reinterpret_cast<Native4Ptr>(xreg(16))(
        xreg(0), xreg(1), xreg(2), xreg(3));
      break;
    case 5:
      result = reinterpret_cast<Native5Ptr>(xreg(16))(
        xreg(0), xreg(1), xreg(2), xreg(3), xreg(4));
      break;
    default:
      not_reached();
  }

  // Trash all caller-saved registers
  auto callerSaved = CPURegList::GetCallerSaved();
  while (!callerSaved.IsEmpty()) {
    auto reg = callerSaved.PopLowestIndex();
    set_xreg(reg.code(), 0xf00dbeef);
  }

  set_xreg(0, result);

  // Skip over the embedded argc
  set_pc(instr->InstructionAtOffset(kHostCallCountOffset + kInstructionSize));
}

}  // namespace vixl

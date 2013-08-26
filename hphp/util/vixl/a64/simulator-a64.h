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

#ifndef VIXL_A64_SIMULATOR_A64_H_
#define VIXL_A64_SIMULATOR_A64_H_

#include "hphp/util/vixl/globals.h"
#include "hphp/util/vixl/utils.h"
#include "hphp/util/vixl/a64/instructions-a64.h"
#include "hphp/util/vixl/a64/assembler-a64.h"
#include "hphp/util/vixl/a64/disasm-a64.h"

namespace vixl {

enum ReverseByteMode {
  Reverse16 = 0,
  Reverse32 = 1,
  Reverse64 = 2
};

// Printf. See debugger-a64.h for more information on pseudo instructions.
//  - type: CPURegister::RegisterType stored as a uint32_t.
//
// Simulate a call to printf.
//
// Floating-point and integer arguments are passed in separate sets of
// registers in AAPCS64 (even for varargs functions), so it is not possible to
// determine the type of location of each argument without some information
// about the values that were passed in. This information could be retrieved
// from the printf format string, but the format string is not trivial to
// parse so we encode the relevant information with the HLT instruction under
// the type argument. Therefore the interface is:
//    x0: The format string
// x1-x7: Optional arguments, if type == CPURegister::kRegister
// d0-d7: Optional arguments, if type == CPURegister::kFPRegister
const Instr kPrintfOpcode = 0xdeb1;
const unsigned kPrintfTypeOffset = 1 * kInstructionSize;
const unsigned kPrintfLength = 2 * kInstructionSize;

const Instr kHostCallOpcode = 0xdeb4;
const unsigned kHostCallCountOffset = 1 * kInstructionSize;

class Simulator : public DecoderVisitor {
 public:
  explicit Simulator(Decoder* decoder, FILE* stream = stdout);
  ~Simulator();

  void ResetState();

  // TODO: We assume little endianness, and the way in which the members of this
  // union overlay. Add tests to ensure this, or fix accessors to no longer
  // require this assumption.
  union SimRegister {
    int64_t x;
    int32_t w;
  };

  union SimFPRegister {
    double d;
    float s;
  };

  // Run the simulator.
  virtual void Run();
  void RunFrom(Instruction* first);

  // Simulation helpers.
  inline Instruction* pc() { return pc_; }
  inline void set_pc(Instruction* new_pc) {
    pc_ = new_pc;
    pc_modified_ = true;
  }

  inline void increment_pc() {
    if (!pc_modified_) {
      pc_ = pc_->NextInstruction();
    }

    pc_modified_ = false;
  }

  inline void ExecuteInstruction() {
    // The program counter should always be aligned.
    assert(IsWordAligned(pc_));
    decoder_->Decode(pc_);
    increment_pc();
  }

  // Declare all Visitor functions.
  #define DECLARE(A)  void Visit##A(Instruction* instr);
  VISITOR_LIST(DECLARE)
  #undef DECLARE

  // Register accessors.
  inline int32_t wreg(unsigned code,
                      Reg31Mode r31mode = Reg31IsZeroRegister) const {
    assert(code < kNumberOfRegisters);
    if ((code == 31) && (r31mode == Reg31IsZeroRegister)) {
      return 0;
    }
    return registers_[code].w;
  }

  inline int64_t xreg(unsigned code,
                      Reg31Mode r31mode = Reg31IsZeroRegister) const {
    assert(code < kNumberOfRegisters);
    if ((code == 31) && (r31mode == Reg31IsZeroRegister)) {
      return 0;
    }
    return registers_[code].x;
  }

  inline int64_t reg(unsigned size,
                     unsigned code,
                     Reg31Mode r31mode = Reg31IsZeroRegister) const {
    switch (size) {
      case kWRegSize: return wreg(code, r31mode) & kWRegMask;
      case kXRegSize: return xreg(code, r31mode);
      default:
        not_reached();
        return 0;
    }
  }

  inline void set_wreg(unsigned code, int32_t value,
                       Reg31Mode r31mode = Reg31IsZeroRegister) {
    assert(code < kNumberOfRegisters);
    if ((code == kZeroRegCode) && (r31mode == Reg31IsZeroRegister)) {
      return;
    }
    registers_[code].x = 0;  // First clear the register top bits.
    registers_[code].w = value;
  }

  inline void set_xreg(unsigned code, int64_t value,
                       Reg31Mode r31mode = Reg31IsZeroRegister) {
    assert(code < kNumberOfRegisters);
    if ((code == kZeroRegCode) && (r31mode == Reg31IsZeroRegister)) {
      return;
    }
    registers_[code].x = value;
  }

  inline void set_reg(unsigned size, unsigned code, int64_t value,
                      Reg31Mode r31mode = Reg31IsZeroRegister) {
    switch (size) {
      case kWRegSize:
        return set_wreg(code, static_cast<int32_t>(value & 0xffffffff),
                        r31mode);
      case kXRegSize:
        return set_xreg(code, value, r31mode);
      default:
        not_reached();
        break;
    }
  }

  #define REG_ACCESSORS(N)                                 \
  inline int32_t w##N() { return wreg(N); }                \
  inline int64_t x##N() { return xreg(N); }                \
  inline void set_w##N(int32_t val) { set_wreg(N, val); }  \
  inline void set_x##N(int64_t val) { set_xreg(N, val); }
  REGISTER_CODE_LIST(REG_ACCESSORS)
  #undef REG_ACCESSORS

  // Aliases.
  #define REG_ALIAS_ACCESSORS(N, wname, xname)                \
  inline int32_t wname() { return wreg(N); }                  \
  inline int64_t xname() { return xreg(N); }                  \
  inline void set_##wname(int32_t val) { set_wreg(N, val); }  \
  inline void set_##xname(int64_t val) { set_xreg(N, val); }
  REG_ALIAS_ACCESSORS(30, wlr, lr);
  #undef REG_ALIAS_ACCESSORS

  // The stack is a special case in aarch64.
  inline int32_t wsp() { return wreg(31, Reg31IsStackPointer); }
  inline int64_t sp() { return xreg(31, Reg31IsStackPointer); }
  inline void set_wsp(int32_t val) {
    set_wreg(31, val, Reg31IsStackPointer);
  }
  inline void set_sp(int64_t val) {
    set_xreg(31, val, Reg31IsStackPointer);
  }

  // FPRegister accessors.
  inline float sreg(unsigned code) const {
    assert(code < kNumberOfFPRegisters);
    return fpregisters_[code].s;
  }

  inline uint32_t sreg_bits(unsigned code) const {
    return float_to_rawbits(sreg(code));
  }

  inline double dreg(unsigned code) const {
    assert(code < kNumberOfFPRegisters);
    return fpregisters_[code].d;
  }

  inline uint64_t dreg_bits(unsigned code) const {
    return double_to_rawbits(dreg(code));
  }

  inline double fpreg(unsigned size, unsigned code) const {
    switch (size) {
      case kSRegSize: return sreg(code);
      case kDRegSize: return dreg(code);
      default: {
        not_reached();
        return 0.0;
      }
    }
  }

  inline void set_sreg(unsigned code, float val) {
    assert(code < kNumberOfFPRegisters);
    // Ensure that the upper word is set to 0.
    set_dreg_bits(code, 0);

    fpregisters_[code].s = val;
  }

  inline void set_sreg_bits(unsigned code, uint32_t rawbits) {
    assert(code < kNumberOfFPRegisters);
    // Ensure that the upper word is set to 0.
    set_dreg_bits(code, 0);

    set_sreg(code, rawbits_to_float(rawbits));
  }

  inline void set_dreg(unsigned code, double val) {
    assert(code < kNumberOfFPRegisters);
    fpregisters_[code].d = val;
  }

  inline void set_dreg_bits(unsigned code, uint64_t rawbits) {
    assert(code < kNumberOfFPRegisters);
    set_dreg(code, rawbits_to_double(rawbits));
  }

  inline void set_fpreg(unsigned size, unsigned code, double value) {
    switch (size) {
      case kSRegSize:
        return set_sreg(code, value);
      case kDRegSize:
        return set_dreg(code, value);
      default:
        not_reached();
        break;
    }
  }

  #define FPREG_ACCESSORS(N)                             \
  inline float s##N() { return sreg(N); }                \
  inline double d##N() { return dreg(N); }               \
  inline void set_s##N(float val) { set_sreg(N, val); }  \
  inline void set_d##N(double val) { set_dreg(N, val); }
  REGISTER_CODE_LIST(FPREG_ACCESSORS)
  #undef FPREG_ACCESSORS

  bool N() { return (psr_ & NFlag) != 0; }
  bool Z() { return (psr_ & ZFlag) != 0; }
  bool C() { return (psr_ & CFlag) != 0; }
  bool V() { return (psr_ & VFlag) != 0; }
  uint32_t nzcv() { return psr_ & (NFlag | ZFlag | CFlag | VFlag); }

  // Debug helpers
  void PrintFlags(bool print_all = false);
  void PrintRegisters(bool print_all_regs = false);
  void PrintFPRegisters(bool print_all_regs = false);
  void PrintProcessorState();

  static const char* WRegNameForCode(unsigned code,
                                     Reg31Mode mode = Reg31IsZeroRegister);
  static const char* XRegNameForCode(unsigned code,
                                     Reg31Mode mode = Reg31IsZeroRegister);
  static const char* SRegNameForCode(unsigned code);
  static const char* DRegNameForCode(unsigned code);
  static const char* VRegNameForCode(unsigned code);

  inline bool coloured_trace() { return coloured_trace_; }
  inline void set_coloured_trace(bool value) { coloured_trace_ = value; }

  inline bool disasm_trace() { return disasm_trace_; }
  inline void set_disasm_trace(bool value) {
    if (value != disasm_trace_) {
      if (value) {
        decoder_->InsertVisitorBefore(print_disasm_, this);
      } else {
        decoder_->RemoveVisitor(print_disasm_);
      }
      disasm_trace_ = value;
    }
  }

 protected:
  // Simulation helpers ------------------------------------
  bool ConditionPassed(Condition cond) {
    switch (cond) {
      case eq:
        return Z();
      case ne:
        return !Z();
      case hs:
        return C();
      case lo:
        return !C();
      case mi:
        return N();
      case pl:
        return !N();
      case vs:
        return V();
      case vc:
        return !V();
      case hi:
        return C() && !Z();
      case ls:
        return !(C() && !Z());
      case ge:
        return N() == V();
      case lt:
        return N() != V();
      case gt:
        return !Z() && (N() == V());
      case le:
        return !(!Z() && (N() == V()));
      case al:
        return true;
      default:
        not_reached();
        return false;
    }
  }

  bool ConditionFailed(Condition cond) {
    return !ConditionPassed(cond);
  }

  void AddSubHelper(Instruction* instr, int64_t op2);
  int64_t AddWithCarry(unsigned reg_size,
                       bool set_flags,
                       int64_t src1,
                       int64_t src2,
                       int64_t carry_in = 0);
  void LogicalHelper(Instruction* instr, int64_t op2);
  void ConditionalCompareHelper(Instruction* instr, int64_t op2);
  void LoadStoreHelper(Instruction* instr,
                       int64_t offset,
                       AddrMode addrmode);
  void LoadStorePairHelper(Instruction* instr, AddrMode addrmode);
  uint8_t* AddressModeHelper(unsigned addr_reg,
                             int64_t offset,
                             AddrMode addrmode);

  uint64_t MemoryRead(const uint8_t* address, unsigned num_bytes);
  uint8_t MemoryRead8(uint8_t* address);
  uint16_t MemoryRead16(uint8_t* address);
  uint32_t MemoryRead32(uint8_t* address);
  float MemoryReadFP32(uint8_t* address);
  uint64_t MemoryRead64(uint8_t* address);
  double MemoryReadFP64(uint8_t* address);

  void MemoryWrite(uint8_t* address, uint64_t value, unsigned num_bytes);
  void MemoryWrite32(uint8_t* address, uint32_t value);
  void MemoryWriteFP32(uint8_t* address, float value);
  void MemoryWrite64(uint8_t* address, uint64_t value);
  void MemoryWriteFP64(uint8_t* address, double value);

  int64_t ShiftOperand(unsigned reg_size,
                       int64_t value,
                       Shift shift_type,
                       unsigned amount);
  int64_t Rotate(unsigned reg_width,
                 int64_t value,
                 Shift shift_type,
                 unsigned amount);
  int64_t ExtendValue(unsigned reg_width,
                      int64_t value,
                      Extend extend_type,
                      unsigned left_shift = 0);

  uint64_t ReverseBits(uint64_t value, unsigned num_bits);
  uint64_t ReverseBytes(uint64_t value, ReverseByteMode mode);

  void FPCompare(double val0, double val1);
  double FPRoundInt(double value, FPRounding round_mode);
  int32_t FPToInt32(double value, FPRounding rmode);
  int64_t FPToInt64(double value, FPRounding rmode);
  uint32_t FPToUInt32(double value, FPRounding rmode);
  uint64_t FPToUInt64(double value, FPRounding rmode);
  double FPMax(double a, double b);
  double FPMin(double a, double b);

  // Pseudo Printf instruction
  void DoPrintf(Instruction* instr);
  // Pseudo HostCall instruction
  void DoHostCall(Instruction* instr);

  // Processor state ---------------------------------------

  // Output stream.
  FILE* stream_;
  PrintDisassembler* print_disasm_;

  // General purpose registers. Register 31 is the stack pointer.
  SimRegister registers_[kNumberOfRegisters];

  // Floating point registers
  SimFPRegister fpregisters_[kNumberOfFPRegisters];

  // Program Status Register.
  // bits[31, 27]: Condition flags N, Z, C, and V.
  //               (Negative, Zero, Carry, Overflow)
  uint32_t psr_;

  // Condition flags.
  void SetFlags(uint32_t new_flags);

  static inline uint32_t CalcNFlag(int64_t result, unsigned reg_size) {
    return ((result >> (reg_size - 1)) & 1) * NFlag;
  }

  static inline uint32_t CalcZFlag(int64_t result) {
    return (result == 0) ? static_cast<uint32_t>(ZFlag) : 0;
  }

  static const uint32_t kConditionFlagsMask = 0xf0000000;

  // Stack
  byte* stack_;
  static const int stack_protection_size_ = 256;
  // 2 KB stack.
  static const int stack_size_ = 2 * 1024 + 2 * stack_protection_size_;
  byte* stack_limit_;

  Decoder* decoder_;
  // Indicates if the pc has been modified by the instruction and should not be
  // automatically incremented.
  bool pc_modified_;
  Instruction* pc_;

  static const char* xreg_names[];
  static const char* wreg_names[];
  static const char* sreg_names[];
  static const char* dreg_names[];
  static const char* vreg_names[];

  static const Instruction* kEndOfSimAddress;

 private:
  bool coloured_trace_;
  // Indicates whether the disassembly trace is active.
  bool disasm_trace_;
};
}  // namespace vixl

#endif  // VIXL_A64_SIMULATOR_A64_H_

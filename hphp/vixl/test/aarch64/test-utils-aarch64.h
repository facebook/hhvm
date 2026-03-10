// Copyright 2014, VIXL authors
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

#ifndef VIXL_AARCH64_TEST_UTILS_AARCH64_H_
#define VIXL_AARCH64_TEST_UTILS_AARCH64_H_

#include "test-runner.h"

#include "aarch64/cpu-aarch64.h"
#include "aarch64/disasm-aarch64.h"
#include "aarch64/macro-assembler-aarch64.h"
#include "aarch64/simulator-aarch64.h"

namespace vixl {
namespace aarch64 {

// Signalling and quiet NaNs in double format, constructed such that the bottom
// 32 bits look like a signalling or quiet NaN (as appropriate) when interpreted
// as a float. These values are not architecturally significant, but they're
// useful in tests for initialising registers.
extern const double kFP64SignallingNaN;
extern const double kFP64QuietNaN;

// Signalling and quiet NaNs in float format.
extern const float kFP32SignallingNaN;
extern const float kFP32QuietNaN;

// Signalling and quiet NaNs in half-precision float format.
extern const Float16 kFP16SignallingNaN;
extern const Float16 kFP16QuietNaN;

// Vector registers don't naturally fit any C++ native type, so define a class
// with convenient accessors.
// Note that this has to be a POD type so that we can use 'offsetof' with it.
template <int kSizeInBytes>
struct VectorValue {
  template <typename T>
  T GetLane(int lane) const {
    size_t lane_size = sizeof(T);
    VIXL_CHECK(lane >= 0);
    VIXL_CHECK(kSizeInBytes >= ((lane + 1) * lane_size));
    T result;
    memcpy(&result, bytes + (lane * lane_size), lane_size);
    return result;
  }

  template <typename T>
  void SetLane(int lane, T value) {
    size_t lane_size = sizeof(value);
    VIXL_CHECK(kSizeInBytes >= ((lane + 1) * lane_size));
    memcpy(bytes + (lane * lane_size), &value, lane_size);
  }

  bool Equals(const VectorValue<kSizeInBytes>& other) const {
    return memcmp(bytes, other.bytes, kSizeInBytes) == 0;
  }

  uint8_t bytes[kSizeInBytes];
};

// It would be convenient to make these subclasses, so we can provide convenient
// constructors and utility methods specific to each register type, but we can't
// do that because it makes the result a non-POD type, and then we can't use
// 'offsetof' in RegisterDump::Dump.
typedef VectorValue<kQRegSizeInBytes> QRegisterValue;
typedef VectorValue<kZRegMaxSizeInBytes> ZRegisterValue;
typedef VectorValue<kPRegMaxSizeInBytes> PRegisterValue;

// RegisterDump: Object allowing integer, floating point and flags registers
// to be saved to itself for future reference.
class RegisterDump {
 public:
  RegisterDump() : completed_(false) {
    VIXL_ASSERT(sizeof(dump_.d_[0]) == kDRegSizeInBytes);
    VIXL_ASSERT(sizeof(dump_.s_[0]) == kSRegSizeInBytes);
    VIXL_ASSERT(sizeof(dump_.h_[0]) == kHRegSizeInBytes);
    VIXL_ASSERT(sizeof(dump_.d_[0]) == kXRegSizeInBytes);
    VIXL_ASSERT(sizeof(dump_.s_[0]) == kWRegSizeInBytes);
    VIXL_ASSERT(sizeof(dump_.x_[0]) == kXRegSizeInBytes);
    VIXL_ASSERT(sizeof(dump_.w_[0]) == kWRegSizeInBytes);
    VIXL_ASSERT(sizeof(dump_.q_[0]) == kQRegSizeInBytes);
  }

  // The Dump method generates code to store a snapshot of the register values.
  // It needs to be able to use the stack temporarily, and requires that the
  // current stack pointer is sp, and is properly aligned.
  //
  // The dumping code is generated though the given MacroAssembler. No registers
  // are corrupted in the process, but the stack is used briefly. The flags will
  // be corrupted during this call.
  void Dump(MacroAssembler* assm);

  // Register accessors.
  inline int32_t wreg(unsigned code) const {
    if (code == kSPRegInternalCode) {
      return wspreg();
    }
    VIXL_ASSERT(RegAliasesMatch(code));
    return dump_.w_[code];
  }

  inline int64_t xreg(unsigned code) const {
    if (code == kSPRegInternalCode) {
      return spreg();
    }
    VIXL_ASSERT(RegAliasesMatch(code));
    return dump_.x_[code];
  }

  // VRegister accessors.
  inline uint16_t hreg_bits(unsigned code) const {
    VIXL_ASSERT(VRegAliasesMatch(code));
    return dump_.h_[code];
  }

  inline uint32_t sreg_bits(unsigned code) const {
    VIXL_ASSERT(VRegAliasesMatch(code));
    return dump_.s_[code];
  }

  inline Float16 hreg(unsigned code) const {
    return RawbitsToFloat16(hreg_bits(code));
  }

  inline float sreg(unsigned code) const {
    return RawbitsToFloat(sreg_bits(code));
  }

  inline uint64_t dreg_bits(unsigned code) const {
    VIXL_ASSERT(VRegAliasesMatch(code));
    return dump_.d_[code];
  }

  inline double dreg(unsigned code) const {
    return RawbitsToDouble(dreg_bits(code));
  }

  inline QRegisterValue qreg(unsigned code) const { return dump_.q_[code]; }

  template <typename T>
  inline T zreg_lane(unsigned code, int lane) const {
    VIXL_ASSERT(VRegAliasesMatch(code));
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
    VIXL_ASSERT(lane < GetSVELaneCount(sizeof(T) * kBitsPerByte));
    return dump_.z_[code].GetLane<T>(lane);
  }

  inline uint64_t zreg_lane(unsigned code,
                            unsigned size_in_bits,
                            int lane) const {
    switch (size_in_bits) {
      case kBRegSize:
        return zreg_lane<uint8_t>(code, lane);
      case kHRegSize:
        return zreg_lane<uint16_t>(code, lane);
      case kSRegSize:
        return zreg_lane<uint32_t>(code, lane);
      case kDRegSize:
        return zreg_lane<uint64_t>(code, lane);
    }
    VIXL_UNREACHABLE();
    return 0;
  }

  inline uint64_t preg_lane(unsigned code,
                            unsigned p_bits_per_lane,
                            int lane) const {
    VIXL_ASSERT(CPUHas(CPUFeatures::kSVE));
    VIXL_ASSERT(lane < GetSVELaneCount(p_bits_per_lane * kZRegBitsPerPRegBit));
    // Load a chunk and extract the necessary bits. The chunk size is arbitrary.
    typedef uint64_t Chunk;
    const size_t kChunkSizeInBits = sizeof(Chunk) * kBitsPerByte;
    VIXL_ASSERT(IsPowerOf2(p_bits_per_lane));
    VIXL_ASSERT(p_bits_per_lane <= kChunkSizeInBits);

    int chunk_index = (lane * p_bits_per_lane) / kChunkSizeInBits;
    int bit_index = (lane * p_bits_per_lane) % kChunkSizeInBits;
    Chunk chunk = dump_.p_[code].GetLane<Chunk>(chunk_index);
    return (chunk >> bit_index) & GetUintMask(p_bits_per_lane);
  }

  inline int GetSVELaneCount(int lane_size_in_bits) const {
    VIXL_ASSERT(lane_size_in_bits > 0);
    VIXL_ASSERT((dump_.vl_ % lane_size_in_bits) == 0);
    uint64_t count = dump_.vl_ / lane_size_in_bits;
    VIXL_ASSERT(count <= INT_MAX);
    return static_cast<int>(count);
  }

  template <typename T>
  inline bool HasSVELane(T reg, int lane) const {
    VIXL_ASSERT(reg.IsZRegister() || reg.IsPRegister());
    return lane < GetSVELaneCount(reg.GetLaneSizeInBits());
  }

  template <typename T>
  inline uint64_t GetSVELane(T reg, int lane) const {
    VIXL_ASSERT(HasSVELane(reg, lane));
    if (reg.IsZRegister()) {
      return zreg_lane(reg.GetCode(), reg.GetLaneSizeInBits(), lane);
    } else if (reg.IsPRegister()) {
      VIXL_ASSERT((reg.GetLaneSizeInBits() % kZRegBitsPerPRegBit) == 0);
      return preg_lane(reg.GetCode(),
                       reg.GetLaneSizeInBits() / kZRegBitsPerPRegBit,
                       lane);
    } else {
      VIXL_ABORT();
    }
  }

  // Stack pointer accessors.
  inline int64_t spreg() const {
    VIXL_ASSERT(SPRegAliasesMatch());
    return dump_.sp_;
  }

  inline int32_t wspreg() const {
    VIXL_ASSERT(SPRegAliasesMatch());
    return static_cast<int32_t>(dump_.wsp_);
  }

  // Flags accessors.
  inline uint32_t flags_nzcv() const {
    VIXL_ASSERT(IsComplete());
    VIXL_ASSERT((dump_.flags_ & ~Flags_mask) == 0);
    return dump_.flags_ & Flags_mask;
  }

  inline bool IsComplete() const { return completed_; }

 private:
  // Indicate whether the dump operation has been completed.
  bool completed_;

  // Check that the lower 32 bits of x<code> exactly match the 32 bits of
  // w<code>. A failure of this test most likely represents a failure in the
  // ::Dump method, or a failure in the simulator.
  bool RegAliasesMatch(unsigned code) const {
    VIXL_ASSERT(IsComplete());
    VIXL_ASSERT(code < kNumberOfRegisters);
    return ((dump_.x_[code] & kWRegMask) == dump_.w_[code]);
  }

  // As RegAliasesMatch, but for the stack pointer.
  bool SPRegAliasesMatch() const {
    VIXL_ASSERT(IsComplete());
    return ((dump_.sp_ & kWRegMask) == dump_.wsp_);
  }

  // As RegAliasesMatch, but for Z and V registers.
  bool VRegAliasesMatch(unsigned code) const {
    VIXL_ASSERT(IsComplete());
    VIXL_ASSERT(code < kNumberOfVRegisters);
    bool match = ((dump_.q_[code].GetLane<uint64_t>(0) == dump_.d_[code]) &&
                  ((dump_.d_[code] & kSRegMask) == dump_.s_[code]) &&
                  ((dump_.s_[code] & kHRegMask) == dump_.h_[code]));
    if (CPUHas(CPUFeatures::kSVE)) {
      bool z_match =
          memcmp(&dump_.q_[code], &dump_.z_[code], kQRegSizeInBytes) == 0;
      match = match && z_match;
    }
    return match;
  }

  // Record the CPUFeatures enabled when Dump was called.
  CPUFeatures dump_cpu_features_;

  // Convenience pass-through for CPU feature checks.
  bool CPUHas(CPUFeatures::Feature feature0,
              CPUFeatures::Feature feature1 = CPUFeatures::kNone,
              CPUFeatures::Feature feature2 = CPUFeatures::kNone,
              CPUFeatures::Feature feature3 = CPUFeatures::kNone) const {
    return dump_cpu_features_.Has(feature0, feature1, feature2, feature3);
  }

  // Store all the dumped elements in a simple struct so the implementation can
  // use offsetof to quickly find the correct field.
  struct dump_t {
    // Core registers.
    uint64_t x_[kNumberOfRegisters];
    uint32_t w_[kNumberOfRegisters];

    // Floating-point registers, as raw bits.
    uint64_t d_[kNumberOfVRegisters];
    uint32_t s_[kNumberOfVRegisters];
    uint16_t h_[kNumberOfVRegisters];

    // Vector registers.
    QRegisterValue q_[kNumberOfVRegisters];
    ZRegisterValue z_[kNumberOfZRegisters];

    PRegisterValue p_[kNumberOfPRegisters];

    // The stack pointer.
    uint64_t sp_;
    uint64_t wsp_;

    // NZCV flags, stored in bits 28 to 31.
    // bit[31] : Negative
    // bit[30] : Zero
    // bit[29] : Carry
    // bit[28] : oVerflow
    uint64_t flags_;

    // The SVE "VL" (vector length) in bits.
    uint64_t vl_;
  } dump_;
};

// Some tests want to check that a value is _not_ equal to a reference value.
// These enum values can be used to control the error reporting behaviour.
enum ExpectedResult { kExpectEqual, kExpectNotEqual };

// The Equal* methods return true if the result matches the reference value.
// They all print an error message to the console if the result is incorrect
// (according to the ExpectedResult argument, or kExpectEqual if it is absent).
//
// Some of these methods don't use the RegisterDump argument, but they have to
// accept them so that they can overload those that take register arguments.
bool Equal32(uint32_t expected, const RegisterDump*, uint32_t result);
bool Equal64(uint64_t reference,
             const RegisterDump*,
             uint64_t result,
             ExpectedResult option = kExpectEqual);
bool Equal64(std::vector<uint64_t> reference_list,
             const RegisterDump*,
             uint64_t result,
             ExpectedResult option = kExpectEqual);
bool Equal128(QRegisterValue expected,
              const RegisterDump*,
              QRegisterValue result);

bool EqualFP16(Float16 expected, const RegisterDump*, uint16_t result);
bool EqualFP32(float expected, const RegisterDump*, float result);
bool EqualFP64(double expected, const RegisterDump*, double result);

bool Equal32(uint32_t expected, const RegisterDump* core, const Register& reg);
bool Equal64(uint64_t reference,
             const RegisterDump* core,
             const Register& reg,
             ExpectedResult option = kExpectEqual);
bool Equal64(std::vector<uint64_t> reference_list,
             const RegisterDump* core,
             const Register& reg,
             ExpectedResult option = kExpectEqual);
bool Equal64(uint64_t expected,
             const RegisterDump* core,
             const VRegister& vreg);

bool EqualFP16(Float16 expected,
               const RegisterDump* core,
               const VRegister& fpreg);
bool EqualFP32(float expected,
               const RegisterDump* core,
               const VRegister& fpreg);
bool EqualFP64(double expected,
               const RegisterDump* core,
               const VRegister& fpreg);

bool Equal64(const Register& reg0,
             const RegisterDump* core,
             const Register& reg1,
             ExpectedResult option = kExpectEqual);
bool Equal128(uint64_t expected_h,
              uint64_t expected_l,
              const RegisterDump* core,
              const VRegister& reg);

bool EqualNzcv(uint32_t expected, uint32_t result);

bool EqualRegisters(const RegisterDump* a, const RegisterDump* b);

template <typename T0, typename T1>
bool NotEqual64(T0 reference, const RegisterDump* core, T1 result) {
  return !Equal64(reference, core, result, kExpectNotEqual);
}

bool EqualSVELane(uint64_t expected,
                  const RegisterDump* core,
                  const ZRegister& reg,
                  int lane);

bool EqualSVELane(uint64_t expected,
                  const RegisterDump* core,
                  const PRegister& reg,
                  int lane);

// Check that each SVE lane matches the corresponding expected[] value. The
// highest-indexed array element maps to the lowest-numbered lane.
template <typename T, int N, typename R>
bool EqualSVE(const T (&expected)[N],
              const RegisterDump* core,
              const R& reg,
              bool* printed_warning) {
  VIXL_ASSERT(reg.IsZRegister() || reg.IsPRegister());
  VIXL_ASSERT(reg.HasLaneSize());
  // Evaluate and report errors on every lane, rather than just the first.
  bool equal = true;
  for (int lane = 0; lane < N; ++lane) {
    if (!core->HasSVELane(reg, lane)) {
      if (*printed_warning == false) {
        *printed_warning = true;
        printf(
            "Warning: Ignoring SVE lanes beyond VL (%d bytes) "
            "because the CPU does not implement them.\n",
            core->GetSVELaneCount(kBRegSize));
      }
      break;
    }
    // Map the highest-indexed array element to the lowest-numbered lane.
    equal = EqualSVELane(expected[N - lane - 1], core, reg, lane) && equal;
  }
  return equal;
}

// Check that each SVE lanes matches the `expected` value.
template <typename R>
bool EqualSVE(uint64_t expected,
              const RegisterDump* core,
              const R& reg,
              bool* printed_warning) {
  VIXL_ASSERT(reg.IsZRegister() || reg.IsPRegister());
  VIXL_ASSERT(reg.HasLaneSize());
  USE(printed_warning);
  // Evaluate and report errors on every lane, rather than just the first.
  bool equal = true;
  for (int lane = 0; lane < core->GetSVELaneCount(reg.GetLaneSizeInBits());
       ++lane) {
    equal = EqualSVELane(expected, core, reg, lane) && equal;
  }
  return equal;
}

// Check that two Z or P registers are equal.
template <typename R>
bool EqualSVE(const R& expected,
              const RegisterDump* core,
              const R& result,
              bool* printed_warning) {
  VIXL_ASSERT(result.IsZRegister() || result.IsPRegister());
  VIXL_ASSERT(AreSameFormat(expected, result));
  USE(printed_warning);

  // If the lane size is omitted, pick a default.
  if (!result.HasLaneSize()) {
    return EqualSVE(expected.VnB(), core, result.VnB(), printed_warning);
  }

  // Evaluate and report errors on every lane, rather than just the first.
  bool equal = true;
  int lane_size = result.GetLaneSizeInBits();
  for (int lane = 0; lane < core->GetSVELaneCount(lane_size); ++lane) {
    uint64_t expected_lane = core->GetSVELane(expected, lane);
    equal = equal && EqualSVELane(expected_lane, core, result, lane);
  }
  return equal;
}

bool EqualMemory(const void* expected,
                 const void* result,
                 size_t size_in_bytes,
                 size_t zero_offset = 0);

// Populate the w, x and r arrays with registers from the 'allowed' mask. The
// r array will be populated with <reg_size>-sized registers,
//
// This allows for tests which use large, parameterized blocks of registers
// (such as the push and pop tests), but where certain registers must be
// avoided as they are used for other purposes.
//
// Any of w, x, or r can be NULL if they are not required.
//
// The return value is a RegList indicating which registers were allocated.
RegList PopulateRegisterArray(Register* w,
                              Register* x,
                              Register* r,
                              int reg_size,
                              int reg_count,
                              RegList allowed);

// As PopulateRegisterArray, but for floating-point registers.
RegList PopulateVRegisterArray(VRegister* s,
                               VRegister* d,
                               VRegister* v,
                               int reg_size,
                               int reg_count,
                               RegList allowed);

// Overwrite the contents of the specified registers. This enables tests to
// check that register contents are written in cases where it's likely that the
// correct outcome could already be stored in the register.
//
// This always overwrites X-sized registers. If tests are operating on W
// registers, a subsequent write into an aliased W register should clear the
// top word anyway, so clobbering the full X registers should make tests more
// rigorous.
void Clobber(MacroAssembler* masm,
             RegList reg_list,
             uint64_t const value = 0xfedcba9876543210);

// As Clobber, but for FP registers.
void ClobberFP(MacroAssembler* masm,
               RegList reg_list,
               double const value = kFP64SignallingNaN);

// As Clobber, but for a CPURegList with either FP or integer registers. When
// using this method, the clobber value is always the default for the basic
// Clobber or ClobberFP functions.
void Clobber(MacroAssembler* masm, CPURegList reg_list);

uint64_t GetSignallingNan(int size_in_bits);

// This class acts as a drop-in replacement for VIXL's MacroAssembler, giving
// CalculateSVEAddress public visibility.
//
// CalculateSVEAddress normally has protected visibility, but it's useful to
// test it in isolation because it is the basis of all SVE non-scatter-gather
// load and store fall-backs.
class CalculateSVEAddressMacroAssembler : public vixl::aarch64::MacroAssembler {
 public:
  void CalculateSVEAddress(const Register& xd,
                           const SVEMemOperand& addr,
                           int vl_divisor_log2) {
    MacroAssembler::CalculateSVEAddress(xd, addr, vl_divisor_log2);
  }

  void CalculateSVEAddress(const Register& xd, const SVEMemOperand& addr) {
    MacroAssembler::CalculateSVEAddress(xd, addr);
  }
};

// This class acts as a drop-in replacement for VIXL's MacroAssembler, with
// fast NaN proparation mode switched on.
class FastNaNPropagationMacroAssembler : public MacroAssembler {
 public:
  FastNaNPropagationMacroAssembler() {
    SetFPNaNPropagationOption(FastNaNPropagation);
  }
};

// This class acts as a drop-in replacement for VIXL's MacroAssembler, with
// strict NaN proparation mode switched on.
class StrictNaNPropagationMacroAssembler : public MacroAssembler {
 public:
  StrictNaNPropagationMacroAssembler() {
    SetFPNaNPropagationOption(StrictNaNPropagation);
  }
};

// If the required features are available, return true.
// Otherwise:
//  - Print a warning message, unless *queried_can_run indicates that we've
//    already done so.
//  - Return false.
//
// If *queried_can_run is NULL, it is treated as false. Otherwise, it is set to
// true, regardless of the return value.
//
// The warning message printed on failure is used by tools/threaded_tests.py to
// count skipped tests. A test must not print more than one such warning
// message. It is safe to call CanRun multiple times per test, as long as
// queried_can_run is propagated correctly between calls, and the first call to
// CanRun requires every feature that is required by subsequent calls. If
// queried_can_run is NULL, CanRun must not be called more than once per test.
bool CanRun(const CPUFeatures& required, bool* queried_can_run = NULL);

// PushCalleeSavedRegisters(), PopCalleeSavedRegisters() and Dump() use NEON, so
// we need to enable it in the infrastructure code for each test.
static const CPUFeatures kInfrastructureCPUFeatures(CPUFeatures::kNEON);

enum InputSet {
  kIntInputSet = 0,
  kFpInputSet,
};

// Initialise CPU registers to a predictable, non-zero set of values. This
// sets core, vector, predicate and flag registers, though leaves the stack
// pointer at its original value.
void SetInitialMachineState(MacroAssembler* masm,
                            InputSet input_set = kIntInputSet);

// Compute a CRC32 hash of the machine state, and store it to dst. The hash
// covers core (not sp), vector (lower 128 bits), predicate (lower 16 bits)
// and flag registers.
void ComputeMachineStateHash(MacroAssembler* masm, uint32_t* dst);

// The TEST_SVE macro works just like the usual TEST macro, but the resulting
// function receives a `const Test& config` argument, to allow it to query the
// vector length.
#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64

#define TEST_SVE_INNER(type, name)                                          \
  void Test##name(Test* config);                                            \
  Test* test_##name##_list[] = {Test::MakeSVETest(128,                      \
                                                  "AARCH64_" type "_" #name \
                                                  "_vl128",                 \
                                                  &Test##name),             \
                                Test::MakeSVETest(384,                      \
                                                  "AARCH64_" type "_" #name \
                                                  "_vl384",                 \
                                                  &Test##name),             \
                                Test::MakeSVETest(2048,                     \
                                                  "AARCH64_" type "_" #name \
                                                  "_vl2048",                \
                                                  &Test##name)};            \
  void Test##name(Test* config)

#define SVE_SETUP_WITH_FEATURES(...) \
  SETUP_WITH_FEATURES(__VA_ARGS__);  \
  simulator.SetVectorLengthInBits(config->sve_vl_in_bits())

#else
// Otherwise, just use whatever the hardware provides.
static const int kSVEVectorLengthInBits =
    CPUFeatures::InferFromOS().Has(CPUFeatures::kSVE)
        ? CPU::ReadSVEVectorLengthInBits()
        : kZRegMinSize;

#define TEST_SVE_INNER(type, name)                           \
  void Test##name(Test* config);                             \
  Test* test_##name##_vlauto =                               \
      Test::MakeSVETest(kSVEVectorLengthInBits,              \
                        "AARCH64_" type "_" #name "_vlauto", \
                        &Test##name);                        \
  void Test##name(Test* config)

#define SVE_SETUP_WITH_FEATURES(...) \
  SETUP_WITH_FEATURES(__VA_ARGS__);  \
  USE(config)

#endif

// Call masm->Insr repeatedly to allow test inputs to be set up concisely. This
// is optimised for call-site clarity, not generated code quality, so it doesn't
// exist in the MacroAssembler itself.
//
// Usage:
//
//    int values[] = { 42, 43, 44 };
//    InsrHelper(&masm, z0.VnS(), values);    // Sets z0.S = { ..., 42, 43, 44 }
//
// The rightmost (highest-indexed) array element maps to the lowest-numbered
// lane.
template <typename T, size_t N>
void InsrHelper(MacroAssembler* masm,
                const ZRegister& zdn,
                const T (&values)[N]) {
  for (size_t i = 0; i < N; i++) {
    masm->Insr(zdn, values[i]);
  }
}

}  // namespace aarch64
}  // namespace vixl

#endif  // VIXL_AARCH64_TEST_UTILS_AARCH64_H_

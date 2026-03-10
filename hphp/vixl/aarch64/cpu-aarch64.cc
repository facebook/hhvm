// Copyright 2015, VIXL authors
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

#if defined(__aarch64__) && (defined(__ANDROID__) || defined(__linux__))
#include <sys/auxv.h>
#define VIXL_USE_LINUX_HWCAP 1
#endif

#include "../utils-vixl.h"

#include "cpu-aarch64.h"

namespace vixl {
namespace aarch64 {


const IDRegister::Field AA64PFR0::kFP(16, Field::kSigned);
const IDRegister::Field AA64PFR0::kAdvSIMD(20, Field::kSigned);
const IDRegister::Field AA64PFR0::kRAS(28);
const IDRegister::Field AA64PFR0::kSVE(32);
const IDRegister::Field AA64PFR0::kDIT(48);
const IDRegister::Field AA64PFR0::kCSV2(56);
const IDRegister::Field AA64PFR0::kCSV3(60);

const IDRegister::Field AA64PFR1::kBT(0);
const IDRegister::Field AA64PFR1::kSSBS(4);
const IDRegister::Field AA64PFR1::kMTE(8);
const IDRegister::Field AA64PFR1::kSME(24);

const IDRegister::Field AA64ISAR0::kAES(4);
const IDRegister::Field AA64ISAR0::kSHA1(8);
const IDRegister::Field AA64ISAR0::kSHA2(12);
const IDRegister::Field AA64ISAR0::kCRC32(16);
const IDRegister::Field AA64ISAR0::kAtomic(20);
const IDRegister::Field AA64ISAR0::kRDM(28);
const IDRegister::Field AA64ISAR0::kSHA3(32);
const IDRegister::Field AA64ISAR0::kSM3(36);
const IDRegister::Field AA64ISAR0::kSM4(40);
const IDRegister::Field AA64ISAR0::kDP(44);
const IDRegister::Field AA64ISAR0::kFHM(48);
const IDRegister::Field AA64ISAR0::kTS(52);
const IDRegister::Field AA64ISAR0::kRNDR(60);

const IDRegister::Field AA64ISAR1::kDPB(0);
const IDRegister::Field AA64ISAR1::kAPA(4);
const IDRegister::Field AA64ISAR1::kAPI(8);
const IDRegister::Field AA64ISAR1::kJSCVT(12);
const IDRegister::Field AA64ISAR1::kFCMA(16);
const IDRegister::Field AA64ISAR1::kLRCPC(20);
const IDRegister::Field AA64ISAR1::kGPA(24);
const IDRegister::Field AA64ISAR1::kGPI(28);
const IDRegister::Field AA64ISAR1::kFRINTTS(32);
const IDRegister::Field AA64ISAR1::kSB(36);
const IDRegister::Field AA64ISAR1::kSPECRES(40);
const IDRegister::Field AA64ISAR1::kBF16(44);
const IDRegister::Field AA64ISAR1::kDGH(48);
const IDRegister::Field AA64ISAR1::kI8MM(52);

const IDRegister::Field AA64ISAR2::kWFXT(0);
const IDRegister::Field AA64ISAR2::kRPRES(4);
const IDRegister::Field AA64ISAR2::kMOPS(16);
const IDRegister::Field AA64ISAR2::kCSSC(52);

const IDRegister::Field AA64MMFR0::kECV(60);

const IDRegister::Field AA64MMFR1::kLO(16);
const IDRegister::Field AA64MMFR1::kAFP(44);

const IDRegister::Field AA64MMFR2::kAT(32);

const IDRegister::Field AA64ZFR0::kSVEver(0);
const IDRegister::Field AA64ZFR0::kAES(4);
const IDRegister::Field AA64ZFR0::kBitPerm(16);
const IDRegister::Field AA64ZFR0::kBF16(20);
const IDRegister::Field AA64ZFR0::kSHA3(32);
const IDRegister::Field AA64ZFR0::kSM4(40);
const IDRegister::Field AA64ZFR0::kI8MM(44);
const IDRegister::Field AA64ZFR0::kF32MM(52);
const IDRegister::Field AA64ZFR0::kF64MM(56);

const IDRegister::Field AA64SMFR0::kSMEf32f32(32, 1);
const IDRegister::Field AA64SMFR0::kSMEb16f32(34, 1);
const IDRegister::Field AA64SMFR0::kSMEf16f32(35, 1);
const IDRegister::Field AA64SMFR0::kSMEi8i32(36);
const IDRegister::Field AA64SMFR0::kSMEf64f64(48, 1);
const IDRegister::Field AA64SMFR0::kSMEi16i64(52);
const IDRegister::Field AA64SMFR0::kSMEfa64(63, 1);

CPUFeatures AA64PFR0::GetCPUFeatures() const {
  CPUFeatures f;
  if (Get(kFP) >= 0) f.Combine(CPUFeatures::kFP);
  if (Get(kFP) >= 1) f.Combine(CPUFeatures::kFPHalf);
  if (Get(kAdvSIMD) >= 0) f.Combine(CPUFeatures::kNEON);
  if (Get(kAdvSIMD) >= 1) f.Combine(CPUFeatures::kNEONHalf);
  if (Get(kRAS) >= 1) f.Combine(CPUFeatures::kRAS);
  if (Get(kSVE) >= 1) f.Combine(CPUFeatures::kSVE);
  if (Get(kDIT) >= 1) f.Combine(CPUFeatures::kDIT);
  if (Get(kCSV2) >= 1) f.Combine(CPUFeatures::kCSV2);
  if (Get(kCSV2) >= 2) f.Combine(CPUFeatures::kSCXTNUM);
  if (Get(kCSV3) >= 1) f.Combine(CPUFeatures::kCSV3);
  return f;
}

CPUFeatures AA64PFR1::GetCPUFeatures() const {
  CPUFeatures f;
  if (Get(kBT) >= 1) f.Combine(CPUFeatures::kBTI);
  if (Get(kSSBS) >= 1) f.Combine(CPUFeatures::kSSBS);
  if (Get(kSSBS) >= 2) f.Combine(CPUFeatures::kSSBSControl);
  if (Get(kMTE) >= 1) f.Combine(CPUFeatures::kMTEInstructions);
  if (Get(kMTE) >= 2) f.Combine(CPUFeatures::kMTE);
  if (Get(kMTE) >= 3) f.Combine(CPUFeatures::kMTE3);
  if (Get(kSME) >= 1) f.Combine(CPUFeatures::kSME);
  return f;
}

CPUFeatures AA64ISAR0::GetCPUFeatures() const {
  CPUFeatures f;
  if (Get(kAES) >= 1) f.Combine(CPUFeatures::kAES);
  if (Get(kAES) >= 2) f.Combine(CPUFeatures::kPmull1Q);
  if (Get(kSHA1) >= 1) f.Combine(CPUFeatures::kSHA1);
  if (Get(kSHA2) >= 1) f.Combine(CPUFeatures::kSHA2);
  if (Get(kSHA2) >= 2) f.Combine(CPUFeatures::kSHA512);
  if (Get(kCRC32) >= 1) f.Combine(CPUFeatures::kCRC32);
  if (Get(kAtomic) >= 1) f.Combine(CPUFeatures::kAtomics);
  if (Get(kRDM) >= 1) f.Combine(CPUFeatures::kRDM);
  if (Get(kSHA3) >= 1) f.Combine(CPUFeatures::kSHA3);
  if (Get(kSM3) >= 1) f.Combine(CPUFeatures::kSM3);
  if (Get(kSM4) >= 1) f.Combine(CPUFeatures::kSM4);
  if (Get(kDP) >= 1) f.Combine(CPUFeatures::kDotProduct);
  if (Get(kFHM) >= 1) f.Combine(CPUFeatures::kFHM);
  if (Get(kTS) >= 1) f.Combine(CPUFeatures::kFlagM);
  if (Get(kTS) >= 2) f.Combine(CPUFeatures::kAXFlag);
  if (Get(kRNDR) >= 1) f.Combine(CPUFeatures::kRNG);
  return f;
}

CPUFeatures AA64ISAR1::GetCPUFeatures() const {
  CPUFeatures f;
  if (Get(kDPB) >= 1) f.Combine(CPUFeatures::kDCPoP);
  if (Get(kDPB) >= 2) f.Combine(CPUFeatures::kDCCVADP);
  if (Get(kJSCVT) >= 1) f.Combine(CPUFeatures::kJSCVT);
  if (Get(kFCMA) >= 1) f.Combine(CPUFeatures::kFcma);
  if (Get(kLRCPC) >= 1) f.Combine(CPUFeatures::kRCpc);
  if (Get(kLRCPC) >= 2) f.Combine(CPUFeatures::kRCpcImm);
  if (Get(kFRINTTS) >= 1) f.Combine(CPUFeatures::kFrintToFixedSizedInt);
  if (Get(kSB) >= 1) f.Combine(CPUFeatures::kSB);
  if (Get(kSPECRES) >= 1) f.Combine(CPUFeatures::kSPECRES);
  if (Get(kBF16) >= 1) f.Combine(CPUFeatures::kBF16);
  if (Get(kBF16) >= 2) f.Combine(CPUFeatures::kEBF16);
  if (Get(kDGH) >= 1) f.Combine(CPUFeatures::kDGH);
  if (Get(kI8MM) >= 1) f.Combine(CPUFeatures::kI8MM);

  // Only one of these fields should be non-zero, but they have the same
  // encodings, so merge the logic.
  int apx = std::max(Get(kAPI), Get(kAPA));
  if (apx >= 1) {
    f.Combine(CPUFeatures::kPAuth);
    // APA (rather than API) indicates QARMA.
    if (Get(kAPA) >= 1) f.Combine(CPUFeatures::kPAuthQARMA);
    if (apx == 0b0010) f.Combine(CPUFeatures::kPAuthEnhancedPAC);
    if (apx >= 0b0011) f.Combine(CPUFeatures::kPAuthEnhancedPAC2);
    if (apx >= 0b0100) f.Combine(CPUFeatures::kPAuthFPAC);
    if (apx >= 0b0101) f.Combine(CPUFeatures::kPAuthFPACCombined);
  }

  if (Get(kGPI) >= 1) f.Combine(CPUFeatures::kPAuthGeneric);
  if (Get(kGPA) >= 1) {
    f.Combine(CPUFeatures::kPAuthGeneric, CPUFeatures::kPAuthGenericQARMA);
  }
  return f;
}

CPUFeatures AA64ISAR2::GetCPUFeatures() const {
  CPUFeatures f;
  if (Get(kWFXT) >= 2) f.Combine(CPUFeatures::kWFXT);
  if (Get(kRPRES) >= 1) f.Combine(CPUFeatures::kRPRES);
  if (Get(kMOPS) >= 1) f.Combine(CPUFeatures::kMOPS);
  if (Get(kCSSC) >= 1) f.Combine(CPUFeatures::kCSSC);
  return f;
}

CPUFeatures AA64MMFR0::GetCPUFeatures() const {
  CPUFeatures f;
  if (Get(kECV) >= 1) f.Combine(CPUFeatures::kECV);
  return f;
}

CPUFeatures AA64MMFR1::GetCPUFeatures() const {
  CPUFeatures f;
  if (Get(kLO) >= 1) f.Combine(CPUFeatures::kLORegions);
  if (Get(kAFP) >= 1) f.Combine(CPUFeatures::kAFP);
  return f;
}

CPUFeatures AA64MMFR2::GetCPUFeatures() const {
  CPUFeatures f;
  if (Get(kAT) >= 1) f.Combine(CPUFeatures::kUSCAT);
  return f;
}

CPUFeatures AA64ZFR0::GetCPUFeatures() const {
  // This register is only available with SVE, but reads-as-zero in its absence,
  // so it's always safe to read it.
  CPUFeatures f;
  if (Get(kF64MM) >= 1) f.Combine(CPUFeatures::kSVEF64MM);
  if (Get(kF32MM) >= 1) f.Combine(CPUFeatures::kSVEF32MM);
  if (Get(kI8MM) >= 1) f.Combine(CPUFeatures::kSVEI8MM);
  if (Get(kSM4) >= 1) f.Combine(CPUFeatures::kSVESM4);
  if (Get(kSHA3) >= 1) f.Combine(CPUFeatures::kSVESHA3);
  if (Get(kBF16) >= 1) f.Combine(CPUFeatures::kSVEBF16);
  if (Get(kBF16) >= 2) f.Combine(CPUFeatures::kSVE_EBF16);
  if (Get(kBitPerm) >= 1) f.Combine(CPUFeatures::kSVEBitPerm);
  if (Get(kAES) >= 1) f.Combine(CPUFeatures::kSVEAES);
  if (Get(kAES) >= 2) f.Combine(CPUFeatures::kSVEPmull128);
  if (Get(kSVEver) >= 1) f.Combine(CPUFeatures::kSVE2);
  return f;
}

CPUFeatures AA64SMFR0::GetCPUFeatures() const {
  CPUFeatures f;
  if (Get(kSMEf32f32) >= 1) f.Combine(CPUFeatures::kSMEf32f32);
  if (Get(kSMEb16f32) >= 1) f.Combine(CPUFeatures::kSMEb16f32);
  if (Get(kSMEf16f32) >= 1) f.Combine(CPUFeatures::kSMEf16f32);
  if (Get(kSMEi8i32) >= 15) f.Combine(CPUFeatures::kSMEi8i32);
  if (Get(kSMEf64f64) >= 1) f.Combine(CPUFeatures::kSMEf64f64);
  if (Get(kSMEi16i64) >= 15) f.Combine(CPUFeatures::kSMEi16i64);
  if (Get(kSMEfa64) >= 1) f.Combine(CPUFeatures::kSMEfa64);
  return f;
}

int IDRegister::Get(IDRegister::Field field) const {
  int msb = field.GetMsb();
  int lsb = field.GetLsb();
  VIXL_STATIC_ASSERT(static_cast<size_t>(Field::kMaxWidthInBits) <
                     (sizeof(int) * kBitsPerByte));
  switch (field.GetType()) {
    case Field::kSigned:
      return static_cast<int>(ExtractSignedBitfield64(msb, lsb, value_));
    case Field::kUnsigned:
      return static_cast<int>(ExtractUnsignedBitfield64(msb, lsb, value_));
  }
  VIXL_UNREACHABLE();
  return 0;
}

CPUFeatures CPU::InferCPUFeaturesFromIDRegisters() {
  CPUFeatures f;
#define VIXL_COMBINE_ID_REG(NAME, MRS_ARG) \
  f.Combine(Read##NAME().GetCPUFeatures());
  VIXL_AARCH64_ID_REG_LIST(VIXL_COMBINE_ID_REG)
#undef VIXL_COMBINE_ID_REG
  return f;
}

CPUFeatures CPU::InferCPUFeaturesFromOS(
    CPUFeatures::QueryIDRegistersOption option) {
  CPUFeatures features;

#ifdef VIXL_USE_LINUX_HWCAP
  // Map each set bit onto a feature. Ideally, we'd use HWCAP_* macros rather
  // than explicit bits, but explicit bits allow us to identify features that
  // the toolchain doesn't know about.
  static const CPUFeatures::Feature kFeatureBitsLow[] =
      {// Bits 0-7
       CPUFeatures::kFP,
       CPUFeatures::kNEON,
       CPUFeatures::kNone,  // "EVTSTRM", which VIXL doesn't track.
       CPUFeatures::kAES,
       CPUFeatures::kPmull1Q,
       CPUFeatures::kSHA1,
       CPUFeatures::kSHA2,
       CPUFeatures::kCRC32,
       // Bits 8-15
       CPUFeatures::kAtomics,
       CPUFeatures::kFPHalf,
       CPUFeatures::kNEONHalf,
       CPUFeatures::kIDRegisterEmulation,
       CPUFeatures::kRDM,
       CPUFeatures::kJSCVT,
       CPUFeatures::kFcma,
       CPUFeatures::kRCpc,
       // Bits 16-23
       CPUFeatures::kDCPoP,
       CPUFeatures::kSHA3,
       CPUFeatures::kSM3,
       CPUFeatures::kSM4,
       CPUFeatures::kDotProduct,
       CPUFeatures::kSHA512,
       CPUFeatures::kSVE,
       CPUFeatures::kFHM,
       // Bits 24-31
       CPUFeatures::kDIT,
       CPUFeatures::kUSCAT,
       CPUFeatures::kRCpcImm,
       CPUFeatures::kFlagM,
       CPUFeatures::kSSBSControl,
       CPUFeatures::kSB,
       CPUFeatures::kPAuth,
       CPUFeatures::kPAuthGeneric};
  VIXL_STATIC_ASSERT(ArrayLength(kFeatureBitsLow) < 64);

  static const CPUFeatures::Feature kFeatureBitsHigh[] =
      {// Bits 0-7
       CPUFeatures::kDCCVADP,
       CPUFeatures::kSVE2,
       CPUFeatures::kSVEAES,
       CPUFeatures::kSVEPmull128,
       CPUFeatures::kSVEBitPerm,
       CPUFeatures::kSVESHA3,
       CPUFeatures::kSVESM4,
       CPUFeatures::kAXFlag,
       // Bits 8-15
       CPUFeatures::kFrintToFixedSizedInt,
       CPUFeatures::kSVEI8MM,
       CPUFeatures::kSVEF32MM,
       CPUFeatures::kSVEF64MM,
       CPUFeatures::kSVEBF16,
       CPUFeatures::kI8MM,
       CPUFeatures::kBF16,
       CPUFeatures::kDGH,
       // Bits 16-23
       CPUFeatures::kRNG,
       CPUFeatures::kBTI,
       CPUFeatures::kMTE,
       CPUFeatures::kECV,
       CPUFeatures::kAFP,
       CPUFeatures::kRPRES,
       CPUFeatures::kMTE3,
       CPUFeatures::kSME,
       // Bits 24-31
       CPUFeatures::kSMEi16i64,
       CPUFeatures::kSMEf64f64,
       CPUFeatures::kSMEi8i32,
       CPUFeatures::kSMEf16f32,
       CPUFeatures::kSMEb16f32,
       CPUFeatures::kSMEf32f32,
       CPUFeatures::kSMEfa64,
       CPUFeatures::kWFXT,
       // Bits 32-39
       CPUFeatures::kEBF16,
       CPUFeatures::kSVE_EBF16};
  VIXL_STATIC_ASSERT(ArrayLength(kFeatureBitsHigh) < 64);

  auto combine_features = [&features](uint64_t hwcap,
                                      const CPUFeatures::Feature* feature_array,
                                      size_t features_size) {
    for (size_t i = 0; i < features_size; i++) {
      if (hwcap & (UINT64_C(1) << i)) features.Combine(feature_array[i]);
    }
  };

  uint64_t hwcap_low = getauxval(AT_HWCAP);
  uint64_t hwcap_high = getauxval(AT_HWCAP2);

  combine_features(hwcap_low, kFeatureBitsLow, ArrayLength(kFeatureBitsLow));
  combine_features(hwcap_high, kFeatureBitsHigh, ArrayLength(kFeatureBitsHigh));

  // MTE support from HWCAP2 signifies FEAT_MTE1 and FEAT_MTE2 support
  if (features.Has(CPUFeatures::kMTE)) {
    features.Combine(CPUFeatures::kMTEInstructions);
  }
#endif  // VIXL_USE_LINUX_HWCAP

  if ((option == CPUFeatures::kQueryIDRegistersIfAvailable) &&
      (features.Has(CPUFeatures::kIDRegisterEmulation))) {
    features.Combine(InferCPUFeaturesFromIDRegisters());
  }
  return features;
}


#ifdef __aarch64__
#define VIXL_READ_ID_REG(NAME, MRS_ARG)        \
  NAME CPU::Read##NAME() {                     \
    uint64_t value = 0;                        \
    __asm__("mrs %0, " MRS_ARG : "=r"(value)); \
    return NAME(value);                        \
  }
#else  // __aarch64__
#define VIXL_READ_ID_REG(NAME, MRS_ARG) \
  NAME CPU::Read##NAME() {              \
    VIXL_UNREACHABLE();                 \
    return NAME(0);                     \
  }
#endif  // __aarch64__

VIXL_AARCH64_ID_REG_LIST(VIXL_READ_ID_REG)

#undef VIXL_READ_ID_REG


// Initialise to smallest possible cache size.
unsigned CPU::dcache_line_size_ = 1;
unsigned CPU::icache_line_size_ = 1;


// Currently computes I and D cache line size.
void CPU::SetUp() {
  uint32_t cache_type_register = GetCacheType();

  // The cache type register holds information about the caches, including I
  // D caches line size.
  static const int kDCacheLineSizeShift = 16;
  static const int kICacheLineSizeShift = 0;
  static const uint32_t kDCacheLineSizeMask = 0xf << kDCacheLineSizeShift;
  static const uint32_t kICacheLineSizeMask = 0xf << kICacheLineSizeShift;

  // The cache type register holds the size of the I and D caches in words as
  // a power of two.
  uint32_t dcache_line_size_power_of_two =
      (cache_type_register & kDCacheLineSizeMask) >> kDCacheLineSizeShift;
  uint32_t icache_line_size_power_of_two =
      (cache_type_register & kICacheLineSizeMask) >> kICacheLineSizeShift;

  dcache_line_size_ = 4 << dcache_line_size_power_of_two;
  icache_line_size_ = 4 << icache_line_size_power_of_two;
}


uint32_t CPU::GetCacheType() {
#ifdef __aarch64__
  uint64_t cache_type_register;
  // Copy the content of the cache type register to a core register.
  __asm__ __volatile__("mrs %[ctr], ctr_el0"  // NOLINT(runtime/references)
                       : [ctr] "=r"(cache_type_register));
  VIXL_ASSERT(IsUint32(cache_type_register));
  return static_cast<uint32_t>(cache_type_register);
#else
  // This will lead to a cache with 1 byte long lines, which is fine since
  // neither EnsureIAndDCacheCoherency nor the simulator will need this
  // information.
  return 0;
#endif
}


// Query the SVE vector length. This requires CPUFeatures::kSVE.
int CPU::ReadSVEVectorLengthInBits() {
#ifdef __aarch64__
  uint64_t vl;
  // To support compilers that don't understand `rdvl`, encode the value
  // directly and move it manually.
  __asm__(
      "   .word 0x04bf5100\n"  // rdvl x0, #8
      "   mov %[vl], x0\n"
      : [vl] "=r"(vl)
      :
      : "x0");
  VIXL_ASSERT(vl <= INT_MAX);
  return static_cast<int>(vl);
#else
  VIXL_UNREACHABLE();
  return 0;
#endif
}


void CPU::EnsureIAndDCacheCoherency(void* address, size_t length) {
#ifdef __aarch64__
  // Implement the cache synchronisation for all targets where AArch64 is the
  // host, even if we're building the simulator for an AAarch64 host. This
  // allows for cases where the user wants to simulate code as well as run it
  // natively.

  if (length == 0) {
    return;
  }

  // The code below assumes user space cache operations are allowed.

  // Work out the line sizes for each cache, and use them to determine the
  // start addresses.
  uintptr_t start = reinterpret_cast<uintptr_t>(address);
  uintptr_t dsize = static_cast<uintptr_t>(dcache_line_size_);
  uintptr_t isize = static_cast<uintptr_t>(icache_line_size_);
  uintptr_t dline = start & ~(dsize - 1);
  uintptr_t iline = start & ~(isize - 1);

  // Cache line sizes are always a power of 2.
  VIXL_ASSERT(IsPowerOf2(dsize));
  VIXL_ASSERT(IsPowerOf2(isize));
  uintptr_t end = start + length;

  do {
    __asm__ __volatile__(
        // Clean each line of the D cache containing the target data.
        //
        // dc       : Data Cache maintenance
        //     c    : Clean
        //      va  : by (Virtual) Address
        //        u : to the point of Unification
        // The point of unification for a processor is the point by which the
        // instruction and data caches are guaranteed to see the same copy of a
        // memory location. See ARM DDI 0406B page B2-12 for more information.
        "   dc    cvau, %[dline]\n"
        :
        : [dline] "r"(dline)
        // This code does not write to memory, but the "memory" dependency
        // prevents GCC from reordering the code.
        : "memory");
    dline += dsize;
  } while (dline < end);

  __asm__ __volatile__(
      // Make sure that the data cache operations (above) complete before the
      // instruction cache operations (below).
      //
      // dsb      : Data Synchronisation Barrier
      //      ish : Inner SHareable domain
      //
      // The point of unification for an Inner Shareable shareability domain is
      // the point by which the instruction and data caches of all the
      // processors
      // in that Inner Shareable shareability domain are guaranteed to see the
      // same copy of a memory location. See ARM DDI 0406B page B2-12 for more
      // information.
      "   dsb   ish\n"
      :
      :
      : "memory");

  do {
    __asm__ __volatile__(
        // Invalidate each line of the I cache containing the target data.
        //
        // ic      : Instruction Cache maintenance
        //    i    : Invalidate
        //     va  : by Address
        //       u : to the point of Unification
        "   ic   ivau, %[iline]\n"
        :
        : [iline] "r"(iline)
        : "memory");
    iline += isize;
  } while (iline < end);

  __asm__ __volatile__(
      // Make sure that the instruction cache operations (above) take effect
      // before the isb (below).
      "   dsb  ish\n"

      // Ensure that any instructions already in the pipeline are discarded and
      // reloaded from the new data.
      // isb : Instruction Synchronisation Barrier
      "   isb\n"
      :
      :
      : "memory");
#else
  // If the host isn't AArch64, we must be using the simulator, so this function
  // doesn't have to do anything.
  USE(address, length);
#endif
}


}  // namespace aarch64
}  // namespace vixl

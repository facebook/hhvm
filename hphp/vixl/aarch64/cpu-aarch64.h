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

#ifndef VIXL_CPU_AARCH64_H
#define VIXL_CPU_AARCH64_H

#include "../cpu-features.h"
#include "../globals-vixl.h"

#include "instructions-aarch64.h"
#include "simulator-aarch64.h"

#ifndef VIXL_INCLUDE_TARGET_AARCH64
// The supporting .cc file is only compiled when the A64 target is selected.
// Throw an explicit error now to avoid a harder-to-debug linker error later.
//
// These helpers _could_ work on any AArch64 host, even when generating AArch32
// code, but we don't support this because the available features may differ
// between AArch32 and AArch64 on the same platform, so basing AArch32 code
// generation on aarch64::CPU features is probably broken.
#error cpu-aarch64.h requires VIXL_INCLUDE_TARGET_AARCH64 (scons target=a64).
#endif

namespace vixl {
namespace aarch64 {

// A CPU ID register, for use with CPUFeatures::kIDRegisterEmulation. Fields
// specific to each register are described in relevant subclasses.
class IDRegister {
 protected:
  explicit IDRegister(uint64_t value = 0) : value_(value) {}

  class Field {
   public:
    enum Type { kUnsigned, kSigned };

    static const int kMaxWidthInBits = 4;

    // This needs to be constexpr so that fields have "constant initialisation".
    // This avoids initialisation order problems when these values are used to
    // (dynamically) initialise static variables, etc.
    explicit constexpr Field(int lsb,
                             int bitWidth = kMaxWidthInBits,
                             Type type = kUnsigned)
        : lsb_(lsb), bitWidth_(bitWidth), type_(type) {}

    int GetWidthInBits() const { return bitWidth_; }
    int GetLsb() const { return lsb_; }
    int GetMsb() const { return lsb_ + GetWidthInBits() - 1; }
    Type GetType() const { return type_; }

   private:
    int lsb_;
    int bitWidth_;
    Type type_;
  };

 public:
  // Extract the specified field, performing sign-extension for signed fields.
  // This allows us to implement the 'value >= number' detection mechanism
  // recommended by the Arm ARM, for both signed and unsigned fields.
  int Get(Field field) const;

 private:
  uint64_t value_;
};

class AA64PFR0 : public IDRegister {
 public:
  explicit AA64PFR0(uint64_t value) : IDRegister(value) {}

  CPUFeatures GetCPUFeatures() const;

 private:
  static const Field kFP;
  static const Field kAdvSIMD;
  static const Field kRAS;
  static const Field kSVE;
  static const Field kDIT;
  static const Field kCSV2;
  static const Field kCSV3;
};

class AA64PFR1 : public IDRegister {
 public:
  explicit AA64PFR1(uint64_t value) : IDRegister(value) {}

  CPUFeatures GetCPUFeatures() const;

 private:
  static const Field kBT;
  static const Field kSSBS;
  static const Field kMTE;
  static const Field kSME;
};

class AA64ISAR0 : public IDRegister {
 public:
  explicit AA64ISAR0(uint64_t value) : IDRegister(value) {}

  CPUFeatures GetCPUFeatures() const;

 private:
  static const Field kAES;
  static const Field kSHA1;
  static const Field kSHA2;
  static const Field kCRC32;
  static const Field kAtomic;
  static const Field kRDM;
  static const Field kSHA3;
  static const Field kSM3;
  static const Field kSM4;
  static const Field kDP;
  static const Field kFHM;
  static const Field kTS;
  static const Field kRNDR;
};

class AA64ISAR1 : public IDRegister {
 public:
  explicit AA64ISAR1(uint64_t value) : IDRegister(value) {}

  CPUFeatures GetCPUFeatures() const;

 private:
  static const Field kDPB;
  static const Field kAPA;
  static const Field kAPI;
  static const Field kJSCVT;
  static const Field kFCMA;
  static const Field kLRCPC;
  static const Field kGPA;
  static const Field kGPI;
  static const Field kFRINTTS;
  static const Field kSB;
  static const Field kSPECRES;
  static const Field kBF16;
  static const Field kDGH;
  static const Field kI8MM;
};

class AA64ISAR2 : public IDRegister {
 public:
  explicit AA64ISAR2(uint64_t value) : IDRegister(value) {}

  CPUFeatures GetCPUFeatures() const;

 private:
  static const Field kWFXT;
  static const Field kRPRES;
  static const Field kMOPS;
  static const Field kCSSC;
};

class AA64MMFR0 : public IDRegister {
 public:
  explicit AA64MMFR0(uint64_t value) : IDRegister(value) {}

  CPUFeatures GetCPUFeatures() const;

 private:
  static const Field kECV;
};

class AA64MMFR1 : public IDRegister {
 public:
  explicit AA64MMFR1(uint64_t value) : IDRegister(value) {}

  CPUFeatures GetCPUFeatures() const;

 private:
  static const Field kLO;
  static const Field kAFP;
};

class AA64MMFR2 : public IDRegister {
 public:
  explicit AA64MMFR2(uint64_t value) : IDRegister(value) {}

  CPUFeatures GetCPUFeatures() const;

 private:
  static const Field kAT;
};

class AA64ZFR0 : public IDRegister {
 public:
  explicit AA64ZFR0(uint64_t value) : IDRegister(value) {}

  CPUFeatures GetCPUFeatures() const;

 private:
  static const Field kSVEver;
  static const Field kAES;
  static const Field kBitPerm;
  static const Field kBF16;
  static const Field kSHA3;
  static const Field kSM4;
  static const Field kI8MM;
  static const Field kF32MM;
  static const Field kF64MM;
};

class AA64SMFR0 : public IDRegister {
 public:
  explicit AA64SMFR0(uint64_t value) : IDRegister(value) {}

  CPUFeatures GetCPUFeatures() const;

 private:
  static const Field kSMEf32f32;
  static const Field kSMEb16f32;
  static const Field kSMEf16f32;
  static const Field kSMEi8i32;
  static const Field kSMEf64f64;
  static const Field kSMEi16i64;
  static const Field kSMEfa64;
};

class CPU {
 public:
  // Initialise CPU support.
  static void SetUp();

  // Ensures the data at a given address and with a given size is the same for
  // the I and D caches. I and D caches are not automatically coherent on ARM
  // so this operation is required before any dynamically generated code can
  // safely run.
  static void EnsureIAndDCacheCoherency(void *address, size_t length);

  // Read and interpret the ID registers. This requires
  // CPUFeatures::kIDRegisterEmulation, and therefore cannot be called on
  // non-AArch64 platforms.
  static CPUFeatures InferCPUFeaturesFromIDRegisters();

  // Read and interpret CPUFeatures reported by the OS. Failed queries (or
  // unsupported platforms) return an empty list. Note that this is
  // indistinguishable from a successful query on a platform that advertises no
  // features.
  //
  // Non-AArch64 hosts are considered to be unsupported platforms, and this
  // function returns an empty list.
  static CPUFeatures InferCPUFeaturesFromOS(
      CPUFeatures::QueryIDRegistersOption option =
          CPUFeatures::kQueryIDRegistersIfAvailable);

  // Query the SVE vector length. This requires CPUFeatures::kSVE.
  static int ReadSVEVectorLengthInBits();

  // Handle tagged pointers.
  template <typename T>
  static T SetPointerTag(T pointer, uint64_t tag) {
    VIXL_ASSERT(IsUintN(kAddressTagWidth, tag));

    // Use C-style casts to get static_cast behaviour for integral types (T),
    // and reinterpret_cast behaviour for other types.

    uint64_t raw = (uint64_t)pointer;
    VIXL_STATIC_ASSERT(sizeof(pointer) == sizeof(raw));

    raw = (raw & ~kAddressTagMask) | (tag << kAddressTagOffset);
    return (T)raw;
  }

  template <typename T>
  static uint64_t GetPointerTag(T pointer) {
    // Use C-style casts to get static_cast behaviour for integral types (T),
    // and reinterpret_cast behaviour for other types.

    uint64_t raw = (uint64_t)pointer;
    VIXL_STATIC_ASSERT(sizeof(pointer) == sizeof(raw));

    return (raw & kAddressTagMask) >> kAddressTagOffset;
  }

 private:
#define VIXL_AARCH64_ID_REG_LIST(V)                                           \
  V(AA64PFR0, "ID_AA64PFR0_EL1")                                              \
  V(AA64PFR1, "ID_AA64PFR1_EL1")                                              \
  V(AA64ISAR0, "ID_AA64ISAR0_EL1")                                            \
  V(AA64ISAR1, "ID_AA64ISAR1_EL1")                                            \
  V(AA64MMFR0, "ID_AA64MMFR0_EL1")                                            \
  V(AA64MMFR1, "ID_AA64MMFR1_EL1")                                            \
  /* These registers are RES0 in the baseline Arm8.0. We can always safely */ \
  /* read them, but some compilers don't accept the symbolic names. */        \
  V(AA64SMFR0, "S3_0_C0_C4_5")                                                \
  V(AA64ISAR2, "S3_0_C0_C6_2")                                                \
  V(AA64MMFR2, "S3_0_C0_C7_2")                                                \
  V(AA64ZFR0, "S3_0_C0_C4_4")

#define VIXL_READ_ID_REG(NAME, MRS_ARG) static NAME Read##NAME();
  // On native AArch64 platforms, read the named CPU ID registers. These require
  // CPUFeatures::kIDRegisterEmulation, and should not be called on non-AArch64
  // platforms.
  VIXL_AARCH64_ID_REG_LIST(VIXL_READ_ID_REG)
#undef VIXL_READ_ID_REG

  // Return the content of the cache type register.
  static uint32_t GetCacheType();

  // I and D cache line size in bytes.
  static unsigned icache_line_size_;
  static unsigned dcache_line_size_;
};

}  // namespace aarch64
}  // namespace vixl

#endif  // VIXL_CPU_AARCH64_H

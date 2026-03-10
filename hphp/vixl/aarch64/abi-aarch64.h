// Copyright 2016, VIXL authors
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

// The ABI features are only supported with C++11 or later.
#if __cplusplus >= 201103L
// This should not be defined manually.
#define VIXL_HAS_ABI_SUPPORT
#elif defined(VIXL_HAS_ABI_SUPPORT)
#error "The ABI support requires C++11 or later."
#endif

#ifdef VIXL_HAS_ABI_SUPPORT

#ifndef VIXL_AARCH64_ABI_AARCH64_H_
#define VIXL_AARCH64_ABI_AARCH64_H_

#include <algorithm>
#include <type_traits>

#include "../globals-vixl.h"

#include "instructions-aarch64.h"
#include "operands-aarch64.h"

namespace vixl {
namespace aarch64 {

// Class describing the AArch64 procedure call standard, as defined in "ARM
// Procedure Call Standard for the ARM 64-bit Architecture (AArch64)",
// release 1.0 (AAPCS below).
//
// The stages in the comments match the description in that document.
//
// Stage B does not apply to arguments handled by this class.
class ABI {
 public:
  explicit ABI(Register stack_pointer = sp) : stack_pointer_(stack_pointer) {
    // Stage A - Initialization
    Reset();
  }

  void Reset() {
    NGRN_ = 0;
    NSRN_ = 0;
    stack_offset_ = 0;
  }

  int GetStackSpaceRequired() { return stack_offset_; }

  // The logic is described in section 5.5 of the AAPCS.
  template <typename T>
  GenericOperand GetReturnGenericOperand() const {
    ABI abi(stack_pointer_);
    GenericOperand result = abi.GetNextParameterGenericOperand<T>();
    VIXL_ASSERT(result.IsCPURegister());
    return result;
  }

  // The logic is described in section 5.4.2 of the AAPCS.
  // The `GenericOperand` returned describes the location reserved for the
  // argument from the point of view of the callee.
  template <typename T>
  GenericOperand GetNextParameterGenericOperand() {
    const bool is_floating_point_type = std::is_floating_point<T>::value;
    const bool is_integral_type =
        std::is_integral<T>::value || std::is_enum<T>::value;
    const bool is_pointer_type = std::is_pointer<T>::value;
    int type_alignment = std::alignment_of<T>::value;

    // We only support basic types.
    VIXL_ASSERT(is_floating_point_type || is_integral_type || is_pointer_type);

    // To ensure we get the correct type of operand when simulating on a 32-bit
    // host, force the size of pointer types to the native AArch64 pointer size.
    unsigned size = is_pointer_type ? 8 : sizeof(T);
    // The size of the 'operand' reserved for the argument.
    unsigned operand_size = AlignUp(size, kWRegSizeInBytes);
    if (size > 8) {
      VIXL_UNIMPLEMENTED();
      return GenericOperand();
    }

    // Stage C.1
    if (is_floating_point_type && (NSRN_ < 8)) {
      return GenericOperand(VRegister(NSRN_++, size * kBitsPerByte));
    }
    // Stages C.2, C.3, and C.4: Unsupported. Caught by the assertions above.
    // Stages C.5 and C.6
    if (is_floating_point_type) {
      VIXL_STATIC_ASSERT(
          !is_floating_point_type ||
          (std::is_same<T, float>::value || std::is_same<T, double>::value));
      int offset = stack_offset_;
      stack_offset_ += 8;
      return GenericOperand(MemOperand(stack_pointer_, offset), operand_size);
    }
    // Stage C.7
    if ((is_integral_type || is_pointer_type) && (size <= 8) && (NGRN_ < 8)) {
      return GenericOperand(Register(NGRN_++, operand_size * kBitsPerByte));
    }
    // Stage C.8
    if (type_alignment == 16) {
      NGRN_ = AlignUp(NGRN_, 2);
    }
    // Stage C.9
    if (is_integral_type && (size == 16) && (NGRN_ < 7)) {
      VIXL_UNIMPLEMENTED();
      return GenericOperand();
    }
    // Stage C.10: Unsupported. Caught by the assertions above.
    // Stage C.11
    NGRN_ = 8;
    // Stage C.12
    stack_offset_ = AlignUp(stack_offset_, std::max(type_alignment, 8));
    // Stage C.13: Unsupported. Caught by the assertions above.
    // Stage C.14
    VIXL_ASSERT(size <= 8u);
    size = std::max(size, 8u);
    int offset = stack_offset_;
    stack_offset_ += size;
    return GenericOperand(MemOperand(stack_pointer_, offset), operand_size);
  }

 private:
  Register stack_pointer_;
  // Next General-purpose Register Number.
  int NGRN_;
  // Next SIMD and Floating-point Register Number.
  int NSRN_;
  // The acronym "NSAA" used in the standard refers to the "Next Stacked
  // Argument Address". Here we deal with offsets from the stack pointer.
  int stack_offset_;
};

template <>
inline GenericOperand ABI::GetReturnGenericOperand<void>() const {
  return GenericOperand();
}
}  // namespace aarch64
}  // namespace vixl

#endif  // VIXL_AARCH64_ABI_AARCH64_H_

#endif  // VIXL_HAS_ABI_SUPPORT

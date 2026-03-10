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

#ifndef VIXL_ASSEMBLER_BASE_H
#define VIXL_ASSEMBLER_BASE_H

#include "code-buffer-vixl.h"

// Microsoft Visual C++ defines a `mvn` macro that conflicts with our own
// definition.
#if defined(_MSC_VER) && defined(mvn)
#undef mvn
#endif

namespace vixl {

class CodeBufferCheckScope;

namespace internal {

class AssemblerBase {
 public:
  AssemblerBase() : allow_assembler_(false) {}
  explicit AssemblerBase(size_t capacity)
      : buffer_(capacity), allow_assembler_(false) {}
  AssemblerBase(byte* buffer, size_t capacity)
      : buffer_(buffer, capacity), allow_assembler_(false) {}

  virtual ~AssemblerBase() {}

  // Finalize a code buffer of generated instructions. This function must be
  // called before executing or copying code from the buffer.
  void FinalizeCode() { GetBuffer()->SetClean(); }

  ptrdiff_t GetCursorOffset() const { return GetBuffer().GetCursorOffset(); }

  // Return the address of the cursor.
  template <typename T>
  T GetCursorAddress() const {
    VIXL_STATIC_ASSERT(sizeof(T) >= sizeof(uintptr_t));
    return GetBuffer().GetOffsetAddress<T>(GetCursorOffset());
  }

  size_t GetSizeOfCodeGenerated() const { return GetCursorOffset(); }

  // Accessors.
  CodeBuffer* GetBuffer() { return &buffer_; }
  const CodeBuffer& GetBuffer() const { return buffer_; }
  bool AllowAssembler() const { return allow_assembler_; }

 protected:
  void SetAllowAssembler(bool allow) { allow_assembler_ = allow; }

  // CodeBufferCheckScope must be able to temporarily allow the assembler.
  friend class vixl::CodeBufferCheckScope;

  // Buffer where the code is emitted.
  CodeBuffer buffer_;

 private:
  bool allow_assembler_;

 public:
  // Deprecated public interface.

  // Return the address of an offset in the buffer.
  template <typename T>
  VIXL_DEPRECATED("GetBuffer().GetOffsetAddress<T>(offset)",
                  T GetOffsetAddress(ptrdiff_t offset) const) {
    return GetBuffer().GetOffsetAddress<T>(offset);
  }

  // Return the address of the start of the buffer.
  template <typename T>
  VIXL_DEPRECATED("GetBuffer().GetStartAddress<T>()",
                  T GetStartAddress() const) {
    return GetBuffer().GetOffsetAddress<T>(0);
  }
};

}  // namespace internal
}  // namespace vixl

#endif  // VIXL_ASSEMBLER_BASE_H

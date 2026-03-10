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


#ifndef VIXL_CODE_GENERATION_SCOPES_H_
#define VIXL_CODE_GENERATION_SCOPES_H_


#include "assembler-base-vixl.h"
#include "macro-assembler-interface.h"


namespace vixl {

// This scope will:
// - Allow code emission from the specified `Assembler`.
// - Optionally reserve space in the `CodeBuffer` (if it is managed by VIXL).
// - Optionally, on destruction, check the size of the generated code.
//   (The size can be either exact or a maximum size.)
class CodeBufferCheckScope {
 public:
  // Tell whether or not the scope needs to ensure the associated CodeBuffer
  // has enough space for the requested size.
  enum BufferSpacePolicy {
    kReserveBufferSpace,
    kDontReserveBufferSpace,

    // Deprecated, but kept for backward compatibility.
    kCheck = kReserveBufferSpace,
    kNoCheck = kDontReserveBufferSpace
  };

  // Tell whether or not the scope should assert the amount of code emitted
  // within the scope is consistent with the requested amount.
  enum SizePolicy {
    kNoAssert,    // Do not check the size of the code emitted.
    kExactSize,   // The code emitted must be exactly size bytes.
    kMaximumSize  // The code emitted must be at most size bytes.
  };

  // This constructor implicitly calls `Open` to initialise the scope
  // (`assembler` must not be `NULL`), so it is ready to use immediately after
  // it has been constructed.
  CodeBufferCheckScope(internal::AssemblerBase* assembler,
                       size_t size,
                       BufferSpacePolicy check_policy = kReserveBufferSpace,
                       SizePolicy size_policy = kMaximumSize)
      : CodeBufferCheckScope() {
    Open(assembler, size, check_policy, size_policy);
  }

  // This constructor does not implicitly initialise the scope. Instead, the
  // user is required to explicitly call the `Open` function before using the
  // scope.
  CodeBufferCheckScope()
      : assembler_(NULL),
        assert_policy_(kMaximumSize),
        limit_(0),
        previous_allow_assembler_(false),
        initialised_(false) {
    // Nothing to do.
  }

  virtual ~CodeBufferCheckScope() { Close(); }

  // This function performs the actual initialisation work.
  void Open(internal::AssemblerBase* assembler,
            size_t size,
            BufferSpacePolicy check_policy = kReserveBufferSpace,
            SizePolicy size_policy = kMaximumSize) {
    VIXL_ASSERT(!initialised_);
    VIXL_ASSERT(assembler != NULL);
    assembler_ = assembler;
    if (check_policy == kReserveBufferSpace) {
      assembler->GetBuffer()->EnsureSpaceFor(size);
    }
#ifdef VIXL_DEBUG
    limit_ = assembler_->GetSizeOfCodeGenerated() + size;
    assert_policy_ = size_policy;
    previous_allow_assembler_ = assembler_->AllowAssembler();
    assembler_->SetAllowAssembler(true);
#else
    USE(size_policy);
#endif
    initialised_ = true;
  }

  // This function performs the cleaning-up work. It must succeed even if the
  // scope has not been opened. It is safe to call multiple times.
  void Close() {
#ifdef VIXL_DEBUG
    if (!initialised_) {
      return;
    }
    assembler_->SetAllowAssembler(previous_allow_assembler_);
    switch (assert_policy_) {
      case kNoAssert:
        break;
      case kExactSize:
        VIXL_ASSERT(assembler_->GetSizeOfCodeGenerated() == limit_);
        break;
      case kMaximumSize:
        VIXL_ASSERT(assembler_->GetSizeOfCodeGenerated() <= limit_);
        break;
      default:
        VIXL_UNREACHABLE();
    }
#endif
    initialised_ = false;
  }

 protected:
  internal::AssemblerBase* assembler_;
  SizePolicy assert_policy_;
  size_t limit_;
  bool previous_allow_assembler_;
  bool initialised_;
};


// This scope will:
// - Do the same as `CodeBufferCheckSCope`, but:
//   - If managed by VIXL, always reserve space in the `CodeBuffer`.
//   - Always check the size (exact or maximum) of the generated code on
//     destruction.
// - Emit pools if the specified size would push them out of range.
// - Block pools emission for the duration of the scope.
// This scope allows the `Assembler` and `MacroAssembler` to be freely and
// safely mixed for its duration.
class EmissionCheckScope : public CodeBufferCheckScope {
 public:
  // This constructor implicitly calls `Open` (when `masm` is not `NULL`) to
  // initialise the scope, so it is ready to use immediately after it has been
  // constructed.
  EmissionCheckScope(MacroAssemblerInterface* masm,
                     size_t size,
                     SizePolicy size_policy = kMaximumSize)
      : EmissionCheckScope() {
    Open(masm, size, size_policy);
  }

  // This constructor does not implicitly initialise the scope. Instead, the
  // user is required to explicitly call the `Open` function before using the
  // scope.
  EmissionCheckScope() : masm_(nullptr), pool_policy_(kBlockPools) {}

  virtual ~EmissionCheckScope() { Close(); }

  enum PoolPolicy {
    // Do not forbid pool emission inside the scope. Pools will not be emitted
    // on `Open` either.
    kIgnorePools,
    // Force pools to be generated on `Open` if necessary and block their
    // emission inside the scope.
    kBlockPools,
    // Deprecated, but kept for backward compatibility.
    kCheckPools = kBlockPools
  };

  void Open(MacroAssemblerInterface* masm,
            size_t size,
            SizePolicy size_policy = kMaximumSize) {
    Open(masm, size, size_policy, kBlockPools);
  }

  void Close() {
    if (!initialised_) {
      return;
    }
    if (masm_ == NULL) {
      // Nothing to do.
      return;
    }
    // Perform the opposite of `Open`, which is:
    //   - Check the code generation limit was not exceeded.
    //   - Release the pools.
    CodeBufferCheckScope::Close();
    if (pool_policy_ == kBlockPools) {
      masm_->ReleasePools();
    }
    VIXL_ASSERT(!initialised_);
  }

 protected:
  void Open(MacroAssemblerInterface* masm,
            size_t size,
            SizePolicy size_policy,
            PoolPolicy pool_policy) {
    if (masm == NULL) {
      // Nothing to do.
      // We may reach this point in a context of conditional code generation.
      // See `aarch64::MacroAssembler::MoveImmediateHelper()` for an example.
      return;
    }
    masm_ = masm;
    pool_policy_ = pool_policy;
    if (pool_policy_ == kBlockPools) {
      // To avoid duplicating the work to check that enough space is available
      // in the buffer, do not use the more generic `EnsureEmitFor()`. It is
      // done below when opening `CodeBufferCheckScope`.
      masm->EnsureEmitPoolsFor(size);
      masm->BlockPools();
    }
    // The buffer should be checked *after* we emit the pools.
    CodeBufferCheckScope::Open(masm->AsAssemblerBase(),
                               size,
                               kReserveBufferSpace,
                               size_policy);
    VIXL_ASSERT(initialised_);
  }

  // This constructor should only be used from code that is *currently
  // generating* the pools, to avoid an infinite loop.
  EmissionCheckScope(MacroAssemblerInterface* masm,
                     size_t size,
                     SizePolicy size_policy,
                     PoolPolicy pool_policy) {
    Open(masm, size, size_policy, pool_policy);
  }

  MacroAssemblerInterface* masm_;
  PoolPolicy pool_policy_;
};

// Use this scope when you need a one-to-one mapping between methods and
// instructions. This scope will:
// - Do the same as `EmissionCheckScope`.
// - Block access to the MacroAssemblerInterface (using run-time assertions).
class ExactAssemblyScope : public EmissionCheckScope {
 public:
  // This constructor implicitly calls `Open` (when `masm` is not `NULL`) to
  // initialise the scope, so it is ready to use immediately after it has been
  // constructed.
  ExactAssemblyScope(MacroAssemblerInterface* masm,
                     size_t size,
                     SizePolicy size_policy = kExactSize)
      : ExactAssemblyScope() {
    Open(masm, size, size_policy);
  }

  // This constructor does not implicitly initialise the scope. Instead, the
  // user is required to explicitly call the `Open` function before using the
  // scope.
  ExactAssemblyScope() : previous_allow_macro_assembler_(false) {}

  virtual ~ExactAssemblyScope() { Close(); }

  void Open(MacroAssemblerInterface* masm,
            size_t size,
            SizePolicy size_policy = kExactSize) {
    Open(masm, size, size_policy, kBlockPools);
  }

  void Close() {
    if (!initialised_) {
      return;
    }
    if (masm_ == NULL) {
      // Nothing to do.
      return;
    }
#ifdef VIXL_DEBUG
    masm_->SetAllowMacroInstructions(previous_allow_macro_assembler_);
#else
    USE(previous_allow_macro_assembler_);
#endif
    EmissionCheckScope::Close();
  }

 protected:
  // This protected constructor allows overriding the pool policy. It is
  // available to allow this scope to be used in code that handles generation
  // of pools.
  ExactAssemblyScope(MacroAssemblerInterface* masm,
                     size_t size,
                     SizePolicy assert_policy,
                     PoolPolicy pool_policy) {
    Open(masm, size, assert_policy, pool_policy);
  }

  void Open(MacroAssemblerInterface* masm,
            size_t size,
            SizePolicy size_policy,
            PoolPolicy pool_policy) {
    VIXL_ASSERT(size_policy != kNoAssert);
    if (masm == NULL) {
      // Nothing to do.
      return;
    }
    // Rely on EmissionCheckScope::Open to initialise `masm_` and
    // `pool_policy_`.
    EmissionCheckScope::Open(masm, size, size_policy, pool_policy);
#ifdef VIXL_DEBUG
    previous_allow_macro_assembler_ = masm->AllowMacroInstructions();
    masm->SetAllowMacroInstructions(false);
#endif
  }

 private:
  bool previous_allow_macro_assembler_;
};


}  // namespace vixl

#endif  // VIXL_CODE_GENERATION_SCOPES_H_

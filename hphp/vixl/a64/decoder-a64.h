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

#ifndef VIXL_A64_DECODER_A64_H_
#define VIXL_A64_DECODER_A64_H_

#include <list>

#include "hphp/vixl/globals.h"
#include "hphp/vixl/a64/instructions-a64.h"


#ifdef DEBUG
  #define UNALLOC(I, P)                                                       \
    if (I->P) {                                                               \
      printf("Instruction 0x%08" PRIx32 " uses an unallocated encoding.\n",   \
             I->InstructionBits());                                           \
    }                                                                         \
    assert(!(I->P));
#else
  #define UNALLOC(I, P) ((void) 0)
#endif

// List macro containing all visitors needed by the decoder class.

#define VISITOR_LIST(V)             \
  V(PCRelAddressing)                \
  V(AddSubImmediate)                \
  V(LogicalImmediate)               \
  V(MoveWideImmediate)              \
  V(Bitfield)                       \
  V(Extract)                        \
  V(UnconditionalBranch)            \
  V(UnconditionalBranchToRegister)  \
  V(CompareBranch)                  \
  V(TestBranch)                     \
  V(ConditionalBranch)              \
  V(System)                         \
  V(Exception)                      \
  V(LoadStorePairPostIndex)         \
  V(LoadStorePairOffset)            \
  V(LoadStorePairPreIndex)          \
  V(LoadStorePairNonTemporal)       \
  V(LoadLiteral)                    \
  V(LoadStoreUnscaledOffset)        \
  V(LoadStorePostIndex)             \
  V(LoadStorePreIndex)              \
  V(LoadStoreRegisterOffset)        \
  V(LoadStoreUnsignedOffset)        \
  V(LogicalShifted)                 \
  V(AddSubShifted)                  \
  V(AddSubExtended)                 \
  V(AddSubWithCarry)                \
  V(ConditionalCompareRegister)     \
  V(ConditionalCompareImmediate)    \
  V(ConditionalSelect)              \
  V(DataProcessing1Source)          \
  V(DataProcessing2Source)          \
  V(DataProcessing3Source)          \
  V(FPCompare)                      \
  V(FPConditionalCompare)           \
  V(FPConditionalSelect)            \
  V(FPImmediate)                    \
  V(FPDataProcessing1Source)        \
  V(FPDataProcessing2Source)        \
  V(FPDataProcessing3Source)        \
  V(FPIntegerConvert)               \
  V(FPFixedPointConvert)            \
  V(Unknown)

namespace vixl {

// The Visitor interface. Disassembler and simulator (and other tools)
// must provide implementations for all of these functions.
class DecoderVisitor {
 public:
  #define DECLARE(A) virtual void Visit##A(Instruction* instr) = 0;
  VISITOR_LIST(DECLARE)
  #undef DECLARE

  virtual ~DecoderVisitor() {}

 private:
  // Visitors are registered in a list.
  std::list<DecoderVisitor*> visitors_;

  friend class Decoder;
};


class Decoder: public DecoderVisitor {
 public:
  Decoder() {}

  // Top-level instruction decoder function. Decodes an instruction and calls
  // the visitor functions registered with the Decoder class.
  void Decode(Instruction *instr);

  // Register a new visitor class with the decoder.
  // Decode() will call the corresponding visitor method from all registered
  // visitor classes when decoding reaches the leaf node of the instruction
  // decode tree.
  // Visitors are called in the order.
  // A visitor can only be registered once.
  // Registering an already registered visitor will update its position.
  //
  //   d.AppendVisitor(V1);
  //   d.AppendVisitor(V2);
  //   d.PrependVisitor(V2);            // Move V2 at the start of the list.
  //   d.InsertVisitorBefore(V3, V2);
  //   d.AppendVisitor(V4);
  //   d.AppendVisitor(V4);             // No effect.
  //
  //   d.Decode(i);
  //
  // will call in order visitor methods in V3, V2, V1, V4.
  void AppendVisitor(DecoderVisitor* visitor);
  void PrependVisitor(DecoderVisitor* visitor);
  void InsertVisitorBefore(DecoderVisitor* new_visitor,
                           DecoderVisitor* registered_visitor);
  void InsertVisitorAfter(DecoderVisitor* new_visitor,
                          DecoderVisitor* registered_visitor);

  // Remove a previously registered visitor class from the list of visitors
  // stored by the decoder.
  void RemoveVisitor(DecoderVisitor *visitor);

  #define DECLARE(A) void Visit##A(Instruction* instr);
  VISITOR_LIST(DECLARE)
  #undef DECLARE

 private:
  // Decode the branch, system command, and exception generation parts of
  // the instruction tree, and call the corresponding visitors.
  // On entry, instruction bits 27:24 = {0x0, 0x4, 0x5, 0x6, 0x7}.
  void DecodeBranchSystemException(Instruction *instr);

  // Decode the load and store parts of the instruction tree, and call
  // the corresponding visitors.
  // On entry, instruction bits 27:24 = {0x8, 0x9, 0xC, 0xD}.
  void DecodeLoadStore(Instruction *instr);

  // Decode the logical immediate and move wide immediate parts of the
  // instruction tree, and call the corresponding visitors.
  // On entry, instruction bits 27:24 = 0x2.
  void DecodeLogical(Instruction *instr);

  // Decode the bitfield and extraction parts of the instruction tree,
  // and call the corresponding visitors.
  // On entry, instruction bits 27:24 = 0x3.
  void DecodeBitfieldExtract(Instruction *instr);

  // Decode the data processing parts of the instruction tree, and call the
  // corresponding visitors.
  // On entry, instruction bits 27:24 = {0x1, 0xA, 0xB}.
  void DecodeDataProcessing(Instruction *instr);

  // Decode the floating point parts of the instruction tree, and call the
  // corresponding visitors.
  // On entry, instruction bits 27:24 = {0xE, 0xF}.
  void DecodeFP(Instruction *instr);
};
}  // namespace vixl

#endif  // VIXL_A64_DECODER_A64_H_

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

#include "hphp/vixl/a64/decoder-a64.h"
#include "hphp/vixl/globals.h"
#include "hphp/vixl/utils.h"

namespace vixl {
// Top-level instruction decode function.
void Decoder::Decode(Instruction *instr) {
  switch (instr->Bits(27, 24)) {
    // 1:   Add/sub immediate.
    // A:   Logical shifted register.
    //      Add/sub with carry.
    //      Conditional compare register.
    //      Conditional compare immediate.
    //      Conditional select.
    //      Data processing 1 source.
    //      Data processing 2 source.
    // B:   Add/sub shifted register.
    //      Add/sub extended register.
    //      Data processing 3 source.
    case 0x1:
    case 0xA:
    case 0xB: DecodeDataProcessing(instr); break;

    // 2:   Logical immediate.
    //      Move wide immediate.
    case 0x2: DecodeLogical(instr); break;

    // 3:   Bitfield.
    //      Extract.
    case 0x3: DecodeBitfieldExtract(instr); break;

    // 0:   PC relative addressing.
    // 4:   Unconditional branch immediate.
    //      Exception generation.
    //      Compare and branch immediate.
    // 5:   Compare and branch immediate.
    //      Conditional branch.
    //      System.
    // 6,7: Unconditional branch.
    //      Test and branch immediate.
    case 0x0:
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7: DecodeBranchSystemException(instr); break;

    // 8,9: Load/store register pair post-index.
    //      Load register literal.
    //      Load/store register unscaled immediate.
    //      Load/store register immediate post-index.
    //      Load/store register immediate pre-index.
    //      Load/store register offset.
    //      Load/store exclusive.
    // C,D: Load/store register pair offset.
    //      Load/store register pair pre-index.
    //      Load/store register unsigned immediate.
    case 0x8:
    case 0x9:
    case 0xC:
    case 0xD: DecodeLoadStore(instr); break;

    // E:   FP fixed point conversion.
    //      FP integer conversion.
    //      FP data processing 1 source.
    //      FP compare.
    //      FP immediate.
    //      FP data processing 2 source.
    //      FP conditional compare.
    //      FP conditional select.
    // F:   FP data processing 3 source.
    case 0xE:
    case 0xF: DecodeFP(instr); break;
  }
}

void Decoder::AppendVisitor(DecoderVisitor* new_visitor) {
  visitors_.remove(new_visitor);
  visitors_.push_front(new_visitor);
}


void Decoder::PrependVisitor(DecoderVisitor* new_visitor) {
  visitors_.remove(new_visitor);
  visitors_.push_back(new_visitor);
}


void Decoder::InsertVisitorBefore(DecoderVisitor* new_visitor,
                                  DecoderVisitor* registered_visitor) {
  visitors_.remove(new_visitor);
  std::list<DecoderVisitor*>::iterator it;
  for (it = visitors_.begin(); it != visitors_.end(); it++) {
    if (*it == registered_visitor) {
      visitors_.insert(it, new_visitor);
      return;
    }
  }
  // We reached the end of the list. The last element must be
  // registered_visitor.
  assert(*it == registered_visitor);
  visitors_.insert(it, new_visitor);
}


void Decoder::InsertVisitorAfter(DecoderVisitor* new_visitor,
                                 DecoderVisitor* registered_visitor) {
  visitors_.remove(new_visitor);
  std::list<DecoderVisitor*>::iterator it;
  for (it = visitors_.begin(); it != visitors_.end(); it++) {
    if (*it == registered_visitor) {
      it++;
      visitors_.insert(it, new_visitor);
      return;
    }
  }
  // We reached the end of the list. The last element must be
  // registered_visitor.
  assert(*it == registered_visitor);
  visitors_.push_back(new_visitor);
}


void Decoder::RemoveVisitor(DecoderVisitor* visitor) {
  visitors_.remove(visitor);
}


void Decoder::DecodeBranchSystemException(Instruction *instr) {
  assert((instr->Bits(27, 24) == 0x0) ||
         (instr->Bits(27, 24) == 0x4) ||
         (instr->Bits(27, 24) == 0x5) ||
         (instr->Bits(27, 24) == 0x6) ||
         (instr->Bits(27, 24) == 0x7) );

  if (instr->Bit(26) == 0) {
    VisitPCRelAddressing(instr);
  } else {
    switch (instr->Bits(31, 29)) {
      case 0:
      case 4: {
        VisitUnconditionalBranch(instr);
        break;
      }
      case 1:
      case 5: {
        if (instr->Bit(25) == 0) {
          VisitCompareBranch(instr);
        } else {
          VisitTestBranch(instr);
        }
        break;
      }
      case 2: {
        UNALLOC(instr, SpacedBits(2, 24, 4) == 0x1);
        UNALLOC(instr, Bit(24) == 0x1);
        VisitConditionalBranch(instr);
        break;
      }
      case 6: {
        if (instr->Bit(25) == 0) {
          if (instr->Bit(24) == 0) {
            UNALLOC(instr, Bits(4, 2) != 0);
            UNALLOC(instr, Mask(0x00E0001D) == 0x00200001);
            UNALLOC(instr, Mask(0x00E0001E) == 0x00200002);
            UNALLOC(instr, Mask(0x00E0001D) == 0x00400001);
            UNALLOC(instr, Mask(0x00E0001E) == 0x00400002);
            UNALLOC(instr, Mask(0x00E0001C) == 0x00600000);
            UNALLOC(instr, Mask(0x00E0001C) == 0x00800000);
            UNALLOC(instr, Mask(0x00E0001F) == 0x00A00000);
            UNALLOC(instr, Mask(0x00C0001C) == 0x00C00000);
            VisitException(instr);
          } else {
            UNALLOC(instr, Mask(0x0038E000) == 0x00000000);
            UNALLOC(instr, Mask(0x0039E000) == 0x00002000);
            UNALLOC(instr, Mask(0x003AE000) == 0x00002000);
            UNALLOC(instr, Mask(0x003CE000) == 0x00042000);
            UNALLOC(instr, Mask(0x003FFFC0) == 0x000320C0);
            UNALLOC(instr, Mask(0x003FF100) == 0x00032100);
            UNALLOC(instr, Mask(0x003FF200) == 0x00032200);
            UNALLOC(instr, Mask(0x003FF400) == 0x00032400);
            UNALLOC(instr, Mask(0x003FF800) == 0x00032800);
            UNALLOC(instr, Mask(0x003FF0E0) == 0x00033000);
            UNALLOC(instr, Mask(0x003FF0E0) == 0x003FF020);
            UNALLOC(instr, Mask(0x003FF0E0) == 0x003FF060);
            UNALLOC(instr, Mask(0x003FF0E0) == 0x003FF0E0);
            UNALLOC(instr, Mask(0x0038F000) == 0x00005000);
            UNALLOC(instr, Mask(0x0038E000) == 0x00006000);
            UNALLOC(instr, SpacedBits(4, 21, 20, 19, 15) == 0x1);
            UNALLOC(instr, Bits(21, 19) == 0x4);
            VisitSystem(instr);
          }
        } else {
          UNALLOC(instr, Bits(20, 16) != 0x1F);
          UNALLOC(instr, Bits(15, 10) != 0);
          UNALLOC(instr, Bits(4, 0) != 0);
          UNALLOC(instr, Bits(24, 21) == 0x3);
          UNALLOC(instr, Bits(24, 22) == 0x3);
          UNALLOC(instr, Bit(24) == 0x1);
          VisitUnconditionalBranchToRegister(instr);
        }
        break;
      }
      default: VisitUnknown(instr);
    }
  }
}


void Decoder::DecodeLoadStore(Instruction *instr) {
  assert((instr->Bits(27, 24) == 0x8) ||
         (instr->Bits(27, 24) == 0x9) ||
         (instr->Bits(27, 24) == 0xC) ||
         (instr->Bits(27, 24) == 0xD) );

  if (instr->Bit(24) == 0) {
    if (instr->Bit(28) == 0) {
      if (instr->Bit(29) == 0) {
        if (instr->Bit(26) == 0) {
          // TODO: VisitLoadStoreExclusive.
          not_implemented();
        } else {
          // TODO: VisitLoadStoreAdvSIMD.
          not_implemented();
        }
      } else {
        UNALLOC(instr, Bits(31, 30) == 0x3);
        UNALLOC(instr, SpacedBits(4, 26, 31, 30, 22) == 0x2);
        if (instr->Bit(23) == 0) {
          UNALLOC(instr, SpacedBits(4, 26, 31, 30, 22) == 0x3);
          VisitLoadStorePairNonTemporal(instr);
        } else {
          VisitLoadStorePairPostIndex(instr);
        }
      }
    } else {
      if (instr->Bit(29) == 0) {
        UNALLOC(instr, SpacedBits(3, 26, 31, 30) == 0x7);
        VisitLoadLiteral(instr);
      } else {
        UNALLOC(instr, SpacedBits(4, 26, 23, 22, 31) == 0x7);
        UNALLOC(instr, SpacedBits(3, 26, 23, 30) == 0x7);
        UNALLOC(instr, SpacedBits(3, 26, 23, 31) == 0x7);
        if (instr->Bit(21) == 0) {
          switch (instr->Bits(11, 10)) {
            case 0: {
              VisitLoadStoreUnscaledOffset(instr);
              break;
            }
            case 1: {
              UNALLOC(instr, SpacedBits(5, 26, 23, 22, 31, 30) == 0xB);
              VisitLoadStorePostIndex(instr);
              break;
            }
            case 3: {
              UNALLOC(instr, SpacedBits(5, 26, 23, 22, 31, 30) == 0xB);
              VisitLoadStorePreIndex(instr);
              break;
            }
            default: VisitUnknown(instr);
          }
        } else {
          UNALLOC(instr, Bit(14) == 0);
          VisitLoadStoreRegisterOffset(instr);
        }
      }
    }
  } else {
    if (instr->Bit(28) == 0) {
      UNALLOC(instr, SpacedBits(4, 26, 31, 30, 22) == 0x2);
      UNALLOC(instr, Bits(31, 30) == 0x3);
      if (instr->Bit(23) == 0) {
        VisitLoadStorePairOffset(instr);
      } else {
        VisitLoadStorePairPreIndex(instr);
      }
    } else {
      UNALLOC(instr, SpacedBits(4, 26, 23, 22, 31) == 0x7);
      UNALLOC(instr, SpacedBits(3, 26, 23, 30) == 0x7);
      UNALLOC(instr, SpacedBits(3, 26, 23, 31) == 0x7);
      VisitLoadStoreUnsignedOffset(instr);
    }
  }
}


void Decoder::DecodeLogical(Instruction *instr) {
  assert(instr->Bits(27, 24) == 0x2);

  UNALLOC(instr, SpacedBits(2, 31, 22) == 0x1);
  if (instr->Bit(23) == 0) {
    VisitLogicalImmediate(instr);
  } else {
    UNALLOC(instr, Bits(30, 29) == 0x1);
    VisitMoveWideImmediate(instr);
  }
}


void Decoder::DecodeBitfieldExtract(Instruction *instr) {
  assert(instr->Bits(27, 24) == 0x3);

  UNALLOC(instr, SpacedBits(2, 31, 22) == 0x2);
  UNALLOC(instr, SpacedBits(2, 31, 22) == 0x1);
  UNALLOC(instr, SpacedBits(2, 31, 15) == 0x1);
  if (instr->Bit(23) == 0) {
    UNALLOC(instr, SpacedBits(2, 31, 21) == 0x1);
    UNALLOC(instr, Bits(30, 29) == 0x3);
    VisitBitfield(instr);
  } else {
    UNALLOC(instr, SpacedBits(3, 30, 29, 21) == 0x1);
    UNALLOC(instr, Bits(30, 29) != 0);
    VisitExtract(instr);
  }
}


void Decoder::DecodeDataProcessing(Instruction *instr) {
  assert((instr->Bits(27, 24) == 0x1) ||
         (instr->Bits(27, 24) == 0xA) ||
         (instr->Bits(27, 24) == 0xB) );

  if (instr->Bit(27) == 0) {
    UNALLOC(instr, Bit(23) == 0x1);
    VisitAddSubImmediate(instr);
  } else if (instr->Bit(24) == 0) {
    if (instr->Bit(28) == 0) {
      UNALLOC(instr, SpacedBits(2, 31, 15) == 0x1);
      VisitLogicalShifted(instr);
    } else {
      switch (instr->Bits(23, 21)) {
        case 0: {
          UNALLOC(instr, Bits(15, 10) != 0);
          VisitAddSubWithCarry(instr);
          break;
        }
        case 2: {
         UNALLOC(instr, SpacedBits(2, 10, 4) != 0);
         UNALLOC(instr, Bit(29) == 0x0);
         if (instr->Bit(11) == 0) {
            VisitConditionalCompareRegister(instr);
          } else {
            VisitConditionalCompareImmediate(instr);
          }
          break;
        }
        case 4: {
          UNALLOC(instr, SpacedBits(2, 11, 29) != 0);
          VisitConditionalSelect(instr);
          break;
        }
        case 6: {
          UNALLOC(instr, Bit(29) == 1);
          UNALLOC(instr, Bits(15, 14) != 0);
          if (instr->Bit(30) == 0) {
            UNALLOC(instr, Bits(15, 11) == 0);
            UNALLOC(instr, Bits(15, 12) == 0x1);
            UNALLOC(instr, Bits(15, 12) == 0x3);
            VisitDataProcessing2Source(instr);
          } else {
            UNALLOC(instr, Bit(13) == 1);
            UNALLOC(instr, Bits(20, 16) != 0);
            UNALLOC(instr, Mask(0xA01FFC00) == 0x00000C00);
            UNALLOC(instr, Mask(0x201FF800) == 0x00001800);
            VisitDataProcessing1Source(instr);
          }
          break;
        }
        default: VisitUnknown(instr);
      }
    }
  } else {
    if (instr->Bit(28) == 0) {
      if (instr->Bit(21) == 0) {
        UNALLOC(instr, Bits(23, 22) == 0x3);
        UNALLOC(instr, SpacedBits(2, 31, 15) == 0x1);
        VisitAddSubShifted(instr);
      } else {
        UNALLOC(instr, SpacedBits(2, 23, 22) != 0);
        UNALLOC(instr, SpacedBits(2, 12, 10) == 0x3);
        UNALLOC(instr, Bits(12, 11) == 0x3);
        VisitAddSubExtended(instr);
      }
    } else {
      UNALLOC(instr, Mask(0xE0E08000) == 0x00200000);
      UNALLOC(instr, Mask(0xE0E08000) == 0x00208000);
      UNALLOC(instr, Mask(0xE0E08000) == 0x00400000);
      UNALLOC(instr, Mask(0x60E08000) == 0x00408000);
      UNALLOC(instr, SpacedBits(5, 30, 29, 23, 22, 21) == 0x3);
      UNALLOC(instr, SpacedBits(5, 30, 29, 23, 22, 21) == 0x4);
      UNALLOC(instr, Mask(0xE0E08000) == 0x00A00000);
      UNALLOC(instr, Mask(0xE0E08000) == 0x00A08000);
      UNALLOC(instr, Mask(0xE0E08000) == 0x00C00000);
      UNALLOC(instr, Mask(0x60E08000) == 0x00C08000);
      UNALLOC(instr, SpacedBits(5, 30, 29, 23, 22, 21) == 0x7);
      UNALLOC(instr, Bits(30, 29) == 0x1);
      UNALLOC(instr, Bit(30) == 0x1);
      VisitDataProcessing3Source(instr);
    }
  }
}


void Decoder::DecodeFP(Instruction *instr) {
  assert((instr->Bits(27, 24) == 0xE) ||
         (instr->Bits(27, 24) == 0xF) );
  UNALLOC(instr, Bit(29) == 0x1);

  if (instr->Bit(24) == 0) {
    if (instr->Bit(21) == 0) {
      UNALLOC(instr, Bit(23) == 1);
      UNALLOC(instr, SpacedBits(2, 31, 15) == 0);
      UNALLOC(instr, SpacedBits(3, 18, 17, 19) == 0);
      UNALLOC(instr, SpacedBits(3, 18, 17, 20) == 0);
      UNALLOC(instr, SpacedBits(3, 18, 17, 19) == 0x3);
      UNALLOC(instr, SpacedBits(3, 18, 17, 20) == 0x3);
      UNALLOC(instr, Bit(18) == 1);
      VisitFPFixedPointConvert(instr);
    } else {
      if (instr->Bits(15, 10) == 0) {
        UNALLOC(instr, SpacedBits(3, 18, 17, 19) == 0x3);
        UNALLOC(instr, SpacedBits(3, 18, 17, 20) == 0x3);
        UNALLOC(instr, SpacedBits(3, 18, 17, 19) == 0x5);
        UNALLOC(instr, SpacedBits(3, 18, 17, 20) == 0x5);
        UNALLOC(instr, Mask(0xA0C60000) == 0x80060000);
        UNALLOC(instr, Mask(0xA0CE0000) == 0x000E0000);
        UNALLOC(instr, Mask(0xA0D60000) == 0x00160000);
        UNALLOC(instr, Mask(0xA0C60000) == 0x00460000);
        UNALLOC(instr, Mask(0xA0CE0000) == 0x804E0000);
        UNALLOC(instr, Mask(0xA0D60000) == 0x80560000);
        UNALLOC(instr, SpacedBits(4, 23, 22, 18, 29) == 0x8);
        UNALLOC(instr, SpacedBits(5, 23, 22, 18, 17, 29) == 0x14);
        UNALLOC(instr, Mask(0xA0C60000) == 0x00860000);
        UNALLOC(instr, Mask(0xA0CE0000) == 0x80860000);
        UNALLOC(instr, Mask(0xA0D60000) == 0x80960000);
        UNALLOC(instr, Bits(23, 22) == 0x3);
        VisitFPIntegerConvert(instr);
      } else if (instr->Bits(14, 10) == 16) {
        UNALLOC(instr, SpacedBits(3, 31, 19, 20) != 0);
        UNALLOC(instr, Mask(0xA0DF8000) == 0x00020000);
        UNALLOC(instr, Mask(0xA0DF8000) == 0x00030000);
        UNALLOC(instr, Mask(0xA0DF8000) == 0x00068000);
        UNALLOC(instr, Mask(0xA0DF8000) == 0x00428000);
        UNALLOC(instr, Mask(0xA0DF8000) == 0x00430000);
        UNALLOC(instr, Mask(0xA0DF8000) == 0x00468000);
        UNALLOC(instr, Mask(0xA0D80000) == 0x00800000);
        UNALLOC(instr, Mask(0xA0DE0000) == 0x00C00000);
        UNALLOC(instr, Mask(0xA0DF0000) == 0x00C30000);
        UNALLOC(instr, Mask(0xA0DC0000) == 0x00C40000);
        VisitFPDataProcessing1Source(instr);
      } else if (instr->Bits(13, 10) == 8) {
        UNALLOC(instr, SpacedBits(2, 31, 23) != 0);
        UNALLOC(instr, Bits(2, 0) != 0);
        UNALLOC(instr, Bits(15, 14) != 0);
        VisitFPCompare(instr);
      } else if (instr->Bits(12, 10) == 4) {
        UNALLOC(instr, Bits(9, 5) != 0);
        UNALLOC(instr, SpacedBits(2, 31, 23) != 0);
        VisitFPImmediate(instr);
      } else {
        UNALLOC(instr, SpacedBits(2, 31, 23) != 0);
        switch (instr->Bits(11, 10)) {
          case 1: {
            VisitFPConditionalCompare(instr);
            break;
          }
          case 2: {
            UNALLOC(instr, SpacedBits(2, 15, 12) == 0x3);
            UNALLOC(instr, SpacedBits(2, 15, 13) == 0x3);
            UNALLOC(instr, Bits(15, 14) == 0x3);
            VisitFPDataProcessing2Source(instr);
            break;
          }
          case 3: {
            VisitFPConditionalSelect(instr);
            break;
          }
          default: VisitUnknown(instr);
        }
      }
    }
  } else {
    UNALLOC(instr, Bit(31) == 0x1);
    UNALLOC(instr, Bit(23) == 0x1);
    VisitFPDataProcessing3Source(instr);
  }
}

#define DEFINE_VISITOR_CALLERS(A)                                              \
  void Decoder::Visit##A(Instruction *instr) {                                 \
    assert(instr->Mask(A##FMask) == A##Fixed);                                 \
    std::list<DecoderVisitor*>::iterator it;                                   \
    for (it = visitors_.begin(); it != visitors_.end(); it++) {                \
      (*it)->Visit##A(instr);                                                  \
    }                                                                          \
  }
VISITOR_LIST(DEFINE_VISITOR_CALLERS)
#undef DEFINE_VISITOR_CALLERS
}  // namespace vixl

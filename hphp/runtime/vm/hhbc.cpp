/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/vm/hhbc.h"

#include <type_traits>
#include <sstream>
#include <cstring>

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/util/text-util.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

int numImmediates(Op opcode) {
  assert(isValidOpcode(opcode));
  static const int8_t values[] = {
#define NA         0
#define ONE(...)   1
#define TWO(...)   2
#define THREE(...) 3
#define FOUR(...)  4
#define O(name, imm, unusedPop, unusedPush, unusedFlags) imm,
    OPCODES
#undef O
#undef NA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
  };
  return values[size_t(opcode)];
}

ArgType immType(const Op opcode, int idx) {
  assert(isValidOpcode(opcode));
  assert(idx >= 0 && idx < numImmediates(opcode));
  always_assert(idx < 4); // No opcodes have more than four immediates
  static const int8_t arg0Types[] = {
#define NA -1,
#define ONE(a) a,
#define TWO(a, b) a,
#define THREE(a, b, c) a,
#define FOUR(a, b, c, d) a,
#define OA(x) OA
#define O(name, imm, unusedPop, unusedPush, unusedFlags) imm
    OPCODES
// re-using definition of O below.
#undef OA
#undef NA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
  };
  static const int8_t arg1Types[] = {
#define NA -1,
#define ONE(a) -1,
#define TWO(a, b) b,
#define THREE(a, b, c) b,
#define FOUR(a, b, c, d) b,
#define OA(x) OA
    OPCODES
// re-using definition of O below.
#undef OA
#undef NA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
  };
  static const int8_t arg2Types[] = {
#define NA -1,
#define ONE(a) -1,
#define TWO(a, b) -1,
#define THREE(a, b, c) c,
#define FOUR(a, b, c, d) c,
#define OA(x) OA
    OPCODES
#undef OA
#undef NA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
  };
  static const int8_t arg3Types[] = {
#define NA -1,
#define ONE(a) -1,
#define TWO(a, b) -1,
#define THREE(a, b, c) -1,
#define FOUR(a, b, c, d) d,
#define OA(x) OA
    OPCODES
#undef OA
#undef O
#undef NA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
  };
  auto opInt = size_t(opcode);
  switch (idx) {
    case 0: return (ArgType)arg0Types[opInt];
    case 1: return (ArgType)arg1Types[opInt];
    case 2: return (ArgType)arg2Types[opInt];
    case 3: return (ArgType)arg3Types[opInt];
    default: assert(false); return (ArgType)-1;
  }
}

int immSize(PC origPC, int idx) {
  auto pc = origPC;
  auto const op = decode_op(pc);
  assert(idx >= 0 && idx < numImmediates(op));
  always_assert(idx < 4); // No origPCs have more than four immediates
  static const int8_t argTypeToSizes[] = {
#define ARGTYPE(nm, type) sizeof(type),
#define ARGTYPEVEC(nm, type) 0,
    ARGTYPES
#undef ARGTYPE
#undef ARGTYPEVEC
  };

  if (immType(op, idx) == IVA ||
      immType(op, idx) == LA ||
      immType(op, idx) == IA) {
    if (idx >= 1) pc += immSize(origPC, 0);
    if (idx >= 2) pc += immSize(origPC, 1);
    if (idx >= 3) pc += immSize(origPC, 2);
    auto const imm = decode_raw<uint8_t>(pc);

    // Low order bit set => 4-byte.
    return (imm & 0x1 ? sizeof(int32_t) : sizeof(unsigned char));
  }

  if (immType(op, idx) == RATA) {
    if (idx >= 1) pc += immSize(origPC, 0);
    if (idx >= 2) pc += immSize(origPC, 1);
    if (idx >= 3) pc += immSize(origPC, 2);
    return encodedRATSize(pc);
  }

  if (immIsVector(op, idx)) {
    if (idx >= 1) pc += immSize(origPC, 0);
    if (idx >= 2) pc += immSize(origPC, 1);
    if (idx >= 3) pc += immSize(origPC, 2);
    int prefixes, vecElemSz;
    auto itype = immType(op, idx);
    if (itype == MA) {
      prefixes = 2;
      vecElemSz = sizeof(uint8_t);
    } else if (itype == BLA) {
      prefixes = 1;
      vecElemSz = sizeof(Offset);
    } else if (itype == ILA) {
      prefixes = 1;
      vecElemSz = 2 * sizeof(uint32_t);
    } else if (itype == VSA) {
      prefixes = 1;
      vecElemSz = sizeof(Id);
    } else {
      assert(itype == SLA);
      prefixes = 1;
      vecElemSz = sizeof(StrVecItem);
    }
    return prefixes * sizeof(int32_t) +
      vecElemSz * decode_raw<int32_t>(pc);
  }

  ArgType type = immType(op, idx);
  return (type >= 0) ? argTypeToSizes[type] : 0;
}

bool immIsVector(Op opcode, int idx) {
  ArgType type = immType(opcode, idx);
  return type == MA || type == BLA || type == SLA || type == ILA ||
    type == VSA;
}

bool hasImmVector(Op opcode) {
  const int num = numImmediates(opcode);
  for (int i = 0; i < num; ++i) {
    if (immIsVector(opcode, i)) return true;
  }
  return false;
}

ArgUnion getImm(PC const origPC, int idx) {
  auto pc = origPC;
  auto const UNUSED op = decode_op(pc);
  assert(idx >= 0 && idx < numImmediates(op));
  ArgUnion retval;
  retval.u_NA = 0;
  int cursor = 0;
  for (cursor = 0; cursor < idx; cursor++) {
    // Advance over this immediate.
    pc += immSize(origPC, cursor);
  }
  always_assert(cursor == idx);
  auto const type = immType(op, idx);
  if (type == IVA || type == LA || type == IA) {
    retval.u_IVA = decodeVariableSizeImm(&pc);
  } else if (!immIsVector(op, cursor)) {
    always_assert(type != RATA);  // Decode RATAs with a different function.
    memcpy(&retval.bytes, pc, immSize(origPC, idx));
  }
  always_assert(numImmediates(op) > idx);
  return retval;
}

ArgUnion* getImmPtr(PC const origPC, int idx) {
  auto pc = origPC;
  auto const UNUSED op = decode_op(pc);
  assert(immType(op, idx) != IVA);
  assert(immType(op, idx) != LA);
  assert(immType(op, idx) != IA);
  assert(immType(op, idx) != RATA);
  for (int i = 0; i < idx; i++) {
    pc += immSize(origPC, i);
  }
  return (ArgUnion*)pc;
}

template<typename T>
T decodeImm(const unsigned char** immPtr) {
  T val = *(T*)*immPtr;
  *immPtr += sizeof(T);
  return val;
}

int64_t decodeMemberCodeImm(const unsigned char** immPtr, MemberCode mcode) {
  switch (mcode) {
    case MEL:
    case MPL:
      return decodeVariableSizeImm(immPtr);

    case MET:
    case MPT:
    case MQT:
      return decodeImm<int32_t>(immPtr);

    case MEI:
      return decodeImm<int64_t>(immPtr);

    case MEC:
    case MPC:
    case MW:
    case InvalidMemberCode:
      break;
  }
  not_reached();
}

// TODO: merge with emitIVA in unit.h
size_t encodeVariableSizeImm(int32_t n, unsigned char* buf) {
  if (LIKELY((n & 0x7f) == n)) {
    *buf = static_cast<unsigned char>(n) << 1;
    return 1;
  }
  assert((n & 0x7fffffff) == n);
  *reinterpret_cast<uint32_t*>(buf) = (uint32_t(n) << 1) | 0x1;
  return 4;
}

void encodeIvaToVector(std::vector<unsigned char>& out, int32_t val) {
  size_t currentLen = out.size();
  out.resize(currentLen + 4);
  out.resize(currentLen + encodeVariableSizeImm(val, &out[currentLen]));
}

int instrLen(PC const origPC) {
  auto pc = origPC;
  auto op = decode_op(pc);
  int nImm = numImmediates(op);
  for (int i = 0; i < nImm; i++) {
    pc += immSize(origPC, i);
  }
  return pc - origPC;
}

Offset* instrJumpOffset(PC const origPC) {
  static const int8_t jumpMask[] = {
#define IMM_NA 0
#define IMM_MA 0
#define IMM_IVA 0
#define IMM_I64A 0
#define IMM_DA 0
#define IMM_SA 0
#define IMM_AA 0
#define IMM_RATA 0
#define IMM_BA 1
#define IMM_BLA 0  // these are jump offsets, but must be handled specially
#define IMM_ILA 0
#define IMM_SLA 0
#define IMM_LA 0
#define IMM_IA 0
#define IMM_OA(x) 0
#define IMM_VSA 0
#define ONE(a) IMM_##a
#define TWO(a, b) (IMM_##a + 2 * IMM_##b)
#define THREE(a, b, c) (IMM_##a + 2 * IMM_##b + 4 * IMM_##c)
#define FOUR(a, b, c, d) (IMM_##a + 2 * IMM_##b + 4 * IMM_##c + 8 * IMM_##d)
#define O(name, imm, pop, push, flags) imm,
    OPCODES
#undef IMM_NA
#undef IMM_MA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_DA
#undef IMM_SA
#undef IMM_AA
#undef IMM_RATA
#undef IMM_LA
#undef IMM_IA
#undef IMM_BA
#undef IMM_BLA
#undef IMM_ILA
#undef IMM_SLA
#undef IMM_OA
#undef IMM_VSA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef O
  };

  auto pc = origPC;
  auto const op = decode_op(pc);
  assert(!isSwitch(op));  // BLA doesn't work here

  if (op == OpIterBreak) {
    auto const veclen = decode_raw<uint32_t>(pc);
    assert(veclen > 0);
    auto const target = const_cast<Offset*>(
      reinterpret_cast<const Offset*>(
        reinterpret_cast<const uint32_t*>(pc) + 2 * veclen
      )
    );
    return target;
  }

  int mask = jumpMask[size_t(op)];
  if (mask == 0) {
    return nullptr;
  }
  int immNum;
  switch (mask) {
  case 0: return nullptr;
  case 1: immNum = 0; break;
  case 2: immNum = 1; break;
  case 4: immNum = 2; break;
  case 8: immNum = 3; break;
  default: assert(false); return nullptr;
  }

  return &getImmPtr(origPC, immNum)->u_BA;
}

Offset instrJumpTarget(PC instrs, Offset pos) {
  Offset* offset = instrJumpOffset(instrs + pos);

  if (!offset) {
    return InvalidAbsoluteOffset;
  } else {
    return *offset + pos;
  }
}

OffsetSet instrSuccOffsets(PC opc, const Unit* unit) {
  OffsetSet succBcOffs;
  auto const bcStart = unit->entry();
  auto const op = peek_op(opc);

  if (!instrIsControlFlow(op)) {
    Offset succOff = opc + instrLen(opc) - bcStart;
    succBcOffs.insert(succOff);
    return succBcOffs;
  }

  if (instrAllowsFallThru(op)) {
    Offset succOff = opc + instrLen(opc) - bcStart;
    succBcOffs.insert(succOff);
  }

  if (isSwitch(op)) {
    foreachSwitchTarget(opc, [&](Offset offset) {
      succBcOffs.insert(offset + opc - bcStart);
    });
  } else {
    Offset target = instrJumpTarget(bcStart, opc - bcStart);
    if (target != InvalidAbsoluteOffset) {
      succBcOffs.insert(target);
    }
  }
  return succBcOffs;
}

/**
 * Return the number of successor-edges including fall-through paths but not
 * implicit exception paths.
 */
int numSuccs(PC const origPC) {
  auto pc = origPC;
  auto const op = decode_op(pc);
  if ((instrFlags(op) & TF) != 0) {
    if (isSwitch(op)) {
      return decode_raw<int32_t>(pc);
    }
    if (isUnconditionalJmp(op) || op == OpIterBreak) return 1;
    return 0;
  }
  if (!instrIsControlFlow(op)) return 1;
  if (instrJumpOffset(origPC)) return 2;
  return 1;
}

/**
 * instrNumPops() returns the number of values consumed from the stack
 * for a given push/pop instruction. For peek/poke instructions, this
 * function returns 0.
 */
int instrNumPops(PC pc) {
  static const int32_t numberOfPops[] = {
#define NOV 0
#define ONE(...) 1
#define TWO(...) 2
#define THREE(...) 3
#define FOUR(...) 4
#define MMANY -1
#define C_MMANY -2
#define V_MMANY -2
#define R_MMANY -2
#define MFINAL -3
#define C_MFINAL -5
#define R_MFINAL C_MFINAL
#define V_MFINAL C_MFINAL
#define FMANY -3
#define CVMANY -3
#define CVUMANY -3
#define CMANY -3
#define SMANY -1
#define IDX_A -4
#define O(name, imm, pop, push, flags) pop,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef MMANY
#undef C_MMANY
#undef V_MMANY
#undef R_MMANY
#undef MFINAL
#undef C_MFINAL
#undef R_MFINAL
#undef V_MFINAL
#undef FMANY
#undef CVMANY
#undef CVUMANY
#undef CMANY
#undef SMANY
#undef IDX_A
#undef O
  };
  int n = numberOfPops[size_t(peek_op(pc))];
  // For most instructions, we know how many values are popped based
  // solely on the opcode
  if (n >= 0) return n;
  // BaseSC and BaseSL remove an A that may be on the top of the stack or one
  // element below the top, depending on the second immediate.
  if (n == -4) return getImm(pc, 1).u_IVA + 1;
  // FCall, NewPackedArray, and some final member operations specify how many
  // values are popped in their first immediate
  if (n == -3) return getImm(pc, 0).u_IVA;
  // Other final member operations pop their first immediate + 1
  if (n == -5) return getImm(pc, 0).u_IVA + 1;
  // For instructions with vector immediates, we have to scan the
  // contents of the vector immediate to determine how many values
  // are popped
  assert(n == -1 || n == -2);
  ImmVector iv = getImmVector(pc);
  // Count the number of values on the stack accounted for by the
  // ImmVector's location and members
  int k = iv.numStackValues();
  // If this instruction also takes a RHS, count that too
  if (n == -2) ++k;
  return k;
}

/**
 * instrNumPushes() returns the number of values pushed onto the stack
 * for a given push/pop instruction. For peek/poke instructions or
 * InsertMid instructions, this function returns 0.
 */
int instrNumPushes(PC pc) {
  static const int8_t numberOfPushes[] = {
#define NOV 0
#define ONE(...) 1
#define TWO(...) 2
#define THREE(...) 3
#define FOUR(...) 4
#define INS_1(...) 0
#define INS_2(...) 0
#define IDX_A -1
#define O(name, imm, pop, push, flags) push,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef INS_1
#undef INS_2
#undef IDX_A
#undef O
  };
  auto const op = peek_op(pc);
  auto const pushes = numberOfPushes[size_t(op)];

  // BaseSC and BaseSL may push back a C that was on top of the A they removed.
  if (pushes == -1) return getImm(pc, 1).u_IVA;

  return pushes;
}

namespace {
FlavorDesc doFlavor(uint32_t i) {
  always_assert(0 && "Invalid stack index");
}
template<typename... Args>
FlavorDesc doFlavor(uint32_t i, FlavorDesc f, Args&&... args) {
  return i == 0 ? f : doFlavor(i - 1, std::forward<Args>(args)...);
}

FlavorDesc minstrFlavor(PC op, uint32_t i, FlavorDesc top) {
  if (top != NOV) {
    if (i == 0) return top;
    --i;
  }

  // First, check for location codes that have a non-Cell stack input.
  auto const location = getMLocation(op);
  auto const numStack = getImmVector(op).numStackValues();
  switch (location.lcode) {
    // No stack input for the location.
    case LL: case LH: case LGL: case LNL: break;

    // CV on top. Handled below.
    case LC: case LGC: case LNC: break;

    // AV on top.
    case LSL: case LSC:
      if (i == 0) return AV;
      break;

    // RV on the bottom.
    case LR:
      if (i == numStack - 1) return RV;
      break;

    case InvalidLocationCode:
      always_assert(false);
  }

  if (i < numStack) return CV;
  always_assert(0 && "Invalid stack index");
}

FlavorDesc manyFlavor(PC op, uint32_t i, FlavorDesc flavor) {
  always_assert(i < uint32_t(instrNumPops(op)));
  return flavor;
}

FlavorDesc baseSFlavor(PC pc, uint32_t i) {
  always_assert(i <= 1);
  auto clsIdx = getImm(pc, 1).u_IVA;
  return i == clsIdx ? AV : CV;
}
}

/**
 * Returns the expected input flavor of stack slot idx.
 */
FlavorDesc instrInputFlavor(PC op, uint32_t idx) {
  auto constexpr nov = NOV;
#define NOV always_assert(0 && "Opcode has no stack inputs");
#define ONE(f1) return doFlavor(idx, f1);
#define TWO(f1, f2) return doFlavor(idx, f1, f2);
#define THREE(f1, f2, f3) return doFlavor(idx, f1, f2, f3);
#define FOUR(f1, f2, f3, f4) return doFlavor(idx, f1, f2, f3, f4);
#define MMANY return minstrFlavor(op, idx, nov);
#define C_MMANY return minstrFlavor(op, idx, CV);
#define V_MMANY return minstrFlavor(op, idx, VV);
#define R_MMANY return minstrFlavor(op, idx, RV);
#define MFINAL return manyFlavor(op, idx, CRV);
#define C_MFINAL return idx == 0 ? CV : CRV;
#define R_MFINAL return idx == 0 ? RV : CRV;
#define V_MFINAL return idx == 0 ? VV : CRV;
#define FMANY return manyFlavor(op, idx, FV);
#define CVMANY return manyFlavor(op, idx, CVV);
#define CVUMANY return manyFlavor(op, idx, CVUV);
#define CMANY return manyFlavor(op, idx, CV);
#define SMANY return manyFlavor(op, idx, CV);
#define IDX_A return baseSFlavor(op, idx);
#define O(name, imm, pop, push, flags) case Op::name: pop
  switch (peek_op(op)) {
    OPCODES
  }
  not_reached();
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef MMANY
#undef C_MMANY
#undef V_MMANY
#undef R_MMANY
#undef MFINAL
#undef C_MFINAL
#undef R_MFINAL
#undef V_MFINAL
#undef FMANY
#undef CVMANY
#undef CVUMANY
#undef CMANY
#undef SMANY
#undef IDX_A
#undef O
}

StackTransInfo instrStackTransInfo(PC opcode) {
  static const StackTransInfo::Kind transKind[] = {
#define NOV StackTransInfo::Kind::PushPop
#define ONE(...) StackTransInfo::Kind::PushPop
#define TWO(...) StackTransInfo::Kind::PushPop
#define THREE(...) StackTransInfo::Kind::PushPop
#define FOUR(...) StackTransInfo::Kind::PushPop
#define IDX_A StackTransInfo::Kind::PushPop
#define INS_1(...) StackTransInfo::Kind::InsertMid
#define INS_2(...) StackTransInfo::Kind::InsertMid
#define O(name, imm, pop, push, flags) push,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef INS_1
#undef INS_2
#undef IDX_A
#undef O
  };
  static const int8_t peekPokeType[] = {
#define NOV -1
#define ONE(...) -1
#define TWO(...) -1
#define THREE(...) -1
#define FOUR(...) -1
#define INS_1(...) 0
#define INS_2(...) 1
#define IDX_A 0
#define O(name, imm, pop, push, flags) push,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef INS_2
#undef INS_1
#undef IDX_A
#undef O
  };
  StackTransInfo ret;
  auto const op = peek_op(opcode);
  ret.kind = transKind[size_t(op)];
  switch (ret.kind) {
  case StackTransInfo::Kind::PushPop:
    ret.pos = 0;
    ret.numPushes = instrNumPushes(opcode);
    ret.numPops = instrNumPops(opcode);
    return ret;
  case StackTransInfo::Kind::InsertMid:
    ret.numPops = 0;
    ret.numPushes = 0;
    ret.pos = peekPokeType[size_t(op)];
    return ret;
  }
  not_reached();
}

bool pushesActRec(Op opcode) {
  switch (opcode) {
    case OpFPushFunc:
    case OpFPushFuncD:
    case OpFPushFuncU:
    case OpFPushObjMethod:
    case OpFPushObjMethodD:
    case OpFPushClsMethod:
    case OpFPushClsMethodF:
    case OpFPushClsMethodD:
    case OpFPushCtor:
    case OpFPushCtorD:
    case OpFPushCufIter:
    case OpFPushCuf:
    case OpFPushCufF:
    case OpFPushCufSafe:
      return true;
    default:
      return false;
  }
}

void staticArrayStreamer(ArrayData* ad, std::ostream& out) {
  out << "array(";
  if (!ad->empty()) {
    bool comma = false;
    for (ArrayIter it(ad); !it.end(); it.next()) {
      if (comma) {
        out << ",";
      } else {
        comma = true;
      }
      Variant key = it.first();

      // Key.
      if (isIntType(key.getType())) {
        out << *key.getInt64Data();
      } else if (isStringType(key.getType())) {
        out << "\""
            << escapeStringForCPP(key.getStringData()->data(),
                                  key.getStringData()->size())
            << "\"";
      } else {
        assert(false);
      }

      out << "=>";

      Variant val = it.second();

      // Value.
      [&] {
        switch (val.getType()) {
          case KindOfUninit:
          case KindOfNull:
            out << "null";
            return;
          case KindOfBoolean:
            out << (val.toBoolean() ? "true" : "false");
            return;
          case KindOfInt64:
            out << *val.getInt64Data();
            return;
          case KindOfDouble:
            out << *val.getDoubleData();
            return;
          case KindOfStaticString:
          case KindOfString:
            out << "\""
                << escapeStringForCPP(val.getStringData()->data(),
                                      val.getStringData()->size())
                << "\"";
            return;
          case KindOfArray:
            staticArrayStreamer(val.getArrayData(), out);
            return;
          case KindOfObject:
          case KindOfResource:
          case KindOfRef:
          case KindOfClass:
            not_reached();
        }
      }();
    }
  }
  out << ")";
}

void staticStreamer(const TypedValue* tv, std::stringstream& out) {
  switch (tv->m_type) {
    case KindOfUninit:
    case KindOfNull:
      out << "null";
      return;
    case KindOfBoolean:
      out << (tv->m_data.num ? "true" : "false");
      return;
    case KindOfInt64:
      out << tv->m_data.num;
      return;
    case KindOfDouble:
      out << tv->m_data.dbl;
      return;
    case KindOfStaticString:
    case KindOfString:
      out << "\"" << tv->m_data.pstr->data() << "\"";
      return;
    case KindOfArray:
      staticArrayStreamer(tv->m_data.parr, out);
      return;
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

const char* const locationNames[] = { "L", "C", "H",
                                      "GL", "GC",
                                      "NL", "NC",
                                      "SL", "SC",
                                      "R" };
const size_t locationNamesCount = sizeof(locationNames) /
                                  sizeof(*locationNames);
static_assert(locationNamesCount == NumLocationCodes,
              "Location code missing for locationCodeString");

const char* locationCodeString(LocationCode lcode) {
  assert(lcode >= 0 && lcode < NumLocationCodes);
  return locationNames[lcode];
}

LocationCode parseLocationCode(const char* s) {
  if (!*s) return InvalidLocationCode;

  switch (*s) {
  case 'L':   return LL;
  case 'C':   return LC;
  case 'H':   return LH;
  case 'R':   return LR;
  default:
    int incr = (s[1] == 'C');
    switch (*s) {
    case 'G': return LocationCode(LGL + incr);
    case 'N': return LocationCode(LNL + incr);
    case 'S': return LocationCode(LSL + incr);
    }
    return InvalidLocationCode;
  }
}

const char* const memberNames[] =
  { "EC", "PC", "EL", "PL", "ET", "PT", "QT", "EI", "W"};
const size_t memberNamesCount = sizeof(memberNames) /
                                sizeof(*memberNames);

static_assert(memberNamesCount == NumMemberCodes,
             "Member code missing for memberCodeString");

const char* memberCodeString(MemberCode mcode) {
  assert(mcode >= 0 && mcode < InvalidMemberCode);
  return memberNames[mcode];
}

MemberCode parseMemberCode(const char* s) {
  for (auto i = 0; i < memberNamesCount; i++) {
    if (!strcmp(memberNames[i], s)) {
      return MemberCode(i);
    }
  }
  return InvalidMemberCode;
}

std::string instrToString(PC it, const Unit* u /* = NULL */) {
  std::stringstream out;
  PC iStart = it;
  Op op = decode_op(it);

  auto readRATA = [&] {
    if (!u) {
      auto const pc = it;
      it += encodedRATSize(pc);
      out << " <RepoAuthType>";
      return;
    }
    auto const rat = decodeRAT(u, it);
    out << ' ' << show(rat);
  };

  switch (op) {

#define READ(t) out << " " << *((t*)&*it); it += sizeof(t)

#define READOFF() do {                                              \
  Offset _value = *(Offset*)it;                                     \
  out << " " << _value;                                             \
  if (u != nullptr) {                                               \
    out << " (" << u->offsetOf(iStart + _value) << ")";             \
  }                                                                 \
  it += sizeof(Offset);                                             \
} while (false)

#define READV() out << " " << decodeVariableSizeImm(&it);

#define READLA() out << " L:" << decodeVariableSizeImm(&it);

#define READIVA() do {                      \
  out << " ";                               \
  auto imm = decodeVariableSizeImm((const uint8_t**)&it);    \
  if (op == OpIncStat && immIdx == 0) {     \
    out << Stats::g_counterNames[imm];      \
  } else {                                  \
    out << imm;                             \
  }                                         \
  immIdx++;                                 \
} while (false)

#define READOA(type) do {                       \
  auto const immVal = static_cast<type>(        \
    *reinterpret_cast<const uint8_t*>(it)       \
  );                                            \
  it += sizeof(unsigned char);                  \
  out << " " << subopToName(immVal);            \
} while (false)

#define READVEC() do {                                                  \
  int sz = *((int*)&*it);                                               \
  it += sizeof(int) * 2;                                                \
  const uint8_t* const start = (uint8_t*)it;                             \
  out << " <";                                                          \
  if (sz > 0) {                                                         \
    int immVal = (int)*((unsigned char*)&*it);                          \
    out << ((immVal >= 0 && size_t(immVal) < locationNamesCount) ?      \
            locationCodeString(LocationCode(immVal)) : "?");            \
    it += sizeof(unsigned char);                                        \
    int numLocImms = numLocationCodeImms(LocationCode(immVal));         \
    for (int i = 0; i < numLocImms; ++i) {                              \
      out << ':' << decodeVariableSizeImm((const uint8_t**)&it);        \
    }                                                                   \
    while (reinterpret_cast<const uint8_t*>(it) - start < sz) {         \
      immVal = (int)*((unsigned char*)&*it);                            \
      out << " " << ((immVal >=0 && size_t(immVal) < memberNamesCount) ? \
                     memberCodeString(MemberCode(immVal)) : "?");       \
      it += sizeof(unsigned char);                                      \
      if (memberCodeHasImm(MemberCode(immVal))) {                       \
        int64_t imm = decodeMemberCodeImm((const uint8_t**)&it,         \
                                          MemberCode(immVal));          \
        out << ':';                                                     \
        if (memberCodeImmIsString(MemberCode(immVal)) && u) {           \
          const StringData* str = u->lookupLitstrId(imm);               \
          int len = str->size();                                        \
          String escaped = string_addslashes(str->data(), len);         \
          out << '"' << escaped.data() << '"';                          \
        } else {                                                        \
          out << imm;                                                   \
        }                                                               \
      }                                                                 \
    }                                                                   \
    assert(reinterpret_cast<const uint8_t*>(it) - start == sz);         \
  }                                                                     \
  out << ">";                                                           \
} while (false)

#define READLITSTR(sep) do {                                      \
  Id id = decode_raw<Id>(it);                                       \
  if (id < 0) {                                                   \
    assert(op == OpSSwitch);                                      \
    out << sep << "-";                                            \
  } else if (u) {                                                 \
    const StringData* sd = u->lookupLitstrId(id);                 \
    out << sep << "\"" <<                                         \
      escapeStringForCPP(sd->data(), sd->size()) << "\"";         \
  } else {                                                        \
    out << sep << id;                                             \
  }                                                               \
} while (false)

#define READSVEC() do {                         \
  int sz = decode_raw<int>(it);                   \
  out << " <";                                  \
  const char* sep = "";                         \
  for (int i = 0; i < sz; ++i) {                \
    out << sep;                                 \
    if (op == OpSSwitch) {                      \
      READLITSTR("");                           \
      out << ":";                               \
    }                                           \
    Offset o = decode_raw<Offset>(it);            \
    if (u != nullptr) {                         \
      if (iStart + o == u->entry() - 1) {       \
        out << "Invalid";                       \
      } else {                                  \
        out << u->offsetOf(iStart + o);         \
      }                                         \
    } else {                                    \
      out << o;                                 \
    }                                           \
    sep = " ";                                  \
  }                                             \
  out << ">";                                   \
} while (false)

#define READIVEC() do {                           \
  int sz = decode_raw<int>(it);                     \
  out << " <";                                    \
  const char* sep = "";                           \
  for (int i = 0; i < sz; ++i) {                  \
    out << sep;                                   \
    IterKind k = (IterKind)decode_raw<Id>(it);      \
    switch(k) {                                   \
      case KindOfIter:  out << "(Iter) ";  break; \
      case KindOfMIter: out << "(MIter) "; break; \
      case KindOfCIter: out << "(CIter) "; break; \
    }                                             \
    out << decode_raw<Id>(it);                      \
    sep = ", ";                                   \
  }                                               \
  out << ">";                                     \
} while (false)

#define ONE(a) H_##a
#define TWO(a, b) H_##a; H_##b
#define THREE(a, b, c) H_##a; H_##b; H_##c;
#define FOUR(a, b, c, d) H_##a; H_##b; H_##c; H_##d;
#define NA
#define H_MA READVEC()
#define H_BLA READSVEC()
#define H_SLA READSVEC()
#define H_ILA READIVEC()
#define H_IVA READIVA()
#define H_I64A READ(int64_t)
#define H_LA READLA()
#define H_IA READV()
#define H_DA READ(double)
#define H_BA READOFF()
#define H_OA(type) READOA(type)
#define H_SA READLITSTR(" ")
#define H_RATA readRATA()
#define H_AA                                                  \
  if (u) {                                                    \
    out << " ";                                               \
    staticArrayStreamer(u->lookupArrayId(*((Id*)it)), out);   \
  } else {                                                    \
    out << " " << *((Id*)it);                                 \
  }                                                           \
  it += sizeof(Id)
#define H_VSA do {                                      \
  int sz = decode_raw<int32_t>(it);                       \
  out << " <";                                          \
  for (int i = 0; i < sz; ++i) {                        \
    H_SA;                                               \
  }                                                     \
  out << " >";                                          \
} while (false)

#define O(name, imm, push, pop, flags)    \
  case Op##name: {                        \
    out << #name;                         \
    UNUSED unsigned immIdx = 0;           \
    imm;                                  \
    break;                                \
  }
OPCODES
#undef O
#undef READ
#undef READV
#undef READLA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef NA
#undef H_MA
#undef H_BLA
#undef H_SLA
#undef H_ILA
#undef H_IVA
#undef H_I64A
#undef H_LA
#undef H_IA
#undef H_DA
#undef H_BA
#undef H_OA
#undef H_SA
#undef H_AA
#undef H_VSA
    default: assert(false);
  };
  return out.str();
}

const char* opcodeToName(Op op) {
  static const char* namesArr[] = {
#define O(name, imm, inputs, outputs, flags) \
    #name ,
    OPCODES
#undef O
  };
  if (op >= Op::LowInvalid && op <= Op::HighInvalid) {
    return namesArr[size_t(op)];
  }
  return "Invalid";
}

//////////////////////////////////////////////////////////////////////

static const char* IsTypeOp_names[] = {
#define ISTYPE_OP(x) #x,
  ISTYPE_OPS
#undef ISTYPE_OP
};

static const char* InitPropOp_names[] = {
#define INITPROP_OP(x) #x,
  INITPROP_OPS
#undef INITPROP_OP
};

static const char* FatalOp_names[] = {
#define FATAL_OP(op) #op,
  FATAL_OPS
#undef FATAL_OP
};

static const char* SetOpOp_names[] = {
#define SETOP_OP(x, y) #x,
  SETOP_OPS
#undef SETOP_OP
};

static const char* IncDecOp_names[] = {
#define INCDEC_OP(x) #x,
  INCDEC_OPS
#undef INCDEC_OP
};

static const char* BareThisOp_names[] = {
#define BARETHIS_OP(x) #x,
  BARETHIS_OPS
#undef BARETHIS_OP
};

static const char* SilenceOp_names[] = {
#define SILENCE_OP(x) #x,
  SILENCE_OPS
#undef SILENCE_OP
};

static const char* OODeclExistsOp_names[] = {
#define OO_DECL_EXISTS_OP(x) #x,
  OO_DECL_EXISTS_OPS
#undef OO_DECL_EXISTS_OP
};

static const char* ObjMethodOp_names[] = {
#define OBJMETHOD_OP(x) #x,
  OBJMETHOD_OPS
#undef OBJMETHOD_OP
};

static const char* SwitchKind_names[] = {
#define KIND(x) #x,
  SWITCH_KINDS
#undef KIND
};

static const char* QueryMOp_names[] = {
#define OP(x) #x,
  QUERY_M_OPS
#undef OP
};

static const char* PropElemOp_names[] = {
#define OP(x) #x,
  PROP_ELEM_OPS
#undef OP
};

template<class T, size_t Sz>
const char* subopToNameImpl(const char* (&arr)[Sz], T opcode) {
  static_assert(
    std::is_same<typename std::underlying_type<T>::type,uint8_t>::value,
    "Subops are all expected to be single-bytes"
  );
  auto const idx = static_cast<uint8_t>(opcode);
  always_assert(idx < Sz);
  return arr[idx];
}

template<class T, size_t Sz>
bool subopValidImpl(const char* (&arr)[Sz], T op) {
  auto const raw = static_cast<typename std::underlying_type<T>::type>(op);
  return raw >= 0 && raw < Sz;
}

template<class T, size_t Sz>
folly::Optional<T> nameToSubopImpl(const char* (&arr)[Sz], const char* str) {
  for (auto i = size_t{0}; i < Sz; ++i) {
    if (!strcmp(str, arr[i])) return static_cast<T>(i);
  }
  return folly::none;
}

namespace {
template<class T> struct NameToSubopHelper;
}

template<class T> folly::Optional<T> nameToSubop(const char* str) {
  return NameToSubopHelper<T>::conv(str);
}

#define X(subop)                                            \
  const char* subopToName(subop op) {                       \
    return subopToNameImpl(subop##_names, op);              \
  }                                                         \
  template<> bool subopValid(subop op) {                    \
    return subopValidImpl(subop##_names, op);               \
  }                                                         \
  namespace {                                               \
  template<> struct NameToSubopHelper<subop> {              \
    static folly::Optional<subop> conv(const char* str) {   \
      return nameToSubopImpl<subop>(subop##_names, str);    \
    }                                                       \
  };                                                        \
  }                                                         \
  template folly::Optional<subop> nameToSubop(const char*);

X(InitPropOp)
X(IsTypeOp)
X(FatalOp)
X(SetOpOp)
X(IncDecOp)
X(BareThisOp)
X(SilenceOp)
X(OODeclExistsOp)
X(ObjMethodOp)
X(SwitchKind)
X(QueryMOp)
X(PropElemOp)

#undef X

/*
 * MOpFlags is a bitmask so it doesn't fit into the [0,n) pattern of the other
 * subops above.
 */
const char* subopToName(MOpFlags f) {
  switch (f) {
#define FLAG(name, val) case MOpFlags::name: return #name;
  M_OP_FLAGS
#undef FLAG
  }
  always_assert_flog(false, "Invalid MOpFlags: {}", uint8_t(f));
}

template<>
bool subopValid(MOpFlags f) {
  switch (f) {
#define FLAG(name, val) case MOpFlags::name: return true;
  M_OP_FLAGS
#undef FLAG
  }
  return false;
}

template<>
folly::Optional<MOpFlags> nameToSubop<MOpFlags>(const char* str) {
#define FLAG(name, val) if (!strcmp(str, #name)) return MOpFlags::name;
  M_OP_FLAGS
#undef FLAG

  return folly::none;
}

//////////////////////////////////////////////////////////////////////

bool instrIsNonCallControlFlow(Op opcode) {
  if (!instrIsControlFlow(opcode) || isFCallStar(opcode)) return false;

  switch (opcode) {
    case OpAwait:
    case OpYield:
    case OpYieldK:
    case OpContEnter:
    case OpContRaise:
    case OpFCallBuiltin:
    case OpIncl:
    case OpInclOnce:
    case OpReq:
    case OpReqOnce:
    case OpReqDoc:
      return false;

    default:
      return true;
  }
}

bool instrAllowsFallThru(Op opcode) {
  InstrFlags opFlags = instrFlags(opcode);
  return (opFlags & TF) == 0;
}

bool instrReadsCurrentFpi(Op opcode) {
  InstrFlags opFlags = instrFlags(opcode);
  return (opFlags & FF) != 0;
}

ImmVector getImmVector(PC opcode) {
  auto const op = peek_op(opcode);
  int numImm = numImmediates(op);
  for (int k = 0; k < numImm; ++k) {
    ArgType t = immType(op, k);
    if (t == MA) {
      void* vp = getImmPtr(opcode, k);
      return ImmVector::createFromStream(
        static_cast<const uint8_t*>(vp));
    } else if (t == BLA || t == SLA || t == ILA) {
      void* vp = getImmPtr(opcode, k);
      return ImmVector::createFromStream(
        static_cast<const int32_t*>(vp));
    } else if (t == VSA) {
      const int32_t* vp = (int32_t*)getImmPtr(opcode, k);
      return ImmVector(reinterpret_cast<const uint8_t*>(vp + 1),
                       vp[0], vp[0]);
    }
  }

  not_reached();
}

MInstrLocation getMLocation(PC opcode) {
  auto immVec = getImmVector(opcode);
  auto vec = immVec.vec();
  auto const lcode = LocationCode(*vec++);
  auto const imm = numLocationCodeImms(lcode) ? decodeVariableSizeImm(&vec)
                                              : 0;
  return {lcode, imm};
}

bool hasMVector(Op op) {
  auto const num = numImmediates(op);
  for (int i = 0; i < num; ++i) {
    if (immType(op, i) == MA) return true;
  }
  return false;
}

std::vector<MVectorItem> getMVector(PC opcode) {
  auto immVec = getImmVector(opcode);
  std::vector<MVectorItem> result;
  auto it = immVec.vec();
  auto end = it + immVec.size();

  // Skip the LocationCode and its immediate
  auto const lcode = LocationCode(*it++);
  if (numLocationCodeImms(lcode)) decodeVariableSizeImm(&it);

  while (it < end) {
    auto const mcode = MemberCode(*it++);
    auto const imm = memberCodeHasImm(mcode) ? decodeMemberCodeImm(&it, mcode)
                                             : 0;
    result.push_back({mcode, imm});
  }

  return result;
}

const uint8_t* ImmVector::findLastMember() const {
  assert(m_length > 0);

  // Loop that does basically the same as numStackValues(), except
  // stop at the last.
  const uint8_t* vec = m_start;
  const LocationCode locCode = LocationCode(*vec++);
  const int numLocImms = numLocationCodeImms(locCode);
  for (int i = 0; i < numLocImms; ++i) {
    decodeVariableSizeImm(&vec);
  }

  for (;;) {
    const uint8_t* ret = vec;
    MemberCode code = MemberCode(*vec++);
    if (memberCodeHasImm(code)) {
      decodeMemberCodeImm(&vec, code);
    }
    if (vec - m_start == m_length) {
      return ret;
    }
    assert(vec - m_start < m_length);
  }

  not_reached();
}

bool ImmVector::decodeLastMember(const Unit* u,
                                 StringData*& sdOut,
                                 MemberCode& membOut,
                                 int64_t* strIdOut /*=NULL*/) const {
  const uint8_t* vec = findLastMember();
  membOut = MemberCode(*vec++);
  if (memberCodeImmIsString(membOut)) {
    int64_t strId = decodeMemberCodeImm(&vec, membOut);
    if (strIdOut) *strIdOut = strId;
    sdOut = u->lookupLitstrId(strId);
    return true;
  }
  return false;
}

int instrSpToArDelta(PC opcode) {
  // This function should only be called for instructions that read
  // the current FPI
  auto const op = peek_op(opcode);
  assert(instrReadsCurrentFpi(op));
  // The delta from sp to ar is equal to the number of values on the stack
  // that will be consumed by this instruction (numPops) plus the number of
  // parameters pushed onto the stack so far that are not being consumed by
  // this instruction (numExtra). For the FPass* instructions, numExtra will
  // be equal to the first immediate argument (param id). For the FCall
  // instructions, numExtra will be 0 because all of the parameters on the
  // stack are already accounted for by numPops.
  int numPops = instrNumPops(opcode);
  int numExtra = isFCallStar(op) ? 0 : getImm(opcode, 0).u_IVA;
  return numPops + numExtra;
}

std::string show(MInstrAttr mia) {
  if (mia == MIA_none) return "none";

  std::string ret;
  auto sep = "";
#define X(n) if (mia & MIA_##n) {               \
    folly::toAppend(sep, #n, &ret);             \
    sep = "|";                                  \
  }
  X(warn);
  X(define);
  X(reffy);
  X(unset);
  X(new);
  X(final_get);
#undef X

  return ret;
}

const MInstrInfo& getMInstrInfo(Op op) {
  static const MInstrInfo mInstrInfo[] = {
#define MII(instr, attrs, bS, iS, vC, fN)                               \
    {MI_##instr##M,                                                     \
     {MInstrAttr((attrs) & MIA_base), /* LL */                          \
      MIA_none,                       /* LC */                          \
      MIA_none,                       /* LH */                          \
      MInstrAttr((attrs) & MIA_base), /* LGL */                         \
      MInstrAttr((attrs) & MIA_base), /* LGC */                         \
      MInstrAttr((attrs) & MIA_base), /* LNL */                         \
      MInstrAttr((attrs) & MIA_base), /* LNC */                         \
      MIA_none,                       /* LSL */                         \
      MIA_none,                       /* LSC */                         \
      MIA_none},                      /* LR */                          \
     {MInstrAttr((attrs) & MIA_intermediate), /* MEC */                 \
      MInstrAttr((attrs) & MIA_intermediate), /* MPC */                 \
      MInstrAttr((attrs) & MIA_intermediate), /* MEL */                 \
      MInstrAttr((attrs) & MIA_intermediate), /* MPL */                 \
      MInstrAttr((attrs) & MIA_intermediate), /* MET */                 \
      MInstrAttr((attrs) & MIA_intermediate), /* MPT */                 \
      MInstrAttr((attrs) & MIA_intermediate), /* MQT */                 \
      MInstrAttr((attrs) & MIA_intermediate), /* MEI */                 \
      MInstrAttr((attrs) & MIA_final)},       /* MW */                  \
     unsigned(vC), bool((attrs) & MIA_new), bool((attrs) & MIA_final_get), \
     #instr},
    MINSTRS
#undef MII
  };

  switch (op) {
#define MII(instr_, attrs, bS, iS, vC, fN) \
  case Op##instr_##M: { \
    const MInstrInfo& mii = mInstrInfo[MI_##instr_##M]; \
    assert(mii.instr() == MI_##instr_##M); \
    return mii; \
  }
  MINSTRS
#undef MII
  default: not_reached();
  }
}

///////////////////////////////////////////////////////////////////////////////
}

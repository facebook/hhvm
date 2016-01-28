/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

static size_t encoded_iva_size(uint8_t lowByte) {
  // Low order bit set => 4-byte.
  return (lowByte & 0x1 ? sizeof(int32_t) : sizeof(int8_t));
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
    return encoded_iva_size(decode_raw<uint8_t>(pc));
  }

  if (immType(op, idx) == KA) {
    if (idx >= 1) pc += immSize(origPC, 0);
    if (idx >= 2) pc += immSize(origPC, 1);
    if (idx >= 3) pc += immSize(origPC, 2);

    switch (decode_raw<MemberCode>(pc)) {
      case MW:
        return 1;
      case MEL: case MPL: case MEC: case MPC:
        return 1 + encoded_iva_size(decode_raw<uint8_t>(pc));
      case MEI:
        return 1 + sizeof(int64_t);
      case MET: case MPT: case MQT:
        return 1 + sizeof(Id);
    }
    not_reached();
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
    int vecElemSz;
    auto itype = immType(op, idx);
    if (itype == BLA) {
      vecElemSz = sizeof(Offset);
    } else if (itype == ILA) {
      vecElemSz = 2 * sizeof(uint32_t);
    } else if (itype == VSA) {
      vecElemSz = sizeof(Id);
    } else {
      assert(itype == SLA);
      vecElemSz = sizeof(StrVecItem);
    }
    return sizeof(int32_t) + vecElemSz * decode_raw<int32_t>(pc);
  }

  ArgType type = immType(op, idx);
  return (type >= 0) ? argTypeToSizes[type] : 0;
}

bool immIsVector(Op opcode, int idx) {
  ArgType type = immType(opcode, idx);
  return type == BLA || type == SLA || type == ILA || type == VSA;
}

bool hasImmVector(Op opcode) {
  const int num = numImmediates(opcode);
  for (int i = 0; i < num; ++i) {
    if (immIsVector(opcode, i)) return true;
  }
  return false;
}

ArgUnion getImm(PC const origPC, int idx, const Unit* unit) {
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
  } else if (type == KA) {
    assert(unit != nullptr);
    retval.u_KA = decode_member_key(pc, unit);
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
#define IMM_KA 0
#define ONE(a) IMM_##a
#define TWO(a, b) (IMM_##a + 2 * IMM_##b)
#define THREE(a, b, c) (IMM_##a + 2 * IMM_##b + 4 * IMM_##c)
#define FOUR(a, b, c, d) (IMM_##a + 2 * IMM_##b + 4 * IMM_##c + 8 * IMM_##d)
#define O(name, imm, pop, push, flags) imm,
    OPCODES
#undef IMM_NA
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
#undef IMM_KA
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
#define MFINAL -3
#define F_MFINAL -6
#define C_MFINAL -5
#define V_MFINAL C_MFINAL
#define FMANY -3
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
#undef MFINAL
#undef F_MFINAL
#undef C_MFINAL
#undef V_MFINAL
#undef FMANY
#undef CVUMANY
#undef CMANY
#undef SMANY
#undef IDX_A
#undef O
  };
  auto const op = peek_op(pc);
  int n = numberOfPops[size_t(op)];
  // For most instructions, we know how many values are popped based
  // solely on the opcode
  if (n >= 0) return n;
  // BaseSC and BaseSL remove an A that may be on the top of the stack or one
  // element below the top, depending on the second immediate.
  if (n == -4) return getImm(pc, 1).u_IVA + 1;
  // FCall, NewPackedArray, and some final member operations specify how many
  // values are popped in their first immediate
  if (n == -3) return getImm(pc, 0).u_IVA;
  // FPassM final operations have paramId as imm 0 and stackCount as imm1
  if (n == -6) return getImm(pc, 1).u_IVA;
  // Other final member operations pop their first immediate + 1
  if (n == -5) return getImm(pc, 0).u_IVA + 1;

  // For instructions with vector immediates, we have to scan the contents of
  // the vector immediate to determine how many values are popped
  assert(n == -1);
  ImmVector iv = getImmVector(pc);
  int k = iv.numStackValues();
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
#define NOV always_assert(0 && "Opcode has no stack inputs");
#define ONE(f1) return doFlavor(idx, f1);
#define TWO(f1, f2) return doFlavor(idx, f1, f2);
#define THREE(f1, f2, f3) return doFlavor(idx, f1, f2, f3);
#define FOUR(f1, f2, f3, f4) return doFlavor(idx, f1, f2, f3, f4);
#define MFINAL return manyFlavor(op, idx, CRV);
#define F_MFINAL MFINAL
#define C_MFINAL return idx == 0 ? CV : CRV;
#define V_MFINAL return idx == 0 ? VV : CRV;
#define FMANY return manyFlavor(op, idx, FV);
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
#undef MFINAL
#undef F_MFINAL
#undef C_MFINAL
#undef V_MFINAL
#undef FMANY
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

void staticArrayStreamer(const ArrayData* ad, std::ostream& out) {
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
          case KindOfPersistentString:
          case KindOfString:
            out << "\""
                << escapeStringForCPP(val.getStringData()->data(),
                                      val.getStringData()->size())
                << "\"";
            return;
          case KindOfPersistentArray:
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
    case KindOfPersistentString:
    case KindOfString:
      out << "\"" << tv->m_data.pstr->data() << "\"";
      return;
    case KindOfPersistentArray:
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

std::string instrToString(PC it, Either<const Unit*, const UnitEmitter*> u) {
  std::stringstream out;
  PC iStart = it;
  Op op = decode_op(it);

  auto readRATA = [&] {
    if (auto unit = u.left()) {
      auto const rat = decodeRAT(unit, it);
      out << ' ' << show(rat);
      return;
    }

    auto const pc = it;
    it += encodedRATSize(pc);
    out << " <RepoAuthType>";
  };

  auto offsetOf = [u](PC pc) {
    return u.match(
      [pc](const Unit* u) { return u->offsetOf(pc); },
      [pc](const UnitEmitter* ue) { return ue->offsetOf(pc); }
    );
  };

  auto lookupLitstrId = [u](Id id) {
    return u.match(
      [id](const Unit* u) { return u->lookupLitstrId(id); },
      [id](const UnitEmitter* ue) { return ue->lookupLitstr(id); }
    );
  };

  auto lookupArrayId = [u](Id id) {
    return u.match(
      [id](const Unit* u) { return u->lookupArrayId(id); },
      [id](const UnitEmitter* ue) { return ue->lookupArray(id); }
    );
  };

  switch (op) {

#define READ(t) out << " " << *((t*)&*it); it += sizeof(t)

#define READOFF() do {                                              \
  Offset _value = *(Offset*)it;                                     \
  out << " " << _value;                                             \
  if (u != nullptr) {                                               \
    out << " (" << offsetOf(iStart + _value) << ")";                \
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

#define READLITSTR(sep) do {                                      \
  Id id = decode_raw<Id>(it);                                     \
  if (id < 0) {                                                   \
    assert(op == OpSSwitch);                                      \
    out << sep << "-";                                            \
  } else {                                                        \
    auto const sd = lookupLitstrId(id);                           \
    out << sep << "\"" <<                                         \
      escapeStringForCPP(sd->data(), sd->size()) << "\"";         \
  }                                                               \
} while (false)

#define READSVEC() do {                         \
  int sz = decode_raw<int>(it);                 \
  out << " <";                                  \
  const char* sep = "";                         \
  for (int i = 0; i < sz; ++i) {                \
    out << sep;                                 \
    if (op == OpSSwitch) {                      \
      READLITSTR("");                           \
      out << ":";                               \
    }                                           \
    Offset o = decode_raw<Offset>(it);          \
    out << offsetOf(iStart + o);                \
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
#define H_AA do {                                                \
  out << ' ';                                                    \
  staticArrayStreamer(lookupArrayId(decode_raw<Id>(it)), out);   \
} while (false)
#define H_VSA do {                                      \
  int sz = decode_raw<int32_t>(it);                     \
  out << " <";                                          \
  for (int i = 0; i < sz; ++i) {                        \
    H_SA;                                               \
  }                                                     \
  out << " >";                                          \
} while (false)
#define H_KA out << ' ' << show(decode_member_key(it, u))

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
#undef H_KA
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
    case OpContEnterDelegate:
    case OpYieldFromDelegate:
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
    if (t == BLA || t == SLA || t == ILA) {
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

int instrFpToArDelta(const Func* func, PC opcode) {
  // This function should only be called for instructions that read the current
  // FPI
  assert(instrReadsCurrentFpi(peek_op(opcode)));
  auto const fpi = func->findFPI(func->unit()->offsetOf(opcode));
  assert(fpi != nullptr);
  return fpi->m_fpOff;
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

///////////////////////////////////////////////////////////////////////////////
}

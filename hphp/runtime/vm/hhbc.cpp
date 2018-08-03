/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
  assertx(isValidOpcode(opcode));
  static const int8_t values[] = {
#define NA         0
#define ONE(...)   1
#define TWO(...)   2
#define THREE(...) 3
#define FOUR(...)  4
#define FIVE(...)  5
#define O(name, imm, unusedPop, unusedPush, unusedFlags) imm,
    OPCODES
#undef O
#undef NA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef FIVE
  };
  return values[size_t(opcode)];
}

ArgType immType(const Op opcode, int idx) {
  assertx(isValidOpcode(opcode));
  assertx(idx >= 0 && idx < numImmediates(opcode));
  always_assert(idx < kMaxHhbcImms); // No opcodes have more than 5 immediates
  static const int8_t argTypes[][kMaxHhbcImms] = {
#define NA                  {-1, -1, -1, -1, -1},
#define ONE(a)              { a, -1, -1, -1, -1},
#define TWO(a, b)           { a,  b, -1, -1, -1},
#define THREE(a, b, c)      { a,  b,  c, -1, -1},
#define FOUR(a, b, c, d)    { a,  b,  c,  d, -1},
#define FIVE(a, b, c, d, e) { a,  b,  c,  d,  e},
#define OA(x) OA
#define O(name, imm, unusedPop, unusedPush, unusedFlags) imm
    OPCODES
#undef O
#undef OA
#undef NA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef FIVE
  };
  auto opInt = size_t(opcode);
  return (ArgType)argTypes[opInt][idx];
}

static size_t encoded_iva_size(uint8_t lowByte) {
  // High order bit set => 4-byte.
  return int8_t(lowByte) >= 0 ? 1 : 4;
}

namespace {

bool argTypeIsVector(ArgType type) {
  return
    type == BLA || type == SLA || type == VSA || type == I32LA || type == BLLA;
}

int immSize(ArgType type, PC immPC) {
  auto pc = immPC;
  static const int8_t argTypeToSizes[] = {
#define ARGTYPE(nm, type) sizeof(type),
#define ARGTYPEVEC(nm, type) 0,
    ARGTYPES
#undef ARGTYPE
#undef ARGTYPEVEC
  };

  if (type == IVA || type == LA || type == IA || type == CAR || type == CAW) {
    return encoded_iva_size(decode_raw<uint8_t>(pc));
  }

  if (type == KA) {
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

  if (type == RATA) {
    return encodedRATSize(pc);
  }

  if (type == LAR) {
    decode_iva(pc); // first
    decode_iva(pc); // restCount
    return pc - immPC;
  }

  if (argTypeIsVector(type)) {
    auto size = decode_iva(pc);
    int vecElemSz;
    switch (type) {
      case BLA:   vecElemSz = sizeof(Offset);     break;
      case SLA:   vecElemSz = sizeof(StrVecItem); break;
      case I32LA: vecElemSz = sizeof(uint32_t);   break;
      case BLLA:  vecElemSz = sizeof(uint8_t);    break;
      case VSA:   vecElemSz = sizeof(Id);         break;
      default: not_reached();
    }

    if (type == BLLA) {
      size = (size + 7) / 8;
    }

    return pc - immPC + vecElemSz * size;
  }

  if (type == ILA) {
    auto const size = decode_iva(pc);
    for (int i = 0; i < size; ++i) {
      auto const kind = static_cast<IterKind>(decode_iva(pc));
      decode_iva(pc);
      if (kind == KindOfLIter) decode_iva(pc);
    }
    return pc - immPC;
  }

  return (type >= 0) ? argTypeToSizes[type] : 0;
}

}

bool hasImmVector(Op opcode) {
  const int num = numImmediates(opcode);
  for (int i = 0; i < num; ++i) {
    if (argTypeIsVector(immType(opcode, i))) return true;
  }
  return false;
}

bool hasIterTable(Op opcode) {
  auto const num = numImmediates(opcode);
  for (int i = 0; i < num; ++i) {
    if (immType(opcode, i) == ILA) return true;
  }
  return false;
}

ArgUnion getImm(const PC origPC, int idx, const Unit* unit) {
  auto pc = origPC;
  auto const op = decode_op(pc);
  assertx(idx >= 0 && idx < numImmediates(op));
  ArgUnion retval;
  retval.u_NA = 0;
  int cursor = 0;
  for (cursor = 0; cursor < idx; cursor++) {
    // Advance over this immediate.
    pc += immSize(immType(op, cursor), pc);
  }
  always_assert(cursor == idx);
  auto const type = immType(op, idx);
  if (type == IVA || type == LA || type == IA ||
      type == CAR || type == CAW) {
    retval.u_IVA = decode_iva(pc);
  } else if (type == KA) {
    assertx(unit != nullptr);
    retval.u_KA = decode_member_key(pc, unit);
  } else if (type == LAR) {
    retval.u_LAR = decodeLocalRange(pc);
  } else if (type == RATA) {
    assertx(unit != nullptr);
    retval.u_RATA = decodeRAT(unit, pc);
  } else if (!argTypeIsVector(type)) {
    memcpy(&retval.bytes, pc, immSize(type, pc));
  }
  always_assert(numImmediates(op) > idx);
  return retval;
}

ArgUnion* getImmPtr(const PC origPC, int idx) {
  auto pc = origPC;
  auto const op = decode_op(pc);
  assertx(immType(op, idx) != IVA);
  assertx(immType(op, idx) != LA);
  assertx(immType(op, idx) != IA);
  assertx(immType(op, idx) != CAR);
  assertx(immType(op, idx) != CAW);
  assertx(immType(op, idx) != RATA);
  for (int i = 0; i < idx; i++) {
    pc += immSize(immType(op, i), pc);
  }
  return (ArgUnion*)pc;
}

template<typename T>
T decodeImm(const unsigned char** immPtr) {
  T val = *(T*)*immPtr;
  *immPtr += sizeof(T);
  return val;
}

int instrLen(const PC origPC) {
  auto pc = origPC;
  auto op = decode_op(pc);
  int nImm = numImmediates(op);
  for (int i = 0; i < nImm; i++) {
    pc += immSize(immType(op, i), pc);
  }
  return pc - origPC;
}

Offset* instrJumpOffset(const PC origPC) {
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
#define IMM_I32LA 0
#define IMM_BLLA 0
#define IMM_SLA 0
#define IMM_LA 0
#define IMM_IA 0
#define IMM_CAR 0
#define IMM_CAW 0
#define IMM_OA(x) 0
#define IMM_VSA 0
#define IMM_KA 0
#define IMM_LAR 0
#define ONE(a) IMM_##a
#define TWO(a, b) (IMM_##a + 2 * IMM_##b)
#define THREE(a, b, c) (IMM_##a + 2 * IMM_##b + 4 * IMM_##c)
#define FOUR(a, b, c, d) (IMM_##a + 2 * IMM_##b + 4 * IMM_##c + 8 * IMM_##d)
#define FIVE(a, b, c, d, e) (IMM_##a + 2 * IMM_##b + 4 * IMM_##c + 8 * IMM_##d + 16 * IMM_##e)
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
#undef IMM_CAR
#undef IMM_CAW
#undef IMM_BA
#undef IMM_BLA
#undef IMM_ILA
#undef IMM_I32LA
#undef IMM_BLLA
#undef IMM_SLA
#undef IMM_OA
#undef IMM_VSA
#undef IMM_KA
#undef IMM_LAR
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef FIVE
#undef O
  };

  auto pc = origPC;
  auto const op = decode_op(pc);
  assertx(!isSwitch(op));  // BLA doesn't work here

  if (op == OpIterBreak) {
    // offset is imm number 0
    return const_cast<Offset*>(reinterpret_cast<const Offset*>(pc));
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
  case 16: immNum = 4; break;
  default: assertx(false); return nullptr;
  }

  return &getImmPtr(origPC, immNum)->u_BA;
}

Offset instrJumpTarget(PC instrs, Offset pos) {
  auto offset = instrJumpOffset(instrs + pos);
  return offset ? *offset + pos : InvalidAbsoluteOffset;
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
int numSuccs(const PC origPC) {
  auto pc = origPC;
  auto const op = decode_op(pc);
  if ((instrFlags(op) & TF) != 0) {
    if (isSwitch(op)) {
      if (op == Op::Switch) {
        decode_raw<SwitchKind>(pc); // skip bounded flag
        decode_raw<int64_t>(pc); // skip base
      }
      return decode_iva(pc); // vector length
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
#define FIVE(...) 5
#define MFINAL -3
#define C_MFINAL -5
#define V_MFINAL C_MFINAL
#define CVMANY -3
#define CVUMANY -3
#define FCALL -4
#define CMANY -3
#define SMANY -1
#define O(name, imm, pop, push, flags) pop,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef FIVE
#undef MFINAL
#undef C_MFINAL
#undef V_MFINAL
#undef CVMANY
#undef CVUMANY
#undef FCALL
#undef CMANY
#undef SMANY
#undef O
  };
  auto const op = peek_op(pc);
  int n = numberOfPops[size_t(op)];
  // For most instructions, we know how many values are popped based
  // solely on the opcode
  if (n >= 0) return n;
  // FCallAwait, NewPackedArray, and some final member operations specify how
  // many values are popped in their first immediate
  if (n == -3) return getImm(pc, 0).u_IVA;
  // FCall pops numArgs (imm0), unpack (imm1) and uninit values (imm2 - 1)
  if (n == -4) {
    return getImm(pc, 0).u_IVA + getImm(pc, 1).u_IVA + getImm(pc, 2).u_IVA - 1;
  }
  // Other final member operations pop their first immediate + 1
  if (n == -5) return getImm(pc, 0).u_IVA + 1;

  // For instructions with vector immediates, we have to scan the contents of
  // the vector immediate to determine how many values are popped
  assertx(n == -1);
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
#define FIVE(...) 5
#define INS_1(...) 0
#define FCALL -1
#define O(name, imm, pop, push, flags) push,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef FIVE
#undef INS_1
#undef FCALL
#undef O
  };
  auto const op = peek_op(pc);
  int n = numberOfPushes[size_t(op)];

  // The FCall call flavors push a tuple of arguments onto the stack
  if (n == -1) return getImm(pc, 2).u_IVA;

  return n;
}

namespace {
FlavorDesc doFlavor(uint32_t /*i*/) {
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

FlavorDesc fcallFlavor(PC op, uint32_t i) {
  always_assert(i < uint32_t(instrNumPops(op)));
  auto const numArgs = getImm(op, 0).u_IVA;
  auto const unpack = getImm(op, 1).u_IVA;
  if (i == 0 && unpack) return CV;
  return i < numArgs + unpack ? CVV : UV;
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
#define FIVE(f1, f2, f3, f4, f5) return doFlavor(idx, f1, f2, f3, f4, f5);
#define MFINAL return manyFlavor(op, idx, CRV);
#define C_MFINAL return idx == 0 ? CV : CRV;
#define V_MFINAL return idx == 0 ? VV : CRV;
#define CVMANY return manyFlavor(op, idx, CVV);
#define CVUMANY return manyFlavor(op, idx, CVUV);
#define FCALL return fcallFlavor(op, idx);
#define CMANY return manyFlavor(op, idx, CV);
#define SMANY return manyFlavor(op, idx, CV);
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
#undef FIVE
#undef MFINAL
#undef C_MFINAL
#undef V_MFINAL
#undef CVMANY
#undef CVUMANY
#undef FCALL
#undef CMANY
#undef SMANY
#undef O
}

StackTransInfo instrStackTransInfo(PC opcode) {
  static const StackTransInfo::Kind transKind[] = {
#define NOV StackTransInfo::Kind::PushPop
#define ONE(...) StackTransInfo::Kind::PushPop
#define TWO(...) StackTransInfo::Kind::PushPop
#define THREE(...) StackTransInfo::Kind::PushPop
#define FOUR(...) StackTransInfo::Kind::PushPop
#define FIVE(...) StackTransInfo::Kind::PushPop
#define FCALL StackTransInfo::Kind::PushPop
#define INS_1(...) StackTransInfo::Kind::InsertMid
#define O(name, imm, pop, push, flags) push,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef FIVE
#undef INS_1
#undef FCALL
#undef O
  };
  static const int8_t peekPokeType[] = {
#define NOV -1
#define ONE(...) -1
#define TWO(...) -1
#define THREE(...) -1
#define FOUR(...) -1
#define FIVE(...) -1
#define FCALL -1
#define INS_1(...) 0
#define O(name, imm, pop, push, flags) push,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef FIVE
#undef INS_1
#undef FCALL
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
    case OpFPushClsMethodS:
    case OpFPushClsMethodSD:
    case OpFPushClsMethodD:
    case OpFPushCtor:
    case OpFPushCtorD:
    case OpFPushCtorI:
    case OpFPushCtorS:
    case OpFPushCufIter:
      return true;
    default:
      return false;
  }
}

void staticArrayStreamer(const ArrayData* ad, std::string& out) {
  if (ad->isVecArray()) out += "vec(";
  else if (ad->isDict()) out += "dict(";
  else if (ad->isKeyset()) out += "keyset(";
  else {
    assertx(ad->isPHPArray());
    if (ad->isVArray()) out += "varray(";
    else if (ad->isDArray()) out += "darray(";
    else out += "array(";
  }

  if (!ad->empty()) {
    bool comma = false;
    for (ArrayIter it(ad); !it.end(); it.next()) {
      if (comma) {
        out += ",";
      } else {
        comma = true;
      }
      Variant key = it.first();

      if (!ad->isVecArray() && !ad->isKeyset()) {
        staticStreamer(key.asTypedValue(), out);
        out += "=>";
      }

      Variant val = it.second();

      staticStreamer(val.asTypedValue(), out);
    }
  }
  out += ")";
}

void staticStreamer(const TypedValue* tv, std::string& out) {
  switch (tv->m_type) {
    case KindOfUninit:
    case KindOfNull:
      out += "null";
      return;
    case KindOfBoolean:
      out += (tv->m_data.num ? "true" : "false");
      return;
    case KindOfInt64:
      out += folly::to<std::string>(tv->m_data.num);
      return;
    case KindOfDouble:
      out += folly::to<std::string>(tv->m_data.dbl);
      return;
    case KindOfPersistentString:
    case KindOfString:
      folly::format(&out, "\"{}\"",
                    escapeStringForCPP(tv->m_data.pstr->data(),
                                       tv->m_data.pstr->size()));
      return;
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
      staticArrayStreamer(tv->m_data.parr, out);
      return;
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
    case KindOfFunc:
    case KindOfClass:
      break;
  }
  not_reached();
}

std::string instrToString(PC it, Either<const Unit*, const UnitEmitter*> u) {
  std::string out;
  PC iStart = it;
  Op op = decode_op(it);

  auto readRATA = [&] {
    if (auto unit = u.left()) {
      auto const rat = decodeRAT(unit, it);
      folly::format(&out, " {}", show(rat));
      return;
    }

    auto const pc = it;
    it += encodedRATSize(pc);
    out += " <RepoAuthType>";
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

#define READ(t) folly::format(&out, " {}", *((t*)&*it)); it += sizeof(t)

#define READOFF() do {                                          \
  Offset _value = *(Offset*)it;                                 \
  folly::format(&out, " {}", _value);                           \
  if (u != nullptr) {                                           \
    folly::format(&out, " ({})", offsetOf(iStart + _value));    \
  }                                                             \
  it += sizeof(Offset);                                         \
} while (false)

#define READV() folly::format(&out, " {}", decode_iva(it));

#define READLA() folly::format(&out, " L:{}", decode_iva(it));

#define READIVA() do {                                          \
  auto imm = decode_iva(it);                                    \
  folly::format(&out, " {}", imm);                              \
  immIdx++;                                                     \
} while (false)

#define READOA(type) do {                               \
  auto const immVal = static_cast<type>(                \
    *reinterpret_cast<const uint8_t*>(it)               \
  );                                                    \
  it += sizeof(unsigned char);                          \
  folly::format(&out, " {}", subopToName(immVal));      \
} while (false)

#define READLITSTR(sep) do {                                    \
  Id id = decode_raw<Id>(it);                                   \
  if (id < 0) {                                                 \
    assertx(op == OpSSwitch);                                    \
    folly::format(&out, "{}-", sep);                            \
  } else {                                                      \
    auto const sd = lookupLitstrId(id);                         \
    folly::format(&out, "{}\"{}\"", sep,                        \
                  escapeStringForCPP(sd->data(), sd->size()));  \
  }                                                             \
} while (false)

#define READSVEC() do {                                 \
  int sz = decode_iva(it);                              \
  out += " <";                                          \
  const char* sep = "";                                 \
  for (int i = 0; i < sz; ++i) {                        \
    out += sep;                                         \
    if (op == OpSSwitch) {                              \
      READLITSTR("");                                   \
      out += ":";                                       \
    }                                                   \
    Offset o = decode_raw<Offset>(it);                  \
    folly::format(&out, "{}", offsetOf(iStart + o));    \
    sep = " ";                                          \
  }                                                     \
  out += ">";                                           \
} while (false)

#define READI32VEC() do {                                      \
  int sz = decode_iva(it);                                     \
  out += " <";                                                 \
  const char* sep = "";                                        \
  for (int i = 0; i < sz; ++i) {                               \
    folly::format(&out, "{}{}", sep, decode_raw<uint32_t>(it));\
    sep = ", ";                                                \
  }                                                            \
  out += ">";                                                  \
} while (false)

#define READBOOLVEC() do {                                     \
  int sz = decode_iva(it);                                     \
  uint8_t tmp = 0;                                             \
  out += " \"";                                                \
  for (int i = 0; i < sz; ++i) {                               \
    if (i % 8 == 0) tmp = decode_raw<uint8_t>(it);             \
    out += ((tmp >> (i % 8)) & 1) ? "1" : "0";                 \
  }                                                            \
  out += "\"";                                                 \
} while (false)

#define READITERTAB() do {                              \
  auto const sz = decode_iva(it);                       \
  out += " <";                                          \
  const char* sep = "";                                 \
  for (int i = 0; i < sz; ++i) {                        \
    out += sep;                                         \
    auto const k = (IterKind)decode_iva(it);            \
    switch (k) {                                        \
      case KindOfIter:  out += "(Iter) ";  break;       \
      case KindOfMIter: out += "(MIter) "; break;       \
      case KindOfCIter: out += "(CIter) "; break;       \
      case KindOfLIter: out += "(LIter) "; break;       \
    }                                                   \
    folly::format(&out, "{}", decode_iva(it));           \
    if (k == KindOfLIter) {                             \
      folly::format(&out, " L:{}", decode_iva(it));     \
    }                                                   \
    sep = ", ";                                         \
  }                                                     \
  out += ">";                                           \
} while (false)

#define ONE(a) H_##a
#define TWO(a, b) H_##a; H_##b
#define THREE(a, b, c) H_##a; H_##b; H_##c;
#define FOUR(a, b, c, d) H_##a; H_##b; H_##c; H_##d;
#define FIVE(a, b, c, d, e) H_##a; H_##b; H_##c; H_##d; H_##e;
#define NA
#define H_BLA READSVEC()
#define H_SLA READSVEC()
#define H_ILA READITERTAB()
#define H_I32LA READI32VEC()
#define H_BLLA READBOOLVEC()
#define H_IVA READIVA()
#define H_I64A READ(int64_t)
#define H_LA READLA()
#define H_IA READV()
#define H_CAR READV()
#define H_CAW READV()
#define H_DA READ(double)
#define H_BA READOFF()
#define H_OA(type) READOA(type)
#define H_SA READLITSTR(" ")
#define H_RATA readRATA()
#define H_AA do {                                                \
  out += ' ';                                                    \
  staticArrayStreamer(lookupArrayId(decode_raw<Id>(it)), out);   \
} while (false)
#define H_VSA do {                                      \
  int sz = decode_iva(it);                              \
  out += " <";                                          \
  for (int i = 0; i < sz; ++i) {                        \
    H_SA;                                               \
  }                                                     \
  out += " >";                                          \
} while (false)
#define H_KA (out += ' ', out += show(decode_member_key(it, u)))
#define H_LAR (out += ' ', out += show(decodeLocalRange(it)))

#define O(name, imm, push, pop, flags)    \
  case Op##name: {                        \
    out += #name;                         \
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
#undef FIVE
#undef NA
#undef H_BLA
#undef H_SLA
#undef H_ILA
#undef H_I32LA
#undef H_BLLA
#undef H_IVA
#undef H_I64A
#undef H_LA
#undef H_IA
#undef H_CAR
#undef H_CAW
#undef H_DA
#undef H_BA
#undef H_OA
#undef H_SA
#undef H_AA
#undef H_VSA
#undef H_KA
    default: assertx(false);
  };
  return out;
}

const char* opcodeToName(Op op) {
  static const char* namesArr[] = {
#define O(name, imm, inputs, outputs, flags) \
    #name ,
    OPCODES
#undef O
  };
  if (size_t(op) < Op_count) {
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

static const char* CollectionType_names[] = {
#define COL(x) #x,
  COLLECTION_TYPES
#undef COL
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

static const char* MOpMode_names[] = {
#define MODE(x) #x,
  M_OP_MODES
#undef MODE
};

static const char* ContCheckOp_names[] = {
#define CONT_CHECK_OP(x) #x,
  CONT_CHECK_OPS
#undef CONT_CHECK_OP
};

static const char* CudOp_names[] = {
#define CUD_OP(x) #x,
  CUD_OPS
#undef CUD_OP
};

static const char* FPassHint_names[] = {
#define OP(x) #x,
  FPASS_HINT_OPS
#undef OP
};

static const char* SpecialClsRef_names[] = {
#define REF(x) #x,
  SPECIAL_CLS_REFS
#undef REF
};

template<class T, size_t Sz>
const char* subopToNameImpl(const char* (&arr)[Sz], T opcode, int off) {
  static_assert(
    std::is_same<typename std::underlying_type<T>::type,uint8_t>::value,
    "Subops are all expected to be single-bytes"
  );
  auto const idx = static_cast<uint8_t>(opcode) - off;
  always_assert(idx < Sz);
  return arr[idx];
}

template <class T, size_t Sz>
bool subopValidImpl(const char* (&/*arr*/)[Sz], T op, int off) {
  auto raw = static_cast<typename std::underlying_type<T>::type>(op) - off;
  return raw >= 0 && raw < Sz;
}

template<class T, size_t Sz>
folly::Optional<T> nameToSubopImpl(const char* (&arr)[Sz],
  const char* str, int off) {
  for (auto i = size_t{0}; i < Sz; ++i) {
    if (!strcmp(str, arr[i])) return static_cast<T>(i + off);
  }
  return folly::none;
}

namespace {
template<class T> struct NameToSubopHelper;
}

template<class T> folly::Optional<T> nameToSubop(const char* str) {
  return NameToSubopHelper<T>::conv(str);
}

#define X(subop, off)                                              \
  const char* subopToName(subop op) {                              \
    return subopToNameImpl(subop##_names, op, off);                \
  }                                                                \
  template<> bool subopValid(subop op) {                           \
    return subopValidImpl(subop##_names, op, off);                 \
  }                                                                \
  namespace {                                                      \
  template<> struct NameToSubopHelper<subop> {                     \
    static folly::Optional<subop> conv(const char* str) {          \
      return nameToSubopImpl<subop>(subop##_names, str, off);      \
    }                                                              \
  };                                                               \
  }                                                                \
  template folly::Optional<subop> nameToSubop(const char*);

// Not all subops start indexing at 0
/*Subop Name      Numerically first value */
X(InitPropOp,     static_cast<int>(InitPropOp::Static))
X(IsTypeOp,       static_cast<int>(IsTypeOp::Null))
X(FatalOp,        static_cast<int>(FatalOp::Runtime))
X(SetOpOp,        static_cast<int>(SetOpOp::PlusEqual))
X(IncDecOp,       static_cast<int>(IncDecOp::PreInc))
X(BareThisOp,     static_cast<int>(BareThisOp::Notice))
X(SilenceOp,      static_cast<int>(SilenceOp::Start))
X(CollectionType, static_cast<int>(HeaderKind::Vector))
X(OODeclExistsOp, static_cast<int>(OODeclExistsOp::Class))
X(ObjMethodOp,    static_cast<int>(ObjMethodOp::NullThrows))
X(SwitchKind,     static_cast<int>(SwitchKind::Unbounded))
X(QueryMOp,       static_cast<int>(QueryMOp::CGet))
X(MOpMode,        static_cast<int>(MOpMode::None))
X(ContCheckOp,    static_cast<int>(ContCheckOp::IgnoreStarted))
X(CudOp,          static_cast<int>(CudOp::IgnoreIter))
X(FPassHint,      static_cast<int>(FPassHint::Any))
X(SpecialClsRef,  static_cast<int>(SpecialClsRef::Self))
#undef X

//////////////////////////////////////////////////////////////////////

bool instrIsNonCallControlFlow(Op opcode) {
  if (!instrIsControlFlow(opcode) || isFCallStar(opcode)) return false;

  switch (opcode) {
    case OpAwait:
    case OpAwaitAll:
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
    if (t == BLA || t == SLA || t == I32LA || t == BLLA || t == VSA) {
      PC vp = getImmPtr(opcode, k)->bytes;
      auto const size = decode_iva(vp);
      return ImmVector(vp, size, t == VSA ? size : 0);
    }
  }

  not_reached();
}

IterTable iterTableFromStream(PC& pc) {
  IterTable ret;
  auto const length = decode_iva(pc);
  for (int32_t i = 0; i < length; ++i) {
    auto const kind = static_cast<IterKind>(decode_iva(pc));
    auto const id = decode_iva(pc);
    auto const local = (kind == KindOfLIter)
      ? static_cast<int32_t>(decode_iva(pc))
      : kInvalidId;
    ret.push_back(IterTableEnt{kind, static_cast<int32_t>(id), local});
  }
  return ret;
}

IterTable getIterTable(PC opcode) {
  auto const op = peek_op(opcode);
  auto const numImm = numImmediates(op);
  for (int k = 0; k < numImm; ++k) {
    auto const type = immType(op, k);
    if (type != ILA) continue;
    auto ptr = reinterpret_cast<PC>(getImmPtr(opcode, k));
    return iterTableFromStream(ptr);
  }
  not_reached();
}

int instrFpToArDelta(const Func* func, PC opcode) {
  // This function should only be called for instructions that read the current
  // FPI
  assertx(instrReadsCurrentFpi(peek_op(opcode)));
  auto const fpi = func->findFPI(func->unit()->offsetOf(opcode));
  assertx(fpi != nullptr);
  return fpi->m_fpOff;
}

///////////////////////////////////////////////////////////////////////////////

std::string show(const LocalRange& range) {
  return folly::sformat(
    "L:{}+{}", range.first, range.count
  );
}

///////////////////////////////////////////////////////////////////////////////
}

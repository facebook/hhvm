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
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unwind.h"
#include "hphp/util/text-util.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

EXTERNALLY_VISIBLE
extern const int8_t numImmediatesTable[] = {
#define NA         0
#define ONE(...)   1
#define TWO(...)   2
#define THREE(...) 3
#define FOUR(...)  4
#define FIVE(...)  5
#define SIX(...)   6
#define O(name, imm, unusedPop, unusedPush, unusedFlags) imm,
    OPCODES
#undef O
#undef NA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef FIVE
#undef SIX
};

int numImmediates(Op opcode) {
  assertx(isValidOpcode(opcode));
  return numImmediatesTable[size_t(opcode)];
}

EXTERNALLY_VISIBLE
extern const int8_t immTypeTable[][kMaxHhbcImms] = {
#define NA                    {-1, -1, -1, -1, -1, -1},
#define ONE(a)                { a, -1, -1, -1, -1, -1},
#define TWO(a, b)             { a,  b, -1, -1, -1, -1},
#define THREE(a, b, c)        { a,  b,  c, -1, -1, -1},
#define FOUR(a, b, c, d)      { a,  b,  c,  d, -1, -1},
#define FIVE(a, b, c, d, e)   { a,  b,  c,  d,  e, -1},
#define SIX(a, b, c, d, e, f) { a,  b,  c,  d,  e,  f},
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
#undef SIX
};

ArgType immType(const Op opcode, int idx) {
  assertx(isValidOpcode(opcode));
  assertx(idx >= 0 && idx < numImmediates(opcode));
  always_assert(idx < kMaxHhbcImms); // No opcodes have more than 6 immediates
  auto opInt = size_t(opcode);
  return (ArgType)immTypeTable[opInt][idx];
}

static size_t encoded_iva_size(uint8_t lowByte) {
  // High order bit set => 4-byte.
  return int8_t(lowByte) >= 0 ? 1 : 4;
}

EXTERNALLY_VISIBLE
extern const int8_t immSizeTable[] = {
#define ARGTYPE(nm, type) sizeof(type),
#define ARGTYPEVEC(nm, type) 0,
    ARGTYPES
#undef ARGTYPE
#undef ARGTYPEVEC
};

namespace {

bool argTypeIsVector(ArgType type) {
  return
    type == BLA || type == SLA || type == VSA;
}

int immSize(Op op, ArgType type, PC immPC) {
  auto pc = immPC;

  if (type == IVA || type == LA || type == ILA || type == IA) {
    return encoded_iva_size(decode_raw<uint8_t>(pc));
  }

  if (type == NLA) {
    auto nextPC = pc;
    auto const nameSize = encoded_iva_size(decode_raw<uint8_t>(pc));
    nextPC += nameSize;
    auto const locSize = encoded_iva_size(decode_raw<uint8_t>(nextPC));
    return nameSize + locSize;
  }

  if (type == KA) {
    switch (decode_raw<MemberCode>(pc)) {
      case MW:
        return 1;
      case MEC: case MPC:
        return 1 + encoded_iva_size(decode_raw<uint8_t>(pc));
      case MEL: case MPL: {
        auto nextPC = pc;
        auto const nameSize = encoded_iva_size(decode_raw<uint8_t>(pc));
        nextPC += nameSize;
        auto const locSize = encoded_iva_size(decode_raw<uint8_t>(nextPC));
        return 1 + nameSize + locSize;
      }
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

  if (type == ITA) {
    decodeIterArgs(pc);
    return pc - immPC;
  }

  if (type == FCA) {
    decodeFCallArgs(op, pc, nullptr /* StringDecoder */);
    return pc - immPC;
  }

  if (argTypeIsVector(type)) {
    auto size = decode_iva(pc);
    int vecElemSz;
    switch (type) {
      case BLA:   vecElemSz = sizeof(Offset);     break;
      case SLA:   vecElemSz = sizeof(StrVecItem); break;
      case VSA:   vecElemSz = sizeof(Id);         break;
      default: not_reached();
    }

    return pc - immPC + vecElemSz * size;
  }

  return (type >= 0) ? immSizeTable[type] : 0;
}

}

bool hasImmVector(Op opcode) {
  const int num = numImmediates(opcode);
  for (int i = 0; i < num; ++i) {
    if (argTypeIsVector(immType(opcode, i))) return true;
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
    pc += immSize(op, immType(op, cursor), pc);
  }
  always_assert(cursor == idx);
  auto const type = immType(op, idx);
  if (type == IVA || type == LA || type == ILA || type == IA) {
    retval.u_IVA = decode_iva(pc);
  } else if (type == NLA) {
    retval.u_NLA = decode_named_local(pc);
  } else if (type == KA) {
    assertx(unit != nullptr);
    retval.u_KA = decode_member_key(pc, unit);
  } else if (type == LAR) {
    retval.u_LAR = decodeLocalRange(pc);
  } else if (type == ITA) {
    retval.u_ITA = decodeIterArgs(pc);
  } else if (type == FCA) {
    retval.u_FCA = decodeFCallArgs(op, pc, unit);
  } else if (type == RATA) {
    assertx(unit != nullptr);
    retval.u_RATA = decodeRAT(unit, pc);
  } else if (!argTypeIsVector(type)) {
    memcpy(&retval.bytes, pc, immSize(op, type, pc));
  }
  always_assert(numImmediates(op) > idx);
  return retval;
}

ArgUnion* getImmPtr(const PC origPC, int idx) {
  auto pc = origPC;
  auto const op = decode_op(pc);
  assertx(immType(op, idx) != IVA);
  assertx(immType(op, idx) != LA);
  assertx(immType(op, idx) != NLA);
  assertx(immType(op, idx) != ILA);
  assertx(immType(op, idx) != IA);
  assertx(immType(op, idx) != RATA);
  for (int i = 0; i < idx; i++) {
    pc += immSize(op, immType(op, i), pc);
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
    pc += immSize(op, immType(op, i), pc);
  }
  return pc - origPC;
}

OffsetList instrJumpOffsets(const PC origPC) {
  static const std::array<uint8_t, kMaxHhbcImms> argTypes[] = {
#define IMM_NA 0
#define IMM_IVA 0
#define IMM_I64A 0
#define IMM_DA 0
#define IMM_SA 0
#define IMM_AA 0
#define IMM_RATA 0
#define IMM_BA 1
#define IMM_BLA 2
#define IMM_SLA 3
#define IMM_LA 0
#define IMM_NLA 0
#define IMM_ILA 0
#define IMM_IA 0
#define IMM_OA(x) 0
#define IMM_VSA 0
#define IMM_KA 0
#define IMM_LAR 0
#define IMM_ITA 0
#define IMM_FCA 0
#define NA                    { 0,        0,        0,        0,        0,       0      },
#define ONE(a)                { IMM_##a,  0,        0,        0,        0,       0      },
#define TWO(a, b)             { IMM_##a,  IMM_##b,  0,        0,        0,       0      },
#define THREE(a, b, c)        { IMM_##a,  IMM_##b,  IMM_##c,  0,        0,       0      },
#define FOUR(a, b, c, d)      { IMM_##a,  IMM_##b,  IMM_##c,  IMM_##d,  0,       0      },
#define FIVE(a, b, c, d, e)   { IMM_##a,  IMM_##b,  IMM_##c,  IMM_##d,  IMM_##e, 0      },
#define SIX(a, b, c, d, e, f) { IMM_##a,  IMM_##b,  IMM_##c,  IMM_##d,  IMM_##e, IMM_##f},
#define OA(x) OA
#define O(name, imm, unusedPop, unusedPush, unusedFlags) imm
    OPCODES
#undef IMM_NA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_DA
#undef IMM_SA
#undef IMM_AA
#undef IMM_RATA
#undef IMM_LA
#undef IMM_NLA
#undef IMM_ILA
#undef IMM_IA
#undef IMM_BA
#undef IMM_BLA
#undef IMM_SLA
#undef IMM_OA
#undef IMM_VSA
#undef IMM_KA
#undef IMM_LAR
#undef IMM_ITA
#undef IMM_FCA
#undef O
#undef OA
#undef NA
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef FIVE
#undef SIX
  };

  auto pc = origPC;
  auto const op = decode_op(pc);

  OffsetList targets;
  if (isFCall(op)) {
    auto const offset = decodeFCallArgs(op, pc, nullptr).asyncEagerOffset;
    if (offset != kInvalidOffset) targets.emplace_back(offset);
    return targets;
  }

  auto const& types = argTypes[size_t(op)];
  for (size_t i = 0; i < types.size(); ++i) {
    switch (types[i]) {
      case 0:
        break;
      case 1:
        pc = origPC;
        targets.emplace_back(getImmPtr(pc, i)->u_BA);
        break;
      case 2: {
        pc = origPC;
        PC vp = getImmPtr(pc, i)->bytes;
        auto const size = decode_iva(vp);
        ImmVector iv(vp, size, 0);
        targets.insert(targets.end(), iv.vec32(), iv.vec32() + iv.size());
        break;
      }
      case 3: {
        pc = origPC;
        PC vp = getImmPtr(pc, i)->bytes;
        auto const size = decode_iva(vp);
        ImmVector iv(vp, size, 0);
        for (size_t j = 0; j < iv.size(); ++j) {
          targets.emplace_back(iv.strvec()[j].dest);
        }
        break;
      }
      default:
        always_assert(false);
    }
  }

  return targets;
}

OffsetList instrJumpTargets(PC instrs, Offset pos) {
  auto offsets = instrJumpOffsets(instrs + pos);
  for (auto& o : offsets) o += pos;
  return offsets;
}

OffsetSet instrSuccOffsets(PC opc, const Func* func) {
  auto const bcStart = func->unit()->entry();
  auto const offsets = instrJumpTargets(bcStart, opc - bcStart);
  OffsetSet offsetsSet{offsets.begin(), offsets.end()};

  auto const op = peek_op(opc);
  if (!instrIsControlFlow(op) || instrAllowsFallThru(op)) {
    Offset succOff = opc + instrLen(opc) - bcStart;
    offsetsSet.emplace(succOff);
  }

  if (op == Op::Await || op == Op::Throw) {
    auto const target = findCatchHandler(func, opc - bcStart);
    if (target != kInvalidOffset) offsetsSet.emplace(target);
  }

  return offsetsSet;
}

/**
 * Return the number of successor-edges including fall-through paths but not
 * implicit exception paths.
 */
int numSuccs(const PC origPC) {
  auto pc = origPC;
  auto numTargets = instrJumpOffsets(pc).size();
  pc = origPC;
  if ((instrFlags(decode_op(pc)) & TF) == 0) ++numTargets;
  return numTargets;
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
#define SIX(...) 6
#define MFINAL -3
#define C_MFINAL(n) -10 - (n)
#define CUMANY -3
#define CMANY_U3 C_MFINAL(3)
#define CALLNATIVE -5
#define FCALL(nin, nobj) -20 - (nin)
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
#undef SIX
#undef MFINAL
#undef C_MFINAL
#undef CUMANY
#undef CMANY_U3
#undef CALLNATIVE
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
  // NewPackedArray and some final member operations specify how
  // many values are popped in their first immediate
  if (n == -3) return getImm(pc, 0).u_IVA;
  // FCallBuiltin pops numArgs and numOut uninit values
  if (n == -5) {
    return getImm(pc, 0).u_IVA + getImm(pc, 2).u_IVA;
  }
  // FCall* opcodes pop number of opcode specific inputs, unpack, numArgs,
  // 3 cells/uninits reserved for ActRec and (numRets - 1) uninit values.
  if (n <= -20) {
    auto const fca = getImm(pc, 0).u_FCA;
    auto const nin = -n - 20;
    return nin + fca.numInputs() + 2 + fca.numRets;
  }
  // Other final member operations pop their first immediate + n
  if (n <= -10) return getImm(pc, 0).u_IVA - n - 10;

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
#define SIX(...) 6
#define CMANY -2
#define FCALL -1
#define CALLNATIVE -3
#define O(name, imm, pop, push, flags) push,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef FIVE
#undef SIX
#undef CMANY
#undef FCALL
#undef CALLNATIVE
#undef O
  };
  auto const op = peek_op(pc);
  int n = numberOfPushes[size_t(op)];

  // FCallBuiltin pushes numOut + 1 return values
  if (n == -3) return getImm(pc, 2).u_IVA + 1;
  // The PopFrame opcode push all arguments onto the stack
  if (n == -2) return getImm(pc, 0).u_IVA;
  // The FCall* opcodes pushes all return values onto the stack
  if (n == -1) return getImm(pc, 0).u_FCA.numRets;

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

template<int nin, int nobj>
FlavorDesc fcallFlavor(PC op, uint32_t i) {
  always_assert(i < uint32_t(instrNumPops(op)));
  auto const fca = getImm(op, 0).u_FCA;
  if (i < nin) return CV;
  i -= nin;
  if (i == 0 && fca.hasGenerics()) return CV;
  i -= fca.hasGenerics() ? 1 : 0;
  if (i == 0 && fca.hasUnpack()) return CV;
  i -= fca.hasUnpack() ? 1 : 0;
  if (i < fca.numArgs) return CV;
  i -= fca.numArgs;
  if (i == 2 && nobj) return CV;
  return UV;
}

FlavorDesc fcallBuiltinFlavor(PC op, uint32_t i) {
  assertx(i < getImm(op, 0).u_IVA + getImm(op, 2).u_IVA);
  auto const nargs = getImm(op, 0).u_IVA;
  if (i < nargs) return CUV;
  return UV;
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
#define SIX(f1, f2, f3, f4, f5, f6) return doFlavor(idx, f1, f2, f3, f4, f5, f6);
#define MFINAL return manyFlavor(op, idx, CV);
#define C_MFINAL(n) return manyFlavor(op, idx, CV);
#define CUMANY return manyFlavor(op, idx, CUV);
#define CMANY_U3 return manyFlavor(op, idx, CUV);
#define CALLNATIVE return fcallBuiltinFlavor(op, idx);
#define FCALL(nin, nobj) return fcallFlavor<nin, nobj>(op, idx);
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
#undef SIX
#undef MFINAL
#undef C_MFINAL
#undef CUMANY
#undef CMANY_U3
#undef CALLNATIVE
#undef FCALL
#undef CMANY
#undef SMANY
#undef O
}

void staticArrayStreamer(const ArrayData* ad, std::string& out) {
  if (ad->isLegacyArray()) out += "legacy_";

  if (ad->isVecType()) out += "vec(";
  else if (ad->isDictType()) out += "dict(";
  else if (ad->isKeysetType()) out += "keyset(";
  else {
    assertx(ad->isPHPArrayType());
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

      if (!ad->isVecType() && !ad->isKeysetType()) {
        staticStreamer(key.asTypedValue(), out);
        out += "=>";
      }

      Variant val = it.second();

      staticStreamer(val.asTypedValue(), out);
    }
  }
  out += ")";

  if (RO::EvalArrayProvenance) {
    auto const tag = arrprov::getTag(ad);
    if (tag.valid()) {
      out += " [";
      out += tag.toString();
      out += "]";
    }
  }
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
    case KindOfLazyClass:
      out += tv->m_data.plazyclass.name()->data();
      return;
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      staticArrayStreamer(tv->m_data.parr, out);
      return;
    case KindOfClsMeth:
    case KindOfRClsMeth:
    case KindOfObject:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfFunc:
    case KindOfClass:
    case KindOfRecord:
      break;
  }
  not_reached();
}

std::string instrToString(PC it, Either<const Func*, const FuncEmitter*> f) {
  std::string out;
  PC iStart = it;
  Op op = decode_op(it);

  auto u = f.match(
    [](const Func* f) -> Either<const Unit*, const UnitEmitter*> { return f->unit(); },
    [](const FuncEmitter* fe) -> Either<const Unit*, const UnitEmitter*> { return &fe->ue(); }
  );

  auto readRATA = [&] {
    if (auto func = f.left()) {
      auto const unit = func->unit();
      auto const rat = decodeRAT(unit, it);
      folly::format(&out, " {}", show(rat));
      return;
    }

    auto const pc = it;
    it += encodedRATSize(pc);
    out += " <RepoAuthType>";
  };

  auto offsetOf = [f](PC pc) {
    return f.match(
      [pc](const Func* f) { return f->offsetOf(pc); },
      [pc](const FuncEmitter* fe) { return fe->offsetOf(pc); }
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

  auto printLocal = [](int32_t local) {
    return folly::to<std::string>(local);
  };

  auto showOffset = [&](Offset offset) {
    if (f == nullptr) return folly::sformat("{}", offset);
    auto const unitOff = offsetOf(iStart + offset);
    return folly::sformat("{} ({})", offset, unitOff);
  };

  switch (op) {

#define READ(t) folly::format(&out, " {}", *((t*)&*it)); it += sizeof(t)

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

#define ONE(a) H_##a
#define TWO(a, b) H_##a; H_##b
#define THREE(a, b, c) H_##a; H_##b; H_##c;
#define FOUR(a, b, c, d) H_##a; H_##b; H_##c; H_##d;
#define FIVE(a, b, c, d, e) H_##a; H_##b; H_##c; H_##d; H_##e;
#define SIX(a, b, c, d, e, f) H_##a; H_##b; H_##c; H_##d; H_##e; H_##f;
#define NA
#define H_BLA READSVEC()
#define H_SLA READSVEC()
#define H_IVA READIVA()
#define H_I64A READ(int64_t)
#define H_LA READLA()
#define H_NLA do {    \
  auto loc = decode_named_local(it);                 \
  folly::format(&out, " L:{}:{}", loc.name, loc.id); \
} while (false)
#define H_ILA READLA()
#define H_IA READV()
#define H_DA READ(double)
#define H_BA (out += ' ', out += showOffset(decode_ba(it)))
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
#define H_ITA (out += ' ', out += show(decodeIterArgs(it), printLocal))
#define H_FCA do {                                               \
  auto const fca = decodeFCallArgs(thisOpcode, it, u);           \
  auto const aeOffset = fca.asyncEagerOffset != kInvalidOffset   \
    ? showOffset(fca.asyncEagerOffset)                           \
    : "-";                                                       \
  out += ' ';                                                    \
  out += show(fca, fca.inoutArgs, aeOffset, fca.context);        \
} while (false)

#define O(name, imm, push, pop, flags)       \
  case Op##name: {                           \
    out += #name;                            \
    UNUSED auto const thisOpcode = Op##name; \
    UNUSED unsigned immIdx = 0;              \
    imm;                                     \
    break;                                   \
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
#undef SIX
#undef NA
#undef H_BLA
#undef H_SLA
#undef H_IVA
#undef H_I64A
#undef H_LA
#undef H_NLA
#undef H_ILA
#undef H_IA
#undef H_DA
#undef H_BA
#undef H_OA
#undef H_SA
#undef H_AA
#undef H_VSA
#undef H_KA
#undef H_LAR
#undef H_ITA
#undef H_FCA
    default: assertx(false);
  };
  return out;
}

EXTERNALLY_VISIBLE
extern const char* const opcodeToNameTable[] = {
#define O(name, imm, inputs, outputs, flags) \
    #name ,
    OPCODES
#undef O
};

const char* opcodeToName(Op op) {
  if (size_t(op) < Op_count) {
    return opcodeToNameTable[size_t(op)];
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

static const char* SetRangeOp_names[] = {
#define OP(x) #x,
  SET_RANGE_OPS
#undef OP
};

static const char* TypeStructResolveOp_names[] = {
#define OP(x) #x,
  TYPE_STRUCT_RESOLVE_OPS
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

static const char* IsLogAsDynamicCallOp_names[] = {
#define IS_LOG_AS_DYNAMIC_CALL_OP(x) #x,
  IS_LOG_AS_DYNAMIC_CALL_OPS
#undef IS_LOG_AS_DYNAMIC_CALL_OP
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
X(SetRangeOp,     static_cast<int>(SetRangeOp::Forward))
X(TypeStructResolveOp,
                  static_cast<int>(TypeStructResolveOp::Resolve))
X(MOpMode,        static_cast<int>(MOpMode::None))
X(ContCheckOp,    static_cast<int>(ContCheckOp::IgnoreStarted))
X(CudOp,          static_cast<int>(CudOp::IgnoreIter))
X(SpecialClsRef,  static_cast<int>(SpecialClsRef::Self))
X(IsLogAsDynamicCallOp,
                  static_cast<int>(IsLogAsDynamicCallOp::LogAsDynamicCall))
#undef X

//////////////////////////////////////////////////////////////////////

namespace {

bool instrIsVMCall(Op opcode) {
  if (isFCall(opcode)) return true;
  switch (opcode) {
    case OpContEnter:
    case OpContRaise:
    case OpEval:
    case OpIncl:
    case OpInclOnce:
    case OpReq:
    case OpReqDoc:
    case OpReqOnce:
      return true;

    default:
      return false;
  }
}

bool instrMayVMCall(Op opcode) {
  return instrIsVMCall(opcode) || opcode == OpIdx;
}

}

bool instrIsNonCallControlFlow(Op opcode) {
  if (!instrIsControlFlow(opcode) || instrIsVMCall(opcode)) return false;

  switch (opcode) {
    case OpAwait:
    case OpAwaitAll:
    case OpYield:
    case OpYieldK:
    case OpFCallBuiltin:
      return false;

    default:
      return true;
  }
}

bool instrAllowsFallThru(Op opcode) {
  InstrFlags opFlags = instrFlags(opcode);
  return (opFlags & TF) == 0;
}

PC skipCall(PC callPC) {
  assertx(instrMayVMCall(peek_op(callPC)));
  return callPC + instrLen(callPC);
}

ImmVector getImmVector(PC opcode) {
  auto const op = peek_op(opcode);
  int numImm = numImmediates(op);
  for (int k = 0; k < numImm; ++k) {
    ArgType t = immType(op, k);
    if (t == BLA || t == SLA || t == VSA) {
      PC vp = getImmPtr(opcode, k)->bytes;
      auto const size = decode_iva(vp);
      return ImmVector(vp, size, t == VSA ? size : 0);
    }
  }

  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

std::string show(const IterArgs& ita, PrintLocal print_local) {
  auto const flags = [&]{
    auto parts = std::vector<std::string>{};
    if (ita.flags & IterArgs::Flags::BaseConst) parts.push_back("BaseConst");
    if (parts.empty()) return std::string{};
    return folly::sformat("<{}> ", folly::join(' ', parts));
  }();

  auto const key = ita.hasKey()
    ? folly::to<std::string>("K:", print_local(ita.keyId))
    : folly::to<std::string>("NK");
  auto const val = "V:" + print_local(ita.valId);
  return folly::sformat("{}{} {} {}", flags, ita.iterId, key, val);
}

std::string show(const LocalRange& range) {
  return folly::sformat(
    "L:{}+{}", range.first, range.count
  );
}

std::string show(const FCallArgsBase& fca, const uint8_t* inoutArgsRaw,
                 std::string asyncEagerLabel, const StringData* ctx) {
  auto const inoutArgs = [&] {
    if (!inoutArgsRaw) return std::string{"\"\""};
    std::string out = "\"";
    uint8_t tmp = 0;
    for (int i = 0; i < fca.numArgs; ++i) {
      if (i % 8 == 0) tmp = *(inoutArgsRaw++);
      out += ((tmp >> (i % 8)) & 1) ? "1" : "0";
    }
    out += "\"";
    return out;
  }();

  std::vector<std::string> flags;
  if (fca.hasUnpack()) flags.push_back("Unpack");
  if (fca.hasGenerics()) flags.push_back("Generics");
  if (fca.lockWhileUnwinding()) flags.push_back("LockWhileUnwinding");
  if (fca.skipRepack()) flags.push_back("SkipRepack");
  return folly::sformat(
    "<{}> {} {} {} {} \"{}\"",
    folly::join(' ', flags), fca.numArgs, fca.numRets, inoutArgs,
    asyncEagerLabel, ctx ? ctx->data() : ""
  );
}

///////////////////////////////////////////////////////////////////////////////
}

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/ext_variable.h>
#include <runtime/vm/hhbc.h>
#include <runtime/vm/unit.h>
#include <sstream>

namespace HPHP {
namespace VM {
///////////////////////////////////////////////////////////////////////////////

bool isValidOpcode(Opcode op) {
  return op > OpLowInvalid && op < OpHighInvalid;
}

int numImmediates(Opcode opcode) {
  ASSERT(isValidOpcode(opcode));
  static const int8_t values[] = {
#define NA         0
#define ONE(...)   1
#define TWO(...)   2
#define THREE(...) 3
#define O(name, imm, unusedPop, unusedPush, unusedFlags) imm,
    OPCODES
#undef O
#undef NA
#undef ONE
#undef TWO
#undef THREE
  };
  return values[opcode];
}

ArgType immType(const Opcode opcode, int idx) {
  ASSERT(isValidOpcode(opcode));
  ASSERT(idx >= 0 && idx < numImmediates(opcode));
  assert(idx < 3); // No opcodes have more than three immediates
  static const int8_t arg0Types[] = {
#define NA -1,
#define ONE(a) a,
#define TWO(a, b) a,
#define THREE(a, b, c) a,
#define O(name, imm, unusedPop, unusedPush, unusedFlags) imm
    OPCODES
// re-using definition of O below.
#undef NA
#undef ONE
#undef TWO
#undef THREE
  };
  static const int8_t arg1Types[] = {
#define NA -1,
#define ONE(a) -1,
#define TWO(a, b) b,
#define THREE(a, b, c) b,
    OPCODES
// re-using definition of O below.
#undef NA
#undef ONE
#undef TWO
#undef THREE
  };
  static const int8_t arg2Types[] = {
#define NA -1,
#define ONE(a) -1,
#define TWO(a, b) -1,
#define THREE(a, b, c) c,
    OPCODES
#undef O
#undef NA
#undef ONE
#undef TWO
#undef THREE
  };
  switch (idx) {
    case 0: return (ArgType)arg0Types[opcode];
    case 1: return (ArgType)arg1Types[opcode];
    case 2: return (ArgType)arg2Types[opcode];
    default: ASSERT(false); return (ArgType)-1;
  }
}

int immSize(const Opcode* opcode, int idx) {
  ASSERT(idx >= 0 && idx < numImmediates(*opcode));
  assert(idx < 3); // No opcodes have more than three immediates
  static const int8_t argTypeToSizes[] = {
#define ARGTYPE(nm, type) sizeof(type),
#define ARGTYPEVEC(nm, type) 0,
    ARGTYPES
#undef ARGTYPE
#undef ARGTYPEVEC
  };
  if (immType(*opcode, idx) == IVA || immType(*opcode, idx) == HA ||
      immType(*opcode, idx) == IA) {
    intptr_t offset = 1;
    if (idx >= 1) offset += immSize(opcode, 0);
    if (idx >= 2) offset += immSize(opcode, 1);
    // variable size
    unsigned char imm = *(unsigned char*)(opcode + offset);
    // Low order bit set => 4-byte.
    return (imm & 0x1 ? sizeof(int32_t) : sizeof(unsigned char));
  } else if (immIsVector(*opcode, idx)) {
    intptr_t offset = 1;
    if (idx >= 1) offset += immSize(opcode, 0);
    if (idx >= 2) offset += immSize(opcode, 1);
    int prefixes, vecElemSz;
    if (immType(*opcode, idx) == MA) {
      prefixes = 2;
      vecElemSz = sizeof(uint8_t);
    } else {
      ASSERT(immType(*opcode, idx) == ILA);
      prefixes = 1;
      vecElemSz = sizeof(int32_t);
    }
    return prefixes * sizeof(int32_t) +
      vecElemSz * *(int32_t*)((int8_t*)opcode + offset);
  } else {
    ArgType type = immType(*opcode, idx);
    return (type >= 0) ? argTypeToSizes[type] : 0;
  }
}

bool immIsVector(Opcode opcode, int idx) {
  ArgType type = immType(opcode, idx);
  return (type == MA || type == ILA);
}

bool hasImmVector(Opcode opcode) {
  const int num = numImmediates(opcode);
  for (int i = 0; i < num; ++i) {
    if (immIsVector(opcode, i)) return true;
  }
  return false;
}

ArgUnion getImm(const Opcode* opcode, int idx) {
  const Opcode* p = opcode + 1;
  ASSERT(idx >= 0 && idx < numImmediates(*opcode));
  ArgUnion retval;
  retval.u_NA = 0;
  int cursor = 0;
  for (cursor = 0; cursor < idx; cursor++) {
    // Advance over this immediate.
    p += immSize(opcode, cursor);
  }
  assert(cursor == idx);
  ArgType type = immType(*opcode, idx);
  if (type == IVA || type == HA || type == IA) {
    retval.u_IVA = decodeVariableSizeImm(&p);
  } else if (!immIsVector(*opcode, cursor)) {
    memcpy(&retval.bytes, p, immSize(opcode, idx));
  }
  assert(numImmediates(*opcode) > idx);
  return retval;
}

ArgUnion* getImmPtr(const Opcode* opcode, int idx) {
  ASSERT(immType(*opcode, idx) != IVA);
  ASSERT(immType(*opcode, idx) != HA);
  ASSERT(immType(*opcode, idx) != IA);
  const Opcode* ptr = opcode + 1;
  for (int i = 0; i < idx; i++) {
    ptr += immSize(opcode, i);
  }
  return (ArgUnion*)ptr;
}

// TODO: merge with emitIVA in unit.h
size_t encodeVariableSizeImm(int32_t n, unsigned char* buf) {
  if (LIKELY((n & 0x7f) == n)) {
    *buf = static_cast<unsigned char>(n) << 1;
    return 1;
  }
  ASSERT((n & 0x7fffffff) == n);
  *reinterpret_cast<uint32_t*>(buf) = (uint32_t(n) << 1) | 0x1;
  return 4;
}

void encodeIvaToVector(std::vector<uchar>& out, int32_t val) {
  size_t currentLen = out.size();
  out.resize(out.size() + 4);
  out.resize(currentLen + encodeVariableSizeImm(val, &out[currentLen]));
}

int instrLen(const Opcode* opcode) {
  int len = 1;
  int nImm = numImmediates(*opcode);
  for (int i = 0; i < nImm; i++) {
    len += immSize(opcode, i);
  }
  return len;
}

InstrFlags instrFlags(Opcode opcode) {
  static const InstrFlags instrFlagsData[] = {
#define O(unusedName, unusedImm, unusedPop, unusedPush, flags) flags,
    OPCODES
#undef O
  };
  return instrFlagsData[opcode];
}

Offset* instrJumpOffset(Opcode* instr) {
  static const int8_t jumpMask[] = {
#define NA 0
#define MA 0
#define IVA 0
#define I64A 0
#define DA 0
#define SA 0
#define AA 0
#define BA 1
#define HA 0
#define IA 0
#define OA 0
#define ONE(a) a
#define TWO(a, b) (a + 2 * b)
#define THREE(a, b, c) (a + 2 * b + 4 * c)
#define O(name, imm, pop, push, flags) imm,
    OPCODES
#undef NA
#undef MA
#undef IVA
#undef I64A
#undef DA
#undef SA
#undef AA
#undef HA
#undef IA
#undef BA
#undef OA
#undef ONE
#undef TWO
#undef THREE
#undef O
  };

  ASSERT(*instr != OpSwitch);
  int mask = jumpMask[*instr];
  if (mask == 0) {
    return NULL;
  }
  int immNum;
  switch (mask) {
    case 0: return NULL;
    case 1: immNum = 0; break;
    case 2: immNum = 1; break;
    case 4: immNum = 2; break;
    default: ASSERT(false); return NULL;
  }

  return &getImmPtr(instr, immNum)->u_BA;
}

Offset instrJumpTarget(const Opcode* instrs, Offset pos) {
  Offset* offset = instrJumpOffset(const_cast<Opcode*>(instrs + pos));

  if (!offset) {
    return InvalidAbsoluteOffset;
  } else {
    return *offset + pos;
  }
}

/**
 * Return the number of successor-edges including fall-through paths but not
 * implicit exception paths.
 */
int numSuccs(const Opcode* instr) {
  if (!instrIsControlFlow(*instr)) return 1;
  if ((instrFlags(*instr) & TF) != 0) {
    if (Op(*instr) == OpSwitch) return *(int*)(instr + 1);
    if (Op(*instr) == OpJmp) return 1;
    return 0;
  }
  if (instrJumpOffset(const_cast<Opcode*>(instr))) return 2;
  return 1;
}

/**
 * instrNumPops() returns the number of values consumed from the stack
 * for a given push/pop instruction. For peek/poke instructions, this
 * function returns 0.
 */
int instrNumPops(const Opcode* opcode) {
  static const int8_t numberOfPops[] = {
#define NOV 0
#define ONE(...) 1
#define TWO(...) 2
#define THREE(...) 3
#define LMANY(...) -1
#define C_LMANY(...) -2
#define V_LMANY(...) -2
#define FMANY -3
#define O(name, imm, pop, push, flags) pop,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef LMANY
#undef C_LMANY
#undef V_LMANY
#undef FMANY
#undef O
  };
  int n = numberOfPops[*opcode];
  // For most instructions, we know how many values are popped based
  // solely on the opcode
  if (n >= 0) return n;
  // FCall specifies how many values are popped in its first immediate
  if (n == -3) return getImm(opcode, 0).u_IVA;
  // For instructions with vector immediates, we have to scan the
  // contents of the vector immediate to determine how many values
  // are popped
  ASSERT(n == -1 || n == -2);
  ImmVector iv = getImmVector(opcode);
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
int instrNumPushes(const Opcode* opcode) {
  static const int8_t numberOfPushes[] = {
#define NOV 0
#define ONE(...) 1
#define TWO(...) 2
#define THREE(...) 3
#define INS_1(...) 0
#define INS_2(...) 0
#define O(name, imm, pop, push, flags) push,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef INS_1
#undef INS_2
#undef O
  };
  return numberOfPushes[*opcode];
}

StackTransInfo instrStackTransInfo(const Opcode* opcode) {
  static const StackTransInfo::Kind transKind[] = {
#define NOV StackTransInfo::PushPop
#define ONE(...) StackTransInfo::PushPop
#define TWO(...) StackTransInfo::PushPop
#define THREE(...) StackTransInfo::PushPop
#define INS_1(...) StackTransInfo::InsertMid
#define INS_2(...) StackTransInfo::InsertMid
#define O(name, imm, pop, push, flags) push,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef INS_1
#undef INS_2
#undef O
  };
  static const int8_t peekPokeType[] = {
#define NOV -1
#define ONE(...) -1
#define TWO(...) -1
#define THREE(...) -1
#define INS_1(...) 0
#define INS_2(...) 1
#define O(name, imm, pop, push, flags) push,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef INS_2
#undef INS_1
#undef O
  };
  StackTransInfo ret;
  ret.kind = transKind[*opcode];
  switch (ret.kind) {
  case StackTransInfo::PushPop:
    ret.pos = 0;
    ret.numPushes = instrNumPushes(opcode);
    ret.numPops = instrNumPops(opcode);
    return ret;
  case StackTransInfo::InsertMid:
    ret.numPops = 0;
    ret.numPushes = 0;
    ret.pos = peekPokeType[*opcode];
    return ret;
  default:
    NOT_REACHED();
  }
}

static void staticArrayStreamer(ArrayData* ad, std::stringstream& out) {
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
      switch (key.getType()) {
      case KindOfInt64: {
        out << *key.getInt64Data();
        break;
      }
      case KindOfStaticString:
      case KindOfString: {
        out << "\""
            << Util::escapeStringForCPP(key.getStringData()->data(),
                                        key.getStringData()->size())
            << "\"";
        break;
      }
      default: ASSERT(false);
      }
      out << "=>";
      // Value.
      Variant val = it.second();
      switch (val.getType()) {
      case KindOfUninit:
      case KindOfNull: {
        out << "null";
        break;
      }
      case KindOfBoolean: {
        out << (val.toBoolean() ? "true" : "false");
        break;
      }
      case KindOfStaticString:
      case KindOfString: {
        out << "\""
            << Util::escapeStringForCPP(val.getStringData()->data(),
                                        val.getStringData()->size())
            << "\"";
        break;
      }
      case KindOfInt64: {
        out << *val.getInt64Data();
        break;
      }
      case KindOfDouble: {
        out << *val.getDoubleData();
        break;
      }
      case KindOfArray: {
        staticArrayStreamer(val.getArrayData(), out);
        break;
      }
      default: ASSERT(false);
      }
    }
  }
  out << ")";
}

void staticStreamer(TypedValue* tv, std::stringstream& out) {
  switch (tv->m_type) {
  case KindOfUninit:
  case KindOfNull: {
    out << "null";
    break;
  }
  case KindOfBoolean: {
    out << (tv->m_data.num ? "true" : "false");
    break;
  }
  case KindOfStaticString:
  case KindOfString: {
    out << "\"" << tv->m_data.pstr->data() << "\"";
    break;
  }
  case KindOfInt64: {
    out << tv->m_data.num;
    break;
  }
  case KindOfDouble: {
    out << tv->m_data.dbl;
    break;
  }
  case KindOfArray: {
    staticArrayStreamer(tv->m_data.parr, out);
    break;
  }
  default: ASSERT(false);
  }
}

const char* const locationNames[] = { "L", "C",
                                      "GL", "GC",
                                      "NL", "NC",
                                      "SL", "SC",
                                      "R" };
const size_t locationNamesCount = sizeof(locationNames) /
                                  sizeof(*locationNames);
static_assert(locationNamesCount == NumLocationCodes,
              "Location code missing for locationCodeString");

const char* locationCodeString(LocationCode lcode) {
  ASSERT(lcode >= 0 && lcode < NumLocationCodes);
  return locationNames[lcode];
}

LocationCode parseLocationCode(const char* s) {
  if (!*s) return InvalidLocationCode;

  switch (*s) {
  case 'L':   return LL;
  case 'C':   return LC;
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

const char* const memberNames[] = { "EC", "PC", "EL", "PL", "W" };
const size_t memberNamesCount = sizeof(memberNames) /
                                sizeof(*memberNames);

static_assert(memberNamesCount == NumMemberCodes,
             "Member code missing for memberCodeString");

const char* memberCodeString(MemberCode mcode) {
  ASSERT(mcode >= 0 && mcode < NumMemberCodes);
  return memberNames[mcode];
}

MemberCode parseMemberCode(const char* s) {
  int incr;
  switch (*s) {
  case 'W': return MW;
  case 'E': incr = 0; break;
  case 'P': incr = 1; break;
  default:  return InvalidMemberCode;
  }
  switch (s[1]) {
  case 'C': return MemberCode(MEC + incr);
  case 'L': return MemberCode(MEL + incr);
  default:  return InvalidMemberCode;
  }
}

std::string instrToString(const Opcode* it, const Unit* u /* = NULL */) {
  // IncDec names
  static const char* incdecNames[] = {
    "PreInc", "PostInc", "PreDec", "PostDec"
  };
  static const int incdecNamesCount =
    (int)(sizeof(incdecNames)/sizeof(const char*));
  // SetOp names
  static const char* setopNames[] = {
#define SETOP_OP(setOpOp, bcOp) #bcOp,
    SETOP_OPS
#undef SETOP_OP
  };
  static const int setopNamesCount =
    (int)(sizeof(setopNames)/sizeof(const char*));

  std::stringstream out;
  const Opcode* iStart = it;
  Op op = (Op)*it;
  ++it;
  switch (op) {

#define READ(t) out << " " << *((t*)&*it); it += sizeof(t)

#define READOFF() do {                                              \
  Offset _value = *(Offset*)it;                                     \
  out << " " << _value;                                             \
  if (u != NULL) {                                                  \
    out << " (" << u->offsetOf(iStart + _value) << ")";             \
  }                                                                 \
  it += sizeof(Offset);                                             \
} while (false)

#define READV() out << " " << decodeVariableSizeImm(&it);

#define READOA() do {                                             \
  int immVal = (int)*((uchar*)&*it);                              \
  it += sizeof(uchar);                                            \
  out << " ";                                                     \
  switch (op) {                                                   \
  case OpIncDecL: case OpIncDecN: case OpIncDecG: case OpIncDecS: \
  case OpIncDecM:                                                 \
    out << ((immVal >= 0 && immVal < incdecNamesCount) ?          \
            incdecNames[immVal] : "?");                           \
    break;                                                        \
  case OpSetOpL: case OpSetOpN: case OpSetOpG: case OpSetOpS:     \
  case OpSetOpM:                                                  \
    out << ((immVal >=0 && immVal < setopNamesCount) ?            \
            setopNames[immVal] : "?");                            \
    break;                                                        \
  default:                                                        \
    out << immVal;                                                \
    break;                                                        \
  }                                                               \
} while (false)

#define READVEC() do {                                                  \
  int sz = *((int*)&*it);                                               \
  it += sizeof(int) * 2;                                                \
  const uint8_t* const start = it;                                      \
  out << " <";                                                          \
  if (sz > 0) {                                                         \
    int immVal = (int)*((uchar*)&*it);                                  \
    out << ((immVal >=0 && size_t(immVal) < locationNamesCount) ?       \
            locationCodeString(LocationCode(immVal)) : "?");            \
    it += sizeof(uchar);                                                \
    int numLocImms = numLocationCodeImms(LocationCode(immVal));         \
    for (int i = 0; i < numLocImms; ++i) {                              \
      out << ':' << decodeVariableSizeImm(&it);                         \
    }                                                                   \
    while (reinterpret_cast<const uint8_t*>(it) - start < sz) {         \
      immVal = (int)*((uchar*)&*it);                                    \
      out << " " << ((immVal >=0 && size_t(immVal) < memberNamesCount) ? \
                     memberCodeString(MemberCode(immVal)) : "?");       \
      it += sizeof(uchar);                                              \
      if (memberCodeHasImm(MemberCode(immVal))) {                       \
        out << ':' << decodeVariableSizeImm(&it);                       \
      }                                                                 \
    }                                                                   \
    ASSERT(reinterpret_cast<const uint8_t*>(it) - start == sz);         \
  }                                                                     \
  out << ">";                                                           \
} while (false)

#define READIVEC() do {                     \
  int sz = *(int*)it;                       \
  it += sizeof(int);                        \
  out << " <";                              \
  std::string sep;                          \
  for (int i = 0; i < sz; ++i) {            \
    Offset o = *(Offset*)it;                \
    it += sizeof(Offset);                   \
    out << sep;                             \
    if (u != NULL) {                        \
      out << u->offsetOf(iStart + o);       \
    } else {                                \
      out << o;                             \
    }                                       \
    sep = " ";                              \
  }                                         \
  out << ">";                               \
} while (false)
#define ONE(a) H_##a
#define TWO(a, b) H_##a; H_##b
#define THREE(a, b, c) H_##a; H_##b; H_##c;
#define NA
#define H_MA READVEC()
#define H_ILA READIVEC()
#define H_IVA READV()
#define H_I64A READ(int64)
#define H_HA READV()
#define H_IA READV()
#define H_DA READ(double)
#define H_BA READOFF()
#define H_OA READOA()
#define H_SA                                                      \
  if (u) {                                                        \
    const StringData* sd = u->lookupLitstrId(*((Id*)it));         \
    out << " \"" <<                                               \
      Util::escapeStringForCPP(sd->data(), sd->size()) << "\"";   \
  } else {                                                        \
    out << " " << *((Id*)it);                                     \
  }                                                               \
  it += sizeof(Id)
#define H_AA                                                  \
  if (u) {                                                    \
    out << " ";                                               \
    staticArrayStreamer(u->lookupArrayId(*((Id*)it)), out);   \
  } else {                                                    \
    out << " " << *((Id*)it);                                 \
  }                                                           \
  it += sizeof(Id)
#define O(name, imm, push, pop, flags)    \
  case Op##name: {                        \
    out << #name;                         \
    imm;                                  \
    break;                                \
  }
OPCODES
#undef O
#undef READ
#undef ONE
#undef TWO
#undef THREE
#undef NA
#undef H_MA
#undef H_IVA
#undef H_I64A
#undef H_HA
#undef H_IA
#undef H_DA
#undef H_BA
#undef H_OA
#undef H_SA
#undef H_AA
    default: ASSERT(false);
  };
  return out.str();
}

std::string opcodeToName(Opcode op) {
  const char* namesArr[] = {
#define O(name, imm, inputs, outputs, flags) \
    #name ,
    OPCODES
#undef O
    "Invalid"
  };
  if (op >= 0 && op < sizeof namesArr / sizeof *namesArr) {
    return namesArr[op];
  }
  return "Invalid";
}

bool instrIsControlFlow(Opcode opcode) {
  InstrFlags opFlags = instrFlags(opcode);
  return (opFlags & CF) != 0;
}

bool instrReadsCurrentFpi(Opcode opcode) {
  InstrFlags opFlags = instrFlags(opcode);
  return (opFlags & FF) != 0;
}

ImmVector getImmVector(const Opcode* opcode) {
  int numImm = numImmediates(*opcode);
  for (int k = 0; k < numImm; ++k) {
    ArgType t = immType(*opcode, k);
    if (t == MA) {
      void* vp = getImmPtr(opcode, k);
      return ImmVector::createFromStream(
        static_cast<const uint8_t*>(vp));
    } else if (t == ILA) {
      void* vp = getImmPtr(opcode, k);
      return ImmVector::createFromStream(
        static_cast<const int32_t*>(vp));
    }
  }

  NOT_REACHED();
}

const uint8_t* ImmVector::findLastMember() const {
  ASSERT(m_length > 0);

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
      decodeVariableSizeImm(&vec);
    }
    if (vec - m_start == m_length) {
      return ret;
    }
    ASSERT(vec - m_start < m_length);
  }

  NOT_REACHED();
}

int instrSpToArDelta(const Opcode* opcode) {
  // This function should only be called for instructions that read
  // the current FPI
  ASSERT(instrReadsCurrentFpi(*opcode));
  // The delta from sp to ar is equal to the number of values on the stack
  // that will be consumed by this instruction (numPops) plus the number of
  // parameters pushed onto the stack so far that are not being consumed by
  // this instruction (numExtra). For the FPass* instructions, numExtra will
  // be equal to the first immediate argument (param id). For the FCall
  // instruction, numExtra will be 0 because all of the parameters on the
  // stack are already accounted for by numPops.
  int numPops = instrNumPops(opcode);
  int numExtra = (*opcode != OpFCall) ? getImm(opcode, 0).u_IVA : 0;
  int delta = numPops + numExtra;
  return delta;
}

///////////////////////////////////////////////////////////////////////////////
}
}


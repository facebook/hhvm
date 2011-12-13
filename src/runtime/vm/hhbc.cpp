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

int immType(const Opcode* opcode, int idx) {
  ASSERT(isValidOpcode(*opcode));
  ASSERT(idx >= 0 && idx < numImmediates(*opcode));
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
    case 0: return arg0Types[*opcode];
    case 1: return arg1Types[*opcode];
    case 2: return arg2Types[*opcode];
    default: ASSERT(false); return -1;
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
  int retval;
  if (immType(opcode, idx) == IVA) {
    // variable size
    unsigned char imm = *(unsigned char*)(opcode + 1);
    // Low order bit set => 4-byte.
    return (imm & 0x1 ? sizeof(int32_t) : sizeof(unsigned char));
  } else if (immIsVector(opcode, idx)) {
    intptr_t offset = 1;
    if (idx >= 1) offset += immSize(opcode, 0);
    if (idx >= 2) offset += immSize(opcode, 1);
    retval = 4 + *(int*)((int8_t*)opcode + offset);
  } else {
    int type = immType(opcode, idx);
    retval = (type >= 0) ? argTypeToSizes[type] : 0;
  }
  return retval;
}

bool immIsVector(const Opcode* opcode, int idx) {
  int type = immType(opcode, idx);
  return (type == LA);
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
  if (immType(opcode, idx) == IVA) {
    retval.u_IVA = decodeVariableSizeImm((unsigned char**)&p);
  } else {
    memcpy(&retval.bytes, p, immSize(opcode, idx));
  }
  assert(numImmediates(*opcode) > idx);
  return retval;
}

ArgUnion* getImmPtr(const Opcode* opcode, int idx) {
  ASSERT(immType(opcode, idx) != IVA);
  const Opcode* ptr = opcode + 1;
  for (int i = 0; i < idx; i++) {
    ptr += immSize(opcode, i);
  }
  return (ArgUnion*)ptr;
}

ImmVector* getImmVector(const Opcode* opcode) {
  int numImm = numImmediates(*opcode);
  for (int k = 0; k < numImm; ++k) {
    if (immIsVector(opcode, k)) {
      return (ImmVector*)getImmPtr(opcode, k);
    }
  }
  return NULL;
}

int32 decodeVariableSizeImm(unsigned char** immPtr) {
  // unsigned so shifts are logical
  unsigned char small = **immPtr;
  if (UNLIKELY(small & 0x1)) {
    unsigned int large = *((unsigned int*)*immPtr);
    *immPtr += sizeof(large);
    return (int32)(large >> 1);
  } else {
    *immPtr += sizeof(small);
    return (int32)(small >> 1);
  }
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
#define LA 0
#define IVA 0
#define I64A 0
#define DA 0
#define SA 0
#define AA 0
#define BA 1
#define OA 0
#define ONE(a) a
#define TWO(a, b) (a + 2 * b)
#define THREE(a, b, c) (a + 2 * b + 4 * c)
#define O(name, imm, pop, push, flags) imm,
    OPCODES
#undef NA
#undef LA
#undef IVA
#undef I64A
#undef DA
#undef SA
#undef AA
#undef BA
#undef OA
#undef ONE
#undef TWO
#undef THREE
#undef O
  };

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
#define POS_1(...) 0
#define POS_N(...) 0
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
#undef POS_1
#undef POS_N
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
  ImmVector* iv = (ImmVector*)getImmVector(opcode);
  ASSERT(iv);
  // Count the number of values on the stack accounted for by the
  // ImmVector's location and members
  int k = iv->numValues();
  // If this instruction also takes a RHS, count that too
  if (n == -2) ++k;
  return k;
}

/**
 * instrNumPushes() returns the number of values pushed onto the stack
 * for a given push/pop instruction. For peek/poke instructions, this
 * function returns 0.
 */
int instrNumPushes(const Opcode* opcode) {
  static const int8_t numberOfPushes[] = {
#define NOV 0
#define ONE(...) 1
#define TWO(...) 2
#define THREE(...) 3
#define POS_1(...) 0
#define POS_N(...) 0
#define O(name, imm, pop, push, flags) push,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef POS_1
#undef POS_N
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
#define POS_1(...) StackTransInfo::PeekPoke
#define POS_N(...) StackTransInfo::PeekPoke
#define O(name, imm, pop, push, flags) push,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef POS_1
#undef POS_N
#undef O
  };
  static const int8_t peekPokeType[] = {
#define NOV -1
#define ONE(...) -1
#define TWO(...) -1
#define THREE(...) -1
#define POS_1(...) 0
#define POS_N(...) 1
#define O(name, imm, pop, push, flags) push,
    OPCODES
#undef NOV
#undef ONE
#undef TWO
#undef THREE
#undef POS_1
#undef POS_N
#undef O
  };
  StackTransInfo ret;
  ret.kind = transKind[*opcode];
  if (ret.kind == StackTransInfo::PushPop) {
    // Handle push/pop instructions
    ret.pos = 0;
    ret.numPushes = instrNumPushes(opcode);
    ret.numPops = instrNumPops(opcode);
    return ret;
  }
  // Handle peek/poke instructions
  ret.numPops = 0;
  ret.numPushes = 0;
  int n = peekPokeType[*opcode];
  if (n == 1) {
    ret.pos = getImm(opcode, 0).u_IVA;
  } else {
    ASSERT(n == 0);
    ret.pos = 1;
  }
  return ret;
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

std::string instrToString(const Opcode* it, const Unit* u /* = NULL */) {
  // Location names
  static const char locationNames[] = { 'H', 'N', 'G', 'S', 'C', 'R' };
  static const int locationNamesCount =
    (int)(sizeof(locationNames)/sizeof(const char));
  // Member names
  static const char memberNames[] = { 'E', 'W', 'P' };
  static const int memberNamesCount =
    (int)(sizeof(memberNames)/sizeof(const char));
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
  Op op = (Op)*it;
  ++it;
  switch (op) {
#define READ(t) out << " " << *((t*)&*it); it += sizeof(t)
#define READV() out << " " << decodeVariableSizeImm((unsigned char**)&it);
#define READOA() do { \
  int immVal = (int)*((uchar*)&*it); \
  it += sizeof(uchar); \
  out << " "; \
  switch (op) { \
  case OpIncDecH: case OpIncDecN: case OpIncDecG: case OpIncDecS: \
  case OpIncDecM: \
    out << ((immVal >= 0 && immVal < incdecNamesCount) ? \
            incdecNames[immVal] : "?"); \
    break; \
  case OpSetOpH: case OpSetOpN: case OpSetOpG: case OpSetOpS: case OpSetOpM: \
    out << ((immVal >=0 && immVal < setopNamesCount) ? \
            setopNames[immVal] : "?"); \
    break; \
  default: \
    out << immVal; \
    break; \
  } \
} while (false)
#define READVEC() do { \
  int sz = *((int*)&*it); \
  it += sizeof(int); \
  out << " <"; \
  if (sz > 0) { \
    int immVal = (int)*((uchar*)&*it); \
    out << ((immVal >=0 && immVal < locationNamesCount) ? \
            locationNames[immVal] : '?'); \
    it += sizeof(uchar); \
    for (int i = 1; i < sz; ++i) { \
      immVal = (int)*((uchar*)&*it); \
      out << " " << ((immVal >=0 && immVal < memberNamesCount) ? \
                     memberNames[immVal] : '?'); \
      it += sizeof(uchar); \
    } \
  } \
  out << ">"; \
} while (false)
#define ONE(a) H_##a
#define TWO(a, b) H_##a; H_##b
#define THREE(a, b, c) H_##a; H_##b; H_##c;
#define NA
#define H_LA READVEC()
#define H_IVA READV()
#define H_I64A READ(int64)
#define H_DA READ(double)
#define H_BA READ(Offset)
#define H_OA READOA()
#define H_SA \
  if (u) { \
    const StringData* sd = u->lookupLitstrId(*((Id*)it)); \
    out << " \"" << \
    Util::escapeStringForCPP(sd->data(), sd->size()) << "\""; \
  } else { \
    out << " " << *((Id*)it); \
  } \
  it += sizeof(Id)
#define H_AA \
  if (u) { \
    out << " "; \
    staticArrayStreamer(u->lookupArrayId(*((Id*)it)), out); \
  } else { \
    out << " " << *((Id*)it); \
  } \
  it += sizeof(Id)
#define O(name, imm, push, pop, flags) \
  case Op##name: { \
    out << #name; \
    imm; \
    break; \
  }
OPCODES
#undef O
#undef READ
#undef ONE
#undef TWO
#undef THREE
#undef NA
#undef H_LA
#undef H_IVA
#undef H_I64A
#undef H_DA
#undef H_BA
#undef H_OA
#undef H_SA
#undef H_AA
    default: ASSERT(false);
  };
  return out.str();
}

bool instrIsControlFlow(Opcode opcode) {
  InstrFlags opFlags = instrFlags(opcode);
  return (opFlags & (UF | CF)) != 0;
}

bool instrReadsCurrentFpi(Opcode opcode) {
  InstrFlags opFlags = instrFlags(opcode);
  return (opFlags & FF) != 0;
}

int ImmVector::numValues() const {
  if (len <= 0) return 0;
  // Count the location; the LS location type accounts for
  // two values on the stack, all other location types account
  // for one value on the stack
  int count = get(0) == LS ? 2 : 1;
  // Count each of the members; ME and MP account for one value
  // on the stack, while MW does not account for any values on
  // the stack
  for (int index = 1; index < len; ++index) {
    if (get(index) != MW) {
      ++count;
    }
  }
  return count;
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


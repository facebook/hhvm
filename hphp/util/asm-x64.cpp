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
#include "hphp/util/asm-x64.h"

#include <folly/Format.h>

#include "hphp/util/safe-cast.h"

namespace HPHP { namespace jit {

// These are in order according to the binary encoding of the X64
// condition codes.
const char* cc_names[] = {
  "O", "NO", "B", "AE", "E", "NE", "BE", "A",
  "S", "NS", "P", "NP", "L", "GE", "LE", "G"
};

const char* show(RoundDirection rd) {
  switch (rd) {
    case RoundDirection::nearest:  return "nearest";
    case RoundDirection::floor:    return "floor";
    case RoundDirection::ceil:     return "ceil";
    case RoundDirection::truncate: return "truncate";
  }
  not_reached();
}

namespace x64 {

void DecodedInstruction::decode(uint8_t* ip) {
  m_ip = ip;
  m_flagsVal = 0;
  m_map_select = 0;
  m_xtra_op = 0;
  m_immSz = sz::nosize;
  m_offSz = sz::nosize;

  while (decodePrefix(ip)) {
    ++ip;
  }
  while (int sz = decodeRexVexXop(ip)) {
    ip += sz;
  }

  ip += decodeOpcode(ip);
  ip += decodeModRm(ip);
  ip += m_offSz + m_immSz;
  m_size = ip - m_ip;
}

bool DecodedInstruction::decodePrefix(uint8_t* ip) {
  switch (*ip) {
    case 0xf0: m_flags.lock = 1;      return true;
    case 0xf2: m_flags.repNE = 1;     return true;
    case 0xf3: m_flags.rep = 1;       return true;

    case 0x26: m_flags.es = 1;        return true;
    case 0x2e: m_flags.bTaken = 1;    return true;
    case 0x36: m_flags.ss = 1;        return true;
    case 0x3e: m_flags.bNotTaken = 1; return true;
    case 0x64: m_flags.fs = 1;        return true;
    case 0x65: m_flags.gs = 1;        return true;

    case 0x66: m_flags.opndSzOvr = 1; return true;
    case 0x67: m_flags.addrSzOvr = 1; return true;
  }
  return false;
}

int DecodedInstruction::decodeRexVexXop(uint8_t* ip) {
  if ((*ip & 0xf0) == 0x40) {
    m_flags.rex = 1;
    m_flags.w = *ip & 8 ? 1 : 0;
    m_flags.r = *ip & 4 ? 1 : 0;
    m_flags.x = *ip & 2 ? 1 : 0;
    m_flags.b = *ip & 1 ? 1 : 0;
    return 1;
  }

  int sz = 0;
  switch (*ip) {
    case 0xc4:
    case 0x8f:
      if (*ip == 0xc4) {
        m_flags.vex = 1;
      } else {
        // 0x8f is both a valid one-byte opcode and the first byte of the
        // 3-byte XOP prefix. Figure out which one we have here by inspecting
        // the next byte.
        if (ip[1] & 0x18) {
          m_flags.xop = 1;
        } else {
          return 0;
        }
      }

      sz = 3;
      m_flags.r = ip[1] & 0x80 ? 0 : 1;
      m_flags.x = ip[1] & 0x40 ? 0 : 1;
      m_flags.b = ip[1] & 0x20 ? 0 : 1;
      m_map_select = ip[1] & 0x1f;
      assert(m_map_select >= 1 && (m_flags.xop || m_map_select <= 3));
      m_flags.w = ip[2] & 0x80 ? 1 : 0;
      ip += 2;
      break;
    case 0xc5:
      sz = 2;
      m_flags.vex = 1;
      m_flags.r = ip[1] & 0x80 ? 0 : 1;
      m_map_select = 1;
      ip++;
      break;
    default:
      return 0;
  }

  // The final 7 bits of all VEX/XOP prefixes are the same:
  m_xtra_op = (~ip[0] >> 3) & 0x0f;
  m_flags.l = ip[0] & 0x04 ? 1 : 0;
  switch (ip[0] & 3) {
    case 0: break;
    case 1: m_flags.opndSzOvr = 1; break;
    case 2: m_flags.rep = 1; break;
    case 3: m_flags.repNE = 1; break;
  }
  return sz;
}

int DecodedInstruction::decodeOpcode(uint8_t* ip) {
  int sz = 1;
  if (*ip == 0x0f) {
    ++ip;
    ++sz;
    m_map_select = 1;
    if (*ip == 0x38) {
      ++ip;
      ++sz;
      m_map_select = 2;
    } else if (*ip == 0x3a) {
      ++ip;
      ++sz;
      m_map_select = 3;
    }
  }

  m_opcode = *ip;
  switch (m_map_select) {
    case 0: determineOperandsMap0(ip); break;
    case 1: determineOperandsMap1(ip); break;
    case 2: determineOperandsMap2(ip); break;
    case 3: determineOperandsMap3(ip); break;
    default: assert(false);
  }
  return sz;
}

void DecodedInstruction::determineOperandsMap0(uint8_t* ip) {
  switch (m_opcode >> 4) {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
      if ((m_opcode & 0x04) == 0) {
        m_flags.hasModRm = true;
      } else if ((m_opcode & 0x07) == 4) {
        m_immSz = sz::byte;
      } else if ((m_opcode & 0x07) == 5) {
        m_immSz = m_flags.opndSzOvr ? sz::word : sz::dword;
      }
      break;
    case 0x04: break; // REX
    case 0x05:
      m_flags.def64 = 1;
      break;
    case 0x06:
      if ((m_opcode & 0x0c) == 0x08) {
        m_immSz = m_opcode & 0x02 ? sz::byte :
          m_flags.opndSzOvr ? sz::word : sz::dword;
      }
      break;
    case 0x07:
      m_flags.picOff = true;
      m_offSz = sz::byte;
      break;
    case 0x08:
      m_flags.hasModRm = true;
      if ((m_opcode & 0x0c) == 0) {
        m_immSz = (m_opcode & 0x0f) != 1 ? sz::byte :
          m_flags.opndSzOvr ? sz::word : sz::dword;
      }
      break;
    case 0x09:
      break;
    case 0x0a:
      if ((m_opcode & 0x0c) == 0) {
        m_offSz = m_flags.addrSzOvr ? sz::dword : sz::qword;
      } else if ((m_opcode & 0x0e) == 8) {
        m_immSz = !(m_opcode & 1) ? sz::byte :
          m_flags.opndSzOvr ? sz::word : sz::dword;
      }
      break;
    case 0x0b:
      m_immSz = ((m_opcode & 8) == 0 ? sz::byte :
                 m_flags.w ? sz::qword :
                 m_flags.opndSzOvr ? sz::word : sz::dword);
      break;
    case 0x0c:
      m_flags.hasModRm = !(m_opcode & 8) && (m_opcode & 6) != 2;
      switch (m_opcode & 0x0f) {
        case 0: case 1: case 6: case 13:
          m_immSz = sz::byte;
          break;
        case 2: case 10:
          m_immSz = sz::word;
          break;
        case 7:
          m_immSz = m_flags.opndSzOvr ? sz::word : sz::dword;
          break;
        case 8:
          m_offSz = sz::word;
          m_immSz = sz::byte;
          break;
      }
      break;
    case 0x0d:
      m_flags.hasModRm = (m_opcode & 0x0c) == 0;
      m_immSz = (m_opcode & 0x0e) == 4 ? sz::byte : sz::nosize;
      break;
    case 0x0e: {
      uint8_t siz = sz::nosize;
      if ((m_opcode & 0x08) == 0 || (m_opcode & 0x0f) == 0xb) {
        siz = sz::byte;
      } else if ((m_opcode & 0x0e) == 0x8) {
        siz = sz::dword;
      }
      if (siz != sz::nosize) {
        if (!(m_opcode & 0x04)) {
          m_offSz = siz;
          m_flags.picOff = true;
        } else {
          m_immSz = siz;
        }
      }
      break;
    }
    case 0x0f:
      if ((m_opcode & 0x06) == 0x06) {
        m_flags.hasModRm = true;
        if (!(m_opcode & 0x08) && !((ip[1] >> 3) & 7)) {
          m_immSz = !(m_opcode & 1) ? sz::byte :
            m_flags.opndSzOvr ? sz::word : sz::dword;
        }
      }
      break;
  }
}

void DecodedInstruction::determineOperandsMap1(uint8_t* /*ip*/) {
  switch (m_opcode >> 4) {
    case 0:
      if ((m_opcode & 15) < 4 || (m_opcode & 15) == 13) {
        m_flags.hasModRm = true;
      }
      break;
    case 0x01: case 0x02: case 0x04: case 0x05: case 0x06:
      m_flags.hasModRm = true;
      break;
    case 0x03:
      break;
    case 0x07:
      m_flags.hasModRm = (m_opcode & 15) != 7;
      m_immSz = (m_opcode & 15) < 4 ? sz::byte : sz::nosize;
      break;
    case 0x08:
      m_offSz = sz::dword;
      m_flags.picOff = true;
      break;
    case 0x09:
      m_flags.hasModRm = true;
      break;
    case 0x0a:
      m_flags.hasModRm = (m_opcode & 7) >= 3;
      if ((m_opcode & 7) == 4) m_immSz = sz::byte;
      break;
    case 0x0b:
      m_flags.hasModRm = true;
      if ((m_opcode & 15) == 0x0a) m_immSz = sz::byte;
      break;
    case 0x0c:
      if (!(m_opcode & 8)) {
        m_flags.hasModRm = true;
        switch (m_opcode & 7) {
          case 0x02:
          case 0x04:
          case 0x05:
          case 0x06:
            m_immSz = sz::byte;
            break;
        }
      }
      break;
    case 0x0d:
    case 0x0e:
    case 0x0f:
      m_flags.hasModRm = true;
      break;
  }
}

void DecodedInstruction::determineOperandsMap2(uint8_t* /*ip*/) {
  m_flags.hasModRm = true;
  if (m_opcode == 0x13) m_immSz = sz::byte;
}

void DecodedInstruction::determineOperandsMap3(uint8_t* /*ip*/) {
  m_flags.hasModRm = true;
  m_immSz = sz::byte;
}

int DecodedInstruction::decodeModRm(uint8_t* ip) {
  if (!m_flags.hasModRm) return 0;
  int size = 1;
  if ((*ip & 0xc7) == 0x05) {
    m_flags.picOff = true;
    m_offSz = sz::dword;
  } else {
    if ((*ip & 0xc0) != 0xc0 &&
        (*ip & 0x07) == 0x04) {
      m_flags.hasSib = true;
      size++;
    }
    if ((*ip & 0xc0) == 0x00) {
      if (m_flags.hasSib && (ip[1] & 7) == 0x05) {
        m_offSz = sz::dword;
      }
    } else if ((*ip & 0xc0) == 0x40) {
      m_offSz = sz::byte;
    } else if ((*ip & 0xc0) == 0x80) {
      m_offSz = sz::dword;
    }
  }
  return size;
}

static int64_t readValue(uint8_t* ip, int size) {
  int64_t value = 0;
  value = (signed char)ip[--size];
  while (size--) {
    value <<= 8;
    value += ip[size];
  }
  return value;
}

static bool writeValue(uint8_t* ip, int size, int64_t v) {
  auto value = uint64_t(v);
  if (size * CHAR_BIT < 64) {
    auto topBit = uint64_t(1) << (size * CHAR_BIT - 1);
    if (value + topBit >= topBit * 2) return false;
  }

  while (size--) {
    *ip++ = (uint8_t)value;
    value >>= CHAR_BIT;
  }
  return true;

}

std::string DecodedInstruction::toString() {
  auto str = folly::format("{:08x} {:02x}",
                           (uint64_t)m_ip,
                           m_opcode).str();
  if (m_flags.hasModRm) {
    auto modRm = getModRm();
    str += folly::format(" ModRM({:02b} {} {})",
                         modRm >> 6,
                         (modRm >> 3) & 7,
                         modRm & 7).str();
    if (m_flags.hasSib) {
      auto sib = m_ip[m_size - m_immSz - m_offSz - 1];
      str += folly::format(" SIB({:02b} {} {})",
                           sib >> 6,
                           (sib >> 3) & 7,
                           sib & 7).str();
    }
  }

  auto ip = m_ip + m_size - m_immSz - m_offSz;
  if (m_offSz) {
    int64_t value = readValue(ip, m_offSz);
    ip += m_offSz;
    str += folly::format(" {}{:+x}",
                         m_flags.picOff ? "rip" : "",
                         value).str();
    if (m_flags.picOff) {
      str += folly::format("({:08x})", uintptr_t(m_ip + m_size + value)).str();
    }
  }

  if (m_immSz) {
    int64_t value = readValue(ip, m_immSz);
    ip += m_immSz;
    str += folly::format(" #{}", value).str();
  }
  return str;
}

int32_t DecodedInstruction::offset() const {
  assert(hasOffset());
  auto const addr = m_ip + m_size;
  return safe_cast<int32_t>(readValue(addr - m_offSz, m_offSz));
}

uint8_t* DecodedInstruction::picAddress() const {
  assert(hasPicOffset());
  uint8_t* addr = m_ip + m_size;
  uint8_t* rel = m_base + m_size;
  return rel + readValue(addr - m_immSz - m_offSz, m_offSz);
}

bool DecodedInstruction::setPicAddress(uint8_t* target) {
  assert(hasPicOffset());
  uint8_t* addr = m_ip + m_size;
  uint8_t* rel = m_base + m_size;
  ptrdiff_t diff = target - rel;

  return writeValue(addr - m_offSz - m_immSz, m_offSz, diff);
}

int64_t DecodedInstruction::immediate() const {
  assert(hasImmediate());
  return readValue(m_ip + m_size - m_immSz, m_immSz);
}

bool DecodedInstruction::setImmediate(int64_t value) {
  assert(hasImmediate());
  return writeValue(m_ip + m_size - m_immSz, m_immSz, value);
}

bool DecodedInstruction::isNop() const {
  if (m_opcode == 0x90) {
    return m_size == 1 || (m_size == 2 && m_flags.opndSzOvr);
  }
  return m_opcode == 0x1f && m_map_select == 1;
}

bool DecodedInstruction::isBranch(BranchType branchType
                               /* = Conditional |
                                    Unconditional */) const {
  if (!m_flags.picOff) return false;
  if (m_map_select == 0) {
    // The one-byte opcode map
    return
      ((m_opcode & 0xf0) == 0x70 /* 8-bit conditional branch */ &&
       (branchType & Conditional)) ||
      ((m_opcode == 0xe9 /* 32-bit unconditional branch */ ||
        m_opcode == 0xeb /* 8-bit unconditional branch */) &&
       (branchType & Unconditional));
  }
  if (m_map_select == 1 && (branchType & Conditional)) {
    // The two-byte opcode map (first byte is 0x0f)
    return (m_opcode & 0xf0) == 0x80 /* 32-bit conditional branch */;
  }
  return false;
}

bool DecodedInstruction::isCall() const {
  if (m_map_select != 0) return false;
  if (m_opcode == 0xe8) return true;
  if (m_opcode != 0xff) return false;
  return ((getModRm() >> 3) & 0x6) == 2;
}

bool DecodedInstruction::isJmp() const {
  if (m_map_select != 0) return false;
  return m_opcode == 0xe9;
}

bool DecodedInstruction::isLea() const {
  if (m_map_select != 0) return false;
  return m_opcode == 0x8d;
}

ConditionCode DecodedInstruction::jccCondCode() const {
  if (m_map_select == 0) {
    assert((m_opcode & 0xf0) == 0x70); // 8-bit jcc
  } else {
    assert(m_map_select == 1);
    assert((m_opcode & 0xf0) == 0x80); // 32-bit jcc
  }
  return static_cast<ConditionCode>(m_opcode & 0x0f);
}

bool DecodedInstruction::shrinkBranch() {
  assert(isBranch());
  if (m_offSz != sz::dword) return false;
  auto addr = m_ip + m_size - m_offSz;
  auto delta = readValue(addr, m_offSz);
  if (m_map_select == 1) {
    if (m_flags.vex) return false;
    assert((m_opcode & 0xf0) == 0x80); // must be a 32-bit conditional branch
    /*
      The pc-relative offset is from the end of the instruction, and the
      instruction is shrinking by 4 bytes (opcode goes from 2 bytes to 1,
      and offset goes from 4 to 1), so we need to adjust delta by 4.
    */
    delta += 4;
    if (-128 > delta || delta > 127) return false;
    addr[-2] = 0x70 | (m_opcode & 0x0f); // make it an 8 bit conditional branch
    addr[-1] = delta;
  } else {
    assert(m_opcode == 0xe9); // must be a 32-bit unconditional branch
    /*
      As above, but opcode was already 1 byte, so the reduction is only 3
      bytes this time.
    */
    delta += 3;
    if (-128 > delta || delta > 127) return false;
    addr[-1] = 0xeb;
    addr[0] = delta;
  }
  decode(m_ip);
  assert(isBranch() && m_offSz == 1);
  return true;
}

void DecodedInstruction::widenBranch() {
  assert(m_offSz == 1 && isBranch());
  auto addr = m_ip + m_size - m_offSz;
  auto delta = readValue(addr, 1);
  if (m_opcode == 0xeb) {
    addr[-1] = 0xe9;
    writeValue(addr, 4, delta + 3);
  } else {
    addr[-1] = 0x0f;
    addr[0] = 0x80 | (m_opcode & 0xf);
    writeValue(addr + 1, 4, delta + 4);
  }
  decode(m_ip);
  assert(isBranch() && m_offSz == 4);
}

uint8_t DecodedInstruction::getModRm() const {
  assert(m_flags.hasModRm);
  return m_ip[m_size - m_immSz - m_offSz - m_flags.hasSib - 1];
}

#define FUSEABLE_INSTRUCTIONS \
  TEST(0x84, 0xFF) \
  TEST(0x85, 0xFF) \
  TEST(0xA8, 0xFF) \
  TEST(0xA9, 0xFF) \
  TEST(0xF6,  0) \
  TEST(0xF6,  1) \
  TEST(0xF7,  0) \
  TEST(0xF7,  1) \
  AND(0x20,  0xFF, true) \
  AND(0x21,  0xFF, true) \
  AND(0x22,  0xFF, false) \
  AND(0x23,  0xFF, false) \
  AND(0x24,  0xFF, false) \
  AND(0x25,  0xFF, false) \
  AND(0x80,   4, true) \
  AND(0x81,   4, true) \
  AND(0x83,   4, true) \
  CMP(0x38,  0xFF) \
  CMP(0x39,  0xFF) \
  CMP(0x3A,  0xFF) \
  CMP(0x3B,  0xFF) \
  CMP(0x3C,  0xFF) \
  CMP(0x3D,  0xFF) \
  CMP(0x80,   7) \
  CMP(0x81,   7) \
  CMP(0x83,   7) \
  ADD(0x00,  0xFF, true) \
  ADD(0x01,  0xFF, true) \
  ADD(0x02,  0xFF, false) \
  ADD(0x03,  0xFF, false) \
  ADD(0x04,  0xFF, false) \
  ADD(0x05,  0xFF, false) \
  ADD(0x80,   0, true) \
  ADD(0x81,   0, true) \
  ADD(0x83,   0, true) \
  SUB(0x28,  0xFF, true) \
  SUB(0x29,  0xFF, true) \
  SUB(0x2A,  0xFF, false) \
  SUB(0x2B,  0xFF, false) \
  SUB(0x2C,  0xFF, false) \
  SUB(0x2D,  0xFF, false) \
  SUB(0x80,   5, true) \
  SUB(0x81,   5, true) \
  SUB(0x83,   5, true) \
  INC(0xFE,   0) \
  INC(0xFF,   0) \
  DEC(0xFE,   1) \
  DEC(0xFF,   1)

#define TEST(i, j) \
  X(i, j, false, CC_O, CC_NO, CC_B, CC_NAE, CC_AE, CC_NB, CC_NC, CC_E, CC_Z, \
    CC_NE, CC_NZ, CC_BE, CC_NA, CC_A, CC_NBE, CC_S, CC_NS, CC_P, CC_NP, CC_L, \
    CC_NGE, CC_GE, CC_NL, CC_LE, CC_NG, CC_G, CC_NLE)
#define AND(i, j, k) \
  X(i, j, k, CC_O, CC_NO, CC_B, CC_NAE, CC_AE, CC_NB, CC_NC, CC_E, CC_Z, \
    CC_NE, CC_NZ, CC_BE, CC_NA, CC_A, CC_NBE, CC_S, CC_NS, CC_P, CC_NP, CC_L, \
    CC_NGE, CC_GE, CC_NL, CC_LE, CC_NG, CC_G, CC_NLE)
#define CMP(i, j) \
  X(i, j, false, CC_B, CC_NAE, CC_AE, CC_NB, CC_NC, CC_E, CC_Z, CC_NE, CC_NZ, \
    CC_BE, CC_NA, CC_A, CC_NBE, CC_L, CC_NGE, CC_GE, CC_NL, CC_LE, CC_NG, \
    CC_G, CC_NLE)
#define ADD(i, j, k) \
  X(i, j, k, CC_B, CC_NAE, CC_AE, CC_NB, CC_NC, CC_E, CC_Z, CC_NE, CC_NZ, \
    CC_BE, CC_NA, CC_A, CC_NBE, CC_L, CC_NGE, CC_GE, CC_NL, CC_LE, CC_NG, \
    CC_G, CC_NLE)
#define SUB(i, j, k) \
  X(i, j, k, CC_B, CC_NAE, CC_AE, CC_NB, CC_NC, CC_E, CC_Z, CC_NE, CC_NZ, \
    CC_BE, CC_NA, CC_A, CC_NBE, CC_L, CC_NGE, CC_GE, CC_NL, CC_LE, CC_NG, \
    CC_G, CC_NLE)
#define INC(i, j) \
  X(i, j, true, CC_E, CC_Z, CC_NE, CC_NZ, CC_L, CC_NGE, CC_GE, CC_NL, CC_LE, \
    CC_NG, CC_G, CC_NLE)
#define DEC(i, j) \
  X(i, j, true, CC_E, CC_Z, CC_NE, CC_NZ, CC_L, CC_NGE, CC_GE, CC_NL, CC_LE, \
    CC_NG, CC_G, CC_NLE)

bool DecodedInstruction::isFuseable(const DecodedInstruction& next) const {
  // Assumes no invalid instructions.
  if (m_map_select != 0 || // No multibyte instructions are fuseable.
      hasPicOffset() || // No rip relative addressing
      !next.isBranch(Conditional)) {
    return false;
  }

  // Find extra 3 bits of opcode in the modrm byte if this opcode has it.
  uint32_t e = 0xFF;
  switch (m_opcode) {
    case 0xF6: case 0xF7: case 0x80: case 0x81: case 0x83: case 0xFE: case 0xFF:
      e = m_flags.hasModRm ? (getModRm() & 0x38) >> 3 : -1;
      break;
    default:
      break;
  }

  switch (e << 16 | m_opcode) {
#define X(opcode, opExt, operand1IsDest, ...) \
    case (opExt << 16 | opcode): { \
      if ((operand1IsDest || hasImmediate()) && \
          m_flags.hasModRm && (getModRm() & 0xC0) != 0xC0) { \
        /* No fusing instruction with immediate and memory operands. */ \
        /* Also cannot fuse instructions with destination memory operand. */ \
        return false; \
      } \
      const ConditionCode ccs[] = { __VA_ARGS__ }; \
      for (size_t i = 0; i < sizeof(ccs) / sizeof(ConditionCode); ++i) { \
        if (next.jccCondCode() == ccs[i]) { \
          return true; \
        } \
      } \
      break; \
    }
FUSEABLE_INSTRUCTIONS
#undef X
    default:
      break;
  }
  return false;
}

#undef TEST
#undef AND
#undef CMP
#undef ADD
#undef SUB
#undef INC
#undef DEC

#undef FUSEABLE_INSTRUCTIONS

}}}

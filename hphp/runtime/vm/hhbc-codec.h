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

#ifndef incl_HPHP_VM_HHBC_CODEC_H_
#define incl_HPHP_VM_HHBC_CODEC_H_

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/util/either.h"

namespace HPHP {

struct Unit;
struct UnitEmitter;

/*
 * This file contains various functions for reading and writing bytecode
 * streams, both opcodes and immediates.
 */

//////////////////////////////////////////////////////////////////////

/*
 * The functions in this section are responsible for encoding and decoding Ops
 * to and from bytecode streams, and should be the only pieces of code that
 * know how to do so.
 *
 * We currently use a variable-width encoding scheme that can store [0, 0xff)
 * using one byte and [0xff, 0x1fe) using two bytes. If and when we hit 0x1fe
 * bytecodes we can adjust the encoding. All bytes are written with their bits
 * flipped, to ensure that any code attempting to read raw Ops from a bytecode
 * stream will fail.
 */

/*
 * Read a T from in, advancing in past the read value.
 */
template<typename T>
T decode_raw(PC& in) {
  T data;
  memcpy(&data, in, sizeof(data));
  in += sizeof(data);
  return data;
}

inline uint8_t decode_byte(PC& pc) {
  return decode_raw<uint8_t>(pc);
}

/*
 * Encode the given Op, using write_byte to write a byte at a time.
 */
template<typename F>
void encode_op(Op op, F write_byte) {
  static_assert(Op_count <= 0x1fe,
                "Op encoding scheme doesn't support Ops >= 0x1fe");
  auto rawVal = static_cast<size_t>(op);
  if (rawVal >= 0xff) {
    // Write a 0xff signal byte, with its bits flipped.
    write_byte(static_cast<uint8_t>(0));
    rawVal -= 0xff;
  }
  assert(rawVal < 0xff);

  write_byte(~rawVal);
}

/*
 * Read on Op from pc, advancing pc past the Op. This version makes no
 * assertions on the validity of the Op returned, and is useful for testing the
 * decoding scheme with arbitrary values.
 */
inline Op decode_op_unchecked(PC& pc) {
  size_t rawVal = static_cast<uint8_t>(~decode_byte(pc));
  if (rawVal == 0xff) {
    auto const byte = static_cast<uint8_t>(~decode_byte(pc));
    assert(byte < 0xff);
    rawVal += byte;
  }

  auto const op = static_cast<Op>(rawVal);
  return op;
}

/*
 * Read an Op from pc, advancing pc past the Op.
 */
inline Op decode_op(PC& pc) {
  auto const op = decode_op_unchecked(pc);
  if (!isValidOpcode(op)) not_reached();
  return op;
}

/*
 * Read an Op from pc, without modifying pc.
 */
inline Op peek_op(PC pc) {
  return decode_op(pc);
}

/*
 * Return the encoded size of the given Op, in bytes.
 */
constexpr size_t encoded_op_size(Op op) {
  return static_cast<size_t>(op) < 0xff ? 1 : 2;
}

//////////////////////////////////////////////////////////////////////

/*
 * Read various immediate types from a pc, optionally advancing pc past the
 * read data.
 */

template<class T> T decode_oa(PC& pc) {
  return decode_raw<T>(pc);
}

ALWAYS_INLINE
int32_t decodeVariableSizeImm(PC* immPtr) {
  auto const small = **immPtr;
  if (UNLIKELY(small & 0x1)) {
    auto const large = decode_raw<uint32_t>(*immPtr);
    return (int32_t)(large >> 1);
  }

  (*immPtr)++;
  return (int32_t)(small >> 1);
}

ALWAYS_INLINE int32_t decode_iva(PC& pc) {
  return decodeVariableSizeImm(&pc);
}

/*
 * Decode a MemberKey, advancing pc past it.
 */
MemberKey decode_member_key(PC& pc, Either<const Unit*, const UnitEmitter*> u);

void encode_member_key(MemberKey mk, UnitEmitter& ue);

//////////////////////////////////////////////////////////////////////

template<typename L>
void foreachSwitchTarget(PC pc, L func) {
  auto const op = decode_op(pc);
  assert(isSwitch(op));
  int32_t size = decode_raw<int32_t>(pc);
  for (int i = 0; i < size; ++i) {
    if (op == Op::SSwitch) decode_raw<Id>(pc);
    func(decode_raw<Offset>(pc));
  }
}

template<typename L>
void foreachSwitchString(PC pc, L func) {
  auto const UNUSED op = decode_op(pc);
  assert(op == Op::SSwitch);
  int32_t size = decode_raw<int32_t>(pc) - 1; // the last item is the default
  for (int i = 0; i < size; ++i) {
    func(decode_raw<Id>(pc));
    decode_raw<Offset>(pc);
  }
}

//////////////////////////////////////////////////////////////////////

}

#endif

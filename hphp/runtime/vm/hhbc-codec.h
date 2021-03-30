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

#pragma once

#include "hphp/runtime/base/repo-auth-type-codec.h"
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
 * bytecodes we can adjust the encoding.
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
    // Write a 0xff signal byte
    write_byte(static_cast<uint8_t>(0xff));
    rawVal -= 0xff;
  }
  assertx(rawVal < 0xff);

  write_byte(rawVal);
}

/*
 * Read on Op from pc, advancing pc past the Op. This version makes no
 * assertions on the validity of the Op returned, and is useful for testing the
 * decoding scheme with arbitrary values.
 */
inline Op decode_op_unchecked(PC& pc) {
  uint32_t raw = decode_byte(pc);
  return LIKELY(raw != 0xff) ? Op(raw) : Op(decode_byte(pc) + 0xff);
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

ALWAYS_INLINE Offset decode_ba(PC& pc) {
  return decode_raw<Offset>(pc);
}

ALWAYS_INLINE uint32_t decode_iva(PC& pc) {
  auto const small = *pc;
  if (UNLIKELY(int8_t(small) < 0)) {
    auto const large = decode_raw<uint32_t>(pc);
    return (large & 0xffffff00) >> 1 | (small & 0x7f);
  }
  pc++;
  return small;
}

using StringDecoder = Either<const Unit*, const UnitEmitter*>;

/*
 * decode a namedlocal, advancing pc past it.
 */
NamedLocal decode_named_local(PC& pc);

/*
 * Decode a MemberKey, advancing pc past it.
 */
MemberKey decode_member_key(PC& pc, StringDecoder u);

void encode_member_key(MemberKey mk, FuncEmitter& fe);

//////////////////////////////////////////////////////////////////////

template<typename L>
void foreachSwitchTarget(PC pc, L func) {
  auto const op = decode_op(pc);
  assertx(isSwitch(op));
  if (op == Op::Switch) {
    (void)decode_oa<SwitchKind>(pc); // skip bounded kind
    (void)decode_raw<int64_t>(pc); // skip base
  }
  int32_t size = decode_iva(pc);
  for (int i = 0; i < size; ++i) {
    if (op == Op::SSwitch) decode_raw<Id>(pc);
    func(decode_raw<Offset>(pc));
  }
}

template<typename L>
void foreachSSwitchString(PC pc, L func) {
  auto const UNUSED op = decode_op(pc);
  assertx(op == Op::SSwitch);
  int32_t size = decode_iva(pc) - 1; // the last item is the default
  for (int i = 0; i < size; ++i) {
    func(decode_raw<Id>(pc));
    decode_raw<Offset>(pc);
  }
}

//////////////////////////////////////////////////////////////////////

void encodeLocalRange(FuncEmitter&, const LocalRange&);
LocalRange decodeLocalRange(const unsigned char*&);

//////////////////////////////////////////////////////////////////////

void encodeIterArgs(FuncEmitter&, const IterArgs&);
IterArgs decodeIterArgs(const unsigned char*&);

//////////////////////////////////////////////////////////////////////

void encodeFCallArgsBase(FuncEmitter&, const FCallArgsBase&,
                         bool hasInoutArgs, bool hasAsyncEagerOffset,
                         bool hasContext);
void encodeFCallArgsIO(FuncEmitter&, int numBytes, const uint8_t* inoutArgs);

FCallArgs decodeFCallArgs(Op, PC&, StringDecoder);

template<typename T, typename IO, typename CTX>
void encodeFCallArgs(FuncEmitter& fe, const FCallArgsBase& fca,
                     bool hasInoutArgs, IO emitInoutArgs,
                     bool hasAsyncEagerOffset, T emitAsyncEagerOffset,
                     bool hasContext, CTX emitContext) {
  encodeFCallArgsBase(fe, fca, hasInoutArgs, hasAsyncEagerOffset, hasContext);
  if (hasInoutArgs) emitInoutArgs();
  if (hasAsyncEagerOffset) emitAsyncEagerOffset();
  if (hasContext) emitContext();
}

//////////////////////////////////////////////////////////////////////

}

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

#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/unit.h"

namespace HPHP {

namespace {

const StringData* decode_string(PC& pc, StringDecoder u) {
  auto const id = decode_raw<Id>(pc);
  if (u.isNull()) return nullptr;
  return u.match(
    [id](const Unit* u) { return u->lookupLitstrId(id); },
    [id](const UnitEmitter* ue) { return ue->lookupLitstr(id); }
  );
}

}

NamedLocal decode_named_local(PC& pc) {
  LocalName lname = decode_iva(pc) - 1;
  int32_t id = decode_iva(pc);
  return NamedLocal{lname, id};
}

///////////////////////////////////////////////////////////////////////////////

MemberKey decode_member_key(PC& pc, Either<const Unit*, const UnitEmitter*> u) {
  auto const mcode = static_cast<MemberCode>(decode_byte(pc));

  switch (mcode) {
    case MEC: case MPC: {
      auto const iva = static_cast<int32_t>(decode_iva(pc));
      return MemberKey{mcode, iva, decode_oa<ReadonlyOp>(pc)};
    }
    case MEL: case MPL: {
      auto const local = decode_named_local(pc);
      return MemberKey{mcode, local, decode_oa<ReadonlyOp>(pc)};
    }
    case MEI: {
      auto const i64 = decode_raw<int64_t>(pc);
      return MemberKey{mcode, i64, decode_oa<ReadonlyOp>(pc)};
    }
    case MET: case MPT: case MQT: {
      auto const str = decode_string(pc, u);
      return MemberKey{mcode, str, decode_oa<ReadonlyOp>(pc)};
    }

    case MW:
      return MemberKey{};
  }
  not_reached();
}

void encode_member_key(MemberKey mk, FuncEmitter& fe) {
  fe.emitByte(mk.mcode);

  switch (mk.mcode) {
    case MEC: case MPC:
      fe.emitIVA(mk.iva);
      fe.emitByte(static_cast<uint8_t>(mk.rop));
      break;

    case MEL: case MPL:
      fe.emitNamedLocal(mk.local);
      fe.emitByte(static_cast<uint8_t>(mk.rop));
      break;

    case MEI:
      fe.emitInt64(mk.int64);
      fe.emitByte(static_cast<uint8_t>(mk.rop));
      break;

    case MET: case MPT: case MQT:
      fe.emitInt32(fe.ue().mergeLitstr(mk.litstr));
      fe.emitByte(static_cast<uint8_t>(mk.rop));
      break;

    case MW:
      // No immediate
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

void encodeLocalRange(FuncEmitter& fe, const LocalRange& range) {
  fe.emitIVA(range.first);
  fe.emitIVA(range.count);
}

LocalRange decodeLocalRange(const unsigned char*& pc) {
  auto const first = decode_iva(pc);
  auto const restCount = decode_iva(pc);
  return LocalRange{uint32_t(first), uint32_t(restCount)};
}

///////////////////////////////////////////////////////////////////////////////

void encodeIterArgs(FuncEmitter& fe, const IterArgs& args) {
  fe.emitByte(args.flags);
  fe.emitIVA(args.iterId);
  fe.emitIVA(args.keyId - IterArgs::kNoKey);
  fe.emitIVA(args.valId);
}

IterArgs decodeIterArgs(PC& pc) {
  auto const flags = static_cast<IterArgs::Flags>(decode_byte(pc));
  auto const iterId = int32_t(decode_iva(pc));
  auto const keyId = int32_t(decode_iva(pc)) + IterArgs::kNoKey;
  auto const valId = int32_t(decode_iva(pc));
  return IterArgs(flags, iterId, keyId, valId);
}

///////////////////////////////////////////////////////////////////////////////

void encodeFCallArgsBase(FuncEmitter& fe, const FCallArgsBase& fca,
                         bool hasInoutArgs, bool hasReadonlyArgs,
                         bool hasAsyncEagerOffset, bool hasContext) {
  auto constexpr kFirstNumArgsBit = FCallArgsBase::kFirstNumArgsBit;
  auto constexpr kLimit =
    std::numeric_limits<std::underlying_type_t<FCallArgsFlags>>::max();
  bool smallNumArgs = ((fca.numArgs + 1) << kFirstNumArgsBit) <= kLimit;
  auto flags = uint16_t{fca.flags};
  assertx(!(flags & ~FCallArgs::kInternalFlags));
  if (smallNumArgs) flags |= ((fca.numArgs + 1) << kFirstNumArgsBit);
  if (fca.numRets != 1) flags |= FCallArgsFlags::HasInOut;
  if (hasInoutArgs) flags |= FCallArgsFlags::EnforceInOut;
  if (hasReadonlyArgs) flags |= FCallArgsFlags::EnforceReadonly;
  if (hasAsyncEagerOffset) flags |= FCallArgsFlags::HasAsyncEagerOffset;
  if (hasContext) flags |= FCallArgsFlags::ExplicitContext;

  static_assert(sizeof(std::underlying_type_t<FCallArgsFlags>) * 8 == 16);
  fe.emitInt16(flags);
  if (!smallNumArgs) fe.emitIVA(fca.numArgs);
  if (fca.numRets != 1) fe.emitIVA(fca.numRets);
}

void encodeFCallArgsBoolVec(FuncEmitter& fe, int numBytes,
                       const uint8_t* boolVec) {
  for (auto i = 0; i < numBytes; ++i) fe.emitByte(boolVec[i]);
}

FCallArgs decodeFCallArgs(Op thisOpcode, PC& pc, StringDecoder u) {
  assertx(isFCall(thisOpcode));
  bool skipContext = true;
  auto const flags = [&]() {
    auto rawFlags = decode_raw<std::underlying_type_t<FCallArgsFlags>>(pc);
    skipContext = !(rawFlags & FCallArgsFlags::ExplicitContext);
    if (u.isNull()) rawFlags &= ~FCallArgsFlags::ExplicitContext;
    return rawFlags;
  }();

  uint32_t numArgs = (flags >> FCallArgs::kFirstNumArgsBit)
    ? (flags >> FCallArgs::kFirstNumArgsBit) - 1 : decode_iva(pc);
  auto const numRets = (flags & FCallArgsFlags::HasInOut) ? decode_iva(pc) : 1;
  auto const inoutArgs = (flags & FCallArgsFlags::EnforceInOut) ? pc : nullptr;
  if (inoutArgs != nullptr) pc += (numArgs + 7) / 8;
  auto const readonlyArgs = (flags & FCallArgsFlags::EnforceReadonly) ? pc : nullptr;
  if (readonlyArgs != nullptr) pc += (numArgs + 7) / 8;
  auto const asyncEagerOffset = (flags & FCallArgsFlags::HasAsyncEagerOffset)
    ? decode_ba(pc) : kInvalidOffset;
  auto const context = !skipContext ? decode_string(pc, u) : nullptr;
  return FCallArgs(
    static_cast<FCallArgsFlags>(flags & FCallArgs::kInternalFlags),
    numArgs, numRets, inoutArgs, readonlyArgs, asyncEagerOffset, context
  );
}

///////////////////////////////////////////////////////////////////////////////
}

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
    case MEC: case MPC:
      return MemberKey{mcode, static_cast<int32_t>(decode_iva(pc))};

    case MEL: case MPL:
      return MemberKey{mcode, decode_named_local(pc)};

    case MEI:
      return MemberKey{mcode, decode_raw<int64_t>(pc)};

    case MET: case MPT: case MQT: {
      return MemberKey{mcode, decode_string(pc, u)};
    }

    case MW:
      return MemberKey{};
  }
  not_reached();
}

void encode_member_key(MemberKey mk, UnitEmitter& ue) {
  ue.emitByte(mk.mcode);

  switch (mk.mcode) {
    case MEC: case MPC:
      ue.emitIVA(mk.iva);
      break;

    case MEL: case MPL:
      ue.emitNamedLocal(mk.local);
      break;

    case MEI:
      ue.emitInt64(mk.int64);
      break;

    case MET: case MPT: case MQT:
      ue.emitInt32(ue.mergeLitstr(mk.litstr));
      break;

    case MW:
      // No immediate
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

void encodeLocalRange(UnitEmitter& ue, const LocalRange& range) {
  ue.emitIVA(range.first);
  ue.emitIVA(range.count);
}

LocalRange decodeLocalRange(const unsigned char*& pc) {
  auto const first = decode_iva(pc);
  auto const restCount = decode_iva(pc);
  return LocalRange{uint32_t(first), uint32_t(restCount)};
}

///////////////////////////////////////////////////////////////////////////////

void encodeIterArgs(UnitEmitter& ue, const IterArgs& args) {
  ue.emitByte(args.flags);
  ue.emitIVA(args.iterId);
  ue.emitIVA(args.keyId - IterArgs::kNoKey);
  ue.emitIVA(args.valId);
}

IterArgs decodeIterArgs(PC& pc) {
  auto const flags = static_cast<IterArgs::Flags>(decode_byte(pc));
  auto const iterId = int32_t(decode_iva(pc));
  auto const keyId = int32_t(decode_iva(pc)) + IterArgs::kNoKey;
  auto const valId = int32_t(decode_iva(pc));
  return IterArgs(flags, iterId, keyId, valId);
}

///////////////////////////////////////////////////////////////////////////////

void encodeFCallArgsBase(UnitEmitter& ue, const FCallArgsBase& fca,
                         bool hasInoutArgs, bool hasAsyncEagerOffset,
                         bool hasContext) {
  auto flags = uint8_t{fca.flags};
  assertx(!(flags & ~FCallArgsBase::kInternalFlags));
  if (!fca.numArgs && fca.skipNumArgsCheck) flags |= FCallArgsBase::NoArgs;
  if (fca.numRets != 1) flags |= FCallArgsBase::HasInOut;
  if (hasInoutArgs) flags |= FCallArgsBase::EnforceInOut;
  if (hasAsyncEagerOffset) flags |= FCallArgsBase::HasAsyncEagerOffset;
  if (hasContext) flags |= FCallArgsBase::ExplicitContext;
  if (fca.lockWhileUnwinding) {
    // intentionally re-using the SupportsAsyncEagerReturn bit
    assertx(!(flags & FCallArgsBase::SupportsAsyncEagerReturn));
    flags |= FCallArgsBase::SupportsAsyncEagerReturn;
  }

  ue.emitByte(flags);
  if (fca.numArgs || !fca.skipNumArgsCheck) {
    ue.emitIVA((uint64_t)fca.numArgs * 2 + fca.skipNumArgsCheck);
  }
  if (fca.numRets != 1) ue.emitIVA(fca.numRets);
}

void encodeFCallArgsIO(UnitEmitter& ue, int numBytes,
                       const uint8_t* inoutArgs) {
  for (auto i = 0; i < numBytes; ++i) ue.emitByte(inoutArgs[i]);
}

FCallArgs decodeFCallArgs(Op thisOpcode, PC& pc, StringDecoder u) {
  assertx(isFCall(thisOpcode));
  bool lockWhileUnwinding = false;
  bool skipContext = true;
  auto const flags = [&]() {
    auto rawFlags = decode_byte(pc);
    if (thisOpcode == Op::FCallCtor &&
        (rawFlags & FCallArgs::SupportsAsyncEagerReturn)) {
      lockWhileUnwinding = true;
      rawFlags &= ~FCallArgs::SupportsAsyncEagerReturn;
    }
    skipContext = !(rawFlags & FCallArgs::ExplicitContext);
    if (u.isNull()) rawFlags &= ~FCallArgs::ExplicitContext;
    return rawFlags;
  }();

  uint32_t numArgs;
  bool skipNumArgsCheck;
  if (flags & FCallArgs::NoArgs) {
    numArgs = 0;
    skipNumArgsCheck = true;
  } else {
    numArgs = decode_iva(pc);
    skipNumArgsCheck = numArgs % 2;
    numArgs /= 2;
  }
  auto const numRets = (flags & FCallArgs::HasInOut) ? decode_iva(pc) : 1;
  auto const inoutArgs = (flags & FCallArgs::EnforceInOut) ? pc : nullptr;
  if (inoutArgs != nullptr) pc += (numArgs + 7) / 8;
  auto const asyncEagerOffset = (flags & FCallArgs::HasAsyncEagerOffset)
    ? decode_ba(pc) : kInvalidOffset;
  auto const context = !skipContext ? decode_string(pc, u) : nullptr;
  return FCallArgs(
    static_cast<FCallArgs::Flags>(flags & FCallArgs::kInternalFlags),
    numArgs, numRets, inoutArgs, asyncEagerOffset, lockWhileUnwinding,
    skipNumArgsCheck, context
  );
}

///////////////////////////////////////////////////////////////////////////////
}

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

MemberKey decode_member_key(PC& pc, Either<const Unit*, const UnitEmitter*> u) {
  auto const mcode = static_cast<MemberCode>(decode_byte(pc));

  switch (mcode) {
    case MEC: case MEL: case MPC: case MPL:
      return MemberKey{mcode, decode_iva(pc)};

    case MEI:
      return MemberKey{mcode, decode_raw<int64_t>(pc)};

    case MET: case MPT: case MQT: {
      auto const id = decode_raw<Id>(pc);
      auto const str = u.match(
        [id](const Unit* u) { return u->lookupLitstrId(id); },
        [id](const UnitEmitter* ue) { return ue->lookupLitstr(id); }
      );
      return MemberKey{mcode, str};
    }

    case MW:
      return MemberKey{};
  }
  not_reached();
}

void encode_member_key(MemberKey mk, UnitEmitter& ue) {
  ue.emitByte(mk.mcode);

  switch (mk.mcode) {
    case MEC: case MEL: case MPC: case MPL:
      ue.emitIVA(mk.iva);
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

void encodeFCallArgsBase(UnitEmitter& ue, const FCallArgsBase& fca,
                         const uint8_t* byRefs, bool hasAsyncEagerOffset) {
  auto constexpr kFirstNumArgsBit = FCallArgsBase::kFirstNumArgsBit;
  bool smallNumArgs = ((fca.numArgs + 1) << kFirstNumArgsBit) <= 0xff;
  auto flags = uint8_t{fca.flags};
  assertx(!(flags & ~FCallArgsBase::kInternalFlags));
  if (smallNumArgs) flags |= (fca.numArgs + 1) << kFirstNumArgsBit;
  if (fca.numRets != 1) flags |= FCallArgsBase::HasInOut;
  if (byRefs != nullptr) flags |= FCallArgsBase::EnforceReffiness;
  if (hasAsyncEagerOffset) flags |= FCallArgsBase::HasAsyncEagerOffset;

  ue.emitByte(flags);
  if (!smallNumArgs) ue.emitIVA(fca.numArgs);
  if (fca.numRets != 1) ue.emitIVA(fca.numRets);

  if (byRefs != nullptr) {
    auto const numBytes = (fca.numArgs + 7) / 8;
    for (auto i = 0; i < numBytes; ++i) ue.emitByte(byRefs[i]);
  }
}

FCallArgs decodeFCallArgs(PC& pc) {
  auto const flags = decode_byte(pc);
  auto const numArgs = (flags >> FCallArgs::kFirstNumArgsBit)
    ? (flags >> FCallArgs::kFirstNumArgsBit) - 1 : decode_iva(pc);
  auto const numRets = (flags & FCallArgs::HasInOut) ? decode_iva(pc) : 1;
  auto const byRefs = (flags & FCallArgs::EnforceReffiness) ? pc : nullptr;
  if (byRefs != nullptr) pc += (numArgs + 7) / 8;
  auto const asyncEagerOffset = (flags & FCallArgs::HasAsyncEagerOffset)
    ? decode_ba(pc) : kInvalidOffset;
  return FCallArgs(
    static_cast<FCallArgs::Flags>(flags & FCallArgs::kInternalFlags),
    numArgs, numRets, byRefs, asyncEagerOffset
  );
}

///////////////////////////////////////////////////////////////////////////////
}

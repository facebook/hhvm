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

}

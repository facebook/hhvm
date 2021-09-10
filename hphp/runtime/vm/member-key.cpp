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


#include "hphp/runtime/vm/member-key.h"

#include "hphp/runtime/base/string-data.h"

#include "hphp/util/text-util.h"

#include <folly/Conv.h>

#include <string.h>
#include <type_traits>

namespace HPHP {

const char* const memberNames[] =
  { "EC", "PC", "EL", "PL", "ET", "PT", "QT", "EI", "W"};
const size_t memberNamesCount = sizeof(memberNames) /
                                sizeof(*memberNames);

static_assert(memberNamesCount == NumMemberCodes,
             "Member code missing for memberCodeString");

const char* memberCodeString(MemberCode mcode) {
  static_assert(
      std::is_unsigned<typename std::underlying_type<MemberCode>::type>::value,
      "MemberCode is expected to be unsigned.");
  assertx(mcode < NumMemberCodes);
  return memberNames[mcode];
}

const char* const readOnlyNames[] = {
  "Any", "ReadOnly", "Mutable", "CheckROCOW", "CheckMutROCOW"
};

const char* readOnlyString(ReadonlyOp op) {
  static_assert(
    std::is_same<typename std::underlying_type<ReadonlyOp>::type,uint8_t>::value,
    "Subops are all expected to be single-bytes"
  );
  return readOnlyNames[static_cast<uint8_t>(op)];
}

Optional<MemberCode> parseMemberCode(const char* s) {
  for (auto i = 0; i < memberNamesCount; i++) {
    if (!strcmp(memberNames[i], s)) {
      return MemberCode(i);
    }
  }
  return std::nullopt;
}

std::string show(const MemberKey& mk) {
  std::string ret = memberCodeString(mk.mcode);
  auto const op = readOnlyString(mk.rop);

  switch (mk.mcode) {
    case MEC: case MPC:
      folly::toAppend(':', mk.iva, " ", op, &ret);
      break;
    case MEL: case MPL:
      folly::toAppend(':', mk.local.name, ':', mk.local.id, " ", op, &ret);
      break;
    case MEI:
      folly::toAppend(':', mk.int64, " ", op, &ret);
      break;
    case MET: case MPT: case MQT:
      folly::toAppend(
        ":\"", escapeStringForCPP(mk.litstr->data(), mk.litstr->size()), '"',
        " ", op, &ret
      );
      break;
    case MW:
      break;
  }

  return ret;
}

}

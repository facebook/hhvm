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


#include "hphp/runtime/vm/member-key.h"

#include "hphp/runtime/base/string-data.h"

#include "hphp/util/text-util.h"

#include <folly/Conv.h>

#include <string.h>

namespace HPHP {

const char* const memberNames[] =
  { "EC", "PC", "EL", "PL", "ET", "PT", "QT", "EI", "W"};
const size_t memberNamesCount = sizeof(memberNames) /
                                sizeof(*memberNames);

static_assert(memberNamesCount == NumMemberCodes,
             "Member code missing for memberCodeString");

const char* memberCodeString(MemberCode mcode) {
  assert(mcode >= 0 && mcode < NumMemberCodes);
  return memberNames[mcode];
}

folly::Optional<MemberCode> parseMemberCode(const char* s) {
  for (auto i = 0; i < memberNamesCount; i++) {
    if (!strcmp(memberNames[i], s)) {
      return MemberCode(i);
    }
  }
  return folly::none;
}

std::string show(MemberKey mk) {
  std::string ret = memberCodeString(mk.mcode);

  switch (mk.mcode) {
    case MEL: case MEC: case MPL: case MPC:
      folly::toAppend(':', mk.iva, &ret);
      break;
    case MEI:
      folly::toAppend(':', mk.int64, &ret);
      break;
    case MET: case MPT: case MQT:
      folly::toAppend(
        ":\"", escapeStringForCPP(mk.litstr->data(), mk.litstr->size()), '"',
        &ret
      );
      break;
    case MW:
      break;
  }

  return ret;
}

}

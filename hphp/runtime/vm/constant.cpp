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

#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/constant.h"
#include "hphp/runtime/vm/hhbc.h"

namespace HPHP {

void Constant::prettyPrint(std::ostream& out) const {
  out << "Constant " << name->data();
  if (type(val) == KindOfUninit) {
    out << " = " << "<non-scalar>";
  } else {
    std::string ss;
    staticStreamer(&val, ss);
    out << " = " << ss;
  }
  out << std::endl;
}

const StringData* Constant::nameFromFuncName(const StringData* func_name) {
  const int prefix_len = sizeof("86cinit_") - 1;
  if (func_name->size() <= prefix_len) {
    return nullptr;
  }
  auto slice = func_name->slice();
  auto ns_pos = slice.rfind('\\');
  folly::StringPiece ns_slice = "";
  if (ns_pos >= 0) {
    ns_slice = slice.subpiece(0, ns_pos + 1);
    slice = slice.subpiece(ns_pos + 1, slice.size() - ns_pos - 1);
  }
  if (!slice.startsWith("86cinit_")) {
    return nullptr;
  }
  slice = slice.subpiece(8, slice.size() - prefix_len);
  return makeStaticString(folly::sformat("{}{}", ns_slice, slice));
}

const StringData* Constant::funcNameFromName(const StringData* name) {
  auto slice = name->slice();
  auto ns_pos = slice.rfind('\\');
  if (ns_pos < 0) {
    return makeStaticString(folly::sformat("86cinit_{}", slice));
  }
  auto ns_slice = slice.subpiece(0, ns_pos + 1);
  slice = slice.subpiece(ns_pos + 1, slice.size() - ns_pos - 1);
  return makeStaticString(folly::sformat("{}86cinit_{}", ns_slice, slice));
}

}

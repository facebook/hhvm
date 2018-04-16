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

#include "datatype.h"

#include "hphp/runtime/base/runtime-option.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

MaybeDataType get_datatype(
  const std::string& name,
  bool can_be_collection,
  bool is_function,
  bool is_xhp,
  bool is_tuple,
  bool is_nullable,
  bool is_soft
) {
  if (is_function || is_xhp || is_tuple) {
    return KindOfObject;
  }
  if (can_be_collection) {
    if (!strcasecmp(name.c_str(), "array"))      return KindOfArray;
    if (!strcasecmp(name.c_str(), "HH\\vec"))    return KindOfVec;
    if (!strcasecmp(name.c_str(), "HH\\dict"))   return KindOfDict;
    if (!strcasecmp(name.c_str(), "HH\\keyset")) return KindOfKeyset;
    if (!strcasecmp(name.c_str(), "HH\\varray")) {
      return RuntimeOption::EvalHackArrDVArrs ? KindOfVec : KindOfArray;
    }
    if (!strcasecmp(name.c_str(), "HH\\darray")) {
      return RuntimeOption::EvalHackArrDVArrs ? KindOfDict : KindOfArray;
    }
    if (!strcasecmp(name.c_str(), "HH\\varray_or_darray")) return folly::none;
    if (!strcasecmp(name.c_str(), "HH\\vec_or_dict")) return folly::none;
    return KindOfObject;
  }
  if (is_nullable || is_soft) {
    return folly::none;
  }
  if (!strcasecmp(name.c_str(), "null") ||
      !strcasecmp(name.c_str(), "HH\\void")) {
    return KindOfNull;
  }
  if (!strcasecmp(name.c_str(), "HH\\bool"))     return KindOfBoolean;
  if (!strcasecmp(name.c_str(), "HH\\int"))      return KindOfInt64;
  if (!strcasecmp(name.c_str(), "HH\\float"))    return KindOfDouble;
  if (!strcasecmp(name.c_str(), "HH\\num"))      return folly::none;
  if (!strcasecmp(name.c_str(), "HH\\arraykey")) return folly::none;
  if (!strcasecmp(name.c_str(), "HH\\string"))   return KindOfString;
  if (!strcasecmp(name.c_str(), "array"))        return KindOfArray;
  if (!strcasecmp(name.c_str(), "HH\\dict"))     return KindOfDict;
  if (!strcasecmp(name.c_str(), "HH\\vec"))      return KindOfVec;
  if (!strcasecmp(name.c_str(), "HH\\keyset"))   return KindOfKeyset;
  if (!strcasecmp(name.c_str(), "HH\\varray")) {
    return RuntimeOption::EvalHackArrDVArrs ? KindOfVec : KindOfArray;
  }
  if (!strcasecmp(name.c_str(), "HH\\darray")) {
    return RuntimeOption::EvalHackArrDVArrs ? KindOfDict : KindOfArray;
  }
  if (!strcasecmp(name.c_str(), "HH\\varray_or_darray")) return folly::none;
  if (!strcasecmp(name.c_str(), "HH\\vec_or_dict")) return folly::none;
  if (!strcasecmp(name.c_str(), "HH\\resource")) return KindOfResource;
  if (!strcasecmp(name.c_str(), "HH\\mixed"))    return folly::none;
  if (!strcasecmp(name.c_str(), "HH\\nonnull"))  return folly::none;

  return KindOfObject;
}

///////////////////////////////////////////////////////////////////////////////

}

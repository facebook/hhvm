/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/repo-auth-type.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

static_assert(sizeof(RepoAuthType) == sizeof(CompactSizedPtr<void>), "");

//////////////////////////////////////////////////////////////////////

folly::Optional<DataType> convertToDataType(RepoAuthType ty) {
  using T = RepoAuthType::Tag;
  switch (ty.tag()) {
  case T::OptBool:
  case T::OptInt:
  case T::OptSArr:
  case T::OptArr:
  case T::OptSStr:
  case T::OptStr:
  case T::OptDbl:
  case T::OptRes:
  case T::OptSubObj:
  case T::OptExactObj:
  case T::OptObj:
  case T::Null:
    return folly::none;

  case T::Cell:
  case T::Ref:
  case T::InitUnc:
  case T::Unc:
  case T::InitCell:
  case T::InitGen:
  case T::Gen:
    return folly::none;

  case T::Uninit:       return KindOfUninit;
  case T::InitNull:     return KindOfNull;
  case T::Bool:         return KindOfBoolean;
  case T::Int:          return KindOfInt64;
  case T::Dbl:          return KindOfDouble;
  case T::Res:          return KindOfResource;

  case T::SStr:
  case T::Str:          return KindOfString;

  case T::SArr:
  case T::Arr:          return KindOfArray;

  case T::Obj:
  case T::SubObj:
  case T::ExactObj:     return KindOfObject;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}

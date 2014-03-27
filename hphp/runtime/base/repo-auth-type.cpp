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

#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/vm/unit.h"

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

bool tvMatchesRepoAuthType(TypedValue tv, RepoAuthType ty) {
  assert(tvIsPlausible(tv));

  bool const initNull = tv.m_type == KindOfNull;

  using T = RepoAuthType::Tag;
  switch (ty.tag()) {
  case T::Uninit:       return tv.m_type == KindOfUninit;
  case T::InitNull:     return initNull;

  case T::OptBool:      if (initNull) return true;
                        // fallthrough
  case T::Bool:         return tv.m_type == KindOfBoolean;
  case T::OptInt:       if (initNull) return true;
                        // fallthrough
  case T::Int:          return tv.m_type == KindOfInt64;
  case T::OptDbl:       if (initNull) return true;
                        // fallthrough
  case T::Dbl:          return tv.m_type == KindOfDouble;
  case T::OptRes:       if (initNull) return true;
                        // fallthrough
  case T::Res:          return tv.m_type == KindOfResource;
  case T::OptObj:       if (initNull) return true;
                        // fallthrough
  case T::Obj:          return tv.m_type == KindOfObject;

  case T::OptSStr:
    if (initNull) return true;
    // fallthrough
  case T::SStr:
    return tv.m_type == KindOfStaticString ||
           (tv.m_type == KindOfString && tv.m_data.pstr->isStatic());

  case T::OptStr:
    if (initNull) return true;
    // fallthrough
  case T::Str:
    return IS_STRING_TYPE(tv.m_type);

  case T::OptSArr:
    if (initNull) return true;
    // fallthrough
  case T::SArr:
    return tv.m_type == KindOfArray && tv.m_data.parr->isStatic();

  case T::OptArr:
    if (initNull) return true;
    // fallthrough
  case T::Arr:
    return tv.m_type == KindOfArray;

  case T::Null:
    return initNull || tv.m_type == KindOfUninit;

  case T::OptSubObj:
    if (initNull) return true;
    // fallthrough
  case T::SubObj:
    {
      auto const cls = Unit::lookupClass(ty.clsName());
      if (!cls) return false;
      return tv.m_type == KindOfObject &&
             tv.m_data.pobj->getVMClass()->classof(cls);
    }

  case T::OptExactObj:
    if (initNull) return true;
    // fallthrough
  case T::ExactObj:
    {
      auto const cls = Unit::lookupClass(ty.clsName());
      if (!cls) return false;
      return tv.m_type == KindOfObject && tv.m_data.pobj->getVMClass() == cls;
    }

  case T::InitUnc:
    if (tv.m_type == KindOfUninit) return false;
    // fallthrough
  case T::Unc:
    return !IS_REFCOUNTED_TYPE(tv.m_type) ||
           (tv.m_type == KindOfString && tv.m_data.pstr->isStatic()) ||
           (tv.m_type == KindOfArray && tv.m_data.parr->isStatic());

  case T::InitCell:
    if (tv.m_type == KindOfUninit) return false;
    // fallthrough
  case T::Cell:
    return tv.m_type != KindOfRef;

  case T::Ref:
    return tv.m_type == KindOfRef;

  case T::InitGen:
    if (tv.m_type == KindOfUninit) return false;
    // fallthrough
  case T::Gen:
    return true;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}

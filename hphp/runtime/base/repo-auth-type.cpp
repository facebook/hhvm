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
#include "hphp/runtime/base/repo-auth-type.h"

#include <vector>

#include <folly/Hash.h>

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/tv-helpers.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/unit.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

static_assert(sizeof(RepoAuthType) == sizeof(CompactTaggedPtr<void>), "");

//////////////////////////////////////////////////////////////////////

namespace {

bool tvMatchesArrayType(TypedValue tv, const RepoAuthType::Array* arrTy) {
  assert(isArrayType(tv.m_type));
  auto const ad = tv.m_data.parr;
  using A = RepoAuthType::Array;

  auto sizeMatches = [&] {
    switch (arrTy->emptiness()) {
    case A::Empty::Maybe:
      return ad->size() == 0 || ad->size() == arrTy->size();
    case A::Empty::No:
      return ad->size() == arrTy->size();
    }
    not_reached();
  };

  // O(N) checks are available if you want them for debugging, but
  // they are too slow for general use in debug builds.  These type
  // matching functions are currently only used for assertions, so
  // it's ok to leave them out.
  auto const use_slow_checks = false;

  switch (arrTy->tag()) {
  case A::Tag::Packed:
    if (!sizeMatches()) return false;
    if (use_slow_checks) {
      for (auto i = uint32_t{0}; i < ad->size(); ++i) {
        auto const elem = ad->nvGet(i);
        if (!tvMatchesRepoAuthType(*elem, arrTy->packedElem(i))) {
          return false;
        }
      }
    }
    break;
  case A::Tag::PackedN:
    if (use_slow_checks) {
      for (auto i = uint32_t{0}; i < ad->size(); ++i) {
        auto const elem = ad->nvGet(i);
        if (!tvMatchesRepoAuthType(*elem, arrTy->elemType())) {
          return false;
        }
      }
    }
    break;
  }

  return true;
}

}

//////////////////////////////////////////////////////////////////////

bool RepoAuthType::operator==(RepoAuthType o) const {
  using T = Tag;
  if (tag() != o.tag()) return false;
  switch (tag()) {
  case T::OptBool:
  case T::OptInt:
  case T::OptSStr:
  case T::OptStr:
  case T::OptDbl:
  case T::OptRes:
  case T::OptObj:
  case T::Null:
  case T::Cell:
  case T::Ref:
  case T::InitUnc:
  case T::Unc:
  case T::InitCell:
  case T::InitGen:
  case T::Gen:
  case T::Uninit:
  case T::InitNull:
  case T::Bool:
  case T::Int:
  case T::Dbl:
  case T::Res:
  case T::SStr:
  case T::Str:
  case T::Obj:
    return true;

  case T::OptSArr:
  case T::OptArr:
    // Can't currently have array() info.
    return true;

  case T::SArr:
  case T::Arr:
    if (array() == nullptr && o.array() == nullptr) {
      return true;
    }
    if ((array() == nullptr) != (o.array() == nullptr)) {
      return false;
    }
    return array()->id() == o.array()->id();

  case T::SubObj:
  case T::ExactObj:
  case T::OptSubObj:
  case T::OptExactObj:
    return clsName() == o.clsName();
  }
  not_reached();
}

size_t RepoAuthType::hash() const {
  auto const iTag = static_cast<size_t>(tag());
  if (hasClassName()) {
    return folly::hash::hash_128_to_64(iTag, clsName()->hash());
  }
  if (mayHaveArrData() && array()) {
    return folly::hash::hash_128_to_64(iTag, array()->id());
  }
  return iTag;
}

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
    return isStringType(tv.m_type) && tv.m_data.pstr->isStatic();

  case T::OptStr:
    if (initNull) return true;
    // fallthrough
  case T::Str:
    return isStringType(tv.m_type);

  case T::OptSArr:
    if (initNull) return true;
    // fallthrough
  case T::SArr:
    if (!isArrayType(tv.m_type) || !tv.m_data.parr->isStatic()) {
      return false;
    }
    if (auto const arr = ty.array()) {
      if (!tvMatchesArrayType(tv, arr)) return false;
    }
    return true;

  case T::OptArr:
    if (initNull) return true;
    // fallthrough
  case T::Arr:
    if (!isArrayType(tv.m_type)) return false;
    if (auto const arr = ty.array()) {
      if (!tvMatchesArrayType(tv, arr)) return false;
    }
    return true;

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
    return !isRefcountedType(tv.m_type) ||
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

std::string show(RepoAuthType rat) {
  auto const tag = rat.tag();
  using T = RepoAuthType::Tag;
  switch (tag) {
  case T::OptBool:  return "?Bool";
  case T::OptInt:   return "?Int";
  case T::OptSStr:  return "?SStr";
  case T::OptStr:   return "?Str";
  case T::OptDbl:   return "?Dbl";
  case T::OptRes:   return "?Res";
  case T::OptObj:   return "?Obj";
  case T::Null:     return "Null";
  case T::Cell:     return "Cell";
  case T::Ref:      return "Ref";
  case T::InitUnc:  return "InitUnc";
  case T::Unc:      return "Unc";
  case T::InitCell: return "InitCell";
  case T::InitGen:  return "InitGen";
  case T::Gen:      return "Gen";
  case T::Uninit:   return "Uninit";
  case T::InitNull: return "InitNull";
  case T::Bool:     return "Bool";
  case T::Int:      return "Int";
  case T::Dbl:      return "Dbl";
  case T::Res:      return "Res";
  case T::SStr:     return "SStr";
  case T::Str:      return "Str";
  case T::Obj:      return "Obj";

  case T::OptSArr:
  case T::OptArr:
  case T::SArr:
  case T::Arr:
    {
      auto ret = std::string{};
      if (tag == T::OptArr || tag == T::OptSArr) {
        ret += '?';
      }
      if (tag == T::SArr || tag == T::OptSArr) {
        ret += 'S';
      }
      ret += "Arr";
      if (auto const ar = rat.array()) {
        folly::format(&ret, "{}", show(*ar));
      }
      return ret;
    }
    break;

  case T::OptSubObj:
  case T::OptExactObj:
  case T::SubObj:
  case T::ExactObj:
    {
      auto ret = std::string{};
      if (tag == T::OptSubObj || tag == T::OptExactObj) {
        ret += '?';
      }
      ret += "Obj";
      if (tag == T::OptSubObj || tag == T::SubObj) {
        ret += "<";
      }
      ret += '=';
      ret += rat.clsName()->data();
      return ret;
    }
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}

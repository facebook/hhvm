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
#include "hphp/runtime/base/repo-auth-type.h"

#include <vector>

#include <folly/Hash.h>

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/unit.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

static_assert(sizeof(RepoAuthType) == sizeof(CompactTaggedPtr<void>), "");

//////////////////////////////////////////////////////////////////////

namespace {

bool tvMatchesArrayType(TypedValue tv, const RepoAuthType::Array* arrTy) {
  assertx(isArrayLikeType(tv.m_type));
  auto const ad = tv.m_data.parr;
  using A = RepoAuthType::Array;

  if (ad->empty()) return arrTy->emptiness() == A::Empty::Maybe;
  if (arrTy->tag() == A::Tag::Packed && ad->size() != arrTy->size()) {
    return false;
  }

  // O(N) checks are available if you want them for debugging, but
  // they are too slow for general use in debug builds.  These type
  // matching functions are currently only used for assertions, so
  // it's ok to leave them out.
  if (false) {
    switch (arrTy->tag()) {
      case A::Tag::Packed:
        if (!ad->isVectorData()) return false;
        for (auto i = uint32_t{0}; i < ad->size(); ++i) {
          auto const elem = ad->at(i);
          if (!tvMatchesRepoAuthType(elem, arrTy->packedElem(i))) {
            return false;
          }
        }
        break;
      case A::Tag::PackedN:
        if (!ad->isVectorData()) return false;
        for (auto i = uint32_t{0}; i < ad->size(); ++i) {
          auto const elem = ad->at(i);
          if (!tvMatchesRepoAuthType(elem, arrTy->elemType())) {
            return false;
          }
        }
        break;
    }
  }

  return true;
}

}

//////////////////////////////////////////////////////////////////////

void RepoAuthType::resolveArray(const UnitEmitter& ue) {
  auto fn = [&](const uint32_t id) {
    return ue.lookupArrayType(id);
  };
  doResolve(fn);
}

const uint32_t RepoAuthType::arrayId() const {
  assertx(mayHaveArrData());
  if (resolved()) return m_data.ptr() ? array()->id() : kInvalidArrayId;

  return reinterpret_cast<uintptr_t>(m_data.ptr());
}

//////////////////////////////////////////////////////////////////////

bool RepoAuthType::operator==(RepoAuthType o) const {
  using T = Tag;
  if (tag() != o.tag()) return false;
  switch (tag()) {
  case T::OptBool:
  case T::UninitBool:
  case T::OptInt:
  case T::UninitInt:
  case T::OptSStr:
  case T::UninitSStr:
  case T::OptStr:
  case T::UninitStr:
  case T::OptDbl:
  case T::OptRes:
  case T::OptObj:
  case T::UninitObj:
  case T::OptFunc:
  case T::OptCls:
  case T::OptClsMeth:
  case T::OptRecord:
  case T::OptLazyCls:
  case T::OptArrKey:
  case T::OptUncArrKey:
  case T::OptStrLike:
  case T::OptUncStrLike:
  case T::OptArrKeyCompat:
  case T::OptUncArrKeyCompat:
  case T::Null:
  case T::Cell:
  case T::InitUnc:
  case T::Unc:
  case T::ArrKey:
  case T::UncArrKey:
  case T::StrLike:
  case T::UncStrLike:
  case T::ArrKeyCompat:
  case T::UncArrKeyCompat:
  case T::InitCell:
  case T::Uninit:
  case T::InitNull:
  case T::Bool:
  case T::Int:
  case T::Dbl:
  case T::Res:
  case T::SStr:
  case T::Str:
  case T::Obj:
  case T::Func:
  case T::Cls:
  case T::ClsMeth:
  case T::Record:
  case T::LazyCls:
  case T::Num:
  case T::OptNum:
  case T::InitPrim:
  case T::NonNull:
    return true;

  case T::SVec:
  case T::Vec:
  case T::OptSVec:
  case T::OptVec:
  case T::SDict:
  case T::Dict:
  case T::OptSDict:
  case T::OptDict:
  case T::SKeyset:
  case T::Keyset:
  case T::OptSKeyset:
  case T::OptKeyset:
  case T::SArrLike:
  case T::ArrLike:
  case T::OptSArrLike:
  case T::OptArrLike:
  case T::VecCompat:
  case T::OptVecCompat:
  case T::ArrLikeCompat:
  case T::OptArrLikeCompat:
    // array id equals to either kInvalidArrayId for null array info, or a
    // regular id. in each case, we just need to compare their id.
    return arrayId() == o.arrayId();

  case T::SubObj:
  case T::ExactObj:
  case T::OptSubObj:
  case T::OptExactObj:
  case T::UninitSubObj:
  case T::UninitExactObj:
  case T::SubCls:
  case T::ExactCls:
  case T::OptSubCls:
  case T::OptExactCls:
    return clsName() == o.clsName();

  case T::SubRecord:
  case T::ExactRecord:
  case T::OptSubRecord:
  case T::OptExactRecord:
    return recordName() == o.recordName();
  }
  not_reached();
}

size_t RepoAuthType::hash() const {
  auto const iTag = static_cast<size_t>(tag());
  if (hasClassName()) {
    return folly::hash::hash_128_to_64(iTag, clsName()->hash());
  }
  if (mayHaveArrData() && (arrayId() != kInvalidArrayId)) {
    return folly::hash::hash_128_to_64(iTag, arrayId());
  }
  return iTag;
}

//////////////////////////////////////////////////////////////////////

bool tvMatchesRepoAuthType(TypedValue tv, RepoAuthType ty) {
  assertx(tvIsPlausible(tv));

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
  case T::OptFunc:      if (initNull) return true;
                        // fallthrough
  case T::Func:         return tv.m_type == KindOfFunc;
  case T::OptCls:       if (initNull) return true;
                        // fallthrough
  case T::Cls:          return tv.m_type == KindOfClass;
  case T::OptClsMeth:   if (initNull) return true;
                        // fallthrough
  case T::ClsMeth:      return tv.m_type == KindOfClsMeth;
  case T::OptRecord:    if (initNull) return true;
                        // fallthrough
  case T::Record:       return tv.m_type == KindOfRecord;
  case T::OptLazyCls:   if (initNull) return true;
                        // fallthrough
  case T::LazyCls:      return tv.m_type == KindOfLazyClass;

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

  case T::OptSVec:
    if (initNull) return true;
    // fallthrough
  case T::SVec:
    if (!isVecType(tv.m_type) || !tv.m_data.parr->isStatic()) return false;
    if (auto const arr = ty.array()) {
      if (!tvMatchesArrayType(tv, arr)) return false;
    }
    return true;

  case T::OptVecCompat:
  case T::OptVec:
    if (initNull) return true;
    // fallthrough
  case T::VecCompat:
    if (ty.tag() != T::OptVec && isClsMethType(tv.m_type)) return true;
    // fallthrough
  case T::Vec:
    if (!isVecType(tv.m_type)) return false;
    if (auto const arr = ty.array()) {
      if (!tvMatchesArrayType(tv, arr)) return false;
    }
    return true;

  case T::OptSDict:
    if (initNull) return true;
    // fallthrough
  case T::SDict:
    if (!isDictType(tv.m_type) || !tv.m_data.parr->isStatic()) return false;
    if (auto const arr = ty.array()) {
      if (!tvMatchesArrayType(tv, arr)) return false;
    }
    return true;

  case T::OptDict:
    if (initNull) return true;
    // fallthrough
  case T::Dict:
    if (!isDictType(tv.m_type)) return false;
    if (auto const arr = ty.array()) {
      if (!tvMatchesArrayType(tv, arr)) return false;
    }
    return true;

  case T::OptSKeyset:
    if (initNull) return true;
    // fallthrough
  case T::SKeyset:
    if (!isKeysetType(tv.m_type) || !tv.m_data.parr->isStatic()) return false;
    if (auto const arr = ty.array()) {
      if (!tvMatchesArrayType(tv, arr)) return false;
    }
    return true;

  case T::OptKeyset:
    if (initNull) return true;
    // fallthrough
  case T::Keyset:
    if (!isKeysetType(tv.m_type)) return false;
    if (auto const arr = ty.array()) {
      if (!tvMatchesArrayType(tv, arr)) return false;
    }
    return true;

  case T::OptSArrLike:
    if (initNull) return true;
    // fallthrough
  case T::SArrLike:
    if (!isArrayLikeType(tv.m_type) || !tv.m_data.parr->isStatic()) {
      return false;
    }
    if (auto const arr = ty.array()) {
      if (!tvMatchesArrayType(tv, arr)) return false;
    }
    return true;

  case T::OptArrLike:
    if (initNull) return true;
    // fallthrough
  case T::ArrLike:
    if (!isArrayLikeType(tv.m_type)) return false;
    if (auto const arr = ty.array()) {
      if (!tvMatchesArrayType(tv, arr)) return false;
    }
    return true;

  case T::OptArrLikeCompat:
    if (initNull) return true;
    // fallthrough
  case T::ArrLikeCompat:
    if (isClsMethType(tv.m_type)) return true;
    if (!isArrayLikeType(tv.m_type)) return false;
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
      auto const cls = Class::lookup(ty.clsName());
      if (!cls) return false;
      return tv.m_type == KindOfObject &&
             tv.m_data.pobj->getVMClass()->classof(cls);
    }

  case T::UninitSubObj:
    {
      if (tv.m_type == KindOfUninit) return true;
      auto const cls = Class::lookup(ty.clsName());
      if (!cls) return false;
      return tv.m_type == KindOfObject &&
             tv.m_data.pobj->getVMClass()->classof(cls);
    }

  case T::OptExactObj:
    if (initNull) return true;
    // fallthrough
  case T::ExactObj:
    {
      auto const cls = Class::lookup(ty.clsName());
      if (!cls) return false;
      return tv.m_type == KindOfObject && tv.m_data.pobj->getVMClass() == cls;
    }

  case T::UninitExactObj:
    {
      if (tv.m_type == KindOfUninit) return true;
      auto const cls = Class::lookup(ty.clsName());
      if (!cls) return false;
      return tv.m_type == KindOfObject && tv.m_data.pobj->getVMClass() == cls;
    }

  case T::OptSubCls:
    if (initNull) return true;
    // fallthrough
  case T::SubCls:
    {
      auto const cls = Class::lookup(ty.clsName());
      if (!cls) return false;
      return tv.m_type == KindOfClass &&
             tv.m_data.pclass->classof(cls);
    }

  case T::OptExactCls:
    if (initNull) return true;
    // fallthrough
  case T::ExactCls:
    {
      auto const cls = Class::lookup(ty.clsName());
      if (!cls) return false;
      return tv.m_type == KindOfClass && tv.m_data.pclass == cls;
    }

  case T::OptSubRecord:
    if (initNull) return true;
    // fallthrough
  case T::SubRecord:
    {
      auto const rec = Unit::lookupRecordDesc(ty.recordName());
      if (!rec) return false;
      return tv.m_type == KindOfRecord &&
             tv.m_data.prec->record()->recordDescOf(rec);
    }

  case T::OptExactRecord:
    if (initNull) return true;
    // fallthrough
  case T::ExactRecord:
    {
      auto const rec = Unit::lookupRecordDesc(ty.recordName());
      if (!rec) return false;
      return tv.m_type == KindOfRecord && tv.m_data.prec->record() == rec;
    }

  case T::InitUnc:
    if (tv.m_type == KindOfUninit) return false;
    // fallthrough
  case T::Unc:
    return !isRefcountedType(tv.m_type) ||
           (tv.m_type == KindOfString && tv.m_data.pstr->isStatic()) ||
           (isArrayLikeType(tv.m_type) && tv.m_data.parr->isStatic());

  case T::OptArrKey:
    if (initNull) return true;
    // fallthrough
  case T::ArrKey:
    return isStringType(tv.m_type) || tv.m_type == KindOfInt64;

  case T::OptUncArrKey:
    if (initNull) return true;
    // fallthrough
  case T::UncArrKey:
    return (isStringType(tv.m_type) && !tv.m_data.pstr->isRefCounted()) ||
      tv.m_type == KindOfInt64;

  case T::OptStrLike:
    if (initNull) return true;
    // fallthrough
  case T::StrLike:
    return isStringType(tv.m_type) ||
      tv.m_type == KindOfClass || tv.m_type == KindOfLazyClass;

  case T::OptUncStrLike:
    if (initNull) return true;
    // fallthrough
  case T::UncStrLike:
    return (isStringType(tv.m_type) && !tv.m_data.pstr->isRefCounted()) ||
      tv.m_type == KindOfClass || tv.m_type == KindOfLazyClass;

  case T::OptArrKeyCompat:
    if (initNull) return true;
    // fallthrough
  case T::ArrKeyCompat:
    return isStringType(tv.m_type) ||
           tv.m_type == KindOfInt64 ||
           isClassType(tv.m_type) ||
           isLazyClassType(tv.m_type);

  case T::OptUncArrKeyCompat:
    if (initNull) return true;
    // fallthrough
  case T::UncArrKeyCompat:
    return (isStringType(tv.m_type) && !tv.m_data.pstr->isRefCounted()) ||
           tv.m_type == KindOfInt64 ||
           isClassType(tv.m_type) || isLazyClassType(tv.m_type);

  case T::OptNum:
    if (initNull) return true;
    // fallthrough
  case T::Num:
    return isIntType(tv.m_type) || isDoubleType(tv.m_type);

  case T::NonNull:
    return tv.m_type != KindOfUninit && !initNull;

  case T::UninitInt:
    return isIntType(tv.m_type) || tv.m_type == KindOfUninit;
  case T::UninitBool:
    return isBoolType(tv.m_type) || tv.m_type == KindOfUninit;
  case T::UninitSStr:
    return
      (isStringType(tv.m_type) && tv.m_data.pstr->isStatic())
      || tv.m_type == KindOfUninit;
  case T::UninitStr:
    return isStringType(tv.m_type) || tv.m_type == KindOfUninit;
  case T::UninitObj:
    return isObjectType(tv.m_type) || tv.m_type == KindOfUninit;

  case T::InitPrim:
    return
      initNull || isIntType(tv.m_type) ||
      isDoubleType(tv.m_type) || isBoolType(tv.m_type);

  case T::InitCell:
    if (tv.m_type == KindOfUninit) return false;
    // fallthrough
  case T::Cell:
    return true;
  }
  not_reached();
}

std::string show(RepoAuthType rat) {
  auto const tag = rat.tag();
  using T = RepoAuthType::Tag;
  switch (tag) {
  case T::OptBool:       return "?Bool";
  case T::UninitBool:    return "UninitBool";
  case T::OptInt:        return "?Int";
  case T::UninitInt:     return "UninitInt";
  case T::OptSStr:       return "?SStr";
  case T::UninitSStr:    return "UninitSStr";
  case T::OptStr:        return "?Str";
  case T::UninitStr:     return "UninitStr";
  case T::OptDbl:        return "?Dbl";
  case T::OptRes:        return "?Res";
  case T::OptObj:        return "?Obj";
  case T::UninitObj:     return "UninitObj";
  case T::OptFunc:       return "?Func";
  case T::OptCls:        return "?Cls";
  case T::OptClsMeth:    return "?ClsMeth";
  case T::OptRecord:     return "?Record";
  case T::OptLazyCls:    return "?LazyCls";
  case T::OptUncArrKey:  return "?UncArrKey";
  case T::OptArrKey:     return "?ArrKey";
  case T::OptUncStrLike: return "?UncStrLike";
  case T::OptStrLike:    return "?StrLike";
  case T::OptUncArrKeyCompat: return "?UncArrKeyCompat";
  case T::OptArrKeyCompat: return "?ArrKeyCompat";
  case T::Null:          return "Null";
  case T::Cell:          return "Cell";
  case T::InitUnc:       return "InitUnc";
  case T::Unc:           return "Unc";
  case T::UncArrKey:     return "UncArrKey";
  case T::ArrKey:        return "ArrKey";
  case T::UncStrLike:    return "UncStrLike";
  case T::StrLike:       return "StrLike";
  case T::UncArrKeyCompat: return "UncArrKeyCompat";
  case T::ArrKeyCompat:    return "ArrKeyCompat";
  case T::InitCell:      return "InitCell";
  case T::Uninit:        return "Uninit";
  case T::InitNull:      return "InitNull";
  case T::Bool:          return "Bool";
  case T::Int:           return "Int";
  case T::Dbl:           return "Dbl";
  case T::Res:           return "Res";
  case T::SStr:          return "SStr";
  case T::Str:           return "Str";
  case T::Obj:           return "Obj";
  case T::Func:          return "Func";
  case T::Cls:           return "Cls";
  case T::ClsMeth:       return "ClsMeth";
  case T::Record:        return "Record";
  case T::LazyCls:       return "LazyCls";
  case T::Num:           return "Num";
  case T::OptNum:        return "OptNum";
  case T::InitPrim:      return "InitPrim";
  case T::NonNull:       return "NonNull";

  case T::SVec:
  case T::Vec:
  case T::OptSVec:
  case T::OptVec:
  case T::SDict:
  case T::Dict:
  case T::OptSDict:
  case T::OptDict:
  case T::SKeyset:
  case T::Keyset:
  case T::OptSKeyset:
  case T::OptKeyset:
  case T::SArrLike:
  case T::ArrLike:
  case T::OptSArrLike:
  case T::OptArrLike:
  case T::VecCompat:
  case T::OptVecCompat:
  case T::ArrLikeCompat:
  case T::OptArrLikeCompat:
    {
      auto ret = std::string{};
      if (tag == T::OptVec    || tag == T::OptSVec ||
          tag == T::OptDict   || tag == T::OptSDict ||
          tag == T::OptKeyset || tag == T::OptSKeyset ||
          tag == T::OptArrLike || tag == T::OptSArrLike ||
          tag == T::OptArrLikeCompat) {
        ret += '?';
      }
      if (tag == T::VecCompat || tag == T::OptVecCompat) {
        ret += "VecCompat";
      }
      if (tag == T::ArrLikeCompat || tag == T::OptArrLikeCompat) {
        ret += "ArrLikeCompat";
      }
      if (tag == T::SVec    || tag == T::OptSVec ||
          tag == T::SDict   || tag == T::OptSDict ||
          tag == T::SKeyset || tag == T::OptSKeyset ||
          tag == T::SArrLike || tag == T::OptSArrLike) {
        ret += 'S';
      }
      if (tag == T::OptVec  || tag == T::Vec ||
          tag == T::OptSVec || tag == T::SVec) {
        ret += "Vec";
      } else if (tag == T::OptDict  || tag == T::Dict ||
                 tag == T::OptSDict || tag == T::SDict) {
        ret += "Dict";
      } else if (tag == T::OptKeyset  || tag == T::Keyset ||
                 tag == T::OptSKeyset || tag == T::SKeyset) {
        ret += "Keyset";
      } else if (tag == T::OptArrLike || tag == T::ArrLike ||
                 tag == T::OptSArrLike || tag == T::SArrLike) {
        ret += "ArrLike";
      }
      if (rat.hasArrData()) folly::format(&ret, "{}", show(*rat.array()));
      return ret;
    }
    break;

  case T::OptSubObj:
  case T::OptExactObj:
  case T::UninitSubObj:
  case T::UninitExactObj:
  case T::SubObj:
  case T::ExactObj:
  case T::OptSubCls:
  case T::OptExactCls:
  case T::SubCls:
  case T::ExactCls:
    {
      auto ret = std::string{};
      if (tag == T::OptSubObj || tag == T::OptExactObj ||
          tag == T::OptSubCls || tag == T::OptExactCls) {
        ret += '?';
      }
      if (tag == T::OptSubObj || tag == T::OptExactObj ||
          tag == T::SubObj || tag == T::ExactObj) {
        ret += "Obj";
      } else if (tag == T::UninitSubObj || tag == T::UninitExactObj) {
        ret += "UninitObj";
      } else {
        ret += "Cls";
      }
      if (tag == T::OptSubObj || tag == T::UninitSubObj || tag == T::SubObj ||
          tag == T::OptSubCls || tag == T::SubCls) {
        ret += "<";
      }
      ret += '=';
      ret += rat.clsName()->data();
      return ret;
    }
  case T::OptSubRecord:
  case T::OptExactRecord:
  case T::SubRecord:
  case T::ExactRecord:
    {
      auto ret = std::string{};
      if (tag == T::OptSubRecord || tag == T::OptExactRecord) ret += '?';
      ret += "Record";
      if (tag == T::OptSubRecord || tag == T::SubRecord) ret += '<';
      ret += '=';
      ret += rat.recordName()->data();
      return ret;
    }
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}

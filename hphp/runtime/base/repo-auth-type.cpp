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
#include "hphp/runtime/vm/blob-helper.h"
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
  if (arrTy->tag() == A::Tag::Tuple && ad->size() != arrTy->size()) {
    return false;
  }

  // O(N) checks are available if you want them for debugging, but
  // they are too slow for general use in debug builds.  These type
  // matching functions are currently only used for assertions, so
  // it's ok to leave them out.
  if (false) {
    switch (arrTy->tag()) {
      case A::Tag::Tuple:
        if (!ad->isVectorData()) return false;
        for (auto i = uint32_t{0}; i < ad->size(); ++i) {
          auto const elem = ad->at(i);
          if (!tvMatchesRepoAuthType(elem, arrTy->tupleElem(i))) {
            return false;
          }
        }
        break;
      case A::Tag::Packed:
        if (!ad->isVectorData()) return false;
        for (auto i = uint32_t{0}; i < ad->size(); ++i) {
          auto const elem = ad->at(i);
          if (!tvMatchesRepoAuthType(elem, arrTy->packedElems())) {
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

bool RepoAuthType::operator==(RepoAuthType o) const {
  if (tag() != o.tag()) return false;
  if (name() != o.name()) return false;
  if (array() != o.array()) return false;
  return true;
}

size_t RepoAuthType::hash() const {
  auto const iTag = static_cast<size_t>(tag());
  if (auto const n = name()) {
    return folly::hash::hash_128_to_64(iTag, n->hash());
  }
  if (auto const a = array()) {
    return folly::hash::hash_128_to_64(iTag, a->id());
  }
  return iTag;
}

//////////////////////////////////////////////////////////////////////

bool tvMatchesRepoAuthType(TypedValue tv, RepoAuthType ty) {
  assertx(tvIsPlausible(tv));

  auto const isUninit = tv.m_type == KindOfUninit;
  auto const isNull = tv.m_type == KindOfNull;

  auto const base = [&] {
    using T = RepoAuthType::Tag;

    #define O(type, check)                         \
      case T::Opt##type: if (isNull) return true;  \
      case T::type:      return check(tv.m_type);  \

    #define U(type, check)                                       \
      case T::Uninit##type: return isUninit || check(tv.m_type); \
      O(type, check)                                             \

    #define S(type, check, ptr)                                                 \
      case T::Opt##type:  if (isNull) return true;                              \
      case T::type:       return check(tv.m_type);                              \
      case T::OptS##type: if (isNull) return true;                              \
      case T::S##type:    return check(tv.m_type) && tv.m_data.ptr->isStatic(); \

    #define N(type, check)                    \
      O(type,        check)                   \
      O(Exact##type, check)                   \
      O(Sub##type,   check)                   \

    #define UN(type, check)                   \
      U(type,        check)                   \
      U(Exact##type, check)                   \
      U(Sub##type,   check)                   \

    #define US(type, check, ptr)                                            \
      case T::Uninit##type: return isUninit || check(tv.m_type);            \
      case T::UninitS##type:                                                \
        return isUninit || (check(tv.m_type) && tv.m_data.ptr->isStatic()); \
      S(type, check, ptr)                                                   \

    #define OU(type, check1, check2)                                    \
      O(type, check1)                                                   \
      case T::OptUnc##type: if (isNull) return true;                    \
      case T::Unc##type:    return check1(tv.m_type) && !check2(tv);    \

    #define A(type, check, ptr)               \
      S(type, check, ptr)                     \
      S(type##Spec, check, ptr)               \

    auto const isNumType = [] (DataType t) {
      return isIntType(t) || isDoubleType(t);
    };
    auto const isInitPrimType = [&] (DataType t) {
      return isNull || isNumType(t) || isBoolType(t);
    };
    auto const isStrLikeType = [] (DataType t) {
      return isStringType(t) || isClassType(t) || isLazyClassType(t);
    };
    auto const isArrLikeCompatType = [] (DataType t) {
      return isArrayLikeType(t) || isClsMethType(t);
    };
    auto const isVecCompatType = [] (DataType t) {
      return isVecType(t) || isClsMethType(t);
    };
    auto const isArrKeyType = [] (DataType t) {
      return isIntType(t) || isStringType(t);
    };
    auto const isArrKeyCompatType = [&] (DataType t) {
      return isArrKeyType(t) || isClassType(t) || isLazyClassType(t);
    };

    auto const isUncounted = [] (const TypedValue& val) {
      return !isRefcountedType(val.m_type) ||
        (val.m_type == KindOfString && val.m_data.pstr->isStatic()) ||
        (isArrayLikeType(val.m_type) && val.m_data.parr->isStatic());
    };

    auto const isCountedStr = [] (const TypedValue& val) {
      return isStringType(val.m_type) && val.m_data.pstr->isRefCounted();
    };

    switch (ty.tag()) {
      U(Bool,          isBoolType)
      U(Int,           isIntType)
      O(Dbl,           isDoubleType)
      O(Res,           isResourceType)
      O(Func,          isFuncType)
      O(ClsMeth,       isClsMethType)
      O(LazyCls,       isLazyClassType)
      O(Num,           isNumType)
      O(VecCompat,     isVecCompatType)
      O(ArrLikeCompat, isArrLikeCompatType)
      N(Cls,           isClassType)
      UN(Obj,          isObjectType)
      OU(ArrKey,       isArrKeyType,       isCountedStr)
      OU(StrLike,      isStrLikeType,      isCountedStr)
      OU(ArrKeyCompat, isArrKeyCompatType, isCountedStr)
      US(Str,          isStringType,       pstr)
      S(ArrLike,       isArrayLikeType,    parr)
      A(Vec,           isVecType,          parr)
      A(Dict,          isDictType,         parr)
      A(Keyset,        isKeysetType,       parr)
      case T::Uninit:   return isUninit;
      case T::InitNull: return isNull;
      case T::Null:     return isNull || isUninit;
      case T::InitPrim: return isInitPrimType(tv.m_type);
      case T::NonNull:  return !isUninit && !isNull;
      case T::InitUnc:  return !isUninit && isUncounted(tv);
      case T::Unc:      return isUncounted(tv);
      case T::InitCell: return !isUninit;
      case T::Cell:     return true;
    }
    not_reached();

    #undef A
    #undef OU
    #undef US
    #undef UN
    #undef N
    #undef S
    #undef U
    #undef O
  }();
  if (!base) return false;
  if (isUninit || isNull) return true;

  if (auto const name = ty.name()) {
    auto const cls = Class::lookup(name);
    if (!cls) return false;
    auto const tvCls = [&] {
      if (tvIsObject(tv)) return tv.m_data.pobj->getVMClass();
      assertx(tvIsClass(tv));
      return tv.m_data.pclass;
    }();
    if (RepoAuthType::tagIsSubName(ty.tag())) {
      return tvCls->classof(cls);
    }
    assertx(RepoAuthType::tagIsExactName(ty.tag()));
    return tvCls == cls;
  }

  if (auto const array = ty.array()) {
    return tvMatchesArrayType(tv, array);
  }

  return true;
}

std::string show(RepoAuthType rat) {
  auto const base = [&] {
    using T = RepoAuthType::Tag;
    switch (rat.tag()) {
      #define TAG(x, name) case T::x: return name;
      REPO_AUTH_TYPE_TAGS(TAG)
      #undef TAG
    }
    not_reached();
  }();
  if (auto const n = rat.name()) {
    return folly::sformat("{}{}", base, n);
  } else if (auto const a = rat.array()) {
    return folly::sformat("{}{}", base, show(*a));
  }
  return base;
}

//////////////////////////////////////////////////////////////////////

template <typename SerDe>
void RepoAuthType::serde(SerDe& sd) {
  if constexpr (SerDe::deserializing) {
    Tag t;
    sd(t);
    if (tagHasArrData(t)) {
      ArrayTypeTable::Id id;
      sd(id);
      m_data.set(t, globalArrayTypeTable().lookup(id));
    } else if (tagHasName(t)) {
      LowStringPtr name;
      sd(name);
      m_data.set(t, name.get());
    } else {
      m_data.set(t, nullptr);
    }
  } else {
    sd(tag());
    if (auto const a = array()) {
      sd(a->id());
    } else if (auto const n = name()) {
      sd(LowStringPtr{n});
    }
  }
}

template void RepoAuthType::serde<>(BlobDecoder&);
template void RepoAuthType::serde<>(BlobEncoder&);

//////////////////////////////////////////////////////////////////////

}

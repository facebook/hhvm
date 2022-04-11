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

#include "hphp/runtime/vm/jit/type-array-elem.h"

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/vanilla-dict-defs.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/base/vanilla-keyset.h"
#include "hphp/runtime/base/vanilla-vec-defs.h"
#include "hphp/runtime/base/vanilla-vec.h"

#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/type-specialization.h"

#include "hphp/util/assertions.h"

namespace HPHP::jit {

///////////////////////////////////////////////////////////////////////////////

namespace {
std::pair<Type, bool> vecElemType(Type arr, Type idx, const Class* ctx) {
  assertx(arr <= TVec);
  assertx(idx <= TInt);

  if (idx.hasConstVal()) {
    auto const idxVal = idx.intVal();
    if (idxVal < 0 || idxVal > VanillaDict::MaxSize) return {TBottom, false};
  }

  if (arr.hasConstVal()) {
    // If both the array and idx are known statically, we can resolve it to the
    // precise type.
    if (idx.hasConstVal()) {
      auto const tv = arr.arrLikeVal()->get(idx.intVal());
      if (tv.is_init()) return {Type::cns(tv), true};
      return {TBottom, false};
    }

    // Otherwise we can constrain the type according to the union of all the
    // types present in the vec/varray.
    auto type = TBottom;
    IterateV(
      arr.arrLikeVal(),
      [&](TypedValue v) { type |= Type::cns(v); }
    );
    return {type, false};
  }

  // Array-likes always contain initialized cells.
  auto type = arr <= TUncounted ? TUncountedInit : TInitCell;

  auto const arrTy = arr.arrSpec().type();
  if (!arrTy) return {type, false};

  using E = RepoAuthType::Array::Empty;
  using T = RepoAuthType::Array::Tag;
  auto const maybeEmpty = arrTy->emptiness() == E::Maybe;

  switch (arrTy->tag()) {
    case T::Tuple: {
      if (idx.hasConstVal(TInt)) {
        auto const intIdx = idx.intVal();
        if (intIdx < 0 || intIdx >= arrTy->size()) return {TBottom, false};
        type &= typeFromRAT(arrTy->tupleElem(intIdx), ctx);
        return {type, !maybeEmpty};
      }
      auto all = TBottom;
      for (auto i = 0; i < arrTy->size(); ++i) {
        all |= typeFromRAT(arrTy->tupleElem(i), ctx);
      }
      type &= all;
      break;
    }
    case T::Packed: {
      auto const present =
        idx.hasConstVal(TInt) && !maybeEmpty && idx.intVal() == 0;
      type &= typeFromRAT(arrTy->packedElems(), ctx);
      return {type, present};
    }
  }

  return {type, false};
}

std::pair<Type, bool> dictElemType(Type arr, Type idx) {
  assertx(arr <= TDict);
  assertx(idx <= (TInt | TStr));

  if (arr.hasConstVal()) {
    // If both the array and idx are known statically, we can resolve it to the
    // precise type.
    if (idx.hasConstVal(TInt)) {
      auto const tv = arr.arrLikeVal()->get(idx.intVal());
      if (tv.is_init()) return {Type::cns(tv), true};
      return {TBottom, false};
    } else if (idx.hasConstVal(TStr)) {
      auto const tv = arr.arrLikeVal()->get(idx.strVal());
      if (tv.is_init()) return {Type::cns(tv), true};
      return {TBottom, false};
    }

    // Otherwise we can constrain the type according to the union of all the
    // types present in the dict.
    auto type = TBottom;
    IterateKV(
      arr.arrLikeVal(),
      [&](TypedValue k, TypedValue v) {
        // Ignore values which can't correspond to the key's type
        if (isIntType(k.m_type)) {
          if (idx.maybe(TInt)) type |= Type::cns(v);
        } else if (isStringType(k.m_type)) {
          if (idx.maybe(TStr)) type |= Type::cns(v);
        }
      }
    );
    return {type, false};
  }

  // Array-likes always contain initialized cells.
  auto const type = arr <= TUncounted ? TUncountedInit : TInitCell;

  return {type, false};
}

std::pair<Type, bool> keysetElemType(Type arr, Type idx) {
  assertx(arr <= TKeyset);
  assertx(idx <= (TInt | TStr));

  if (arr.hasConstVal()) {
    // If both the array and idx are known statically, we can resolve it to the
    // precise type.
    if (idx.hasConstVal(TInt)) {
      auto const tv = arr.arrLikeVal()->get(idx.intVal());
      if (tv.is_init()) return {Type::cns(tv), true};
      return {TBottom, false};
    } else if (idx.hasConstVal(TStr)) {
      auto const tv = arr.arrLikeVal()->get(idx.strVal());
      if (tv.is_init()) return {Type::cns(tv), true};
      return {TBottom, false};
    }

    // Otherwise we can constrain the type according to the union of all the
    // types present in the keyset.
    auto type = TBottom;
    IterateV(
      arr.arrLikeVal(),
      [&](TypedValue v) {
        // Ignore values which can't correspond to the key's type
        if (isIntType(v.m_type)) {
          if (idx.maybe(TInt)) type |= Type::cns(v);
        } else {
          assertx(isStringType(v.m_type));
          if (idx.maybe(TStr)) type |= Type::cns(v);
        }
      }
    );
    return {type, false};
  }

  // Keysets always contain strings or integers. We can further constrain this
  // if we know the idx type, as the key is always the value.
  auto const type = arr <= TUncounted ? TUncountedInit : TInitCell;
  return {type & idx, false};
}

std::pair<Type, bool> vecFirstLastValueType(Type arr,
                                            bool isFirst,
                                            const Class* ctx) {
  assertx(arr <= TVec);

  if (arr.hasConstVal()) {
    auto const val = arr.arrLikeVal();
    if (val->empty()) return {TBottom, false};
    auto const pos = isFirst ? val->iter_begin() : val->iter_last();
    return {Type::cns(val->nvGetVal(pos)), true};
  }

  auto type = arr <= TUncounted ? TUncountedInit : TInitCell;

  auto const arrTy = arr.arrSpec().type();
  if (!arrTy) return {type, false};

  using E = RepoAuthType::Array::Empty;
  using T = RepoAuthType::Array::Tag;
  auto const maybeEmpty = arrTy->emptiness() == E::Maybe;

  switch (arrTy->tag()) {
    case T::Tuple: {
      auto const sz = arrTy->size();
      if (sz == 0) return {TBottom, false};
      type = type & typeFromRAT(arrTy->tupleElem(isFirst ? 0 : sz - 1), ctx);
      break;
    }
    case T::Packed: {
      type = type & typeFromRAT(arrTy->packedElems(), ctx);
      break;
    }
  }
  return {type, !maybeEmpty};
}

std::pair<Type, bool> vecFirstLastKeyType(Type arr, bool isFirst) {
  assertx(arr <= TVec);

  if (arr.hasConstVal()) {
    auto const val = arr.arrLikeVal();
    if (val->empty()) return {TBottom, false};
    auto const pos = isFirst ? val->iter_begin() : val->iter_last();
    return {Type::cns(val->nvGetKey(pos)), true};
  }

  auto const arrTy = arr.arrSpec().type();
  if (!arrTy) return {TInt, false};

  using E = RepoAuthType::Array::Empty;
  using T = RepoAuthType::Array::Tag;

  auto const maybeEmpty = arrTy->emptiness() == E::Maybe;

  if (arrTy->tag() == T::Tuple) {
    auto const sz = arrTy->size();
    if (sz == 0) return {TBottom, false};
    return {Type::cns(isFirst ? 0 : sz - 1), !maybeEmpty};
  }

  return {isFirst ? Type::cns(0) : TInt, !maybeEmpty};
}

std::pair<Type, bool> dictFirstLastType(Type arr, bool isFirst, bool isKey) {
  assertx(arr <= TDict);

  if (arr.hasConstVal()) {
    auto const val = arr.arrLikeVal();
    if (val->empty()) return {TBottom, false};
    auto const pos = isFirst ? val->iter_begin() : val->iter_last();
    auto const tv = isKey ? val->nvGetKey(pos) : val->nvGetVal(pos);
    return {Type::cns(tv), true};
  }

  auto const type = arr <= TUncounted ? TUncountedInit : TInitCell;

  auto const arrTy = arr.arrSpec().type();
  if (!arrTy) return {type, false};

  using E = RepoAuthType::Array::Empty;
  auto const maybeEmpty = arrTy->emptiness() == E::Maybe;

  return {type, !maybeEmpty};
}

std::pair<Type, bool> keysetFirstLastType(Type arr, bool isFirst) {
  assertx(arr <= TKeyset);

  if (arr.hasConstVal()) {
    auto const val = arr.arrLikeVal();
    if (val->empty()) return {TBottom, false};
    auto const pos = isFirst ? val->iter_begin() : val->iter_last();
    return {Type::cns(val->nvGetVal(pos)), true};
  }

  auto type = TStr | TInt;
  if (arr <= TUncounted) type &= TUncountedInit;

  auto const arrTy = arr.arrSpec().type();
  if (!arrTy) return {type, false};

  using E = RepoAuthType::Array::Empty;
  auto const maybeEmpty = arrTy->emptiness() == E::Maybe;

  return {type, !maybeEmpty};
}

Type dictPosType(Type arr, Type pos, bool isKey, const Class* ctx) {
  assertx(arr <= TDict);
  assertx(pos <= TInt);

  if (arr.hasConstVal()) {
    if (pos.hasConstVal()) {
      auto const val = arr.arrLikeVal();
      if (val->empty()) return TBottom;
      auto const idx = pos.intVal();
      if (!val->posIsValid(idx)) return TBottom;
      return Type::cns(isKey ? val->nvGetKey(idx) : val->nvGetVal(idx));
    }

    // Otherwise we can constrain the type according to the union of
    // all the types present in the dict.
    auto type = TBottom;
    IterateKV(
      arr.arrLikeVal(),
      [&] (TypedValue k, TypedValue v) {
        type |= (isKey ? Type::cns(k) : Type::cns(v));
      }
    );
    return type;
  }

  auto type = arr <= TUncounted ? TUncountedInit : TInitCell;
  if (isKey) type &= (TInt | TStr);

  auto const arrTy = arr.arrSpec().type();
  if (!arrTy) return type;

  using T = RepoAuthType::Array::Tag;
  switch (arrTy->tag()) {
    case T::Tuple: {
      auto const sz = arrTy->size();
      if (sz == 0) return TBottom;
      if (pos.hasConstVal()) {
        auto const idx = pos.intVal();
        if (idx < 0 || idx >= arrTy->size()) return TBottom;
        type &= (isKey ? pos : typeFromRAT(arrTy->tupleElem(idx), ctx));
        return type;
      }
      auto all = TBottom;
      for (auto i = 0; i < arrTy->size(); ++i) {
        all |= (isKey ? Type::cns(i) : typeFromRAT(arrTy->tupleElem(i), ctx));
      }
      type &= all;
      break;
    }
    case T::Packed:
      if (pos.hasConstVal()) {
        auto const idx = pos.intVal();
        if (idx < 0 || idx > VanillaDict::MaxSize) return TBottom;
      }
      type &= (isKey ? pos : typeFromRAT(arrTy->packedElems(), ctx));
      break;
  }

  return type;
}

Type keysetPosType(Type arr, Type pos, const Class* ctx) {
  assertx(arr <= TKeyset);
  assertx(pos <= TInt);

  if (arr.hasConstVal()) {
    if (pos.hasConstVal()) {
      auto const val = arr.arrLikeVal();
      if (val->empty()) return TBottom;
      auto const idx = pos.intVal();
      if (!val->posIsValid(idx)) return TBottom;
      return Type::cns(val->nvGetKey(idx));
    }

    auto type = TBottom;
    IterateKV(
      arr.arrLikeVal(),
      [&] (TypedValue k, TypedValue) { type |= Type::cns(k); }
    );
  }

  auto type = TStr | TInt;
  if (arr <= TUncounted) type &= TUncountedInit;

  auto const arrTy = arr.arrSpec().type();
  if (!arrTy) return type;

  using T = RepoAuthType::Array::Tag;
  switch (arrTy->tag()) {
    case T::Tuple: {
      auto const sz = arrTy->size();
      if (sz == 0) return TBottom;
      if (pos.hasConstVal()) {
        auto const idx = pos.intVal();
        if (idx < 0 || idx >= arrTy->size()) return TBottom;
      }
      type &= pos;
      break;
    }
    case T::Packed:
      type &= pos;
      break;
  }

  return type;
}

}

///////////////////////////////////////////////////////////////////////////////

std::pair<Type, bool> arrLikeElemType(Type arr, Type idx, const Class* ctx) {
  assertx(idx <= (TInt | TStr));
  assertx(arr.isKnownDataType());
  auto const ratDeducedType = [&] () -> std::pair<Type, bool> {
    if (arr <= TBottom) return {TBottom, false};
    if (arr <= TVec)    return vecElemType(arr, idx, ctx);
    if (arr <= TDict)   return dictElemType(arr, idx);
    return keysetElemType(arr, idx);
  }();
  auto const layoutType = arr.arrSpec().layout().elemType(idx);
  return {ratDeducedType.first & layoutType.first,
          ratDeducedType.second || layoutType.second};
}

std::pair<Type, bool> arrLikeFirstLastType(
    Type arr, bool isFirst, bool isKey, const Class* ctx) {
  assertx(arr.isKnownDataType());
  auto const ratDeducedType = [&] () -> std::pair<Type, bool> {
    if (arr <= TBottom) return {TBottom, false};
    if (arr <= TVec) {
      return isKey ? vecFirstLastKeyType(arr, isFirst)
                   : vecFirstLastValueType(arr, isFirst, ctx);
    }
    if (arr <= TDict) return dictFirstLastType(arr, isFirst, isKey);
    return keysetFirstLastType(arr, isFirst);
  }();
  auto const layoutType = arr.arrSpec().layout().firstLastType(isFirst, isKey);
  return {ratDeducedType.first & layoutType.first,
          ratDeducedType.second || layoutType.second};
}

Type arrLikePosType(Type arr, Type pos, bool isKey, const Class* ctx) {
  assertx(arr.isKnownDataType());
  assertx(pos <= TInt);
  auto const deducedType = [&] {
    if (arr <= TVec)  return isKey ? pos : vecElemType(arr, pos, ctx).first;
    if (arr <= TDict) return dictPosType(arr, pos, isKey, ctx);
    return keysetPosType(arr, pos, ctx);
  }();
  auto const layoutType = arr.arrSpec().layout().iterPosType(pos, isKey);
  return deducedType & layoutType;
}

///////////////////////////////////////////////////////////////////////////////

VecBounds vecBoundsStaticCheck(Type arrayType, Optional<int64_t> idx) {
  assertx(arrayType <= TVec);
  if (idx && (*idx < 0 || *idx > VanillaDict::MaxSize)) return VecBounds::Out;

  if (arrayType.hasConstVal()) {
    auto const val = arrayType.arrLikeVal();
    if (val->empty()) return VecBounds::Out;
    if (!idx) return VecBounds::Unknown;
    return *idx < val->size() ? VecBounds::In : VecBounds::Out;
  };

  if (!idx) return VecBounds::Unknown;

  auto const at = arrayType.arrSpec().type();
  if (!at) return VecBounds::Unknown;

  using A = RepoAuthType::Array;
  switch (at->tag()) {
  case A::Tag::Tuple:
    if (at->emptiness() == A::Empty::No) {
      return *idx < at->size() ? VecBounds::In : VecBounds::Out;
    } else if (*idx >= at->size()) {
      return VecBounds::Out;
    }
    return VecBounds::Unknown;
  case A::Tag::Packed:
    if (*idx == 0 && at->emptiness() == A::Empty::No) {
      return VecBounds::In;
    }
  }
  return VecBounds::Unknown;
}

///////////////////////////////////////////////////////////////////////////////

}

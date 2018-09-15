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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/repo-auth-type-array.h"

#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/type-specialization.h"

#include "hphp/util/assertions.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

PackedBounds packedArrayBoundsStaticCheck(Type arrayType,
                                          folly::Optional<int64_t> idx) {
  assertx(arrayType.subtypeOfAny(TArr, TVec));
  if (idx && (*idx < 0 || *idx > MixedArray::MaxSize)) return PackedBounds::Out;

  auto const const_check = [&] (const ArrayData* val) {
    assertx(val->hasPackedLayout());
    if (val->empty()) return PackedBounds::Out;
    if (!idx) return PackedBounds::Unknown;
    return *idx < val->size() ? PackedBounds::In : PackedBounds::Out;
  };
  if (arrayType.hasConstVal(TArr)) return const_check(arrayType.arrVal());
  if (arrayType.hasConstVal(TVec)) return const_check(arrayType.vecVal());

  if (!idx) return PackedBounds::Unknown;

  auto const at = arrayType.arrSpec().type();
  if (!at) return PackedBounds::Unknown;

  using A = RepoAuthType::Array;
  switch (at->tag()) {
  case A::Tag::Packed:
    if (at->emptiness() == A::Empty::No) {
      return *idx < at->size() ? PackedBounds::In : PackedBounds::Out;
    } else if (*idx >= at->size()) {
      return PackedBounds::Out;
    }
    return PackedBounds::Unknown;
  case A::Tag::PackedN:
    if (*idx == 0 && at->emptiness() == A::Empty::No) {
      return PackedBounds::In;
    }
  }
  return PackedBounds::Unknown;
}

///////////////////////////////////////////////////////////////////////////////

std::pair<Type, bool> arrElemType(Type arr, Type idx, const Class* ctx) {
  assertx(arr <= TArr);

  auto const dissected = [&]{
    if (idx.hasConstVal(TStr)) {
      int64_t val;
      if (idx.strVal()->isStrictlyInteger(val)) return Type::cns(val);
      return idx;
    }
    if (idx <= TInt) return idx;
    return TInt | TStr;
  }();

  if (arr.hasConstVal()) {
    if (dissected.hasConstVal(TInt)) {
      auto const rval = arr.arrVal()->rval(dissected.intVal());
      if (rval) return {Type::cns(rval.tv()), true};
      return {TBottom, false};
    }

    if (dissected.hasConstVal(TStr)) {
      auto const rval = arr.arrVal()->rval(dissected.strVal());
      if (rval) return {Type::cns(rval.tv()), true};
      return {TBottom, false};
    }

    auto type = TBottom;
    IterateKV(
      arr.arrVal(),
      [&](Cell k, TypedValue v) {
        // Ignore values which can't correspond to the key's type
        if (isIntType(k.m_type)) {
          if (dissected.maybe(TInt)) type |= Type::cns(v);
        } else if (isStringType(k.m_type)) {
          if (dissected.maybe(TStr)) type |= Type::cns(v);
        }
      }
    );
    return {type, false};
  }

  if (arr <= Type::Array(ArrayData::kPackedKind)) {
    if (!dissected.maybe(TInt)) return {TBottom, false};
    if (dissected.hasConstVal(TInt)) {
      auto const intIdx = dissected.intVal();
      if (intIdx < 0 || intIdx > MixedArray::MaxSize) return {TBottom, false};
    }
  }

  auto type = (arr <= TPersistentArr) ? TUncountedInit : TInitGen;

  auto const arrTy = arr.arrSpec().type();
  if (!arrTy) return {type, false};

  using E = RepoAuthType::Array::Empty;
  using T = RepoAuthType::Array::Tag;
  auto const maybeEmpty = arrTy->emptiness() == E::Maybe;

  switch (arrTy->tag()) {
    case T::Packed: {
      if (!dissected.maybe(TInt)) return {TBottom, false};
      if (dissected.hasConstVal(TInt)) {
        auto const intIdx = dissected.intVal();
        if (intIdx < 0 || intIdx >= arrTy->size()) return {TBottom, false};
        type &= typeFromRAT(arrTy->packedElem(intIdx), ctx);
        return {type, !maybeEmpty};
      }
      auto all = TBottom;
      for (auto i = 0; i < arrTy->size(); ++i) {
        all |= typeFromRAT(arrTy->packedElem(i), ctx);
      }
      type &= all;
      break;
    }
    case T::PackedN: {
      auto present = false;
      if (!dissected.maybe(TInt)) return {TBottom, false};
      if (dissected.hasConstVal(TInt)) {
        auto const intIdx = dissected.intVal();
        if (intIdx < 0 || intIdx > MixedArray::MaxSize) {
          return {TBottom, false};
        }
        present = !maybeEmpty && (intIdx == 0);
      }
      type &= typeFromRAT(arrTy->elemType(), ctx);
      return {type, present};
    }
  }

  return {type, false};
}

std::pair<Type, bool> vecElemType(Type arr, Type idx, const Class* ctx) {
  assertx(arr <= TVec);
  assertx(idx <= TInt);

  if (idx.hasConstVal()) {
    auto const idxVal = idx.intVal();
    if (idxVal < 0 || idxVal > MixedArray::MaxSize) return {TBottom, false};
  }

  if (arr.hasConstVal()) {
    // If both the array and idx are known statically, we can resolve it to the
    // precise type.
    if (idx.hasConstVal()) {
      auto const idxVal = idx.intVal();
      if (idxVal >= 0 && idxVal < arr.vecVal()->size()) {
        auto const rval = PackedArray::RvalIntVec(arr.vecVal(), idxVal);
        return {Type::cns(rval.tv()), true};
      }
      return {TBottom, false};
    }

    // Otherwise we can constrain the type according to the union of all the
    // types present in the vec.
    auto type = TBottom;
    PackedArray::IterateV(
      arr.vecVal(),
      [&](TypedValue v) { type |= Type::cns(v); }
    );
    return {type, false};
  }

  // Vecs always contain initialized cells
  auto type = (arr <= TPersistentVec) ? TUncountedInit : TInitCell;

  auto const arrTy = arr.arrSpec().type();
  if (!arrTy) return {type, false};

  using E = RepoAuthType::Array::Empty;
  using T = RepoAuthType::Array::Tag;
  auto const maybeEmpty = arrTy->emptiness() == E::Maybe;

  switch (arrTy->tag()) {
    case T::Packed: {
      if (idx.hasConstVal(TInt)) {
        auto const intIdx = idx.intVal();
        if (intIdx < 0 || intIdx >= arrTy->size()) return {TBottom, false};
        type &= typeFromRAT(arrTy->packedElem(intIdx), ctx);
        return {type, !maybeEmpty};
      }
      auto all = TBottom;
      for (auto i = 0; i < arrTy->size(); ++i) {
        all |= typeFromRAT(arrTy->packedElem(i), ctx);
      }
      type &= all;
      break;
    }
    case T::PackedN: {
      auto const present =
        idx.hasConstVal(TInt) && !maybeEmpty && idx.intVal() == 0;
      type &= typeFromRAT(arrTy->elemType(), ctx);
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
      auto const idxVal = idx.intVal();
      auto const rval = MixedArray::RvalIntDict(arr.dictVal(), idxVal);
      if (rval) return {Type::cns(rval.tv()), true};
      return {TBottom, false};
    }

    if (idx.hasConstVal(TStr)) {
      auto const idxVal = idx.strVal();
      auto const rval = MixedArray::RvalStrDict(arr.dictVal(), idxVal);
      if (rval) return {Type::cns(rval.tv()), true};
      return {TBottom, false};
    }

    // Otherwise we can constrain the type according to the union of all the
    // types present in the dict.
    auto type = TBottom;
    MixedArray::IterateKV(
      MixedArray::asMixed(arr.dictVal()),
      [&](Cell k, TypedValue v) {
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

  // Dicts always contain initialized cells
  auto const type = (arr <= TPersistentDict) ? TUncountedInit : TInitCell;
  return {type, false};
}

std::pair<Type, bool> keysetElemType(Type arr, Type idx) {
  assertx(arr <= TKeyset);
  assertx(idx <= (TInt | TStr));

  if (arr.hasConstVal()) {
    // If both the array and idx are known statically, we can resolve it to the
    // precise type.
    if (idx.hasConstVal(TInt)) {
      auto const idxVal = idx.intVal();
      auto const rval = SetArray::RvalInt(arr.keysetVal(), idxVal);
      if (rval) return {Type::cns(rval.tv()), true};
      return {TBottom, false};
    }

    if (idx.hasConstVal(TStr)) {
      auto const idxVal = idx.strVal();
      auto const rval = SetArray::RvalStr(arr.keysetVal(), idxVal);
      if (rval) return {Type::cns(rval.tv()), true};
      return {TBottom, false};
    }

    // Otherwise we can constrain the type according to the union of all the
    // types present in the keyset.
    auto type = TBottom;
    SetArray::Iterate(
      SetArray::asSet(arr.keysetVal()),
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
  auto type = TStr | TInt;
  if (idx <= TInt) type &= TInt;
  if (idx <= TStr) type &= TStr;
  if (arr <= TPersistentKeyset) type &= TUncountedInit;
  return {type, false};
}

///////////////////////////////////////////////////////////////////////////////

}}

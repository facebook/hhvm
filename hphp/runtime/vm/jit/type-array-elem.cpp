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

PackedBounds packedArrayBoundsStaticCheck(Type arrayType, int64_t idxVal) {
  if (idxVal < 0 || idxVal > MixedArray::MaxSize) return PackedBounds::Out;

  if (arrayType.hasConstVal()) {
    return idxVal < arrayType.arrVal()->size()
      ? PackedBounds::In
      : PackedBounds::Out;
  }

  auto const at = arrayType.arrSpec().type();
  if (!at) return PackedBounds::Unknown;

  using A = RepoAuthType::Array;
  switch (at->tag()) {
  case A::Tag::Packed:
    if (idxVal < at->size() && at->emptiness() == A::Empty::No) {
      return PackedBounds::In;
    }
    // fallthrough
  case A::Tag::PackedN:
    if (idxVal == 0 && at->emptiness() == A::Empty::No) {
      return PackedBounds::In;
    }
  }
  return PackedBounds::Unknown;
}

///////////////////////////////////////////////////////////////////////////////

Type packedArrayElemType(SSATmp* arr, SSATmp* idx, const Class* ctx) {
  assertx(arr->isA(TArr) &&
          arr->type().arrSpec().kind() == ArrayData::kPackedKind &&
          idx->isA(TInt));

  if (arr->hasConstVal() && idx->hasConstVal()) {
    auto const idxVal = idx->intVal();
    if (idxVal >= 0 && idxVal < arr->arrVal()->size()) {
      return Type(arr->arrVal()->at(idxVal).m_type);
    }
    return TInitNull;
  }

  Type t = arr->isA(TPersistentArr) ? TInitCell : TGen;

  auto const at = arr->type().arrSpec().type();
  if (!at) return t;

  switch (at->tag()) {
    case RepoAuthType::Array::Tag::Packed:
    {
      if (idx->hasConstVal(TInt)) {
        auto const idxVal = idx->intVal();
        if (idxVal >= 0 && idxVal < at->size()) {
          return typeFromRAT(at->packedElem(idxVal), ctx) & t;
        }
        return TInitNull;
      }
      Type elemType = TBottom;
      for (uint32_t i = 0; i < at->size(); ++i) {
        elemType |= typeFromRAT(at->packedElem(i), ctx);
      }
      return elemType & t;
    }
    case RepoAuthType::Array::Tag::PackedN:
      return typeFromRAT(at->elemType(), ctx) & t;
  }
  not_reached();
}

Type vecElemType(SSATmp* arr, SSATmp* idx) {
  assertx(arr->isA(TVec));
  assertx(!idx || idx->isA(TInt));

  if (arr->hasConstVal()) {
    // If both the array and idx are known statically, we can resolve it to the
    // precise type.
    if (idx && idx->hasConstVal()) {
      auto const idxVal = idx->intVal();
      if (idxVal >= 0 && idxVal < arr->vecVal()->size()) {
        auto const rval = PackedArray::RvalIntVec(arr->vecVal(), idxVal);
        return rval ? Type(rval.type()) : TBottom;
      }
      return TBottom;
    }

    // Otherwise we can constrain the type according to the union of all the
    // types present in the vec.
    Type type{TBottom};
    PackedArray::IterateV(
      arr->vecVal(),
      [&](TypedValue v) { type |= Type(v.m_type); }
    );
    return type;
  }

  // Vecs always contain initialized cells
  return arr->isA(TPersistentVec) ? TUncountedInit : TInitCell;
}

Type dictElemType(SSATmp* arr, SSATmp* idx) {
  assertx(arr->isA(TDict));
  assertx(!idx || idx->isA(TInt | TStr));

  if (arr->hasConstVal()) {
    // If both the array and idx are known statically, we can resolve it to the
    // precise type.
    if (idx && idx->hasConstVal(TInt)) {
      auto const idxVal = idx->intVal();
      auto const rval = MixedArray::RvalIntDict(arr->dictVal(), idxVal);
      return rval ? Type(rval.type()) : TBottom;
    }

    if (idx && idx->hasConstVal(TStr)) {
      auto const idxVal = idx->strVal();
      auto const rval = MixedArray::RvalStrDict(arr->dictVal(), idxVal);
      return rval ? Type(rval.type()) : TBottom;
    }

    // Otherwise we can constrain the type according to the union of all the
    // types present in the dict.
    Type type{TBottom};
    MixedArray::IterateKV(
      MixedArray::asMixed(arr->dictVal()),
      [&](Cell k, TypedValue v) {
        // Ignore values which can't correspond to the key's type
        if (isIntType(k.m_type)) {
          if (!idx || idx->type().maybe(TInt)) type |= Type(v.m_type);
        } else if (isStringType(k.m_type)) {
          if (!idx || idx->type().maybe(TStr)) type |= Type(v.m_type);
        }
      }
    );
    return type;
  }

  // Dicts always contain initialized cells
  return arr->isA(TPersistentDict) ? TUncountedInit : TInitCell;
}

Type keysetElemType(SSATmp* arr, SSATmp* idx) {
  assertx(arr->isA(TKeyset));
  assertx(!idx || idx->isA(TInt | TStr));

  if (arr->hasConstVal()) {
    // If both the array and idx are known statically, we can resolve it to the
    // precise type.
    if (idx && idx->hasConstVal(TInt)) {
      auto const idxVal = idx->intVal();
      auto const rval = SetArray::RvalInt(arr->keysetVal(), idxVal);
      return rval ? Type(rval.type()) : TBottom;
    }

    if (idx && idx->hasConstVal(TStr)) {
      auto const idxVal = idx->strVal();
      auto const rval = SetArray::RvalStr(arr->keysetVal(), idxVal);
      return rval ? Type(rval.type()) : TBottom;
    }

    // Otherwise we can constrain the type according to the union of all the
    // types present in the keyset.
    Type type{TBottom};
    SetArray::Iterate(
      SetArray::asSet(arr->keysetVal()),
      [&](TypedValue v) {
        // Ignore values which can't correspond to the key's type
        if (isIntType(v.m_type)) {
          if (!idx || idx->type().maybe(TInt)) type |= Type(v.m_type);
        } else {
          assertx(isStringType(v.m_type));
          if (!idx || idx->type().maybe(TStr)) type |= Type(v.m_type);
        }
      }
    );

    // The key is always the value, so, for instance, if there's nothing but
    // strings in the keyset, we know an int idx can't access a valid value.
    if (idx) {
      if (idx->isA(TInt)) type &= TInt;
      if (idx->isA(TStr)) type &= TStr;
    }
    return type;
  }

  // Keysets always contain strings or integers. We can further constrain this
  // if we know the idx type, as the key is always the value.
  auto type = TStr | TInt;
  if (idx) {
    if (idx->isA(TInt)) type &= TInt;
    if (idx->isA(TStr)) type &= TStr;
  }
  if (arr->isA(TPersistentKeyset)) type &= TUncountedInit;
  return type;
}

///////////////////////////////////////////////////////////////////////////////

}}

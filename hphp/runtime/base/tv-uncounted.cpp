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

#include "hphp/runtime/base/tv-uncounted.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/func.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////////////

void ConvertTvToUncounted(tv_lval source, DataWalker::PointerMap* seen) {
  auto& data = source.val();
  auto& type = source.type();
  auto const handlePersistent = [&] (MaybeCountable* elm) {
    if (elm->isRefCounted()) return false;
    if (elm->isStatic()) return true;
    if (elm->uncountedIncRef()) return true;
    if (seen) seen->emplace(elm, nullptr);
    return false;
  };

  // `source' won't be Object or Resource, as these should never appear in an
  // uncounted array.  Thus we only need to deal with strings/arrays.
  switch (type) {
    case KindOfFunc:
    if (RuntimeOption::EvalAPCSerializeFuncs) {
      assertx(data.pfunc->isPersistent());
      break;
    }
    invalidFuncConversion("string");
    case KindOfClass:
      if (data.pclass->isPersistent()) break;
      data.plazyclass = LazyClassData::create(data.pclass->name());
      type = KindOfLazyClass;
      break;
    case KindOfString:
      type = KindOfPersistentString;
      // Fall-through.
    case KindOfPersistentString: {
      auto& str = data.pstr;
      if (handlePersistent(str)) break;
      if (str->empty()) str = staticEmptyString();
      else if (auto const st = lookupStaticString(str)) str = st;
      else {
        HeapObject** seenStr = nullptr;
        if (seen && str->hasMultipleRefs()) {
          seenStr = &(*seen)[str];
          if (auto const st = static_cast<StringData*>(*seenStr)) {
            if (st->uncountedIncRef()) {
              str = st;
              break;
            }
          }
        }
        str = StringData::MakeUncounted(str->slice());
        if (seenStr) *seenStr = str;
      }
      break;
    }
    case KindOfVec:
      type = KindOfPersistentVec;
      // Fall-through.
    case KindOfPersistentVec: {
      auto& ad = data.parr;
      assertx(ad->isVecType());
      if (handlePersistent(ad)) break;
      if (ad->empty()) {
        ad = ArrayData::CreateVec(ad->isLegacyArray());
      } else if (ad->isVanilla()) {
        ad = PackedArray::MakeUncounted(ad, false, seen);
      } else {
        ad = BespokeArray::MakeUncounted(ad, false, seen);
      }
      break;
    }

    case KindOfDict:
      type = KindOfPersistentDict;
      // Fall-through.
    case KindOfPersistentDict: {
      auto& ad = data.parr;
      assertx(ad->isDictType());
      if (handlePersistent(ad)) break;
      if (ad->empty()) {
        ad = ArrayData::CreateDict(ad->isLegacyArray());
      } else if (ad->isVanilla()) {
        ad = MixedArray::MakeUncounted(ad, false, seen);
      } else {
        ad = BespokeArray::MakeUncounted(ad, false, seen);
      }
      break;
    }

    case KindOfKeyset:
      type = KindOfPersistentKeyset;
      // Fall-through.
    case KindOfPersistentKeyset: {
      auto& ad = data.parr;
      assertx(ad->isKeysetType());
      if (handlePersistent(ad)) break;
      if (ad->empty()) {
        ad = ArrayData::CreateKeyset();
      } else if (ad->isVanilla()) {
        ad = SetArray::MakeUncounted(ad, false, seen);
      } else {
        ad = BespokeArray::MakeUncounted(ad, false, seen);
      }
      break;
    }

    case KindOfUninit: {
      type = KindOfNull;
      break;
    }
    case KindOfClsMeth: {
      if (RO::EvalAPCSerializeClsMeth) {
        assertx(use_lowptr);
        assertx(data.pclsmeth->getCls()->isPersistent());
        break;
      }
      tvCastToVecInPlace(source);
      type = KindOfPersistentVec;
      auto& ad = data.parr;
      if (handlePersistent(ad)) break;
      assertx(!ad->empty());
      assertx(ad->isVanillaVec());
      ad = PackedArray::MakeUncounted(ad, false, seen);
      break;
    }
    case KindOfLazyClass:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble: {
      break;
    }
    case KindOfRecord:
      raise_error(Strings::RECORD_NOT_SUPPORTED);
    case KindOfObject:
    case KindOfResource:
    case KindOfRFunc:
    case KindOfRClsMeth:
      not_reached();
  }
}

void ReleaseUncountedTv(tv_lval lval) {
  if (isStringType(type(lval))) {
    auto const str = val(lval).pstr;
    assertx(!str->isRefCounted());
    if (str->isUncounted()) {
      StringData::ReleaseUncounted(str);
    }
    return;
  }
  if (isArrayLikeType(type(lval))) {
    auto const arr = val(lval).parr;
    assertx(!arr->isRefCounted());
    if (!arr->isStatic()) {
      if (arr->isVanillaVec()) PackedArray::ReleaseUncounted(arr);
      else if (arr->isVanillaDict()) MixedArray::ReleaseUncounted(arr);
      else if (arr->isVanillaKeyset()) SetArray::ReleaseUncounted(arr);
      else BespokeArray::ReleaseUncounted(arr);
    }
    return;
  }
  assertx(!isRefcountedType(type(lval)));
}

//////////////////////////////////////////////////////////////////////////////

}

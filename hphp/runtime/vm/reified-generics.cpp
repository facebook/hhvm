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

#include "hphp/runtime/vm/reified-generics.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"

#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/reified-generics-info.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

ArrayData* addToTypeReifiedGenericsTable(
  const StringData* name,
  ArrayData* tsList
) {
  auto const ne = NamedType::get(name, true);
  auto const generics = ne->getCachedReifiedGenerics();
  if (!generics) {
    // We have created a new entry on the named entity table
    // TODO(T31677864): If the type structures only contain persistent data,
    // mark it as persistent
    ne->m_cachedReifiedGenerics.bind(
      rds::Mode::Normal,
      rds::LinkName{"ReifiedGenerics", name}
    );
    ArrayData::GetScalarArray(&tsList);
    ne->setCachedReifiedGenerics(tsList);
    return tsList;
  }
  // Same key should never result in different values.
  // If this assertion fires, there's a high chance that two different type
  // structure mangle to the same name and they should not.
  assert_flog(tsList->same(generics),
              "reified ts mismatch\nname: {}\ntsList: {}\ngenerics: {}\n",
              name,
              [&](){ std::string s; staticArrayStreamer(tsList, s); return s; }(),
              [&](){ std::string s; staticArrayStreamer(generics, s); return s; }());
  decRefArr(tsList);
  return generics;
}

ArrayData* getClsReifiedGenericsProp(Class* cls, ObjectData* obj) {
  if (!cls->hasReifiedGenerics()) {
    raise_error("Cannot get reified generics property of a non reified class");
  }
  auto const slot = cls->lookupReifiedInitProp();
  assertx(slot != kInvalidSlot);
  auto index = cls->propSlotToIndex(slot);
  auto tv = obj->props()->at(index).tv();
  assertx(tvIsVec(tv));
  return tv.m_data.parr;
}

ReifiedGenericsInfo
extractSizeAndPosFromReifiedAttribute(const ArrayData* arr) {
  size_t len = 0, cur = 0, numReified = 0;
  bool isReified = false, isSoft = false, hasAnySoft = false;
  uint32_t bitmap = 0;
  std::vector<TypeParamInfo> tpList;
  IterateKV(
    arr,
    [&](TypedValue k, TypedValue v) {
      assertx(isIntType(k.m_type));
      assertx(isIntType(v.m_type));
      if (k.m_data.num == 0) {
        len = (size_t) v.m_data.num;
      } else {
        if (k.m_data.num % 3 == 1) {
          // This is the reified generic index
          // Insert the non reified ones
          auto const numErased = v.m_data.num - cur;
          tpList.insert(tpList.end(), numErased, {});
          bitmap = (bitmap << (numErased + 1)) | 1;
          cur = v.m_data.num;
          isReified = true;
        } else if (k.m_data.num % 3 == 2) {
          isSoft = (bool) v.m_data.num;
          hasAnySoft |= isSoft;
        } else {
          // k.m_data.num % 3 == 0
          numReified++;
          cur++;
          tpList.push_back({isReified, isSoft, (bool) v.m_data.num});
        }
      }
    }
  );
  // Insert the non reified ones at the end
  tpList.insert(tpList.end(), len - cur, {});
  bitmap = bitmap << (len - cur);
  return {numReified, hasAnySoft, bitmap, tpList};
}

// Raises a runtime error if the location of reified generics of f does not
// match the location of reified_generics
template <bool fun>
void checkReifiedGenericMismatchHelper(
  const ReifiedGenericsInfo& info,
  const StringData* name,
  const ArrayData* reified_generics
) {
  auto const& generics = info.m_typeParamInfo;
  auto const len = generics.size();
  if (len != reified_generics->size()) {
    if (reified_generics->size() == 0) {
      if (areAllGenericsSoft(info)) {
        raise_warning_for_soft_reified(0, fun, name);
        return;
      }
    }
    SystemLib::throwBadMethodCallExceptionObject(
      folly::sformat("{} {} requires {} generics but {} given",
                     fun ? "Function" : "Class",
                     name->data(),
                     len,
                     reified_generics->size()));
  }
  auto it = generics.begin();
  IterateKV(
    reified_generics,
    [&](TypedValue k, TypedValue v) {
      assertx(isIntType(k.m_type));
      assertx(isArrayLikeType(v.m_type));
      auto const i = k.m_data.num;
      if (isWildCard(v.m_data.parr) && it->m_isReified) {
        if (!it->m_isSoft) {
          SystemLib::throwBadMethodCallExceptionObject(
            folly::sformat("{} {} expects a reified generic at index {}",
                           fun ? "Function" : "Class",
                           name->data(),
                           i));
        }
        raise_warning_for_soft_reified(i, fun, name);
      }
      ++it;
    }
  );
}

void checkFunReifiedGenericMismatch(
  const Func* f,
  const ArrayData* reified_generics
) {
  checkReifiedGenericMismatchHelper<true>(
    f->getReifiedGenericsInfo(),
    f->fullName(),
    reified_generics
  );
}

void checkClassReifiedGenericMismatch(
  const Class* c,
  const ArrayData* reified_generics
) {
  checkReifiedGenericMismatchHelper<false>(
    c->getReifiedGenericsInfo(),
    c->name(),
    reified_generics
  );
}

uint16_t getGenericsBitmap(const ArrayData* generics) {
  assertx(generics);
  if (generics->size() > 15) return 0;
  uint16_t bitmap = 1;
  IterateV(
    generics,
    [&](TypedValue v) {
      assertx(isArrayLikeType(v.m_type));
      bitmap = (bitmap << 1) | !isWildCard(v.m_data.parr);
    }
  );
  return bitmap;
}

uint16_t getGenericsBitmap(const Func* f) {
  assertx(f);
  if (!f->hasReifiedGenerics()) return 0;
  auto const& info = f->getReifiedGenericsInfo();
  if (info.m_typeParamInfo.size() > 15) return 0;
  uint16_t bitmap = 1;
  for (auto const& tinfo : info.m_typeParamInfo) {
    bitmap = (bitmap << 1) | (tinfo.m_isReified && !tinfo.m_isSoft);
  }
  return bitmap;
}

bool areAllGenericsSoft(const ReifiedGenericsInfo& info) {
  for (auto const& tp : info.m_typeParamInfo) {
    if (!tp.m_isSoft) return false;
  }
  return true;
}

void raise_warning_for_soft_reified(size_t i, bool fun,
                                    const StringData *name) {
  raise_warning("Generic at index %zu to %s %s must be reified,"
                " erased given",
                i,
                fun ? "Function" : "Class",
                name->data());
}

void checkClassReifiedGenericsSoft(const Class* cls) {
  assertx(cls->hasReifiedGenerics());
  if (areAllGenericsSoft(cls->getReifiedGenericsInfo())) {
    raise_warning_for_soft_reified(0, false, cls->name());
  } else {
    raise_error("Cannot create a new instance of a reified class without "
                "the reified generics");
  }
}

///////////////////////////////////////////////////////////////////////////////
}

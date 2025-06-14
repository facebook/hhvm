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
#include "hphp/runtime/vm/generics-info.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

ArrayData* addToTypeReifiedGenericsTable(
  const StringData* name,
  ArrayData* tsList
) {
  auto const ne = NamedType::getOrCreate(name);
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
              staticArrayStreamer(tsList),
              staticArrayStreamer(generics));
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

GenericsInfo
extractSizeAndPosFromReifiedAttribute(
  const ArrayData* arr,
  const folly::Range<const LowStringPtr*>& typeParamNames
) {
  size_t len = 0, cur = 0;
  bool isReified = false, isSoft = false;
  std::vector<TypeParamInfo> tpList;
  IterateKV(
    arr,
    [&](TypedValue k, TypedValue v) {
      assertx(isIntType(k.m_type));
      assertx(isIntType(v.m_type));
      if (k.m_data.num == 0) {
        len = (size_t) v.m_data.num;
        always_assert(len == typeParamNames.size());
      } else {
        if (k.m_data.num % 3 == 1) {
          // This is the reified generic index
          // Insert the non reified ones
          auto const numErased = v.m_data.num - cur;
          for (auto i = 0; i < numErased; ++i) {
            tpList.emplace_back(
              false, false, false, typeParamNames[cur + i]
            );
          }
          cur = v.m_data.num;
          isReified = true;
        } else if (k.m_data.num % 3 == 2) {
          isSoft = (bool) v.m_data.num;
        } else {
          // k.m_data.num % 3 == 0
          tpList.emplace_back(
            isReified,
            isSoft,
            (bool) v.m_data.num,
            typeParamNames[cur]
          );
          cur++;
        }
      }
    }
  );
  // Insert the non reified ones at the end
  for (auto i = 0; i < len - cur; ++i) {
    tpList.emplace_back(
      false,
      false,
      false,
      typeParamNames[cur + i]
    );
  }

  return GenericsInfo(std::move(tpList));
}

// Raises a runtime error if the location of reified generics of f does not
// match the location of reified_generics
template <bool fun>
void checkReifiedGenericMismatchHelper(
  const GenericsInfo& info,
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
    f->getGenericsInfo(),
    f->fullName(),
    reified_generics
  );
}

void checkClassReifiedGenericMismatch(
  const Class* c,
  const ArrayData* reified_generics
) {
  checkReifiedGenericMismatchHelper<false>(
    c->getGenericsInfo(),
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
  auto const& info = f->getGenericsInfo();
  if (info.m_typeParamInfo.size() > 15) return 0;
  uint16_t bitmap = 1;
  for (auto const& tinfo : info.m_typeParamInfo) {
    bitmap = (bitmap << 1) | (tinfo.m_isReified && !tinfo.m_isSoft);
  }
  return bitmap;
}

bool areAllGenericsSoft(const GenericsInfo& info) {
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
  if (areAllGenericsSoft(cls->getGenericsInfo())) {
    raise_warning_for_soft_reified(0, false, cls->name());
  } else {
    raise_error("Cannot create a new instance of a reified class without "
                "the reified generics");
  }
}

void tryClassReifiedInit(Class* cls, ArrayData* generics, ObjectData* obj) {
  if (cls->hasReifiedGenerics()) {
    if (generics->empty()) {
      checkClassReifiedGenericsSoft(cls);
      return;
    }
    obj->setReifiedGenerics(cls, generics);
    return;
  }
  if (cls->hasReifiedParent()) {
    obj->setReifiedGenerics(cls, generics);
  }
}

size_t extractSizeFromReifiedAttribute(const ArrayData* arr) {
  size_t len = 0;
  IterateKV(
    arr,
    [&](TypedValue k, TypedValue v) {
      assertx(isIntType(k.m_type));
      assertx(isIntType(v.m_type));
      if (k.m_data.num == 0) {
        len = (size_t) v.m_data.num;
        return true;
      }
      return false;
    }
  );
  return len;
}

bool areAllGenericsSoft(const ArrayData* arr) {
  bool allSoft = true;
  IterateKV(
    arr,
    [&](TypedValue k, TypedValue v) {
      if (k.m_data.num % 3 == 2) {
        allSoft &= (bool) v.m_data.num;
      }
    }
  );
  return allSoft;
}

///////////////////////////////////////////////////////////////////////////////
}

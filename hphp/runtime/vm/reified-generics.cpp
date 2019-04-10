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

#include "hphp/runtime/base/type-structure-helpers-defs.h"

#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/reified-generics-info.h"

#include "hphp/util/debug.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

ArrayData* addToReifiedGenericsTable(
  const StringData* name,
  ArrayData* tsList,
  bool& didAdd
) {
  auto const ne = NamedEntity::get(name, true);
  auto const generics = ne->getCachedReifiedGenerics();
  if (!generics) {
    // We have created a new entry on the named entity table
    // TODO(T31677864): If the type structures only contain persistent data,
    // mark it as persistent
    ne->m_cachedReifiedGenerics.bind(rds::Mode::Normal);
    ArrayData::GetScalarArray(&tsList);
    ne->setCachedReifiedGenerics(tsList);
    didAdd = true;
    return tsList;
  }
  // it already exists on the named entity table
  if (debug && !tsList->equal(generics, true)) {
    raise_error("Mismatched reified types");
  }
  didAdd = false;
  return generics;
}

ArrayData* getReifiedTypeList(const StringData* name) {
  auto const bail = [&] {
    raise_error("%s does not exist on the reified generics table",
                name->data());
  };
  auto const ne = NamedEntity::get(name, false);
  if (!ne) bail();
  auto const generics = ne->getCachedReifiedGenerics();
  if (!generics) bail();
  return generics;
}

ArrayData* getClsReifiedGenericsProp(Class* cls, ObjectData* obj) {
  if (!cls->hasReifiedGenerics()) {
    raise_error("Cannot get reified generics property of a non reified class");
  }
  auto const slot = cls->lookupReifiedInitProp();
  assertx(slot != kInvalidSlot);
  auto tv = obj->propVec()[slot];
  assertx(tvIsVecOrVArray(tv));
  return tv.m_data.parr;
}

ArrayData* getClsReifiedGenericsProp(Class* cls, ActRec* ar) {
  auto const this_ = ar->getThis();
  return getClsReifiedGenericsProp(cls, this_);
}

ReifiedGenericsInfo
extractSizeAndPosFromReifiedAttribute(const ArrayData* arr) {
  size_t len = 0, cur = 0, numReified = 0;
  bool isReified = false, isSoft = false;
  std::vector<TypeParamInfo> tpList;
  IterateKV(
    arr,
    [&](Cell k, TypedValue v) {
      assertx(isIntType(k.m_type));
      assertx(isIntType(v.m_type));
      if (k.m_data.num == 0) {
        len = (size_t) v.m_data.num;
      } else {
        if (k.m_data.num % 3 == 1) {
          // This is the reified generic index
          // Insert the non reified ones
          tpList.insert(tpList.end(), v.m_data.num - cur, {});
          cur = v.m_data.num;
          isReified = true;
        } else if (k.m_data.num % 3 == 2) {
          isSoft = (bool) v.m_data.num;
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
  return {numReified, tpList};
}

// Raises a runtime error if the location of reified generics of f does not
// match the location of reified_generics
template <bool fun>
void checkReifiedGenericMismatchHelper(
  const ReifiedGenericsInfo& info,
  const StringData* name,
  const ArrayData* reified_generics
) {
  auto const generics = info.m_typeParamInfo;
  auto const len = generics.size();
  if (len != reified_generics->size()) {
    raise_error("%s %s requires %zu generics but %zu given",
                fun ? "Function" : "Class",
                name->data(),
                len,
                reified_generics->size());
  }
  auto it = generics.begin();
  IterateKV(
    reified_generics,
    [&](Cell k, TypedValue v) {
      assertx(isIntType(k.m_type));
      assertx(isArrayLikeType(v.m_type));
      bool wildcard = false;
      auto const i = k.m_data.num;
      if (get_ts_kind(v.m_data.parr) == TypeStructure::Kind::T_typevar &&
          v.m_data.parr->exists(s_name.get())) {
        wildcard = (get_ts_name(v.m_data.parr)->equal(s_wildcard.get()));
      }
      if (wildcard && it->m_isReified) {
        if (!it->m_isSoft) {
          raise_error("%s %s expects a reified generic at index %zu",
                      fun ? "Function" : "Class",
                      name->data(),
                      i);
        }
        raise_warning("Generic at index %zu to %s %s must be reified,"
                      " erased given",
                      i,
                      fun ? "Function" : "Class",
                      name->data());
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

///////////////////////////////////////////////////////////////////////////////
}

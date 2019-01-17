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
#include "hphp/runtime/vm/reified-generics-info.h"

#include "hphp/util/debug.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

namespace {
using ReifiedGenericsTable = hphp_string_map<ArrayData*>;
ReifiedGenericsTable g_reified_generics_table;
} // namespace

void addToReifiedGenericsTable(
  const std::string& name,
  ArrayData* tsList
) {
  auto e = g_reified_generics_table.find(name);

  if (UNLIKELY(e == g_reified_generics_table.end())) {
    g_reified_generics_table[name] = tsList;
    return;
  }
  if (debug) {
    if (!tsList->equal(e->second, true)) {
      raise_error("Mismatched reified types");
    }
  }
  return;
}

ArrayData* getReifiedTypeList(const std::string& name) {
  auto e = g_reified_generics_table.find(name);
  if (LIKELY(e != g_reified_generics_table.end())) {
    return e->second;
  }
  raise_error("No such entry in the reified classes table");
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
  size_t len = 0;
  std::vector<size_t> pos;
  std::vector<size_t> soft;
  IterateKV(
    arr,
    [&](Cell k, TypedValue v) {
      assertx(isIntType(k.m_type));
      assertx(isIntType(v.m_type));
      if (k.m_data.num == 0) {
        len = (size_t) v.m_data.num;
      } else if (k.m_data.num % 2 == 0) {
        // whether soft or not
        // If soft, add the last reified index
        if (v.m_data.num) soft.emplace_back(pos.back());
      } else {
        // reified index
        pos.emplace_back(v.m_data.num);
      }
    }
  );
  assertx(pos.size() <= len);
  assertx(soft.size() <= pos.size());
  return {len, pos, soft};
}

// Raises a runtime error if the location of reified generics of f does not
// match the location of reified_generics
template <bool fun>
void checkReifiedGenericMismatchHelper(
  const ReifiedGenericsInfo& info,
  const StringData* name,
  const ArrayData* reified_generics
) {
  auto const len = info.m_numGenerics;
  auto const locations = info.m_reifiedGenericPositions;
  auto const soft = info.m_softReifiedGenericPositions;
  if (len != reified_generics->size()) {
    raise_error("%s %s requires %zu generics but %zu given",
                fun ? "Function" : "Class",
                name->data(),
                len,
                reified_generics->size());
  }
  auto it = locations.begin();
  auto softit = soft.begin();
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
      if (wildcard) {
        if (it == locations.end() || *it > i) return false;
        if (softit == soft.end() || *softit != i) {
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
      // It is not a wildcard
      if (it == locations.end() || *it != i) return false;
      if (softit != soft.end() && *softit == *it) softit++;
      ++it;
      return false;
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

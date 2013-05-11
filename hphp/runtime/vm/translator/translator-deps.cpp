/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include <runtime/vm/translator/translator-x64.h>
#include <runtime/vm/translator/translator-deps.h>

namespace HPHP {
namespace VM {
namespace Transl {

PreConstDepMap gPreConsts;

TRACE_SET_MOD(txdeps);

const TypedValue* preConstVecHasUnique(const PreConstPtrVec& preConsts) {
  if (preConsts.empty()) {
    return nullptr;
  }

  assert(preConsts.size() >= 1);
  const TypedValue* first = &preConsts.front()->value;
  if (first->m_type == KindOfUninit) {
    return nullptr;
  }
  for (size_t i = 1; i < preConsts.size(); ++i) {
    if (!tvSame(first, &preConsts[i]->value)) {
      return nullptr;
    }
  }
  return first;
}

static void mergePreConstImpl(const PreConst& pc,
                              SrcKeySet& invalidateKeys) {
  PreConstDepMap::accessor acc;
  bool isNew = Transl::gPreConsts.insert(acc, pc.name);
  PreConstDep& dep = acc->second;
  // We're going to add a new PreConst below. Invalidate dependent
  // SrcKeys iff (there's currently a unique value AND the new value
  // is not the same) OR (there are no preConsts right now AND there
  // used to be a unique value AND the value we're adding is
  // different from the old value)
  bool invalidate = false;
  if (!isNew && !dep.srcKeys.empty()) {
    const TypedValue* uniq = preConstVecHasUnique(dep.preConsts);
    if (uniq && !tvSame(uniq, &pc.value)) {
      invalidate = true;
    } else if (dep.preConsts.empty() &&
               dep.lastUniqueVal.m_type != KindOfUninit &&
               !tvSame(&dep.lastUniqueVal, &pc.value)) {
      invalidate = true;
    }
  }
  if (invalidate) {
    TRACE(1, "%s: Invalidating %lu SrcKeys for preConst %s\n",
          __FUNCTION__, dep.srcKeys.size(), pc.name->data());
    // The invalidation is deferred to avoid a lock rank violation
    // between gPreConsts and the write lease.
    invalidateKeys.insert(dep.srcKeys.begin(), dep.srcKeys.end());
    dep.srcKeys.clear();
  }

  TRACE(3, "%s: %lu preConst(s) for %s, adding %s\n",
        __FUNCTION__, dep.preConsts.size(), pc.name->data(),
        pc.value.pretty().c_str());
  dep.preConsts.push_back(&pc);
  if (const TypedValue* tv = preConstVecHasUnique(dep.preConsts)) {
    dep.lastUniqueVal = *tv;
  } else {
    dep.lastUniqueVal.m_type = KindOfUninit;
  }
}

void mergePreConst(const PreConst& pc) {
  SrcKeySet invalidateKeys;
  mergePreConstImpl(pc, invalidateKeys);
  if (!invalidateKeys.empty()) {
    TranslatorX64::Get()->invalidateSrcKeys(invalidateKeys);
  }
}

void mergePreConsts(const PreConstVec& preConsts) {
  SrcKeySet invalidateKeys;
  for (PreConstVec::const_iterator i = preConsts.begin();
       i != preConsts.end(); ++i) {
    mergePreConstImpl(*i, invalidateKeys);
  }
  if (!invalidateKeys.empty()) {
    TranslatorX64::Get()->invalidateSrcKeys(invalidateKeys);
  }
}

void unmergePreConsts(const PreConstVec& preConsts, void* owner) {
  ConstStringDataSet visitedNames;
  for (PreConstVec::const_iterator mi = preConsts.begin();
       mi != preConsts.end(); ++mi) {
    assert(mi->owner == owner);
    if (visitedNames.find(mi->name) != visitedNames.end()) {
      continue;
    }
    visitedNames.insert(mi->name);
    PreConstDepMap::accessor acc;
    UNUSED bool found = gPreConsts.find(acc, mi->name);
    assert(found);
    UNUSED int erased = 0;
    PreConstPtrVec& pcPtrs = acc->second.preConsts;
    for (PreConstPtrVec::iterator vi = pcPtrs.begin(); vi != pcPtrs.end(); ) {
      if ((*vi)->owner == owner) {
        vi = pcPtrs.erase(vi);
        erased++;
      } else {
        ++vi;
      }
    }
    assert(erased > 0);
    TRACE(3, "%s: Erased %d preConsts for %s\n",
          __FUNCTION__, erased, mi->name->data());
    // We erased one or more of the pcPtrs registered for this
    // name. If the value is newly unique, we should update
    // lastUniqueVal appropriately. If it's still non-unique or all
    // the preconsts are gone, lastUniqueVal already has an
    // appropriate value.
    if (const TypedValue* tv = preConstVecHasUnique(pcPtrs)) {
      TRACE(2, "%s: Constant %s %s is newly unique\n",
            __FUNCTION__, mi->name->data(), tv->pretty().c_str());
      acc->second.lastUniqueVal = *tv;
    }
  }
}

} } } // HPHP::VM::Transl

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef _TRANSLATOR_DEPS_H_
#define _TRANSLATOR_DEPS_H

#include <vector>
#include <tbb/concurrent_hash_map.h>

#include <runtime/base/complex_types.h>
#include <runtime/base/string_data.h>
#include <runtime/vm/unit.h>
#include <runtime/vm/translator/translator.h>
#include <util/mutex.h>
#include <util/trace.h>

// Invalidation tracking for constants

namespace HPHP {
namespace VM {
namespace Transl {
typedef std::vector<const PreConst*> PreConstPtrVec;

/*
 * We create one of these structs per constant name that has ever been
 * seen. It contains a list of all PreConsts from currently live
 * Units, a set of SrcKeys that depend on there being a unique value,
 * and the most recently unique value.
 */
struct PreConstDep {
  PreConstDep() {
    lastUniqueVal.m_type = KindOfUninit;
  }
  SrcKeySet srcKeys;
  PreConstPtrVec preConsts;
  TypedValue lastUniqueVal; // Will be KindOfUninit if the value is
                            // not currently unique, and for a short
                            // moment before any preConsts are added
                            // after creation
};
typedef RankedCHM<const StringData*, PreConstDep,
                  StringDataHashCompare, RankPreConstDep> PreConstDepMap;
extern PreConstDepMap gPreConsts;

const TypedValue* preConstVecHasUnique(const PreConstPtrVec& preConsts);

template<typename Accessor>
const TypedValue* findUniquePreConst(Accessor& acc, const StringData* name) {
  assert(!RuntimeOption::RepoAuthoritative);
  TRACE_SET_MOD(txdeps);
  if (gPreConsts.find(acc, name)) {
    const PreConstDep& dep = acc->second;
    if (const TypedValue* tv = preConstVecHasUnique(dep.preConsts)) {
      TRACE(3, "%s: Found unique preConst value %s for %s\n",
            __FUNCTION__, tv->pretty().c_str(), name->data());
      return tv;
    }
    TRACE(2, "%s: Found %lu non-unique preConsts values for %s\n",
          __FUNCTION__, dep.preConsts.size(), name->data());
    acc.release();
  } else {
    TRACE(2, "%s: No preConsts for %s\n", __FUNCTION__, name->data());
  }

  return nullptr;
}

void mergePreConst(const PreConst& preConst);
void mergePreConsts(const PreConstVec& preConsts);
void unmergePreConsts(const PreConstVec& preConsts, void* owner);

} } } // HPHP::VM::Transl

#endif // _TRANSLATOR_DEPS_H_

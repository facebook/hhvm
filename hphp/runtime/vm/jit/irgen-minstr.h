/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_IRGEN_MINSTR_H_
#define incl_HPHP_JIT_IRGEN_MINSTR_H_

#include "hphp/runtime/base/static-string-table.h"

#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/jit/mixed-array-offset-profile.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/target-profile.h"

namespace HPHP { namespace jit { namespace irgen {

///////////////////////////////////////////////////////////////////////////////

/*
 * Use profiling data from a MixedArrayOffsetProfile to conditionally optimize
 * the array access represented by `generic' as `direct'.
 *
 * For profiling translations, this generates the profiling instructions, then
 * falls back to `generic'.  If we can perform the optimization, this branches
 * on a CheckMixedArrayOffset to either `direct' or `generic'; otherwise, we
 * fall back to `generic'.
 *
 * The callback function signatures should be:
 *
 *    SSATmp* direct(SSATmp* key, uint32_t pos);
 *    SSATmp* generic(SSATmp* key);
 */
template<class DirectFn, class GenericFn>
SSATmp* profiledArrayAccess(IRGS& env, SSATmp* arr, SSATmp* key,
                            DirectFn direct, GenericFn generic,
                            bool cow_check = false) {
  auto const profile = TargetProfile<MixedArrayOffsetProfile> {
    env.context,
    env.irb->curMarker(),
    makeStaticString("MixedArrayOffset")
  };

  if (profile.profiling()) {
    gen(env, ProfileMixedArrayOffset,
        RDSHandleData { profile.handle() }, arr, key);
    return generic(key);
  }

  if (profile.optimizing()) {
    auto const data = profile.data(MixedArrayOffsetProfile::reduce);

    if (auto const pos = data.choose()) {
      return cond(
        env,
        [&] (Block* taken) {
          env.irb->constrainValue(
            arr,
            TypeConstraint(DataTypeSpecialized).setWantArrayKind()
          );
          auto const TMixedArr = Type::Array(ArrayData::kMixedKind);
          auto const marr = gen(env, CheckType, TMixedArr, taken, arr);

          auto const extra = IndexData { *pos };
          gen(env, CheckMixedArrayOffset, extra, taken, marr, key);

          if (cow_check) {
            gen(env, CheckArrayCOW, taken, marr);
          }
          return marr;
        },
        [&] (SSATmp* arr) { return direct(arr, key, *pos); },
        [&] { return generic(key); }
      );
    }
  }
  return generic(key);
}

///////////////////////////////////////////////////////////////////////////////

}}}

#endif

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
#pragma once

#include "hphp/runtime/vm/containers.h"
#include "hphp/runtime/vm/member-key.h"
#include "hphp/runtime/vm/name-value-table.h"

#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {

struct Class;
struct StringData;
struct TypeConstraint;
template<typename T> struct TypeIntersectionConstraintT;
using VMTypeIntersectionConstraint = TypeIntersectionConstraintT<VMCompactVector<TypeConstraint>>;

namespace jit {

struct Block;

namespace irgen {

//////////////////////////////////////////////////////////////////////

struct ClsPropLookup {
  SSATmp* propPtr;
  Type knownType;
  const TypeConstraint* tc;
  const VMTypeIntersectionConstraint* ubs;
  Slot slot;
};

struct LdClsPropOptions {
  const ReadonlyOp readOnlyCheck;
  bool raise;
  bool ignoreLateInit;
  bool writeMode;
};

ClsPropLookup ldClsPropAddrKnown(IRGS&, const Class*, const StringData*,
                                 bool, bool, ReadonlyOp);
ClsPropLookup ldClsPropAddr(IRGS&, SSATmp*, SSATmp*, const LdClsPropOptions&);

//////////////////////////////////////////////////////////////////////

/*
 * Emit an access to a global named 'name', potentially profiling the
 * access, or taking advantage of existing profiling information. The
 * global may be loaded from RDS instead of the normal location.
 *
 * The callbacks are:
 *
 * SSATmp* get(Block*)
 * SSATmp* success(SSATmp*, Type)
 * SSATmp* fail()
 *
 * get() should return a Lval to the global in question, and is used
 * for the "vanilla" case. If the access can fail, it should branch to
 * the given block.
 *
 * success() is called when a Lval to the global is successfully
 * obtained. This can be from get(), or from a RDS slot.
 *
 * fail() is called when the Lval isn't present. This can be a result
 * of the RDS slot not being initialized, or get() branching to the
 * given block.
 *
 * If 'checkType' is true, then a check on the global's type might be
 * done (depending on profiling information). The specific type is
 * passed to success().
 *
 * NB: This might side-exit, so no side-effects should be committed
 * before this is called.
 */
template <typename G, typename S, typename F>
SSATmp* profiledGlobalAccess(IRGS& env, SSATmp* name,
                             G get, S success, F fail,
                             bool checkType) {
  assertx(name->isA(TStr));

  auto const profiling = isProfiling(env.context.kind);

  // The non-RDS case:
  auto const normal = [&] {
    return cond(
      env,
      [&] (Block* taken) {
        // Profile the global if we're profiling. This will record
        // whether the global is present and it's type.
        if (profiling && shouldProfileGlobals()) {
          gen(env, ProfileGlobal, name);
        }
        // Use the generic getter
        return get(taken);
      },
      [&] (SSATmp* ptr) { return success(ptr, TCell); },
      [&] { return fail(); }
    );
  };

  // We can use RDS if we statically know the name, we're not
  // profiling, and we have a bound link for this name.
  if (!name->hasConstVal(TStr) || profiling) return normal();
  auto const link = linkForGlobal(name->strVal());
  if (!link.bound()) return normal();
  auto const handle = link.handle();

  return cond(
    env,
    [&] (Block* taken) {
      // Load from RDS:
      gen(env, CheckRDSInitialized, taken, RDSHandleData { handle });
      auto const lval = gen(
        env,
        ConvPtrToLval,
        gen(env, LdRDSAddr, RDSHandleAndType { handle, TCell }, TPtrToGbl)
      );

      if (checkType) {
        // If the caller asked for refined type, do a check against
        // the profiled type (if any). If the type check fails, just
        // side-exit.
        auto const type = predictedTypeForGlobal(name->strVal());
        if (type < TCell) {
          gen(env, CheckTypeMem, type, makeExitSlow(env), lval);
          // Recalculate the RDS addr. This may seem wasteful, but
          // LdRDSAddr can be compiled away in most cases, and it lets
          // us provide a more refined type for pointer analysis.
          return success(
            gen(
              env,
              ConvPtrToLval,
              gen(env, LdRDSAddr, RDSHandleAndType { handle, type }, TPtrToGbl)
            ),
            type
          );
        }
      }
      // No type-check, it's just a TCell.
      return success(lval, TCell);
    },
    [&] (SSATmp* ptr) { return ptr; },
    [&] {
      // If profiling indicated that this global is almost always
      // present, side-exit if it's not. This keeps the main trace
      // cleaner.
      if (globalMainlyPresent(name->strVal())) {
        hint(env, Block::Hint::Unused);
        gen(env, Jmp, makeExitSlow(env));
        return cns(env, TBottom);
      }
      // Otherwise it fails enough that we don't want to
      // side-exit. Use the failure case.
      return fail();
    }
  );
}

//////////////////////////////////////////////////////////////////////

}}}

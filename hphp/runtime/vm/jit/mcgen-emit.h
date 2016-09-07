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

#ifndef incl_HPHP_JIT_MCGEN_EMIT_H_
#define incl_HPHP_JIT_MCGEN_EMIT_H_

#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include <memory>

namespace HPHP {

struct Func;

namespace jit {

struct ProfTransRec;

namespace mcgen {

/*
 * The state of a partially-complete translation.
 *
 * It is used to transfer context between translate() and emitTranslation()
 * when the initial phase of translation can be done without the write lease.
 */
struct TransEnv {
  explicit TransEnv(const TransArgs& args) : args(args) {}

  TransEnv(TransEnv&&) = default;
  TransEnv& operator=(TransEnv&&) = default;

  /*
   * Context for the translation process.
   */
  TransArgs args;
  FPInvOffset initSpOffset;
  TransID transID{kInvalidTransID};

  /*
   * hhir and vasm units. Both will be set iff bytecode -> hhir lowering was
   * successful (hhir -> vasm lowering never fails).
   */
  std::unique_ptr<IRUnit> unit;
  std::unique_ptr<Vunit> vunit;

  /*
   * Metadata collected during bytecode -> hhir lowering.
   */
  PostConditions pconds;
  Annotations annotations;
};

/*
 * Returns a bool indicated whether or not the global translation limit has
 * been reached.
 */
bool canTranslate();

/*
 * Invalidate the SrcDB entry for sk.
 */
void invalidateSrcKey(SrcKey sk);

/*
 * Invalidate the SrcDB entries for func's SrcKeys that have any
 * Profile translation.
 */
void invalidateFuncProfSrcKeys(const Func* func);

/*
 * Emit machine code for env. Returns nullptr if the global translation limit
 * has been reached, generates an interp request if vunit is null or codegen
 * fails.
 */
TCA emitTranslation(TransEnv env);

/*
 * Emit a new prologue for func-- returns nullptr if the global translation
 * limit has been reached.
 */
TCA emitFuncPrologue(Func* func, int argc, TransKind kind);

/*
 * Smashes the callers of the prologue for rec, and creates a new optimized
 * DV initializer funclet if one is associated with this prologue. If a funclet
 * is emitted and its srckey is the same as triggerSk, its address is returned.
 */
TCA smashFuncPrologue(TCA start, ProfTransRec* rec, SrcKey triggerSk,
                      bool& emittedDVInit);

}}}

#endif

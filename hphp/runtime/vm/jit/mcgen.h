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

#ifndef incl_HPHP_JIT_MCGEN_H_
#define incl_HPHP_JIT_MCGEN_H_

#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP {

struct ActRec;

namespace jit {

struct IRUnit;
struct Vunit;

/*
 * Arguments for the translate() entry points in Translator.
 *
 * These include a variety of flags that help decide what to translate.
 */
struct TransArgs {
  explicit TransArgs(SrcKey sk) : sk{sk} {}

  SrcKey sk;
  Annotations annotations;
  TransFlags flags{0};
  TransID transId{kInvalidTransID};
  TransKind kind{TransKind::Invalid};
  RegionDescPtr region{nullptr};
};

/*
 * The state of a partially-complete translation.
 *
 * It is used to transfer context between translate() and emitTranslation()
 * when the initial phase of translation can be done without the write lease.
 */
struct TransEnv {
  explicit TransEnv(const TransArgs& args) : args(args) {}
  ~TransEnv();

  TransEnv(TransEnv&&) = default;
  TransEnv& operator=(TransEnv&&) = default;

  /*
   * Context for the translation process.
   */
  TransArgs args;
  FPInvOffset initSpOffset;
  TransID transID{kInvalidTransID};

  /*
   * If set regenerate the prologue for this trans rec. This is intended for
   * optimized translations of DV initializers which should always follow their
   * prologues.
   */
  ProfTransRec* prologue{nullptr};

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

////////////////////////////////////////////////////////////////////////////////

namespace mcgen {

/*
 * Look up or translate a func prologue or func body.
 */
TCA getFuncPrologue(Func* func, int nPassed);

/*
 * Create a live or profile retranslation based on args.
 *
 * Will return null if the write-lease could not be obtained or a translation
 * could not be generated.
 */
TCA retranslate(TransArgs args, const RegionContext& ctx);

/*
 * Generate an optimized translation for sk using profile data from transId.
 */
TCA retranslateOpt(SrcKey sk, TransID transId);

/*
 * Called once when the JIT is activated to initialize internal mcgen structures
 */
void processInit();

/*
 * True iff mcgen::processInit() has been called
 */
bool initialized();

/*
 * Return the timestamp at which mcgen::processInit was called
 */
int64_t jitInitTime();

/*
 * Whether we should dump TC annotations for translations of `func' of
 * `transKind'.
 */
bool dumpTCAnnotation(const Func& func, TransKind transKind);

}}}

#endif

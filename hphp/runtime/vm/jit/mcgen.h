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

#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP {

struct ActRec;

namespace jit {

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
 * Whether we should try profile-guided optimization when translating `sk'.
 */
bool profileSrcKey(SrcKey sk);

/*
 * Whether we should emit a translation of kind for func.
 */
bool shouldTranslate(const Func* func, TransKind kind);

/*
 * Look up or translate a func prologue or func body.
 */
TCA getFuncPrologue(Func* func, int nPassed, ActRec* ar = nullptr,
                    bool forRegeneratePrologue = false);

/*
 * Get the entry point for the body of func. The returned address will be for
 * a func-body dispatch should the function contain DV initializers, otherwise
 * it will correspond to the translation for the entry src-key of func.
 */
TCA getFuncBody(Func* func);

/*
 * Find or create a translation for `args'. Returns TCA of "best" current
 * translation. May return nullptr if it is currently impossible to create a
 * translation.
 */
TCA getTranslation(const TransArgs& args);

/*
 * Create a live or profile retranslation based on args.
 *
 * Will return null if the write-lease could not be obtained or a translation
 * could not be generated.
 */
TCA retranslate(TransArgs args);

/*
 * Generate an optimized translation for sk using profile data from transId.
 */
TCA retranslateOpt(SrcKey sk, TransID transId);

}}

#endif

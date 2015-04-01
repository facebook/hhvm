/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_TRANSLATE_REGION_H_
#define incl_HPHP_TRANSLATE_REGION_H_

#include "hphp/runtime/vm/jit/types.h"  // TransFlags
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/prof-src-key.h"

namespace HPHP { namespace jit {

struct IRGS;

//////////////////////////////////////////////////////////////////////

enum class TranslateResult {
  Failure,
  Retry,
  Success
};
const char* show(TranslateResult);

/*
 * Blacklisted instruction set.
 *
 * Used by translateRegion() to track instructions that must be interpreted.
 */
using RegionBlacklist = ProfSrcKeySet;

/*
 * Translate `region'.
 *
 * The `toInterp' RegionBlacklist is a set of instructions which must be
 * interpreted.  When an instruction fails in translation, Retry is returned,
 * and the instruction is added to `interp' so that it will be interpreted on
 * the next attempt.
 */
TranslateResult translateRegion(IRGS&,
                                const RegionDesc&,
                                RegionBlacklist& toInterp,
                                TransFlags trflags,
                                PostConditions& pconds);

//////////////////////////////////////////////////////////////////////

}}

#endif

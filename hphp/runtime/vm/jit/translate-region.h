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
 * Data used by translateRegion() to pass information between retries.
 */
struct TranslateRetryContext {
  // Instructions that must be interpreted.
  ProfSrcKeySet toInterp;

  // Regions to not inline
  std::unordered_set<ProfSrcKey, ProfSrcKey::Hasher> inlineBlacklist;
};

/*
 * Translate `region'.
 *
 * The caller is expected to continue calling translateRegion() until either
 * Success or Failure is returned.  Otherwise, Retry is returned, and the
 * caller is responsible for threading the same RetryContext through to the
 * retried translations.
 */
TranslateResult translateRegion(IRGS& irgs,
                                const RegionDesc& region,
                                TranslateRetryContext& retry,
                                TransFlags trflags,
                                PostConditions& pconds);

//////////////////////////////////////////////////////////////////////

}}

#endif

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
#ifndef incl_HPHP_TC_RELOCATE_H_
#define incl_HPHP_TC_RELOCATE_H_

#include <set>
#include <cstdio>
#include <vector>

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP {

struct CodeCache;

namespace jit {

struct AsmInfo;
struct IRUnit;

//////////////////////////////////////////////////////////////////////

namespace tc {

std::string perfRelocMapInfo(
  TCA start, TCA end,
  TCA coldStart, TCA coldEnd,
  SrcKey sk, int argNum,
  const GrowableVector<IncomingBranch> &incomingBranches,
  CGMeta& fixups);

bool relocateNewTranslation(TransLoc& loc, CodeCache::View cache,
                            CGMeta& fixups,
                            TCA* adjust = nullptr);

/*
 * Relocate a new translation into a free region in the TC and update the
 * TransLoc.
 *
 * Attempt to relocate the main, cold, and frozen portions of the translation
 * loc into memory freed memory in the TC their respective code blocks. In
 * addition, reusable stubs associated with this translation will be relocated
 * to be outside of loc so that they can be managed separately.
 */
void tryRelocateNewTranslation(SrcKey sk, TransLoc& loc,
                               CodeCache::View code, CGMeta& fixups);

//////////////////////////////////////////////////////////////////////

}}}

#endif

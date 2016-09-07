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

#ifndef incl_HPHP_JIT_MC_GENERATOR_H_
#define incl_HPHP_JIT_MC_GENERATOR_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/util/data-block.h"
#include "hphp/util/eh-frame.h"

#include <folly/Optional.h>

#include <fstream>
#include <mutex>
#include <utility>
#include <vector>

namespace HPHP {

struct ActRec;
struct Func;
struct Unit;

namespace jit {

struct CGMeta;
struct MCGenerator;
struct TransArgs;

///////////////////////////////////////////////////////////////////////////////

extern MCGenerator* mcg;

///////////////////////////////////////////////////////////////////////////////

/*
 * MCGenerator handles the machine-level details of code generation (e.g.,
 * translation cache entry, code smashing, code cache management) and delegates
 * the bytecode-to-asm translation process to translateRegion().
 *
 * There are a number of different locks that protect data owned by or related
 * to MCGenerator, described here in ascending Rank order (see
 * hphp/util/rank.h).
 *
 * - Global write lease (Translator::WriteLease()). The write lease has a
 *   number of heuristics that are used to ensure we lay out Live translations
 *   in a good order. When Eval.JitConcurrently == 0, holding the write lease
 *   gives the owning thread exclusive permission to write to the Translation
 *   Cache (TC) and all associated metadata tables. When Eval.JitConcurrently >
 *   0, the global write lease is only used to influence code layout and
 *   provides no protection against data races. In the latter case, the
 *   remaining locks are used to protect the integrity of the TC and its
 *   metadata:
 *
 * - Func-specific write leases (Func* argument to LeaseHolder). These locks
 *   give the owning thread exclusive permission to write to the SrcRec for
 *   translations of the corresponding Func, modify its prologue table, and
 *   read/write any ProfTransRecs for translations in the Func. Note that the
 *   Func-specific lease *must* be held to modify any Func-specific metadata
 *   when Eval.JitConcurrently > 0, even if the current thread holds the global
 *   write lease.
 *
 * - MCGenerator::lockCode() gives the owning thread exclusive permission to
 *   modify the contents and frontiers of all code/data blocks in m_code.
 *
 * - MCGenerator::lockMetadata() gives the owning thread exclusive permission
 *   to modify the metadata tables owned by mcg (m_fixupMap, or m_srcDB, etc).
 */
struct MCGenerator {
  MCGenerator();
  ~MCGenerator();

  MCGenerator(const MCGenerator&) = delete;
  MCGenerator& operator=(const MCGenerator&) = delete;
};

///////////////////////////////////////////////////////////////////////////////

}}

#endif

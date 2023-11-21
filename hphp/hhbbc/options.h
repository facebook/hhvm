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

#include <string>
#include <utility>

#include "hphp/util/hash.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/hash-set.h"
#include "hphp/util/optional.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

// Map case-insensitive class name => Set<case-sensitive method name>
using MethodMap = hphp_fast_string_imap<hphp_fast_string_set>;

//////////////////////////////////////////////////////////////////////

/*
 * Publically-settable options that control compilation.
 */
struct Options {
  /*
   * When debugging, it can be useful to ask for certain functions to be traced
   * at a higher level than the rest of the program.
   */
  MethodMap TraceFunctions;

  //////////////////////////////////////////////////////////////////////

  /*
   * Flags for various limits on when to perform widening operations.
   * See analyze.cpp for details.
   */
  uint32_t analyzeFuncWideningLimit = 8;
  uint32_t analyzeClassWideningLimit = 6;

  /*
   * When to stop refining return types.
   *
   * This needs to be limited because types can walk downwards in our
   * type lattice indefinitely.  The index never contains incorrect
   * return types, since the return types only shrink, which means we
   * can just stop refining a return type whenever we want to without
   * causing problems.
   *
   * For an example of where this can occur, imagine the analysis of the
   * following function:
   *
   *    function foo() { return array('x' => foo()); }
   *
   * Each time we visit `foo', we'll discover a slightly smaller return
   * type, in a downward-moving sequence that would never terminate:
   *
   *   InitCell, CArrN(x:InitCell), CArrN(x:CArrN(x:InitCell)), ...
   */
  uint32_t returnTypeRefineLimit = 8;

  /*
   * Limit public static property refinement for the same reason.
   */
  uint32_t publicSPropRefineLimit = 8;

  //////////////////////////////////////////////////////////////////////

  /*
   * Only track the subclasses of a class if the total number of
   * subclasses is below this limit. Some classes can have
   * pathologically large subclass lists, which become very expensive
   * to process.
   */
  uint32_t preciseSubclassLimit = 10000;

  //////////////////////////////////////////////////////////////////////

  /*
   * If true, analyze calls to functions in a context-sensitive way.
   *
   * Disabled by default because its slow, with very little gain.
   */
  bool ContextSensitiveInterp = false;

  //////////////////////////////////////////////////////////////////////
  // Flags below this line perform optimizations that intentionally
  // may have user-visible changes to program behavior.
  //////////////////////////////////////////////////////////////////////

  /*
   * If set, replace the File and Dir bytecodes with constant strings,
   * using this as the SourceRoot.
   */
  Optional<std::string> SourceRootForFileBC;

  /*
   * Whether to dump core when crashing.
   */
  bool CoreDump = true;

  //////////////////////////////////////////////////////////////////////////////
  // Flags below this line do not affect HHBBC in a way that matters
  // to its extern-worker jobs. Therefore they are not
  // serialized. Adding unnecessary things to the serialization can
  // cause unnecessary cache misses.
  //////////////////////////////////////////////////////////////////////////////

  /*
   * Whether to produce extended stats information.  (Takes extra
   * time.)
   */
  bool extendedStats = false;

  /*
   * The filepath where to save the stats file.  If the path is empty, then we
   * save the stats file to a temporary file.
   */
  std::string stats_file;

  /*
   * If non-empty, dump jemalloc memory profiles at key points during
   * the build, using this as a prefix.
   */
  std::string profileMemory;

  /*
   * Extern-worker config
   */
  std::string ExternWorkerUseCase;
  std::string ExternWorkerWorkingDir;
  bool ExternWorkerForceSubprocess = false;
  bool ExternWorkerAllowFallback = true;
  bool ExternWorkerUseExecCache = true;
  bool ExternWorkerCleanup = true;
  bool ExternWorkerUseRichClient = true;
  bool ExternWorkerUseZippyRichClient = false;
  bool ExternWorkerUseP2P = false;
  bool ExternWorkerVerboseLogging = false;
  bool ExternWorkerAsyncCleanup = true;
  int ExternWorkerTimeoutSecs = 0;
  int ExternWorkerThrottleRetries = -1;
  int ExternWorkerThrottleBaseWaitMSecs = -1;
  size_t ExternWorkerCASConnectionCount = 16;
  size_t ExternWorkerEngineConnectionCount = 4;
  size_t ExternWorkerActionCacheConnectionCount = 16;
  std::string ExternWorkerFeaturesFile;

  template <typename SerDe> void serde(SerDe& sd) {
    sd(TraceFunctions, string_lessi{}, std::less<std::string>{})
      (analyzeFuncWideningLimit)
      (analyzeClassWideningLimit)
      (returnTypeRefineLimit)
      (publicSPropRefineLimit)
      (preciseSubclassLimit)
      (ContextSensitiveInterp)
      (SourceRootForFileBC)
      (CoreDump)
      ;
  }
};
extern Options options;

}

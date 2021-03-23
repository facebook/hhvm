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

#include <stdint.h>
#include <memory>

namespace HPHP { namespace Treadmill {

//////////////////////////////////////////////////////////////////////

/*
 * List of potentiall users of the treadmill, useful for debugging halts
 */
enum class SessionKind {
  None,
  DebuggerClient,
  PreloadRepo,
  Watchman,
  Vsdebug,
  FactsWorker,
  CLIServer,
  AdminPort,
  HttpRequest,
  RpcRequest,
  TranslateWorker,
  Retranslate,
  RetranslateAll,
  ProfData,
  UnitTests,
  CompileRepo,
  HHBBC,
  CompilerEmit,
  CompilerAnalysis,
  CLISession,
  UnitReaper
};

/*
 * An invalid request index.
 */
constexpr int64_t kInvalidRequestIdx = -1;
/*
 * Return the current thread's index.
 */
int64_t requestIdx();

/*
 * The Treadmill allows us to defer work until all currently
 * outstanding requests have finished.  We hook request start and
 * finish to know when these events happen.
 */
void startRequest(SessionKind session_kind);
void finishRequest();

/*
 * Returns the unique GenCount identifying the oldest request in
 * flight (or zero if there is none).
 */
int64_t getOldestRequestGenCount();

/*
 * Returns the oldest start time in seconds of all requests in flight.
 * If there are no requests in flight the return value is 0.
 * The value returned should be used as a conservative and true value
 * for the oldest start time of any request in flight. In that sense
 * the value is safe to compare against in a less-than relationship.
 */
int64_t getOldestStartTime();

/*
 * Returns for how long (wall time) the oldest request in flight has
 * been running, in seconds.
 */
int64_t getAgeOldestRequest();

/*
 * Returns the unique GenCount identifying this request.
 */
int64_t getRequestGenCount();

/*
 * Ask for memory to be freed (as in free, not delete) by the next
 * appropriate treadmill round.
 */
void deferredFree(void*);

/*
 * Used to get debug information about the treadmill.
 */
std::string dumpTreadmillInfo();

/*
 * Schedule a function to run on the next appropriate treadmill round.
 *
 * The function will be called from the base mutex rank.
 *
 * Note: the function passed here escapes (naturally).  If you use a
 * lambda here, copy things into the lambda by value.
 *
 * Important: f() must not throw an exception.
 */
template<class F> void enqueue(F&& f);

struct Session final {
  Session(SessionKind session_kind) { startRequest(session_kind); }
  ~Session() { finishRequest(); }
  Session(Session&&) = delete;
  Session& operator=(Session&&) = delete;
};

//////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/vm/treadmill-inl.h"

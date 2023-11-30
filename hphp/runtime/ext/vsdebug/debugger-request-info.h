/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/watchman.h"

namespace HPHP::VSDEBUG {

// Structure to represent the state of a single request.
struct DebuggerRequestInfo {

  // Request flags are read by the debugger hook prior to acquiring
  // the debugger lock, so we can short-circuit and avoid calling
  // into the debugger in certain cases.
  union {
    struct {
      uint32_t memoryLimitRemoved : 1;
      uint32_t compilationUnitsMapped : 1;
      uint32_t doNotBreak : 1;
      uint32_t outputHooked : 1;
      uint32_t requestUrlInitialized : 1;
      uint32_t terminateRequest : 1;
      uint32_t unresolvedBps : 1;
      uint32_t alive : 1;
      uint32_t unused : 24;
    } m_flags;
    uint32_t m_allFlags;
  };

  const char* m_stepReason {nullptr};
  CommandQueue m_commandQueue;
  RequestBreakpointInfo* m_breakpointInfo {nullptr};

  // Object IDs sent to the debugger client.
  std::unordered_map<int, unsigned int> m_scopeIds;
  std::unordered_map<void*, unsigned int> m_objectIds;
  std::unordered_map<unsigned int, ServerObject*> m_serverObjects;

  // Compilation units produced by debugger evaluations for this
  // request - to be cleaned up when the request exits.
  std::vector<std::unique_ptr<HPHP::Unit>> m_evaluationUnits;

  struct {
    std::string path {""};
    int line {0};
  } m_runToLocationInfo;

  // Info for allowing us to step over multi-line statements without hitting
  // the statement multiple times.
  std::vector<StepNextFilterInfo> m_nextFilterInfo;

  // Number of evaluation frames on this request's stack right now.
  int m_evaluateCommandDepth {0};

  // Number of recursive calls into processCommandsQueue for this request
  // right now.
  int m_pauseRecurseCount {0};

  // Number of times this request has entered the command queue since starting.
  unsigned int m_totalPauseCount {0};

  // Non-TLS copy of the request's URL to display in the client. Each request
  // has this string in its ExecutionContext, but since that is thread-local,
  // we cannot get at that copy when responding to a ThreadsRequest from the
  // debugger client, so the debugger needs a copy.
  std::string m_requestUrl {""};

  // Last time the file change query was run. Valid only for the REPL request.
  Optional<watchman::Clock> m_lastClock {};

  // Did this request hit any breakpoint?
  bool m_firstBpHit {false};
};
} // namespace HPHP::VSDEBUG


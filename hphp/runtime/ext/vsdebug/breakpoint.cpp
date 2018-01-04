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

#include "hphp/runtime/ext/vsdebug/breakpoint.h"

namespace HPHP {
namespace VSDEBUG {

// Breakpoint IDs need to never be reused, and so this needs to be global and
// not tied to the lifetime of BreakpointManager. The reason for this is if a
// client sets some breakpoints and then disconnects, the breakpoint IDs could
// remain installed on in-progress requests that have not called back into the
// debugger yet. If a new client connects and installs new breakpoints, and the
// IDs were reused for the new session, we won't be able to tell if a request
// hit a bp for the previous debugger client, and we'll overactively re-hit.
static int g_nextBreakpointId {0};

Breakpoint::Breakpoint(
  int id,
  int line,
  int column,
  const std::string path,
  const std::string condition,
  const std::string hitCondition
) : m_id(id),
    m_type(BreakpointType::Source),
    m_line(line),
    m_column(column),
    m_path(path),
    m_resolvedLocation({0}),
    m_hitCount(0) {

  updateConditions(condition, hitCondition);
}

void Breakpoint::updateConditions(
  std::string condition,
  std::string hitCondition
) {
  m_condition = condition;
  m_hitCondition = hitCondition;
  // TODO: compile condition and hitCondition here.
}

BreakpointManager::BreakpointManager(Debugger* debugger) :
  m_debugger(debugger) {
}

BreakpointManager::~BreakpointManager() {
}

void BreakpointManager::setExceptionBreakMode(ExceptionBreakMode mode) {
  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Exception break mode set to: %d",
    (int)mode
  );
  m_exceptionSettings.m_breakMode = mode;
}

ExceptionBreakMode BreakpointManager::getExceptionBreakMode() const {
  return m_exceptionSettings.m_breakMode;
}

const std::unordered_set<int> BreakpointManager::getAllBreakpointIds() const {
  std::unordered_set<int> ids;
  for (auto it = m_breakpoints.begin(); it != m_breakpoints.end(); it++) {
    ids.insert(it->first);
  }
  return ids;
}

int BreakpointManager::addBreakpoint(
  int line,
  int column,
  const std::string& path,
  const std::string& condition,
  const std::string& hitCondition
) {
  const int id = ++g_nextBreakpointId;
  Breakpoint bp = Breakpoint(id, line, column, path, condition, hitCondition);

  // Add the new breakpoint to the breakpoint map.
  m_breakpoints.emplace(id, bp);

  // If this is the first time we've seen a bp for this file, create a new
  // set of breakpoint IDs for this file.
  auto it = m_sourceBreakpoints.find(bp.m_path);
  if (it == m_sourceBreakpoints.end()) {
    m_sourceBreakpoints.emplace(bp.m_path, std::unordered_set<int>());
    it = m_sourceBreakpoints.find(bp.m_path);
  }

  // Add this ID to the set of breakpoints in this file.
  assert(it != m_sourceBreakpoints.end());
  std::unordered_set<int>& fileBreakpoints = it->second;
  fileBreakpoints.emplace(id);

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Stored new breakpoint request (id = %d), (file = %s), (line = %d)",
    id,
    bp.m_path.c_str(),
    bp.m_line
  );

  // Tell the client the bp was installed.
  sendBreakpointEvent(id, ReasonNew);
  return id;
}

void BreakpointManager::removeBreakpoint(int id) {
  const auto it = m_breakpoints.find(id);
  if (it == m_breakpoints.end()) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Not removing breakpoint with id: %d, no such breakpoint exists.",
      id
    );
    return;
  }

  // Remove the breakpoint.
  const std::string filePath = it->second.m_path;
  m_breakpoints.erase(it);

  // If it was resolved, remove it from the resolved set.
  const auto resolvedIt = m_resolvedBreakpoints.find(id);
  if (resolvedIt != m_resolvedBreakpoints.end()) {
    m_resolvedBreakpoints.erase(resolvedIt);
  }

  // Remove it from the per-file list.
  const auto fileIt = m_sourceBreakpoints.find(filePath);
  if (fileIt != m_sourceBreakpoints.end()) {
    std::unordered_set<int>& set = fileIt->second;
    const auto setIt = set.find(id);
    if (setIt != set.end()) {
      set.erase(setIt);
    }

    if (set.empty()) {
      m_sourceBreakpoints.erase(fileIt);
    }
  }

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Client removed breakpoint: (id = %d), (file = %s), (line = %d)",
    id,
    it->second.m_path.c_str(),
    it->second.m_line
  );

  // Tell the client the breakpoint is gone.
  sendBreakpointEvent(id, ReasonRemoved);
}

void BreakpointManager::onBreakpointResolved(
  int id,
  int startLine,
  int endLine,
  int startColumn,
  int endColumn,
  const std::string& path
) {
  Breakpoint* bp = nullptr;
  const auto it = m_breakpoints.find(id);
  if (it != m_breakpoints.end()) {
    bp = &it->second;
  }

  if (bp == nullptr) {
    // This is okay, the breakpoint could have been deleted by the client
    // before a request thread had a chance to resolve it.
    return;
  }

  if (isBreakpointResolved(id)) {
    // Already resolved by another request.
    return;
  }

  // Note: this is a bit strange. The client expects a single resolved
  // notification if/when a breakpoint is placed in the program. For most
  // backends, that applies to the entire program. For HHVM, each request
  // needs to resolve the breakpoint on its own. We currently send resolved
  // for the first request that resolves it, but have no way to indicate
  // if the breakpoint is resolved for further requests.
  m_resolvedBreakpoints.insert(id);

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Breakpoint ID %d resolved!",
    id
  );

  // Update the breakpoint with the calibrated line info.
  bp->m_resolvedLocation.m_startLine = startLine;
  bp->m_resolvedLocation.m_endLine = endLine;
  bp->m_resolvedLocation.m_startCol = startColumn;
  bp->m_resolvedLocation.m_endCol = endColumn;
  bp->m_resolvedLocation.m_path = path;

  sendBreakpointEvent(id, ReasonChanged);
}

// Helper to honor client's lines start at 0 or 1 preference.
int BreakpointManager::adjustLineNumber(
  const ClientPreferences& preferences,
  int line,
  bool column
) {
  if (column) {
    return preferences.columnsStartAt1 ? line : line - 1;
  }
  return preferences.linesStartAt1 ? line : line - 1;
}

void BreakpointManager::sendBreakpointEvent(
  int breakpointId,
  const char* reason
) {
  const Breakpoint* breakpoint = getBreakpointById(breakpointId);
  if (breakpoint == nullptr) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Not sending breakpoint event for id %d. Breakpoint not found.",
      breakpointId
    );
    return;
  }

  bool resolved = isBreakpointResolved(breakpointId);
  ClientPreferences preferences = m_debugger->getClientPreferences();
  folly::dynamic bp = folly::dynamic::object;
  bp["id"] = breakpointId;
  bp["verified"] = resolved;

  if (resolved) {
    bp["line"] = adjustLineNumber(
                   preferences,
                   breakpoint->m_resolvedLocation.m_startLine,
                   false
                 );
    bp["endLine"] = adjustLineNumber(
                      preferences,
                      breakpoint->m_resolvedLocation.m_endLine,
                      false
                    );
    bp["column"] = adjustLineNumber(
                      preferences,
                      breakpoint->m_resolvedLocation.m_startCol,
                      true
                    );
    bp["endColumn"] = adjustLineNumber(
                        preferences,
                        breakpoint->m_resolvedLocation.m_endCol,
                        true
                      );

    bp["originalLine"] = adjustLineNumber(
                           preferences,
                           breakpoint->m_line,
                           false
                         );
  } else {
    bp["line"] = adjustLineNumber(
                   preferences,
                   breakpoint->m_line,
                   false
                 );
    bp["column"] = adjustLineNumber(
                     preferences,
                     breakpoint->m_column,
                     false
                   );
  }

  folly::dynamic source = folly::dynamic::object;
  source["path"] = resolved
    ? breakpoint->m_resolvedLocation.m_path
    : breakpoint->m_path;
  bp["source"] = source;

  // Breakpoint hit count.
  // NOTE: This is a Nuclide-specific extension to the protocol.
  bp["nuclide_hitCount"] = breakpoint->m_hitCount;

  folly::dynamic event = folly::dynamic::object;
  event["reason"] = reason;
  event["breakpoint"] = bp;
  m_debugger->sendEventMessage(event, "breakpoint");
}

Breakpoint* BreakpointManager::getBreakpointById(int id) {
  auto it = m_breakpoints.find(id);
  if (it != m_breakpoints.end()) {
    return &it->second;
  }

  return nullptr;
}

const std::unordered_set<int> BreakpointManager::getBreakpointIdsByFile(
  const std::string& sourcePath
) const {
  const auto it = m_sourceBreakpoints.find(sourcePath);
  if (it != m_sourceBreakpoints.end()) {
    // Return a copy of the vector.
    // Note: we expect the list of breakpoints for any given debugger
    // session to be pretty small.
    return std::unordered_set<int>(it->second);
  }

  return std::unordered_set<int>();
}

bool BreakpointManager::isBreakpointResolved(int id) const {
  return m_resolvedBreakpoints.find(id) != m_resolvedBreakpoints.end();
}

void BreakpointManager::onBreakpointHit(int id) {
  const auto it = m_breakpoints.find(id);
  if (it != m_breakpoints.end()) {
    Breakpoint* bp = &it->second;
    bp->m_hitCount++;
    sendBreakpointEvent(id, ReasonChanged);
  }
}

}
}

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
#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/logging.h"
#include "hphp/runtime/vm/runtime-compiler.h"
#include "hphp/runtime/base/execution-context.h"

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
  const std::string& path,
  const std::string& condition,
  const std::string& hitCondition
) : m_id(id),
    m_type(BreakpointType::Source),
    m_line(line),
    m_column(column),
    m_path(path),
    m_filePath(std::filesystem::path(path)),
    m_function(""),
    m_hitCount(0) {

  updateConditions(condition, hitCondition);
}

Breakpoint::Breakpoint(
  int id,
  const std::string& function,
  const std::string& condition,
  const std::string& hitCondition
) : m_id(id),
    m_type(BreakpointType::Function),
    m_line(-1),
    m_column(-1),
    m_path(""),
    m_filePath(std::filesystem::path("")),
    m_function(function),
    m_hitCount(0) {

    updateConditions(condition, hitCondition);
}

Breakpoint::~Breakpoint() {
  for (auto it = m_unitCache.begin(); it != m_unitCache.end();) {
    if (it->second != nullptr) {
      delete it->second;
    }
    it = m_unitCache.erase(it);
  }
}

bool Breakpoint::isRelativeBp() const {
  return !m_filePath.is_absolute();
}

void Breakpoint::clearCachedConditionUnit(request_id_t requestId) {
  auto it = m_unitCache.find(requestId);
  if (it != m_unitCache.end()) {
    if (it->second != nullptr) {
      delete it->second;
    }
    m_unitCache.erase(it);
  }
}

void Breakpoint::updateConditions(
  const std::string& condition,
  const std::string& hitCondition
) {
  m_condition = condition;
  m_hitCondition = hitCondition;
  m_unitCache.clear();

  // TODO: Deal with hit condition breakpoints.
}

BreakpointManager::BreakpointManager(Debugger* debugger) :
  m_debugger(debugger) {
}

BreakpointManager::~BreakpointManager() {
  for (auto it = m_breakpoints.begin(); it != m_breakpoints.end();) {
    delete it->second;
    it = m_breakpoints.erase(it);
  }

  assertx(m_breakpoints.empty());
}

bool BreakpointManager::bpMatchesPath(
  const Breakpoint* bp,
  const std::filesystem::path& unitPath
) {
  if (bp->m_type != BreakpointType::Source) {
    return false;
  }

  // A breakpoint matches the specified path if the breakpoint
  // has an absolute path and the full file path matches exactly OR
  // the breakpoint has a relative path and the filenames match.
  const auto& bpPath = bp->m_filePath;
  if (bpPath.is_absolute()) {
    return bpPath == unitPath;
  } else {
    return bpPath.filename() == unitPath.filename();
  }
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

const std::unordered_set<int> BreakpointManager::getBreakpointIdsForPath(
  const std::string& unitPath
) const {
  std::unordered_set<int> ids;
  const auto path = std::filesystem::path(unitPath);
  for (auto it = m_breakpoints.begin(); it != m_breakpoints.end(); it++) {
    if (bpMatchesPath(it->second, path)) {
      ids.insert(it->first);
    }
  }
  return ids;
}

ResolvedLocation BreakpointManager::bpResolvedInfoForFile(
  const Breakpoint* bp,
  const std::string& filePath
) {
  for (auto it = bp->m_resolvedLocations.begin();
       it != bp->m_resolvedLocations.end();
       it++) {

    if (it->m_path == filePath) {
      return *it;
    }
  }

  ResolvedLocation empty = {0};
  return empty;
}

int BreakpointManager::addSourceLineBreakpoint(
  int line,
  int column,
  const std::string& path,
  const std::string& condition,
  const std::string& hitCondition
) {
  const int id = ++g_nextBreakpointId;
  const auto bp = new Breakpoint(
    id,
    line,
    column,
    path,
    condition,
    hitCondition
  );

  // Add the new breakpoint to the breakpoint map.
  m_breakpoints.emplace(id, bp);

  // If this is the first time we've seen a bp for this file, create a new
  // set of breakpoint IDs for this file.
  auto it = m_sourceBreakpoints.find(bp->m_path);
  if (it == m_sourceBreakpoints.end()) {
    m_sourceBreakpoints.emplace(bp->m_path, std::unordered_set<int>());
    it = m_sourceBreakpoints.find(bp->m_path);
  }

  // Add this ID to the set of breakpoints in this file.
  assertx(it != m_sourceBreakpoints.end());
  std::unordered_set<int>& fileBreakpoints = it->second;
  fileBreakpoints.emplace(id);

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Stored new breakpoint request (id = %d), (file = %s), (line = %d)",
    id,
    bp->m_path.c_str(),
    bp->m_line
  );

  // Tell the client the bp was installed.
  sendBreakpointEvent(id, ReasonNew);
  return id;
}

void BreakpointManager::onFuncBreakpointResolved(
  Breakpoint& bp,
  const Func* func
) {
  if (func == nullptr ||
      func->unit() == nullptr ||
      func->unit()->filepath() == nullptr) {
    return;
  }

  std::string filePath = func->unit()->filepath()->toCppString();

  auto it = m_sourceBreakpoints.find(filePath);
  if (it == m_sourceBreakpoints.end()) {
    m_sourceBreakpoints.emplace(filePath, std::unordered_set<int>());
    it = m_sourceBreakpoints.find(filePath);
  }

  // Add this ID to the set of breakpoints in this file.
  assertx(it != m_sourceBreakpoints.end());
  std::unordered_set<int>& fileBreakpoints = it->second;
  fileBreakpoints.emplace(bp.m_id);

  ClientPreferences preferences = m_debugger->getClientPreferences();
  onBreakpointResolved(
    bp.m_id,
    adjustLineNumber(
      preferences,
      func->line1(),
      false
    ),
    adjustLineNumber(
      preferences,
      func->line1(),
      false
    ),
    adjustLineNumber(
      preferences,
      1,
      true
    ),
    adjustLineNumber(
      preferences,
      1,
      true
    ),
    filePath,
    func->name()->toCppString()
  );
}

bool BreakpointManager::warningSentForBp(int bpId) const {
  const auto it = m_userNotifyBps.find(bpId);
  return it != m_userNotifyBps.end();
}

void BreakpointManager::sendWarningForBp(
  int bpId,
  std::string& warningMessage
) {
  const auto it = m_userNotifyBps.find(bpId);
  if (it == m_userNotifyBps.end()) {
    m_userNotifyBps.emplace(bpId);
  }

  m_debugger->sendUserMessage(
    warningMessage.c_str(),
    DebugTransport::OutputLevelWarning
  );
}

void BreakpointManager::sendMemoizeWarning(
  int bpId
) {
  if (warningSentForBp(bpId)) {
    return;
  }

  std::string msg = "Breakpoint ";
  msg += std::to_string(bpId);
  msg += " resolved to a function that might be memoized. This means ";
  msg += "that the function results are cached and will not be executed ";
  msg += "the next time the function is invoked with the same parameters. ";
  msg += "As a result, this breakpoint might not be hit. Please see ";
  msg += "https://fburl.com/hhvm_memoize for more information.";

  sendWarningForBp(bpId, msg);
}

void BreakpointManager::sendBpInterceptedWarning(
  request_id_t requestId,
  int bpId,
  std::string& name
) {

  // Only warn once per breakpoint per request.
  if (warningSentForBp(bpId)) {
    return;
  }

  std::string msg = "Breakpoint #";
  msg += std::to_string(bpId);
  msg += " may resolve to a location ";
  msg += "that is intercepted or mocked in request ";
  msg += std::to_string(requestId);
  msg += ". It might therefore not be reachable. ";
  msg += "Double check the breakpoint location. ";
  msg += "If you are using any testing or mocking frameworks, check ";
  msg += "that you have configured them correctly. ";
  msg += "Function ";
  msg += name;
  msg += " has an intercept handler registered.";

  sendWarningForBp(bpId, msg);
}

void BreakpointManager::onFuncIntercepted(
  request_id_t requestId,
  std::string name
) {
  if (name == "") {
    return;
  }

  // Always use lowercase, case-insensitive compare.
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);

  auto requestIt = m_interceptedFuncs.find(requestId);
  if (requestIt == m_interceptedFuncs.end()) {
    m_interceptedFuncs.emplace(requestId, std::unordered_set<std::string>());
    requestIt = m_interceptedFuncs.find(requestId);
  }

  auto interceptedFuncs = requestIt->second;
  if (interceptedFuncs.find(name) == interceptedFuncs.end()) {
    interceptedFuncs.insert(name);
  }

  // Warn about any breakpoints in this request that resolved to
  // functions that match the intercepted function name.
  for (auto it = m_breakpoints.begin(); it != m_breakpoints.end(); it++) {
    auto bpId = it->first;
    const auto bp = it->second;
    if (bp->m_functionFullName == name) {
      sendBpInterceptedWarning(requestId, bpId, name);
    }
  }
}

int BreakpointManager::addFunctionBreakpoint(
  const std::string& function,
  const std::string& condition,
  const std::string& hitCondition
) {
  const int id = ++g_nextBreakpointId;
  Breakpoint* bp = new Breakpoint(id, function, condition, hitCondition);
  // Add the new breakpoint to the breakpoint map.
  m_breakpoints.emplace(id, bp);
  m_fnBreakpoints.emplace(function, id);

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Stored new function breakpoint request (id = %d), (func = %s)",
    id,
    function.c_str()
  );

  // Tell the client the bp was installed.
  sendBreakpointEvent(id, ReasonNew);

  return id;
}

const std::unordered_set<int>
BreakpointManager::getFunctionBreakpoints() const {
  std::unordered_set<int> ids;
  for (auto it = m_fnBreakpoints.begin(); it != m_fnBreakpoints.end(); it++) {
    ids.insert(it->second);
  }
  return ids;
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
  const std::string filePath = it->second->m_path;
  const std::string function = it->second->m_function;
  auto const line = it->second->m_line;

  delete it->second;
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

  // Remove it from the function breakpoint list.
  const auto fnIt = m_fnBreakpoints.find(function);
  if (fnIt != m_fnBreakpoints.end()) {
      m_fnBreakpoints.erase(fnIt);
  }

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Client removed breakpoint: (id = %d), (file = %s), (line = %d)",
    id,
    filePath.c_str(),
    line
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
  const std::string& path,
  std::string functionName
) {
  Breakpoint* bp = nullptr;
  const auto it = m_breakpoints.find(id);
  if (it != m_breakpoints.end()) {
    bp = it->second;
  }

  if (bp == nullptr) {
    // This is okay, the breakpoint could have been deleted by the client
    // before a request thread had a chance to resolve it.
    return;
  }

  if (functionName != "") {
    // Lowercase function names and compare case-insensitive.
    std::transform(
      functionName.begin(),
      functionName.end(),
      functionName.begin(),
      ::tolower
    );

    bp->m_functionFullName = functionName;

    auto interceptedFuncs =
      m_interceptedFuncs.find(m_debugger->getCurrentThreadId());

    if (interceptedFuncs != m_interceptedFuncs.end()) {
      auto interceptIt = interceptedFuncs->second.find(functionName);
      if (interceptIt != interceptedFuncs->second.end()) {
        // This function is already intercepted.
        sendBpInterceptedWarning(
          m_debugger->getCurrentThreadId(),
          id,
          functionName
        );
      }
    }
  }

  ResolvedLocation loc = {0};
  loc.m_startLine = startLine;
  loc.m_endLine = endLine;
  loc.m_startCol = startColumn;
  loc.m_endCol = endColumn;
  loc.m_path = path;
  bp->m_resolvedLocations.push_back(loc);

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

  sendBreakpointEvent(id, ReasonChanged);

  // If calibration moved the breakpoint, tell the user that this was
  // intentional.
  if (m_debugger->getDebuggerOptions().notifyOnBpCalibration &&
      bp->m_type == BreakpointType::Source &&
      (startLine != bp->m_line || endLine != bp->m_line)) {

    std::string msg = "The breakpoint at line ";
    msg += std::to_string(bp->m_line);
    msg += " of ";
    msg += path;

    if (startLine == endLine) {
      // Single-line expression.
      msg += " was resolved to line ";
      msg += std::to_string(startLine);
      msg += ", which is actually where the nearest executable instruction in ";
      msg += "the source map for this file is.";
      m_debugger->sendUserMessage(
        msg.c_str(),
        DebugTransport::OutputLevelInfo
      );
    }
  }
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
                   breakpoint->m_resolvedLocations[0].m_startLine,
                   false
                 );

    int endLine = adjustLineNumber(
                      preferences,
                      breakpoint->m_resolvedLocations[0].m_endLine,
                      false
                    );
    if (endLine > 0) {
      bp["endLine"] = endLine;
    }

    int col = adjustLineNumber(
                      preferences,
                      breakpoint->m_resolvedLocations[0].m_startCol,
                      true
                    );
    if (col > 0) {
      bp["column"] = col;
    }

    int endCol = adjustLineNumber(
                        preferences,
                        breakpoint->m_resolvedLocations[0].m_endCol,
                        true
                      );

    if (endCol > 0) {
      bp["endColumn"] = endCol;
    }

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

    int col = adjustLineNumber(
                     preferences,
                     breakpoint->m_column,
                     false
                   );
    if (col > 0) {
      bp["column"] = col;
    }
  }

  folly::dynamic source = folly::dynamic::object;
  source["path"] = resolved
    ? breakpoint->m_resolvedLocations[0].m_path
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
    return it->second;
  }

  return nullptr;
}

bool BreakpointManager::isBreakpointResolved(int id) const {
  return m_resolvedBreakpoints.find(id) != m_resolvedBreakpoints.end();
}

void BreakpointManager::onBreakpointHit(int id) {
  const auto it = m_breakpoints.find(id);
  if (it != m_breakpoints.end()) {
    Breakpoint* bp = it->second;
    bp->m_hitCount++;
    sendBreakpointEvent(id, ReasonChanged);
  }
}

void BreakpointManager::sendBpError(
  int bpId,
  const char* error,
  const std::string& condition
) {
  std::string errorMsg = "The condition for breakpoint ";
  errorMsg += std::to_string(bpId);
  errorMsg += " (";
  errorMsg += condition;
  errorMsg += ") ";
  errorMsg += error;
  m_debugger->sendUserMessage(
    errorMsg.c_str(),
    DebugTransport::OutputLevelWarning
  );
}

void BreakpointManager::onRequestShutdown(request_id_t requestId) {
  for (auto it = m_breakpoints.begin(); it != m_breakpoints.end(); it++) {
    Breakpoint* bp = it->second;
    bp->clearCachedConditionUnit(requestId);
  }

  // When a request ends, it no longer has any intercepted functions.
  auto interceptIt = m_interceptedFuncs.find(requestId);
  if (interceptIt != m_interceptedFuncs.end()) {
    interceptIt->second.clear();
    m_interceptedFuncs.erase(interceptIt);
  }
}

bool BreakpointManager::isBreakConditionSatisified(
  DebuggerRequestInfo* ri,
  Breakpoint* bp
) {
  std::string condition = std::string(bp->getCondition());
  if (condition.size() == 0) {
    // This breakpoint has no condition.
    return true;
  }

  request_id_t requestId = m_debugger->getCurrentThreadId();
  HPHP::Unit* unit = bp->getCachedConditionUnit(requestId);
  if (unit == nullptr && !condition.empty()) {
    try {
      auto const cond = EvaluateCommand::prepareEvalExpression(condition);
      unit = compile_debugger_string(
        cond.c_str(), cond.size(), g_context->getRepoOptionsForCurrentFrame());
      bp->cacheConditionUnit(requestId, unit);
    } catch (...) {
      // Errors will be printed to stderr already, and we'll err on the side
      // of breaking when the bp is hit.
      unit = nullptr;
    }
  }

  if (unit == nullptr) {
    // This means the condition is invalid: it failed to compile.
    // Bother the user, and break anyway.
    sendBpError(bp->m_id, "did not compile.", condition);
    return true;
  }

  // SilentEvaluationContext suppresses all breakpoints and flow stepping
  // behavior during the evaluation, and re-enables it when it is destroyed.
  SilentEvaluationContext silentContext(m_debugger, ri, false);

  g_context->debuggerSettings.bypassCheck = true;
  SCOPE_EXIT {
    g_context->debuggerSettings.bypassCheck = false;
  };

  // Go ahead and evaluate the condition, and the current frame depth.
  const auto& result = g_context->evalPHPDebugger(unit, 0);
  if (result.failed) {
    // The condition compiled but didn't run.
    // Bother the user, and break anyway.
    sendBpError(bp->m_id, "failed to execute.", condition);
    return true;
  }

  const Variant& resultData = result.result;
  if (resultData.isBoolean()) {
    return resultData.asBooleanVal();
  } else {
    // The condition ran but returned something other than a bool.
    // Break anyway and tell the user.
    sendBpError(
      bp->m_id,
      "returned a value that was not a Boolean.",
      condition
    );
    return true;
  }
}

}
}

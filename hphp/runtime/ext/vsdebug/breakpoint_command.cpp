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

#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/ext/vsdebug/breakpoint.h"
#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"

namespace HPHP {
namespace VSDEBUG {

SetBreakpointsCommand::SetBreakpointsCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message) {
}

SetBreakpointsCommand::~SetBreakpointsCommand() {
}

void SetBreakpointsCommand::setFnBreakpoints(
   DebuggerSession* session,
   const folly::dynamic& args,
   folly::dynamic& responseBps
) {
  const folly::dynamic& bps = tryGetArray(args, "breakpoints");

  // Remove existing function breakpoints. The protocol requires that the
  // SetFunctionBreakpoints request includes the new, full, list of bps.
  BreakpointManager* bpMgr = session->getBreakpointManager();
  const auto existingBps = bpMgr->getFunctionBreakpoints();
  for (const auto id : existingBps) {
    bpMgr->removeBreakpoint(id);
  }

  for (const folly::dynamic& bp : bps) {
    if (!bp.isObject()) {
      continue;
    }

    const std::string& name = tryGetString(bp, "name", "");
    if (name.empty()) {
      continue;
    }

    const std::string& condition = tryGetString(bp, "condition", "");
    const std::string& hitCondition = tryGetString(bp, "hitCondition", "");

    // Create a new breakpoint.
    int newBpId = bpMgr->addFunctionBreakpoint(
      name,
      condition,
      hitCondition
    );

    m_debugger->onBreakpointAdded(newBpId);

    folly::dynamic newBreakpointInfo = folly::dynamic::object;
    newBreakpointInfo["id"] = newBpId;
    newBreakpointInfo["verified"] = false;
    responseBps.push_back(newBreakpointInfo);
  }
}

bool SetBreakpointsCommand::executeImpl(
  DebuggerSession* session,
  folly::dynamic* responseMsg
) {
  folly::dynamic& message = getMessage();
  const bool fnBreakpoint =
    tryGetString(message, "command", "") == "setFunctionBreakpoints";
  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  const folly::dynamic& source = tryGetObject(args, "source", s_emptyArgs);

  (*responseMsg)["body"] = folly::dynamic::object;
  (*responseMsg)["body"]["breakpoints"] = folly::dynamic::array();
  auto& responseBps = (*responseMsg)["body"]["breakpoints"];

  if (fnBreakpoint) {
    setFnBreakpoints(session, args, responseBps);

    // Completion of this command does not resume the target.
    return false;
  }

  // Otherwise, we have a source + line breakpoint.

  // SourceReference is not supported.
  int sourceRef = tryGetInt(source, "sourceReference", INT_MIN);
  if (sourceRef != INT_MIN) {
    throw DebuggerCommandException("SourceReference breakpoint not supported.");
  }

  const std::string& filePath = tryGetString(source, "path", "");
  if (filePath.empty()) {
    throw DebuggerCommandException(
      "Breakpoint source path not specified."
    );
  }

  const auto realPath = realpathLibc(
      File::TranslatePathKeepRelative(String(filePath)).data());

  const std::string& path = realPath.empty() && !filePath.empty()
    ? filePath
    : realPath;

  BreakpointManager* bpMgr = session->getBreakpointManager();

  // Make a map of line -> breakpoint for all breakpoints in this file before
  // this set breakpoints operation.
  std::unordered_map<int, Breakpoint*> oldBpLines;
  const auto oldBpIds = bpMgr->getBreakpointIdsForPath(filePath);
  for (auto it = oldBpIds.begin(); it != oldBpIds.end(); it++) {
    Breakpoint* bp = bpMgr->getBreakpointById(*it);
    std::pair<int, Breakpoint*> pair;
    pair.first = bp->m_line;
    pair.second = bp;
    oldBpLines.emplace(pair);
  }

  const ClientPreferences& prefs = m_debugger->getClientPreferences();
  const std::string empty = "";

  try {
    const folly::dynamic& sourceBps = args["breakpoints"];
    if (sourceBps.isArray()) {
      for (auto it = sourceBps.begin(); it != sourceBps.end(); it++) {
        const folly::dynamic& sourceBp = *it;
        if (sourceBp.isObject()) {
          int line = tryGetInt(sourceBp, "line", -1);
          if (!prefs.linesStartAt1) {
            // If client lines start at 0, make them 1 based.
            line++;
          }

          if (line <= 0) {
            throw DebuggerCommandException(
              "Breakpoint has invalid line number."
            );
          }

          int column = tryGetInt(sourceBp, "column", 0);
          if (!prefs.columnsStartAt1) {
            // If client cols start at 0, make them 1 based.
            column++;
          }

          if (column < 0) {
            column = 0;
          }

          const std::string& condition =
            tryGetString(sourceBp, "condition", empty);
          const std::string& hitCondition =
            tryGetString(sourceBp, "hitCondition", empty);

          auto bpIt = oldBpLines.find(line);
          if (bpIt == oldBpLines.end()) {
            // Create a new breakpoint.
            int newBpId = bpMgr->addSourceLineBreakpoint(
              line,
              column,
              path,
              condition,
              hitCondition
            );

            m_debugger->onBreakpointAdded(newBpId);

            folly::dynamic newBreakpointInfo = folly::dynamic::object;
            newBreakpointInfo["id"] = newBpId;
            // We cannot resolve the breakpoints when they are set because
            // the file may not be loaded, so we set the "verified" field
            // to true here as a hack, so that the UI does not grey it out.
            newBreakpointInfo["verified"] = true;
            responseBps.push_back(newBreakpointInfo);
          } else {
            Breakpoint* bp = bpIt->second;
            // Update breakpoint conditions in case they've changed.
            bp->updateConditions(condition, hitCondition);

            bool verified = bpMgr->isBreakpointResolved(bp->m_id);
            int responseLine = line;
            if (verified) {
              responseLine = bp->m_resolvedLocations[0].m_startLine;
              if (!prefs.linesStartAt1) {
                responseLine--;
              }
            }

            folly::dynamic bpInfo = folly::dynamic::object;
            bpInfo["id"] = bp->m_id;
            bpInfo["line"] = responseLine;
            // See the comment for newBreakpointInfo.
            bpInfo["verified"] = true;
            responseBps.push_back(bpInfo);

            // Remove this bp from oldBpLines so that after processing this
            // loop, oldBpLines contains any breakpoints that did not match
            // a bp in the request from the client.
            oldBpLines.erase(bpIt);
          }
        }
      }

      // Remove any breakpoints that should no longer exist in this file.
      for (auto it = oldBpLines.begin(); it != oldBpLines.end(); it++) {
        const Breakpoint* bp = it->second;
        bpMgr->removeBreakpoint(bp->m_id);
      }
    }
  } catch (std::out_of_range &) {
  }

  // Completion of this command does not resume the target.
  return false;
}

SetExceptionBreakpointsCommand::SetExceptionBreakpointsCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message) {
}

SetExceptionBreakpointsCommand::~SetExceptionBreakpointsCommand() {
}

bool SetExceptionBreakpointsCommand::executeImpl(DebuggerSession* session,
                                                 folly::dynamic* /*responseMsg*/
) {
  folly::dynamic& message = getMessage();
  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);

  const auto exceptionOptions =
    tryGetObject(args, "exceptionOptions", s_emptyArgs);
  auto breakMode = tryGetString(exceptionOptions, "breakMode", "");

  // TODO: Named exception breakpoint filters not supported yet.
  if (breakMode == "") {
    const folly::dynamic& filters = args["filters"];
    if (filters.isArray()) {
      for (auto it = filters.begin(); it != filters.end(); it++) {
        const folly::dynamic& filterName = *it;
        if (filterName.isString()) {
          const std::string& filterString = filterName.getString();
          breakMode = filterString;
          break;
        }
      }
    }
  }

  ExceptionBreakMode mode = ExceptionBreakMode::BreakNone;
  if (breakMode == "" || breakMode == "never" || breakMode == "none") {
    mode = ExceptionBreakMode::BreakNone;
  } else if (breakMode == "always" || breakMode == "all") {
    mode = ExceptionBreakMode::BreakAll;
  } else if (breakMode == "unhandled" || breakMode == "uncaught") {
    mode = ExceptionBreakMode::BreakUnhandled;
  } else if (breakMode == "userUnhandled") {
    mode = ExceptionBreakMode::BreakUserUnhandled;
  } else {
    throw DebuggerCommandException(
      "Invalid exception break mode specified."
    );
  }

  BreakpointManager* bpMgr = session->getBreakpointManager();
  auto oldMode = bpMgr->getExceptionBreakMode();
  bpMgr->setExceptionBreakMode(mode);

  if (mode != oldMode) {
    m_debugger->onExceptionBreakpointChanged(
      mode != ExceptionBreakMode::BreakNone
    );
  }

  // Completion of this command does not resume the target.
  return false;
}

}
}

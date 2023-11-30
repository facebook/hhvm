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

#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/debugger-request-info.h"

namespace HPHP {
namespace VSDEBUG {

StepCommand::StepCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message) {
}

StepCommand::~StepCommand() {
}

bool StepCommand::executeImpl(DebuggerSession* /*session*/,
                              folly::dynamic* /*responseMsg*/
) {
  const folly::dynamic& message = getMessage();
  const std::string& command = tryGetString(message, "command", "");

  DebuggerRequestInfo* ri = m_debugger->getRequestInfo();

  if (command == "next") {
    // Step over.
    phpDebuggerNext();
    ri->m_stepReason = "step";

    // When next stepping, setup filter info so that we don't land on the
    // same line multiple times when stepping over a multi-line statement.
    auto currentLocation = Debugger::getVmLocation();
    const Unit* unit = currentLocation.first;
    const HPHP::SourceLoc& loc = currentLocation.second;
    if (unit != nullptr && loc.line0 != loc.line1) {
      StepNextFilterInfo info;
      info.stepStartUnit = unit;
      info.skipLine0 = loc.line0;
      info.skipLine1 = loc.line1;
      ri->m_nextFilterInfo.push_back(std::move(info));
    }
  } else if (command == "stepIn") {
    // Step in.
    phpDebuggerStepIn();
    ri->m_stepReason = "step in";
  } else if (command == "stepOut") {
    // Step out.
    phpDebuggerStepOut();
    ri->m_stepReason = "step out";
  } else {
    throw DebuggerCommandException("Unexpected step request.");
  }

  // Returning from this command resumes only this request thread.
  return true;
}


}
}

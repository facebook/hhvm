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

#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/command.h"

namespace HPHP {
namespace VSDEBUG {

LaunchAttachCommand::LaunchAttachCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message) {
}

LaunchAttachCommand::~LaunchAttachCommand() {
}

int64_t LaunchAttachCommand::targetThreadId() {
  return -1;
}

bool LaunchAttachCommand::executeImpl(folly::dynamic* responseMsg) {
  folly::dynamic& message = getMessage();
  const folly::dynamic emptyArgs = folly::dynamic::object;
  const folly::dynamic& args = tryGetObject(message, "arguments", emptyArgs);
  const std::string noDocument = std::string("");

  const auto& startupDoc =
    tryGetString(args, "startupDocumentPath", noDocument);

  if (!startupDoc.empty()) {
    m_debugger->startDummyRequest(startupDoc);
  } else {
    m_debugger->startDummyRequest(noDocument);
  }

  // Send the InitializedEvent to indicate to the front-end that we are up
  // and ready for breakpoint requests.
  folly::dynamic event = folly::dynamic::object;
  m_debugger->sendEventMessage(event, "initialized");

  // Completion of this command does not resume the target.
  return false;
}

}
}

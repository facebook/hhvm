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

ResolveBreakpointsCommand::ResolveBreakpointsCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message) {
}

ResolveBreakpointsCommand::~ResolveBreakpointsCommand() {
}

ResolveBreakpointsCommand*
ResolveBreakpointsCommand::createInstance(Debugger* debugger) {
  return new ResolveBreakpointsCommand(debugger, folly::dynamic::object);
}

bool ResolveBreakpointsCommand::executeImpl(DebuggerSession* /*session*/,
                                            folly::dynamic* /*responseMsg*/
) {
  m_debugger->tryInstallBreakpoints(m_debugger->getRequestInfo());

  // Resolving breakpoints does not resume the target.
  return false;
}

}
}

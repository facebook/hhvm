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

#include "hphp/runtime/ext/vsdebug/session.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/program-functions.h"

namespace HPHP {
namespace VSDEBUG {

DebuggerSession::DebuggerSession(Debugger* debugger) :
  m_debugger(debugger),
  m_dummyThread(this, &DebuggerSession::runDummy),
  m_dummyStartupDoc("") {

  assert(m_debugger != nullptr);
}

DebuggerSession::~DebuggerSession() {
  m_dummyCommandQueue.shutdown();
  m_dummyThread.waitForEnd();
}

void DebuggerSession::startDummyRequest(const std::string& startupDoc) {
  m_dummyStartupDoc = File::TranslatePath(startupDoc).data();
  m_dummyThread.start();
}

void DebuggerSession::invokeDummyStartupDocument() {
  m_debugger->sendUserMessage(
    "Preparing your Hack/PHP console. Please wait...",
    DebugTransport::OutputLevelWarning
  );

  // If a startup document was specified, invoke it now.
  bool error;
  std::string errorMsg;
  bool ret = hphp_invoke(g_context.getCheck(),
                         m_dummyStartupDoc,
                         false,
                         null_array,
                         uninit_null(),
                         "",
                         "",
                         error,
                         errorMsg,
                         true,
                         false,
                         true);
  if (!ret || error) {
    std::string displayError =
      std::string("Failed to prepare the Hack/PHP console: ") + errorMsg;

    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "%s",
      displayError.c_str()
    );

    m_debugger->sendUserMessage(
      displayError.c_str(),
      DebugTransport::OutputLevelError
    );
  } else {
    m_debugger->sendUserMessage(
      "The Hack/PHP console is now ready to use.",
      DebugTransport::OutputLevelSuccess
    );
  }
}

const StaticString s_memory_limit("memory_limit");

void DebuggerSession::runDummy() {
  hphp_session_init();
  SCOPE_EXIT {
    hphp_context_exit();
    hphp_session_exit();
  };

  if (!DebuggerHook::attach<VSDebugHook>()) {
    m_debugger->sendUserMessage(
      "Failed to attach the debugger to the Hack/PHP console: another debugger "
      "is already attached!",
      DebugTransport::OutputLevelError
    );
    return;
  }

  // Remove the artificial memory limit for this request since there is a
  // debugger attached to it.
  IniSetting::SetUser(s_memory_limit, std::numeric_limits<int64_t>::max());

  if (!m_dummyStartupDoc.empty()) {
    invokeDummyStartupDocument();
  } else {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelWarning,
      "Dummy request started without a startup document."
    );
  }

  m_dummyCommandQueue.processCommands();
  DebuggerHook::detach();
}

void DebuggerSession::enqueueDummyCommand(VSCommand* command) {
  m_dummyCommandQueue.dispatchCommand(command);
}

}
}

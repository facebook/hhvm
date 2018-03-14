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

#include <string>
#include "hphp/runtime/ext/vsdebug/ext_vsdebug.h"
#include "hphp/runtime/ext/vsdebug/logging.h"

namespace HPHP {
namespace VSDEBUG {

VSDebugExtension::~VSDebugExtension() {
  if (s_debugger != nullptr) {
    delete s_debugger;
    s_debugger = nullptr;
  }
}

void VSDebugExtension::moduleLoad(const IniSetting::Map& ini, const Hdf hdf) {
  // This extension is ** disabled ** by default, unless the configuration
  // says otherwise. When !m_enabled, the other hooks in this module no-op.
  Config::Bind(s_configEnabled, ini, hdf, "Eval.Debugger.VSDebugEnable", false);

  // Set up logging for the extension.
  // Note: Logging for the extension is disabled by default, unless
  // VSDebugLogFile is explicitly defined in the configuration settings.
  Config::Bind(s_logFilePath, ini, hdf, "Eval.Debugger.VSDebugLogFile", "");

  Config::Bind(
    s_attachListenPort,
    ini,
    hdf,
    "Eval.Debugger.VSDebugListenPort",
    DefaultListenPort);

  bool commandLineEnabled = RuntimeOption::EnableVSDebugger;
  if (!s_configEnabled && !commandLineEnabled) {
   m_enabled = false;
   return;
  }

  m_enabled = true;

  VSDebugLogger::InitializeLogging(s_logFilePath);
  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "VSDebugExtension module loaded. Extension enabled in config."
  );
}

void VSDebugExtension::moduleInit() {
  SCOPE_EXIT {
    // Memory barrier to ensure release semantics for our write of s_debugger
    // and m_enabled in moduleLoad and this routine.
    std::atomic_thread_fence(std::memory_order_release);
    VSDebugLogger::LogFlush();
  };

  if (!m_enabled) {
    return;
  }

  s_debugger = new Debugger();
  if (s_debugger == nullptr) {
    // Failed to allocate debugger, disable the extension.
    m_enabled = false;
    return;
  }

  DebugTransport* transport = nullptr;
  if (RuntimeOption::ServerExecutionMode()) {
    // If HHVM is running in server mode, start up the debugger socket server
    // and listen for debugger clients to connect.
    VSDebugLogger::Log(VSDebugLogger::LogLevelInfo,
                       "Extension started in SERVER mode");

    transport = new SocketTransport(s_debugger, s_attachListenPort);
  } else {
    // Otherwise, HHVM is running in script or interactive mode. Communicate
    // with the debugger client locally via known file descriptors.
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "Extension started in SCRIPT mode."
    );

    // If a listen port was specified on the command line, use the TCP socket
    // transport even in script mode. Otherwise, fall back to using a pipe with
    // our parent process.
    if (RuntimeOption::VSDebuggerListenPort > 0) {
      VSDebugLogger::Log(
        VSDebugLogger::LogLevelInfo,
        "Blocking script startup. Waiting for debugger to attach on port: %d",
        RuntimeOption::VSDebuggerListenPort
      );

      transport = new SocketTransport(
        s_debugger,
        RuntimeOption::VSDebuggerListenPort
      );
    } else {
      try {
        transport = new FdTransport(s_debugger);
        s_launchMode = true;
      } catch (...) {
        assertx(transport == nullptr);
      }
    }
  }

  if (transport == nullptr) {
    // Failed to allocate transport, disable the extension.
    m_enabled = false;
    return;
  }

  assertx(s_debugger != nullptr);
  s_debugger->setTransport(transport);
}

void VSDebugExtension::moduleShutdown() {
  if (s_debugger != nullptr) {
    delete s_debugger;
    s_debugger = nullptr;
  }

  VSDebugLogger::FinalizeLogging();
}

void VSDebugExtension::requestInit() {
  // Memory barrier to ensure acquire semantics for the read of m_enabled below.
  std::atomic_thread_fence(std::memory_order_acquire);

  if (!m_enabled) {
    return;
  }

  assertx(s_debugger != nullptr);

  // If we're in SCRIPT mode and a TCP listen port was specified on the command
  // line, we need to block starting the script until the debugger client
  // connects.
  if (!RuntimeOption::ServerExecutionMode() &&
      RuntimeOption::VSDebuggerListenPort > 0) {

    if (!RuntimeOption::VSDebuggerNoWait) {
      VSDebugLogger::Log(
        VSDebugLogger::LogLevelInfo,
        "Blocking script startup until debugger client connects..."
      );
      s_debugger->waitForClientConnection();
      VSDebugLogger::Log(
        VSDebugLogger::LogLevelInfo,
        "Debugger client connected."
      );
    }
  }

  s_debugger->requestInit();
}

void VSDebugExtension::requestShutdown() {
  if (!m_enabled) {
    return;
  }

  assertx(s_debugger != nullptr);
  s_debugger->requestShutdown();
}

// Linkage for the debugger extension.
static VSDebugExtension s_vsdebug_extension;

// Linkage for configuration options.
bool VSDebugExtension::s_configEnabled {false};
std::string VSDebugExtension::s_logFilePath {""};
int VSDebugExtension::s_attachListenPort {-1};
bool VSDebugExtension::s_launchMode {false};
Debugger* VSDebugExtension::s_debugger {nullptr};

}
}

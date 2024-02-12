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

#include "hphp/runtime/ext/vsdebug/ext_vsdebug.h"

#include <string>

#include "hphp/runtime/base/configs/debugger.h"
#include "hphp/runtime/ext/vsdebug/logging.h"

namespace HPHP {
namespace VSDEBUG {

VSDebugExtension::~VSDebugExtension() {
  std::atomic_thread_fence(std::memory_order_acquire);
  if (s_debugger != nullptr) {
    delete s_debugger;
    s_debugger = nullptr;
  }

  std::atomic_thread_fence(std::memory_order_release);
}

void VSDebugExtension::moduleLoad(const IniSetting::Map& ini, const Hdf hdf) {
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

  Config::Bind(
    getUnixSocketPath(),
    ini,
    hdf,
    "Eval.Debugger.VSDebugDomainSocketPath",
    "");

  Config::Bind(
    s_domainSocketGroup,
    ini,
    hdf,
    "Eval.Debugger.VSDebugDomainSocketGroup",
    "");

  if (!Cfg::Debugger::EnableVSDebugger) {
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
  SocketTransportOptions opts{};

  if (RuntimeOption::ServerExecutionMode()) {
    // If HHVM is running in server mode, start up the debugger socket server
    // and listen for debugger clients to connect.
    VSDebugLogger::Log(VSDebugLogger::LogLevelInfo,
                       "Extension started in SERVER mode");

    VSDebugLogger::Log(VSDebugLogger::LogLevelInfo,
                       "Socket path: %s",
                       getUnixSocketPath().c_str());
    opts.domainSocketPath = getUnixSocketPath();
    opts.tcpListenPort = s_attachListenPort;
    transport = new SocketTransport(s_debugger, opts);
  } else {
    // Otherwise, HHVM is running in script or interactive mode. Communicate
    // with the debugger client locally via known file descriptors.
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "Extension started in SCRIPT mode."
    );

    // If a listen port or domain socket was specified on the command line,
    // use socket transport even in script mode. Otherwise, fall back to
    // using a pipe with our parent process.
    if (Cfg::Debugger::VSDebuggerListenPort > 0 ||
        !Cfg::Debugger::VSDebuggerDomainSocketPath.empty()) {
      VSDebugLogger::Log(
        VSDebugLogger::LogLevelInfo,
        "Blocking script startup. Waiting for debugger to attach: %s",
        Cfg::Debugger::VSDebuggerDomainSocketPath.empty()
          ? std::to_string(Cfg::Debugger::VSDebuggerListenPort).c_str()
          : Cfg::Debugger::VSDebuggerDomainSocketPath.c_str()
      );

      opts.tcpListenPort = Cfg::Debugger::VSDebuggerListenPort;
      opts.domainSocketPath = Cfg::Debugger::VSDebuggerDomainSocketPath;
      transport = new SocketTransport(s_debugger, opts);
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
  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "VSDebugExtension::moduleShutdown called."
  );
  VSDebugLogger::LogFlush();

  std::atomic_thread_fence(std::memory_order_acquire);
  if (s_debugger != nullptr) {
    s_debugger->shutdown();
    delete s_debugger;
    s_debugger = nullptr;
  }

  VSDebugLogger::FinalizeLogging();
  std::atomic_thread_fence(std::memory_order_release);
}

void VSDebugExtension::requestInit() {
  // Memory barrier to ensure acquire semantics for the read of m_enabled below.
  std::atomic_thread_fence(std::memory_order_acquire);

  if (!m_enabled) {
    return;
  }

  assertx(s_debugger != nullptr);

  // If we're in SCRIPT mode and a listen port/path was specified on the command
  // line, we need to block starting the script until the debugger client
  // connects.
  if (!RuntimeOption::ServerExecutionMode() &&
      (Cfg::Debugger::VSDebuggerListenPort > 0 ||
        !Cfg::Debugger::VSDebuggerDomainSocketPath.empty())) {

    if (!Cfg::Debugger::VSDebuggerNoWait) {
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
  // Barrier for m_enabled and s_debugger
  std::atomic_thread_fence(std::memory_order_acquire);
  if (!m_enabled || s_debugger == nullptr) {
    return;
  }

  assertx(s_debugger != nullptr);
  s_debugger->requestShutdown();
}

void VSDebugExtension::threadShutdown() {
  // NB some threads may be in the list, but we never get a requestShutdown
  // on them because they either never run PHP code or are done by the time
  // we query the list. Catch those threads and clean them up.
  // TODO(T40097246): hphp_thread_exit can currently run after moduleShutdown
  // as a workaround until this is fixed, check to see if we've already shut
  // down instead of asserting that we haven't
  if (!m_enabled || s_debugger == nullptr) return;
  s_debugger->requestShutdown();
}

std::string& VSDebugExtension::getUnixSocketPath() {
  static std::string s_unixSocketPath = "";
  return s_unixSocketPath;
}

std::string VSDebugExtension::getDomainSocketGroup() {
  return s_domainSocketGroup;
}

// Linkage for the debugger extension.
static VSDebugExtension s_vsdebug_extension;

// Linkage for configuration options.
std::string VSDebugExtension::s_logFilePath {""};
std::string VSDebugExtension::s_domainSocketGroup {""};
int VSDebugExtension::s_attachListenPort {-1};
bool VSDebugExtension::s_launchMode {false};
Debugger* VSDebugExtension::s_debugger {nullptr};

}
}

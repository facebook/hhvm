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
  VSDebugLogger::FinalizeLogging();
}

void VSDebugExtension::moduleLoad(const IniSetting::Map& ini, Hdf hdf) {
  // This extension is ** disabled ** by default, unless the configuration
  // says otherwise. When !m_enabled, the other hooks in this module no-op.
  Config::Bind(s_configEnabled, ini, hdf, "Eval.Debugger.VSDebugEnable", false);
  if (!m_enabled) {
    return;
  }

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

  bool commandLineEnabled = false;
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
  if (!m_enabled) {
    return;
  }

  if (RuntimeOption::ServerExecutionMode()) {
    // If HHVM is running in server mode, start up the debugger socket server
    // and listen for debugger clients to connect.
    VSDebugLogger::Log(VSDebugLogger::LogLevelInfo,
                       "Extension started in SERVER mode");
  } else {
    // Otherwise, HHVM is running in script or interactive mode. Communicate
    // with the debugger client locally via known file descriptors.
    VSDebugLogger::Log(VSDebugLogger::LogLevelInfo,
                      "Extension started in SCRIPT mode");
  }

  VSDebugLogger::LogFlush();
}

void VSDebugExtension::requestInit() {
  if (!m_enabled) {
    return;
  }

  // TODO: (Ericblue) Notify debugger of new request thread.
}

void VSDebugExtension::requestShutdown() {
  if (!m_enabled) {
    return;
  }

  // TODO: (Ericblue) Notify debugger that a thread has exited.
}

// Linkage for the debugger extension.
static VSDebugExtension s_vsdebug_extension;

// Linkage for configuration options.
bool VSDebugExtension::s_configEnabled {false};
std::string VSDebugExtension::s_logFilePath {""};
int VSDebugExtension::s_attachListenPort {-1};
}
}

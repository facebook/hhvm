/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/eval/debugger/dummy_sandbox.h>
#include <runtime/eval/debugger/debugger.h>
#include <runtime/base/program_functions.h>
#include <runtime/base/externals.h>
#include <system/gen/sys/system_globals.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DummySandbox::DummySandbox(const SandboxInfo &sandbox,
                           const std::string &startupFile)
    : m_sandbox(sandbox), m_startupFile(startupFile),
      m_thread(this, &DummySandbox::run), m_stopped(false) {
}

DummySandbox::~DummySandbox() {
  stop();
}

void DummySandbox::start() {
  m_thread.start();
}

void DummySandbox::stop() {
  m_stopped = true;
  m_thread.waitForEnd();
}

void DummySandbox::run() {
  try {
    while (true) {
      try {
        char *argv[] = {"", NULL};
        execute_command_line_begin(1, argv, 0);

        SystemGlobals *g = (SystemGlobals *)get_global_variables();
        Variant &server = g->gv__SERVER;
        server.set("HPHP_SANDBOX_USER", m_sandbox.user);
        server.set("HPHP_SANDBOX_NAME", m_sandbox.name);
        server.set("HPHP_SANDBOX_PATH", m_sandbox.path);

        if (!m_startupFile.empty()) {
          string file = m_sandbox.path + m_startupFile;
          bool error; string errorMsg;
          hphp_invoke(g_context.get(), file.c_str(),
                      false, Array(), null, "", "", "", error, errorMsg);
        }

        Debugger::InterruptSessionStarted();

      } catch (const DebuggerRestartException &e) {
        execute_command_line_end(0, false);
      }
    }
  } catch (const DebuggerExitException &e) {
    // end user quitting debugger
    execute_command_line_end(0, false);
  }
}

///////////////////////////////////////////////////////////////////////////////
}}

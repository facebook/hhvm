/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <runtime/eval/debugger/cmd/cmd_signal.h>
#include <runtime/base/program_functions.h>
#include <runtime/base/server/source_root_info.h>
#include <runtime/base/externals.h>
#include <system/gen/sys/system_globals.h>
#include <util/process.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DummySandbox::DummySandbox(DebuggerProxy *proxy,
                           const std::string &defaultPath,
                           const std::string &startupFile)
    : m_proxy(proxy), m_defaultPath(defaultPath), m_startupFile(startupFile),
      m_thread(this, &DummySandbox::run), m_inited(false), m_stopped(false),
      m_signum(CmdSignal::SignalNone) {
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
  while (!m_stopped) {
    try {
      char *argv[] = {"", NULL};
      execute_command_line_begin(1, argv, 0);

      FUNCTION_INJECTION("_", FrameInjection::PseudoMain);

      DSandboxInfo sandbox = m_proxy->getSandbox();
      string msg;
      if (m_inited) {
        SystemGlobals *g = (SystemGlobals *)get_global_variables();
        SourceRootInfo sri(sandbox.m_user, sandbox.m_name);
        if (sandbox.m_path.empty()) {
          sandbox.m_path = sri.path();
        }
        if (!sri.sandboxOn()) {
          msg = "Invalid sandbox was specified. "
            "PHP files may not be loaded properly.\n";
          // force HPHP_SANDBOX_ID to be set, so we can still talk to client
          g->GV(_SERVER).set("HPHP_SANDBOX_ID", sandbox.id());
        } else {
          sri.setServerVariables(g->GV(_SERVER));
        }

        std::string doc = getStartupDoc(sandbox);
        bool error; string errorMsg;
        bool ret = hphp_invoke(g_context.getNoCheck(), doc, false, null_array,
                               null, "", "", "", error, errorMsg);
        if (!ret || error) {
          msg += "Unable to pre-load " + doc;
          if (!errorMsg.empty()) {
            msg += ": " + errorMsg;
          }
        }
      }

      m_inited = true;
      Debugger::RegisterSandbox(sandbox);
      Debugger::InterruptSessionStarted(NULL, msg.c_str());

      // Blocking until Ctrl-C is issued by end user and DebuggerProxy cannot
      // find a real sandbox thread to handle it.
      {
        Lock lock(this);
        while (!m_stopped && m_signum != CmdSignal::SignalBreak) {
          wait(1);
        }
        m_signum = CmdSignal::SignalNone;
      }
    } catch (const DebuggerException &e) {}
    execute_command_line_end(0, false, NULL);
  }
}

void DummySandbox::notifySignal(int signum) {
  Lock lock(this);
  m_signum = signum;
  notify();
}

std::string DummySandbox::getStartupDoc(const DSandboxInfo &sandbox) {
  string path;
  if (!m_startupFile.empty()) {
    // if relative path, prepend directory
    if (m_startupFile[0] != '/' && m_startupFile[0] != '~') {
      path = sandbox.m_path;
      if (path.empty()) {
        path = m_defaultPath;
      }
    }
    if (!path.empty() && path[path.size() - 1] != '/') {
      path += '/';
    }
    path += m_startupFile;

    // resolving home directory
    if (path[0] == '~') {
      string user, home;
      size_t pos = path.find('/');
      if (pos == string::npos) pos = path.size();
      if (pos > 1) {
        user = path.substr(1, pos - 1);
      }
      if (user.empty()) user = sandbox.m_user;
      if (user.empty() || user == Process::GetCurrentUser()) {
        home = Process::GetHomeDirectory();
      } else {
        home = "/home/" + user + "/";
      }
      if (pos + 1 < path.size()) {
        path = home + path.substr(pos + 1);
      } else {
        path = home;
      }
    }
  }
  return path;
}

///////////////////////////////////////////////////////////////////////////////
}}

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/debugger/dummy_sandbox.h"

#include <boost/noncopyable.hpp>

#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/debugger/cmd/cmd_signal.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/hphp-system.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////
TRACE_SET_MOD(debugger);

DummySandbox::DummySandbox(DebuggerProxy *proxy,
                           const std::string &defaultPath,
                           const std::string &startupFile)
    : m_proxy(proxy), m_defaultPath(defaultPath), m_startupFile(startupFile),
      m_thread(this, &DummySandbox::run), m_stopped(false),
      m_signum(CmdSignal::SignalNone) { }

void DummySandbox::start() {
  TRACE(2, "DummySandbox::start\n");
  m_thread.start();
}

// Stop the sandbox thread, and wait for it to end. Timeout is in
// seconds. This can be called multiple times.
bool DummySandbox::stop(int timeout) {
  TRACE(2, "DummySandbox::stop\n");
  m_stopped = true;
  notify(); // Wakeup the sandbox thread so it will notice the stopped flag
  return m_thread.waitForEnd(timeout);
}

namespace {

struct CLISession : private boost::noncopyable {
  CLISession() {
    TRACE(2, "CLISession::CLISession\n");
    char *argv[] = {"", nullptr};
    execute_command_line_begin(1, argv, 0);
  }
  ~CLISession() {
    TRACE(2, "CLISession::~CLISession\n");
    Debugger::UnregisterSandbox(g_context->getSandboxId());
    ThreadInfo::s_threadInfo.getNoCheck()->
      m_reqInjectionData.setDebugger(false);
    execute_command_line_end(0, false, nullptr);
  }
};

}

const StaticString s__SERVER("_SERVER");

void DummySandbox::run() {
  TRACE(2, "DummySandbox::run\n");
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  while (!m_stopped) {
    try {
      CLISession hphpSession;

      DSandboxInfo sandbox = m_proxy->getSandbox();
      string msg;
      if (sandbox.valid()) {
        GlobalVariables *g = get_global_variables();
        SourceRootInfo sri(sandbox.m_user, sandbox.m_name);
        if (sandbox.m_path.empty()) {
          sandbox.m_path = sri.path();
        }
        if (!sri.sandboxOn()) {
          msg = "Invalid sandbox was specified. "
            "PHP files may not be loaded properly.\n";
        } else {
          sri.setServerVariables(g->getRef(s__SERVER));
        }
        Debugger::RegisterSandbox(sandbox);
        g_context->setSandboxId(sandbox.id());

        std::string doc = getStartupDoc(sandbox);
        if (!doc.empty()) {
          char cwd[PATH_MAX];
          getcwd(cwd, sizeof(cwd));
          Logger::Info("Start loading startup doc '%s', pwd = '%s'",
                       doc.c_str(), cwd);
          bool error; string errorMsg;
          bool ret = hphp_invoke(g_context.getNoCheck(), doc, false, null_array,
                                 uninit_null(), "", "", error, errorMsg, true,
                                 false, true);
          if (!ret || error) {
            msg += "Unable to pre-load " + doc;
            if (!errorMsg.empty()) {
              msg += ": " + errorMsg;
            }
          }
          Logger::Info("Startup doc " + doc + " loaded");
        }
      } else {
        g_context->setSandboxId(m_proxy->getDummyInfo().id());
      }

      ti->m_reqInjectionData.setDebugger(true);
      {
        DebuggerDummyEnv dde;
        // This is really the entire point of having the dummy sandbox. This
        // fires the initial session started interrupt to the client after
        // it first attaches.
        Debugger::InterruptSessionStarted(nullptr, msg.c_str());
      }

      // Blocking until Ctrl-C is issued by end user and DebuggerProxy cannot
      // find a real sandbox thread to handle it.
      {
        Lock lock(this);
        while (!m_stopped && m_signum != CmdSignal::SignalBreak) {
          wait(1);
        }
        if (m_stopped) {
          // stopped by worker thread
          break;
        }
        m_signum = CmdSignal::SignalNone;
      }
    } catch (const DebuggerClientExitException &e) {
      // stopped by the dummy sandbox thread itself
      break;
    } catch (const DebuggerException &e) {
    }
  }
}

void DummySandbox::notifySignal(int signum) {
  TRACE(2, "DummySandbox::notifySignal\n");
  Lock lock(this);
  m_signum = signum;
  notify();
}

std::string DummySandbox::getStartupDoc(const DSandboxInfo &sandbox) {
  TRACE(2, "DummySandbox::getStartupDoc\n");
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

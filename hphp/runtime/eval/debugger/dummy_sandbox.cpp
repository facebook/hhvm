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

#include <boost/noncopyable.hpp>

#include <runtime/eval/debugger/dummy_sandbox.h>
#include <runtime/eval/debugger/debugger.h>
#include <runtime/eval/debugger/cmd/cmd_signal.h>
#include <runtime/base/program_functions.h>
#include <runtime/base/server/source_root_info.h>
#include <runtime/base/externals.h>
#include <runtime/base/hphp_system.h>
#include <util/logger.h>
#include <util/process.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DummySandbox::DummySandbox(DebuggerProxy *proxy,
                           const std::string &defaultPath,
                           const std::string &startupFile)
    : m_proxy(proxy), m_defaultPath(defaultPath), m_startupFile(startupFile),
      m_stopped(false),
      m_signum(CmdSignal::SignalNone) {
  m_thread = new AsyncFunc<DummySandbox>(this, &DummySandbox::run);
}

bool DummySandbox::waitForEnd(int seconds) {
  bool ret = m_thread->waitForEnd(seconds);
  if (ret) {
    delete m_thread;
  }
  return ret;
}

void DummySandbox::start() {
  m_thread->start();
}

void DummySandbox::stop() {
  m_stopped = true;
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  if (ti->m_reqInjectionData.dummySandbox) {
    // called from dummy sandbox thread itself, schedule retirement
    Debugger::RetireDummySandboxThread(this);
  } else {
    // called from worker thread, we wait for the dummySandbox to end
    m_thread->waitForEnd();
    // we are sure it's always created by new and this is the last thing
    // on this object
    delete this;
  }
}

namespace {

struct CLISession : boost::noncopyable {
  CLISession() {
    char *argv[] = {"", nullptr};
    execute_command_line_begin(1, argv, 0);
  }
  ~CLISession() {
    Debugger::UnregisterSandbox(g_context->getSandboxId());
    ThreadInfo::s_threadInfo.getNoCheck()->
      m_reqInjectionData.debugger = false;
    execute_command_line_end(0, false, nullptr);
  }
};

}

void DummySandbox::run() {
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  Debugger::RegisterThread();
  ti->m_reqInjectionData.dummySandbox = true;
  while (!m_stopped) {
    try {
      CLISession hphpSession;

      DSandboxInfo sandbox = m_proxy->getSandbox();
      string msg;
      if (sandbox.valid()) {
        SystemGlobals *g = (SystemGlobals *)get_global_variables();
        SourceRootInfo sri(sandbox.m_user, sandbox.m_name);
        if (sandbox.m_path.empty()) {
          sandbox.m_path = sri.path();
        }
        if (!sri.sandboxOn()) {
          msg = "Invalid sandbox was specified. "
            "PHP files may not be loaded properly.\n";
        } else {
          sri.setServerVariables(g->GV(_SERVER));
        }
        Debugger::RegisterSandbox(sandbox);
        g_context->setSandboxId(sandbox.id());

        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        std::string doc = getStartupDoc(sandbox);
        Logger::Info("Start loading startup doc '%s', pwd = '%s'",
                     doc.c_str(), cwd);
        bool error; string errorMsg;
        bool ret = hphp_invoke(g_context.getNoCheck(), doc, false, null_array,
                               null, "", "", error, errorMsg, true, false,
                               true);
        if (!ret || error) {
          msg += "Unable to pre-load " + doc;
          if (!errorMsg.empty()) {
            msg += ": " + errorMsg;
          }
        }
        Logger::Info("Startup doc " + doc + " loaded");
      } else {
        g_context->setSandboxId(m_proxy->getDummyInfo().id());
      }

      ti->m_reqInjectionData.debugger = true;
      {
        DebuggerDummyEnv dde;
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

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/debugger/debugger_hook_handler.h"
#include "hphp/runtime/debugger/cmd/cmd_signal.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/base/file-util.h"

#include "hphp/util/logger.h"

namespace HPHP::Eval {
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

struct CLISession {
  CLISession() {
    TRACE(2, "CLISession::CLISession\n");
    char *argv[] = {"", nullptr};
    execute_command_line_begin(1, argv);
  }
  ~CLISession() {
    TRACE(2, "CLISession::~CLISession\n");
    Debugger::UnregisterSandbox(g_context->getSandboxId());
    DebuggerHook::detach();
    execute_command_line_end(false, nullptr);
  }

  CLISession(const CLISession&) = delete;
  CLISession& operator=(const CLISession&) = delete;
};

}

const StaticString s__SERVER("_SERVER");

void DummySandbox::run() {
  TRACE(2, "DummySandbox::run\n");
  RequestInfo *ti = RequestInfo::s_requestInfo.getNoCheck();
  while (!m_stopped) {
    try {
      CLISession hphpSession;

      DSandboxInfo sandbox = m_proxy->getSandbox();
      std::string msg;
      if (sandbox.valid()) {
        SourceRootInfo::WithRoot sri(sandbox.m_user, sandbox.m_name);
        if (sandbox.m_path.empty()) {
          sandbox.m_path = SourceRootInfo::GetCurrentSourceRoot();
        }
        if (!SourceRootInfo::SandboxOn()) {
          msg = "Invalid sandbox was specified. "
            "PHP files may not be loaded properly.\n";
        } else {
          auto server = php_global_exchange(s__SERVER, init_null());
          forceToDict(server);
          Array arr = server.asArrRef();
          server.unset();
          php_global_set(s__SERVER,
                         SourceRootInfo::SetServerVariables(std::move(arr)));
        }
        Debugger::RegisterSandbox(sandbox);
        g_context->setSandboxId(sandbox.id());

        std::string doc = getStartupDoc(sandbox);
        if (!doc.empty()) {
          char cwd[PATH_MAX];
          getcwd(cwd, sizeof(cwd));
          Logger::Info("Start loading startup doc '%s', pwd = '%s'",
                       doc.c_str(), cwd);
          bool error; std::string errorMsg;
          bool ret = hphp_invoke(g_context.getNoCheck(), doc, false, null_array,
                                 nullptr, "", "", error, errorMsg, true,
                                 false, true, RuntimeOption::EvalPreludePath);
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

      if (!DebuggerHook::attach<HphpdHook>(ti)) {
        const char* fail = "Could not attach hphpd to request: another debugger"
                           " is already attached.";
        Logger::Error("%s", fail);
        Debugger::InterruptSessionStarted(nullptr, fail);
        throw DebuggerClientAttachFailureException();
      }
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
    } catch (const DebuggerClientExitException& ) {
      // stopped by the dummy sandbox thread itself
      break;
    } catch (const DebuggerException& ) {
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
  std::string path;
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
    path = FileUtil::expandUser(path, sandbox.m_user);
  }
  return path;
}

///////////////////////////////////////////////////////////////////////////////
}

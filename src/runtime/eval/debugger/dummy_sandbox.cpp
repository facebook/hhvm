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
#include <util/process.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DummySandbox::DummySandbox(DebuggerProxy *proxy,
                           const std::string &defaultPath,
                           const std::string &startupFile)
    : m_proxy(proxy), m_defaultPath(defaultPath), m_startupFile(startupFile),
      m_thread(this, &DummySandbox::run), m_stopped(false), m_signum(0) {
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

      SystemGlobals *g = (SystemGlobals *)get_global_variables();
      Variant &server = g->gv__SERVER;
      const DSandboxInfo &sandbox = m_proxy->getSandbox();
      server.set("HPHP_SANDBOX_USER", sandbox.m_user);
      server.set("HPHP_SANDBOX_NAME", sandbox.m_name);
      server.set("HPHP_SANDBOX_PATH", sandbox.m_path);
      Eval::Debugger::RegisterSandbox(sandbox);

      if (!m_startupFile.empty()) {
        hphp_invoke_simple(getStartupDoc(sandbox));
      }

      Debugger::InterruptSessionStarted(NULL);

      Lock lock(this);
      while (!m_stopped && m_signum != SIGINT) {
        wait(1);
      }
      m_signum = 0;

    } catch (const DebuggerClientExitException &e) {
      execute_command_line_end(0, false, NULL);
      break; // end user quitting debugger
    }
  }

  Debugger::OnServerShutdown();
}

void DummySandbox::notifySignal(int signum) {
  Lock lock(this);
  m_signum = signum;
  notify();
}

std::string DummySandbox::getStartupDoc(const DSandboxInfo &sandbox) {
  string path;

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
      path.substr(1, pos - 1);
    }
    if (user.empty()) user = sandbox.m_user;
    if (user.empty() || user == Process::GetCurrentUser()) {
      home = Process::GetHomeDirectory();
    } else {
      home = "/home/" + user + "/";
    }
    path = home;
    if (pos < path.size()) {
      path += path.substr(pos);
    }
  }

  return path;
}

///////////////////////////////////////////////////////////////////////////////
}}

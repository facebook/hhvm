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

#include <runtime/base/server/source_root_info.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/preg.h>
#include <runtime/base/server/http_request_handler.h>
#include <runtime/base/server/transport.h>
#include <runtime/eval/debugger/debugger.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

SourceRootInfo::SourceRootInfo(const char *host)
  : m_sandboxCond(RuntimeOption::SandboxMode ? SandboxOn : SandboxOff) {
  if (!sandboxOn()) return;
  Variant matches;
  Variant r = preg_match(String(RuntimeOption::SandboxPattern.c_str(),
        RuntimeOption::SandboxPattern.size(), AttachLiteral), host, matches);
  if (!r.same(1)) {
    m_sandboxCond = SandboxOff;
    return;
  }
  if (RuntimeOption::SandboxFromCommonRoot) {
    String sandboxName = matches.rvalAt(1).toString();
    createFromCommonRoot(sandboxName);
  } else {
    Array pair = StringUtil::Explode(matches.rvalAt(1), "-", 2);
    m_user = pair.rvalAt(0).toString();
    bool defaultSb = pair.size() == 1;
    if (defaultSb) {
      m_sandbox = "default";
    } else {
      m_sandbox = pair.rvalAt(1).toString();
    }

    createFromUserConfig();
  }
}

SourceRootInfo::SourceRootInfo(const std::string &user,
                               const std::string &sandbox)
    : m_sandboxCond(RuntimeOption::SandboxMode ? SandboxOn : SandboxOff) {
  if (!sandboxOn()) return;
  m_user = user;
  m_sandbox = sandbox;
  createFromUserConfig();
}

void SourceRootInfo::createFromCommonRoot(const String &sandboxName) {
  m_user = "";
  m_sandbox = string(sandboxName);
  String sandboxesRoot = String(RuntimeOption::SandboxDirectoriesRoot);
  String logsRoot = String(RuntimeOption::SandboxLogsRoot);
  m_path = sandboxesRoot + "/" + sandboxName;
  if (m_path.charAt(m_path.size() - 1) != '/') {
    m_path += "/";
  }
  String logPath = logsRoot + "/" + sandboxName + "_error.log";
  String accessLogPath = logsRoot + "/" + sandboxName + "_access.log";
  if (!Logger::SetThreadLog(logPath.c_str())) {
    Logger::Warning("Sandbox error log %s could not be opened",
                    logPath.c_str());
  }
  if (!HttpRequestHandler::GetAccessLog().setThreadLog(accessLogPath.c_str())) {
    Logger::Warning("Sandbox access log %s could not be opened",
                    accessLogPath.c_str());
  }
}



void SourceRootInfo::createFromUserConfig() {
  String homePath = String(RuntimeOption::SandboxHome) + "/" + m_user + "/";
  {
    struct stat hstat;
    if (stat(homePath.c_str(), &hstat) != 0) {
      if (!RuntimeOption::SandboxFallback.empty()) {
        homePath = String(RuntimeOption::SandboxFallback) + "/" + m_user + "/";
        if (stat(homePath.c_str(), &hstat) != 0) {
          m_sandboxCond = SandboxOff;
          return;
        }
      }
    }
  }

  string confpath = string(homePath.c_str()) +
    RuntimeOption::SandboxConfFile;
  Hdf config, serverVars;
  String sp, lp, alp, userOverride;
  try {
    config.open(confpath);
    userOverride = config["user_override"].get();
    Hdf sboxConf = config[m_sandbox.c_str()];
    if (sboxConf.exists()) {
      sp = sboxConf["path"].get();
      lp = sboxConf["log"].get();
      alp = sboxConf["accesslog"].get();
      serverVars = sboxConf["ServerVars"];
    }
  } catch (HdfException &e) {
    Logger::Error("%s ignored: %s", confpath.c_str(),
                  e.getMessage().c_str());
  }
  if (serverVars.exists()) {
    for (Hdf hdf = serverVars.firstChild(); hdf.exists(); hdf = hdf.next()) {
      m_serverVars.set(String(hdf.getName()), String(hdf.getString()));
    }
  }
  if (!userOverride.empty()) {
    m_user = userOverride;
  }
  if (m_sandbox == "default") {
    if (sp.isNull()) {
      sp = "www/";
    }
  }
  if (sp.isNull()) {
    m_sandboxCond = SandboxError;
    return;
  }
  if (sp.charAt(0) == '/') {
    m_path = sp;
  } else {
    m_path = homePath + sp;
  }
  if (m_path.charAt(m_path.size() - 1) != '/') {
    m_path += "/";
  }
  if (!lp.isNull()) {
    if (lp.charAt(0) != '/') {
      lp = homePath + lp;
    }
    if (!Logger::SetThreadLog(lp.c_str())) {
      Logger::Warning("Sandbox error log %s could not be opened",
                      lp.c_str());
    }
  }
  if (!alp.isNull()) {
    if (alp.charAt(0) != '/') {
      alp = homePath + alp;
    }
    if (!HttpRequestHandler::GetAccessLog().setThreadLog(alp.c_str())) {
      Logger::Warning("Sandbox access log %s could not be opened",
                      alp.c_str());
    }
  }
}

SourceRootInfo::~SourceRootInfo() {
  if (sandboxOn()) {
    Logger::ClearThreadLog();
    HttpRequestHandler::GetAccessLog().clearThreadLog();
  }
}

void SourceRootInfo::handleError(Transport *t) {
  t->sendString("Sandbox not found", 200);
}

///////////////////////////////////////////////////////////////////////////////

void SourceRootInfo::setServerVariables(Variant &server) const {
  if (!sandboxOn()) return;
  for (map<string, string>::const_iterator it =
         RuntimeOption::SandboxServerVariables.begin();
       it != RuntimeOption::SandboxServerVariables.end(); ++it) {
    server.set(String(it->first),
               String(parseSandboxServerVariable(it->second)));
  }

  if (!m_serverVars.empty()) {
    server += m_serverVars;
  }

  Eval::DSandboxInfo sandbox;
  sandbox.m_user = m_user.data();
  sandbox.m_name = m_sandbox.data();
  sandbox.m_path = m_path.data();
  server.set("HPHP_SANDBOX_ID", sandbox.id()); // EvalDebugger needs this
  Eval::Debugger::RegisterSandbox(sandbox);
}

string SourceRootInfo::parseSandboxServerVariable(const string &format) const {
  ostringstream res;
  bool control = false;
  for (uint i = 0; i < format.size(); i++) {
    char c = format[i];
    if (control) {
      switch (c) {
      case 'p':
        {
          // skip trailing /
          const char *data = m_path.data();
          int n = m_path.size() - 1;
          ASSERT(data[n] == '/');
          res.write(data, n);
          break;
        }
      case 's': res << m_sandbox; break;
      case 'u': res << m_user; break;
      default: res << c; break;
      }
      control = false;
    } else if (c == '%') {
      control = true;
    } else {
      res << c;
    }
  }
  return res.str();
}

string SourceRootInfo::path() const {
  if (sandboxOn()) {
    return string(m_path.data(), m_path.size());
  } else {
    return RuntimeOption::SourceRoot;
  }
}

///////////////////////////////////////////////////////////////////////////////
}

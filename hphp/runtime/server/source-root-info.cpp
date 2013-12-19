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

#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/base/tv-arith.h"

using std::map;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL_NO_CHECK(string, SourceRootInfo::s_path);
IMPLEMENT_THREAD_LOCAL_NO_CHECK(string, SourceRootInfo::s_phproot);

SourceRootInfo::SourceRootInfo(Transport* transport)
    : m_sandboxCond(RuntimeOption::SandboxMode ? SandboxCondition::On :
                                                 SandboxCondition::Off) {
  s_path.destroy();
  s_phproot.destroy();

  auto documentRoot = transport->getDocumentRoot();
  if (!documentRoot.empty()) {
    // The transport take precedence over the config file
    m_path = documentRoot;
    *s_path.getCheck() = documentRoot;
    return;
  }

  if (!sandboxOn()) return;
  auto host = transport->getHeader("Host");
  Variant matches;
  Variant r = preg_match(String(RuntimeOption::SandboxPattern.c_str(),
                                RuntimeOption::SandboxPattern.size(),
                                CopyString), host, matches);
  if (!same(r, 1)) {
    m_sandboxCond = SandboxCondition::Off;
    return;
  }
  if (RuntimeOption::SandboxFromCommonRoot) {
    String sandboxName = matches.rvalAt(1).toString();
    createFromCommonRoot(sandboxName);
  } else {
    Array pair = StringUtil::Explode(matches.rvalAt(1), "-", 2).toArray();
    m_user = pair.rvalAt(0).toString();
    bool defaultSb = pair.size() == 1;
    if (defaultSb) {
      m_sandbox = "default";
    } else {
      m_sandbox = pair.rvalAt(1).toString();
    }

    createFromUserConfig();
  }
  *s_path.getCheck() = m_path.c_str();
}

SourceRootInfo::SourceRootInfo(const std::string &user,
                               const std::string &sandbox)
    : m_sandboxCond(RuntimeOption::SandboxMode ? SandboxCondition::On :
                                                 SandboxCondition::Off) {
  s_path.destroy();
  s_phproot.destroy();
  if (!sandboxOn()) return;
  m_user = user;
  m_sandbox = sandbox;
  createFromUserConfig();
  *s_path.getCheck() = m_path.c_str();
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

const StaticString s_default("default");

void SourceRootInfo::createFromUserConfig() {
  String homePath = String(RuntimeOption::SandboxHome) + "/" + m_user + "/";
  {
    struct stat hstat;
    if (stat(homePath.c_str(), &hstat) != 0) {
      if (!RuntimeOption::SandboxFallback.empty()) {
        homePath = String(RuntimeOption::SandboxFallback) + "/" + m_user + "/";
        if (stat(homePath.c_str(), &hstat) != 0) {
          m_sandboxCond = SandboxCondition::Off;
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
    m_user = std::move(userOverride);
  }
  if (m_sandbox == s_default) {
    if (sp.isNull()) {
      sp = "www/";
    }
  }
  if (sp.isNull()) {
    m_sandboxCond = SandboxCondition::Error;
    return;
  }
  if (sp.charAt(0) == '/') {
    m_path = std::move(sp);
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
    cellAddEq(*server.asCell(), make_tv<KindOfArray>(m_serverVars.get()));
  }
}

Eval::DSandboxInfo SourceRootInfo::getSandboxInfo() const {
  Eval::DSandboxInfo sandbox;
  sandbox.m_user = m_user.data();
  sandbox.m_name = m_sandbox.data();
  sandbox.m_path = m_path.data();
  return sandbox;
}

string SourceRootInfo::parseSandboxServerVariable(const string &format) const {
  std::ostringstream res;
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
          assert(data[n] == '/');
          res.write(data, n);
          break;
        }
      case 's': res << m_sandbox.c_str(); break;
      case 'u': res << m_user.c_str(); break;
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
  if (sandboxOn() || !m_path.empty()) {
    return string(m_path.data(), m_path.size());
  } else {
    return RuntimeOption::SourceRoot;
  }
}

const StaticString
  s_SERVER("_SERVER"),
  s_PHP_ROOT("PHP_ROOT");

string& SourceRootInfo::initPhpRoot() {
  GlobalVariables *g = get_global_variables();
  CVarRef server = g->get(s_SERVER);
  CVarRef v = server.rvalAt(s_PHP_ROOT);
  if (v.isString()) {
    *s_phproot.getCheck() = string(v.asCStrRef().data()) + string("/");
  } else {
    // Our best guess at the source root.
    *s_phproot.getCheck() = GetCurrentSourceRoot();
  }
  return *s_phproot.getCheck();
}

///////////////////////////////////////////////////////////////////////////////
}

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/tv-arith.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/server/transport.h"

using std::map;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL_NO_CHECK(std::string, SourceRootInfo::s_path);
IMPLEMENT_THREAD_LOCAL_NO_CHECK(std::string, SourceRootInfo::s_phproot);

const StaticString s_default("default");
const StaticString s___builtin("__builtin");
SourceRootInfo::SourceRootInfo(Transport* transport)
    : m_sandboxCond(RuntimeOption::SandboxMode ? SandboxCondition::On :
                                                 SandboxCondition::Off) {
  s_path.destroy();
  s_phproot.destroy();

  auto documentRoot = transport->getDocumentRoot();
  if (!documentRoot.empty()) {
    m_user = s___builtin;
    m_sandbox = s_default;
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
                                CopyString), host, &matches);
  if (!same(r, 1)) {
    m_sandboxCond = SandboxCondition::Off;
    return;
  }
  if (RuntimeOption::SandboxFromCommonRoot) {
    String sandboxName = matches.toArray().rvalAt(1).toString();
    createFromCommonRoot(sandboxName);
  } else {
    Array pair = StringUtil::Explode(
      matches.toArray().rvalAt(1), "-", 2).toArray();
    m_user = pair.rvalAt(0).toString();
    bool defaultSb = pair.size() == 1;
    if (defaultSb) {
      m_sandbox = s_default;
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
  m_sandbox = std::string(sandboxName);
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
          m_sandboxCond = SandboxCondition::Off;
          return;
        }
      }
    }
  }

  std::string confFileName = std::string(homePath.c_str()) +
    RuntimeOption::SandboxConfFile;
  IniSetting::Map ini = IniSetting::Map::object;
  Hdf config;
  String sp, lp, alp, userOverride;
  try {
    Config::ParseConfigFile(confFileName, ini, config);
    userOverride = Config::Get(ini, config, "user_override");
    sp = Config::Get(ini, config, (m_sandbox + ".path").c_str());
    lp = Config::Get(ini, config, (m_sandbox + ".log").c_str());
    alp = Config::Get(ini, config, (m_sandbox + ".accesslog").c_str());
  } catch (HdfException &e) {
    Logger::Error("%s ignored: %s", confFileName.c_str(),
                  e.getMessage().c_str());
  }
  if (config[(m_sandbox + ".ServerVars").c_str()]. exists()) {
    for (Hdf hdf = config[(m_sandbox + ".ServerVars").c_str()].firstChild();
                   hdf.exists(); hdf = hdf.next()) {
      m_serverVars.set(String(hdf.getName()),
                       String(Config::GetString(ini, hdf)));
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

Array SourceRootInfo::setServerVariables(Array server) const {
  if (!sandboxOn()) return std::move(server);
  for (auto it = RuntimeOption::SandboxServerVariables.begin();
       it != RuntimeOption::SandboxServerVariables.end();
       ++it) {
    server.set(String(it->first),
               String(parseSandboxServerVariable(it->second)));
  }

  if (!m_serverVars.empty()) {
    server += m_serverVars;
  }

  return std::move(server);
}

Eval::DSandboxInfo SourceRootInfo::getSandboxInfo() const {
  Eval::DSandboxInfo sandbox;
  sandbox.m_user = m_user.data();
  sandbox.m_name = m_sandbox.data();
  sandbox.m_path = m_path.data();
  return sandbox;
}

std::string
SourceRootInfo::parseSandboxServerVariable(const std::string &format) const {
  std::ostringstream res;
  bool control = false;
  for (uint32_t i = 0; i < format.size(); i++) {
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

std::string SourceRootInfo::path() const {
  if (sandboxOn() || !m_path.empty()) {
    return std::string(m_path.data(), m_path.size());
  } else {
    return RuntimeOption::SourceRoot;
  }
}

const StaticString
  s_SERVER("_SERVER"),
  s_PHP_ROOT("PHP_ROOT");

std::string& SourceRootInfo::initPhpRoot() {
  auto v = php_global(s_SERVER).toArray().rvalAt(s_PHP_ROOT);
  if (v.isString()) {
    *s_phproot.getCheck() = std::string(v.asCStrRef().data()) + "/";
  } else {
    // Our best guess at the source root.
    *s_phproot.getCheck() = GetCurrentSourceRoot();
  }
  return *s_phproot.getCheck();
}

///////////////////////////////////////////////////////////////////////////////
}

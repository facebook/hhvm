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

#include "hphp/runtime/server/source-root-info.h"

#include <sstream>

#include "hphp/runtime/base/array-iterator.h"
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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

THREAD_LOCAL_NO_CHECK(std::string, SourceRootInfo::s_path);

const StaticString s_default("default");
const StaticString s___builtin("__builtin");
SourceRootInfo::SourceRootInfo(Transport* transport)
    : m_sandboxCond(RuntimeOption::SandboxMode ? SandboxCondition::On :
                                                 SandboxCondition::Off) {
  s_path.destroy();

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
  if (!same(r, static_cast<int64_t>(1))) {
    auto const user = RO::GetDefaultUser();
    if (!user.empty()) {
      m_user = user;
      m_sandbox = s_default;
      createFromUserConfig();
      if (sandboxOn()) *s_path.getCheck() = m_path.c_str();
      return;
    }
    m_sandboxCond = SandboxCondition::Off;
    return;
  }

  auto const alias = RO::SandboxHostAlias;
  auto const name = tvCastToString(matches.toArray().lookup(1));
  if (!alias.empty() && !strcmp(name.data(), alias.data())) {
    m_sandboxCond = SandboxCondition::Off;
    return;
  } else if (RuntimeOption::SandboxFromCommonRoot) {
    createFromCommonRoot(name);
  } else {
    Array pair = StringUtil::Explode(name, "-", 2).toArray();
    m_user = tvCastToString(pair.lookup(0));
    bool defaultSb = pair.size() == 1;
    if (defaultSb) {
      m_sandbox = s_default;
    } else {
      m_sandbox = tvCastToString(pair.lookup(1));
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
  if (!Logger::SetThreadLog(logPath.c_str(), false)) {
    Logger::Warning("Sandbox error log %s could not be opened",
                    logPath.c_str());
  }
  if (!HttpRequestHandler::GetAccessLog().setThreadLog(accessLogPath.c_str())) {
    Logger::Warning("Sandbox access log %s could not be opened",
                    accessLogPath.c_str());
  }
}

void SourceRootInfo::createFromUserConfig() {
  auto maybeHomePath = RuntimeOption::GetHomePath(m_user.get()->slice());
  if (!maybeHomePath) {
    m_sandboxCond = SandboxCondition::Off;
    return;
  }

  IniSetting::Map ini = IniSetting::Map::object;
  Hdf config;
  bool success = RuntimeOption::ReadPerUserSettings(
      (*maybeHomePath) / RuntimeOption::SandboxConfFile, ini, config
  );
  if (!success) {
    m_sandboxCond = SandboxCondition::Off;
    return;
  }

  // Do not prepend "hhvm." to these when accessing.
  String userOverride = Config::Get(ini, config, "user_override", "", false);
  String sp = Config::Get(
      ini, config, (m_sandbox + ".path").c_str(), "", false
  );
  String lp = Config::Get(ini, config, (m_sandbox + ".log").c_str(), "", false);
  String alp = Config::Get(
      ini, config, (m_sandbox + ".accesslog").c_str(), "", false
  );
  auto sandbox_servervars_callback = [&] (const IniSetting::Map &ini_ss,
                                          const Hdf &hdf_ss,
                                          const std::string &ini_ss_key) {
    std::string name = hdf_ss.exists() && !hdf_ss.isEmpty()
                     ? hdf_ss.getName()
                     : ini_ss_key;
    m_serverVars.set(String(name), String(Config::GetString(ini_ss, hdf_ss)));
  };
  Config::Iterate(sandbox_servervars_callback, ini, config,
                  (m_sandbox + ".ServerVars").c_str());
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
  String homePath = maybeHomePath->native();
  if (sp.charAt(0) == '/') {
    m_path = std::move(sp);
  } else {
    m_path = homePath + "/" + sp;
  }
  if (m_path.charAt(m_path.size() - 1) != '/') {
    m_path += "/";
  }
  if (!lp.isNull()) {
    if (lp.charAt(0) != '/') {
      lp = homePath + lp;
    }
    if (!Logger::SetThreadLog(lp.c_str(), false)) {
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
  if (!sandboxOn()) return server;
  for (auto const& it : RuntimeOption::SandboxServerVariables) {
    String idx(it.first);
    const auto arrkey = server.convertKey<IntishCast::Cast>(idx);
    String str(parseSandboxServerVariable(it.second));
    server.set(arrkey, make_tv<KindOfString>(str.get()), true);
  }

  if (!m_serverVars.empty()) {
    IterateKV(
      m_serverVars.get(),
      [&](TypedValue key, TypedValue val) {
        if (!server.exists(key)) {
          server.set(key, val);
        }
      }
    );
  }

  return server;
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
          assertx(data[n] == '/');
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

///////////////////////////////////////////////////////////////////////////////
}

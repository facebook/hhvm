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
#include "hphp/runtime/base/init-fini-node.h"
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

namespace SourceRootInfo {

namespace fs = std::filesystem;

struct Info {
  std::string user;
  std::string sandbox;
  fs::path path;
  bool sandboxOn;
  bool hasDocRoot;

  // Per-user settings
  IniSetting::Map ini;
  Hdf config;
};

static THREAD_LOCAL_NO_CHECK(Info, tl_info);

namespace {
#ifndef NDEBUG
InitFiniNode r_debug_destroy_info(
  [] { tl_info.destroy(); },
  InitFiniNode::When::RequestFini
);
#endif

bool createFromCommonRoot(Info* info, const String& sandboxName) {
  auto sroot = RO::SandboxDirectoriesRoot;
  auto sname = sandboxName.toCppString();

  info->user = "";
  info->sandbox = sname;
  String logsRoot(RO::SandboxLogsRoot);

  if (!sroot.empty() && sroot.back() != '/') sroot += '/';
  if (!sname.empty() && sname.back() != '/') sname += '/';
  info->path = fs::weakly_canonical(fs::path(sroot + sname)) / "";

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
  return true;
}

bool createFromUserConfig(Info* info) {
  auto maybeHomePath = RO::GetHomePath(info->user);
  if (!maybeHomePath) {
    info->sandboxOn = false;
    return true;
  }

  IniSetting::Map ini = IniSetting::Map::object;
  Hdf config;
  bool success = RuntimeOption::ReadPerUserSettings(
      (*maybeHomePath) / RO::SandboxConfFile, ini, config
  );
  if (!success) {
    info->sandboxOn = false;
    return true;
  }

  // Do not prepend "hhvm." to these when accessing.
  auto userOverride = Config::GetString(
    ini, config, "user_override", "", false
  );
  auto sp = Config::GetString(
    ini, config, (info->sandbox + ".path").c_str(), "", false
  );
  auto lp = Config::GetString(
    ini, config, (info->sandbox + ".log").c_str(), "", false
  );
  auto alp = Config::GetString(
      ini, config, (info->sandbox + ".accesslog").c_str(), "", false
  );
  if (!userOverride.empty()) {
    info->user = std::move(userOverride);
  }

  info->ini = std::move(ini);
  info->config = std::move(config);

  if (info->sandbox == "default" && sp.empty()) sp = "www/";
  else if (sp.empty()) return false;
  else if (sp.back() != '/') sp += '/';

  if (sp.front() == '/') info->path = fs::weakly_canonical(sp);
  else                   info->path = fs::weakly_canonical(*maybeHomePath / sp);
  info->path /= "";

  if (!lp.empty()) {
    if (lp.front() != '/') lp = (*maybeHomePath / lp).native();
    if (!Logger::SetThreadLog(lp.c_str(), false)) {
      Logger::Warning("Sandbox error log %s could not be opened",
                      lp.c_str());
    }
  }
  if (!alp.empty()) {
    if (alp.front() != '/') alp = (*maybeHomePath / alp).native();
    if (!HttpRequestHandler::GetAccessLog().setThreadLog(alp.c_str())) {
      Logger::Warning("Sandbox access log %s could not be opened",
                      alp.c_str());
    }
  }
  return true;
}

std::string parseServerVariable(const Info* info, const std::string& format) {
  std::ostringstream res;
  bool control = false;
  for (uint32_t i = 0; i < format.size(); i++) {
    char c = format[i];
    if (control) {
      switch (c) {
      case 'p':
        {
          // skip trailing /
          auto const p = info->path.native();
          const char* data = p.data();
          int n = p.size() - 1;
          assertx(data[n] == '/');
          res.write(data, n);
          break;
        }
      case 's': res << info->sandbox.c_str(); break;
      case 'u': res << info->user.c_str(); break;
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

///////////////////////////////////////////////////////////////////////////////
}

bool Init(Transport* transport) {
  auto info = tl_info.getCheck();
  info->sandboxOn = info->hasDocRoot = RO::SandboxMode;

  auto documentRoot = transport->getDocumentRoot();
  if (!documentRoot.empty()) {
    info->user = "__builtin";
    info->sandbox = "default";
    // The transport take precedence over the config file
    if (documentRoot.back() != '/') documentRoot += '/';
    info->path = fs::weakly_canonical(documentRoot) / "";
    info->hasDocRoot = true;
    return true;
  }

  if (!RO::SandboxMode) return true;

  auto host = transport->getHeader("Host");
  Variant matches;
  Variant r = preg_match(RO::SandboxPattern, host, &matches);
  if (!same(r, static_cast<int64_t>(1))) {
    auto const user = RO::GetDefaultUser();
    if (!user.empty()) {
      info->user = user;
      info->sandbox = "default";
      return createFromUserConfig(info);
    }
    info->sandboxOn = false;
    info->hasDocRoot = false;
    return true;
  }

  auto const alias = RO::SandboxHostAlias;
  auto const name = tvCastToString(matches.toArray().lookup(1));
  if (!alias.empty() && !strcmp(name.data(), alias.data())) {
    info->sandboxOn = false;
    info->hasDocRoot = false;
    return true;
  } else if (RuntimeOption::SandboxFromCommonRoot) {
    return createFromCommonRoot(info, name);
  } else {
    Array pair = StringUtil::Explode(name, "-", 2).toArray();
    info->user = tvCastToString(pair.lookup(0)).toCppString();
    bool defaultSb = pair.size() == 1;
    if (defaultSb) {
      info->sandbox = "default";
    } else {
      info->sandbox = tvCastToString(pair.lookup(1)).toCppString();
    }

    return createFromUserConfig(info);
  }

  not_reached();
}

bool Init(const std::string& user, const std::string& sandbox) {
  auto info = tl_info.getCheck();
  info->sandboxOn = info->hasDocRoot = RO::SandboxMode;
  if (!RO::SandboxMode) return true;
  info->user = user;
  info->sandbox = sandbox;
  info->sandboxOn = true;
  info->hasDocRoot = true;
  return createFromUserConfig(info);
}

void ResetLogging() {
  if (!tl_info.isNull() && tl_info->sandboxOn) {
    Logger::ClearThreadLog();
    HttpRequestHandler::GetAccessLog().clearThreadLog();
  }
}

bool SandboxOn() {
  return !tl_info.isNull() && tl_info->sandboxOn;
}

bool IsInitialized() {
  return !tl_info.isNull();
}

///////////////////////////////////////////////////////////////////////////////

Array SetServerVariables(Array server) {
  if (!SandboxOn()) return server;
  auto const info = tl_info.get();

  for (auto const& it : RO::SandboxServerVariables) {
    String idx(it.first);
    const auto arrkey = server.convertKey<IntishCast::Cast>(idx);
    String str(parseServerVariable(info, it.second));
    server.set(arrkey, make_tv<KindOfString>(str.get()), true);
  }

  auto sandbox_servervars_callback = [&] (const IniSetting::Map &ini_ss,
                                          const Hdf &hdf_ss,
                                          const std::string &ini_ss_key) {
    String name(
      hdf_ss.exists() && !hdf_ss.isEmpty() ? hdf_ss.getName() : ini_ss_key
    );
    if (!server.exists(name)) {
      server.set(name, String(Config::GetString(ini_ss, hdf_ss)));
    }
  };
  Config::Iterate(sandbox_servervars_callback, info->ini, info->config,
                  (info->sandbox + ".ServerVars").c_str());

  return server;
}

Eval::DSandboxInfo GetSandboxInfo() {
  Eval::DSandboxInfo sandbox;

  if (!SandboxOn()) return sandbox;
  sandbox.m_user = tl_info->user;
  sandbox.m_name = tl_info->sandbox;
  sandbox.m_path = tl_info->path.native();
  return sandbox;
}

fs::path GetCurrentSourceRoot() {
  if (!tl_info.isNull() && tl_info->hasDocRoot) {
    return tl_info->path;
  } else {
    return Cfg::Server::SourceRoot;
  }
}

String RelativeToPhpRoot(const String& path) {
  return (GetCurrentSourceRoot() / path.toCppString()).native();
}

///////////////////////////////////////////////////////////////////////////////
}}

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

#include <runtime/base/server/source_root_info.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/preg.h>
#include <runtime/base/server/http_request_handler.h>
#include <runtime/base/server/transport.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

SourceRootInfo::SourceRootInfo(const char *host)
  : m_sandboxCond(RuntimeOption::SandboxMode ? SandboxOn : SandboxOff) {
  if (!sandboxOn()) return;
  Variant matches;
  Variant r = preg_match(RuntimeOption::SandboxPattern, host, matches);
  if (!r.same(1)) {
    m_sandboxCond = SandboxOff;
    return;
  }
  Array pair = StringUtil::Explode(matches.rvalAt(1), "-", 2);
  m_user = pair.rvalAt(0).toString();
  String homePath = String(RuntimeOption::SandboxHome) + "/" + m_user + "/";
  {
    struct stat hstat;
    if (stat(homePath.c_str(), &hstat) != 0) {
      m_sandboxCond = SandboxOff;
      return;
    }
  }
  bool defaultSb = pair.size() == 1;
  if (defaultSb) {
    m_sandbox = "default";
  } else {
    m_sandbox = pair.rvalAt(1).toString();
  }

  string confpath = string(homePath.c_str()) +
    RuntimeOption::SandboxConfFile;
  Hdf config;
  String sp, lp, alp;
  try {
    config.open(confpath);
    Hdf sboxConf = config[m_sandbox.c_str()];
    if (sboxConf.exists()) {
      sp = sboxConf["path"].get();
      lp = sboxConf["log"].get();
      alp = sboxConf["accesslog"].get();
    }
  } catch (HdfException &e) {
  }
  if (defaultSb) {
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
}

string SourceRootInfo::parseSandboxServerVariable(const string &format) const {
  ostringstream res;
  bool control = false;
  for (uint i = 0; i < format.size(); i++) {
    char c = format[i];
    if (control) {
      switch (c) {
      case 'p': res << m_path; break;
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

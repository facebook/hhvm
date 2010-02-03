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

#include <cpp/base/comparisons.h>
#include <cpp/base/server/virtual_host.h>
#include <cpp/base/preg.h>
#include <cpp/base/runtime_option.h>
#include <cpp/base/comparisons.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static VirtualHost s_default_vhost;
static ThreadLocalProxy<VirtualHost, false> s_current_vhost;

VirtualHost &VirtualHost::GetDefault() { return s_default_vhost; }

void VirtualHost::SetCurrent(VirtualHost *vhost) {
  s_current_vhost.set(vhost);
}

const VirtualHost *VirtualHost::GetCurrent() {
  VirtualHost *ret = s_current_vhost.get();
  if (ret == NULL) {
    ret = &s_default_vhost;
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

std::string format_pattern(const std::string &pattern) {
  if (pattern.empty()) return pattern;

  std::string ret = "#";
  for (unsigned int i = 0; i < pattern.size(); i++) {
    char ch = pattern[i];
    if (ch == '#') {
      ret += "\\#";
    } else {
      ret += ch;
    }
  }
  ret += '#';
  return ret;
}

VirtualHost::VirtualHost() : m_disabled(false) {
}


VirtualHost::VirtualHost(Hdf vh) : m_disabled(false) {
  init(vh);
}

void VirtualHost::init(Hdf vh) {
  m_name = vh.getName();

  const char *prefix = vh["Prefix"].get();
  const char *pattern = vh["Pattern"].get();
  const char *pathTranslation = vh["PathTranslation"].get();

  if (prefix) m_prefix = prefix;
  if (pattern) m_pattern = format_pattern(pattern);
  if (pathTranslation) {
    m_pathTranslation = pathTranslation;
    if (!m_pathTranslation.empty() &&
        m_pathTranslation[m_pathTranslation.length() - 1] != '/') {
      m_pathTranslation += '/';
    }
  }
  m_disabled = vh["Disabled"].getBool(false);

  m_documentRoot = RuntimeOption::SourceRoot + m_pathTranslation;
  if (!m_documentRoot.empty() &&
      m_documentRoot[m_documentRoot.length() - 1] == '/') {
    m_documentRoot = m_documentRoot.substr(0, m_documentRoot.length() - 1);
  }

  Hdf rewriteRules = vh["RewriteRules"];
  for (Hdf hdf = rewriteRules.firstChild(); hdf.exists(); hdf = hdf.next()) {
    RewriteRule dummy;
    m_rewriteRules.push_back(dummy);
    RewriteRule &rule = m_rewriteRules.back();
    rule.pattern = format_pattern(hdf["pattern"].getString(""));
    rule.to = hdf["to"].getString("");
    rule.qsa = hdf["qsa"].getBool(false);
    rule.redirect = hdf["redirect"].getInt16(0);

    if (rule.pattern.empty() || rule.to.empty()) {
      throw InvalidArgumentException("rewrite rule", "(empty pattern or to)");
    }
    Hdf rewriteConds = hdf["Conds"];
    for (Hdf chdf = rewriteConds.firstChild(); chdf.exists();
         chdf = chdf.next()) {
      RewriteCond dummy;
      rule.rewriteConds.push_back(dummy);
      RewriteCond &cond = rule.rewriteConds.back();
      cond.pattern = format_pattern(chdf["pattern"].getString(""));
      if (cond.pattern.empty()) {
        throw InvalidArgumentException("rewrite rule", "(empty cond pattern)");
      }
      const char *type = chdf["type"].get();
      if (type) {
        if (strcasecmp(type, "host") == 0) {
          cond.type = RewriteCond::Host;
        } else if (strcasecmp(type, "request") == 0) {
          cond.type = RewriteCond::Request;
        } else {
          throw InvalidArgumentException("rewrite rule",
                                         "(invalid cond type)");
        }
      } else {
        cond.type = RewriteCond::Request;
      }
      cond.negate = chdf["negate"].getBool(false);
    }

  }

  if (vh["IpBlockMap"].firstChild().exists()) {
    Hdf ipblocks = vh["IpBlockMap"];
    m_ipBlocks = IpBlockMapPtr(new IpBlockMap(ipblocks));
  }

  vh["ServerVariables"].get(m_serverVars);
  m_serverName = vh["ServerName"].getString(RuntimeOption::Host);
}

bool VirtualHost::match(const string &host) const {
  if (!m_pattern.empty()) {
    Variant ret = preg_match(String(m_pattern.c_str(), m_pattern.size(),
                                    AttachLiteral),
                             String(host.c_str(), host.size(),
                                    AttachLiteral));
    return ret.toInt64() > 0;
  } else if (!m_prefix.empty()) {
    return strncmp(host.c_str(), m_prefix.c_str(), m_prefix.size()) == 0;
  }
  return true;
}

bool VirtualHost::rewriteURL(CStrRef host, String &url, bool &qsa,
                             int &redirect) const {
  String normalized = url;
  if (normalized.empty() || normalized.charAt(0) != '/') {
    normalized = String("/") + normalized;
  }

  for (unsigned int i = 0; i < m_rewriteRules.size(); i++) {
    const RewriteRule &rule = m_rewriteRules[i];

    bool passed = true;
    for (vector<RewriteCond>::const_iterator it = rule.rewriteConds.begin();
         it != rule.rewriteConds.end(); ++it) {
      String subject;
      if (it->type == RewriteCond::Request) {
        subject = normalized;
      } else {
        subject = host;
      }
      Variant ret = preg_match(it->pattern.c_str(), subject);
      if (!ret.same(it->negate ? 0 : 1)) {
          passed = false;
          break;
      }
    }
    if (!passed) continue;
    Variant ret;
    int count = preg_replace(ret, rule.pattern.c_str(), rule.to.c_str(),
                             normalized, 1);
    if (!same(ret, false) && count > 0) {
      url = ret.toString();
      qsa = rule.qsa;
      redirect = rule.redirect;
      return true;
    }
  }
  return false;
}

bool VirtualHost::isBlocking(const std::string &command,
                             const std::string &ip) const {
  if (!m_ipBlocks) {
    return RuntimeOption::IpBlocks->isBlocking(command, ip);
  }
  return m_ipBlocks->isBlocking(command, ip);
}

const std::string &VirtualHost::serverName() const {
  if (m_serverName.empty()) {
    return RuntimeOption::Host;
  } else {
    return m_serverName;
  }
}

///////////////////////////////////////////////////////////////////////////////
}

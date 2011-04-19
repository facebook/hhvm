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

#include <runtime/base/comparisons.h>
#include <runtime/base/server/virtual_host.h>
#include <runtime/base/preg.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/comparisons.h>
#include <runtime/base/timeout_thread.h>
#include <util/util.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static VirtualHost s_default_vhost;

VirtualHost &VirtualHost::GetDefault() { return s_default_vhost; }

void VirtualHost::SetCurrent(VirtualHost *vhost) {
  g_context->setVirtualHost(vhost ? vhost : &s_default_vhost);
}

const VirtualHost *VirtualHost::GetCurrent() {
  const VirtualHost *ret = g_context->getVirtualHost();
  if (!ret) ret = &s_default_vhost;
  return ret;
}

int64 VirtualHost::GetMaxPostSize() {
  const VirtualHost *vh = GetCurrent();
  ASSERT(vh);
  if (vh->m_runtimeOption.maxPostSize != -1) {
    return vh->m_runtimeOption.maxPostSize;
  }
  return RuntimeOption::MaxPostSize;
}

int64 VirtualHost::GetUploadMaxFileSize() {
  const VirtualHost *vh = GetCurrent();
  ASSERT(vh);
  if (vh->m_runtimeOption.uploadMaxFileSize != -1) {
    return vh->m_runtimeOption.uploadMaxFileSize;
  }
  return RuntimeOption::UploadMaxFileSize;
}

const vector<string> &VirtualHost::GetAllowedDirectories() {
  const VirtualHost *vh = GetCurrent();
  ASSERT(vh);
  if (!vh->m_runtimeOption.allowedDirectories.empty()) {
    return vh->m_runtimeOption.allowedDirectories;
  }
  return RuntimeOption::AllowedDirectories;
}

///////////////////////////////////////////////////////////////////////////////

void VirtualHost::initRuntimeOption(Hdf overwrite) {
  int requestTimeoutSeconds =
    overwrite["Server.RequestTimeoutSeconds"].getInt32(-1);
  int64 maxPostSize =
    overwrite["Server.MaxPostSize"].getInt32(-1);
  if (maxPostSize != -1) maxPostSize *= (1LL << 20);
  int64 uploadMaxFileSize =
    overwrite["Server.Upload.UploadMaxFileSize"].getInt32(-1);
  if (uploadMaxFileSize != -1) uploadMaxFileSize *= (1LL << 20);
  overwrite["Server.AllowedDirectories"].
    get(m_runtimeOption.allowedDirectories);
  m_runtimeOption.requestTimeoutSeconds = requestTimeoutSeconds;
  m_runtimeOption.maxPostSize = maxPostSize;
  m_runtimeOption.uploadMaxFileSize = uploadMaxFileSize;
}

void VirtualHost::setRequestTimeoutSeconds() const {
  if (m_runtimeOption.requestTimeoutSeconds != -1) {
    TimeoutThread::DeferTimeout(m_runtimeOption.requestTimeoutSeconds);
  }
}

VirtualHost::VirtualHost() : m_disabled(false) {
  Hdf empty;
  initRuntimeOption(empty);
}

VirtualHost::VirtualHost(Hdf vh) : m_disabled(false) {
  init(vh);
}

void VirtualHost::init(Hdf vh) {
  m_name = vh.getName();

  const char *prefix = vh["Prefix"].get("");
  const char *pattern = vh["Pattern"].get("");
  const char *pathTranslation = vh["PathTranslation"].get("");
  Hdf overwrite = vh["overwrite"];
  initRuntimeOption(overwrite);

  if (prefix) m_prefix = prefix;
  if (pattern) {
    m_pattern = Util::format_pattern(pattern, true);
    if (!m_pattern.empty()) {
      m_pattern += "i"; // case-insensitive
    }
  }
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
    rule.pattern = Util::format_pattern(hdf["pattern"].getString(""), true);
    rule.to = hdf["to"].getString("");
    rule.qsa = hdf["qsa"].getBool(false);
    rule.redirect = hdf["redirect"].getInt16(0);

    if (rule.pattern.empty() || rule.to.empty()) {
      throw InvalidArgumentException("rewrite rule", "(empty pattern or to)");
    }
    Hdf rewriteConds = hdf["conditions"];
    for (Hdf chdf = rewriteConds.firstChild(); chdf.exists();
         chdf = chdf.next()) {
      RewriteCond dummy;
      rule.rewriteConds.push_back(dummy);
      RewriteCond &cond = rule.rewriteConds.back();
      cond.pattern = Util::format_pattern(chdf["pattern"].getString(""), true);
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

  Hdf logFilters = vh["LogFilters"];
  for (Hdf hdf = logFilters.firstChild(); hdf.exists(); hdf = hdf.next()) {
    QueryStringFilter filter;
    filter.urlPattern = Util::format_pattern(hdf["url"].getString(""), true);
    filter.replaceWith = hdf["value"].getString("");
    filter.replaceWith = "\\1=" + filter.replaceWith;

    string pattern = hdf["pattern"].getString("");
    vector<string> names;
    hdf["params"].get(names);

    if (pattern.empty()) {
      for (unsigned int i = 0; i < names.size(); i++) {
        if (pattern.empty()) {
          pattern = "(?<=[&\?])(";
        } else {
          pattern += "|";
        }
        pattern += names[i];
      }
      if (!pattern.empty()) {
        pattern += ")=.*?(?=(&|$))";
        pattern = Util::format_pattern(pattern, false);
      }
    } else if (!names.empty()) {
      throw InvalidArgumentException
        ("log filter", "(cannot specify both params and pattern)");
    }

    filter.namePattern = pattern;
    m_queryStringFilters.push_back(filter);
  }

  vh["ServerVariables"].get(m_serverVars);
  m_serverName = vh["ServerName"].getString();
}

bool VirtualHost::match(const string &host) const {
  if (!m_pattern.empty()) {
    Variant ret = preg_match(String(m_pattern.c_str(), m_pattern.size(),
                                    AttachLiteral),
                             String(host.c_str(), host.size(),
                                    AttachLiteral));
    return ret.toInt64() > 0;
  } else if (!m_prefix.empty()) {
    return strncasecmp(host.c_str(), m_prefix.c_str(), m_prefix.size()) == 0;
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
      Variant ret = preg_match(String(it->pattern.c_str(), it->pattern.size(),
                                      AttachLiteral), subject);
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

std::string VirtualHost::serverName(const std::string &host) const {
  if (!m_serverName.empty()) {
    return m_serverName;
  }

  if (!RuntimeOption::DefaultServerNameSuffix.empty()) {
    if (!m_pattern.empty()) {
      Variant matches;
      Variant ret = preg_match(String(m_pattern.c_str(), m_pattern.size(),
                                      AttachLiteral),
                               String(host.c_str(), host.size(),
                                      AttachLiteral),
                               matches);
      if (ret.toInt64() > 0) {
        String prefix = matches[1].toString();
        if (prefix.empty()) {
          prefix = matches[0].toString();
        }
        if (!prefix.empty()) {
          return std::string(prefix.data()) +
            RuntimeOption::DefaultServerNameSuffix;
        }
      }
    } else if (!m_prefix.empty()) {
      return m_prefix + RuntimeOption::DefaultServerNameSuffix;
    }
  }

  return RuntimeOption::Host;
}

///////////////////////////////////////////////////////////////////////////////
// query string filter

std::string VirtualHost::filterUrl(const std::string &url) const {
  ASSERT(!m_queryStringFilters.empty());

  for (unsigned int i = 0; i < m_queryStringFilters.size(); i++) {
    const QueryStringFilter &filter = m_queryStringFilters[i];

    bool match = true;
    if (!filter.urlPattern.empty()) {
      Variant ret = preg_match(String(filter.urlPattern.c_str(),
                                      filter.urlPattern.size(),
                                      AttachLiteral),
                               String(url.c_str(), url.size(),
                                      AttachLiteral));
      match = (ret.toInt64() > 0);
    }

    if (match) {
      if (filter.namePattern.empty()) {
        return "";
      }

      Variant ret;
      int count = preg_replace(ret, filter.namePattern.c_str(),
                               String(filter.replaceWith.c_str(),
                                      filter.replaceWith.size(),
                                      AttachLiteral),
                               String(url.c_str(), url.size(),
                                      AttachLiteral));
      if (!same(ret, false) && count > 0) {
        return ret.toString().data();
      }

      return url;
    }
  }

  return url;
}

///////////////////////////////////////////////////////////////////////////////
}

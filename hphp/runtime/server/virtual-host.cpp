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
#include "hphp/runtime/server/virtual-host.h"

#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/util/text-util.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

VirtualHost &VirtualHost::GetDefault() {
  // VirtualHost acquires global mutexes in its constructor, so we allocate
  // s_default_vhost lazily to ensure that all of the global mutexes have
  // been initialized before we enter the constructor.
  static VirtualHost s_default_vhost;
  return s_default_vhost;
}

void VirtualHost::SetCurrent(VirtualHost *vhost) {
  g_context->setVirtualHost(vhost ? vhost : &VirtualHost::GetDefault());
}

const VirtualHost *VirtualHost::GetCurrent() {
  const VirtualHost *ret = g_context->getVirtualHost();
  if (!ret) ret = &VirtualHost::GetDefault();
  return ret;
}

int64_t VirtualHost::GetMaxPostSize() {
  const VirtualHost *vh = GetCurrent();
  assert(vh);
  if (vh->m_runtimeOption.maxPostSize != -1) {
    return vh->m_runtimeOption.maxPostSize;
  }
  return RuntimeOption::MaxPostSize;
}

int64_t VirtualHost::GetUploadMaxFileSize() {
  const VirtualHost *vh = GetCurrent();
  assert(vh);
  if (vh->m_runtimeOption.uploadMaxFileSize != -1) {
    return vh->m_runtimeOption.uploadMaxFileSize;
  }
  return RuntimeOption::UploadMaxFileSize;
}

const std::vector<std::string> &VirtualHost::GetAllowedDirectories() {
  const VirtualHost *vh = GetCurrent();
  assert(vh);
  if (!vh->m_runtimeOption.allowedDirectories.empty()) {
    return vh->m_runtimeOption.allowedDirectories;
  }
  return RuntimeOption::AllowedDirectories;
}

void VirtualHost::SortAllowedDirectories(std::vector<std::string>& dirs) {
  /*
    Make sure corresponding realpath's are also allowed
  */
  for (unsigned int i = 0, s = dirs.size(); i < s; i++) {
    std::string &directory = dirs[i];
    char resolved_path[PATH_MAX];
    if (realpath(directory.c_str(), resolved_path) &&
        directory != resolved_path) {
      dirs.push_back(resolved_path);
    }
  }
  /*
    sort so we can use upper_bound to find the right prefix,
    rather than using a linear scan (and so we can remove
    duplicates, etc below)
  */
  std::sort(dirs.begin(), dirs.end());
  /*
     AllowedDirectories is a list of prefixes, so if x is a substring
     of y, we dont need y (also remove any duplicates).
  */
  dirs.erase(std::unique(dirs.begin(), dirs.end(),
                         [](const std::string &a, const std::string &b) {
                           if (a.size() < b.size()) {
                             return !b.compare(0, a.size(), a);
                           }
                           return !a.compare(0, b.size(), b);
                         }),
             dirs.end());
}

///////////////////////////////////////////////////////////////////////////////

void VirtualHost::initRuntimeOption(Hdf overwrite) {
  int requestTimeoutSeconds =
    overwrite["Server.RequestTimeoutSeconds"].getInt32(-1);
  int64_t maxPostSize =
    overwrite["Server.MaxPostSize"].getInt32(-1);
  if (maxPostSize != -1) maxPostSize *= (1LL << 20);
  int64_t uploadMaxFileSize =
    overwrite["Server.Upload.UploadMaxFileSize"].getInt32(-1);
  if (uploadMaxFileSize != -1) uploadMaxFileSize *= (1LL << 20);
  overwrite["Server.AllowedDirectories"].
    get(m_runtimeOption.allowedDirectories);
  m_runtimeOption.requestTimeoutSeconds = requestTimeoutSeconds;
  m_runtimeOption.maxPostSize = maxPostSize;
  m_runtimeOption.uploadMaxFileSize = uploadMaxFileSize;

  m_documentRoot = RuntimeOption::SourceRoot + m_pathTranslation;
  if (!m_documentRoot.empty() &&
      m_documentRoot[m_documentRoot.length() - 1] == '/') {
    m_documentRoot = m_documentRoot.substr(0, m_documentRoot.length() - 1);
  }
}

void VirtualHost::addAllowedDirectories(const std::vector<std::string>& dirs) {
  if (!m_runtimeOption.allowedDirectories.empty()) {
    m_runtimeOption.allowedDirectories.insert(
      m_runtimeOption.allowedDirectories.end(),
      dirs.begin(), dirs.end());
    SortAllowedDirectories(m_runtimeOption.allowedDirectories);
  }
}

int VirtualHost::getRequestTimeoutSeconds(int defaultTimeout) const {
  return m_runtimeOption.requestTimeoutSeconds < 0 ?
    defaultTimeout : m_runtimeOption.requestTimeoutSeconds;
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

  if (prefix) m_prefix = prefix;
  if (pattern) {
    m_pattern = format_pattern(pattern, false);
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
  initRuntimeOption(overwrite);

  m_disabled = vh["Disabled"].getBool(false);

  m_checkExistenceBeforeRewrite =
    vh["CheckExistenceBeforeRewrite"].getBool(true);

  Hdf rewriteRules = vh["RewriteRules"];
  for (Hdf hdf = rewriteRules.firstChild(); hdf.exists(); hdf = hdf.next()) {
    RewriteRule dummy;
    m_rewriteRules.push_back(dummy);
    RewriteRule &rule = m_rewriteRules.back();
    rule.pattern = format_pattern(hdf["pattern"].getString(""), true);
    rule.to = hdf["to"].getString("");
    rule.qsa = hdf["qsa"].getBool(false);
    rule.redirect = hdf["redirect"].getInt16(0);
    rule.encode_backrefs = hdf["encode_backrefs"].getBool(false);

    if (rule.pattern.empty() || rule.to.empty()) {
      throw InvalidArgumentException("rewrite rule", "(empty pattern or to)");
    }
    Hdf rewriteConds = hdf["conditions"];
    for (Hdf chdf = rewriteConds.firstChild(); chdf.exists();
         chdf = chdf.next()) {
      RewriteCond dummy;
      rule.rewriteConds.push_back(dummy);
      RewriteCond &cond = rule.rewriteConds.back();
      cond.pattern = format_pattern(chdf["pattern"].getString(""), true);
      if (cond.pattern.empty()) {
        throw InvalidArgumentException("rewrite rule", "(empty cond pattern)");
      }
      const char *type = chdf["type"].get();
      if (type) {
        if (strcasecmp(type, "host") == 0) {
          cond.type = RewriteCond::Type::Host;
        } else if (strcasecmp(type, "request") == 0) {
          cond.type = RewriteCond::Type::Request;
        } else {
          throw InvalidArgumentException("rewrite rule",
                                         "(invalid cond type)");
        }
      } else {
        cond.type = RewriteCond::Type::Request;
      }
      cond.negate = chdf["negate"].getBool(false);
    }

  }

  if (vh["IpBlockMap"].firstChild().exists()) {
    Hdf ipblocks = vh["IpBlockMap"];
    m_ipBlocks = std::make_shared<IpBlockMap>(ipblocks);
  }

  Hdf logFilters = vh["LogFilters"];
  for (Hdf hdf = logFilters.firstChild(); hdf.exists(); hdf = hdf.next()) {
    QueryStringFilter filter;
    filter.urlPattern = format_pattern(hdf["url"].getString(""), true);
    filter.replaceWith = hdf["value"].getString("");
    filter.replaceWith = "\\1=" + filter.replaceWith;

    std::string pattern = hdf["pattern"].getString("");
    std::vector<std::string> names;
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
        pattern = format_pattern(pattern, false);
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

bool VirtualHost::match(const std::string &host) const {
  if (!m_pattern.empty()) {
    Variant ret = preg_match(String(m_pattern.c_str(), m_pattern.size(),
                                    CopyString),
                             String(host.c_str(), host.size(),
                                    CopyString));
    return ret.toInt64() > 0;
  } else if (!m_prefix.empty()) {
    return strncasecmp(host.c_str(), m_prefix.c_str(), m_prefix.size()) == 0;
  }
  return true;
}

static int get_backref(const char **s) {
  assert('0' <= **s && **s <= '9');
  int val = **s - '0';
  *s += 1;
  if ('0' <= **s && **s <= '9') {
    val = val * 10 + **s - '0';
    *s += 1;
  }
  return val;
}

bool VirtualHost::rewriteURL(const String& host, String &url, bool &qsa,
                             int &redirect) const {
  String normalized = url;
  if (normalized.empty() || normalized.charAt(0) != '/') {
    normalized = String("/") + normalized;
  }

  for (unsigned int i = 0; i < m_rewriteRules.size(); i++) {
    const RewriteRule &rule = m_rewriteRules[i];

    bool passed = true;
    for (auto it = rule.rewriteConds.begin();
        it != rule.rewriteConds.end(); ++it) {
      String subject;
      if (it->type == RewriteCond::Type::Request) {
        subject = normalized;
      } else {
        subject = host;
      }
      Variant ret = preg_match(String(it->pattern.c_str(), it->pattern.size(),
                                      CopyString), subject);
      if (!same(ret, it->negate ? 0 : 1)) {
        passed = false;
        break;
      }
    }
    if (!passed) continue;
    Variant matches;
    int count = preg_match(rule.pattern.c_str(),
                           normalized,
                           matches).toInt64();
    if (count > 0) {
      const char *s = rule.to.c_str();
      StringBuffer ret;
      while (*s) {
        int backref = -1;
        if (*s == '\\') {
          if ('0' <= s[1] && s[1] <= '9') {
            s++;
            backref = get_backref(&s);
          } else if (s[1] == '\\') {
            s++;
          }
        } else if (*s == '$') {
          if (s[1] == '{') {
            const char *t = s+2;
            if ('0' <= *t && *t <= '9') {
              backref = get_backref(&t);
              if (*t != '}') {
                backref = -1;
              } else {
                s = t+1;
              }
            }
          } else if ('0' <= s[1] && s[1] <= '9') {
            s++;
            backref = get_backref(&s);
          }
        }
        if (backref >= 0) {
          String br = matches.toArray()[backref].toString();
          if (rule.encode_backrefs) {
            br = StringUtil::UrlEncode(br);
          }
          ret.append(br);
        } else {
          ret.append(s, 1);
          s++;
        }
      }

      url = ret.detach();
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
                                      CopyString),
                               String(host.c_str(), host.size(),
                                      CopyString),
                               matches);
      if (ret.toInt64() > 0) {
        String prefix = matches.toArray()[1].toString();
        if (prefix.empty()) {
          prefix = matches.toArray()[0].toString();
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
  assert(!m_queryStringFilters.empty());

  for (unsigned int i = 0; i < m_queryStringFilters.size(); i++) {
    const QueryStringFilter &filter = m_queryStringFilters[i];

    bool match = true;
    if (!filter.urlPattern.empty()) {
      Variant ret = preg_match(String(filter.urlPattern.c_str(),
                                      filter.urlPattern.size(),
                                      CopyString),
                               String(url.c_str(), url.size(),
                                      CopyString));
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
                                      CopyString),
                               String(url.c_str(), url.size(),
                                      CopyString));
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

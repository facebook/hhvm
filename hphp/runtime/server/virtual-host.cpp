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

#include "hphp/runtime/server/virtual-host.h"

#include <stdexcept>

#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/util/logger.h"
#include "hphp/util/text-util.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool VirtualHost::IsDefault(const IniSetting::Map& /*ini*/, const Hdf& vh,
                            const std::string& ini_key /* = "" */) {
  if (vh.exists() && !vh.isEmpty()) {
    return (vh.getName() == "default");
  } else {
    return (ini_key == "default");
  }
}

VirtualHost &VirtualHost::GetDefault() {
  // VirtualHost acquires global mutexes in its constructor, so we allocate
  // s_default_vhost lazily to ensure that all of the global mutexes have
  // been initialized before we enter the constructor.
  static VirtualHost s_default_vhost;
  return s_default_vhost;
}

void VirtualHost::SetCurrent(VirtualHost *vhost) {
  g_context->setVirtualHost(vhost ? vhost : &VirtualHost::GetDefault());
  UpdateSerializationSizeLimit();
}

const VirtualHost *VirtualHost::GetCurrent() {
  const VirtualHost *ret = g_context.isNull() ?
    nullptr : g_context->getVirtualHost();
  if (!ret) ret = &VirtualHost::GetDefault();
  return ret;
}

VirtualHost* VirtualHost::Resolve(const std::string& host) {
  auto const hostString = String(host.c_str(), host.size(), CopyString);
  for (auto vhost : RuntimeOption::VirtualHosts) {
    if (vhost->match(hostString)) {
      return vhost.get();
    }
  }
  return nullptr;
}

int64_t VirtualHost::getMaxPostSize() const {
  if (m_runtimeOption.maxPostSize != -1) {
    return m_runtimeOption.maxPostSize;
  }
  return Cfg::Server::MaxPostSize;
}

int64_t VirtualHost::GetLowestMaxPostSize() {
  auto lowest = Cfg::Server::MaxPostSize;
  for (auto vhost : RuntimeOption::VirtualHosts) {
    auto max = vhost->getMaxPostSize();
    lowest = std::min(lowest, max);
  }
  return lowest;
}

int64_t VirtualHost::GetMaxPostSize() {
  const VirtualHost *vh = GetCurrent();
  assertx(vh);
  return vh->getMaxPostSize();
}

int64_t VirtualHost::GetUploadMaxFileSize() {
  const VirtualHost *vh = GetCurrent();
  assertx(vh);
  if (vh->m_runtimeOption.uploadMaxFileSize != -1) {
    return vh->m_runtimeOption.uploadMaxFileSize;
  }
  return Cfg::Server::UploadMaxFileSize;
}

void VirtualHost::UpdateSerializationSizeLimit() {
  const VirtualHost *vh = GetCurrent();
  assertx(vh);
  if (vh->m_runtimeOption.serializationSizeLimit != StringData::MaxSize) {
    VariableSerializer::serializationSizeLimit->value =
      vh->m_runtimeOption.serializationSizeLimit;
  }
}

bool VirtualHost::alwaysDecodePostData(const String& origPath) const {
  if (!m_alwaysDecodePostData) return false;
  if (m_decodePostDataBlackList.empty()) return true;
  return !m_decodePostDataBlackList.count(origPath.toCppString());
}

const std::vector<std::string> &VirtualHost::GetAllowedDirectories() {
  const VirtualHost *vh = GetCurrent();
  assertx(vh);
  if (!vh->m_runtimeOption.allowedDirectories.empty()) {
    return vh->m_runtimeOption.allowedDirectories;
  }
  return Cfg::Server::AllowedDirectories;
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

void VirtualHost::initRuntimeOption(const IniSetting::Map& ini, const Hdf& vh) {
  int requestTimeoutSeconds =
    Config::GetInt32(ini, vh, "overwrite.Server.RequestTimeoutSeconds", -1,
                     false);
  int64_t maxPostSize =
    Config::GetInt32(ini, vh, "overwrite.Server.MaxPostSize", -1, false);
  if (maxPostSize != -1) maxPostSize *= (1LL << 20);
  int64_t uploadMaxFileSize =
    Config::GetInt32(ini, vh, "overwrite.Server.Upload.UploadMaxFileSize", -1,
                     false);
  if (uploadMaxFileSize != -1) uploadMaxFileSize *= (1LL << 20);
  int64_t serializationSizeLimit =
    Config::GetInt32(
      ini,
      vh, "overwrite.ResourceLimit.SerializationSizeLimit",
      StringData::MaxSize, false);
  m_runtimeOption.allowedDirectories = Config::GetStrVector(
    ini,
    vh, "overwrite.Server.AllowedDirectories",
    m_runtimeOption.allowedDirectories, false);
  m_runtimeOption.requestTimeoutSeconds = requestTimeoutSeconds;
  m_runtimeOption.maxPostSize = maxPostSize;
  m_runtimeOption.uploadMaxFileSize = uploadMaxFileSize;
  m_runtimeOption.serializationSizeLimit = serializationSizeLimit;

  m_documentRoot = Cfg::Server::SourceRoot + m_pathTranslation;
  if (m_documentRoot.length() > 1 &&
      m_documentRoot.back() == '/') {
    m_documentRoot.pop_back();
    // Make sure we've not converted "/" to "" (which is why we're checking
    // length() > 1 instead of !empty() above)
    assertx(!m_documentRoot.empty());
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
  IniSetting::Map ini = IniSetting::Map::object;
  Hdf empty;
  // Pass empty config maps; the default values are being used
  // for these runtime option values / overwrites.
  initRuntimeOption(ini, empty);
}

VirtualHost::VirtualHost(const IniSetting::Map& ini, const Hdf& vh,
                         const std::string &ini_key /* = "" */
                        ) : m_disabled(false) {
  init(ini, vh, ini_key);
}

void VirtualHost::init(const IniSetting::Map& ini, const Hdf& vh,
                       const std::string& ini_key /* = "" */) {
  m_name = vh.exists() && !vh.isEmpty() ? vh.getName() : ini_key;

  m_prefix = Config::GetString(ini, vh, "Prefix", "", false);
  auto pattern = format_pattern(Config::GetString(ini, vh, "Pattern", "",
                                                  false), false);
  m_pathTranslation = Config::GetString(ini, vh, "PathTranslation", "",
                                        false);
  if (!pattern.empty()) {
    pattern += "i"; // case-insensitive
    m_pattern = makeStaticString(pattern);
  }

  if (!m_pathTranslation.empty() &&
      m_pathTranslation[m_pathTranslation.length() - 1] != '/') {
    m_pathTranslation += '/';
  }

  initRuntimeOption(ini, vh); // overwrites

  m_disabled = Config::GetBool(ini, vh, "Disabled", false, false);

  m_checkExistenceBeforeRewrite =
    Config::GetBool(ini, vh, "CheckExistenceBeforeRewrite", true, false);

  m_alwaysDecodePostData = Config::GetBool(
    ini,
    vh,
    "AlwaysDecodePostData",
    Cfg::Server::AlwaysDecodePostDataDefault,
    false);

  m_decodePostDataBlackList =
    Config::GetSetC(ini, vh, "DecodePostDataBlackList");

  auto rr_callback = [&](const IniSetting::Map& ini_rr, const Hdf& hdf_rr,
                         const std::string& /*ini_rr_key*/) {
    RewriteRule dummy;
    m_rewriteRules.push_back(dummy);
    RewriteRule &rule = m_rewriteRules.back();
    rule.pattern = makeStaticString(format_pattern(
          Config::GetString(ini_rr, hdf_rr, "pattern", "", false),
          true));
    rule.to = Config::GetString(ini_rr, hdf_rr, "to", "", false);
    rule.qsa = Config::GetBool(ini_rr, hdf_rr, "qsa", false, false);
    rule.redirect = Config::GetInt16(ini_rr, hdf_rr, "redirect", 0, false);
    rule.encode_backrefs = Config::GetBool(ini_rr, hdf_rr, "encode_backrefs",
                                           false, false);

    if (rule.pattern->empty() || rule.to.empty()) {
      throw std::runtime_error("Invalid rewrite rule: (empty pattern or to)");
    }

    auto rc_callback = [&](const IniSetting::Map& ini_rc, const Hdf& hdf_rc,
                           const std::string& /*ini_rc_key*/) {
      RewriteCond dummy;
      rule.rewriteConds.push_back(dummy);
      RewriteCond &cond = rule.rewriteConds.back();
      cond.pattern = makeStaticString(format_pattern(
            Config::GetString(ini_rc, hdf_rc, "pattern", "", false), true));
      if (cond.pattern->empty()) {
        throw std::runtime_error("Invalid rewrite rule: (empty cond pattern)");
      }
      std::string type = Config::GetString(ini_rc, hdf_rc, "type", "", false);
      if (!type.empty()) {
        if (strcasecmp(type.c_str(), "host") == 0) {
          cond.type = RewriteCond::Type::Host;
        } else if (strcasecmp(type.c_str(), "request") == 0) {
          cond.type = RewriteCond::Type::Request;
        } else {
          throw std::runtime_error("Invalid rewrite rule: (invalid "
            "cond type)");
        }
      } else {
        cond.type = RewriteCond::Type::Request;
      }
      cond.negate = Config::GetBool(ini_rc, hdf_rc, "negate", false, false);
    };
    Config::Iterate(rc_callback, ini_rr, hdf_rr, "conditions", false);
  };
  Config::Iterate(rr_callback, ini, vh, "RewriteRules", false);

  m_ipBlocks = std::make_shared<IpBlockMap>(ini, vh);

  auto lf_callback = [&](const IniSetting::Map& ini_lf, const Hdf& hdf_lf,
                         const std::string& /*ini_lf_key*/) {
    QueryStringFilter filter;
    filter.urlPattern = format_pattern(Config::GetString(ini_lf, hdf_lf, "url",
                                                         "", false),
                                       true);
    filter.replaceWith = Config::GetString(ini_lf, hdf_lf, "value", "", false);
    filter.replaceWith = "\\1=" + filter.replaceWith;

    std::string namePattern = Config::GetString(ini_lf, hdf_lf, "pattern", "",
                                            false);
    std::vector<std::string> names;
    names = Config::GetStrVector(ini_lf, hdf_lf, "params", names, false);

    if (namePattern.empty()) {
      for (unsigned int i = 0; i < names.size(); i++) {
        if (namePattern.empty()) {
          namePattern = "(?<=[&\?])(";
        } else {
          namePattern += "|";
        }
        namePattern += names[i];
      }
      if (!namePattern.empty()) {
        namePattern += ")=.*?(?=(&|$))";
        namePattern = format_pattern(namePattern, false);
      }
    } else if (!names.empty()) {
      throw std::runtime_error("Invalid log filter: (cannot specify "
        "both params and pattern)");
    }

    filter.namePattern = namePattern;
    m_queryStringFilters.push_back(filter);
  };
  Config::Iterate(lf_callback, ini, vh, "LogFilters", false);

  m_serverVars = Config::GetMap(ini, vh, "ServerVariables", m_serverVars,
                                false);
  m_serverName = Config::GetString(ini, vh, "ServerName", m_serverName, false);

  // We don't require a prefix or pattern for the default
  if (m_name != "default" && !valid()) {
    throw std::runtime_error("virtual host missing prefix or pattern");
  }
}

bool VirtualHost::match(const String &host) const {
  if (m_pattern) {
    Variant ret = preg_match(m_pattern, host.get());
    return ret.toInt64() > 0;
  } else if (!m_prefix.empty()) {
    return strncasecmp(host.c_str(), m_prefix.c_str(), m_prefix.size()) == 0;
  }
  return true;
}

static int get_backref(const char **s) {
  assertx('0' <= **s && **s <= '9');
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

  Logger::Verbose("Matching host:%s url:%s using vhost:%s",
                  host.c_str(), url.c_str(), m_name.c_str());

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
      Variant ret = preg_match(it->pattern, subject.get());
      if (!same(ret, static_cast<int64_t>(it->negate ? 0 : 1))) {
        passed = false;
        break;
      }
    }
    if (!passed) continue;
    Variant matches;
    int count = preg_match(rule.pattern,
                           normalized.get(),
                           &matches).toInt64();
    if (count > 0) {
      Logger::Verbose("Matched pattern %s", rule.pattern->data());

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
    } else {
      Logger::Verbose("Did not match pattern %s", rule.pattern->data());
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

  if (!Cfg::Server::DefaultServerNameSuffix.empty()) {
    if (m_pattern) {
      Variant matches;
      Variant ret = preg_match(m_pattern,
                               String(host.c_str(), host.size(),
                                      CopyString).get(),
                               &matches);
      if (ret.toInt64() > 0) {
        String prefix = matches.toArray()[1].toString();
        if (prefix.empty()) {
          prefix = matches.toArray()[0].toString();
        }
        if (!prefix.empty()) {
          return std::string(prefix.data()) +
            Cfg::Server::DefaultServerNameSuffix;
        }
      }
    } else if (!m_prefix.empty()) {
      return m_prefix + Cfg::Server::DefaultServerNameSuffix;
    }
  }

  return Cfg::Server::Host;
}

///////////////////////////////////////////////////////////////////////////////
// query string filter

std::string VirtualHost::filterUrl(const std::string &url) const {
  assertx(!m_queryStringFilters.empty());

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

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

#ifndef incl_HPHP_VIRTUAL_HOST_H_
#define incl_HPHP_VIRTUAL_HOST_H_

#include "hphp/util/hdf.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/server/ip-block-map.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(VirtualHost);
class VirtualHost {
public:
  static VirtualHost &GetDefault();

  static void SetCurrent(VirtualHost *vhost);
  static const VirtualHost *GetCurrent();
  static int64_t GetMaxPostSize();
  static int64_t GetUploadMaxFileSize();
  static const std::vector<std::string> &GetAllowedDirectories();
  static void SortAllowedDirectories(std::vector<std::string>& dirs);
public:
  VirtualHost();
  explicit VirtualHost(Hdf vh);

  void init(Hdf vh);
  void addAllowedDirectories(const std::vector<std::string>& dirs);
  int getRequestTimeoutSeconds(int defaultTimeout) const;

  const std::string &getName() const { return m_name;}
  const std::string &getPathTranslation() const { return m_pathTranslation;}
  const std::string &getDocumentRoot() const { return m_documentRoot;}
  const std::map<std::string, std::string> &getServerVars() const {
    return m_serverVars;
  }
  std::string serverName(const std::string &host) const;

  bool valid() const { return !(m_prefix.empty() && m_pattern.empty()); }
  bool match(const std::string &host) const;
  bool disabled() const { return m_disabled; }

  // whether to check (and serve) files that exist before applying rewrite rules
  bool checkExistenceBeforeRewrite() const {
    return m_checkExistenceBeforeRewrite;
  }

  // url rewrite rules
  bool rewriteURL(const String& host, String &url, bool &qsa, int &redirect) const;

  // ip blocking rules
  bool isBlocking(const std::string &command, const std::string &ip) const;

  // query string filters
  bool hasLogFilter() const { return !m_queryStringFilters.empty();}
  std::string filterUrl(const std::string &url) const;

private:
  struct RewriteCond {
    enum class Type {
      Request,
      Host
    };
    Type type;
    std::string pattern;
    bool negate;
  };

  struct RewriteRule {
    std::string pattern;
    std::string to;
    bool qsa;      // whether to append original query string
    bool encode_backrefs;
    int redirect;  // redirect status code (301 or 302) or 0 for no redirect
    std::vector<RewriteCond> rewriteConds;
  };

  struct QueryStringFilter {
    std::string urlPattern;  // matching URLs
    std::string namePattern; // matching parameter names
    std::string replaceWith; // what to replace with
  };

  struct VhostRuntimeOption {
  public:
    int requestTimeoutSeconds;
    int64_t maxPostSize;
    int64_t uploadMaxFileSize;
    std::vector<std::string> allowedDirectories;
  };

  void initRuntimeOption(Hdf overwrite);
  bool m_disabled;
  bool m_checkExistenceBeforeRewrite;
  std::string m_name;
  std::string m_prefix;
  std::string m_pattern;

  std::string m_serverName;
  std::map<std::string, std::string> m_serverVars;
  std::string m_pathTranslation;
  std::string m_documentRoot;

  std::vector<RewriteRule> m_rewriteRules;
  IpBlockMapPtr m_ipBlocks;
  std::vector<QueryStringFilter> m_queryStringFilters;

  VhostRuntimeOption m_runtimeOption;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_VIRTUAL_HOST_H_

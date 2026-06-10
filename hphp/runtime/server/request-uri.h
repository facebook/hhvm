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

#pragma once

#include "hphp/runtime/base/type-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct VirtualHost;
struct Transport;

struct RequestURI {
  RequestURI(const VirtualHost *vhost, Transport *transport,
             const std::string &pathTranslation,
             const std::string &sourceRoot);
  explicit RequestURI(const std::string & rpcFunc);

  const OptString& originalURL() const { return m_originalURL; }
  const OptString& resolvedURL() const { return m_resolvedURL; }
  const OptString& queryString() const { return m_queryString; }

  const OptString& path() const { return m_path; }
  const char *ext() const { return m_ext; }
  const OptString& absolutePath() const { return m_absolutePath; }
  const OptString& pathInfo() const { return m_pathInfo; }
  const OptString& origPathInfo() const { return m_origPathInfo; }

  bool rewritten() const { return m_rewritten; }
  bool defaultDoc() const { return m_defaultDoc; }
  bool globalDoc() const { return m_globalDoc; }
  bool done() const { return m_done; }
  bool forbidden() const { return m_forbidden; }

  void dump();
  void clear();

  static void splitURL(OptString url, OptString &base, OptString &query);
private:
  OptString m_originalURL;  // without being rewritten, without query string
  OptString m_queryString;
  OptString m_rewrittenURL; // possibly rewritten
  OptString m_resolvedURL;  // possibly appended with default document and
                         // without pathinfo

  OptString m_pathInfo;
  OptString m_origPathInfo;
  OptString m_absolutePath;
  OptString m_path;  // path relative to SourceRoot

  bool m_rewritten;  // whether rewrite rules have applied
  bool m_defaultDoc; // whether DefaultDocument was appended
  bool m_globalDoc; // whether GlobalDocument was used
  bool m_done;
  bool m_forbidden;
  const char *m_ext;   // file extension

  bool process(const VirtualHost *vhost, Transport *transport,
               const std::string &pathTranslation,
               const std::string &sourceRoot, const char *url);
  bool rewriteURL(const VirtualHost *vhost, Transport *transport,
                  const std::string &pathTranslation,
                  const std::string &sourceRoot);
  bool rewriteURLNoDirCheck(const VirtualHost *vhost, Transport *transport,
                            const std::string &pathTranslation,
                            const std::string &sourceRoot);
  bool rewriteURLForDir(const VirtualHost *vhost, Transport *transport,
                             const std::string &pathTranslation,
                             const std::string &sourceRoot);
  bool resolveGlobalURL(const VirtualHost *vhost,
                        const std::string &pathTranslation,
                        const std::string &sourceRoot);
  bool resolveURL(const VirtualHost *vhost,
                  const std::string &pathTranslation,
                  const std::string &sourceRoot);
  bool virtualFileExists(const VirtualHost *vhost,
                         const std::string &pathTranslation,
                         const std::string &sourceRoot,
                         const OptString& filename);
  bool virtualFolderExists(const VirtualHost *vhost,
                           const std::string &pathTranslation,
                           const std::string &sourceRoot,
                           const OptString& foldername);
  void processExt();

  std::vector<std::string> m_triedURLs;
  const std::string getDefault404();

  static const char *parseExt(const OptString& s);
  static void PrependSlash(OptString &s);

};

///////////////////////////////////////////////////////////////////////////////
}


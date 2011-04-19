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

#ifndef __HPHP_REQUEST_URI_H__
#define __HPHP_REQUEST_URI_H__

#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class VirtualHost;
class Transport;

DECLARE_BOOST_TYPES(RequestURI);
class RequestURI {
public:
  RequestURI(const VirtualHost *vhost, Transport *transport,
             const std::string &pathTranslation,
             const std::string &sourceRoot);
  RequestURI(const std::string rpcFunc);

  CStrRef originalURL() const { return m_originalURL; }
  CStrRef resolvedURL() const { return m_resolvedURL; }
  CStrRef queryString() const { return m_queryString; }

  CStrRef path() const { return m_path; }
  CStrRef absolutePath() const { return m_absolutePath; }
  CStrRef pathInfo() const { return m_pathInfo; }

  bool rewritten() const { return m_rewritten; }
  bool defaultDoc() const { return m_defaultDoc; }
  bool done() const { return m_done; }

  void dump();
  void clear();

  static void splitURL(String url, String &base, String &query);
private:
  String m_originalURL;  // without being rewritten, without query string
  String m_queryString;
  String m_rewrittenURL; // possibly rewritten
  String m_resolvedURL;  // possibly appended with default document and
                         // without pathinfo

  String m_pathInfo;
  String m_absolutePath;
  String m_path;  // path relative to SourceRoot

  bool m_rewritten;  // whether rewrite rules have applied
  bool m_defaultDoc; // whether DefaultDocument was appended
  bool m_done;

  bool process(const VirtualHost *vhost, Transport *transport,
               const std::string &pathTranslation,
               const std::string &sourceRoot, const char *url);
  bool rewriteURL(const VirtualHost *vhost, Transport *transport,
                  const std::string &pathTranslation,
                  const std::string &sourceRoot);
  bool resolveURL(const VirtualHost *vhost,
                  const std::string &pathTranslation,
                  const std::string &sourceRoot);
  bool virtualFileExists(const VirtualHost *vhost,
                         const std::string &pathTranslation,
                         const std::string &sourceRoot,
                         CStrRef filename);
  bool virtualFolderExists(const VirtualHost *vhost,
                           const std::string &pathTranslation,
                           const std::string &sourceRoot,
                           CStrRef foldername);

  static void PrependSlash(String &s);

};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_REQUEST_URI_H__

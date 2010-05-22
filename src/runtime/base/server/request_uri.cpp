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

#include <runtime/base/server/request_uri.h>
#include <runtime/base/server/virtual_host.h>
#include <runtime/base/server/transport.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/static_content_cache.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

RequestURI::RequestURI(const VirtualHost *vhost, Transport *transport,
                       const std::string &sourceRoot,
                       const std::string &pathTranslation)
  :  m_rewritten(false), m_defaultDoc(false), m_done(false) {
  if (!process(vhost, transport, sourceRoot, pathTranslation,
               transport->getUrl())) {
    if (RuntimeOption::ErrorDocument404.empty() ||
        !process(vhost, transport, sourceRoot, pathTranslation,
                 RuntimeOption::ErrorDocument404.c_str())) {
      transport->sendString("Not Found", 404);
      m_done = true;
    }
  }
}

RequestURI::RequestURI(const std::string rpcFunc)
  :  m_rewritten(false), m_defaultDoc(false), m_done(false) {
  m_originalURL = m_rewrittenURL = m_resolvedURL = String(rpcFunc);
}

bool RequestURI::process(const VirtualHost *vhost, Transport *transport,
                         const string &sourceRoot,
                         const string &pathTranslation, const char *url) {
  splitURL(url, m_originalURL, m_queryString);

  // Fast path for files that exist
  String canon = Util::canonicalize(string(m_originalURL.c_str(),
                                           m_originalURL.size()));
  String relUrl(canon.charAt(0) == '/' ? canon.substr(1) :
                canon);
  if (virtualFileExists(vhost, sourceRoot, pathTranslation, relUrl)) {
    m_rewrittenURL = relUrl;
    m_resolvedURL = relUrl;
    PrependSlash(m_resolvedURL);
    return true;
  }

  if (!rewriteURL(vhost, transport, pathTranslation, sourceRoot)) {
    // Redirection
    m_done = true;
    return true;
  }
  if (!resolveURL(vhost, pathTranslation, sourceRoot)) {
    // Can't find
    return false;
  }
  return true;
}

void RequestURI::splitURL(String surl, String &base, String &querys) {
  const char *url = surl.c_str();
  const char *query = strchr(url, '?');
  if (query) {
    base = String(url, query - url, CopyString);
    ++query; // skipping ?
    querys = String(query, CopyString);
  } else {
    base = String(url, CopyString);
    querys = "";
  }
}

/**
 * Precondition: m_originalURL and m_queryString are set
 * Postcondition: Output is false and we are redirecting OR
 *  m_rewrittenURL is set and m_queryString is updated if needed
 */
bool RequestURI::rewriteURL(const VirtualHost *vhost, Transport *transport,
                            const string &pathTranslation,
                            const string &sourceRoot) {
  bool qsa = false;
  int redirect = 0;
  string host = transport->getHeader("host");
  m_rewrittenURL = m_originalURL;
  if (vhost->rewriteURL(host, m_rewrittenURL, qsa, redirect)) {
    m_rewritten = true;
    if (qsa && !m_queryString.empty()) {
      m_rewrittenURL += (m_rewrittenURL.find('?') < 0) ? "?" : "&";
      m_rewrittenURL += m_queryString;
    }
    if (redirect) {
      if (m_rewrittenURL.substr(0, 7) != "http://" &&
          m_rewrittenURL.substr(0, 8) != "https://") {
        PrependSlash(m_rewrittenURL);
      }
      transport->redirect(m_rewrittenURL, redirect);
      return false;
    }
    splitURL(m_rewrittenURL, m_rewrittenURL, m_queryString);
  }
  m_rewrittenURL = Util::canonicalize(string(m_rewrittenURL.c_str(),
                                             m_rewrittenURL.size()));
  if (!m_rewritten && m_rewrittenURL.charAt(0) == '/') {
    // A un-rewritten URL is always relative, so remove prepending /
    m_rewrittenURL = m_rewrittenURL.substr(1);
  }

  // If the URL refers to a folder but does not end
  // with a slash, then we need to redictect
  String url = m_rewrittenURL;
  if (!url.empty() &&
      url.charAt(url.length() - 1) != '/') {
    if (virtualFolderExists(vhost, sourceRoot, pathTranslation, url)) {
      url += "/";
      m_rewritten = true;
      String queryStr;
      m_rewrittenURL = m_originalURL;
      m_rewrittenURL += "/";
      if (!m_queryString.empty()) {
        m_rewrittenURL += "?";
        m_rewrittenURL += m_queryString;
      }
      if (m_rewrittenURL.substr(0, 7) != "http://" &&
          m_rewrittenURL.substr(0, 8) != "https://") {
        PrependSlash(m_rewrittenURL);
      }
      transport->redirect(m_rewrittenURL, 301);
      return false;
    }
  }

  return true;
}

/**
 * Precondition: m_rewrittenURL is set
 * Postcondition: Output is true and m_path and m_absolutePath are set OR
 *   output is false and no file was found
 */
bool RequestURI::resolveURL(const VirtualHost *vhost,
                            const string &pathTranslation,
                            const string &sourceRoot) {
  m_resolvedURL = m_rewrittenURL;
  while (!virtualFileExists(vhost, sourceRoot, pathTranslation,
                            m_resolvedURL)) {
    int pos = m_resolvedURL.rfind('/');
    if (pos <= 0) {
      // when none of the <subpath> exists, we give up, and try default doc
      m_resolvedURL = m_rewrittenURL;
      if (!m_resolvedURL.empty() &&
          m_resolvedURL.charAt(m_resolvedURL.length() - 1) != '/') {
        m_resolvedURL += "/";
      }
      m_resolvedURL += String(RuntimeOption::DefaultDocument);
      m_pathInfo.reset();
      if (virtualFileExists(vhost, sourceRoot, pathTranslation,
                            m_resolvedURL)) {
        m_defaultDoc = true;
        PrependSlash(m_resolvedURL);
        return true;
      }
      return false;
    }
    m_resolvedURL = m_rewrittenURL.substr(0, pos);
    m_pathInfo = m_rewrittenURL.substr(pos);
  }
  PrependSlash(m_resolvedURL);
  return true;
}

bool RequestURI::virtualFileExists(const VirtualHost *vhost,
                                   const string &sourceRoot,
                                   const string &pathTranslation,
                                   CStrRef filename) {
  if (filename.empty() || filename.charAt(filename.length() - 1) == '/') {
    return false;
  }
  if (!vhost->getDocumentRoot().empty()) {
    string fullname = filename.data();
    if (fullname[0] == '/') {
      fullname = fullname.substr(1);
    } else {
      fullname = pathTranslation + fullname;
    }
    m_path = fullname;
    m_absolutePath = String(sourceRoot) + m_path;

    if (StaticContentCache::TheFileCache && !fullname.empty() &&
        StaticContentCache::TheFileCache->fileExists(fullname.c_str())) {
      return true;
    }

    struct stat st;
    return RuntimeOption::AllowedFiles.find(fullname.c_str()) !=
      RuntimeOption::AllowedFiles.end() ||
      (stat(m_absolutePath.c_str(), &st) == 0 &&
       (st.st_mode & S_IFMT) == S_IFREG);
  }
  m_path = filename;
  m_absolutePath = String(sourceRoot) + filename;
  return true;
}

bool RequestURI::virtualFolderExists(const VirtualHost *vhost,
                                     const string &sourceRoot,
                                     const string &pathTranslation,
                                     CStrRef foldername) {
  if (!vhost->getDocumentRoot().empty()) {
    string fullname = foldername.data();
    // If there is a trailing slash, remove it
    if (fullname.size() > 0 && fullname[fullname.size()-1] == '/') {
      fullname = fullname.substr(fullname.size()-1);
    }
    if (fullname[0] == '/') {
      fullname = fullname.substr(1);
    } else {
      fullname = pathTranslation + fullname;
    }
    m_path = fullname;
    m_absolutePath = String(sourceRoot) + m_path;

    if (StaticContentCache::TheFileCache && !fullname.empty() &&
        StaticContentCache::TheFileCache->dirExists(fullname.c_str())) {
      return true;
    }

    if (find(RuntimeOption::AllowedDirectories.begin(),
             RuntimeOption::AllowedDirectories.end(),
             fullname.c_str()) != RuntimeOption::AllowedDirectories.end())
      return true;

    struct stat st;
    return (stat(m_absolutePath.c_str(), &st) == 0 &&
            (st.st_mode & S_IFMT) == S_IFDIR);
  }
  m_path = foldername;
  m_absolutePath = String(sourceRoot) + foldername;
  return true;
}

void RequestURI::PrependSlash(String &s) {
  if (!s.empty() && s.charAt(0) != '/') {
    s = String("/") + s;
  }
}

void RequestURI::dump() {
  m_originalURL.dump();
  m_queryString.dump();
  m_rewrittenURL.dump();
  m_resolvedURL.dump();
  m_pathInfo.dump();
  m_absolutePath.dump();
  m_path.dump();
}

void RequestURI::clear() {
  m_originalURL.reset();
  m_queryString.reset();
  m_rewrittenURL.reset();
  m_resolvedURL.reset();
  m_pathInfo.reset();
  m_absolutePath.reset();
  m_path.reset();
}

///////////////////////////////////////////////////////////////////////////////
}

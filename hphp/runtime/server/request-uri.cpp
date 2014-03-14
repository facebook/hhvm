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
#include "hphp/runtime/server/request-uri.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hphp/runtime/server/virtual-host.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/util/file-util.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

RequestURI::RequestURI(const VirtualHost *vhost, Transport *transport,
                       const std::string &pathTranslation,
                       const std::string &sourceRoot)
  :  m_rewritten(false), m_defaultDoc(false), m_done(false),
     m_forbidden(false), m_ext(nullptr) {
  if (!process(vhost, transport, sourceRoot, pathTranslation,
               transport->getServerObject()) ||
      (m_forbidden && RuntimeOption::ForbiddenAs404)) {
    m_forbidden = false; // put down forbidden flag since we are redirecting
    if (!RuntimeOption::ErrorDocument404.empty()) {
      String redirectURL(RuntimeOption::ErrorDocument404);
      if (!m_queryString.empty()) {
        if (redirectURL.find('?') == -1) {
          redirectURL += "?";
        } else {
          // has query in 404 string
          redirectURL += "&";
        }
        redirectURL += m_queryString;
      }
      if (process(vhost, transport, sourceRoot, pathTranslation,
                  redirectURL.data())) {
        // 404 redirection succeed
        return;
      }
    }
    transport->sendString(getDefault404(), 404);
    transport->onSendEnd();
    m_done = true;
  }
}

RequestURI::RequestURI(const std::string & rpcFunc)
  :  m_rewritten(false), m_defaultDoc(false), m_done(false) {
  m_originalURL = m_rewrittenURL = m_resolvedURL = String(rpcFunc);
}

bool RequestURI::process(const VirtualHost *vhost, Transport *transport,
                         const std::string &sourceRoot,
                         const std::string &pathTranslation, const char *url) {
  splitURL(url, m_originalURL, m_queryString);
  m_originalURL = StringUtil::UrlDecode(m_originalURL, false);
  m_rewritten = false;

  auto pathTranslated = transport->getPathTranslated();
  if (!pathTranslated.empty()) {
    // The transport is overriding everything and just handing us the filename
    m_originalURL = pathTranslated;
    if (!resolveURL(vhost, pathTranslation, sourceRoot)) {
      return false;
    }
    return true;
  }

  // Fast path for files that exist
  if (vhost->checkExistenceBeforeRewrite()) {
    String canon(
      FileUtil::canonicalize(m_originalURL.c_str(), m_originalURL.size()),
      AttachString);
    if (virtualFileExists(vhost, sourceRoot, pathTranslation, canon)) {
      m_rewrittenURL = canon;
      m_resolvedURL = canon;
      return true;
    }
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
  const char *fragment = strchr(url, '#');
  if (fragment) {
    // ignore everything after the #
    if (query && fragment > query) {
      base = String(url, query - url, CopyString);
      ++query; // skipping ?
      querys = String(query, fragment - query, CopyString);
    } else {
      base = String(url, fragment - url, CopyString);
      querys = "";
    }
  } else if (query) {
    base = String(url, query - url, CopyString);
    ++query; // skipping ?
    querys = String(query, CopyString);
  } else {
    base = String(url, CopyString);
    querys = "";
  }
}

const StaticString s_http("http://");
const StaticString s_https("https://");

/**
 * Precondition: m_originalURL and m_queryString are set
 * Postcondition: Output is false and we are redirecting OR
 *  m_rewrittenURL is set and m_queryString is updated if needed
 */
bool RequestURI::rewriteURL(const VirtualHost *vhost, Transport *transport,
                            const std::string &pathTranslation,
                            const std::string &sourceRoot) {
  bool qsa = false;
  int redirect = 0;
  std::string host = transport->getHeader("host");
  m_rewrittenURL = m_originalURL;
  if (vhost->rewriteURL(host, m_rewrittenURL, qsa, redirect)) {
    m_rewritten = true;
    if (qsa && !m_queryString.empty()) {
      m_rewrittenURL += (m_rewrittenURL.find('?') < 0) ? "?" : "&";
      m_rewrittenURL += m_queryString;
    }
    if (redirect) {
      if (m_rewrittenURL.substr(0, 7) != s_http &&
          m_rewrittenURL.substr(0, 8) != s_https) {
        PrependSlash(m_rewrittenURL);
      }
      transport->redirect(m_rewrittenURL.c_str(), redirect, "rewriteURL");
      return false;
    }
    splitURL(m_rewrittenURL, m_rewrittenURL, m_queryString);
  }
  m_rewrittenURL = String(
      FileUtil::canonicalize(m_rewrittenURL.c_str(), m_rewrittenURL.size()),
      AttachString);
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
      if (m_rewrittenURL.substr(0, 7) != s_http &&
          m_rewrittenURL.substr(0, 8) != s_https) {
        PrependSlash(m_rewrittenURL);
      }
      transport->redirect(m_rewrittenURL.c_str(), 301, "rewriteURL");
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
                            const std::string &pathTranslation,
                            const std::string &sourceRoot) {

  String startURL;
  if (m_rewritten) {
    startURL = m_rewrittenURL;
  } else {
    startURL = m_originalURL;
  }
  startURL = String(
      FileUtil::canonicalize(startURL.c_str(), startURL.size(), false),
      AttachString);
  m_resolvedURL = startURL;

  while (!virtualFileExists(vhost, sourceRoot, pathTranslation,
                            m_resolvedURL)) {
    int pos = m_resolvedURL.rfind('/');
    if (pos <= 0) {
      // when none of the <subpath> exists, we give up, and try default doc
      m_resolvedURL = startURL;
      if (!m_resolvedURL.empty() &&
          m_resolvedURL.charAt(m_resolvedURL.length() - 1) != '/') {
        m_resolvedURL += "/";
      }
      m_resolvedURL += String(RuntimeOption::DefaultDocument);
      m_origPathInfo.reset();
      if (virtualFileExists(vhost, sourceRoot, pathTranslation,
                            m_resolvedURL)) {
        m_defaultDoc = true;
        return true;
      }
      return false;
    }
    m_resolvedURL = startURL.substr(0, pos);
    m_origPathInfo = startURL.substr(pos);
  }
  if (!m_resolvedURL.empty() &&
      m_resolvedURL.charAt(0) != '/') {
    m_resolvedURL = "/" + m_resolvedURL;
  }
  if (!m_originalURL.empty() &&
      m_originalURL.charAt(0) != '/') {
    m_originalURL = "/" + m_originalURL;
  }
  m_pathInfo = String(
      FileUtil::canonicalize(m_origPathInfo.c_str(), m_origPathInfo.size()),
      AttachString);
  return true;
}

bool RequestURI::virtualFileExists(const VirtualHost *vhost,
                                   const std::string &sourceRoot,
                                   const std::string &pathTranslation,
                                   const String& filename) {
  if (filename.empty() || filename.charAt(filename.length() - 1) == '/') {
    return false;
  }
  String canon(FileUtil::canonicalize(filename.c_str(), filename.size()),
               AttachString);
  if (!vhost->getDocumentRoot().empty()) {
    std::string fullname = canon.data();
    int i = 0;
    while (i < fullname.size() && fullname[i] == '/') ++i;
    if (i) {
      fullname = fullname.substr(i);
    }
    if (!i || !m_rewritten) {
      fullname = pathTranslation + fullname;
    }
    m_path = fullname;
    m_absolutePath = String(sourceRoot) + m_path;
    processExt();
    if (RuntimeOption::PathDebug) {
      m_triedURLs.push_back(m_absolutePath.toCppString());
    }

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
  m_path = canon;
  m_absolutePath = String(sourceRoot) + canon;
  processExt();
  return true;
}

bool RequestURI::virtualFolderExists(const VirtualHost *vhost,
                                     const std::string &sourceRoot,
                                     const std::string &pathTranslation,
                                     const String& foldername) {
  if (!vhost->getDocumentRoot().empty()) {
    std::string fullname = foldername.data();
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
    processExt();

    if (StaticContentCache::TheFileCache && !fullname.empty() &&
        StaticContentCache::TheFileCache->dirExists(fullname.c_str())) {
      return true;
    }

    const std::vector<std::string> &allowedDirectories =
      VirtualHost::GetAllowedDirectories();
    if (find(allowedDirectories.begin(),
             allowedDirectories.end(),
             fullname.c_str()) != allowedDirectories.end()) {
      return true;
    }
    struct stat st;
    return (stat(m_absolutePath.c_str(), &st) == 0 &&
            (st.st_mode & S_IFMT) == S_IFDIR);
  }
  m_path = foldername;
  m_absolutePath = String(sourceRoot) + foldername;
  processExt();
  return true;
}

void RequestURI::processExt() {
  m_ext = parseExt(m_path);
  if (RuntimeOption::ForbiddenFileExtensions.empty()) {
    return;
  }
  if (m_ext &&
      RuntimeOption::ForbiddenFileExtensions.find(m_ext) !=
      RuntimeOption::ForbiddenFileExtensions.end()) {
    m_forbidden = true;
  }
}

/*
 * Parse file extension from a path
 */
const char *RequestURI::parseExt(const String& s) {
  int pos = s.rfind('.');
  if (pos == -1) {
    return nullptr;
  }
  if (s.find('/', pos) != -1) {
    // '/' after '.' is not extension, e.g., "./foo" "../bar"
    return nullptr;
  }
  return s.data() + pos + 1;
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
  m_origPathInfo.dump();
  m_absolutePath.dump();
  m_path.dump();
}

void RequestURI::clear() {
  m_originalURL.reset();
  m_queryString.reset();
  m_rewrittenURL.reset();
  m_resolvedURL.reset();
  m_pathInfo.reset();
  m_origPathInfo.reset();
  m_absolutePath.reset();
  m_path.reset();
}

const std::string RequestURI::getDefault404() {
  std::string ret = "404 File Not Found";
  if (RuntimeOption::PathDebug) {
    ret += "<br/>Paths examined:<ul>";
    for (auto& url : m_triedURLs) {
      ret += "<li>" + url + "</li>";
    }
    ret += "</ul>";
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}

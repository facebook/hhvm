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

#include <runtime/base/server/http_request_handler.h>
#include <runtime/base/program_functions.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/runtime_option.h>
#include <util/timer.h>
#include <runtime/base/server/static_content_cache.h>
#include <runtime/base/server/dynamic_content_cache.h>
#include <runtime/base/server/server_stats.h>
#include <util/network.h>
#include <runtime/base/preg.h>
#include <runtime/ext/ext_function.h>
#include <runtime/base/server/access_log.h>
#include <runtime/base/server/source_root_info.h>
#include <runtime/base/server/request_uri.h>
#include <runtime/base/server/http_protocol.h>
#include <runtime/base/time/datetime.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL(AccessLog::ThreadData,
                       HttpRequestHandler::s_accessLog_tl);
AccessLog HttpRequestHandler::s_accessLog(HttpRequestHandler::s_accessLog_tl);

HttpRequestHandler::HttpRequestHandler()
  : m_pathTranslation(true) {
}

void HttpRequestHandler::sendStaticContent(Transport *transport,
                                           const char *data, int len,
                                           time_t mtime,
                                           bool compressed,
                                           const std::string &cmd) {
  size_t pos = cmd.rfind('.');
  ASSERT(pos != string::npos);
  const char *ext = cmd.c_str() + pos + 1;
  map<string, string>::const_iterator iter =
    RuntimeOption::StaticFileExtensions.find(ext);
  if (iter != RuntimeOption::StaticFileExtensions.end()) {
    string val = iter->second;
    if (val == "text/plain" || val == "text/html") {
      // Apache adds character set for these two types
      val += "; charset=";
      val += RuntimeOption::DefaultCharsetName;
    }
    transport->addHeader("Content-Type", val.c_str());
  }

  time_t base = time(NULL);
  if (RuntimeOption::ExpiresActive) {
    time_t expires = base + RuntimeOption::ExpiresDefault;
    char age[20];
    snprintf(age, sizeof(age), "max-age=%d", RuntimeOption::ExpiresDefault);
    transport->addHeader("Cache-Control", age);
    transport->addHeader
      ("Expires", DateTime(expires, true).toString(DateTime::HttpHeader));
  }

  if (mtime) {
    transport->addHeader
      ("Last-Modified", DateTime(mtime, true).toString(DateTime::HttpHeader));
  }
  transport->addHeader("Accept-Ranges", "bytes");

  for (unsigned int i = 0; i < RuntimeOption::FilesMatches.size(); i++) {
    FilesMatch &rule = *RuntimeOption::FilesMatches[i];
    if (rule.match(cmd)) {
      const vector<string> &headers = rule.getHeaders();
      for (unsigned int j = 0; j < headers.size(); j++) {
        transport->addHeader(String(headers[j]));
      }
    }
  }

  // misnomer, it means we have made decision on compression, transport
  // should not attempt to compress it.
  transport->disableCompression();

  transport->sendRaw((void*)data, len, 200, compressed);
}

void HttpRequestHandler::handleRequest(Transport *transport) {
  Logger::OnNewRequest();
  GetAccessLog().onNewRequest();
  transport->enableCompression();

  Logger::Verbose("receiving %s", transport->getCommand().c_str());
  ServerStatsHelper ssh("all", true);

  // resolve virtual host
  const VirtualHost *vhost = HttpProtocol::GetVirtualHost(transport);
  ASSERT(vhost);
  if (vhost->disabled() ||
      vhost->isBlocking(transport->getCommand(), transport->getRemoteHost())) {
    transport->sendString("Not Found", 404);
    return;
  }
  ServerStats::StartRequest(transport->getCommand().c_str(),
                            transport->getRemoteHost(),
                            vhost->getName().c_str());

  // resolve source root
  string host = transport->getHeader("Host");
  SourceRootInfo sourceRootInfo(host.c_str());

  if (sourceRootInfo.error()) {
    sourceRootInfo.handleError(transport);
    return;
  }

  // request URI
  string pathTranslation = m_pathTranslation ?
    vhost->getPathTranslation().c_str() : "";
  RequestURI reqURI(vhost, transport, sourceRootInfo.path(), pathTranslation);
  if (reqURI.done()) {
    return; // already handled with redirection or 404
  }
  string path = reqURI.path().data();
  string absPath = reqURI.absolutePath().data();

  // determine whether we should compress response
  bool compressed = transport->decideCompression();

  const char *data; int len;
  size_t pos = path.rfind('.');
  const char *ext = (pos != string::npos) ? (path.c_str() + pos + 1) : NULL;
  bool cachableDynamicContent =
    (!RuntimeOption::StaticFileGenerators.empty() &&
     RuntimeOption::StaticFileGenerators.find(path) !=
     RuntimeOption::StaticFileGenerators.end());

  // If this is not a php file, check the static and dynamic content caches
  if (ext && strcasecmp(ext, "php") != 0) {
    if (RuntimeOption::EnableStaticContentCache) {
      // check against static content cache
      if (StaticContentCache::TheCache.find(path, data, len, compressed)) {
        struct stat st;
        st.st_mtime = 0;
        // (qigao) not calling stat at this point because the timestamp of
        // local cache file is not valuable, maybe misleading. This way
        // the Last-Modified header will not show in response.
        // stat(RuntimeOption::FileCache.c_str(), &st);
        sendStaticContent(transport, data, len, st.st_mtime, compressed, path);
        ServerStats::LogPage(path, 200);
        return;
      }
    }

    if (RuntimeOption::EnableStaticContentFromDisk &&
        RuntimeOption::StaticFileExtensions.find(ext) !=
        RuntimeOption::StaticFileExtensions.end()) {
      String translated = File::TranslatePath(String(absPath));
      if (!translated.empty()) {
        StringBuffer sb(translated.data());
        if (sb.valid()) {
          struct stat st;
          st.st_mtime = 0;
          stat(translated.data(), &st);
          sendStaticContent(transport, sb.data(), sb.size(), st.st_mtime,
                            false, path);
          ServerStats::LogPage(path, 200);
          return;
        }
      }
    }

    // check static contents that were generated by dynamic pages
    if (cachableDynamicContent) {
      // check against dynamic content cache
      ASSERT(transport->getUrl());
      string key = path + transport->getUrl();
      if (DynamicContentCache::TheCache.find(key, data, len, compressed)) {
        sendStaticContent(transport, data, len, 0, compressed, path);
        ServerStats::LogPage(path, 200);
        return;
      }
    }
  }

  // proxy any URLs that not specified in ServeURLs
  if (!RuntimeOption::ProxyOrigin.empty() &&
      ((RuntimeOption::UseServeURLs &&
        RuntimeOption::ServeURLs.find(path) ==
        RuntimeOption::ServeURLs.end()) ||
       (RuntimeOption::UseProxyURLs &&
        (RuntimeOption::ProxyURLs.find(path) !=
         RuntimeOption::ProxyURLs.end() ||
         MatchAnyPattern(path, RuntimeOption::ProxyPatterns) ||
         (abs(rand()) % 100) < RuntimeOption::ProxyPercentage)))) {
    for (int i = 0; i < RuntimeOption::ProxyRetry; i++) {
      bool force = (i == RuntimeOption::ProxyRetry - 1); // last one
      if (handleProxyRequest(transport, force)) break;
    }
    return;
  }

  // record request for debugging purpose
  std::string tmpfile = HttpProtocol::RecordRequest(transport);

  // main body
  hphp_session_init();

  bool ret = false;
  try {
    ret = executePHPRequest(transport, reqURI, sourceRootInfo,
                            cachableDynamicContent);
  } catch (...) {
    Logger::Error("Unhandled exception in HPHP server engine.");
  }
  GetAccessLog().log(transport);
  hphp_session_exit();

  HttpProtocol::ClearRecord(ret, tmpfile);
}

bool HttpRequestHandler::executePHPRequest(Transport *transport,
                                           RequestURI &reqURI,
                                           SourceRootInfo &sourceRootInfo,
                                           bool cachableDynamicContent) {
  ExecutionContext *context = hphp_context_init();
  if (RuntimeOption::ImplicitFlush) {
    context->obSetImplicitFlush(true);
  }
  if (RuntimeOption::EnableOutputBuffering) {
    if (RuntimeOption::OutputHandler.empty()) {
      context->obStart();
    } else {
      context->obStart(String(RuntimeOption::OutputHandler));
    }
  }
  context->setTransport(transport);

  string file = reqURI.absolutePath().c_str();
  {
    ServerStatsHelper ssh("input");
    HttpProtocol::PrepareSystemVariables(transport, reqURI, sourceRootInfo);
    reqURI.clear();
    sourceRootInfo.clear();
  }

  int code;
  bool ret = true;
  if (!RuntimeOption::ForbiddenFileExtensions.empty()) {
    size_t pos = file.rfind('.');
    if (pos != string::npos) {
      const char *ext = file.c_str() + pos + 1;
      if (RuntimeOption::ForbiddenFileExtensions.find(ext) !=
          RuntimeOption::ForbiddenFileExtensions.end()) {
        code = 403;
        transport->sendString("Forbidden", 403);
        ret = false;
      }
    }
  }

  if (ret) {
    bool error = false;
    std::string errorMsg = "Internal Server Error";
    ret = hphp_invoke(context, file, false, Array(), null,
                      RuntimeOption::WarmupDocument,
                      RuntimeOption::RequestInitFunction,
                      error, errorMsg);

    if (ret) {
      std::string content = context->getContents();
      if (cachableDynamicContent && !content.empty()) {
        ASSERT(transport->getUrl());
        string key = file + transport->getUrl();
        DynamicContentCache::TheCache.store(key, content.data(),
                                            content.size());
      }
      code = 200;
      transport->sendRaw((void*)content.data(), content.size());
    } else if (error) {
      code = 500;

      string errorPage = context->getErrorPage().data();
      if (errorPage.empty()) {
        errorPage = RuntimeOption::ErrorDocument500;
      }
      if (!errorPage.empty()) {
        context->obEndAll();
        context->obStart();
        context->obProtect(true);
        ret = hphp_invoke(context, errorPage, false, Array(), null,
                          RuntimeOption::WarmupDocument,
                          RuntimeOption::RequestInitFunction,
                          error, errorMsg);
        if (ret) {
          std::string content = context->getContents();
          transport->sendRaw((void*)content.data(), content.size());
        } else {
          errorPage.clear(); // so we fall back to 500 return
        }
      }
      if (errorPage.empty()) {
        if (RuntimeOption::ServerErrorMessage) {
          transport->sendString(errorMsg, 500);
        } else {
          transport->sendString(RuntimeOption::FatalErrorMessage, 500);
        }
      }
    } else {
      code = 404;
      transport->sendString("Not Found", 404);
    }
  }

  transport->onSendEnd();
  ServerStats::LogPage(file, code);
  hphp_context_exit(context, true);
  return ret;
}

bool HttpRequestHandler::handleProxyRequest(Transport *transport, bool force) {
  string url = RuntimeOption::ProxyOrigin + transport->getServerObject();

  int code = 0;
  std::string error;
  StringBuffer response;
  if (!HttpProtocol::ProxyRequest(transport, force, url, code, error,
                                  response)) {
    return false;
  }
  if (code == 0) {
    transport->sendString(error, 500);
    return true;
  }

  const char* respData = response.data();
  if (!respData) {
    respData = "";
  }
  transport->sendRaw((void*)respData, response.size(), code);
  return true;
}

bool HttpRequestHandler::MatchAnyPattern
(const std::string &path, const std::vector<std::string> &patterns) {
  String spath(path.c_str(), path.size(), AttachLiteral);
  for (unsigned int i = 0; i < patterns.size(); i++) {
    Variant ret = preg_match(String(patterns[i].c_str(), patterns[i].size(),
                                    AttachLiteral),
                             spath);
    if (ret.toInt64() > 0) return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}

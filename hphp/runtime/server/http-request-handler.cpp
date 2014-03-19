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
#include "hphp/runtime/server/http-request-handler.h"

#include <string>
#include <vector>

#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/timer.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/server/dynamic-content-cache.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/util/network.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/server/access-log.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/server/request-uri.h"
#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/server/files-match.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/util/alloc.h"
#include "hphp/util/service-data.h"

namespace HPHP {

using std::string;
using std::vector;

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL(AccessLog::ThreadData,
                       HttpRequestHandler::s_accessLogThreadData);

AccessLog HttpRequestHandler::s_accessLog(
  &(HttpRequestHandler::getAccessLogThreadData));

HttpRequestHandler::HttpRequestHandler(int timeout)
    : RequestHandler(timeout), m_pathTranslation(true)
    , m_requestTimedOutOnQueue(ServiceData::createTimeseries(
                                 "requests_timed_out_on_queue",
                                 {ServiceData::StatsType::COUNT})) { }

void HttpRequestHandler::sendStaticContent(Transport *transport,
                                           const char *data, int len,
                                           time_t mtime,
                                           bool compressed,
                                           const std::string &cmd,
                                           const char *ext) {
  assert(ext);
  assert(cmd.rfind('.') != std::string::npos);
  assert(strcmp(ext, cmd.c_str() + cmd.rfind('.') + 1) == 0);

  auto iter = RuntimeOption::StaticFileExtensions.find(ext);
  if (iter != RuntimeOption::StaticFileExtensions.end()) {
    string val = iter->second;
    const char *valp = val.c_str();
    if (strncmp(valp, "text/", 5)  == 0 &&
        (strcmp(valp + 5, "plain") == 0 ||
         strcmp(valp + 5, "html")  == 0)) {
      // Apache adds character set for these two types
      val += "; charset=";
      val += IniSetting::Get("default_charset");
      valp = val.c_str();
    }
    transport->addHeader("Content-Type", valp);
  } else {
    transport->addHeader("Content-Type", "application/octet-stream");
  }

  time_t base = time(nullptr);
  if (RuntimeOption::ExpiresActive) {
    time_t exp = base + RuntimeOption::ExpiresDefault;
    char age[20];
    snprintf(age, sizeof(age), "max-age=%d", RuntimeOption::ExpiresDefault);
    transport->addHeader("Cache-Control", age);
    transport->addHeader("Expires",
      DateTime(exp, true).toString(DateTime::DateFormat::HttpHeader).c_str());
  }

  if (mtime) {
    transport->addHeader("Last-Modified",
      DateTime(mtime, true).toString(DateTime::DateFormat::HttpHeader).c_str());
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
  ExecutionProfiler ep(ThreadInfo::RuntimeFunctions);

  Logger::OnNewRequest();
  GetAccessLog().onNewRequest();
  transport->enableCompression();

  ServerStatsHelper ssh("all", ServerStatsHelper::TRACK_MEMORY);
  Logger::Verbose("receiving %s", transport->getCommand().c_str());

  // will clear all extra logging when this function goes out of scope
  StackTraceNoHeap::ExtraLoggingClearer clearer;
  StackTraceNoHeap::AddExtraLogging("URL", transport->getUrl());

  // resolve virtual host
  const VirtualHost *vhost = HttpProtocol::GetVirtualHost(transport);
  assert(vhost);
  if (vhost->disabled() ||
      vhost->isBlocking(transport->getCommand(), transport->getRemoteHost())) {
    transport->sendString("Not Found", 404);
    return;
  }

  // don't serve the request if it's been sitting in queue for longer than our
  // allowed request timeout.
  int requestTimeoutSeconds =
    vhost->getRequestTimeoutSeconds(getDefaultTimeout());
  if (requestTimeoutSeconds > 0) {
    timespec now;
    Timer::GetMonotonicTime(now);
    const timespec& queueTime = transport->getQueueTime();

    if (gettime_diff_us(queueTime, now) > requestTimeoutSeconds * 1000000) {
      transport->sendString("Service Unavailable", 503);
      m_requestTimedOutOnQueue->addValue(1);
      return;
    }
  }

  ServerStats::StartRequest(transport->getCommand().c_str(),
                            transport->getRemoteHost(),
                            vhost->getName().c_str());

  // resolve source root
  SourceRootInfo sourceRootInfo(transport);

  if (sourceRootInfo.error()) {
    sourceRootInfo.handleError(transport);
    return;
  }

  // request URI
  string pathTranslation = m_pathTranslation ?
    vhost->getPathTranslation().c_str() : "";
  RequestURI reqURI(vhost, transport, pathTranslation, sourceRootInfo.path());
  if (reqURI.done()) {
    return; // already handled with redirection or 404
  }
  string path = reqURI.path().data();
  string absPath = reqURI.absolutePath().data();

  // determine whether we should compress response
  bool compressed = transport->decideCompression();

  const char *data; int len;
  const char *ext = reqURI.ext();

  if (reqURI.forbidden()) {
    transport->sendString("Forbidden", 403);
    return;
  }

  bool cachableDynamicContent =
    (!RuntimeOption::StaticFileGenerators.empty() &&
     RuntimeOption::StaticFileGenerators.find(path) !=
     RuntimeOption::StaticFileGenerators.end());

  // Determine which extensions should be treated as php
  // source code. If the execution engine doesn't understand
  // the source, the content will be spit out verbatim.
  bool treatAsContent = ext &&
       strcasecmp(ext, "php") != 0 &&
       strcasecmp(ext, "hh") != 0 &&
       (RuntimeOption::PhpFileExtensions.empty() ||
        !RuntimeOption::PhpFileExtensions.count(ext));

  // If this is not a php file, check the static and dynamic content caches
  if (treatAsContent) {
    bool original = compressed;
    // check against static content cache
    if (StaticContentCache::TheCache.find(path, data, len, compressed)) {
      ScopedMem decompressed_data;
      // (qigao) not calling stat at this point because the timestamp of
      // local cache file is not valuable, maybe misleading. This way
      // the Last-Modified header will not show in response.
      // stat(RuntimeOption::FileCache.c_str(), &st);
      if (!original && compressed) {
        data = gzdecode(data, len);
        if (data == nullptr) {
          throw FatalErrorException("cannot unzip compressed data");
        }
        decompressed_data = const_cast<char*>(data);
        compressed = false;
      }
      sendStaticContent(transport, data, len, 0, compressed, path, ext);
      ServerStats::LogPage(path, 200);
      GetAccessLog().log(transport, vhost);
      return;
    }

    if (RuntimeOption::EnableStaticContentFromDisk) {
      String translated = File::TranslatePath(String(absPath));
      if (!translated.empty()) {
        CstrBuffer sb(translated.data());
        if (sb.valid()) {
          struct stat st;
          st.st_mtime = 0;
          stat(translated.data(), &st);
          sendStaticContent(transport, sb.data(), sb.size(), st.st_mtime,
                            false, path, ext);
          ServerStats::LogPage(path, 200);
          GetAccessLog().log(transport, vhost);
          return;
        }
      }
    }

    // check static contents that were generated by dynamic pages
    if (cachableDynamicContent) {
      // check against dynamic content cache
      assert(transport->getUrl());
      string key = path + transport->getUrl();
      if (DynamicContentCache::TheCache.find(key, data, len, compressed)) {
        sendStaticContent(transport, data, len, 0, compressed, path, ext);
        ServerStats::LogPage(path, 200);
        GetAccessLog().log(transport, vhost);
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
  ThreadInfo::s_threadInfo->m_reqInjectionData.
    setTimeout(requestTimeoutSeconds);

  bool ret = false;
  try {
    ret = executePHPRequest(transport, reqURI, sourceRootInfo,
                            cachableDynamicContent);
  } catch (const Eval::DebuggerException &e) {
    transport->sendString(e.what(), 200);
    transport->onSendEnd();
    hphp_context_exit(g_context.getNoCheck(), true, true, transport->getUrl());
  } catch (...) {
    Logger::Error("Unhandled exception in HPHP server engine.");
  }
  GetAccessLog().log(transport, vhost);
  /*
   * HPHP logs may need to access data in ServerStats, so we have to
   * clear the hashtable after writing the log entry.
   */
  ServerStats::Reset();
  hphp_session_exit();

  HttpProtocol::ClearRecord(ret, tmpfile);
}

void HttpRequestHandler::abortRequest(Transport *transport) {
  GetAccessLog().onNewRequest();
  const VirtualHost *vhost = HttpProtocol::GetVirtualHost(transport);
  assert(vhost);
  transport->sendString("Service Unavailable", 503);
  GetAccessLog().log(transport, vhost);
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
    Extension::RequestInitModules();

    if (RuntimeOption::EnableDebugger) {
      Eval::DSandboxInfo sInfo = sourceRootInfo.getSandboxInfo();
      Eval::Debugger::RegisterSandbox(sInfo);
      context->setSandboxId(sInfo.id());
    }
    reqURI.clear();
    sourceRootInfo.clear();
  }

  int code;
  bool ret = true;

  if (RuntimeOption::EnableDebugger) {
    Eval::Debugger::InterruptRequestStarted(transport->getUrl());
  }

  bool error = false;
  std::string errorMsg = "Internal Server Error";
  ret = hphp_invoke(context, file, false, Array(), uninit_null(),
                    RuntimeOption::RequestInitFunction,
                    RuntimeOption::RequestInitDocument,
                    error, errorMsg);

  if (ret) {
    String content = context->obDetachContents();
    if (cachableDynamicContent && !content.empty()) {
      assert(transport->getUrl());
      string key = file + transport->getUrl();
      DynamicContentCache::TheCache.store(key, content.data(),
                                          content.size());
    }
    transport->sendRaw((void*)content.data(), content.size());
    code = transport->getResponseCode();
  } else if (error) {
    code = 500;

    string errorPage = context->getErrorPage().data();
    if (errorPage.empty()) {
      errorPage = RuntimeOption::ErrorDocument500;
    }
    if (!errorPage.empty()) {
      context->obProtect(false);
      context->obEndAll();
      context->obStart();
      context->obProtect(true);
      ret = hphp_invoke(context, errorPage, false, Array(), uninit_null(),
                        RuntimeOption::RequestInitFunction,
                        RuntimeOption::RequestInitDocument,
                        error, errorMsg);
      if (ret) {
        String content = context->obDetachContents();
        transport->sendRaw((void*)content.data(), content.size());
        code = transport->getResponseCode();
      } else {
        Logger::Error("Unable to invoke error page %s", errorPage.c_str());
        errorPage.clear(); // so we fall back to 500 return
      }
    }
    if (errorPage.empty()) {
      if (RuntimeOption::ServerErrorMessage) {
        transport->sendString(errorMsg, 500, false, false, "hphp_invoke");
      } else {
        transport->sendString(RuntimeOption::FatalErrorMessage,
                              500, false, false, "hphp_invoke");
      }
    }
  } else {
    code = 404;
    transport->sendString("Not Found", 404);
  }

  if (RuntimeOption::EnableDebugger) {
    Eval::Debugger::InterruptRequestEnded(transport->getUrl());
  }

  transport->onSendEnd();
  hphp_context_exit(context, true, true, transport->getUrl());
  ServerStats::LogPage(file, code);
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
    transport->sendString(error, 500, false, false, "handleProxyRequest");
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
  String spath(path.c_str(), path.size(), CopyString);
  for (unsigned int i = 0; i < patterns.size(); i++) {
    Variant ret = preg_match(String(patterns[i].c_str(), patterns[i].size(),
                                    CopyString),
                             spath);
    if (ret.toInt64() > 0) return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}

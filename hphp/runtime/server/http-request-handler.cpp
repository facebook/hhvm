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
#include "hphp/runtime/server/http-request-handler.h"

#include <string>
#include <vector>

#include "hphp/runtime/base/configs/debugger.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/hhprof.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/request-id.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/ext/extension-registry.h"
#include "hphp/runtime/ext/server/ext_server.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/server/access-log.h"
#include "hphp/runtime/server/files-match.h"
#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/server/request-uri.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/treadmill.h"

#include "hphp/util/alloc.h"
#include "hphp/util/hardware-counter.h"
#include "hphp/util/lock.h"
#include "hphp/util/mutex.h"
#include "hphp/util/network.h"
#include "hphp/util/service-data.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"

namespace HPHP {

namespace {
const StaticString s_defaultCharset("default_charset");
}

using std::string;
using std::vector;

///////////////////////////////////////////////////////////////////////////////

static ReadWriteMutex s_proxyMutex;
static __thread unsigned int s_randState = 0xfaceb00c;

static bool matchAnyPattern(const std::string &path,
                            const std::vector<std::string> &patterns) {
  String spath(path.c_str(), path.size(), CopyString);
  for (unsigned int i = 0; i < patterns.size(); i++) {
    Variant ret = preg_match(String(patterns[i].c_str(), patterns[i].size(),
                                    CopyString),
                             spath);
    if (ret.toInt64() > 0) return true;
  }
  return false;
}

/*
 * Returns true iff a request to the given path should be delegated to the
 * proxy origin.
 */
static bool shouldProxyPath(const std::string& path) {
  ReadLock lock(s_proxyMutex);

  if (RuntimeOption::ProxyOriginRaw.empty()) return false;

  if (RuntimeOption::UseServeURLs && RuntimeOption::ServeURLs.count(path)) {
    return true;
  }

  if (RuntimeOption::UseProxyURLs) {
    if (RuntimeOption::ProxyURLs.count(path)) return true;
    if (matchAnyPattern(path, RuntimeOption::ProxyPatterns)) return true;
  }

  if (RuntimeOption::ProxyPercentageRaw > 0) {
    if ((abs(rand_r(&s_randState)) % 100) < RuntimeOption::ProxyPercentageRaw) {
      return true;
    }
  }

  return false;
}

static std::string getProxyPath(const char* origPath) {
  ReadLock lock(s_proxyMutex);

  return RuntimeOption::ProxyOriginRaw + origPath;
}

void setProxyOriginPercentage(const std::string& origin, int percentage) {
  WriteLock lock(s_proxyMutex);

  RuntimeOption::ProxyOriginRaw = origin;
  RuntimeOption::ProxyPercentageRaw = percentage;
  Logger::Warning("Updated proxy origin to `%s' and percentage to %d\n",
                  origin.c_str(), percentage);
}

///////////////////////////////////////////////////////////////////////////////

THREAD_LOCAL(AccessLog::ThreadData, HttpRequestHandler::s_accessLogThreadData);

AccessLog HttpRequestHandler::s_accessLog(
  &(HttpRequestHandler::getAccessLogThreadData));

HttpRequestHandler::HttpRequestHandler(int timeout)
    : RequestHandler(timeout), m_pathTranslation(true)
    , m_requestTimedOutOnQueue(ServiceData::createTimeSeries(
                                 "requests_timed_out_on_queue",
                                 {ServiceData::StatsType::COUNT})) { }

void HttpRequestHandler::sendStaticContent(Transport *transport,
                                           const char *data, int len,
                                           time_t mtime,
                                           bool compressed,
                                           const std::string &cmd,
                                           const char *ext) {
  assertx(ext);
  assertx(cmd.rfind('.') != std::string::npos);
  assertx(strcmp(ext, cmd.c_str() + cmd.rfind('.') + 1) == 0);

  auto iter = RuntimeOption::StaticFileExtensions.find(ext);
  if (iter != RuntimeOption::StaticFileExtensions.end()) {
    string val = iter->second;
    const char *valp = val.c_str();
    if (strncmp(valp, "text/", 5)  == 0 &&
        (strcmp(valp + 5, "plain") == 0 ||
         strcmp(valp + 5, "html")  == 0)) {
      // Apache adds character set for these two types
      val += "; charset=";
      val += IniSetting::Get(s_defaultCharset);
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
      req::make<DateTime>(exp, true)->toString(
        DateTime::DateFormat::HttpHeader).c_str());
  }

  if (mtime) {
    transport->addHeader("Last-Modified",
      req::make<DateTime>(mtime, true)->toString(
        DateTime::DateFormat::HttpHeader).c_str());
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

  transport->sendRaw(data, len, 200, compressed);
  transport->onSendEnd();
}

void HttpRequestHandler::logToAccessLog(Transport* transport) {
  GetAccessLog().onNewRequest();
  GetAccessLog().log(transport, VirtualHost::GetCurrent());
}

void HttpRequestHandler::setupRequest(Transport* transport) {
  HHProf::Request::Setup(transport);

  g_context.getCheck()->setTransport(transport);

  GetAccessLog().onNewRequest();

  // Set current virtual host.
  HttpProtocol::GetVirtualHost(transport);
}

void HttpRequestHandler::teardownRequest(Transport* transport) noexcept {
  SCOPE_EXIT { always_assert(tl_heap->empty()); };

  const VirtualHost *vhost = VirtualHost::GetCurrent();
  GetAccessLog().log(transport, vhost);

  // HPHP logs may need to access data in ServerStats, so we have to clear the
  // hashtable after writing the log entry.
  ServerStats::Reset();
  SourceRootInfo::ResetLogging();

  if (is_hphp_session_initialized()) {
    hphp_session_exit();
  } else {
    // Even though there are no sessions, memory is allocated to perform
    // INI setting bindings when the thread is initialized.
    hphp_memory_cleanup();
  }

  HHProf::Request::Teardown();
}

void HttpRequestHandler::handleRequest(Transport *transport) {
  ExecutionProfiler ep(RequestInfo::RuntimeFunctions);

  auto const requestId = RequestId::allocate();
  Logger::OnNewRequest(requestId.id());
  transport->enableCompression();

  ServerStatsHelper ssh("all",
                        ServerStatsHelper::TRACK_MEMORY |
                        ServerStatsHelper::TRACK_HWINST);
  Logger::Verbose("receiving %s", transport->getCommand().c_str());

  // will clear all extra logging when this function goes out of scope
  StackTraceNoHeap::ExtraLoggingClearer clearer;
  StackTraceNoHeap::AddExtraLogging("URL", transport->getUrl());

  // resolve virtual host
  const VirtualHost *vhost = VirtualHost::GetCurrent();
  assertx(vhost);
  if (vhost->disabled() ||
      vhost->isBlocking(transport->getCommand(), transport->getRemoteHost())) {
    transport->sendString("Not Found", 404);
    transport->onSendEnd();
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

    if (gettime_diff_us(queueTime, now) > requestTimeoutSeconds * 1000000LL) {
      if (RuntimeOption::Server503RetryAfterSeconds >= 0) {
        transport->addHeader("Retry-After", folly::to<std::string>(
              RuntimeOption::Server503RetryAfterSeconds).c_str());
      }
      transport->sendString("Service Unavailable", 503);
      transport->onSendEnd();
      m_requestTimedOutOnQueue->addValue(1);
      return;
    }
  }

  ServerStats::StartRequest(transport->getCommand().c_str(),
                            transport->getRemoteHost(),
                            vhost->getName().c_str());

  // resolve source root
  if (!SourceRootInfo::Init(transport)) {
    transport->sendString("Sandbox not found", 200);
    return;
  }

  // request URI
  string pathTranslation = m_pathTranslation ?
    vhost->getPathTranslation().c_str() : "";
  RequestURI reqURI(vhost, transport, pathTranslation,
                    SourceRootInfo::GetCurrentSourceRoot());
  if (reqURI.done()) {
    return; // already handled with redirection or 404
  }
  string path = reqURI.path().data();
  string absPath = reqURI.absolutePath().data();

  const char *ext = reqURI.ext();

  if (reqURI.forbidden()) {
    transport->sendString("Forbidden", 403);
    transport->onSendEnd();
    return;
  }

  // Determine which extensions should be treated as php
  // source code. If the execution engine doesn't understand
  // the source, the content will be spit out verbatim.
  bool treatAsContent = ext &&
       strcasecmp(ext, "php") != 0 &&
       strcasecmp(ext, "hh") != 0 &&
       strcasecmp(ext, "hack") != 0 &&
       strcasecmp(ext, "hackpartial") != 0 &&
       (RuntimeOption::PhpFileExtensions.empty() ||
        !RuntimeOption::PhpFileExtensions.count(ext));

  // If this is not a php file, check the static content cache
  if (treatAsContent) {
    // bool compressed = acceptsPrecompressed;
    // check against static content cache
    if (StaticContentCache::TheFileCache) {
      auto content = StaticContentCache::TheFileCache->content(path);
      if (content) {
        ScopedMem decompressed_data;
        // (qigao) not calling stat at this point because the timestamp of
        // local cache file is not valuable, maybe misleading. This way
        // the Last-Modified header will not show in response.
        // stat(RuntimeOption::FileCache.c_str(), &st);
        sendStaticContent(transport, content->buffer, content->size, 0, false,
                          path, ext);
        ServerStats::LogPage(path, 200);
        return;
      }
    }

    if (RuntimeOption::EnableStaticContentFromDisk) {
      String translated = File::TranslatePath(String(absPath));
      if (!translated.empty() &&
          handleFileRequest(transport, translated, path, ext)) {
        return;
      }
    }
  }

  // proxy any URLs that not specified in ServeURLs
  if (shouldProxyPath(path)) {
    for (int i = 0; i < RuntimeOption::ProxyRetry; i++) {
      bool force = (i == RuntimeOption::ProxyRetry - 1); // last one
      if (handleProxyRequest(transport, force)) break;
    }
    return;
  }

  // record request for debugging purpose
  std::string tmpfile = HttpProtocol::RecordRequest(transport);

  // main body
  hphp_session_init(Treadmill::SessionKind::HttpRequest, transport, requestId);
  RequestInfo::s_requestInfo->m_reqInjectionData.
    setTimeout(requestTimeoutSeconds);

  bool ret = false;
  try {
    ret = executePHPRequest(transport, reqURI);
  } catch (...) {
    string emsg;
    string response;
    int code = 500;
    try {
      throw;
    } catch (const Eval::DebuggerException& e) {
      code = 200;
      response = e.what();
    } catch (Object &e) {
      try {
        emsg = throwable_to_string(e.get()).data();
      } catch (...) {
        emsg = "Unknown";
      }
    } catch (const std::exception& e) {
      emsg = e.what();
    } catch (...) {
      emsg = "Unknown";
    }
    g_context->onShutdownPostSend();
    Eval::Debugger::InterruptPSPEnded(transport->getUrl());
    if (code != 200) {
      Logger::Error("Unhandled server exception: %s", emsg.c_str());
    }
    transport->sendString(response, code);
    transport->onSendEnd();
    hphp_context_exit();
  }
  HttpProtocol::ClearRecord(ret, tmpfile);
}

void HttpRequestHandler::abortRequest(Transport* transport) {
  // TODO: t5284137 add some tests for abortRequest
  if (RuntimeOption::Server503RetryAfterSeconds >= 0) {
    transport->addHeader("Retry-After", folly::to<std::string>(
          RuntimeOption::Server503RetryAfterSeconds).c_str());
  }
  transport->sendString("Service Unavailable", 503);
  transport->onSendEnd();
}

bool HttpRequestHandler::executePHPRequest(Transport *transport,
                                           RequestURI &reqURI) {
  tracing::Request _{
    "http-request",
    reqURI.originalURL().c_str(),
    [&] {
      return tracing::Props{}
        .add("url", reqURI.originalURL().c_str())
        .add("absolute_path", reqURI.absolutePath().c_str());
    }
  };

  auto context = g_context.getNoCheck();
  OBFlags obFlags = OBFlags::Default;
  if (transport->getHTTPVersion() != "1.1") {
    obFlags |= OBFlags::OutputDisabled;
  }
  context->obStart(uninit_null(), 0, obFlags);
  context->obProtect(true);
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
  InitFiniNode::RequestStart();

  string file = reqURI.absolutePath().c_str();
  {
    ServerStatsHelper ssh("input");
    HttpProtocol::PrepareSystemVariables(transport, reqURI);

    if (Cfg::Debugger::EnableHphpd) {
      Eval::DSandboxInfo sInfo = SourceRootInfo::GetSandboxInfo();
      Eval::Debugger::RegisterSandbox(sInfo);
      context->setSandboxId(sInfo.id());
    }
    reqURI.clear();
  }

  int code;
  bool ret = true;

  // Let the debugger initialize.
  // FIXME: hphpd can be initialized this way as well
  DEBUGGER_ATTACHED_ONLY(phpDebuggerRequestInitHook());
  if (Cfg::Debugger::EnableHphpd) {
    Eval::Debugger::InterruptRequestStarted(transport->getUrl());
  }

  bool error = false;
  std::string errorMsg = "Internal Server Error";
  ret = hphp_invoke(context, file, false, Array(), nullptr,
                    RuntimeOption::RequestInitFunction,
                    RuntimeOption::RequestInitDocument,
                    error, errorMsg,
                    true /* once */,
                    false /* warmupOnly */,
                    false /* richErrorMessage */,
                    RuntimeOption::EvalPreludePath,
                    true /* allowDynCallNoPointer */);

  if (ret) {
    String content = context->obDetachContents();
    transport->sendRaw(content.data(), content.size());
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
      ret = hphp_invoke(context, errorPage, false, Array(), nullptr,
                        RuntimeOption::RequestInitFunction,
                        RuntimeOption::RequestInitDocument,
                        error, errorMsg,
                        true /* once */,
                        false /* warmupOnly */,
                        false /* richErrorMessage */,
                        RuntimeOption::EvalPreludePath,
                        true /* allowDynCallNoPointer */);
      if (ret) {
        String content = context->obDetachContents();
        transport->sendRaw(content.data(), content.size());
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
    transport->sendString("RequestInitDocument Not Found", 404);
  }

  if (Cfg::Debugger::EnableHphpd) {
    Eval::Debugger::InterruptRequestEnded(transport->getUrl());
  }

  StructuredLogEntry* entry = nullptr;
  if (RuntimeOption::EvalProfileHWStructLog) {
    entry = transport->createStructuredLogEntry();
    entry->setInt("response_code", code);
    auto queueBegin = transport->getQueueTime();
    auto const queueTimeUs = gettime_diff_us(queueBegin,
                                             transport->getWallTime());
    entry->setInt("queue-time-us", queueTimeUs);
    StructuredLog::recordRequestGlobals(*entry);
    tl_heap->recordStats(*entry);
    entry->setInt("uptime", HHVM_FN(server_uptime)());
  }
  HardwareCounter::UpdateServiceData(transport->getCpuTime(),
                                     transport->getWallTime(),
                                     entry,
                                     false /*psp*/);
  if (entry) {
    StructuredLog::log("hhvm_request_perf", *entry);
  }

  transport->onSendEnd();
  context->onShutdownPostSend();
  Eval::Debugger::InterruptPSPEnded(transport->getUrl());

  if (RuntimeOption::EvalProfileHWStructLog) {
    // We now reuse the same entry created previously for non-psp, with updates
    // on certain metrics (memory and hardware counters).
    entry->setInt("response_code", transport->getResponseCode());
    tl_heap->recordStats(*entry);
    entry->setInt("uptime", HHVM_FN(server_uptime)());
    entry->setInt("rss", ProcStatus::adjustedRssKb());
    if (use_lowptr) {
      entry->setInt("low_mem", alloc::getLowMapped());
    }
  }
  HardwareCounter::UpdateServiceData(transport->getCpuTime(),
                                     transport->getWallTime(),
                                     entry,
                                     true /*psp*/);
  if (entry) {
    StructuredLog::log("hhvm_request_perf", *entry);
    transport->resetStructuredLogEntry();
  }

  hphp_context_exit();
  ServerStats::LogPage(file, code);
  return ret;
}

bool HttpRequestHandler::handleProxyRequest(Transport *transport, bool force) {
  auto const url = getProxyPath(transport->getServerObject());

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
  transport->sendRaw(respData, response.size(), code);
  return true;
}

bool HttpRequestHandler::handleFileRequest(Transport* transport,
                                           const String& translated,
                                           const std::string& path,
                                           const char* ext) {
  static constexpr size_t kMaxCap = INT_MAX - 1;
  auto filename = translated.data();
  int fd = ::open(filename, O_RDONLY);
  if (fd != -1) {
    struct stat stat_buf;
    stat_buf.st_mtime = 0;
    if (fstat(fd, &stat_buf) == 0) {
      size_t cap = stat_buf.st_size;
      if (cap > kMaxCap) {
        ::close(fd);
        auto const str = folly::to<std::string>(
          "file ", filename, " is too large"
        );
        throw StringBufferLimitException(kMaxCap, String(str.c_str()));
      }
      auto buffer = (char*)safe_malloc(cap + 1);
      size_t len = 0;
      while (len < cap) {
        auto n = ::read(fd, buffer + len, cap - len);
        if (n == -1 && errno == EINTR) continue;
        if (n <= 0) break;
        len += n;
      }
      ::close(fd);
      buffer[len] = 0;
      sendStaticContent(transport, buffer, len, stat_buf.st_mtime,
                        false, path, ext);
      ServerStats::LogPage(path, 200);
      return true;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}

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

#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/base/hphp-system.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/server/request-uri.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/util/logger.h"
#include "hphp/util/util.h"
#include "hphp/runtime/server/upload.h"
#include "hphp/runtime/server/replay-transport.h"
#include "hphp/runtime/server/virtual-host.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/vm/jit/arch.h"
#include <boost/lexical_cast.hpp>

#define DEFAULT_POST_CONTENT_TYPE "application/x-www-form-urlencoded"

using std::map;
using boost::lexical_cast;
using boost::bad_lexical_cast;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// helper functions

static bool read_all_post_data(Transport *transport,
                               const void *&data, int &size) {
  if (transport->hasMorePostData()) {
    data = Util::buffer_duplicate(data, size);
    do {
      int delta = 0;
      const void *extra = transport->getMorePostData(delta);
      if (size + delta < VirtualHost::GetMaxPostSize()) {
        data = Util::buffer_append(data, size, extra, delta);
        size += delta;
      }
    } while (transport->hasMorePostData());
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

const VirtualHost *HttpProtocol::GetVirtualHost(Transport *transport) {
  if (!RuntimeOption::VirtualHosts.empty()) {
    string host = transport->getHeader("Host");
    for (unsigned int i = 0; i < RuntimeOption::VirtualHosts.size(); i++) {
      VirtualHostPtr vhost = RuntimeOption::VirtualHosts[i];
      if (vhost->match(host)) {
        VirtualHost::SetCurrent(vhost.get());
        return vhost.get();
      }
    }
  }
  VirtualHost::SetCurrent(nullptr);
  return VirtualHost::GetCurrent();
}

const StaticString
  s_REQUEST_START_TIME("REQUEST_START_TIME"),
  s_HPHP("HPHP"),
  s_HHVM("HHVM"),
  s_HHVM_JIT("HHVM_JIT"),
  s_HHVM_ARCH("HHVM_ARCH"),
  s_HPHP_SERVER("HPHP_SERVER"),
  s_HPHP_HOTPROFILER("HPHP_HOTPROFILER"),
  s_HTTP_HOST("HTTP_HOST"),
  s_CONTENT_TYPE("CONTENT_TYPE"),
  s_CONTENT_LENGTH("CONTENT_LENGTH"),
  s_PHP_AUTH_USER("PHP_AUTH_USER"),
  s_PHP_AUTH_PW("PHP_AUTH_PW"),
  s_REQUEST_URI("REQUEST_URI"),
  s_SCRIPT_URL("SCRIPT_URL"),
  s_SCRIPT_URI("SCRIPT_URI"),
  s_SCRIPT_NAME("SCRIPT_NAME"),
  s_PHP_SELF("PHP_SELF"),
  s_SCRIPT_FILENAME("SCRIPT_FILENAME"),
  s_PATH_TRANSLATED("PATH_TRANSLATED"),
  s_PATH_INFO("PATH_INFO"),
  s_argc("argc"),
  s_argv("argv"),
  s__SERVER("_SERVER"),
  s__GET("_GET"),
  s__POST("_POST"),
  s__REQUEST("_REQUEST"),
  s__SESSION("_SESSION"),
  s__ENV("_ENV"),
  s__COOKIE("_COOKIE"),
  s_HTTP_RAW_POST_DATA("HTTP_RAW_POST_DATA"),
  s__FILES("_FILES"),
  s_GATEWAY_INTERFACE("GATEWAY_INTERFACE"),
  s_CGI_1_1("CGI/1.1"),
  s_SERVER_ADDR("SERVER_ADDR"),
  s_SERVER_NAME("SERVER_NAME"),
  s_SERVER_PORT("SERVER_PORT"),
  s_SERVER_SOFTWARE("SERVER_SOFTWARE"),
  s_SERVER_PROTOCOL("SERVER_PROTOCOL"),
  s_SERVER_ADMIN("SERVER_ADMIN"),
  s_SERVER_SIGNATURE("SERVER_SIGNATURE"),
  s_REQUEST_METHOD("REQUEST_METHOD"),
  s_GET("GET"),
  s_HEAD("HEAD"),
  s_POST("POST"),
  s_HTTPS("HTTPS"),
  s_1("1"),
  s_REQUEST_TIME("REQUEST_TIME"),
  s_REQUEST_TIME_FLOAT("REQUEST_TIME_FLOAT"),
  s_QUERY_STRING("QUERY_STRING"),
  s_REMOTE_ADDR("REMOTE_ADDR"),
  s_REMOTE_HOST("REMOTE_HOST"),
  s_REMOTE_PORT("REMOTE_PORT"),
  s_DOCUMENT_ROOT("DOCUMENT_ROOT"),
  s_THREAD_TYPE("THREAD_TYPE"),
  s_dash("-"),
  s_underscore("_"),
  s_HTTP_("HTTP_");

static auto const s_arraysToClear = {
  s__SERVER,
  s__GET,
  s__POST,
  s__FILES,
  s__REQUEST,
  s__ENV,
  s__COOKIE,
};

static auto const s_arraysToUnset = {
  s__SESSION,
};

/**
 * PHP has "EGPCS" processing order of these global variables, and this
 * order is important in preparing $_REQUEST that needs to know which to
 * overwrite what when name happens to be the same.
 */
void HttpProtocol::PrepareSystemVariables(Transport *transport,
                                          const RequestURI &r,
                                          const SourceRootInfo &sri) {

  const VirtualHost *vhost = VirtualHost::GetCurrent();
  GlobalVariables *g = get_global_variables();
  Variant emptyArr(HphpArray::GetStaticEmptyArray());
  for (auto& key : s_arraysToClear) {
    g->remove(key.get(), false);
    g->set(key.get(), emptyArr, false);
  }
  for (auto& key : s_arraysToUnset) {
    g->remove(key.get(), false);
  }
  g->set(s_HTTP_RAW_POST_DATA, empty_string, false);

  StartRequest();
  PrepareEnv(g->getRef(s__ENV), transport);
  PrepareRequestVariables(g->getRef(s__REQUEST),
                          g->getRef(s__GET),
                          g->getRef(s__POST),
                          g->getRef(s_HTTP_RAW_POST_DATA),
                          g->getRef(s__FILES),
                          g->getRef(s__COOKIE),
                          transport,
                          r);
  PrepareServerVariable(g->getRef(s__SERVER),
                        transport,
                        r,
                        sri,
                        vhost);
}

void HttpProtocol::PrepareEnv(Variant& env,
                              Transport *transport) {
  // $_ENV
  process_env_variables(env);
  env.set(s_HPHP, 1);
  env.set(s_HHVM, 1);
  if (RuntimeOption::EvalJit) {
    env.set(s_HHVM_JIT, 1);
  }
  switch (JIT::arch()) {
  case JIT::Arch::X64:
    env.set(s_HHVM_ARCH, "x64");
    break;
  case JIT::Arch::ARM:
    env.set(s_HHVM_ARCH, "arm");
    break;
  }

  bool isServer = RuntimeOption::ServerExecutionMode();
  if (isServer) {
    env.set(s_HPHP_SERVER, 1);
#ifdef HOTPROFILER
    env.set(s_HPHP_HOTPROFILER, 1);
#endif
  }
}

void HttpProtocol::PrepareRequestVariables(Variant& request,
                                           Variant& get,
                                           Variant& post,
                                           Variant& raw_post,
                                           Variant& files,
                                           Variant& cookie,
                                           Transport *transport,
                                           const RequestURI &r) {

  // $_GET and $_REQUEST
  if (!r.queryString().empty()) {
    PrepareGetVariable(get, r);
    CopyParams(request, get);
  }

  // $_POST and $_REQUEST
  if (transport->getMethod() == Transport::Method::POST) {
    PreparePostVariables(post, raw_post, files, transport);
    CopyParams(request, post);
  }

  // $_COOKIE
  if (PrepareCookieVariable(cookie, transport)) {
    CopyParams(request, cookie);
  }
}

void HttpProtocol::PrepareGetVariable(Variant& get,
                                      const RequestURI &r) {
  DecodeParameters(get,
                   r.queryString().data(),
                   r.queryString().size());
}

void HttpProtocol::PreparePostVariables(Variant& post,
                                        Variant& raw_post,
                                        Variant& files,
                                        Transport *transport) {

  string contentType = transport->getHeader("Content-Type");
  string contentLength = transport->getHeader("Content-Length");

  bool needDelete = false;
  int size = 0;
  const void *data = transport->getPostData(size);
  if (data && size) {
    string boundary;
    int content_length = atoi(contentLength.c_str());
    bool rfc1867Post = IsRfc1867(contentType, boundary);
    string files_str;
    if (rfc1867Post) {
      if (content_length > VirtualHost::GetMaxPostSize()) {
        // $_POST and $_FILES are empty
        Logger::Warning("POST Content-Length of %d bytes exceeds "
                        "the limit of %" PRId64 " bytes",
                        content_length, VirtualHost::GetMaxPostSize());
        while (transport->hasMorePostData()) {
          int delta = 0;
          transport->getMorePostData(delta);
        }
      } else {
        if (transport->hasMorePostData()) {
          needDelete = true;
          data = Util::buffer_duplicate(data, size);
        }
        DecodeRfc1867(transport,
                      post,
                      files,
                      content_length,
                      data,
                      size,
                      boundary);
      }
      assert(!transport->getFiles(files_str));
    } else {
      needDelete = read_all_post_data(transport, data, size);

      bool decodeData = strncasecmp(contentType.c_str(),
                                    DEFAULT_POST_CONTENT_TYPE,
                                    sizeof(DEFAULT_POST_CONTENT_TYPE)-1) == 0;
      // Always decode data for now. (macvicar)
      decodeData = true;

      if (decodeData) {
        DecodeParameters(post, (const char*)data, size, true);
      }

      bool ret = transport->getFiles(files_str);
      if (ret) {
        files = unserialize_from_string(files_str);
      }
    }

    if (needDelete) {
      if (RuntimeOption::AlwaysPopulateRawPostData &&
          uint32_t(size) <= StringData::MaxSize) {
        raw_post = String((char*)data, size, AttachString);
      } else {
        free((void *)data);
      }
    } else {
      // For literal we disregard RuntimeOption::AlwaysPopulateRawPostData
      if (uint32_t(size) <= StringData::MaxSize) {
        raw_post = String((char*)data, size, CopyString);
      }
    }
  }
}

bool HttpProtocol::PrepareCookieVariable(Variant& cookie,
                                         Transport *transport) {

  string cookie_data = transport->getHeader("Cookie");
  if (!cookie_data.empty()) {
    StringBuffer sb;
    sb.append(cookie_data);
    DecodeCookies(cookie, (char*)sb.data());
    return true;
  } else {
    return false;
  }
}

void HttpProtocol::CopyHeaderVariables(Variant& server,
                                       const HeaderMap& headers) {
  static std::atomic<int> badRequests(-1);

  std::vector<std::string> badHeaders;
  for (auto const& header : headers) {
    auto const& key = header.first;
    auto const& values = header.second;
    auto normalizedKey = s_HTTP_ +
                         string_replace(f_strtoupper(key), s_dash,
                                        s_underscore);

    // Detect suspicious headers.  We are about to modify header names for
    // the SERVER variable.  This means that it is possible to deliberately
    // cause a header collision, which an attacker could use to sneak a
    // header past a proxy that would either overwrite or filter it
    // otherwise.  Client code should use apache_request_headers() to
    // retrieve the original headers if they are security-critical.
    if (RuntimeOption::LogHeaderMangle != 0 &&
        server.asArrRef().exists(normalizedKey)) {
      badHeaders.push_back(key);
    }

    if (!values.empty()) {
      // When a header has multiple values, we always take the last one.
      server.set(normalizedKey, String(values.back()));
    }
  }

  if (!badHeaders.empty()) {
    auto reqId = badRequests.fetch_add(1, std::memory_order_acq_rel) + 1;
    if (!(reqId % RuntimeOption::LogHeaderMangle)) {
      std::string badNames = folly::join(", ", badHeaders);
      std::string allHeaders;

      const char* separator = "";
      for (auto const& header : headers) {
        for (auto const& value : header.second) {
          folly::toAppend(separator, header.first, ": ", value,
                          &allHeaders);
          separator = "\n";
        }
      }

      Logger::Warning(
        "HeaderMangle warning: "
        "The header(s) [%s] overwrote other headers which mapped to the same "
        "key. This happens because PHP normalises - to _, ie AN_EXAMPLE "
        "and AN-EXAMPLE are equivalent. You should treat this as "
        "malicious. All headers from this request:\n%s",
        badNames.c_str(), allHeaders.c_str());
    }
  }
}

void HttpProtocol::CopyServerInfo(Variant& server,
                                  Transport *transport,
                                  const VirtualHost *vhost) {

  string hostHeader = transport->getHeader("Host");
  String hostName(vhost->serverName(hostHeader));

  if (hostHeader.empty()) {
    server.set(s_HTTP_HOST, hostName);
    StackTraceNoHeap::AddExtraLogging("Server", hostName.data());
  } else {
    StackTraceNoHeap::AddExtraLogging("Server", hostHeader.c_str());
  }
  if (hostName.empty() || RuntimeOption::ForceServerNameToHeader) {
    hostName = hostHeader;
    // _SERVER['SERVER_NAME'] shouldn't contain the port number
    int colonPos = hostName.find(':');
    if (colonPos != String::npos) {
      hostName = hostName.substr(0, colonPos);
    }
  }

  server.set(s_GATEWAY_INTERFACE, s_CGI_1_1);
  server.set(s_SERVER_ADDR, String(RuntimeOption::ServerPrimaryIP));
  server.set(s_SERVER_NAME, hostName);
  server.set(s_SERVER_PORT, RuntimeOption::ServerPort);
  server.set(s_SERVER_SOFTWARE, s_HPHP);
  server.set(s_SERVER_PROTOCOL, "HTTP/" + transport->getHTTPVersion());
  server.set(s_SERVER_ADMIN, empty_string);
  server.set(s_SERVER_SIGNATURE, empty_string);
}

void HttpProtocol::CopyRemoteInfo(Variant& server,
                                  Transport *transport) {
  server.set(s_REMOTE_ADDR, String(transport->getRemoteHost(), CopyString));
  server.set(s_REMOTE_HOST, empty_string); // I don't think we need to nslookup
  server.set(s_REMOTE_PORT, transport->getRemotePort());
}

void HttpProtocol::CopyAuthInfo(Variant& server,
                                Transport *transport) {
  // APE processes Authorization: Basic into PHP_AUTH_USER and PHP_AUTH_PW
  string authorization = transport->getHeader("Authorization");
  if (!authorization.empty()) {
    if (strncmp(authorization.c_str(), "Basic ", 6) == 0) {
      // it's safe to pass this as a string literal since authorization
      // outlives decodedAuth; this saves us a superfluous copy.
      String decodedAuth =
        StringUtil::Base64Decode(String(authorization.c_str() + 6));
      int colonPos = decodedAuth.find(':');
      if (colonPos != String::npos) {
        server.set(s_PHP_AUTH_USER, decodedAuth.substr(0, colonPos));
        server.set(s_PHP_AUTH_PW, decodedAuth.substr(colonPos + 1));
      }
    }
  }
}

void HttpProtocol::CopyPathInfo(Variant& server,
                                Transport *transport,
                                const RequestURI& r,
                                const VirtualHost *vhost) {
  server.set(s_REQUEST_URI, String(transport->getUrl(), CopyString));
  server.set(s_SCRIPT_URL, r.originalURL());
  String prefix(transport->isSSL() ? "https://" : "http://");

  // Need to append port
  assert(server.toCArrRef().exists(s_SERVER_PORT));
  std::string serverPort = "80";
  if (server.toCArrRef().exists(s_SERVER_PORT)) {
    CHECK(server[s_SERVER_PORT].isInteger() ||
          server[s_SERVER_PORT].isString());
    if (server[s_SERVER_PORT].isInteger()) {
      serverPort = lexical_cast<string>(server[s_SERVER_PORT].toInt32());
    } else {
      serverPort = server[s_SERVER_PORT].toString()->data();
    }
  }

  String port_suffix("");
  if (!transport->isSSL() && serverPort != "80") {
    port_suffix = folly::format(":{}", serverPort).str();
  }

  string hostHeader;
  if (server.toCArrRef().exists(s_HTTP_HOST)) {
    hostHeader = server[s_HTTP_HOST].toCStrRef()->data();
  }
  String hostName;
  if (server.toCArrRef().exists(s_SERVER_NAME)) {
    assert(server[s_SERVER_NAME].isString());
    hostName = server[s_SERVER_NAME].toCStrRef();
  }
  server.set(s_SCRIPT_URI,
             String(prefix + (hostHeader.empty() ? hostName + port_suffix :
                              String(hostHeader)) + r.originalURL()));

  if (r.rewritten()) {
    // when URL is rewritten, PHP decided to put original URL as SCRIPT_NAME
    String name = r.originalURL();
    if (!r.pathInfo().empty()) {
      int pos = name.find(r.pathInfo());
      if (pos >= 0) {
        name = name.substr(0, pos);
      }
    }
    if (r.defaultDoc()) {
      if (!name.empty() && name[name.length() - 1] != '/') {
        name += "/";
      }
      name += String(RuntimeOption::DefaultDocument);
    }
    server.set(s_SCRIPT_NAME, name);
  } else {
    server.set(s_SCRIPT_NAME, r.resolvedURL());
  }

  if (r.rewritten()) {
    server.set(s_PHP_SELF, r.originalURL());
  } else {
    server.set(s_PHP_SELF, r.resolvedURL() + r.origPathInfo());
  }

  String documentRoot = transport->getDocumentRoot();
  if (documentRoot.empty()) {
    // Right now this is just RuntimeOption::SourceRoot but mwilliams wants to
    // fix it so it is settable, so I'll leave this for now
    documentRoot = vhost->getDocumentRoot();
  }
  server.set(s_DOCUMENT_ROOT, documentRoot);
  server.set(s_SCRIPT_FILENAME, r.absolutePath());

  if (r.pathInfo().empty()) {
    server.set(s_PATH_TRANSLATED, r.absolutePath());
  } else {
    assert(server.toCArrRef().exists(s_DOCUMENT_ROOT));
    assert(server[s_DOCUMENT_ROOT].isString());
    server.set(s_PATH_TRANSLATED,
               String(server[s_DOCUMENT_ROOT].toCStrRef() +
                      r.pathInfo().data()));
    server.set(s_PATH_INFO, r.pathInfo());
  }

  switch (transport->getMethod()) {
  case Transport::Method::GET:  server.set(s_REQUEST_METHOD, s_GET);  break;
  case Transport::Method::HEAD: server.set(s_REQUEST_METHOD, s_HEAD); break;
  case Transport::Method::POST:
    if (transport->getExtendedMethod() == nullptr) {
      server.set(s_REQUEST_METHOD, s_POST);
    } else {
      server.set(s_REQUEST_METHOD, transport->getExtendedMethod());
    }
    break;
  default:
    server.set(s_REQUEST_METHOD, empty_string); break;
  }
  server.set(s_HTTPS, transport->isSSL() ? s_1 : empty_string);
  server.set(s_QUERY_STRING, r.queryString());

  server.set(s_argv, make_packed_array(r.queryString()));
  server.set(s_argc, 1);
}

void HttpProtocol::StartRequest() {
  GlobalVariables *g = get_global_variables();
  Variant& server = g->getRef(s__SERVER);
  server.set(s_REQUEST_START_TIME, time(nullptr));
  time_t now;
  struct timeval tp = {0};
  double now_double;
  if (!gettimeofday(&tp, nullptr)) {
    now_double = (double)(tp.tv_sec + tp.tv_usec / 1000000.00);
    now = tp.tv_sec;
  } else {
    now = time(nullptr);
    now_double = (double)now;
  }
  server.set(s_REQUEST_TIME, now);
  server.set(s_REQUEST_TIME_FLOAT, now_double);
}

void HttpProtocol::PrepareServerVariable(Variant& server,
                                         Transport *transport,
                                         const RequestURI &r,
                                         const SourceRootInfo &sri,
                                         const VirtualHost *vhost) {
  // $_SERVER

  string contentType = transport->getHeader("Content-Type");
  string contentLength = transport->getHeader("Content-Length");

  // HTTP_ headers -- we don't exclude headers we handle elsewhere (e.g.,
  // Content-Type, Authorization), since the CGI "spec" merely says the server
  // "may" exclude them; this is not what APE does, but it's harmless.
  HeaderMap headers;
  transport->getHeaders(headers);
  CopyHeaderVariables(server, headers);
  CopyServerInfo(server, transport, vhost);
  CopyRemoteInfo(server, transport);
  CopyAuthInfo(server, transport);

  CopyPathInfo(server, transport, r, vhost);

  // APE sets CONTENT_TYPE and CONTENT_LENGTH without HTTP_
  if (!contentType.empty()) {
    server.set(s_CONTENT_TYPE, String(contentType));
  }
  if (!contentLength.empty()) {
    server.set(s_CONTENT_LENGTH, String(contentLength));
  }

  CopyPathInfo(server, transport, r, vhost);

  for (map<string, string>::const_iterator iter =
         RuntimeOption::ServerVariables.begin();
       iter != RuntimeOption::ServerVariables.end(); ++iter) {
      server.set(String(iter->first), String(iter->second));
  }
  const map<string, string> &vServerVars = vhost->getServerVars();
  for (map<string, string>::const_iterator iter =
         vServerVars.begin();
       iter != vServerVars.end(); ++iter) {
    server.set(String(iter->first), String(iter->second));
  }
  sri.setServerVariables(server);

  const char *threadType = transport->getThreadTypeName();
  server.set(s_THREAD_TYPE, threadType);
  StackTraceNoHeap::AddExtraLogging("ThreadType", threadType);
}

std::string HttpProtocol::RecordRequest(Transport *transport) {
  char tmpfile[PATH_MAX + 1];
  if (RuntimeOption::RecordInput) {
    strcpy(tmpfile, "/tmp/hphp_request_XXXXXX");
    close(mkstemp(tmpfile));

    ReplayTransport rt;
    rt.recordInput(transport, tmpfile);
    Logger::Info("request recorded in %s", tmpfile);
    return tmpfile;
  }
  return "";
}

void HttpProtocol::ClearRecord(bool success, const std::string &tmpfile) {
  if (success && RuntimeOption::ClearInputOnSuccess && !tmpfile.empty()) {
    unlink(tmpfile.c_str());
    Logger::Info("request %s deleted", tmpfile.c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////

void HttpProtocol::CopyParams(Variant &dest, Variant &src) {
  if (src.isArray()) {
    Array srcArray = src.toArray();
    for (ArrayIter iter(srcArray); iter; ++iter) {
      dest.set(iter.first(), iter.second());
    }
  }
}

void HttpProtocol::DecodeParameters(Variant &variables, const char *data,
                                    int size, bool post /* = false */) {
  if (data == nullptr || size == 0) {
    return;
  }

  const char *s = data;
  const char *e = s + size;
  const char *p, *val;

  while (s < e && (p = (const char *)memchr(s, '&', (e - s)))) {
  last_value:
    if ((val = (const char *)memchr(s, '=', (p - s)))) {
      int len = val - s;
      char *name = url_decode(s, len);
      String sname(name, len, AttachString);

      val++;
      len = p - val;
      char *value = url_decode(val, len);
      if (RuntimeOption::EnableMagicQuotesGpc) {
        char *slashedvalue = string_addslashes(value, len);
        free(value);
        value = slashedvalue;
      }
      String svalue(value, len, AttachString);

      register_variable(variables, (char*)sname.data(), svalue);
    } else if (!post) {
      int len = p - s;
      char *name = url_decode(s, len);
      String sname(name, len, AttachString);
      register_variable(variables, (char*)sname.data(), "");
    }
    s = p + 1;
  }
  if (s < e) {
    p = e;
    goto last_value;
  }
}

void HttpProtocol::DecodeCookies(Variant &variables, char *data) {
  assert(data && *data);

  char *strtok_buf = nullptr;
  char *var = strtok_r(data, ";", &strtok_buf);
  while (var) {
    char *val = strchr(var, '=');

    // Remove leading spaces from cookie names, needed for multi-cookie
    // header where ; can be followed by a space */
    while (isspace(*var)) {
      var++;
    }

    if (var != val && *var != '\0') {
      if (val) { /* have a value */
        int len = val - var;
        char *name = url_decode(var, len);
        String sname(name, len, AttachString);

        ++val;
        len = strlen(val);
        char *value = url_decode(val, len);
        if (RuntimeOption::EnableMagicQuotesGpc) {
          char *slashedvalue = string_addslashes(value, len);
          free(value);
          value = slashedvalue;
        }
        String svalue(value, len, AttachString);

        register_variable(variables, (char*)sname.data(), svalue, false);
      } else {
        int len = strlen(var);
        char *name = url_decode(var, len);
        String sname(name, len, AttachString);

        register_variable(variables, (char*)sname.data(), "", false);
      }
    }

    var = strtok_r(nullptr, ";", &strtok_buf);
  }
}

bool HttpProtocol::IsRfc1867(const string contentType, string &boundary) {
  if (contentType.empty()) return false;
  const char *ctstr = contentType.c_str();
  char *s;
  char *e;
  for (s = (char*)ctstr; *s && !(*s == ';' || *s == ',' || *s == ' '); s++) ;
  if (strncasecmp(ctstr, MULTIPART_CONTENT_TYPE, s - ctstr)) {
    return false;
  }
  s = strstr(s, "boundary");
  if (!s || !(s=strchr(s, '='))) {
    Logger::Warning("Missing boundary in multipart/form-data POST data");
    return false;
  }
  s++;
  if (s[0] == '"') {
    s++;
    e = strchr(s, '"');
    if (!e) {
      Logger::Warning("Invalid boundary in multipart/form-data POST data");
      return false;
    }
  } else {
    /* search for the end of the boundary */
    e = strchr(s, ',');
  }
  if (e) {
    e[0] = '\0';
  }
  boundary = s;
  return true;
}

void HttpProtocol::DecodeRfc1867(Transport *transport,
                                 Variant &post,
                                 Variant &files,
                                 int contentLength,
                                 const void *&data,
                                 int &size,
                                 string boundary) {
  rfc1867PostHandler(transport,
                     post,
                     files,
                     contentLength,
                     data,
                     size,
                     boundary);
}

const char *HttpProtocol::GetReasonString(int code) {
  switch (code) {
  case 100: return "Continue";
  case 101: return "Switching Protocols";
  case 200: return "OK";
  case 201: return "Created";
  case 202: return "Accepted";
  case 203: return "Non-Authoritative Information";
  case 204: return "No Content";
  case 205: return "Reset Content";
  case 206: return "Partial Content";
  case 207: return "Multi-Status";
  case 300: return "Multiple Choices";
  case 301: return "Moved Permanently";
  case 302: return "Found";
  case 303: return "See Other";
  case 304: return "Not Modified";
  case 305: return "Use Proxy";
  case 307: return "Temporary Redirect";
  case 400: return "Bad Request";
  case 401: return "Unauthorized";
  case 402: return "Payment Required";
  case 403: return "Forbidden";
  case 404: return "Not Found";
  case 405: return "Method Not Allowed";
  case 406: return "Not Acceptable";
  case 407: return "Proxy Authentication Required";
  case 408: return "Request Time-out";
  case 409: return "Conflict";
  case 410: return "Gone";
  case 411: return "Length Required";
  case 412: return "Precondition Failed";
  case 413: return "Request Entity Too Large";
  case 414: return "Request-URI Too Large";
  case 415: return "Unsupported Media Type";
  case 416: return "Requested range not satisfiable";
  case 417: return "Expectation Failed";
  case 500: return "Internal Server Error";
  case 501: return "Not Implemented";
  case 502: return "Bad Gateway";
  case 503: return "Service Unavailable";
  case 504: return "Gateway Time-out";
  case 505: return "HTTP Version not supported";
  }
  return "Bad Response";
}

///////////////////////////////////////////////////////////////////////////////

bool HttpProtocol::ProxyRequest(Transport *transport, bool force,
                                const std::string &url,
                                int &code, std::string &error,
                                StringBuffer &response,
                                HeaderMap *extraHeaders /* = NULL */) {
  assert(transport);
  if (transport->headersSent()) {
    raise_warning("Cannot proxy request - headers already sent");
    return false;
  }

  HeaderMap requestHeaders;
  transport->getHeaders(requestHeaders);
  if (extraHeaders) {
    for (HeaderMap::const_iterator iter = extraHeaders->begin();
         iter != extraHeaders->end(); ++iter) {
      vector<string> &values = requestHeaders[iter->first];
      values.insert(values.end(), iter->second.begin(), iter->second.end());
    }
  }

  int size = 0;
  const char *data = nullptr;
  if (transport->getMethod() == Transport::Method::POST) {
    data = (const char *)transport->getPostData(size);
  }

  code = 0; // HTTP status of curl or 0 for "no server response code"
  vector<String> responseHeaders;
  HttpClient http;
  if (data && size) {
    code = http.post(url.c_str(), data, size, response, &requestHeaders,
                     &responseHeaders);
  } else {
    code = http.get(url.c_str(), response, &requestHeaders, &responseHeaders);
  }
  if (code == 0) {
    if (!force) return false; // so we can retry
    Logger::Error("Unable to proxy %s: %s", url.c_str(),
                  http.getLastError().c_str());
    error = http.getLastError();
    return true;
  }

  for (unsigned int i = 0; i < responseHeaders.size(); i++) {
    String &header = responseHeaders[i];
    if (header.find(":") != String::npos &&
        header.find("Content-Length: ") != 0 &&
        header.find("Client-Transfer-Encoding: ") != 0 &&
        header.find("Transfer-Encoding: ") != 0 &&
        header.find("Connection: ") != 0) {
      transport->addHeader(header.data());
    }
  }
  const char* respData = response.data();
  if (!respData) {
    respData = "";
  }
  Logger::Verbose("Response code was %d when proxying %s", code, url.c_str());
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}

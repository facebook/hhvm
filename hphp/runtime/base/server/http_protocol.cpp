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

#include "hphp/runtime/base/server/http_protocol.h"
#include "hphp/runtime/base/hphp_system.h"
#include "hphp/runtime/base/zend/zend_url.h"
#include "hphp/runtime/base/zend/zend_string.h"
#include "hphp/runtime/base/program_functions.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/base/server/source_root_info.h"
#include "hphp/runtime/base/server/request_uri.h"
#include "hphp/runtime/base/server/transport.h"
#include "hphp/util/logger.h"
#include "hphp/util/util.h"
#include "hphp/runtime/base/server/upload.h"
#include "hphp/runtime/base/server/replay_transport.h"
#include "hphp/runtime/base/server/virtual_host.h"
#include "hphp/runtime/base/util/http_client.h"

#define DEFAULT_POST_CONTENT_TYPE "application/x-www-form-urlencoded"

using std::map;

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

static const StaticString
  s_REQUEST_START_TIME("REQUEST_START_TIME"),
  s_HPHP("HPHP"),
  s_HHVM("HHVM"),
  s_HHVM_JIT("HHVM_JIT"),
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
  s_THREAD_TYPE("THREAD_TYPE");

/**
 * PHP has "EGPCS" processing order of these global variables, and this
 * order is important in preparing $_REQUEST that needs to know which to
 * overwrite what when name happens to be the same.
 */
void HttpProtocol::PrepareSystemVariables(Transport *transport,
                                          const RequestURI &r,
                                          const SourceRootInfo &sri) {
  SystemGlobals *g = (SystemGlobals*)get_global_variables();
  const VirtualHost *vhost = VirtualHost::GetCurrent();

  Variant &server = g->GV(_SERVER);
  server.set(s_REQUEST_START_TIME, time(nullptr));

  // $_ENV
  process_env_variables(g->GV(_ENV));
  g->GV(_ENV).set(s_HPHP, 1);
  g->GV(_ENV).set(s_HHVM, 1);
  if (RuntimeOption::EvalJit) {
    g->GV(_ENV).set(s_HHVM_JIT, 1);
  }

  bool isServer = RuntimeOption::serverExecutionMode();
  if (isServer) {
    g->GV(_ENV).set(s_HPHP_SERVER, 1);
#ifdef HOTPROFILER
    g->GV(_ENV).set(s_HPHP_HOTPROFILER, 1);
#endif
  }

  Variant &request = g->GV(_REQUEST);

  // $_GET and $_REQUEST
  if (!r.queryString().empty()) {
    DecodeParameters(g->GV(_GET), r.queryString().data(),
                     r.queryString().size());
    CopyParams(request, g->GV(_GET));
  }

  string contentType = transport->getHeader("Content-Type");
  string contentLength = transport->getHeader("Content-Length");
  // $_POST and $_REQUEST
  if (transport->getMethod() == Transport::POST) {
    bool needDelete = false;
    int size = 0;
    const void *data = transport->getPostData(size);
    if (data && size) {
      assert(((char*)data)[size] == 0); // we need a NULL terminated string
      string boundary;
      int content_length = atoi(contentLength.c_str());
      bool rfc1867Post = IsRfc1867(contentType, boundary);
      string files;
      if (rfc1867Post) {
        if (content_length > VirtualHost::GetMaxPostSize()) {
          // $_POST and $_FILES are empty
          Logger::Warning("POST Content-Length of %d bytes exceeds "
                          "the limit of %lld bytes",
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
          DecodeRfc1867(transport, g->GV(_POST), g->GV(_FILES),
                        content_length, data, size, boundary);
        }
        assert(!transport->getFiles(files));
      } else {
        needDelete = read_all_post_data(transport, data, size);

        bool decodeData = strncasecmp(contentType.c_str(),
                                       DEFAULT_POST_CONTENT_TYPE,
                                       sizeof(DEFAULT_POST_CONTENT_TYPE)-1) == 0;
        // Always decode data for now. (macvicar)
        decodeData = true;

        if (decodeData) {
          DecodeParameters(g->GV(_POST), (const char*)data, size, true);
        }

        bool ret = transport->getFiles(files);
        if (ret) {
          g->GV(_FILES) = unserialize_from_string(files);
        }
      }
      CopyParams(request, g->GV(_POST));
      if (needDelete) {
        if (RuntimeOption::AlwaysPopulateRawPostData &&
            uint32_t(size) <= StringData::MaxSize) {
          g->GV(HTTP_RAW_POST_DATA) = String((char*)data, size, AttachString);
        } else {
          free((void *)data);
        }
      } else {
        // For literal we disregard RuntimeOption::AlwaysPopulateRawPostData
        if (uint32_t(size) <= StringData::MaxSize) {
          g->GV(HTTP_RAW_POST_DATA) = String((char*)data, size, AttachLiteral);
        }
      }
    }
  }

  // $_COOKIE
  string cookie_data = transport->getHeader("Cookie");
  if (!cookie_data.empty()) {
    StringBuffer sb;
    sb.append(cookie_data);
    DecodeCookies(g->GV(_COOKIE), (char*)sb.data());
    CopyParams(request, g->GV(_COOKIE));
  }

  // $_SERVER

  // HTTP_ headers -- we don't exclude headers we handle elsewhere (e.g.,
  // Content-Type, Authorization), since the CGI "spec" merely says the server
  // "may" exclude them; this is not what APE does, but it's harmless.
  HeaderMap headers;
  transport->getHeaders(headers);

  static int bad_request_count = -1;
  for (HeaderMap::const_iterator iter = headers.begin();
       iter != headers.end(); ++iter) {
    const vector<string> &values = iter->second;

    // Detect suspicious headers.  We are about to modify header names
    // for the SERVER variable.  This means that it is possible to
    // deliberately cause a header collision, which an attacker could
    // use to sneak a header past a proxy that would either overwrite
    // or filter it otherwise.  Client code should use
    // apache_request_headers() to retrieve the original headers if
    // they are security-critical.
    if (RuntimeOption::LogHeaderMangle != 0) {
      String key = "HTTP_";
      key += StringUtil::ToUpper(iter->first).replace("-","_");
      if (server.asArrRef().exists(key)) {
        if (!(++bad_request_count % RuntimeOption::LogHeaderMangle)) {
          Logger::Warning(
            "HeaderMangle warning: "
            "The header %s overwrote another header which mapped to the same "
            "key. This happens because PHP normalises - to _, ie AN_EXAMPLE "
            "and AN-EXAMPLE are equivalent.  You should treat this as "
            "malicious.",
            iter->first.c_str());
        }
      }
    }

    for (unsigned int i = 0; i < values.size(); i++) {
      String key = "HTTP_";
      key += StringUtil::ToUpper(iter->first).replace("-", "_");
      server.set(key, String(values[i]));
    }
  }

  string host = transport->getHeader("Host");
  String hostName(VirtualHost::GetCurrent()->serverName(host));
  string hostHeader(host);
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

  // APE sets CONTENT_TYPE and CONTENT_LENGTH without HTTP_
  if (!contentType.empty()) {
    server.set(s_CONTENT_TYPE, String(contentType));
  }
  if (!contentLength.empty()) {
    server.set(s_CONTENT_LENGTH, String(contentLength));
  }

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

  server.set(s_REQUEST_URI, String(transport->getUrl(), CopyString));
  server.set(s_SCRIPT_URL, r.originalURL());
  String prefix(transport->isSSL() ? "https://" : "http://");
  String port_suffix("");

  // Need to append port
  if (!transport->isSSL() && RuntimeOption::ServerPort != 80) {
    port_suffix = ":" + RuntimeOption::ServerPort;
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
  if (!r.rewritten() && r.pathInfo().empty()) {
    server.set(s_PHP_SELF, r.resolvedURL());
  } else {
    // when URL is rewritten, or pathinfo is not empty, use original URL
    server.set(s_PHP_SELF, r.originalURL());
  }

  server.set(s_SCRIPT_FILENAME, r.absolutePath());
  if (r.pathInfo().empty()) {
    server.set(s_PATH_TRANSLATED, r.absolutePath());
  } else {
    server.set(s_PATH_TRANSLATED,
               String(vhost->getDocumentRoot() + r.pathInfo().data()));
    server.set(s_PATH_INFO, r.pathInfo());
  }

  server.set(s_argv, r.queryString());
  server.set(s_argc, 0);
  server.set(s_GATEWAY_INTERFACE, s_CGI_1_1);
  server.set(s_SERVER_ADDR, String(RuntimeOption::ServerPrimaryIP));
  server.set(s_SERVER_NAME, hostName);
  server.set(s_SERVER_PORT, RuntimeOption::ServerPort);
  server.set(s_SERVER_SOFTWARE, s_HPHP);
  server.set(s_SERVER_PROTOCOL, "HTTP/" + transport->getHTTPVersion());
  server.set(s_SERVER_ADMIN, empty_string);
  server.set(s_SERVER_SIGNATURE, empty_string);
  switch (transport->getMethod()) {
  case Transport::GET:  server.set(s_REQUEST_METHOD, s_GET);  break;
  case Transport::HEAD: server.set(s_REQUEST_METHOD, s_HEAD); break;
  case Transport::POST:
    if (transport->getExtendedMethod() == nullptr) {
      server.set(s_REQUEST_METHOD, s_POST);
    } else {
      server.set(s_REQUEST_METHOD, transport->getExtendedMethod());
    }
    break;
  default:              server.set(s_REQUEST_METHOD, empty_string);     break;
  }
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
  server.set(s_HTTPS, transport->isSSL() ? s_1 : empty_string);
  server.set(s_REQUEST_TIME, now);
  server.set(s_REQUEST_TIME_FLOAT, now_double);
  server.set(s_QUERY_STRING, r.queryString());

  server.set(s_REMOTE_ADDR, String(transport->getRemoteHost(), CopyString));
  server.set(s_REMOTE_HOST, empty_string); // I don't think we need to nslookup
  server.set(s_REMOTE_PORT, transport->getRemotePort());

  server.set(s_DOCUMENT_ROOT, String(vhost->getDocumentRoot()));

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
                                 Variant &post, Variant &files,
                                 int contentLength, const void *&data,
                                 int &size, string boundary) {
  rfc1867PostHandler(transport, post, files, contentLength,
                     data, size, boundary);
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
  if (transport->getMethod() == Transport::POST) {
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

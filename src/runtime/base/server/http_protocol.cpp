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

#include <runtime/base/server/http_protocol.h>
#include <runtime/base/zend/zend_url.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/program_functions.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/source_root_info.h>
#include <runtime/base/server/request_uri.h>
#include <runtime/base/server/transport.h>
#include <util/logger.h>
#include <system/gen/sys/system_globals.h>
#include <system/gen/php/globals/symbols.h>
#include <runtime/base/server/upload.h>
#include <runtime/base/server/replay_transport.h>
#include <runtime/base/util/http_client.h>

using namespace std;

namespace HPHP {
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
  VirtualHost::SetCurrent(NULL);
  return VirtualHost::GetCurrent();
}

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

  // reset global symbols to nulls or empty arrays
  pm_php$globals$symbols_php();

  // $_ENV
  process_env_variables(g->gv__ENV);
  g->gv__ENV.set("HPHP", 1);
  g->gv__ENV.set("HPHP_SERVER", 1);
#ifdef HOTPROFILER
  g->gv__ENV.set("HPHP_HOTPROFILER", 1);
#endif

  Variant &request = g->gv__REQUEST;

  // $_GET and $_REQUEST
  if (!r.queryString().empty()) {
    DecodeParameters(g->gv__GET, r.queryString().data(),
                     r.queryString().size());
    CopyParams(request, g->gv__GET);
  }

  string contentType = transport->getHeader("Content-Type");
  string contentLength = transport->getHeader("Content-Length");
  // $_POST and $_REQUEST
  if (transport->getMethod() == Transport::POST) {
    int size = 0;
    const void *data = transport->getPostData(size);
    if (data && size) {
      ASSERT(((char*)data)[size] == 0); // we need a NULL terminated string
      string boundary;
      bool rfc1867Post = IsRfc1867(contentType, boundary);
      if (rfc1867Post) {
        int content_length = atoi(contentLength.c_str());
        if (content_length > RuntimeOption::MaxPostSize) {
          // $_POST and $_FILES are empty
          Logger::Warning("POST Content-Length of %d bytes exceeds "
                          "the limit of %ld bytes",
                          size, RuntimeOption::MaxPostSize);
        } else {
          DecodeRfc1867(g->gv__POST, g->gv__FILES, content_length,
                        (const char*)data, size, boundary);
        }
      } else {
        DecodeParameters(g->gv__POST, (const char*)data, size, true);
      }
      CopyParams(request, g->gv__POST);
      g->gv_HTTP_RAW_POST_DATA = String((char*)data, size, AttachLiteral);
    }
  }

  // $_COOKIE
  string cookie_data = transport->getHeader("Cookie");
  if (!cookie_data.empty()) {
    StringBuffer sb;
    sb.append(cookie_data);
    DecodeCookies(g->gv__COOKIE, (char*)sb.data());
    CopyParams(request, g->gv__COOKIE);
  }

  // $_SERVER
  Variant &server = g->gv__SERVER;

  // HTTP_ headers -- we don't exclude headers we handle elsewhere (e.g.,
  // Content-Type, Authorization), since the CGI "spec" merely says the server
  // "may" exclude them; this is not what APE does, but it's harmless.
  HeaderMap headers;
  transport->getHeaders(headers);
  for (HeaderMap::const_iterator iter = headers.begin();
       iter != headers.end(); ++iter) {
    const vector<string> &values = iter->second;
    for (unsigned int i = 0; i < values.size(); i++) {
      String key = "HTTP_";
      key += StringUtil::ToUpper(iter->first).replace("-", "_");
      server.set(key, String(values[i]));
    }
  }
  String hostName(VirtualHost::GetCurrent()->serverName());
  string hostHeader(transport->getHeader("Host"));
  if (hostHeader.empty()) {
    server.set("HTTP_HOST", hostName);
  }
  if (hostName.empty() || RuntimeOption::ForceServerNameToHeader) {
    hostName = hostHeader;
  }

  // APE sets CONTENT_TYPE and CONTENT_LENGTH without HTTP_
  if (!contentType.empty()) {
    server.set("CONTENT_TYPE", String(contentType));
  }
  if (!contentLength.empty()) {
    server.set("CONTENT_LENGTH", String(contentLength));
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
        server.set("PHP_AUTH_USER", decodedAuth.substr(0, colonPos));
        server.set("PHP_AUTH_PW", decodedAuth.substr(colonPos + 1));
      }
    }
  }

  server.set("REQUEST_URI", String(transport->getUrl(), CopyString));
  server.set("SCRIPT_URL", r.originalURL());
  server.set("SCRIPT_URI", String(string("http://") + hostName.data() +
                                  r.originalURL().data()));
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
    server.set("SCRIPT_NAME", name);
  } else {
    server.set("SCRIPT_NAME", r.resolvedURL());
  }
  if (!r.rewritten() && r.pathInfo().empty()) {
    server.set("PHP_SELF", r.resolvedURL());
  } else {
    // when URL is rewritten, or pathinfo is not empty, use original URL
    server.set("PHP_SELF", r.originalURL());
  }

  server.set("SCRIPT_FILENAME", r.absolutePath());
  if (r.pathInfo().empty()) {
    server.set("PATH_TRANSLATED", r.absolutePath());
  } else {
    server.set("PATH_TRANSLATED",
               String(vhost->getDocumentRoot() + r.pathInfo().data()));
    server.set("PATH_INFO", r.pathInfo());
  }

  server.set("argv", r.queryString());
  server.set("argc", 0);
  server.set("GATEWAY_INTERFACE", "CGI/1.1");
  server.set("SERVER_ADDR", String(RuntimeOption::ServerPrimaryIP));
  server.set("SERVER_NAME", hostName);
  server.set("SERVER_PORT", RuntimeOption::ServerPort);
  server.set("SERVER_SOFTWARE", "HPHP");
  server.set("SERVER_PROTOCOL", "HTTP/1.1");
  server.set("SERVER_ADMIN", "");
  server.set("SERVER_SIGNATURE", "");
  switch (transport->getMethod()) {
  case Transport::GET:  server.set("REQUEST_METHOD", "GET");  break;
  case Transport::HEAD: server.set("REQUEST_METHOD", "HEAD"); break;
  case Transport::POST:
    if (transport->getExtendedMethod() == NULL) {
      server.set("REQUEST_METHOD", "POST");
    } else {
      server.set("REQUEST_METHOD", transport->getExtendedMethod());
    }
    break;
  default:              server.set("REQUEST_METHOD", "");     break;
  }
  server.set("HTTPS", "");
  server.set("REQUEST_TIME", time(NULL));
  server.set("QUERY_STRING", r.queryString());

  server.set("REMOTE_ADDR", String(transport->getRemoteHost(), CopyString));
  server.set("REMOTE_HOST", ""); // I don't think we need to nslookup
  server.set("REMOTE_PORT", 0);  // TODO: quite useless

  server.set("DOCUMENT_ROOT", String(vhost->getDocumentRoot()));

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

  switch (transport->getThreadType()) {
  case Transport::RequestThread:
    server.set("THREAD_TYPE", "REQUEST");
    break;
  case Transport::XboxThread:
    server.set("THREAD_TYPE", "XBOX");
    break;
  case Transport::RpcThread:
    server.set("THREAD_TYPE", "RPC");
    break;
  }
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
  if (data == NULL || size == 0) {
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
  ASSERT(data && *data);

  char *strtok_buf = NULL;
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

    var = strtok_r(NULL, ";", &strtok_buf);
  }
}

bool HttpProtocol::IsRfc1867(const string contentType, string &boundary) {
  if (contentType.empty()) return false;
  const char *ctstr = contentType.c_str();
  char *s;
  char *e;
  for (s = (char*)ctstr; *s && !(*s == ';' || *s == ',' || *s == ' '); s++) ;
  if (strncmp(ctstr, MULTIPART_CONTENT_TYPE, s - ctstr)) {
    return false;
  }
  s = strstr(s, "boundary");
  if (!s || !(s=strchr(s, '='))) {
    Logger::Warning("Missing boundary in multipart/form-data POST data");
    return false;
  }
  s++;
  int len = strlen(s);
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
    len = e - s;
  }
  boundary = s;
  return true;
}

void HttpProtocol::DecodeRfc1867(Variant &post, Variant &files,
                                 int contentLength, const char *data, int size,
                                 string boundary) {
  rfc1867PostHandler(post, files, contentLength, data, size, boundary);
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
  const char *data = NULL;
  if (transport->getMethod() == Transport::POST) {
    data = (const char *)transport->getPostData(size);
  }

  code = 0;
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

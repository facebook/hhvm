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

#include <runtime/base/server/transport.h>
#include <runtime/base/server/server.h>
#include <runtime/base/server/upload.h>
#include <runtime/base/server/server_stats.h>
#include <runtime/base/file/file.h>
#include <runtime/base/string_util.h>
#include <runtime/base/time/datetime.h>
#include <runtime/base/zend/zend_url.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/access_log.h>
#include <util/compression.h>
#include <util/util.h>
#include <util/logger.h>
#include <util/compatibility.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Transport::Transport()
  : m_url(NULL), m_postData(NULL), m_postDataParsed(false),
    m_chunkedEncoding(false), m_headerSent(false),
    m_responseCode(-1), m_firstHeaderSet(false), m_firstHeaderLine(0),
    m_responseSize(0), m_responseTotalSize(0), m_responseSentSize(0),
    m_flushTimeUs(0), m_sendContentType(true),
    m_compression(true), m_compressor(NULL), m_isSSL(false),
    m_compressionDecision(NotDecidedYet), m_threadType(RequestThread) {
  memset(&m_queueTime, 0, sizeof(m_queueTime));
  memset(&m_wallTime, 0, sizeof(m_wallTime));
  memset(&m_cpuTime, 0, sizeof(m_cpuTime));
}

Transport::~Transport() {
  if (m_url) {
    free(m_url);
  }
  if (m_postData) {
    free(m_postData);
  }
  if (m_compressor) {
    delete m_compressor;
  }
}

void Transport::onRequestStart(const timespec &queueTime) {
  m_queueTime = queueTime;
  gettime(CLOCK_MONOTONIC, &m_wallTime);
  gettime(CLOCK_THREAD_CPUTIME_ID, &m_cpuTime);
}

const char *Transport::getMethodName() {
  switch (getMethod()) {
    case GET:  return "GET";
    case HEAD: return "HEAD";
    case POST: {
      const char *m = getExtendedMethod();
      return m ? m : "POST";
    }
    default:
      break;
    }
  return "";
}

///////////////////////////////////////////////////////////////////////////////
// url

const char *Transport::getServerObject() {
  const char *url = getUrl();
  int strip = 0;
  if (strncmp(url, "http://", 7) == 0) {
    strip = 7;
  } else if (strncmp(url, "https://", 8) == 0) {
    strip = 8;
  }
  if (strip == 0) return url;
  const char *p = strchr(url + strip, '/');
  if (p) return p;

  return "";
}

string Transport::getCommand() {
  const char *url = getServerObject();
  ASSERT(url);
  if (!*url) {
    return "";
  }

  if (*url == '/') {
    ++url;
  }
  const char *v = strchr(url, '?');
  if (v) {
    return string(url, v - url);
  }
  return url;
}

///////////////////////////////////////////////////////////////////////////////
// parameters

// copied and re-factored from clearsilver-0.10.5/cgi/cgi.c
void Transport::urlUnescape(char *value) {
  ASSERT(value && *value); // check before calling this function

  int i = 0, o = 0;
  unsigned char *s = (unsigned char *)value;

  while (s[i]) {
    if (s[i] == '+') {
      s[o++] = ' ';
      i++;
    } else if (s[i] == '%' && isxdigit(s[i+1]) && isxdigit(s[i+2])) {
      char num;
      num = (s[i+1] >= 'A') ? ((s[i+1] & 0xdf) - 'A') + 10 : (s[i+1] - '0');
      num *= 16;
      num += (s[i+2] >= 'A') ? ((s[i+2] & 0xdf) - 'A') + 10 : (s[i+2] - '0');
      s[o++] = num;
      i+=3;
    } else {
      s[o++] = s[i++];
    }
  }
  if (i && o) s[o] = '\0';
}

void Transport::parseQuery(char *query, ParamMap &params) {
  if (!query || !*query) return;

  char *l;
  char *k = strtok_r(query, "&", &l);
  while (k && *k) {
    char *v = strchr(k, '=');
    const char *fv;
    if (v == NULL) {
      fv = "";
    } else {
      *v = '\0';
      v++;
      if (*v) urlUnescape(v);
      fv = v;
    }
    if (*k) urlUnescape(k);
    params[k].push_back(fv);
    k = strtok_r(NULL, "&", &l);
  }
}

void Transport::parseGetParams() {
  FiberWriteLock lock(this);

  if (m_url == NULL) {
    const char *url = getServerObject();
    ASSERT(url);

    const char *p = strchr(url, '?');
    if (p) {
      m_url = strdup(p + 1);
    } else {
      m_url = strdup("");
    }

    parseQuery(m_url, m_getParams);
  }
}

void Transport::parsePostParams() {
  FiberWriteLock lock(this);

  if (!m_postDataParsed) {
    ASSERT(m_postData == NULL);
    int size;
    const char *data = (const char *)getPostData(size);
    if (data && *data && size) {
      // Post data may be binary, but if parsePostParams() is called, it is
      // correct to handle it as a null-terminated string
      m_postData = strdup(data);
      parseQuery(m_postData, m_postParams);
    }
    m_postDataParsed = true;
  }
}

bool Transport::paramExists(const char *name, Method method /* = GET */) {
  ASSERT(name && *name);
  FiberReadLock lock(this);

  if (method == GET || method == AUTO) {
    if (m_url == NULL) {
      FiberReadLockViolator unlock(this);
      parseGetParams();
    }
    if (m_getParams.find(name) != m_getParams.end()) {
      return true;
    }
  }

  if (method == POST || method == AUTO) {
    if (!m_postDataParsed) {
      FiberReadLockViolator unlock(this);
      parsePostParams();
    }
    if (m_postParams.find(name) != m_postParams.end()) {
      return true;
    }
  }

  return false;
}

std::string Transport::getParam(const char *name,  Method method /* = GET */) {
  ASSERT(name && *name);
  FiberReadLock lock(this);

  if (method == GET || method == AUTO) {
    if (m_url == NULL) {
      FiberReadLockViolator unlock(this);
      parseGetParams();
    }
    ParamMap::const_iterator iter = m_getParams.find(name);
    if (iter != m_getParams.end()) {
      return iter->second[0];
    }
  }

  if (method == POST || method == AUTO) {
    if (!m_postDataParsed) {
      FiberReadLockViolator unlock(this);
      parsePostParams();
    }
    ParamMap::const_iterator iter = m_postParams.find(name);
    if (iter != m_postParams.end()) {
      return iter->second[0];
    }
  }

  return "";
}

int Transport::getIntParam(const char *name, Method method /* = GET */) {
  std::string param = getParam(name, method);
  if (param.empty()) {
    return 0;
  }
  return atoi(param.c_str());
}

long long Transport::getInt64Param(const char *name,
                                   Method method /* = GET */) {
  std::string param = getParam(name, method);
  if (param.empty()) {
    return 0;
  }
  return atoll(param.c_str());
}

void Transport::getArrayParam(const char *name,
                              std::vector<std::string> &values,
                              Method method /* = GET */) {
  FiberReadLock lock(this);

  if (method == GET || method == AUTO) {
    if (m_url == NULL) {
      FiberReadLockViolator unlock(this);
      parseGetParams();
    }
    ParamMap::const_iterator iter = m_getParams.find(name);
    if (iter != m_getParams.end()) {
      const vector<const char *> &params = iter->second;
      values.insert(values.end(), params.begin(), params.end());
    }
  }

  if (method == POST || method == AUTO) {
    if (!m_postDataParsed) {
      FiberReadLockViolator unlock(this);
      parsePostParams();
    }
    ParamMap::const_iterator iter = m_postParams.find(name);
    if (iter != m_postParams.end()) {
      const vector<const char *> &params = iter->second;
      values.insert(values.end(), params.begin(), params.end());
    }
  }
}

void Transport::getSplitParam(const char *name,
                              std::vector<std::string> &values,
                              char delimiter, Method method /* = GET */) {
  std::string param = getParam(name, method);
  if (!param.empty()) {
    Util::split(delimiter, param.c_str(), values);
  }
}

///////////////////////////////////////////////////////////////////////////////
// headers

bool Transport::splitHeader(CStrRef header, String &name, const char *&value) {
  int pos = header.find(':');

  if (pos != String::npos) {
    name = header.substr(0, pos);
    value = header.data() + pos;

    do {
      value++;
    } while (*value == ' ');

    return true;
  }

  // header("HTTP/1.0 404 Not Found");
  // header("HTTP/1.0 404");
  if (strncasecmp(header.data(), "http/", 5) == 0) {
    int pos1 = header.find(' ');
    if (pos1 != String::npos) {
      int pos2 = header.find(' ', pos1 + 1);
      if (pos2 == String::npos) pos2 = header.size();
      if (pos2 - pos1 > 1) {
        setResponse(atoi(header.data() + pos1),
                    getResponseInfo().empty() ? "splitHeader"
                                              : getResponseInfo().c_str()
                   );
        return false;
      }
    }
  }

  throw InvalidArgumentException("header", header.c_str());
}

void Transport::addHeaderNoLock(const char *name, const char *value) {
  ASSERT(name && *name);
  ASSERT(value);

  if (!m_firstHeaderSet) {
    m_firstHeaderSet = true;
    m_firstHeaderFile = FrameInjection::GetContainingFileName(true).data();
    m_firstHeaderLine = FrameInjection::GetLine(true);
  }

  string svalue = value;
  Util::replaceAll(svalue, "\n", "");
  m_responseHeaders[name].push_back(svalue);

  if (strcasecmp(name, "Location") == 0 && m_responseCode != 201 &&
      !(m_responseCode >= 300 && m_responseCode <=307)) {
    /* Zend seems to set 303 on a post with HTTP version > 1.0 in the code but
     * in our testing we can only get it to give 302.
    Method m = getMethod();
    if (m != GET && m != HEAD) {
      setResponse(303);
    } else {
      setResponse(302);
    }
    */
    setResponse(302, "forced.302");
  }
}

void Transport::addHeader(const char *name, const char *value) {
  ASSERT(name && *name);
  ASSERT(value);
  FiberWriteLock lock(this);
  addHeaderNoLock(name, value);
}

void Transport::addHeader(CStrRef header) {
  String name;
  const char *value;
  if (splitHeader(header, name, value)) {
    addHeader(name.data(), value);
  }
}

void Transport::replaceHeader(const char *name, const char *value) {
  ASSERT(name && *name);
  ASSERT(value);
  FiberWriteLock lock(this);
  m_responseHeaders[name].clear();
  addHeaderNoLock(name, value);
}

void Transport::replaceHeader(CStrRef header) {
  String name;
  const char *value;
  if (splitHeader(header, name, value)) {
    replaceHeader(name.data(), value);
  }
}

void Transport::removeHeader(const char *name) {
  if (name && *name) {
    FiberWriteLock lock(this);
    m_responseHeaders.erase(name);
    if (strcasecmp(name, "Set-Cookie") == 0) {
      m_responseCookies.clear();
    }
  }
}

void Transport::removeAllHeaders() {
  FiberWriteLock lock(this);
  m_responseHeaders.clear();
  m_responseCookies.clear();
}

void Transport::getResponseHeaders(HeaderMap &headers) {
  FiberReadLock lock(this);
  headers = m_responseHeaders;

  vector<string> &cookies = headers["Set-Cookie"];
  for (CookieMap::const_iterator iter = m_responseCookies.begin();
       iter != m_responseCookies.end(); ++iter) {
    cookies.push_back(iter->second);
  }
}

bool Transport::acceptEncoding(const char *encoding) {
  ASSERT(encoding && *encoding);
  string header = getHeader("Accept-Encoding");

  // This is testing a substring than a word match, but in practice, this
  // always works.
  return header.find(encoding) != string::npos;
}

bool Transport::cookieExists(const char *name) {
  ASSERT(name && *name);
  string header = getHeader("Cookie");
  int len = strlen(name);
  bool hasValue = (strchr(name, '=') != NULL);
  for (size_t pos = header.find(name); pos != string::npos;
       pos = header.find(name, pos)) {
    if (pos == 0 || isspace(header[pos-1]) || header[pos-1] == ';') {
      pos += len;
      if (hasValue) {
        if (pos == header.size() || header[pos] == ';') return true;
      } else {
        if (header[pos] == '=') return true;
      }
    }
  }
  return false;
}

bool Transport::decideCompression() {
  ASSERT(m_compressionDecision == NotDecidedYet);

  if (!RuntimeOption::ForceCompressionURL.empty() &&
      getCommand() == RuntimeOption::ForceCompressionURL) {
    m_compressionDecision = HasToCompress;
    return true;
  }

  if (acceptEncoding("gzip") ||
      (!RuntimeOption::ForceCompressionCookie.empty() &&
       cookieExists(RuntimeOption::ForceCompressionCookie.c_str())) ||
      (!RuntimeOption::ForceCompressionParam.empty() &&
       paramExists(RuntimeOption::ForceCompressionParam.c_str()))) {
    m_compressionDecision = ShouldCompress;
    return true;
  }

  m_compressionDecision = ShouldNotCompress;
  return false;
}

std::string Transport::getHTTPVersion() const {
  return "1.1";
}

void Transport::setMimeType(CStrRef mimeType) {
  FiberWriteLock lock(this);
  m_mimeType = mimeType.data();
}

String Transport::getMimeType() {
  FiberReadLock lock(this);
  return String(m_mimeType);
}

///////////////////////////////////////////////////////////////////////////////
// cookies

bool Transport::setCookie(CStrRef name, CStrRef value, int64 expire /* = 0 */,
                          CStrRef path /* = "" */, CStrRef domain /* = "" */,
                          bool secure /* = false */,
                          bool httponly /* = false */,
                          bool encode_url /* = true */) {
  if (!name.empty() && strpbrk(name.data(), "=,; \t\r\n\013\014")) {
    Logger::Warning("Cookie names can not contain any of the following "
                    "'=,; \\t\\r\\n\\013\\014'");
    return false;
  }

  if (!encode_url &&
      !value.empty() && strpbrk(value.data(), ",; \t\r\n\013\014")) {
    Logger::Warning("Cookie values can not contain any of the following "
                    "',; \\t\\r\\n\\013\\014'");
    return false;
  }

  char *encoded_value = NULL;
  int len = 0;
  if (!value.empty() && encode_url) {
    int encoded_value_len = value.size();
    encoded_value = url_encode(value.data(), encoded_value_len);
    len += encoded_value_len;
  } else if (!value.empty()) {
    encoded_value = strdup(value.data());
    len += value.size();
  }
  len += path.size();
  len += domain.size();

  std::string cookie;
  cookie.reserve(len + 100);
  if (value.empty()) {
    /*
     * MSIE doesn't delete a cookie when you set it to a null value
     * so in order to force cookies to be deleted, even on MSIE, we
     * pick an expiry date 1 year and 1 second in the past
     */
    String sdt = DateTime(time(NULL) - 31536001, true)
      .toString(DateTime::Cookie);
    cookie += name.data();
    cookie += "=deleted; expires=";
    cookie += sdt.data();
  } else {
    cookie += name.data();
    cookie += "=";
    cookie += encoded_value ? encoded_value : "";
    if (expire > 0) {
      if (expire > 253402300799LL) {
        raise_warning("Expiry date cannot have a year greater then 9999");
        return false;
      }
      cookie += "; expires=";
      String sdt = DateTime(expire, true).toString(DateTime::Cookie);
      cookie += sdt.data();
    }
  }

  if (encoded_value) {
    free(encoded_value);
  }

  if (!path.empty()) {
    cookie += "; path=";
    cookie += path.data();
  }
  if (!domain.empty()) {
    cookie += "; domain=";
    cookie += domain.data();
  }
  if (secure) {
    cookie += "; secure";
  }
  if (httponly) {
    cookie += "; httponly";
  }

  FiberWriteLock lock(this);
  m_responseCookies[name.data()] = cookie;
  return true;
}

///////////////////////////////////////////////////////////////////////////////

void Transport::prepareHeaders(bool compressed, const void *data, int size) {
  for (HeaderMap::const_iterator iter = m_responseHeaders.begin();
       iter != m_responseHeaders.end(); ++iter) {
    const vector<string> &values = iter->second;
    for (unsigned int i = 0; i < values.size(); i++) {
      addHeaderImpl(iter->first.c_str(), values[i].c_str());
    }
  }

  for (CookieMap::const_iterator iter = m_responseCookies.begin();
       iter != m_responseCookies.end(); ++iter) {
    addHeaderImpl("Set-Cookie", iter->second.c_str());
  }

  if (compressed) {
    addHeaderImpl("Content-Encoding", "gzip");
    removeHeaderImpl("Content-Length");
    if (m_responseHeaders.find("Content-MD5") != m_responseHeaders.end()) {
      String response((const char *)data, size, AttachLiteral);
      removeHeaderImpl("Content-MD5");
      addHeaderImpl("Content-MD5", StringUtil::Base64Encode(
                      StringUtil::MD5(response, true)).c_str());
    }
  }

  if (m_responseHeaders.find("Content-Type") == m_responseHeaders.end() &&
      m_responseCode != 304) {
    string contentType = "text/html; charset="
                         + RuntimeOption::DefaultCharsetName;
    addHeaderImpl("Content-Type", contentType.c_str());
  }

  if (RuntimeOption::ExposeHPHP) {
    addHeaderImpl("X-Powered-By", "HPHP");
  }

  if (RuntimeOption::ExposeXFBServer &&
      m_responseHeaders.find("X-FB-Server") == m_responseHeaders.end()) {
    addHeaderImpl("X-FB-Server", String(RuntimeOption::ServerPrimaryIP));
  }

  // shutting down servers, so need to terminate all Keep-Alive connections
  if (!RuntimeOption::EnableKeepAlive || isServerStopping()) {
    addHeaderImpl("Connection", "close");
    removeHeaderImpl("Keep-Alive");

    // so lower level transports can ignore incoming "Connection: keep-alive"
    removeRequestHeaderImpl("Connection");
  }
}

String Transport::prepareResponse(const void *data, int size, bool &compressed,
                                  bool last) {
  String response((const char *)data, size, AttachLiteral);

  // we don't use chunk encoding to send anything pre-compressed
  ASSERT(!compressed || !m_chunkedEncoding);

  if (m_compressionDecision == NotDecidedYet) {
    decideCompression();
  }
  if (compressed || !isCompressionEnabled() ||
      m_compressionDecision == ShouldNotCompress) {
    return response;
  }

  // There isn't that much need to gzip response, when it can fit into one
  // Ethernet packet (1500 bytes), unless we are doing chunked encoding,
  // where we don't really know if next chunk will benefit from compresseion.
  if (m_chunkedEncoding || size > 1000 ||
      m_compressionDecision == HasToCompress) {
    if (m_compressor == NULL) {
      m_compressor = new StreamCompressor(RuntimeOption::GzipCompressionLevel,
                                          CODING_GZIP, true);
    }
    int len = size;
    char *compressedData =
      m_compressor->compress((const char*)data, len, last);
    if (compressedData) {
      String deleter(compressedData, len, AttachString);
      if (m_chunkedEncoding || len < size ||
          m_compressionDecision == HasToCompress) {
        response = deleter;
        compressed = true;
      }
    } else {
      Logger::Error("Unable to compress response: level=%d len=%d",
                    RuntimeOption::GzipCompressionLevel, len);
    }
  }

  return response;
}

void Transport::sendRawLocked(void *data, int size, int code /* = 200 */,
                              bool compressed /* = false */,
                              bool chunked /* = false */,
                              const char *codeInfo /* = "" */
                              ) {
  if (!compressed && RuntimeOption::ForceChunkedEncoding) {
    chunked = true;
  }
  if (m_chunkedEncoding) {
    chunked = true;
    ASSERT(!compressed);
  } else if (chunked) {
    m_chunkedEncoding = true;
    ASSERT(!compressed);
  }

  // I don't think there is any need to send an empty chunk, other than sending
  // out headers earlier, which seems to be a useless feature.
  if (chunked && size == 0) {
    return;
  }

  // compression handling
  ServerStatsHelper ssh("send");
  String response = prepareResponse(data, size, compressed, !chunked);

  if (m_responseCode < 0) {
    m_responseCode = code;
    m_responseCodeInfo = codeInfo ? codeInfo: "";
  }

  // HTTP header handling
  if (!m_headerSent) {
    prepareHeaders(compressed, data, size);
    m_headerSent = true;
  }

  m_responseSize += response.size();
  ServerStats::SetThreadMode(ServerStats::Writing);
  sendImpl(response.data(), response.size(), m_responseCode, chunked);
  ServerStats::SetThreadMode(ServerStats::Processing);

  ServerStats::LogBytes(size);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableWebStats) {
    ServerStats::Log("network.uncompressed", size);
    ServerStats::Log("network.compressed", response.size());
  }
}

void Transport::sendRaw(void *data, int size, int code /* = 200 */,
                        bool compressed /* = false */,
                        bool chunked /* = false */,
                        const char *codeInfo /* = "" */
                        ) {
  FiberWriteLock lock(this);
  sendRawLocked(data, size, code, compressed, chunked, codeInfo);
}

void Transport::onSendEnd() {
  FiberWriteLock lock(this);
  if (m_compressor && m_chunkedEncoding) {
    bool compressed = false;
    String response = prepareResponse("", 0, compressed, true);
    sendImpl(response.data(), response.size(), m_responseCode, true);
  }
  onSendEndImpl();
}

void Transport::redirect(const char *location, int code /* = 302 */,
                         const char *info) {
  FiberWriteLock lock(this);
  addHeaderImpl("Location", location);
  setResponse(code, info);
  sendStringLocked(location, code);
}

void Transport::onFlushProgress(int writtenSize, int64 delayUs) {
  m_responseSentSize += writtenSize;
  m_flushTimeUs += delayUs;
}

///////////////////////////////////////////////////////////////////////////////
// support rfc1867

bool Transport::isUploadedFile(CStrRef filename) {
  return is_uploaded_file(filename.c_str());
}

// Move a file if and only if it was created by an upload
bool Transport::moveUploadedFileHelper(CStrRef filename, CStrRef destination) {
  // Do access check.
  String dest = File::TranslatePath(destination);
  if (Util::rename(filename.c_str(), dest.c_str()) < 0) {
    Logger::Error("Unable to move uploaded file %s to %s: %s.",
                  filename.c_str(), dest.c_str(),
                  Util::safe_strerror(errno).c_str());
    return false;
  }
  Logger::Verbose("Successfully moved uploaded file %s to %s.",
                  filename.c_str(), dest.c_str());
  return true;
}

bool Transport::moveUploadedFile(CStrRef filename, CStrRef destination) {
  if (!is_uploaded_file(filename.c_str())) {
    Logger::Error("%s is not an uploaded file.", filename.c_str());
    return false;
  }
  return moveUploadedFileHelper(filename, destination);
}

///////////////////////////////////////////////////////////////////////////////
// IDebuggable

const char *Transport::getThreadTypeName() const {
  switch (m_threadType) {
    case RequestThread: return "Web Request";
    case PageletThread: return "Pagelet Thread";
    case XboxThread:    return "Xbox Thread";
    case RpcThread:     return "RPC Thread";
  }
  return "(unknown)";
}

void Transport::debuggerInfo(InfoVec &info) {
  Add(info, "Thread Type", getThreadTypeName());
  Add(info, "URL",         getCommand());
  Add(info, "HTTP",        getHTTPVersion());
  Add(info, "Method",      getMethodName());
  if (getMethod() == Transport::POST) {
    int size; getPostData(size);
    Add(info, "Post Data", FormatSize(size));
  }
}

///////////////////////////////////////////////////////////////////////////////
}

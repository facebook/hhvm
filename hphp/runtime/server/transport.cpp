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

#include "hphp/runtime/server/transport.h"
#include "hphp/runtime/server/server.h"
#include "hphp/runtime/server/upload.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/url.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/server/access-log.h"
#include "hphp/runtime/ext/ext_openssl.h"
#include "hphp/util/compression.h"
#include "hphp/util/util.h"
#include "hphp/util/service-data.h"
#include "hphp/util/logger.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/timer.h"
#include "hphp/runtime/base/hardware-counter.h"
#include "folly/String.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static const char HTTP_RESPONSE_STATS_PREFIX[] = "http_response_";

Transport::Transport()
  : m_instructions(0), m_sleepTime(0), m_usleepTime(0),
    m_nsleepTimeS(0), m_nsleepTimeN(0), m_url(nullptr),
    m_postData(nullptr), m_postDataParsed(false),
    m_chunkedEncoding(false), m_headerSent(false),
    m_headerCallback(uninit_null()), m_headerCallbackDone(false),
    m_responseCode(-1), m_firstHeaderSet(false), m_firstHeaderLine(0),
    m_responseSize(0), m_responseTotalSize(0), m_responseSentSize(0),
    m_flushTimeUs(0), m_sendContentType(true),
    m_compression(true), m_compressor(nullptr), m_isSSL(false),
    m_compressionDecision(CompressionDecision::NotDecidedYet),
    m_threadType(ThreadType::RequestThread) {
  memset(&m_queueTime, 0, sizeof(m_queueTime));
  memset(&m_wallTime, 0, sizeof(m_wallTime));
  memset(&m_cpuTime, 0, sizeof(m_cpuTime));
  m_chunksSentSizes.clear();
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
  m_chunksSentSizes.clear();
}

void Transport::onRequestStart(const timespec &queueTime) {
  m_queueTime = queueTime;
  Timer::GetMonotonicTime(m_wallTime);
#ifdef CLOCK_THREAD_CPUTIME_ID
  gettime(CLOCK_THREAD_CPUTIME_ID, &m_cpuTime);
#endif
  /*
   * The hardware counter is only 48 bits, so reset this at the beginning
   * of every request to make sure we don't overflow.
   */
  HardwareCounter::Reset();
  m_instructions = HardwareCounter::GetInstructionCount();
}

const char *Transport::getMethodName() {
  switch (getMethod()) {
  case Method::GET:  return "GET";
  case Method::HEAD: return "HEAD";
  case Method::POST: {
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
  return URL::getServerObject(url);
}

string Transport::getCommand() {
  const char *url = getServerObject();
  return URL::getCommand(url);
}

///////////////////////////////////////////////////////////////////////////////
// parameters

// copied and re-factored from clearsilver-0.10.5/cgi/cgi.c
void Transport::urlUnescape(char *value) {
  assert(value && *value); // check before calling this function

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
    if (v == nullptr) {
      fv = "";
    } else {
      *v = '\0';
      v++;
      if (*v) urlUnescape(v);
      fv = v;
    }
    if (*k) urlUnescape(k);
    params[k].push_back(fv);
    k = strtok_r(nullptr, "&", &l);
  }
}

void Transport::parseGetParams() {
  if (m_url == nullptr) {
    const char *url = getServerObject();
    assert(url);

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
  if (!m_postDataParsed) {
    assert(m_postData == nullptr);
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

bool Transport::paramExists(const char *name,
                            Method method /* = Method::GET */) {
  assert(name && *name);
  if (method == Method::GET || method == Method::AUTO) {
    if (m_url == nullptr) {
      parseGetParams();
    }
    if (m_getParams.find(name) != m_getParams.end()) {
      return true;
    }
  }

  if (method == Method::POST || method == Method::AUTO) {
    if (!m_postDataParsed) {
      parsePostParams();
    }
    if (m_postParams.find(name) != m_postParams.end()) {
      return true;
    }
  }

  return false;
}

std::string Transport::getParam(const char *name,
                                Method method /* = Method::GET */) {
  assert(name && *name);

  if (method == Method::GET || method == Method::AUTO) {
    if (m_url == nullptr) {
      parseGetParams();
    }
    ParamMap::const_iterator iter = m_getParams.find(name);
    if (iter != m_getParams.end()) {
      return iter->second[0];
    }
  }

  if (method == Method::POST || method == Method::AUTO) {
    if (!m_postDataParsed) {
      parsePostParams();
    }
    ParamMap::const_iterator iter = m_postParams.find(name);
    if (iter != m_postParams.end()) {
      return iter->second[0];
    }
  }

  return "";
}

int Transport::getIntParam(const char *name,
                           Method method /* = Method::GET */) {
  std::string param = getParam(name, method);
  if (param.empty()) {
    return 0;
  }
  return atoi(param.c_str());
}

long long Transport::getInt64Param(const char *name,
                                   Method method /* = Method::GET */) {
  std::string param = getParam(name, method);
  if (param.empty()) {
    return 0;
  }
  return atoll(param.c_str());
}

void Transport::getArrayParam(const char *name,
                              std::vector<std::string> &values,
                              Method method /* = GET */) {
  if (method == Method::GET || method == Method::AUTO) {
    if (m_url == nullptr) {
      parseGetParams();
    }
    ParamMap::const_iterator iter = m_getParams.find(name);
    if (iter != m_getParams.end()) {
      const vector<const char *> &params = iter->second;
      values.insert(values.end(), params.begin(), params.end());
    }
  }

  if (method == Method::POST || method == Method::AUTO) {
    if (!m_postDataParsed) {
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
                              char delimiter,
                              Method method /* = Method::GET */) {
  std::string param = getParam(name, method);
  if (!param.empty()) {
    Util::split(delimiter, param.c_str(), values);
  }
}

///////////////////////////////////////////////////////////////////////////////
// headers

bool Transport::splitHeader(const String& header, String &name, const char *&value) {
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
  assert(name && *name);
  assert(value);

  if (!m_firstHeaderSet) {
    m_firstHeaderSet = true;
    m_firstHeaderFile = g_vmContext->getContainingFileName().data();
    m_firstHeaderLine = g_vmContext->getLine();
  }

  string svalue = value;
  Util::replaceAll(svalue, "\n", "");
  m_responseHeaders[name].push_back(svalue);

  if (strcasecmp(name, "Location") == 0 && m_responseCode != 201 &&
      !(m_responseCode >= 300 && m_responseCode <=307)) {
    /* Zend seems to set 303 on a post with HTTP version > 1.0 in the code but
     * in our testing we can only get it to give 302.
    Method m = getMethod();
    if (m != Method::GET && m != Method::HEAD) {
      setResponse(303);
    } else {
      setResponse(302);
    }
    */
    setResponse(302, "forced.302");
  }
}

void Transport::addHeader(const char *name, const char *value) {
  assert(name && *name);
  assert(value);
  addHeaderNoLock(name, value);
}

void Transport::addHeader(const String& header) {
  String name;
  const char *value;
  if (splitHeader(header, name, value)) {
    addHeader(name.data(), value);
  }
}

void Transport::replaceHeader(const char *name, const char *value) {
  assert(name && *name);
  assert(value);
  m_responseHeaders[name].clear();
  addHeaderNoLock(name, value);
}

void Transport::replaceHeader(const String& header) {
  String name;
  const char *value;
  if (splitHeader(header, name, value)) {
    replaceHeader(name.data(), value);
  }
}

void Transport::removeHeader(const char *name) {
  if (name && *name) {
    m_responseHeaders.erase(name);
    if (strcasecmp(name, "Set-Cookie") == 0) {
      m_responseCookies.clear();
    }
  }
}

void Transport::removeAllHeaders() {
  m_responseHeaders.clear();
  m_responseCookies.clear();
}

void Transport::getResponseHeaders(HeaderMap &headers) {
  headers = m_responseHeaders;

  vector<string> &cookies = headers["Set-Cookie"];
  for (CookieMap::const_iterator iter = m_responseCookies.begin();
       iter != m_responseCookies.end(); ++iter) {
    cookies.push_back(iter->second);
  }
}

bool Transport::acceptEncoding(const char *encoding) {
  assert(encoding && *encoding);
  string header = getHeader("Accept-Encoding");

  // This is testing a substring than a word match, but in practice, this
  // always works.
  return header.find(encoding) != string::npos;
}

bool Transport::cookieExists(const char *name) {
  assert(name && *name);
  string header = getHeader("Cookie");
  int len = strlen(name);
  bool hasValue = (strchr(name, '=') != nullptr);
  for (size_t pos = header.find(name); pos != string::npos;
       pos = header.find(name, pos + 1)) {
    if (pos == 0 || isspace(header[pos-1]) || header[pos-1] == ';') {
      pos += len;
      if (hasValue) {
        if (pos == header.size() || header[pos] == ';') return true;
      } else {
        if (pos < header.size() && header[pos] == '=') return true;
      }
    }
  }
  return false;
}

string Transport::getCookie(const string &name) {
  assert(!name.empty());
  string header = getHeader("Cookie");
  for (size_t pos = header.find(name); pos != string::npos;
       pos = header.find(name, pos + 1)) {
    if (pos == 0 || isspace(header[pos-1]) || header[pos-1] == ';') {
      pos += name.size();
      if (pos < header.size() && header[pos] == '=') {
        size_t end = header.find(';', pos + 1);
        if (end != string::npos) end -= pos + 1;
        return header.substr(pos + 1, end);
      }
    }
  }
  return "";
}

bool Transport::decideCompression() {
  assert(m_compressionDecision == CompressionDecision::NotDecidedYet);

  if (!RuntimeOption::ForceCompressionURL.empty() &&
      getCommand() == RuntimeOption::ForceCompressionURL) {
    m_compressionDecision = CompressionDecision::HasTo;
    return true;
  }

  if (acceptEncoding("gzip") ||
      (!RuntimeOption::ForceCompressionCookie.empty() &&
       cookieExists(RuntimeOption::ForceCompressionCookie.c_str())) ||
      (!RuntimeOption::ForceCompressionParam.empty() &&
       paramExists(RuntimeOption::ForceCompressionParam.c_str()))) {
    m_compressionDecision = CompressionDecision::Should;
    return true;
  }

  m_compressionDecision = CompressionDecision::ShouldNot;
  return false;
}

std::string Transport::getHTTPVersion() const {
  return "1.1";
}

int Transport::getRequestSize() const {
  return 0;
}

void Transport::setMimeType(const String& mimeType) {
  m_mimeType = mimeType.data();
}

String Transport::getMimeType() {
  return String(m_mimeType);
}

///////////////////////////////////////////////////////////////////////////////
// cookies

bool Transport::setCookie(const String& name, const String& value, int64_t expire /* = 0 */,
                          const String& path /* = "" */, const String& domain /* = "" */,
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

  char *encoded_value = nullptr;
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
     * pick an expiry date in the past
     */
    String sdt = DateTime(1, true).toString(DateTime::DateFormat::Cookie);
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
      String sdt =
        DateTime(expire, true).toString(DateTime::DateFormat::Cookie);
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

  m_responseCookies[name.data()] = cookie;
  return true;
}

///////////////////////////////////////////////////////////////////////////////

void Transport::prepareHeaders(bool compressed, bool chunked,
    const String &response, const String& orig_response) {
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
    // Remove the Content-MD5 header coming from PHP if we compressed the data,
    // as the checksum is going to be invalid.
    auto it = m_responseHeaders.find("Content-MD5");
    if (it != m_responseHeaders.end()) {
      removeHeaderImpl("Content-MD5");
      // Re-add it back unless this is a chunked response. We'd have to buffer
      // the response completely to compute the MD5, which defeats the purpose
      // of chunking.
      if (chunked) {
        raise_warning("Cannot use chunked HTTP response and Content-MD5 header "
          "at the same time. Dropping Content-MD5.");
      } else {
        string cur_md5 = it->second[0];
        String expected_md5 = StringUtil::Base64Encode(StringUtil::MD5(
          orig_response, true));
        // Can never trust these PHP people...
        if (expected_md5.c_str() != cur_md5) {
          raise_warning("Content-MD5 mismatch. Expected: %s, Got: %s",
            expected_md5.c_str(), cur_md5.c_str());
        }
        addHeaderImpl("Content-MD5", StringUtil::Base64Encode(StringUtil::MD5(
          response, true)).c_str());
      }
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

  if ((RuntimeOption::ExposeXFBServer || RuntimeOption::ExposeXFBDebug) &&
      !RuntimeOption::XFBDebugSSLKey.empty() &&
      m_responseHeaders.find("X-FB-Debug") == m_responseHeaders.end()) {
    String ip = RuntimeOption::ServerPrimaryIP;
    String key = RuntimeOption::XFBDebugSSLKey;
    String cipher("AES-256-CBC");
    int iv_len = f_openssl_cipher_iv_length(cipher).toInt32();
    String iv = f_openssl_random_pseudo_bytes(iv_len);
    String encrypted =
      f_openssl_encrypt(ip, cipher, key, k_OPENSSL_RAW_DATA, iv);
    String output = StringUtil::Base64Encode(iv + encrypted);
    if (debug) {
      String decrypted =
        f_openssl_decrypt(encrypted, cipher, key, k_OPENSSL_RAW_DATA, iv);
      assert(decrypted->same(ip.get()));
    }
    addHeaderImpl("X-FB-Debug", output.c_str());
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
  String response((const char *)data, size, CopyString);

  // we don't use chunk encoding to send anything pre-compressed
  assert(!compressed || !m_chunkedEncoding);

  if (m_compressionDecision == CompressionDecision::NotDecidedYet) {
    decideCompression();
  }
  if (compressed || !isCompressionEnabled() ||
      m_compressionDecision == CompressionDecision::ShouldNot) {
    return response;
  }

  // There isn't that much need to gzip response, when it can fit into one
  // Ethernet packet (1500 bytes), unless we are doing chunked encoding,
  // where we don't really know if next chunk will benefit from compresseion.
  if (m_chunkedEncoding || size > 1000 ||
      m_compressionDecision == CompressionDecision::HasTo) {
    if (m_compressor == nullptr) {
      m_compressor = new StreamCompressor(RuntimeOption::GzipCompressionLevel,
                                          CODING_GZIP, true);
    }
    int len = size;
    char *compressedData =
      m_compressor->compress((const char*)data, len, last);
    if (compressedData) {
      String deleter(compressedData, len, AttachString);
      if (m_chunkedEncoding || len < size ||
          m_compressionDecision == CompressionDecision::HasTo) {
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

bool Transport::setHeaderCallback(CVarRef callback) {
  if (m_headerCallback.toBoolean()) {
    // return false if a callback has already been set.
    return false;
  }
  m_headerCallback = callback;
  return true;
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
    assert(!compressed);
  } else if (chunked) {
    m_chunkedEncoding = true;
    assert(!compressed);
  }

  // I don't think there is any need to send an empty chunk, other than sending
  // out headers earlier, which seems to be a useless feature.
  if (chunked && size == 0) {
    return;
  }

  if (!m_headerCallbackDone && !m_headerCallback.isNull()) {
    // We could use m_headerSent here, however it seems we can still
    // end up in an infinite loop when:
    // m_headerCallback calls flush()
    // flush() triggers php's recursion guard
    // the recursion guard calls back into m_headerCallback
    m_headerCallbackDone = true;
    vm_call_user_func(m_headerCallback, init_null_variant);
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
    String orig_response((const char *)data, size, CopyString);
    prepareHeaders(compressed, chunked, response, orig_response);
    m_headerSent = true;
  }

  m_responseSize += response.size();
  ServerStats::SetThreadMode(ServerStats::ThreadMode::Writing);
  sendImpl(response.data(), response.size(), m_responseCode, chunked);
  ServerStats::SetThreadMode(ServerStats::ThreadMode::Processing);

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
  sendRawLocked(data, size, code, compressed, chunked, codeInfo);
}

void Transport::onSendEnd() {
  if (m_compressor && m_chunkedEncoding) {
    bool compressed = false;
    String response = prepareResponse("", 0, compressed, true);
    sendImpl(response.data(), response.size(), m_responseCode, true);
  }
  auto httpResponseStats = ServiceData::createTimeseries(
    folly::to<string>(HTTP_RESPONSE_STATS_PREFIX, getResponseCode()),
    {ServiceData::StatsType::SUM});
  httpResponseStats->addValue(1);
  onSendEndImpl();
}

void Transport::redirect(const char *location, int code /* = 302 */,
                         const char *info) {
  addHeaderImpl("Location", location);
  setResponse(code, info);
  sendStringLocked("Moved", code);
}

void Transport::onFlushProgress(int writtenSize, int64_t delayUs) {
  m_responseSentSize += writtenSize;
  m_flushTimeUs += delayUs;
  m_chunksSentSizes.push_back(writtenSize);
}

void Transport::onChunkedProgress(int writtenSize) {
  m_responseSentSize += writtenSize;
  m_chunksSentSizes.push_back(writtenSize);
}

void Transport::getChunkSentSizes(Array &ret) {
  for (unsigned int i = 0; i < m_chunksSentSizes.size(); i++) {
    ret.append(m_chunksSentSizes[i]);
  }
}

int Transport::getLastChunkSentSize() {
  size_t size = m_chunksSentSizes.size();
  return size == 0 ? 0 : m_chunksSentSizes.back();
}

///////////////////////////////////////////////////////////////////////////////
// support rfc1867

bool Transport::isUploadedFile(const String& filename) {
  return is_uploaded_file(filename.c_str());
}

// Move a file if and only if it was created by an upload
bool Transport::moveUploadedFileHelper(const String& filename, const String& destination) {
  // Do access check.
  String dest = File::TranslatePath(destination);
  if (Util::rename(filename.c_str(), dest.c_str()) < 0) {
    Logger::Error("Unable to move uploaded file %s to %s: %s.",
                  filename.c_str(), dest.c_str(),
                  folly::errnoStr(errno).c_str());
    return false;
  }
  Logger::Verbose("Successfully moved uploaded file %s to %s.",
                  filename.c_str(), dest.c_str());
  return true;
}

bool Transport::moveUploadedFile(const String& filename, const String& destination) {
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
    case ThreadType::RequestThread: return "Web Request";
    case ThreadType::PageletThread: return "Pagelet Thread";
    case ThreadType::XboxThread:    return "Xbox Thread";
    case ThreadType::RpcThread:     return "RPC Thread";
  }
  return "(unknown)";
}

void Transport::debuggerInfo(InfoVec &info) {
  Add(info, "Thread Type", getThreadTypeName());
  Add(info, "URL",         getCommand());
  Add(info, "HTTP",        getHTTPVersion());
  Add(info, "Method",      getMethodName());
  if (getMethod() == Method::POST) {
    int size; getPostData(size);
    Add(info, "Post Data", FormatSize(size));
  }
}

///////////////////////////////////////////////////////////////////////////////
}

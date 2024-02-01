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

#include "hphp/runtime/server/transport.h"

#include "hphp/runtime/server/server.h"
#include "hphp/runtime/server/upload.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/server/stream-transport.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/url.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/server/access-log.h"
#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/server/compression.h"
#include "hphp/runtime/ext/openssl/ext_openssl.h"
#include "hphp/runtime/ext/string/ext_string.h"

#include "hphp/util/compatibility.h"
#include "hphp/util/hardware-counter.h"
#include "hphp/util/logger.h"
#include "hphp/util/service-data.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/text-util.h"
#include "hphp/util/timer.h"

#include <folly/Random.h>
#include <folly/String.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static const char HTTP_RESPONSE_STATS_PREFIX[] = "http_response_";
const StaticString s_defaultCharset("default_charset");

Transport::Transport() {}

Transport::~Transport() {
  if (m_url) {
    free(m_url);
  }
  if (m_postData) {
    free(m_postData);
  }
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

  auto const rate = RuntimeOption::EvalTraceServerRequestRate;
  if (rate && folly::Random::rand32(rate) == 0) forceInitRequestTrace();
}

const char *Transport::getMethodName() {
  switch (getMethod()) {
  case Method::GET:  return "GET";
  case Method::HEAD: return "HEAD";
  case Method::POST: {
    const char *m = getExtendedMethod();
    return m ? m : "POST";
  }
  case Method::Unknown: {
    // m should really never be nullptr. "Unknown" is an actionable log line
    // since it indicates a missing HTTP method (since it is not capitalized).
    const char *m = getExtendedMethod();
    return m ? m : "Unknown";
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

std::string Transport::getCommand() {
  const char *url = getServerObject();
  return URL::getCommand(url);
}

///////////////////////////////////////////////////////////////////////////////
// parameters

// copied and re-factored from clearsilver-0.10.5/cgi/cgi.c
void Transport::urlUnescape(char *value) {
  assertx(value && *value); // check before calling this function

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
    assertx(url);

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
    assertx(m_postData == nullptr);
    size_t size;
    const char *data = (const char *)getPostData(size);
    if (data && *data && size) {
      // Post data may be binary, but if parsePostParams() is called, any
      // wellformed data cannot have embedded NULs. If it does, we simply
      // truncate it.
      m_postData = strndup(data, size);
      parseQuery(m_postData, m_postParams);
    }
    m_postDataParsed = true;
  }
}

std::string Transport::getParam(const char *name,
                                Method method /* = Method::GET */) {
  assertx(name && *name);

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
      auto const& params = iter->second;
      values.insert(values.end(), params.begin(), params.end());
    }
  }

  if (method == Method::POST || method == Method::AUTO) {
    if (!m_postDataParsed) {
      parsePostParams();
    }
    ParamMap::const_iterator iter = m_postParams.find(name);
    if (iter != m_postParams.end()) {
      auto const& params = iter->second;
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
    folly::split(delimiter, param, values);
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
                    header.size() - pos2 > 1 ? header.data() + pos2 : nullptr);
        return false;
      }
    }
  }

  throw ExtendedException(
    "Invalid argument \"header\": [%s]", header.c_str());
}

void Transport::addHeaderNoLock(const char *name, const char *value) {
  assertx(name && *name);
  assertx(value);

  if (!m_firstHeaderSet) {
    m_firstHeaderSet = true;
    m_firstHeaderFile = g_context->getContainingFileName()->data();
    m_firstHeaderLine = g_context->getLine();
  }

  std::string svalue = value;
  replaceAll(svalue, "\n", "");
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
    setResponse(302);
  }
}

void Transport::addHeader(const char *name, const char *value) {
  assertx(name && *name);
  assertx(value);
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
  assertx(name && *name);
  assertx(value);
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

  auto& cookies = headers["Set-Cookie"];
  for (auto& cookie : m_responseCookies) {
    cookies.push_back(cookie.second);
  }
}

void Transport::addToCommaSeparatedHeader(const char* name, const char* value) {
  assertx(name && *name);
  assertx(value);
  const auto it = m_responseHeaders.find(name);
  if (it != m_responseHeaders.end() && !it->second.empty()) {
    it->second[0] = it->second[0] + std::string(", ") + value;
  } else {
    addHeader(name, value);
  }
}

bool Transport::cookieExists(const char *name) {
  assertx(name && *name);
  std::string header = getHeader("Cookie");
  int len = strlen(name);
  bool hasValue = (strchr(name, '=') != nullptr);
  for (size_t pos = header.find(name); pos != std::string::npos;
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

std::string Transport::getCookie(const std::string &name) {
  assertx(!name.empty());
  std::string header = getHeader("Cookie");
  for (size_t pos = header.find(name); pos != std::string::npos;
       pos = header.find(name, pos + 1)) {
    if (pos == 0 || isspace(header[pos-1]) || header[pos-1] == ';') {
      pos += name.size();
      if (pos < header.size() && header[pos] == '=') {
        size_t end = header.find(';', pos + 1);
        if (end != std::string::npos) end -= pos + 1;
        return header.substr(pos + 1, end);
      }
    }
  }
  return "";
}

bool Transport::acceptEncoding(const char *encoding) {
  return acceptsEncoding(this, encoding);
}

void Transport::setResponse(int code, const char *info) {
  m_responseCode = code;
  m_responseCodeInfo = info ? info : HttpProtocol::GetReasonString(code);
}

std::string Transport::getHTTPVersion() const {
  return "1.1";
}

size_t Transport::getRequestSize() const {
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

namespace {

// Make sure cookie names do not contain any illegal characters.
// Throw a fatal exception if one does.
void validateCookieNameString(const String& str) {
  if (!str.empty() && strpbrk(str.data(), "=,; \t\r\n\013\014")) {
    raise_error("Cookie names can not contain any of the following "
                "'=,; \\t\\r\\n\\013\\014'");
  }
}

// Make sure a component (path, value, domain) of a cookie does not
// contain any illegal characters.  Throw a fatal exception if it
// does.
void validateCookieString(const String& str, const char* component) {
  if (!str.empty() && strpbrk(str.data(), ",; \t\r\n\013\014")) {
    raise_error("Cookie %s can not contain any of the following "
                "',; \\t\\r\\n\\013\\014'", component);
  }
}

}

bool Transport::setCookie(const String& name, const String& value, int64_t expire /* = 0 */,
                          const String& path /* = "" */, const String& domain /* = "" */,
                          bool secure /* = false */,
                          bool httponly /* = false */,
                          bool encode_url /* = true */) {
  validateCookieNameString(name);

  if (!encode_url) {
    validateCookieString(value, "values");
  }

  validateCookieString(path, "paths");

  validateCookieString(domain, "domains");

  String encoded_value;
  int len = 0;
  if (!value.empty()) {
    encoded_value = encode_url ? url_encode(value.data(), value.size())
                               : value;
    len += encoded_value.size();
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
    String sdt = req::make<DateTime>(1, true)->
      toString(DateTime::DateFormat::Cookie);
    cookie += name.data();
    cookie += "=deleted; expires=";
    cookie += sdt.data();
    cookie += "; Max-Age=0";
  } else {
    cookie += name.data();
    cookie += "=";
    cookie += encoded_value.isNull() ? "" : encoded_value.data();
    if (expire > 0) {
      if (expire > 253402300799LL) {
        raise_warning("Expiry date cannot have a year greater than 9999");
        return false;
      }
      cookie += "; expires=";
      String sdt = req::make<DateTime>(expire, true)->
        toString(DateTime::DateFormat::Cookie);
      cookie += sdt.data();
      cookie += "; Max-Age=";
      String sdelta = String(expire - time(0));
      cookie += sdelta.data();
    }
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

  // PHP5 does not deduplicate cookies. That behavior is preserved when
  // CookieDeduplicate is not enabled. Otherwise, we will only keep the
  // last cookie for a given name-domain-path triplet.
  String dedup_key = name + "\n" + domain + "\n" + path;

  m_responseCookies[dedup_key.data()] = cookie;

  return true;
}

///////////////////////////////////////////////////////////////////////////////

void Transport::prepareHeaders(bool precompressed, bool chunked,
    const StringHolder &response, const StringHolder& orig_response) {
  for (HeaderMap::const_iterator iter = m_responseHeaders.begin();
       iter != m_responseHeaders.end(); ++iter) {
    const auto& values = iter->second;
    for (unsigned int i = 0; i < values.size(); i++) {
      addHeaderImpl(iter->first.c_str(), values[i].c_str());
    }
  }

  for (auto& cookie : m_responseCookies) {
    addHeaderImpl("Set-Cookie", cookie.second.c_str());
  }

  auto& compressor = getCompressor();

  // should never double-compress
  assertx(!precompressed || !compressor.isCompressed());

  if (precompressed) {
    // pre-compressed content is currently always gzip compressed.
    addHeaderImpl("Content-Encoding", "gzip");
    if (RuntimeOption::ServerAddVaryEncoding) {
      addHeaderImpl("Vary", "Accept-Encoding");
    }
  }

  if (precompressed || compressor.isCompressed()) {
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
        std::string cur_md5 = it->second[0];
        String expected_md5 = StringUtil::Base64Encode(StringUtil::MD5(
          orig_response.data(), orig_response.size(), true));
        // Can never trust these PHP people...
        if (expected_md5.c_str() != cur_md5) {
          raise_warning("Content-MD5 mismatch. Expected: %s, Got: %s",
            expected_md5.c_str(), cur_md5.c_str());
        }
        addHeaderImpl("Content-MD5", StringUtil::Base64Encode(StringUtil::MD5(
          response.data(), response.size(), true)).c_str());
      }
    }
  }

  if (m_responseHeaders.find("Content-Type") == m_responseHeaders.end() &&
      m_responseCode != 304) {
    std::string contentType = "text/html";
    auto const defaultCharset = IniSetting::Get(s_defaultCharset);
    if (defaultCharset != "") {
      contentType += "; charset=" + defaultCharset;
    }
    addHeaderImpl("Content-Type", contentType.c_str());
  }

  if (RuntimeOption::ExposeHPHP) {
    addHeaderImpl("X-Powered-By", (String("HHVM/") + HHVM_VERSION).c_str());
  }

  if ((RuntimeOption::ExposeXFBServer || RuntimeOption::ExposeXFBDebug) &&
      !RuntimeOption::XFBDebugSSLKey.empty() &&
      m_responseHeaders.find("X-FB-Debug") == m_responseHeaders.end()) {
    String ip = this->getServerAddr();
    String key = RuntimeOption::XFBDebugSSLKey;
    String cipher("AES-256-CBC");
    bool crypto_strong = false;
    auto const iv_len = (int)HHVM_FN(openssl_cipher_iv_length)(cipher).toInt64();
    auto const iv = HHVM_FN(openssl_random_pseudo_bytes)(
      iv_len, crypto_strong
    ).toString();
    auto const encrypted = HHVM_FN(openssl_encrypt)(
      ip, cipher, key, k_OPENSSL_RAW_DATA, iv
    ).toString();
    auto const output = StringUtil::Base64Encode(iv + encrypted);
    if (debug) {
      auto const decrypted = HHVM_FN(openssl_decrypt)(
        encrypted, cipher, key, k_OPENSSL_RAW_DATA, iv
      ).toString();
      assertx(decrypted.get()->same(ip.get()));
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

namespace {

void LogException(const char* msg) {
  try {
    throw;
  } catch (Exception& e) {
    Logger::Error("%s: %s", msg, e.getMessage().c_str());
  } catch (std::exception& e) {
    Logger::Error("%s: %s", msg, e.what());
  } catch (Object& e) {
    try {
      Logger::Error("%s: %s", msg, throwable_to_string(e.get()).c_str());
    } catch (...) {
      Logger::Error("%s: (e.toString() failed)", msg);
    }
  } catch (...) {
    Logger::Error("%s: (unknown exception)", msg);
  }
}
} // anonymous namespace

StringHolder Transport::compressResponse(
    const char* data, int size, bool last) {
  StringHolder response(data, size, FreeType::NoFree);
  auto compressedResponse = getCompressor().compressResponse(data, size, last);
  if (compressedResponse.data() != nullptr) {
    response = std::move(compressedResponse);
  }

  return response;
}

ResponseCompressorManager& Transport::getCompressor() {
  if (!m_compressor) {
    m_compressor = std::make_unique<ResponseCompressorManager>(this);
  }
  return *m_compressor;
}

void Transport::enableCompression() {
  getCompressor().enable();
}

void Transport::disableCompression() {
  getCompressor().disable();
}

bool Transport::isCompressionEnabled() {
  return getCompressor().isEnabled();
}

void Transport::sendRaw(const char *data, int size, int code /* = 200 */,
                        bool precompressed /* = false */,
                        bool chunked /* = false */,
                        const char *codeInfo /* = nullptr */
                       ) {
  // There are post-send functions that can run. Any output from them should
  // be ignored as it doesn't make sense to try and send data after the
  // request has ended.
  if (m_sendEnded) {
    return;
  }

  // Note: This API is used when `isStreamTransport()` to report request errors
  // (such as 404) but if we already started sending then ignore this.
  if (m_sendStarted && isStreamTransport()) {
    return;
  }

  if (!precompressed && RuntimeOption::ForceChunkedEncoding) {
    chunked = true;
  }

  // I don't think there is any need to send an empty chunk, other than sending
  // out headers earlier, which seems to be a useless feature.
  if (size == 0 && (chunked || m_chunkedEncoding)) {
    return;
  }

  if (chunked) {
    m_chunkedEncoding = true;
  }
  if (m_chunkedEncoding) {
    assertx(!precompressed);
  }

  sendRawInternal(data, size, code, precompressed, codeInfo);
}

void Transport::sendRawInternal(const char *data, int size,
                                int code /* = 200 */,
                                bool precompressed /* = false */,
                                const char *codeInfo /* = nullptr */
                               ) {

  bool chunked = m_chunkedEncoding;

  if (!g_context->m_headerCallbackDone &&
      !tvIsNull(&g_context->m_headerCallback)) {
    // We could use m_headerSent here, however it seems we can still
    // end up in an infinite loop when:
    // m_headerCallback calls flush()
    // flush() triggers php's recursion guard
    // the recursion guard calls back into m_headerCallback
    g_context->m_headerCallbackDone = true;
    try {
      vm_call_user_func(tvAsVariant(g_context->m_headerCallback),
                        init_null_variant);
    } catch (...) {
      LogException("HeaderCallback");
    }
  }

  // compression handling
  ServerStatsHelper ssh("send");

  if (precompressed) {
    disableCompression();
  }
  StringHolder response = compressResponse(data, size, !chunked);

  if (m_responseCode < 0) {
    setResponse(code, codeInfo);
  }

  // HTTP header handling
  if (!m_headerSent) {
    prepareHeaders(precompressed, chunked, response,
                   StringHolder(data, size, FreeType::NoFree));
    m_headerSent = true;
  }

  m_responseSize += response.size();
  ServerStats::SetThreadMode(ServerStats::ThreadMode::Writing);
  sendImpl(response.data(), response.size(), m_responseCode, chunked, false);
  ServerStats::SetThreadMode(ServerStats::ThreadMode::Processing);

  ServerStats::LogBytes(size);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableWebStats) {
    ServerStats::Log("network.uncompressed", size);
    ServerStats::Log("network.compressed", response.size());
  }
}

void Transport::onSendEnd() {
  if (auto stream = getStreamTransport()) {
    stream->close();
    return;
  }
  bool eomSent = false;
  if (m_chunkedEncoding) {
    assertx(m_headerSent);
    StringHolder response = compressResponse("", 0, true);
    sendImpl(response.data(), response.size(), m_responseCode, true, true);
    eomSent = true;
  } else if (!m_headerSent) {
    sendRawInternal("", 0);
  }
  auto httpResponseStats = ServiceData::createTimeSeries(
    folly::to<std::string>(HTTP_RESPONSE_STATS_PREFIX, getResponseCode()),
    {ServiceData::StatsType::SUM});
  httpResponseStats->addValue(1);
  if (!eomSent) {
    onSendEndImpl();
  }
  // Record that we have ended the request so any further output is discarded.
  m_sendEnded = true;
}

void Transport::redirect(const char *location, int code /* = 302 */,
                         const char *info /* = nullptr */) {
  if (auto stream = getStreamTransport()) {
    stream->close();
    return;
  }
  addHeaderImpl("Location", location);
  setResponse(code, info);
  sendString("Moved", code);
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
    size_t size; getPostData(size);
    Add(info, "Post Data", FormatSize(size));
  }
}

///////////////////////////////////////////////////////////////////////////////
// StructuredLog

StructuredLogEntry* Transport::createStructuredLogEntry() {
  assertx(!m_structLogEntry);
  m_structLogEntry = std::make_unique<StructuredLogEntry>();
  return m_structLogEntry.get();
}

StructuredLogEntry* Transport::getStructuredLogEntry() {
  return m_structLogEntry.get();
}

void Transport::resetStructuredLogEntry() {
  m_structLogEntry.reset();
}

///////////////////////////////////////////////////////////////////////////////
// Request tracing

void Transport::forceInitRequestTrace() {
  if (m_requestTrace) return;
  m_requestTrace.emplace();
  rqtrace::ScopeGuard(
    getRequestTrace(), "REQUEST_QUEUE", getQueueTime()
  ).finish(getWallTime());
}

///////////////////////////////////////////////////////////////////////////////
}

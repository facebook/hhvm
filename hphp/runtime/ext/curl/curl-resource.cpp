#include "hphp/runtime/ext/curl/curl-resource.h"
#include "hphp/runtime/ext/curl/curl-multi-resource.h"
#include "hphp/runtime/ext/curl/curl-share-resource.h"
#include "hphp/runtime/ext/curl/ext_curl.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/curl-tls-workarounds.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/stack-logger.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/vm/vm-regs.h"

#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/multi.h>
#include <folly/portability/OpenSSL.h>

#define PHP_CURL_STDOUT 0
#define PHP_CURL_FILE   1
#define PHP_CURL_USER   2
#define PHP_CURL_DIRECT 3
#define PHP_CURL_RETURN 4
#define PHP_CURL_ASCII  5
#define PHP_CURL_BINARY 6
#define PHP_CURL_IGNORE 7

namespace {
const HPHP::StaticString
  s_name("name"),
  s_mime("mime"),
  s_postname("postname");
}

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////
// CurlResource

CurlResource::ToFree::~ToFree() {
  for (unsigned int i = 0; i < str.size(); i++) {
    req::free(str[i]);
  }
  for (unsigned int i = 0; i < post.size(); i++) {
    curl_formfree(post[i]);
  }
  for (unsigned int i = 0; i < slist.size(); i++) {
    curl_slist_free_all(slist[i]);
  }
}

CurlResource::CurlResource(const String& url)
    : m_emptyPost(true), m_safeUpload(true) {
  m_cp = curl_easy_init();
  m_multi = nullptr;
  m_url = url;

  memset(m_error_str, 0, sizeof(m_error_str));
  m_error_no = CURLE_OK;
  m_to_free = req::make_shared<ToFree>();

  m_write.method = PHP_CURL_STDOUT;
  m_write.type   = PHP_CURL_ASCII;
  m_read.method  = PHP_CURL_DIRECT;
  m_write_header.method = PHP_CURL_IGNORE;

  setDefaultOptions();

  if (!url.empty()) {
#if LIBCURL_VERSION_NUM >= 0x071100
    /* Strings passed to libcurl as 'char *' arguments, are copied by
       the library... NOTE: before 7.17.0 strings were not copied. */
    curl_easy_setopt(m_cp, CURLOPT_URL, url.c_str());
#else
    char *urlcopy = req::strndup(url.data(), url.size());
    curl_easy_setopt(m_cp, CURLOPT_URL, urlcopy);
    m_to_free->str.push_back(urlcopy);
#endif
  }
}

void CurlResource::sweep() {
  m_write.buf.release();
  m_write_header.buf.release();
  closeForSweep();
}

void CurlResource::close() {
  if (m_in_callback) {
    raise_warning("curl_close(): Attempt to close cURL in callback, ignored.");
    return;
  }
  closeForSweep();
  m_opts.clear();
  m_to_free.reset();
}

void CurlResource::closeForSweep() {
  assertx(!m_exception);
  assertx(IMPLIES(m_multi, m_cp));
  if (!m_cp) return;
  if (m_multi) m_multi->remove(this, /*leak=*/true);
  curl_easy_cleanup(m_cp);
  m_cp = nullptr;
}

void CurlResource::check_exception() {
  if (!m_exception) return;
  auto e = std::move(m_exception);
  assertx(!m_exception);
  if (isPhpException(e)) {
    throw_object(getPhpException(e));
  } else {
    getCppException(e)->throwException();
  }
}

void CurlResource::reseat() {
  // Note: this is the minimum set of things to point the CURL*
 // to this CurlHandle
  curl_easy_setopt(m_cp, CURLOPT_ERRORBUFFER,       m_error_str);
  curl_easy_setopt(m_cp, CURLOPT_FILE,              (void*)this);
  curl_easy_setopt(m_cp, CURLOPT_INFILE,            (void*)this);
  curl_easy_setopt(m_cp, CURLOPT_WRITEHEADER,       (void*)this);
  curl_easy_setopt(m_cp, CURLOPT_SSL_CTX_DATA,      (void*)this);
}

void CurlResource::reset() {
  curl_easy_reset(m_cp);
  setDefaultOptions();
}

void CurlResource::setDefaultOptions() {
  curl_easy_setopt(m_cp, CURLOPT_NOPROGRESS,        1);
  curl_easy_setopt(m_cp, CURLOPT_VERBOSE,           0);
  curl_easy_setopt(m_cp, CURLOPT_WRITEFUNCTION,     curl_write);
  curl_easy_setopt(m_cp, CURLOPT_READFUNCTION,      curl_read);
  curl_easy_setopt(m_cp, CURLOPT_HEADERFUNCTION,    curl_write_header);
  curl_easy_setopt(m_cp, CURLOPT_DNS_USE_GLOBAL_CACHE, 0); // for thread-safe
  curl_easy_setopt(m_cp, CURLOPT_DNS_CACHE_TIMEOUT, 120);
  curl_easy_setopt(m_cp, CURLOPT_MAXREDIRS, 20); // no infinite redirects
  curl_easy_setopt(m_cp, CURLOPT_NOSIGNAL, 1); // for multithreading mode
  curl_easy_setopt(m_cp, CURLOPT_SSL_CTX_FUNCTION,
                   CurlResource::ssl_ctx_callback);

  curl_easy_setopt(m_cp, CURLOPT_TIMEOUT,
                   minTimeout(RuntimeOption::HttpDefaultTimeout));
  curl_easy_setopt(m_cp, CURLOPT_CONNECTTIMEOUT,
                   minTimeout(RuntimeOption::HttpDefaultTimeout));
  reseat();
}

void CurlResource::prepare() {
  if (!useCertCache()) {
    if (auto const path = cainfo(false).get()) {
      curl_easy_setopt(m_cp, CURLOPT_CAINFO, path->data());
    }
    if (auto const path = cainfo(true).get()) {
      curl_easy_setopt(m_cp, CURLOPT_PROXY_CAINFO, path->data());
    }
  } else {
    curl_easy_setopt(m_cp, CURLOPT_CAINFO, NULL);
    curl_easy_setopt(m_cp, CURLOPT_PROXY_CAINFO, NULL);
  }

  if (m_emptyPost) {
    // As per curl docs, an empty post must set POSTFIELDSIZE to be 0 or
    // the reader function will be called
    curl_easy_setopt(m_cp, CURLOPT_POSTFIELDSIZE, 0);
  }
}

Variant CurlResource::execute() {
  assertx(!m_exception);
  if (m_cp == nullptr) {
    return false;
  }
  m_write.buf.clear();
  m_write.content.clear();
  m_header.clear();
  memset(m_error_str, 0, sizeof(m_error_str));

  prepare();

  {
    IOStatusHelper io("curl_easy_perform", m_url.data());
    SYNC_VM_REGS_SCOPED();
    if (m_in_exec) {
      log_native_stack("unexpected re-entry into curl_exec");
    }
    m_in_exec = true;
    // T29358191: curl_easy_perform should not throw... trust but verify
    try {
      m_error_no = curl_easy_perform(m_cp);
    } catch (...) {
      m_in_exec = false;
      log_native_stack("unexpected exception from curl_easy_perform");
      throw;
    }
    m_in_exec = false;
    check_exception();
  }
  set_curl_statuses(m_cp, m_url.data());

  /* CURLE_PARTIAL_FILE is returned by HEAD requests */
  if (m_error_no != CURLE_OK && m_error_no != CURLE_PARTIAL_FILE) {
    m_write.buf.clear();
    m_write.content.clear();
    return false;
  }
  if (m_write.method == PHP_CURL_RETURN) {
    if (!m_write.buf.empty()) {
      m_write.content = m_write.buf.detach();
    }
    if (!m_write.content.empty()) {
      return m_write.content;
    }
  }
  if (m_write.method == PHP_CURL_RETURN) {
    return empty_string_variant();
  }
  return true;
}

String CurlResource::getContents() {
  if (m_write.method == PHP_CURL_RETURN) {
    if (!m_write.buf.empty()) {
      m_write.content = m_write.buf.detach();
    }
    return m_write.content;
  }
  return String();
}

bool CurlResource::setOption(long option, const Variant& value) {
  if (m_cp == nullptr) {
    return false;
  }
  m_error_no = CURLE_OK;

  bool ret;
  if (isLongOption(option)) {
    ret = setLongOption(option, value.toInt64());
  } else if (isStringOption(option)) {
    ret = setStringOption(option, value.toString());
  } else if (isNullableStringOption(option)) {
    ret = setNullableStringOption(option, value);
  } else if (isFileOption(option)) {
    auto fp = dyn_cast_or_null<File>(value);
    if (!fp) return false;
    ret = setFileOption(option, fp);
  } else if (isStringListOption(option)) {
    ret = setStringListOption(option, value);
  } else if (isNonCurlOption(option)) {
    ret = setNonCurlOption(option, value);
  } else if (isBlobOption(option)) {
    ret = setBlobOption(option, value.toString());
  } else if (option == CURLOPT_POSTFIELDS) {
    ret = setPostFieldsOption(value);
  } else if (option == CURLOPT_SHARE) {
    auto curlsh = dyn_cast_or_null<CurlShareResource>(value);
    if (!curlsh || curlsh->isInvalid()) {
      return false;
    }
    m_error_no = curlsh->attachToCurlHandle(m_cp);
    ret = true;
  } else if (option == CURLINFO_HEADER_OUT) {
    if (value.toInt64() == 1) {
      curl_easy_setopt(m_cp, CURLOPT_DEBUGFUNCTION, curl_debug);
      curl_easy_setopt(m_cp, CURLOPT_DEBUGDATA, (void *)this);
      curl_easy_setopt(m_cp, CURLOPT_VERBOSE, 1);
    } else {
      curl_easy_setopt(m_cp, CURLOPT_DEBUGFUNCTION, nullptr);
      curl_easy_setopt(m_cp, CURLOPT_DEBUGDATA, nullptr);
      curl_easy_setopt(m_cp, CURLOPT_VERBOSE, 0);
    }
    ret = true;
  } else {
    m_error_no = CURLE_FAILED_INIT;
    raise_invalid_argument_warning("option: %ld", option);
    ret = false;
  }

  if (!ret) { return false; }

  m_opts.set(int64_t(option), value);
  return m_error_no == CURLE_OK;
}

Variant CurlResource::getOption(long option) {
  if (option == 0) {
    return m_opts;
  }

  if (m_opts.exists(int64_t(option))) {
    return m_opts[int64_t(option)];
  }

  return false;
}

CURL* CurlResource::get() {
  if (!m_cp) {
    throw_null_pointer_exception();
  }
  return m_cp;
}

bool CurlResource::isLongOption(long option) {
  switch (option) {
    case CURLOPT_DNS_USE_GLOBAL_CACHE:
      // This is not thread-safe when set to true, so pretend we don't know what
      // it is.
      return false;

    // These first few are in their own case statements in PHP
    case CURLOPT_CLOSEPOLICY:
    case CURLOPT_SSL_VERIFYHOST:
    case CURLOPT_FOLLOWLOCATION:
#if LIBCURL_VERSION_NUM >= 0x071301
    case CURLOPT_POSTREDIR:
#endif

    // Everything else
    case CURLOPT_AUTOREFERER:
    case CURLOPT_BUFFERSIZE:
    case CURLOPT_CONNECTTIMEOUT:
    case CURLOPT_COOKIESESSION:
    case CURLOPT_CRLF:
    case CURLOPT_DNS_CACHE_TIMEOUT:
    case CURLOPT_FAILONERROR:
    case CURLOPT_FILETIME:
    case CURLOPT_FORBID_REUSE:
    case CURLOPT_FRESH_CONNECT:
    case CURLOPT_FTP_USE_EPRT:
    case CURLOPT_FTP_USE_EPSV:
    case CURLOPT_HEADER:
    case CURLOPT_HTTPGET:
    case CURLOPT_HTTPPROXYTUNNEL:
    case CURLOPT_HTTP_VERSION:
    case CURLOPT_INFILESIZE:
    case CURLOPT_LOW_SPEED_LIMIT:
    case CURLOPT_LOW_SPEED_TIME:
    case CURLOPT_MAXCONNECTS:
    case CURLOPT_MAXREDIRS:
    case CURLOPT_NETRC:
    case CURLOPT_NOBODY:
    case CURLOPT_NOPROGRESS:
    case CURLOPT_NOSIGNAL:
    case CURLOPT_PORT:
    case CURLOPT_POST:
    case CURLOPT_PROXYPORT:
    case CURLOPT_PROXYTYPE:
    case CURLOPT_PUT:
    case CURLOPT_RESUME_FROM:
    case CURLOPT_SSLVERSION:
    case CURLOPT_SSL_VERIFYPEER:
    case CURLOPT_TIMECONDITION:
    case CURLOPT_TIMEOUT:
    case CURLOPT_TIMEVALUE:
    case CURLOPT_TRANSFERTEXT:
    case CURLOPT_UNRESTRICTED_AUTH:
    case CURLOPT_UPLOAD:
    case CURLOPT_VERBOSE:
#if LIBCURL_VERSION_NUM >= 0x070a06 /* Available since 7.10.6 */
    case CURLOPT_HTTPAUTH:
#endif
#if LIBCURL_VERSION_NUM >= 0x070a07 /* Available since 7.10.7 */
    case CURLOPT_FTP_CREATE_MISSING_DIRS:
    case CURLOPT_PROXYAUTH:
#endif
#if LIBCURL_VERSION_NUM >= 0x070a08 /* Available since 7.10.8 */
    case CURLOPT_FTP_RESPONSE_TIMEOUT:
    case CURLOPT_IPRESOLVE:
    case CURLOPT_MAXFILESIZE:
#endif
#if LIBCURL_VERSION_NUM >= 0x070b02 /* Available since 7.11.2 */
    case CURLOPT_TCP_NODELAY:
#endif
#if LIBCURL_VERSION_NUM >= 0x070c02 /* Available since 7.12.2 */
    case CURLOPT_FTPSSLAUTH:
#endif
#if LIBCURL_VERSION_NUM >= 0x070e01 /* Available since 7.14.1 */
    case CURLOPT_IGNORE_CONTENT_LENGTH:
#endif
#if LIBCURL_VERSION_NUM >= 0x070f00 /* Available since 7.15.0 */
    case CURLOPT_FTP_SKIP_PASV_IP:
#endif
#if LIBCURL_VERSION_NUM >= 0x070f01 /* Available since 7.15.1 */
    case CURLOPT_FTP_FILEMETHOD:
#endif
#if LIBCURL_VERSION_NUM >= 0x070f02 /* Available since 7.15.2 */
    case CURLOPT_CONNECT_ONLY:
    case CURLOPT_LOCALPORT:
    case CURLOPT_LOCALPORTRANGE:
#endif
#if LIBCURL_VERSION_NUM >= 0x071000 /* Available since 7.16.0 */
    case CURLOPT_SSL_SESSIONID_CACHE:
#endif
#if LIBCURL_VERSION_NUM >= 0x071001 /* Available since 7.16.1 */
    case CURLOPT_FTP_SSL_CCC:
    case CURLOPT_SSH_AUTH_TYPES:
#endif
#if LIBCURL_VERSION_NUM >= 0x071002 /* Available since 7.16.2 */
    case CURLOPT_CONNECTTIMEOUT_MS:
    case CURLOPT_HTTP_CONTENT_DECODING:
    case CURLOPT_HTTP_TRANSFER_DECODING:
    case CURLOPT_TIMEOUT_MS:
#endif
#if LIBCURL_VERSION_NUM >= 0x071004 /* Available since 7.16.4 */
    case CURLOPT_NEW_DIRECTORY_PERMS:
    case CURLOPT_NEW_FILE_PERMS:
#endif
#if LIBCURL_VERSION_NUM >= 0x071100 /* Available since 7.17.0 */
    case CURLOPT_USE_SSL:
#elif LIBCURL_VERSION_NUM >= 0x070b00 /* Available since 7.11.0 */
    case CURLOPT_FTP_SSL:
#endif
#if LIBCURL_VERSION_NUM >= 0x071100 /* Available since 7.17.0 */
    case CURLOPT_APPEND:
    case CURLOPT_DIRLISTONLY:
#else
    case CURLOPT_FTPAPPEND:
    case CURLOPT_FTPLISTONLY:
#endif
#if LIBCURL_VERSION_NUM >= 0x071200 /* Available since 7.18.0 */
    case CURLOPT_PROXY_TRANSFER_MODE:
#endif
#if LIBCURL_VERSION_NUM >= 0x071300 /* Available since 7.19.0 */
    case CURLOPT_ADDRESS_SCOPE:
#endif
#if LIBCURL_VERSION_NUM >  0x071301 /* Available since 7.19.1 */
    case CURLOPT_CERTINFO:
#endif
#if LIBCURL_VERSION_NUM >= 0x071304 /* Available since 7.19.4 */
    case CURLOPT_PROTOCOLS:
    case CURLOPT_REDIR_PROTOCOLS:
    case CURLOPT_SOCKS5_GSSAPI_NEC:
    case CURLOPT_TFTP_BLKSIZE:
#endif
#if LIBCURL_VERSION_NUM >= 0x071400 /* Available since 7.20.0 */
    case CURLOPT_FTP_USE_PRET:
    case CURLOPT_RTSP_CLIENT_CSEQ:
    case CURLOPT_RTSP_REQUEST:
    case CURLOPT_RTSP_SERVER_CSEQ:
#endif
#if LIBCURL_VERSION_NUM >= 0x071500 /* Available since 7.21.0 */
    case CURLOPT_WILDCARDMATCH:
#endif
#if LIBCURL_VERSION_NUM >= 0x071504 /* Available since 7.21.4 */
    case CURLOPT_TLSAUTH_TYPE:
#endif
#if LIBCURL_VERSION_NUM >= 0x071600 /* Available since 7.22.0 */
    case CURLOPT_GSSAPI_DELEGATION:
#endif
#if LIBCURL_VERSION_NUM >= 0x071800 /* Available since 7.24.0 */
    case CURLOPT_ACCEPTTIMEOUT_MS:
#endif
#if LIBCURL_VERSION_NUM >= 0x071900 /* Available since 7.25.0 */
    case CURLOPT_SSL_OPTIONS:
    case CURLOPT_TCP_KEEPALIVE:
    case CURLOPT_TCP_KEEPIDLE:
    case CURLOPT_TCP_KEEPINTVL:
#endif
#if LIBCURL_VERSION_NUM >= 0x071f00 /* Available since 7.31.0 */
    case CURLOPT_SASL_IR:
#endif
#if LIBCURL_VERSION_NUM >= 0x072400 /* Available since 7.36.0 */
    case CURLOPT_EXPECT_100_TIMEOUT_MS:
    case CURLOPT_SSL_ENABLE_ALPN:
    case CURLOPT_SSL_ENABLE_NPN:
#endif
#if LIBCURL_VERSION_NUM >= 0x072500 /* Available since 7.37.0 */
    case CURLOPT_HEADEROPT:
#endif
#if LIBCURL_VERSION_NUM >= 0x072900 /* Available since 7.41.0 */
    case CURLOPT_SSL_VERIFYSTATUS:
#endif
#if LIBCURL_VERSION_NUM >= 0x072a00 /* Available since 7.42.0 */
    case CURLOPT_PATH_AS_IS:
    case CURLOPT_SSL_FALSESTART:
#endif
#if LIBCURL_VERSION_NUM >= 0x072b00 /* Available since 7.43.0 */
    case CURLOPT_PIPEWAIT:
#endif
#if LIBCURL_VERSION_NUM >= 0x072e00 /* Available since 7.46.0 */
    case CURLOPT_STREAM_WEIGHT:
#endif
#if LIBCURL_VERSION_NUM >= 0x073000 /* Available since 7.48.0 */
    case CURLOPT_TFTP_NO_OPTIONS:
#endif
#if LIBCURL_VERSION_NUM >= 0x073100 /* Available since 7.49.0 */
    case CURLOPT_TCP_FASTOPEN:
#endif
#if LIBCURL_VERSION_NUM >= 0x073300 /* Available since 7.51.0 */
    case CURLOPT_KEEP_SENDING_ON_ERROR:
#endif
#if LIBCURL_VERSION_NUM >= 0x073400 /* Available since 7.52.0 */
    case CURLOPT_PROXY_SSLVERSION:
    case CURLOPT_PROXY_SSL_OPTIONS:
    case CURLOPT_PROXY_SSL_VERIFYHOST:
    case CURLOPT_PROXY_SSL_VERIFYPEER:
#endif
#if LIBCURL_VERSION_NUM >= 0x073600 /* Available since 7.54.0 */
    case CURLOPT_SUPPRESS_CONNECT_HEADERS:
#endif
#if LIBCURL_VERSION_NUM >= 0x073700 /* Available since 7.55.0 */
    case CURLOPT_SOCKS5_AUTH:
#endif
#if LIBCURL_VERSION_NUM >= 0x073800 /* Available since 7.56.0 */
    case CURLOPT_SSH_COMPRESSION:
#endif
#if LIBCURL_VERSION_NUM >= 0x073b00 /* Available since 7.59.0 */
    case CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS:
    case CURLOPT_TIMEVALUE_LARGE:
#endif
#if LIBCURL_VERSION_NUM >= 0x073c00 /* Available since 7.60.0 */
    case CURLOPT_DNS_SHUFFLE_ADDRESSES:
    case CURLOPT_HAPROXYPROTOCOL:
#endif
#if LIBCURL_VERSION_NUM >= 0x073d00 /* Available since 7.61.0 */
    case CURLOPT_DISALLOW_USERNAME_IN_URL:
#endif
#if LIBCURL_VERSION_NUM >= 0x073e00 /* Available since 7.62.0 */
    case CURLOPT_UPKEEP_INTERVAL_MS:
    case CURLOPT_UPLOAD_BUFFERSIZE:
#endif
#if LIBCURL_VERSION_NUM >= 0x074000 /* Available since 7.64.0 */
    case CURLOPT_HTTP09_ALLOWED:
#endif
#if LIBCURL_VERSION_NUM >= 0x074001 /* Available since 7.64.1 */
    case CURLOPT_ALTSVC_CTRL:
#endif
#if LIBCURL_VERSION_NUM >= 0x074100 /* Available since 7.65.0 */
    case CURLOPT_MAXAGE_CONN:
#endif
#if LIBCURL_VERSION_NUM >= 0x074a00 /* Available since 7.74.0 */
    case CURLOPT_HSTS_CTRL:
#endif
#if LIBCURL_VERSION_NUM >= 0x074c00 /* Available since 7.76.0 */
    case CURLOPT_DOH_SSL_VERIFYHOST:
    case CURLOPT_DOH_SSL_VERIFYPEER:
    case CURLOPT_DOH_SSL_VERIFYSTATUS:
#endif
#if LIBCURL_VERSION_NUM >= 0x075000 /* Available since 7.80.0 */
    case CURLOPT_MAXLIFETIME_CONN:
#endif
#if LIBCURL_VERSION_NUM >= 0x075100 /* Available since 7.81.0 */
    case CURLOPT_MIME_OPTIONS:
#endif
#if CURLOPT_MUTE != 0
    case CURLOPT_MUTE:
#endif
      return true;
    default:
      return false;
  }
}

bool CurlResource::setLongOption(long option, long value) {
  if (option == CURLOPT_TIMEOUT) {
    value = minTimeout(value);
#if LIBCURL_VERSION_NUM >= 0x071002
  } else if (option == CURLOPT_TIMEOUT_MS) {
    value = minTimeoutMS(value);
#endif
  } else if ((option == CURLOPT_SSL_VERIFYHOST) && (value == 1)) {
    raise_notice(
      "curl_setopt(): CURLOPT_SSL_VERIFYHOST set to true which disables "
      "common name validation "
      "(setting CURLOPT_SSL_VERIFYHOST to 2 enables common name validation)"
    );
  }

  m_error_no = curl_easy_setopt(m_cp, (CURLoption)option, value);
  return m_error_no == CURLE_OK;
}

bool CurlResource::isStringFilePathOption(long option) {
  switch (option) {
    case CURLOPT_COOKIEFILE:
    case CURLOPT_COOKIEJAR:
    case CURLOPT_RANDOM_FILE:
    case CURLOPT_SSLCERT:
#if LIBCURL_VERSION_NUM >= 0x070b00 /* Available since 7.11.0 */
    case CURLOPT_NETRC_FILE:
#endif
#if LIBCURL_VERSION_NUM >= 0x071001 /* Available since 7.16.1 */
    case CURLOPT_SSH_PRIVATE_KEYFILE:
    case CURLOPT_SSH_PUBLIC_KEYFILE:
#endif
#if LIBCURL_VERSION_NUM >= 0x071300 /* Available since 7.19.0 */
    case CURLOPT_CRLFILE:
    case CURLOPT_ISSUERCERT:
#endif
#if LIBCURL_VERSION_NUM >= 0x071306 /* Available since 7.19.6 */
    case CURLOPT_SSH_KNOWNHOSTS:
#endif
#if LIBCURL_VERSION_NUM >= 0x073400 /* Available since 7.52.0 */
    case CURLOPT_PROXY_CRLFILE:
    case CURLOPT_PROXY_SSLCERT:
#endif
#if LIBCURL_VERSION_NUM >= 0x074700 /* Available since 7.71.0 */
    case CURLOPT_PROXY_ISSUERCERT:
#endif
      return true;
    default:
      return false;
  }
}

bool CurlResource::isStringOption(long option) {
  switch (option) {
    // Not in PHP's main string case
    case CURLOPT_PRIVATE:
    case CURLOPT_URL:
    case CURLOPT_KRB4LEVEL:

    // Everything else
    case CURLOPT_CAINFO:
    case CURLOPT_CAPATH:
    case CURLOPT_COOKIE:
    case CURLOPT_EGDSOCKET:
    case CURLOPT_INTERFACE:
    case CURLOPT_PROXY:
    case CURLOPT_PROXYUSERPWD:
    case CURLOPT_REFERER:
    case CURLOPT_SSLCERTTYPE:
    case CURLOPT_SSLENGINE:
    case CURLOPT_SSLENGINE_DEFAULT:
    case CURLOPT_SSLKEY:
    case CURLOPT_SSLKEYPASSWD:
    case CURLOPT_SSLKEYTYPE:
    case CURLOPT_SSL_CIPHER_LIST:
    case CURLOPT_USERAGENT:
    case CURLOPT_USERPWD:
#if LIBCURL_VERSION_NUM >= 0x070e01 /* Available since 7.14.1 */
    case CURLOPT_COOKIELIST:
#endif
#if LIBCURL_VERSION_NUM >= 0x070f05 /* Available since 7.15.5 */
    case CURLOPT_FTP_ALTERNATIVE_TO_USER:
#endif
#if LIBCURL_VERSION_NUM >= 0x071101 /* Available since 7.17.1 */
    case CURLOPT_SSH_HOST_PUBLIC_KEY_MD5:
#endif
#if LIBCURL_VERSION_NUM >= 0x071301 /* Available since 7.19.1 */
    case CURLOPT_PASSWORD:
    case CURLOPT_PROXYPASSWORD:
    case CURLOPT_PROXYUSERNAME:
    case CURLOPT_USERNAME:
#endif
#if LIBCURL_VERSION_NUM >= 0x071304 /* Available since 7.19.4 */
    case CURLOPT_NOPROXY:
    case CURLOPT_SOCKS5_GSSAPI_SERVICE:
#endif
#if LIBCURL_VERSION_NUM >= 0x071400 /* Available since 7.20.0 */
    case CURLOPT_MAIL_FROM:
    case CURLOPT_RTSP_STREAM_URI:
    case CURLOPT_RTSP_TRANSPORT:
#endif
#if LIBCURL_VERSION_NUM >= 0x071504 /* Available since 7.21.4 */
    case CURLOPT_TLSAUTH_PASSWORD:
    case CURLOPT_TLSAUTH_USERNAME:
#endif
#if LIBCURL_VERSION_NUM >= 0x071506 /* Available since 7.21.6 */
    case CURLOPT_ACCEPT_ENCODING:
    case CURLOPT_TRANSFER_ENCODING:
#else
    case CURLOPT_ENCODING:
#endif
#if LIBCURL_VERSION_NUM >= 0x071800 /* Available since 7.24.0 */
    case CURLOPT_DNS_SERVERS:
#endif
#if LIBCURL_VERSION_NUM >= 0x071900 /* Available since 7.25.0 */
    case CURLOPT_MAIL_AUTH:
#endif
#if LIBCURL_VERSION_NUM >= 0x072200 /* Available since 7.34.0 */
    case CURLOPT_LOGIN_OPTIONS:
#endif
#if LIBCURL_VERSION_NUM >= 0x072700 /* Available since 7.39.0 */
    case CURLOPT_PINNEDPUBLICKEY:
#endif
#if LIBCURL_VERSION_NUM >= 0x072b00 /* Available since 7.43.0 */
    case CURLOPT_PROXY_SERVICE_NAME:
    case CURLOPT_SERVICE_NAME:
#endif
#if LIBCURL_VERSION_NUM >= 0x072d00 /* Available since 7.45.0 */
    case CURLOPT_DEFAULT_PROTOCOL:
#endif
#if LIBCURL_VERSION_NUM >= 0x073400 /* Available since 7.52.0 */
    case CURLOPT_PROXY_CAINFO:
    case CURLOPT_PROXY_CAPATH:
    case CURLOPT_PROXY_KEYPASSWD:
    case CURLOPT_PROXY_PINNEDPUBLICKEY:
    case CURLOPT_PROXY_SSLCERTTYPE:
    case CURLOPT_PROXY_SSLKEY:
    case CURLOPT_PROXY_SSLKEYTYPE:
    case CURLOPT_PROXY_SSL_CIPHER_LIST:
    case CURLOPT_PROXY_TLSAUTH_PASSWORD:
    case CURLOPT_PROXY_TLSAUTH_TYPE:
    case CURLOPT_PROXY_TLSAUTH_USERNAME:
#endif
#if LIBCURL_VERSION_NUM >= 0x073d00 /* Available since 7.61.0 */
    case CURLOPT_PROXY_TLS13_CIPHERS:
    case CURLOPT_TLS13_CIPHERS:
#endif
#if LIBCURL_VERSION_NUM >= 0x073e00 /* Available since 7.62.0 */
    case CURLOPT_DOH_URL:
#endif
#if LIBCURL_VERSION_NUM >= 0x074001 /* Available since 7.64.1 */
    case CURLOPT_ALTSVC:
#endif
#if LIBCURL_VERSION_NUM >= 0x074200 /* Available since 7.66.0 */
    case CURLOPT_SASL_AUTHZID:
#endif
#if LIBCURL_VERSION_NUM >= 0x074900 /* Available since 7.73.0 */
    case CURLOPT_SSL_EC_CURVES:
#endif
#if LIBCURL_VERSION_NUM >= 0x074a00 /* Available since 7.74.0 */
    case CURLOPT_HSTS:
#endif
#if LIBCURL_VERSION_NUM >= 0x074b00 /* Available since 7.75.0 */
    case CURLOPT_AWS_SIGV4:
#endif
#if LIBCURL_VERSION_NUM >= 0x075000 /* Available since 7.80.0 */
    case CURLOPT_SSH_HOST_PUBLIC_KEY_SHA256:
#endif
      return true;
    default:
      return isStringFilePathOption(option);
  }
}

bool CurlResource::setStringOption(long option, const String& value) {
  assertx(isStringOption(option));

  // the following options deal with files, therefore the open_basedir check
  // is required.
  if (isStringFilePathOption(option) && !value.empty()) {
    String filename = File::TranslatePath(value);
    if (filename.empty()) {
      raise_warning(
        "open_basedir restriction in effect. File(%s) is not within "
        "the allowed paths",
        value.data()
      );
      return false;
    }
  }

  if (option == CURLOPT_URL) m_url = value;

#if LIBCURL_VERSION_NUM >= 0x071100
  /* Strings passed to libcurl as 'char *' arguments, are copied
     by the library... NOTE: before 7.17.0 strings were not copied. */
  m_error_no = curl_easy_setopt(m_cp, (CURLoption)option, value.c_str());
#else
  char *copystr = req::strndup(value.data(), value.size());
  m_to_free->str.push_back(copystr);
  m_error_no = curl_easy_setopt(m_cp, (CURLoption)option, copystr);
#endif

  return m_error_no == CURLE_OK;
}

bool CurlResource::isNullableStringOption(long option) {
  switch (option) {
    case CURLOPT_CUSTOMREQUEST:
    case CURLOPT_FTPPORT:
    case CURLOPT_RANGE:
#if LIBCURL_VERSION_NUM >= 0x070d00 /* Available since 7.13.0 */
    case CURLOPT_FTP_ACCOUNT:
#endif
#if LIBCURL_VERSION_NUM >= 0x071400 /* Available since 7.20.0 */
    case CURLOPT_RTSP_SESSION_ID:
#endif
#if LIBCURL_VERSION_NUM >= 0x072100 /* Available since 7.33.0 */
    case CURLOPT_DNS_INTERFACE:
    case CURLOPT_DNS_LOCAL_IP4:
    case CURLOPT_DNS_LOCAL_IP6:
    case CURLOPT_XOAUTH2_BEARER:
#endif
#if LIBCURL_VERSION_NUM >= 0x072800 /* Available since 7.40.0 */
    case CURLOPT_UNIX_SOCKET_PATH:
#endif
#if LIBCURL_VERSION_NUM >= 0x073500 /* Available since 7.53.0 */
    case CURLOPT_ABSTRACT_UNIX_SOCKET:
#endif
#if LIBCURL_VERSION_NUM >= 0x073700 /* Available since 7.55.0 */
    case CURLOPT_REQUEST_TARGET:
#endif
#if LIBCURL_VERSION_NUM >= 0x071004 /* Available since 7.16.4 */
    case CURLOPT_KRBLEVEL:
#else
    case CURLOPT_KRB4LEVEL:
#endif
      return true;
    default:
      return false;
  }
}

bool CurlResource::setNullableStringOption(long option, const Variant& value) {
  assertx(isNullableStringOption(option));

  if (value.isNull()) {
    m_error_no = curl_easy_setopt(m_cp, (CURLoption)option, nullptr);
  } else {
    auto const strValue = value.toString();
#if LIBCURL_VERSION_NUM >= 0x071100
    /* Strings passed to libcurl as 'char *' arguments, are copied
       by the library... NOTE: before 7.17.0 strings were not copied. */
    m_error_no = curl_easy_setopt(m_cp, (CURLoption)option, strValue.c_str());
#else
    char *copystr = req::strndup(strValue.data(), strValue.size());
    m_to_free->str.push_back(copystr);
    m_error_no = curl_easy_setopt(m_cp, (CURLoption)option, copystr);
#endif
  }

  return m_error_no == CURLE_OK;
}

bool CurlResource::isBlobOption(long option) {
  switch (option) {
#if LIBCURL_VERSION_NUM >= 0x074700 /* Available since 7.71.0 */
    case CURLOPT_SSLCERT_BLOB:
    case CURLOPT_SSLKEY_BLOB:
    case CURLOPT_PROXY_SSLCERT_BLOB:
    case CURLOPT_PROXY_SSLKEY_BLOB:
    case CURLOPT_ISSUERCERT_BLOB:
    case CURLOPT_PROXY_ISSUERCERT_BLOB:
      return true;
#endif
#if LIBCURL_VERSION_NUM >= 0x074d00 /* Available since 7.77.0 */
    case CURLOPT_CAINFO_BLOB:
    case CURLOPT_PROXY_CAINFO_BLOB:
      return true;
#endif
    default:
      return false;
  }
}

bool CurlResource::setBlobOption(long option, const String& value) {
#if LIBCURL_VERSION_NUM >= 0x074700 /* Available since 7.71.0 */
  assertx(isBlobOption(option));
  struct curl_blob blob;
  blob.data = (void *)value.data();
  blob.len = value.size();
  blob.flags = CURL_BLOB_COPY;
  m_error_no = curl_easy_setopt(m_cp, (CURLoption)option, &blob);
  return m_error_no == CURLE_OK;
#else
  return false;
#endif
}

bool CurlResource::setPostFieldsOption(const Variant& value) {
  m_emptyPost = false;

  if (!value.isArray() && !value.is(KindOfObject)) {
    String svalue = value.toString();
#if LIBCURL_VERSION_NUM >= 0x071100
    /* with curl 7.17.0 and later, we can use COPYPOSTFIELDS,
       but we have to provide size before */
    m_error_no = curl_easy_setopt(m_cp, CURLOPT_POSTFIELDSIZE,
                                  svalue.size());
    m_error_no = curl_easy_setopt(m_cp, CURLOPT_COPYPOSTFIELDS,
                                svalue.c_str());
#else
    char *post = req::strndup(svalue.data(), svalue.size());
    m_to_free->str.push_back(post);

    m_error_no = curl_easy_setopt(m_cp, CURLOPT_POSTFIELDS, post);
    m_error_no = curl_easy_setopt(m_cp, CURLOPT_POSTFIELDSIZE,
                                  svalue.size());
#endif
    return m_error_no == CURLE_OK;
  }

  Array arr = value.toArray();
  curl_httppost *first = nullptr;
  curl_httppost *last  = nullptr;
  for (ArrayIter iter(arr); iter; ++iter) {
    String key = iter.first().toString();
    Variant var_val = iter.second();
    if (UNLIKELY(var_val.isObject()
        && var_val.toObject()->instanceof(SystemLib::getCURLFileClass()))) {
      Object val = var_val.toObject();

      String name = val->o_get(s_name).toString();
      String mime = val->o_get(s_mime).toString();
      String postname = val->o_get(s_postname).toString();

      m_error_no = (CURLcode)curl_formadd
        (&first, &last,
         CURLFORM_COPYNAME, key.data(),
         CURLFORM_NAMELENGTH, (long)key.size(),
         CURLFORM_FILENAME, postname.empty()
                            ? name.c_str()
                            : postname.c_str(),
         CURLFORM_CONTENTTYPE, mime.empty()
                               ? "application/octet-stream"
                               : mime.c_str(),
         CURLFORM_FILE, name.c_str(),
         CURLFORM_END);
    } else {
      String val = var_val.toString();
      auto postval = val.data();

      if (!RuntimeOption::PHP7_DisallowUnsafeCurlUploads &&
          !m_safeUpload &&
          *postval == '@' &&
          strlen(postval) == val.size()) {
        /* Given a string like:
         *   "@/foo/bar;type=herp/derp;filename=ponies\0"
         * - Temporarily convert to:
         *   "@/foo/bar\0type=herp/derp\0filename=ponies\0"
         * - Pass pointers to the relevant null-terminated substrings to
         *   curl_formadd
         * - Revert changes to postval at the end
         */
        raise_deprecated(
          "curl_setopt(): The usage of the @filename API for file uploading "
          "is deprecated. Please use the CURLFile class instead"
        );

        if (val.get()->hasMultipleRefs()) {
          val = String::attach(
            StringData::Make(val.data(), val.size(), CopyString));
        }
        auto slice = val.bufferSlice();
        char* mutablePostval = slice.begin() + 1;
        char* type = strstr(mutablePostval, ";type=");
        char* filename = strstr(mutablePostval, ";filename=");

        if (type) { *type = '\0'; }
        if (filename) { *filename = '\0'; }

        String localName = File::TranslatePath(mutablePostval);

        /* The arguments after _NAMELENGTH and _CONTENTSLENGTH
         * must be explicitly cast to long in curl_formadd
         * use since curl needs a long not an int. */
        m_error_no = (CURLcode)curl_formadd
          (&first, &last,
           CURLFORM_COPYNAME, key.data(),
           CURLFORM_NAMELENGTH, (long)key.size(),
           CURLFORM_FILENAME, filename
                              ? filename + sizeof(";filename=") - 1
                              : postval,
           CURLFORM_CONTENTTYPE, type
                                 ? type + sizeof(";type=") - 1
                                 : "application/octet-stream",
           CURLFORM_FILE, localName.c_str(),
           CURLFORM_END);

        if (type) { *type = ';'; }
        if (filename) { *filename = ';'; }
      } else {
        m_error_no = (CURLcode)curl_formadd
          (&first, &last,
           CURLFORM_COPYNAME, key.data(),
           CURLFORM_NAMELENGTH, (long)key.size(),
           CURLFORM_COPYCONTENTS, postval,
           CURLFORM_CONTENTSLENGTH,(long)val.size(),
           CURLFORM_END);
      }
    }
  }

  if (m_error_no != CURLE_OK) {
    return false;
  }
  m_to_free->post.push_back(first);
  m_error_no = curl_easy_setopt(m_cp, CURLOPT_HTTPPOST, first);
  return true;
}

bool CurlResource::isFileOption(long option) {
  return (option == CURLOPT_FILE) ||
         (option == CURLOPT_INFILE) ||
         (option == CURLOPT_WRITEHEADER) ||
         (option == CURLOPT_STDERR);
}

inline bool checkWritable(const std::string& mode) {
  // This check is really hinky, but it's what PHP does. :/
  if ((mode.find('r') != 0) || (mode.find('+') == 1)) {
    return true;
  }
  raise_warning("curl_setopt(): the provided file handle is not writable");
  return false;
}

bool CurlResource::setFileOption(long option, const req::ptr<File>& fp) {
  assertx(isFileOption(option));
  if (option == CURLOPT_FILE) {
    if (!checkWritable(fp->getMode())) return false;
    m_write.fp = fp;
    m_write.method = PHP_CURL_FILE;
    return true;
  }
  if (option == CURLOPT_WRITEHEADER) {
    if (!checkWritable(fp->getMode())) return false;
    m_write_header.fp = fp;
    m_write_header.method = PHP_CURL_FILE;
    return true;
  }

  if (option == CURLOPT_INFILE) {
    m_read.fp = fp;
    m_emptyPost = false;
    return true;
  }

  assertx(option == CURLOPT_STDERR);
  auto pf = dyn_cast<PlainFile>(fp);
  if (!pf) {
    return false;
  }
  auto innerFp = pf->getStream();
  if (!innerFp) {
    return false;
  }
  m_error_no = curl_easy_setopt(m_cp, (CURLoption)option, innerFp);
  return true;
}

bool CurlResource::isStringListOption(long option) {
  switch (option) {
    case CURLOPT_HTTP200ALIASES:
    case CURLOPT_HTTPHEADER:
    case CURLOPT_POSTQUOTE:
    case CURLOPT_PREQUOTE:
    case CURLOPT_QUOTE:
    case CURLOPT_TELNETOPTIONS:
#if LIBCURL_VERSION_NUM >= 0x071400 /* Available since 7.20.0 */
    case CURLOPT_MAIL_RCPT:
#endif
#if LIBCURL_VERSION_NUM >= 0x071503 /* Available since 7.21.3 */
    case CURLOPT_RESOLVE:
#endif
#if LIBCURL_VERSION_NUM >= 0x072500 /* Available since 7.37.0 */
    case CURLOPT_PROXYHEADER:
#endif
#if LIBCURL_VERSION_NUM >= 0x073100 /* Available since 7.49.0 */
    case CURLOPT_CONNECT_TO:
#endif
      return true;
    default:
      return false;
  }
}

bool CurlResource::setStringListOption(long option, const Variant& value) {
  if (!value.isArray() && !value.is(KindOfObject)) {
    raise_warning("You must pass either an object or an array with "
                  "the CURLOPT_HTTPHEADER, CURLOPT_PROXYHEADER, CURLOPT_QUOTE, "
                  "CURLOPT_HTTP200ALIASES, CURLOPT_POSTQUOTE "
                  "and CURLOPT_RESOLVE arguments");
    return false;
  }

  Array arr = value.toArray();
  curl_slist *slist = nullptr;
  for (ArrayIter iter(arr); iter; ++iter) {
    String key = iter.first().toString();
    String val = iter.second().toString();

    slist = curl_slist_append(slist, val.c_str());
    if (!slist) {
      raise_warning("Could not build curl_slist");
      return false;
    }
  }

  m_to_free->slist.push_back(slist);
  m_error_no = curl_easy_setopt(m_cp, (CURLoption)option, slist);
  return true;
}

bool CurlResource::isNonCurlOption(long option) {
  return (option == CURLOPT_RETURNTRANSFER) ||
         (option == CURLOPT_BINARYTRANSFER) ||
         (option == CURLOPT_WRITEFUNCTION) ||
         (option == CURLOPT_READFUNCTION) ||
         (option == CURLOPT_HEADERFUNCTION) ||
         (option == CURLOPT_PROGRESSFUNCTION) ||
         (option == CURLOPT_FB_TLS_VER_MAX) ||
         (option == CURLOPT_FB_TLS_CIPHER_SPEC) ||
         (option == CURLOPT_SAFE_UPLOAD);
}

bool CurlResource::setNonCurlOption(long option, const Variant& value) {
  assertx(isNonCurlOption(option));
  switch (option) {
    case CURLOPT_RETURNTRANSFER:
      m_write.method = value.toBoolean() ? PHP_CURL_RETURN : PHP_CURL_STDOUT;
      return true;
    case CURLOPT_BINARYTRANSFER:
      m_write.type = value.toBoolean() ? PHP_CURL_BINARY : PHP_CURL_ASCII;
      return true;
    case CURLOPT_WRITEFUNCTION:
      m_write.callback = value;
      m_write.method = PHP_CURL_USER;
      return true;
    case CURLOPT_READFUNCTION:
      m_read.callback = value;
      m_read.method = PHP_CURL_USER;
      m_emptyPost = false;
      return true;
    case CURLOPT_HEADERFUNCTION:
      m_write_header.callback = value;
      m_write_header.method = PHP_CURL_USER;
      return true;
    case CURLOPT_PROGRESSFUNCTION:
      m_progress_callback = value;
      curl_easy_setopt(m_cp, CURLOPT_PROGRESSDATA, (void*) this);
      curl_easy_setopt(m_cp, CURLOPT_PROGRESSFUNCTION, curl_progress);
      return true;
    case CURLOPT_FB_TLS_VER_MAX:
      if (value.isInteger()) {
        auto val = value.toInt64();
        if (val == CURLOPT_FB_TLS_VER_MAX_1_0 ||
            val == CURLOPT_FB_TLS_VER_MAX_1_1 ||
            val == CURLOPT_FB_TLS_VER_MAX_NONE) {
          return true;
        }
      }
      raise_warning("You must pass CURLOPT_FB_TLS_VER_MAX_1_0, "
                    "CURLOPT_FB_TLS_VER_MAX_1_1 or "
                    "CURLOPT_FB_TLS_VER_MAX_NONE with "
                    "CURLOPT_FB_TLS_VER_MAX");
      return false;
    case CURLOPT_FB_TLS_CIPHER_SPEC:
      if (!value.isString() || value.toString().empty()) {
        raise_warning("CURLOPT_FB_TLS_CIPHER_SPEC requires a non-empty string");
      }
      return false;
    case CURLOPT_SAFE_UPLOAD:
      if (RuntimeOption::PHP7_DisallowUnsafeCurlUploads &&
          value.toInt64() == 0) {
        raise_warning(
          "curl_setopt(): Disabling safe uploads is no longer supported"
        );
        return false;
      }
      m_safeUpload = value.toBoolean();
      return true;
  }
  return false;
}

inline int64_t minTimeoutImpl(int64_t timeout, int64_t multiple) {
  auto info = RequestInfo::s_requestInfo.getNoCheck();
  auto& data = info->m_reqInjectionData;
  if (!data.getTimeout()) {
    return timeout;
  }
  return std::min<int64_t>(multiple * data.getRemainingTime(), timeout);
}

int64_t CurlResource::minTimeout(int64_t timeout) {
  return minTimeoutImpl(timeout, 1);
}

int64_t CurlResource::minTimeoutMS(int64_t timeout) {
  return minTimeoutImpl(timeout, 1000);
}

void CurlResource::handle_exception() {
  assertx(!m_exception);
  try {
    throw;
  } catch (const Object& e) {
    m_exception.emplace(e);
  } catch (Exception& e) {
    m_exception.emplace(e.clone());
  } catch (std::exception& e) {
    m_exception.emplace(
      new FatalErrorException(0,
                              "Unexpected error in curl callback: %s",
                              e.what())
    );
  } catch (...) {
    m_exception.emplace(
      new FatalErrorException("Unknown error in curl callback")
    );
  }
}

size_t CurlResource::curl_read(char *data,
                               size_t size, size_t nmemb, void *ctx) {
  CurlResource *ch = (CurlResource *)ctx;
  if (!ch->m_in_exec) {
    // T29358191: who's calling, and are they dealing with m_exception?
    log_native_stack("unexpected curl_read");
  }
  ReadHandler *t  = &ch->m_read;

  int length = -1;
  try {
    switch (t->method) {
      case PHP_CURL_DIRECT:
        if (t->fp) {
          int data_size = size * nmemb;
          String ret = t->fp->read(data_size);
          length = ret.size();
          if (length) {
            memcpy(data, ret.data(), length);
          }
        }
        break;
      case PHP_CURL_USER: {
        int data_size = size * nmemb;
        ch->m_in_callback = true;
        SCOPE_EXIT {
          ch->m_in_callback = false;
        };
        Variant ret = vm_call_user_func(
          t->callback,
          make_vec_array(Resource(ch), Resource(t->fp), data_size));
        if (ret.isString()) {
          String sret = ret.toString();
          length = data_size < sret.size() ? data_size : sret.size();
          memcpy(data, sret.data(), length);
        }
        break;
      }
    }
  } catch (...) {
    ch->handle_exception();
    return CURL_READFUNC_ABORT;
  }
  return length;
}

size_t CurlResource::curl_write(char *data,
                                size_t size, size_t nmemb, void *ctx) {
  CurlResource *ch = (CurlResource *)ctx;
  if (!ch->m_in_exec) {
    // T29358191: who's calling, and are they dealing with m_exception?
    log_native_stack("unexpected curl_write");
  }
  WriteHandler *t  = &ch->m_write;
  size_t length = size * nmemb;

  try {
    switch (t->method) {
      case PHP_CURL_STDOUT:
        g_context->write(data, length);
        break;
      case PHP_CURL_FILE:
        return t->fp->write(String(data, length, CopyString), length);
      case PHP_CURL_RETURN:
        if (length > 0) {
          t->buf.append(data, (int)length);
        }
        break;
      case PHP_CURL_USER: {
        ch->m_in_callback = true;
        SCOPE_EXIT {
          ch->m_in_callback = false;
        };
        Variant ret = vm_call_user_func(
          t->callback,
          make_vec_array(Resource(ch), String(data, length, CopyString)));
        length = ret.toInt64();
        break;
      }
    }
  } catch (...) {
    ch->handle_exception();
    return 0;
  }
  return length;
}

size_t CurlResource::curl_write_header(char *data,
                                       size_t size, size_t nmemb, void *ctx) {
  CurlResource *ch = (CurlResource *)ctx;
  if (!ch->m_in_exec) {
    // T29358191: who's calling, and are they dealing with m_exception?
    log_native_stack("unexpected curl_write_header");
  }
  WriteHandler *t  = &ch->m_write_header;
  size_t length = size * nmemb;

  try {
    switch (t->method) {
      case PHP_CURL_STDOUT:
        // Handle special case write when we're returning the entire transfer
        if (ch->m_write.method == PHP_CURL_RETURN && length > 0) {
          ch->m_write.buf.append(data, (int)length);
        } else {
          g_context->write(data, length);
        }
        break;
      case PHP_CURL_FILE:
        return t->fp->write(String(data, length, CopyString), length);
      case PHP_CURL_USER: {
        ch->m_in_callback = true;
        SCOPE_EXIT {
          ch->m_in_callback = false;
        };
        Variant ret = vm_call_user_func(
          t->callback,
          make_vec_array(Resource(ch), String(data, length, CopyString)));
        length = ret.toInt64();
        break;
      }
      case PHP_CURL_IGNORE:
        return length;
      default:
        return (size_t)-1;
    }
  } catch (...) {
    ch->handle_exception();
    return 0;
  }
  return length;
}

int CurlResource::curl_debug(CURL* /*cp*/, curl_infotype type, char* buf,
                             size_t buf_len, void* ctx) {
  CurlResource *ch = (CurlResource *)ctx;
  if (type == CURLINFO_HEADER_OUT && buf_len > 0) {
    ch->m_header = String(buf, buf_len, CopyString);
  }
  return 0;
}

int CurlResource::curl_progress(void* p,
                                double dltotal, double dlnow,
                                double ultotal, double ulnow) {
  assertx(p);
  CurlResource* curl = static_cast<CurlResource*>(p);
  if (!curl->m_in_exec) {
    // T29358191: who's calling, and are they dealing with m_exception?
    log_native_stack("unexpected curl_progress");
  }

  VecInit pai(5);
  pai.append(Resource(curl));
  pai.append(dltotal);
  pai.append(dlnow);
  pai.append(ultotal);
  pai.append(ulnow);

  try {
    Variant result = vm_call_user_func(
      curl->m_progress_callback,
      pai.toArray()
    );
    // Both PHP and libcurl are documented as return 0 to continue, non-zero
    // to abort, however this is what Zend actually implements
    return result.toInt64() == 0 ? 0 : 1;
  } catch (...) {
    curl->handle_exception();
    return 1;
  }
}

namespace {
hphp_fast_string_map<X509_STORE*> s_certCache;
folly::SharedMutex s_mutex;
}

bool CurlResource::useCertCache() const {
  auto const isNonEmpty = [&](int64_t option) {
    if (!m_opts.exists(option)) return false;
    Variant untyped_value = m_opts[option];
    if (untyped_value.isString() && !untyped_value.toString().empty()) {
      return true;
    }
    return false;
  };

  if (isNonEmpty(CURLOPT_PROXY) && cainfo(false) != cainfo(true)) {
    return false;
  }
  if (isNonEmpty(CURLOPT_CRLFILE)) {
    return false;
  }
  if (isNonEmpty(CURLOPT_CAPATH)) {
    return false;
  }

  return true;
}

String CurlResource::cainfo(bool proxy) const {
  auto const option = proxy ? CURLOPT_PROXY_CAINFO : CURLOPT_CAINFO;

  static auto const defaultCainfoData = [&] () -> StringData* {
    auto const curlInfo = curl_version_info(CURLVERSION_NOW);
    if (!curlInfo->cainfo) return nullptr;
    return makeStaticString(curlInfo->cainfo);
  }();

  auto const defaultCainfo = defaultCainfoData ? String{defaultCainfoData} : String{};
  if (!m_opts.exists(int64_t(option))) return defaultCainfo;

  Variant untyped_value = m_opts[int64_t(option)];
  if (!untyped_value.isString()) return defaultCainfo;

  auto const string_value = untyped_value.toString();
  if (string_value.empty()) return defaultCainfo;

  return string_value;
}

CURLcode CurlResource::ssl_ctx_callback(CURL *curl, void *sslctx, void *parm) {
  // Set defaults from config.hdf
  CURLcode r = curl_tls_workarounds_cb(curl, sslctx, parm);
  if (r != CURLE_OK) { return r; }

  // Convert params to proper types.
  SSL_CTX* ctx = (SSL_CTX*)sslctx;
  if (ctx == nullptr) {
    raise_warning("supplied argument is not a valid SSL_CTX");
    return CURLE_FAILED_INIT;
  }
  CurlResource* cp = (CurlResource*)parm;
  if (cp == nullptr) {
    raise_warning("supplied argument is not a valid cURL handle resource");
    return CURLE_FAILED_INIT;
  }

  // Load the CA from the cache.
  if (cp->useCertCache()) {
    auto const cainfo = cp->cainfo(false).toCppString();
    if (!cainfo.empty()) {
      auto const store = [&] () -> X509_STORE* {
        {
          folly::SharedMutex::ReadHolder lock(s_mutex);
          auto const iter = s_certCache.find(cainfo);
          if (iter != s_certCache.end()) return iter->second;
        }

        STACK_OF(X509_INFO) *stack;
        BIO *in;

        in = BIO_new_file(cainfo.data(), "r");
        if (!in) return nullptr;
        stack = PEM_X509_INFO_read_bio(in, nullptr, nullptr, (void*)"");
        BIO_free(in);
        if (!stack) return nullptr;

        auto const store = X509_STORE_new();
        if (!store) return nullptr;
        X509_STORE_set_flags(store, X509_V_FLAG_TRUSTED_FIRST);
        X509_STORE_set_flags(store, X509_V_FLAG_PARTIAL_CHAIN);

        unsigned count = 0;
        for (int i = 0; i < sk_X509_INFO_num(stack); i++) {
          X509_INFO* info;
          info = sk_X509_INFO_value(stack, i);
          if (info->x509) {
            if (!X509_STORE_add_cert(store, info->x509)) {
              X509_STORE_free(store);
              sk_X509_INFO_pop_free(stack, X509_INFO_free);
              return nullptr;
            }
            count++;
          }
          if (info->crl) {
            if (!X509_STORE_add_crl(store, info->crl)) {
              X509_STORE_free(store);
              sk_X509_INFO_pop_free(stack, X509_INFO_free);
              return nullptr;
            }
            count++;
          }
        }
        sk_X509_INFO_pop_free(stack, X509_INFO_free);
        if (count == 0) {
          X509_STORE_free(store);
          return nullptr;
        }

        folly::SharedMutex::WriteHolder lock(s_mutex);
        auto const iter = s_certCache.find(cainfo);
        if (iter != s_certCache.end()) {
          // Lost the race to parse & insert the cached item.
          X509_STORE_free(store);
          return iter->second;
        } else {
          s_certCache.emplace(cainfo, store);
          return store;
        }
      }();
      if (!store) return CURLE_FAILED_INIT;
      SSL_CTX_set1_cert_store(ctx, store);
    }
  }

  // Override cipher specs if necessary.
  if (cp->m_opts.exists(int64_t(CURLOPT_FB_TLS_CIPHER_SPEC))) {
    Variant untyped_value = cp->m_opts[int64_t(CURLOPT_FB_TLS_CIPHER_SPEC)];
    if (untyped_value.isString() && !untyped_value.toString().empty()) {
      SSL_CTX_set_cipher_list(ctx, untyped_value.toString().c_str());
    } else {
      raise_warning("CURLOPT_FB_TLS_CIPHER_SPEC requires a non-empty string");
    }
  }

  // Override the maximum client TLS version if necessary.
  if (cp->m_opts.exists(int64_t(CURLOPT_FB_TLS_VER_MAX))) {
    // Get current options, unsetting the NO_TLSv1_* bits.
    long cur_opts = SSL_CTX_get_options(ctx);
#ifdef SSL_OP_NO_TLSv1_1
    cur_opts &= ~SSL_OP_NO_TLSv1_1;
#endif
#ifdef SSL_OP_NO_TLSv1_2
    cur_opts &= ~SSL_OP_NO_TLSv1_2;
#endif
    int64_t value = cp->m_opts[int64_t(CURLOPT_FB_TLS_VER_MAX)].toInt64();
    if (value == CURLOPT_FB_TLS_VER_MAX_1_0) {
#if defined (SSL_OP_NO_TLSv1_1) && defined (SSL_OP_NO_TLSv1_2)
      cur_opts |= SSL_OP_NO_TLSv1_1 | SSL_OP_NO_TLSv1_2;
#else
      raise_notice("Requesting SSL_OP_NO_TLSv1_1, but this version of "
                   "SSL does not support that option");
#endif
    } else if (value == CURLOPT_FB_TLS_VER_MAX_1_1) {
#ifdef SSL_OP_NO_TLSv1_2
      cur_opts |= SSL_OP_NO_TLSv1_2;
#else
      raise_notice("Requesting SSL_OP_NO_TLSv1_2, but this version of "
                   "SSL does not support that option");
#endif
    } else if (value != CURLOPT_FB_TLS_VER_MAX_NONE) {
      raise_notice("Invalid CURLOPT_FB_TLS_VER_MAX value");
    }
    SSL_CTX_set_options(ctx, cur_opts);
  }

  return CURLE_OK;
}

/////////////////////////////////////////////////////////////////////////////
}

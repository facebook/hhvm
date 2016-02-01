/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/curl/ext_curl.h"
#include "hphp/runtime/ext/curl/curl-multi-await.h"
#include "hphp/runtime/ext/curl/curl-multi-resource.h"
#include "hphp/runtime/ext/curl/curl-pool.h"
#include "hphp/runtime/ext/curl/curl-resource.h"
#include "hphp/runtime/ext/asio/socket-event.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/libevent-http-client.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/extension-registry.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/lock.h"
#include <boost/algorithm/string.hpp>
#include <boost/variant.hpp>
#include <folly/Optional.h>
#include <openssl/ssl.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/multi.h>
#include <memory>
#include <vector>

#define CURLOPT_MUTE -2
#define CURLOPT_PASSWDFUNCTION -3

namespace HPHP {

using std::string;
using std::vector;

///////////////////////////////////////////////////////////////////////////////

#define CHECK_RESOURCE(curl)                                                \
  auto curl = dyn_cast_or_null<CurlResource>(ch);                           \
  if (curl == nullptr) {                                                    \
    raise_warning("supplied argument is not a valid cURL handle resource"); \
    return false;                                                           \
  }                                                                         \

#define CHECK_RESOURCE_RETURN_VOID(curl)                                    \
  auto curl = dyn_cast_or_null<CurlResource>(ch);                           \
  if (curl == nullptr) {                                                    \
    raise_warning("supplied argument is not a valid cURL handle resource"); \
    return;                                                                 \
  }                                                                         \

Variant HHVM_FUNCTION(curl_init, const Variant& url /* = null_string */) {
  if (url.isNull()) {
    return Variant(req::make<CurlResource>(null_string));
  } else {
    return Variant(req::make<CurlResource>(url.toString()));
  }
}

Variant HHVM_FUNCTION(curl_init_pooled,
    const String& poolName,
    const Variant& url /* = null_string */)
{
  bool poolExists = (CurlHandlePool::namedPools.find(poolName.toCppString()) !=
      CurlHandlePool::namedPools.end());
  if (!poolExists) {
    raise_warning("Attempting to use connection pooling without "
                  "specifying an existent connection pool!");
  }
  CurlHandlePool *pool = poolExists ?
    CurlHandlePool::namedPools.at(poolName.toCppString()) : nullptr;

  return url.isNull() ? Variant(req::make<CurlResource>(null_string, pool)) :
         Variant(req::make<CurlResource>(url.toString(), pool));
}

Variant HHVM_FUNCTION(curl_copy_handle, const Resource& ch) {
  CHECK_RESOURCE(curl);
  return Variant(req::make<CurlResource>(curl));
}

const StaticString
  s_version_number("version_number"),
  s_age("age"),
  s_features("features"),
  s_ssl_version_number("ssl_version_number"),
  s_version("version"),
  s_host("host"),
  s_ssl_version("ssl_version"),
  s_libz_version("libz_version"),
  s_protocols("protocols");

Variant HHVM_FUNCTION(curl_version, int uversion /* = CURLVERSION_NOW */) {
  curl_version_info_data *d = curl_version_info((CURLversion)uversion);
  if (d == nullptr) {
    return false;
  }

  ArrayInit ret(9, ArrayInit::Map{});
  ret.set(s_version_number,     (int)d->version_num);
  ret.set(s_age,                d->age);
  ret.set(s_features,           d->features);
  ret.set(s_ssl_version_number, d->ssl_version_num);
  ret.set(s_version,            d->version);
  ret.set(s_host,               d->host);
  ret.set(s_ssl_version,        d->ssl_version);
  ret.set(s_libz_version,       d->libz_version);

  // Add an array of protocols
  char **p = (char **) d->protocols;
  Array protocol_list;
  while (*p != nullptr) {
    protocol_list.append(String(*p++, CopyString));
  }
  ret.set(s_protocols, protocol_list);
  return ret.toVariant();
}

bool HHVM_FUNCTION(curl_setopt, const Resource& ch, int option, const Variant& value) {
  CHECK_RESOURCE(curl);
  return curl->setOption(option, value);
}

bool HHVM_FUNCTION(curl_setopt_array, const Resource& ch, const Array& options) {
  CHECK_RESOURCE(curl);
  for (ArrayIter iter(options); iter; ++iter) {
    if (!curl->setOption(iter.first().toInt32(), iter.second())) {
      return false;
    }
  }
  return true;
}

Variant HHVM_FUNCTION(fb_curl_getopt, const Resource& ch, int64_t opt /* = 0 */) {
  CHECK_RESOURCE(curl);
  return curl->getOption(opt);
}

Variant HHVM_FUNCTION(curl_exec, const Resource& ch) {
  CHECK_RESOURCE(curl);
  return curl->execute();
}

const StaticString
  s_url("url"),
  s_content_type("content_type"),
  s_http_code("http_code"),
  s_header_size("header_size"),
  s_request_size("request_size"),
  s_filetime("filetime"),
  s_ssl_verify_result("ssl_verify_result"),
  s_redirect_count("redirect_count"),
  s_local_port("local_port"),
  s_total_time("total_time"),
  s_namelookup_time("namelookup_time"),
  s_connect_time("connect_time"),
  s_pretransfer_time("pretransfer_time"),
  s_size_upload("size_upload"),
  s_size_download("size_download"),
  s_speed_download("speed_download"),
  s_speed_upload("speed_upload"),
  s_download_content_length("download_content_length"),
  s_upload_content_length("upload_content_length"),
  s_starttransfer_time("starttransfer_time"),
  s_redirect_time("redirect_time"),
  s_request_header("request_header");

Variant HHVM_FUNCTION(curl_getinfo, const Resource& ch, int opt /* = 0 */) {
  CHECK_RESOURCE(curl);
  CURL *cp = curl->get();

  if (opt == 0) {
    char   *s_code;
    long    l_code;
    double  d_code;

    Array ret;
    if (curl_easy_getinfo(cp, CURLINFO_EFFECTIVE_URL, &s_code) == CURLE_OK) {
      ret.set(s_url, String(s_code, CopyString));
    }
    if (curl_easy_getinfo(cp, CURLINFO_CONTENT_TYPE, &s_code) == CURLE_OK) {
      if (s_code != nullptr) {
        ret.set(s_content_type, String(s_code, CopyString));
      } else {
        ret.set(s_content_type, init_null());
      }
    }
    if (curl_easy_getinfo(cp, CURLINFO_HTTP_CODE, &l_code) == CURLE_OK) {
      ret.set(s_http_code, l_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_HEADER_SIZE, &l_code) == CURLE_OK) {
      ret.set(s_header_size, l_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_REQUEST_SIZE, &l_code) == CURLE_OK) {
      ret.set(s_request_size, l_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_FILETIME, &l_code) == CURLE_OK) {
      ret.set(s_filetime, l_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_SSL_VERIFYRESULT, &l_code) ==
        CURLE_OK) {
      ret.set(s_ssl_verify_result, l_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_REDIRECT_COUNT, &l_code) == CURLE_OK) {
      ret.set(s_redirect_count, l_code);
    }
#if LIBCURL_VERSION_NUM >= 0x071500
    if (curl_easy_getinfo(cp, CURLINFO_LOCAL_PORT, &l_code) == CURLE_OK) {
      ret.set(s_local_port, l_code);
    }
#endif
    if (curl_easy_getinfo(cp, CURLINFO_TOTAL_TIME, &d_code) == CURLE_OK) {
      ret.set(s_total_time, d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_NAMELOOKUP_TIME, &d_code) == CURLE_OK) {
      ret.set(s_namelookup_time, d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_CONNECT_TIME, &d_code) == CURLE_OK) {
      ret.set(s_connect_time, d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_PRETRANSFER_TIME, &d_code) ==
        CURLE_OK) {
      ret.set(s_pretransfer_time, d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_SIZE_UPLOAD, &d_code) == CURLE_OK) {
      ret.set(s_size_upload, d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_SIZE_DOWNLOAD, &d_code) == CURLE_OK) {
      ret.set(s_size_download, d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_SPEED_DOWNLOAD, &d_code) == CURLE_OK) {
      ret.set(s_speed_download, d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_SPEED_UPLOAD, &d_code) == CURLE_OK) {
      ret.set(s_speed_upload, d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &d_code) ==
        CURLE_OK) {
      ret.set(s_download_content_length, d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_CONTENT_LENGTH_UPLOAD, &d_code) ==
        CURLE_OK) {
      ret.set(s_upload_content_length, d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_STARTTRANSFER_TIME, &d_code) ==
        CURLE_OK) {
      ret.set(s_starttransfer_time, d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_REDIRECT_TIME, &d_code) == CURLE_OK) {
      ret.set(s_redirect_time, d_code);
    }
    String header = curl->getHeader();
    if (!header.empty()) {
      ret.set(s_request_header, header);
    }
    return ret;
  }

  switch (opt) {
  case CURLINFO_PRIVATE:
  case CURLINFO_EFFECTIVE_URL:
  case CURLINFO_CONTENT_TYPE: {
    char *s_code = nullptr;
    if (curl_easy_getinfo(cp, (CURLINFO)opt, &s_code) == CURLE_OK &&
        s_code) {
      return String(s_code, CopyString);
    }
    return false;
  }
  case CURLINFO_HTTP_CODE:
  case CURLINFO_HEADER_SIZE:
  case CURLINFO_REQUEST_SIZE:
  case CURLINFO_FILETIME:
  case CURLINFO_SSL_VERIFYRESULT:
#if LIBCURL_VERSION_NUM >= 0x071500
  case CURLINFO_LOCAL_PORT:
#endif
  case CURLINFO_REDIRECT_COUNT: {
    long code = 0;
    if (curl_easy_getinfo(cp, (CURLINFO)opt, &code) == CURLE_OK) {
      return code;
    }
    return false;
  }
  case CURLINFO_TOTAL_TIME:
  case CURLINFO_NAMELOOKUP_TIME:
  case CURLINFO_CONNECT_TIME:
  case CURLINFO_PRETRANSFER_TIME:
  case CURLINFO_SIZE_UPLOAD:
  case CURLINFO_SIZE_DOWNLOAD:
  case CURLINFO_SPEED_DOWNLOAD:
  case CURLINFO_SPEED_UPLOAD:
  case CURLINFO_CONTENT_LENGTH_DOWNLOAD:
  case CURLINFO_CONTENT_LENGTH_UPLOAD:
  case CURLINFO_STARTTRANSFER_TIME:
  case CURLINFO_REDIRECT_TIME: {
    double code = 0.0;
    if (curl_easy_getinfo(cp, (CURLINFO)opt, &code) == CURLE_OK) {
      return code;
    }
    return false;
  }
  case CURLINFO_HEADER_OUT:
    {
      String header = curl->getHeader();
      if (!header.empty()) {
        return header;
      }
      return false;
    }
  }

  return init_null();
}

Variant HHVM_FUNCTION(curl_errno, const Resource& ch) {
  CHECK_RESOURCE(curl);
  return curl->getError();
}

Variant HHVM_FUNCTION(curl_error, const Resource& ch) {
  CHECK_RESOURCE(curl);
  return curl->getErrorString();
}

String HHVM_FUNCTION(curl_strerror, int code) {
  return curl_easy_strerror((CURLcode)code);
}

Variant HHVM_FUNCTION(curl_close, const Resource& ch) {
  CHECK_RESOURCE(curl);
  curl->close();
  return init_null();
}

void HHVM_FUNCTION(curl_reset, const Resource& ch) {
  CHECK_RESOURCE_RETURN_VOID(curl);
  curl->reset();
}

///////////////////////////////////////////////////////////////////////////////

#define CURLM_ARG_WARNING "expects parameter 1 to be cURL multi resource"

#define CHECK_MULTI_RESOURCE(curlm)                                      \
  auto curlm = dyn_cast_or_null<CurlMultiResource>(mh);                  \
  if (!curlm || curlm->isInvalid()) {                                    \
    raise_warning(CURLM_ARG_WARNING);                                    \
    return init_null();                                                  \
  }

#define CHECK_MULTI_RESOURCE_RETURN_VOID(curlm) \
  auto curlm = dyn_cast_or_null<CurlMultiResource>(mh);                  \
  if (!curlm || curlm->isInvalid()) {                                    \
    raise_warning(CURLM_ARG_WARNING);                                    \
    return;                                                              \
  }

#define CHECK_MULTI_RESOURCE_THROW(curlm)                               \
  auto curlm = dyn_cast_or_null<CurlMultiResource>(mh);                 \
  if (!curlm || curlm->isInvalid()) {                                   \
    SystemLib::throwExceptionObject(CURLM_ARG_WARNING);                 \
  }

Resource HHVM_FUNCTION(curl_multi_init) {
  return Resource(req::make<CurlMultiResource>());
}

Variant HHVM_FUNCTION(curl_multi_strerror, int64_t code) {
  const char *str = curl_multi_strerror((CURLMcode)code);
  if (str) {
    return str;
  } else {
    return init_null();
  }
}

Variant HHVM_FUNCTION(curl_multi_add_handle, const Resource& mh, const Resource& ch) {
  CHECK_MULTI_RESOURCE(curlm);
  auto curle = cast<CurlResource>(ch);
  curlm->add(ch);
  return curl_multi_add_handle(curlm->get(), curle->get());
}

Variant HHVM_FUNCTION(curl_multi_remove_handle, const Resource& mh, const Resource& ch) {
  CHECK_MULTI_RESOURCE(curlm);
  auto curle = cast<CurlResource>(ch);
  curlm->remove(curle);
  return curl_multi_remove_handle(curlm->get(), curle->get());
}

Variant HHVM_FUNCTION(curl_multi_exec, const Resource& mh, VRefParam still_running) {
  CHECK_MULTI_RESOURCE(curlm);
  int running = 0;
  IOStatusHelper io("curl_multi_exec");
  SYNC_VM_REGS_SCOPED();
  int result = curl_multi_perform(curlm->get(), &running);
  curlm->check_exceptions();
  still_running.assignIfRef(running);
  return result;
}

bool HHVM_FUNCTION(curl_multi_setopt, const Resource& mh,
                   int option, const Variant& value) {
  CHECK_MULTI_RESOURCE_THROW(curlm);
  return curlm->setOption(option, value);
}


/* Fallback implementation of curl_multi_select()
 *
 * This allows the OSS build to work with older package
 * versions of libcurl, but will fail with file descriptors
 * over 1024.
 */
UNUSED
static void hphp_curl_multi_select(CURLM *mh, int timeout_ms, int *ret) {
  fd_set read_fds, write_fds, except_fds;
  int maxfds, nfds = -1;
  struct timeval tv;

  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_ZERO(&except_fds);

  tv.tv_sec  =  timeout_ms / 1000;
  tv.tv_usec = (timeout_ms * 1000) % 1000000;

  curl_multi_fdset(mh, &read_fds, &write_fds, &except_fds, &maxfds);
  if (maxfds < 1024) {
    nfds = select(maxfds + 1, &read_fds, &write_fds, &except_fds, &tv);
  } else {
    /* fd_set can only hold sockets from 0 to 1023,
     * anything higher is ignored by FD_SET()
     * avoid "unexplained" behavior by failing outright
     */
    raise_warning("libcurl versions < 7.28.0 do not support selecting on "
                  "file descriptors of 1024 or higher.");
  }
  if (ret) {
    *ret = nfds;
  }
}

#ifndef HAVE_CURL_MULTI_SELECT
# ifdef HAVE_CURL_MULTI_WAIT
#  define curl_multi_select_func(mh, tm, ret) curl_multi_wait((mh), nullptr, 0, (tm), (ret))
# else
#  define curl_multi_select_func hphp_curl_multi_select
# endif
#else
#define curl_multi_select_func(mh, tm, ret) curl_multi_wait((mh), nullptr, 0, (tm), (ret))
#endif

Variant HHVM_FUNCTION(curl_multi_select, const Resource& mh,
                                         double timeout /* = 1.0 */) {
  CHECK_MULTI_RESOURCE(curlm);
  int ret;
  unsigned long timeout_ms = (unsigned long)(timeout * 1000.0);
  IOStatusHelper io("curl_multi_select");
  curl_multi_select_func(curlm->get(), timeout_ms, &ret);
  return ret;
}

Object HHVM_FUNCTION(curl_multi_await, const Resource& mh,
                                       double timeout /*=1.0*/) {
  CHECK_MULTI_RESOURCE_THROW(curlm);
  auto ev = new CurlMultiAwait(curlm, timeout);
  try {
    return Object{ev->getWaitHandle()};
  } catch (...) {
    assert(false);
    ev->abandon();
    throw;
  }
}

Variant HHVM_FUNCTION(curl_multi_getcontent, const Resource& ch) {
  CHECK_RESOURCE(curl);
  return curl->getContents();
}

Array curl_convert_fd_to_stream(fd_set *fd, int max_fd) {
  Array ret = Array::Create();
  for (int i=0; i<=max_fd; i++) {
    if (FD_ISSET(i, fd)) {
      ret.append(Variant(req::make<BuiltinFile>(i)));
    }
  }
  return ret;
}

Variant HHVM_FUNCTION(fb_curl_multi_fdset, const Resource& mh,
                      VRefParam read_fd_set,
                      VRefParam write_fd_set,
                      VRefParam exc_fd_set,
                      VRefParam max_fd /* = null_object */) {
  CHECK_MULTI_RESOURCE(curlm);

  fd_set read_set;
  fd_set write_set;
  fd_set exc_set;
  int max = 0;

  FD_ZERO(&read_set);
  FD_ZERO(&write_set);
  FD_ZERO(&exc_set);

  int r = curl_multi_fdset(curlm->get(), &read_set, &write_set, &exc_set, &max);
  read_fd_set.assignIfRef(curl_convert_fd_to_stream(&read_set, max));
  write_fd_set.assignIfRef(curl_convert_fd_to_stream(&write_set, max));
  exc_fd_set.assignIfRef(curl_convert_fd_to_stream(&exc_set, max));
  max_fd.assignIfRef(max);

  return r;
}

const StaticString
  s_msg("msg"),
  s_result("result"),
  s_handle("handle");

Variant HHVM_FUNCTION(curl_multi_info_read, const Resource& mh,
                      VRefParam msgs_in_queue /* = null */) {
  CHECK_MULTI_RESOURCE(curlm);

  int queued_msgs;
  CURLMsg *tmp_msg = curl_multi_info_read(curlm->get(), &queued_msgs);
  curlm->check_exceptions();
  if (tmp_msg == nullptr) {
    return false;
  }
  msgs_in_queue.assignIfRef(queued_msgs);

  Array ret;
  ret.set(s_msg, tmp_msg->msg);
  ret.set(s_result, tmp_msg->data.result);
  Resource curle = curlm->find(tmp_msg->easy_handle);
  if (!curle.isNull()) {
    ret.set(s_handle, curle);
  }
  return ret;
}

Variant HHVM_FUNCTION(curl_multi_close, const Resource& mh) {
  CHECK_MULTI_RESOURCE(curlm);
  curlm->close();
  return init_null();
}

///////////////////////////////////////////////////////////////////////////////

static int s_poolSize, s_reuseLimit, s_getTimeout;
static std::string s_namedPools;

class CurlExtension final : public Extension {
 public:
  CurlExtension() : Extension("curl") {}
  void moduleInit() override {
#if LIBCURL_VERSION_NUM >= 0x071500
    HHVM_RC_INT_SAME(CURLINFO_LOCAL_PORT);
#endif

#if LIBCURL_VERSION_NUM >= 0x071002
    HHVM_RC_INT_SAME(CURLOPT_TIMEOUT_MS);
    HHVM_RC_INT_SAME(CURLOPT_CONNECTTIMEOUT_MS);
#endif

#if LIBCURL_VERSION_NUM >= 0x071000 /* Available since 7.16.0 */
    HHVM_RC_INT_SAME(CURLMOPT_PIPELINING);
#endif

#if LIBCURL_VERSION_NUM >= 0x071003 /* Available since 7.16.3 */
    HHVM_RC_INT_SAME(CURLMOPT_MAXCONNECTS);
#endif

    HHVM_RC_INT_SAME(CURLAUTH_ANY);
    HHVM_RC_INT_SAME(CURLAUTH_ANYSAFE);
    HHVM_RC_INT_SAME(CURLAUTH_BASIC);
    HHVM_RC_INT_SAME(CURLAUTH_DIGEST);
    HHVM_RC_INT_SAME(CURLAUTH_GSSNEGOTIATE);
    HHVM_RC_INT_SAME(CURLAUTH_NTLM);
    HHVM_RC_INT_SAME(CURLCLOSEPOLICY_CALLBACK);
    HHVM_RC_INT_SAME(CURLCLOSEPOLICY_LEAST_RECENTLY_USED);
    HHVM_RC_INT_SAME(CURLCLOSEPOLICY_LEAST_TRAFFIC);
    HHVM_RC_INT_SAME(CURLCLOSEPOLICY_OLDEST);
    HHVM_RC_INT_SAME(CURLCLOSEPOLICY_SLOWEST);
    HHVM_RC_INT_SAME(CURLE_ABORTED_BY_CALLBACK);
    HHVM_RC_INT_SAME(CURLE_BAD_CALLING_ORDER);
    HHVM_RC_INT_SAME(CURLE_BAD_CONTENT_ENCODING);
    HHVM_RC_INT_SAME(CURLE_BAD_FUNCTION_ARGUMENT);
    HHVM_RC_INT_SAME(CURLE_BAD_PASSWORD_ENTERED);
    HHVM_RC_INT_SAME(CURLE_COULDNT_CONNECT);
    HHVM_RC_INT_SAME(CURLE_COULDNT_RESOLVE_HOST);
    HHVM_RC_INT_SAME(CURLE_COULDNT_RESOLVE_PROXY);
    HHVM_RC_INT_SAME(CURLE_FAILED_INIT);
    HHVM_RC_INT_SAME(CURLE_FILESIZE_EXCEEDED);
    HHVM_RC_INT_SAME(CURLE_FILE_COULDNT_READ_FILE);
    HHVM_RC_INT_SAME(CURLE_FTP_ACCESS_DENIED);
    HHVM_RC_INT_SAME(CURLE_FTP_BAD_DOWNLOAD_RESUME);
    HHVM_RC_INT_SAME(CURLE_FTP_CANT_GET_HOST);
    HHVM_RC_INT_SAME(CURLE_FTP_CANT_RECONNECT);
    HHVM_RC_INT_SAME(CURLE_FTP_COULDNT_GET_SIZE);
    HHVM_RC_INT_SAME(CURLE_FTP_COULDNT_RETR_FILE);
    HHVM_RC_INT_SAME(CURLE_FTP_COULDNT_SET_ASCII);
    HHVM_RC_INT_SAME(CURLE_FTP_COULDNT_SET_BINARY);
    HHVM_RC_INT_SAME(CURLE_FTP_COULDNT_STOR_FILE);
    HHVM_RC_INT_SAME(CURLE_FTP_COULDNT_USE_REST);
    HHVM_RC_INT_SAME(CURLE_FTP_PORT_FAILED);
    HHVM_RC_INT_SAME(CURLE_FTP_QUOTE_ERROR);
    HHVM_RC_INT_SAME(CURLE_FTP_SSL_FAILED);
    HHVM_RC_INT_SAME(CURLE_FTP_USER_PASSWORD_INCORRECT);
    HHVM_RC_INT_SAME(CURLE_FTP_WEIRD_227_FORMAT);
    HHVM_RC_INT_SAME(CURLE_FTP_WEIRD_PASS_REPLY);
    HHVM_RC_INT_SAME(CURLE_FTP_WEIRD_PASV_REPLY);
    HHVM_RC_INT_SAME(CURLE_FTP_WEIRD_SERVER_REPLY);
    HHVM_RC_INT_SAME(CURLE_FTP_WEIRD_USER_REPLY);
    HHVM_RC_INT_SAME(CURLE_FTP_WRITE_ERROR);
    HHVM_RC_INT_SAME(CURLE_FUNCTION_NOT_FOUND);
    HHVM_RC_INT_SAME(CURLE_GOT_NOTHING);
    HHVM_RC_INT_SAME(CURLE_HTTP_NOT_FOUND);
    HHVM_RC_INT_SAME(CURLE_HTTP_PORT_FAILED);
    HHVM_RC_INT_SAME(CURLE_HTTP_POST_ERROR);
    HHVM_RC_INT_SAME(CURLE_HTTP_RANGE_ERROR);
    HHVM_RC_INT_SAME(CURLE_LDAP_CANNOT_BIND);
    HHVM_RC_INT_SAME(CURLE_LDAP_INVALID_URL);
    HHVM_RC_INT_SAME(CURLE_LDAP_SEARCH_FAILED);
    HHVM_RC_INT_SAME(CURLE_LIBRARY_NOT_FOUND);
    HHVM_RC_INT_SAME(CURLE_MALFORMAT_USER);
    HHVM_RC_INT_SAME(CURLE_OBSOLETE);
    HHVM_RC_INT_SAME(CURLE_OK);
    HHVM_RC_INT(CURLE_OPERATION_TIMEDOUT, CURLE_OPERATION_TIMEOUTED);
    HHVM_RC_INT_SAME(CURLE_OPERATION_TIMEOUTED);
    HHVM_RC_INT_SAME(CURLE_OUT_OF_MEMORY);
    HHVM_RC_INT_SAME(CURLE_PARTIAL_FILE);
    HHVM_RC_INT_SAME(CURLE_READ_ERROR);
    HHVM_RC_INT_SAME(CURLE_RECV_ERROR);
    HHVM_RC_INT_SAME(CURLE_SEND_ERROR);
    HHVM_RC_INT_SAME(CURLE_SHARE_IN_USE);
    HHVM_RC_INT_SAME(CURLE_SSL_CACERT);
    HHVM_RC_INT_SAME(CURLE_SSL_CERTPROBLEM);
    HHVM_RC_INT_SAME(CURLE_SSL_CIPHER);
    HHVM_RC_INT_SAME(CURLE_SSL_CONNECT_ERROR);
    HHVM_RC_INT_SAME(CURLE_SSL_ENGINE_NOTFOUND);
    HHVM_RC_INT_SAME(CURLE_SSL_ENGINE_SETFAILED);
    HHVM_RC_INT_SAME(CURLE_SSL_PEER_CERTIFICATE);
    HHVM_RC_INT_SAME(CURLE_TELNET_OPTION_SYNTAX);
    HHVM_RC_INT_SAME(CURLE_TOO_MANY_REDIRECTS);
    HHVM_RC_INT_SAME(CURLE_UNKNOWN_TELNET_OPTION);
    HHVM_RC_INT_SAME(CURLE_UNSUPPORTED_PROTOCOL);
    HHVM_RC_INT_SAME(CURLE_URL_MALFORMAT);
    HHVM_RC_INT_SAME(CURLE_URL_MALFORMAT_USER);
    HHVM_RC_INT_SAME(CURLE_WRITE_ERROR);
    HHVM_RC_INT_SAME(CURLFTPAUTH_DEFAULT);
    HHVM_RC_INT_SAME(CURLFTPAUTH_SSL);
    HHVM_RC_INT_SAME(CURLFTPAUTH_TLS);
    HHVM_RC_INT_SAME(CURLFTPSSL_ALL);
    HHVM_RC_INT_SAME(CURLFTPSSL_CONTROL);
    HHVM_RC_INT_SAME(CURLFTPSSL_NONE);
    HHVM_RC_INT_SAME(CURLFTPSSL_TRY);
    HHVM_RC_INT_SAME(CURLINFO_CONNECT_TIME);
    HHVM_RC_INT_SAME(CURLINFO_CONTENT_LENGTH_DOWNLOAD);
    HHVM_RC_INT_SAME(CURLINFO_CONTENT_LENGTH_UPLOAD);
    HHVM_RC_INT_SAME(CURLINFO_CONTENT_TYPE);
    HHVM_RC_INT_SAME(CURLINFO_EFFECTIVE_URL);
    HHVM_RC_INT_SAME(CURLINFO_FILETIME);
    HHVM_RC_INT_SAME(CURLINFO_HEADER_OUT);
    HHVM_RC_INT_SAME(CURLINFO_HEADER_SIZE);
    HHVM_RC_INT_SAME(CURLINFO_HTTP_CODE);
    HHVM_RC_INT_SAME(CURLINFO_NAMELOOKUP_TIME);
    HHVM_RC_INT_SAME(CURLINFO_PRETRANSFER_TIME);
    HHVM_RC_INT_SAME(CURLINFO_PRIVATE);
    HHVM_RC_INT_SAME(CURLINFO_REDIRECT_COUNT);
    HHVM_RC_INT_SAME(CURLINFO_REDIRECT_TIME);
    HHVM_RC_INT_SAME(CURLINFO_REQUEST_SIZE);
    HHVM_RC_INT_SAME(CURLINFO_RESPONSE_CODE);
    HHVM_RC_INT_SAME(CURLINFO_SIZE_DOWNLOAD);
    HHVM_RC_INT_SAME(CURLINFO_SIZE_UPLOAD);
    HHVM_RC_INT_SAME(CURLINFO_SPEED_DOWNLOAD);
    HHVM_RC_INT_SAME(CURLINFO_SPEED_UPLOAD);
    HHVM_RC_INT_SAME(CURLINFO_SSL_VERIFYRESULT);
    HHVM_RC_INT_SAME(CURLINFO_STARTTRANSFER_TIME);
    HHVM_RC_INT_SAME(CURLINFO_TOTAL_TIME);
    HHVM_RC_INT_SAME(CURLMSG_DONE);
    HHVM_RC_INT_SAME(CURLM_BAD_EASY_HANDLE);
    HHVM_RC_INT_SAME(CURLM_BAD_HANDLE);
    HHVM_RC_INT_SAME(CURLM_CALL_MULTI_PERFORM);
    HHVM_RC_INT_SAME(CURLM_INTERNAL_ERROR);
    HHVM_RC_INT_SAME(CURLM_OK);
    HHVM_RC_INT_SAME(CURLM_OUT_OF_MEMORY);
    HHVM_RC_INT_SAME(CURLOPT_AUTOREFERER);
    HHVM_RC_INT_SAME(CURLOPT_BINARYTRANSFER);
    HHVM_RC_INT_SAME(CURLOPT_BUFFERSIZE);
    HHVM_RC_INT_SAME(CURLOPT_CAINFO);
    HHVM_RC_INT_SAME(CURLOPT_CAPATH);
    HHVM_RC_INT_SAME(CURLOPT_CLOSEPOLICY);
    HHVM_RC_INT_SAME(CURLOPT_CONNECTTIMEOUT);
    HHVM_RC_INT_SAME(CURLOPT_COOKIE);
    HHVM_RC_INT_SAME(CURLOPT_COOKIEFILE);
    HHVM_RC_INT_SAME(CURLOPT_COOKIEJAR);
    HHVM_RC_INT_SAME(CURLOPT_COOKIESESSION);
    HHVM_RC_INT_SAME(CURLOPT_CRLF);
    HHVM_RC_INT_SAME(CURLOPT_CUSTOMREQUEST);
    HHVM_RC_INT_SAME(CURLOPT_DNS_CACHE_TIMEOUT);
    HHVM_RC_INT_SAME(CURLOPT_DNS_USE_GLOBAL_CACHE);
    HHVM_RC_INT_SAME(CURLOPT_EGDSOCKET);
    HHVM_RC_INT_SAME(CURLOPT_ENCODING);
    HHVM_RC_INT_SAME(CURLOPT_FAILONERROR);
    HHVM_RC_INT_SAME(CURLOPT_FILE);
    HHVM_RC_INT_SAME(CURLOPT_FILETIME);
    HHVM_RC_INT_SAME(CURLOPT_FOLLOWLOCATION);
    HHVM_RC_INT_SAME(CURLOPT_FORBID_REUSE);
    HHVM_RC_INT_SAME(CURLOPT_FRESH_CONNECT);
    HHVM_RC_INT_SAME(CURLOPT_FTPAPPEND);
    HHVM_RC_INT_SAME(CURLOPT_FTPLISTONLY);
    HHVM_RC_INT_SAME(CURLOPT_FTPPORT);
    HHVM_RC_INT_SAME(CURLOPT_FTPSSLAUTH);
    HHVM_RC_INT_SAME(CURLOPT_FTP_CREATE_MISSING_DIRS);
    HHVM_RC_INT_SAME(CURLOPT_FTP_SSL);
    HHVM_RC_INT_SAME(CURLOPT_FTP_USE_EPRT);
    HHVM_RC_INT_SAME(CURLOPT_FTP_USE_EPSV);
    HHVM_RC_INT_SAME(CURLOPT_HEADER);
    HHVM_RC_INT_SAME(CURLOPT_HEADERFUNCTION);
    HHVM_RC_INT_SAME(CURLOPT_HTTP200ALIASES);
    HHVM_RC_INT_SAME(CURLOPT_HTTPAUTH);
    HHVM_RC_INT_SAME(CURLOPT_HTTPGET);
    HHVM_RC_INT_SAME(CURLOPT_HTTPHEADER);
    HHVM_RC_INT_SAME(CURLOPT_HTTPPROXYTUNNEL);
    HHVM_RC_INT_SAME(CURLOPT_HTTP_VERSION);
    HHVM_RC_INT_SAME(CURLOPT_INFILE);
    HHVM_RC_INT_SAME(CURLOPT_INFILESIZE);
    HHVM_RC_INT_SAME(CURLOPT_INTERFACE);
    HHVM_RC_INT_SAME(CURLOPT_IPRESOLVE);
    HHVM_RC_INT_SAME(CURLOPT_KRB4LEVEL);
    HHVM_RC_INT_SAME(CURLOPT_LOW_SPEED_LIMIT);
    HHVM_RC_INT_SAME(CURLOPT_LOW_SPEED_TIME);
    HHVM_RC_INT_SAME(CURLOPT_MAXCONNECTS);
    HHVM_RC_INT_SAME(CURLOPT_MAXREDIRS);
    HHVM_RC_INT_SAME(CURLOPT_MUTE);
    HHVM_RC_INT_SAME(CURLOPT_NETRC);
    HHVM_RC_INT_SAME(CURLOPT_NOBODY);
    HHVM_RC_INT_SAME(CURLOPT_NOPROGRESS);
    HHVM_RC_INT_SAME(CURLOPT_NOSIGNAL);
    HHVM_RC_INT_SAME(CURLOPT_PASSWDFUNCTION);
    HHVM_RC_INT_SAME(CURLOPT_PORT);
    HHVM_RC_INT_SAME(CURLOPT_POST);
    HHVM_RC_INT_SAME(CURLOPT_POSTFIELDS);
    HHVM_RC_INT_SAME(CURLOPT_POSTREDIR);
    HHVM_RC_INT_SAME(CURLOPT_POSTQUOTE);
    HHVM_RC_INT_SAME(CURLOPT_PROTOCOLS);
    HHVM_RC_INT_SAME(CURLOPT_REDIR_PROTOCOLS);
    HHVM_RC_INT_SAME(CURLOPT_PRIVATE);
    HHVM_RC_INT_SAME(CURLOPT_PROGRESSDATA);
    HHVM_RC_INT_SAME(CURLOPT_PROGRESSFUNCTION);
    HHVM_RC_INT_SAME(CURLOPT_PROXY);
    HHVM_RC_INT_SAME(CURLOPT_PROXYAUTH);
    HHVM_RC_INT_SAME(CURLOPT_PROXYPORT);
    HHVM_RC_INT_SAME(CURLOPT_PROXYTYPE);
    HHVM_RC_INT_SAME(CURLOPT_PROXYUSERPWD);
    HHVM_RC_INT_SAME(CURLOPT_PUT);
    HHVM_RC_INT_SAME(CURLOPT_QUOTE);
    HHVM_RC_INT_SAME(CURLOPT_RANDOM_FILE);
    HHVM_RC_INT_SAME(CURLOPT_RANGE);
    HHVM_RC_INT_SAME(CURLOPT_READDATA);
    HHVM_RC_INT_SAME(CURLOPT_READFUNCTION);
    HHVM_RC_INT_SAME(CURLOPT_REFERER);
    HHVM_RC_INT_SAME(CURLOPT_RESOLVE);
    HHVM_RC_INT_SAME(CURLOPT_RESUME_FROM);
    HHVM_RC_INT_SAME(CURLOPT_RETURNTRANSFER);
#ifdef FACEBOOK
    HHVM_RC_INT_SAME(CURLOPT_SERVICE_NAME);
#endif
    HHVM_RC_INT_SAME(CURLOPT_SSLCERT);
    HHVM_RC_INT_SAME(CURLOPT_SSLCERTPASSWD);
    HHVM_RC_INT_SAME(CURLOPT_SSLCERTTYPE);
    HHVM_RC_INT_SAME(CURLOPT_SSLENGINE);
    HHVM_RC_INT_SAME(CURLOPT_SSLENGINE_DEFAULT);
    HHVM_RC_INT_SAME(CURLOPT_SSLKEY);
    HHVM_RC_INT_SAME(CURLOPT_SSLKEYPASSWD);
    HHVM_RC_INT_SAME(CURLOPT_SSLKEYTYPE);
    HHVM_RC_INT_SAME(CURLOPT_SSLVERSION);
    HHVM_RC_INT_SAME(CURLOPT_SSL_CIPHER_LIST);
    HHVM_RC_INT_SAME(CURLOPT_SSL_VERIFYHOST);
    HHVM_RC_INT_SAME(CURLOPT_SSL_VERIFYPEER);
    HHVM_RC_INT_SAME(CURLOPT_STDERR);
    HHVM_RC_INT_SAME(CURLOPT_TCP_NODELAY);
    HHVM_RC_INT_SAME(CURLOPT_TIMECONDITION);
    HHVM_RC_INT_SAME(CURLOPT_TIMEOUT);
    HHVM_RC_INT_SAME(CURLOPT_TIMEVALUE);
    HHVM_RC_INT_SAME(CURLOPT_TRANSFERTEXT);
    HHVM_RC_INT_SAME(CURLOPT_UNRESTRICTED_AUTH);
    HHVM_RC_INT_SAME(CURLOPT_UPLOAD);
    HHVM_RC_INT_SAME(CURLOPT_URL);
    HHVM_RC_INT_SAME(CURLOPT_USERAGENT);
    HHVM_RC_INT_SAME(CURLOPT_USERPWD);
    HHVM_RC_INT_SAME(CURLOPT_VERBOSE);
    HHVM_RC_INT_SAME(CURLOPT_WRITEFUNCTION);
    HHVM_RC_INT_SAME(CURLOPT_WRITEHEADER);
    HHVM_RC_INT_SAME(CURLPROXY_HTTP);
    HHVM_RC_INT_SAME(CURLPROXY_SOCKS5);
    HHVM_RC_INT_SAME(CURLVERSION_NOW);
    HHVM_RC_INT_SAME(CURL_HTTP_VERSION_1_0);
    HHVM_RC_INT_SAME(CURL_HTTP_VERSION_1_1);
    HHVM_RC_INT_SAME(CURL_HTTP_VERSION_NONE);
    HHVM_RC_INT_SAME(CURL_IPRESOLVE_V4);
    HHVM_RC_INT_SAME(CURL_IPRESOLVE_V6);
    HHVM_RC_INT_SAME(CURL_IPRESOLVE_WHATEVER);
    HHVM_RC_INT_SAME(CURL_NETRC_IGNORED);
    HHVM_RC_INT_SAME(CURL_NETRC_OPTIONAL);
    HHVM_RC_INT_SAME(CURL_NETRC_REQUIRED);
    HHVM_RC_INT_SAME(CURL_TIMECOND_IFMODSINCE);
    HHVM_RC_INT_SAME(CURL_TIMECOND_IFUNMODSINCE);
    HHVM_RC_INT_SAME(CURL_TIMECOND_LASTMOD);
    HHVM_RC_INT_SAME(CURL_VERSION_IPV6);
    HHVM_RC_INT_SAME(CURL_VERSION_KERBEROS4);
    HHVM_RC_INT_SAME(CURL_VERSION_LIBZ);
    HHVM_RC_INT_SAME(CURL_VERSION_SSL);

    HHVM_RC_INT_SAME(CURLPROTO_HTTP);
    HHVM_RC_INT_SAME(CURLPROTO_HTTPS);
    HHVM_RC_INT_SAME(CURLPROTO_FTP);
    HHVM_RC_INT_SAME(CURLPROTO_FTPS);
    HHVM_RC_INT_SAME(CURLPROTO_SCP);
    HHVM_RC_INT_SAME(CURLPROTO_SFTP);
    HHVM_RC_INT_SAME(CURLPROTO_TELNET);
    HHVM_RC_INT_SAME(CURLPROTO_LDAP);
    HHVM_RC_INT_SAME(CURLPROTO_LDAPS);
    HHVM_RC_INT_SAME(CURLPROTO_DICT);
    HHVM_RC_INT_SAME(CURLPROTO_FILE);
    HHVM_RC_INT_SAME(CURLPROTO_TFTP);
    HHVM_RC_INT_SAME(CURLPROTO_ALL);

    HHVM_RC_INT(CURLOPT_FB_TLS_VER_MAX,
                CurlResource::fb_specific_options::CURLOPT_FB_TLS_VER_MAX);
    HHVM_RC_INT(CURLOPT_FB_TLS_VER_MAX_NONE,
                CurlResource::fb_specific_options::CURLOPT_FB_TLS_VER_MAX_NONE);
    HHVM_RC_INT(CURLOPT_FB_TLS_VER_MAX_1_1,
                CurlResource::fb_specific_options::CURLOPT_FB_TLS_VER_MAX_1_1);
    HHVM_RC_INT(CURLOPT_FB_TLS_VER_MAX_1_0,
                CurlResource::fb_specific_options::CURLOPT_FB_TLS_VER_MAX_1_0);
    HHVM_RC_INT(CURLOPT_FB_TLS_CIPHER_SPEC,
                CurlResource::fb_specific_options::CURLOPT_FB_TLS_CIPHER_SPEC);

    HHVM_FE(curl_init);
    HHVM_FE(curl_init_pooled);
    HHVM_FE(curl_copy_handle);
    HHVM_FE(curl_version);
    HHVM_FE(curl_setopt);
    HHVM_FE(curl_setopt_array);
    HHVM_FE(fb_curl_getopt);
    HHVM_FE(curl_exec);
    HHVM_FE(curl_getinfo);
    HHVM_FE(curl_errno);
    HHVM_FE(curl_error);
    HHVM_FE(curl_close);
    HHVM_FE(curl_reset);
    HHVM_FE(curl_multi_init);
    HHVM_FE(curl_multi_strerror);
    HHVM_FE(curl_multi_add_handle);
    HHVM_FE(curl_multi_remove_handle);
    HHVM_FE(curl_multi_exec);
    HHVM_FE(curl_multi_select);
    HHVM_FE(curl_multi_await);
    HHVM_FE(curl_multi_getcontent);
    HHVM_FE(curl_multi_setopt);
    HHVM_FE(fb_curl_multi_fdset);
    HHVM_FE(curl_multi_info_read);
    HHVM_FE(curl_multi_close);
    HHVM_FE(curl_strerror);

    Extension* ext = ExtensionRegistry::get("curl");
    assert(ext);

    IniSetting::Bind(ext, IniSetting::PHP_INI_SYSTEM, "curl.namedPools",
      "", &s_namedPools);
    if (s_namedPools.length() > 0) {

      // split on commas, search and bind ini settings for each pool
      std::vector<string> pools;
      boost::split(pools, s_namedPools, boost::is_any_of(","));

      for (std::string poolname: pools) {
        if (poolname.length() == 0) { continue; }

        // get the user-entered settings for this pool, if there are any
        std::string poolSizeIni = "curl.namedPools." + poolname + ".size";
        std::string reuseLimitIni =
          "curl.namedPools." + poolname + ".reuseLimit";
        std::string getTimeoutIni =
          "curl.namedPools." + poolname + ".connGetTimeout";

        IniSetting::Bind(ext, IniSetting::PHP_INI_SYSTEM, poolSizeIni,
            "5", &s_poolSize);
        IniSetting::Bind(ext, IniSetting::PHP_INI_SYSTEM, reuseLimitIni,
            "100", &s_reuseLimit);
        IniSetting::Bind(ext, IniSetting::PHP_INI_SYSTEM, getTimeoutIni,
            "5000", &s_getTimeout);

        CurlHandlePool *hp =
          new CurlHandlePool(s_poolSize, s_getTimeout, s_reuseLimit);
        CurlHandlePool::namedPools[poolname] = hp;
      }
    }

    loadSystemlib();
  }

  void moduleShutdown() override {
    for (auto const kvItr: CurlHandlePool::namedPools) {
      delete kvItr.second;
    }
  }

} s_curl_extension;

}

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/ext/curl/curl-share-resource.h"
#include "hphp/runtime/ext/asio/socket-event.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/libevent-http-client.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stack-logger.h"
#include "hphp/runtime/ext/extension-registry.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/lock.h"
#include <boost/algorithm/string.hpp>
#include <boost/variant.hpp>
#include <folly/Optional.h>
#include <folly/portability/OpenSSL.h>
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
  CurlHandlePoolPtr pool;
  {
    std::string cppPoolName = poolName.toCppString();
    ReadLock lock(CurlHandlePool::namedPoolsMutex);
    bool poolExists = (CurlHandlePool::namedPools.find(cppPoolName) !=
        CurlHandlePool::namedPools.end());
    if (!poolExists) {
      raise_warning("Attempting to use connection pooling without "
                    "specifying an existent connection pool!");
      return false;
    }
    pool = CurlHandlePool::namedPools.at(cppPoolName);
  }
  if (url.isNull()) {
    return Variant(req::make<CurlResource>(null_string, pool));
  } else {
    return Variant(req::make<CurlResource>(url.toString(), pool));
  }
}

void HHVM_FUNCTION(curl_create_pool, const String& poolName,
                   int size /* = 5 */, int connGetTimeout /* = 5000 */,
                   int reuseLimit /* = 500 */) {
  auto hp = std::make_shared<CurlHandlePool>(size, connGetTimeout, reuseLimit);
  WriteLock lock(CurlHandlePool::namedPoolsMutex);
  CurlHandlePool::namedPools[poolName.toCppString()] = hp;
}

bool HHVM_FUNCTION(curl_destroy_pool, const String& poolName) {
  WriteLock lock(CurlHandlePool::namedPoolsMutex);
  return CurlHandlePool::namedPools.erase(poolName.toCppString()) > 0;
}

const StaticString
  s_size("size"),
  s_connGetTimeout("connGetTimeout"),
  s_reuseLimit("reuseLimit"),
  s_stats("stats"),
  s_fetches("fetches"),
  s_empty("empty"),
  s_fetchMs("fetchMs");

Array HHVM_FUNCTION(curl_list_pools) {
  ReadLock lock(CurlHandlePool::namedPoolsMutex);
  auto size = CurlHandlePool::namedPools.size();
  if (!size) return empty_darray();

  DArrayInit ret(size);
  for (auto it: CurlHandlePool::namedPools) {
    auto pool = it.second;
    auto stats = make_darray(
      s_fetches, pool->statsFetches(),
      s_empty, pool->statsEmpty(),
      s_fetchMs, pool->statsFetchUs() / 1000
    );
    ret.set(String(it.first), make_darray(s_size, pool->size(),
                                          s_connGetTimeout,
                                          pool->connGetTimeout(),
                                          s_reuseLimit,
                                          pool->reuseLimit(),
                                          s_stats, stats));
  }
  return ret.toArray();
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

#if LIBCURL_VERSION_NUM >= 0x071301 /* Available since 7.19.1 */
Array create_certinfo(struct curl_certinfo *ci) {
  Array ret = Array::CreateVArray();
  if (ci) {
    for (int i = 0; i < ci->num_of_certs; i++) {
      struct curl_slist *slist = ci->certinfo[i];

      Array certData = Array::CreateDArray();
      while (slist) {
        Array parts = StringUtil::Explode(
          String(slist->data, CopyString),
          ":",
          2).toArray();
        if (parts.size() == 2) {
          certData.set(parts.lookup(0), parts.lookup(1));
        } else {
          raise_warning("Could not extract hash key from certificate info");
        }
        slist = slist->next;
      }
      ret.append(certData);
    }
  }
  return ret;
}
#endif

const StaticString
  s_url("url"),
  s_content_type("content_type"),
  s_http_code("http_code"),
  s_header_size("header_size"),
  s_request_size("request_size"),
  s_filetime("filetime"),
  s_ssl_verify_result("ssl_verify_result"),
  s_redirect_count("redirect_count"),
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
  s_redirect_url("redirect_url"),
  s_primary_ip("primary_ip"),
  s_primary_port("primary_port"),
  s_local_ip("local_ip"),
  s_local_port("local_port"),
  s_certinfo("certinfo"),
  s_request_header("request_header");

Variant HHVM_FUNCTION(curl_getinfo, const Resource& ch, int opt /* = 0 */) {
  CHECK_RESOURCE(curl);
  CURL *cp = curl->get();

  if (opt == 0) {
    char   *s_code;
    long    l_code;
    double  d_code;
#if LIBCURL_VERSION_NUM >  0x071301 /* Available since 7.19.1 */
    struct curl_certinfo *ci = nullptr;
#endif

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
#if LIBCURL_VERSION_NUM >= 0x071202 /* Available since 7.18.2 */
    if (curl_easy_getinfo(cp, CURLINFO_REDIRECT_URL, &s_code) == CURLE_OK) {
      ret.set(s_redirect_url, String(s_code, CopyString));
    }
#endif
#if LIBCURL_VERSION_NUM >= 0x071300 /* Available since 7.19.0 */
    if (curl_easy_getinfo(cp, CURLINFO_PRIMARY_IP, &s_code) == CURLE_OK) {
      ret.set(s_primary_ip, String(s_code, CopyString));
    }
#endif
#if LIBCURL_VERSION_NUM >= 0x071301 /* Available since 7.19.1 */
    if (curl_easy_getinfo(cp, CURLINFO_CERTINFO, &ci) == CURLE_OK) {
      ret.set(s_certinfo, create_certinfo(ci));
    }
#endif
#if LIBCURL_VERSION_NUM >= 0x071500 /* Available since 7.21.0 */
    if (curl_easy_getinfo(cp, CURLINFO_PRIMARY_PORT, &l_code) == CURLE_OK) {
      ret.set(s_primary_port, l_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_LOCAL_IP, &s_code) == CURLE_OK) {
      ret.set(s_local_ip, String(s_code, CopyString));
    }
    if (curl_easy_getinfo(cp, CURLINFO_LOCAL_PORT, &l_code) == CURLE_OK) {
      ret.set(s_local_port, l_code);
    }
#endif
    String header = curl->getHeader();
    if (!header.empty()) {
      ret.set(s_request_header, header);
    }
    return ret;
  }

  switch (opt) {
    case CURLINFO_HEADER_OUT: {
      String header = curl->getHeader();
      if (!header.empty()) {
        return header;
      }
      return false;
    }
#if LIBCURL_VERSION_NUM >= 0x071301 /* Available since 7.19.1 */
    case CURLINFO_CERTINFO: {
      struct curl_certinfo *ci = nullptr;
      if (curl_easy_getinfo(cp, CURLINFO_CERTINFO, &ci) == CURLE_OK) {
        return create_certinfo(ci);
      }
      return false;
    }
#endif
  }

  switch (CURLINFO_TYPEMASK & opt) {
    case CURLINFO_STRING: {
      char *s_code = nullptr;
      if (curl_easy_getinfo(cp, (CURLINFO)opt, &s_code) == CURLE_OK &&
          s_code) {
        return String(s_code, CopyString);
      }
      return false;
    }
    case CURLINFO_LONG: {
      long code = 0;
      if (curl_easy_getinfo(cp, (CURLINFO)opt, &code) == CURLE_OK) {
        return code;
      }
      return false;
    }
    case CURLINFO_DOUBLE: {
      double code = 0.0;
      if (curl_easy_getinfo(cp, (CURLINFO)opt, &code) == CURLE_OK) {
        return code;
      }
      return false;
    }
#if LIBCURL_VERSION_NUM >= 0x070c03 /* Available since 7.12.3 */
    case CURLINFO_SLIST: {
      struct curl_slist *slist;
      Array ret = Array::CreateVArray();
      if (curl_easy_getinfo(cp, (CURLINFO)opt, &slist) == CURLE_OK) {
        while (slist) {
          ret.append(slist->data);
          slist = slist->next;
        }
        curl_slist_free_all(slist);
        return ret;
      }
      return false;
    }
#endif
    default:
      return false;
  }
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

Variant HHVM_FUNCTION(curl_multi_add_handle, const Resource& mh,
                      const Resource& ch) {
  CHECK_MULTI_RESOURCE(curlm);
  auto curle = cast<CurlResource>(ch);
  curlm->add(ch);
  return curl_multi_add_handle(curlm->get(), curle->get());
}

Variant HHVM_FUNCTION(curl_multi_remove_handle, const Resource& mh,
                      const Resource& ch) {
  CHECK_MULTI_RESOURCE(curlm);
  auto curle = cast<CurlResource>(ch);
  curlm->remove(curle);
  return curl_multi_remove_handle(curlm->get(), curle->get());
}

Variant HHVM_FUNCTION(curl_multi_exec, const Resource& mh,
                      int64_t& still_running) {
  CHECK_MULTI_RESOURCE(curlm);
  int running = 0;
  IOStatusHelper io("curl_multi_exec");
  SYNC_VM_REGS_SCOPED();
  if (curlm->anyInExec()) {
    log_native_stack("unexpected reentry into curl_multi_exec");
  }
  curlm->setInExec(true);
  // T29358191: curl_multi_perform should not throw... trust but verify
  int result;
  try {
    result = curl_multi_perform(curlm->get(), &running);
  } catch (...) {
    curlm->setInExec(false);
    log_native_stack("unexpected exception from curl_multi_perform");
    throw;
  }
  curlm->setInExec(false);
  curlm->check_exceptions();
  still_running = running;
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
    assertx(false);
    ev->abandon();
    throw;
  }
}

Variant HHVM_FUNCTION(curl_multi_getcontent, const Resource& ch) {
  CHECK_RESOURCE(curl);
  return curl->getContents();
}

Array curl_convert_fd_to_stream(fd_set *fd, int max_fd) {
  Array ret = Array::CreateVArray();
  for (int i=0; i<=max_fd; i++) {
    if (FD_ISSET(i, fd)) {
      ret.append(Variant(req::make<BuiltinFile>(i)));
    }
  }
  return ret;
}

Variant HHVM_FUNCTION(fb_curl_multi_fdset, const Resource& mh,
                      Array& read_fd_set,
                      Array& write_fd_set,
                      Array& exc_fd_set,
                      int64_t& max_fd) {
  CHECK_MULTI_RESOURCE(curlm);

  fd_set read_set;
  fd_set write_set;
  fd_set exc_set;
  int max = 0;

  FD_ZERO(&read_set);
  FD_ZERO(&write_set);
  FD_ZERO(&exc_set);

  int r = curl_multi_fdset(curlm->get(), &read_set, &write_set, &exc_set, &max);
  read_fd_set = curl_convert_fd_to_stream(&read_set, max);
  write_fd_set = curl_convert_fd_to_stream(&write_set, max);
  exc_fd_set = curl_convert_fd_to_stream(&exc_set, max);
  max_fd = max;

  return r;
}

const StaticString
  s_msg("msg"),
  s_result("result"),
  s_handle("handle");

Variant HHVM_FUNCTION(curl_multi_info_read, const Resource& mh,
                      int64_t& msgs_in_queue) {
  CHECK_MULTI_RESOURCE(curlm);

  int queued_msgs;
  CURLMsg *tmp_msg = curl_multi_info_read(curlm->get(), &queued_msgs);
  curlm->check_exceptions();
  if (tmp_msg == nullptr) {
    return false;
  }
  msgs_in_queue = queued_msgs;

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

static std::string CURL_SHARE_Warning
  = "expects parameter 1 to be cURL share resource";

Resource HHVM_FUNCTION(curl_share_init) {
  return Resource(req::make<CurlShareResource>());
}

void HHVM_FUNCTION(curl_share_close, const Resource& sh) {
  auto curlsh = dyn_cast_or_null<CurlShareResource>(sh);
  if (!curlsh || curlsh->isInvalid()) {
    raise_warning(CURL_SHARE_Warning);
  }
  curlsh->close();
}

bool HHVM_FUNCTION(curl_share_setopt, const Resource& sh,
                   int option, const Variant& value) {
  auto curlsh = dyn_cast_or_null<CurlShareResource>(sh);
  if (!curlsh || curlsh->isInvalid())
    SystemLib::throwExceptionObject(CURL_SHARE_Warning);
  return curlsh->setOption(option, value);
}


///////////////////////////////////////////////////////////////////////////////

static int  s_sizeVal, s_connGetTimeoutVal, s_reuseLimitVal;
static std::string s_namedPools;

struct CurlExtension final : Extension {
  CurlExtension() : Extension("curl") {}
  void moduleInit() override {

    /* See https://curl.haxx.se/libcurl/c/symbols-in-versions.html
       or curl src/docs/libcurl/symbols-in-versions for a (almost) complete list
       of options and which version they were introduced */

    /* Constants for curl_setopt() */
    HHVM_RC_INT_SAME(CURLOPT_AUTOREFERER);
    HHVM_RC_INT_SAME(CURLOPT_BINARYTRANSFER);
    HHVM_RC_INT_SAME(CURLOPT_BUFFERSIZE);
    HHVM_RC_INT_SAME(CURLOPT_CAINFO);
    HHVM_RC_INT_SAME(CURLOPT_CAPATH);
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
    HHVM_RC_INT_SAME(CURLOPT_FTP_USE_EPRT);
    HHVM_RC_INT_SAME(CURLOPT_FTP_USE_EPSV);
    HHVM_RC_INT_SAME(CURLOPT_HEADER);
    HHVM_RC_INT_SAME(CURLOPT_HEADERFUNCTION);
    HHVM_RC_INT_SAME(CURLOPT_HTTP200ALIASES);
    HHVM_RC_INT_SAME(CURLOPT_HTTPGET);
    HHVM_RC_INT_SAME(CURLOPT_HTTPHEADER);
    HHVM_RC_INT_SAME(CURLOPT_HTTPPROXYTUNNEL);
    HHVM_RC_INT_SAME(CURLOPT_HTTP_VERSION);
    HHVM_RC_INT_SAME(CURLOPT_INFILE);
    HHVM_RC_INT_SAME(CURLOPT_INFILESIZE);
    HHVM_RC_INT_SAME(CURLOPT_INTERFACE);
    HHVM_RC_INT_SAME(CURLOPT_KRB4LEVEL);
    HHVM_RC_INT_SAME(CURLOPT_LOW_SPEED_LIMIT);
    HHVM_RC_INT_SAME(CURLOPT_LOW_SPEED_TIME);
    HHVM_RC_INT_SAME(CURLOPT_MAXCONNECTS);
    HHVM_RC_INT_SAME(CURLOPT_MAXREDIRS);
    HHVM_RC_INT_SAME(CURLOPT_NETRC);
    HHVM_RC_INT_SAME(CURLOPT_NOBODY);
    HHVM_RC_INT_SAME(CURLOPT_NOPROGRESS);
    HHVM_RC_INT_SAME(CURLOPT_NOSIGNAL);
    HHVM_RC_INT_SAME(CURLOPT_PORT);
    HHVM_RC_INT_SAME(CURLOPT_POST);
    HHVM_RC_INT_SAME(CURLOPT_POSTFIELDS);
    HHVM_RC_INT_SAME(CURLOPT_POSTQUOTE);
    HHVM_RC_INT_SAME(CURLOPT_PREQUOTE);
    HHVM_RC_INT_SAME(CURLOPT_PRIVATE);
    HHVM_RC_INT_SAME(CURLOPT_PROGRESSFUNCTION);
    HHVM_RC_INT_SAME(CURLOPT_PROXY);
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
    HHVM_RC_INT_SAME(CURLOPT_RESUME_FROM);
    HHVM_RC_INT_SAME(CURLOPT_RETURNTRANSFER);
    HHVM_RC_INT_SAME(CURLOPT_SHARE);
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
    HHVM_RC_INT_SAME(CURLOPT_TELNETOPTIONS);
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

    /* */
    HHVM_RC_INT_SAME(CURLE_ABORTED_BY_CALLBACK);
    HHVM_RC_INT_SAME(CURLE_BAD_CALLING_ORDER);
    HHVM_RC_INT_SAME(CURLE_BAD_CONTENT_ENCODING);
    HHVM_RC_INT_SAME(CURLE_BAD_DOWNLOAD_RESUME);
    HHVM_RC_INT_SAME(CURLE_BAD_FUNCTION_ARGUMENT);
    HHVM_RC_INT_SAME(CURLE_BAD_PASSWORD_ENTERED);
    HHVM_RC_INT_SAME(CURLE_COULDNT_CONNECT);
    HHVM_RC_INT_SAME(CURLE_COULDNT_RESOLVE_HOST);
    HHVM_RC_INT_SAME(CURLE_COULDNT_RESOLVE_PROXY);
    HHVM_RC_INT_SAME(CURLE_FAILED_INIT);
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
    HHVM_RC_INT_SAME(CURLE_FTP_PARTIAL_FILE);
    HHVM_RC_INT_SAME(CURLE_FTP_PORT_FAILED);
    HHVM_RC_INT_SAME(CURLE_FTP_QUOTE_ERROR);
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
    HHVM_RC_INT_SAME(CURLE_HTTP_RETURNED_ERROR);
    HHVM_RC_INT_SAME(CURLE_LDAP_CANNOT_BIND);
    HHVM_RC_INT_SAME(CURLE_LDAP_SEARCH_FAILED);
    HHVM_RC_INT_SAME(CURLE_LIBRARY_NOT_FOUND);
    HHVM_RC_INT_SAME(CURLE_MALFORMAT_USER);
    HHVM_RC_INT_SAME(CURLE_OBSOLETE);
    HHVM_RC_INT_SAME(CURLE_OK);
    HHVM_RC_INT_SAME(CURLE_OPERATION_TIMEDOUT);
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
    if (CURLE_SSL_PEER_CERTIFICATE == CURLE_SSL_CACERT) {
      // In older curl libraries, this was 51, but then it became
      // equal to CURLE_SSL_CACERT, which breaks enums which include
      // both values. Keep the old value for now. We can consider
      // killing it later (after removing it from the hack code that
      // uses it).
      HHVM_RC_INT(CURLE_SSL_PEER_CERTIFICATE, 51);
    } else {
      HHVM_RC_INT_SAME(CURLE_SSL_PEER_CERTIFICATE);
    }
    HHVM_RC_INT_SAME(CURLE_TELNET_OPTION_SYNTAX);
    HHVM_RC_INT_SAME(CURLE_TOO_MANY_REDIRECTS);
    HHVM_RC_INT_SAME(CURLE_UNKNOWN_TELNET_OPTION);
    HHVM_RC_INT_SAME(CURLE_UNSUPPORTED_PROTOCOL);
    HHVM_RC_INT_SAME(CURLE_URL_MALFORMAT);
    HHVM_RC_INT_SAME(CURLE_URL_MALFORMAT_USER);
    HHVM_RC_INT_SAME(CURLE_WRITE_ERROR);

    /* cURL info constants */
    HHVM_RC_INT_SAME(CURLINFO_CONNECT_TIME);
    HHVM_RC_INT_SAME(CURLINFO_CONTENT_LENGTH_DOWNLOAD);
    HHVM_RC_INT_SAME(CURLINFO_CONTENT_LENGTH_UPLOAD);
    HHVM_RC_INT_SAME(CURLINFO_CONTENT_TYPE);
    HHVM_RC_INT_SAME(CURLINFO_EFFECTIVE_URL);
    HHVM_RC_INT_SAME(CURLINFO_FILETIME);
    HHVM_RC_INT_SAME(CURLINFO_HEADER_OUT);
    HHVM_RC_INT_SAME(CURLINFO_HEADER_SIZE);
    HHVM_RC_INT_SAME(CURLINFO_HTTP_CODE);
    HHVM_RC_INT_SAME(CURLINFO_LASTONE);
    HHVM_RC_INT_SAME(CURLINFO_NAMELOOKUP_TIME);
    HHVM_RC_INT_SAME(CURLINFO_PRETRANSFER_TIME);
    HHVM_RC_INT_SAME(CURLINFO_PRIVATE);
    HHVM_RC_INT_SAME(CURLINFO_REDIRECT_COUNT);
    HHVM_RC_INT_SAME(CURLINFO_REDIRECT_TIME);
    HHVM_RC_INT_SAME(CURLINFO_REQUEST_SIZE);
    HHVM_RC_INT_SAME(CURLINFO_SIZE_DOWNLOAD);
    HHVM_RC_INT_SAME(CURLINFO_SIZE_UPLOAD);
    HHVM_RC_INT_SAME(CURLINFO_SPEED_DOWNLOAD);
    HHVM_RC_INT_SAME(CURLINFO_SPEED_UPLOAD);
    HHVM_RC_INT_SAME(CURLINFO_SSL_VERIFYRESULT);
    HHVM_RC_INT_SAME(CURLINFO_STARTTRANSFER_TIME);
    HHVM_RC_INT_SAME(CURLINFO_TOTAL_TIME);

    /* Other */
    HHVM_RC_INT_SAME(CURLMSG_DONE);
    HHVM_RC_INT_SAME(CURLVERSION_NOW);

    /* Curl Multi Constants */
    HHVM_RC_INT_SAME(CURLM_BAD_EASY_HANDLE);
    HHVM_RC_INT_SAME(CURLM_BAD_HANDLE);
    HHVM_RC_INT_SAME(CURLM_CALL_MULTI_PERFORM);
    HHVM_RC_INT_SAME(CURLM_INTERNAL_ERROR);
    HHVM_RC_INT_SAME(CURLM_OK);
    HHVM_RC_INT_SAME(CURLM_OUT_OF_MEMORY);

    /* Curl proxy constants */
    HHVM_RC_INT_SAME(CURLPROXY_HTTP);
    HHVM_RC_INT_SAME(CURLPROXY_SOCKS4);
    HHVM_RC_INT_SAME(CURLPROXY_SOCKS5);

#if LIBCURL_VERSION_NUM >= 0x071200 /* Available since 7.18.0 */
    HHVM_RC_INT_SAME(CURLPROXY_SOCKS4A);
    HHVM_RC_INT_SAME(CURLPROXY_SOCKS5_HOSTNAME);
#endif

    /* Curl Share constants */
    HHVM_RC_INT_SAME(CURLSHOPT_NONE);
    HHVM_RC_INT_SAME(CURLSHOPT_SHARE);
    HHVM_RC_INT_SAME(CURLSHOPT_UNSHARE);

    /* Curl Http Version constants (CURLOPT_HTTP_VERSION) */
    HHVM_RC_INT_SAME(CURL_HTTP_VERSION_1_0);
    HHVM_RC_INT_SAME(CURL_HTTP_VERSION_1_1);
    HHVM_RC_INT_SAME(CURL_HTTP_VERSION_NONE);

    /* Curl Lock constants */
    HHVM_RC_INT_SAME(CURL_LOCK_DATA_COOKIE);
    HHVM_RC_INT_SAME(CURL_LOCK_DATA_DNS);
    HHVM_RC_INT_SAME(CURL_LOCK_DATA_SSL_SESSION);

    /* Curl NETRC constants (CURLOPT_NETRC) */
    HHVM_RC_INT_SAME(CURL_NETRC_IGNORED);
    HHVM_RC_INT_SAME(CURL_NETRC_OPTIONAL);
    HHVM_RC_INT_SAME(CURL_NETRC_REQUIRED);

    /* Curl SSL Version constants (CURLOPT_SSLVERSION) */
    HHVM_RC_INT_SAME(CURL_SSLVERSION_DEFAULT);
    HHVM_RC_INT_SAME(CURL_SSLVERSION_SSLv2);
    HHVM_RC_INT_SAME(CURL_SSLVERSION_SSLv3);
    HHVM_RC_INT_SAME(CURL_SSLVERSION_TLSv1);

    /* Curl TIMECOND constants (CURLOPT_TIMECONDITION) */
    HHVM_RC_INT_SAME(CURL_TIMECOND_IFMODSINCE);
    HHVM_RC_INT_SAME(CURL_TIMECOND_IFUNMODSINCE);
    HHVM_RC_INT_SAME(CURL_TIMECOND_LASTMOD);
    HHVM_RC_INT_SAME(CURL_TIMECOND_NONE);

    /* Curl version constants */
    HHVM_RC_INT_SAME(CURL_VERSION_IPV6);
    HHVM_RC_INT_SAME(CURL_VERSION_KERBEROS4);
    HHVM_RC_INT_SAME(CURL_VERSION_LIBZ);
    HHVM_RC_INT_SAME(CURL_VERSION_SSL);

#if LIBCURL_VERSION_NUM >= 0x070a06 /* Available since 7.10.6 */
    HHVM_RC_INT_SAME(CURLOPT_HTTPAUTH);
    /* http authentication options */
    // These two options are uint64_t values with the top bit set,
    // but they are bit masks, so its ok to convert to int64_t.
    HHVM_RC_INT(CURLAUTH_ANY, static_cast<int64_t>(CURLAUTH_ANY));
    HHVM_RC_INT(CURLAUTH_ANYSAFE, static_cast<int64_t>(CURLAUTH_ANYSAFE));
    HHVM_RC_INT_SAME(CURLAUTH_BASIC);
    HHVM_RC_INT_SAME(CURLAUTH_DIGEST);
    HHVM_RC_INT_SAME(CURLAUTH_GSSNEGOTIATE);
    HHVM_RC_INT_SAME(CURLAUTH_NONE);
    HHVM_RC_INT_SAME(CURLAUTH_NTLM);
#endif

#if LIBCURL_VERSION_NUM >= 0x070a07 /* Available since 7.10.7 */
    HHVM_RC_INT_SAME(CURLINFO_HTTP_CONNECTCODE);
    HHVM_RC_INT_SAME(CURLOPT_FTP_CREATE_MISSING_DIRS);
    HHVM_RC_INT_SAME(CURLOPT_PROXYAUTH);
#endif

#if LIBCURL_VERSION_NUM >= 0x070a08 /* Available since 7.10.8 */
    HHVM_RC_INT_SAME(CURLE_FILESIZE_EXCEEDED);
    HHVM_RC_INT_SAME(CURLE_LDAP_INVALID_URL);
    HHVM_RC_INT_SAME(CURLINFO_HTTPAUTH_AVAIL);
    HHVM_RC_INT_SAME(CURLINFO_RESPONSE_CODE);
    HHVM_RC_INT_SAME(CURLINFO_PROXYAUTH_AVAIL);
    HHVM_RC_INT_SAME(CURLOPT_FTP_RESPONSE_TIMEOUT);
    HHVM_RC_INT_SAME(CURLOPT_IPRESOLVE);
    HHVM_RC_INT_SAME(CURLOPT_MAXFILESIZE);
    HHVM_RC_INT_SAME(CURL_IPRESOLVE_V4);
    HHVM_RC_INT_SAME(CURL_IPRESOLVE_V6);
    HHVM_RC_INT_SAME(CURL_IPRESOLVE_WHATEVER);
#endif

#if LIBCURL_VERSION_NUM >= 0x070b00 /* Available since 7.11.0 */
    HHVM_RC_INT_SAME(CURLE_FTP_SSL_FAILED);
    HHVM_RC_INT_SAME(CURLFTPSSL_ALL);
    HHVM_RC_INT_SAME(CURLFTPSSL_CONTROL);
    HHVM_RC_INT_SAME(CURLFTPSSL_NONE);
    HHVM_RC_INT_SAME(CURLFTPSSL_TRY);
    HHVM_RC_INT_SAME(CURLOPT_FTP_SSL);
    HHVM_RC_INT_SAME(CURLOPT_NETRC_FILE);
#endif

#if LIBCURL_VERSION_NUM >= 0x070c02 /* Available since 7.12.2 */
    HHVM_RC_INT_SAME(CURLFTPAUTH_DEFAULT);
    HHVM_RC_INT_SAME(CURLFTPAUTH_SSL);
    HHVM_RC_INT_SAME(CURLFTPAUTH_TLS);
    HHVM_RC_INT_SAME(CURLOPT_FTPSSLAUTH);
#endif

#if LIBCURL_VERSION_NUM >= 0x070d00 /* Available since 7.13.0 */
    HHVM_RC_INT_SAME(CURLOPT_FTP_ACCOUNT);
#endif

#if LIBCURL_VERSION_NUM >= 0x070b02 /* Available since 7.11.2 */
    HHVM_RC_INT_SAME(CURLOPT_TCP_NODELAY);
#endif

#if LIBCURL_VERSION_NUM >= 0x070c02 /* Available since 7.12.2 */
    HHVM_RC_INT_SAME(CURLINFO_OS_ERRNO);
#endif

#if LIBCURL_VERSION_NUM >= 0x070c03 /* Available since 7.12.3 */
    HHVM_RC_INT_SAME(CURLINFO_NUM_CONNECTS);
    HHVM_RC_INT_SAME(CURLINFO_SSL_ENGINES);
#endif

#if LIBCURL_VERSION_NUM >= 0x070e01 /* Available since 7.14.1 */
    HHVM_RC_INT_SAME(CURLINFO_COOKIELIST);
    HHVM_RC_INT_SAME(CURLOPT_COOKIELIST);
    HHVM_RC_INT_SAME(CURLOPT_IGNORE_CONTENT_LENGTH);
#endif

#if LIBCURL_VERSION_NUM >= 0x070f00 /* Available since 7.15.0 */
    HHVM_RC_INT_SAME(CURLOPT_FTP_SKIP_PASV_IP);
#endif

#if LIBCURL_VERSION_NUM >= 0x070f01 /* Available since 7.15.1 */
    HHVM_RC_INT_SAME(CURLOPT_FTP_FILEMETHOD);
#endif

#if LIBCURL_VERSION_NUM >= 0x070f02 /* Available since 7.15.2 */
    HHVM_RC_INT_SAME(CURLOPT_CONNECT_ONLY);
    HHVM_RC_INT_SAME(CURLOPT_LOCALPORT);
    HHVM_RC_INT_SAME(CURLOPT_LOCALPORTRANGE);
#endif

#if LIBCURL_VERSION_NUM >= 0x070f03 /* Available since 7.15.3 */
    HHVM_RC_INT_SAME(CURLFTPMETHOD_MULTICWD);
    HHVM_RC_INT_SAME(CURLFTPMETHOD_NOCWD);
    HHVM_RC_INT_SAME(CURLFTPMETHOD_SINGLECWD);
#endif

#if LIBCURL_VERSION_NUM >= 0x070f04 /* Available since 7.15.4 */
    HHVM_RC_INT_SAME(CURLINFO_FTP_ENTRY_PATH);
#endif

#if LIBCURL_VERSION_NUM >= 0x070f05 /* Available since 7.15.5 */
    HHVM_RC_INT_SAME(CURLOPT_FTP_ALTERNATIVE_TO_USER);
    HHVM_RC_INT_SAME(CURLOPT_MAX_RECV_SPEED_LARGE);
    HHVM_RC_INT_SAME(CURLOPT_MAX_SEND_SPEED_LARGE);
#endif

#if LIBCURL_VERSION_NUM >= 0x071000 /* Available since 7.16.0 */
    HHVM_RC_INT_SAME(CURLOPT_SSL_SESSIONID_CACHE);
    HHVM_RC_INT_SAME(CURLMOPT_PIPELINING);
#endif

#if LIBCURL_VERSION_NUM >= 0x071001 /* Available since 7.16.1 */
    HHVM_RC_INT_SAME(CURLE_SSH);
    HHVM_RC_INT_SAME(CURLOPT_FTP_SSL_CCC);
    HHVM_RC_INT_SAME(CURLOPT_SSH_AUTH_TYPES);
    HHVM_RC_INT_SAME(CURLOPT_SSH_PRIVATE_KEYFILE);
    HHVM_RC_INT_SAME(CURLOPT_SSH_PUBLIC_KEYFILE);
    HHVM_RC_INT_SAME(CURLFTPSSL_CCC_ACTIVE);
    HHVM_RC_INT_SAME(CURLFTPSSL_CCC_NONE);
    HHVM_RC_INT_SAME(CURLFTPSSL_CCC_PASSIVE);
#endif

#if LIBCURL_VERSION_NUM >= 0x071002 /* Available since 7.16.2 */
    HHVM_RC_INT_SAME(CURLOPT_CONNECTTIMEOUT_MS);
    HHVM_RC_INT_SAME(CURLOPT_HTTP_CONTENT_DECODING);
    HHVM_RC_INT_SAME(CURLOPT_HTTP_TRANSFER_DECODING);
    HHVM_RC_INT_SAME(CURLOPT_TIMEOUT_MS);
#endif

#if LIBCURL_VERSION_NUM >= 0x071003 /* Available since 7.16.3 */
    HHVM_RC_INT_SAME(CURLMOPT_MAXCONNECTS);
#endif

#if LIBCURL_VERSION_NUM >= 0x071004 /* Available since 7.16.4 */
    HHVM_RC_INT_SAME(CURLOPT_KRBLEVEL);
    HHVM_RC_INT_SAME(CURLOPT_NEW_DIRECTORY_PERMS);
    HHVM_RC_INT_SAME(CURLOPT_NEW_FILE_PERMS);
#endif

#if LIBCURL_VERSION_NUM >= 0x071100 /* Available since 7.17.0 */
    HHVM_RC_INT_SAME(CURLOPT_APPEND);
    HHVM_RC_INT_SAME(CURLOPT_DIRLISTONLY);
    HHVM_RC_INT_SAME(CURLOPT_USE_SSL);
    /* Curl SSL Constants */
    HHVM_RC_INT_SAME(CURLUSESSL_ALL);
    HHVM_RC_INT_SAME(CURLUSESSL_CONTROL);
    HHVM_RC_INT_SAME(CURLUSESSL_NONE);
    HHVM_RC_INT_SAME(CURLUSESSL_TRY);
#endif

#if LIBCURL_VERSION_NUM >= 0x071101 /* Available since 7.17.1 */
    HHVM_RC_INT_SAME(CURLOPT_SSH_HOST_PUBLIC_KEY_MD5);
#endif

#if LIBCURL_VERSION_NUM >= 0x071200 /* Available since 7.18.0 */
    HHVM_RC_INT_SAME(CURLOPT_PROXY_TRANSFER_MODE);
    HHVM_RC_INT_SAME(CURLPAUSE_ALL);
    HHVM_RC_INT_SAME(CURLPAUSE_CONT);
    HHVM_RC_INT_SAME(CURLPAUSE_RECV);
    HHVM_RC_INT_SAME(CURLPAUSE_RECV_CONT);
    HHVM_RC_INT_SAME(CURLPAUSE_SEND);
    HHVM_RC_INT_SAME(CURLPAUSE_SEND_CONT);
    HHVM_RC_INT_SAME(CURL_READFUNC_PAUSE);
    HHVM_RC_INT_SAME(CURL_WRITEFUNC_PAUSE);
#endif

#if LIBCURL_VERSION_NUM >= 0x071202 /* Available since 7.18.2 */
    HHVM_RC_INT_SAME(CURLINFO_REDIRECT_URL);
#endif

#if LIBCURL_VERSION_NUM >= 0x071300 /* Available since 7.19.0 */
    HHVM_RC_INT_SAME(CURLINFO_APPCONNECT_TIME);
    HHVM_RC_INT_SAME(CURLINFO_PRIMARY_IP);

    HHVM_RC_INT_SAME(CURLOPT_ADDRESS_SCOPE);
    HHVM_RC_INT_SAME(CURLOPT_CRLFILE);
    HHVM_RC_INT_SAME(CURLOPT_ISSUERCERT);
    HHVM_RC_INT_SAME(CURLOPT_KEYPASSWD);

    HHVM_RC_INT_SAME(CURLSSH_AUTH_ANY);
    HHVM_RC_INT_SAME(CURLSSH_AUTH_DEFAULT);
    HHVM_RC_INT_SAME(CURLSSH_AUTH_HOST);
    HHVM_RC_INT_SAME(CURLSSH_AUTH_KEYBOARD);
    HHVM_RC_INT_SAME(CURLSSH_AUTH_NONE);
    HHVM_RC_INT_SAME(CURLSSH_AUTH_PASSWORD);
    HHVM_RC_INT_SAME(CURLSSH_AUTH_PUBLICKEY);
#endif

#if LIBCURL_VERSION_NUM >= 0x071301 /* Available since 7.19.1 */
    HHVM_RC_INT_SAME(CURLINFO_CERTINFO);
    HHVM_RC_INT_SAME(CURLOPT_CERTINFO);
    HHVM_RC_INT_SAME(CURLOPT_PASSWORD);
    HHVM_RC_INT_SAME(CURLOPT_POSTREDIR);
    HHVM_RC_INT_SAME(CURLOPT_PROXYPASSWORD);
    HHVM_RC_INT_SAME(CURLOPT_PROXYUSERNAME);
    HHVM_RC_INT_SAME(CURLOPT_USERNAME);
#endif

#if LIBCURL_VERSION_NUM >= 0x071303 /* Available since 7.19.3 */
    HHVM_RC_INT_SAME(CURLAUTH_DIGEST_IE);
#endif

#if LIBCURL_VERSION_NUM >= 0x071304 /* Available since 7.19.4 */
    HHVM_RC_INT_SAME(CURLINFO_CONDITION_UNMET);

    HHVM_RC_INT_SAME(CURLOPT_NOPROXY);
    HHVM_RC_INT_SAME(CURLOPT_PROTOCOLS);
    HHVM_RC_INT_SAME(CURLOPT_REDIR_PROTOCOLS);
    HHVM_RC_INT_SAME(CURLOPT_SOCKS5_GSSAPI_NEC);
    HHVM_RC_INT_SAME(CURLOPT_SOCKS5_GSSAPI_SERVICE);
    HHVM_RC_INT_SAME(CURLOPT_TFTP_BLKSIZE);

    HHVM_RC_INT_SAME(CURLPROTO_ALL);
    HHVM_RC_INT_SAME(CURLPROTO_DICT);
    HHVM_RC_INT_SAME(CURLPROTO_FILE);
    HHVM_RC_INT_SAME(CURLPROTO_FTP);
    HHVM_RC_INT_SAME(CURLPROTO_FTPS);
    HHVM_RC_INT_SAME(CURLPROTO_HTTP);
    HHVM_RC_INT_SAME(CURLPROTO_HTTPS);
    HHVM_RC_INT_SAME(CURLPROTO_LDAP);
    HHVM_RC_INT_SAME(CURLPROTO_LDAPS);
    HHVM_RC_INT_SAME(CURLPROTO_SCP);
    HHVM_RC_INT_SAME(CURLPROTO_SFTP);
    HHVM_RC_INT_SAME(CURLPROTO_TELNET);
    HHVM_RC_INT_SAME(CURLPROTO_TFTP);
#endif

#if LIBCURL_VERSION_NUM >= 0x071306 /* Available since 7.19.6 */
    HHVM_RC_INT_SAME(CURLOPT_SSH_KNOWNHOSTS);
#endif

#if LIBCURL_VERSION_NUM >= 0x071400 /* Available since 7.20.0 */
    HHVM_RC_INT_SAME(CURLINFO_RTSP_CLIENT_CSEQ);
    HHVM_RC_INT_SAME(CURLINFO_RTSP_CSEQ_RECV);
    HHVM_RC_INT_SAME(CURLINFO_RTSP_SERVER_CSEQ);
    HHVM_RC_INT_SAME(CURLINFO_RTSP_SESSION_ID);
    HHVM_RC_INT_SAME(CURLOPT_FTP_USE_PRET);
    HHVM_RC_INT_SAME(CURLOPT_MAIL_FROM);
    HHVM_RC_INT_SAME(CURLOPT_MAIL_RCPT);
    HHVM_RC_INT_SAME(CURLOPT_RTSP_CLIENT_CSEQ);
    HHVM_RC_INT_SAME(CURLOPT_RTSP_REQUEST);
    HHVM_RC_INT_SAME(CURLOPT_RTSP_SERVER_CSEQ);
    HHVM_RC_INT_SAME(CURLOPT_RTSP_SESSION_ID);
    HHVM_RC_INT_SAME(CURLOPT_RTSP_STREAM_URI);
    HHVM_RC_INT_SAME(CURLOPT_RTSP_TRANSPORT);
    HHVM_RC_INT_SAME(CURLPROTO_IMAP);
    HHVM_RC_INT_SAME(CURLPROTO_IMAPS);
    HHVM_RC_INT_SAME(CURLPROTO_POP3);
    HHVM_RC_INT_SAME(CURLPROTO_POP3S);
    HHVM_RC_INT_SAME(CURLPROTO_RTSP);
    HHVM_RC_INT_SAME(CURLPROTO_SMTP);
    HHVM_RC_INT_SAME(CURLPROTO_SMTPS);
    HHVM_RC_INT_SAME(CURL_RTSPREQ_ANNOUNCE);
    HHVM_RC_INT_SAME(CURL_RTSPREQ_DESCRIBE);
    HHVM_RC_INT_SAME(CURL_RTSPREQ_GET_PARAMETER);
    HHVM_RC_INT_SAME(CURL_RTSPREQ_OPTIONS);
    HHVM_RC_INT_SAME(CURL_RTSPREQ_PAUSE);
    HHVM_RC_INT_SAME(CURL_RTSPREQ_PLAY);
    HHVM_RC_INT_SAME(CURL_RTSPREQ_RECEIVE);
    HHVM_RC_INT_SAME(CURL_RTSPREQ_RECORD);
    HHVM_RC_INT_SAME(CURL_RTSPREQ_SETUP);
    HHVM_RC_INT_SAME(CURL_RTSPREQ_SET_PARAMETER);
    HHVM_RC_INT_SAME(CURL_RTSPREQ_TEARDOWN);
#endif

#if LIBCURL_VERSION_NUM >= 0x071500 /* Available since 7.21.0 */
    HHVM_RC_INT_SAME(CURLINFO_LOCAL_IP);
    HHVM_RC_INT_SAME(CURLINFO_LOCAL_PORT);
    HHVM_RC_INT_SAME(CURLINFO_PRIMARY_PORT);
    HHVM_RC_INT_SAME(CURLOPT_FNMATCH_FUNCTION);
    HHVM_RC_INT_SAME(CURLOPT_WILDCARDMATCH);
    HHVM_RC_INT_SAME(CURLPROTO_RTMP);
    HHVM_RC_INT_SAME(CURLPROTO_RTMPE);
    HHVM_RC_INT_SAME(CURLPROTO_RTMPS);
    HHVM_RC_INT_SAME(CURLPROTO_RTMPT);
    HHVM_RC_INT_SAME(CURLPROTO_RTMPTE);
    HHVM_RC_INT_SAME(CURLPROTO_RTMPTS);
    HHVM_RC_INT_SAME(CURL_FNMATCHFUNC_FAIL);
    HHVM_RC_INT_SAME(CURL_FNMATCHFUNC_MATCH);
    HHVM_RC_INT_SAME(CURL_FNMATCHFUNC_NOMATCH);
#endif

#if LIBCURL_VERSION_NUM >= 0x071502 /* Available since 7.21.2 */
    HHVM_RC_INT_SAME(CURLPROTO_GOPHER);
#endif

#if LIBCURL_VERSION_NUM >= 0x071503 /* Available since 7.21.3 */
    HHVM_RC_INT_SAME(CURLAUTH_ONLY);
    HHVM_RC_INT_SAME(CURLOPT_RESOLVE);
#endif

#if LIBCURL_VERSION_NUM >= 0x071504 /* Available since 7.21.4 */
    HHVM_RC_INT_SAME(CURLOPT_TLSAUTH_PASSWORD);
    HHVM_RC_INT_SAME(CURLOPT_TLSAUTH_TYPE);
    HHVM_RC_INT_SAME(CURLOPT_TLSAUTH_USERNAME);
    HHVM_RC_INT_SAME(CURL_TLSAUTH_SRP);
#endif

#if LIBCURL_VERSION_NUM >= 0x071506 /* Available since 7.21.6 */
    HHVM_RC_INT_SAME(CURLOPT_ACCEPT_ENCODING);
    HHVM_RC_INT_SAME(CURLOPT_TRANSFER_ENCODING);
#endif

#if LIBCURL_VERSION_NUM >= 0x071600 /* Available since 7.22.0 */
    HHVM_RC_INT_SAME(CURLGSSAPI_DELEGATION_FLAG);
    HHVM_RC_INT_SAME(CURLGSSAPI_DELEGATION_POLICY_FLAG);
    HHVM_RC_INT_SAME(CURLOPT_GSSAPI_DELEGATION);
#endif

#if LIBCURL_VERSION_NUM >= 0x071800 /* Available since 7.24.0 */
    HHVM_RC_INT_SAME(CURLOPT_ACCEPTTIMEOUT_MS);
    HHVM_RC_INT_SAME(CURLOPT_DNS_SERVERS);
#endif

#if LIBCURL_VERSION_NUM >= 0x071900 /* Available since 7.25.0 */
    HHVM_RC_INT_SAME(CURLOPT_MAIL_AUTH);
    HHVM_RC_INT_SAME(CURLOPT_SSL_OPTIONS);
    HHVM_RC_INT_SAME(CURLOPT_TCP_KEEPALIVE);
    HHVM_RC_INT_SAME(CURLOPT_TCP_KEEPIDLE);
    HHVM_RC_INT_SAME(CURLOPT_TCP_KEEPINTVL);
    HHVM_RC_INT_SAME(CURLSSLOPT_ALLOW_BEAST);
#endif

#if LIBCURL_VERSION_NUM >= 0x071901 /* Available since 7.25.1 */
    HHVM_RC_INT_SAME(CURL_REDIR_POST_303);
#endif

#if LIBCURL_VERSION_NUM >= 0x071c00 /* Available since 7.28.0 */
    HHVM_RC_INT_SAME(CURLSSH_AUTH_AGENT);
#endif

#if LIBCURL_VERSION_NUM >= 0x071e00 /* Available since 7.30.0 */
    HHVM_RC_INT_SAME(CURLMOPT_CHUNK_LENGTH_PENALTY_SIZE);
    HHVM_RC_INT_SAME(CURLMOPT_CONTENT_LENGTH_PENALTY_SIZE);
    HHVM_RC_INT_SAME(CURLMOPT_MAX_HOST_CONNECTIONS);
    HHVM_RC_INT_SAME(CURLMOPT_MAX_PIPELINE_LENGTH);
    HHVM_RC_INT_SAME(CURLMOPT_MAX_TOTAL_CONNECTIONS);
#endif

#if LIBCURL_VERSION_NUM >= 0x071f00 /* Available since 7.31.0 */
    HHVM_RC_INT_SAME(CURLOPT_SASL_IR);
#endif

#if LIBCURL_VERSION_NUM >= 0x072100 /* Available since 7.33.0 */
    HHVM_RC_INT_SAME(CURLOPT_DNS_INTERFACE);
    HHVM_RC_INT_SAME(CURLOPT_DNS_LOCAL_IP4);
    HHVM_RC_INT_SAME(CURLOPT_DNS_LOCAL_IP6);
    HHVM_RC_INT_SAME(CURLOPT_XOAUTH2_BEARER);

    HHVM_RC_INT_SAME(CURL_HTTP_VERSION_2_0);
    HHVM_RC_INT_SAME(CURL_VERSION_HTTP2);
#endif

#if LIBCURL_VERSION_NUM >= 0x072200 /* Available since 7.34.0 */
    HHVM_RC_INT_SAME(CURLOPT_LOGIN_OPTIONS);

    HHVM_RC_INT_SAME(CURL_SSLVERSION_TLSv1_0);
    HHVM_RC_INT_SAME(CURL_SSLVERSION_TLSv1_1);
    HHVM_RC_INT_SAME(CURL_SSLVERSION_TLSv1_2);
#endif

#if LIBCURL_VERSION_NUM >= 0x072400 /* Available since 7.36.0 */
    HHVM_RC_INT_SAME(CURLOPT_EXPECT_100_TIMEOUT_MS);
    HHVM_RC_INT_SAME(CURLOPT_SSL_ENABLE_ALPN);
    HHVM_RC_INT_SAME(CURLOPT_SSL_ENABLE_NPN);
#endif

#if LIBCURL_VERSION_NUM >= 0x072500 /* Available since 7.37.0 */
    HHVM_RC_INT_SAME(CURLHEADER_SEPARATE);
    HHVM_RC_INT_SAME(CURLHEADER_UNIFIED);
    HHVM_RC_INT_SAME(CURLOPT_HEADEROPT);
    HHVM_RC_INT_SAME(CURLOPT_PROXYHEADER);
#endif

#if LIBCURL_VERSION_NUM >= 0x072600 /* Available since 7.38.0 */
    HHVM_RC_INT_SAME(CURLAUTH_NEGOTIATE);
#endif

#if LIBCURL_VERSION_NUM >= 0x072700 /* Available since 7.39.0 */
    HHVM_RC_INT_SAME(CURLOPT_PINNEDPUBLICKEY);
#endif

#if LIBCURL_VERSION_NUM >= 0x072800 /* Available since 7.40.0 */
    HHVM_RC_INT_SAME(CURLOPT_UNIX_SOCKET_PATH);

    HHVM_RC_INT_SAME(CURLPROTO_SMB);
    HHVM_RC_INT_SAME(CURLPROTO_SMBS);
#endif

#if LIBCURL_VERSION_NUM >= 0x072900 /* Available since 7.41.0 */
    HHVM_RC_INT_SAME(CURLOPT_SSL_VERIFYSTATUS);
#endif

#if LIBCURL_VERSION_NUM >= 0x072a00 /* Available since 7.42.0 */
    HHVM_RC_INT_SAME(CURLOPT_PATH_AS_IS);
    HHVM_RC_INT_SAME(CURLOPT_SSL_FALSESTART);
#endif

#if LIBCURL_VERSION_NUM >= 0x072b00 /* Available since 7.43.0 */
    HHVM_RC_INT_SAME(CURL_HTTP_VERSION_2);

    HHVM_RC_INT_SAME(CURLOPT_PIPEWAIT);
    HHVM_RC_INT_SAME(CURLOPT_PROXY_SERVICE_NAME);
    HHVM_RC_INT_SAME(CURLOPT_SERVICE_NAME);

    HHVM_RC_INT_SAME(CURLPIPE_NOTHING);
    HHVM_RC_INT_SAME(CURLPIPE_HTTP1);
    HHVM_RC_INT_SAME(CURLPIPE_MULTIPLEX);
#endif

#if LIBCURL_VERSION_NUM >= 0x072c00 /* Available since 7.44.0 */
    HHVM_RC_INT_SAME(CURLSSLOPT_NO_REVOKE);
#endif

#if LIBCURL_VERSION_NUM >= 0x072d00 /* Available since 7.45.0 */
    HHVM_RC_INT_SAME(CURLOPT_DEFAULT_PROTOCOL);
#endif

#if LIBCURL_VERSION_NUM >= 0x072e00 /* Available since 7.46.0 */
    HHVM_RC_INT_SAME(CURLOPT_STREAM_WEIGHT);
    HHVM_RC_INT_SAME(CURLMOPT_PUSHFUNCTION);
    HHVM_RC_INT_SAME(CURL_PUSH_OK);
    HHVM_RC_INT_SAME(CURL_PUSH_DENY);
#endif

#if LIBCURL_VERSION_NUM >= 0x072f00 /* Available since 7.47.0 */
    HHVM_RC_INT_SAME(CURL_HTTP_VERSION_2TLS);
#endif

#if LIBCURL_VERSION_NUM >= 0x073000 /* Available since 7.48.0 */
    HHVM_RC_INT_SAME(CURLOPT_TFTP_NO_OPTIONS);
#endif

#if LIBCURL_VERSION_NUM >= 0x073100 /* Available since 7.49.0 */
    HHVM_RC_INT_SAME(CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE);
    HHVM_RC_INT_SAME(CURLOPT_CONNECT_TO);
    HHVM_RC_INT_SAME(CURLOPT_TCP_FASTOPEN);
#endif

#if LIBCURL_VERSION_NUM >= 0x073200 /* Available since 7.50.0 */
    HHVM_RC_INT_SAME(CURLINFO_HTTP_VERSION)
#endif

#if LIBCURL_VERSION_NUM >= 0x073300 /* Available since 7.51.0 */
    HHVM_RC_INT_SAME(CURLOPT_KEEP_SENDING_ON_ERROR)
#endif

#if LIBCURL_VERSION_NUM >= 0x073400 /* Available since 7.52.0 */
    HHVM_RC_INT_SAME(CURL_SSLVERSION_TLSv1_3)
    HHVM_RC_INT_SAME(CURLINFO_SCHEME)
    HHVM_RC_INT_SAME(CURLINFO_PROTOCOL)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_CAINFO)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_CAPATH)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_CRLFILE)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_KEYPASSWD)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_PINNEDPUBLICKEY)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_SSLCERT)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_SSLCERTTYPE)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_SSLKEY)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_SSLKEYTYPE)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_SSLVERSION)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_SSL_CIPHER_LIST)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_SSL_OPTIONS)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_SSL_VERIFYHOST)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_SSL_VERIFYPEER)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_TLSAUTH_PASSWORD)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_TLSAUTH_TYPE)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_TLSAUTH_USERNAME)
    HHVM_RC_INT_SAME(CURLPROXY_HTTPS)
#endif

#if LIBCURL_VERSION_NUM >= 0x073500 /* Available since 7.53.0 */
    HHVM_RC_INT_SAME(CURLOPT_ABSTRACT_UNIX_SOCKET)
#endif

#if LIBCURL_VERSION_NUM >= 0x073600 /* Available since 7.54.0 */
    HHVM_RC_INT_SAME(CURLOPT_SUPPRESS_CONNECT_HEADERS)

    HHVM_RC_INT_SAME(CURL_SSLVERSION_MAX_DEFAULT)
    HHVM_RC_INT_SAME(CURL_SSLVERSION_MAX_TLSv1_0)
    HHVM_RC_INT_SAME(CURL_SSLVERSION_MAX_TLSv1_1)
    HHVM_RC_INT_SAME(CURL_SSLVERSION_MAX_TLSv1_2)
    HHVM_RC_INT_SAME(CURL_SSLVERSION_MAX_TLSv1_3)
#endif

#if LIBCURL_VERSION_NUM >= 0x073700 /* Available since 7.55.0 */
    HHVM_RC_INT_SAME(CURLOPT_REQUEST_TARGET)
    HHVM_RC_INT_SAME(CURLOPT_SOCKS5_AUTH)
#endif

#if LIBCURL_VERSION_NUM >= 0x073800 /* Available since 7.56.0 */
    HHVM_RC_INT_SAME(CURLOPT_SSH_COMPRESSION)
    HHVM_RC_INT_SAME(CURL_VERSION_MULTI_SSL)
#endif

#if LIBCURL_VERSION_NUM >= 0x073900 /* Available since 7.57.0 */
    HHVM_RC_INT_SAME(CURL_VERSION_BROTLI)
#endif

#if LIBCURL_VERSION_NUM >= 0x073a00 /* Available since 7.58.0 */
    HHVM_RC_INT_SAME(CURLSSH_AUTH_GSSAPI)
#endif

#if LIBCURL_VERSION_NUM >= 0x073b00 /* Available since 7.59.0 */
    HHVM_RC_INT_SAME(CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS)
    HHVM_RC_INT_SAME(CURLOPT_TIMEVALUE_LARGE)
#endif

#if LIBCURL_VERSION_NUM >= 0x073c00 /* Available since 7.60.0 */
    HHVM_RC_INT_SAME(CURLOPT_DNS_SHUFFLE_ADDRESSES)
    HHVM_RC_INT_SAME(CURLOPT_HAPROXYPROTOCOL)
#endif

#if LIBCURL_VERSION_NUM >= 0x073d00 /* Available since 7.61.0 */
    HHVM_RC_INT_SAME(CURLAUTH_BEARER)
    HHVM_RC_INT_SAME(CURLOPT_DISALLOW_USERNAME_IN_URL)
    HHVM_RC_INT_SAME(CURLOPT_PROXY_TLS13_CIPHERS)
    HHVM_RC_INT_SAME(CURLOPT_TLS13_CIPHERS)
#endif

#if LIBCURL_VERSION_NUM >= 0x073e00 /* Available since 7.62.0 */
    HHVM_RC_INT_SAME(CURLOPT_DOH_URL);
    HHVM_RC_INT_SAME(CURLOPT_UPKEEP_INTERVAL_MS);
    HHVM_RC_INT_SAME(CURLOPT_UPLOAD_BUFFERSIZE);
#endif

#if CURLOPT_FTPASCII != 0
    HHVM_RC_INT_SAME(CURLOPT_FTPASCII);
#endif
#if CURLOPT_MUTE != 0
    HHVM_RC_INT_SAME(CURLOPT_MUTE);
#endif
#if CURLOPT_PASSWDFUNCTION != 0
    HHVM_RC_INT_SAME(CURLOPT_PASSWDFUNCTION);
#endif
    HHVM_RC_INT_SAME(CURLOPT_SAFE_UPLOAD);

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
    HHVM_FALIAS(HH\\curl_init_pooled, curl_init_pooled);
    HHVM_FALIAS(HH\\curl_create_pool, curl_create_pool);
    HHVM_FALIAS(HH\\curl_destroy_pool, curl_destroy_pool);
    HHVM_FALIAS(HH\\curl_list_pools, curl_list_pools);
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
    HHVM_FE(curl_share_init);
    HHVM_FE(curl_share_setopt);
    HHVM_FE(curl_share_close);

    Extension* ext = ExtensionRegistry::get("curl");
    assertx(ext);

    IniSetting::Bind(ext, IniSetting::PHP_INI_SYSTEM, "curl.namedPools",
      "", &s_namedPools);
    if (s_namedPools.length() > 0) {

      // split on commas, search and bind ini settings for each pool
      std::vector<string> pools;
      boost::split(pools, s_namedPools, boost::is_any_of(","));

      for (std::string poolname: pools) {
        if (poolname.length() == 0) { continue; }

        // get the user-entered settings for this pool, if there are any
        std::string sizeIni = "curl.namedPools." + poolname + ".size";
        std::string reuseLimitIni =
          "curl.namedPools." + poolname + ".reuseLimit";
        std::string connGetTimeoutIni =
          "curl.namedPools." + poolname + ".connGetTimeout";

        IniSetting::Bind(ext, IniSetting::PHP_INI_SYSTEM, sizeIni,
            "5", &s_sizeVal);
        IniSetting::Bind(ext, IniSetting::PHP_INI_SYSTEM, reuseLimitIni,
            "100", &s_reuseLimitVal);
        IniSetting::Bind(ext, IniSetting::PHP_INI_SYSTEM, connGetTimeoutIni,
            "5000", &s_connGetTimeoutVal);

        auto hp = std::make_shared<CurlHandlePool>(s_sizeVal,
                                                   s_connGetTimeoutVal,
                                                   s_reuseLimitVal);
        CurlHandlePool::namedPools[poolname] = hp;
      }
    }

    loadSystemlib();
  }

  void moduleShutdown() override {
    CurlHandlePool::namedPools.clear();
  }

} s_curl_extension;

}

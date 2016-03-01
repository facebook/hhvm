/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/asio/socket-event.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/libevent-http-client.h"
#include "hphp/runtime/base/curl-tls-workarounds.h"
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

#define CURLOPT_BINARYTRANSFER 19914
#define CURLOPT_MUTE -2
#define CURLOPT_PASSWDFUNCTION -3
#define PHP_CURL_STDOUT 0
#define PHP_CURL_FILE   1
#define PHP_CURL_USER   2
#define PHP_CURL_DIRECT 3
#define PHP_CURL_RETURN 4
#define PHP_CURL_ASCII  5
#define PHP_CURL_BINARY 6
#define PHP_CURL_IGNORE 7


namespace HPHP {

using std::string;
using std::vector;

namespace {

const StaticString
  s_exception("exception"),
  s_previous("previous");

using ExceptionType = folly::Optional<boost::variant<Object,Exception*>>;

bool isPhpException(const ExceptionType& e) {
  return e && boost::get<Object>(&e.value()) != nullptr;
}

Object getPhpException(const ExceptionType& e) {
  assert(e && isPhpException(e));
  return boost::get<Object>(*e);
}

Exception* getCppException(const ExceptionType& e) {
  assert(e && !isPhpException(e));
  return boost::get<Exception*>(*e);
}

void throwException(ExceptionType& ex) {
  if (!ex) return;
  auto e = std::move(ex);
  assert(!ex);
  if (isPhpException(e)) {
    throw getPhpException(e);
  } else {
    getCppException(e)->throwException();
  }
}

}

///////////////////////////////////////////////////////////////////////////////
/**
 * This is a helper class used to wrap Curl handles that are pooled.
 * Operations on this class are _NOT_ thread safe!
 */
class PooledCurlHandle {
public:
  explicit PooledCurlHandle(int connRecycleAfter)
  : m_handle(nullptr), m_numUsages(0), m_connRecycleAfter(connRecycleAfter) { }

  CURL* useHandle() {
    if (m_handle == nullptr) {
      m_handle = curl_easy_init();
    }

    if (m_connRecycleAfter > 0 &&
        m_numUsages % m_connRecycleAfter == 0) {
      curl_easy_cleanup(m_handle);
      m_handle = curl_easy_init();
      m_numUsages = 0;
    }

    m_numUsages++;
    return m_handle;
  }

  void resetHandle() {
    if (m_handle != nullptr) {
      curl_easy_reset(m_handle);
    }
  }

  ~PooledCurlHandle() {
    if (m_handle != nullptr) {
      curl_easy_cleanup(m_handle);
    }
  }

private:
  CURL* m_handle;
  int m_numUsages;
  int m_connRecycleAfter;
};

///////////////////////////////////////////////////////////////////////////////
/**
 * This is a helper class used to implement a process-wide pool of libcurl
 * handles. This provides very large performance benefits, as libcurl handles
 * hold connections open and cache SSL session ids over their lifetimes.
 * All operations on this class are thread safe.
 */
class CurlHandlePool {
public:
  static std::map<std::string, CurlHandlePool*> namedPools;

  explicit CurlHandlePool(int poolSize, int waitTimeout, int numConnReuses)
  : m_waitTimeout(waitTimeout) {
    for (int i = 0; i < poolSize; i++) {
      m_handleStack.push(new PooledCurlHandle(numConnReuses));
    }
    pthread_cond_init(&m_cond, nullptr);
  }

  PooledCurlHandle* fetch() {
    Lock lock(m_mutex);

    // wait until the user-specified timeout for an available handle
    struct timespec ts;
    gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += m_waitTimeout / 1000;
    ts.tv_nsec += 1000000 * (m_waitTimeout % 1000);
    while (m_handleStack.empty()) {
      if (ETIMEDOUT == pthread_cond_timedwait(&m_cond, &m_mutex.getRaw(), &ts))
      {
        raise_error("Timeout reached waiting for an "
                    "available pooled curl connection!");
      }
    }

    PooledCurlHandle* ret = m_handleStack.top();
    assert(ret);
    m_handleStack.pop();
    return ret;
  }

  void store(PooledCurlHandle* handle) {
    Lock lock(m_mutex);
    handle->resetHandle();
    m_handleStack.push(handle);
    pthread_cond_signal(&m_cond);
  }

  ~CurlHandlePool() {
    Lock lock(m_mutex);
    while (!m_handleStack.empty()) {
      PooledCurlHandle *handle = m_handleStack.top();
      m_handleStack.pop();
      delete handle;
    }
  }

private:
  std::stack<PooledCurlHandle*> m_handleStack;
  Mutex m_mutex;
  pthread_cond_t m_cond;
  int m_waitTimeout;
};

std::map<std::string, CurlHandlePool*> CurlHandlePool::namedPools;

///////////////////////////////////////////////////////////////////////////////
// helper data structure

class CurlResource : public SweepableResourceData {
private:
  DECLARE_RESOURCE_ALLOCATION(CurlResource)

  class WriteHandler {
  public:
    WriteHandler() : method(0), type(0) {}

    int                method;
    Variant            callback;
    req::ptr<File>     fp;
    StringBuffer       buf;
    String             content;
    int                type;
  };

  class ReadHandler {
  public:
    ReadHandler() : method(0) {}

    int                method;
    Variant            callback;
    req::ptr<File>     fp;
  };

  class ToFree {
  public:
    std::vector<char*>          str;
    std::vector<curl_httppost*> post;
    std::vector<curl_slist*>    slist;

    ~ToFree() {
      for (unsigned int i = 0; i < str.size(); i++) {
        free(str[i]);
      }
      for (unsigned int i = 0; i < post.size(); i++) {
        curl_formfree(post[i]);
      }
      for (unsigned int i = 0; i < slist.size(); i++) {
        curl_slist_free_all(slist[i]);
      }
    }
  };

public:
  CLASSNAME_IS("curl")
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  explicit CurlResource(const String& url, CurlHandlePool *pool = nullptr)
  : m_emptyPost(true), m_connPool(pool), m_pooledHandle(nullptr) {
    if (m_connPool) {
      m_pooledHandle = m_connPool->fetch();
      m_cp = m_pooledHandle->useHandle();
    } else {
      m_cp = curl_easy_init();
    }
    m_url = url;

    memset(m_error_str, 0, sizeof(m_error_str));
    m_error_no = CURLE_OK;
    m_to_free = std::make_shared<ToFree>();

    m_write.method = PHP_CURL_STDOUT;
    m_write.type   = PHP_CURL_ASCII;
    m_read.method  = PHP_CURL_DIRECT;
    m_write_header.method = PHP_CURL_IGNORE;

    reset();

    if (!url.empty()) {
#if LIBCURL_VERSION_NUM >= 0x071100
      /* Strings passed to libcurl as 'char *' arguments, are copied by
         the library... NOTE: before 7.17.0 strings were not copied. */
      curl_easy_setopt(m_cp, CURLOPT_URL, url.c_str());
#else
      char *urlcopy = strndup(url.data(), url.size());
      curl_easy_setopt(m_cp, CURLOPT_URL, urlcopy);
      m_to_free->str.push_back(urlcopy);
#endif
    }
  }

  explicit CurlResource(req::ptr<CurlResource> src)
  : m_connPool(nullptr), m_pooledHandle(nullptr) {
    // NOTE: we never pool copied curl handles, because all spots in
    // the pool are pre-populated

    assert(src && src != this);
    assert(!src->m_exception);

    m_cp = curl_easy_duphandle(src->get());
    m_url = src->m_url;

    memset(m_error_str, 0, sizeof(m_error_str));
    m_error_no = CURLE_OK;

    m_write.method = src->m_write.method;
    m_write.type   = src->m_write.type;
    m_read.method  = src->m_read.method;
    m_write_header.method = src->m_write_header.method;

    m_write.fp        = src->m_write.fp;
    m_write_header.fp = src->m_write_header.fp;
    m_read.fp         = src->m_read.fp;

    m_write.callback = src->m_write.callback;
    m_read.callback = src->m_read.callback;
    m_write_header.callback = src->m_write_header.callback;

    reseat();

    m_to_free = src->m_to_free;
    m_emptyPost = src->m_emptyPost;
  }

  ~CurlResource() {
    close();
  }

  bool isInvalid() const override {
    return !m_cp;
  }

  void closeForSweep() {
    assert(!m_exception);
    if (m_cp) {
      if (m_connPool) {
        // reuse this curl handle if we're pooling
        assert(m_pooledHandle);
        m_connPool->store(m_pooledHandle);
        m_pooledHandle = nullptr;
      } else {
        curl_easy_cleanup(m_cp);
      }
      m_cp = nullptr;
    }
    m_to_free.reset();
  }

  void close() {
    closeForSweep();
    m_opts.clear();
  }

  void check_exception() {
    throwException(m_exception);
  }

  ExceptionType getAndClearException() {
    return std::move(m_exception);
  }

  static int64_t minTimeout(int64_t timeout) {
    auto info = ThreadInfo::s_threadInfo.getNoCheck();
    auto& data = info->m_reqInjectionData;
    if (!data.getTimeout()) {
      return timeout;
    }
    auto remaining = int64_t(data.getRemainingTime());
    return std::min(remaining, timeout);
  }

  static int64_t minTimeoutMS(int64_t timeout) {
    auto info = ThreadInfo::s_threadInfo.getNoCheck();
    auto& data = info->m_reqInjectionData;
    if (!data.getTimeout()) {
      return timeout;
    }
    auto remaining = int64_t(data.getRemainingTime());
    return std::min(1000 * remaining, timeout);
  }

  void reseat() {
    // Note: this is the minimum set of things to point the CURL*
    // to this CurlHandle
    curl_easy_setopt(m_cp, CURLOPT_ERRORBUFFER,       m_error_str);
    curl_easy_setopt(m_cp, CURLOPT_FILE,              (void*)this);
    curl_easy_setopt(m_cp, CURLOPT_INFILE,            (void*)this);
    curl_easy_setopt(m_cp, CURLOPT_WRITEHEADER,       (void*)this);
    curl_easy_setopt(m_cp, CURLOPT_SSL_CTX_DATA,      (void*)this);
  }

  void reset() {
    curl_easy_reset(m_cp);

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

  Variant execute() {
    assert(!m_exception);
    if (m_cp == nullptr) {
      return false;
    }
    if (m_emptyPost) {
      // As per curl docs, an empty post must set POSTFIELDSIZE to be 0 or
      // the reader function will be called
      curl_easy_setopt(m_cp, CURLOPT_POSTFIELDSIZE, 0);
    }
    m_write.buf.clear();
    m_write.content.clear();
    m_header.clear();
    memset(m_error_str, 0, sizeof(m_error_str));

    {
      IOStatusHelper io("curl_easy_perform", m_url.data());
      SYNC_VM_REGS_SCOPED();
      m_error_no = curl_easy_perform(m_cp);
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

  String getUrl() {
    return m_url;
  }

  String getHeader() {
    return m_header;
  }

  String getContents() {
    if (m_write.method == PHP_CURL_RETURN) {
      if (!m_write.buf.empty()) {
        m_write.content = m_write.buf.detach();
      }
      return m_write.content;
    }
    return String();
  }

  bool setOption(long option, const Variant& value) {
    if (m_cp == nullptr) {
      return false;
    }
    m_error_no = CURLE_OK;

    switch (option) {
    case CURLOPT_TIMEOUT: {
      auto timeout = minTimeout(value.toInt64());
      m_error_no = curl_easy_setopt(m_cp, (CURLoption)option, timeout);
      break;
    }
#if LIBCURL_VERSION_NUM >= 0x071002
    case CURLOPT_TIMEOUT_MS: {
      auto timeout = minTimeoutMS(value.toInt64());
      m_error_no = curl_easy_setopt(m_cp, (CURLoption)option, timeout);
      break;
    }
#endif
    case CURLOPT_INFILESIZE:
    case CURLOPT_VERBOSE:
    case CURLOPT_HEADER:
    case CURLOPT_NOPROGRESS:
    case CURLOPT_NOBODY:
    case CURLOPT_FAILONERROR:
    case CURLOPT_UPLOAD:
    case CURLOPT_POST:
#if LIBCURL_VERSION_NUM >= 0x071301
    case CURLOPT_POSTREDIR:
#endif
    case CURLOPT_PROTOCOLS:
    case CURLOPT_REDIR_PROTOCOLS:
    case CURLOPT_FTPLISTONLY:
    case CURLOPT_FTPAPPEND:
    case CURLOPT_NETRC:
    case CURLOPT_PUT:
    case CURLOPT_FTP_USE_EPSV:
    case CURLOPT_LOW_SPEED_LIMIT:
    case CURLOPT_SSLVERSION:
    case CURLOPT_LOW_SPEED_TIME:
    case CURLOPT_RESUME_FROM:
    case CURLOPT_TIMEVALUE:
    case CURLOPT_TIMECONDITION:
    case CURLOPT_TRANSFERTEXT:
    case CURLOPT_HTTPPROXYTUNNEL:
    case CURLOPT_FILETIME:
    case CURLOPT_MAXREDIRS:
    case CURLOPT_MAXCONNECTS:
    case CURLOPT_CLOSEPOLICY:
    case CURLOPT_FRESH_CONNECT:
    case CURLOPT_FORBID_REUSE:
    case CURLOPT_CONNECTTIMEOUT:
#if LIBCURL_VERSION_NUM >= 0x071002
    case CURLOPT_CONNECTTIMEOUT_MS:
#endif
    case CURLOPT_SSL_VERIFYHOST:
    case CURLOPT_SSL_VERIFYPEER:
      //case CURLOPT_DNS_USE_GLOBAL_CACHE: not thread-safe when set to true
    case CURLOPT_NOSIGNAL:
    case CURLOPT_PROXYTYPE:
    case CURLOPT_BUFFERSIZE:
    case CURLOPT_HTTPGET:
    case CURLOPT_HTTP_VERSION:
    case CURLOPT_CRLF:
    case CURLOPT_DNS_CACHE_TIMEOUT:
    case CURLOPT_PROXYPORT:
    case CURLOPT_FTP_USE_EPRT:
    case CURLOPT_HTTPAUTH:
    case CURLOPT_PROXYAUTH:
    case CURLOPT_FTP_CREATE_MISSING_DIRS:
    case CURLOPT_FTPSSLAUTH:
    case CURLOPT_FTP_SSL:
    case CURLOPT_UNRESTRICTED_AUTH:
    case CURLOPT_PORT:
    case CURLOPT_AUTOREFERER:
    case CURLOPT_COOKIESESSION:
    case CURLOPT_TCP_NODELAY:
    case CURLOPT_IPRESOLVE:
    case CURLOPT_FOLLOWLOCATION:
      m_error_no = curl_easy_setopt(m_cp, (CURLoption)option, value.toInt64());
      break;
    case CURLOPT_RETURNTRANSFER:
      m_write.method = value.toBoolean() ? PHP_CURL_RETURN : PHP_CURL_STDOUT;
      break;
    case CURLOPT_BINARYTRANSFER:
      m_write.type = value.toBoolean() ? PHP_CURL_BINARY : PHP_CURL_ASCII;
      break;
    case CURLOPT_PRIVATE:
    case CURLOPT_URL:
    case CURLOPT_PROXY:
    case CURLOPT_USERPWD:
    case CURLOPT_PROXYUSERPWD:
    case CURLOPT_RANGE:
    case CURLOPT_CUSTOMREQUEST:
    case CURLOPT_USERAGENT:
    case CURLOPT_FTPPORT:
    case CURLOPT_COOKIE:
    case CURLOPT_REFERER:
    case CURLOPT_INTERFACE:
    case CURLOPT_KRB4LEVEL:
    case CURLOPT_EGDSOCKET:
    case CURLOPT_CAINFO:
    case CURLOPT_CAPATH:
#ifdef FACEBOOK
    case CURLOPT_SERVICE_NAME:
#endif
    case CURLOPT_SSL_CIPHER_LIST:
    case CURLOPT_SSLKEY:
    case CURLOPT_SSLKEYTYPE:
    case CURLOPT_SSLKEYPASSWD:
    case CURLOPT_SSLENGINE:
    case CURLOPT_SSLENGINE_DEFAULT:
    case CURLOPT_SSLCERTTYPE:
    case CURLOPT_ENCODING:
    case CURLOPT_COOKIEJAR:
    case CURLOPT_SSLCERT:
    case CURLOPT_RANDOM_FILE:
    case CURLOPT_COOKIEFILE:
      {
        String svalue = value.toString();
#if LIBCURL_VERSION_NUM >= 0x071100
        /* Strings passed to libcurl as 'char *' arguments, are copied
           by the library... NOTE: before 7.17.0 strings were not copied. */
        m_error_no = curl_easy_setopt(m_cp, (CURLoption)option, svalue.c_str());
#else
        char *copystr = strndup(svalue.data(), svalue.size());
        m_to_free->str.push_back(copystr);
        m_error_no = curl_easy_setopt(m_cp, (CURLoption)option, copystr);
#endif
        if (option == CURLOPT_URL) m_url = value;
      }
      break;
    case CURLOPT_FILE:
    case CURLOPT_INFILE:
    case CURLOPT_WRITEHEADER:
    case CURLOPT_STDERR:
      {
        auto fp = dyn_cast_or_null<File>(value);
        if (!fp) return false;

        switch (option) {
          case CURLOPT_FILE:
            m_write.fp = fp;
            m_write.method = PHP_CURL_FILE;
            break;
          case CURLOPT_WRITEHEADER:
            m_write_header.fp = fp;
            m_write_header.method = PHP_CURL_FILE;
            break;
          case CURLOPT_INFILE:
            m_read.fp = fp;
            m_emptyPost = false;
            break;
          default: {
            auto pf = dyn_cast<PlainFile>(fp);
            if (!pf) {
              return false;
            }
            FILE *fp = pf->getStream();
            if (!fp) {
              return false;
            }
            m_error_no = curl_easy_setopt(m_cp, (CURLoption)option, fp);
            break;
          }
        }
      }
      break;
    case CURLOPT_WRITEFUNCTION:
      m_write.callback = value;
      m_write.method = PHP_CURL_USER;
      break;
    case CURLOPT_READFUNCTION:
      m_read.callback = value;
      m_read.method = PHP_CURL_USER;
      m_emptyPost = false;
      break;
    case CURLOPT_HEADERFUNCTION:
      m_write_header.callback = value;
      m_write_header.method = PHP_CURL_USER;
      break;
    case CURLOPT_PROGRESSFUNCTION:
      m_progress_callback = value;
      curl_easy_setopt(m_cp, CURLOPT_PROGRESSDATA, (void*) this);
      curl_easy_setopt(m_cp, CURLOPT_PROGRESSFUNCTION, curl_progress);
      break;
    case CURLOPT_POSTFIELDS:
      m_emptyPost = false;
      if (value.isArray() || value.is(KindOfObject)) {
        Array arr = value.toArray();
        curl_httppost *first = nullptr;
        curl_httppost *last  = nullptr;
        for (ArrayIter iter(arr); iter; ++iter) {
          String key = iter.first().toString();
          Variant var_val = iter.second();
          if (UNLIKELY(var_val.isObject()
              && var_val.toObject()->instanceof(SystemLib::s_CURLFileClass))) {
            Object val = var_val.toObject();

            static const StaticString s_name("name");
            static const StaticString s_mime("mime");
            static const StaticString s_postname("postname");

            String name = val.o_get(s_name).toString();
            String mime = val.o_get(s_mime).toString();
            String postname = val.o_get(s_postname).toString();

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
            const char *postval = val.data();

            if (*postval == '@' && strlen(postval) == val.size()) {
              /* Given a string like:
               *   "@/foo/bar;type=herp/derp;filename=ponies\0"
               * - Temporarily convert to:
               *   "@/foo/bar\0type=herp/derp\0filename=ponies\0"
               * - Pass pointers to the relevant null-terminated substrings to
               *   curl_formadd
               * - Revert changes to postval at the end
               */
              char* mutablePostval = const_cast<char*>(postval) + 1;
              char* type = strstr(mutablePostval, ";type=");
              char* filename = strstr(mutablePostval, ";filename=");

              if (type) {
                *type = '\0';
              }
              if (filename) {
                *filename = '\0';
              }

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

              if (type) {
                *type = ';';
              }
              if (filename) {
                *filename = ';';
              }
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

      } else {
        String svalue = value.toString();
#if LIBCURL_VERSION_NUM >= 0x071100
        /* with curl 7.17.0 and later, we can use COPYPOSTFIELDS,
           but we have to provide size before */
        m_error_no = curl_easy_setopt(m_cp, CURLOPT_POSTFIELDSIZE,
                                      svalue.size());
        m_error_no = curl_easy_setopt(m_cp, CURLOPT_COPYPOSTFIELDS,
                                      svalue.c_str());
#else
        char *post = strndup(svalue.data(), svalue.size());
        m_to_free->str.push_back(post);

        m_error_no = curl_easy_setopt(m_cp, CURLOPT_POSTFIELDS, post);
        m_error_no = curl_easy_setopt(m_cp, CURLOPT_POSTFIELDSIZE,
                                      svalue.size());
#endif
      }
      break;
    case CURLOPT_HTTPHEADER:
    case CURLOPT_QUOTE:
    case CURLOPT_HTTP200ALIASES:
    case CURLOPT_POSTQUOTE:
    case CURLOPT_RESOLVE:
      if (value.isArray() || value.is(KindOfObject)) {
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

      } else {
        raise_warning("You must pass either an object or an array with "
                      "the CURLOPT_HTTPHEADER, CURLOPT_QUOTE, "
                      "CURLOPT_HTTP200ALIASES, CURLOPT_POSTQUOTE "
                      "and CURLOPT_RESOLVE arguments");
        return false;
      }
      break;

    case CURLINFO_HEADER_OUT:
      if (value.toInt64() == 1) {
        curl_easy_setopt(m_cp, CURLOPT_DEBUGFUNCTION, curl_debug);
        curl_easy_setopt(m_cp, CURLOPT_DEBUGDATA, (void *)this);
        curl_easy_setopt(m_cp, CURLOPT_VERBOSE, 1);
      } else {
        curl_easy_setopt(m_cp, CURLOPT_DEBUGFUNCTION, nullptr);
        curl_easy_setopt(m_cp, CURLOPT_DEBUGDATA, nullptr);
        curl_easy_setopt(m_cp, CURLOPT_VERBOSE, 0);
      }
      break;

    case CURLOPT_FB_TLS_VER_MAX:
      {
        int64_t val = value.toInt64();
        if (value.isInteger() &&
            (val == CURLOPT_FB_TLS_VER_MAX_1_0 ||
             val == CURLOPT_FB_TLS_VER_MAX_1_1 ||
             val == CURLOPT_FB_TLS_VER_MAX_NONE)) {
            m_opts.set(int64_t(option), value);
        } else {
          raise_warning("You must pass CURLOPT_FB_TLS_VER_MAX_1_0, "
                        "CURLOPT_FB_TLS_VER_MAX_1_1 or "
                        "CURLOPT_FB_TLS_VER_MAX_NONE with "
                        "CURLOPT_FB_TLS_VER_MAX");
        }
      }
      break;
    case CURLOPT_FB_TLS_CIPHER_SPEC:
      if (value.isString() && !value.toString().empty()) {
        m_opts.set(int64_t(option), value);
      } else {
        raise_warning("CURLOPT_FB_TLS_CIPHER_SPEC requires a non-empty string");
      }
      break;

    default:
      m_error_no = CURLE_FAILED_INIT;
      throw_invalid_argument("option: %ld", option);
      break;
    }

    m_opts.set(int64_t(option), value);

    return m_error_no == CURLE_OK;
  }

  Variant getOption(long option) {

    if (option != 0) {
      if (!m_opts.exists(int64_t(option))) {
        return false;
      }
      return m_opts[int64_t(option)];
    }

    return m_opts;
  }

  static int curl_debug(CURL *cp, curl_infotype type, char *buf,
                        size_t buf_len, void *ctx) {
    CurlResource *ch = (CurlResource *)ctx;
    if (type == CURLINFO_HEADER_OUT && buf_len > 0) {
      ch->m_header = String(buf, buf_len, CopyString);
    }
    return 0;
  }

  Variant do_callback(const Variant& cb, const Array& args) {
    assert(!m_exception);
    try {
      return vm_call_user_func(cb, args);
    } catch (const Object &e) {
      m_exception.assign(e);
    } catch (Exception &e) {
      m_exception.assign(e.clone());
    }
    return init_null();
  }

  static int curl_progress(void* p,
                           double dltotal, double dlnow,
                           double ultotal, double ulnow) {
    assert(p);
    CurlResource* curl = static_cast<CurlResource*>(p);

    PackedArrayInit pai(5);
    pai.append(Resource(curl));
    pai.append(dltotal);
    pai.append(dlnow);
    pai.append(ultotal);
    pai.append(ulnow);

    Variant result = vm_call_user_func(
      curl->m_progress_callback,
      pai.toArray()
    );
    // Both PHP and libcurl are documented as return 0 to continue, non-zero
    // to abort, however this is what Zend actually implements
    return result.toInt64() == 0 ? 0 : 1;
  }

  static size_t curl_read(char *data, size_t size, size_t nmemb, void *ctx) {
    CurlResource *ch = (CurlResource *)ctx;
    ReadHandler *t  = &ch->m_read;

    int length = -1;
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
    case PHP_CURL_USER:
      {
        int data_size = size * nmemb;
        Variant ret = ch->do_callback(
          t->callback,
          make_packed_array(Resource(ch), Resource(t->fp), data_size));
        if (ret.isString()) {
          String sret = ret.toString();
          length = data_size < sret.size() ? data_size : sret.size();
          memcpy(data, sret.data(), length);
        }
        break;
      }
    }
    return length;
  }

  static size_t curl_write(char *data, size_t size, size_t nmemb, void *ctx) {
    CurlResource *ch = (CurlResource *)ctx;
    WriteHandler *t  = &ch->m_write;
    size_t length = size * nmemb;

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
    case PHP_CURL_USER:
      {
        Variant ret = ch->do_callback(
          t->callback,
          make_packed_array(Resource(ch), String(data, length, CopyString)));
        length = ret.toInt64();
      }
      break;
    }

    return length;
  }

  static size_t curl_write_header(char *data, size_t size, size_t nmemb,
                                  void *ctx) {
    CurlResource *ch = (CurlResource *)ctx;
    WriteHandler *t  = &ch->m_write_header;
    size_t length = size * nmemb;

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
    case PHP_CURL_USER:
      {
        Variant ret = ch->do_callback(
          t->callback,
          make_packed_array(Resource(ch), String(data, length, CopyString)));
        length = ret.toInt64();
      }
      break;
    case PHP_CURL_IGNORE:
      return length;
    default:
      return (size_t)-1;
    }

    return length;
  }

  CURL *get(bool nullOkay = false) {
    if (m_cp == nullptr && !nullOkay) {
      throw_null_pointer_exception();
    }
    return m_cp;
  }

  int getError() {
    return m_error_no;
  }

  String getErrorString() {
    return String(m_error_str, CopyString);
  }

  typedef enum {
    CURLOPT_FB_TLS_VER_MAX = 2147482624,
    CURLOPT_FB_TLS_VER_MAX_NONE = 2147482625,
    CURLOPT_FB_TLS_VER_MAX_1_1 = 2147482626,
    CURLOPT_FB_TLS_VER_MAX_1_0 = 2147482627,
    CURLOPT_FB_TLS_CIPHER_SPEC = 2147482628
  } fb_specific_options;

private:
  CURL *m_cp;
  ExceptionType m_exception;

  char m_error_str[CURL_ERROR_SIZE + 1];
  CURLcode m_error_no;

  std::shared_ptr<ToFree> m_to_free;

  String m_url;
  String m_header;
  Array  m_opts;

  WriteHandler m_write;
  WriteHandler m_write_header;
  ReadHandler  m_read;
  Variant      m_progress_callback;

  bool m_emptyPost;
  CurlHandlePool* m_connPool;
  PooledCurlHandle* m_pooledHandle;

  static CURLcode ssl_ctx_callback(CURL *curl, void *sslctx, void *parm);
};

void CurlResource::sweep() {
  m_write.buf.release();
  m_write_header.buf.release();
  closeForSweep();
}

CURLcode CurlResource::ssl_ctx_callback(CURL *curl, void *sslctx, void *parm) {
  // Set defaults from config.hdf
  CURLcode r = curl_tls_workarounds_cb(curl, sslctx, parm);
  if (r != CURLE_OK) {
    return r;
  }

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

class CurlMultiResource : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(CurlMultiResource)

  CLASSNAME_IS("curl_multi")
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  CurlMultiResource() {
    m_multi = curl_multi_init();
  }

  ~CurlMultiResource() {
    close();
  }

  void close() {
    if (m_multi) {
      curl_multi_cleanup(m_multi);
      m_easyh.clear();
      m_multi = nullptr;
    }
  }

  bool setOption(int option, const Variant& value) {
#if LIBCURL_VERSION_NUM <= 0x070f04 /* 7.15.4 */
    return false;
#endif
    if (m_multi == nullptr) {
      return false;
    }

    CURLMcode error = CURLM_OK;
    switch (option) {
#if LIBCURL_VERSION_NUM >= 0x071000 /* 7.16.0 */
      case CURLMOPT_PIPELINING:

#if LIBCURL_VERSION_NUM >= 0x071003 /* 7.16.3 */
      case CURLMOPT_MAXCONNECTS:
#endif
        error = curl_multi_setopt(m_multi,
                                  (CURLMoption)option,
                                  value.toInt64());
        break;
#endif
      default:
        raise_warning("curl_multi_setopt():"
                      "Invalid curl multi configuration option");
        error = CURLM_UNKNOWN_OPTION;
        break;
    }

    if (error != CURLM_OK) {
        return false;
    } else {
        return true;
    }
  }

  bool isInvalid() const override {
    return !m_multi;
  }

  void add(const Resource& ch) {
    m_easyh.append(ch);
  }

  void remove(req::ptr<CurlResource> curle) {
    for (ArrayIter iter(m_easyh); iter; ++iter) {
      if (cast<CurlResource>(iter.second())->get(true) ==
          curle->get()) {
        m_easyh.remove(iter.first());
        return;
      }
    }
  }

  Resource find(CURL *cp) {
    for (ArrayIter iter(m_easyh); iter; ++iter) {
      if (cast<CurlResource>(iter.second())->get(true) == cp) {
        return iter.second().toResource();
      }
    }
    return Resource();
  }

  void check_exceptions() {
    Exception* cppException = nullptr;
    Object phpException;
    for (ArrayIter iter(m_easyh); iter; ++iter) {
      auto curl = cast<CurlResource>(iter.second());
      ExceptionType nextException(curl->getAndClearException());
      if (!nextException) continue;
      if (isPhpException(nextException)) {
        Object e(getPhpException(nextException));
        e->o_set(s_previous, phpException, s_exception);
        phpException = std::move(e);
      } else {
        auto e = getCppException(nextException);
        if (auto f = dynamic_cast<FatalErrorException*>(e)) {
          if (!f->isRecoverable()) f->throwException();
        }
        delete cppException;
        cppException = e;
      }
    }
    if (cppException) cppException->throwException();
    if (!phpException.isNull()) throw phpException;
  }

  CURLM *get() {
    if (m_multi == nullptr) {
      throw_null_pointer_exception();
    }
    return m_multi;
  }

  const Array& getEasyHandles() const {
    return m_easyh;
  }

private:
  CURLM *m_multi;
  Array m_easyh;
};

void CurlMultiResource::sweep() {
  if (m_multi) {
    curl_multi_cleanup(m_multi);
  }
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

class CurlMultiAwait;

class CurlEventHandler : public AsioEventHandler {
 public:
  CurlEventHandler(AsioEventBase* base, int fd, CurlMultiAwait* cma):
    AsioEventHandler(base, fd), m_curlMultiAwait(cma), m_fd(fd) {}

  void handlerReady(uint16_t events) noexcept override;
 private:
  CurlMultiAwait* m_curlMultiAwait;
  int m_fd;
};

class CurlTimeoutHandler : public AsioTimeoutHandler {
 public:
  CurlTimeoutHandler(AsioEventBase* base, CurlMultiAwait* cma):
    AsioTimeoutHandler(base), m_curlMultiAwait(cma) {}

  void timeoutExpired() noexcept override;
 private:
  CurlMultiAwait* m_curlMultiAwait;
};

class CurlMultiAwait : public AsioExternalThreadEvent {
 public:
  CurlMultiAwait(req::ptr<CurlMultiResource> multi, double timeout) {
    if ((addLowHandles(multi) + addHighHandles(multi)) == 0) {
      // Nothing to do
      markAsFinished();
      return;
    }

    // Add optional timeout
    int64_t timeout_ms = timeout * 1000;
    if (timeout_ms > 0) {
      auto asio_event_base = getSingleton<AsioEventBase>();
      m_timeout = std::make_shared<CurlTimeoutHandler>(asio_event_base.get(),
                                                       this);

      asio_event_base->runInEventBaseThreadAndWait([this,timeout_ms] {
        m_timeout->scheduleTimeout(timeout_ms);
      });
    }
  }

  ~CurlMultiAwait() {
    for (auto handler : m_handlers) {
      handler->unregisterHandler();
    }
    if (m_timeout) {
      auto asio_event_base = getSingleton<AsioEventBase>();
      auto to = std::move(m_timeout);
      asio_event_base->runInEventBaseThreadAndWait([to] {
        to.get()->cancelTimeout();
      });
    }
    m_handlers.clear();
  }

  void unserialize(Cell& c) {
    c.m_type = KindOfInt64;
    c.m_data.num = m_result;
  }

  void setFinished(int fd) {
    if (m_result < fd) {
      m_result = fd;
    }
    if (!m_finished) {
      markAsFinished();
      m_finished = true;
    }
  }

 private:
  void addHandle(int fd, int events) {
    auto asio_event_base = getSingleton<AsioEventBase>();
    auto handler =
      std::make_shared<CurlEventHandler>(asio_event_base.get(), fd, this);
    handler->registerHandler(events);
    m_handlers.push_back(handler);
  }

  // Ask curl_multi for its handles directly
  // This is preferable as we get to know which
  // are blocking on reads, and which on writes.
  int addLowHandles(req::ptr<CurlMultiResource> multi) {
    fd_set read_fds, write_fds;
    int max_fd = -1, count = 0;
    FD_ZERO(&read_fds); FD_ZERO(&write_fds);
    if ((CURLM_OK != curl_multi_fdset(multi->get(), &read_fds, &write_fds,
                                      nullptr, &max_fd)) ||
        (max_fd < 0)) {
      return count;
    }
    for (int i = 0 ; i <= max_fd; ++i) {
      int events = 0;
      if (FD_ISSET(i, &read_fds))  events |= AsioEventHandler::READ;
      if (FD_ISSET(i, &write_fds)) events |= AsioEventHandler::WRITE;
      if (events) {
        addHandle(i, events);
        ++count;
      }
    }
    return count;
  }

  // Check for file descriptors >= FD_SETSIZE
  // which can't be returned in an fdset
  // This is a little hacky, but necessary given cURL's APIs
  int addHighHandles(req::ptr<CurlMultiResource> multi) {
    int count = 0;
    auto easy_handles = multi->getEasyHandles();
    for (ArrayIter iter(easy_handles); iter; ++iter) {
      Variant easy_handle = iter.second();
      auto easy = dyn_cast_or_null<CurlResource>(easy_handle);
      if (!easy) continue;
      long sock;
      if ((curl_easy_getinfo(easy->get(),
                             CURLINFO_LASTSOCKET, &sock) != CURLE_OK) ||
          (sock < FD_SETSIZE)) {
        continue;
      }
      // No idea which type of event it needs, ask for everything
      addHandle(sock, AsioEventHandler::READ_WRITE);
      ++count;
    }
    return count;
  }

  std::shared_ptr<CurlTimeoutHandler> m_timeout;
  std::vector<std::shared_ptr<CurlEventHandler>> m_handlers;
  int m_result{-1};
  bool m_finished{false};
};

void CurlEventHandler::handlerReady(uint16_t events) noexcept {
  m_curlMultiAwait->setFinished(m_fd);
}

void CurlTimeoutHandler::timeoutExpired() noexcept {
  m_curlMultiAwait->setFinished(-1);
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

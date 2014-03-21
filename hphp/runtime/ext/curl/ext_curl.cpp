/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/libevent-http-client.h"
#include "hphp/runtime/base/curl-tls-workarounds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include <openssl/ssl.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/multi.h>
#include <memory>
#include <vector>

#define CURLOPT_RETURNTRANSFER 19913
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

const StaticString
  s_exception("exception"),
  s_previous("previous");

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
    SmartResource<File> fp;
    StringBuffer       buf;
    String             content;
    int                type;
  };

  class ReadHandler {
  public:
    ReadHandler() : method(0) {}

    int                method;
    Variant            callback;
    SmartResource<File> fp;
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
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  explicit CurlResource(const String& url)
    : m_exception(nullptr), m_phpException(false), m_emptyPost(true) {
    m_cp = curl_easy_init();
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

  explicit CurlResource(CurlResource *src)
    : m_exception(nullptr), m_phpException(false) {
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

    reset();

    m_to_free = src->m_to_free;
    m_emptyPost = src->m_emptyPost;
  }

  ~CurlResource() {
    close();
  }

  virtual bool isInvalid() const {
    return !m_cp;
  }

  void closeForSweep() {
    assert(!m_exception);
    if (m_cp) {
      curl_easy_cleanup(m_cp);
      m_cp = nullptr;
    }
    m_to_free.reset();
  }

  void close() {
    closeForSweep();
    m_opts.clear();
  }

  void check_exception() {
    if (m_exception) {
      if (m_phpException) {
        Object e((ObjectData*)m_exception);
        m_exception = nullptr;
        e.get()->decRefCount();
        throw e;
      } else {
        Exception *e = (Exception*)m_exception;
        m_exception = nullptr;
        e->throwException();
      }
    }
  }

  ObjectData* getAndClearPhpException() {
    if (m_exception && m_phpException) {
      ObjectData* ret = (ObjectData*)m_exception;
      m_exception = nullptr;
      return ret;
    }
    return nullptr;
  }

  Exception* getAndClearCppException() {
    if (!m_phpException) {
      Exception* e = (Exception*)m_exception;
      m_exception = nullptr;
      return e;
    }
    return nullptr;
  }

  void reset() {
    curl_easy_reset(m_cp);

    curl_easy_setopt(m_cp, CURLOPT_NOPROGRESS,        1);
    curl_easy_setopt(m_cp, CURLOPT_VERBOSE,           0);
    curl_easy_setopt(m_cp, CURLOPT_ERRORBUFFER,       m_error_str);
    curl_easy_setopt(m_cp, CURLOPT_WRITEFUNCTION,     curl_write);
    curl_easy_setopt(m_cp, CURLOPT_FILE,              (void*)this);
    curl_easy_setopt(m_cp, CURLOPT_READFUNCTION,      curl_read);
    curl_easy_setopt(m_cp, CURLOPT_INFILE,            (void*)this);
    curl_easy_setopt(m_cp, CURLOPT_HEADERFUNCTION,    curl_write_header);
    curl_easy_setopt(m_cp, CURLOPT_WRITEHEADER,       (void*)this);
    curl_easy_setopt(m_cp, CURLOPT_DNS_USE_GLOBAL_CACHE, 0); // for thread-safe
    curl_easy_setopt(m_cp, CURLOPT_DNS_CACHE_TIMEOUT, 120);
    curl_easy_setopt(m_cp, CURLOPT_MAXREDIRS, 20); // no infinite redirects
    curl_easy_setopt(m_cp, CURLOPT_NOSIGNAL, 1); // for multithreading mode
    curl_easy_setopt(m_cp, CURLOPT_SSL_CTX_FUNCTION,
                     CurlResource::ssl_ctx_callback);
    curl_easy_setopt(m_cp, CURLOPT_SSL_CTX_DATA, (void*)this);

    curl_easy_setopt(m_cp, CURLOPT_TIMEOUT,
                     RuntimeOption::HttpDefaultTimeout);
    curl_easy_setopt(m_cp, CURLOPT_CONNECTTIMEOUT,
                     RuntimeOption::HttpDefaultTimeout);
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
      return String("");
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
    case CURLOPT_FTPLISTONLY:
    case CURLOPT_FTPAPPEND:
    case CURLOPT_NETRC:
    case CURLOPT_PUT:
    case CURLOPT_TIMEOUT:
#if LIBCURL_VERSION_NUM >= 0x071002
    case CURLOPT_TIMEOUT_MS:
#endif
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
        if (!value.isResource()) {
          return false;
        }

        Resource obj = value.toResource();
        if (obj.isNull() || obj.getTyped<File>(true) == nullptr) {
          return false;
        }

        switch (option) {
          case CURLOPT_FILE:
            m_write.fp = obj;
            m_write.method = PHP_CURL_FILE;
            break;
          case CURLOPT_WRITEHEADER:
            m_write_header.fp = obj;
            m_write_header.method = PHP_CURL_FILE;
            break;
          case CURLOPT_INFILE:
            m_read.fp = obj;
            m_emptyPost = false;
            break;
          default: {
            if (obj.getTyped<PlainFile>(true) == nullptr) {
              return false;
            }
            FILE *fp = obj.getTyped<PlainFile>()->getStream();
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
      if (value.is(KindOfArray) || value.is(KindOfObject)) {
        Array arr = value.toArray();
        curl_httppost *first = nullptr;
        curl_httppost *last  = nullptr;
        for (ArrayIter iter(arr); iter; ++iter) {
          String key = iter.first().toString();
          String val = iter.second().toString();
          const char *postval = val.data();

          if (*postval == '@') {
            /* Given a string like:
             *   "@/foo/bar;type=herp/derp;filename=ponies\0"
             * - Temporarily convert to:
             *   "@/foo/bar\0type=herp/derp\0filename=ponies\0"
             * - Pass pointers to the relevant null-terminated substrings to
             *   curl_formadd
             * - Revert changes to postval at the end
             */
            char* mutablePostval = const_cast<char*>(postval);
            char* type = strstr(mutablePostval, ";type=");
            char* filename = strstr(mutablePostval, ";filename=");

            if (type) {
              *type = '\0';
            }
            if (filename) {
              *filename = '\0';
            }

            /* The arguments after _NAMELENGTH and _CONTENTSLENGTH
             * must be explicitly cast to long in curl_formadd
             * use since curl needs a long not an int. */
            ++postval;
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
               CURLFORM_FILE, postval,
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
      if (value.is(KindOfArray) || value.is(KindOfObject)) {
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
                      "CURLOPT_HTTP200ALIASES and CURLOPT_POSTQUOTE "
                      "arguments");
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
    } catch (Object &e) {
      ObjectData *od = e.get();
      od->incRefCount();
      m_exception = od;
      m_phpException = true;
    } catch (Exception &e) {
      m_exception = e.clone();
      m_phpException = false;
    }
    return uninit_null();
  }

  static int curl_progress(void* p,
                           double dltotal, double dlnow,
                           double ultotal, double ulnow) {
    assert(p);
    CurlResource* curl = static_cast<CurlResource*>(p);

    ArrayInit ai(5);
    ai.set(Resource(curl));
    ai.set(dltotal);
    ai.set(dlnow);
    ai.set(ultotal);
    ai.set(ulnow);

    Variant result = vm_call_user_func(
      curl->m_progress_callback,
      ai.toArray()
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
      if (!t->fp.isNull()) {
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
          t->callback, make_packed_array(Resource(ch), t->fp, data_size));
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
      throw NullPointerException();
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
  void *m_exception;

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

  bool m_phpException;
  bool m_emptyPost;

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
  CurlResource *curl = ch.getTyped<CurlResource>(true, true);               \
  if (curl == nullptr) {                                                    \
    raise_warning("supplied argument is not a valid cURL handle resource"); \
    return false;                                                           \
  }                                                                         \

#define CHECK_RESOURCE_RETURN_VOID(curl)                                    \
  CurlResource *curl = ch.getTyped<CurlResource>(true, true);               \
  if (curl == nullptr) {                                                    \
    raise_warning("supplied argument is not a valid cURL handle resource"); \
    return;                                                                 \
  }                                                                         \

Variant HHVM_FUNCTION(curl_init, const Variant& url /* = null_string */) {
  if (url.isNull()) {
    return NEWOBJ(CurlResource)(null_string);
  } else {
    return NEWOBJ(CurlResource)(url.toString());
  }
}

Variant HHVM_FUNCTION(curl_copy_handle, const Resource& ch) {
  CHECK_RESOURCE(curl);
  return NEWOBJ(CurlResource)(curl);
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

Variant HHVM_FUNCTION(curl_version, int uversion /* = k_CURLVERSION_NOW */) {
  curl_version_info_data *d = curl_version_info((CURLversion)uversion);
  if (d == nullptr) {
    return false;
  }

  ArrayInit ret(9);
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
  return ret.create();
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
        ret.set(s_content_type, uninit_null());
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

  return uninit_null();
}

Variant HHVM_FUNCTION(curl_errno, const Resource& ch) {
  CHECK_RESOURCE(curl);
  return curl->getError();
}

Variant HHVM_FUNCTION(curl_error, const Resource& ch) {
  CHECK_RESOURCE(curl);
  return curl->getErrorString();
}

Variant HHVM_FUNCTION(curl_close, const Resource& ch) {
  CHECK_RESOURCE(curl);
  curl->close();
  return uninit_null();
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
  const String& o_getClassNameHook() const { return classnameof(); }

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

  void add(const Resource& ch) {
    m_easyh.append(ch);
  }

  void remove(CurlResource *curle) {
    for (ArrayIter iter(m_easyh); iter; ++iter) {
      if (iter.second().toResource().getTyped<CurlResource>()->get(true) ==
          curle->get()) {
        m_easyh.remove(iter.first());
        return;
      }
    }
  }

  Resource find(CURL *cp) {
    for (ArrayIter iter(m_easyh); iter; ++iter) {
      if (iter.second().toResource().
            getTyped<CurlResource>()->get(true) == cp) {
        return iter.second().toResource();
      }
    }
    return Resource();
  }

  void check_exceptions() {
    ObjectData* phpException = 0;
    Exception* cppException = 0;
    for (ArrayIter iter(m_easyh); iter; ++iter) {
      CurlResource* curl = iter.second().toResource().getTyped<CurlResource>();
      if (ObjectData* e = curl->getAndClearPhpException()) {
        if (phpException) {
          e->o_set(s_previous, Variant(phpException), s_exception);
          phpException->decRefCount();
        }
        phpException = e;
      } else if (Exception *e = curl->getAndClearCppException()) {
        delete cppException;
        cppException = e;
      }
    }
    if (cppException) {
      if (phpException) decRefObj(phpException);
      cppException->throwException();
    }
    if (phpException) {
      Object e(phpException);
      phpException->decRefCount();
      throw e;
    }
  }

  CURLM *get() {
    if (m_multi == nullptr) {
      throw NullPointerException();
    }
    return m_multi;
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

#define CHECK_MULTI_RESOURCE(curlm)                                      \
  CurlMultiResource *curlm = mh.getTyped<CurlMultiResource>(true, true); \
  if (curlm == nullptr) {                                                \
    raise_warning("expects parameter 1 to be cURL multi resource");      \
    return uninit_null();                                                \
  }                                                                      \

Resource HHVM_FUNCTION(curl_multi_init) {
  return NEWOBJ(CurlMultiResource)();
}

Variant HHVM_FUNCTION(curl_multi_add_handle, const Resource& mh, const Resource& ch) {
  CHECK_MULTI_RESOURCE(curlm);
  CurlResource *curle = ch.getTyped<CurlResource>();
  curlm->add(ch);
  return curl_multi_add_handle(curlm->get(), curle->get());
}

Variant HHVM_FUNCTION(curl_multi_remove_handle, const Resource& mh, const Resource& ch) {
  CHECK_MULTI_RESOURCE(curlm);
  CurlResource *curle = ch.getTyped<CurlResource>();
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
  still_running = running;
  return result;
}

/* Fallback implementation of curl_multi_select() for
 * libcurl < 7.28.0 without FB's curl_multi_select() patch
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
#define curl_multi_select_func curl_multi_select
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

Variant HHVM_FUNCTION(curl_multi_getcontent, const Resource& ch) {
  CHECK_RESOURCE(curl);
  return curl->getContents();
}

Array curl_convert_fd_to_stream(fd_set *fd, int max_fd) {
  Array ret = Array::Create();
  for (int i=0; i<=max_fd; i++) {
    if (FD_ISSET(i, fd)) {
      BuiltinFile *file = NEWOBJ(BuiltinFile)(i);
      ret.append(file);
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
                               VRefParam msgs_in_queue /* = null */) {
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
  return uninit_null();
}

///////////////////////////////////////////////////////////////////////////////

#if LIBCURL_VERSION_NUM >= 0x071500
const int64_t k_CURLINFO_LOCAL_PORT = CURLINFO_LOCAL_PORT;
#endif

#if LIBCURL_VERSION_NUM >= 0x071002
const int64_t k_CURLOPT_TIMEOUT_MS = CURLOPT_TIMEOUT_MS;
const int64_t k_CURLOPT_CONNECTTIMEOUT_MS = CURLOPT_CONNECTTIMEOUT_MS;
#endif

const int64_t k_CURLAUTH_ANY = CURLAUTH_ANY;
const int64_t k_CURLAUTH_ANYSAFE = CURLAUTH_ANYSAFE;
const int64_t k_CURLAUTH_BASIC = CURLAUTH_BASIC;
const int64_t k_CURLAUTH_DIGEST = CURLAUTH_DIGEST;
const int64_t k_CURLAUTH_GSSNEGOTIATE = CURLAUTH_GSSNEGOTIATE;
const int64_t k_CURLAUTH_NTLM = CURLAUTH_NTLM;
const int64_t k_CURLCLOSEPOLICY_CALLBACK = CURLCLOSEPOLICY_CALLBACK;
const int64_t k_CURLCLOSEPOLICY_LEAST_RECENTLY_USED =
  CURLCLOSEPOLICY_LEAST_RECENTLY_USED;
const int64_t k_CURLCLOSEPOLICY_LEAST_TRAFFIC = CURLCLOSEPOLICY_LEAST_TRAFFIC;
const int64_t k_CURLCLOSEPOLICY_OLDEST = CURLCLOSEPOLICY_OLDEST;
const int64_t k_CURLCLOSEPOLICY_SLOWEST = CURLCLOSEPOLICY_SLOWEST;
const int64_t k_CURLE_ABORTED_BY_CALLBACK = CURLE_ABORTED_BY_CALLBACK;
const int64_t k_CURLE_BAD_CALLING_ORDER = CURLE_BAD_CALLING_ORDER;
const int64_t k_CURLE_BAD_CONTENT_ENCODING = CURLE_BAD_CONTENT_ENCODING;
const int64_t k_CURLE_BAD_FUNCTION_ARGUMENT = CURLE_BAD_FUNCTION_ARGUMENT;
const int64_t k_CURLE_BAD_PASSWORD_ENTERED = CURLE_BAD_PASSWORD_ENTERED;
const int64_t k_CURLE_COULDNT_CONNECT = CURLE_COULDNT_CONNECT;
const int64_t k_CURLE_COULDNT_RESOLVE_HOST = CURLE_COULDNT_RESOLVE_HOST;
const int64_t k_CURLE_COULDNT_RESOLVE_PROXY = CURLE_COULDNT_RESOLVE_PROXY;
const int64_t k_CURLE_FAILED_INIT = CURLE_FAILED_INIT;
const int64_t k_CURLE_FILESIZE_EXCEEDED = CURLE_FILESIZE_EXCEEDED;
const int64_t k_CURLE_FILE_COULDNT_READ_FILE = CURLE_FILE_COULDNT_READ_FILE;
const int64_t k_CURLE_FTP_ACCESS_DENIED = CURLE_FTP_ACCESS_DENIED;
const int64_t k_CURLE_FTP_BAD_DOWNLOAD_RESUME = CURLE_FTP_BAD_DOWNLOAD_RESUME;
const int64_t k_CURLE_FTP_CANT_GET_HOST = CURLE_FTP_CANT_GET_HOST;
const int64_t k_CURLE_FTP_CANT_RECONNECT = CURLE_FTP_CANT_RECONNECT;
const int64_t k_CURLE_FTP_COULDNT_GET_SIZE = CURLE_FTP_COULDNT_GET_SIZE;
const int64_t k_CURLE_FTP_COULDNT_RETR_FILE = CURLE_FTP_COULDNT_RETR_FILE;
const int64_t k_CURLE_FTP_COULDNT_SET_ASCII = CURLE_FTP_COULDNT_SET_ASCII;
const int64_t k_CURLE_FTP_COULDNT_SET_BINARY = CURLE_FTP_COULDNT_SET_BINARY;
const int64_t k_CURLE_FTP_COULDNT_STOR_FILE = CURLE_FTP_COULDNT_STOR_FILE;
const int64_t k_CURLE_FTP_COULDNT_USE_REST = CURLE_FTP_COULDNT_USE_REST;
const int64_t k_CURLE_FTP_PORT_FAILED = CURLE_FTP_PORT_FAILED;
const int64_t k_CURLE_FTP_QUOTE_ERROR = CURLE_FTP_QUOTE_ERROR;
const int64_t k_CURLE_FTP_SSL_FAILED = CURLE_FTP_SSL_FAILED;
const int64_t k_CURLE_FTP_USER_PASSWORD_INCORRECT =
  CURLE_FTP_USER_PASSWORD_INCORRECT;
const int64_t k_CURLE_FTP_WEIRD_227_FORMAT = CURLE_FTP_WEIRD_227_FORMAT;
const int64_t k_CURLE_FTP_WEIRD_PASS_REPLY = CURLE_FTP_WEIRD_PASS_REPLY;
const int64_t k_CURLE_FTP_WEIRD_PASV_REPLY = CURLE_FTP_WEIRD_PASV_REPLY;
const int64_t k_CURLE_FTP_WEIRD_SERVER_REPLY = CURLE_FTP_WEIRD_SERVER_REPLY;
const int64_t k_CURLE_FTP_WEIRD_USER_REPLY = CURLE_FTP_WEIRD_USER_REPLY;
const int64_t k_CURLE_FTP_WRITE_ERROR = CURLE_FTP_WRITE_ERROR;
const int64_t k_CURLE_FUNCTION_NOT_FOUND = CURLE_FUNCTION_NOT_FOUND;
const int64_t k_CURLE_GOT_NOTHING = CURLE_GOT_NOTHING;
const int64_t k_CURLE_HTTP_NOT_FOUND = CURLE_HTTP_NOT_FOUND;
const int64_t k_CURLE_HTTP_PORT_FAILED = CURLE_HTTP_PORT_FAILED;
const int64_t k_CURLE_HTTP_POST_ERROR = CURLE_HTTP_POST_ERROR;
const int64_t k_CURLE_HTTP_RANGE_ERROR = CURLE_HTTP_RANGE_ERROR;
const int64_t k_CURLE_LDAP_CANNOT_BIND = CURLE_LDAP_CANNOT_BIND;
const int64_t k_CURLE_LDAP_INVALID_URL = CURLE_LDAP_INVALID_URL;
const int64_t k_CURLE_LDAP_SEARCH_FAILED = CURLE_LDAP_SEARCH_FAILED;
const int64_t k_CURLE_LIBRARY_NOT_FOUND = CURLE_LIBRARY_NOT_FOUND;
const int64_t k_CURLE_MALFORMAT_USER = CURLE_MALFORMAT_USER;
const int64_t k_CURLE_OBSOLETE = CURLE_OBSOLETE;
const int64_t k_CURLE_OK = CURLE_OK;
const int64_t k_CURLE_OPERATION_TIMEOUTED = CURLE_OPERATION_TIMEOUTED;
const int64_t k_CURLE_OUT_OF_MEMORY = CURLE_OUT_OF_MEMORY;
const int64_t k_CURLE_PARTIAL_FILE = CURLE_PARTIAL_FILE;
const int64_t k_CURLE_READ_ERROR = CURLE_READ_ERROR;
const int64_t k_CURLE_RECV_ERROR = CURLE_RECV_ERROR;
const int64_t k_CURLE_SEND_ERROR = CURLE_SEND_ERROR;
const int64_t k_CURLE_SHARE_IN_USE = CURLE_SHARE_IN_USE;
const int64_t k_CURLE_SSL_CACERT = CURLE_SSL_CACERT;
const int64_t k_CURLE_SSL_CERTPROBLEM = CURLE_SSL_CERTPROBLEM;
const int64_t k_CURLE_SSL_CIPHER = CURLE_SSL_CIPHER;
const int64_t k_CURLE_SSL_CONNECT_ERROR = CURLE_SSL_CONNECT_ERROR;
const int64_t k_CURLE_SSL_ENGINE_NOTFOUND = CURLE_SSL_ENGINE_NOTFOUND;
const int64_t k_CURLE_SSL_ENGINE_SETFAILED = CURLE_SSL_ENGINE_SETFAILED;
const int64_t k_CURLE_SSL_PEER_CERTIFICATE = CURLE_SSL_PEER_CERTIFICATE;
const int64_t k_CURLE_TELNET_OPTION_SYNTAX = CURLE_TELNET_OPTION_SYNTAX;
const int64_t k_CURLE_TOO_MANY_REDIRECTS = CURLE_TOO_MANY_REDIRECTS;
const int64_t k_CURLE_UNKNOWN_TELNET_OPTION = CURLE_UNKNOWN_TELNET_OPTION;
const int64_t k_CURLE_UNSUPPORTED_PROTOCOL = CURLE_UNSUPPORTED_PROTOCOL;
const int64_t k_CURLE_URL_MALFORMAT = CURLE_URL_MALFORMAT;
const int64_t k_CURLE_URL_MALFORMAT_USER = CURLE_URL_MALFORMAT_USER;
const int64_t k_CURLE_WRITE_ERROR = CURLE_WRITE_ERROR;
const int64_t k_CURLFTPAUTH_DEFAULT = CURLFTPAUTH_DEFAULT;
const int64_t k_CURLFTPAUTH_SSL = CURLFTPAUTH_SSL;
const int64_t k_CURLFTPAUTH_TLS = CURLFTPAUTH_TLS;
const int64_t k_CURLFTPSSL_ALL = CURLFTPSSL_ALL;
const int64_t k_CURLFTPSSL_CONTROL = CURLFTPSSL_CONTROL;
const int64_t k_CURLFTPSSL_NONE = CURLFTPSSL_NONE;
const int64_t k_CURLFTPSSL_TRY = CURLFTPSSL_TRY;
const int64_t k_CURLINFO_CONNECT_TIME = CURLINFO_CONNECT_TIME;
const int64_t k_CURLINFO_CONTENT_LENGTH_DOWNLOAD =
  CURLINFO_CONTENT_LENGTH_DOWNLOAD;
const int64_t k_CURLINFO_CONTENT_LENGTH_UPLOAD = CURLINFO_CONTENT_LENGTH_UPLOAD;
const int64_t k_CURLINFO_CONTENT_TYPE = CURLINFO_CONTENT_TYPE;
const int64_t k_CURLINFO_EFFECTIVE_URL = CURLINFO_EFFECTIVE_URL;
const int64_t k_CURLINFO_FILETIME = CURLINFO_FILETIME;
const int64_t k_CURLINFO_HEADER_OUT = CURLINFO_HEADER_OUT;
const int64_t k_CURLINFO_HEADER_SIZE = CURLINFO_HEADER_SIZE;
const int64_t k_CURLINFO_HTTP_CODE = CURLINFO_HTTP_CODE;
const int64_t k_CURLINFO_NAMELOOKUP_TIME = CURLINFO_NAMELOOKUP_TIME;
const int64_t k_CURLINFO_PRETRANSFER_TIME = CURLINFO_PRETRANSFER_TIME;
const int64_t k_CURLINFO_PRIVATE = CURLINFO_PRIVATE;
const int64_t k_CURLINFO_REDIRECT_COUNT = CURLINFO_REDIRECT_COUNT;
const int64_t k_CURLINFO_REDIRECT_TIME = CURLINFO_REDIRECT_TIME;
const int64_t k_CURLINFO_REQUEST_SIZE = CURLINFO_REQUEST_SIZE;
const int64_t k_CURLINFO_SIZE_DOWNLOAD = CURLINFO_SIZE_DOWNLOAD;
const int64_t k_CURLINFO_SIZE_UPLOAD = CURLINFO_SIZE_UPLOAD;
const int64_t k_CURLINFO_SPEED_DOWNLOAD = CURLINFO_SPEED_DOWNLOAD;
const int64_t k_CURLINFO_SPEED_UPLOAD = CURLINFO_SPEED_UPLOAD;
const int64_t k_CURLINFO_SSL_VERIFYRESULT = CURLINFO_SSL_VERIFYRESULT;
const int64_t k_CURLINFO_STARTTRANSFER_TIME = CURLINFO_STARTTRANSFER_TIME;
const int64_t k_CURLINFO_TOTAL_TIME = CURLINFO_TOTAL_TIME;
const int64_t k_CURLMSG_DONE = CURLMSG_DONE;
const int64_t k_CURLM_BAD_EASY_HANDLE = CURLM_BAD_EASY_HANDLE;
const int64_t k_CURLM_BAD_HANDLE = CURLM_BAD_HANDLE;
const int64_t k_CURLM_CALL_MULTI_PERFORM = CURLM_CALL_MULTI_PERFORM;
const int64_t k_CURLM_INTERNAL_ERROR = CURLM_INTERNAL_ERROR;
const int64_t k_CURLM_OK = CURLM_OK;
const int64_t k_CURLM_OUT_OF_MEMORY = CURLM_OUT_OF_MEMORY;
const int64_t k_CURLOPT_AUTOREFERER = CURLOPT_AUTOREFERER;
const int64_t k_CURLOPT_BINARYTRANSFER = CURLOPT_BINARYTRANSFER;
const int64_t k_CURLOPT_BUFFERSIZE = CURLOPT_BUFFERSIZE;
const int64_t k_CURLOPT_CAINFO = CURLOPT_CAINFO;
const int64_t k_CURLOPT_CAPATH = CURLOPT_CAPATH;
const int64_t k_CURLOPT_CLOSEPOLICY = CURLOPT_CLOSEPOLICY;
const int64_t k_CURLOPT_CONNECTTIMEOUT = CURLOPT_CONNECTTIMEOUT;
const int64_t k_CURLOPT_COOKIE = CURLOPT_COOKIE;
const int64_t k_CURLOPT_COOKIEFILE = CURLOPT_COOKIEFILE;
const int64_t k_CURLOPT_COOKIEJAR = CURLOPT_COOKIEJAR;
const int64_t k_CURLOPT_COOKIESESSION = CURLOPT_COOKIESESSION;
const int64_t k_CURLOPT_CRLF = CURLOPT_CRLF;
const int64_t k_CURLOPT_CUSTOMREQUEST = CURLOPT_CUSTOMREQUEST;
const int64_t k_CURLOPT_DNS_CACHE_TIMEOUT = CURLOPT_DNS_CACHE_TIMEOUT;
const int64_t k_CURLOPT_DNS_USE_GLOBAL_CACHE = CURLOPT_DNS_USE_GLOBAL_CACHE;
const int64_t k_CURLOPT_EGDSOCKET = CURLOPT_EGDSOCKET;
const int64_t k_CURLOPT_ENCODING = CURLOPT_ENCODING;
const int64_t k_CURLOPT_FAILONERROR = CURLOPT_FAILONERROR;
const int64_t k_CURLOPT_FILE = CURLOPT_FILE;
const int64_t k_CURLOPT_FILETIME = CURLOPT_FILETIME;
const int64_t k_CURLOPT_FOLLOWLOCATION = CURLOPT_FOLLOWLOCATION;
const int64_t k_CURLOPT_FORBID_REUSE = CURLOPT_FORBID_REUSE;
const int64_t k_CURLOPT_FRESH_CONNECT = CURLOPT_FRESH_CONNECT;
const int64_t k_CURLOPT_FTPAPPEND = CURLOPT_FTPAPPEND;
const int64_t k_CURLOPT_FTPLISTONLY = CURLOPT_FTPLISTONLY;
const int64_t k_CURLOPT_FTPPORT = CURLOPT_FTPPORT;
const int64_t k_CURLOPT_FTPSSLAUTH = CURLOPT_FTPSSLAUTH;
const int64_t k_CURLOPT_FTP_CREATE_MISSING_DIRS =
  CURLOPT_FTP_CREATE_MISSING_DIRS;
const int64_t k_CURLOPT_FTP_SSL = CURLOPT_FTP_SSL;
const int64_t k_CURLOPT_FTP_USE_EPRT = CURLOPT_FTP_USE_EPRT;
const int64_t k_CURLOPT_FTP_USE_EPSV = CURLOPT_FTP_USE_EPSV;
const int64_t k_CURLOPT_HEADER = CURLOPT_HEADER;
const int64_t k_CURLOPT_HEADERFUNCTION = CURLOPT_HEADERFUNCTION;
const int64_t k_CURLOPT_HTTP200ALIASES = CURLOPT_HTTP200ALIASES;
const int64_t k_CURLOPT_HTTPAUTH = CURLOPT_HTTPAUTH;
const int64_t k_CURLOPT_HTTPGET = CURLOPT_HTTPGET;
const int64_t k_CURLOPT_HTTPHEADER = CURLOPT_HTTPHEADER;
const int64_t k_CURLOPT_HTTPPROXYTUNNEL = CURLOPT_HTTPPROXYTUNNEL;
const int64_t k_CURLOPT_HTTP_VERSION = CURLOPT_HTTP_VERSION;
const int64_t k_CURLOPT_INFILE = CURLOPT_INFILE;
const int64_t k_CURLOPT_INFILESIZE = CURLOPT_INFILESIZE;
const int64_t k_CURLOPT_INTERFACE = CURLOPT_INTERFACE;
const int64_t k_CURLOPT_IPRESOLVE = CURLOPT_IPRESOLVE;
const int64_t k_CURLOPT_KRB4LEVEL = CURLOPT_KRB4LEVEL;
const int64_t k_CURLOPT_LOW_SPEED_LIMIT = CURLOPT_LOW_SPEED_LIMIT;
const int64_t k_CURLOPT_LOW_SPEED_TIME = CURLOPT_LOW_SPEED_TIME;
const int64_t k_CURLOPT_MAXCONNECTS = CURLOPT_MAXCONNECTS;
const int64_t k_CURLOPT_MAXREDIRS = CURLOPT_MAXREDIRS;
const int64_t k_CURLOPT_MUTE = CURLOPT_MUTE;
const int64_t k_CURLOPT_NETRC = CURLOPT_NETRC;
const int64_t k_CURLOPT_NOBODY = CURLOPT_NOBODY;
const int64_t k_CURLOPT_NOPROGRESS = CURLOPT_NOPROGRESS;
const int64_t k_CURLOPT_NOSIGNAL = CURLOPT_NOSIGNAL;
const int64_t k_CURLOPT_PASSWDFUNCTION = CURLOPT_PASSWDFUNCTION;
const int64_t k_CURLOPT_PORT = CURLOPT_PORT;
const int64_t k_CURLOPT_POST = CURLOPT_POST;
const int64_t k_CURLOPT_POSTFIELDS = CURLOPT_POSTFIELDS;
const int64_t k_CURLOPT_POSTREDIR = CURLOPT_POSTREDIR;
const int64_t k_CURLOPT_POSTQUOTE = CURLOPT_POSTQUOTE;
const int64_t k_CURLOPT_PRIVATE = CURLOPT_PRIVATE;
const int64_t k_CURLOPT_PROGRESSDATA = CURLOPT_PROGRESSDATA;
const int64_t k_CURLOPT_PROGRESSFUNCTION = CURLOPT_PROGRESSFUNCTION;
const int64_t k_CURLOPT_PROXY = CURLOPT_PROXY;
const int64_t k_CURLOPT_PROXYAUTH = CURLOPT_PROXYAUTH;
const int64_t k_CURLOPT_PROXYPORT = CURLOPT_PROXYPORT;
const int64_t k_CURLOPT_PROXYTYPE = CURLOPT_PROXYTYPE;
const int64_t k_CURLOPT_PROXYUSERPWD = CURLOPT_PROXYUSERPWD;
const int64_t k_CURLOPT_PUT = CURLOPT_PUT;
const int64_t k_CURLOPT_QUOTE = CURLOPT_QUOTE;
const int64_t k_CURLOPT_RANDOM_FILE = CURLOPT_RANDOM_FILE;
const int64_t k_CURLOPT_RANGE = CURLOPT_RANGE;
const int64_t k_CURLOPT_READDATA = CURLOPT_READDATA;
const int64_t k_CURLOPT_READFUNCTION = CURLOPT_READFUNCTION;
const int64_t k_CURLOPT_REFERER = CURLOPT_REFERER;
const int64_t k_CURLOPT_RESUME_FROM = CURLOPT_RESUME_FROM;
const int64_t k_CURLOPT_RETURNTRANSFER = CURLOPT_RETURNTRANSFER;
const int64_t k_CURLOPT_SSLCERT = CURLOPT_SSLCERT;
const int64_t k_CURLOPT_SSLCERTPASSWD = CURLOPT_SSLCERTPASSWD;
const int64_t k_CURLOPT_SSLCERTTYPE = CURLOPT_SSLCERTTYPE;
const int64_t k_CURLOPT_SSLENGINE = CURLOPT_SSLENGINE;
const int64_t k_CURLOPT_SSLENGINE_DEFAULT = CURLOPT_SSLENGINE_DEFAULT;
const int64_t k_CURLOPT_SSLKEY = CURLOPT_SSLKEY;
const int64_t k_CURLOPT_SSLKEYPASSWD = CURLOPT_SSLKEYPASSWD;
const int64_t k_CURLOPT_SSLKEYTYPE = CURLOPT_SSLKEYTYPE;
const int64_t k_CURLOPT_SSLVERSION = CURLOPT_SSLVERSION;
const int64_t k_CURLOPT_SSL_CIPHER_LIST = CURLOPT_SSL_CIPHER_LIST;
const int64_t k_CURLOPT_SSL_VERIFYHOST = CURLOPT_SSL_VERIFYHOST;
const int64_t k_CURLOPT_SSL_VERIFYPEER = CURLOPT_SSL_VERIFYPEER;
const int64_t k_CURLOPT_STDERR = CURLOPT_STDERR;
const int64_t k_CURLOPT_TCP_NODELAY = CURLOPT_TCP_NODELAY;
const int64_t k_CURLOPT_TIMECONDITION = CURLOPT_TIMECONDITION;
const int64_t k_CURLOPT_TIMEOUT = CURLOPT_TIMEOUT;
const int64_t k_CURLOPT_TIMEVALUE = CURLOPT_TIMEVALUE;
const int64_t k_CURLOPT_TRANSFERTEXT = CURLOPT_TRANSFERTEXT;
const int64_t k_CURLOPT_UNRESTRICTED_AUTH = CURLOPT_UNRESTRICTED_AUTH;
const int64_t k_CURLOPT_UPLOAD = CURLOPT_UPLOAD;
const int64_t k_CURLOPT_URL = CURLOPT_URL;
const int64_t k_CURLOPT_USERAGENT = CURLOPT_USERAGENT;
const int64_t k_CURLOPT_USERPWD = CURLOPT_USERPWD;
const int64_t k_CURLOPT_VERBOSE = CURLOPT_VERBOSE;
const int64_t k_CURLOPT_WRITEFUNCTION = CURLOPT_WRITEFUNCTION;
const int64_t k_CURLOPT_WRITEHEADER = CURLOPT_WRITEHEADER;
const int64_t k_CURLOPT_FB_TLS_VER_MAX =
  CurlResource::fb_specific_options::CURLOPT_FB_TLS_VER_MAX;
const int64_t k_CURLOPT_FB_TLS_VER_MAX_NONE =
  CurlResource::fb_specific_options::CURLOPT_FB_TLS_VER_MAX_NONE;
const int64_t k_CURLOPT_FB_TLS_VER_MAX_1_1 =
  CurlResource::fb_specific_options::CURLOPT_FB_TLS_VER_MAX_1_1;
const int64_t k_CURLOPT_FB_TLS_VER_MAX_1_0 =
  CurlResource::fb_specific_options::CURLOPT_FB_TLS_VER_MAX_1_0;
const int64_t k_CURLOPT_FB_TLS_CIPHER_SPEC =
  CurlResource::fb_specific_options::CURLOPT_FB_TLS_CIPHER_SPEC;
const int64_t k_CURLPROXY_HTTP = CURLPROXY_HTTP;
const int64_t k_CURLPROXY_SOCKS5 = CURLPROXY_SOCKS5;
const int64_t k_CURLVERSION_NOW = CURLVERSION_NOW;
const int64_t k_CURL_HTTP_VERSION_1_0 = CURL_HTTP_VERSION_1_0;
const int64_t k_CURL_HTTP_VERSION_1_1 = CURL_HTTP_VERSION_1_1;
const int64_t k_CURL_HTTP_VERSION_NONE = CURL_HTTP_VERSION_NONE;
const int64_t k_CURL_IPRESOLVE_V4 = CURL_IPRESOLVE_V4;
const int64_t k_CURL_IPRESOLVE_V6 = CURL_IPRESOLVE_V6;
const int64_t k_CURL_IPRESOLVE_WHATEVER = CURL_IPRESOLVE_WHATEVER;
const int64_t k_CURL_NETRC_IGNORED = CURL_NETRC_IGNORED;
const int64_t k_CURL_NETRC_OPTIONAL = CURL_NETRC_OPTIONAL;
const int64_t k_CURL_NETRC_REQUIRED = CURL_NETRC_REQUIRED;
const int64_t k_CURL_TIMECOND_IFMODSINCE = CURL_TIMECOND_IFMODSINCE;
const int64_t k_CURL_TIMECOND_IFUNMODSINCE = CURL_TIMECOND_IFUNMODSINCE;
const int64_t k_CURL_TIMECOND_LASTMOD = CURL_TIMECOND_LASTMOD;
const int64_t k_CURL_VERSION_IPV6 = CURL_VERSION_IPV6;
const int64_t k_CURL_VERSION_KERBEROS4 = CURL_VERSION_KERBEROS4;
const int64_t k_CURL_VERSION_LIBZ = CURL_VERSION_LIBZ;
const int64_t k_CURL_VERSION_SSL = CURL_VERSION_SSL;

///////////////////////////////////////////////////////////////////////////////

#if LIBCURL_VERSION_NUM >= 0x071500
const StaticString s_CURLINFO_LOCAL_PORT("CURLINFO_LOCAL_PORT");
#endif
#if LIBCURL_VERSION_NUM >= 0x071002
const StaticString s_CURLOPT_TIMEOUT_MS("CURLOPT_TIMEOUT_MS");
const StaticString s_CURLOPT_CONNECTTIMEOUT_MS("CURLOPT_CONNECTTIMEOUT_MS");
#endif
const StaticString s_CURLAUTH_ANY("CURLAUTH_ANY");
const StaticString s_CURLAUTH_ANYSAFE("CURLAUTH_ANYSAFE");
const StaticString s_CURLAUTH_BASIC("CURLAUTH_BASIC");
const StaticString s_CURLAUTH_DIGEST("CURLAUTH_DIGEST");
const StaticString s_CURLAUTH_GSSNEGOTIATE("CURLAUTH_GSSNEGOTIATE");
const StaticString s_CURLAUTH_NTLM("CURLAUTH_NTLM");
const StaticString s_CURLCLOSEPOLICY_CALLBACK("CURLCLOSEPOLICY_CALLBACK");
const StaticString
  s_CURLCLOSEPOLICY_LEAST_RECENTLY_USED("CURLCLOSEPOLICY_LEAST_RECENTLY_USED");
const StaticString
  s_CURLCLOSEPOLICY_LEAST_TRAFFIC("CURLCLOSEPOLICY_LEAST_TRAFFIC");
const StaticString s_CURLCLOSEPOLICY_OLDEST("CURLCLOSEPOLICY_OLDEST");
const StaticString s_CURLCLOSEPOLICY_SLOWEST("CURLCLOSEPOLICY_SLOWEST");
const StaticString s_CURLE_ABORTED_BY_CALLBACK("CURLE_ABORTED_BY_CALLBACK");
const StaticString s_CURLE_BAD_CALLING_ORDER("CURLE_BAD_CALLING_ORDER");
const StaticString s_CURLE_BAD_CONTENT_ENCODING("CURLE_BAD_CONTENT_ENCODING");
const StaticString s_CURLE_BAD_FUNCTION_ARGUMENT("CURLE_BAD_FUNCTION_ARGUMENT");
const StaticString s_CURLE_BAD_PASSWORD_ENTERED("CURLE_BAD_PASSWORD_ENTERED");
const StaticString s_CURLE_COULDNT_CONNECT("CURLE_COULDNT_CONNECT");
const StaticString s_CURLE_COULDNT_RESOLVE_HOST("CURLE_COULDNT_RESOLVE_HOST");
const StaticString s_CURLE_COULDNT_RESOLVE_PROXY("CURLE_COULDNT_RESOLVE_PROXY");
const StaticString s_CURLE_FAILED_INIT("CURLE_FAILED_INIT");
const StaticString s_CURLE_FILESIZE_EXCEEDED("CURLE_FILESIZE_EXCEEDED");
const StaticString
  s_CURLE_FILE_COULDNT_READ_FILE("CURLE_FILE_COULDNT_READ_FILE");
const StaticString s_CURLE_FTP_ACCESS_DENIED("CURLE_FTP_ACCESS_DENIED");
const StaticString
  s_CURLE_FTP_BAD_DOWNLOAD_RESUME("CURLE_FTP_BAD_DOWNLOAD_RESUME");
const StaticString s_CURLE_FTP_CANT_GET_HOST("CURLE_FTP_CANT_GET_HOST");
const StaticString s_CURLE_FTP_CANT_RECONNECT("CURLE_FTP_CANT_RECONNECT");
const StaticString s_CURLE_FTP_COULDNT_GET_SIZE("CURLE_FTP_COULDNT_GET_SIZE");
const StaticString s_CURLE_FTP_COULDNT_RETR_FILE("CURLE_FTP_COULDNT_RETR_FILE");
const StaticString s_CURLE_FTP_COULDNT_SET_ASCII("CURLE_FTP_COULDNT_SET_ASCII");
const StaticString
  s_CURLE_FTP_COULDNT_SET_BINARY("CURLE_FTP_COULDNT_SET_BINARY");
const StaticString s_CURLE_FTP_COULDNT_STOR_FILE("CURLE_FTP_COULDNT_STOR_FILE");
const StaticString s_CURLE_FTP_COULDNT_USE_REST("CURLE_FTP_COULDNT_USE_REST");
const StaticString s_CURLE_FTP_PORT_FAILED("CURLE_FTP_PORT_FAILED");
const StaticString s_CURLE_FTP_QUOTE_ERROR("CURLE_FTP_QUOTE_ERROR");
const StaticString s_CURLE_FTP_SSL_FAILED("CURLE_FTP_SSL_FAILED");
const StaticString
  s_CURLE_FTP_USER_PASSWORD_INCORRECT("CURLE_FTP_USER_PASSWORD_INCORRECT");
const StaticString s_CURLE_FTP_WEIRD_227_FORMAT("CURLE_FTP_WEIRD_227_FORMAT");
const StaticString s_CURLE_FTP_WEIRD_PASS_REPLY("CURLE_FTP_WEIRD_PASS_REPLY");
const StaticString s_CURLE_FTP_WEIRD_PASV_REPLY("CURLE_FTP_WEIRD_PASV_REPLY");
const StaticString
  s_CURLE_FTP_WEIRD_SERVER_REPLY("CURLE_FTP_WEIRD_SERVER_REPLY");
const StaticString s_CURLE_FTP_WEIRD_USER_REPLY("CURLE_FTP_WEIRD_USER_REPLY");
const StaticString s_CURLE_FTP_WRITE_ERROR("CURLE_FTP_WRITE_ERROR");
const StaticString s_CURLE_FUNCTION_NOT_FOUND("CURLE_FUNCTION_NOT_FOUND");
const StaticString s_CURLE_GOT_NOTHING("CURLE_GOT_NOTHING");
const StaticString s_CURLE_HTTP_NOT_FOUND("CURLE_HTTP_NOT_FOUND");
const StaticString s_CURLE_HTTP_PORT_FAILED("CURLE_HTTP_PORT_FAILED");
const StaticString s_CURLE_HTTP_POST_ERROR("CURLE_HTTP_POST_ERROR");
const StaticString s_CURLE_HTTP_RANGE_ERROR("CURLE_HTTP_RANGE_ERROR");
const StaticString s_CURLE_LDAP_CANNOT_BIND("CURLE_LDAP_CANNOT_BIND");
const StaticString s_CURLE_LDAP_INVALID_URL("CURLE_LDAP_INVALID_URL");
const StaticString s_CURLE_LDAP_SEARCH_FAILED("CURLE_LDAP_SEARCH_FAILED");
const StaticString s_CURLE_LIBRARY_NOT_FOUND("CURLE_LIBRARY_NOT_FOUND");
const StaticString s_CURLE_MALFORMAT_USER("CURLE_MALFORMAT_USER");
const StaticString s_CURLE_OBSOLETE("CURLE_OBSOLETE");
const StaticString s_CURLE_OK("CURLE_OK");
const StaticString s_CURLE_OPERATION_TIMEOUTED("CURLE_OPERATION_TIMEOUTED");
const StaticString s_CURLE_OUT_OF_MEMORY("CURLE_OUT_OF_MEMORY");
const StaticString s_CURLE_PARTIAL_FILE("CURLE_PARTIAL_FILE");
const StaticString s_CURLE_READ_ERROR("CURLE_READ_ERROR");
const StaticString s_CURLE_RECV_ERROR("CURLE_RECV_ERROR");
const StaticString s_CURLE_SEND_ERROR("CURLE_SEND_ERROR");
const StaticString s_CURLE_SHARE_IN_USE("CURLE_SHARE_IN_USE");
const StaticString s_CURLE_SSL_CACERT("CURLE_SSL_CACERT");
const StaticString s_CURLE_SSL_CERTPROBLEM("CURLE_SSL_CERTPROBLEM");
const StaticString s_CURLE_SSL_CIPHER("CURLE_SSL_CIPHER");
const StaticString s_CURLE_SSL_CONNECT_ERROR("CURLE_SSL_CONNECT_ERROR");
const StaticString s_CURLE_SSL_ENGINE_NOTFOUND("CURLE_SSL_ENGINE_NOTFOUND");
const StaticString s_CURLE_SSL_ENGINE_SETFAILED("CURLE_SSL_ENGINE_SETFAILED");
const StaticString s_CURLE_SSL_PEER_CERTIFICATE("CURLE_SSL_PEER_CERTIFICATE");
const StaticString s_CURLE_TELNET_OPTION_SYNTAX("CURLE_TELNET_OPTION_SYNTAX");
const StaticString s_CURLE_TOO_MANY_REDIRECTS("CURLE_TOO_MANY_REDIRECTS");
const StaticString s_CURLE_UNKNOWN_TELNET_OPTION("CURLE_UNKNOWN_TELNET_OPTION");
const StaticString s_CURLE_UNSUPPORTED_PROTOCOL("CURLE_UNSUPPORTED_PROTOCOL");
const StaticString s_CURLE_URL_MALFORMAT("CURLE_URL_MALFORMAT");
const StaticString s_CURLE_URL_MALFORMAT_USER("CURLE_URL_MALFORMAT_USER");
const StaticString s_CURLE_WRITE_ERROR("CURLE_WRITE_ERROR");
const StaticString s_CURLFTPAUTH_DEFAULT("CURLFTPAUTH_DEFAULT");
const StaticString s_CURLFTPAUTH_SSL("CURLFTPAUTH_SSL");
const StaticString s_CURLFTPAUTH_TLS("CURLFTPAUTH_TLS");
const StaticString s_CURLFTPSSL_ALL("CURLFTPSSL_ALL");
const StaticString s_CURLFTPSSL_CONTROL("CURLFTPSSL_CONTROL");
const StaticString s_CURLFTPSSL_NONE("CURLFTPSSL_NONE");
const StaticString s_CURLFTPSSL_TRY("CURLFTPSSL_TRY");
const StaticString s_CURLINFO_CONNECT_TIME("CURLINFO_CONNECT_TIME");
const StaticString
  s_CURLINFO_CONTENT_LENGTH_DOWNLOAD("CURLINFO_CONTENT_LENGTH_DOWNLOAD");
const StaticString
  s_CURLINFO_CONTENT_LENGTH_UPLOAD("CURLINFO_CONTENT_LENGTH_UPLOAD");
const StaticString s_CURLINFO_CONTENT_TYPE("CURLINFO_CONTENT_TYPE");
const StaticString s_CURLINFO_EFFECTIVE_URL("CURLINFO_EFFECTIVE_URL");
const StaticString s_CURLINFO_FILETIME("CURLINFO_FILETIME");
const StaticString s_CURLINFO_HEADER_OUT("CURLINFO_HEADER_OUT");
const StaticString s_CURLINFO_HEADER_SIZE("CURLINFO_HEADER_SIZE");
const StaticString s_CURLINFO_HTTP_CODE("CURLINFO_HTTP_CODE");
const StaticString s_CURLINFO_NAMELOOKUP_TIME("CURLINFO_NAMELOOKUP_TIME");
const StaticString s_CURLINFO_PRETRANSFER_TIME("CURLINFO_PRETRANSFER_TIME");
const StaticString s_CURLINFO_PRIVATE("CURLINFO_PRIVATE");
const StaticString s_CURLINFO_REDIRECT_COUNT("CURLINFO_REDIRECT_COUNT");
const StaticString s_CURLINFO_REDIRECT_TIME("CURLINFO_REDIRECT_TIME");
const StaticString s_CURLINFO_REQUEST_SIZE("CURLINFO_REQUEST_SIZE");
const StaticString s_CURLINFO_SIZE_DOWNLOAD("CURLINFO_SIZE_DOWNLOAD");
const StaticString s_CURLINFO_SIZE_UPLOAD("CURLINFO_SIZE_UPLOAD");
const StaticString s_CURLINFO_SPEED_DOWNLOAD("CURLINFO_SPEED_DOWNLOAD");
const StaticString s_CURLINFO_SPEED_UPLOAD("CURLINFO_SPEED_UPLOAD");
const StaticString s_CURLINFO_SSL_VERIFYRESULT("CURLINFO_SSL_VERIFYRESULT");
const StaticString s_CURLINFO_STARTTRANSFER_TIME("CURLINFO_STARTTRANSFER_TIME");
const StaticString s_CURLINFO_TOTAL_TIME("CURLINFO_TOTAL_TIME");
const StaticString s_CURLMSG_DONE("CURLMSG_DONE");
const StaticString s_CURLM_BAD_EASY_HANDLE("CURLM_BAD_EASY_HANDLE");
const StaticString s_CURLM_BAD_HANDLE("CURLM_BAD_HANDLE");
const StaticString s_CURLM_CALL_MULTI_PERFORM("CURLM_CALL_MULTI_PERFORM");
const StaticString s_CURLM_INTERNAL_ERROR("CURLM_INTERNAL_ERROR");
const StaticString s_CURLM_OK("CURLM_OK");
const StaticString s_CURLM_OUT_OF_MEMORY("CURLM_OUT_OF_MEMORY");
const StaticString s_CURLOPT_AUTOREFERER("CURLOPT_AUTOREFERER");
const StaticString s_CURLOPT_BINARYTRANSFER("CURLOPT_BINARYTRANSFER");
const StaticString s_CURLOPT_BUFFERSIZE("CURLOPT_BUFFERSIZE");
const StaticString s_CURLOPT_CAINFO("CURLOPT_CAINFO");
const StaticString s_CURLOPT_CAPATH("CURLOPT_CAPATH");
const StaticString s_CURLOPT_CLOSEPOLICY("CURLOPT_CLOSEPOLICY");
const StaticString s_CURLOPT_CONNECTTIMEOUT("CURLOPT_CONNECTTIMEOUT");
const StaticString s_CURLOPT_COOKIE("CURLOPT_COOKIE");
const StaticString s_CURLOPT_COOKIEFILE("CURLOPT_COOKIEFILE");
const StaticString s_CURLOPT_COOKIEJAR("CURLOPT_COOKIEJAR");
const StaticString s_CURLOPT_COOKIESESSION("CURLOPT_COOKIESESSION");
const StaticString s_CURLOPT_CRLF("CURLOPT_CRLF");
const StaticString s_CURLOPT_CUSTOMREQUEST("CURLOPT_CUSTOMREQUEST");
const StaticString s_CURLOPT_DNS_CACHE_TIMEOUT("CURLOPT_DNS_CACHE_TIMEOUT");
const StaticString
  s_CURLOPT_DNS_USE_GLOBAL_CACHE("CURLOPT_DNS_USE_GLOBAL_CACHE");
const StaticString s_CURLOPT_EGDSOCKET("CURLOPT_EGDSOCKET");
const StaticString s_CURLOPT_ENCODING("CURLOPT_ENCODING");
const StaticString s_CURLOPT_FAILONERROR("CURLOPT_FAILONERROR");
const StaticString s_CURLOPT_FILE("CURLOPT_FILE");
const StaticString s_CURLOPT_FILETIME("CURLOPT_FILETIME");
const StaticString s_CURLOPT_FOLLOWLOCATION("CURLOPT_FOLLOWLOCATION");
const StaticString s_CURLOPT_FORBID_REUSE("CURLOPT_FORBID_REUSE");
const StaticString s_CURLOPT_FRESH_CONNECT("CURLOPT_FRESH_CONNECT");
const StaticString s_CURLOPT_FTPAPPEND("CURLOPT_FTPAPPEND");
const StaticString s_CURLOPT_FTPLISTONLY("CURLOPT_FTPLISTONLY");
const StaticString s_CURLOPT_FTPPORT("CURLOPT_FTPPORT");
const StaticString s_CURLOPT_FTPSSLAUTH("CURLOPT_FTPSSLAUTH");
const StaticString
  s_CURLOPT_FTP_CREATE_MISSING_DIRS("CURLOPT_FTP_CREATE_MISSING_DIRS");
const StaticString s_CURLOPT_FTP_SSL("CURLOPT_FTP_SSL");
const StaticString s_CURLOPT_FTP_USE_EPRT("CURLOPT_FTP_USE_EPRT");
const StaticString s_CURLOPT_FTP_USE_EPSV("CURLOPT_FTP_USE_EPSV");
const StaticString s_CURLOPT_HEADER("CURLOPT_HEADER");
const StaticString s_CURLOPT_HEADERFUNCTION("CURLOPT_HEADERFUNCTION");
const StaticString s_CURLOPT_HTTP200ALIASES("CURLOPT_HTTP200ALIASES");
const StaticString s_CURLOPT_HTTPAUTH("CURLOPT_HTTPAUTH");
const StaticString s_CURLOPT_HTTPGET("CURLOPT_HTTPGET");
const StaticString s_CURLOPT_HTTPHEADER("CURLOPT_HTTPHEADER");
const StaticString s_CURLOPT_HTTPPROXYTUNNEL("CURLOPT_HTTPPROXYTUNNEL");
const StaticString s_CURLOPT_HTTP_VERSION("CURLOPT_HTTP_VERSION");
const StaticString s_CURLOPT_INFILE("CURLOPT_INFILE");
const StaticString s_CURLOPT_INFILESIZE("CURLOPT_INFILESIZE");
const StaticString s_CURLOPT_INTERFACE("CURLOPT_INTERFACE");
const StaticString s_CURLOPT_IPRESOLVE("CURLOPT_IPRESOLVE");
const StaticString s_CURLOPT_KRB4LEVEL("CURLOPT_KRB4LEVEL");
const StaticString s_CURLOPT_LOW_SPEED_LIMIT("CURLOPT_LOW_SPEED_LIMIT");
const StaticString s_CURLOPT_LOW_SPEED_TIME("CURLOPT_LOW_SPEED_TIME");
const StaticString s_CURLOPT_MAXCONNECTS("CURLOPT_MAXCONNECTS");
const StaticString s_CURLOPT_MAXREDIRS("CURLOPT_MAXREDIRS");
const StaticString s_CURLOPT_MUTE("CURLOPT_MUTE");
const StaticString s_CURLOPT_NETRC("CURLOPT_NETRC");
const StaticString s_CURLOPT_NOBODY("CURLOPT_NOBODY");
const StaticString s_CURLOPT_NOPROGRESS("CURLOPT_NOPROGRESS");
const StaticString s_CURLOPT_NOSIGNAL("CURLOPT_NOSIGNAL");
const StaticString s_CURLOPT_PASSWDFUNCTION("CURLOPT_PASSWDFUNCTION");
const StaticString s_CURLOPT_PORT("CURLOPT_PORT");
const StaticString s_CURLOPT_POST("CURLOPT_POST");
const StaticString s_CURLOPT_POSTFIELDS("CURLOPT_POSTFIELDS");
const StaticString s_CURLOPT_POSTREDIR("CURLOPT_POSTREDIR");
const StaticString s_CURLOPT_POSTQUOTE("CURLOPT_POSTQUOTE");
const StaticString s_CURLOPT_PRIVATE("CURLOPT_PRIVATE");
const StaticString s_CURLOPT_PROGRESSFUNCTION("CURLOPT_PROGRESSFUNCTION");
const StaticString s_CURLOPT_PROXY("CURLOPT_PROXY");
const StaticString s_CURLOPT_PROXYAUTH("CURLOPT_PROXYAUTH");
const StaticString s_CURLOPT_PROXYPORT("CURLOPT_PROXYPORT");
const StaticString s_CURLOPT_PROXYTYPE("CURLOPT_PROXYTYPE");
const StaticString s_CURLOPT_PROXYUSERPWD("CURLOPT_PROXYUSERPWD");
const StaticString s_CURLOPT_PUT("CURLOPT_PUT");
const StaticString s_CURLOPT_QUOTE("CURLOPT_QUOTE");
const StaticString s_CURLOPT_RANDOM_FILE("CURLOPT_RANDOM_FILE");
const StaticString s_CURLOPT_RANGE("CURLOPT_RANGE");
const StaticString s_CURLOPT_READDATA("CURLOPT_READDATA");
const StaticString s_CURLOPT_READFUNCTION("CURLOPT_READFUNCTION");
const StaticString s_CURLOPT_REFERER("CURLOPT_REFERER");
const StaticString s_CURLOPT_RESUME_FROM("CURLOPT_RESUME_FROM");
const StaticString s_CURLOPT_RETURNTRANSFER("CURLOPT_RETURNTRANSFER");
const StaticString s_CURLOPT_SSLCERT("CURLOPT_SSLCERT");
const StaticString s_CURLOPT_SSLCERTPASSWD("CURLOPT_SSLCERTPASSWD");
const StaticString s_CURLOPT_SSLCERTTYPE("CURLOPT_SSLCERTTYPE");
const StaticString s_CURLOPT_SSLENGINE("CURLOPT_SSLENGINE");
const StaticString s_CURLOPT_SSLENGINE_DEFAULT("CURLOPT_SSLENGINE_DEFAULT");
const StaticString s_CURLOPT_SSLKEY("CURLOPT_SSLKEY");
const StaticString s_CURLOPT_SSLKEYPASSWD("CURLOPT_SSLKEYPASSWD");
const StaticString s_CURLOPT_SSLKEYTYPE("CURLOPT_SSLKEYTYPE");
const StaticString s_CURLOPT_SSLVERSION("CURLOPT_SSLVERSION");
const StaticString s_CURLOPT_SSL_CIPHER_LIST("CURLOPT_SSL_CIPHER_LIST");
const StaticString s_CURLOPT_SSL_VERIFYHOST("CURLOPT_SSL_VERIFYHOST");
const StaticString s_CURLOPT_SSL_VERIFYPEER("CURLOPT_SSL_VERIFYPEER");
const StaticString s_CURLOPT_STDERR("CURLOPT_STDERR");
const StaticString s_CURLOPT_TCP_NODELAY("CURLOPT_TCP_NODELAY");
const StaticString s_CURLOPT_TIMECONDITION("CURLOPT_TIMECONDITION");
const StaticString s_CURLOPT_TIMEOUT("CURLOPT_TIMEOUT");
const StaticString s_CURLOPT_TIMEVALUE("CURLOPT_TIMEVALUE");
const StaticString s_CURLOPT_TRANSFERTEXT("CURLOPT_TRANSFERTEXT");
const StaticString s_CURLOPT_UNRESTRICTED_AUTH("CURLOPT_UNRESTRICTED_AUTH");
const StaticString s_CURLOPT_UPLOAD("CURLOPT_UPLOAD");
const StaticString s_CURLOPT_URL("CURLOPT_URL");
const StaticString s_CURLOPT_USERAGENT("CURLOPT_USERAGENT");
const StaticString s_CURLOPT_USERPWD("CURLOPT_USERPWD");
const StaticString s_CURLOPT_VERBOSE("CURLOPT_VERBOSE");
const StaticString s_CURLOPT_WRITEFUNCTION("CURLOPT_WRITEFUNCTION");
const StaticString s_CURLOPT_WRITEHEADER("CURLOPT_WRITEHEADER");
const StaticString s_CURLOPT_FB_TLS_VER_MAX("CURLOPT_FB_TLS_VER_MAX");
const StaticString s_CURLOPT_FB_TLS_VER_MAX_NONE("CURLOPT_FB_TLS_VER_MAX_NONE");
const StaticString s_CURLOPT_FB_TLS_VER_MAX_1_1("CURLOPT_FB_TLS_VER_MAX_1_1");
const StaticString s_CURLOPT_FB_TLS_VER_MAX_1_0("CURLOPT_FB_TLS_VER_MAX_1_0");
const StaticString s_CURLOPT_FB_TLS_CIPHER_SPEC("CURLOPT_FB_TLS_CIPHER_SPEC");
const StaticString s_CURLPROXY_HTTP("CURLPROXY_HTTP");
const StaticString s_CURLPROXY_SOCKS5("CURLPROXY_SOCKS5");
const StaticString s_CURLVERSION_NOW("CURLVERSION_NOW");
const StaticString s_CURL_HTTP_VERSION_1_0("CURL_HTTP_VERSION_1_0");
const StaticString s_CURL_HTTP_VERSION_1_1("CURL_HTTP_VERSION_1_1");
const StaticString s_CURL_HTTP_VERSION_NONE("CURL_HTTP_VERSION_NONE");
const StaticString s_CURL_IPRESOLVE_V4("CURL_IPRESOLVE_V4");
const StaticString s_CURL_IPRESOLVE_V6("CURL_IPRESOLVE_V6");
const StaticString s_CURL_IPRESOLVE_WHATEVER("CURL_IPRESOLVE_WHATEVER");
const StaticString s_CURL_NETRC_IGNORED("CURL_NETRC_IGNORED");
const StaticString s_CURL_NETRC_OPTIONAL("CURL_NETRC_OPTIONAL");
const StaticString s_CURL_NETRC_REQUIRED("CURL_NETRC_REQUIRED");
const StaticString s_CURL_TIMECOND_IFMODSINCE("CURL_TIMECOND_IFMODSINCE");
const StaticString s_CURL_TIMECOND_IFUNMODSINCE("CURL_TIMECOND_IFUNMODSINCE");
const StaticString s_CURL_TIMECOND_LASTMOD("CURL_TIMECOND_LASTMOD");
const StaticString s_CURL_VERSION_IPV6("CURL_VERSION_IPV6");
const StaticString s_CURL_VERSION_KERBEROS4("CURL_VERSION_KERBEROS4");
const StaticString s_CURL_VERSION_LIBZ("CURL_VERSION_LIBZ");
const StaticString s_CURL_VERSION_SSL("CURL_VERSION_SSL");

class CurlExtension : public Extension {
 public:
  CurlExtension() : Extension("curl") {}
  virtual void moduleInit() {
#if LIBCURL_VERSION_NUM >= 0x071500
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_LOCAL_PORT.get(), k_CURLINFO_LOCAL_PORT
    );
#endif
#if LIBCURL_VERSION_NUM >= 0x071002
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_TIMEOUT_MS.get(), k_CURLOPT_TIMEOUT_MS
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_CONNECTTIMEOUT_MS.get(), k_CURLOPT_CONNECTTIMEOUT_MS
    );
#endif
    Native::registerConstant<KindOfInt64>(
      s_CURLAUTH_ANY.get(), k_CURLAUTH_ANY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLAUTH_ANYSAFE.get(), k_CURLAUTH_ANYSAFE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLAUTH_BASIC.get(), k_CURLAUTH_BASIC
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLAUTH_DIGEST.get(), k_CURLAUTH_DIGEST
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLAUTH_GSSNEGOTIATE.get(), k_CURLAUTH_GSSNEGOTIATE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLAUTH_NTLM.get(), k_CURLAUTH_NTLM
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLCLOSEPOLICY_CALLBACK.get(), k_CURLCLOSEPOLICY_CALLBACK
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLCLOSEPOLICY_LEAST_RECENTLY_USED.get(),
      k_CURLCLOSEPOLICY_LEAST_RECENTLY_USED
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLCLOSEPOLICY_LEAST_TRAFFIC.get(), k_CURLCLOSEPOLICY_LEAST_TRAFFIC
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLCLOSEPOLICY_OLDEST.get(), k_CURLCLOSEPOLICY_OLDEST
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLCLOSEPOLICY_SLOWEST.get(), k_CURLCLOSEPOLICY_SLOWEST
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_ABORTED_BY_CALLBACK.get(), k_CURLE_ABORTED_BY_CALLBACK
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_BAD_CALLING_ORDER.get(), k_CURLE_BAD_CALLING_ORDER
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_BAD_CONTENT_ENCODING.get(), k_CURLE_BAD_CONTENT_ENCODING
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_BAD_FUNCTION_ARGUMENT.get(), k_CURLE_BAD_FUNCTION_ARGUMENT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_BAD_PASSWORD_ENTERED.get(), k_CURLE_BAD_PASSWORD_ENTERED
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_COULDNT_CONNECT.get(), k_CURLE_COULDNT_CONNECT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_COULDNT_RESOLVE_HOST.get(), k_CURLE_COULDNT_RESOLVE_HOST
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_COULDNT_RESOLVE_PROXY.get(), k_CURLE_COULDNT_RESOLVE_PROXY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FAILED_INIT.get(), k_CURLE_FAILED_INIT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FILESIZE_EXCEEDED.get(), k_CURLE_FILESIZE_EXCEEDED
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FILE_COULDNT_READ_FILE.get(), k_CURLE_FILE_COULDNT_READ_FILE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_ACCESS_DENIED.get(), k_CURLE_FTP_ACCESS_DENIED
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_BAD_DOWNLOAD_RESUME.get(), k_CURLE_FTP_BAD_DOWNLOAD_RESUME
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_CANT_GET_HOST.get(), k_CURLE_FTP_CANT_GET_HOST
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_CANT_RECONNECT.get(), k_CURLE_FTP_CANT_RECONNECT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_COULDNT_GET_SIZE.get(), k_CURLE_FTP_COULDNT_GET_SIZE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_COULDNT_RETR_FILE.get(), k_CURLE_FTP_COULDNT_RETR_FILE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_COULDNT_SET_ASCII.get(), k_CURLE_FTP_COULDNT_SET_ASCII
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_COULDNT_SET_BINARY.get(), k_CURLE_FTP_COULDNT_SET_BINARY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_COULDNT_STOR_FILE.get(), k_CURLE_FTP_COULDNT_STOR_FILE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_COULDNT_USE_REST.get(), k_CURLE_FTP_COULDNT_USE_REST
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_PORT_FAILED.get(), k_CURLE_FTP_PORT_FAILED
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_QUOTE_ERROR.get(), k_CURLE_FTP_QUOTE_ERROR
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_SSL_FAILED.get(), k_CURLE_FTP_SSL_FAILED
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_USER_PASSWORD_INCORRECT.get(),
      k_CURLE_FTP_USER_PASSWORD_INCORRECT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_WEIRD_227_FORMAT.get(), k_CURLE_FTP_WEIRD_227_FORMAT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_WEIRD_PASS_REPLY.get(), k_CURLE_FTP_WEIRD_PASS_REPLY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_WEIRD_PASV_REPLY.get(), k_CURLE_FTP_WEIRD_PASV_REPLY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_WEIRD_SERVER_REPLY.get(), k_CURLE_FTP_WEIRD_SERVER_REPLY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_WEIRD_USER_REPLY.get(), k_CURLE_FTP_WEIRD_USER_REPLY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FTP_WRITE_ERROR.get(), k_CURLE_FTP_WRITE_ERROR
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_FUNCTION_NOT_FOUND.get(), k_CURLE_FUNCTION_NOT_FOUND
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_GOT_NOTHING.get(), k_CURLE_GOT_NOTHING
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_HTTP_NOT_FOUND.get(), k_CURLE_HTTP_NOT_FOUND
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_HTTP_PORT_FAILED.get(), k_CURLE_HTTP_PORT_FAILED
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_HTTP_POST_ERROR.get(), k_CURLE_HTTP_POST_ERROR
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_HTTP_RANGE_ERROR.get(), k_CURLE_HTTP_RANGE_ERROR
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_LDAP_CANNOT_BIND.get(), k_CURLE_LDAP_CANNOT_BIND
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_LDAP_INVALID_URL.get(), k_CURLE_LDAP_INVALID_URL
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_LDAP_SEARCH_FAILED.get(), k_CURLE_LDAP_SEARCH_FAILED
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_LIBRARY_NOT_FOUND.get(), k_CURLE_LIBRARY_NOT_FOUND
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_MALFORMAT_USER.get(), k_CURLE_MALFORMAT_USER
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_OBSOLETE.get(), k_CURLE_OBSOLETE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_OK.get(), k_CURLE_OK
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_OPERATION_TIMEOUTED.get(), k_CURLE_OPERATION_TIMEOUTED
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_OUT_OF_MEMORY.get(), k_CURLE_OUT_OF_MEMORY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_PARTIAL_FILE.get(), k_CURLE_PARTIAL_FILE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_READ_ERROR.get(), k_CURLE_READ_ERROR
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_RECV_ERROR.get(), k_CURLE_RECV_ERROR
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_SEND_ERROR.get(), k_CURLE_SEND_ERROR
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_SHARE_IN_USE.get(), k_CURLE_SHARE_IN_USE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_SSL_CACERT.get(), k_CURLE_SSL_CACERT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_SSL_CERTPROBLEM.get(), k_CURLE_SSL_CERTPROBLEM
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_SSL_CIPHER.get(), k_CURLE_SSL_CIPHER
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_SSL_CONNECT_ERROR.get(), k_CURLE_SSL_CONNECT_ERROR
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_SSL_ENGINE_NOTFOUND.get(), k_CURLE_SSL_ENGINE_NOTFOUND
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_SSL_ENGINE_SETFAILED.get(), k_CURLE_SSL_ENGINE_SETFAILED
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_SSL_PEER_CERTIFICATE.get(), k_CURLE_SSL_PEER_CERTIFICATE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_TELNET_OPTION_SYNTAX.get(), k_CURLE_TELNET_OPTION_SYNTAX
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_TOO_MANY_REDIRECTS.get(), k_CURLE_TOO_MANY_REDIRECTS
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_UNKNOWN_TELNET_OPTION.get(), k_CURLE_UNKNOWN_TELNET_OPTION
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_UNSUPPORTED_PROTOCOL.get(), k_CURLE_UNSUPPORTED_PROTOCOL
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_URL_MALFORMAT.get(), k_CURLE_URL_MALFORMAT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_URL_MALFORMAT_USER.get(), k_CURLE_URL_MALFORMAT_USER
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLE_WRITE_ERROR.get(), k_CURLE_WRITE_ERROR
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLFTPAUTH_DEFAULT.get(), k_CURLFTPAUTH_DEFAULT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLFTPAUTH_SSL.get(), k_CURLFTPAUTH_SSL
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLFTPAUTH_TLS.get(), k_CURLFTPAUTH_TLS
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLFTPSSL_ALL.get(), k_CURLFTPSSL_ALL
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLFTPSSL_CONTROL.get(), k_CURLFTPSSL_CONTROL
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLFTPSSL_NONE.get(), k_CURLFTPSSL_NONE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLFTPSSL_TRY.get(), k_CURLFTPSSL_TRY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_CONNECT_TIME.get(), k_CURLINFO_CONNECT_TIME
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_CONTENT_LENGTH_DOWNLOAD.get(),
      k_CURLINFO_CONTENT_LENGTH_DOWNLOAD
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_CONTENT_LENGTH_UPLOAD.get(), k_CURLINFO_CONTENT_LENGTH_UPLOAD
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_CONTENT_TYPE.get(), k_CURLINFO_CONTENT_TYPE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_EFFECTIVE_URL.get(), k_CURLINFO_EFFECTIVE_URL
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_FILETIME.get(), k_CURLINFO_FILETIME
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_HEADER_OUT.get(), k_CURLINFO_HEADER_OUT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_HEADER_SIZE.get(), k_CURLINFO_HEADER_SIZE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_HTTP_CODE.get(), k_CURLINFO_HTTP_CODE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_NAMELOOKUP_TIME.get(), k_CURLINFO_NAMELOOKUP_TIME
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_PRETRANSFER_TIME.get(), k_CURLINFO_PRETRANSFER_TIME
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_PRIVATE.get(), k_CURLINFO_PRIVATE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_REDIRECT_COUNT.get(), k_CURLINFO_REDIRECT_COUNT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_REDIRECT_TIME.get(), k_CURLINFO_REDIRECT_TIME
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_REQUEST_SIZE.get(), k_CURLINFO_REQUEST_SIZE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_SIZE_DOWNLOAD.get(), k_CURLINFO_SIZE_DOWNLOAD
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_SIZE_UPLOAD.get(), k_CURLINFO_SIZE_UPLOAD
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_SPEED_DOWNLOAD.get(), k_CURLINFO_SPEED_DOWNLOAD
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_SPEED_UPLOAD.get(), k_CURLINFO_SPEED_UPLOAD
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_SSL_VERIFYRESULT.get(), k_CURLINFO_SSL_VERIFYRESULT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_STARTTRANSFER_TIME.get(), k_CURLINFO_STARTTRANSFER_TIME
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLINFO_TOTAL_TIME.get(), k_CURLINFO_TOTAL_TIME
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLMSG_DONE.get(), k_CURLMSG_DONE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLM_BAD_EASY_HANDLE.get(), k_CURLM_BAD_EASY_HANDLE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLM_BAD_HANDLE.get(), k_CURLM_BAD_HANDLE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLM_CALL_MULTI_PERFORM.get(), k_CURLM_CALL_MULTI_PERFORM
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLM_INTERNAL_ERROR.get(), k_CURLM_INTERNAL_ERROR
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLM_OK.get(), k_CURLM_OK
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLM_OUT_OF_MEMORY.get(), k_CURLM_OUT_OF_MEMORY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_AUTOREFERER.get(), k_CURLOPT_AUTOREFERER
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_BINARYTRANSFER.get(), k_CURLOPT_BINARYTRANSFER
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_BUFFERSIZE.get(), k_CURLOPT_BUFFERSIZE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_CAINFO.get(), k_CURLOPT_CAINFO
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_CAPATH.get(), k_CURLOPT_CAPATH
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_CLOSEPOLICY.get(), k_CURLOPT_CLOSEPOLICY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_CONNECTTIMEOUT.get(), k_CURLOPT_CONNECTTIMEOUT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_COOKIE.get(), k_CURLOPT_COOKIE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_COOKIEFILE.get(), k_CURLOPT_COOKIEFILE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_COOKIEJAR.get(), k_CURLOPT_COOKIEJAR
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_COOKIESESSION.get(), k_CURLOPT_COOKIESESSION
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_CRLF.get(), k_CURLOPT_CRLF
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_CUSTOMREQUEST.get(), k_CURLOPT_CUSTOMREQUEST
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_DNS_CACHE_TIMEOUT.get(), k_CURLOPT_DNS_CACHE_TIMEOUT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_DNS_USE_GLOBAL_CACHE.get(), k_CURLOPT_DNS_USE_GLOBAL_CACHE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_EGDSOCKET.get(), k_CURLOPT_EGDSOCKET
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_ENCODING.get(), k_CURLOPT_ENCODING
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FAILONERROR.get(), k_CURLOPT_FAILONERROR
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FILE.get(), k_CURLOPT_FILE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FILETIME.get(), k_CURLOPT_FILETIME
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FOLLOWLOCATION.get(), k_CURLOPT_FOLLOWLOCATION
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FORBID_REUSE.get(), k_CURLOPT_FORBID_REUSE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FRESH_CONNECT.get(), k_CURLOPT_FRESH_CONNECT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FTPAPPEND.get(), k_CURLOPT_FTPAPPEND
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FTPLISTONLY.get(), k_CURLOPT_FTPLISTONLY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FTPPORT.get(), k_CURLOPT_FTPPORT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FTPSSLAUTH.get(), k_CURLOPT_FTPSSLAUTH
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FTP_CREATE_MISSING_DIRS.get(), k_CURLOPT_FTP_CREATE_MISSING_DIRS
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FTP_SSL.get(), k_CURLOPT_FTP_SSL
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FTP_USE_EPRT.get(), k_CURLOPT_FTP_USE_EPRT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FTP_USE_EPSV.get(), k_CURLOPT_FTP_USE_EPSV
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_HEADER.get(), k_CURLOPT_HEADER
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_HEADERFUNCTION.get(), k_CURLOPT_HEADERFUNCTION
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_HTTP200ALIASES.get(), k_CURLOPT_HTTP200ALIASES
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_HTTPAUTH.get(), k_CURLOPT_HTTPAUTH
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_HTTPGET.get(), k_CURLOPT_HTTPGET
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_HTTPHEADER.get(), k_CURLOPT_HTTPHEADER
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_HTTPPROXYTUNNEL.get(), k_CURLOPT_HTTPPROXYTUNNEL
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_HTTP_VERSION.get(), k_CURLOPT_HTTP_VERSION
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_INFILE.get(), k_CURLOPT_INFILE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_INFILESIZE.get(), k_CURLOPT_INFILESIZE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_INTERFACE.get(), k_CURLOPT_INTERFACE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_IPRESOLVE.get(), k_CURLOPT_IPRESOLVE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_KRB4LEVEL.get(), k_CURLOPT_KRB4LEVEL
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_LOW_SPEED_LIMIT.get(), k_CURLOPT_LOW_SPEED_LIMIT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_LOW_SPEED_TIME.get(), k_CURLOPT_LOW_SPEED_TIME
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_MAXCONNECTS.get(), k_CURLOPT_MAXCONNECTS
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_MAXREDIRS.get(), k_CURLOPT_MAXREDIRS
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_MUTE.get(), k_CURLOPT_MUTE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_NETRC.get(), k_CURLOPT_NETRC
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_NOBODY.get(), k_CURLOPT_NOBODY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_NOPROGRESS.get(), k_CURLOPT_NOPROGRESS
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_NOSIGNAL.get(), k_CURLOPT_NOSIGNAL
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_PASSWDFUNCTION.get(), k_CURLOPT_PASSWDFUNCTION
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_PORT.get(), k_CURLOPT_PORT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_POST.get(), k_CURLOPT_POST
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_POSTFIELDS.get(), k_CURLOPT_POSTFIELDS
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_POSTREDIR.get(), k_CURLOPT_POSTREDIR
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_POSTQUOTE.get(), k_CURLOPT_POSTQUOTE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_PRIVATE.get(), k_CURLOPT_PRIVATE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_PROGRESSFUNCTION.get(), k_CURLOPT_PROGRESSFUNCTION
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_PROXY.get(), k_CURLOPT_PROXY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_PROXYAUTH.get(), k_CURLOPT_PROXYAUTH
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_PROXYPORT.get(), k_CURLOPT_PROXYPORT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_PROXYTYPE.get(), k_CURLOPT_PROXYTYPE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_PROXYUSERPWD.get(), k_CURLOPT_PROXYUSERPWD
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_PUT.get(), k_CURLOPT_PUT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_QUOTE.get(), k_CURLOPT_QUOTE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_RANDOM_FILE.get(), k_CURLOPT_RANDOM_FILE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_RANGE.get(), k_CURLOPT_RANGE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_READDATA.get(), k_CURLOPT_READDATA
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_READFUNCTION.get(), k_CURLOPT_READFUNCTION
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_REFERER.get(), k_CURLOPT_REFERER
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_RESUME_FROM.get(), k_CURLOPT_RESUME_FROM
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_RETURNTRANSFER.get(), k_CURLOPT_RETURNTRANSFER
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_SSLCERT.get(), k_CURLOPT_SSLCERT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_SSLCERTPASSWD.get(), k_CURLOPT_SSLCERTPASSWD
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_SSLCERTTYPE.get(), k_CURLOPT_SSLCERTTYPE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_SSLENGINE.get(), k_CURLOPT_SSLENGINE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_SSLENGINE_DEFAULT.get(), k_CURLOPT_SSLENGINE_DEFAULT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_SSLKEY.get(), k_CURLOPT_SSLKEY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_SSLKEYPASSWD.get(), k_CURLOPT_SSLKEYPASSWD
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_SSLKEYTYPE.get(), k_CURLOPT_SSLKEYTYPE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_SSLVERSION.get(), k_CURLOPT_SSLVERSION
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_SSL_CIPHER_LIST.get(), k_CURLOPT_SSL_CIPHER_LIST
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_SSL_VERIFYHOST.get(), k_CURLOPT_SSL_VERIFYHOST
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_SSL_VERIFYPEER.get(), k_CURLOPT_SSL_VERIFYPEER
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_STDERR.get(), k_CURLOPT_STDERR
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_TCP_NODELAY.get(), k_CURLOPT_TCP_NODELAY
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_TIMECONDITION.get(), k_CURLOPT_TIMECONDITION
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_TIMEOUT.get(), k_CURLOPT_TIMEOUT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_TIMEVALUE.get(), k_CURLOPT_TIMEVALUE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_TRANSFERTEXT.get(), k_CURLOPT_TRANSFERTEXT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_UNRESTRICTED_AUTH.get(), k_CURLOPT_UNRESTRICTED_AUTH
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_UPLOAD.get(), k_CURLOPT_UPLOAD
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_URL.get(), k_CURLOPT_URL
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_USERAGENT.get(), k_CURLOPT_USERAGENT
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_USERPWD.get(), k_CURLOPT_USERPWD
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_VERBOSE.get(), k_CURLOPT_VERBOSE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_WRITEFUNCTION.get(), k_CURLOPT_WRITEFUNCTION
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_WRITEHEADER.get(), k_CURLOPT_WRITEHEADER
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FB_TLS_VER_MAX.get(), k_CURLOPT_FB_TLS_VER_MAX
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FB_TLS_VER_MAX_NONE.get(), k_CURLOPT_FB_TLS_VER_MAX_NONE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FB_TLS_VER_MAX_1_1.get(), k_CURLOPT_FB_TLS_VER_MAX_1_1
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FB_TLS_VER_MAX_1_0.get(), k_CURLOPT_FB_TLS_VER_MAX_1_0
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLOPT_FB_TLS_CIPHER_SPEC.get(), k_CURLOPT_FB_TLS_CIPHER_SPEC
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLPROXY_HTTP.get(), k_CURLPROXY_HTTP
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLPROXY_SOCKS5.get(), k_CURLPROXY_SOCKS5
    );
    Native::registerConstant<KindOfInt64>(
      s_CURLVERSION_NOW.get(), k_CURLVERSION_NOW
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_HTTP_VERSION_1_0.get(), k_CURL_HTTP_VERSION_1_0
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_HTTP_VERSION_1_1.get(), k_CURL_HTTP_VERSION_1_1
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_HTTP_VERSION_NONE.get(), k_CURL_HTTP_VERSION_NONE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_IPRESOLVE_V4.get(), k_CURL_IPRESOLVE_V4
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_IPRESOLVE_V6.get(), k_CURL_IPRESOLVE_V6
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_IPRESOLVE_WHATEVER.get(), k_CURL_IPRESOLVE_WHATEVER
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_NETRC_IGNORED.get(), k_CURL_NETRC_IGNORED
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_NETRC_OPTIONAL.get(), k_CURL_NETRC_OPTIONAL
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_NETRC_REQUIRED.get(), k_CURL_NETRC_REQUIRED
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_TIMECOND_IFMODSINCE.get(), k_CURL_TIMECOND_IFMODSINCE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_TIMECOND_IFUNMODSINCE.get(), k_CURL_TIMECOND_IFUNMODSINCE
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_TIMECOND_LASTMOD.get(), k_CURL_TIMECOND_LASTMOD
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_VERSION_IPV6.get(), k_CURL_VERSION_IPV6
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_VERSION_KERBEROS4.get(), k_CURL_VERSION_KERBEROS4
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_VERSION_LIBZ.get(), k_CURL_VERSION_LIBZ
    );
    Native::registerConstant<KindOfInt64>(
      s_CURL_VERSION_SSL.get(), k_CURL_VERSION_SSL
    );

    HHVM_FE(curl_init);
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
    HHVM_FE(curl_multi_add_handle);
    HHVM_FE(curl_multi_remove_handle);
    HHVM_FE(curl_multi_exec);
    HHVM_FE(curl_multi_select);
    HHVM_FE(curl_multi_getcontent);
    HHVM_FE(fb_curl_multi_fdset);
    HHVM_FE(curl_multi_info_read);
    HHVM_FE(curl_multi_close);

    loadSystemlib();
  }
} s_curl_extension;

}

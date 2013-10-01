/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_curl.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/libevent-http-client.h"
#include "hphp/runtime/base/curl-tls-workarounds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include <openssl/ssl.h>

#define CURLOPT_RETURNTRANSFER 19913
#define CURLOPT_BINARYTRANSFER 19914
#define PHP_CURL_STDOUT 0
#define PHP_CURL_FILE   1
#define PHP_CURL_USER   2
#define PHP_CURL_DIRECT 3
#define PHP_CURL_RETURN 4
#define PHP_CURL_ASCII  5
#define PHP_CURL_BINARY 6
#define PHP_CURL_IGNORE 7


namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(curl);

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

  DECLARE_BOOST_TYPES(ToFree);
  class ToFree {
  public:
    vector<char*>          str;
    vector<curl_httppost*> post;
    vector<curl_slist*>    slist;

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
  CLASSNAME_IS("cURL handle")
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  explicit CurlResource(const String& url)
    : m_exception(nullptr), m_phpException(false), m_emptyPost(true) {
    m_cp = curl_easy_init();
    m_url = url;

    memset(m_error_str, 0, sizeof(m_error_str));
    m_error_no = CURLE_OK;
    m_to_free = ToFreePtr(new ToFree());

    m_write.method = PHP_CURL_STDOUT;
    m_write.type   = PHP_CURL_ASCII;
    m_read.method  = PHP_CURL_DIRECT;
    m_write_header.method = PHP_CURL_IGNORE;

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

    curl_easy_setopt(m_cp, CURLOPT_ERRORBUFFER,       m_error_str);
    curl_easy_setopt(m_cp, CURLOPT_FILE,              (void*)this);
    curl_easy_setopt(m_cp, CURLOPT_INFILE,            (void*)this);
    curl_easy_setopt(m_cp, CURLOPT_WRITEHEADER,       (void*)this);

    m_to_free = src->m_to_free;
    m_emptyPost = src->m_emptyPost;
  }

  ~CurlResource() {
    close();
  }

  void closeForSweep() {
    assert(!m_exception);
    if (m_cp) {
      curl_easy_cleanup(m_cp);
      m_cp = NULL;
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
        m_exception = NULL;
        e.get()->decRefCount();
        throw e;
      } else {
        Exception *e = (Exception*)m_exception;
        m_exception = NULL;
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

  Variant execute() {
    assert(!m_exception);
    if (m_cp == NULL) {
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

  bool setOption(long option, CVarRef value) {
    if (m_cp == NULL) {
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
        if (obj.isNull() || obj.getTyped<File>(true) == NULL) {
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
            if (obj.getTyped<PlainFile>(true) == NULL) {
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
    case CURLOPT_POSTFIELDS:
      m_emptyPost = false;
      if (value.is(KindOfArray) || value.is(KindOfObject)) {
        Array arr = value.toArray();
        curl_httppost *first = NULL;
        curl_httppost *last  = NULL;
        for (ArrayIter iter(arr); iter; ++iter) {
          String key = iter.first().toString();
          String val = iter.second().toString();
          const char *postval = val.data();

          /* The arguments after _NAMELENGTH and _CONTENTSLENGTH
           * must be explicitly cast to long in curl_formadd
           * use since curl needs a long not an int. */
          if (*postval == '@') {
            ++postval;
            m_error_no = (CURLcode)curl_formadd
              (&first, &last,
               CURLFORM_COPYNAME, key.data(),
               CURLFORM_NAMELENGTH, (long)key.size(),
               CURLFORM_FILE, postval,
               CURLFORM_END);
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
        curl_slist *slist = NULL;
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
        curl_easy_setopt(m_cp, CURLOPT_DEBUGFUNCTION, NULL);
        curl_easy_setopt(m_cp, CURLOPT_DEBUGDATA, NULL);
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

  Variant do_callback(CVarRef cb, CArrRef args) {
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
          t->callback, make_packed_array(Resource(ch), t->fp->fd(), data_size));
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
    if (m_cp == NULL && !nullOkay) {
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

private:
  CURL *m_cp;
  void *m_exception;

  char m_error_str[CURL_ERROR_SIZE + 1];
  CURLcode m_error_no;

  ToFreePtr m_to_free;

  String m_url;
  String m_header;
  Array  m_opts;

  WriteHandler m_write;
  WriteHandler m_write_header;
  ReadHandler  m_read;

  bool m_phpException;
  bool m_emptyPost;

  static CURLcode ssl_ctx_callback(CURL *curl, void *sslctx, void *parm);
  typedef enum {
    CURLOPT_FB_TLS_VER_MAX = 2147482624,
    CURLOPT_FB_TLS_VER_MAX_NONE = 2147482625,
    CURLOPT_FB_TLS_VER_MAX_1_1 = 2147482626,
    CURLOPT_FB_TLS_VER_MAX_1_0 = 2147482627,
    CURLOPT_FB_TLS_CIPHER_SPEC = 2147482628
  } fb_specific_options;
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

#define CHECK_RESOURCE(curl)                                            \
  CurlResource *curl = ch.getTyped<CurlResource>(true, true);           \
  if (curl == NULL) {                                                   \
    raise_warning("supplied argument is not a valid cURL handle resource"); \
    return false;                                                       \
  }                                                                     \

Variant f_curl_init(const String& url /* = null_string */) {
  return NEWOBJ(CurlResource)(url);
}

Variant f_curl_copy_handle(CResRef ch) {
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

Variant f_curl_version(int uversion /* = k_CURLVERSION_NOW */) {
  curl_version_info_data *d = curl_version_info((CURLversion)uversion);
  if (d == NULL) {
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
  while (*p != NULL) {
    protocol_list.append(String(*p++, CopyString));
  }
  ret.set(s_protocols, protocol_list);
  return ret.create();
}

bool f_curl_setopt(CResRef ch, int option, CVarRef value) {
  CHECK_RESOURCE(curl);
  return curl->setOption(option, value);
}

bool f_curl_setopt_array(CResRef ch, CArrRef options) {
  CHECK_RESOURCE(curl);
  for (ArrayIter iter(options); iter; ++iter) {
    if (!curl->setOption(iter.first().toInt32(), iter.second())) {
      return false;
    }
  }
  return true;
}

Variant f_fb_curl_getopt(CResRef ch, int opt /* = 0 */) {
  CHECK_RESOURCE(curl);
  return curl->getOption(opt);
}

Variant f_curl_exec(CResRef ch) {
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

Variant f_curl_getinfo(CResRef ch, int opt /* = 0 */) {
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
      if (s_code != NULL) {
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
    char *s_code = NULL;
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

Variant f_curl_errno(CResRef ch) {
  CHECK_RESOURCE(curl);
  return curl->getError();
}

Variant f_curl_error(CResRef ch) {
  CHECK_RESOURCE(curl);
  return curl->getErrorString();
}

Variant f_curl_close(CResRef ch) {
  CHECK_RESOURCE(curl);
  curl->close();
  return uninit_null();
}

///////////////////////////////////////////////////////////////////////////////

class CurlMultiResource : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(CurlMultiResource)

  CLASSNAME_IS("cURL Multi Handle")
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
      m_multi = NULL;
    }
  }

  void add(CResRef ch) {
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
    if (m_multi == NULL) {
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

#define CHECK_MULTI_RESOURCE(curlm)                                     \
  CurlMultiResource *curlm = mh.getTyped<CurlMultiResource>(true, true); \
  if (curlm == NULL) {                                                  \
    raise_warning("expects parameter 1 to be cURL multi resource");     \
    return uninit_null();                                                        \
  }                                                                     \

Resource f_curl_multi_init() {
  return NEWOBJ(CurlMultiResource)();
}

Variant f_curl_multi_add_handle(CResRef mh, CResRef ch) {
  CHECK_MULTI_RESOURCE(curlm);
  CurlResource *curle = ch.getTyped<CurlResource>();
  curlm->add(ch);
  return curl_multi_add_handle(curlm->get(), curle->get());
}

Variant f_curl_multi_remove_handle(CResRef mh, CResRef ch) {
  CHECK_MULTI_RESOURCE(curlm);
  CurlResource *curle = ch.getTyped<CurlResource>();
  curlm->remove(curle);
  return curl_multi_remove_handle(curlm->get(), curle->get());
}

Variant f_curl_multi_exec(CResRef mh, VRefParam still_running) {
  CHECK_MULTI_RESOURCE(curlm);
  int running = still_running;
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
#  define curl_multi_select(mh, tm, ret) curl_multi_wait((mh), NULL, 0, (tm), (ret))
# else
#  define curl_multi_select hphp_curl_multi_select
# endif
#endif

Variant f_curl_multi_select(CResRef mh, double timeout /* = 1.0 */) {
  CHECK_MULTI_RESOURCE(curlm);
  int ret;
  unsigned long timeout_ms = (unsigned long)(timeout * 1000.0);
  IOStatusHelper io("curl_multi_select");
  curl_multi_select(curlm->get(), timeout_ms, &ret);
  return ret;
}

Variant f_curl_multi_getcontent(CResRef ch) {
  CHECK_RESOURCE(curl);
  return curl->getContents();
}

Array f_curl_convert_fd_to_stream(fd_set *fd, int max_fd) {
  Array ret = Array::Create();
  for (int i=0; i<=max_fd; i++) {
    if (FD_ISSET(i, fd)) {
      BuiltinFile *file = NEWOBJ(BuiltinFile)(i);
      ret.append(file);
    }
  }
  return ret;
}

Variant f_fb_curl_multi_fdset(CResRef mh,
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
  read_fd_set = f_curl_convert_fd_to_stream(&read_set, max);
  write_fd_set = f_curl_convert_fd_to_stream(&write_set, max);
  exc_fd_set = f_curl_convert_fd_to_stream(&exc_set, max);
  max_fd = max;

  return r;
}

const StaticString
  s_msg("msg"),
  s_result("result"),
  s_handle("handle"),
  s_headers("headers"),
  s_requests("requests");

Variant f_curl_multi_info_read(CResRef mh,
                               VRefParam msgs_in_queue /* = null */) {
  CHECK_MULTI_RESOURCE(curlm);

  int queued_msgs;
  CURLMsg *tmp_msg = curl_multi_info_read(curlm->get(), &queued_msgs);
  curlm->check_exceptions();
  if (tmp_msg == NULL) {
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

Variant f_curl_multi_close(CResRef mh) {
  CHECK_MULTI_RESOURCE(curlm);
  curlm->close();
  return uninit_null();
}

///////////////////////////////////////////////////////////////////////////////
// evhttp functions

class LibEventHttpHandle : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(LibEventHttpHandle)

  CLASSNAME_IS("LibEventHttp");
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  explicit LibEventHttpHandle(LibEventHttpClientPtr client) : m_client(client) {
  }

  ~LibEventHttpHandle() {
    if (m_client) {
      m_client->release();
    }
  }

  LibEventHttpClientPtr m_client;
};
IMPLEMENT_OBJECT_ALLOCATION(LibEventHttpHandle)

static LibEventHttpClientPtr prepare_client
(const String& url, const String& data, CArrRef headers, int timeout,
 bool async, bool post) {
  string sUrl = url.data();
  if (sUrl.size() < 7 || sUrl.substr(0, 7) != "http://") {
    raise_warning("Invalid URL: %s", sUrl.c_str());
    return LibEventHttpClientPtr();
  }

  // parsing server address
  size_t pos = sUrl.find('/', 7);
  string path;
  if (pos == string::npos) {
    pos = sUrl.length();
    path = "/";
  } else if (pos == 7) {
    raise_warning("Invalid URL: %s", sUrl.c_str());
    return LibEventHttpClientPtr();
  } else {
    path = sUrl.substr(pos);
  }
  string address = sUrl.substr(7, pos - 7);

  // parsing server port
  pos = address.find(':');
  int port = 80;
  if (pos != string::npos) {
    if (pos < address.length() - 1) {
      string sport = address.substr(pos + 1, address.length() - pos - 1);
      port = atoi(sport.c_str());
    }
    address = address.substr(0, pos);
  }

  LibEventHttpClientPtr client = LibEventHttpClient::Get(address, port);
  if (!client) {
    return client;
  }

  vector<string> sheaders;
  for (ArrayIter iter(headers); iter; ++iter) {
    sheaders.push_back(iter.second().toString().data());
  }
  if (!client->send(path.c_str(), sheaders, timeout, async,
                    post ? (void*)data.data() : NULL,
                    post ? data.size() : 0)) {
    return LibEventHttpClientPtr();
  }
  return client;
}

const StaticString
  s_code("code"),
  s_response("response");

static Array prepare_response(LibEventHttpClientPtr client) {
  int len = 0;
  char *res = client->recv(len); // block on return

  ArrayInit ret(4);
  ret.set(s_code, client->getCode());
  ret.set(s_response, String(res, len, AttachString));

  Array headers = Array::Create();
  const vector<string> &responseHeaders = client->getResponseHeaders();
  for (unsigned int i = 0; i < responseHeaders.size(); i++) {
    headers.append(String(responseHeaders[i]));
  }
  ret.set(s_headers, headers);
  ret.set(s_requests, client->getRequests());
  return ret.create();
}

///////////////////////////////////////////////////////////////////////////////

void f_evhttp_set_cache(const String& address, int max_conn,
                        int port /* = 80 */) {
  if (RuntimeOption::ServerHttpSafeMode) {
    throw_fatal("evhttp_set_cache is disabled");
  }
  LibEventHttpClient::SetCache(address.data(), port, max_conn);
}

Variant f_evhttp_get(const String& url, CArrRef headers /* = null_array */,
                     int timeout /* = 5 */) {
  if (RuntimeOption::ServerHttpSafeMode) {
    throw_fatal("evhttp_set_cache is disabled");
  }
  LibEventHttpClientPtr client = prepare_client(url, "", headers, timeout,
                                                false, false);
  if (client) {
    Variant ret = prepare_response(client);
    client->release();
    return ret;
  }
  return false;
}

Variant f_evhttp_post(const String& url, const String& data,
                      CArrRef headers /* = null_array */,
                      int timeout /* = 5 */) {
  if (RuntimeOption::ServerHttpSafeMode) {
    throw_fatal("evhttp_post is disabled");
  }
  LibEventHttpClientPtr client = prepare_client(url, data, headers, timeout,
                                                false, true);
  if (client) {
    Variant ret = prepare_response(client);
    client->release();
    return ret;
  }
  return false;
}

Variant f_evhttp_async_get(const String& url,
                           CArrRef headers /* = null_array */,
                           int timeout /* = 5 */) {
  if (RuntimeOption::ServerHttpSafeMode) {
    throw_fatal("evhttp_async_get is disabled");
  }
  LibEventHttpClientPtr client = prepare_client(url, "", headers, timeout,
                                                true, false);
  if (client) {
    return Resource(NEWOBJ(LibEventHttpHandle)(client));
  }
  return false;
}

Variant f_evhttp_async_post(const String& url, const String& data,
                            CArrRef headers /* = null_array */,
                            int timeout /* = 5 */) {
  if (RuntimeOption::ServerHttpSafeMode) {
    throw_fatal("evhttp_async_post is disabled");
  }
  LibEventHttpClientPtr client = prepare_client(url, data, headers, timeout,
                                                true, true);
  if (client) {
    return Resource(NEWOBJ(LibEventHttpHandle)(client));
  }
  return false;
}

Variant f_evhttp_recv(CResRef handle) {
  if (RuntimeOption::ServerHttpSafeMode) {
    throw_fatal("evhttp_recv is disabled");
  }
  LibEventHttpHandle *obj = handle.getTyped<LibEventHttpHandle>();
  if (obj->m_client) {
    return prepare_response(obj->m_client);
  }
  return false;
}

#if LIBCURL_VERSION_NUM >= 0x071500
const int64_t k_CURLINFO_LOCAL_PORT = CURLINFO_LOCAL_PORT;
#else
const int64_t k_CURLINFO_LOCAL_PORT = CURLINFO_NONE;
#endif

#if LIBCURL_VERSION_NUM >= 0x071002
const int64_t k_CURLOPT_TIMEOUT_MS = CURLOPT_TIMEOUT_MS;
const int64_t k_CURLOPT_CONNECTTIMEOUT_MS = CURLOPT_CONNECTTIMEOUT_MS;
#else
const int64_t k_CURLOPT_TIMEOUT_MS = CURLOPT_LASTENTRY;
const int64_t k_CURLOPT_CONNECTTIMEOUT_MS = CURLOPT_LASTENTRY;
#endif

///////////////////////////////////////////////////////////////////////////////
}

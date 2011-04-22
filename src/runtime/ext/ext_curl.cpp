/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/ext_curl.h>
#include <runtime/ext/ext_function.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/util/libevent_http_client.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/server_stats.h>

using namespace std;

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
///////////////////////////////////////////////////////////////////////////////
// helper data structure

class CurlResource : public SweepableResourceData {
private:
  DECLARE_OBJECT_ALLOCATION(CurlResource)

  class WriteHandler {
  public:
    WriteHandler() : method(0), type(0) {}

    int                method;
    Variant            callback;
    SmartObject<File>  fp;
    StringBuffer       buf;
    String             content;
    int                type;
  };

  class ReadHandler {
  public:
    ReadHandler() : method(0) {}

    int                method;
    Variant            callback;
    SmartObject<File>  fp;
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
  static StaticString s_class_name;
  // overriding ResourceData
  virtual CStrRef o_getClassName() const { return s_class_name; }

  CurlResource(CStrRef url) : m_emptyPost(true) {
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

  CurlResource(CurlResource *src) {
    ASSERT(src && src != this);
    m_cp = curl_easy_duphandle(src->get());

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

  void close() {
    if (m_cp) {
      curl_easy_cleanup(m_cp);
      m_cp = NULL;
    }
    m_to_free.reset();
  }

  Variant execute() {
    if (m_cp == NULL) {
      return false;
    }
    if (m_emptyPost) {
      // As per curl docs, an empty post must set POSTFIELDSIZE to be 0 or
      // the reader function will be called
      curl_easy_setopt(m_cp, CURLOPT_POSTFIELDSIZE, 0);
    }
    m_write.buf.reset();
    m_write.content.clear();
    m_header.clear();
    memset(m_error_str, 0, sizeof(m_error_str));

    {
      IOStatusHelper io("curl_easy_perform", m_url.data());
      m_error_no = curl_easy_perform(m_cp);
    }
    set_curl_statuses(m_cp, m_url.data());

    /* CURLE_PARTIAL_FILE is returned by HEAD requests */
    if (m_error_no != CURLE_OK && m_error_no != CURLE_PARTIAL_FILE) {
      m_write.buf.reset();
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
        if (!value.is(KindOfObject)) {
          return false;
        }

        Object obj = value.toObject();
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

    default:
      throw_invalid_argument("option: %d", option);
      break;
    }

    return m_error_no == CURLE_OK;
  }

  static int curl_debug(CURL *cp, curl_infotype type, char *buf,
                        size_t buf_len, void *ctx) {
    CurlResource *ch = (CurlResource *)ctx;
    if (type == CURLINFO_HEADER_OUT && buf_len > 0) {
      ch->m_header = String(buf, buf_len, CopyString);
    }
    return 0;
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
        Variant ret = f_call_user_func_array
          (t->callback, CREATE_VECTOR3(Object(ch), t->fp->fd(), data_size));
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
      echo(String(data, length, AttachLiteral));
      break;
    case PHP_CURL_FILE:
      return t->fp->write(String(data, length, AttachLiteral), length);
    case PHP_CURL_RETURN:
      if (length > 0) {
        t->buf.append(data, (int)length);
      }
      break;
    case PHP_CURL_USER:
      {
        Variant ret = f_call_user_func_array
          (t->callback,
           CREATE_VECTOR2(Object(ch), String(data, length, CopyString)));
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
        echo(String(data, length, AttachLiteral));
      }
      break;
    case PHP_CURL_FILE:
      return t->fp->write(String(data, length, AttachLiteral), length);
    case PHP_CURL_USER:
      {
        Variant ret = f_call_user_func_array
          (t->callback,
           CREATE_VECTOR2(Object(ch), String(data, length, CopyString)));
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

  char m_error_str[CURL_ERROR_SIZE + 1];
  CURLcode m_error_no;

  ToFreePtr m_to_free;

  String m_url;
  String m_header;

  WriteHandler m_write;
  WriteHandler m_write_header;
  ReadHandler  m_read;

  bool m_emptyPost;
};
IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(CurlResource);
void CurlResource::sweep() {
  m_write.buf.release();
  m_write_header.buf.release();
  close();
}

StaticString CurlResource::s_class_name("cURL handle");

///////////////////////////////////////////////////////////////////////////////

#define CHECK_RESOURCE(curl)                                            \
  CurlResource *curl = ch.getTyped<CurlResource>(true, true);           \
  if (curl == NULL) {                                                   \
    raise_warning("supplied argument is not a valid cURL handle resource"); \
    return false;                                                       \
  }                                                                     \

Variant f_curl_init(CStrRef url /* = null_string */) {
  return NEWOBJ(CurlResource)(url);
}

Variant f_curl_copy_handle(CObjRef ch) {
  CHECK_RESOURCE(curl);
  return NEWOBJ(CurlResource)(curl);
}

Variant f_curl_version(int uversion /* = k_CURLVERSION_NOW */) {
  curl_version_info_data *d = curl_version_info((CURLversion)uversion);
  if (d == NULL) {
    return false;
  }

  Array ret;
  ret.set("version_number",     (int)d->version_num);
  ret.set("age",                d->age);
  ret.set("features",           d->features);
  ret.set("ssl_version_number", d->ssl_version_num);
  ret.set("version",            d->version);
  ret.set("host",               d->host);
  ret.set("ssl_version",        d->ssl_version);
  ret.set("libz_version",       d->libz_version);

  // Add an array of protocols
  char **p = (char **) d->protocols;
  Array protocol_list;
  while (*p != NULL) {
    protocol_list.append(String(*p++, CopyString));
  }
  ret.set("protocols", protocol_list);

  return ret;
}

bool f_curl_setopt(CObjRef ch, int option, CVarRef value) {
  CHECK_RESOURCE(curl);
  return curl->setOption(option, value);
}

bool f_curl_setopt_array(CObjRef ch, CArrRef options) {
  CHECK_RESOURCE(curl);
  for (ArrayIter iter(options); iter; ++iter) {
    if (!curl->setOption(iter.first().toInt32(), iter.second())) {
      return false;
    }
  }
  return true;
}

Variant f_curl_exec(CObjRef ch) {
  CHECK_RESOURCE(curl);
  return curl->execute();
}

Variant f_curl_getinfo(CObjRef ch, int opt /* = 0 */) {
  CHECK_RESOURCE(curl);
  CURL *cp = curl->get();

  if (opt == 0) {
    char   *s_code;
    long    l_code;
    double  d_code;

    Array ret;
    if (curl_easy_getinfo(cp, CURLINFO_EFFECTIVE_URL, &s_code) == CURLE_OK) {
      ret.set("url", String(s_code, CopyString));
    }
    if (curl_easy_getinfo(cp, CURLINFO_CONTENT_TYPE, &s_code) == CURLE_OK) {
      if (s_code != NULL) {
        ret.set("content_type", String(s_code, CopyString));
      } else {
        ret.set("content_type", null);
      }
    }
    if (curl_easy_getinfo(cp, CURLINFO_HTTP_CODE, &l_code) == CURLE_OK) {
      ret.set("http_code", l_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_HEADER_SIZE, &l_code) == CURLE_OK) {
      ret.set("header_size", l_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_REQUEST_SIZE, &l_code) == CURLE_OK) {
      ret.set("request_size", l_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_FILETIME, &l_code) == CURLE_OK) {
      ret.set("filetime", l_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_SSL_VERIFYRESULT, &l_code) ==
        CURLE_OK) {
      ret.set("ssl_verify_result", l_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_REDIRECT_COUNT, &l_code) == CURLE_OK) {
      ret.set("redirect_count", l_code);
    }
#if LIBCURL_VERSION_NUM >= 0x071500
    if (curl_easy_getinfo(cp, CURLINFO_LOCAL_PORT, &l_code) == CURLE_OK) {
      ret.set("local_port", l_code);
    }
#endif
    if (curl_easy_getinfo(cp, CURLINFO_TOTAL_TIME, &d_code) == CURLE_OK) {
      ret.set("total_time", d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_NAMELOOKUP_TIME, &d_code) == CURLE_OK) {
      ret.set("namelookup_time", d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_CONNECT_TIME, &d_code) == CURLE_OK) {
      ret.set("connect_time", d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_PRETRANSFER_TIME, &d_code) ==
        CURLE_OK) {
      ret.set("pretransfer_time", d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_SIZE_UPLOAD, &d_code) == CURLE_OK) {
      ret.set("size_upload", d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_SIZE_DOWNLOAD, &d_code) == CURLE_OK) {
      ret.set("size_download", d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_SPEED_DOWNLOAD, &d_code) == CURLE_OK) {
      ret.set("speed_download", d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_SPEED_UPLOAD, &d_code) == CURLE_OK) {
      ret.set("speed_upload", d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &d_code) ==
        CURLE_OK) {
      ret.set("download_content_length", d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_CONTENT_LENGTH_UPLOAD, &d_code) ==
        CURLE_OK) {
      ret.set("upload_content_length", d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_STARTTRANSFER_TIME, &d_code) ==
        CURLE_OK) {
      ret.set("starttransfer_time", d_code);
    }
    if (curl_easy_getinfo(cp, CURLINFO_REDIRECT_TIME, &d_code) == CURLE_OK) {
      ret.set("redirect_time", d_code);
    }
    String header = curl->getHeader();
    if (!header.empty()) {
      ret.set("request_header", header);
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

  return null;
}

Variant f_curl_errno(CObjRef ch) {
  CHECK_RESOURCE(curl);
  return curl->getError();
}

Variant f_curl_error(CObjRef ch) {
  CHECK_RESOURCE(curl);
  return curl->getErrorString();
}

Variant f_curl_close(CObjRef ch) {
  CHECK_RESOURCE(curl);
  curl->close();
  return null;
}

///////////////////////////////////////////////////////////////////////////////

class CurlMultiResource : public SweepableResourceData {
public:
  DECLARE_OBJECT_ALLOCATION(CurlMultiResource)

  static StaticString s_class_name;
  // overriding ResourceData
  CStrRef o_getClassName() const { return s_class_name; }

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

  void add(CObjRef ch) {
    m_easyh.append(ch);
  }

  void remove(CurlResource *curle) {
    for (ArrayIter iter(m_easyh); iter; ++iter) {
      if (toObject(iter.second()).getTyped<CurlResource>()->get(true) ==
          curle->get()) {
        m_easyh.remove(iter.first());
        return;
      }
    }
  }

  Object find(CURL *cp) {
    for (ArrayIter iter(m_easyh); iter; ++iter) {
      if (toObject(iter.second()).getTyped<CurlResource>()->get(true) == cp) {
        return iter.second();
      }
    }
    return Object();
  }

  CURLM *get() {
    if (m_multi == NULL) {
      throw NullPointerException();
    }
    return m_multi;
  }

private:
  int m_still_running;
  CURLM *m_multi;
  Array m_easyh;
};
IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(CurlMultiResource);
void CurlMultiResource::sweep() {
  if (m_multi) {
    curl_multi_cleanup(m_multi);
  }
}

StaticString CurlMultiResource::s_class_name("cURL Multi Handle");

///////////////////////////////////////////////////////////////////////////////

#define CHECK_MULTI_RESOURCE(curlm)                                     \
  CurlMultiResource *curlm = mh.getTyped<CurlMultiResource>(true, true); \
  if (curlm == NULL) {                                                  \
    raise_warning("expects parameter 1 to be cURL multi resource");     \
    return null;                                                        \
  }                                                                     \

Object f_curl_multi_init() {
  return NEWOBJ(CurlMultiResource)();
}

Variant f_curl_multi_add_handle(CObjRef mh, CObjRef ch) {
  CHECK_MULTI_RESOURCE(curlm);
  CurlResource *curle = ch.getTyped<CurlResource>();
  curlm->add(ch);
  return curl_multi_add_handle(curlm->get(), curle->get());
}

Variant f_curl_multi_remove_handle(CObjRef mh, CObjRef ch) {
  CHECK_MULTI_RESOURCE(curlm);
  CurlResource *curle = ch.getTyped<CurlResource>();
  curlm->remove(curle);
  return curl_multi_remove_handle(curlm->get(), curle->get());
}

Variant f_curl_multi_exec(CObjRef mh, VRefParam still_running) {
  CHECK_MULTI_RESOURCE(curlm);
  int running = still_running;
  IOStatusHelper io("curl_multi_exec");
  int result = curl_multi_perform(curlm->get(), &running);
  still_running = running;
  return result;
}

Variant f_curl_multi_select(CObjRef mh, double timeout /* = 1.0 */) {
  CHECK_MULTI_RESOURCE(curlm);
  int ret;
  unsigned long timeout_ms = (unsigned long)(timeout * 1000.0);
  IOStatusHelper io("curl_multi_select");
  curl_multi_select(curlm->get(), timeout_ms, &ret);
  return ret;
}

Variant f_curl_multi_getcontent(CObjRef ch) {
  CHECK_RESOURCE(curl);
  return curl->getContents();
}

Variant f_curl_multi_info_read(CObjRef mh,
                               VRefParam msgs_in_queue /* = null */) {
  CHECK_MULTI_RESOURCE(curlm);

  int queued_msgs;
  CURLMsg *tmp_msg = curl_multi_info_read(curlm->get(), &queued_msgs);
  if (tmp_msg == NULL) {
    return false;
  }
  msgs_in_queue = queued_msgs;

  Array ret;
  ret.set("msg", tmp_msg->msg);
  ret.set("result", tmp_msg->data.result);
  Object curle = curlm->find(tmp_msg->easy_handle);
  if (!curle.isNull()) {
    ret.set("handle", curle);
  }
  return ret;
}

Variant f_curl_multi_close(CObjRef mh) {
  CHECK_MULTI_RESOURCE(curlm);
  curlm->close();
  return null;
}

///////////////////////////////////////////////////////////////////////////////
// evhttp functions

class LibEventHttpHandle : public SweepableResourceData {
public:
  DECLARE_OBJECT_ALLOCATION(LibEventHttpHandle)

  static StaticString s_class_name;
  // overriding ResourceData
  virtual CStrRef o_getClassName() const { return s_class_name; }

  LibEventHttpHandle(LibEventHttpClientPtr client) : m_client(client) {
  }

  ~LibEventHttpHandle() {
    if (m_client) {
      m_client->release();
    }
  }

  LibEventHttpClientPtr m_client;
};
IMPLEMENT_OBJECT_ALLOCATION(LibEventHttpHandle)

StaticString LibEventHttpHandle::s_class_name("LibEventHttp");

static LibEventHttpClientPtr prepare_client
(CStrRef url, CStrRef data, CArrRef headers, int timeout,
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

static Array prepare_response(LibEventHttpClientPtr client) {
  int len = 0;
  char *res = client->recv(len); // block on return

  Array ret = Array::Create();
  ret.set("code", client->getCode());
  ret.set("response", String(res, len, AttachString));

  Array headers = Array::Create();
  const vector<string> &responseHeaders = client->getResponseHeaders();
  for (unsigned int i = 0; i < responseHeaders.size(); i++) {
    headers.append(String(responseHeaders[i]));
  }
  ret.set("headers", headers);
  ret.set("requests", client->getRequests());
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

void f_evhttp_set_cache(CStrRef address, int max_conn, int port /* = 80 */) {
  LibEventHttpClient::SetCache(address.data(), port, max_conn);
}

Variant f_evhttp_get(CStrRef url, CArrRef headers /* = null_array */,
                     int timeout /* = 5 */) {
  LibEventHttpClientPtr client = prepare_client(url, "", headers, timeout,
                                                false, false);
  if (client) {
    Variant ret = prepare_response(client);
    client->release();
    return ret;
  }
  return false;
}

Variant f_evhttp_post(CStrRef url, CStrRef data,
                      CArrRef headers /* = null_array */,
                      int timeout /* = 5 */) {
  LibEventHttpClientPtr client = prepare_client(url, data, headers, timeout,
                                                false, true);
  if (client) {
    Variant ret = prepare_response(client);
    client->release();
    return ret;
  }
  return false;
}

Variant f_evhttp_async_get(CStrRef url, CArrRef headers /* = null_array */,
                           int timeout /* = 5 */) {
  LibEventHttpClientPtr client = prepare_client(url, "", headers, timeout,
                                                true, false);
  if (client) {
    return Object(NEWOBJ(LibEventHttpHandle)(client));
  }
  return false;
}

Variant f_evhttp_async_post(CStrRef url, CStrRef data,
                            CArrRef headers /* = null_array */,
                            int timeout /* = 5 */) {
  LibEventHttpClientPtr client = prepare_client(url, data, headers, timeout,
                                                true, true);
  if (client) {
    return Object(NEWOBJ(LibEventHttpHandle)(client));
  }
  return false;
}

Variant f_evhttp_recv(CObjRef handle) {
  LibEventHttpHandle *obj = handle.getTyped<LibEventHttpHandle>();
  if (obj->m_client) {
    return prepare_response(obj->m_client);
  }
  return false;
}

#if LIBCURL_VERSION_NUM >= 0x071500
const int64 k_CURLINFO_LOCAL_PORT = CURLINFO_LOCAL_PORT;
#else
const int64 k_CURLINFO_LOCAL_PORT = CURLINFO_NONE;
#endif

#if LIBCURL_VERSION_NUM >= 0x071002
const int64 k_CURLOPT_TIMEOUT_MS = CURLOPT_TIMEOUT_MS;
const int64 k_CURLOPT_CONNECTTIMEOUT_MS = CURLOPT_CONNECTTIMEOUT_MS;
#else
const int64 k_CURLOPT_TIMEOUT_MS = CURLOPT_LASTENTRY;
const int64 k_CURLOPT_CONNECTTIMEOUT_MS = CURLOPT_LASTENTRY;
#endif

///////////////////////////////////////////////////////////////////////////////
}

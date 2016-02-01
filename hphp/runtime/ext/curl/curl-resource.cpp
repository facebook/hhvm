#include "hphp/runtime/ext/curl/curl-resource.h"
#include "hphp/runtime/ext/curl/curl-pool.h"
#include "hphp/runtime/ext/curl/ext_curl.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/curl-tls-workarounds.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/vm/vm-regs.h"

#include <curl/easy.h>
#include <curl/multi.h>
#include <openssl/ssl.h>

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

CurlResource::CurlResource(const String& url,
                           CurlHandlePool *pool /*=nullptr */)
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
  m_to_free = req::make_shared<ToFree>();

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
    char *urlcopy = req::strndup(url.data(), url.size());
    curl_easy_setopt(m_cp, CURLOPT_URL, urlcopy);
    m_to_free->str.push_back(urlcopy);
#endif
  }
}

CurlResource::CurlResource(req::ptr<CurlResource> src)
: m_connPool(nullptr), m_pooledHandle(nullptr) {
  // NOTE: we never pool copied curl handles, because all spots in
  // the pool are pre-populated

  assertx(src && src != this);
  assertx(!src->m_exception);

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

void CurlResource::sweep() {
  m_write.buf.release();
  m_write_header.buf.release();
  closeForSweep();
}

void CurlResource::closeForSweep() {
  assertx(!m_exception);
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

Variant CurlResource::execute() {
  assertx(!m_exception);
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
  } else if (isFileOption(option)) {
    auto fp = dyn_cast_or_null<File>(value);
    if (!fp) return false;
    ret = setFileOption(option, fp);
  } else if (isStringListOption(option)) {
    ret = setStringListOption(option, value);
  } else if (isNonCurlOption(option)) {
    ret = setNonCurlOption(option, value);
  } else if (option == CURLOPT_POSTFIELDS) {
    ret = setPostFieldsOption(value);
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
    throw_invalid_argument("option: %ld", option);
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

CURL* CurlResource::get(bool nullOkay /*=false*/) {
  if (m_cp == nullptr && !nullOkay) {
    throw_null_pointer_exception();
  }
  return m_cp;
}

bool CurlResource::isLongOption(long option) {
  switch (option) {
    case CURLOPT_TIMEOUT:
    case CURLOPT_TIMEOUT_MS:
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

bool CurlResource::isStringOption(long option) {
  switch (option) {
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
      return true;
    default:
      return false;
  }
}

bool CurlResource::setStringOption(long option, const String& value) {
  assertx(isStringOption(option));
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
        && var_val.toObject()->instanceof(SystemLib::s_CURLFileClass))) {
      Object val = var_val.toObject();

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

      if (*postval == '@') {
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
  return (option == CURLOPT_HTTPHEADER) ||
         (option == CURLOPT_QUOTE) ||
         (option == CURLOPT_HTTP200ALIASES) ||
         (option == CURLOPT_POSTQUOTE) ||
         (option == CURLOPT_RESOLVE);
}

bool CurlResource::setStringListOption(long option, const Variant& value) {
  if (!value.isArray() && !value.is(KindOfObject)) {
    raise_warning("You must pass either an object or an array with "
                  "the CURLOPT_HTTPHEADER, CURLOPT_QUOTE, "
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
         (option == CURLOPT_FB_TLS_CIPHER_SPEC);
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
  }
  return false;
}

inline int64_t minTimeoutImpl(int64_t timeout, int64_t multiple) {
  auto info = ThreadInfo::s_threadInfo.getNoCheck();
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

Variant CurlResource::do_callback(const Variant& cb, const Array& args) {
  assertx(!m_exception);
  try {
    return vm_call_user_func(cb, args);
  } catch (const Object &e) {
    m_exception.assign(e);
  } catch (Exception &e) {
    m_exception.assign(e.clone());
  }
  return init_null();
}

size_t CurlResource::curl_read(char *data,
                               size_t size, size_t nmemb, void *ctx) {
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
    case PHP_CURL_USER: {
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

size_t CurlResource::curl_write(char *data,
                                size_t size, size_t nmemb, void *ctx) {
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
    case PHP_CURL_USER: {
      Variant ret = ch->do_callback(
        t->callback,
        make_packed_array(Resource(ch), String(data, length, CopyString)));
      length = ret.toInt64();
      break;
    }
  }

  return length;
}

size_t CurlResource::curl_write_header(char *data,
                                       size_t size, size_t nmemb, void *ctx) {
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
    case PHP_CURL_USER: {
        Variant ret = ch->do_callback(
          t->callback,
          make_packed_array(Resource(ch), String(data, length, CopyString)));
        length = ret.toInt64();
      break;
    }
    case PHP_CURL_IGNORE:
      return length;
    default:
      return (size_t)-1;
  }

  return length;
}

int CurlResource::curl_debug(CURL *cp, curl_infotype type, char *buf,
                             size_t buf_len, void *ctx) {
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

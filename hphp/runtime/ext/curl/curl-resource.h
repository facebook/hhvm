#ifndef incl_HPHP_CURL_RESOURCE_H
#define incl_HPHP_CURL_RESOURCE_H

#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/ext/extension.h"

#include <curl/curl.h>

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////
// CurlResource

struct CurlHandlePool;
struct PooledCurlHandle;

struct CurlResource : SweepableResourceData {
  using ExceptionType = folly::Optional<boost::variant<Object,Exception*>>;

  struct WriteHandler {
    int                method{0};
    Variant            callback;
    req::ptr<File>     fp;
    StringBuffer       buf;
    String             content;
    int                type{0};
  };

  struct ReadHandler {
    int                method{0};
    Variant            callback;
    req::ptr<File>     fp;
  };

  struct ToFree {
    ~ToFree();
    std::vector<char*>          str;
    std::vector<curl_httppost*> post;
    std::vector<curl_slist*>    slist;
  };

  using fb_specific_options = enum {
    CURLOPT_FB_TLS_VER_MAX = 2147482624,
    CURLOPT_FB_TLS_VER_MAX_NONE = 2147482625,
    CURLOPT_FB_TLS_VER_MAX_1_1 = 2147482626,
    CURLOPT_FB_TLS_VER_MAX_1_0 = 2147482627,
    CURLOPT_FB_TLS_CIPHER_SPEC = 2147482628
  };

  CLASSNAME_IS("curl")
  const String& o_getClassNameHook() const override { return classnameof(); }
  DECLARE_RESOURCE_ALLOCATION(CurlResource)
  bool isInvalid() const override { return !m_cp; }

  explicit CurlResource(const String& url, CurlHandlePool *pool = nullptr);
  explicit CurlResource(req::ptr<CurlResource> src);
  ~CurlResource() { close(); }

  void closeForSweep();
  void close() {
    closeForSweep();
    m_opts.clear();
    m_to_free.reset();
  }
  void reseat();
  void reset();

  Variant execute();
  String getUrl() { return m_url; }
  String getHeader() { return m_header; }
  String getContents();

  bool setOption(long option, const Variant& value);
  Variant getOption(long option);

  int getError() { return m_error_no; }
  String getErrorString() { return String(m_error_str, CopyString); }

  CURL *get(bool nullOkay = false);

  void check_exception();
  ExceptionType getAndClearException() { return std::move(m_exception); }
  static bool isPhpException(const ExceptionType& e) {
    return e && boost::get<Object>(&e.value()) != nullptr;
  }
  static Object getPhpException(const ExceptionType& e) {
    assertx(e && isPhpException(e));
    return boost::get<Object>(*e);
  }
  static Exception* getCppException(const ExceptionType& e) {
    assertx(e && !isPhpException(e));
    return boost::get<Exception*>(*e);
  }

 private:
  static int64_t minTimeout(int64_t timeout);
  static int64_t minTimeoutMS(int64_t timeout);

  static bool isLongOption(long option);
  bool setLongOption(long option, long value);
  static bool isStringOption(long option);
  bool setStringOption(long option, const String& value);
  bool setPostFieldsOption(const Variant& value);
  static bool isFileOption(long option);
  bool setFileOption(long option, const req::ptr<File>& fp);
  static bool isStringListOption(long option);
  bool setStringListOption(long option, const Variant& value);
  static bool isNonCurlOption(long option);
  bool setNonCurlOption(long option, const Variant& value);

  Variant do_callback(const Variant& cb, const Array& args);
  static size_t curl_read(char *data, size_t size, size_t nmemb, void *ctx);
  static size_t curl_write(char *data, size_t size, size_t nmemb, void *ctx);
  static size_t curl_write_header(char *data,
                                  size_t size, size_t nmemb, void *ctx);
  static int curl_debug(CURL *cp, curl_infotype type, char *buf,
                        size_t buf_len, void *ctx);
  static int curl_progress(void* p,
                           double dltotal, double dlnow,
                           double ultotal, double ulnow);

  static CURLcode ssl_ctx_callback(CURL *curl, void *sslctx, void *parm);

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
};

/////////////////////////////////////////////////////////////////////////////
}
#endif

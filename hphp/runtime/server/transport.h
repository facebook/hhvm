/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_HTTP_SERVER_TRANSPORT_H_
#define incl_HPHP_HTTP_SERVER_TRANSPORT_H_

#include <list>
#include <string>
#include <unordered_map>
#include <utility>

#include "hphp/util/compression.h"
#include "hphp/util/functional.h"
#include "hphp/runtime/base/debuggable.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-holder.h"
#include "hphp/runtime/base/type-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Array;
struct Variant;

/**
 * For storing headers and cookies.
 */
template <typename V>
using CaseInsenMap =
  std::unordered_map<std::string, V, string_hashi, string_eqstri>;

using HeaderMap = CaseInsenMap<std::vector<std::string>>;
using CookieList = std::vector<std::pair<std::string, std::string>>;

/**
 * A class defining an interface that request handler can use to query
 * transport related information.
 *
 * Note that one transport object is created for each request, and
 * one transport is ONLY accessed from one single thread.
 */
class Transport : public IDebuggable {
public:
  enum class Method {
    Unknown,

    GET,
    POST,
    HEAD,
    AUTO, // check GET parameter first, then POST
  };

  // TODO: add all status codes
  // (http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html)
  enum class StatusCode {
    Unknown,

    // Success
    OK = 200,

    // Redirection
    MOVED_PERMANENTLY = 301,

    // Client Error
    BAD_REQUEST = 400,
    FORBIDDEN = 403,
    NOT_FOUND = 404,

    // Server Error
    INTERNAL_SERVER_ERROR = 500,
    SERVICE_UNAVAILABLE = 503,
  };

  enum class ThreadType {
    RequestThread,
    PageletThread,
    XboxThread,
    RpcThread,
  };

public:
  Transport();
  virtual ~Transport();

  void onRequestStart(const timespec &queueTime);
  const timespec &getQueueTime() const { return m_queueTime; }
  const timespec &getWallTime() const { return m_wallTime; }
  const timespec &getCpuTime() const { return m_cpuTime; }
  const int64_t &getInstructions() const { return m_instructions; }
  const int64_t &getSleepTime() const { return m_sleepTime; }
  void incSleepTime(unsigned int seconds) { m_sleepTime += seconds; }
  const int64_t &getuSleepTime() const { return m_usleepTime; }
  void incuSleepTime(unsigned int useconds) { m_usleepTime += useconds; }
  const int64_t &getnSleepTimeS() const { return m_nsleepTimeS; }
  const int32_t &getnSleepTimeN() const { return m_nsleepTimeN; }
  void incnSleepTime(unsigned long int seconds, unsigned int nseconds) {
    m_nsleepTimeN += nseconds;
    m_nsleepTimeS += seconds + (m_nsleepTimeN / 1000000000);
    m_nsleepTimeN %= 1000000000;
  }

  ///////////////////////////////////////////////////////////////////////////
  // Functions sub-classes have to implement.

  /**
   * Request URI.
   */
  virtual const char *getUrl() = 0;
  virtual const char *getRemoteHost() = 0;
  virtual uint16_t getRemotePort() = 0;
  // The transport can override REMOTE_ADDR if it has one
  virtual const char *getRemoteAddr() { return ""; }
  // The transport can override the virtualhosts' docroot
  virtual const std::string getDocumentRoot() { return ""; }
  // The transport can say exactly what script to use
  virtual const std::string getScriptFilename() { return ""; }
  virtual const std::string getPathTranslated() { return ""; }
  virtual const std::string getPathInfo() { return ""; }
  virtual bool isPathInfoSet() {return false; }

  /**
   * Server Headers
   */
  virtual const char *getServerName() {
    return "";
  };
  virtual const std::string& getServerAddr() {
    auto const& ipv4 = RuntimeOption::GetServerPrimaryIPv4();
    return ipv4.empty() ? RuntimeOption::GetServerPrimaryIPv6() : ipv4;
  };
  virtual uint16_t getServerPort() {
    return RuntimeOption::ServerPort;
  };
  virtual const char *getServerSoftware() {
    return "HPHP";
  };

  /**
   * POST request's data.
   */
  virtual const void *getPostData(int &size) = 0;
  virtual bool hasMorePostData() { return false; }
  virtual const void *getMorePostData(int &size) { size = 0; return nullptr; }
  virtual bool getFiles(std::string &files) { return false; }
  /**
   * Is this a GET, POST or anything?
   */
  virtual Method getMethod() = 0;
  virtual const char *getExtendedMethod() { return nullptr;}
  const char *getMethodName();

  /**
   * What version of HTTP was the request?
   */
  virtual std::string getHTTPVersion() const;

  /**
   * Get http request size.
   */
  virtual int getRequestSize() const;

  /**
   * Get request header(s).
   */
  virtual std::string getHeader(const char *name) = 0;
  virtual void getHeaders(HeaderMap &headers) = 0;
  virtual void getTransportParams(HeaderMap &serverParams) {};


  /**
   * Get/set response headers.
   */
  void addHeaderNoLock(const char *name, const char *value);
  void addHeader(const char *name, const char *value);
  void addHeader(const String& header);
  void replaceHeader(const char *name, const char *value);
  void replaceHeader(const String& header);
  void removeHeader(const char *name);
  void removeAllHeaders();
  void getResponseHeaders(HeaderMap &headers);
  std::string getFirstHeaderFile() const { return m_firstHeaderFile;}
  int getFirstHeaderLine() const { return m_firstHeaderLine;}

  /**
   * Content/MIME type related functions.
   */
  void setMimeType(const String& mimeType);
  String getMimeType();
  bool getUseDefaultContentType() { return m_sendContentType;}
  void setUseDefaultContentType(bool send) { m_sendContentType = send;}

  /**
   * Can we gzip response?
   */
  void enableCompression() { m_compression = true;}
  void disableCompression() { m_compression = false;}
  bool isCompressionEnabled() const {
    return m_compression && RuntimeOption::GzipCompressionLevel;
  }

  /**
   * Set cookie response header.
   */
  bool setCookie(const String& name, const String& value, int64_t expire = 0,
                 const String& path = "", const String& domain = "", bool secure = false,
                 bool httponly = false, bool encode_url = true);

  /**
   * Add/remove a response header.
   */
  virtual void addHeaderImpl(const char *name, const char *value) = 0;
  virtual void removeHeaderImpl(const char *name) = 0;

  /**
   * Add/remove a request header. Default is no-op, because not all transports
   * need to support incoming request header manipulations.
   */
  virtual void addRequestHeaderImpl(const char *name, const char *value) {}
  virtual void removeRequestHeaderImpl(const char *name) {}

  /**
   * Called when all sending should be done by this time point. Designed for
   * sending last chunk of response for chunked encoding.
   */
  void onSendEnd();

  /**
   * Send back a response with specified code.
   * Caller deletes data, callee must copy
   */
  virtual void sendImpl(const void *data, int size, int code,
                        bool chunked, bool eom) = 0;

  /**
   * Override to implement more send end logic.
   */
  virtual void onSendEndImpl() {}

  /**
   * Returns true if this transport supports server pushed resources
   */
  virtual bool supportsServerPush() { return false; }

  /**
   * Attempt to push the resource identified by host/path on this transport
   *
   * @param priority (3 bit priority, 0 = highest, 7 = lowest),
   * @param headers HTTP headers for this resource
   * @param body body bytes (optional)
   * @param size length of @p body or 0
   * @param eom true if no more body bytes are expected
   *
   * @return an ID that can be passed to pushResourceBody if more body
   *         is being streamed later.  0 indicates that the push failed
   *         immediately.
   */
  virtual int64_t pushResource(const char *host, const char *path,
                               uint8_t priority, const Array& headers,
                                const void *data, int size, bool eom) {
    return 0;
  };

  /**
   * Stream body and/or EOM marker for a pushed resource
   *
   * @param id ID returned by pushResource
   * @param data body bytes (optional if eom is true)
   * @param size length of @p body
   * @param eom true if no more body bytes are expected
   */
  virtual void pushResourceBody(int64_t id, const void *data, int size,
                                bool eom) {}

  /**
   * Need this implementation to break keep-alive connections.
   */
  virtual bool isServerStopping() { return false;}

  ///////////////////////////////////////////////////////////////////////////
  // Pre-implemented utitlity functions.

  /**
   * We define a "server object" as the part of URL without domain name:
   *
   *   http://facebook.com/foo?x=1       server object is "/foo?x=1"
   *   http://facebook.com/foo/bar?x=1   server object is "/foo/bar?x=1"
   */
  virtual const char *getServerObject();

  /**
   * We define a "command" as the part of URL without parameters:
   *
   *   /foo?x=1      command is "foo"
   *   foo?x=1       command is "foo"
   *   foo/bar?x=1   command is "foo/bar"
   *   /foo/bar?x=1  command is "foo/bar"
   */
  std::string getCommand();

  /**
   * Whether a parameter exists. Normally this is not needed to know, unless
   * "null" is different from an empty string or 0.
   */
  bool paramExists(const char *name, Method method = Method::GET);

  /**
   * Get value of a parameter. Returns empty string is not present.
   */
  std::string getParam(const char *name, Method method = Method::GET);

  /**
   * Turn a string parameter into an integer.
   */
  int getIntParam(const char *name, Method method = Method::GET);

  /**
   * Turn a string parameter into a 64-bit number.
   */
  long long getInt64Param(const char *name, Method method = Method::GET);

  /**
   * Collect multiple string parameters with the same name into "values".
   *
   *   /foo?x=1&x=2&x=3
   */
  void getArrayParam(const char *name, std::vector<std::string> &values,
                     Method method = Method::GET);

  /**
   * Split a string parameter into multiple sub-strings.
   *
   *  /foo?x=1:2:3
   */
  void getSplitParam(const char *name, std::vector<std::string> &values,
                     char delimiter, Method method = Method::GET);

  /**
   * Test whether client accepts a certain encoding.
   */
  bool acceptEncoding(const char *encoding);

  /**
   * Test whether cookie header has the "name=".
   */
  bool cookieExists(const char *name);

  /**
   * Get value of cookie "name"
   */
  std::string getCookie(const std::string &name);

  /**
   * Test whether client is okay to accept compressed response.
   */
  bool decideCompression();

  /**
   * Sending back a response.
   */
  void setResponse(int code, const char *info = nullptr);
  const std::string &getResponseInfo() const { return m_responseCodeInfo; }
  bool headersSent() { return m_headerSent;}
  bool setHeaderCallback(const Variant& callback);
  void sendRaw(void *data, int size, int code = 200,
               bool compressed = false, bool chunked = false,
               const char *codeInfo = nullptr);
private:
  void sendRawInternal(const void *data, int size, int code = 200,
                       bool compressed = false,
                       const char *codeInfo = nullptr);
public:
  void sendString(const char *data, int code = 200, bool compressed = false,
                  bool chunked = false,
                  const char * codeInfo = nullptr) {
    sendRaw((void*)data, strlen(data), code, compressed, chunked, codeInfo);
  }
  void sendString(const std::string &data, int code = 200,
                  bool compressed = false, bool chunked = false,
                  const char *codeInfo = nullptr) {
    sendRaw((void*)data.c_str(), data.length(), code, compressed, chunked,
            codeInfo);
  }
  void redirect(const char *location, int code, const char *info = nullptr);

  // TODO: support rfc1867
  virtual bool isUploadedFile(const String& filename);

  int getResponseSize() const { return m_responseSize; }
  int getResponseCode() const { return m_responseCode; }

  int getResponseTotalSize() const { return m_responseTotalSize; }
  int getResponseSentSize() const { return m_responseSentSize; }
  int64_t getFlushTime() const { return m_flushTimeUs; }
  int getLastChunkSentSize();
  void getChunkSentSizes(Array &ret);
  void onFlushBegin(int totalSize) { m_responseTotalSize = totalSize; }
  void onFlushProgress(int writtenSize, int64_t delayUs);
  void onChunkedProgress(int writtenSize);

  void setThreadType(ThreadType type) { m_threadType = type;}
  ThreadType getThreadType() const { return m_threadType;}
  const char *getThreadTypeName() const;

  // implementing IDebuggable
  virtual void debuggerInfo(InfoVec &info);

  void setSSL() {m_isSSL = true;}
  bool isSSL() const {return m_isSSL;}

protected:
  template <typename F> friend void scan(const Transport&, F&);

  /**
   * Parameter parsing in this class is done by making just one copy of the
   * entire query (either URL or post data), then insert termintaing NULLs
   * at end of tokens (name and value), url decode in-place and then store
   * token's start char * addresses in ParamMaps. Therefore, this entire
   * process is very efficient without excessive string copying.
   */
  typedef hphp_hash_map<const char*, std::vector<const char*>,
                        cstr_hash, eqstr> ParamMap;

  // timers
  timespec m_queueTime;
  timespec m_wallTime;
  timespec m_cpuTime;

  int64_t m_instructions;

  int64_t m_sleepTime;
  int64_t m_usleepTime;
  int64_t m_nsleepTimeS;
  int32_t m_nsleepTimeN;

  // input
  char *m_url;
  char *m_postData;
  bool m_postDataParsed;
  ParamMap m_getParams;
  ParamMap m_postParams;

  // output
  bool m_chunkedEncoding;
  bool m_headerSent;
  Cell m_headerCallback;
  bool m_headerCallbackDone;  // used to prevent infinite loops
  int m_responseCode;
  std::string m_responseCodeInfo;
  HeaderMap m_responseHeaders;
  bool m_firstHeaderSet;
  std::string m_firstHeaderFile;
  int m_firstHeaderLine;
  CookieList m_responseCookiesList;
  int m_responseSize;
  int m_responseTotalSize; // including added headers
  int m_responseSentSize;
  int64_t m_flushTimeUs;
  bool m_sendEnded;

  std::vector<int> m_chunksSentSizes;

  std::string m_mimeType;
  bool m_sendContentType;
  bool m_compression;
  StreamCompressor *m_compressor;

  bool m_isSSL;

  enum class CompressionDecision {
    NotDecidedYet,
    ShouldNot,
    Should,
    HasTo,
  };
  CompressionDecision m_compressionDecision;

  ThreadType m_threadType;

  // helpers
  void parseGetParams();
  void parsePostParams();
  static void parseQuery(char *query, ParamMap &params);
  static void urlUnescape(char *value);
  bool splitHeader(const String& header, String &name, const char *&value);
  std::list<std::string> getCookieLines();

  StringHolder prepareResponse(const void *data, int size, bool &compressed,
                               bool last);

private:
  void prepareHeaders(bool compressed, bool chunked,
    const StringHolder &response, const StringHolder& orig_response);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_SERVER_TRANSPORT_H_

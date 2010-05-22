/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __LIBEVENT_HTTP_CLIENT_H__
#define __LIBEVENT_HTTP_CLIENT_H__

#include <util/base.h>
#include <util/async_func.h>
#include <util/lock.h>
#include <evhttp.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Use evhttp as our HTTP client. This isn't the same as HttpClient that's CURL
 * based. HttpClient supports SSL and follows redirections, whereas this class
 * doesn't. But this class allows keep-alive connections to be pooled for
 * repetitively HTTP requests.
 */
DECLARE_BOOST_TYPES(LibEventHttpClient);
class LibEventHttpClient {
public:
  /**
   * Specify an address:port to be cached. Set to 0 to clear it.
   */
  static void SetCache(const std::string &address, int port,
                       int maxConnection);

  /**
   * Get an http client for the specified URL.
   */
  static LibEventHttpClientPtr Get(const std::string &address, int port);

private:
  static ReadWriteMutex ConnectionPoolMutex;
  static std::map<std::string, LibEventHttpClientPtrVec> ConnectionPool;

  /**
   * address:port => max connection to pool
   */
  static std::map<std::string, int> ConnectionPoolConfig;

  LibEventHttpClient(const std::string &address, int port);

public:
  ~LibEventHttpClient();

  /**
   * Done with this object, can release back to pool. Cannot access this object
   * afterwards, without calling Get() again.
   */
  void release();

  /**
   * Synchronously or asynchronously GET/POST an URL.
   * If data is NULL, do GET, otherwise, do POST.
   */
  bool send(const std::string &url, const std::vector<std::string> &headers,
            int timeoutSeconds, bool async, const void *data = NULL,
            int size = 0);

  /**
   * Block until last send() returns some data. Caller is in charge of free-ing
   * returned char*.
   */
  char *recv(int &len);

  int getRequests() const { return m_requests; }
  int getCode() const { return m_code; }
  const std::string &getCodeLine() const { return m_codeLine; }
  const std::vector<std::string> &getResponseHeaders() const {
    return m_responseHeaders;
  }

public:
  // libevent callbacks
  void onRequestCompleted(evhttp_request* request);
  void onConnectionClosed();

private:
  bool m_busy;               // telling connection pool this object is in use
  std::string m_address;     // server address
  unsigned short m_port;     // server port
  int m_requests;            // number of requests we've sent on this conn.

  event_base *m_eventBase;   // event base
  evhttp_connection *m_conn; // evhttp connection object
  event m_eventTimeout;      // for timeout purpose

  AsyncFunc<LibEventHttpClient> *m_thread; // for async GET/POST

  std::string m_url;         // most recent URL
  int m_code;                // response code
  std::string m_codeLine;    // human readable response code line
  char *m_response;          // final response buffer
  int m_len;                 // final response length
  std::vector<std::string> m_responseHeaders;

  void sendImpl();
  void clear();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __LIBEVENT_HTTP_CLIENT_H__

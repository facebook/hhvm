/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_WARMUP_REQUEST_HANDLER_H_
#define incl_HPHP_WARMUP_REQUEST_HANDLER_H_

#include "hphp/runtime/server/server.h"
#include "hphp/runtime/server/http-request-handler.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(WarmupRequestHandlerFactory);

/**
 * WarmupRequestHandler is a small shim on top of HttpRequestHandler.
 * It counts the number of requests, and adds additional worker threads to the
 * server after a specified threshold.
 */
class WarmupRequestHandler : public RequestHandler {
public:
  explicit WarmupRequestHandler(int timeout,
                                const WarmupRequestHandlerFactoryPtr& factory)
    : RequestHandler(timeout), m_factory(factory), m_reqHandler(timeout) {}

  virtual void handleRequest(Transport *transport);
  virtual void abortRequest(Transport *transport);

private:
  WarmupRequestHandlerFactoryPtr m_factory;
  HttpRequestHandler m_reqHandler;
};

class WarmupRequestHandlerFactory :
  public std::enable_shared_from_this<WarmupRequestHandlerFactory> {
public:
  WarmupRequestHandlerFactory(ServerPtr server,
                              uint32_t additionalThreads,
                              uint32_t reqCount,
                              int timeout)
    : m_additionalThreads(additionalThreads),
      m_reqNumber(0),
      m_warmupReqThreshold(reqCount),
      m_timeout(timeout),
      m_server(server) {}

  std::unique_ptr<RequestHandler> createHandler();

  void bumpReqCount();

private:
  std::atomic<uint32_t> m_additionalThreads;
  std::atomic<uint32_t> m_reqNumber;
  uint32_t const m_warmupReqThreshold;
  int m_timeout;
  // The server has a shared pointer to us, so use a weak pointer to the
  // server to avoid a circular reference.
  ServerWeakPtr m_server;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_WARMUP_REQUEST_HANDLER_H_

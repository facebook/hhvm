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

#ifndef __HPHP_RPC_REQUEST_HANDLER_H__
#define __HPHP_RPC_REQUEST_HANDLER_H__

#include <runtime/base/server/server.h>

namespace HPHP {

class SourceRootInfo;
class RequestURI;
class Transport;
DECLARE_BOOST_TYPES(SatelliteServerInfo);
///////////////////////////////////////////////////////////////////////////////

class RPCRequestHandler : public RequestHandler {
public:
  RPCRequestHandler();
  virtual ~RPCRequestHandler();

  void setServerInfo(SatelliteServerInfoPtr info) { m_serverInfo = info;}

  // implementing RequestHandler
  virtual void handleRequest(Transport *transport);

  /**
   * Count how many requests have been processed on this handler.
   */
  int incRequest() { return ++m_count;}

  /**
   * Whether state has been dirtied.
   */
  bool needReset() const;

private:
  SatelliteServerInfoPtr m_serverInfo;
  int m_count;
  bool m_reset;
  time_t m_created;

  bool executePHPFunction(Transport *transport,
                          SourceRootInfo &sourceRootInfo);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_RPC_REQUEST_HANDLER_H__

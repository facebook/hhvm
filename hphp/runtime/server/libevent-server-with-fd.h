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

#ifndef incl_HPHP_HTTP_SERVER_LIB_EVENT_SERVER_WITH_FD_H_
#define incl_HPHP_HTTP_SERVER_LIB_EVENT_SERVER_WITH_FD_H_

#include "hphp/runtime/server/libevent-server.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * LibEventServer that supports using existing/inherited accept sockets.
 */
class LibEventServerWithFd : public LibEventServer {
public:
  LibEventServerWithFd(const std::string &address, int port, int thread);

  void setServerSocketFd(int sock_fd) {
    m_accept_sock = sock_fd;
  }

  void setSSLSocketFd(int sock_fd) {
    m_accept_sock_ssl = sock_fd;
  }

protected:
  virtual int getAcceptSocket();
  virtual int getAcceptSocketSSL();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_SERVER_LIB_EVENT_SERVER_WITH_FD_H_

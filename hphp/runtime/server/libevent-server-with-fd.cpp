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

#include "hphp/runtime/server/libevent-server-with-fd.h"
#include "hphp/util/logger.h"
#include "folly/String.h"

/*
 * LibEventServer that supports using existing/inherited accept sockets.
 */

namespace HPHP {

LibEventServerWithFd::LibEventServerWithFd
(const std::string &address, int port, int thread)
  : LibEventServer(address, port, thread)
{
}

int LibEventServerWithFd::getAcceptSocket() {
  if (m_accept_sock == -1) {
    Logger::Info("inheritfd: off for server socket");
    return LibEventServer::getAcceptSocket();
  }

  Logger::Info("inheritfd: using inherited fd %d for server", m_accept_sock);

  int ret = listen(m_accept_sock, RuntimeOption::ServerBacklog);
  if (ret != 0) {
    Logger::Error("inheritfd: listen() failed: %s",
        folly::errnoStr(errno).c_str());
    return -1;
  }

  ret = evhttp_accept_socket(m_server, m_accept_sock);
  if (ret < 0) {
    Logger::Error("evhttp_accept_socket: %s",
        folly::errnoStr(errno).c_str());
    int errno_save = errno;
    close(m_accept_sock);
    m_accept_sock = -1;
    errno = errno_save;
    return -1;
  }

  return 0;
}

int LibEventServerWithFd::getAcceptSocketSSL() {
  if (m_accept_sock_ssl == -1) {
    Logger::Info("inheritfd: off for ssl socket");
    return LibEventServer::getAcceptSocketSSL();
  }

  Logger::Info("inheritfd: using inherited fd %d for ssl", m_accept_sock_ssl);

  int ret = listen(m_accept_sock_ssl, RuntimeOption::ServerBacklog);
  if (ret != 0) {
    Logger::Error("inheritfd: listen() failed for ssl: %s",
        folly::errnoStr(errno).c_str());
    return -1;
  }

  ret = evhttp_accept_socket(m_server_ssl, m_accept_sock_ssl);
  if (ret < 0) {
    Logger::Error("evhttp_accept_socket: (ssl) %s",
        folly::errnoStr(errno).c_str());
    int errno_save = errno;
    close(m_accept_sock_ssl);
    m_accept_sock_ssl = -1;
    errno = errno_save;
    return -1;
  }

  return 0;
}

}

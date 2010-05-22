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

#ifndef __HPHP_SOCKET_H__
#define __HPHP_SOCKET_H__

#include <runtime/base/file/file.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SOCKET_ERROR(sock, msg, errn)                           \
  sock->setError(errn);                                         \
  raise_warning("%s [%d]: %s", msg, errn,                       \
                  Util::safe_strerror(errn).c_str())            \

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * TCP/UDP sockets.
 */
class Socket : public File {
public:
  // We cannot use object allocation for this class, because
  // we need to support pfsockopen() that can make a socket persistent.

  Socket();
  Socket(int sockfd, int type, const char *address = NULL, int port = 0);
  virtual ~Socket();

  // overriding ResourceData
  const char *o_getClassName() const { return "Socket";}

  // implementing File
  virtual bool open(CStrRef filename, CStrRef mode);
  virtual bool close();
  virtual int readImpl(char *buffer, int length);
  virtual int writeImpl(const char *buffer, int length);
  virtual bool eof();
  virtual Array getMetaData();
  virtual int tell();

  // allows SSLSocket to perform special checking
  virtual bool checkLiveness() { return true;}

  void setError(int err) { m_error = err;}
  int getError() const { return m_error;}
  int getType() const { return m_type;}

  // This is only for updating a local copy of timeouts set by setsockopt()
  // outside of this class.
  void setTimeout(struct timeval &tv);

  bool setBlocking(bool blocking);

protected:
  std::string m_address;
  int m_port;

  int m_type;
  int m_error;
  bool m_eof;

  int m_timeout; // in micro-seconds;
  bool m_timedOut;

  int m_bytesSent;

  bool closeImpl();
  bool waitForData();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_SOCKET_H__

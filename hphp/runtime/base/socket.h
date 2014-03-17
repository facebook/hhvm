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

#ifndef incl_HPHP_SOCKET_H_
#define incl_HPHP_SOCKET_H_

#include "hphp/runtime/base/file.h"
#include <sys/types.h>
#include <sys/socket.h>

#define SOCKET_ERROR(sock, msg, errn)                                 \
  sock->setError(errn);                                               \
  if (errn != EAGAIN && errn != EWOULDBLOCK && errn != EINPROGRESS) { \
    raise_warning("%s [%d]: %s", msg, errn,                           \
                    folly::errnoStr(errn).c_str());                   \
  }                                                                   \

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * TCP/UDP sockets.
 */
class Socket : public File {
public:
  // We cannot use object allocation for this class, because
  // we need to support pfsockopen() that can make a socket persistent.
  void* operator new(size_t s) {
    return ::operator new(s);
  }
  void operator delete(void* p) {
    return ::operator delete(p);
  }

  Socket();
  Socket(int sockfd, int type, const char *address = nullptr, int port = 0,
         double timeout = 0, const StaticString& streamType = empty_string);
  virtual ~Socket();

  // overriding ResourceData
  const String& o_getClassNameHook() const { return classnameof(); }

  // implementing File
  virtual bool open(const String& filename, const String& mode);
  virtual bool close();
  virtual int64_t readImpl(char *buffer, int64_t length);
  virtual int64_t writeImpl(const char *buffer, int64_t length);
  virtual bool eof();
  virtual Array getMetaData();
  virtual int64_t tell();
  virtual void sweep() FOLLY_OVERRIDE;

  // check if the socket is still open
  virtual bool checkLiveness();

  void setError(int err);
  int getError() const { return m_error;}
  static int getLastError();
  int getType() const { return m_type;}

  // This is only for updating a local copy of timeouts set by setsockopt()
  // outside of this class.
  void setTimeout(struct timeval &tv);

  bool setBlocking(bool blocking);

  std::string getAddress() const { return m_address; }
  int         getPort() const    { return m_port; }
protected:
  std::string m_address;
  int m_port;

  int m_type;
  int m_error;

  int m_timeout; // in micro-seconds;
  bool m_timedOut;

  int64_t m_bytesSent;

  bool closeImpl();
  bool waitForData();
private:
  void inferStreamType();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_SOCKET_H_

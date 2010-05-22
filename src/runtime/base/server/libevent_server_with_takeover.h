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

#ifndef __HTTP_SERVER_LIB_EVENT_SERVER_WITH_TAKEOVER_H__
#define __HTTP_SERVER_LIB_EVENT_SERVER_WITH_TAKEOVER_H__

#include <runtime/base/server/libevent_server.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class TakeoverListener;

/**
 * LibEventServer that adds the ability to take over an accept socket
 * from another process, and give its accept socket up.
 */
class LibEventServerWithTakeover : public LibEventServer {
public:
  LibEventServerWithTakeover(const std::string &address, int port, int thread,
                             int timeoutSeconds);

  virtual void stop();

  // Set the name of the file to be used for a Unix domain socket
  // over which to transfer the accept socket.
  void setTransferFilename(const std::string &fname) {
    ASSERT(m_transfer_fname.empty());
    m_transfer_fname = fname;
  }

  void addTakeoverListener(TakeoverListener* lisener) {
    m_takeover_listeners.insert(lisener);
  }
  void removeTakeoverListener(TakeoverListener* lisener) {
    m_takeover_listeners.erase(lisener);
  }

  // These are public so they can be called from a C-style callback.
  // They are not a part of the public interface.
  void afdtResponse(String response, int fd);
  int afdtRequest(String request, String* response);

protected:
  virtual void start();
  virtual int getAcceptSocket();

  void setupFdServer();
  void notifyTakeoverComplete();

  void* m_delete_handle;
  std::string m_transfer_fname;
  std::set<TakeoverListener*> m_takeover_listeners;
  bool m_took_over;
};

class TakeoverListener {
public:
  virtual ~TakeoverListener();
  virtual void takeoverShutdown(LibEventServerWithTakeover* server) = 0;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HTTP_SERVER_LIB_EVENT_SERVER_H__

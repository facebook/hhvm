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

#ifndef incl_HPHP_HTTP_SERVER_TAKEOVER_AGENT_H_
#define incl_HPHP_HTTP_SERVER_TAKEOVER_AGENT_H_

#include "hphp/runtime/base/complex-types.h"

#include <event.h>

#include <chrono>
#include <set>
#include <string>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * A callback to be informed when a server is shutting down because its socket
 * has been taken over by a new process.
 */
class TakeoverListener {
public:
  virtual ~TakeoverListener();
  virtual void takeoverShutdown() = 0;
};


/**
 * Agent with the ability to take over an accept socket
 * from another process, and give its accept socket up.
 */
class TakeoverAgent {
public:
  enum class RequestType {
    LISTEN_SOCKET,
    TERMINATE,
   };

  class Callback {
   public:
    virtual ~Callback() {}
    // Called by the TakeoverAgent when it receives a request for takeover
    // Returns non zero on error, which terminates the takeover action
    virtual int onTakeoverRequest(RequestType type) = 0;

    // Called by the TakeoverAgent when it is shutdown mid-way through a
    // takeover.
    virtual void takeoverAborted() = 0;
  };

  explicit TakeoverAgent(const std::string &fname);

  // execute a takeover and return the fd.  -1 if an fd could not be acquired
  int takeover(std::chrono::seconds timeout = std::chrono::seconds(2));

  // instruct the old server to shutdown
  void requestShutdown();

  // setup a server to listen for takeover requests
  int setupFdServer(event_base *eventBase, int sock, Callback *callback);

  // stop the takeover agent, including in-progress takeovers
  void stop();

  void addTakeoverListener(TakeoverListener* listener) {
    m_takeover_listeners.insert(listener);
  }
  void removeTakeoverListener(TakeoverListener* listener) {
    m_takeover_listeners.erase(listener);
  }

  // These are public so they can be called from a C-style callback.
  // They are not a part of the public interface.
  void afdtResponse(String response, int fd);
  int afdtRequest(String request, String* response);

protected:

  enum class TakeoverState {
    NotStarted,
    Started,
    Complete,
   };

  void notifyTakeoverComplete();

  void* m_delete_handle;
  std::string m_transfer_fname;
  std::set<TakeoverListener*> m_takeover_listeners;

  // Was this server initiated with a socket from another server?
  bool m_took_over;

  // The state of taking over this server's socket
  TakeoverState m_takeover_state;

  // Target socket
  int m_sock{-1};

  // User callback for events
  Callback *m_callback{nullptr};
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_SERVER_TAKEOVER_AGENT_H_

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

#ifndef incl_HPHP_XBOX_SERVER_H_
#define incl_HPHP_XBOX_SERVER_H_

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/satellite-server.h"
#include "hphp/runtime/server/server-task-event.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct XboxServerInfo;
class RPCRequestHandler;
class XboxTransport;

class XboxServer {
public:
  /**
   * Start or restart xbox server.
   */
  static void Restart();
  static void Stop();

public:
  /**
   * Send/PostMessage paradigm for local and remote RPC.
   */
  static bool SendMessage(const String& message,
                          Array& ret,
                          int timeout_ms,
                          const String& host = "localhost");
  static bool PostMessage(const String& message, const String& host = "localhost");

  /**
   * Local tasklet for parallel processing.
   */
  static Resource TaskStart(const String& msg, const String& reqInitDoc = "",
      ServerTaskEvent<XboxServer, XboxTransport> *event = nullptr);
  static bool TaskStatus(const Resource& task);
  static int TaskResult(const Resource& task, int timeout_ms, Variant &ret);
  static int TaskResult(XboxTransport* const job, int timeout_ms, Variant &ret);

  /**
   * Gets the ServerInfo and RequestHandler for the current xbox worker thread.
   * Returns NULL for non-xbox threads.
   */
  static std::shared_ptr<XboxServerInfo> GetServerInfo();
  static RPCRequestHandler *GetRequestHandler();
};

///////////////////////////////////////////////////////////////////////////////

class XboxServerInfo : public SatelliteServerInfo {
public:
  XboxServerInfo() : SatelliteServerInfo(Hdf()) {
    m_type = SatelliteServer::Type::KindOfXboxServer;
    m_name = "xbox";
    reload();
  }

  void reload() {
    m_threadCount = RuntimeOption::XboxServerThreadCount;
    m_port        = RuntimeOption::XboxServerPort;
    m_maxRequest  = RuntimeOption::XboxServerInfoMaxRequest;
    m_maxDuration = RuntimeOption::XboxServerInfoDuration;
    m_reqInitFunc = RuntimeOption::XboxServerInfoReqInitFunc;
    m_reqInitDoc  = RuntimeOption::XboxServerInfoReqInitDoc;
    m_alwaysReset = RuntimeOption::XboxServerInfoAlwaysReset;
  }

  void setMaxDuration(int duration) { m_maxDuration = duration; }
};

///////////////////////////////////////////////////////////////////////////////

class XboxTransport : public Transport, public Synchronizable {
public:
  explicit XboxTransport(const String& message, const String& reqInitDoc = "");

  timespec getStartTimer() const { return m_queueTime; }

  /**
   * Request URI.
   */
  virtual const char *getUrl();
  virtual const char *getRemoteHost() { return "127.0.0.1"; }
  virtual uint16_t getRemotePort() { return 0; }

  /**
   * Request data.
   */
  virtual Method getMethod() { return Transport::Method::POST; }
  virtual const void *getPostData(int &size) {
    size = m_message.size();
    return m_message.data();
  }

  /**
   * Manage headers.
   */
  virtual std::string getHeader(const char *name);
  virtual void getHeaders(HeaderMap &headers) {}
  virtual void addHeaderImpl(const char *name, const char *value) {}
  virtual void removeHeaderImpl(const char *name) {}

  virtual void sendImpl(const void *data, int size, int code, bool chunked);
  virtual void onSendEndImpl();

  /**
   * Task interface.
   */
  bool isDone() { return m_done; }
  String getResults(int &code, int timeout_ms = 0);

  void setHost(const std::string &host) { m_host = host;}
  void setAsioEvent(ServerTaskEvent<XboxServer, XboxTransport> *event) {
    m_event = event;
  }

  // Refcounting.
  void incRefCount() {
    ++m_refCount;
  }
  void decRefCount() {
    assert(m_refCount.load());
    if (--m_refCount == 0) {
      delete this;
    }
  }

private:
  std::atomic<int> m_refCount;

  std::string m_message;

  bool m_done;
  std::string m_response;
  int m_code;
  std::string m_host;
  std::string m_reqInitDoc;

  ServerTaskEvent<XboxServer, XboxTransport> *m_event;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_XBOX_SERVER_H_

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/server/satellite-server.h"
#include "hphp/runtime/server/server-task-event.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/util/synchronizable.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Array;
struct XboxServerInfo;
struct RPCRequestHandler;
struct XboxTransport;

struct XboxServer {
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
  static void TaskStartFromNonRequest(
    const folly::StringPiece msg,
    const folly::StringPiece reqInitDoc = "");
  static bool TaskStatus(const Resource& task);
  static int TaskResult(const Resource& task, int timeout_ms, Variant *ret);
  static int TaskResult(XboxTransport* const job, int timeout_ms, Variant *ret);

  /**
   * Gets the ServerInfo and RequestHandler for the current xbox worker thread.
   * Returns NULL for non-xbox threads.
   */
  static std::shared_ptr<XboxServerInfo> GetServerInfo();
  static RPCRequestHandler *GetRequestHandler();
};

///////////////////////////////////////////////////////////////////////////////

struct XboxServerInfo : SatelliteServerInfo {
  XboxServerInfo() : SatelliteServerInfo(IniSetting::Map::object, Hdf()) {
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

const StaticString s_xbox("xbox");

struct XboxTransport final : Transport, Synchronizable {
  explicit XboxTransport(
    const folly::StringPiece message,
    const folly::StringPiece reqInitDoc = "");

  timespec getStartTimer() const { return m_queueTime; }

  /**
   * Request URI.
   */
  const char *getUrl() override;
  const char *getRemoteHost() override { return "127.0.0.1"; }
  uint16_t getRemotePort() override { return 0; }
  const std::string& getServerAddr() override {
    auto const& ipv4 = RuntimeOption::GetServerPrimaryIPv4();
    return ipv4.empty() ? RuntimeOption::GetServerPrimaryIPv6() : ipv4;
  }

  /**
   * Request data.
   */
  Method getMethod() override { return Transport::Method::POST; }
  const void *getPostData(size_t &size) override {
    size = m_message.size();
    return m_message.data();
  }

  /**
   * Manage headers.
   */
  std::string getHeader(const char *name) override;
  void getHeaders(HeaderMap& /*headers*/) override {}
  void addHeaderImpl(const char* /*name*/, const char* /*value*/) override {}
  void removeHeaderImpl(const char* /*name*/) override {}

  void sendImpl(const void *data, int size, int code, bool chunked, bool eom)
       override;
  void onSendEndImpl() override;

  /**
   * Get a description of the type of transport.
   */
  String describe() const override {
    return s_xbox;
  }

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

  // points to an event with an attached waithandle from a different request
  ServerTaskEvent<XboxServer, XboxTransport> *m_event;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_XBOX_SERVER_H_

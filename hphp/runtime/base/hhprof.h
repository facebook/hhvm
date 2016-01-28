/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_HHPROF_H_
#define incl_HPHP_HHPROF_H_

#include "hphp/util/alloc.h"
#include "hphp/runtime/server/transport.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct HHProf {
  struct Request {
    static DECLARE_THREAD_LOCAL(Request, s_request);

    static inline void Setup(Transport* transport) {
      if (!RuntimeOption::HHProfEnabled) return;
      if (!RuntimeOption::HHProfRequest) return;
      s_request->setupImpl(transport);
    }
    static inline void Teardown() {
      if (!RuntimeOption::HHProfEnabled) return;
      if (!RuntimeOption::HHProfRequest) return;
      s_request->teardownImpl();
    }
    static inline void StartProfiling() {
      if (!RuntimeOption::HHProfEnabled) return;
      if (!RuntimeOption::HHProfRequest) return;
      s_request->startProfilingImpl();
    }
    static inline void FinishProfiling() {
      if (!RuntimeOption::HHProfEnabled) return;
      if (!RuntimeOption::HHProfRequest) return;
      s_request->finishProfilingImpl();
    }

   private:
    void setupImpl(Transport* transport);
    void teardownImpl();
    void startProfilingImpl();
    void finishProfilingImpl();

    bool m_active = false;
  };

  struct DumpResult {
    /* implicit */ DumpResult()
      : m_id{0}, m_status{204}, m_message{"No dump available"} {}
    explicit DumpResult(const std::string& dump, unsigned id, int status=200)
      : m_dump{dump}, m_id{id}, m_status{status} {}
    DumpResult(const std::string& dump, unsigned id, int status,
               const std::string& message)
      : m_dump{dump}, m_id{id}, m_status{status}, m_message{message} {}

    const std::string dump() const { return m_dump; }
    unsigned id() const { return m_id; }
    int status() const { return m_status; }
    const std::string message() const { return m_message; }

    void send(Transport* transport) const {
      transport->sendString(m_status == 200 ? m_dump : m_message, m_status);
    }

   private:
    std::string m_dump;
    unsigned m_id;
    int m_status;
    std::string m_message;
  };

  HHProf()
    : m_requestType{RequestType::None}, m_requestState{RequestState::None} {}

  static void Init();
  static void HandleHHProfStart(Transport* transport);
  static void HandleHHProfStatus(Transport* transport);
  static void HandleHHProfStop(Transport* transport);
  static void HandlePProfCmdline(Transport* transport);
  static void HandlePProfHeap(Transport* transport);
  static void HandlePProfSymbol(Transport* transport);

 private:
  std::mutex m_mutex;
  std::string m_url;
  DumpResult m_dumpResult;
  unsigned m_nextID{1};

  enum struct RequestType {
    None,
    Next,    // Profile the next request.
    NextURL, // Profile the next request to m_url.
    All      // Profile all requests until stopped.
  };
  RequestType m_requestType;

  enum struct RequestState {
    None,
    Pending,
    Sampling,
    Dumped
  };
  RequestState m_requestState;

  bool setupRequest(Transport* transport);
  void teardownRequest();
  void startProfiledRequest();
  void finishProfiledRequest();

  bool reset(Transport* transport);
  void start(Transport* transport);
  void handleHHProfStartImpl(Transport* transport);
  void handleHHProfStatusImpl(Transport* transport);
  void handleHHProfStopImpl(Transport* transport);
  DumpResult captureDump();
  void handlePProfHeapImpl(Transport* transport);
  bool shouldProfileRequest(Transport* transport);
  void setupRequestImpl(Transport* transport);
  void teardownRequestImpl();
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_HHPROF_H_

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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
#include "hphp/runtime/ext/xenon/ext_xenon.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/implicit-context.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/server/pagelet-server.h"
#include "hphp/runtime/server/xbox-server.h"
#include "hphp/runtime/ext/json/ext_json.h"

#include "hphp/util/configs/xenon.h"
#include "hphp/util/rds-local.h"
#include "hphp/util/timer.h"
#include <chrono>

#include <folly/Random.h>

namespace HPHP {

TRACE_SET_MOD(xenon)

// Data that is kept per request and is only valid per request.
// This structure gathers a php and async stack trace when log is called.
// These logged stacks can be then gathered via a php call, xenon_get_data.
// It needs to allocate and free its Array per request, because Array lifetime
// is per-request.  So the flow for these objects are:
// allocated when a web request begins (if Xenon is enabled)
// grab snapshots of the php and async stack when log is called
// detach itself from its snapshots when the request is ending.
struct XenonRequestLocalData final {
  XenonRequestLocalData();
  ~XenonRequestLocalData();
  void log(
    Xenon::SampleType t,
    EventHook::Source sourceType,
    c_WaitableWaitHandle* wh = nullptr,
    int64_t triggerTime = 0
  );
  static StaticString show(EventHook::Source sourceType);

  Array createResponse();

  void requestInit();
  void requestShutdown();
  bool getIsProfiledRequest();

  // an array of php stacks
  Array m_stackSnapshots;

  // Set to true if this request is going to handle the log calls
  bool m_isProfiledRequest{false};
};
static RDS_LOCAL(XenonRequestLocalData, s_xenonData);

///////////////////////////////////////////////////////////////////////////////
// statics used by the Xenon classes

const StaticString
  s_asio("Asio"),
  s_class("class"),
  s_function("function"),
  s_file("file"),
  s_interpreter("Interpreter"),
  s_implicit_context("implicitContext"),
  s_isWait("ioWaitSample"),
  s_tid("tid"),
  s_jit("Jit"),
  s_line("line"),
  s_metadata("metadata"),
  s_native("Native"),
  s_sourceType("sourceType"),
  s_stack("stack"),
  s_time_ns("timeNano"),
  s_lastTriggerTime("lastTriggerTimeNano"),
  s_unwinder("Unwinder"),
  s_pagelets_workers("PageletWorkers"),
  s_xbox_workers("XboxWorkers"),
  s_http_workers("HttpWorkers"),
  s_cli_workers("CliWorkers");


///////////////////////////////////////////////////////////////////////////
// A singleton object that handles the two Xenon modes (always or timer).
// If in always on mode, the Xenon Surprise flags have to be on for each thread
// and are never cleared.
// For timer mode, when start is invoked, it starts a new thread, responsible
// for periodically setting Xenon surprise flags.

Xenon& Xenon::getInstance() noexcept {
  static Xenon instance;
  return instance;
}

Xenon::Xenon() noexcept
  : m_missedSampleCount(0), m_lastSurpriseTime(0), m_stopping(false) {
}

void Xenon::incrementMissedSampleCount(ssize_t val) {
  m_missedSampleCount += val;
}

int64_t Xenon::getAndClearMissedSampleCount() {
  return std::atomic_exchange<int64_t>(&m_missedSampleCount, 0);
}

// Start Xenon profiler. Needs to be done once per process invocation.
void Xenon::start() {
  TRACE(1, "XenonForceAlwaysOn %d\n", Cfg::Xenon::ForceAlwaysOn);

  // No initialization needed if always on.
  if (Cfg::Xenon::ForceAlwaysOn) return;

  // Xenon not enabled.
  if (Cfg::Xenon::PeriodSeconds <= 0) return;

  assertx(!m_thread.joinable());
  assertx(!m_stopping);
  m_thread = std::thread([](Xenon* xenon) { xenon->run(); }, this);

  TRACE(1, "Xenon::start periodic %.2f seconds\n",
        Cfg::Xenon::PeriodSeconds);
}

// If Xenon owns a pthread, tell it to stop, also clean up anything from start.
void Xenon::stop() {
  if (!m_thread.joinable()) return;

  {
    std::lock_guard<std::mutex> lock(m_shutdownMutex);
    m_stopping = true;
  }
  m_shutdownCondition.notify_one();
  m_thread.join();

  TRACE(1, "Xenon::stop has stopped the waiting thread\n");
}

void Xenon::run() {
  auto const interval = std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::duration<double>(Cfg::Xenon::PeriodSeconds));

  // Stagger the initial event.
  auto next = std::chrono::steady_clock::now() + std::chrono::nanoseconds(
    folly::Random::rand64(interval.count()));

  std::unique_lock<std::mutex> l(m_shutdownMutex);

  while (!m_shutdownCondition.wait_until(l, next, [&] { return m_stopping; })) {
    surpriseAll();
    next = std::max(next + interval, std::chrono::steady_clock::now());
  }
}

// Xenon data is gathered for logging per request, "if we should"
// meaning that if Xenon's Surprise flag has been turned on by someone, we
// should log the stacks.  If we are in XenonForceAlwaysOn, do not clear
// the Surprise flag.  The data is gathered in thread local storage.
// If the sample is Enter, then do not record this function name because it
// hasn't done anything.  The sample belongs to the previous function.
void Xenon::log(SampleType t,
                EventHook::Source sourceType,
                c_WaitableWaitHandle* wh) const {
  if (getSurpriseFlag(XenonSignalFlag)) {
    if (!Cfg::Xenon::ForceAlwaysOn) {
      clearSurpriseFlag(XenonSignalFlag);
    }
    TRACE(1, "Xenon::log %s\n", show(t));
    s_xenonData->log(
      t, sourceType, wh, m_lastSurpriseTime.load(std::memory_order_acquire));
  }
}

// Turns on the Xenon Surprise flag for every thread via a lambda function
// passed to ExecutePerThread.
void Xenon::surpriseAll() {
  TRACE(1, "Xenon::surpriseAll\n");
  m_lastSurpriseTime.store(
    gettime_ns(CLOCK_REALTIME), std::memory_order_release);
  RequestInfo::ExecutePerRequest(
    [] (RequestInfo* t) { t->m_reqInjectionData.setFlag(XenonSignalFlag); }
  );
}

bool Xenon::getIsProfiledRequest() {
  return s_xenonData->getIsProfiledRequest();
}

///////////////////////////////////////////////////////////////////////////////
// There is one XenonRequestLocalData per thread, stored in thread local area

XenonRequestLocalData::XenonRequestLocalData() {
  TRACE(1, "XenonRequestLocalData\n");
}

XenonRequestLocalData::~XenonRequestLocalData() {
  TRACE(1, "~XenonRequestLocalData\n");
}


// Creates an array to respond to the Xenon PHP extension;
// builds the data into the format neeeded.
Array XenonRequestLocalData::createResponse() {
  VecInit stacks(m_stackSnapshots.size());
  for (ArrayIter it(m_stackSnapshots); it; ++it) {
    const auto& frame = it.second().toArray();
    stacks.append(make_dict_array(
      s_time_ns, frame[s_time_ns],
      s_lastTriggerTime, frame[s_lastTriggerTime],
      s_stack, frame[s_stack].toArray(),
      s_isWait, frame[s_isWait],
      s_sourceType, frame[s_sourceType],
      s_xbox_workers, frame[s_xbox_workers],
      s_pagelets_workers, frame[s_pagelets_workers],
      s_http_workers, frame[s_http_workers],
      s_cli_workers, frame[s_cli_workers],
      s_implicit_context, frame[s_implicit_context]
    ));
  }
  return stacks.toArray();
}

StaticString XenonRequestLocalData::show(EventHook::Source sourceType) {
  switch (sourceType) {
    case EventHook::Source::Asio: return s_asio;
    case EventHook::Source::Interpreter: return s_interpreter;
    case EventHook::Source::Jit: return s_jit;
    case EventHook::Source::Native: return s_native;
    case EventHook::Source::Unwinder: return s_unwinder;
  }
  always_assert(false);
}

void XenonRequestLocalData::log(Xenon::SampleType t,
                                EventHook::Source sourceType,
                                c_WaitableWaitHandle* wh,
                                int64_t triggerTime
) {
  if (!m_isProfiledRequest) return;

  TRACE(1, "XenonRequestLocalData::log\n");
  auto now_ns = gettime_ns(CLOCK_REALTIME);
  auto bt = createBacktrace(BacktraceArgs()
                             .skipTop(t == Xenon::EnterSample)
                             .skipInlined(t == Xenon::EnterSample)
                             .fromWaitHandle(wh)
                             .withMetadata()
                             .ignoreArgs());
  
  auto const obj = *ImplicitContext::activeCtx;
  auto const context = Native::data<ImplicitContext>(obj);
  auto& existing_m_map = context->m_map;
  DictInit ic_state(existing_m_map.size());
  for (auto const& p : existing_m_map) {
    ic_state.set(String(p.first->nameStr()), p.second.first);
  }

  Array dict = make_dict_array(
    s_time_ns, now_ns,
    s_lastTriggerTime, triggerTime,
    s_stack, bt,
    s_sourceType, show(sourceType),
    s_isWait, !Xenon::isCPUTime(t),
    s_pagelets_workers,
    Cfg::Xenon::TrackActiveWorkers ? PageletServer::GetActiveWorker() : -1,
    s_xbox_workers,
    Cfg::Xenon::TrackActiveWorkers ? XboxServer::GetActiveWorkers() : -1,
    s_http_workers,
    Cfg::Xenon::TrackActiveWorkers ?
    (HttpServer::Server ? HttpServer::Server->getPageServer()->getActiveWorker() : 0) : -1,
    s_cli_workers,
    Cfg::Xenon::TrackActiveWorkers ? cli_server_active_workers() : -1,
    s_implicit_context,
    ic_state.toArray()
  );
  m_stackSnapshots.append(dict);

  const String path = g_context->m_xenonRequestOutputFile;
  if (!path.isNull() && !path.empty()) {
    // Since callers may use the same file for multiple requests,
    // we want to include the originating thread ID and provide locking.
    dict.set(s_tid, Process::GetThreadPid());
    auto const opts = k_JSON_FB_FORCE_HACK_ARRAYS;
    String out = Variant::attach(HHVM_FN(json_encode)(dict, opts)).toString();

    // Synchronize file append using a static mutex
    static std::mutex xenon_file_mutex;
    std::lock_guard<std::mutex> lock(xenon_file_mutex);
    auto f = std::fopen(path.c_str(), "a");
    if (f) {
      std::fprintf(f, "%s\n", out.c_str());
      std::fclose(f);
    }
  }
}

void XenonRequestLocalData::requestInit() {
  TRACE(1, "XenonRequestLocalData::requestInit\n");

  assertx(!m_isProfiledRequest);
  assertx(m_stackSnapshots.get() == nullptr);
  if (Cfg::Xenon::ForceAlwaysOn) {
    setSurpriseFlag(XenonSignalFlag);
  } else {
    // Clear any Xenon flags that might still be on in this thread so that we do
    // not have a bias towards the first function.
    clearSurpriseFlag(XenonSignalFlag);
  }

  uint32_t freq = Cfg::Xenon::RequestFreq;
  m_isProfiledRequest = (freq > 0 && folly::Random::rand32(freq) == 0);
}

void XenonRequestLocalData::requestShutdown() {
  TRACE(1, "XenonRequestLocalData::requestShutdown\n");

  m_isProfiledRequest = false;
  clearSurpriseFlag(XenonSignalFlag);
  Xenon::getInstance().incrementMissedSampleCount(m_stackSnapshots.size());
  m_stackSnapshots.reset();
}

bool XenonRequestLocalData::getIsProfiledRequest() {
  return m_isProfiledRequest;
}

///////////////////////////////////////////////////////////////////////////////
// Function that allows php code to access request local data that has been
// gathered via surprise flags.

Array HHVM_FUNCTION(xenon_get_data, void) {
  if (Cfg::Xenon::ForceAlwaysOn ||
      Cfg::Xenon::PeriodSeconds > 0) {
    TRACE(1, "xenon_get_data\n");
    return s_xenonData->createResponse();
  }
  return Array::CreateVec();
}

Array HHVM_FUNCTION(xenon_get_and_clear_samples, void) {
  if (Cfg::Xenon::ForceAlwaysOn ||
      Cfg::Xenon::PeriodSeconds > 0) {
    TRACE(1, "xenon_get_and_clear_samples\n");
    Array ret = s_xenonData->createResponse();
    s_xenonData->m_stackSnapshots.reset();
    return ret;
  }
  return Array::CreateVec();
}

int64_t HHVM_FUNCTION(xenon_get_and_clear_missed_sample_count, void) {
    TRACE(1, "xenon_get_and_clear_missed_sample_count\n");
    return Xenon::getInstance().getAndClearMissedSampleCount();
}

bool HHVM_FUNCTION(xenon_get_is_profiled_request, void) {
  return Xenon::getInstance().getIsProfiledRequest();
}

void HHVM_FUNCTION(xenon_set_request_output_file, const String& path) {
  g_context->m_xenonRequestOutputFile = path;
}


struct xenonExtension final : Extension {
  xenonExtension() : Extension("xenon", "2.0", NO_ONCALL_YET) { }

  void moduleRegisterNative() override {
    HHVM_FALIAS(HH\\xenon_get_data, xenon_get_data);
    HHVM_FALIAS(HH\\xenon_get_and_clear_samples, xenon_get_and_clear_samples);
    HHVM_FALIAS(HH\\xenon_get_and_clear_missed_sample_count,
                xenon_get_and_clear_missed_sample_count);
    HHVM_FALIAS(HH\\xenon_get_is_profiled_request,
                xenon_get_is_profiled_request);
    HHVM_FALIAS(HH\\xenon_set_request_output_file,
                xenon_set_request_output_file);
  }

  void requestInit() override {
    s_xenonData->requestInit();
  }

  void requestShutdown() override {
    s_xenonData->requestShutdown();
  }
} s_xenon_extension;

///////////////////////////////////////////////////////////////////////////////
}

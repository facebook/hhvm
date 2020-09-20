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

#include "hphp/runtime/ext/strobelight/ext_strobelight.h"
#include "hphp/runtime/ext/strobelight/tracing_types.h"
#include "hphp/runtime/ext/xenon/ext_xenon.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/util/sync-signal.h"

#include <mutex>
#include <time.h>

#include <folly/tracing/StaticTracepoint.h>
FOLLY_SDT_DEFINE_SEMAPHORE(hhvm, hhvm_stack);

namespace HPHP {

TRACE_SET_MOD(strobelight);

namespace {

// statics used by the Strobelight classes
const StaticString
  s_class("class"),
  s_function("function"),
  s_file("file"),
  s_line("line");

// Shared memory for communicating to BPF
struct strobelight::backtrace_t bt_slab;
std::mutex usdt_mutex;

void onStrobelightSignal(int signo) {
  if (!RuntimeOption::StrobelightEnabled) {
    // Handle the signal so we don't crash, but do nothing.
    TRACE(1, "Strobelight signaled, but disabled\n");
    return;
  }

  if (signo == strobelight::kSignumCurrent) {
    // sets on only current thread
    if (rds::header()) {
      // Ignore threads that are not serving requests, otherwise this segfaults
      if (!Strobelight::isXenonActive()) {
        // Do not set the flag if Xenon is actively profiling this request
        setSurpriseFlag(XenonSignalFlag);
      }
    }
  } else if (signo == strobelight::kSignumAll) {
    // sets on ALL threads
    Strobelight::getInstance().surpriseAll();
  }

  FOLLY_SDT(hhvm, hhvm_surprise);
}

/**
 * Hands a pointer to a well-defined (see tracing_types.h) readable
 * backtrace off to a User Statically-Defined Tracing probe
 */
bool logToUSDT(const Array& bt) {
  std::lock_guard<std::mutex> lock(usdt_mutex);

  memset(&bt_slab, 0, sizeof(bt_slab));

  int i = 0;
  IterateVNoInc(
    bt.get(),
    [&](TypedValue tv) -> bool {

      if (i >= strobelight::kMaxStackframes) {
        return true;
      }

      assertx(isArrayLikeType(type(tv)));
      ArrayData* bt_frame = val(tv).parr;
      strobelight::backtrace_frame_t* frame = &bt_slab.frames[i];

      auto const line = bt_frame->get(s_line.get());
      if (line.is_init()) {
        assertx(isIntType(type(line)));
        frame->line = val(line).num;
      }

      auto const file_name = bt_frame->get(s_file.get());
      if (file_name.is_init()) {
        assertx(isStringType(type(file_name)));
        strncpy(frame->file_name,
                val(file_name).pstr->data(),
                std::min(val(file_name).pstr->size(), strobelight::kFileNameMax));
        frame->file_name[strobelight::kFileNameMax - 1] = '\0';
      }

      auto const class_name = bt_frame->get(s_class.get());
      if (class_name.is_init()) {
        assertx(isStringType(type(class_name)));
        strncpy(frame->class_name,
                val(class_name).pstr->data(),
                std::min(val(class_name).pstr->size(), strobelight::kClassNameMax));
        frame->class_name[strobelight::kClassNameMax - 1] = '\0';
      }

      auto const function_name = bt_frame->get(s_function.get());
      if (function_name.is_init()) {
        assertx(isStringType(type(function_name)));
        strncpy(frame->function,
                val(function_name).pstr->data(),
                std::min(val(function_name).pstr->size(),
                         strobelight::kFunctionMax));
        frame->function[strobelight::kFunctionMax - 1] = '\0';
      }

      i++;
      return false;
    }
  );
  bt_slab.len = i;

  // Allow BPF to read the now-formatted stacktrace
  FOLLY_SDT_WITH_SEMAPHORE(hhvm, hhvm_stack, &bt_slab);

  return true;
}

} // namespace

///////////////////////////////////////////////////////////////////////////
// A singleton object that handles Strobelight logging
Strobelight& Strobelight::getInstance() noexcept {
  static Strobelight instance;
  return instance;
}


void Strobelight::init() {
#if !defined(__APPLE__) && !defined(_MSC_VER)
  signal(strobelight::kSignumCurrent, onStrobelightSignal);
  sync_signal(strobelight::kSignumAll, onStrobelightSignal);
#endif
}

bool Strobelight::active() {
  if (rds::header() && isXenonActive()) {
    // if Xenon owns this request, back off
    return false;
  }

  // return true if a USDT probe function is listening
  return FOLLY_SDT_IS_ENABLED(hhvm, hhvm_stack);
}

bool Strobelight::isXenonActive() {
  TRACE(1, "Strobelight::isXenonActive\n");
  if (RuntimeOption::XenonForceAlwaysOn) {
    TRACE(2, "Strobelight::isXenonActive => true, forced\n");
    return true;
  }

  bool xenonProfiled = Xenon::getInstance().getIsProfiledRequest();
  if (xenonProfiled) {
    TRACE(2, "Strobelight::isXenonActive => true, profiled\n");
    return true;
  }

  TRACE(2, "Strobelight::isXenonActive => false\n");
  return false;
}

void Strobelight::log(c_WaitableWaitHandle* wh) const {
  if (RuntimeOption::XenonForceAlwaysOn) {
    // Disable strobelight if Xenon forced on
    // TODO remove this when strobelight has its own surpriseFlag
    return;
  }

  if (getSurpriseFlag(XenonSignalFlag)) {
    // TODO remove this when strobelight has its own surpriseFlag
    clearSurpriseFlag(XenonSignalFlag);
  }

  TRACE(1, "Strobelight::log\n");
  if (active()) {
    // TODO We should filter only to hhvm samples which directly
    // caused a PMU event to fire. This is doable by storing hhvm
    // request IDs in a bpf map and checking for an entry here.
    auto bt = createBacktrace(BacktraceArgs()
                              .fromWaitHandle(wh)
                              // TODO
                              // .withMetadata()
                              .ignoreArgs());
    logToUSDT(bt);
  }
}

void Strobelight::surpriseAll() {
  TRACE(1, "Strobelight::surpriseAll\n");

  RequestInfo::ExecutePerRequest(
    [] (RequestInfo* t) {
      // TODO: get a dedicated surprise flag to avoid colliding with xenon
      // Set the strobelight flag to collect a sample
      if (!isXenonActive()) {
        // Xenon has first crack at profiling requests. If a request
        // is marked as being profiled, we do not allow strobelight to
        // interfere with Xenon's profiling. In practice, collisions
        // should be extremely rare.
        t->m_reqInjectionData.setFlag(XenonSignalFlag);
      }
    }
  );
}

///////////////////////////////////////////////////////////////////////////////
}

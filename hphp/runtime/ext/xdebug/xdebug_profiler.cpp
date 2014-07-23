/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/xdebug/xdebug_profiler.h"
#include "hphp/runtime/ext/xdebug/ext_xdebug.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/ext/ext_hotprofiler.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/timer.h"

namespace HPHP {

void XDebugProfiler::ensureBufferSpace() {
  if (m_nextFrameIdx < m_frameBufferSize) {
    return;
  }

  // The initial buffer size is 0
  size_t new_buf_size = (m_frameBufferSize == 0)?
    XDebugExtension::FramebufSize :
    m_frameBufferSize * XDebugExtension::FramebufExpansion;

  try {
    size_t new_buf_bytes = new_buf_size * sizeof(FrameData);
    m_frameBuffer = (FrameData*) smart_realloc((void*) m_frameBuffer,
                                               new_buf_bytes);
    m_frameBufferSize = new_buf_size;
  } catch (const OutOfMemoryException& e) {
    raise_error("Cannot allocate more memory for the xdebug profiler. Consider "
                "turning off profiling or tracing. Note that certain ini "
                "settings such as hhvm.xdebug.collect_memory and "
                "hhvm.xdebug.collect_time implicitly "
                "turn on tracing, so turn those off if this is unexpected.\n"
                "Current frame buffer length: %zu\n"
                "Failed to expand to length: %zu\n",
                m_frameBufferSize,
                new_buf_size);
  }
}

void XDebugProfiler::collectFrameData(FrameData& frameData,
                                      const TypedValue* retVal) {
  VMRegAnchor _; // Ensure consistent state for vmfp and vmpc
  ActRec* fp = vmfp();
  bool is_func_begin = retVal == nullptr;
  frameData.is_func_begin = is_func_begin;

  // The function reference and line numbers are stored when tracing/profiling
  // on function enter
  if ((m_tracingEnabled || m_profilingEnabled) && is_func_begin) {
    frameData.func = fp->func();
    frameData.line = fp->func()->unit()->offsetOf(vmpc());
  } else {
    frameData.func = nullptr;
    frameData.line = 0;
  }

  // Time is stored if collect_time is enabled, but it only need to be
  // collected on function exit if tracing
  if (XDebugExtension::CollectTime && (is_func_begin || m_tracingEnabled)) {
    frameData.time = Timer::GetCurrentTimeMicros();
  } else {
    frameData.time = 0;
  }

  // Memory usage is stored if collect_memory is enabled, but it only
  // needs to be collected on function exit if tracing
  if (XDebugExtension::CollectMemory && (is_func_begin || m_tracingEnabled)) {
    frameData.memory_usage = MM().getStats().usage;
  } else {
    frameData.memory_usage = 0;
  }

  // If tracing is enabled, we may need to collect a serialized version of
  // the arguments or the return value.
  if (m_tracingEnabled && is_func_begin && XDebugExtension::CollectParams > 0) {
    // TODO(#4489053) This is either going to require a bunch of copied and
    //                pasted code or a refactor of debugBacktrace to pull the
    //                arguments list from the more general location.
    //                This relies on xdebug_var_dump anyways.
    throw_not_implemented("Tracing with collect_params enabled");
  } else if (m_tracingEnabled && !is_func_begin &&
             XDebugExtension::CollectReturn) {
    // TODO(#4489053) This relies on xdebug_var_dump
    throw_not_implemented("Tracing with collect_return enabled");
  } else {
    frameData.context_str = nullptr;
  }
}

void XDebugProfiler::recordFrame(const TypedValue* retVal) {
  ensureBufferSpace();
  FrameData* fd = &m_frameBuffer[m_nextFrameIdx++];
  collectFrameData(*fd, retVal);
}

void XDebugProfiler::beginFrame(const char *symbol) {
  recordFrame(nullptr);
}

void XDebugProfiler::endFrame(const TypedValue* retVal,
                              const char *symbol,
                              bool endMain /* = false */) {
  // If tracing or profiling are enabled, we need to store end frames as well.
  // Otherwise we can just overwrite the most recent begin frame
  if (m_tracingEnabled || m_profilingEnabled) {
    recordFrame(retVal);
  } else {
    m_nextFrameIdx--;
  }
}

void XDebugProfiler::enableTracing() {
  assert(!m_tracingEnabled);
  m_tracingEnabled = true;
  m_tracingStartIdx = m_nextFrameIdx;
}

}

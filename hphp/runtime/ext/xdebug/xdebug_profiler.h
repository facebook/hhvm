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

#ifndef incl_HPHP_XDEBUG_PROFILER_H_
#define incl_HPHP_XDEBUG_PROFILER_H_

#include "hphp/runtime/ext/xdebug/ext_xdebug.h"
#include "hphp/runtime/ext/ext_hotprofiler.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Frame data gathered on function enter/exit.
struct FrameData {
  const Func* func; // So we can grab its name and filename later
  int64_t line;
  int64_t time;
  // We don't need all 64 bits for memory_usage and we need to save space as
  // this struct is massive and we allocate a ton of them. The TraceProfiler
  // in ext_hotprofiler.cpp does the same.
  int64_t memory_usage : 63;
  bool is_func_begin : 1; // Whether or not this is an enter event
  // If is_func_begin, then this will be the serialized aruments. Otherwise it
  // will be the serialized return value
  const char* context_str;
};

// TODO(#4489053) consider allowing user to set the maximum buffer size
class XDebugProfiler : public Profiler {
public:
  explicit XDebugProfiler(bool tracingEnabled, bool profilingEnabled)
    : m_frameBuffer(nullptr)
    , m_frameBufferSize(0)
    , m_nextFrameIdx(0)
    , m_tracingEnabled(tracingEnabled)
    , m_profilingEnabled(profilingEnabled)
  {};

  ~XDebugProfiler() {
    if (m_profilingEnabled) {
      writeProfilingResults();
    }
    if (m_tracingEnabled) {
      disableTracing();
    }
    smart_free(m_frameBuffer);
  }

  // Enable/disable tracing
  void enableTracing();
  void disableTracing() XDEBUG_NOTIMPLEMENTED

  // TODO (#4489053) Iteration up the stack
  inline void begin() XDEBUG_NOTIMPLEMENTED
  inline void end() XDEBUG_NOTIMPLEMENTED
private:
  // Allocates more buffer space if needed, otherwise does nothing.
  // On failure, a human-readable error is thrown.
  void ensureBufferSpace();

  // Populates the passed frame data. All fields should be filled, because
  // the passed frameData is filled with junk.
  virtual void collectFrameData(FrameData& frameData, const TypedValue* retVal);

  // Helper for begin/end frame that grabs the next frame data and populates it
  // If retVal is null, this is a begin frame. Otherwise this is a frame end
  // event and retVal is the returned value
  void recordFrame(const TypedValue* retVal);

  // Functions called on frame begin/end
  virtual void beginFrame(const char* symbol);
  virtual void endFrame(const TypedValue* retVal,
                        const char* symbol,
                        bool endMain = false);
  virtual inline void endAllFrames() {}

  // Called on profiler destruction, writes the profiling results.
  // TODO(#4489053) Implement
  void writeProfilingResults()
    XDEBUG_NOTIMPLEMENTED

  // xdebug has no need to write stats to php array
  virtual inline void writeStats(Array &ret) {}

  FrameData* m_frameBuffer;
  size_t m_frameBufferSize;
  size_t m_nextFrameIdx;
  size_t m_tracingStartIdx;
  bool m_tracingEnabled;
  bool m_profilingEnabled;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_XDEBUG_PROFILER_H_

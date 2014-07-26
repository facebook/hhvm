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
// TODO(#4489053) Should look into reducing size
struct FrameData {
  const Func* func;
  int64_t time;
  Offset line; // For begin frames, the line called from
  // We don't need all 64 bits for memory_usage and we need to save space as
  // this struct is massive and we allocate a ton of them. The TraceProfiler
  // in ext_hotprofiler.cpp does the same.
  int64_t memory_usage : 63;
  bool is_func_begin : 1; // Whether or not this is an enter event
  // If is_func_begin, then this will be the serialized aruments. Otherwise it
  // will be the serialized return value
  const StringData* context_str;
};

// TODO(#4489053) consider allowing user to set the maximum buffer size
class XDebugProfiler : public Profiler {
public:
  explicit XDebugProfiler() {}
  ~XDebugProfiler() {
    if (m_profilingEnabled) {
      writeProfilingResults();
    }
    if (m_tracingEnabled) {
      disableTracing();
    }
    smart_free(m_frameBuffer);
  }

  // Whether or not the profiler is collecting data
  inline bool isCollecting() {
    return m_profilingEnabled || m_tracingEnabled || m_collectMemory ||
           m_collectTime;
  }

  // Set memory/time collecting
  inline void setCollectMemory(bool collect) { m_collectMemory = collect; }
  inline void setCollectTime(bool collect) { m_collectTime = collect; }

  // Enables profiling. Profiling cannot be disabled.
  void enableProfiling(const String& filename, int64_t opts);
  inline bool isProfiling() { return m_profilingEnabled; }
  inline const String getProfilingFilename() {
    return m_profilingEnabled ? m_profilingFilename : empty_string();
  }

  // Enable/disable tracing
  void enableTracing(const String& filename, int64_t opts);
  void disableTracing();
  inline bool isTracing() { return m_tracingEnabled; }
  inline const String getTracingFilename() {
    return m_tracingEnabled ? m_tracingFilename : empty_string();
  }

  // Functions called on frame begin/end
  virtual void beginFrame(const char* symbol);
  virtual void endFrame(const TypedValue* retVal,
                        const char* symbol,
                        bool endMain = false);
  virtual inline void endAllFrames() {}

  // xdebug has no need to write stats to php array
  virtual inline void writeStats(Array &ret) {}

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

  // Used when we have both the beginning and end frame data
  struct Frame {
    FrameData& begin;
    FrameData& end;
    explicit Frame(FrameData& beginFrame, FrameData& endFrame)
      : begin(beginFrame), end(endFrame) {}
  };

  // Called on profiler destruction, writes the profiling results.
  void writeProfilingResults();

  // Writes a stack frame's profiling information to the profiling file. The
  // frame's begin frame data starts at startIdx in the internal buffer. Returns
  // the index in the internal buffer of the frame's end frame data or -1 if
  // an error occurred.
  ssize_t writeProfilingFrame(ssize_t startIdx);

  // Writes the given frame in cachegrind format to the given file
  void writeCachegrindFrame(const Frame& frame,
                            const std::vector<Frame>& children,
                            int64_t childrenCost,
                            bool isTopPseudoMain);

  // Writes the given function's filename in cachegrind format
  void writeCachegrindFuncFileName(const Func* func);

  // Writes the given function's name in cachegrind format
  void writeCachegrindFuncName(const Func* func, bool isTopPseudoMain);

  FrameData* m_frameBuffer = nullptr;
  uint64_t m_frameBufferSize = 0;
  uint64_t m_nextFrameIdx = 0;

  bool m_collectMemory = false;
  bool m_collectTime = false;

  bool m_profilingEnabled = false;
  int64_t m_profilingOpts = 0;
  String m_profilingFilename;
  FILE* m_profilingFile;

  bool m_tracingEnabled = false;
  uint64_t m_tracingStartIdx = 0;
  int64_t m_tracingOpts = 0;
  String m_tracingFilename;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_XDEBUG_PROFILER_H_

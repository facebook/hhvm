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
#include "hphp/runtime/ext/xdebug/xdebug_utils.h"

#include "hphp/runtime/ext/hotprofiler/ext_hotprofiler.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Frame data gathered on function enter/exit.
struct FrameData {
  const Func* func;
  int64_t time;
  Offset line; // For begin frames, the line called from
  // We don't need all 64 bits for memory_usage and we need to save space as
  // this struct is massive and we allocate a ton of them. The TraceProfiler
  // in ext_hotprofiler.cpp does the same.
  int64_t memory_usage : 63;
  bool is_func_begin : 1; // Whether or not this is an enter event
  // TODO(#3704) need a string field for serialized arguments or return value.
};

// TODO(#3704) Allow user to set maximum buffer size
struct XDebugProfiler : Profiler {
  explicit XDebugProfiler() : Profiler(true) {}
  ~XDebugProfiler() {
    if (m_profilingEnabled) {
      writeProfilingResults();
    }
    if (m_tracingEnabled) {
      disableTracing();
    }
    req::free(m_frameBuffer);
  }

  // Returns true if function enter/exit event collection is required by the
  // extension settings
  static inline bool isCollectionNeeded() {
    return
      XDEBUG_GLOBAL(MaxNestingLevel) ||
      XDEBUG_GLOBAL(CollectTime) ||
      XDEBUG_GLOBAL(CollectMemory);
  }

  // Returns true if profiling is required by the extension settings or the
  // environment
  static inline bool isProfilingNeeded() {
    return
      XDEBUG_GLOBAL(ProfilerEnable) ||
      (XDEBUG_GLOBAL(ProfilerEnableTrigger) &&
       XDebugUtils::isTriggerSet("XDEBUG_PROFILE"));
  }

  // Returns true if tracing is required by the extension settings or the
  // environment
  static inline bool isTracingNeeded() {
    return
      XDEBUG_GLOBAL(AutoTrace) ||
      (XDEBUG_GLOBAL(TraceEnableTrigger) &&
       XDebugUtils::isTriggerSet("XDEBUG_TRACE"));
  }

  // Returns true if a profiler should be attached to the current thread
  static inline bool isAttachNeeded() {
    return isCollectionNeeded() || isProfilingNeeded() || isTracingNeeded();
  }

  // Whether or not this profiler is collecting frame/exit enter information
  inline bool isCollecting() {
    return
      m_profilingEnabled ||
      m_tracingEnabled ||
      m_collectMemory ||
      m_collectTime;
  }

  // Whether or not this profiler is needed
  inline bool isNeeded() {
    return m_maxDepth != 0 || isCollecting();
  }

  // Ini setting setters
  inline void setCollectMemory(bool collect) { m_collectMemory = collect; }
  inline void setCollectTime(bool collect) { m_collectTime = collect; }
  inline void setMaxNestingLevel(int depth) { m_maxDepth = depth; }

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

  // TODO (#3704) Need some way to get stack time/memory information for when
  //              we print the stack trace
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

  // Helper used to convert a microseconds since epoch into the format xdebug
  // uses: microseconds since request init, as a double
  inline double timeSinceBase(int64_t time) {
    return (time - XDEBUG_GLOBAL(InitTime)) * 1.0e-6;
  }

  // The different types of tracefile output
  enum class TraceOutputType {
    NORMAL,
    COMPUTERIZED,
    HTML
  };

  // Write the tracing results in normal/computerized/html format. A template is
  // used to prevent unnecessary runtime casing.
  template <TraceOutputType outputType>
  void writeTracingResults();

  template <TraceOutputType outputType>
  void writeTracingResultsHeader();

  template <TraceOutputType outputType>
  void writeTracingResultsFooter();

  // Write a given frame in normal/computerized/html format. The frame is at
  // stack level level and its' begin frame starts at startIdx in the internal
  // buffer. Its parent begin frame is passed. Returns the index of the frame's
  // end frame in the internal buffer, or -1 if there was no end frame.
  template <TraceOutputType outputType>
  int64_t writeTracingFrame(int64_t level,
                            int64_t startIdx,
                            const FrameData* parentBegin);

  template <TraceOutputType outputType>
  void writeTracingLinePrefix();

  template <TraceOutputType outputType>
  void writeTracingLevel(int64_t level);

  template <TraceOutputType outputType>
  void writeTracingFrameId(uint64_t id);

  template <TraceOutputType outputType>
  void writeTracingTime(int64_t time);

  template<TraceOutputType outputType>
  void writeTracingMemory(int64_t memory);

  template <TraceOutputType outputType>
  void writeTracingIndent(int64_t level);

  void writeTracingFuncName(const Func* func, bool isTopPseudoMain);

  template<TraceOutputType outputType>
  void writeTracingFunc(FrameData& frame, bool isTopPseudoMain);

  template <TraceOutputType outputType>
  void writeTracingCallsite(FrameData& frame, const FrameData* parent);

  template <TraceOutputType outputType>
  void writeTracingLineSuffix();

  template <TraceOutputType outputType>
  void writeTracingEndFrame(FrameData& frame, int64_t level, int64_t id);

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
  int64_t writeProfilingFrame(int64_t startIdx);

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
  int64_t m_frameBufferSize = 0;
  int64_t m_nextFrameIdx = 0;

  bool m_collectMemory = false;
  bool m_collectTime = false;

  bool m_profilingEnabled = false;
  int64_t m_profilingOpts = 0;
  String m_profilingFilename;
  FILE* m_profilingFile;

  bool m_tracingEnabled = false;
  int64_t m_tracingOpts = 0;
  int64_t m_tracingStartIdx = 0;
  std::vector<FrameData> m_tracingStartFrameData;
  String m_tracingFilename;
  FILE* m_tracingFile;

  // Maximum stack depth allowed. 0 implies infinite
  uint64_t m_maxDepth = 0;
  uint64_t m_depth = XDebugUtils::stackDepth(); // current stack depth

  // When writing the tracing file in computerized and html output we need to
  // assign each begin/end frame pair an id.
  uint64_t m_tracingNextFrameId = 0;

  // When writing the tracing file with show_mem_delta we need a reference to
  // the previous begin frame
  FrameData* m_tracingPrevBegin = nullptr;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_XDEBUG_PROFILER_H_

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

using std::vector;

namespace HPHP {

void XDebugProfiler::ensureBufferSpace() {
  if (m_nextFrameIdx < m_frameBufferSize) {
    return;
  }

  // The initial buffer size is 0
  int64_t new_buf_size = (m_frameBufferSize == 0)?
    XDebugExtension::FramebufSize :
    m_frameBufferSize * XDebugExtension::FramebufExpansion;

  try {
    int64_t new_buf_bytes = new_buf_size * sizeof(FrameData);
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
  // If we're not collecting any data, this shouldn't be running
  assert(isCollecting());

  VMRegAnchor _; // Ensure consistent state for vmfp and vmpc
  ActRec* fp = vmfp();
  bool is_func_begin = retVal == nullptr;
  frameData.is_func_begin = is_func_begin;

  // The function reference and call file/line are stored when tracing/profiling
  // on function enter
  if ((m_tracingEnabled || m_profilingEnabled) && is_func_begin) {
    frameData.func = fp->func();

    // Need the previous frame in order to get the call line. If we cannot
    // get the previous frame, default to 0
    Offset offset;
    const ActRec* prevFp = g_context->getPrevVMState(fp, &offset);
    if (prevFp != nullptr) {
      frameData.line = prevFp->unit()->getLineNumber(offset);
    } else {
      frameData.line = 0;
    }
  } else {
    frameData.func = nullptr;
    frameData.line = 0;
  }

  // Time is stored if profiling, tracing, or collect_time is enabled, but it
  // only needs to be collected on function exit if profiling.
  if (m_profilingEnabled ||
      (is_func_begin && (m_collectTime || m_tracingEnabled))) {
    frameData.time = Timer::GetCurrentTimeMicros();
  } else {
    frameData.time = 0;
  }

  // Memory usage is stored on function begin if tracing or if collect_memory
  // is enabled
  if (is_func_begin && (m_tracingEnabled || m_collectMemory)) {
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

///////////////////////////////////////////////////////////////////////////////
// Tracing

void XDebugProfiler::enableTracing(const String& filename, int64_t opts) {
  assert(!m_tracingEnabled);

  // Attempt to open the passed filename. php5 xdebug doesn't enable tracing
  // if we cannot open the file, so we need to open it now as opposed to when we
  // actually do the writing in order to ensure we handle this case. We keep the
  // file handle open in order to ensure we can still write on tracing stop
  FILE* file;
  if (opts & k_XDEBUG_TRACE_APPEND) {
    file = fopen(filename.data(), "a");
  } else {
    file = fopen(filename.data(), "w");
  }

  // If file is null, opening the passed filename failed. php5 xdebug doesn't
  // do anything in this case, but we should probably notify the user
  if (file == nullptr) {
    raise_warning("xdebug profiler failed to open tracing file %s for writing.",
                  filename.data());
    return;
  }

  m_tracingEnabled = true;
  m_tracingStartIdx = m_nextFrameIdx;
  m_tracingFilename = filename;
  m_tracingFile = file;
  m_tracingOpts = opts;

  // If we're not at the top level, need to grab the call sites for each frame
  // on the stack.
  VMRegAnchor _;
  Offset offset;
  ActRec* fp = vmfp();
  while ((fp = g_context->getPrevVMState(fp, &offset)) != nullptr) {
    FrameData frame;
    frame.func = fp->func();
    frame.line = fp->unit()->getLineNumber(offset);
    m_tracingStartFrameData.push_back(frame);
  }
}

// TODO(#4489053) If we aren't profiling, we should try to save space by
// removing trace data
void XDebugProfiler::disableTracing() {
  if (m_tracingOpts & k_XDEBUG_TRACE_COMPUTERIZED) {
    writeTracingResults<TraceOutputType::COMPUTERIZED>();
  } else if (m_tracingOpts & k_XDEBUG_TRACE_HTML) {
    writeTracingResults<TraceOutputType::HTML>();
  } else {
    writeTracingResults<TraceOutputType::NORMAL>();
  }

  // Cleanup
  m_tracingEnabled = false;
  m_tracingStartFrameData.clear();
  fclose(m_tracingFile);
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingResults() {
  int64_t buf_idx = m_tracingStartIdx;
  int64_t level = m_tracingStartFrameData.size();

  // We don't necessarily start at a top-level frame so we need to continue
  // iterating until we run out of frames
  writeTracingResultsHeader<outputType>();
  while (buf_idx < m_nextFrameIdx) {
    assert(level >= 0);
    if (!m_frameBuffer[buf_idx].is_func_begin) {
      buf_idx++;
      level--;
      continue;
    }

    // Grab the precomputed frame data for this level
    FrameData* frameData = (level > 0)?
      &m_tracingStartFrameData[level - 1] :
      nullptr;

    // Write the current frame at this level
    int64_t end;
    if ((end = writeTracingFrame<outputType>(level, buf_idx, frameData)) < 0) {
      break;
    }
    buf_idx = end + 1;
  }
  writeTracingResultsFooter<outputType>();
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingResultsHeader() {
  switch (outputType) {
    case TraceOutputType::NORMAL:
      fprintf(m_tracingFile, "TRACE START");
      fprintTimestamp(m_tracingFile);
      fprintf(m_tracingFile, "\n");
      break;
    default:
      throw_not_implemented("Writing tracing results in this format is not "
                            "currently supported.");
  }
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingResultsFooter() {
  switch (outputType) {
    case TraceOutputType::NORMAL:
      fprintf(m_tracingFile, "TRACE END");
      fprintTimestamp(m_tracingFile);
      fprintf(m_tracingFile, "\n");
      break;
    default:
      throw_not_implemented("Writing tracing results in this format is not "
                            "currently supported.");
  }
}

template<XDebugProfiler::TraceOutputType outputType>
int64_t XDebugProfiler::writeTracingFrame(int64_t level,
                                          int64_t startIdx,
                                          const FrameData* parentBegin) {
  FrameData& begin = m_frameBuffer[startIdx];
  assert(begin.is_func_begin);

  writeTracingTime<outputType>(begin.time);
  writeTracingMemory<outputType>(begin.memory_usage);
  writeTracingIndent<outputType>(level);
  writeTracingFuncName<outputType>(begin, level == 0);
  writeTracingCallsite<outputType>(begin, parentBegin);
  fprintf(m_tracingFile, "\n");

  // This is needed to determine the delta memory usage
  m_tracingPrevBegin = &begin;

  // Iterate over children
  int64_t buf_idx = startIdx + 1;
  while (buf_idx < m_nextFrameIdx) {
    // TODO(#4489053) If collect_return and !is_func_begin, write return value
    if (!m_frameBuffer[buf_idx].is_func_begin) {
      return buf_idx;
    }

    // Recursively write the child
    int64_t end;
    if ((end = writeTracingFrame<outputType>(level + 1, buf_idx, &begin)) < 0) {
      return -1;
    }
    buf_idx = end + 1;
  }

  // If we get here, there was no end frame. Since the user can stop the trace
  // at any time, this isn't an error.
  return -1;
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingTime(int64_t time) {
  // TODO(#4489053) Support other types of output types
  switch (outputType) {
    case TraceOutputType::NORMAL:
      fprintf(m_tracingFile, "%10.4f ", timeSinceBase(time));
      break;
    default:
      throw_not_implemented("Writing tracing results in this format is not "
                            "currently supported.");
  }
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingMemory(int64_t memory) {
  // TODO(#4489053) Support other types of output types
  switch (outputType) {
    case TraceOutputType::NORMAL:
      fprintf(m_tracingFile, "%10lu ", memory);
      // Delta Memory (since the last begin frame)
      if (XDebugExtension::ShowMemDelta) {
        if (m_tracingPrevBegin != nullptr) {
          int64_t prev_usage = m_tracingPrevBegin->memory_usage;
          fprintf(m_tracingFile, "%+8ld", memory - prev_usage);
        } else {
          fprintf(m_tracingFile, "        ");
        }
      }
      break;
    default:
      throw_not_implemented("Writing tracing results in this format is not "
                            "currently supported.");
  }
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingIndent(int64_t level) {
  // TODO(#4489053) Support other types of output types
  switch (outputType) {
    case TraceOutputType::NORMAL:
      for (int i = 0; i < level + 1; i++) {
        fprintf(m_tracingFile, "  ");
      }
      fprintf(m_tracingFile, "-> ");
      break;
    default:
      throw_not_implemented("Writing tracing results in this format is not "
                            "currently supported.");
  }
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingFuncName(FrameData& frame,
                                          bool isTopPseudoMain) {
  // TODO(#4489053) If collect_params, write the arguments
  // TODO(#4489053) Support other types of output types
  switch (outputType) {
    case TraceOutputType::NORMAL:
      if (isTopPseudoMain) {
        fprintf(m_tracingFile, "{main} ");
      } else if (frame.func->isPseudoMain()) {
        fprintf(m_tracingFile, "include(%s) ", frame.func->filename()->data());
      } else {
        fprintf(m_tracingFile, "%s() ", frame.func->fullName()->data());
      }
      break;
    default:
      throw_not_implemented("Writing tracing results in this format is not "
                            "currently supported.");
  }
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingCallsite(FrameData& frame,
                                          const FrameData* parent) {
  if (parent == nullptr) {
    return;
  }

  // TODO(#4489053) Support other types of output types
  switch (outputType) {
    case TraceOutputType::NORMAL:
      fprintf(m_tracingFile, "%s", parent->func->filename()->data());
      fprintf(m_tracingFile, ":%d", frame.line);
      break;
    default:
      throw_not_implemented("Writing tracing results in this format is not "
                            "currently supported.");
  }
}

///////////////////////////////////////////////////////////////////////////////
// Profiling

// Used to grab $_SERVER[SCRIPT_NAME]
static const StaticString
  s_SERVER("_SERVER"),
  s_SCRIPT_NAME("SCRIPT_NAME");

void XDebugProfiler::enableProfiling(const String& filename, int64_t opts) {
  assert(!m_profilingEnabled);

  // Attempt to open the passed filename. php5 xdebug doesn't enable profiling
  // if we cannot open the file, so we need to open it now as opposed to when we
  // actually do the writing in order to ensure we handle this case. We keep the
  // file handle open in order to ensure we can still write on request shutdown
  FILE* file;
  if (opts & k_XDEBUG_PROFILE_APPEND) {
    file = fopen(filename.data(), "a");
  } else {
    file = fopen(filename.data(), "w");
  }

  // If file is null, opening the passed filename failed. php5 xdebug doesn't
  // do anything in this case, but we should probaly notify the user
  if (file == nullptr) {
    raise_warning("xdebug profiler failed to open profiling file %s for "
                  "writing.", filename.data());
    return;
  }

  m_profilingEnabled = true;
  m_profilingFilename = filename;
  m_profilingFile = file;
  m_profilingOpts = opts;
}

void XDebugProfiler::writeProfilingResults() {
  // If we're appending to the file, start a new section
  if (m_profilingOpts & k_XDEBUG_PROFILE_APPEND) {
    fprintf(m_profilingFile, "\n==== NEW PROFILING FILE ======================="
                             "=======================\n");
  }

  // Grab $_SERVER['SCRIPT_NAME'] so we can match xdebug %s filename format
  // option
  Array server = get_global_variables()->asArrayData()->get(s_SERVER).toArray();
  const char* scriptname = server[s_SCRIPT_NAME].toString().data();

  // Print the header and body
  fprintf(m_profilingFile, "version: 1\ncreator: xdebug %s\n", XDEBUG_VERSION);
  fprintf(m_profilingFile, "cmd: %s\npart: 1\npositions: line\n\n", scriptname);
  fprintf(m_profilingFile, "events: Time\n\n");
  if (m_frameBufferSize > 0 && writeProfilingFrame(0) < 0) {
    fprintf(stderr, "Error when writing xdebug profiling file %s. Frame buffer "
                    "invalid.", m_profilingFilename.data());
  }

  // Cleanup
  m_profilingEnabled = false;
  fclose(m_profilingFile);
}

int64_t XDebugProfiler::writeProfilingFrame(int64_t startIdx) {
  assert(m_frameBuffer[startIdx].is_func_begin);

  // We need to store the child calls so we don't have to find
  // them again in writeCacheGrindFrame. Theoretically, this could be stored
  // within the frameData itself, but that's probably not worth the runtime
  // memory penalty so we take the performance hit now. We've already completed
  // the request at this point anyways.
  vector<Frame> children;
  int64_t children_cost = 0; // Time spent in children

  // Iterate until we find the end frame
  int64_t buf_idx = startIdx + 1;
  while (buf_idx < m_nextFrameIdx) {
    FrameData& frame_data = m_frameBuffer[buf_idx];
    if (frame_data.is_func_begin) {
      // This is the beginning of a child frame, recursively write it
      int64_t end;
      if ((end = writeProfilingFrame(buf_idx)) < 0) {
        break;
      }

      // Record the children cost, then push it onto the children list
      FrameData& end_frame_data = m_frameBuffer[end];
      children_cost += end_frame_data.time - frame_data.time;
      children.push_back(Frame(frame_data, end_frame_data));
      buf_idx = end + 1;
    } else {
      // This is the end frame, write it then return its index
      const Frame frame(m_frameBuffer[startIdx], frame_data);
      writeCachegrindFrame(frame, children, children_cost, startIdx == 0);
      return buf_idx;
    }
  }

  // Should never get here or the buffer was invalid
  return -1;
}


void XDebugProfiler::writeCachegrindFrame(const Frame& frame,
                                          const vector<Frame>& children,
                                          int64_t childrenCost,
                                          bool isTopPseudoMain) {
  // Write out the frame's info
  writeCachegrindFuncFileName(frame.begin.func);
  writeCachegrindFuncName(frame.begin.func, isTopPseudoMain);
  if (isTopPseudoMain) {
    fprintf(m_profilingFile, "\nSummary: %lu\n\n",
                            frame.end.time - frame.begin.time);
  }
  fprintf(m_profilingFile, "%d %ld\n",
          frame.begin.func->line1(),
          frame.end.time - frame.begin.time - childrenCost);

  // Write each child call
  for (const Frame& child_frame : children) {
    const Func* child_func = child_frame.begin.func;

    // child filename and func name should be prepended with 'c' (cfl & cfn)
    fprintf(m_profilingFile, "c");
    writeCachegrindFuncFileName(child_func);
    fprintf(m_profilingFile, "c");
    writeCachegrindFuncName(child_func, false);

    // Technically we should be coallescing these child calls, but this is what
    // php5 xdebug does.
    fprintf(m_profilingFile, "calls=1 0 0\n");
    fprintf(m_profilingFile, "%d %ld\n",
            child_frame.begin.line,
            child_frame.end.time - child_frame.begin.time);
  }
  fprintf(m_profilingFile, "\n");
}

void XDebugProfiler::writeCachegrindFuncFileName(const Func* func) {
  if (func->isBuiltin()) {
    fprintf(m_profilingFile, "fl=php:internal\n");
  } else {
    fprintf(m_profilingFile, "fl=%s\n", func->filename()->data());
  }
}

void XDebugProfiler::writeCachegrindFuncName(const Func* func,
                                             bool isTopPseudoMain) {
  if (isTopPseudoMain) {
    fprintf(m_profilingFile, "fn={main}\n");
  } else if (func->isPseudoMain()) {
    fprintf(m_profilingFile, "fn=include::%s\n", func->filename()->data());
  } else if (func->isBuiltin()) {
    fprintf(m_profilingFile, "fn=php::%s\n", func->fullName()->data());
  } else {
    fprintf(m_profilingFile, "fn=%s\n", func->fullName()->data());
  }
}

///////////////////////////////////////////////////////////////////////////////
}

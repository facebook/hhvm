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
#include "hphp/runtime/ext/xdebug/xdebug_utils.h"

#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/ext/hotprofiler/ext_hotprofiler.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/timer.h"

namespace HPHP {

void XDebugProfiler::ensureBufferSpace() {
  if (m_nextFrameIdx < m_frameBufferSize) {
    return;
  }

  // The initial buffer size is 0
  int64_t new_buf_size = (m_frameBufferSize == 0)?
    XDEBUG_GLOBAL(FramebufSize) :
    m_frameBufferSize * XDEBUG_GLOBAL(FramebufExpansion);

  try {
    int64_t new_buf_bytes = new_buf_size * sizeof(FrameData);
    m_frameBuffer = (FrameData*) req::realloc((void*) m_frameBuffer,
                                               new_buf_bytes);
    m_frameBufferSize = new_buf_size;
  } catch (const OutOfMemoryException& e) {
    raise_error("Cannot allocate more memory for the xdebug profiler. Consider "
                "turning off profiling or tracing. Note that certain ini "
                "settings such as xdebug.collect_memory and "
                "xdebug.collect_time implicitly turn on tracing, so turn those "
                " off if this is unexpected.\n"
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

  // The function reference and call file/line are stored when tracing/profiling
  // on function enter
  if ((m_tracingEnabled || m_profilingEnabled) && is_func_begin) {
    frameData.func = fp->func();

    // Need the previous frame in order to get the call line. If we cannot
    // get the previous frame, default to 1
    Offset offset;
    const ActRec* prevFp = g_context->getPrevVMState(fp, &offset);
    if (prevFp != nullptr) {
      frameData.line = prevFp->unit()->getLineNumber(offset);
    } else {
      frameData.line = 1;
    }
  } else {
    frameData.func = nullptr;
    frameData.line = 1;
  }

  // Time is stored if profiling, tracing, or collect_time is enabled, but it
  // only needs to be collected on function exit if profiling or if computerized
  // tracing output is enabled
  if (m_profilingEnabled ||
      (is_func_begin && (m_collectTime || m_tracingEnabled)) ||
      (m_tracingEnabled && (m_tracingOpts & k_XDEBUG_TRACE_COMPUTERIZED))) {
    frameData.time = Timer::GetCurrentTimeMicros();
  } else {
    frameData.time = 0;
  }

  // Memory usage is stored on function begin if tracing, or if collect_memory
  // is enabled, or on function end if computerized tracing output is enabled
  if ((is_func_begin && (m_tracingEnabled || m_collectMemory)) ||
      (m_tracingEnabled && (m_tracingOpts & k_XDEBUG_TRACE_COMPUTERIZED))) {
    frameData.memory_usage = MM().getStats().usage;
  } else {
    frameData.memory_usage = 0;
  }

  // If tracing is enabled, we may need to collect a serialized version of
  // the arguments or the return value.
  if (m_tracingEnabled && is_func_begin && XDEBUG_GLOBAL(CollectParams) > 0) {
    // TODO(#3704) This relies on xdebug_var_dump
    throw_not_implemented("Tracing with collect_params enabled");
  } else if (m_tracingEnabled && !is_func_begin &&
             XDEBUG_GLOBAL(CollectReturn)) {
    // TODO(#3704) This relies on xdebug_var_dump
    throw_not_implemented("Tracing with collect_return enabled");
  }
}

void XDebugProfiler::recordFrame(const TypedValue* retVal) {
  ensureBufferSpace();
  FrameData* fd = &m_frameBuffer[m_nextFrameIdx++];
  collectFrameData(*fd, retVal);
}

void XDebugProfiler::beginFrame(const char *symbol) {
  assert(isNeeded());

  // Check the stack depth, abort if we've reached the limit
  m_depth++;
  if (m_maxDepth != 0 && m_depth >= m_maxDepth) {
    raise_error("Maximum function nesting level of '%lu' reached, aborting!",
                m_maxDepth);
  }

  // Record the frame if we are collecting
  if (isCollecting()) {
    recordFrame(nullptr);
  }
}

void XDebugProfiler::endFrame(const TypedValue* retVal,
                              const char *symbol,
                              bool endMain /* = false */) {
  assert(isNeeded());
  m_depth--;

  if (isCollecting()) {
    // If tracing or profiling are enabled, we need to store end frames as well.
    // Otherwise we can just overwrite the most recent begin frame
    if (m_tracingEnabled || m_profilingEnabled) {
      recordFrame(retVal);
    } else {
      m_nextFrameIdx--;
    }
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

// TODO(#3704) If we aren't profiling, we should try to save space by
// removing unneeded trace data
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
    case TraceOutputType::COMPUTERIZED:
      fprintf(m_tracingFile, "Version: %s\n", XDEBUG_VERSION);
      fprintf(m_tracingFile, "File format: 2\n");
      /* fall through */
    case TraceOutputType::NORMAL:
      fprintf(m_tracingFile, "TRACE START ");
      XDebugUtils::fprintTimestamp(m_tracingFile);
      fprintf(m_tracingFile, "\n");
      break;
    case TraceOutputType::HTML:
      fprintf(m_tracingFile,
        "<table class='xdebug-trace' dir='ltr' border='1' cellspacing='0'>\n"
          "<tr>"
            "<th>#</th>"
            "<th>Time</th>"
            "<th>Mem</th>"
            "<th colspan='2'>Function</th>"
            "<th>Location</th>"
          "</tr>\n");
      break;
  }
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingResultsFooter() {
  switch (outputType) {
    case TraceOutputType::NORMAL:
    case TraceOutputType::COMPUTERIZED:
      fprintf(m_tracingFile, "TRACE END   ");
      XDebugUtils::fprintTimestamp(m_tracingFile);
      fprintf(m_tracingFile, "\n");
      break;
    case TraceOutputType::HTML:
      fprintf(m_tracingFile, "</table>");
      break;
  }
}

template<XDebugProfiler::TraceOutputType outputType>
int64_t XDebugProfiler::writeTracingFrame(int64_t level,
                                          int64_t startIdx,
                                          const FrameData* parentBegin) {
  uint64_t id = m_tracingNextFrameId++;
  FrameData& begin = m_frameBuffer[startIdx];
  assert(begin.is_func_begin);

  writeTracingLinePrefix<outputType>();
  writeTracingLevel<outputType>(level);
  writeTracingFrameId<outputType>(id);
  writeTracingTime<outputType>(begin.time);
  writeTracingMemory<outputType>(begin.memory_usage);
  writeTracingIndent<outputType>(level);
  writeTracingFunc<outputType>(begin, level == 0);
  writeTracingCallsite<outputType>(begin, parentBegin);
  writeTracingLineSuffix<outputType>();
  fprintf(m_tracingFile, "\n");

  // This is needed to determine the delta memory usage
  m_tracingPrevBegin = &begin;

  // Iterate over children
  int64_t buf_idx = startIdx + 1;
  while (buf_idx < m_nextFrameIdx) {
    // TODO(#3704) If collect_return and !is_func_begin, write return value
    FrameData& cur_frame = m_frameBuffer[buf_idx];
    if (!cur_frame.is_func_begin) {
      writeTracingEndFrame<outputType>(cur_frame, level, id);
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
void XDebugProfiler::writeTracingLinePrefix() {
  if (outputType == TraceOutputType::HTML) {
      fprintf(m_tracingFile, "\t<tr>");
  }
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingLevel(int64_t level) {
  if (outputType == TraceOutputType::COMPUTERIZED) {
      fprintf(m_tracingFile, "%ld\t", level);
  }
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingFrameId(uint64_t id) {
  switch (outputType) {
    case TraceOutputType::NORMAL:
      break;
    case TraceOutputType::HTML:
      fprintf(m_tracingFile, "<td>%ld</td>", id);
      break;
    case TraceOutputType::COMPUTERIZED:
      fprintf(m_tracingFile, "%ld\t", id);
      fprintf(m_tracingFile, "0\t"); // func exit column
      break;
  }
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingTime(int64_t time) {
  switch (outputType) {
    case TraceOutputType::NORMAL:
      fprintf(m_tracingFile, "%10.4f ", timeSinceBase(time));
      break;
    case TraceOutputType::HTML:
      fprintf(m_tracingFile, "<td>%0.6f</td>", timeSinceBase(time));
      break;
    case TraceOutputType::COMPUTERIZED:
      fprintf(m_tracingFile, "%f\t", timeSinceBase(time));
      break;
  }
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingMemory(int64_t memory) {
  switch (outputType) {
    case TraceOutputType::NORMAL:
      fprintf(m_tracingFile, "%10lu ", memory);
      // Delta Memory (since the last begin frame)
      if (XDEBUG_GLOBAL(ShowMemDelta)) {
        if (m_tracingPrevBegin != nullptr) {
          int64_t prev_usage = m_tracingPrevBegin->memory_usage;
          fprintf(m_tracingFile, "%+8ld", memory - prev_usage);
        } else {
          fprintf(m_tracingFile, "        ");
        }
      }
      break;
    case TraceOutputType::HTML:
      fprintf(m_tracingFile, "<td align='right'>%ld</td>", memory);
      break;
    case TraceOutputType::COMPUTERIZED:
      fprintf(m_tracingFile, "%lu\t", memory);
      break;
  }
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingIndent(int64_t level) {
  switch (outputType) {
    case TraceOutputType::NORMAL:
      for (int i = 0; i < level + 1; i++) {
        fprintf(m_tracingFile, "  ");
      }
      fprintf(m_tracingFile, "-> ");
      break;
    case TraceOutputType::HTML:
      fprintf(m_tracingFile, "<td align='left'>");
      for (int i = 0; i < level; i++) {
        fprintf(m_tracingFile, "&nbsp; &nbsp;");
      }
      fprintf(m_tracingFile, "-&gt;</td>");
    case TraceOutputType::COMPUTERIZED:
      break;
  }
}

void XDebugProfiler::writeTracingFuncName(const Func* func,
                                          bool isTopPseudoMain) {
  if (isTopPseudoMain) {
    fprintf(m_tracingFile, "main");
  } else if (func->isPseudoMain()) {
    fprintf(m_tracingFile, "include");
  } else {
    fprintf(m_tracingFile, "%s", func->fullName()->data());
  }
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingFunc(FrameData& frame,
                                      bool isTopPseudoMain) {
  if (outputType == TraceOutputType::HTML) {
    fprintf(m_tracingFile, "<td>");
  }

  // TODO(#3704) Support collect_params output
  writeTracingFuncName(frame.func, isTopPseudoMain);
  switch (outputType) {
    case TraceOutputType::HTML:
    case TraceOutputType::NORMAL:
      if (!isTopPseudoMain && frame.func->isPseudoMain()) {
        fprintf(m_tracingFile, "(%s) ", frame.func->filename()->data());
      } else {
        fprintf(m_tracingFile, "() ");
      }
      break;
    case TraceOutputType::COMPUTERIZED:
      fprintf(m_tracingFile, "\t");
      // User defined/internal
      if (frame.func->isBuiltin()) {
        fprintf(m_tracingFile, "%d\t", 0);
      } else {
        fprintf(m_tracingFile, "%d\t", 1);
      }
      // Include file
      if (!isTopPseudoMain && frame.func->isPseudoMain()) {
        fprintf(m_tracingFile, "%s\t", frame.func->filename()->data());
      } else {
        fprintf(m_tracingFile, "\t");
      }
      break;
  }

  if (outputType == TraceOutputType::HTML) {
      fprintf(m_tracingFile, "</td>");
  }
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingCallsite(FrameData& frame,
                                          const FrameData* parent) {
  switch (outputType) {
    case TraceOutputType::NORMAL:
      if (parent != nullptr) {
        fprintf(m_tracingFile, "%s", parent->func->filename()->data());
        fprintf(m_tracingFile, ":%d", frame.line);
      }
      break;
    case TraceOutputType::HTML:
      fprintf(m_tracingFile, "<td>");
      fprintf(m_tracingFile, "%s", parent->func->filename()->data());
      fprintf(m_tracingFile, ":%d", frame.line);
      fprintf(m_tracingFile, "</td>");
      break;
    case TraceOutputType::COMPUTERIZED:
      if (parent != nullptr) {
        fprintf(m_tracingFile, "%s\t", parent->func->filename()->data());
        fprintf(m_tracingFile, "%d\t", frame.line);
      } else {
        fprintf(m_tracingFile, "\t\t");
      }
      break;
  }
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingLineSuffix() {
  if (outputType == TraceOutputType::HTML) {
      fprintf(m_tracingFile, "</tr>");
  }
}

template<XDebugProfiler::TraceOutputType outputType>
void XDebugProfiler::writeTracingEndFrame(FrameData& frame,
                                          int64_t level,
                                          int64_t id) {
  if (outputType != TraceOutputType::COMPUTERIZED) {
    return;
  }

  // Computerized output has a line for exit events
  fprintf(m_tracingFile, "%ld\t", level);
  fprintf(m_tracingFile, "%ld\t", id);
  fprintf(m_tracingFile, "1\t"); // frame exit column
  fprintf(m_tracingFile, "%f\t", timeSinceBase(frame.time));
  fprintf(m_tracingFile, "%ld\n", frame.memory_usage);
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
  std::vector<Frame> children;
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
                                          const std::vector<Frame>& children,
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

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
#include "hphp/util/stack-trace.h"

#include <fstream>
#include <mutex>
#include <set>
#include <string>

#include <execinfo.h>
#include <sys/types.h>
#include <fcntl.h>

#include <folly/Conv.h>
#include <folly/Demangle.h>
#include <folly/FileUtil.h>
#include <folly/Format.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>

#include "hphp/util/assertions.h"
#include "hphp/util/conv-10.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/process.h"
#include "hphp/util/thread-local.h"

#include <folly/debugging/symbolizer/Symbolizer.h>

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

const char* const s_defaultIgnorelist[] = {
  "_ZN4HPHP16StackTraceNoHeap",
  "_ZN5folly10symbolizer17getStackTraceSafeEPmm",
};

bool StackTraceBase::Enabled = true;

const char* const* StackTraceBase::FunctionIgnorelist = s_defaultIgnorelist;
unsigned StackTraceBase::FunctionIgnorelistCount = 2;

////////////////////////////////////////////////////////////////////////////////

namespace {

void printStr(int fd, folly::StringPiece s) {
  write(fd, s.begin(), s.size());
}

void printInt(int fd, int64_t val) {
  char buf[24];
  printStr(fd, conv_10(val, buf + sizeof buf));
}

void printFrameHdr(int fd, int fr) {
  printStr(fd, "# ");
  printInt(fd, fr);
  printStr(fd, (fr < 10 ? "  " : " "));
}

void printPair(int fd,
               folly::StringPiece first,
               folly::StringPiece second) {
  printStr(fd, first);
  write(fd, ": ", 2);
  printStr(fd, second);
  write(fd, "\n", 1);
}

void printPair(int fd,
               folly::StringPiece first,
               int64_t second) {
  char buf[24];
  printPair(fd, first, conv_10(second, buf + sizeof buf));
}

/*
 * Writes a file path to a file while folding any consecutive forward slashes.
 */
void write_path(int fd, folly::StringPiece path) {
  while (!path.empty()) {
    auto const pos = path.find('/');
    if (pos == std::string::npos) {
      folly::writeNoInt(fd, path.data(), path.size());
      break;
    }

    auto const left = path.subpiece(0, pos + 1);
    folly::writeNoInt(fd, left.data(), left.size());

    auto right = path.subpiece(pos);
    while (!right.empty() && right[0] == '/') {
      right = right.subpiece(1);
    }

    path = right;
  }
}

bool isIgnorelisted(const char* funcname) {
  for (int i = 0; i < StackTraceBase::FunctionIgnorelistCount; i++) {
    auto ignoreFunc = StackTraceBase::FunctionIgnorelist[i];
    if (strncmp(funcname, ignoreFunc, strlen(ignoreFunc)) == 0) {
      return true;
    }
  }
  return false;
}

/*
 * Demangle a function name and return it as a string.
 */
std::string demangle(const char* mangled) {
  auto const skip_first =
    static_cast<int>(mangled[0] == '.' || mangled[0] == '$');

  auto const result = folly::demangle(mangled + skip_first);
  auto const ret = std::string(result.data(), result.size());
  if (mangled[0] == '.') {
    return "." + ret;
  }

  return ret;
}

/*
 * Demangle a function name and write it to a file.
 */
void demangle(int fd, const char* mangled) {
  if (mangled == nullptr || mangled[0] == '\0') {
    write(fd, "??", 2);
    return;
  }

  auto const skip_first =
    static_cast<int>(mangled[0] == '.' || mangled[0] == '$');

  char buf[2048];
  auto sz = folly::demangle(mangled + skip_first, buf, sizeof buf);
  if (sz > sizeof buf) {
    write(fd, mangled, strlen(mangled));
    return;
  }

  if (mangled[0] == '.') write(fd, ".", 1);
  write(fd, buf, sz);
}

std::mutex s_perfMapCacheMutex;
StackTrace::PerfMap s_perfMapCache;
std::set<void*> s_perfMapNegCache;

void translateFromPerfMap(StackFrameExtra* frame) {
  std::lock_guard<std::mutex> lock(s_perfMapCacheMutex);

  if (s_perfMapCache.translate(frame)) {
    if (s_perfMapNegCache.contains(frame->addr)) {
      // A prior failed lookup of frame already triggered a rebuild.
      return;
    }
    // Rebuild the cache, then search again.
    s_perfMapCache.rebuild();
    if (s_perfMapCache.translate(frame)) {
      s_perfMapNegCache.insert(frame->addr);
    }
  }
}

template <bool safe>
int ALWAYS_INLINE get_backtrace(void** frame, int max) {
  int ret = 0;

  if (safe) {
    ret = folly::symbolizer::getStackTraceSafe((uintptr_t*)frame, max);
  } else {
    ret = folly::symbolizer::getStackTrace((uintptr_t*)frame, max);
  }

  if (ret < 0 || ret > max) {
    return 0;
  }
  return ret;
}

}

struct StackTraceLog {
  hphp_string_map<std::string> data;

  static THREAD_LOCAL(StackTraceLog, s_logData);
};
THREAD_LOCAL(StackTraceLog, StackTraceLog::s_logData);

////////////////////////////////////////////////////////////////////////////////

StackTrace::StackTrace(bool trace) {
  if (trace && Enabled) {
    m_frames.resize(kMaxFrame);
    m_frames.resize(get_backtrace<false>(m_frames.data(), kMaxFrame));
  }
}

StackTrace::StackTrace(StackTrace::Force) {
  m_frames.resize(kMaxFrame);
  m_frames.resize(get_backtrace<false>(m_frames.data(), kMaxFrame));
}

StackTrace::StackTrace(void* const* ips, size_t count) {
  m_frames.resize(count);
  m_frames.assign(ips, ips + count);
}

StackTrace::StackTrace(folly::StringPiece hexEncoded) {
  // Can't split into StringPieces, strtoll() expects a null terminated string.
  std::vector<std::string> frames;
  folly::split(':', hexEncoded, frames);
  for (auto const& frame : frames) {
    m_frames.push_back((void*)strtoll(frame.c_str(), nullptr, 16));
  }
}

void StackTrace::get(std::vector<std::shared_ptr<StackFrameExtra>>&
                     frames) const {
  frames.clear();
  for (auto const frame_ptr : m_frames) {
    frames.push_back(Translate(frame_ptr));
  }
}

std::string StackTrace::hexEncode(int minLevel /* = 0 */,
                                  int maxLevel /* = 999 */) const {
  std::string bts;
  for (int i = minLevel; i < (int)m_frames.size() && i < maxLevel; i++) {
    if (i > minLevel) bts += ':';
    char buf[20];
    snprintf(buf, sizeof(buf), "%" PRIx64, (int64_t)m_frames[i]);
    bts.append(buf);
  }
  return bts;
}

const std::string& StackTrace::toString(int skip, int limit) const {
  auto usable = skip == 0 && limit == -1;
  if (!usable || !m_trace_usable) {
    m_trace.clear();
  }

  if (!m_trace.empty()) return m_trace;

  size_t frame = 0;
  for (auto const frame_ptr : m_frames) {
    auto const framename = Translate(frame_ptr)->toString();
    // Ignore frames in the StackTrace class.
    if (framename.find("StackTrace::") != std::string::npos) {
      continue;
    }
    if (skip-- > 0) continue;
    m_trace += "# ";
    m_trace += folly::to<std::string>(frame);
    if (frame < 10) m_trace += " ";

    m_trace += " ";
    m_trace += framename;
    m_trace += "\n";
    ++frame;
    if ((int)frame == limit) break;
  }

  m_trace_usable = usable;
  return m_trace;
}

void StackTrace::PerfMap::rebuild() {
  m_map.clear();

  char filename[64];
  snprintf(filename, sizeof(filename), "/tmp/perf-%d.map", getpid());

  std::ifstream perf_map(filename);
  if (!perf_map) return;

  std::string line;

  while (!perf_map.bad() && !perf_map.eof()) {
    if (!std::getline(perf_map, line)) continue;

    uintptr_t addr;
    uint32_t size;
    int name_pos = 0;
    // We compare < 2 because some implementations of sscanf() increment the
    // assignment count for %n against the specification.
    if (sscanf(line.c_str(), "%lx %x %n", &addr, &size, &name_pos) < 2 ||
        name_pos < 4 /* not assigned, or not enough spaces */) {
      continue;
    }
    if (name_pos >= line.size()) continue;

    auto const past = addr + size;
    auto const range = PerfMap::Range { addr, past };

    m_map[range] = line.substr(name_pos);
  }

  m_built = true;
}

bool StackTrace::PerfMap::translate(StackFrameExtra* frame) const {
  if (!m_built) {
    const_cast<StackTrace::PerfMap*>(this)->rebuild();
  }

  // Use a key with non-zero span, because otherwise a key right at the base of
  // a range will be treated as before the range (bad) rather than within it
  // (good).
  PerfMap::Range key{uintptr_t(frame->addr), uintptr_t(frame->addr)+1};
  auto const& it = m_map.find(key);
  if (it == m_map.end()) {
    // Not found.
    frame->filename = "?";
    frame->funcname = "TC?"; // Note HHProf::HandlePProfSymbol() dependency.
    return true;
  }

  // Found.
  auto const& filefunc = it->second;
  auto const colon_pos = filefunc.find("::");
  if (colon_pos == std::string::npos) {
    frame->funcname = filefunc;
    return false;
  }

  auto const prefix = std::string(filefunc, 0, colon_pos);
  if (prefix == "HHVM") {
    // HHVM unique stubs are simply "HHVM::stubName".
    frame->funcname = filefunc + "()";
    return false;
  }

  if (prefix != "PHP") {
    // We don't recognize the prefix, so don't do any munging.
    frame->funcname = filefunc;
    return false;
  }

  // Jitted PHP functions have the format "PHP::file.php::Full::funcName".
  auto const file_pos = colon_pos + 2;
  auto const file_end_pos = filefunc.find("::", file_pos);
  if (file_end_pos == std::string::npos) {
    // Bad PHP function descriptor.
    frame->funcname = filefunc;
    return false;
  }
  frame->filename = std::string(filefunc, file_pos, file_end_pos - file_pos);
  frame->funcname = "PHP::" + std::string(filefunc, file_end_pos + 2) + "()";
  return false;
}

////////////////////////////////////////////////////////////////////////////////

std::string StackFrameExtra::toString() const {
  constexpr folly::StringPiece qq{"??"};
  return folly::sformat(
    "{} at {}:{}",
    funcname.empty() ? qq : folly::StringPiece{funcname},
    filename.empty() ? qq : folly::StringPiece{filename},
    lineno
  );
}

////////////////////////////////////////////////////////////////////////////////

StackTraceNoHeap::StackTraceNoHeap(bool trace, bool fallback) {
  if (trace && Enabled) {
#if defined __GLIBC__
    if (fallback) {
      m_frame_count = backtrace(m_frames, kMaxFrame);
      return;
    }
#endif
    m_frame_count = get_backtrace<true>(m_frames, kMaxFrame);
  }
}

void StackTraceNoHeap::AddExtraLogging(const char* name,
                                       const std::string& value) {
  assertx(name != nullptr && name[0] != '\0');
  StackTraceLog::s_logData->data[name] = value;
}

void StackTraceNoHeap::ClearAllExtraLogging() {
  StackTraceLog::s_logData->data.clear();
}

void StackTraceNoHeap::log(const char* errorType, int fd, const char* buildId,
                           int debuggerCount) {
  assert(fd >= 0);

  printPair(fd, "Host", Process::GetHostName().c_str());
  printPair(fd, "ProcessID", (int64_t)getpid());
  printPair(fd, "ThreadID", (int64_t)Process::GetThreadId());
  printPair(fd, "ThreadPID", Process::GetThreadPid());
  printPair(fd, "Name", Process::GetAppName().c_str());
  printPair(fd, "CmdLine", Process::GetCommandLine(getpid()).c_str());
  printPair(fd, "Type", errorType ? errorType : "(unknown error)");
  printPair(fd, "Runtime", "hhvm");
  printPair(fd, "Version", buildId);
  printPair(fd, "DebuggerCount", debuggerCount);
  write(fd, "\n", 1);

  for (auto const& pair : StackTraceLog::s_logData->data) {
    printPair(fd, pair.first.c_str(), pair.second.c_str());
  }
  write(fd, "\n", 1);
}

////////////////////////////////////////////////////////////////////////////////

void StackTraceNoHeap::printStackTrace(int fd) const {
  folly::symbolizer::SignalSafeElfCache elfCache;
  folly::symbolizer::Symbolizer symbolizer(&elfCache);
  folly::symbolizer::SymbolizedFrame frames[kMaxFrame];
  symbolizer.symbolize((uintptr_t*)m_frames, frames, m_frame_count);
  for (int i = 0, fr = 0; i < m_frame_count; i++) {
    auto const& frame = frames[i];
    if (!frame.name ||
        !frame.name[0] ||
        isIgnorelisted(frame.name)) {
      continue;
    }
    printFrameHdr(fd, fr);
    demangle(fd, frame.name);
    if (frame.location.hasFileAndLine) {
      char fileBuf[PATH_MAX];
      fileBuf[0] = '\0';
      frame.location.file.toBuffer(fileBuf, sizeof(fileBuf));
      printStr(fd, " at ");
      write_path(fd, fileBuf);
      printStr(fd, ":");
      printInt(fd, frame.location.line);
    }
    printStr(fd, "\n");
    fr++;
  }
}

std::shared_ptr<StackFrameExtra> StackTrace::Translate(void* frame_addr,
                                                       PerfMap* pm) {
  folly::symbolizer::Symbolizer symbolizer;
  folly::symbolizer::SymbolizedFrame sf;
  symbolizer.symbolize((uintptr_t*)&frame_addr, &sf, 1);

  auto frame = std::make_shared<StackFrameExtra>(frame_addr);
  if (!sf.found ||
      !sf.name ||
      !sf.name[0]) {
    // Lookup failed, so this is probably a PHP symbol.  Let's
    // check the perf map.
    if (pm != nullptr) {
      pm->translate(frame.get());
    } else {
      translateFromPerfMap(frame.get());
    }
    return frame;
  }

  if (sf.location.hasFileAndLine) {
    frame->filename = sf.location.file.toString();
    frame->lineno = sf.location.line;
  }

  frame->funcname = sf.name ? demangle(sf.name) : "";

  return frame;
}

////////////////////////////////////////////////////////////////////////////////
}

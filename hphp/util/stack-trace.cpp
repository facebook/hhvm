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
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <folly/Conv.h>
#include <folly/Demangle.h>
#include <folly/FileUtil.h>
#include <folly/Format.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>

#include "hphp/util/assertions.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/conv-10.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/hash.h"
#include "hphp/util/process.h"
#include "hphp/util/thread-local.h"

#if defined USE_FOLLY_SYMBOLIZER

#include <folly/experimental/symbolizer/Symbolizer.h>

#elif defined HAVE_LIBBFD

#include <bfd.h>

#endif

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
    if (s_perfMapNegCache.count(frame->addr)) {
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
#if defined USE_FOLLY_SYMBOLIZER
  if (safe) {
    ret = folly::symbolizer::getStackTraceSafe((uintptr_t*)frame, max);
  } else {
    ret = folly::symbolizer::getStackTrace((uintptr_t*)frame, max);
  }
#elif defined __GLIBC__
  ret = backtrace(frame, max);
#endif
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

StackTraceBase::StackTraceBase() {
#if !defined USE_FOLLY_SYMBOLIZER && defined HAVE_LIBBFD
  bfd_init();
#endif
}

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

#if defined USE_FOLLY_SYMBOLIZER

void StackTraceNoHeap::printStackTrace(int fd) const {
  folly::symbolizer::Symbolizer symbolizer;
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

#elif defined HAVE_LIBBFD

namespace {
////////////////////////////////////////////////////////////////////////////////

constexpr int MaxKey = 100;

struct BfdCache {
  bfd* abfd{nullptr};
  asymbol** syms{nullptr};

  ~BfdCache() {
    if (abfd == nullptr) return;
    bfd_cache_close(abfd);
    bfd_free_cached_info(abfd);
    bfd_close_all_done(abfd);
  }
};

struct NamedBfd {
  BfdCache bc;
  char key[MaxKey];
};

using NamedBfdRange = folly::Range<NamedBfd*>;

struct Addr2lineData {
  asymbol** syms;
  bfd_vma pc;
  const char* filename{nullptr};
  const char* functionname{nullptr};
  unsigned line{0};
  bfd_boolean found{FALSE};
};

using BfdMap = hphp_hash_map<
  std::string,
  std::shared_ptr<BfdCache>,
  string_hash
>;

/*
 * We cache opened bfd file pointers that in turn cache frame pointer lookup
 * tables.
 */
std::mutex s_bfdMutex;
BfdMap s_bfds;

////////////////////////////////////////////////////////////////////////////////

/* Copied and re-factored from addr2line. */

void find_address_in_section(bfd* abfd, asection* section, void* data) {
  auto adata = reinterpret_cast<Addr2lineData*>(data);
  if (adata->found) {
    return;
  }

  if ((bfd_get_section_flags(abfd, section) & SEC_ALLOC) == 0) {
    return;
  }

  auto const vma = bfd_get_section_vma(abfd, section);
  if (adata->pc < vma) {
    return;
  }

  auto const size = bfd_get_section_size(section);
  if (adata->pc >= vma + size) {
    return;
  }

  // libdwarf allocates its own unaligned memory so it doesn't play well with
  // valgrind.
#ifndef VALGRIND
  adata->found = bfd_find_nearest_line(
    abfd, section, adata->syms, adata->pc - vma, &adata->filename,
    &adata->functionname, &adata->line
  );
#endif

  if (adata->found) {
    auto file = adata->filename;
    auto line = adata->line;
    bfd_boolean found = TRUE;
    while (found) {
      found = bfd_find_inliner_info(abfd, &file, &adata->functionname, &line);
    }
  }
}

bool slurp_symtab(asymbol*** syms, bfd* abfd) {
  unsigned size;

  auto symcount = bfd_read_minisymbols(abfd, FALSE, (void**)syms, &size);
  if (symcount == 0) {
    symcount = bfd_read_minisymbols(
      abfd, TRUE /* dynamic */, (void**)syms, &size
    );
  }
  return symcount >= 0;
}

bool translate_addresses(bfd* abfd, const char* addr, Addr2lineData* adata) {
  if (abfd == nullptr) return false;
  adata->pc = bfd_scan_vma(addr, nullptr, 16);

  adata->found = FALSE;
  bfd_map_over_sections(abfd, find_address_in_section, adata);

  if (!adata->found || !adata->functionname || !*adata->functionname) {
    return false;
  }
  return true;
}

bool fill_bfd_cache(folly::StringPiece filename, BfdCache& p) {
  // Hard to avoid heap here!
  auto abfd = bfd_openr(filename.begin(), nullptr);
  if (abfd == nullptr) return true;

  // Some systems don't have the BFD_DECOMPRESS flag.
#ifdef BFD_DECOMPRESS
  abfd->flags |= BFD_DECOMPRESS;
#endif

  p.abfd = nullptr;
  p.syms = nullptr;
  char** match;
  if (bfd_check_format(abfd, bfd_archive) ||
      !bfd_check_format_matches(abfd, bfd_object, &match) ||
      !slurp_symtab(&p.syms, abfd)) {
    bfd_close(abfd);
    return true;
  }
  p.abfd = abfd;
  return false;
}

std::shared_ptr<BfdCache> get_bfd_cache(folly::StringPiece filename) {
  // Heterogeneous lookup is in C++14.  Otherwise we'll end up making a
  // std::string copy.
  auto iter = std::find_if(
    s_bfds.begin(),
    s_bfds.end(),
    [&] (const BfdMap::value_type& pair) { return pair.first == filename; }
  );

  if (iter != s_bfds.end()) {
    return iter->second;
  }

  auto p = std::make_shared<BfdCache>();
  if (fill_bfd_cache(filename, *p)) {
    p.reset();
  }
  s_bfds[filename.str()] = p;
  return p;
}

BfdCache* get_bfd_cache(folly::StringPiece filename, NamedBfdRange bfds) {
  auto probe = hash_string_cs(filename.begin(), filename.size()) % bfds.size();

  // Match on the end of filename instead of the beginning, if necessary.
  if (filename.size() >= MaxKey) {
    filename = filename.subpiece(filename.size() - MaxKey + 1);
  }

  while (bfds[probe].key[0] && strcmp(filename.begin(), bfds[probe].key) != 0) {
    probe = probe ? probe-1 : bfds.size()-1;
  }

  auto p = &bfds[probe].bc;
  if (bfds[probe].key[0]) return p;

  assert(filename.size() < MaxKey);
  assert(filename.begin()[filename.size()] == 0);
  // Accept the rare collision on keys (requires probe collision too).
  memcpy(bfds[probe].key, filename.begin(), filename.size() + 1);
  fill_bfd_cache(filename, *p);
  return p;
}

/*
 * Run addr2line to translate a function pointer into function name and line
 * number.
 */
bool addr2line(folly::StringPiece filename, StackFrame* frame,
               Addr2lineData* data, NamedBfdRange bfds) {
  char address[32];
  snprintf(address, sizeof(address), "%p", frame->addr);

  std::lock_guard<std::mutex> lock(s_bfdMutex);
  data->filename = nullptr;
  data->functionname = nullptr;
  data->line = 0;
  bool ret;

  if (bfds.empty()) {
    auto p = get_bfd_cache(filename);
    data->syms = p->syms;
    ret = translate_addresses(p->abfd, address, data);
  } else {
    // Don't let shared_ptr malloc behind the scenes in this case.
    auto q = get_bfd_cache(filename, bfds);
    data->syms = q->syms;
    ret = translate_addresses(q->abfd, address, data);
  }

  if (ret) {
    frame->lineno = data->line;
  }
  return ret;
}

/*
 * Translate a frame pointer to file name and line number pair.
 */
bool translate(StackFrame* frame, Dl_info& dlInfo, Addr2lineData* data,
               NamedBfdRange bfds = NamedBfdRange()) {
  if (!dladdr(frame->addr, &dlInfo)) {
    return false;
  }

  // Frame pointer offset in previous frame.
  frame->offset = (char*)frame->addr - (char*)dlInfo.dli_saddr;

  if (dlInfo.dli_fname) {
    // First attempt without offsetting base address.
    if (!addr2line(dlInfo.dli_fname, frame, data, bfds) &&
        dlInfo.dli_fname && strstr(dlInfo.dli_fname, ".so")) {
      // Offset shared lib's base address.

      frame->addr = (char*)frame->addr - (size_t)dlInfo.dli_fbase;

      // Use addr2line to get line number info.
      addr2line(dlInfo.dli_fname, frame, data, bfds);
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Variant of translate() used by StackTraceNoHeap.
 */
bool translate(int fd, void* frame_addr, int frame_num, NamedBfdRange bfds) {
  // Frame pointer offset in previous frame.
  Dl_info dlInfo;
  Addr2lineData adata;
  StackFrame frame(frame_addr);
  if (!translate(&frame, dlInfo, &adata, bfds)) {
    return false;
  }

  auto filename = adata.filename ? adata.filename : dlInfo.dli_fname;
  if (filename == nullptr) filename = "??";
  auto funcname = adata.functionname ? adata.functionname : dlInfo.dli_sname;
  if (funcname == nullptr) funcname = "??";

  // Ignore some frames that are always present.
  if (isIgnorelisted(funcname)) return false;

  printFrameHdr(fd, frame_num);
  demangle(fd, funcname);
  printStr(fd, " at ");
  write_path(fd, filename);
  printStr(fd, ":");
  printInt(fd, frame.lineno);
  printStr(fd, "\n");

  return true;
}

////////////////////////////////////////////////////////////////////////////////
}

std::shared_ptr<StackFrameExtra> StackTrace::Translate(void* frame_addr,
                                                       PerfMap* pm) {
  Dl_info dlInfo;
  Addr2lineData adata;

  auto frame = std::make_shared<StackFrameExtra>(frame_addr);
  if (!translate(frame.get(), dlInfo, &adata)) {
    // Lookup using dladdr() failed, so this is probably a PHP symbol.  Let's
    // check the perf map.
    if (pm != nullptr) {
      pm->translate(frame.get());
    } else {
      translateFromPerfMap(frame.get());
    }
    return frame;
  }

  if (adata.filename) {
    frame->filename = adata.filename;
  }
  if (adata.functionname) {
    frame->funcname = demangle(adata.functionname);
  }
  if (frame->filename.empty() && dlInfo.dli_fname) {
    frame->filename = dlInfo.dli_fname;
  }
  if (frame->funcname.empty() && dlInfo.dli_sname) {
    frame->funcname = demangle(dlInfo.dli_sname);
  }

  return frame;
}

void StackTraceNoHeap::printStackTrace(int fd) const {
  // m_frame_count must be an upper bound on the number of filenames then *2 for
  // tolerable hash table behavior.
  auto const size = m_frame_count * 2;

  // Using the heap in "NoHeap" is bad but we do it anyway.
  const std::unique_ptr<NamedBfd[]> bfds(new NamedBfd[size]);
  for (unsigned i = 0; i < size; i++) {
    bfds.get()[i].key[0] = '\0';
  }

  int frame = 0;
  for (unsigned i = 0; i < m_frame_count; i++) {
    auto range = NamedBfdRange(bfds.get(), size);
    if (translate(fd, m_frames[i], frame, range)) {
      frame++;
    }
  }
  // ~bfds[i].bc here (unlike the heap case).
}

////////////////////////////////////////////////////////////////////////////////

#else // No libbfd or folly::Symbolizer

namespace {

void printHex(int fd, uint64_t val) {
  char buf[16];
  auto ptr = buf + sizeof buf;

  while (ptr > buf) {
    auto ch = val & 0xf;
    *--ptr = ch + (ch >= 10 ? 'a' - 10 : '0');
    val >>= 4;
  }

  printStr(fd, folly::StringPiece(buf, sizeof buf));
}

}

std::shared_ptr<StackFrameExtra> StackTrace::Translate(void* bt, PerfMap* pm) {
  return std::make_shared<StackFrameExtra>(bt);
}

void StackTraceNoHeap::printStackTrace(int fd) const {
  for (int i = 0; i < m_frame_count; i++) {
    printStr(fd, "# ");
    printInt(fd, i);
    printStr(fd, (i < 10 ? "  " : " "));
    printHex(fd, (uintptr_t)m_frames[i]);
    printStr(fd, "\n");
  }
}

#endif

////////////////////////////////////////////////////////////////////////////////
}

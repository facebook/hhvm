/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <mutex>
#include <set>

#if (!defined(__CYGWIN__) && !defined(__MINGW__) && !defined(_MSC_VER))
#include <execinfo.h>
#endif

#ifdef HAVE_LIBBFD
#include <bfd.h>
#endif

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <folly/Conv.h>
#include <folly/Format.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>

#include "hphp/util/assertions.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/hash-map-typedefs.h"
#include "hphp/util/hash.h"
#include "hphp/util/process.h"
#include "hphp/util/thread-local.h"

namespace HPHP {
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

#ifdef HAVE_LIBBFD

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

std::mutex s_perfMapCacheMutex;
StackTrace::PerfMap s_perfMapCache;
std::set<void*> s_perfMapNegCache;

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
  auto probe = hash_string(filename.begin()) % bfds.size();

  // Match on the end of filename instead of the beginning, if necessary.
  if (filename.size() > MaxKey) {
    filename = filename.subpiece(filename.size() - MaxKey);
  }

  while (bfds[probe].key[0] && strcmp(filename.begin(), bfds[probe].key) != 0) {
    probe = probe ? probe-1 : bfds.size()-1;
  }

  auto p = &bfds[probe].bc;
  if (bfds[probe].key[0]) return p;

  // Accept the rare collision on keys (requires probe collision too).
  // FIXME: This is a NUL terminator bug waiting to blow up.
  strncpy(bfds[probe].key, filename.begin(), filename.size());
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

/* Copied and re-factored from demangle/c++filt. */

#define DMGL_PARAMS   (1 << 0)  /* Include function args */
#define DMGL_ANSI     (1 << 1)  /* Include const, volatile, etc */
#define DMGL_VERBOSE  (1 << 3)  /* Include implementation details. */

extern "C" char* cplus_demangle(const char* mangled, int options);

auto constexpr demangle_opt = DMGL_PARAMS | DMGL_ANSI | DMGL_VERBOSE;

/*
 * Demangle a function name and return it as a string.
 */
std::string demangle(const char* mangled) {
  if (mangled == nullptr || mangled[0] == '\0') {
    return "";
  }

  size_t skip_first = 0;
  if (mangled[0] == '.' || mangled[0] == '$') ++skip_first;

  auto result = cplus_demangle(mangled + skip_first, demangle_opt);
  if (result == nullptr) return mangled;
  SCOPE_EXIT { free(result); };

  std::string ret;
  if (mangled[0] == '.') ret += '.';
  ret += result;
  return ret;
}

/*
 * Demangle a function name and write it to a file.
 */
void demangle(int fd, const char* mangled) {
  if (mangled == nullptr || mangled[0] == '\0') {
    dprintf(fd, "??");
    return;
  }

  size_t skip_first = 0;
  if (mangled[0] == '.' || mangled[0] == '$') ++skip_first;

  auto result = cplus_demangle(mangled + skip_first, demangle_opt);
  SCOPE_EXIT { free(result); };

  if (result == nullptr) {
    dprintf(fd, "%s", mangled);
    return;
  }
  dprintf(fd, "%s%s", mangled[0] == '.' ? "." : "", result);
}

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
  for (int i = 0; i < StackTraceBase::FunctionBlacklistCount; i++) {
    auto ignoreFunc = StackTraceBase::FunctionBlacklist[i];
    if (strncmp(funcname, ignoreFunc, strlen(ignoreFunc)) == 0) {
      return false;
    }
  }

  dprintf(fd, "# %d%s ", frame_num, frame_num < 10 ? " " : "");
  demangle(fd, funcname);
  dprintf(fd, " at %s:%u\n", filename, frame.lineno);

  return true;
}

////////////////////////////////////////////////////////////////////////////////
}

const char* s_defaultBlacklist[] = {"_ZN4HPHP16StackTraceNoHeap"};

bool StackTraceBase::Enabled = true;

const char** StackTraceBase::FunctionBlacklist = s_defaultBlacklist;
unsigned StackTraceBase::FunctionBlacklistCount = 1;

////////////////////////////////////////////////////////////////////////////////

StackTraceBase::StackTraceBase() {
  bfd_init();
}

StackTrace::StackTrace(bool trace) {
  if (trace && Enabled) {
    create();
  }
}

StackTraceNoHeap::StackTraceNoHeap(bool trace) {
  if (trace && Enabled) {
    create();
  }
}

void StackTrace::initFromHex(folly::StringPiece hexEncoded) {
  // Can't split into StringPieces, strtoll() expects a null terminated string.
  std::vector<std::string> frames;
  folly::split(':', hexEncoded, frames);
  for (auto const& frame : frames) {
    m_frames.push_back((void*)strtoll(frame.c_str(), nullptr, 16));
  }
}

StackTrace::StackTrace(folly::StringPiece hexEncoded) {
  initFromHex(hexEncoded);
}

void StackTrace::create() {
  void* frames[kMaxFrame];
  auto const framecount = backtrace(frames, kMaxFrame);
  if (framecount <= 0 || framecount > (int)kMaxFrame) {
    m_frames.clear();
    return;
  }
  m_frames.resize(framecount);
  for (int i = 0; i < framecount; ++i) {
    m_frames[i] = frames[i];
  }
}

void StackTraceNoHeap::create() {
  // backtrace() actually does use the heap.  Whoops.
  auto const framecount = backtrace(m_frames, kMaxFrame);
  if (framecount <= 0 || framecount > (int)kMaxFrame) {
    m_frame_count = 0;
    return;
  }
  m_frame_count = framecount;
}

////////////////////////////////////////////////////////////////////////////////

const std::string& StackTrace::toString(int skip, int limit) const {
  if (skip != 0 || limit != -1) {
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

  return m_trace;
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

void StackTrace::get(std::vector<void*>& frames) const {
  frames = m_frames;
}

void StackTrace::get(
  std::vector<std::shared_ptr<StackFrameExtra>>& frames
) const {
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

////////////////////////////////////////////////////////////////////////////////

struct StackTraceLog {
  hphp_string_map<std::string> data;

  static DECLARE_THREAD_LOCAL(StackTraceLog, s_logData);
};
IMPLEMENT_THREAD_LOCAL(StackTraceLog, StackTraceLog::s_logData);

void StackTraceNoHeap::AddExtraLogging(const char* name,
                                       const std::string& value) {
  assertx(name != nullptr && name[0] != '\0');
  StackTraceLog::s_logData->data[name] = value;
}

void StackTraceNoHeap::ClearAllExtraLogging() {
  StackTraceLog::s_logData->data.clear();
}

void StackTraceNoHeap::log(const char* errorType, int fd, const char* buildId,
                           int debuggerCount) const {
  assert(fd >= 0);

  dprintf(fd, "Host: %s\n",Process::GetHostName().c_str());
  dprintf(fd, "ProcessID: %" PRId64 "\n", (int64_t)getpid());
  dprintf(fd, "ThreadID: %" PRIx64"\n", (int64_t)Process::GetThreadId());
  dprintf(fd, "ThreadPID: %u\n", Process::GetThreadPid());
  dprintf(fd, "Name: %s\n", Process::GetAppName().c_str());
  dprintf(fd, "Type: %s\n", errorType ? errorType : "(unknown error)");
  dprintf(fd, "Runtime: hhvm\n");
  dprintf(fd, "Version: %s\n", buildId);
  dprintf(fd, "DebuggerCount: %d\n", debuggerCount);
  dprintf(fd, "\n");

  for (auto const& pair : StackTraceLog::s_logData->data) {
    dprintf(fd, "%s: %s\n", pair.first.c_str(), pair.second.c_str());
  }
  dprintf(fd, "\n");

  printStackTrace(fd);
}

////////////////////////////////////////////////////////////////////////////////

void StackTrace::PerfMap::rebuild() {
  m_map.clear();

  char perfMapName[64];
  snprintf(perfMapName, sizeof(perfMapName), "/tmp/perf-%d.map", getpid());
  auto perfMap = fopen(perfMapName, "r");
  if (!perfMap) {
    return;
  }
  SCOPE_EXIT { fclose(perfMap); };
  uintptr_t addr;
  uint32_t size;
  char name[1024];
  while (fscanf(perfMap, "%lx %x %1023s", &addr, &size, name) == 3) {
    uintptr_t past = addr + size;
    PerfMap::Range range = {addr, past};
    m_map[range] = name;
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
  auto endPhp = filefunc.find("::");
  if (endPhp == std::string::npos) {
    frame->funcname = filefunc;
    return false;
  }
  // Skip the ::
  endPhp += 2;
  auto const endFile = filefunc.find("::", endPhp);
  frame->filename = std::string(filefunc, endPhp, endFile - endPhp);
  frame->funcname = "PHP::" + std::string(filefunc, endFile + 2) + "()";
  return false;
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
      TranslateFromPerfMap(frame.get());
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

void StackTrace::TranslateFromPerfMap(StackFrameExtra* frame) {
  std::lock_guard<std::mutex> lock(s_perfMapCacheMutex);

  if (s_perfMapCache.translate(frame)) {
    if (s_perfMapNegCache.find(frame->addr) != s_perfMapNegCache.end()) {
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

////////////////////////////////////////////////////////////////////////////////

#else // HAVE_LIBBFD

/* Basically everything in here requires libbfd.  So, stub city it is. */

bool StackTraceBase::Enabled = false;
const char** StackTraceBase::FunctionBlacklist = {};
unsigned StackTraceBase::FunctionBlacklistCount = 0;

StackTraceBase::StackTraceBase() {}

StackTrace::StackTrace(bool) {}

std::shared_ptr<StackFrameExtra> StackTrace::Translate(void* bt, PerfMap* pm) {
  return std::make_shared<StackFrameExtra>(bt);
}

void StackTrace::TranslateFromPerfMap(StackFrameExtra*) {}

StackTrace::StackTrace(folly::StringPiece) {}

const std::string& StackTrace::toString(int, int) const {
  return m_trace;
}

void StackTrace::get(std::vector<std::shared_ptr<StackFrameExtra>>&) const {}

std::string StackTrace::hexEncode(int, int) const {
  return "";
}

StackTraceNoHeap::StackTraceNoHeap(bool) {}

void StackTraceNoHeap::log(const char*, int, const char*, int) const {}

void StackTraceNoHeap::AddExtraLogging(const char*, const std::string&) {}

void StackTraceNoHeap::ClearAllExtraLogging() {}

#endif // HAVE_LIBBFD

////////////////////////////////////////////////////////////////////////////////
}

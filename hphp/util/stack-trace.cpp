/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <folly/String.h>
#include <folly/ScopeGuard.h>
#include <folly/Conv.h>

#include "hphp/util/process.h"
#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include "hphp/util/light-process.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/hash.h"

namespace HPHP {

#ifdef HAVE_LIBBFD

///////////////////////////////////////////////////////////////////////////////

std::string StackTrace::Frame::toString() const {
  std::string out;
  out = funcname.empty() ? "??" : funcname;
  out += " at ";
  out += filename.empty() ? "??" : filename;
  out += ":";
  out += folly::to<std::string>(lineno);
  return out;
}

///////////////////////////////////////////////////////////////////////////////
// signal handler

///////////////////////////////////////////////////////////////////////////////
// Types
struct bfd_cache {
  bfd_cache() : abfd(nullptr) {}
  bfd *abfd;
  asymbol **syms;

  ~bfd_cache() {
    if (abfd) {
      bfd_cache_close(abfd);
      bfd_free_cached_info(abfd);
      bfd_close_all_done(abfd);
    }
  }
};

static const int MaxKey = 100;
struct NamedBfd {
  bfd_cache bc;
  char key [MaxKey] ;
};

///////////////////////////////////////////////////////////////////////////////
// statics

bool StackTraceBase::Enabled = true;
static const char* s_defaultBlacklist[] = {"_ZN4HPHP16StackTraceNoHeap"};
const char** StackTraceBase::FunctionBlacklist = s_defaultBlacklist;
unsigned int StackTraceBase::FunctionBlacklistCount = 1;

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

StackTraceBase::StackTraceBase() {
  bfd_init();
}

StackTrace::StackTrace(const StackTrace &bt) {
  assert(this != &bt);

  m_bt_pointers = bt.m_bt_pointers;
  m_bt = bt.m_bt;
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

void StackTrace::initFromHex(const char *hexEncoded) {
  std::vector<std::string> frames;
  folly::split(':', hexEncoded, frames);
  for (unsigned int i = 0; i < frames.size(); i++) {
    m_bt_pointers.push_back((void*)strtoll(frames[i].c_str(), nullptr, 16));
  }
}

StackTrace::StackTrace(const std::string &hexEncoded) {
  initFromHex(hexEncoded.c_str());
}

StackTrace::StackTrace(const char *hexEncoded) {
  initFromHex(hexEncoded);
}

void StackTrace::create() {
  void *btpointers[MAXFRAME];
  int framecount = 0;
  framecount = backtrace(btpointers, MAXFRAME);
  if (framecount <= 0 || framecount > (signed) MAXFRAME) {
    m_bt_pointers.clear();
    return;
  }
  m_bt_pointers.resize(framecount);
  for (int i = 0; i < framecount; i++) {
    m_bt_pointers[i] = btpointers[i];
  }
}

void StackTraceNoHeap::create() {
  int unsigned framecount = 0;
  framecount = backtrace(m_btpointers, MAXFRAME);
  if (framecount <= 0 || framecount > MAXFRAME) {
    m_btpointers_cnt = 0;
    return;
  }
  m_btpointers_cnt = framecount;
}

///////////////////////////////////////////////////////////////////////////////
// reporting functions

const std::string &StackTrace::toString(int skip, int limit) const {
  if (skip != 0 || limit != -1) m_bt.clear();
  if (m_bt.empty()) {
    size_t frame = 0;
    for (auto btpi = m_bt_pointers.begin();
         btpi != m_bt_pointers.end(); ++btpi) {
      std::string framename = Translate(*btpi)->toString();
      if (framename.find("StackTrace::") != std::string::npos) {
        continue; // ignore frames in the StackTrace class
      }
      if (skip-- > 0) continue;
      m_bt += "# ";
      m_bt += folly::to<std::string>(frame);
      if (frame < 10) m_bt += " ";

      m_bt += " ";
      m_bt += framename;
      m_bt += "\n";
      ++frame;
      if ((int)frame == limit) break;
    }
  }
  return m_bt;
}

void StackTraceNoHeap::printStackTrace(int fd) const {

  int frame = 0;
  // m_btpointers_cnt must be an upper bound on the number of filenames
  // then *2 for tolerable hash table behavior
  unsigned int bfds_size = m_btpointers_cnt * 2;
  const std::unique_ptr<NamedBfd[]> bfds (new NamedBfd[bfds_size]);
  for (unsigned int i = 0; i < bfds_size; i++) bfds.get()[i].key[0]='\0';
  for (unsigned int i = 0; i < m_btpointers_cnt; i++) {
    if (Translate(fd, m_btpointers[i], frame, bfds.get(), bfds_size)) {
      frame++;
    }
  }
  // ~bfds[i].bc here (unlike the heap case)
}

void StackTrace::get(std::vector<std::shared_ptr<Frame>> &frames) const {
  frames.clear();
  for (auto btpi = m_bt_pointers.begin();
       btpi != m_bt_pointers.end(); ++btpi) {
    frames.push_back(Translate(*btpi));
  }
}

std::string StackTrace::hexEncode(int minLevel /* = 0 */,
                                  int maxLevel /* = 999 */) const {
  std::string bts;
  for (int i = minLevel; i < (int)m_bt_pointers.size() && i < maxLevel; i++) {
    if (i > minLevel) bts += ':';
    char buf[20];
    snprintf(buf, sizeof(buf), "%" PRIx64, (int64_t)m_bt_pointers[i]);
    bts.append(buf);
  }
  return bts;
}

///////////////////////////////////////////////////////////////////////////////
// crash log

class StackTraceLog {
public:
  hphp_string_map<std::string> data;

  static DECLARE_THREAD_LOCAL(StackTraceLog, s_logData);
};
IMPLEMENT_THREAD_LOCAL(StackTraceLog, StackTraceLog::s_logData);

void StackTraceNoHeap::AddExtraLogging(const char *name,
                                       const std::string &value) {
  assert(name && *name);
  StackTraceLog::s_logData->data[name] = value;
}

void StackTraceNoHeap::ClearAllExtraLogging() {
  StackTraceLog::s_logData->data.clear();
}

void StackTraceNoHeap::log(const char *errorType, int fd, const char *buildId,
                           int debuggerCount) const {
  assert(fd >= 0);

  dprintf(fd, "Host: %s\n",Process::GetHostName().c_str());
  dprintf(fd, "ProcessID: %u\n", Process::GetProcessId());
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

///////////////////////////////////////////////////////////////////////////////
// helpers

struct addr2line_data {
  asymbol **syms;
  bfd_vma pc;
  const char *filename;
  const char *functionname;
  unsigned int line;
  bfd_boolean found;
};


bool StackTraceBase::Translate(void *frame, StackTraceBase::Frame * f,
                               Dl_info &dlInfo, void* data,
                               void *bfds, unsigned bfds_size) {
  char sframe[32];
  snprintf(sframe, sizeof(sframe), "%p", frame);

  if (!dladdr(frame, &dlInfo)) {
    return false;
  }

  // frame pointer offset in previous frame
  f->offset = (char*)frame - (char*)dlInfo.dli_saddr;

  if (dlInfo.dli_fname) {

    // 1st attempt without offsetting base address
    if (!Addr2line(dlInfo.dli_fname, sframe, f, data, bfds, bfds_size) &&
        dlInfo.dli_fname && strstr(dlInfo.dli_fname,".so")) {
      // offset shared lib's base address
      frame = (char*)frame - (size_t)dlInfo.dli_fbase;
      snprintf(sframe, sizeof(sframe), "%p", frame);

      // Use addr2line to get line number info.
      Addr2line(dlInfo.dli_fname, sframe, f, data, bfds, bfds_size);
    }
  }
  return true;
}

std::shared_ptr<StackTrace::Frame> StackTrace::Translate(void *frame) {
  Dl_info dlInfo;
  addr2line_data adata;

  Frame * f1 = new Frame(frame);
  std::shared_ptr<Frame> f(f1);
  if (!StackTraceBase::Translate(frame, f1, dlInfo, &adata)) {
    // Lookup using dladdr() failed, so this is probably a PHP symbol.
    // Let's check the perf map.
    StackTrace::TranslateFromPerfMap(frame, f1);
    return f;
  }

  if (adata.filename) {
    f->filename = adata.filename;
  }
  if (adata.functionname) {
    f->funcname = Demangle(adata.functionname);
  }
  if (f->filename.empty() && dlInfo.dli_fname) {
    f->filename = dlInfo.dli_fname;
  }
  if (f->funcname.empty() && dlInfo.dli_sname) {
    f->funcname = Demangle(dlInfo.dli_sname);
  }

  return f;
}

void StackTrace::TranslateFromPerfMap(void* bt, Frame* f) {
  // For now just read the whole map file every time.  If this is
  // egregiously slow I'll build a map and cache it.
  char perfMapName[64];
  snprintf(perfMapName,
           sizeof(perfMapName),
           "/tmp/perf-%d.map", getpid());
  FILE* perfMap = fopen(perfMapName, "r");
  if (!perfMap) {
    f->filename = "?";
    f->funcname = "TC?";
    return;
  }
  SCOPE_EXIT { fclose(perfMap); };
  uintptr_t begin;
  uint32_t size;
  char name[256];
  while (fscanf(perfMap, "%lx %x %255s", &begin, &size, name) == 3) {
    uintptr_t end = begin + size;
    if (bt >= (void*)begin && bt <= (void*)end) {
      break;
    }
  }
  std::string filefunc = std::string(name);
  size_t endPhp = filefunc.find("::");
  if (endPhp == std::string::npos) {
    f->funcname = std::string(filefunc);
    return;
  }
  endPhp += 2;   // Skip the ::
  size_t endFile = filefunc.find("::", endPhp);
  f->filename = std::string(filefunc, endPhp, endFile - endPhp);
  f->funcname = "PHP::" + std::string(filefunc, endFile + 2) + "()";
}

bool StackTraceNoHeap::Translate(int fd, void *frame, int frame_num,
                                 void *bfds, unsigned bfds_size) {
  // frame pointer offset in previous frame
  Dl_info dlInfo;
  addr2line_data adata;
  Frame f(frame);
  if (!StackTraceBase::Translate(frame, &f, dlInfo, &adata, bfds,
                                 bfds_size))  {
    return false;
  }

  const char *filename = adata.filename ? adata.filename : dlInfo.dli_fname;
  if (!filename) filename = "??";
  const char *funcname = adata.functionname ? adata.functionname
                                            : dlInfo.dli_sname;
  if (!funcname) funcname = "??";

  // ignore some frames that are always present
  for (int i = 0; i < FunctionBlacklistCount; i++) {
    auto ignoreFunc = FunctionBlacklist[i];
    if (strncmp(funcname, ignoreFunc, strlen(ignoreFunc)) == 0) {
      return false;
    }
  }

  dprintf(fd, "# %d%s ", frame_num, frame_num < 10 ? " " : "");
  Demangle(fd, funcname);
  dprintf(fd, " at %s:%u\n", filename, f.lineno);

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// copied and re-factored from addr2line

static void find_address_in_section(bfd *abfd, asection *section, void *data) {
  addr2line_data *adata = reinterpret_cast<addr2line_data*>(data);

  bfd_vma vma;
  bfd_size_type size;

  if (adata->found) {
    return;
  }

  if ((bfd_get_section_flags(abfd, section) & SEC_ALLOC) == 0) {
    return;
  }

  vma = bfd_get_section_vma(abfd, section);
  if (adata->pc < vma) {
    return;
  }

  size = bfd_get_section_size(section);
  if (adata->pc >= vma + size) {
    return;
  }

  // libdwarf allocates its own unaligned memory so it doesn't play well with
  // valgrind
#ifndef VALGRIND
  adata->found = bfd_find_nearest_line(abfd, section, adata->syms,
                                       adata->pc - vma, &adata->filename,
                                       &adata->functionname, &adata->line);
#endif

  if (adata->found) {
    const char *file = adata->filename;
    unsigned int line = adata->line;
    bfd_boolean found = TRUE;
    while (found) {
      found = bfd_find_inliner_info(abfd, &file, &adata->functionname, &line);
    }
  }
}

static bool slurp_symtab(asymbol ***syms, bfd *abfd) {
  long symcount;
  unsigned int size;

  symcount = bfd_read_minisymbols(abfd, FALSE, (void **)syms, &size);
  if (symcount == 0) {
    symcount = bfd_read_minisymbols(abfd, TRUE /* dynamic */, (void **)syms,
                                    &size);
  }
  return symcount >= 0;
}

static bool translate_addresses(bfd *abfd, const char *addr,
                                addr2line_data *adata) {
  if (!abfd) return false;
  adata->pc = bfd_scan_vma(addr, nullptr, 16);

  adata->found = FALSE;
  bfd_map_over_sections(abfd, find_address_in_section, adata);

  if (!adata->found || !adata->functionname || !*adata->functionname) {
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// We cache opened bfd file pointers that in turn cached frame pointer lookup
// tables.


typedef std::shared_ptr<bfd_cache> bfd_cache_ptr;
typedef hphp_hash_map<std::string, bfd_cache_ptr, string_hash> bfdMap;
static Mutex s_bfdMutex;
static bfdMap s_bfds;

static bool fill_bfd_cache(const char *filename, bfd_cache *p) {
  bfd *abfd = bfd_openr(filename, nullptr); // hard to avoid heap here!
  if (!abfd) return true;
// Some systems don't have the BFD_DECOMPRESS flag
#ifdef BFD_DECOMPRESS
  abfd->flags |= BFD_DECOMPRESS;
#endif
  p->abfd = nullptr;
  p->syms = nullptr;
  char **match;
  if (bfd_check_format(abfd, bfd_archive) ||
      !bfd_check_format_matches(abfd, bfd_object, &match) ||
      !slurp_symtab(&p->syms, abfd)) {
    bfd_close(abfd);
    return true;
  }
  p->abfd = abfd;
  return false;
}

static bfd_cache_ptr get_bfd_cache(const char *filename) {
  bfdMap::const_iterator iter = s_bfds.find(filename);
  if (iter != s_bfds.end()) {
    return iter->second;
  }
  bfd_cache_ptr p(new bfd_cache());
  if (fill_bfd_cache(filename, p.get())) {
    p.reset();
  }
  s_bfds[filename] = p;
  return p;
}

static bfd_cache * get_bfd_cache(const char *filename, NamedBfd* bfds,
                                   int bfds_size) {
  int probe = hash_string(filename) % bfds_size;
  // match on the end of filename instead of the beginning, if necessary
  int tooLong = strlen(filename) - MaxKey;
  if (tooLong > 0) filename += tooLong;
  while (bfds[probe].key[0]
         && strncmp(filename, bfds[probe].key, MaxKey) != 0) {
     probe = probe ? probe-1 : bfds_size-1;
  }
  bfd_cache *p = &bfds[probe].bc;
  if (bfds[probe].key[0]) return p;
  // accept the rare collision on keys (requires probe collision too)
  strncpy(bfds[probe].key, filename, MaxKey);
  fill_bfd_cache(filename, p);
  return p;
}

bool StackTraceBase::Addr2line(const char *filename, const char *address,
                           Frame *frame, void *adata,
                           void *bfds, unsigned bfds_size) {
  Lock lock(s_bfdMutex);
  addr2line_data *data = reinterpret_cast<addr2line_data*>(adata);
  data->filename = nullptr;
  data->functionname = nullptr;
  data->line = 0;
  bool ret;

  if (!bfds) {
    bfd_cache_ptr p = get_bfd_cache(filename);
    if (!p) return false;
    data->syms = p->syms;
    ret = translate_addresses(p->abfd, address, data);
  } else {
    // don't let bfd_cache_ptr malloc behind the scenes in this case
    bfd_cache *q = get_bfd_cache(filename, (NamedBfd*)bfds, bfds_size);
    if (!q) return false;
    data->syms = q->syms;
    ret = translate_addresses(q->abfd, address, data);
  }

  if (ret) {
    frame->lineno = data->line;
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// copied and re-factored from demangle/c++filt

#define DMGL_PARAMS   (1 << 0)  /* Include function args */
#define DMGL_ANSI     (1 << 1)  /* Include const, volatile, etc */
#define DMGL_VERBOSE  (1 << 3)  /* Include implementation details. */

extern "C" {
  extern char *cplus_demangle (const char *mangled, int options);
}

std::string StackTrace::Demangle(const char *mangled) {
  assert(mangled);
  if (!mangled || !*mangled) {
    return "";
  }

  size_t skip_first = 0;
  if (mangled[0] == '.' || mangled[0] == '$') ++skip_first;
  //if (mangled[skip_first] == '_') ++skip_first;

  char *result = cplus_demangle(mangled + skip_first, DMGL_PARAMS | DMGL_ANSI | DMGL_VERBOSE);
  if (result == nullptr) return mangled;

  std::string ret;
  if (mangled[0] == '.') ret += '.';
  ret += result;
  free (result);
  return ret;
}

void StackTraceNoHeap::Demangle(int fd, const char *mangled) {
  assert(mangled);
  if (!mangled || !*mangled) {
    dprintf(fd, "??");
    return ;
  }

  size_t skip_first = 0;
  if (mangled[0] == '.' || mangled[0] == '$') ++skip_first;
  //if (mangled[skip_first] == '_') ++skip_first;

  char *result = cplus_demangle(mangled + skip_first,
                                DMGL_PARAMS | DMGL_ANSI | DMGL_VERBOSE);
  if (result == nullptr) {
    dprintf(fd, "%s", mangled);
    return;
  }
  dprintf(fd, "%s%s", mangled[0]=='.' ? "." : "", result);
  return ;
}

///////////////////////////////////////////////////////////////////////////////

#else // HAVE_LIBBFD

// Basically everything in here requires libbfd. So, stub city it is.

std::string StackTraceBase::Frame::toString() const {
  return "";
}

bool StackTraceBase::Enabled = false;
const char** StackTraceBase::FunctionBlacklist = {};
unsigned int StackTraceBase::FunctionBlacklistCount = 0;

StackTraceBase::StackTraceBase() {
}

std::string StackTrace::Frame::toString() const {
  return "";
}

StackTrace::StackTrace(bool trace) {
}

std::shared_ptr<StackTrace::Frame> StackTrace::Translate(void *bt) {
  return std::shared_ptr<StackTrace::Frame>(new Frame(bt));
}

void StackTrace::TranslateFromPerfMap(void* bt, Frame* f) {
}

std::string Demangle(const char *mangled) {
  return "";
}

StackTrace::StackTrace(const StackTrace &bt) {
}

StackTrace::StackTrace(const std::string &hexEncoded) {
}

StackTrace::StackTrace(const char *hexEncoded) {
}

const std::string &StackTrace::toString(int skip, int limit) const {
  return m_bt;
}

void StackTrace::get(std::vector<std::shared_ptr<Frame>> &frames) const {
}

std::string StackTrace::hexEncode(int minLevel, int maxLevel) const {
  return "";
}

StackTraceNoHeap::StackTraceNoHeap(bool trace) {
}

void StackTraceNoHeap::log(const char *errorType, int fd, const char *buildId,
    int debuggerCount) const {
}

void StackTraceNoHeap::AddExtraLogging(const char *name,
    const std::string &value) {
}

void StackTraceNoHeap::ClearAllExtraLogging() {
}

#endif // HAVE_LIBBFD

}

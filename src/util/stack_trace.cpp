/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include "stack_trace.h"
#include "process.h"
#include "base.h"
#include "lock.h"
#include "util.h"

#ifndef MAC_OS_X
#include <execinfo.h>
#include <bfd.h>
#endif

#include <dlfcn.h>
#include <signal.h>

using namespace std;
using namespace boost;

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

std::string StackTrace::Frame::toString() const {
  string out;
  out = funcname.empty() ? "??" : funcname;
  out += " at ";
  out += filename.empty() ? "??" : filename;
  out += ":";
  out += lexical_cast<string>(lineno);
  return out;
}

///////////////////////////////////////////////////////////////////////////////
// signal handler

static void bt_handler(int sig) {
  // In case we crash again in the signal hander or something
  signal(sig, SIG_DFL);

  // Calling all of these library functions in a signal handler
  // is completely undefined behavior, but we seem to get away with it.

  Logger::Error("Core dumped: %s", strsignal(sig));

  StackTrace st;
  std::string logmessage;
  std::string outfn = st.log(strsignal(sig), &logmessage);

  //cerr << logmessage << endl;
  if (!StackTrace::ReportEmail.empty()) {
    //cerr << "Emailing trace to " << StackTrace::ReportEmail << endl;
    string cmdline = "cat " + outfn + string(" | mail -s \"Stack Trace from ")
      + Process::GetAppName() + string("\" ") + StackTrace::ReportEmail;
    Util::ssystem(cmdline.c_str());
  }

  // re-raise the signal and pass it to the default handler
  // to terminate the process.
  raise(sig);
}

///////////////////////////////////////////////////////////////////////////////
// statics

string StackTrace::ReportEmail;

void StackTrace::InstallReportOnSignal(int sig) {
  signal(sig, bt_handler);
}

void StackTrace::InstallReportOnErrors() {
  static bool already_set = false;
  if (already_set) return;
  already_set = true;

  // Turn on bt-on-sig for a good default set of error signals
  signal(SIGQUIT, bt_handler);
  signal(SIGTERM, bt_handler);
  signal(SIGILL,  bt_handler);
  signal(SIGFPE,  bt_handler);
  signal(SIGSEGV, bt_handler);
  signal(SIGBUS,  bt_handler);
  signal(SIGABRT, bt_handler);
}

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

StackTrace::StackTrace() {
  create();
#ifndef MAC_OS_X
  bfd_init();
#endif
}

StackTrace::StackTrace(const StackTrace &bt) {
  assert(this != &bt);

  m_bt_pointers = bt.m_bt_pointers;
  m_bt = bt.m_bt;
}

StackTrace::StackTrace(const std::string &hexEncoded) {
  vector<string> frames;
  Util::split(':', hexEncoded.c_str(), frames);
  for (unsigned int i = 0; i < frames.size(); i++) {
    m_bt_pointers.push_back((void*)strtoll(frames[i].c_str(), NULL, 16));
  }
}

void StackTrace::create() {
  const int MAXFRAME = 175;
  void *btpointers[MAXFRAME];
  int framecount = 0;
#ifndef MAC_OS_X
  framecount = backtrace(btpointers, MAXFRAME);
#endif
  if (framecount <= 0 || framecount > MAXFRAME) {
    m_bt_pointers.clear();
    return;
  }
  m_bt_pointers.resize(framecount);
  for (int i = 0; i < framecount; i++) {
    m_bt_pointers[i] = btpointers[i];
  }
}

///////////////////////////////////////////////////////////////////////////////
// reporting functions

const std::string &StackTrace::toString() const {
  if (m_bt.empty()) {
    size_t frame = 0;
    for (vector<void*>::const_iterator btpi = m_bt_pointers.begin();
         btpi != m_bt_pointers.end(); ++btpi) {
      string framename = Translate(*btpi)->toString();
      if (framename.find("StackTrace::") != string::npos) {
        continue; // ignore frames in the StackTrace class
      }
      m_bt += "# ";
      m_bt += lexical_cast<string>(frame);
      if (frame < 10) m_bt += " ";

      m_bt += " ";
      m_bt += framename;
      m_bt += "\n";
      ++frame;
    }
  }
  return m_bt;
}

void StackTrace::get(FramePtrVec &frames) const {
  frames.clear();
  for (vector<void*>::const_iterator btpi = m_bt_pointers.begin();
       btpi != m_bt_pointers.end(); ++btpi) {
    frames.push_back(Translate(*btpi));
  }
}

std::string StackTrace::hexEncode(int minLevel /* = 0 */,
                                  int maxLevel /* = 999 */) const {
  string bts;
  for (int i = minLevel; i < (int)m_bt_pointers.size() && i < maxLevel; i++) {
    if (i > minLevel) bts += ':';
    char buf[20];
    snprintf(buf, sizeof(buf), "%llx", (int64)m_bt_pointers[i]);
    bts.append(buf);
  }
  return bts;
}

string StackTrace::log(const char *errorType, string *out /* = NULL */) const {
  string pid = lexical_cast<string>(Process::GetProcessId());

  string msg;
  msg += "Host: " + Process::GetHostName();
  msg += "\nProcessID: " + pid;
  msg += "\nThreadID: " + lexical_cast<string>(Process::GetThreadId());
  msg += "\nName: " + Process::GetAppName();
  msg += "\nType: ";
  if (errorType) {
    msg += errorType;
  } else {
    msg += "(unknown error)";
  }
  msg += "\n\n";
  msg += toString();
  msg += "\n";

  string tracefn = "/tmp/stacktrace." + pid + ".log";
  ofstream f(tracefn.c_str());
  if (f) {
    f << msg;
    f.close();
  }

  if (out) {
    *out = msg;
  }
  return tracefn;
}

///////////////////////////////////////////////////////////////////////////////
// helpers

StackTrace::FramePtr StackTrace::Translate(void *frame) {
  FramePtr f(new Frame(frame));

  char sframe[32];
  snprintf(sframe, sizeof(sframe), "%p", frame);
  string original_frame = sframe;

  Dl_info dlInfo;
  if (!dladdr(frame, &dlInfo)) {
    return f;
  }

  // frame pointer offset in previous frame
  f->offset = (char*)frame - (char*)dlInfo.dli_saddr;

  string out;
  if (dlInfo.dli_fname) {
    string fname = dlInfo.dli_fname;

    // 1st attempt without offsetting base address
    if (!Addr2line(dlInfo.dli_fname, sframe, f) &&
        fname.find(".so") != string::npos) {
      // offset shared lib's base address
      frame = (char*)frame - (size_t)dlInfo.dli_fbase;
      snprintf(sframe, sizeof(sframe), "%p", frame);

      // Use addr2line to get line number info.
      Addr2line(dlInfo.dli_fname, sframe, f);
    }
  }
  if (f->filename.empty() && dlInfo.dli_fname) {
    f->filename = dlInfo.dli_fname;
  }
  if (f->funcname.empty() && dlInfo.dli_sname) {
    f->funcname = Demangle(dlInfo.dli_sname);
  }

  return f;
}

///////////////////////////////////////////////////////////////////////////////
// copied and re-factored from addr2line

#ifndef MAC_OS_X

struct addr2line_data {
  asymbol **syms;
  bfd_vma pc;
  const char *filename;
  const char *functionname;
  unsigned int line;
  bfd_boolean found;
};

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

  adata->found = bfd_find_nearest_line(abfd, section, adata->syms,
                                       adata->pc - vma, &adata->filename,
                                       &adata->functionname, &adata->line);
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
  adata->pc = bfd_scan_vma(addr, NULL, 16);

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

struct bfd_cache {
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
typedef boost::shared_ptr<bfd_cache> bfd_cache_ptr;
typedef __gnu_cxx::hash_map<std::string, bfd_cache_ptr, string_hash> bfdMap;
static Mutex s_bfdMutex;
static bfdMap s_bfds;

static bfd_cache_ptr get_bfd_cache(const char *filename) {
  bfdMap::const_iterator iter = s_bfds.find(filename);
  if (iter != s_bfds.end()) {
    return iter->second;
  }
  bfd_cache_ptr p(new bfd_cache());
  bfd *abfd = bfd_openr(filename, NULL);
  if (abfd) {
    p->abfd = abfd;
    p->syms = NULL;
    char **match;
    if (bfd_check_format(abfd, bfd_archive) ||
        !bfd_check_format_matches(abfd, bfd_object, &match) ||
        !slurp_symtab(&p->syms, abfd)) {
      bfd_close(abfd);
      p.reset();
    }
  } else {
    p.reset();
  }
  s_bfds[filename] = p;
  return p;
}

#endif

bool StackTrace::Addr2line(const char *filename, const char *address,
                           FramePtr &frame) {
#ifndef MAC_OS_X
  Lock lock(s_bfdMutex);
  bfd_cache_ptr p = get_bfd_cache(filename);
  if (!p) return false;

  addr2line_data data;
  data.filename = NULL;
  data.functionname = NULL;
  data.line = 0;
  data.syms = p->syms;
  bool ret = translate_addresses(p->abfd, address, &data);
  if (ret) {
    if (data.filename) {
      frame->filename = data.filename;
    }
    if (data.functionname) {
      frame->funcname = Demangle(data.functionname);
    }
    frame->lineno = data.line;
  }
  return ret;
#else
  return false;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// copied and re-factored from demangle/c++filt

#define DMGL_PARAMS	 (1 << 0)	/* Include function args */
#define DMGL_ANSI	 (1 << 1)	/* Include const, volatile, etc */
#define DMGL_VERBOSE	 (1 << 3)	/* Include implementation details.  */

#ifndef MAC_OS_X

extern "C" {
  extern char *cplus_demangle (const char *mangled, int options);
}

#endif

std::string StackTrace::Demangle(const char *mangled) {
  assert(mangled);
  if (!mangled || !*mangled) {
    return "";
  }

#ifndef MAC_OS_X
  size_t skip_first = 0;
  if (mangled[0] == '.' || mangled[0] == '$') ++skip_first;
  //if (mangled[skip_first] == '_') ++skip_first;

  char *result = cplus_demangle(mangled + skip_first, DMGL_PARAMS | DMGL_ANSI | DMGL_VERBOSE);
  if (result == NULL) return mangled;

  string ret;
  if (mangled[0] == '.') ret += '.';
  ret += result;
  free (result);
  return ret;
#else
  return mangled;
#endif
}

///////////////////////////////////////////////////////////////////////////////
}

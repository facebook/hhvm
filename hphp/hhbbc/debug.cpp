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
#include "hphp/hhbbc/debug.h"

#include <string>
#include <utility>
#include <cstdlib>
#include <fstream>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <vector>

#include <folly/portability/Stdlib.h>

#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/context.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/parallel.h"
#include "hphp/hhbbc/parse.h"

namespace HPHP::HHBBC {

namespace fs = std::filesystem;

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString s_invoke("__invoke");

template<class Operation>
void with_file(fs::path dir, const php::Unit& u, Operation op) {
  // Paths for systemlib units start with /, which gets interpreted as
  // an absolute path, so strip it.
  auto filename = u.filename->data();
  if (filename[0] == '/') ++filename;

  auto const file = dir / fs::path(filename);
  fs::create_directories(fs::path(file).remove_filename());

  std::ofstream out(file);
  if (!out.is_open()) {
    throw std::runtime_error(std::string("failed to open file ") +
      file.string());
  }

  op(out);

  if (out.bad()) {
    throw std::runtime_error(std::string("couldn't write file ") +
      file.string());
  }
}

using NameTy = std::pair<SString,PropStateElem>;
std::vector<NameTy> sorted_prop_state(const PropState& ps) {
  std::vector<NameTy> ret(begin(ps), end(ps));
  std::sort(
    begin(ret), end(ret),
    [&] (NameTy a, NameTy b) { return a.first->compare(b.first) < 0; }
  );
  return ret;
}

void dump_class_state(std::ostream& out,
                      const Index& index,
                      const php::Class* c) {
  auto const clsName = c->name->toCppString();

  auto const add_attrs = [&] (Attr attrs) {
    if (!(attrs & AttrInitialSatisfiesTC)) out << " (no-satisfies-tc)";
    if (attrs & AttrDeepInit) out << " (deep-init)";
  };

  if (auto const s = index.lookup_iface_vtable_slot(c); s != kInvalidSlot) {
    out << clsName << " vtable slot #" << s << '\n';
  }

  if (is_closure(*c)) {
    auto const invoke = find_method(c, s_invoke.get());
    auto const useVars = index.lookup_closure_use_vars(invoke);
    for (auto i = size_t{0}; i < useVars.size(); ++i) {
      out << clsName << "->" << c->properties[i].name->data() << " :: "
          << show(useVars[i]) << '\n';
    }
  } else {
     auto const pprops = sorted_prop_state(
      index.lookup_private_props(c)
    );
    for (auto const& kv : pprops) {
      out << clsName << "->" << kv.first->data() << " :: "
          << show(kv.second.ty);
      add_attrs(kv.second.attrs);
      out << '\n';
    }

    auto const private_sprops = sorted_prop_state(
      index.lookup_private_statics(c)
    );
    for (auto const& kv : private_sprops) {
      out << clsName << "::$" << kv.first->data() << " :: "
          << show(kv.second.ty);
      if (!kv.second.everModified) out << " (persistent)";
      add_attrs(kv.second.attrs);
      out << '\n';
    }

    auto const public_sprops = sorted_prop_state(
      index.lookup_public_statics(c)
    );
    for (auto const& kv : public_sprops) {
      out << clsName << "::$" << kv.first->data() << " :: "
          << show(kv.second.ty);
      if (!kv.second.everModified) out << " (persistent)";
      add_attrs(kv.second.attrs);
      out << '\n';
    }
  }

  std::vector<const php::Const*> constants;
  constants.reserve(c->constants.size());
  for (auto const& constant : c->constants) {
    constants.emplace_back(&constant);
  }
  std::sort(
    begin(constants), end(constants),
    [] (const php::Const* a, const php::Const* b) {
      return string_data_lt{}(a->name, b->name);
    }
  );

  for (auto const constant : constants) {
    if (constant->val) {
      auto const ty = from_cell(*constant->val);
      out << clsName << "::" << constant->name->data() << " :: "
          << (ty.subtypeOf(BUninit) ? "<dynamic>" : show(ty));
      if (constant->kind == ConstModifiers::Kind::Type) {
        if (constant->resolvedTypeStructure) {
          out << " (" << show(dict_val(constant->resolvedTypeStructure)) << ")";
          switch ((php::Const::Invariance)constant->invariance) {
            case php::Const::Invariance::None:
              break;
            case php::Const::Invariance::Present:
              out << " <present>";
              break;
            case php::Const::Invariance::ClassnamePresent:
              out << " <classname>";
              break;
            case php::Const::Invariance::Same:
              out << " <same>";
              break;
          }
        } else {
          out << " <unresolved>";
        }
      }
      out << '\n';
    }
  }
}

void dump_func_state(std::ostream& out,
                     const Index& index,
                     const php::Func& f) {
  auto const name = f.cls
    ? folly::sformat(
        "{}::{}()",
        f.cls->name, f.name
      )
    : folly::sformat("{}()", f.name);

  auto const [retTy, effectFree] = index.lookup_return_type_raw(&f).first;
  out << name << " :: " << show(retTy) <<
    (effectFree ? " (effect-free)\n" : "\n");
}

}

//////////////////////////////////////////////////////////////////////

std::string debug_dump_to() {
  if (!Trace::moduleEnabledRelease(Trace::hhbbc_dump, 1)) return "";

  trace_time tracer("debug dump");

  auto dir = [&]{
    if (auto const dumpDir = getenv("HHBBC_DUMP_DIR")) {
      return fs::path(dumpDir);
    } else {
      char dirBuf[] = "/tmp/hhbbcXXXXXX";
      auto const dtmpRet = mkdtemp(dirBuf);
      if (!dtmpRet) {
        throw std::runtime_error(
          std::string("Failed to create temporary directory") +
          strerror(errno));
      }
      return fs::path(dtmpRet);
    }
  }();
  fs::create_directory(dir);

  Trace::ftraceRelease("debug dump going to {}\n", dir.string());
  return dir.string();
}

void dump_representation(const std::string& dir,
                         const Index& index,
                         const php::Unit& unit) {
  auto const rep_dir = fs::path{dir} / "representation";
  with_file(rep_dir, unit, [&] (std::ostream& out) {
    out << show(unit, index);
  });
}

void dump_index(const std::string& dir,
                const Index& index,
                const php::Unit& unit) {
  if (!*unit.filename->data()) {
    // The native systemlibs: for now just skip.
    return;
  }

  auto ind_dir = fs::path{dir} / "index";

  with_file(ind_dir, unit, [&] (std::ostream& out) {
    std::vector<const php::Class*> classes;
    index.for_each_unit_class(
      unit,
      [&] (const php::Class& c) { classes.emplace_back(&c); }
    );
    std::sort(
      begin(classes), end(classes),
      [] (const php::Class* a, const php::Class* b) {
        return string_data_lti{}(a->name, b->name);
      }
    );

    for (auto const c : classes) {
      dump_class_state(out, index, c);

      std::vector<const php::Func*> funcs;
      funcs.reserve(c->methods.size());
      for (auto const& m : c->methods) {
        if (!m) continue;
        funcs.emplace_back(m.get());
      }
      std::sort(
        begin(funcs), end(funcs),
        [] (const php::Func* a, const php::Func* b) {
          return string_data_lt{}(a->name, b->name);
        }
      );
      for (auto const f : funcs) dump_func_state(out, index, *f);
    }

    std::vector<const php::Func*> funcs;
    index.for_each_unit_func(
      unit,
      [&] (const php::Func& f) { funcs.emplace_back(&f); }
    );
    std::sort(
      begin(funcs), end(funcs),
      [] (const php::Func* a, const php::Func* b) {
        return string_data_lt{}(a->name, b->name);
      }
    );
    for (auto const f : funcs) dump_func_state(out, index, *f);
  });
}

//////////////////////////////////////////////////////////////////////

void state_after(const char* when,
                 const php::Unit& u,
                 const Index& index) {
  TRACE_SET_MOD(hhbbc);
  Trace::Bump bumper{Trace::hhbbc, kSystemLibBump, is_systemlib_part(u)};
  FTRACE(4, "{:-^70}\n{}{:-^70}\n", when, show(u, index), "");
}

void state_after(const char* when, const ParsedUnit& parsed) {
  TRACE_SET_MOD(hhbbc);
  Trace::Bump bumper{
    Trace::hhbbc,
    kSystemLibBump,
    is_systemlib_part(*parsed.unit)
  };

  std::vector<const php::Func*> funcs;
  std::vector<const php::Class*> classes;
  for (auto const& f : parsed.funcs) {
    funcs.emplace_back(f.get());
  }
  for (auto const& c : parsed.classes) {
    classes.emplace_back(c.get());
  }

  FTRACE(
    4,
    "{:-^70}\n{}{:-^70}\n",
    when,
    show(*parsed.unit, classes, funcs),
    ""
  );
}

//////////////////////////////////////////////////////////////////////

namespace {

template <typename Clock>
std::string ts(typename Clock::time_point t) {
  char snow[64];
  auto tm = Clock::to_time_t(t);
  ctime_r(&tm, snow);
  // Eliminate trailing newline from ctime_r.
  snow[24] = '\0';
  return snow;
}

std::string format_bytes(size_t bytes) {
  auto s = folly::prettyPrint(
    bytes,
    folly::PRETTY_BYTES
  );
  // prettyPrint sometimes inserts a trailing blank space
  if (!s.empty() && s[s.size()-1] == ' ') s.resize(s.size()-1);
  return s;
}

std::string format_duration(std::chrono::microseconds usecs) {
  auto s = prettyPrint(
    double(usecs.count()) / 1000000.0,
    folly::PRETTY_TIME_HMS,
    false
  );
  if (!s.empty() && s[s.size()-1] == ' ') s.resize(s.size()-1);
  return s;
}

std::string client_stats(const extern_worker::Client::Stats& stats) {
  auto const pct = [] (size_t a, size_t b) -> std::string {
    if (!b) return "--";
    return folly::sformat("{:.2f}%", double(a) / b * 100.0);
  };

  auto const execWorkItems = stats.execWorkItems.load();
  auto const allocatedCores = stats.execAllocatedCores.load();
  auto const cpuUsecs = stats.execCpuUsec.load();
  auto const execCalls = stats.execCalls.load();
  auto const storeCalls = stats.storeCalls.load();
  auto const loadCalls = stats.loadCalls.load();

  return folly::sformat(
    "  Execs: {:,} ({:,}) total, {:,} cache-hits ({})\n"
    "  Workers: {} usage, {:,} cores ({}/core), {} max used, {} reserved\n"
    "  Blobs: {:,} total, {:,} uploaded ({})\n"
    "  {:,} downloads ({}), {:,} throttles, (E: {} S: {} L: {}) avg latency\n",
    execCalls,
    execWorkItems,
    stats.execCacheHits.load(),
    pct(stats.execCacheHits.load(), execCalls),
    format_duration(std::chrono::microseconds{cpuUsecs}),
    allocatedCores,
    format_duration(
      std::chrono::microseconds{allocatedCores ? (cpuUsecs / allocatedCores) : 0}
    ),
    format_bytes(stats.execMaxUsedMem.load()),
    format_bytes(stats.execReservedMem.load()),
    stats.blobs.load(),
    stats.blobsUploaded.load(),
    format_bytes(stats.blobBytesUploaded.load()),
    stats.downloads.load(),
    format_bytes(stats.bytesDownloaded.load()),
    stats.throttles.load(),
    format_duration(
      std::chrono::microseconds{execCalls ? (stats.execLatencyUsec.load() / execCalls) : 0}
    ),
    format_duration(
      std::chrono::microseconds{storeCalls ? (stats.storeLatencyUsec.load() / storeCalls) : 0}
    ),
    format_duration(
      std::chrono::microseconds{loadCalls ? (stats.loadLatencyUsec.load() / loadCalls) : 0}
    )
  );
}

extern_worker::Client::Stats::Ptr g_clientStats;

}

trace_time::trace_time(const char* what,
                       std::string extra_,
                       StructuredLogEntry* logEntry)
  : what{what}
  , start(clock::now())
  , extra{std::move(extra_)}
  , logEntry{logEntry}
  , beforeRss{Process::GetMemUsageMb() * 1024 * 1024}
{
  if (g_clientStats) clientBefore = g_clientStats->copy();

  profile_memory(what, "start", extra);
  if (!Trace::moduleEnabledRelease(Trace::hhbbc_time, 1)) return;
  Trace::ftraceRelease(
    "{}: {}: start{}\n"
    "  RSS: {}\n",
    ts<clock>(start),
    what,
    !extra.empty() ? folly::sformat(" ({})", extra) : extra,
    format_bytes(Process::GetMemUsageMb() * 1024 * 1024)
  );
}

trace_time::~trace_time() {
  namespace C = std::chrono;
  auto const end = clock::now();
  auto const elapsed = C::duration_cast<C::milliseconds>(
    end - start
  );

  auto const clientDiff = (g_clientStats && clientBefore)
    ? (*g_clientStats - *clientBefore)
    : nullptr;

  profile_memory(what, "end", extra);

  auto const afterRss = Process::GetMemUsageMb() * 1024 * 1024;

  if (logEntry) {
    auto phase = folly::sformat("hhbbc_{}", what);
    while (true) {
      auto const pos = phase.find_first_of(" :\"'");
      if (pos == std::string::npos) break;
      phase[pos] = '_';
    }

    logEntry->setInt(
      phase + "_micros",
      C::duration_cast<C::microseconds>(elapsed).count()
    );
    logEntry->setInt(phase + "_before_rss_bytes", beforeRss);
    logEntry->setInt(phase + "_after_rss_bytes", afterRss);
    logEntry->setInt(phase + "_rss_delta_bytes", afterRss - beforeRss);
    if (clientDiff) clientDiff->logSample(phase, *logEntry);
  }

  if (!Trace::moduleEnabledRelease(Trace::hhbbc_time, 1)) return;
  Trace::ftraceRelease(
    "{}: {}: {} elapsed\n"
    "  RSS: {}\n{}",
    ts<clock>(end), what, format_duration(elapsed),
    format_bytes(afterRss),
    clientDiff ? client_stats(*clientDiff) : ""
  );
}

void trace_time::ignore_client_stats() {
  clientBefore.reset();
}

void trace_time::register_client_stats(extern_worker::Client::Stats::Ptr p) {
  g_clientStats = std::move(p);
}

//////////////////////////////////////////////////////////////////////

}

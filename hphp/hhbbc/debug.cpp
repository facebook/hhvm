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
#include <iostream>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <algorithm>
#include <memory>
#include <vector>

#include <folly/portability/Stdlib.h>

#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/parallel.h"

namespace HPHP { namespace HHBBC {

namespace fs = boost::filesystem;

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString s_invoke("__invoke");

template<class Operation>
void with_file(fs::path dir, borrowed_ptr<const php::Unit> u, Operation op) {
  auto const file = dir / fs::path(u->filename->data());
  fs::create_directories(fs::path(file).remove_filename());

  fs::ofstream out(file);
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

void dump_representation(fs::path dir, const php::Program& program) {
  parallel::for_each(
    program.units,
    [&] (const std::unique_ptr<php::Unit>& u) {
      with_file(dir, borrow(u), [&] (std::ostream& out) {
        out << show(*u);
      });
    }
  );
}

using NameTy = std::pair<SString,Type>;
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
                      borrowed_ptr<const php::Class> c) {
  if (is_closure(*c)) {
    auto const invoke = find_method(c, s_invoke.get());
    auto const useVars = index.lookup_closure_use_vars(invoke);
    for (auto i = size_t{0}; i < useVars.size(); ++i) {
      out << c->name->data() << "->" << c->properties[i].name->data() << " :: "
          << show(useVars[i]) << '\n';
    }
  } else {
    auto const pprops = sorted_prop_state(
      index.lookup_private_props(c)
    );
    for (auto const& kv : pprops) {
      out << c->name->data() << "->" << kv.first->data() << " :: "
          << show(kv.second) << '\n';
    }

    auto const sprops = sorted_prop_state(
      index.lookup_private_statics(c)
    );
    for (auto const& kv : sprops) {
      out << c->name->data() << "::$" << kv.first->data() << " :: "
          << show(kv.second) << '\n';
    }
  }

  for (auto const& constant : c->constants) {
    if (constant.val) {
      auto const ty = from_cell(*constant.val);
      out << c->name->data() << "::" << constant.name->data() << " :: "
          << (ty.subtypeOf(TUninit) ? "<dynamic>" : show(ty)) << '\n';
    }
  }
}

void dump_func_state(std::ostream& out,
                     const Index& index,
                     borrowed_ptr<const php::Func> f) {
  if (f->unit->pseudomain.get() == f) return;

  auto const name = f->cls
    ? folly::sformat("{}::{}()", f->cls->name->data(), f->name->data())
    : folly::sformat("{}()", f->name->toCppString());

  auto const retTy = index.lookup_return_type_raw(f);
  out << name << " :: " << show(retTy) << '\n';

  auto const localStatics = index.lookup_local_static_types(f);
  for (auto i = size_t{0}; i < localStatics.size(); ++i) {
    if (localStatics[i].subtypeOf(TBottom)) continue;
    out << name << "::" << local_string(*f, i)
        << " :: " << show(localStatics[i]) << '\n';
  }
}

void dump_index(fs::path dir,
                const Index& index,
                const php::Program& program) {
  parallel::for_each(
    program.units,
    [&] (const std::unique_ptr<php::Unit>& u) {
      if (!*u->filename->data()) {
        // The native systemlibs: for now just skip.
        return;
      }

      with_file(dir, borrow(u), [&] (std::ostream& out) {
        for (auto& c : u->classes) {
          dump_class_state(out, index, borrow(c));
          for (auto& m : c->methods) {
            dump_func_state(out, index, borrow(m));
          }
        }

        for (auto& f : u->funcs) {
          dump_func_state(out, index, borrow(f));
        }
      });
    }
  );
}

}

//////////////////////////////////////////////////////////////////////

void debug_dump_program(const Index& index, const php::Program& program) {
  if (!Trace::moduleEnabledRelease(Trace::hhbbc_dump, 1)) return;

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

  FTRACE_MOD(Trace::hhbbc_dump, 1, "debug dump going to {}\n", dir.string());

  if (Trace::moduleEnabledRelease(Trace::hhbbc_dump, 2)) {
    trace_time tracer2("debug dump: representation");
    dump_representation(dir / "representation", program);
  }

  {
    trace_time tracer2("debug dump: index");
    dump_index(dir / "index", index, program);
  }

  FTRACE_MOD(Trace::hhbbc_dump, 1, "debug dump done\n");
}

//////////////////////////////////////////////////////////////////////

}}

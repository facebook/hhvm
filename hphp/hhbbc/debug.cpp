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

using NameTy = std::pair<SString,PropStateElem<>>;
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
          << show(kv.second.ty) << '\n';
    }

    auto const private_sprops = sorted_prop_state(
      index.lookup_private_statics(c)
    );
    for (auto const& kv : private_sprops) {
      out << clsName << "::$" << kv.first->data() << " :: "
          << show(kv.second.ty);
      if (!kv.second.everModified) out << " (persistent)";
      out << '\n';
    }

    auto const public_sprops = sorted_prop_state(
      index.lookup_public_statics(c)
    );
    for (auto const& kv : public_sprops) {
      out << clsName << "::$" << kv.first->data() << " :: "
          << show(kv.second.ty);
      if (!kv.second.everModified) out << " (persistent)";
      out << '\n';
    }
  }

  for (auto const& constant : c->constants) {
    if (constant.val) {
      auto const ty = from_cell(*constant.val);
      out << clsName << "::" << constant.name->data() << " :: "
          << (ty.subtypeOf(BUninit) ? "<dynamic>" : show(ty));
      if (constant.kind == ConstModifiers::Kind::Type) {
        if (constant.resolvedTypeStructure) {
          out << " (" << show(dict_val(constant.resolvedTypeStructure)) << ")";
          switch ((php::Const::Invariance)constant.invariance) {
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

  auto const retTy = index.lookup_return_type_raw(&f).first;
  out << name << " :: " << show(retTy) <<
    (index.is_effect_free_raw(&f) ? " (effect-free)\n" : "\n");
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
    index.for_each_unit_class(
      unit,
      [&] (const php::Class& c) {
        dump_class_state(out, index, &c);
        for (auto const& m : c.methods) {
          if (!m) continue;
          dump_func_state(out, index, *m);
        }
      }
    );
    index.for_each_unit_func(
      unit,
      [&] (const php::Func& f) { dump_func_state(out, index, f); }
    );
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

}

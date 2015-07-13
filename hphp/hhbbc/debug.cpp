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

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/parallel.h"

namespace HPHP { namespace HHBBC {

namespace fs = boost::filesystem;

//////////////////////////////////////////////////////////////////////

namespace {

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

void dump_class_propstate(std::ostream& out,
                          const Index& index,
                          borrowed_ptr<const php::Class> c) {
  out << "Class " << c->name->data() << '\n';

  auto const pprops = sorted_prop_state(
    index.lookup_private_props(c)
  );
  for (auto& kv : pprops) {
    out << "$this->" << kv.first->data() << " :: "
        << show(kv.second) << '\n';
  }

  auto const sprops = sorted_prop_state(
    index.lookup_private_statics(c)
  );
  for (auto& kv : sprops) {
    out << "self::$" << kv.first->data() << " :: "
        << show(kv.second) << '\n';
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
          dump_class_propstate(out, index, borrow(c));
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

  char dirBuf[] = "/tmp/hhbbcXXXXXX";
  auto const dtmpRet = mkdtemp(dirBuf);
  if (!dtmpRet) {
    throw std::runtime_error(
      std::string("Failed to create temporary directory") +
        strerror(errno));
  }
  auto const dir = fs::path(dtmpRet);
  fs::create_directory(dir);
  std::cout << "debug dump going to " << dir << '\n';

  {
    trace_time tracer("debug dump: representation");
    dump_representation(dir / "representation", program);
  }
  {
    trace_time tracer("debug dump: index");
    dump_index(dir / "index", index, program);
  }

  std::cout << "debug dump done\n";
}

//////////////////////////////////////////////////////////////////////

}}


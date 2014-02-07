/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/hhbbc/hhbbc.h"

#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <string>
#include <memory>
#include <cstdint>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "folly/ScopeGuard.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/hhvm/process-init.h"
#include "hphp/runtime/vm/repo.h"

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/parallel.h"

namespace HPHP { namespace HHBBC {

namespace {

//////////////////////////////////////////////////////////////////////

std::string output_repo;
std::string input_repo;

//////////////////////////////////////////////////////////////////////

void parse_options(int argc, char** argv) {
  namespace po = boost::program_options;

  po::options_description basic("Options");
  basic.add_options()
    ("help", "display help message")
    ("output,o",
      po::value(&output_repo)->default_value("hhvm.hhbbc"),
      "output hhbc repo path")
    ("input",
      po::value(&input_repo)->default_value("hhvm.hhbc"),
      "input hhbc repo path")
    ("no-optimizations",
      po::bool_switch(&options.NoOptimizations),
      "turn off all optimizations")
    ;

  po::options_description oflags("Optimization Flags");
  oflags.add_options()
    ("remove-dead-blocks",      po::value(&options.RemoveDeadBlocks))
    ("constant-prop",           po::value(&options.ConstantProp))
    ("insert-assertions",       po::value(&options.InsertAssertions))
    ("insert-stack-assertions", po::value(&options.InsertStackAssertions))
    ("filter-assertions",       po::value(&options.FilterAssertions))
    ("strength-reduce",         po::value(&options.StrengthReduce))
    ("enable-func-families",    po::value(&options.EnableFuncFamilies))

    ("hard-const-prop",         po::value(&options.HardConstProp))
    ("hard-type-hints",         po::value(&options.HardTypeHints))
    ("hard-private-prop",       po::value(&options.HardPrivatePropInference))
    ;

  po::options_description all;
  all.add(basic).add(oflags);

  po::positional_options_description pd;
  pd.add("input", 1);

  po::variables_map vm;
  po::store(
    po::command_line_parser(argc, argv)
      .options(all)
      .positional(pd)
      .extra_parser(
        [&] (const std::string& s) -> std::pair<std::string,std::string> {
          if (s.find("-f") == 0) {
            if (s.substr(2, 3) == "no-") {
              return { s.substr(5), "false" };
            }
            return { s.substr(2), "true" };
          }
          return {};
        })
      .run(),
    vm
  );
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << basic << "\n"
"Individual optimizations may be turned on and off using gcc-style -fflag\n"
"and -fno-flag arguments.  The various optimization flags are documented\n"
"in the code.\n";
    std::exit(0);
  }
}

//////////////////////////////////////////////////////////////////////

void open_repo(const std::string& path) {
  RuntimeOption::RepoCentralPath = path;
  Repo::get();
}

std::vector<std::unique_ptr<UnitEmitter>> load_input() {
  std::vector<std::unique_ptr<UnitEmitter>> ret;

  trace_time timer("load units");

  open_repo(input_repo);
  SCOPE_EXIT { Repo::shutdown(); };

  if (Repo::get().global().UsedHHBBC) {
    throw std::runtime_error("This hhbc repo has already been "
      "optimized by hhbbc");
  }

  return parallel_map(
    Repo::get().enumerateUnits(),
    [&] (const std::pair<std::string,MD5>& kv) {
      return Repo::get().urp().loadEmitter(kv.first, kv.second);
    }
  );
}

void write_output(std::vector<std::unique_ptr<UnitEmitter>> ues) {
  RuntimeOption::RepoCommit = true;
  RuntimeOption::RepoEvalMode = "local";
  open_repo(output_repo);
  SCOPE_EXIT { Repo::shutdown(); };
  batchCommit(std::move(ues));

  auto gd                     = Repo::GlobalData{};
  gd.UsedHHBBC                = true;
  gd.HardTypeHints            = options.HardTypeHints;
  gd.HardPrivatePropInference = options.HardPrivatePropInference;
  Repo::get().saveGlobalData(gd);
}

void compile_repo() {
  auto ues = load_input();
  std::cout << folly::format("{} units\n", ues.size());

  ues = whole_program(std::move(ues));

  {
    trace_time timer("writing output repo");
    write_output(std::move(ues));
  }
}

//////////////////////////////////////////////////////////////////////

}

int main(int argc, char** argv) try {
  parse_options(argc, argv);

  if (boost::filesystem::exists(output_repo)) {
    std::cout << "output repo already exists; removing it\n";
    if (unlink(output_repo.c_str())) {
      std::cerr << "failed to unlink output repo: "
                << strerror(errno) << '\n';
      return 1;
    }
  }

  Hdf config;
  RuntimeOption::Load(config);
  RuntimeOption::RepoLocalPath     = "/tmp/hhbbc.repo";
  RuntimeOption::RepoCentralPath   = input_repo;
  RuntimeOption::RepoLocalMode     = "--";
  RuntimeOption::RepoJournal       = "memory";
  RuntimeOption::RepoCommit        = false;
  RuntimeOption::RepoAuthoritative = true;

  register_process_init();
  initialize_repo();
  Repo::shutdown();

  hphp_process_init();

  Trace::BumpRelease bumper(Trace::hhbbc_time, -1);
  compile_repo();
  return 0;
}

catch (std::exception& e) {
  std::cerr << e.what() << '\n';
  return 1;
}

//////////////////////////////////////////////////////////////////////

}}

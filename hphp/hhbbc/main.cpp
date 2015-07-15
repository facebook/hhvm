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
#include "hphp/hhbbc/hhbbc.h"

#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <string>
#include <memory>
#include <cstdint>
#include <algorithm>
#include <unistd.h>
#include <exception>
#include <utility>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <folly/ScopeGuard.h>
#include <folly/String.h>

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/hhvm/process-init.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/stats.h"
#include "hphp/hhbbc/parallel.h"

namespace HPHP { namespace HHBBC {

namespace {

namespace fs = boost::filesystem;

//////////////////////////////////////////////////////////////////////

std::string output_repo;
std::string input_repo;
bool logging = true;


//////////////////////////////////////////////////////////////////////

void parse_options(int argc, char** argv) {
  namespace po = boost::program_options;

  auto const defaultThreadCount =
    std::max<long>(sysconf(_SC_NPROCESSORS_ONLN) - 1, 1);

  std::vector<std::string> interceptable_fns;
  std::vector<std::string> trace_fns;
  bool no_logging = false;

  po::options_description basic("Options");
  basic.add_options()
    ("help", "display help message")
    ("output,o",
      po::value(&output_repo)->default_value("hhvm.hhbbc"),
      "output hhbc repo path")
    ("input",
      po::value(&input_repo)->default_value("hhvm.hhbc"),
      "input hhbc repo path")
    ("stats-file",
      po::value(&options.stats_file)->default_value(""),
      "stats file path")
    ("no-optimizations",
      po::bool_switch(&options.NoOptimizations),
      "turn off all optimizations")
    ("no-logging",
      po::bool_switch(&no_logging),
      "turn off logging")
    ("extended-stats",
      po::bool_switch(&options.extendedStats),
      "Spend time to produce extra stats")
    ("parallel-num-threads",
      po::value(&parallel::num_threads)->default_value(defaultThreadCount),
      "Number of threads to use for parallelism")
    ("parallel-work-size",
      po::value(&parallel::work_chunk)->default_value(120),
      "Work unit size for parallelism")
    ("interceptable",
      po::value(&interceptable_fns)->composing(),
      "Add an interceptable function")
    ("trace",
      po::value(&trace_fns)->composing(),
      "Add a function to increase tracing level on (for debugging)")
    ;

  // Some extra esoteric options that aren't exposed in --help for
  // now.
  po::options_description extended("Extended Options");
  extended.add_options()
    ("analyze-func-wlimit",  po::value(&options.analyzeFuncWideningLimit))
    ("analyze-class-wlimit", po::value(&options.analyzeClassWideningLimit))
    ("return-refine-limit",  po::value(&options.returnTypeRefineLimit))
    ;

  po::options_description oflags("Optimization Flags");
  oflags.add_options()
    ("context-sensitive-interp",
                                po::value(&options.ContextSensitiveInterp))
    ("remove-dead-blocks",      po::value(&options.RemoveDeadBlocks))
    ("constant-prop",           po::value(&options.ConstantProp))
    ("constant-fold-builtins",  po::value(&options.ConstantFoldBuiltins))
    ("peephole",                po::value(&options.Peephole))
    ("local-dce",               po::value(&options.LocalDCE))
    ("global-dce",              po::value(&options.GlobalDCE))
    ("remove-unused-locals",    po::value(&options.RemoveUnusedLocals))
    ("insert-assertions",       po::value(&options.InsertAssertions))
    ("insert-stack-assertions", po::value(&options.InsertStackAssertions))
    ("filter-assertions",       po::value(&options.FilterAssertions))
    ("strength-reduce",         po::value(&options.StrengthReduce))
    ("func-families",           po::value(&options.FuncFamilies))

    ("hard-const-prop",         po::value(&options.HardConstProp))
    ("hard-type-hints",         po::value(&options.HardTypeHints))
    ("hard-return-type-hints",  po::value(&options.HardReturnTypeHints))
    ("hard-private-prop",       po::value(&options.HardPrivatePropInference))
    ("disallow-dyn-var-env-funcs",
                                po::value(&options.DisallowDynamicVarEnvFuncs))
    ("all-funcs-interceptable", po::value(&options.AllFuncsInterceptable))
    ("analyze-pseudomains",     po::value(&options.AnalyzePseudomains))
    ("analyze-public-statics",  po::value(&options.AnalyzePublicStatics))
    ;

  po::options_description all;
  all.add(basic).add(extended).add(oflags);

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

  options.InterceptableFunctions = make_method_map(interceptable_fns);
  options.TraceFunctions         = make_method_map(trace_fns);
  logging = !no_logging;
}

void validate_options() {
  if (parallel::work_chunk <= 10 || parallel::num_threads < 1) {
    std::cerr << "Invalid parallelism configuration.\n";
    std::exit(1);
  }

  if (options.AnalyzePublicStatics && !options.AnalyzePseudomains) {
    std::cerr << "-fanalyze-public-statics requires -fanalyze-pseudomains\n";
    std::exit(1);
  }

  if (options.RemoveUnusedLocals && !options.GlobalDCE) {
    std::cerr << "-fremove-unused-locals requires -fglobal-dce\n";
    std::exit(1);
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

  return parallel::map(
    Repo::get().enumerateUnits(RepoIdCentral, false, true),
    [&] (const std::pair<std::string,MD5>& kv) {
      return Repo::get().urp().loadEmitter(kv.first, kv.second);
    }
  );
}

void write_output(std::vector<std::unique_ptr<UnitEmitter>> ues,
                  std::unique_ptr<ArrayTypeTable::Builder> arrTable) {
  RuntimeOption::RepoCommit = true;
  RuntimeOption::RepoEvalMode = "local";
  open_repo(output_repo);
  SCOPE_EXIT { Repo::shutdown(); };
  batchCommit(std::move(ues));

  auto gd                     = Repo::GlobalData{};
  gd.UsedHHBBC                = true;
  gd.HardTypeHints            = options.HardTypeHints;
  gd.HardReturnTypeHints      = options.HardReturnTypeHints;
  gd.HardPrivatePropInference = options.HardPrivatePropInference;
  gd.DisallowDynamicVarEnvFuncs = options.DisallowDynamicVarEnvFuncs;

  gd.arrayTypeTable.repopulate(*arrTable);
  Repo::get().saveGlobalData(gd);
}

void compile_repo() {
  auto ues = load_input();
  if (logging) {
    std::cout << folly::format("{} units\n", ues.size());
  }

  auto pair = whole_program(std::move(ues));
  {
    trace_time timer("writing output repo");
    write_output(std::move(pair.first), std::move(pair.second));
  }
}

//////////////////////////////////////////////////////////////////////

}

int main(int argc, char** argv) try {
  parse_options(argc, argv);

  if (fs::exists(output_repo)) {
    std::cout << "output repo already exists; removing it\n";
    if (unlink(output_repo.c_str())) {
      std::cerr << "failed to unlink output repo: "
                << strerror(errno) << '\n';
      return 1;
    }
  }
  if (!fs::exists(input_repo)) {
    std::cerr << "input repo `" << input_repo << "' not found\n";
    return 1;
  }

  Hdf config;
  IniSetting::Map ini = IniSetting::Map::object;
  RuntimeOption::Load(ini, config);
  RuntimeOption::RepoLocalPath       = "/tmp/hhbbc.repo";
  RuntimeOption::RepoCentralPath     = input_repo;
  RuntimeOption::RepoLocalMode       = "--";
  RuntimeOption::RepoJournal         = "memory";
  RuntimeOption::RepoCommit          = false;
  RuntimeOption::EvalJit             = false;

  register_process_init();
  initialize_repo();
  Repo::shutdown();

  hphp_process_init();

  // We only need to set this flag so Repo::global will let us access
  // it.
  RuntimeOption::RepoAuthoritative = true;

  Trace::BumpRelease bumper(Trace::hhbbc_time, -1, logging);
  compile_repo();
  return 0;
}

catch (std::exception& e) {
  std::cerr << e.what() << '\n';
  return 1;
}

//////////////////////////////////////////////////////////////////////

}}

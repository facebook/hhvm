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
#include "hphp/hhbbc/hhbbc.h"

#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <string>
#include <memory>
#include <cstdint>
#include <algorithm>
#include <exception>
#include <utility>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <folly/ScopeGuard.h>
#include <folly/String.h>
#include <folly/portability/Unistd.h>

#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/rds-local.h"
#include "hphp/runtime/base/vm-worker.h"
#include "hphp/hhvm/process-init.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/treadmill.h"

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/stats.h"
#include "hphp/hhbbc/parallel.h"

namespace HPHP { namespace HHBBC {

namespace {

namespace fs = boost::filesystem;

//////////////////////////////////////////////////////////////////////

std::string output_repo;
std::string input_repo;
std::string hack_compiler_extract_path;
bool logging = true;
bool print_bytecode_stats_and_exit = false;


//////////////////////////////////////////////////////////////////////

template<class SinglePassReadableRange>
MethodMap make_method_map(SinglePassReadableRange& range) {
  auto ret = MethodMap{};
  for (auto& str : range) {
    std::vector<std::string> parts;
    folly::split("::", str, parts);
    if (parts.size() != 2) {
      ret[""].insert(str);
      continue;
    }
    ret[parts[0]].insert(parts[1]);
  }
  return ret;
}

template<class SinglePassReadableRange>
OpcodeSet make_bytecode_map(SinglePassReadableRange& bcs) {
  if (bcs.empty()) return {};
  hphp_fast_map<std::string,Op> bcmap;
  for (auto i = 0; i < Op_count; i++) {
    auto const op = static_cast<Op>(i);
    bcmap[opcodeToName(op)] = op;
  }
  OpcodeSet oset;
  for (auto& n : bcs) {
    if (bcmap.count(n)) oset.insert(bcmap[n]);
  }
  return oset;
}

void parse_options(int argc, char** argv) {
  namespace po = boost::program_options;

  auto const defaultThreadCount =
    std::max<long>(sysconf(_SC_NPROCESSORS_ONLN) - 1, 1);

  std::vector<std::string> interceptable_fns;
  std::vector<std::string> trace_fns;
  std::vector<std::string> trace_bcs;
  bool no_logging = false;
  bool no_cores = false;

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
    ("no-cores",
      po::bool_switch(&no_cores),
      "turn off core dumps (useful when running lots of tests in parallel)")
    ("extended-stats",
      po::bool_switch(&options.extendedStats),
      "Spend time to produce extra stats")
    ("profile-memory",
     po::value(&options.profileMemory)->default_value(""),
      "If non-empty, dump jemalloc memory profiles at key points")
    ("parallel-num-threads",
      po::value(&parallel::num_threads)->default_value(defaultThreadCount),
      "Number of threads to use for parallelism")
    ("parallel-final-threads",
      po::value(&parallel::final_threads)->default_value(
        parallel::final_threads),
      "Number of threads to use for the final pass")
    ("parallel-work-size",
      po::value(&parallel::work_chunk)->default_value(120),
      "Work unit size for parallelism")
    ("trace",
      po::value(&trace_fns)->composing(),
      "Add a function to increase tracing level on (for debugging)")
    ("trace-bytecode",
      po::value(&trace_bcs)->composing(),
      "Add a bytecode to trace (for debugging)")
    ("hack-compiler-extract-path",
      po::value(&hack_compiler_extract_path)->default_value(""),
      "hack compiler extract path")
    ;

  // Some extra esoteric options that aren't exposed in --help for
  // now.
  po::options_description extended("Extended Options");
  extended.add_options()
    ("analyze-func-wlimit",  po::value(&options.analyzeFuncWideningLimit))
    ("analyze-class-wlimit", po::value(&options.analyzeClassWideningLimit))
    ("return-refine-limit",  po::value(&options.returnTypeRefineLimit))
    ("public-sprop-refine-limit", po::value(&options.publicSPropRefineLimit))
    ("bytecode-stats",       po::bool_switch(&print_bytecode_stats_and_exit))
    ;

  po::options_description oflags("Optimization Flags");
  oflags.add_options()
    ("context-sensitive-interp",
                                po::value(&options.ContextSensitiveInterp))
    ("remove-dead-blocks",      po::value(&options.RemoveDeadBlocks))
    ("constant-prop",           po::value(&options.ConstantProp))
    ("constant-fold-builtins",  po::value(&options.ConstantFoldBuiltins))
    ("local-dce",               po::value(&options.LocalDCE))
    ("global-dce",              po::value(&options.GlobalDCE))
    ("remove-unused-locals",    po::value(&options.RemoveUnusedLocals))
    ("remove-unused-clsref-slots", po::value(&options.RemoveUnusedClsRefSlots))
    ("insert-assertions",       po::value(&options.InsertAssertions))
    ("insert-stack-assertions", po::value(&options.InsertStackAssertions))
    ("filter-assertions",       po::value(&options.FilterAssertions))
    ("strength-reduce",         po::value(&options.StrengthReduce))
    ("func-families",           po::value(&options.FuncFamilies))
    ("hard-const-prop",         po::value(&options.HardConstProp))
    ("hard-private-prop",       po::value(&options.HardPrivatePropInference))
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

  if (no_cores) {
    struct rlimit rl{};
    setrlimit(RLIMIT_CORE, &rl);
  }

  if (!options.ConstantProp) options.ConstantFoldBuiltins = false;

  options.TraceFunctions = make_method_map(trace_fns);
  options.TraceBytecodes = make_bytecode_map(trace_bcs);

  if (!options.profileMemory.empty()) {
    mallctlWrite("prof.active", true);
    mallctlWrite("prof.thread_active_init", true);
  }

  logging = !no_logging;
}

UNUSED void validate_options() {
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

  if (options.RemoveUnusedClsRefSlots && !options.GlobalDCE) {
    std::cerr << "-fremove-unused-clsref-slots requires -fglobal-dce\n";
    std::exit(1);
  }
}

//////////////////////////////////////////////////////////////////////

void open_repo(const std::string& path) {
  RuntimeOption::RepoCentralPath = path;
  // Make sure the changes take effect
  Repo::shutdown();
  Repo::get();
}

std::pair<std::vector<std::unique_ptr<UnitEmitter>>,
          std::vector<SString>> load_input() {
  trace_time timer("load units");

  open_repo(input_repo);
  Repo::get().loadGlobalData();
  SCOPE_EXIT { Repo::shutdown(); };

  auto const& gd = Repo::get().global();
  // When running hhbbc, these option is loaded from GD, and will override CLI.
  // When running hhvm, these option is not loaded from GD, but read from CLI.
  RuntimeOption::EvalJitEnableRenameFunction = gd.EnableRenameFunction;
  RuntimeOption::EvalHackArrCompatNotices =
    RuntimeOption::EvalHackArrCompatCheckRefBind =
    RuntimeOption::EvalHackArrCompatCheckFalseyPromote =
    RuntimeOption::EvalHackArrCompatCheckCompare =
    RuntimeOption::EvalHackArrCompatCheckArrayPlus =
    RuntimeOption::EvalHackArrCompatCheckArrayKeyCast =
      gd.HackArrCompatNotices;
  RuntimeOption::EvalForbidDynamicCalls      = gd.ForbidDynamicCalls;
  RuntimeOption::EvalNoticeOnBuiltinDynamicCalls =
    gd.NoticeOnBuiltinDynamicCalls;
  RuntimeOption::EvalHackArrCompatIsArrayNotices =
    gd.HackArrCompatIsArrayNotices;
  RuntimeOption::EvalHackArrCompatTypeHintNotices =
    gd.HackArrCompatTypeHintNotices;
  RuntimeOption::EvalHackArrCompatDVCmpNotices =
    gd.HackArrCompatDVCmpNotices;
  RuntimeOption::EvalHackArrCompatSerializeNotices =
    gd.HackArrCompatSerializeNotices;
  RuntimeOption::EvalHackArrDVArrs = gd.HackArrDVArrs;
  RuntimeOption::EvalAbortBuildOnVerifyError = gd.AbortBuildOnVerifyError;
  RuntimeOption::EnableArgsInBacktraces = gd.EnableArgsInBacktraces;
  RuntimeOption::EvalEmitClsMethPointers = gd.EmitClsMethPointers;
  RuntimeOption::EvalIsVecNotices = gd.IsVecNotices;
  RuntimeOption::EvalIsCompatibleClsMethType = gd.IsCompatibleClsMethType;
  RuntimeOption::EvalArrayProvenance = gd.ArrayProvenance;
  RuntimeOption::StrictArrayFillKeys = gd.StrictArrayFillKeys;

  return {
    parallel::map(Repo::get().enumerateUnits(RepoIdCentral, true),
      [&] (const std::pair<std::string,SHA1>& kv) {
        return Repo::get().urp().loadEmitter(
          kv.first, kv.second, Native::s_noNativeFuncs
        );
      }),
    Repo().get().global().APCProfile
  };
}

void write_units(UnitEmitterQueue& ueq) {
  folly::Optional<trace_time> timer;

  RuntimeOption::RepoCommit = true;
  RuntimeOption::RepoEvalMode = "local";
  RuntimeOption::RepoDebugInfo = false; // Don't record UnitSourceLoc
  open_repo(output_repo);
  SCOPE_EXIT { Repo::shutdown(); };

  std::vector<std::unique_ptr<UnitEmitter>> ues;
  while (auto ue = ueq.pop()) {
    if (!timer) timer.emplace("writing output repo");
    ues.push_back(std::move(ue));
    if (ues.size() == 8) {
      batchCommit(ues);
      ues.clear();
    }
  }

  batchCommit(ues);
  ues.clear();
}

void write_global_data(
  std::unique_ptr<ArrayTypeTable::Builder>& arrTable,
  std::vector<SString> apcProfile) {

  auto const now = std::chrono::high_resolution_clock::now();
  auto const nanos =
    std::chrono::duration_cast<std::chrono::nanoseconds>(
      now.time_since_epoch()
    );

  auto gd                        = Repo::GlobalData{};
  gd.UsedHHBBC                   = true;
  gd.Signature                   = nanos.count();
  gd.ThisTypeHintLevel           = RuntimeOption::EvalThisTypeHintLevel;
  gd.HardReturnTypeHints         = RuntimeOption::EvalCheckReturnTypeHints >= 3;
  gd.CheckPropTypeHints          = RuntimeOption::EvalCheckPropTypeHints;
  gd.HardPrivatePropInference    = options.HardPrivatePropInference;
  gd.ElideAutoloadInvokes        = options.ElideAutoloadInvokes;
  gd.PHP7_IntSemantics           = RuntimeOption::PHP7_IntSemantics;
  gd.PHP7_NoHexNumerics          = RuntimeOption::PHP7_NoHexNumerics;
  gd.PHP7_Substr                 = RuntimeOption::PHP7_Substr;
  gd.PHP7_Builtins               = RuntimeOption::PHP7_Builtins;
  gd.PromoteEmptyObject          = RuntimeOption::EvalPromoteEmptyObject;
  gd.EnableRenameFunction        = RuntimeOption::EvalJitEnableRenameFunction;
  gd.HackArrCompatNotices        = RuntimeOption::EvalHackArrCompatNotices;
  gd.EnableIntrinsicsExtension   = RuntimeOption::EnableIntrinsicsExtension;
  gd.APCProfile                  = std::move(apcProfile);
  gd.ReffinessInvariance         = RuntimeOption::EvalReffinessInvariance;
  gd.ForbidDynamicCalls          = RuntimeOption::EvalForbidDynamicCalls;
  gd.AbortBuildOnVerifyError     = RuntimeOption::EvalAbortBuildOnVerifyError;
  gd.EnableArgsInBacktraces      = RuntimeOption::EnableArgsInBacktraces;
  gd.NoticeOnBuiltinDynamicCalls =
    RuntimeOption::EvalNoticeOnBuiltinDynamicCalls;
  gd.HackArrCompatIsArrayNotices =
    RuntimeOption::EvalHackArrCompatIsArrayNotices;
  gd.HackArrCompatTypeHintNotices =
    RuntimeOption::EvalHackArrCompatTypeHintNotices;
  gd.HackArrCompatDVCmpNotices =
    RuntimeOption::EvalHackArrCompatDVCmpNotices;
  gd.HackArrCompatSerializeNotices =
    RuntimeOption::EvalHackArrCompatSerializeNotices;
  gd.HackArrDVArrs = RuntimeOption::EvalHackArrDVArrs;
  gd.InitialNamedEntityTableSize  =
    RuntimeOption::EvalInitialNamedEntityTableSize;
  gd.InitialStaticStringTableSize =
    RuntimeOption::EvalInitialStaticStringTableSize;
  gd.EmitClsMethPointers = RuntimeOption::EvalEmitClsMethPointers;
  gd.IsVecNotices = RuntimeOption::EvalIsVecNotices;
  gd.IsCompatibleClsMethType = RuntimeOption::EvalIsCompatibleClsMethType;
  gd.ArrayProvenance = RuntimeOption::EvalArrayProvenance;
  gd.StrictArrayFillKeys = RuntimeOption::StrictArrayFillKeys;

  for (auto const& elm : RuntimeOption::ConstantFunctions) {
    gd.ConstantFunctions.push_back(elm);
  }

  globalArrayTypeTable().repopulate(*arrTable);
  // NOTE: There's no way to tell if saveGlobalData() fails for some reason.
  Repo::get().saveGlobalData(gd);
}

void compile_repo() {
  auto input = load_input();
  if (logging) {
    std::cout << folly::format("{} units\n", input.first.size());
  }

  UnitEmitterQueue ueq;
  std::unique_ptr<ArrayTypeTable::Builder> arrTable;
  VMWorker wp_thread(
    [&] {
      HphpSession _{Treadmill::SessionKind::CompileRepo};
      Trace::BumpRelease bumper(Trace::hhbbc_time, -1, logging);
      whole_program(std::move(input.first), ueq, arrTable);
    }
  );
  wp_thread.start();
  write_units(ueq);
  LitstrTable::get().setReading();
  write_global_data(arrTable, input.second);
  wp_thread.waitForEnd();
}

void print_repo_bytecode_stats() {
  std::array<uint64_t,Op_count> op_counts{};

  auto const input = load_input();
  for (auto const& ue : input.first) {
    auto pc = ue->bc();
    auto const end = pc + ue->bcPos();
    for (; pc < end; pc += instrLen(pc)) {
      op_counts[static_cast<uint16_t>(peek_op(pc))]++;
    }
  }

  for (auto i = uint32_t{}; i < op_counts.size(); ++i) {
    std::cout << folly::format(
      "{: <20} {}\n",
      opcodeToName(static_cast<Op>(i)),
      op_counts[i]
    );
  }
}

//////////////////////////////////////////////////////////////////////

}

int main(int argc, char** argv) try {
  parse_options(argc, argv);

  if (!print_bytecode_stats_and_exit && fs::exists(output_repo)) {
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

  initialize_repo();

  // We need to set this flag so Repo::global will let us access it.
  RuntimeOption::RepoAuthoritative = true;

  LitstrTable::init();
  RuntimeOption::RepoLocalMode = "--";
  RuntimeOption::RepoEvalMode = "readonly";
  open_repo(input_repo);
  Repo::get().loadGlobalData(false);
  LitstrTable::fini();
  auto const& gd = Repo::get().global();
  if (gd.InitialNamedEntityTableSize) {
    RuntimeOption::EvalInitialNamedEntityTableSize =
      gd.InitialNamedEntityTableSize;
  }
  if (gd.InitialStaticStringTableSize) {
    RuntimeOption::EvalInitialStaticStringTableSize =
      gd.InitialStaticStringTableSize;
  }

  rds::local::init();
  SCOPE_EXIT { rds::local::fini(); };

  Hdf config;
  IniSetting::Map ini = IniSetting::Map::object;
  RuntimeOption::Load(ini, config);
  RuntimeOption::RepoLocalPath       = "/tmp/hhbbc.repo";
  RuntimeOption::RepoCentralPath     = input_repo;
  RuntimeOption::RepoLocalMode       = "--";
  RuntimeOption::RepoJournal         = "memory";
  RuntimeOption::RepoCommit          = false;
  RuntimeOption::EvalJit             = false;

  if (!hack_compiler_extract_path.empty()) {
    RuntimeOption::EvalHackCompilerExtractPath = hack_compiler_extract_path;
  }

  RuntimeOption::EvalThisTypeHintLevel = gd.ThisTypeHintLevel;

  register_process_init();

  hphp_process_init();
  SCOPE_EXIT { hphp_process_exit(); };

  Repo::shutdown();

  if (print_bytecode_stats_and_exit) {
    print_repo_bytecode_stats();
    return 0;
  }

  Trace::BumpRelease bumper(Trace::hhbbc_time, -1, logging);
  compile_repo(); // NOTE: errors ignored
  return 0;
}

catch (std::exception& e) {
  std::cerr << e.what() << '\n';
  return 1;
}

//////////////////////////////////////////////////////////////////////

}}

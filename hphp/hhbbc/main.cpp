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
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/vm-worker.h"
#include "hphp/hhvm/process-init.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/repo-autoload-map-builder.h"
#include "hphp/runtime/vm/repo-file.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/treadmill.h"

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/options.h"
#include "hphp/hhbbc/stats.h"
#include "hphp/hhbbc/parallel.h"
#include "hphp/hhbbc/representation.h"

#include "hphp/util/rds-local.h"
#include "hphp/util/logger.h"

namespace HPHP::HHBBC {

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
  auto const defaultFinalThreadCount =
    std::max<long>(defaultThreadCount - 2, 1);

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
        defaultFinalThreadCount),
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
    ("test-compression",     po::bool_switch(&options.TestCompression))
    ;

  po::options_description oflags("Optimization Flags");
  oflags.add_options()
    ("context-sensitive-interp",  po::value(&options.ContextSensitiveInterp))
    ("remove-dead-blocks",        po::value(&options.RemoveDeadBlocks))
    ("constant-prop",             po::value(&options.ConstantProp))
    ("constant-fold-builtins",    po::value(&options.ConstantFoldBuiltins))
    ("local-dce",                 po::value(&options.LocalDCE))
    ("global-dce",                po::value(&options.GlobalDCE))
    ("remove-unused-local-names", po::value(&options.RemoveUnusedLocalNames))
    ("compact-local-slots",       po::value(&options.CompactLocalSlots))
    ("insert-assertions",         po::value(&options.InsertAssertions))
    ("insert-stack-assertions",   po::value(&options.InsertStackAssertions))
    ("filter-assertions",         po::value(&options.FilterAssertions))
    ("strength-reduce",           po::value(&options.StrengthReduce))
    ("func-families",             po::value(&options.FuncFamilies))
    ("hard-private-prop",         po::value(&options.HardPrivatePropInference))
    ("analyze-public-statics",    po::value(&options.AnalyzePublicStatics))
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

  if (options.RemoveUnusedLocalNames && !options.GlobalDCE) {
    std::cerr << "-fremove-unused-local-names requires -fglobal-dce\n";
    std::exit(1);
  }

  if (options.CompactLocalSlots && !options.GlobalDCE) {
    std::cerr << "-fcompact-local-slots requires -fglobal-dce\n";
    std::exit(1);
  }
}

//////////////////////////////////////////////////////////////////////

template<typename F> void load_input(F&& fun) {
  trace_time timer("load units");

  SCOPE_EXIT { RepoFile::destroy(); };

  auto const units = RepoFile::enumerateUnits();
  auto const size = units.size();
  fun(size, nullptr);
  parallel::for_each(
    units,
    [&] (const StringData* path) {
      fun(
        size,
        RepoFile::loadUnitEmitter(path, path, Native::s_noNativeFuncs, false)
      );
    }
  );
}

RepoGlobalData get_global_data() {
  auto const now = std::chrono::high_resolution_clock::now();
  auto const nanos =
    std::chrono::duration_cast<std::chrono::nanoseconds>(
      now.time_since_epoch()
    );

  auto gd                        = RepoGlobalData{};
  gd.Signature                   = nanos.count();
  gd.HardGenericsUB              = RuntimeOption::EvalEnforceGenericsUB >= 2;
  gd.CheckPropTypeHints          = RuntimeOption::EvalCheckPropTypeHints;
  gd.HardPrivatePropInference    = options.HardPrivatePropInference;
  gd.PHP7_NoHexNumerics          = RuntimeOption::PHP7_NoHexNumerics;
  gd.PHP7_Substr                 = RuntimeOption::PHP7_Substr;
  gd.PHP7_Builtins               = RuntimeOption::PHP7_Builtins;
  gd.EnableIntrinsicsExtension   = RuntimeOption::EnableIntrinsicsExtension;
  gd.ForbidDynamicCallsToFunc    = RuntimeOption::EvalForbidDynamicCallsToFunc;
  gd.ForbidDynamicCallsToClsMeth =
    RuntimeOption::EvalForbidDynamicCallsToClsMeth;
  gd.ForbidDynamicCallsToInstMeth =
    RuntimeOption::EvalForbidDynamicCallsToInstMeth;
  gd.ForbidDynamicConstructs     = RuntimeOption::EvalForbidDynamicConstructs;
  gd.ForbidDynamicCallsWithAttr =
    RuntimeOption::EvalForbidDynamicCallsWithAttr;
  gd.LogKnownMethodsAsDynamicCalls =
    RuntimeOption::EvalLogKnownMethodsAsDynamicCalls;
  gd.AbortBuildOnVerifyError     = RuntimeOption::EvalAbortBuildOnVerifyError;
  gd.EnableArgsInBacktraces      = RuntimeOption::EnableArgsInBacktraces;
  gd.NoticeOnBuiltinDynamicCalls =
    RuntimeOption::EvalNoticeOnBuiltinDynamicCalls;
  gd.HackArrCompatSerializeNotices =
    RuntimeOption::EvalHackArrCompatSerializeNotices;
  gd.InitialNamedEntityTableSize  =
    RuntimeOption::EvalInitialNamedEntityTableSize;
  gd.InitialStaticStringTableSize =
    RuntimeOption::EvalInitialStaticStringTableSize;
  gd.EmitClassPointers = RuntimeOption::EvalEmitClassPointers;
  gd.EmitClsMethPointers = RuntimeOption::EvalEmitClsMethPointers;
  gd.IsVecNotices = RuntimeOption::EvalIsVecNotices;
  gd.RaiseClassConversionWarning =
    RuntimeOption::EvalRaiseClassConversionWarning;
  gd.ClassPassesClassname =
    RuntimeOption::EvalClassPassesClassname;
  gd.ClassnameNotices =
    RuntimeOption::EvalClassnameNotices;
  gd.ClassIsStringNotices = RuntimeOption::EvalClassIsStringNotices;
  gd.StrictArrayFillKeys = RuntimeOption::StrictArrayFillKeys;
  gd.TraitConstantInterfaceBehavior =
    RuntimeOption::EvalTraitConstantInterfaceBehavior;
  gd.BuildMayNoticeOnMethCallerHelperIsObject =
    RuntimeOption::EvalBuildMayNoticeOnMethCallerHelperIsObject;
  gd.DiamondTraitMethods = RuntimeOption::EvalDiamondTraitMethods;
  gd.EvalCoeffectEnforcementLevels = RO::EvalCoeffectEnforcementLevels;
  gd.SourceRootForFileBC = options.SourceRootForFileBC;

  for (auto const& elm : RuntimeOption::ConstantFunctions) {
    auto const s = internal_serialize(tvAsCVarRef(elm.second));
    gd.ConstantFunctions.emplace_back(elm.first, s.toCppString());
  }

  return gd;
}

void compile_repo() {
  auto program = make_program();

  load_input(
    [&] (size_t size, std::unique_ptr<UnitEmitter> ue) {
      if (!ue) {
        if (logging) {
          std::cout << folly::format("{} units\n", size);
        }
        return;
      }
      add_unit_to_program(ue.get(), *program);
    }
  );

  RepoAutoloadMapBuilder autoloadMapBuilder;
  UnitEmitterQueue ueq{&autoloadMapBuilder};

  std::exception_ptr wp_thread_ex = nullptr;
  VMWorker wp_thread(
    [&] {
      HphpSession _{Treadmill::SessionKind::CompileRepo};
      Trace::BumpRelease bumper(Trace::hhbbc_time, -1, logging);
      try {
        StructuredLogEntry sample;
        whole_program(std::move(program), ueq, std::move(sample));
      } catch (...) {
        wp_thread_ex = std::current_exception();
        ueq.finish();
      }
    }
  );
  wp_thread.start();

  {
    RepoFileBuilder repoBuilder{output_repo};

    Optional<trace_time> timer;
    while (auto ue = ueq.pop()) {
      if (!timer) timer.emplace("writing output repo");
      repoBuilder.add(*ue);
    }

    if (!wp_thread_ex) {
      trace_time timer2("finalizing repo");
      repoBuilder.finish(
        get_global_data(),
        autoloadMapBuilder
      );
    }
  }

  wp_thread.waitForEnd();
  if (wp_thread_ex) {
    rethrow_exception(wp_thread_ex);
  }
}

void print_repo_bytecode_stats() {
  std::array<std::atomic<uint64_t>,Op_count> op_counts{};

  load_input(
    [&] (size_t, std::unique_ptr<UnitEmitter> ue) {
      if (!ue) return;

      auto countInstrs = [&](const FuncEmitter& fe) {
        auto pc = fe.bc();
        auto const end = pc + fe.bcPos();
        for (; pc < end; pc += instrLen(pc)) {
          auto &opc = op_counts[static_cast<uint16_t>(peek_op(pc))];
          opc.fetch_add(1, std::memory_order_relaxed);
        }
      };

      for (auto& fe : ue->fevec()) {
        countInstrs(*fe.get());
      }
      for (Id i = 0; i < ue->numPreClasses(); i++) {
        for (auto fe : ue->pce(i)->methods()) {
          countInstrs(*fe);
        }
      }
    }
  );

  for (auto i = uint32_t{}; i < op_counts.size(); ++i) {
    std::cout << folly::format(
      "{: <20} {}\n",
      opcodeToName(static_cast<Op>(i)),
      op_counts[i].load(std::memory_order_relaxed)
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

  RepoFile::init(input_repo);

  auto const& gd = RepoFile::globalData();
  gd.load(false);
  options.SourceRootForFileBC = gd.SourceRootForFileBC;
  if (gd.InitialNamedEntityTableSize) {
    RO::EvalInitialNamedEntityTableSize  = gd.InitialNamedEntityTableSize;
  }
  if (gd.InitialStaticStringTableSize) {
    RO::EvalInitialStaticStringTableSize = gd.InitialStaticStringTableSize;
  }

  rds::local::init();
  SCOPE_EXIT { rds::local::fini(); };

  Logger::Escape = false;
  Logger::AlwaysEscapeLog = false;

  Hdf config;
  IniSetting::Map ini = IniSetting::Map::object;

  // We need to write correct coeffects before we load
  for (auto const& [name, value] : gd.EvalCoeffectEnforcementLevels) {
    config["Eval"]["CoeffectEnforcementLevels"][name] = value;
  }

  RO::Load(ini, config);
  // T103431933 RO::Load() loads default runtime option which might not be correct.
  RO::RepoAuthoritative                     = true;
  RO::EvalJit                               = false;
  RO::EvalLowStaticArrays                   = false;
  RO::RepoDebugInfo                         = false;

  register_process_init();

  hphp_process_init();
  SCOPE_EXIT { hphp_process_exit(); };

  // When running hhbbc, these options are loaded from GD, and will override CLI.
  // When running hhvm, these options are not loaded from GD, but read from CLI.
  // NB: These are only needed if RepoGlobalData::load() does not currently write
  // them which is called in hphp_process_init().
  RO::EvalForbidDynamicCallsToFunc              = gd.ForbidDynamicCallsToFunc;
  RO::EvalForbidDynamicCallsToClsMeth           = gd.ForbidDynamicCallsToClsMeth;
  RO::EvalForbidDynamicCallsToInstMeth          = gd.ForbidDynamicCallsToInstMeth;
  RO::EvalForbidDynamicConstructs               = gd.ForbidDynamicConstructs;
  RO::EvalForbidDynamicCallsWithAttr            = gd.ForbidDynamicCallsWithAttr;
  RO::EvalLogKnownMethodsAsDynamicCalls         = gd.LogKnownMethodsAsDynamicCalls;
  RO::EvalNoticeOnBuiltinDynamicCalls           = gd.NoticeOnBuiltinDynamicCalls;
  RO::EvalHackArrCompatSerializeNotices         = gd.HackArrCompatSerializeNotices;
  RO::EvalIsVecNotices                          = gd.IsVecNotices;
  RO::EvalEnforceGenericsUB                     = gd.HardGenericsUB ? 2 : 1;

  if (print_bytecode_stats_and_exit) {
    print_repo_bytecode_stats();
    return 0;
  }

  Trace::BumpRelease bumper(Trace::hhbbc_time, -1, logging);
  compile_repo();
  return 0;
}

catch (std::exception& e) {
  Logger::Error("std::exception: %s", e.what());
  return 1;
}

//////////////////////////////////////////////////////////////////////

}

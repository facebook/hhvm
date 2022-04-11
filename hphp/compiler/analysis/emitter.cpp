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

#include "hphp/compiler/analysis/emitter.h"

#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/option.h"
#include "hphp/hhbbc/hhbbc.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/vm/disas.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/repo-autoload-map-builder.h"
#include "hphp/runtime/vm/repo-file.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-alias-emitter.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-parser.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/logger.h"

#include <folly/ScopeGuard.h>

#include <exception>
#include <fstream>
#include <future>
#include <iostream>
#include <vector>

namespace HPHP {

namespace Compiler {
///////////////////////////////////////////////////////////////////////////////

namespace {

void genText(UnitEmitter* ue, const std::string& outputPath) {
  std::unique_ptr<Unit> unit(ue->create());
  auto const basePath = AnalysisResult::prepareFile(
    outputPath.c_str(),
    "php/" + unit->filepath()->toCppString(),
    true, false);

  if (Option::GenerateTextHHBC) {
    auto const fullPath = basePath + ".hhbc.txt";

    std::ofstream f(fullPath.c_str());
    if (!f) {
      Logger::Error("Unable to open %s for write", fullPath.c_str());
    } else {
      f << "Hash: " << ue->sha1().toString() << std::endl;
      f << unit->toString();
      f.close();
    }
  }

  if (Option::GenerateHhasHHBC) {
    auto const fullPath = basePath + ".hhas";

    std::ofstream f(fullPath.c_str());
    if (!f) {
      Logger::Error("Unable to open %s for write", fullPath.c_str());
    } else {
      f << disassemble(unit.get());
      f.close();
    }
  }
}

class GenTextWorker
    : public JobQueueWorker<UnitEmitter*, const std::string*, true, true> {
 public:
  void doJob(JobType job) override {
    try {
      genText(job, *m_context);
    } catch (Exception& e) {
      Logger::Error(e.getMessage());
    } catch (...) {
      Logger::Error("Fatal: An unexpected exception was thrown");
    }
  }
  void onThreadEnter() override {
    g_context.getCheck();
  }
  void onThreadExit() override {
    hphp_memory_cleanup();
  }
};

void genText(const std::vector<std::unique_ptr<UnitEmitter>>& ues,
             const std::string& outputPath) {
  if (!ues.size()) return;

  Timer timer(Timer::WallTime, "Generating text bytecode");
  if (ues.size() > Option::ParserThreadCount && Option::ParserThreadCount > 1) {
    JobQueueDispatcher<GenTextWorker> dispatcher {
      Option::ParserThreadCount,
      Option::ParserThreadCount,
      0, false, &outputPath
    };
    dispatcher.start();
    for (auto& ue : ues) {
      dispatcher.enqueue(ue.get());
    }
    dispatcher.waitEmpty();
  } else {
    for (auto& ue : ues) {
      genText(ue.get(), outputPath);
    }
  }
}

RepoGlobalData getGlobalData() {
  auto const now = std::chrono::high_resolution_clock::now();
  auto const nanos =
    std::chrono::duration_cast<std::chrono::nanoseconds>(
      now.time_since_epoch()
    );

  auto gd                        = RepoGlobalData{};
  gd.Signature                   = nanos.count();
  gd.CheckPropTypeHints          = RuntimeOption::EvalCheckPropTypeHints;
  gd.HardPrivatePropInference    = true;
  gd.PHP7_NoHexNumerics          = RuntimeOption::PHP7_NoHexNumerics;
  gd.PHP7_Substr                 = RuntimeOption::PHP7_Substr;
  gd.PHP7_Builtins               = RuntimeOption::PHP7_Builtins;
  gd.HardGenericsUB              = RuntimeOption::EvalEnforceGenericsUB >= 2;
  gd.EnableIntrinsicsExtension   = RuntimeOption::EnableIntrinsicsExtension;
  gd.ForbidDynamicCallsToFunc    = RuntimeOption::EvalForbidDynamicCallsToFunc;
  gd.ForbidDynamicCallsWithAttr  =
    RuntimeOption::EvalForbidDynamicCallsWithAttr;
  gd.ForbidDynamicCallsToClsMeth =
    RuntimeOption::EvalForbidDynamicCallsToClsMeth;
  gd.ForbidDynamicCallsToInstMeth =
    RuntimeOption::EvalForbidDynamicCallsToInstMeth;
  gd.ForbidDynamicConstructs     = RuntimeOption::EvalForbidDynamicConstructs;
  gd.LogKnownMethodsAsDynamicCalls =
    RuntimeOption::EvalLogKnownMethodsAsDynamicCalls;
  gd.EnableArgsInBacktraces      = RuntimeOption::EnableArgsInBacktraces;
  gd.NoticeOnBuiltinDynamicCalls =
    RuntimeOption::EvalNoticeOnBuiltinDynamicCalls;
  gd.InitialNamedEntityTableSize =
    RuntimeOption::EvalInitialNamedEntityTableSize;
  gd.InitialStaticStringTableSize =
    RuntimeOption::EvalInitialStaticStringTableSize;
  gd.HackArrCompatSerializeNotices =
    RuntimeOption::EvalHackArrCompatSerializeNotices;
  gd.AbortBuildOnVerifyError = RuntimeOption::EvalAbortBuildOnVerifyError;
  gd.EmitClassPointers = RuntimeOption::EvalEmitClassPointers;
  gd.EmitClsMethPointers = RuntimeOption::EvalEmitClsMethPointers;
  gd.IsVecNotices = RuntimeOption::EvalIsVecNotices;
  gd.RaiseClassConversionWarning =
    RuntimeOption::EvalRaiseClassConversionWarning;
  gd.ClassPassesClassname = RuntimeOption::EvalClassPassesClassname;
  gd.ClassnameNotices = RuntimeOption::EvalClassnameNotices;
  gd.ClassIsStringNotices = RuntimeOption::EvalClassIsStringNotices;
  gd.StrictArrayFillKeys = RuntimeOption::StrictArrayFillKeys;
  gd.TraitConstantInterfaceBehavior =
    RuntimeOption::EvalTraitConstantInterfaceBehavior;
  gd.BuildMayNoticeOnMethCallerHelperIsObject =
    RO::EvalBuildMayNoticeOnMethCallerHelperIsObject;
  gd.DiamondTraitMethods = RuntimeOption::EvalDiamondTraitMethods;
  gd.EvalCoeffectEnforcementLevels = RO::EvalCoeffectEnforcementLevels;
  gd.EnableImplicitContext = RO::EvalEnableImplicitContext;

  for (auto const& elm : RuntimeOption::ConstantFunctions) {
    auto const s = internal_serialize(tvAsCVarRef(elm.second));
    gd.ConstantFunctions.emplace_back(elm.first, s.toCppString());
  }
  return gd;
}

/*
 * It's an invariant that symbols in the repo must be Unique and
 * Persistent. Normally HHBBC verifies this for us, but if we're not
 * using HHBBC and writing directly to the repo, we must do it
 * ourself. Verify all relevant symbols are unique and set the
 * appropriate Attrs.
 *
 * We use a common set of verification functions exported from HHBBC
 * (to keep error messages identical), so we need store the data in a
 * certain way it expects.
 */
struct SymbolSets {
  struct Unit {
    const StringData* filename;
  };
  struct Data {
    const StringData* name;
    std::unique_ptr<Unit> unit;
    Attr attrs;
  };

  using IMap = hphp_fast_map<
    const StringData*,
    std::unique_ptr<Data>,
    string_data_hash,
    string_data_isame
  >;
  using Map = hphp_fast_map<
    const StringData*,
    std::unique_ptr<Data>,
    string_data_hash,
    string_data_same
  >;

  IMap enums;
  IMap classes;
  IMap funcs;
  IMap typeAliases;
  Map constants;
  Map modules;

  static std::unique_ptr<Data> make(const UnitEmitter* ue,
                                    const StringData* name,
                                    Attr attrs) {
    assertx(name->isStatic());
    assertx(!ue || ue->m_filepath->isStatic());
    std::unique_ptr<Unit> unit;
    if (ue) {
      unit = std::make_unique<SymbolSets::Unit>();
      unit->filename = ue->m_filepath;
    }
    auto data = std::make_unique<SymbolSets::Data>();
    data->name = name;
    data->unit = std::move(unit);
    data->attrs = attrs;
    return data;
  }

  SymbolSets() {
    // These aren't stored in the repo, but we still need to check for
    // collisions against them, so put them in the maps.
    for (auto const& kv : Native::getConstants()) {
      assertx(kv.second.m_type != KindOfUninit ||
              kv.second.dynamic());
      HHBBC::add_symbol(
        constants,
        make(nullptr, kv.first, AttrUnique | AttrPersistent),
        "constant"
      );
    }
  }
};

void writeUnit(UnitEmitter& ue,
               RepoFileBuilder& repoBuilder,
               RepoAutoloadMapBuilder& autoloadMapBuilder,
               SymbolSets& sets) {
  // Verify uniqueness of symbols, set Attrs, then write to actual
  // repo.
  auto const make = [&] (const StringData* name, Attr attrs) {
    return SymbolSets::make(&ue, name, attrs);
  };

  for (size_t n = 0; n < ue.numPreClasses(); ++n) {
    auto pce = ue.pce(n);
    pce->setAttrs(pce->attrs() | AttrUnique | AttrPersistent);
    if (pce->attrs() & AttrEnum) {
      HHBBC::add_symbol(sets.enums, make(pce->name(), pce->attrs()), "enum");
    }
    HHBBC::add_symbol(sets.classes, make(pce->name(), pce->attrs()), "class",
                      sets.typeAliases);
  }
  for (auto& fe : ue.fevec()) {
    // Dedup meth_caller wrappers
    if (fe->attrs & AttrIsMethCaller && sets.funcs.count(fe->name)) continue;
    fe->attrs |= AttrUnique | AttrPersistent;
    HHBBC::add_symbol(sets.funcs, make(fe->name, fe->attrs), "function");
  }
  for (auto& te : ue.typeAliases()) {
    te->setAttrs(te->attrs() | AttrUnique | AttrPersistent);
    HHBBC::add_symbol(sets.typeAliases, make(te->name(), te->attrs()),
                      "type alias", sets.classes);
  }
  for (auto& c : ue.constants()) {
    c.attrs |= AttrUnique | AttrPersistent;
    HHBBC::add_symbol(sets.constants, make(c.name, c.attrs), "constant");
  }
  for (auto& m : ue.modules()) {
    m.attrs |= AttrUnique | AttrPersistent;
    HHBBC::add_symbol(sets.modules, make(m.name, m.attrs), "module");
  }

  autoloadMapBuilder.addUnit(ue);
  repoBuilder.add(ue);
}

}

/*
 * This is the entry point for offline bytecode generation.
 */
void emitAllHHBC(AnalysisResultPtr&& ar) {
  auto ues = ar->getHhasFiles();
  decltype(ues) ues_to_print;
  auto const outputPath = ar->getOutputPath();

  std::thread wp_thread;

  auto unexpectedException = [&] (const char* what) {
    if (wp_thread.joinable()) {
      Logger::Error("emitAllHHBC exited via an exception "
                    "before wp_thread was joined: %s", what);
    }
    throw;
  };

  try {
    {
      SCOPE_EXIT {
        genText(ues_to_print, outputPath);
      };

      Optional<RepoAutoloadMapBuilder> autoloadMapBuilder;
      Optional<RepoFileBuilder> repoBuilder;
      Optional<SymbolSets> symbolSets;
      if (Option::GenerateBinaryHHBC) {
        autoloadMapBuilder.emplace();
        repoBuilder.emplace(RuntimeOption::RepoPath);
        symbolSets.emplace();
      }

      auto program = std::move(ar->program());
      if (!program.get()) {
        uint32_t id = 0;
        for (auto& ue : ues) {
          ue->m_symbol_refs.clear();
          ue->m_sn = id;
          ue->setSha1(SHA1 { id });
          if (repoBuilder) {
            try {
              writeUnit(*ue, *repoBuilder, *autoloadMapBuilder, *symbolSets);
            } catch (const HHBBC::NonUniqueSymbolException&) {
              ar->setFinish({});
              throw;
            }
          }
          if (Option::GenerateTextHHBC || Option::GenerateHhasHHBC) {
            ues_to_print.emplace_back(std::move(ue));
          }
          id++;
        }

        ar->finish();
        ar.reset();

        if (repoBuilder) {
          Timer finalizeTime(Timer::WallTime, "finalizing repo");
          repoBuilder->finish(getGlobalData(), *autoloadMapBuilder);
        }
        return;
      }

      assertx(ues.size() == 0);

      ar->finish();
      ar.reset();

      HHBBC::UnitEmitterQueue ueq{
        repoBuilder ? &*autoloadMapBuilder : nullptr,
        Option::GenerateTextHHBC || Option::GenerateHhasHHBC
      };

      RuntimeOption::EvalJit = false; // For HHBBC to invoke builtins.

      std::exception_ptr wpExn;
      wp_thread = std::thread(
        [program = std::move(program), &ueq, &wpExn] () mutable {
          Timer timer(Timer::WallTime, "running HHBBC");
          HphpSessionAndThread _(Treadmill::SessionKind::CompilerEmit);
          try {
            // We rely on this function to provide a value to arrTable
            HHBBC::whole_program(
              std::move(program), ueq,
              Option::ParserThreadCount > 0 ? Option::ParserThreadCount : 0);
          } catch (...) {
            wpExn = std::current_exception();
            ueq.finish();
          }
        }
      );

      Optional<Timer> commitTime;
      while (auto encoded = ueq.pop()) {
        if (!commitTime) {
          commitTime.emplace(Timer::WallTime, "committing units to repo");
        }
        if (repoBuilder) repoBuilder->add(*encoded);
        if (Option::GenerateTextHHBC || Option::GenerateHhasHHBC) {
          if (auto ue = ueq.popUnitEmitter()) {
            ues_to_print.emplace_back(std::move(ue));
          }
        }
      }
      if (wpExn) {
        wp_thread.join();
        std::rethrow_exception(wpExn);
      }

      Timer finalizeTime(Timer::WallTime, "finalizing repo");
      if (repoBuilder) {
        repoBuilder->finish(getGlobalData(), *autoloadMapBuilder);
      }
    }

    wp_thread.join();
  } catch (std::exception& ex) {
    unexpectedException(ex.what());
  } catch (const Object& o) {
    unexpectedException("Object");
  } catch (...) {
    unexpectedException("non-standard-exception");
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}

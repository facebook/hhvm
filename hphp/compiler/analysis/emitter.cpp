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
#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/option.h"
#include "hphp/hhbbc/hhbbc.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/vm/disas.h"
#include "hphp/runtime/vm/extern-compiler.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-autoload-map-builder.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/unit.h"
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
  std::unique_ptr<Unit> unit(ue->create(true));
  auto const basePath = AnalysisResult::prepareFile(
    outputPath.c_str(),
    Option::UserFilePrefix + unit->filepath()->toCppString(),
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

  Timer timer(Timer::WallTime, "Generating text bytcode");
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

void commitGlobalData(std::unique_ptr<ArrayTypeTable::Builder> arrTable,
                      const RepoAutoloadMapBuilder& autoloadMapBuilder) {
  auto const now = std::chrono::high_resolution_clock::now();
  auto const nanos =
    std::chrono::duration_cast<std::chrono::nanoseconds>(
      now.time_since_epoch()
    );

  auto gd                        = Repo::GlobalData{};
  gd.Signature                   = nanos.count();
  gd.CheckPropTypeHints          = RuntimeOption::EvalCheckPropTypeHints;
  gd.HardPrivatePropInference    = true;
  gd.PHP7_NoHexNumerics          = RuntimeOption::PHP7_NoHexNumerics;
  gd.PHP7_Substr                 = RuntimeOption::PHP7_Substr;
  gd.PHP7_Builtins               = RuntimeOption::PHP7_Builtins;
  gd.HardGenericsUB              = RuntimeOption::EvalEnforceGenericsUB >= 2;
  gd.HackArrCompatNotices        = RuntimeOption::EvalHackArrCompatNotices;
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
  gd.HackArrCompatIsVecDictNotices =
    RuntimeOption::EvalHackArrCompatIsVecDictNotices;
  gd.HackArrCompatSerializeNotices =
    RuntimeOption::EvalHackArrCompatSerializeNotices;
  gd.AbortBuildOnVerifyError = RuntimeOption::EvalAbortBuildOnVerifyError;
  gd.EmitClassPointers = RuntimeOption::EvalEmitClassPointers;
  gd.EmitClsMethPointers = RuntimeOption::EvalEmitClsMethPointers;
  gd.IsVecNotices = RuntimeOption::EvalIsVecNotices;
  gd.IsCompatibleClsMethType = RuntimeOption::EvalIsCompatibleClsMethType;
  gd.RaiseClassConversionWarning =
    RuntimeOption::EvalRaiseClassConversionWarning;
  gd.ClassPassesClassname = RuntimeOption::EvalClassPassesClassname;
  gd.ClassnameNotices = RuntimeOption::EvalClassnameNotices;
  gd.RaiseClsMethConversionWarning =
    RuntimeOption::EvalRaiseClsMethConversionWarning;
  gd.StrictArrayFillKeys = RuntimeOption::StrictArrayFillKeys;
  gd.NoticeOnCoerceForStrConcat =
    RuntimeOption::EvalNoticeOnCoerceForStrConcat;
  gd.NoticeOnCoerceForBitOp =
    RuntimeOption::EvalNoticeOnCoerceForBitOp;

  for (auto const& elm : RuntimeOption::ConstantFunctions) {
    gd.ConstantFunctions.push_back(elm);
  }
  if (arrTable) globalArrayTypeTable().repopulate(*arrTable);
  Repo::get().saveGlobalData(std::move(gd), autoloadMapBuilder);
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
  std::future<void> fut;

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

      auto commitSome = [&] (decltype(ues)& emitters) {
        auto const DEBUG_ONLY err = batchCommitWithoutRetry(emitters, true);
        always_assert(!err);
        if (Option::GenerateTextHHBC || Option::GenerateHhasHHBC) {
          std::move(emitters.begin(), emitters.end(),
                    std::back_inserter(ues_to_print));
        }
        emitters.clear();
      };

      RepoAutoloadMapBuilder autoloadMapBuilder;

      auto program = std::move(ar->program());
      if (!program.get()) {
        if (ues.size()) {
          uint32_t id = 0;
          for (auto& ue : ues) {
            ue->m_symbol_refs.clear();
            ue->m_sn = id;
            ue->setSha1(SHA1 { id });
            autoloadMapBuilder.addUnit(*ue);
            id++;
          }
          commitSome(ues);
        }

        ar->finish();
        ar.reset();

        if (Option::GenerateBinaryHHBC) {
          commitGlobalData(std::unique_ptr<ArrayTypeTable::Builder>{},
                           autoloadMapBuilder);
        }
        return;
      }

      assertx(ues.size() == 0);

      ar->finish();
      ar.reset();

      HHBBC::UnitEmitterQueue ueq;
      auto commitLoop = [&] {
        folly::Optional<Timer> commitTime;
        // kBatchSize needs to strike a balance between reducing
        // transaction commit overhead (bigger batches are better), and
        // limiting the cost incurred by failed commits due to identical
        // units that require rollback and retry (smaller batches have
        // less to lose).  Empirical results indicate that a value in
        // the 2-10 range is reasonable.
        static const unsigned kBatchSize = 8;

        while (auto ue = ueq.pop()) {
          if (!commitTime) {
            commitTime.emplace(Timer::WallTime, "committing units to repo");
          }
          autoloadMapBuilder.addUnit(*ue);
          ues.push_back(std::move(ue));
          if (ues.size() == kBatchSize) {
            commitSome(ues);
          }
        }
        if (ues.size()) commitSome(ues);
      };

      RuntimeOption::EvalJit = false; // For HHBBC to invoke builtins.
      std::unique_ptr<ArrayTypeTable::Builder> arrTable;
      std::promise<void> arrTableReady;
      fut = arrTableReady.get_future();

      wp_thread = std::thread([program = std::move(program),
                               &ueq,
                               &arrTable,
                               &arrTableReady] () mutable {
          Timer timer(Timer::WallTime, "running HHBBC");
          HphpSessionAndThread _(Treadmill::SessionKind::CompilerEmit);
          try {
            // We rely on this function to provide a value to arrTable
            HHBBC::whole_program(
              std::move(program), ueq, arrTable,
              Option::ParserThreadCount > 0 ? Option::ParserThreadCount : 0,
              &arrTableReady);
          } catch (...) {
            arrTableReady.set_exception(std::current_exception());
            ueq.push(nullptr);
          }
        });
      {
        commitLoop();
        fut.wait();
        if (arrTable) // Commit anyway if arrTable was initialised.
          commitGlobalData(std::move(arrTable), autoloadMapBuilder);
      }
    }

    wp_thread.join();
    fut.get(); // Exception thrown here if it holds one, otherwise no-op.
  } catch (std::exception& ex) {
    unexpectedException(ex.what());
  } catch (const Object& o) {
    unexpectedException("Object");
  } catch (...) {
    unexpectedException("non-standard-exception");
  }
}

extern "C" {

/**
 * This is the entry point from the runtime; i.e. online bytecode generation.
 * The 'filename' parameter may be NULL if there is no file associated with
 * the source code.
 *
 * Before being actually used, hphp_compiler_parse must be called with
 * a NULL `code' parameter to do initialization.
 */

Unit* hphp_compiler_parse(const char* code, int codeLen, const SHA1& sha1,
                          const char* filename,
                          const Native::FuncTable& nativeFuncs,
                          Unit** releaseUnit, bool forDebuggerEval,
                          const RepoOptions& options) {
  if (UNLIKELY(!code)) {
    // Do initialization when code is null; see above.
    Option::RecordErrors = false;
    Option::WholeProgram = false;
    TypeConstraint tc;
    return nullptr;
  }

  tracing::Block _{
    "parse",
    [&] {
      return tracing::Props{}
        .add("filename", filename ? filename : "")
        .add("code_size", codeLen);
    }
  };

  // Do not count memory used during parsing/emitting towards OOM.
  MemoryManager::SuppressOOM so(*tl_heap);

  SCOPE_ASSERT_DETAIL("hphp_compiler_parse") { return filename; };
  std::unique_ptr<Unit> unit;
  SCOPE_EXIT {
    if (unit && releaseUnit) *releaseUnit = unit.release();
  };

  // We don't want to invoke the JIT when trying to run PHP code.
  auto const prevFolding = RID().getJitFolding();
  RID().setJitFolding(true);
  SCOPE_EXIT { RID().setJitFolding(prevFolding); };

  try {
    UnitOrigin unitOrigin = UnitOrigin::File;
    if (!filename) {
      filename = "";
      unitOrigin = UnitOrigin::Eval;
    }

    std::unique_ptr<UnitEmitter> ue;
    // Check if this file contains raw hip hop bytecode instead of
    // php.  This is dictated by a special file extension.
    if (RuntimeOption::EvalAllowHhas) {
      if (const char* dot = strrchr(filename, '.')) {
        const char hhbc_ext[] = "hhas";
        if (!strcmp(dot + 1, hhbc_ext)) {
          ue = assemble_string(code, codeLen, filename, sha1, nativeFuncs);
        }
      }
    }

    // If ue != nullptr then we assembled it above, so don't feed it into
    // the extern compiler
    if (!ue) {
      auto uc = UnitCompiler::create(code, codeLen, filename, sha1,
                                     nativeFuncs, forDebuggerEval, options);
      assertx(uc);
      try {
        tracing::BlockNoTrace _{"unit-compiler-run"};
        ue = uc->compile();
      } catch (const BadCompilerException& exc) {
        Logger::Error("Bad external compiler: %s", exc.what());
        return nullptr;
      }
    }

    // NOTE: Repo errors are ignored!
    Repo::get().commitUnit(ue.get(), unitOrigin, false);

    if (RO::EvalStressUnitSerde) ue = ue->stressSerde();
    unit = ue->create();
    if (BuiltinSymbols::s_systemAr) {
      assertx(ue->m_filepath->data()[0] == '/' &&
              ue->m_filepath->data()[1] == ':');
      BuiltinSymbols::s_systemAr->addHhasFile(std::move(ue));
    } else {
      ue.reset();

      if (unit->sn() == -1 && RuntimeOption::RepoCommit) {
        // the unit was not committed to the Repo, probably because
        // another thread did it first. Try to use the winner.
        auto u = Repo::get().loadUnit(filename ? filename : "",
                                      sha1,
                                      nativeFuncs);
        if (u != nullptr) {
          return u.release();
        }
      }
    }

    return unit.release();
  } catch (const std::exception&) {
    // extern "C" function should not be throwing exceptions...
    return nullptr;
  }
}

} // extern "C"

///////////////////////////////////////////////////////////////////////////////
}
}

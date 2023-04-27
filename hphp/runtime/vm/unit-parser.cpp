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

#include "hphp/runtime/vm/unit-parser.h"

#include <cinttypes>
#include <condition_variable>
#include <fstream>
#include <iterator>
#include <memory>
#include <mutex>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>

#include <folly/compression/Zstd.h>
#include <folly/DynamicConverter.h>
#include <folly/json.h>
#include <folly/FileUtil.h>
#include <folly/system/ThreadName.h>

#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs.h"
#include "hphp/hack/src/hackc/hhbc-ast.h"
#include "hphp/hack/src/parser/ffi_bridge/parser_ffi.rs.h"
#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/vm/disas.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/decl-provider.h"
#include "hphp/runtime/vm/hackc-translator.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/unit-gen-helpers.h"
#include "hphp/util/atomic-vector.h"
#include "hphp/util/embedded-data.h"
#include "hphp/util/gzip.h"
#include "hphp/util/hackc-log.h"
#include "hphp/util/light-process.h"
#include "hphp/util/logger.h"
#include "hphp/util/match.h"
#include "hphp/util/sha1.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/zend/zend-string.h"
#include "hphp/zend/zend-strtod.h"

namespace HPHP {

TRACE_SET_MOD(unit_parse);

UnitEmitterCacheHook g_unit_emitter_cache_hook = nullptr;

namespace {

struct CompileException : Exception {
  template<class... A>
  explicit CompileException(A&&... args)
    : Exception(folly::sformat(std::forward<A>(args)...))
  {}
};

[[noreturn]] void throwErrno(const char* what) {
  throw CompileException("{}: {}", what, folly::errnoStr(errno));
}

CompilerResult unitEmitterFromHackCUnitHandleErrors(const hackc::hhbc::Unit& unit,
                                                    const char* filename,
	                                                  const SHA1& sha1,
	                                                  const SHA1& bcSha1,
                                                    const Native::FuncTable& nativeFuncs,
                                                    bool& internal_error,
                                                    CompileAbortMode mode,
                                                    const PackageInfo& packageInfo) {
  try {
    return unitEmitterFromHackCUnit(unit, filename, sha1, bcSha1,
                                    nativeFuncs, false, packageInfo);
  } catch (const FatalErrorException&) {
    throw;
  } catch (const TranslationFatal& ex) {
    if (mode >= CompileAbortMode::VerifyErrors) internal_error = true;
    return ex.what();
  } catch (const std::exception& ex) {
    internal_error = true;
    return ex.what();
  }
}

CompilerResult assemble_string_handle_errors(const char* code,
                                             const std::string& hhas,
                                             const char* filename,
                                             const SHA1& sha1,
                                             const Native::FuncTable& nativeFuncs,
                                             bool& internal_error,
                                             CompileAbortMode mode,
                                             const PackageInfo& packageInfo) {
  try {
    return assemble_string(hhas.c_str(),
                           hhas.length(),
                           filename,
                           sha1,
                           nativeFuncs,
                           packageInfo,
                           false);  /* swallow errors */
  } catch (const FatalErrorException&) {
    throw;
  } catch (const TranslationFatal& ex) {
    // Assembler returned an error when building this unit
    if (mode >= CompileAbortMode::VerifyErrors) internal_error = true;
    return ex.what();
  } catch (const AssemblerUnserializationError& ex) {
    // Variable unserializer threw when called from the assembler, treat it
    // as an internal error.
    internal_error = true;
    return ex.what();
  } catch (const AssemblerError& ex) {
    if (mode >= CompileAbortMode::VerifyErrors) internal_error = true;

    if (RuntimeOption::EvalHackCompilerVerboseErrors) {
      auto const msg = folly::sformat(
        "{}\n"
        "========== PHP Source ==========\n"
        "{}\n"
        "========== HackC Result ==========\n"
        "{}\n",
        ex.what(),
        code,
        hhas
      );
      Logger::FError("HackC Generated a bad unit: {}", msg);
      return msg;
    } else {
      return ex.what();
    }
  } catch (const std::exception& ex) {
    internal_error = true;
    return ex.what();
  }
}

////////////////////////////////////////////////////////////////////////////////

CompilerResult hackc_compile(
  const char* code,
  const char* filename,
  const SHA1& sha1,
  const Native::FuncTable& nativeFuncs,
  bool isSystemLib,
  bool forDebuggerEval,
  bool& internal_error,
  const RepoOptionsFlags& options,
  CompileAbortMode mode,
  hackc::DeclProvider* provider
) {
  hackc::NativeEnv native_env{
    .decl_provider = reinterpret_cast<uint64_t>(provider),
    .filepath = filename,
    .hhbc_flags = hackc::HhbcFlags {
      .repo_authoritative = RO::RepoAuthoritative,
      .jit_enable_rename_function = RO::EvalJitEnableRenameFunction,
      .log_extern_compiler_perf = RO::EvalLogExternCompilerPerf,
      .enable_intrinsics_extension = RO::EnableIntrinsicsExtension,
      .emit_cls_meth_pointers = RO::EvalEmitClsMethPointers,
      .emit_meth_caller_func_pointers = RO::EvalEmitMethCallerFuncPointers,
      .fold_lazy_class_keys = RO::EvalFoldLazyClassKeys,
    },
    .parser_flags = hackc::ParserFlags {
      .enable_class_level_where_clauses = RO::EnableClassLevelWhereClauses,
    },
    .flags = hackc::EnvFlags {
      .is_systemlib = isSystemLib,
      .for_debugger_eval = forDebuggerEval,
      .disable_toplevel_elaboration = false,
      .enable_ir = false,
    }
  };
  options.initAliasedNamespaces(native_env);
  options.initHhbcFlags(native_env.hhbc_flags);
  options.initParserFlags(native_env.parser_flags);
  if (RO::EvalHackCompilerInheritConfig) {
    for (auto& [k, v] : RO::IncludeRoots) {
      native_env.include_roots.emplace_back(hackc::StringMapEntry{k, v});
    }
  }

  auto const fromHackCUnit = [&]() -> CompilerResult {
    rust::Box<hackc::UnitWrapper> unit_wrapped = [&] {
      tracing::Block _{
        "hackc_translator",
        [&] {
          return tracing::Props{}
            .add("filename", filename ? filename : "")
            .add("code_size", strlen(code));
        }
      };
      return hackc::compile_unit_from_text_cpp_ffi(native_env, code);
    }();

    auto const bcSha1 = SHA1(hash_unit(*unit_wrapped));
    const hackc::hhbc::Unit* unit = hackCUnitRaw(unit_wrapped);
    auto hackCResult = unitEmitterFromHackCUnitHandleErrors(
      *unit, filename, sha1, bcSha1, nativeFuncs,
      internal_error, mode, options.packageInfo()
    );
    return hackCResult;
  };

  auto const fromHhas = [&]() -> CompilerResult {
    // Invoke hackc, producing a rust Vec<u8> containing HHAS.
    rust::Vec<uint8_t> hhas_vec = [&] {
      tracing::Block _{
        "hackc",
        [&] {
          return tracing::Props{}
            .add("filename", filename ? filename : "")
            .add("code_size", strlen(code));
        }
      };
      return hackc::compile_from_text_cpp_ffi(native_env, code);
    }();
    auto const hhas = std::string(hhas_vec.begin(), hhas_vec.end());

    // Assemble HHAS into a UnitEmitter, or a std::string if there were errors.
    auto res = assemble_string_handle_errors(code,
                                            hhas,
                                            filename,
                                            sha1,
                                            nativeFuncs,
                                            internal_error,
                                            mode,
                                            options.packageInfo());
    return res;
  };

  if (RO::EvalVerifyTranslateHackC) {
    auto const disassembleResult = [](CompilerResult& res) -> std::string {
      if (auto ue = boost::get<std::unique_ptr<UnitEmitter>>(&res)) {
        (*ue)->finish();
        return disassemble((*ue)->create().get());
      }
      return boost::get<std::string>(res);
    };
    auto assemblerResult =  fromHhas();
    auto hackCTranslatorResult = fromHackCUnit();
    auto const assemblerOut = disassembleResult(assemblerResult);
    auto const hackCTranslatorOut = disassembleResult(hackCTranslatorResult);
    always_assert(hackCTranslatorOut == assemblerOut);
    return hackCTranslatorResult;
  }
  if (RO::EvalTranslateHackC){
    return fromHackCUnit();
  } else {
    return fromHhas();
  }
}

/// A simple UnitCompiler that invokes hackc in-process.
struct HackcUnitCompiler final : UnitCompiler {
  using UnitCompiler::UnitCompiler;

  std::unique_ptr<UnitEmitter> compile(
    bool& cacheHit,
    HhvmDeclProvider*,
    CompileAbortMode = CompileAbortMode::Never) override;

  const char* getName() const override { return "HackC"; }
};

// UnitCompiler which first tries to retrieve the UnitEmitter via the
// g_unit_emitter_cache_hook. If that fails, delegate to the
// UnitCompiler produced by the "makeFallback" lambda (this avoids
// having to create the fallback UnitEmitter until we need it). The
// lambda will only be called once. Its output is cached afterwards.
struct CacheUnitCompiler final : UnitCompiler {
  CacheUnitCompiler(LazyUnitContentsLoader& loader,
                    const char* filename,
                    const Native::FuncTable& nativeFuncs,
                    AutoloadMap* map,
                    bool isSystemLib,
                    bool forDebuggerEval,
                    std::function<std::unique_ptr<UnitCompiler>()> makeFallback)
    : UnitCompiler{
        loader,
        filename,
        nativeFuncs,
        map,
        isSystemLib,
        forDebuggerEval
      }
    , m_makeFallback{std::move(makeFallback)} {}

  std::unique_ptr<UnitEmitter> compile(
    bool& cacheHit,
    HhvmDeclProvider*,
    CompileAbortMode = CompileAbortMode::Never) override;

  const char* getName() const override { return "Cache"; }
private:
  std::function<std::unique_ptr<UnitCompiler>()> m_makeFallback;
  std::unique_ptr<UnitCompiler> m_fallback;
};

////////////////////////////////////////////////////////////////////////////////
}

CompilerAbort::CompilerAbort(const std::string& filename,
                             const std::string& error)
  : std::runtime_error{
      folly::sformat(
        "Encountered an internal error while processing HHAS for {}, "
        "bailing because Eval.AbortBuildOnCompilerError is set\n\n{}",
        filename, error
      )
    }
{
}

ParseFactsResult extract_facts(
  const std::string& filename,
  const std::string& code,
  const RepoOptionsFlags& options,
  folly::StringPiece expect_sha1
) {
  auto const get_facts = [&](const std::string& source_text) -> ParseFactsResult {
    auto actual_sha1 = string_sha1(source_text);
    if (!expect_sha1.empty()) {
      if (actual_sha1 != expect_sha1) {
        return folly::sformat(
            "Unexpected SHA1: {} != {}", actual_sha1, expect_sha1
        );
      }
    }
    try {
      hackc::DeclParserConfig decl_config;
      options.initDeclConfig(decl_config);
      auto const decls = hackc::direct_decl_parse(
        decl_config, filename, source_text
      );
      auto const facts = hackc::decls_to_facts_cpp_ffi(decls, actual_sha1);
      rust::String json = hackc::facts_to_json_cpp_ffi(
          facts, /* pretty= */ false
      );
      return FactsJSONString { std::string(json) };
    } catch (const std::exception& e) {
      return FactsJSONString { "" }; // Swallow errors from HackC
    }
  };

  if (!code.empty()) {
    return get_facts(code);
  } else {
    auto w = Stream::getWrapperFromURI(StrNR(filename));
    if (!(w && dynamic_cast<FileStreamWrapper*>(w))) {
      throwErrno("Failed to extract facts: Could not get FileStreamWrapper.");
    }
    const auto f = w->open(StrNR(filename), "r", 0, nullptr);
    if (!f) throwErrno("Failed to extract facts: Could not read source code.");
    auto const str = f->read();
    return get_facts(str.get()->toCppString());
  }
}

FfpResult ffp_parse_file(
  const std::string& contents,
  const RepoOptionsFlags& options
) {
  auto const env = options.getParserEnvironment();
  auto const json = hackc_parse_positioned_full_trivia_cpp_ffi(contents, env);
  return FfpJSONString { std::string(json.begin(), json.end()) };
}

std::unique_ptr<UnitCompiler>
UnitCompiler::create(LazyUnitContentsLoader& loader,
                     const char* filename,
                     const Native::FuncTable& nativeFuncs,
                     AutoloadMap* map,
                     bool isSystemLib,
                     bool forDebuggerEval) {
  auto make = [&loader, &nativeFuncs, filename, isSystemLib, forDebuggerEval,
               map] {
    return std::make_unique<HackcUnitCompiler>(
      loader,
      filename,
      nativeFuncs,
      map,
      isSystemLib,
      forDebuggerEval
    );
  };

  if (g_unit_emitter_cache_hook && !forDebuggerEval) {
    return std::make_unique<CacheUnitCompiler>(
      loader,
      filename,
      nativeFuncs,
      map,
      isSystemLib,
      false,
      std::move(make)
    );
  } else {
    return make();
  }
}

std::unique_ptr<UnitEmitter> UnitCompiler::compile(
    bool& cacheHit,
    CompileAbortMode mode) {
  auto provider = HhvmDeclProvider::create(
    m_map,
    m_loader.options(),
    m_loader.repoRoot()
  );
  return compile(cacheHit, provider.get(), mode);
}

std::unique_ptr<UnitEmitter> compile_unit(
  const char* code,
  const char* filename,
  const SHA1& sha1,
  const Native::FuncTable& nativeFuncs,
  bool isSystemLib,
  bool forDebuggerEval,
  const RepoOptionsFlags& options,
  CompileAbortMode mode,
  hackc::DeclProvider* provider
) {
  bool ice = false;
  auto res = hackc_compile(code, filename, sha1, nativeFuncs, isSystemLib,
      forDebuggerEval, ice, options, mode, provider);
  auto unitEmitter = match<std::unique_ptr<UnitEmitter>>(res,
    [&] (std::unique_ptr<UnitEmitter>& ue) {
      ue->finish();
      return std::move(ue);
    },
    [&] (std::string& err) {
      switch (mode) {
      case CompileAbortMode::Never:
        break;
      case CompileAbortMode::AllErrorsNull: {
        auto ue = std::unique_ptr<UnitEmitter>{};
        ue->finish();
        return ue;
      }
      case CompileAbortMode::OnlyICE:
      case CompileAbortMode::VerifyErrors:
      case CompileAbortMode::AllErrors:
        // Abort on internal compile errors as appropriate based on mode
        if (ice) throw CompilerAbort{filename, err};
      }
      return createFatalUnit(makeStaticString(filename), sha1,
                             FatalOp::Runtime, err);
    }
  );

  if (unitEmitter) unitEmitter->m_ICE = ice;
  return unitEmitter;
}

std::unique_ptr<UnitEmitter> HackcUnitCompiler::compile(
    bool& cacheHit,
    HhvmDeclProvider* provider,
    CompileAbortMode mode) {
  cacheHit = false;
  auto unitEmitter = compile_unit(m_loader.contents().data(),
                                  m_filename,
                                  m_loader.sha1(),
                                  m_nativeFuncs,
                                  m_isSystemLib,
                                  m_forDebuggerEval,
                                  m_loader.options(),
                                  mode,
                                  provider);
  if (unitEmitter && provider) unitEmitter->m_deps = provider->getFlatDeps();
  return unitEmitter;
}

std::unique_ptr<UnitEmitter>
CacheUnitCompiler::compile(bool& cacheHit,
                           HhvmDeclProvider* provider,
                           CompileAbortMode mode) {
  assertx(g_unit_emitter_cache_hook);
  cacheHit = true;
  return g_unit_emitter_cache_hook(
    m_filename,
    m_loader.sha1(),
    m_loader.fileLength(),
    provider,
    [&] (bool wantsICE) {
      if (!m_fallback) m_fallback = m_makeFallback();
      assertx(m_fallback);
      return m_fallback->compile(
        cacheHit,
        provider,
        wantsICE ? mode : CompileAbortMode::AllErrorsNull
      );
    },
    m_nativeFuncs
  );
}

////////////////////////////////////////////////////////////////////////////////
}

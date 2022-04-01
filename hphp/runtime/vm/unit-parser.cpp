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

#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs"
#include "hphp/hack/src/hackc/hhbc-ast.h"
#include "hphp/hack/src/parser/ffi_bridge/parser_ffi.rs"
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
#include "hphp/zend/zend-strtod.h"

namespace HPHP {

TRACE_SET_MOD(unit_parse);

UnitEmitterCacheHook g_unit_emitter_cache_hook = nullptr;
static std::string s_misc_config;

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

CompilerResult assemble_string_handle_errors(const char* code,
                                             const std::string& hhas,
                                             const char* filename,
                                             const SHA1& sha1,
                                             const Native::FuncTable& nativeFuncs,
                                             bool& internal_error,
                                             CompileAbortMode mode) {
  try {
    return assemble_string(hhas.c_str(),
                           hhas.length(),
                           filename,
                           sha1,
                           nativeFuncs,
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

struct ConfigBuilder {
  template<typename T>
  ConfigBuilder& addField(folly::StringPiece key, const T& data) {
    if (!m_config.isObject()) {
      m_config = folly::dynamic::object();
    }

    m_config[key] = folly::dynamic::object(
      "global_value", folly::toDynamic(data));

    return *this;
  }

  std::string toString() const {
    return m_config.isNull() ? "" : folly::toJson(m_config);
  }

 private:
  folly::dynamic m_config{nullptr};
};

CompilerResult hackc_compile(
  const char* code,
  const char* filename,
  const SHA1& sha1,
  const Native::FuncTable& nativeFuncs,
  bool isSystemLib,
  bool forDebuggerEval,
  bool& internal_error,
  const RepoOptionsFlags& options,
  CompileAbortMode mode
) {
  // Create DeclProvider. Returns nullptr if disabled or too early.
  auto aliased_namespaces = options.getAliasedNamespacesConfig();
  auto provider = HhvmDeclProvider::create(options);

  uint8_t flags = make_env_flags(
    isSystemLib,                    // is_systemlib
    false,                          // is_evaled
    forDebuggerEval,                // for_debugger_eval
    true,                           // dump_symbol_refs
    false                           // disable_toplevel_elaboration
  );

  NativeEnv const native_env{
    reinterpret_cast<uint64_t>(provider.get()),
    reinterpret_cast<uint64_t>(provider ? &hhvm_decl_provider_get_decl : nullptr),
    filename,
    aliased_namespaces,
    s_misc_config,
    RuntimeOption::EvalEmitClassPointers,
    RuntimeOption::CheckIntOverflow,
    options.getCompilerFlags(),
    options.getParserFlags(),
    flags
  };

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
    return hackc_compile_from_text_cpp_ffi(native_env, code);
  }();
  auto const hhas = std::string(hhas_vec.begin(), hhas_vec.end());

  // Assemble HHAS into a UnitEmitter, or a std::string if there were errors.
  auto res = assemble_string_handle_errors(code,
                                           hhas,
                                           filename,
                                           sha1,
                                           nativeFuncs,
                                           internal_error,
                                           mode);

  if (RO::EvalTranslateHackC) {
    rust::Box<HackCUnitWrapper> unit_wrapped =
      hackc_compile_unit_from_text_cpp_ffi(native_env, code);
    rust::Vec<uint8_t> hhbc {hackc_unit_to_string_cpp_ffi(native_env, *unit_wrapped)};
    std::string hhasString(hhbc.begin(), hhbc.end());

    auto const assemblerOut = [&]() -> std::string {
      if (auto ue = boost::get<std::unique_ptr<UnitEmitter>>(&res)) {
        (*ue)->finish();
        return disassemble((*ue)->create().get(), true);
      }
      return boost::get<std::string>(res);
    }();
    const hackc::hhbc::HackCUnit* unit = hackCUnitRaw(unit_wrapped);
    try {
      auto const ue = unitEmitterFromHackCUnit(*unit,
                                               filename,
                                               sha1,
                                               nativeFuncs,
                                               hhasString);
      auto const hackCTranslatorOut = disassemble(ue->create().get(), true);

      if (hackCTranslatorOut.length() != assemblerOut.length()) {
        Logger::FError("HackC Translator incorrect length: {}\n", filename);
      }

      UNUSED auto start_of_line = 0;
      for(int i = 0; i < hackCTranslatorOut.length(); i++) {
        if (hackCTranslatorOut[i] == '\n' || assemblerOut[i] == '\n') {
          start_of_line = i;
        }
        if (hackCTranslatorOut[i] != assemblerOut[i]) {
          while (i < hackCTranslatorOut.length() &&
                 hackCTranslatorOut[i] != '\n' && assemblerOut[i] != '\n') {
            i++;
          }
          Logger::FError("HackC Translator incorrect: {}\n", filename);
          ITRACE(3, "HackC Translator: {}\n\nassembler: {}\n\n",
            hackCTranslatorOut.substr(start_of_line,i),
            assemblerOut.substr(start_of_line,i)
          );
          ITRACE(4, "HackC Translator:\n{}\n", hackCTranslatorOut);
          ITRACE(4, "assembler:\n{}\n", assemblerOut);
          break;
        }
      }
    } catch (const TranslationFatal& ex) {
      auto const err = ex.what();
      if (std::strcmp(err, assemblerOut.c_str())) {
        Logger::FError("HackC Translator incorrect error: {}\n", filename);
      }
      ITRACE(4, "HackC Translator Err: {}\n", err);
    }
  }

  return res;
}
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

void compilers_start() {
  // Some configs, like IncludeRoots, can't easily be Config::Bind(ed), so here
  // we create a place to dump miscellaneous config values HackC might want.
  s_misc_config = []() -> std::string {
    if (RuntimeOption::EvalHackCompilerInheritConfig) {
      return ConfigBuilder()
        .addField("hhvm.include_roots", RuntimeOption::IncludeRoots)
        .toString();
    }
    return "";
  }();
}

ParseFactsResult extract_facts(
  const std::string& filename,
  const std::string& code,
  const RepoOptionsFlags& options
) {
  auto const get_facts = [&](const std::string& source_text) -> ParseFactsResult {
    try {
      std::int32_t decl_flags = options.getDeclFlags();
      rust::Box<DeclParserOptions> decl_opts =
        hackc_create_direct_decl_parse_options(
          decl_flags,
          options.getAliasedNamespacesConfig());
      DeclResult decls = hackc_direct_decl_parse(*decl_opts, filename, source_text);
      FactsResult facts = hackc_decls_to_facts_cpp_ffi(decl_flags, decls, source_text);
      rust::String facts_as_json = hackc_facts_to_json_cpp_ffi(facts, source_text);
      return FactsJSONString { std::string(facts_as_json) };
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
  auto const parse_tree = hackc_parse_positioned_full_trivia_cpp_ffi(contents, env);
  return FfpJSONString {  std::string(parse_tree) };
}

std::unique_ptr<UnitCompiler>
UnitCompiler::create(LazyUnitContentsLoader& loader,
                     const char* filename,
                     const Native::FuncTable& nativeFuncs,
                     bool isSystemLib,
                     bool forDebuggerEval) {
  auto make = [&loader, &nativeFuncs, filename, isSystemLib, forDebuggerEval] {
    return std::make_unique<HackcUnitCompiler>(
      loader,
      filename,
      nativeFuncs,
      isSystemLib,
      forDebuggerEval
    );
  };

  if (g_unit_emitter_cache_hook && !forDebuggerEval) {
    return std::make_unique<CacheUnitCompiler>(
      loader,
      filename,
      nativeFuncs,
      isSystemLib,
      false,
      std::move(make)
    );
  } else {
    return make();
  }
}

std::unique_ptr<UnitEmitter> HackcUnitCompiler::compile(
    bool& cacheHit,
    CompileAbortMode mode) {
  auto ice = false;
  cacheHit = false;

  auto res = hackc_compile(m_loader.contents().data(),
                           m_filename,
                           m_loader.sha1(),
                           m_nativeFuncs,
                           m_isSystemLib,
                           m_forDebuggerEval,
                           ice,
                           m_loader.options(),
                           mode);
  auto unitEmitter = match<std::unique_ptr<UnitEmitter>>(
    res,
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
        // run_compiler will promote errors to ICE as appropriate based on mode
        if (ice) throw CompilerAbort{m_filename, err};
      }
      return createFatalUnit(
        makeStaticString(m_filename),
        m_loader.sha1(),
        FatalOp::Runtime,
        err
      );
    }
  );

  if (unitEmitter) unitEmitter->m_ICE = ice;
  return unitEmitter;
}

std::unique_ptr<UnitEmitter>
CacheUnitCompiler::compile(bool& cacheHit, CompileAbortMode mode) {
  assertx(g_unit_emitter_cache_hook);
  cacheHit = true;
  return g_unit_emitter_cache_hook(
    m_filename,
    m_loader.sha1(),
    m_loader.fileLength(),
    [&] (bool wantsICE) {
      if (!m_fallback) m_fallback = m_makeFallback();
      assertx(m_fallback);
      return m_fallback->compile(
        cacheHit,
        wantsICE ? mode : CompileAbortMode::AllErrorsNull
      );
    },
    m_nativeFuncs
  );
}

////////////////////////////////////////////////////////////////////////////////
}

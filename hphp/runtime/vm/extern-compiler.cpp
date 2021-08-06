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

#include "hphp/runtime/vm/extern-compiler.h"

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

#include "hphp/hack/src/facts/rust_facts_ffi.h"
#include "hphp/hack/src/hhbc/compile_ffi.h"
#include "hphp/hack/src/hhbc/compile_ffi_types.h"
#include "hphp/hack/src/parser/positioned_full_trivia_parser_ffi.h"
#include "hphp/hack/src/parser/positioned_full_trivia_parser_ffi_types.h"
#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/hhvm_decl_provider.h"
#include "hphp/runtime/vm/unit-emitter.h"
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

#include <iostream>

namespace HPHP {

TRACE_SET_MOD(extern_compiler);

UnitEmitterCacheHook g_unit_emitter_cache_hook = nullptr;
static std::string s_misc_config;

namespace {

struct CompileException : Exception {
  explicit CompileException(const std::string& what) : Exception(what) {}
  template<class... A>
  explicit CompileException(A&&... args)
    : Exception(folly::sformat(std::forward<A>(args)...))
  {}
};

[[noreturn]] void throwErrno(const char* what) {
  throw CompileException("{}: {}", what, folly::errnoStr(errno));
}

CompilerResult assemble_string_handle_errors(const char* code,
                                             const char* hhas,
                                             size_t hhas_size,
                                             const char* filename,
                                             const SHA1& sha1,
                                             const Native::FuncTable& nativeFuncs,
                                             bool& internal_error,
                                             CompileAbortMode mode) {
  try {
    return assemble_string(hhas,
                           hhas_size,
                           filename,
                           sha1,
                           nativeFuncs,
                           false);  /* swallow errors */
  } catch (const FatalErrorException&) {
    throw;
  } catch (const AssemblerFatal& ex) {
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
  int len,
  const char* filename,
  const SHA1& sha1,
  const Native::FuncTable& nativeFuncs,
  bool forDebuggerEval,
  bool& internal_error,
  const RepoOptions& options,
  CompileAbortMode mode
) {
  using namespace ::HPHP::hackc::compile;

  std::uint8_t flags = 0;
  if(forDebuggerEval) {
    flags |= FOR_DEBUGGER_EVAL;
  }
  if(!SystemLib::s_inited) {
    flags |= IS_SYSTEMLIB;
  }
  if (RuntimeOption::EvalEnableDecl) {
    flags |= ENABLE_DECL;
  }
  flags |= DUMP_SYMBOL_REFS;

  HhvmDeclProvider decl_provider;

  std::string aliased_namespaces = options.getAliasedNamespacesConfig();

  native_environment const native_env{
    &hhvm_decl_provider_get_decl,
    &decl_provider,
    filename,
    aliased_namespaces.data(),
    s_misc_config.data(),
    RuntimeOption::EvalEmitClassPointers,
    RuntimeOption::CheckIntOverflow,
    options.getCompilerFlags(),
    options.getParserFlags(),
    flags
  };

  output_config const output{true, nullptr};

  std::array<char, 256> buf;
  buf.fill(0);
  error_buf_t error_buf {buf.data(), buf.size()};

  hackc_compile_from_text_ptr hhas{
    hackc_compile_from_text(&native_env, code, &output, &error_buf)
  };
  if (!hhas) throwErrno(buf.data());
  return assemble_string_handle_errors(code,
                                       hhas.get(),
                                       strlen(hhas.get()),
                                       filename,
                                       sha1,
                                       nativeFuncs,
                                       internal_error,
                                       mode);
}
////////////////////////////////////////////////////////////////////////////////
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
  const char* code,
  int len,
  const RepoOptions& options
) {
  auto const get_facts = [&](const char* source_text) -> ParseFactsResult {
    try {
      hackc_extract_as_json_ptr facts{
        hackc_extract_as_json(options.getFactsFlags(), filename.data(), source_text, true)
      };
      if (facts) {
        std::string facts_str{facts.get()};
        return FactsJSONString { facts_str };
      }
      return FactsJSONString { "" }; // Swallow errors from HackC
    } catch (const std::exception& e) {
      return FactsJSONString { "" }; // Swallow errors from HackC
    }
  };

  if (code && code[0] != '\0') {
    return get_facts(code);
  } else {
    auto w = Stream::getWrapperFromURI(StrNR(filename));
    if (!(w && dynamic_cast<FileStreamWrapper*>(w))) {
      throwErrno("Failed to extract facts: Could not get FileStreamWrapper.");
    }
    const auto f = w->open(StrNR(filename), "r", 0, nullptr);
    if (!f) throwErrno("Failed to extract facts: Could not read source code.");
    auto const str = f->read();
    return get_facts(str.data());
  }
}

FfpResult ffp_parse_file(
  std::string file,
  const char *contents,
  int size,
  const RepoOptions& options
) {
  auto const env = options.getParserEnvironment();
  hackc_parse_positioned_full_trivia_ptr parse_tree{
    hackc_parse_positioned_full_trivia(file.c_str(), contents, &env)
  };
  if (parse_tree) {
    std::string ffp_str{parse_tree.get()};
    return FfpJSONString { ffp_str };
  } else {
    return FfpJSONString { "{}" };
  }
}

std::unique_ptr<UnitCompiler>
UnitCompiler::create(const char* code,
                     int codeLen,
                     const char* filename,
                     const SHA1& sha1,
                     const Native::FuncTable& nativeFuncs,
                     bool forDebuggerEval,
                     const RepoOptions& options) {
  auto const make = [code, codeLen, filename, sha1, forDebuggerEval,
                     &nativeFuncs, &options] {
    return std::make_unique<HackcUnitCompiler>(
      code,
      codeLen,
      filename,
      sha1,
      nativeFuncs,
      forDebuggerEval,
      options
    );
  };

  if (g_unit_emitter_cache_hook && !forDebuggerEval) {
    return std::make_unique<CacheUnitCompiler>(
      code,
      codeLen,
      filename,
      sha1,
      nativeFuncs,
      false,
      options,
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
  auto res = hackc_compile(m_code,
                           m_codeLen,
                           m_filename,
                           m_sha1,
                           m_nativeFuncs,
                           m_forDebuggerEval,
                           ice,
                           m_options,
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
        if (ice) {
          fprintf(
            stderr,
            "Encountered an internal error while processing HHAS for %s, "
            "bailing because Eval.AbortBuildOnCompilerError is set\n\n%s",
            m_filename, err.data()
          );
          _Exit(1);
        }
      }
      return createFatalUnit(
        makeStaticString(m_filename),
        m_sha1,
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
    m_sha1,
    m_codeLen,
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

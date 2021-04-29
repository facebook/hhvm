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

#pragma once

#include <boost/variant.hpp>
#include <folly/Range.h>

#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/unit-emitter.h"

namespace HPHP {

namespace Native {
struct FuncTable;
}

struct SHA1;

struct BadCompilerException : Exception {
  explicit BadCompilerException(const std::string& what) : Exception(what) {}
  template<class... A>
  explicit BadCompilerException(A&&... args)
    : Exception(folly::sformat(std::forward<A>(args)...))
  {}
};

void compilers_start();
void compilers_shutdown();
void compilers_set_user(const std::string& username);
void compilers_detach_after_fork();

// On success return a verified unit, and on failure return a string stating the
// type of error encountered
using CompilerResult = boost::variant<std::unique_ptr<UnitEmitter>,std::string>;

struct FactsParser {
  virtual ~FactsParser() {
  }
};

std::unique_ptr<FactsParser> acquire_facts_parser();

struct FactsJSONString {
  std::string value;
};

struct FfpJSONString {
  std::string value;
};

// On success returns a Json with value containing json-serialized results of
// facts extraction and on failure returns a string with error text
using ParseFactsResult = boost::variant<FactsJSONString, std::string>;
using FfpResult = boost::variant<FfpJSONString, std::string>;

ParseFactsResult extract_facts(const FactsParser&,
                               const std::string& filename,
                               const char* code,
                               int len,
                               const RepoOptions& options);
FfpResult ffp_parse_file(std::string file,
                         const char* contents,
                         int size,
                         const RepoOptions& options);

std::string hackc_version();

/*
 * Controls handling for errors detected during compilation. By default errors
 * are converted to Fatal units.
 *
 * Never - all errors generate fatal units
 * OnlyICE - internal errors (IPC and serializer) raise abort signals
 * VerifyErrors - internal errors and assembler verification errors raise aborts
 * AllErrors - internal, verification, and hackc errors raise aborts
 * AllErrorsNull - like AllErrors, but returns nullptr instead of aborting
 */
enum class CompileAbortMode {
  Never, OnlyICE, VerifyErrors, AllErrors, AllErrorsNull
};

struct UnitCompiler {
  UnitCompiler(const char* code,
               int codeLen,
               const char* filename,
               const SHA1& sha1,
               const Native::FuncTable& nativeFuncs,
               bool forDebuggerEval,
               const RepoOptions& options)
      : m_code(code),
        m_codeLen(codeLen),
        m_filename(filename),
        m_sha1(sha1),
        m_nativeFuncs(nativeFuncs),
        m_forDebuggerEval(forDebuggerEval),
        m_options(options)
    {}
  virtual ~UnitCompiler() = default;

  static std::unique_ptr<UnitCompiler> create(
    const char* code,
    int codeLen,
    const char* filename,
    const SHA1& sha1,
    const Native::FuncTable& nativeFuncs,
    bool forDebuggerEval,
    const RepoOptions& options);

  virtual std::unique_ptr<UnitEmitter> compile(
    bool& cacheHit,
    CompileAbortMode = CompileAbortMode::Never) = 0;

  virtual const char* getName() const = 0;

 protected:
  const char* m_code;
  int m_codeLen;
  const char* m_filename;
  SHA1 m_sha1;
  const Native::FuncTable& m_nativeFuncs;
  bool m_forDebuggerEval;
  const RepoOptions& m_options;
};

struct HackcUnitCompiler : public UnitCompiler {
  using UnitCompiler::UnitCompiler;

  virtual std::unique_ptr<UnitEmitter> compile(
    bool& cacheHit,
    CompileAbortMode = CompileAbortMode::Never) override;

  virtual const char* getName() const override { return "HackC"; }
};

// UnitCompiler which first tries to retrieve the UnitEmitter via the
// g_unit_emitter_cache_hook. If that fails, delegate to the
// UnitCompiler produced by the "makeFallback" lambda (this avoids
// having to create the fallback UnitEmitter until we need it). The
// lambda will only be called once. Its output is cached afterwards.
struct CacheUnitCompiler : public UnitCompiler {
  explicit CacheUnitCompiler(
      const char* code,
      int codeLen,
      const char* filename,
      const SHA1& sha1,
      const Native::FuncTable& nativeFuncs,
      bool forDebuggerEval,
      const RepoOptions& options,
      std::function<std::unique_ptr<UnitCompiler>()> makeFallback)
    : UnitCompiler{code, codeLen, filename, sha1,
                   nativeFuncs, forDebuggerEval, options}
    , m_makeFallback{std::move(makeFallback)} {}

  virtual std::unique_ptr<UnitEmitter> compile(
    bool& cacheHit,
    CompileAbortMode = CompileAbortMode::Never) override;

  virtual const char* getName() const override { return "Cache"; }
private:
  std::function<std::unique_ptr<UnitCompiler>()> m_makeFallback;
  std::unique_ptr<UnitCompiler> m_fallback;
};

using UnitEmitterCacheHook =
  std::unique_ptr<UnitEmitter> (*)(
    const char*,
    const SHA1&,
    folly::StringPiece::size_type,
    // First parameter is whether ICE UEs are allowed. If not, a
    // nullptr will be returned for ICE UEs instead.
    const std::function<std::unique_ptr<UnitEmitter>(bool)>&,
    const Native::FuncTable&
  );
extern UnitEmitterCacheHook g_unit_emitter_cache_hook;

}

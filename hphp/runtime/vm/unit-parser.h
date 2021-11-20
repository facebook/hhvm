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

struct LazyUnitContentsLoader;
struct SHA1;

void compilers_start();

// On success return a verified unit, and on failure return a string stating the
// type of error encountered
using CompilerResult = boost::variant<std::unique_ptr<UnitEmitter>,std::string>;

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

ParseFactsResult extract_facts(const std::string& filename,
                               const std::string& code,
                               const RepoOptions& options);
FfpResult ffp_parse_file(const std::string& contents,
                         const RepoOptions& options);

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
  UnitCompiler(LazyUnitContentsLoader& loader,
               const char* filename,
               const Native::FuncTable& nativeFuncs,
               bool forDebuggerEval)
    : m_loader{loader}
    , m_filename{filename}
    , m_nativeFuncs{nativeFuncs}
    , m_forDebuggerEval{forDebuggerEval}
  {}
  virtual ~UnitCompiler() = default;

  static std::unique_ptr<UnitCompiler> create(
    LazyUnitContentsLoader& loader,
    const char* filename,
    const Native::FuncTable& nativeFuncs,
    bool forDebuggerEval
  );

  virtual std::unique_ptr<UnitEmitter> compile(
    bool& cacheHit,
    CompileAbortMode = CompileAbortMode::Never) = 0;

  virtual const char* getName() const = 0;

 protected:
  LazyUnitContentsLoader& m_loader;
  const char* m_filename;
  const Native::FuncTable& m_nativeFuncs;
  bool m_forDebuggerEval;
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
  CacheUnitCompiler(LazyUnitContentsLoader& loader,
                    const char* filename,
                    const Native::FuncTable& nativeFuncs,
                    bool forDebuggerEval,
                    std::function<std::unique_ptr<UnitCompiler>()> makeFallback)
    : UnitCompiler{loader, filename, nativeFuncs, forDebuggerEval}
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

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

#include <variant>
#include <folly/Range.h>

#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/unit-emitter.h"

namespace HPHP {

namespace hackc {
struct DeclProvider;
}

namespace Native {
struct FuncTable;
}

struct HhvmDeclProvider;
struct LazyUnitContentsLoader;
struct SHA1;

// On success return a verified unit, and on failure return a string stating the
// type of error encountered
using CompilerResult = std::variant<std::unique_ptr<UnitEmitter>,std::string>;

struct FactsBinaryString {
  std::string value;
};

struct FfpJSONString {
  std::string value;
};

// On success returns a std::string with value containing binary-serialized
// results of facts extraction. On failure returns a string with error text
using ParseFactsResult = std::variant<FactsBinaryString, std::string>;
using FfpResult = std::variant<FfpJSONString, std::string>;

// Parse facts from the given file with `options`.
// If expect_sha1 is non-empty, return an error message if
// the actual sha1 of the source text is different.
ParseFactsResult extract_facts(const std::string& filename,
                               const RepoOptionsFlags& options,
                               folly::StringPiece expect_sha1);

FfpResult ffp_parse_file(const std::string& contents,
                         const RepoOptionsFlags& options);

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

// Thrown if CompilerAbortMode is anything other than None or
// AllErrorsNull and we obtain an ICE unit.
struct CompilerAbort : public std::runtime_error {
  CompilerAbort(const std::string& filename,
                const std::string& error);
};

enum class CodeSource {
  User,
  Eval,
  Debugger,
  Systemlib,
};

struct UnitCompiler {
  UnitCompiler(LazyUnitContentsLoader& loader,
               CodeSource codeSource,
               const char* filename,
               const Extension* extension,
               AutoloadMap* map,
               bool isSystemLib,
               bool forDebuggerEval)
    : m_loader{loader}
    , m_codeSource(codeSource)
    , m_filename{filename}
    , m_extension{extension}
    , m_map{map}
    , m_isSystemLib{isSystemLib}
    , m_forDebuggerEval{forDebuggerEval}
  {}
  virtual ~UnitCompiler() = default;

  static std::unique_ptr<UnitCompiler> create(
    LazyUnitContentsLoader& loader,
    CodeSource codeSource,
    const char* filename,
    const Extension* extension,
    AutoloadMap* map,
    bool isSystemLib,
    bool forDebuggerEval
  );

  virtual const char* getName() const = 0;

  virtual std::unique_ptr<UnitEmitter> compile(
    bool& cacheHit,
    HhvmDeclProvider*,
    CompileAbortMode = CompileAbortMode::Never) = 0;

  std::unique_ptr<UnitEmitter> compile(
    bool& cacheHit,
    CompileAbortMode = CompileAbortMode::Never);

 protected:
  LazyUnitContentsLoader& m_loader;
  CodeSource m_codeSource;
  const char* m_filename;
  const Extension* m_extension;
  AutoloadMap* m_map;
  bool m_isSystemLib;
  bool m_forDebuggerEval;
};

using UnitEmitterCacheHook =
  std::unique_ptr<UnitEmitter> (*)(
    const char*,
    const SHA1&,
    folly::StringPiece::size_type,
    HhvmDeclProvider* provider,
    // First parameter is whether ICE UEs are allowed. If not, a
    // nullptr will be returned for ICE UEs instead.
    const std::function<std::unique_ptr<UnitEmitter>(bool)>&
  );
// Alias used as the layer registration type.
using UnitEmitterCacheLayer = UnitEmitterCacheHook;

extern UnitEmitterCacheHook g_unit_emitter_cache_hook;

// Explicit priorities for the composable cache-layer dispatcher.
// Lower values are outermost (called first): LocalDisk -> RemoteCache -> HackC.
enum class UnitEmitterCacheHookPriority : int {
  LocalDisk   = 10,  // e.g. SBCC — fastest, purely local
  RemoteCache = 20,  // e.g. BCCache — network-backed
};

// Freeze registration and install the dispatcher as g_unit_emitter_cache_hook.
// Only installs the dispatcher if at least one layer was registered.
// Called once during hphp_process_init().
void initUnitEmitterCacheDispatcher();

// Register a named cache layer with an explicit dispatch priority.
// Lower priority values are invoked first (outermost) in the chain:
//   LocalDisk(10) -> RemoteCache(20) -> HackC.
// Duplicate layer names abort at startup.
// Must be called during moduleLoad(), before initUnitEmitterCacheDispatcher()
// freezes registration in hphp_process_init().
void registerUnitEmitterCacheLayer(
    std::string_view name,
    UnitEmitterCacheHookPriority priority,
    UnitEmitterCacheLayer layer);

// Invoke hackc directly without any caching.
std::unique_ptr<UnitEmitter> compile_unit(
  folly::StringPiece code,
  CodeSource codeSource,
  const char* filename,
  const SHA1& sha1,
  const Extension* extension,
  bool isSystemLib,
  bool forDebuggerEval,
  const RepoOptionsFlags& options,
  UnitEmitterAttributes,
  CompileAbortMode mode,
  hackc::DeclProvider* provider
);

}

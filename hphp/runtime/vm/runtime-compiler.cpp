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

#include "hphp/runtime/vm/runtime-compiler.h"

#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/recorder.h"
#include "hphp/runtime/base/replayer.h"
#include "hphp/runtime/base/repo-autoload-map.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/unit-cache.h"

#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/unit-parser.h"

#include "hphp/util/assertions.h"
#include "hphp/util/sha1.h"

#include "hphp/zend/zend-string.h"

#include <folly/Likely.h>
#include <folly/Range.h>

namespace HPHP {

TRACE_SET_MOD(runtime);

namespace {

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<UnitEmitter> parse(LazyUnitContentsLoader& loader,
                                   CodeSource codeSource,
                                   const char* filename,
                                   const Extension* extension,
                                   AutoloadMap* map,
                                   Unit** releaseUnit,
                                   bool isSystemLib,
                                   bool forDebuggerEval) {
  if (!filename) filename = "";

  tracing::Block _{
    "parse",
    [&] {
      return tracing::Props{}
        .add("filename", filename ? filename : "")
        .add("code_size", loader.fileLength());
    }
  };
  auto const wasLoaded = loader.didLoad();
  SCOPE_EXIT {
    if (!wasLoaded && loader.didLoad()) {
      tracing::updateName("parse-load");
    }
  };

  // Do not count memory used during parsing/emitting towards OOM.
  MemoryManager::SuppressOOM so(*tl_heap);

  SCOPE_ASSERT_DETAIL("parsing file") { return filename; };
  std::unique_ptr<Unit> unit;
  SCOPE_EXIT {
    if (unit && releaseUnit) *releaseUnit = unit.release();
  };

  // We don't want to invoke the JIT when trying to run PHP code.
  auto const prevFolding = RID().getJitFolding();
  RID().setJitFolding(true);
  SCOPE_EXIT { RID().setJitFolding(prevFolding); };

  std::unique_ptr<UnitEmitter> ue;
  // Check if this file contains raw HHAS instead of Hack source code.
  // This is dictated by a special file extension.
  if (RuntimeOption::EvalAllowHhas &&
      folly::StringPiece(filename).endsWith(".hhas")) {
    auto const& contents = loader.contents();
    ue = assemble_string(
      contents.data(),
      contents.size(),
      filename,
      loader.sha1(),
      extension,
      RepoOptions::forFile(filename).packageInfo()
    );
  }

  // If ue != nullptr then we assembled it above, so don't feed it into
  // the extern compiler
  if (!ue) {
    auto uc = UnitCompiler::create(
      loader,
      codeSource,
      filename,
      extension,
      map,
      isSystemLib,
      forDebuggerEval
    );
    assertx(uc);
    tracing::BlockNoTrace _2{"unit-compiler-run"};
    SCOPE_EXIT {
      if (!wasLoaded && loader.didLoad()) {
        tracing::updateName("unit-compiler-run-load");
      }
    };
    try {
      bool cache_hit;
      ue = uc->compile(cache_hit);
    } catch (const CompilerAbort& exn) {
      fprintf(stderr, "%s", exn.what());
      _Exit(HPHP_EXIT_FAILURE);
    }
  }

  assertx(ue);
  ue->logDeclInfo();

  if (UNLIKELY(RO::EvalRecordReplay && RO::EvalReplay && ue->m_sn == -1)) {
    if (codeSource != CodeSource::Eval) {
      ue->m_sn = Replayer::onParse(filename);
    }
  }

  return ue;
}

///////////////////////////////////////////////////////////////////////////////

}

Unit* compile_file(LazyUnitContentsLoader& loader,
                   CodeSource codeSource,
                   const char* filename,
                   const Extension* extension,
                   AutoloadMap* map,
                   Unit** releaseUnit) {
  assertx(!filename || filename[0] != '/' || filename[1] != ':');
  return parse(
    loader,
    codeSource,
    filename,
    extension,
    map,
    releaseUnit,
    false,
    false
  )->create().release();
}

Unit* compile_string(const char* s,
                     size_t sz,
                     CodeSource codeSource,
                     const char* fname,
                     const Extension* extension,
                     AutoloadMap* map,
                     const RepoOptions& options,
                     bool isSystemLib,
                     bool forDebuggerEval) {
  // If the file is too large it may OOM the request
  MemoryManager::SuppressOOM so{*tl_heap};

  auto const name = fname ? fname : "";
  auto const sha1 = SHA1{mangleUnitSha1(
    string_sha1(folly::StringPiece{s, sz}), name, options.flags()
  )};
  LazyUnitContentsLoader loader{sha1, {s, sz}, options.flags(), options.dir()};
  return parse(
    loader,
    codeSource,
    fname,
    extension,
    map,
    nullptr,
    isSystemLib,
    forDebuggerEval
  )->create().release();
}

Unit* compile_systemlib_string(const char* s, size_t sz,
                               const char* fname,
                               const Extension* extension) {
  assertx(fname && fname[0] == '/' && fname[1] == ':');

  if (UNLIKELY(RO::EvalRecordReplay && RO::EvalRecordSampleRate)) {
    Recorder::onCompileSystemlibString(fname);
  }

  if (RuntimeOption::RepoAuthoritative &&
      !(RO::EvalRecordReplay && RO::EvalReplay)) {
    if (auto u = lookupSyslibUnit(makeStaticString(fname))) {
      return u;
    }
  }

  auto const sha1 = SHA1{mangleUnitSha1(
    string_sha1(folly::StringPiece{s, sz}),
    fname,
    RepoOptions::defaults().flags()
  )};
  auto const& defaults = RepoOptions::defaults();
  LazyUnitContentsLoader loader{
    sha1,
    {s, sz},
    defaults.flags(),
    defaults.dir()
  };
  // We provide the empty autoload map here: there are no symbols that are going
  // to be provided outside of systemlib itself. The created decl provider will
  // hook into `s_builtin_symbols` to provide the relevant decls.
  RepoAutoloadMap empty_map({}, {}, {}, {}, {});
  auto ue = parse(
    loader,
    CodeSource::Systemlib,
    fname,
    extension,
    RuntimeOption::EvalEnableDecl ? &empty_map : nullptr,
    nullptr,
    true,
    false
  );
  always_assert(ue);

  auto u = ue->create();
  SystemLib::registerUnitEmitter(std::move(ue));
  return u.release();
}

Unit* compile_debugger_string(
  const char* s, size_t sz, const RepoOptions& options
) {
  auto const map = [] () -> AutoloadMap* {
    if (!AutoloadHandler::s_instance) {
      // It is not safe to autoinit AutoloadHandler outside a normal request.
      return nullptr;
    }
    return AutoloadHandler::s_instance->getAutoloadMap();
  }();

  return compile_string(
    s,
    sz,
    CodeSource::Debugger,
    nullptr,
    nullptr,
    map,
    options,
    false,
    true
  );
}

}

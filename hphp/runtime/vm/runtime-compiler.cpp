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
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/zend/zend-string.h"
#include "hphp/util/assertions.h"
#include "hphp/util/sha1.h"
#include <folly/Range.h>

namespace HPHP {

TRACE_SET_MOD(runtime);

CompileStringFn g_hphp_compiler_parse;

void hphp_compiler_init() {
  g_hphp_compiler_parse(nullptr, 0, SHA1(), nullptr, Native::s_noNativeFuncs,
                        nullptr, false, RepoOptions::defaults());
}

Unit* compile_file(const char* s, size_t sz, const SHA1& sha1,
                   const char* fname, const Native::FuncTable& nativeFuncs,
                   const RepoOptions& options, Unit** releaseUnit) {
  return g_hphp_compiler_parse(s, sz, sha1, fname, nativeFuncs, releaseUnit,
                               false, options);
}

Unit* compile_string(const char* s,
                     size_t sz,
                     const char* fname,
                     const Native::FuncTable& nativeFuncs,
                     const RepoOptions& options,
                     bool forDebuggerEval) {
  auto const name = fname ? fname : "";
  auto const sha1 = SHA1{
    mangleUnitSha1(string_sha1(folly::StringPiece{s, sz}), name, options)};
  // NB: fname needs to be long-lived if generating a bytecode repo because it
  // can be cached via a Location ultimately contained by ErrorInfo for printing
  // code errors.
  return g_hphp_compiler_parse(s, sz, sha1, fname, nativeFuncs, nullptr,
                               forDebuggerEval, options);
}

Unit* compile_systemlib_string(const char* s, size_t sz, const char* fname,
                               const Native::FuncTable& nativeFuncs) {
  assertx(fname[0] == '/' && fname[1] == ':');
  if (RuntimeOption::RepoAuthoritative) {
    if (auto u = lookupSyslibUnit(makeStaticString(fname), nativeFuncs)) {
      return u;
    }
  }
  return compile_string(s, sz, fname, nativeFuncs, RepoOptions::defaults());
}

Unit* compile_debugger_string(
  const char* s, size_t sz, const RepoOptions& options
) {
  return compile_string(
    s,
    sz,
    nullptr,
    Native::s_noNativeFuncs,
    options,
    true
  );
}

}

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
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/zend/zend-string.h"
#include "hphp/util/assertions.h"
#include "hphp/util/md5.h"
#include <folly/Range.h>

namespace HPHP {

TRACE_SET_MOD(runtime);

CompileStringFn g_hphp_compiler_parse;

void hphp_compiler_init() {
  g_hphp_compiler_parse(nullptr, 0, MD5(), nullptr, Native::s_noNativeFuncs,
                        nullptr, false);
}

Unit* compile_file(const char* s, size_t sz, const MD5& md5,
                   const char* fname, const Native::FuncTable& nativeFuncs,
                   Unit** releaseUnit) {
  return g_hphp_compiler_parse(s, sz, md5, fname, nativeFuncs, releaseUnit,
                               false);
}

Unit* compile_string(const char* s,
                     size_t sz,
                     const char* fname,
                     const Native::FuncTable& nativeFuncs,
                     bool forDebuggerEval) {
  auto const md5 = MD5{mangleUnitMd5(string_md5(folly::StringPiece{s, sz}))};
  if (auto u = Repo::get().loadUnit(
        fname ? fname : "",
        md5, nativeFuncs).release()) {
    return u;
  }
  // NB: fname needs to be long-lived if generating a bytecode repo because it
  // can be cached via a Location ultimately contained by ErrorInfo for printing
  // code errors.
  return g_hphp_compiler_parse(s, sz, md5, fname, nativeFuncs, nullptr,
                               forDebuggerEval);
}

Unit* compile_systemlib_string(const char* s, size_t sz, const char* fname,
                               const Native::FuncTable& nativeFuncs) {
  assertx(fname[0] == '/' && fname[1] == ':');
  if (auto u = lookupSyslibUnit(makeStaticString(fname), nativeFuncs)) {
    return u;
  }
  return compile_string(s, sz, fname, nativeFuncs);
}

Unit* compile_debugger_string(const char* s, size_t sz) {
  return compile_string(s, sz, nullptr, Native::s_noNativeFuncs, true);
}

}

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

Unit* compile_file(const char* s, size_t sz, const MD5& md5,
                   const char* fname, Unit** releaseUnit) {
  return g_hphp_compiler_parse(s, sz, md5, fname, releaseUnit, false);
}

Unit* compile_string(const char* s,
                     size_t sz,
                     const char* fname,
                     bool forDebuggerEval) {
  auto const md5 = MD5{mangleUnitMd5(string_md5(folly::StringPiece{s, sz}))};
  if (auto u = Repo::get().loadUnit(fname ? fname : "", md5).release()) {
    return u;
  }
  // NB: fname needs to be long-lived if generating a bytecode repo because it
  // can be cached via a Location ultimately contained by ErrorInfo for printing
  // code errors.
  return g_hphp_compiler_parse(s, sz, md5, fname, nullptr, forDebuggerEval);
}

Unit* compile_systemlib_string(const char* s, size_t sz, const char* fname) {
  assertx(fname[0] == '/' && fname[1] == ':');
  if (auto u = lookupSyslibUnit(makeStaticString(fname))) return u;

  return compile_string(s, sz, fname);
}

}

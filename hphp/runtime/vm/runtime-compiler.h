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
#ifndef incl_HPHP_VM_RUNTIME_COMPILER_H_
#define incl_HPHP_VM_RUNTIME_COMPILER_H_

#include <cstddef>

namespace HPHP {

struct Unit;
struct SHA1;
struct RepoOptions;

namespace Native {
struct FuncTable;
}

// Called once at process startup to initialize compiler
void hphp_compiler_init();

// If set, releaseUnit will contain a pointer to any extraneous unit created due
// to race-conditions while compiling
Unit* compile_file(const char* s, size_t sz, const SHA1& sha1,
                   const char* fname, const Native::FuncTable& nativeFuncs,
                   const RepoOptions&, Unit** releaseUnit = nullptr);

// If forDebuggerEval is true, and the unit contains a single expression
// statement, then we will turn the statement into a return statement while
// compiling so that eval-ing this unit will return the value.
// forDebuggerEval is only meant to be used by debuggers, where humans may
// enter a statement and we wish to eval it and display the resulting value,
// if any.
Unit* compile_string(const char* s, size_t sz, const char* fname,
                     const Native::FuncTable& nativeFuncs, const RepoOptions&,
                     bool forDebuggerEval = false);

Unit* compile_debugger_string(const char* s, size_t sz, const RepoOptions&);

Unit* compile_systemlib_string(const char* s, size_t sz, const char* fname,
                               const Native::FuncTable& nativeFuncs);

/*
 * A few functions are exposed by libhphp_analysis and used in
 * VM-specific parts of the runtime.
 *
 * Currently we handle this by using these global pointers, which must
 * be set up before you use those parts of the runtime.
 */

using CompileStringFn = Unit* (*)(const char*, int, const SHA1&, const char*,
                                  const Native::FuncTable&, Unit**, bool,
                                  const RepoOptions&);

extern CompileStringFn g_hphp_compiler_parse;

}
#endif

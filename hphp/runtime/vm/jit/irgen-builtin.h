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
#ifndef incl_HPHP_JIT_IRGEN_BUILTIN_H_
#define incl_HPHP_JIT_IRGEN_BUILTIN_H_

#include <cstdint>

namespace HPHP {

struct Func;

namespace jit {

struct SSATmp;
struct Type;

namespace irgen {

struct IRGS;

//////////////////////////////////////////////////////////////////////

// Returns the index of a parameter that should be a vanilla-array like,
// or -1 if our optimized irgen for that builtin has no such requirement.
int getBuiltinVanillaParam(const char* name);

SSATmp* optimizedCallIsObject(IRGS&, SSATmp*);

// The builtin's inferred return type (without taking into account coercion
// failures). Appropriate for CallBuiltin. For regular PHP calls to a builtin,
// use callReturnType() instead.
Type builtinReturnType(const Func* builtin);
Type builtinOutType(const Func* builtin, uint32_t i);

//////////////////////////////////////////////////////////////////////

}}}

#endif

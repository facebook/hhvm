/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_IRGEN_FUNC_PROLOGUE_H
#define incl_HPHP_JIT_IRGEN_FUNC_PROLOGUE_H

#include <cstdint>

#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct IRGS;

///////////////////////////////////////////////////////////////////////////////

namespace irgen {

///////////////////////////////////////////////////////////////////////////////

void emitFuncPrologue(IRGS& env, uint32_t argc, TransID transID);

///////////////////////////////////////////////////////////////////////////////

}}}

#endif // incl_HPHP_JIT_IRGEN_FUNC_PROLOGUE_H

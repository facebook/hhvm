/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_VASM_EMIT_H_
#define incl_HPHP_JIT_VASM_EMIT_H_

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace jit {

struct Abi;
struct AsmInfo;
struct CGMeta;
struct IRUnit;
struct Vtext;
struct Vunit;

///////////////////////////////////////////////////////////////////////////////

/*
 * Optimize, lower for the specified architecture, register allocate, and
 * perform more optimizations on the given unit.
 */
void optimizeX64(Vunit&, const Abi&);
void optimizeARM(Vunit&, const Abi&);
void optimizePPC64(Vunit&, const Abi&);

/*
 * Emit code for the given unit using the given code areas. The unit must have
 * already been through the corresponding optimizeArch() function.
 */
void emitX64(const Vunit&, Vtext&, CGMeta&, AsmInfo*);
void emitARM(const Vunit&, Vtext&, CGMeta&, AsmInfo*);
void emitPPC64(const Vunit&, Vtext&, CGMeta&, AsmInfo*);

/*
 * Emit code for the given Vunit, which must already be register-allocated, to
 * the given CodeBlocks.
 */
void emitVunit(Vunit& vunit, const IRUnit& unit,
               CodeCache::View code, CGMeta& fixups,
               Annotations* annotations = nullptr);

///////////////////////////////////////////////////////////////////////////////
}}

#endif

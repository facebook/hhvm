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

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct Abi;
struct AsmInfo;
struct Vtext;
struct Vunit;

///////////////////////////////////////////////////////////////////////////////

/*
 * Optimize, lower for x64, register allocator, and perform more optimizations
 * on `unit'.
 */
void optimizeX64(Vunit& unit, const Abi&);

/*
 * Emit code for the given unit using the given code areas. The unit should
 * have already been through optimizeX64().
 */
void emitX64(const Vunit&, Vtext&, AsmInfo*);

/*
 * Optimize, register allocate, and emit ARM code for the given unit.
 */
void finishARM(Vunit&, Vtext&, const Abi&, AsmInfo*);

/*
 * Optimize, register allocate, and emit PPC64 code for the given unit.
 */
void finishPPC64(Vunit&, Vtext&, const Abi&, AsmInfo*);

///////////////////////////////////////////////////////////////////////////////
}}

#endif

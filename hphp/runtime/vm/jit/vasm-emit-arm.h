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

namespace HPHP::jit {

struct Abi;
struct AsmInfo;
struct CGMeta;
struct Vtext;
struct Vunit;

namespace arm {

///////////////////////////////////////////////////////////////////////////////

/*
 * Optimize, lower for the specified architecture, register allocate (if
 * desired), and perform more optimizations on the given unit.
 */
void optimize(Vunit&, const Abi&, bool regalloc);

/*
 * Emit code for the given unit using the given code areas. The unit must have
 * already been through the optimize() function.
 */
void emit(Vunit&, Vtext&, CGMeta&, AsmInfo*);

///////////////////////////////////////////////////////////////////////////////

}

}

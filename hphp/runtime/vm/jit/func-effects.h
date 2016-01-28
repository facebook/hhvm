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

#ifndef incl_HPHP_FUNC_EFFECTS_H_
#define incl_HPHP_FUNC_EFFECTS_H_

namespace HPHP {
class Func;

namespace jit {
class NormalizedInstruction;

/*
 * Could the CPP builtin function `callee` destroy the locals
 * in the environment of its caller?
 *
 * This occurs, e.g., if `func' is extract().
 */
bool builtinFuncDestroysLocals(const Func*);
/*
 * Could `inst' clobber the locals in the environment of `caller'?
 *
 * This occurs, e.g., if `inst' is a call to extract().
 */
bool callDestroysLocals(const NormalizedInstruction&, const Func*);

/*
 * Could the CPP builtin function `callee` attempt to read the caller frame?
 *
 * This occurs, e.g., if `func' is is_callable().
 */
bool builtinFuncNeedsCallerFrame(const Func*);

/*
 * Could `inst' attempt to read the caller frame?
 *
 * This occurs, e.g., if `inst' is a call to is_callable().
 */
bool callNeedsCallerFrame(const NormalizedInstruction&, const Func*);
}}

#endif

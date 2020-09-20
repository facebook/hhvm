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

namespace HPHP { namespace jit {

struct SSATmp;

namespace irgen {

struct IRGS;

///////////////////////////////////////////////////////////////////////////////

/*
 * Return control from an inlined callee to the caller.
 *
 * This also does bookkeeping for `env' to pop the current inlined frame.
 */
void implInlineReturn(IRGS& env);

/*
 * Emit a return from an inlined function.
 */
void retFromInlined(IRGS&);

/*
 * Exit the (now suspended) inline frame. The frame must no longer be live, and
 * its contents must now reside in waithandle.
 */
void suspendFromInlined(IRGS&, SSATmp* waithandle);

///////////////////////////////////////////////////////////////////////////////

}}}


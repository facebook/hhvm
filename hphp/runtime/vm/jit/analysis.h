/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_ANALYSIS_H_
#define incl_HPHP_ANALYSIS_H_

namespace HPHP { namespace jit {

struct SSATmp;
struct IRInstruction;

//////////////////////////////////////////////////////////////////////

/*
 * Utility routines for performing very simple queries on the IR, that may be
 * needed from multiple optimization passes, or even during IR generation.
 *
 * The functions in this module never mutate the IR, even if they take
 * pointers-to-non-const.
 */

//////////////////////////////////////////////////////////////////////

/*
 * Returns the canonical version of the given value by tracing through any
 * passthrough instructions (Mov, CheckType, etc...).
 */
const SSATmp* canonical(const SSATmp*);
SSATmp* canonical(SSATmp*);

/*
 * Assuming sp is the VM stack pointer either from inside an FPI region or an
 * inlined call, find the SpillFrame instruction that defined the current
 * frame. Returns nullptr if the frame can't be found.
 */
IRInstruction* findSpillFrame(SSATmp* sp);

//////////////////////////////////////////////////////////////////////

}}


#endif

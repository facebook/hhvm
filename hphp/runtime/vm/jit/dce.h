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
#ifndef incl_HPHP_DCE_H_
#define incl_HPHP_DCE_H_

namespace HPHP { namespace jit {

struct IRUnit;

//////////////////////////////////////////////////////////////////////

/*
 * Perform very minimal DCE required to preserve the IR invariant that there
 * are no unreachable blocks in the unit.  All other IR invariants must hold
 * before running this function.
 */
void mandatoryDCE(IRUnit&);

/*
 * Perform a full DCE pass, which implies the mandatoryDCE as well.
 *
 * All IR invariants must hold before running this pass, except the one
 * restored by mandatoryDCE, which we allow passes to violate and rely on a
 * fullDCE run to fix.
 */
void fullDCE(IRUnit&);

//////////////////////////////////////////////////////////////////////

}}

#endif

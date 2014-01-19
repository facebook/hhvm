/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VM_CHECK_H_
#define incl_HPHP_VM_CHECK_H_

namespace HPHP {
namespace JIT {

class IRUnit;
struct RegAllocInfo;

/*
 * Ensure valid SSA properties; each SSATmp must be defined exactly once,
 * only used in positions dominated by the definition.
 */
bool checkCfg(const IRUnit&);

/*
 * We can't have SSATmps spanning php-level calls, except for frame
 * pointers and constant values.
 *
 * We have no callee-saved registers in php, and there'd be nowhere to
 * spill these because all translations share the spill space.
 */
bool checkTmpsSpanningCalls(const IRUnit&);

/*
 * Make sure there's no shuffle instructions. (called right before register
 * allocation.
 */
bool checkNoShuffles(const IRUnit&);

/*
 * Check register and spill slot assignments; registers and spill slots must
 * contain the correct SSATmp value at every point of use.
 */
bool checkRegisters(const IRUnit&, const RegAllocInfo&);

}}

#endif

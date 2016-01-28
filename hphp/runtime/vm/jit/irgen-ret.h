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
#ifndef incl_HPHP_IRGEN_RET_H_
#define incl_HPHP_IRGEN_RET_H_

namespace HPHP { namespace jit {
struct IRSPOffset;
struct IRGS;
}}
namespace HPHP { namespace jit { namespace irgen {

//////////////////////////////////////////////////////////////////////

/*
 * Helper to return the offset from the IR stack pointer to the frame's return
 * value slot.  This is used during return sequences from non-resumables.
 */
IRSPOffset offsetToReturnSlot(IRGS&);

//////////////////////////////////////////////////////////////////////

}}}

#endif

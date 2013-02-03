/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#ifndef incl_HHIR_OPT_H_
#define incl_HHIR_OPT_H_

namespace HPHP { namespace VM { namespace JIT {

class Trace;
class TraceBuilder;
class IRFactory;
class IRInstruction;

/*
 * The main optimization passes, in the order they run.
 */
void optimizeMemoryAccesses(Trace*, IRFactory*);
void eliminateDeadCode(Trace*, IRFactory*);
void optimizeJumps(Trace*, IRFactory*);

/*
 * Run all the optimization passes.
 */
void optimizeTrace(Trace*, TraceBuilder*);

}}}

#endif

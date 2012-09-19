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
#ifndef __HHIR_DCE_H__
#define __HHIR_DCE_H__


#include "opt.h"


namespace HPHP {
namespace VM {
namespace JIT {


class Trace;
class IRFactory;

#define DEAD 0
#define LIVE 1
// An IncRef is marked as REFCOUNT_CONSUMED[_OFF_TRACE], if it is consumed by
// an instruction other than DecRefNZ that decrements the ref count.
// * REFCOUNT_CONSUMED: consumed by such an instruction in the main trace
// * REFCOUNT_CONSUMED_OFF_TRACE: consumed by such an instruction only
//   in exit traces.
#define REFCOUNT_CONSUMED 2
#define REFCOUNT_CONSUMED_OFF_TRACE 3

void eliminateDeadCode(Trace* trace, IRFactory* factory);

void removeDeadInstructions(Trace* trace);

} } }


#endif // __HHIR_DCE_H__

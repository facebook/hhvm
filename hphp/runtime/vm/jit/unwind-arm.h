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
#ifndef incl_HPHP_RUNTIME_JIT_UNWIND_ARM_H
#define incl_HPHP_RUNTIME_JIT_UNWIND_ARM_H

#include <exception>

#include "hphp/vixl/a64/instructions-a64.h"
#include "hphp/vixl/a64/simulator-a64.h"

#include "hphp/runtime/vm/jit/unwind-x64.h"

namespace HPHP { namespace jit { namespace arm {

vixl::Instruction* simulatorExceptionHook(vixl::Simulator* sim,
                                          std::exception_ptr exn);

}}}

#endif

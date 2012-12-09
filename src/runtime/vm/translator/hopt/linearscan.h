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

#ifndef incl_HPHP_VM_LINEAR_SCAN_H_
#define incl_HPHP_VM_LINEAR_SCAN_H_

#include <boost/noncopyable.hpp>

#include "runtime/vm/translator/physreg.h"
#include "runtime/vm/translator/abi-x64.h"
#include "runtime/vm/translator/hopt/ir.h"
#include "runtime/vm/translator/hopt/tracebuilder.h"
#include "runtime/vm/translator/hopt/codegen.h"

namespace HPHP { namespace VM { namespace JIT {

/*
 * The main entry point for register allocation.  Called prior to code
 * generation.
 */
void allocRegsForTrace(Trace*, IRFactory*, TraceBuilder*);

}}}

#endif

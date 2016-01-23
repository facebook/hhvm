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

#include "hphp/runtime/vm/jit/unwind.h"

#include <vector>
#include <memory>
#ifndef _MSC_VER
#include <cxxabi.h>
#endif
#include <boost/mpl/identity.hpp>

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unwind.h"
#include "hphp/util/abi-cxx.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"

namespace HPHP { namespace jit {

rds::Link<UnwindRDS> unwindRdsInfo(rds::kInvalidHandle);

_Unwind_Reason_Code
tc_unwind_personality(int version,
                      _Unwind_Action actions,
                      uint64_t exceptionClass,
                      _Unwind_Exception* exceptionObj,
                      _Unwind_Context* context) {

  if (arch() == Arch::X64) {
    return x64::tc_unwind_personality(version,
                                      actions,
                                      exceptionClass,
                                      exceptionObj,
                                      context);
  } else {
   not_implemented();
   // using a dummy return to avoid return non void function warning
   _Unwind_Reason_Code dummy;
   return dummy;
 }
}

TCUnwindInfo tc_unwind_resume(ActRec* fp) {

  if (arch() == Arch::X64) {
    return x64::tc_unwind_resume(fp);

  } else {
   not_implemented();
   // using a dummy return to avoid return non void function warning
   TCUnwindInfo dummy;
   return dummy;
 }
}

///////////////////////////////////////////////////////////////////////////////

UnwindInfoHandle
register_unwind_region(unsigned char* startAddr, size_t size) {

  if (arch() == Arch::X64) {
    return x64::register_unwind_region(startAddr, size);

  } else {
   not_implemented();
   UnwindInfoHandle dummy;
   return dummy;
  }

}

//////////////////////////////////////////////////////////////////////

}}

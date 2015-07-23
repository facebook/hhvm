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

#include "hphp/runtime/vm/jit/abi.h"

#include "hphp/runtime/base/arch.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/abi-x64.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

PhysReg rvmfp() {
  switch (arch()) {
    case Arch::X64:
      return x64::rVmFp;
    case Arch::ARM:
      return arm::rVmFp;
  }
  not_reached();
}

PhysReg rvmsp() {
  switch (arch()) {
    case Arch::X64:
      return x64::rVmSp;
    case Arch::ARM:
      return arm::rVmSp;
  }
  not_reached();
}

PhysReg rvmtl() {
  switch (arch()) {
    case Arch::X64:
      return x64::rVmTl;
    case Arch::ARM:
      return arm::rVmTl;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

PhysReg r_svcreq_req() {
  switch (arch()) {
    case Arch::X64:
      return reg::rdi;
    case Arch::ARM:
      not_implemented();
  }
  not_reached();
}

PhysReg r_svcreq_stub() {
  switch (arch()) {
    case Arch::X64:
      return x64::rAsm;
    case Arch::ARM:
      not_implemented();
  }
  not_reached();
}

PhysReg r_svcreq_arg(unsigned i) {
  switch (arch()) {
    case Arch::X64:
      return x64::kSvcReqArgRegs[i];
    case Arch::ARM:
      return arm::svcReqArgReg(i);
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

}}

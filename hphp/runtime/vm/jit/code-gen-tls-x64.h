/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/thread-local.h"

namespace HPHP::jit::x64::detail {

///////////////////////////////////////////////////////////////////////////////

/*
 * x86 terminology review: "Virtual addresses" are subject to both segmented
 * translation and paged translation.  "Linear addresses" are post-segmentation
 * address, subject only to paging.  C and C++ generally only have access to
 * bitwise linear addresses.
 *
 * On Linux, TLS data live at negative virtual addresses off FS: the first
 * datum is typically at VA(FS:-sizeof(datum)).  Linux's x64 ABI stores the
 * linear address of the base of TLS at VA(FS:0).  While this is just a
 * convention, it is firm: gcc builds binaries that assume it when, e.g.,
 * evaluating "&myTlsDatum".
 *
 * The virtual addresses of TLS data are not exposed to C/C++.  To figure it
 * out, we take a datum's linear address, and subtract it from the linear
 * address where TLS starts.
 */
template <typename T>
Vptr emitTLSAddr(Vout& /*v*/, TLSDatum<T> datum) {
  uintptr_t vaddr = uintptr_t(datum.tls) - tlsBase();
  return Vptr{baseless(vaddr), Segment::FS};
}

template<typename T>
Vreg emitTLSLea(Vout& v, TLSDatum<T> datum, int offset) {
  auto const base = v.makeReg();
  auto const addr = v.makeReg();
  v << load{Vptr{baseless(0), Segment::FS}, base};

  auto const tlsBaseAddr = tlsBase();
  auto const datumAddr = uintptr_t(datum.tls) + offset;
  if (datumAddr < tlsBaseAddr) {
    v << subq{v.cns(tlsBaseAddr - datumAddr), base, addr, v.makeReg()};
  } else {
    v << addq{v.cns(datumAddr - tlsBaseAddr), base, addr, v.makeReg()};
  }
  return addr;
}

///////////////////////////////////////////////////////////////////////////////

}

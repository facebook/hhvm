/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2016 Qualcomm Datacenter Technologies, Inc.            |
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

#ifndef incl_HPHP_VM_CODE_GEN_TLS_ARM_H_
#define incl_HPHP_VM_CODE_GEN_TLS_ARM_H_

#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/thread-local.h"

namespace HPHP { namespace jit { namespace arm { namespace detail {

///////////////////////////////////////////////////////////////////////////////

/*
 * See detailed comment in code-gen-tls-x64.h.
 */
template<typename T>
Vptr emitTLSAddr(Vout& v, TLSDatum<T> datum) {
  uintptr_t vaddr = uintptr_t(datum.tls) - tlsBase();

  auto const b = v.makeReg();
  v << mrs{vixl::SystemRegister::TPIDR_EL0, b};

  return b[vaddr];
}

template<typename T>
Vreg emitTLSLea(Vout& v, TLSDatum<T> datum, int offset) {
  auto const b = v.makeReg();
  v << lea{detail::emitTLSAddr(v, datum) + offset, b};
  return b;
}

///////////////////////////////////////////////////////////////////////////////

}}}}

#endif

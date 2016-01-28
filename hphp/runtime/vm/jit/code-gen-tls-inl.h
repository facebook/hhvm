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

#include "hphp/runtime/base/arch.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/phys-reg-saver.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/safe-cast.h"
#include "hphp/util/thread-local.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

template<typename T>
inline Vptr emitTLSAddr(Vout& v, TLSDatum<T> datum) {
  if (arch() != Arch::X64) not_implemented();
  return x64::detail::emitTLSAddr(v, datum);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_GCC_FAST_TLS

template<typename T>
inline void
emitTLSLoad(Vout& v, TLSDatum<ThreadLocalNoCheck<T>> datum, Vreg d) {
  auto const off = offsetof(ThreadLocalNoCheck<T>, m_node) +
                   offsetof(decltype(ThreadLocalNoCheck<T>::m_node), m_p);
  v << load{emitTLSAddr(v, datum) + safe_cast<int32_t>(off), d};
}

#else // USE_GCC_FAST_TLS

template<typename T>
inline void
emitTLSLoad(Vout& v, TLSDatum<ThreadLocalNoCheck<T>> datum, Vreg d) {
  // We don't know for sure what's live.
  PhysRegSaver(v, abi().gpUnreserved - abi().calleeSaved);

  v << vcall{
    CallSpec::direct(pthread_getspecific),
    v.makeVcallArgs({{v.cns(datum.tls->m_key)}}),
    v.makeTuple({d}),
    Fixup{},
    DestType::SSA
  };
}

#endif // USE_GCC_FAST_TLS

///////////////////////////////////////////////////////////////////////////////

}}

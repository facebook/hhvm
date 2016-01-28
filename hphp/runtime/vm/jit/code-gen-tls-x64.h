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

#ifndef incl_HPHP_VM_CODE_GEN_TLS_X64_H_
#define incl_HPHP_VM_CODE_GEN_TLS_X64_H_

#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/thread-local.h"

namespace HPHP { namespace jit { namespace x64 { namespace detail {

///////////////////////////////////////////////////////////////////////////////

/*
 * We support x64 linux (linked off FS), and x64 MacOS using some reverse
 * engineering of their generated code.
 */

#ifndef __APPLE__
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
template<typename T>
Vptr emitTLSAddr(Vout& v, TLSDatum<T> datum) {
  uintptr_t vaddr = uintptr_t(datum.tls) - tlsBase();
  return Vptr{baseless(vaddr), Vptr::FS};
}

#else // __APPLE__
/*
 * In MacOS an __thread variable has a "key" allocated in global memory, under
 * the name of the object.  The key consists of three longs.  The first holds
 * the address of a function, which if called with $rdi pointing at the key
 * (i.e., $rdi = &key; call 0x0($rdi)) will return the address of the thread
 * local (possibly allocating it, if this is the first time its been accessed
 * in the current thread).
 *
 * The function is very short (in the case that the thread local has already
 * been allocated), and preserves all the registers except for $rax (which gets
 * the address of the thread local)---so we could just use it to access the
 * thread locals.  But we can do better.
 *
 * The second long contains a gs-relative index to a pointer to the block of
 * memory containing the thread-local, and the third long contains the offset
 * to it within that block.  So if $rdi points at the key,
 *
 *  movq 8($rdi), $rax
 *  movq gs:(,$rax,8), $rax
 *  addq 16($rdi), $rax
 *
 * will get us the address of the thread local.
 *
 * But we can still do better.  key[1] and key[2] are constants for the
 * duration of the program; so we can burn their values into the tc:
 *
 *  movq gs:{key[1]*8}, $rax
 *  addq {key[2]}, $rax
 *
 * But note that the add will often fold into a following load or store.
 *
 * So the only remaining problem is to get the address of the key:
 *
 *   __asm__("lea %1, %0" : "=r"(ret) : "m"(tlvar));
 *
 * Unfortunately, clang is too smart here, and converts the lea into:
 *
 *  lea _tlvar, $rdi
 *  call ($rdi)
 *  move $rax, ret
 *
 * Fortunately, the helper preserves all regs (except $rax), and so $rdi now
 * has the address of the key.  We use that trick in the OSX version of
 * tls_datum().
 *
 * Finally note that all of this is only valid if the thread local has already
 * been accessed in the current thread---but we can easily ensure that (it's
 * already true for any ThreadLocalNoCheck variable).
 */
template<typename T>
Vptr emitTLSAddr(Vout& v, TLSDatum<T> datum) {
  auto const scratch = v.makeReg();

  v << load{Vptr{baseless(datum.raw[1] * 8), Vptr::GS}, scratch};
  return scratch[datum.raw[2]];
}

#endif

///////////////////////////////////////////////////////////////////////////////

}}}}

#endif

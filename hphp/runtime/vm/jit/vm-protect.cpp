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

#include "hphp/runtime/vm/jit/vm-protect.h"

#ifndef NDEBUG

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/vm-regs.h"

#include <sys/mman.h>
#include <unistd.h>

namespace HPHP { namespace jit {

namespace {

__thread const VMProtect* tl_active_prot{nullptr};

void protect() {
  rds::tl_base = nullptr;
  rds::threadInit();

  // The current thread may attempt to read the Gen numbers of the normal
  // portion of rds. These will all be invalid. No writes to non-persistent rds
  // should occur while this guard is active. Leave the first page unprotected
  // since surprise flags live there and we still do things in the jit that
  // could write to them (like allocating request memory).
  auto const base = static_cast<char*>(rds::tl_base) + sysconf(_SC_PAGESIZE);
  auto const protlen = rds::persistentSection().begin() - base;
  if (protlen > 0) {
    auto const result = mprotect(base, protlen, PROT_READ);
    always_assert(result == 0);
  }

  tl_regState = VMRegState::DIRTY;
  VMProtect::is_protected = true;
}

void unprotect(void* base, VMRegState state) {
  rds::threadExit();
  rds::tl_base = base;
  tl_regState = state;
  VMProtect::is_protected = false;
}

}

__thread bool VMProtect::is_protected{false};

VMProtect::VMProtect()
  : m_oldBase(rds::tl_base)
  , m_oldState(tl_regState)
{
  if (is_protected) return;

  assertx(tl_active_prot == nullptr);
  tl_active_prot = this;
  protect();
}

VMProtect::~VMProtect() {
  if (tl_active_prot != this) return;

  unprotect(m_oldBase, m_oldState);
  tl_active_prot = nullptr;
}

VMProtect::Pause::Pause() {
  if (auto const prot = tl_active_prot) {
    unprotect(prot->m_oldBase, prot->m_oldState);
  }
}

VMProtect::Pause::~Pause() {
  if (tl_active_prot) protect();
}

}}

#endif

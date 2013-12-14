/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/asm-x64.h"

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "folly/String.h"

#include "hphp/util/assertions.h"
#include "hphp/util/maphuge.h"

namespace HPHP { namespace JIT {

StoreImmPatcher::StoreImmPatcher(CodeBlock& cb, uint64_t initial,
                                 RegNumber reg,
                                 int32_t offset, RegNumber base) {
  X64Assembler as { cb };
  m_is32 = deltaFits(initial, sz::dword);
  if (m_is32) {
    as.store_imm64_disp_reg64(initial, offset, base);
    m_addr = cb.frontier() - 4;
  } else {
    as.mov_imm64_reg(initial, reg);
    m_addr = cb.frontier() - 8;
    as.store_reg64_disp_reg64(reg, offset, base);
  }
  assert((m_is32 ?  (uint64_t)*(int32_t*)m_addr : *(uint64_t*)m_addr)
         == initial);
}

void StoreImmPatcher::patch(uint64_t actual) {
  if (m_is32) {
    if (deltaFits(actual, sz::dword)) {
      *(uint32_t*)m_addr = actual;
    } else {
      not_reached();
    }
  } else {
    *(uint64_t*)m_addr = actual;
  }
}

}}

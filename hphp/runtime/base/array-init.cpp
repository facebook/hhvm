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
#include "hphp/runtime/base/array-init.h"

#include "hphp/runtime/base/vanilla-dict-defs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

DictInit::DictInit(size_t n, CheckAllocation)
  : ArrayInitBase(n, CheckAllocation{})
{
  if (n > std::numeric_limits<int>::max()) {
    tl_heap->forceOOM();
    check_non_safepoint_surprise();
  }
  auto const allocsz = VanillaDict::computeAllocBytes(
                         VanillaDict::computeScaleFromSize(n)
                       );
  if (UNLIKELY(allocsz > kMaxSmallSize && tl_heap->preAllocOOM(allocsz))) {
    check_non_safepoint_surprise();
  }
  m_arr = VanillaDict::MakeReserveDict(n);
  assertx(m_arr->hasExactlyOneRef());
  check_non_safepoint_surprise();
}

KeysetInit::KeysetInit(size_t n, CheckAllocation)
  : ArrayInitBase(n, CheckAllocation{})
{
  if (n > std::numeric_limits<int>::max()) {
    tl_heap->forceOOM();
    check_non_safepoint_surprise();
  }
  auto const allocsz = VanillaKeyset::computeAllocBytes(
                         VanillaKeyset::computeScaleFromSize(n)
                       );
  if (UNLIKELY(allocsz > kMaxSmallSize && tl_heap->preAllocOOM(allocsz))) {
    check_non_safepoint_surprise();
  }
  m_arr = VanillaKeyset::MakeReserveSet(n);
  assertx(m_arr->hasExactlyOneRef());
  check_non_safepoint_surprise();
}

//////////////////////////////////////////////////////////////////////

}

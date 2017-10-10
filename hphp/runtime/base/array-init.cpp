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
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/runtime-option.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

ArrayInit::ArrayInit(size_t n, Map, CheckAllocation)
  : ArrayInitBase(n, CheckAllocation{})
{
  if (n > std::numeric_limits<int>::max()) {
    tl_heap->forceOOM();
    check_non_safepoint_surprise();
  }
  auto const allocsz = MixedArray::computeAllocBytes(
                         MixedArray::computeScaleFromSize(n)
                       );
  if (UNLIKELY(allocsz > kMaxSmallSize && tl_heap->preAllocOOM(allocsz))) {
    check_non_safepoint_surprise();
  }
  m_arr = MixedArray::MakeReserveMixed(n);
  assert(m_arr->hasExactlyOneRef());
  check_non_safepoint_surprise();
}

DictInit::DictInit(size_t n, CheckAllocation)
  : ArrayInitBase(n, CheckAllocation{})
{
  if (n > std::numeric_limits<int>::max()) {
    tl_heap->forceOOM();
    check_non_safepoint_surprise();
  }
  auto const allocsz = MixedArray::computeAllocBytes(
                         MixedArray::computeScaleFromSize(n)
                       );
  if (UNLIKELY(allocsz > kMaxSmallSize && tl_heap->preAllocOOM(allocsz))) {
    check_non_safepoint_surprise();
  }
  m_arr = MixedArray::MakeReserveDict(n);
  assert(m_arr->hasExactlyOneRef());
  check_non_safepoint_surprise();
}

KeysetInit::KeysetInit(size_t n, CheckAllocation)
  : ArrayInitBase(n, CheckAllocation{})
{
  if (n > std::numeric_limits<int>::max()) {
    tl_heap->forceOOM();
    check_non_safepoint_surprise();
  }
  auto const allocsz = SetArray::computeAllocBytes(
                         SetArray::computeScaleFromSize(n)
                       );
  if (UNLIKELY(allocsz > kMaxSmallSize && tl_heap->preAllocOOM(allocsz))) {
    check_non_safepoint_surprise();
  }
  m_arr = SetArray::MakeReserveSet(n);
  assert(m_arr->hasExactlyOneRef());
  check_non_safepoint_surprise();
}

//////////////////////////////////////////////////////////////////////

}

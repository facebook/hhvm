/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

ArrayInit::ArrayInit(size_t n, Map)
#ifdef DEBUG
  : m_addCount(0)
  , m_expectedCount(n)
#endif
{
  m_data = MixedArray::MakeReserveMixed(n);
  assert(m_data->hasExactlyOneRef());
}

ArrayInit::ArrayInit(size_t n, Map, CheckAllocation)
#ifdef DEBUG
  : m_addCount(0)
  , m_expectedCount(n)
#endif
{
  if (n > std::numeric_limits<int>::max()) {
    MM().forceOOM();
    check_request_surprise_unlikely();
  }
  auto const allocsz = computeAllocBytes(computeScaleFromSize(n));
  if (UNLIKELY(allocsz > kMaxSmallSize && MM().preAllocOOM(allocsz))) {
    check_request_surprise_unlikely();
  }
  m_data = MixedArray::MakeReserveMixed(n);
  assert(m_data->hasExactlyOneRef());
  check_request_surprise_unlikely();
}

//////////////////////////////////////////////////////////////////////

}

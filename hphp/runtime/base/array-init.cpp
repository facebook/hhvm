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

DictInit::DictInit(size_t n)
#ifdef DEBUG
  : m_addCount(0)
  , m_expectedCount(n)
#endif
{
  m_data = MixedArray::MakeReserveDict(n);
  assert(m_data->hasExactlyOneRef());
}

DictInit::DictInit(size_t n, CheckAllocation)
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
  m_data = MixedArray::MakeReserveDict(n);
  assert(m_data->hasExactlyOneRef());
  check_request_surprise_unlikely();
}

KeysetInit::KeysetInit(size_t n, CheckAllocation)
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
  m_keyset = MixedArray::MakeReserveKeyset(n);
  assert(m_keyset->hasExactlyOneRef());
  check_request_surprise_unlikely();
}

//////////////////////////////////////////////////////////////////////

}

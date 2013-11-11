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
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/hphp-array.h"
#include "hphp/runtime/base/runtime-option.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// ArrayInit

ArrayInit::ArrayInit(size_t n)
#ifdef DEBUG
  : m_addCount(0)
  , m_expectedCount(n)
#endif
{
  if (!n) {
    m_data = HphpArray::GetStaticEmptyArray();
  } else {
    m_data = HphpArray::MakeReserve(n);
    m_data->setRefCount(0);
  }
}

ArrayInit::ArrayInit(size_t n, MapInit)
  : m_data(HphpArray::MakeReserve(n))
#ifdef DEBUG
  , m_addCount(0)
  , m_expectedCount(n)
#endif
{
  m_data->setRefCount(0);
}

///////////////////////////////////////////////////////////////////////////////
}

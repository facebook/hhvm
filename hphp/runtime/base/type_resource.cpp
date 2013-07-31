/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/variable_serializer.h"
#include "hphp/runtime/base/execution_context.h"
#include "hphp/runtime/base/builtin_functions.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const Resource Resource::s_nullResource = Resource();

Resource::~Resource() {
  if (LIKELY(m_px != 0)) {
    if (UNLIKELY(m_px->decRefCount() == 0)) {
      delete m_px;
    }
  }
}

Array Resource::toArray() const {
  return m_px ? m_px->o_toArray() : Array();
}

///////////////////////////////////////////////////////////////////////////////
}

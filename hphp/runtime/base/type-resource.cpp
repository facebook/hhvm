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

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/builtin-functions.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const Resource Resource::s_nullResource = Resource();

Resource::~Resource() {
  // force it out of line
}

Array Resource::toArray() const {
  return m_px ? m_px->o_toArray() : Array();
}

///////////////////////////////////////////////////////////////////////////////
}

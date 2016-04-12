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

#include "hphp/runtime/base/type-resource.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/type-string.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const Resource null_resource = Resource();

Resource::~Resource() {
  // force it out of line
}

String Resource::toString() const {
  return m_res ? m_res->data()->o_toString() : String();
}

Array Resource::toArray() const {
  return m_res ? m_res->data()->o_toArray() : Array();
}

const char* Resource::classname_cstr() const {
  return m_res->data()->o_getClassName().c_str();
}

void Resource::compileTimeAssertions() {
  static_assert(sizeof(Resource) == sizeof(req::ptr<ResourceHdr>), "");
}

///////////////////////////////////////////////////////////////////////////////
}

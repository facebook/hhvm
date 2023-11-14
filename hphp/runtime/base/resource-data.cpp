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
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// resources have a separate id space
static RDS_LOCAL(int, s_max_resource_id);

void ResourceHdr::resetMaxId() {
  *s_max_resource_id = 0;
}

void ResourceHdr::setId(int id) {
  assertx(id >= 1 && id <= 3); // only for STDIN, STDOUT, STDERR
  if (m_id != id) {
    if (m_id == *s_max_resource_id) --*s_max_resource_id;
    m_id = id;
  }
}

ResourceData::ResourceData() {
  // reserving 1, 2, 3 for STDIN, STDOUT, STDERR
  if (*s_max_resource_id < 3) *s_max_resource_id = 3;
  hdr()->setRawId(++*s_max_resource_id);
}

ResourceData::~ResourceData() {
  auto id = getId();
  if (id && id == *s_max_resource_id) {
    --*s_max_resource_id;
  }
  hdr()->setRawId(-1);
}

String ResourceData::o_toString() const {
  return String("Resource id #") + String(getId());
}

Array ResourceData::o_toArray() const {
  return make_vec_array(Variant(OptResource(const_cast<ResourceData*>(this))));
}

const StaticString s_Unknown("Unknown");

const String& ResourceData::o_getClassName() const {
  if (isInvalid()) return s_Unknown;
  return o_getClassNameHook();
}

const String& ResourceData::o_getClassNameHook() const {
  raise_fatal_error("Resource did not provide a name");
}

const String& ResourceData::o_getResourceName() const {
  return o_getClassName();
}

///////////////////////////////////////////////////////////////////////////////
}

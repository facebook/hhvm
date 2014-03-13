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

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/types.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// resources have a separate id space
__thread int ResourceData::os_max_resource_id;

ResourceData::ResourceData() : m_count(0) {
  assert(uintptr_t(this) % sizeof(TypedValue) == 0);
  int& pmax = os_max_resource_id;
  if (pmax < 3) pmax = 3; // reserving 1, 2, 3 for STDIN, STDOUT, STDERR
  o_id = ++pmax;
}

void ResourceData::o_setId(int id) {
  assert(id >= 1 && id <= 3); // only for STDIN, STDOUT, STDERR
  int &pmax = os_max_resource_id;
  if (o_id != id) {
    if (o_id == pmax) --pmax;
    o_id = id;
  }
}

ResourceData::~ResourceData() {
  int &pmax = os_max_resource_id;
  if (o_id && o_id == pmax) {
    --pmax;
  }
  o_id = -1;
}

String ResourceData::o_toString() const {
  return String("Resource id #") + String(o_id);
}

Array ResourceData::o_toArray() const {
  return empty_array;
}

const StaticString s_Unknown("Unknown");

const String& ResourceData::o_getClassName() const {
  if (isInvalid()) return s_Unknown;
  return o_getClassNameHook();
}

const String& ResourceData::o_getClassNameHook() const {
  throw FatalErrorException("Resource did not provide a name");
}

void ResourceData::serializeImpl(VariableSerializer *serializer) const {
  String saveName;
  int saveId;
  serializer->getResourceInfo(saveName, saveId);
  serializer->setResourceInfo(o_getResourceName(), o_id);
  o_toArray().serialize(serializer);
  serializer->setResourceInfo(saveName, saveId);
}

const String& ResourceData::o_getResourceName() const {
  return o_getClassName();
}

void ResourceData::serialize(VariableSerializer* serializer) const {
  if (UNLIKELY(serializer->incNestedLevel((void*)this, true))) {
    serializer->writeOverflow((void*)this, true);
  } else {
    serializeImpl(serializer);
  }
  serializer->decNestedLevel((void*)this);
}

void ResourceData::compileTimeAssertions() {
  static_assert(offsetof(ResourceData, m_count) == FAST_REFCOUNT_OFFSET, "");
}

///////////////////////////////////////////////////////////////////////////////
}

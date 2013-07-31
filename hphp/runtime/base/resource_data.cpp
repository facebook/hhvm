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

// resources have a separate id space
IMPLEMENT_THREAD_LOCAL_NO_CHECK_HOT(int, ResourceData::os_max_resource_id);

ResourceData::ResourceData() : m_count(0), m_cls(SystemLib::s_resourceClass) {
  assert(uintptr_t(this) % sizeof(TypedValue) == 0);
  int& pmax = *os_max_resource_id;
  if (pmax < 3) pmax = 3; // reserving 1, 2, 3 for STDIN, STDOUT, STDERR
  o_id = ++pmax;
}

int ResourceData::GetMaxResourceId() {
  return *(os_max_resource_id.getCheck());
}

void ResourceData::o_setId(int id) {
  assert(id >= 1 && id <= 3); // only for STDIN, STDOUT, STDERR
  int &pmax = *os_max_resource_id;
  if (o_id != id) {
    if (o_id == pmax) --pmax;
    o_id = id;
  }
}

ResourceData::~ResourceData() {
  int &pmax = *os_max_resource_id;
  if (o_id && o_id == pmax) {
    --pmax;
  }
  o_id = -1;
}

Array ResourceData::o_toArray() const {
  return empty_array;
}

void ResourceData::dump() const {
  o_toArray().dump();
}

const StaticString s_Unknown("Unknown");

CStrRef ResourceData::o_getClassName() const {
  if (isInvalid()) return s_Unknown;
  return o_getClassNameHook();
}

CStrRef ResourceData::o_getClassNameHook() const {
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

CStrRef ResourceData::o_getResourceName() const {
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

///////////////////////////////////////////////////////////////////////////////
}

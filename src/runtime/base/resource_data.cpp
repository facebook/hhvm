/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/resource_data.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/variable_serializer.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// resources have a separate id space
static IMPLEMENT_THREAD_LOCAL_NO_CHECK(int, os_max_resource_id);

int ResourceData::GetMaxResourceId() {
  return *(os_max_resource_id.getCheck());
}

ResourceData::ResourceData() : ObjectData(true), m_static (false) {
  int &pmax = *os_max_resource_id;
  if (pmax < 3) pmax = 3; // reserving 1, 2, 3 for STDIN, STDOUT, STDERR
  o_id = ++pmax;
}

void ResourceData::o_setId(int id) {
  ASSERT(id >= 1 && id <= 3); // only for STDIN, STDOUT, STDERR
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

String ResourceData::t___tostring() {
  return String("Resource id #") + String(o_getId());
}

ObjectData* ResourceData::cloneImpl() {
  return NULL;
}

void ResourceData::serialize(VariableSerializer *serializer) const {
  if (serializer->incNestedLevel((void*)this, true)) {
    serializer->writeOverflow((void*)this, true);
  } else {
    String saveName;
    int saveId;
    serializer->getResourceInfo(saveName, saveId);
    serializer->setResourceInfo(o_getResourceName(), o_getResourceId());
    o_toArray().serialize(serializer);
    serializer->setResourceInfo(saveName, saveId);
  }
  serializer->decNestedLevel((void*)this);
}

CStrRef ResourceData::o_getResourceName() const {
  return o_getClassName();
}

Object ResourceData::fiberMarshal(FiberReferenceMap &refMap) const {
  return Object();
}

Object ResourceData::fiberUnmarshal(FiberReferenceMap &refMap) const {
  return Object();
}

///////////////////////////////////////////////////////////////////////////////
}

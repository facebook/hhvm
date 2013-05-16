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

#include "hphp/runtime/base/resource_data.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/variable_serializer.h"

#include "hphp/system/lib/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// resources have a separate id space
static IMPLEMENT_THREAD_LOCAL_NO_CHECK_HOT(int, os_max_resource_id);

int ResourceData::GetMaxResourceId() {
  return *(os_max_resource_id.getCheck());
}

ResourceData::ResourceData()
    : Instance(SystemLib::s_resourceClass, true) {
  assert(!m_cls->callsCustomInstanceInit());
  int &pmax = *os_max_resource_id;
  if (pmax < 3) pmax = 3; // reserving 1, 2, 3 for STDIN, STDOUT, STDERR
  o_id = ++pmax;
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

String ResourceData::t___tostring() {
  return String("Resource id #") + String(o_getId());
}

void ResourceData::serializeImpl(VariableSerializer *serializer) const {
  String saveName;
  int saveId;
  serializer->getResourceInfo(saveName, saveId);
  serializer->setResourceInfo(o_getResourceName(), o_getResourceId());
  o_toArray().serialize(serializer);
  serializer->setResourceInfo(saveName, saveId);
}

CStrRef ResourceData::o_getResourceName() const {
  return o_getClassName();
}

///////////////////////////////////////////////////////////////////////////////
}

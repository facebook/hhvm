/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
    std::string saveName;
    int saveId;
    serializer->getResourceInfo(saveName, saveId);
    serializer->setResourceInfo(o_getResourceName(), o_getResourceId());
    o_toArray().serialize(serializer);
    serializer->setResourceInfo(saveName.c_str(), saveId);
  }
  serializer->decNestedLevel((void*)this);
}

///////////////////////////////////////////////////////////////////////////////
}

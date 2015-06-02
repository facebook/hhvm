/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/smart-ptr.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/resource-data.h"

namespace HPHP {

const char* getClassNameCstr(const ResourceData* p) {
  return p->o_getClassName().c_str();
}

const char* getClassNameCstr(const ObjectData* p) {
  return p->getClassName().c_str();
}

}

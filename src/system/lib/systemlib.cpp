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

#include <system/lib/systemlib.h>
#include <runtime/base/complex_types.h>
#include <system/gen/php/classes/exception.h>
#include <system/gen/php/classes/stdclass.h>
#include <system/gen/php/classes/soapfault.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

ObjectData* SystemLib::AllocStdClassObject() {
  return NEWOBJ(c_stdClass)();
}

ObjectData* SystemLib::AllocExceptionObject(CVarRef message) {
  return (NEWOBJ(c_Exception)())->create(message);
}

ObjectData* SystemLib::AllocDOMExceptionObject(CVarRef message, CVarRef code) {
  return (NEWOBJ(c_DOMException)())->create(message, code);
}

ObjectData* SystemLib::AllocPDOExceptionObject() {
  return (NEWOBJ(c_PDOException)())->create();
}

ObjectData*
SystemLib::AllocSoapFaultObject(CVarRef code,
                                CVarRef message,
                                CVarRef actor /* = null_variant */,
                                CVarRef detail /* = null_variant */,
                                CVarRef name /* = null_variant */,
                                CVarRef header /* = null_variant */) {
  return (NEWOBJ(c_SoapFault)())->create(code, message, actor, detail, name,
                                         header);
}

///////////////////////////////////////////////////////////////////////////////
}


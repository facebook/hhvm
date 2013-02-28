/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <runtime/ext/ext_closure.h>
#include <runtime/base/builtin_functions.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

c_Closure::c_Closure(const ObjectStaticCallbacks *cb) : ExtObjectData(cb) {}
c_Closure::~c_Closure() {}

void c_Closure::t___construct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Closure, Closure::__construct);
}

Variant c_Closure::t___invoke(int _argc, CArrRef _argv) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Closure, Closure::__invoke);
  always_assert(false);
  return null;
}

Variant c_Closure::t___clone() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Closure, Closure::__clone);
  throw_fatal("Trying to clone an uncloneable object of class Closure");
  return null;
}

bool c_Closure::php_sleep(Variant &ret) {
  ret = false;
  return true;
}

///////////////////////////////////////////////////////////////////////////////

c_DummyClosure::c_DummyClosure(const ObjectStaticCallbacks *cb) :
  ExtObjectData(cb) {
}

c_DummyClosure::~c_DummyClosure() {}

void c_DummyClosure::t___construct() {
}

///////////////////////////////////////////////////////////////////////////////
}

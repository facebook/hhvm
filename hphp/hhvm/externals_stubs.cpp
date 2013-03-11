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
#include "runtime/eval/eval.h"
#include "runtime/base/externals.h"

/*
 * HHVM's stubs for various hphpc-style externals.h functions.
 *
 * These exist for legacy reasons---much of the runtime assumed
 * generated versions of some of these functions would exist (as
 * output from hphpc).
 *
 * In the VM, we still fake a few of them while phasing this out.
 */

namespace HPHP {

//////////////////////////////////////////////////////////////////////

Object create_object_only(CStrRef s, ObjectData* root /* = NULL*/) {
  ObjectData *obj = eval_create_object_only_hook(s, root);
  if (UNLIKELY(!obj)) throw_missing_class(s);
  Object r = obj;
  obj->init();
  return r;
}

//////////////////////////////////////////////////////////////////////

}

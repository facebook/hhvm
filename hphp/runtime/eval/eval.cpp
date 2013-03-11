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

#include <runtime/eval/eval.h>
#include <runtime/base/hphp_system.h>

namespace HPHP {

using namespace Eval;

///////////////////////////////////////////////////////////////////////////////

ObjectData *eval_create_object_only_hook(CStrRef s, ObjectData *root) {
  assert_not_implemented(root == nullptr);
  const StringData* className = StringData::GetStaticString(s.get());
  return g_vmContext->createObjectOnly((StringData*)className);
}

///////////////////////////////////////////////////////////////////////////////
}

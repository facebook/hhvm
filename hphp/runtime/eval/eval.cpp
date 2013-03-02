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
#include <runtime/eval/runtime/file_repository.h>

namespace HPHP {

using namespace Eval;

///////////////////////////////////////////////////////////////////////////////

ObjectData *eval_create_object_only_hook(CStrRef s, ObjectData *root) {
  assert_not_implemented(root == nullptr);
  const StringData* className = StringData::GetStaticString(s.get());
  return g_vmContext->createObjectOnly((StringData*)className);
}
bool eval_invoke_file_hook(Variant &res, CStrRef path, bool once,
                           LVariableTable* variables, const char *currentDir) {
  bool initial;
  HPHP::Eval::PhpFile* efile =
    g_vmContext->lookupPhpFile(path.get(), currentDir, &initial);
  HPHP::VM::Unit* u = nullptr;
  if (efile) u = efile->unit();
  if (u == nullptr) {
    return false;
  }
  if (!once || initial) {
    g_vmContext->invokeUnit((TypedValue*)(&res), u);
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}

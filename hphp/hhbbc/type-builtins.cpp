/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/hhbbc/type-builtins.h"

#include "hphp/runtime/base/type-string.h"
#include "hphp/hhbbc/representation.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

const StaticString s_Vector("HH\\Vector");
const StaticString s_Map("HH\\Map");
const StaticString s_Set("HH\\Set");

const StaticString s_add("add");
const StaticString s_addall("addall");
const StaticString s_append("append");
const StaticString s_clear("clear");
const StaticString s_remove("remove");
const StaticString s_removeall("removeall");
const StaticString s_removekey("removekey");
const StaticString s_set("set");
const StaticString s_setall("setall");

//////////////////////////////////////////////////////////////////////

bool is_collection_method_returning_this(borrowed_ptr<php::Class> cls,
                                         borrowed_ptr<php::Func> func) {
  if (!cls) return false;

  if (cls->name->isame(s_Vector.get())) {
    return
      func->name->isame(s_add.get()) ||
      func->name->isame(s_addall.get()) ||
      func->name->isame(s_append.get()) ||
      func->name->isame(s_clear.get()) ||
      func->name->isame(s_removekey.get()) ||
      func->name->isame(s_set.get()) ||
      func->name->isame(s_setall.get());
  }

  if (cls->name->isame(s_Map.get())) {
    return
      func->name->isame(s_add.get()) ||
      func->name->isame(s_addall.get()) ||
      func->name->isame(s_clear.get()) ||
      func->name->isame(s_remove.get()) ||
      func->name->isame(s_set.get()) ||
      func->name->isame(s_setall.get());
  }

  if (cls->name->isame(s_Set.get())) {
    return
      func->name->isame(s_add.get()) ||
      func->name->isame(s_addall.get()) ||
      func->name->isame(s_clear.get()) ||
      func->name->isame(s_remove.get()) ||
      func->name->isame(s_removeall.get());
  }

  return false;
}

Type native_function_return_type(borrowed_ptr<const php::Func> f) {
  if (!f->nativeInfo->returnType) {
    if (f->attrs & AttrReference) {
      return TRef;
    }
    return TInitCell;
  }
  auto t = from_DataType(*f->nativeInfo->returnType);
  // Regardless of ParamCoerceMode, native functions can return null if
  // too many arguments are passed.
  t = union_of(t, TInitNull);
  if (f->attrs & AttrParamCoerceModeFalse) {
    t = union_of(t, TFalse);
  }
  return t;
}

}}

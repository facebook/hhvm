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

#ifndef __EVAL_EVAL_H__
#define __EVAL_EVAL_H__

#include <runtime/base/base_includes.h>
#include <runtime/eval/ast/function_call_expression.h>
#include <runtime/eval/base/function.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant eval(LVariableTable *vars, CObjRef self, CStrRef code_str,
             bool prepend_php = true);

bool eval_invoke_hook(Variant &res, const char *s, CArrRef params, int64 hash);
bool eval_get_class_var_init_hook(Variant &res, const char *s,
                                  const char *var);
bool eval_create_object_hook(Variant &res, const char *s, CArrRef params,
                             bool init, ObjectData *root);
bool eval_try_autoload(const char *s);
bool eval_invoke_static_method_hook(Variant &res, const char *s,
                                    const char* method, CArrRef params,
                                    bool &foundClass);
bool eval_get_static_property_hook(Variant &res, const char *s,
                                   const char* prop);
bool eval_get_static_property_lv_hook(Variant *&res, const char *s,
                                      const char *prop);
bool eval_get_class_constant_hook(Variant &res, const char *s,
                                  const char* constant);
bool eval_constant_hook(Variant &res, CStrRef name);
bool eval_invoke_file_hook(Variant &res, CStrRef path, bool once,
                           LVariableTable* variables, const char *currentDir);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EVAL_EVAL_H__

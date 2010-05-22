/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/ext/ext_variable.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/variable_unserializer.h>
#include <runtime/base/builtin_functions.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

String f_gettype(CVarRef v) {
  switch (v.getType()) {
  case KindOfNull:    return "NULL";
  case KindOfBoolean: return "boolean";
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:   return "integer";
  case KindOfDouble:  return "double";
  case LiteralString:
  case KindOfString:  return "string";
  case KindOfArray:   return "array";
  case KindOfObject:  return "object";
  default:
    ASSERT(false);
    break;
  }
  return "";
}

String f_get_resource_type(CObjRef handle) {
  if (handle.isResource()) {
    return handle->o_getClassName();
  }
  return "";
}

bool f_settype(Variant var, CStrRef type) {
  if      (type == "boolean") var = var.toBoolean();
  else if (type == "bool"   ) var = var.toBoolean();
  else if (type == "integer") var = var.toInt64();
  else if (type == "int"    ) var = var.toInt64();
  else if (type == "float"  ) var = var.toDouble();
  else if (type == "string" ) var = var.toString();
  else if (type == "array"  ) var = var.toArray();
  else if (type == "object" ) var = var.toObject();
  else if (type == "null"   ) var = null;
  else return false;
  return true;
}

bool f_is_object(CVarRef v) {
  return v.is(KindOfObject) && !v.isResource();
}

///////////////////////////////////////////////////////////////////////////////
// input/output

Variant f_print_r(CVarRef expression, bool ret /* = false */) {
  VariableSerializer vs(VariableSerializer::PrintR);
  return vs.serialize(expression, ret);
}

Variant f_var_export(CVarRef expression, bool ret /* = false */) {
  VariableSerializer vs(VariableSerializer::VarExport);
  return vs.serialize(expression, ret);
}

void f_var_dump(CVarRef v) {
  VariableSerializer vs(VariableSerializer::VarDump);
  // manipulate maxCount to match PHP behavior
  if (!v.isObject()) {
    vs.incMaxCount();
  }
  vs.serialize(v, false);
  if (v.isContagious()) {
    v.clearContagious();
  }
}

void f_var_dump(int _argc, CVarRef expression, CArrRef _argv /* = null_array */) {
  f_var_dump(expression);
  for (int i = 0; i < _argv.size(); i++) {
    f_var_dump(_argv[i]);
  }
}

void f_debug_zval_dump(CVarRef variable) {
  VariableSerializer vs(VariableSerializer::DebugDump);
  vs.serialize(variable, false);
}

///////////////////////////////////////////////////////////////////////////////
// variable table

Array f_get_defined_vars() {
  return Array::Create();
}

Array get_defined_vars(LVariableTable *variables) {
  return variables->getDefinedVars();
}

bool f_import_request_variables(CStrRef types, CStrRef prefix /* = "" */) {
  throw NotSupportedException(__func__, "It is bad coding practice to remove scoping of variables just to achieve coding convenience, esp. in a language that encourages global variables. This is possible to implement though, by declaring those global variables beforehand and assign with scoped ones when this function is called.");
}

int f_extract(CArrRef var_array, int extract_type /* = EXTR_OVERWRITE */,
              CStrRef prefix /* = "" */) {
  throw FatalErrorException("bad HPHP code generation");
}
int extract(LVariableTable *variables, CArrRef var_array,
            int extract_type /* = EXTR_OVERWRITE */,
            String prefix /* = "" */) {
  FUNCTION_INJECTION(extract);

  bool reference = extract_type & EXTR_REFS;
  extract_type &= ~EXTR_REFS;

  int count = 0;
  for (ArrayIter iter(var_array); iter; ++iter) {
    String name = iter.first();

    switch (extract_type) {
    case EXTR_SKIP:
      if (variables->exists(name)) continue;
      break;
    case EXTR_IF_EXISTS:
      if (!variables->exists(name)) continue;
      break;
    case EXTR_PREFIX_SAME:
      if (variables->exists(name)) name = prefix + "_" + name;
      break;
    case EXTR_PREFIX_ALL:
      name = prefix + "_" + name;
      break;
    case EXTR_PREFIX_INVALID:
      if (!name.isValidVariableName()) name = prefix + "_" + name;
      break;
    case EXTR_PREFIX_IF_EXISTS:
      if (!variables->exists(name)) continue;
      name = prefix + "_" + name;
      break;
    default:
      break;
    }

    if (reference) {
      variables->get(name) = ref(iter.secondRef());
    } else {
      variables->get(name) = iter.second();
    }
    count++;
  }
  return count;
}

///////////////////////////////////////////////////////////////////////////////
}

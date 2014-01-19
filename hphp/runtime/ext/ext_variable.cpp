/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_variable.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/util/logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_unknown_type("unknown type"),
  s_boolean("boolean"),
  s_bool("bool"),
  s_integer("integer"),
  s_int("int"),
  s_float("float"),
  s_string("string"),
  s_object("object"),
  s_array("array"),
  s_null("null");

String f_gettype(CVarRef v) {
  if (v.getType() == KindOfResource && v.getResourceData()->isInvalid()) {
    return s_unknown_type;
  }
  return getDataTypeString(v.getType());
}

String f_get_resource_type(CResRef handle) {
  return handle->o_getResourceName();
}

int64_t f_intval(CVarRef v, int64_t base /* = 10 */) { return v.toInt64(base);}
double f_doubleval(CVarRef v) { return v.toDouble();}
double f_floatval(CVarRef v) { return v.toDouble();}
String f_strval(CVarRef v) { return v.toString();}
bool f_boolval(CVarRef v) { return v.toBoolean();}

bool f_settype(VRefParam var, const String& type) {
  if      (type == s_boolean) var = var.toBoolean();
  else if (type == s_bool   ) var = var.toBoolean();
  else if (type == s_integer) var = var.toInt64();
  else if (type == s_int    ) var = var.toInt64();
  else if (type == s_float  ) var = var.toDouble();
  else if (type == s_string ) var = var.toString();
  else if (type == s_array  ) var = var.toArray();
  else if (type == s_object ) var = var.toObject();
  else if (type == s_null   ) var = uninit_null();
  else return false;
  return true;
}

bool f_is_bool(CVarRef v) {
  return is_bool(v);
}

bool f_is_int(CVarRef v) {
  return is_int(v);
}

bool f_is_integer(CVarRef v) {
  return is_int(v);
}

bool f_is_long(CVarRef v) {
  return is_int(v);
}

bool f_is_double(CVarRef v) {
  return is_double(v);
}

bool f_is_float(CVarRef v) {
  return is_double(v);
}

bool f_is_numeric(CVarRef v) {
  return v.isNumeric(true);
}

bool f_is_real(CVarRef v) {
  return is_double(v);
}

bool f_is_string(CVarRef v) {
  return is_string(v);
}

bool f_is_scalar(CVarRef v) {
  return v.isScalar();
}

bool f_is_array(CVarRef v) {
  return is_array(v);
}

bool f_is_object(CVarRef v) {
  return is_object(v);
}

bool f_is_resource(CVarRef v) {
  return (v.getType() == KindOfResource && !v.getResourceData()->isInvalid());
}

bool f_is_null(CVarRef v) {
  return is_null(v);
}

///////////////////////////////////////////////////////////////////////////////
// input/output

Variant f_print_r(CVarRef expression, bool ret /* = false */) {
  Variant res;
  try {
    VariableSerializer vs(VariableSerializer::Type::PrintR);
    if (ret) {
      res = vs.serialize(expression, ret);
    } else {
      vs.serialize(expression, ret);
      res = true;
    }
  } catch (StringBufferLimitException &e) {
    raise_notice("print_r() exceeded max bytes limit");
    res = e.m_result;
  }
  return res;
}

Variant f_var_export(CVarRef expression, bool ret /* = false */) {
  Variant res;
  try {
    VariableSerializer vs(VariableSerializer::Type::VarExport);
    if (ret) {
      res = vs.serialize(expression, ret);
    } else {
      vs.serialize(expression, ret);
      res = true;
    }
  } catch (StringBufferLimitException &e) {
    raise_notice("var_export() exceeded max bytes limit");
  }
  return res;
}

void f_var_dump(CVarRef v) {
  VariableSerializer vs(VariableSerializer::Type::VarDump, 0, 2);
  // manipulate maxCount to match PHP behavior
  if (!v.isObject()) {
    vs.incMaxCount();
  }
  vs.serialize(v, false);
}

void f_var_dump(int _argc, CVarRef expression,
                CArrRef _argv /* = null_array */) {
  f_var_dump(expression);
  for (int i = 0; i < _argv.size(); i++) {
    f_var_dump(_argv[i]);
  }
}

void f_debug_zval_dump(CVarRef variable) {
  VariableSerializer vs(VariableSerializer::Type::DebugDump);
  vs.serialize(variable, false);
}

Variant f_unserialize(const String& str, CArrRef class_whitelist /* = empty_array */) {
  return unserialize_from_string(str, class_whitelist);
}

///////////////////////////////////////////////////////////////////////////////
// variable table

Array f_get_defined_vars() {
  VarEnv* v = g_vmContext->getVarEnv();
  if (v) {
    return v->getDefinedVariables();
  } else {
    return Array::Create();
  }
}

bool f_import_request_variables(const String& types, const String& prefix /* = "" */) {
  throw NotSupportedException(__func__,
                              "It is bad coding practice to remove scoping of "
                              "variables just to achieve coding convenience, "
                              "esp. in a language that encourages global "
                              "variables. This is possible to implement "
                              "though, by declaring those global variables "
                              "beforehand and assign with scoped ones when "
                              "this function is called.");
}

int64_t f_extract(CArrRef var_array, int extract_type /* = EXTR_OVERWRITE */,
                  const String& prefix /* = "" */) {
  bool reference = extract_type & EXTR_REFS;
  extract_type &= ~EXTR_REFS;

  VarEnv* v = g_vmContext->getVarEnv();
  if (!v) return 0;
  int count = 0;
  for (ArrayIter iter(var_array); iter; ++iter) {
    String name = iter.first();
    StringData* nameData = name.get();
    switch (extract_type) {
      case EXTR_SKIP:
        if (v->lookup(nameData) != NULL) {
          continue;
        }
        break;
      case EXTR_IF_EXISTS:
        if (v->lookup(nameData) == NULL) {
          continue;
        }
        break;
      case EXTR_PREFIX_SAME:
        if (v->lookup(nameData) != NULL) {
          name = prefix + "_" + name;
        }
        break;
      case EXTR_PREFIX_ALL:
        name = prefix + "_" + name;
        break;
      case EXTR_PREFIX_INVALID:
        if (!is_valid_var_name(nameData->data(), nameData->size())) {
          name = prefix + "_" + name;
        }
        break;
      case EXTR_PREFIX_IF_EXISTS:
        if (v->lookup(nameData) == NULL) {
          continue;
        }
        name = prefix + "_" + name;
        break;
      default:
        break;
    }
    nameData = name.get();
    // skip invalid variable names, as in PHP
    if (!is_valid_var_name(nameData->data(), nameData->size())) {
      continue;
    }
    g_vmContext->setVar(nameData, iter.nvSecond(), reference);
    count++;
  }
  return count;
}

///////////////////////////////////////////////////////////////////////////////
}

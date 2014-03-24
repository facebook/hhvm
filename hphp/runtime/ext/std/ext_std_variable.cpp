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

#include "hphp/runtime/ext/std/ext_std_variable.h"
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

String HHVM_FUNCTION(gettype, const Variant& v) {
  if (v.getType() == KindOfResource && v.getResourceData()->isInvalid()) {
    return s_unknown_type;
  }
  return getDataTypeString(v.getType());
}

String HHVM_FUNCTION(get_resource_type, const Resource& handle) {
  return handle->o_getResourceName();
}

bool HHVM_FUNCTION(boolval, const Variant& v) {
  return v.toBoolean();
}

int64_t HHVM_FUNCTION(intval, const Variant& v, int64_t base /* = 10 */) {
  return v.toInt64(base);
}

double HHVM_FUNCTION(floatval, const Variant& v) {
  return v.toDouble();
}

String HHVM_FUNCTION(strval, const Variant& v) {
  return v.toString();
}

bool HHVM_FUNCTION(settype, VRefParam var, const String& type) {
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

bool HHVM_FUNCTION(is_null, const Variant& v) {
  return is_null(v);
}

bool HHVM_FUNCTION(is_bool, const Variant& v) {
  return is_bool(v);
}

bool HHVM_FUNCTION(is_int, const Variant& v) {
  return is_int(v);
}

bool HHVM_FUNCTION(is_float, const Variant& v) {
  return is_double(v);
}

bool HHVM_FUNCTION(is_numeric, const Variant& v) {
  return v.isNumeric(true);
}

bool HHVM_FUNCTION(is_string, const Variant& v) {
  return is_string(v);
}

bool HHVM_FUNCTION(is_scalar, const Variant& v) {
  return v.isScalar();
}

bool HHVM_FUNCTION(is_array, const Variant& v) {
  return is_array(v);
}

bool HHVM_FUNCTION(is_object, const Variant& v) {
  return is_object(v);
}

bool HHVM_FUNCTION(is_resource, const Variant& v) {
  return (v.getType() == KindOfResource && !v.getResourceData()->isInvalid());
}

///////////////////////////////////////////////////////////////////////////////
// input/output

Variant HHVM_FUNCTION(print_r, const Variant& expression,
                               bool ret /* = false */) {
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

Variant HHVM_FUNCTION(var_export, const Variant& expression,
                                  bool ret /* = false */) {
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

void f_var_dump(const Variant& v) {
  VariableSerializer vs(VariableSerializer::Type::VarDump, 0, 2);
  // manipulate maxCount to match PHP behavior
  if (!v.isObject()) {
    vs.incMaxCount();
  }
  vs.serialize(v, false);
}

void f_var_dump(int _argc, const Variant& expression,
                const Array& _argv /* = null_array */) {
  f_var_dump(expression);
  for (int i = 0; i < _argv.size(); i++) {
    f_var_dump(_argv[i]);
  }
}

void HHVM_FUNCTION(debug_zval_dump, const Variant& variable) {
  VariableSerializer vs(VariableSerializer::Type::DebugDump);
  vs.serialize(variable, false);
}

String HHVM_FUNCTION(serialize, const Variant& value) {
  switch (value.getType()) {
  case KindOfUninit:
  case KindOfNull:
    return "N;";
  case KindOfBoolean:
    return value.getBoolean() ? "b:1;" : "b:0;";
  case KindOfInt64: {
    StringBuffer sb;
    sb.append("i:");
    sb.append(value.getInt64());
    sb.append(';');
    return sb.detach();
  }
  case KindOfStaticString:
  case KindOfString: {
    StringData *str = value.getStringData();
    StringBuffer sb;
    sb.append("s:");
    sb.append(str->size());
    sb.append(":\"");
    sb.append(str->data(), str->size());
    sb.append("\";");
    return sb.detach();
  }
  case KindOfArray: {
    ArrayData *arr = value.getArrayData();
    if (arr->empty()) return "a:0:{}";
    // fall-through
  }
  case KindOfObject:
  case KindOfResource:
  case KindOfDouble: {
    VariableSerializer vs(VariableSerializer::Type::Serialize);
    return vs.serialize(value, true);
  }
  default:
    assert(false);
    break;
  }
  return "";
}

Variant HHVM_FUNCTION(unserialize, const String& str,
                                   const Array& class_whitelist /* =[] */) {
  return unserialize_from_string(str, class_whitelist);
}

///////////////////////////////////////////////////////////////////////////////
// variable table

Array HHVM_FUNCTION(get_defined_vars) {
  VarEnv* v = g_context->getVarEnv();
  if (v) {
    return v->getDefinedVariables();
  } else {
    return Array::Create();
  }
}

int64_t HHVM_FUNCTION(extract, const Array& var_array,
                               int extract_type /* = EXTR_OVERWRITE */,
                               const String& prefix /* = "" */) {
  bool reference = extract_type & EXTR_REFS;
  extract_type &= ~EXTR_REFS;

  VarEnv* v = g_context->getVarEnv();
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
    g_context->setVar(nameData, iter.nvSecond(), reference);
    count++;
  }
  return count;
}

/////////////////////////////////////////////////////////////////////////////

#define EXTR_CONST(v) Native::registerConstant<KindOfInt64> \
                                   (makeStaticString("EXTR_" #v), EXTR_##v);

void StandardExtension::initVariable() {
  EXTR_CONST(IF_EXISTS);
  EXTR_CONST(OVERWRITE);
  EXTR_CONST(PREFIX_ALL);
  EXTR_CONST(PREFIX_IF_EXISTS);
  EXTR_CONST(PREFIX_INVALID);
  EXTR_CONST(PREFIX_SAME);
  EXTR_CONST(REFS);
  EXTR_CONST(SKIP);

  HHVM_FE(is_null);
  HHVM_FE(is_bool);
  HHVM_FE(is_int);
  HHVM_FALIAS(is_integer, is_int);
  HHVM_FALIAS(is_long, is_int);
  HHVM_FE(is_float);
  HHVM_FALIAS(is_double, is_float);
  HHVM_FALIAS(is_real, is_float);
  HHVM_FE(is_numeric);
  HHVM_FE(is_string);
  HHVM_FE(is_scalar);
  HHVM_FE(is_array);
  HHVM_FE(is_object);
  HHVM_FE(is_resource);
  HHVM_FE(boolval);
  HHVM_FE(intval);
  HHVM_FE(floatval);
  HHVM_FALIAS(doubleval, floatval);
  HHVM_FE(strval);
  HHVM_FE(gettype);
  HHVM_FE(get_resource_type);
  HHVM_FE(settype);
  HHVM_FE(print_r);
  HHVM_FE(var_export);
  HHVM_FE(debug_zval_dump);
  HHVM_FE(serialize);
  HHVM_FE(unserialize);
  HHVM_FE(get_defined_vars);
  HHVM_FE(extract);

  loadSystemlib("std_variable");
}

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

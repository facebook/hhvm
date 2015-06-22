/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <folly/Likely.h>

#include "hphp/util/logger.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/ext/xdebug/ext_xdebug.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/server/http-protocol.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_unknown_type("unknown type"),
  s_boolean("boolean"),
  s_bool("bool"),
  s_integer("integer"),
  s_int("int"),
  s_float("float"),
  s_double("double"),
  s_string("string"),
  s_object("object"),
  s_array("array"),
  s_null("null");

String HHVM_FUNCTION(gettype, const Variant& v) {
  if (v.getType() == KindOfResource && v.toCResRef().isInvalid()) {
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
  else if (type == s_double ) var = var.toDouble();
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
  return (v.getType() == KindOfResource && !v.toCResRef().isInvalid());
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

static ALWAYS_INLINE void do_var_dump(VariableSerializer vs,
                                      const Variant& expression) {
  // manipulate maxCount to match PHP behavior
  if (!expression.isObject()) {
    vs.incMaxCount();
  }
  vs.serialize(expression, false);
}

void HHVM_FUNCTION(var_dump, const Variant& expression,
                             const Array& _argv /*=null_array */) {
  if (UNLIKELY(XDEBUG_GLOBAL(OverloadVarDump) &&
               XDEBUG_GLOBAL(DefaultEnable))) {
    HHVM_FN(xdebug_var_dump)(expression, _argv);
    return;
  }

  VariableSerializer vs(VariableSerializer::Type::VarDump, 0, 2);
  do_var_dump(vs, expression);

  auto sz = _argv.size();
  for (int i = 0; i < sz; i++) {
    do_var_dump(vs, _argv[i]);
  }
}

void HHVM_FUNCTION(debug_zval_dump, const Variant& variable) {
  VariableSerializer vs(VariableSerializer::Type::DebugDump);
  vs.serialize(variable, false);
}

const StaticString
  s_Null("N;"),
  s_True("b:1;"),
  s_False("b:0;"),
  s_Res("i:0;"),
  s_EmptyArray("a:0:{}");

String HHVM_FUNCTION(serialize, const Variant& value) {
  switch (value.getType()) {
    case KindOfUninit:
    case KindOfNull:
      return s_Null;
    case KindOfBoolean:
      return value.getBoolean() ? s_True : s_False;
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
    case KindOfResource:
      return s_Res;
    case KindOfArray: {
      ArrayData *arr = value.getArrayData();
      if (arr->empty()) return s_EmptyArray;
      // fall-through
    }
    case KindOfDouble:
    case KindOfObject: {
      VariableSerializer vs(VariableSerializer::Type::Serialize);
      return vs.serialize(value, true);
    }
    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

Variant HHVM_FUNCTION(unserialize, const String& str,
                                   const Array& class_whitelist /* =[] */) {
  return unserialize_from_string(str, class_whitelist);
}

///////////////////////////////////////////////////////////////////////////////
// variable table

ALWAYS_INLINE
static Array get_defined_vars() {
  VarEnv* v = g_context->getOrCreateVarEnv();
  return v ? v->getDefinedVariables() : empty_array();
}

Array HHVM_FUNCTION(get_defined_vars) {
  raise_disallowed_dynamic_call("get_defined_vars should not be "
    "called dynamically");
  return get_defined_vars();
}

// accessible as __SystemLib\\get_defined_vars
Array HHVM_FUNCTION(SystemLib_get_defined_vars) {
  return get_defined_vars();
}

const StaticString
  s_GLOBALS("GLOBALS"),
  s_this("this");

static const Func* arGetContextFunc(const ActRec* ar) {
  if (ar == nullptr) {
    return nullptr;
  }
  if (ar->m_func->isPseudoMain() || ar->m_func->isBuiltin()) {
    // Pseudomains inherit the context of their caller
    auto const context = g_context.getNoCheck();
    ar = context->getPrevVMState(ar);
    while (ar != nullptr &&
             (ar->m_func->isPseudoMain() || ar->m_func->isBuiltin())) {
      ar = context->getPrevVMState(ar);
    }
    if (ar == nullptr) {
      return nullptr;
    }
  }
  return ar->m_func;
}

static bool modify_extract_name(VarEnv* v,
                                String& name,
                                int64_t extract_type,
                                const String& prefix) {
  switch (extract_type) {
  case EXTR_SKIP:
    if (v->lookup(name.get()) != nullptr) {
      return false;
    }
    break;
  case EXTR_IF_EXISTS:
    if (v->lookup(name.get()) == nullptr) {
      return false;
    } else {
      goto namechecks;
    }
    break;
  case EXTR_PREFIX_SAME:
    if (v->lookup(name.get()) != nullptr) {
      name = prefix + "_" + name;
    } else {
      goto namechecks;
    }
    break;
  case EXTR_PREFIX_ALL:
    name = prefix + "_" + name;
    break;
  case EXTR_PREFIX_INVALID:
    if (!is_valid_var_name(name.get()->data(), name.size())) {
      name = prefix + "_" + name;
    } else {
      goto namechecks;
    }
    break;
  case EXTR_PREFIX_IF_EXISTS:
    if (v->lookup(name.get()) == nullptr) {
      return false;
    }
    name = prefix + "_" + name;
    break;
  case EXTR_OVERWRITE:
    namechecks:
    if (name == s_GLOBALS) {
      return false;
    }
    if (name == s_this) {
      // Only disallow $this when inside a non-static method, or a static method
      // that has defined $this (matches Zend)
      CallerFrame cf;
      const Func* func = arGetContextFunc(cf());

      if (func && func->isMethod() && v->lookup(s_this.get()) != nullptr) {
        return false;
      }
    }
  default:
    break;
  }

  // skip invalid variable names, as in PHP
  return is_valid_var_name(name.get()->data(), name.size());
}

ALWAYS_INLINE static
int64_t extract_impl(VRefParam vref_array,
                     int extract_type /* = EXTR_OVERWRITE */,
                     const String& prefix /* = "" */) {
  bool reference = extract_type & EXTR_REFS;
  extract_type &= ~EXTR_REFS;

  if (!vref_array.wrapped().isArray()) {
    raise_warning("extract() expects parameter 1 to be array");
    return 0;
  }

  VMRegAnchor _;
  auto const varEnv = g_context->getOrCreateVarEnv();
  if (!varEnv) return 0;

  if (UNLIKELY(reference)) {
    auto& arr = vref_array.wrapped().toArrRef();
    int count = 0;
    for (ArrayIter iter(arr); iter; ++iter) {
      String name = iter.first();
      if (!modify_extract_name(varEnv, name, extract_type, prefix)) continue;
      g_context->bindVar(name.get(), arr.lvalAt(name).asTypedValue());
      ++count;
    }
    return count;
  }

  auto const var_array = vref_array.wrapped().toArray();
  int count = 0;
  for (ArrayIter iter(var_array); iter; ++iter) {
    String name = iter.first();
    if (!modify_extract_name(varEnv, name, extract_type, prefix)) continue;
    g_context->setVar(name.get(), iter.secondRef().asTypedValue());
    ++count;
  }
  return count;
}

int64_t HHVM_FUNCTION(extract, VRefParam vref_array,
                      int64_t extract_type /* = EXTR_OVERWRITE */,
                      const String& prefix /* = "" */) {
  raise_disallowed_dynamic_call("extract should not be called dynamically");
  return extract_impl(vref_array, extract_type, prefix);
}

int64_t HHVM_FUNCTION(SystemLib_extract,
                      VRefParam vref_array,
                      int64_t extract_type = EXTR_OVERWRITE,
                      const String& prefix = "") {
  return extract_impl(vref_array, extract_type, prefix);
}

static void parse_str_impl(const String& str, VRefParam arr) {
  Array result = Array::Create();
  HttpProtocol::DecodeParameters(result, str.data(), str.size());
  if (!arr.isReferenced()) {
    HHVM_FN(SystemLib_extract)(result);
    return;
  }
  arr = result;
}

void HHVM_FUNCTION(parse_str,
                   const String& str,
                   VRefParam arr /* = null */) {
  raise_disallowed_dynamic_call("parse_str should not be called dynamically");
  parse_str_impl(str, arr);
}

void HHVM_FUNCTION(SystemLib_parse_str,
                   const String& str,
                   VRefParam arr /* = null */) {
  parse_str_impl(str, arr);
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
  HHVM_FE(var_dump);
  HHVM_FE(serialize);
  HHVM_FE(unserialize);
  HHVM_FE(get_defined_vars);
  HHVM_FALIAS(__SystemLib\\get_defined_vars, SystemLib_get_defined_vars);
  HHVM_FE(extract);
  HHVM_FE(parse_str);
  HHVM_FALIAS(__SystemLib\\extract, SystemLib_extract);
  HHVM_FALIAS(__SystemLib\\parse_str, SystemLib_parse_str);

  loadSystemlib("std_variable");
}

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

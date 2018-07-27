/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/util/hphp-config.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/zend-functions.h"
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
  s_NULL("NULL"),
  s_null("null");

String HHVM_FUNCTION(gettype, const Variant& v) {
  if (v.getType() == KindOfResource && v.toCResRef().isInvalid()) {
    return s_unknown_type;
  }
  /* Although getDataTypeString also handles the null type, it returns "null"
   * (lower case). Unfortunately, PHP returns "NULL" (upper case) for
   * gettype(). So we make an exception here. */
  if (v.isNull()) {
    return s_NULL;
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
  Variant val;
  if      (type == s_boolean) val = var.toBoolean();
  else if (type == s_bool   ) val = var.toBoolean();
  else if (type == s_integer) val = var.toInt64();
  else if (type == s_int    ) val = var.toInt64();
  else if (type == s_float  ) val = var.toDouble();
  else if (type == s_double ) val = var.toDouble();
  else if (type == s_string ) val = var.toString();
  else if (type == s_array  ) val = var.toArray();
  else if (type == s_object ) val = var.toObject();
  else if (type == s_null   ) val = uninit_null();
  else return false;
  var.assignIfRef(val);
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
  if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsArrayNotices)) {
    if (v.isPHPArray()) {
      return true;
    } else if (v.isVecArray()) {
      raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_VEC_IS_ARR);
    } else if (v.isDict()) {
      raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_DICT_IS_ARR);
    } else if (v.isKeyset()) {
      raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_KEYSET_IS_ARR);
    }
    return false;
  }
  return is_array(v);
}

bool HHVM_FUNCTION(HH_is_vec, const Variant& v) {
  if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsVecDictNotices)) {
    if (v.isPHPArray()) {
      auto const& arr = v.toCArrRef();
      if (arr.isVArray()) {
        raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_VARR_IS_VEC);
      }
      return false;
    }
  }
  return is_vec(v);
}

bool HHVM_FUNCTION(HH_is_dict, const Variant& v) {
  if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsVecDictNotices)) {
    if (v.isPHPArray()) {
      auto const& arr = v.toCArrRef();
      if (arr.isDArray()) {
        raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_DARR_IS_DICT);
      }
      return false;
    }
  }
  return is_dict(v);
}

bool HHVM_FUNCTION(HH_is_keyset, const Variant& v) {
  return is_keyset(v);
}

bool HHVM_FUNCTION(HH_is_varray, const Variant& val) {
  if (RuntimeOption::EvalHackArrDVArrs) return is_vec(val);
  if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsVecDictNotices)) {
    if (val.isVecArray()) {
      raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_VEC_IS_VARR);
      return false;
    }
  }
  return val.isPHPArray() && val.toCArrRef().isVArray();
}

bool HHVM_FUNCTION(HH_is_darray, const Variant& val) {
  if (RuntimeOption::EvalHackArrDVArrs) return is_dict(val);
  if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsVecDictNotices)) {
    if (val.isDict()) {
      raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_DICT_IS_DARR);
      return false;
    }
  }
  return val.isPHPArray() && val.toCArrRef().isDArray();
}

bool HHVM_FUNCTION(HH_is_any_array, const Variant& val) {
  return val.isArray();
}

bool HHVM_FUNCTION(is_object, const Variant& v) {
  return is_object(v);
}

bool HHVM_FUNCTION(is_resource, const Variant& v) {
  return (v.getType() == KindOfResource && !v.toCResRef().isInvalid());
}

///////////////////////////////////////////////////////////////////////////////

Array HHVM_FUNCTION(HH_object_prop_array, const Object& obj) {
  return obj.toArray();
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
  } catch (StringBufferLimitException& e) {
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
  } catch (StringBufferLimitException& e) {
    raise_notice("var_export() exceeded max bytes limit");
  }
  return res;
}

static ALWAYS_INLINE void do_var_dump(VariableSerializer& vs,
                                      const Variant& expression) {
  // manipulate maxCount to match PHP behavior
  if (!expression.isObject()) {
    vs.incMaxCount();
  }
  vs.serialize(expression, false);
}

void HHVM_FUNCTION(var_dump, const Variant& expression,
                             const Array& _argv /*=null_array */) {

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

namespace {

const StaticString
  s_Null("N;"),
  s_True("b:1;"),
  s_False("b:0;"),
  s_Res("i:0;"),
  s_EmptyArray("a:0:{}"),
  s_EmptyVArray("y:0:{}"),
  s_EmptyDArray("Y:0:{}"),
  s_EmptyVecArray("v:0:{}"),
  s_EmptyDictArray("D:0:{}"),
  s_EmptyKeysetArray("k:0:{}");

ALWAYS_INLINE String serialize_impl(const Variant& value,
                                    bool keepDVArrays,
                                    bool forcePHPArrays,
                                    bool hackWarn,
                                    bool phpWarn) {
  auto const empty_hack = [&](const ArrayData* arr, const StaticString& empty) {
    if (UNLIKELY(RuntimeOption::EvalHackArrCompatSerializeNotices &&
                 hackWarn)) {
      raise_hack_arr_compat_serialize_notice(arr);
    }
    return forcePHPArrays ? s_EmptyArray : empty;
  };

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
    case KindOfPersistentString:
    case KindOfString: {
      StringData *str = value.getStringData();
      auto const size = str->size();
      if (size >= RuntimeOption::MaxSerializedStringSize) {
        throw Exception("Size of serialized string (%d) exceeds max", size);
      }
      StringBuffer sb;
      sb.append("s:");
      sb.append(size);
      sb.append(":\"");
      sb.append(str->data(), size);
      sb.append("\";");
      return sb.detach();
    }
    case KindOfResource:
      return s_Res;

    case KindOfPersistentVec:
    case KindOfVec: {
      ArrayData* arr = value.getArrayData();
      assertx(arr->isVecArray());
      if (arr->empty()) {
        return UNLIKELY(arr->isLegacyArray())
          ? s_EmptyArray
          : empty_hack(arr, s_EmptyVecArray);
      }
      break;
    }

    case KindOfPersistentDict:
    case KindOfDict: {
      ArrayData* arr = value.getArrayData();
      assertx(arr->isDict());
      if (arr->empty()) {
        return UNLIKELY(arr->isLegacyArray())
          ? s_EmptyArray
          : empty_hack(arr, s_EmptyDictArray);
      }
      break;
    }

    case KindOfPersistentKeyset:
    case KindOfKeyset: {
      ArrayData* arr = value.getArrayData();
      assertx(arr->isKeyset());
      if (arr->empty()) return empty_hack(arr, s_EmptyKeysetArray);
      break;
    }

    case KindOfPersistentArray:
    case KindOfArray: {
      ArrayData *arr = value.getArrayData();
      assertx(arr->isPHPArray());
      assertx(!RuntimeOption::EvalHackArrDVArrs || arr->isNotDVArray());
      if (arr->empty()) {
        if (UNLIKELY(RuntimeOption::EvalHackArrCompatSerializeNotices &&
                     phpWarn)) {
          raise_hack_arr_compat_serialize_notice(arr);
        }
        if (keepDVArrays && !forcePHPArrays) {
          if (arr->isVArray()) return s_EmptyVArray;
          if (arr->isDArray()) return s_EmptyDArray;
        }
        return s_EmptyArray;
      }
      break;
    }
    case KindOfDouble:
    case KindOfObject:
    // TODO (T29639296)
    case KindOfFunc:
    case KindOfClass:
      break;

    case KindOfRef:
      not_reached();
  }

  VariableSerializer vs(VariableSerializer::Type::Serialize);
  if (keepDVArrays)   vs.keepDVArrays();
  if (forcePHPArrays) vs.setForcePHPArrays();
  if (hackWarn)       vs.setHackWarn();
  if (phpWarn)        vs.setPHPWarn();
  // Keep the count so recursive calls to serialize() embed references properly.
  return vs.serialize(value, true, true);
}

}

String HHVM_FUNCTION(serialize, const Variant& value) {
  return serialize_impl(value, false, false, false, false);
}

const StaticString
  s_forcePHPArrays("forcePHPArrays"),
  s_warnOnHackArrays("warnOnHackArrays"),
  s_warnOnPHPArrays("warnOnPHPArrays");

String HHVM_FUNCTION(HH_serialize_with_options,
                     const Variant& value, const Array& options) {
  return serialize_impl(
    value,
    false,
    options.exists(s_forcePHPArrays) &&
      options[s_forcePHPArrays].toBoolean(),
    options.exists(s_warnOnHackArrays) &&
      options[s_warnOnHackArrays].toBoolean(),
    options.exists(s_warnOnPHPArrays) &&
      options[s_warnOnPHPArrays].toBoolean()
  );
}

String HHVM_FUNCTION(hhvm_intrinsics_serialize_keep_dvarrays,
                     const Variant& value) {
  return serialize_impl(value, true, false, false, false);
}

Variant HHVM_FUNCTION(unserialize, const String& str,
                                   const Array& options /* =[] */) {
  return unserialize_from_string(
    str,
    VariableUnserializer::Type::Serialize,
    options
  );
}

Variant HHVM_FUNCTION(hhvm_intrinsics_unserialize_keep_dvarrays,
                      const String& str) {
  return unserialize_from_string(
    str,
    // This is fine because the only difference between Serialize and Internal
    // right now is d/varray serialization.
    VariableUnserializer::Type::Internal,
    null_array
  );
}

///////////////////////////////////////////////////////////////////////////////
// variable table

Array HHVM_FUNCTION(get_defined_vars) {
  VarEnv* v = g_context->getOrCreateVarEnv();
  return v ? v->getDefinedVariables() : empty_array();
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
      auto const func = arGetContextFunc(GetCallerFrame());

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

int64_t HHVM_FUNCTION(extract,
                      VRefParam vref_array,
                      int64_t extract_type = EXTR_OVERWRITE,
                      const String& prefix = "") {
  auto arrByRef = false;
  auto arr_tv = vref_array.wrapped().asTypedValue();
  if (isRefType(arr_tv->m_type)) {
    arr_tv = arr_tv->m_data.pref->tv();
    arrByRef = true;
  }
  if (!isArrayLikeType(arr_tv->m_type)) {
    raise_warning("extract() expects parameter 1 to be array");
    return 0;
  }

  bool reference = extract_type & EXTR_REFS;
  extract_type &= ~EXTR_REFS;

  VMRegAnchor _;
  auto const varEnv = g_context->getOrCreateVarEnv();
  if (!varEnv) return 0;

  auto& carr = tvAsCVarRef(arr_tv).asCArrRef();
  if (UNLIKELY(reference)) {
    auto extr_refs = [&](Array& arr) {
      if (arr.size() > 0) {
        // force arr to escalate (if necessary) by getting an lvalue to the
        // first element.
        ArrayData* ad = arr.get();
        auto const& first_key = ad->getKey(ad->iter_begin());
        arr.lvalAt(first_key);
      }
      int count = 0;
      for (ArrayIter iter(arr); iter; ++iter) {
        auto name = iter.first().toString();
        if (!modify_extract_name(varEnv, name, extract_type, prefix)) continue;
        // The as_lval() is safe because we escalated the array.  We can't use
        // arr.lvalAt(name), because arr may have been modified as a side
        // effect of an earlier iteration.
        auto const rval = iter.secondRval();
        g_context->bindVar(name.get(), rval.as_lval());
        ++count;
      }
      return count;
    };

    if (arrByRef) {
      return extr_refs(tvAsVariant(vref_array.getRefData()->tv()).asArrRef());
    }
    Array tmp = carr;
    return extr_refs(tmp);
  }

  int count = 0;
  for (ArrayIter iter(carr); iter; ++iter) {
    auto name = iter.first().toString();
    if (!modify_extract_name(varEnv, name, extract_type, prefix)) continue;
    g_context->setVar(name.get(), iter.secondRval());
    ++count;
  }
  return count;
}

void HHVM_FUNCTION(parse_str,
                   const String& str,
                   VRefParam arr /* = null */) {
  SuppressHackArrCompatNotices suppress;
  Array result = Array::Create();
  HttpProtocol::DecodeParameters(result, str.data(), str.size());
  if (!arr.isReferenced()) {
    HHVM_FN(extract)(result);
    return;
  }
  arr.assignIfRef(result);
}

/////////////////////////////////////////////////////////////////////////////

void StandardExtension::initVariable() {
  HHVM_RC_INT_SAME(EXTR_IF_EXISTS);
  HHVM_RC_INT_SAME(EXTR_OVERWRITE);
  HHVM_RC_INT_SAME(EXTR_PREFIX_ALL);
  HHVM_RC_INT_SAME(EXTR_PREFIX_IF_EXISTS);
  HHVM_RC_INT_SAME(EXTR_PREFIX_INVALID);
  HHVM_RC_INT_SAME(EXTR_PREFIX_SAME);
  HHVM_RC_INT_SAME(EXTR_REFS);
  HHVM_RC_INT_SAME(EXTR_SKIP);

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
  HHVM_FALIAS(HH\\is_vec, HH_is_vec);
  HHVM_FALIAS(HH\\is_dict, HH_is_dict);
  HHVM_FALIAS(HH\\is_keyset, HH_is_keyset);
  HHVM_FALIAS(HH\\is_varray, HH_is_varray);
  HHVM_FALIAS(HH\\is_darray, HH_is_darray);
  HHVM_FALIAS(HH\\is_any_array, HH_is_any_array);
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
  HHVM_FE(extract);
  HHVM_FE(parse_str);
  HHVM_FALIAS(HH\\object_prop_array, HH_object_prop_array);
  HHVM_FALIAS(HH\\serialize_with_options, HH_serialize_with_options);

  if (RuntimeOption::EnableIntrinsicsExtension) {
    HHVM_FALIAS(__hhvm_intrinsics\\serialize_keep_dvarrays,
                hhvm_intrinsics_serialize_keep_dvarrays);
    HHVM_FALIAS(__hhvm_intrinsics\\deserialize_keep_dvarrays,
                hhvm_intrinsics_unserialize_keep_dvarrays);
  }

  loadSystemlib("std_variable");
}

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

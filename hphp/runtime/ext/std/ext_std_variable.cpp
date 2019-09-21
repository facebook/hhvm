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
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/globals-array.h"
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
  s_null("null"),
  s_meth_caller_cls("__SystemLib\\MethCallerHelper");

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
  if (RuntimeOption::EvalLogArrayProvenance &&
      (v.isVecArray() || v.isDict())) {
    raise_array_serialization_notice("gettype", v.getArrayData());
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

bool HHVM_FUNCTION(is_null, const Variant& v) {
  return is_null(v.asTypedValue());
}

bool HHVM_FUNCTION(is_bool, const Variant& v) {
  return is_bool(v.asTypedValue());
}

bool HHVM_FUNCTION(is_int, const Variant& v) {
  return is_int(v.asTypedValue());
}

bool HHVM_FUNCTION(is_float, const Variant& v) {
  return is_double(v.asTypedValue());
}

bool HHVM_FUNCTION(is_numeric, const Variant& v) {
  return v.isNumeric(true);
}

bool HHVM_FUNCTION(is_string, const Variant& v) {
  return is_string(v.asTypedValue());
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
  return is_array(v.asTypedValue());
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
  auto const ret =  is_vec(v.asTypedValue());
  if (ret && UNLIKELY(RuntimeOption::EvalLogArrayProvenance)) {
    raise_array_serialization_notice("is_vec", v.asCArrRef().get());
  }
  return ret;
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
  auto const ret = is_dict(v.asTypedValue());
  if (ret && UNLIKELY(RuntimeOption::EvalLogArrayProvenance)) {
    raise_array_serialization_notice("is_dict", v.asCArrRef().get());
  }
  return ret;
}

bool HHVM_FUNCTION(HH_is_keyset, const Variant& v) {
  return is_keyset(v.asTypedValue());
}

bool HHVM_FUNCTION(HH_is_varray, const Variant& val) {
  if (tvIsClsMeth(val.asTypedValue())) {
    return !RuntimeOption::EvalHackArrDVArrs;
  }
  auto const cell = val.asTypedValue();
  if (RuntimeOption::EvalHackArrDVArrs) return is_vec(cell);
  if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsVecDictNotices)) {
    if (val.isVecArray()) {
      raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_VEC_IS_VARR);
      return false;
    }
  }
  return tvIsArray(cell) && cell->m_data.parr->isVArray();
}

bool HHVM_FUNCTION(HH_is_darray, const Variant& val) {
  auto const cell = val.asTypedValue();
  if (RuntimeOption::EvalHackArrDVArrs) return is_dict(cell);
  if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsVecDictNotices)) {
    if (val.isDict()) {
      raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_DICT_IS_DARR);
      return false;
    }
  }
  return tvIsArray(cell) && cell->m_data.parr->isDArray();
}

bool HHVM_FUNCTION(HH_is_any_array, const Variant& val) {
  return tvIsArrayLike(val.asTypedValue()) || tvIsClsMeth(val.asTypedValue());
}

bool HHVM_FUNCTION(HH_is_list_like, const Variant& val) {
  if (val.isClsMeth()) return true;
  if (!val.isArray()) return false;
  auto const& arr = val.toCArrRef();
  return arr->isVectorData();
}

bool HHVM_FUNCTION(is_object, const Variant& v) {
  return is_object(v.asTypedValue());
}

bool HHVM_FUNCTION(is_resource, const Variant& v) {
  return (v.getType() == KindOfResource && !v.toCResRef().isInvalid());
}

bool HHVM_FUNCTION(HH_is_meth_caller, TypedValue v) {
  if (tvIsFunc(v)) {
    return val(v).pfunc->isMethCaller();
  } else if (tvIsObject(v)) {
    auto const mcCls = Unit::lookupClass(s_meth_caller_cls.get());
    assertx(mcCls);
    return mcCls == val(v).pobj->getVMClass();
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

Array HHVM_FUNCTION(HH_object_prop_array, const Object& obj) {
  return obj.toArray().toDArray();
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

/*
 * container intrinsic for HH\traversable, including
 * 1. array:
 *   array, vec, dict, keyset
 * 2. collection: Vector, Map, Set
 * not including Objects that implement \HH\Iterable or \Iterator.
 */
Variant HHVM_FUNCTION(HH_first, const Variant& v) {
  // 1. array, vec, dict, keyset
  if (v.isArray() || v.isClsMeth()) {
    auto arr = v.toArray();
    if (arr->empty()) {
      return init_null();
    }
    return arr->getValue(arr->iter_begin());
  }

  if (v.isObject()) {
    auto obj = v.asCObjRef();
    // 2. collection
    if (obj->isCollection()) {
      // Pair
      if (obj->collectionType() == CollectionType::Pair) {
        auto const pair = static_cast<c_Pair*>(obj.get());
        return Variant::wrap(*pair->at(0));
      }

      // Vector, Map, Set, and Imm variants
      auto arr = collections::asArray(obj.get());
      if (arr->empty()) {
        return init_null();
      }
      return arr->getValue(arr->iter_begin());
    }
  }
  SystemLib::throwInvalidArgumentExceptionObject(
    "Argument 1 passed to HH\\Lib\\_Private\\Native\\first() "
     "must be a Container");
}

Variant HHVM_FUNCTION(HH_last, const Variant& v) {
  // 1. array, vec, dict, keyset
  if (v.isArray() || v.isClsMeth()) {
    auto arr = v.toArray();
    if (arr->empty()) {
      return init_null();
    }
    return arr->getValue(arr->iter_last());
  }

  if (v.isObject()) {
    auto obj = v.asCObjRef();
    // 2. collection
    if (obj->isCollection()) {
      // Pair
      if (obj->collectionType() == CollectionType::Pair) {
        auto const pair = static_cast<c_Pair*>(obj.get());
        return Variant::wrap(*pair->at(1));
      }

      // Vector, Map, Set, and Imm variants
      auto arr = collections::asArray(obj.get());
      if (arr->empty()) {
        return init_null();
      }
      return arr->getValue(arr->iter_last());
    }
  }
  SystemLib::throwInvalidArgumentExceptionObject(
    "Argument 1 passed to HH\\Lib\\_Private\\Native\\last() "
    "must be a Container");
}

Variant HHVM_FUNCTION(HH_first_key, const Variant& v) {
  // 1. array, vec, dict, keyset
  if (v.isArray() || v.isClsMeth()) {
    auto arr = v.toArray();
    if (arr->empty()) {
      return init_null();
    }
    return arr->getKey(arr->iter_begin());
  }

  if (v.isObject()) {
    auto obj = v.asCObjRef();
    // 2. collection
    if (obj->isCollection()) {
      // Pair
      if (obj->collectionType() == CollectionType::Pair) {
        return Variant::wrap(make_tv<KindOfInt64>(0));
      }

      // Vector, Map, Set, and Imm variants
      auto arr = collections::asArray(obj.get());
      if (arr->empty()) {
        return init_null();
      }
      return arr->getKey(arr->iter_begin());
    }
  }
  SystemLib::throwInvalidArgumentExceptionObject(
    "Argument 1 passed to HH\\Lib\\_Private\\Native\\first_key() "
    "must be a Container");
}

Variant HHVM_FUNCTION(HH_last_key, const Variant& v) {
  // 1. array, vec, dict, keyset
  if (v.isArray() || v.isClsMeth()) {
    auto arr = v.toArray();
    if (arr->empty()) {
      return init_null();
    }
    return arr->getKey(arr->iter_last());
  }

  if (v.isObject()) {
    auto obj = v.asCObjRef();
    // 2. collection
    if (obj->isCollection()) {
      // Pair
      if (obj->collectionType() == CollectionType::Pair) {
        return Variant::wrap(make_tv<KindOfInt64>(1));
      }

      // Vector, Map, Set, and Imm variants
      auto arr = collections::asArray(obj.get());
      if (arr->empty()) {
        return init_null();
      }
      return arr->getKey(arr->iter_last());
    }
  }
  SystemLib::throwInvalidArgumentExceptionObject(
    "Argument 1 passed to HH\\Lib\\_Private\\Native\\last_key() "
    "must be a Container");
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
                                    bool phpWarn,
                                    bool ignoreLateInit) {
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
    case KindOfClass:
    case KindOfFunc:
    case KindOfPersistentString:
    case KindOfString: {
      auto const str =
        isStringType(value.getType()) ? value.getStringData() :
        isFuncType(value.getType())   ? funcToStringHelper(value.toFuncVal()) :
                                        classToStringHelper(value.toClassVal());
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
    case KindOfClsMeth:
    case KindOfRecord:
      break;

    case KindOfRef:
      not_reached();
  }

  VariableSerializer vs(VariableSerializer::Type::Serialize);
  if (keepDVArrays)   vs.keepDVArrays();
  if (forcePHPArrays) vs.setForcePHPArrays();
  if (hackWarn)       vs.setHackWarn();
  if (phpWarn)        vs.setPHPWarn();
  if (ignoreLateInit) vs.setIgnoreLateInit();
  // Keep the count so recursive calls to serialize() embed references properly.
  return vs.serialize(value, true, true);
}

}

String HHVM_FUNCTION(serialize, const Variant& value) {
  return serialize_impl(value, false, false, false, false, false);
}

const StaticString
  s_forcePHPArrays("forcePHPArrays"),
  s_warnOnHackArrays("warnOnHackArrays"),
  s_warnOnPHPArrays("warnOnPHPArrays"),
  s_ignoreLateInit("ignoreLateInit");

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
      options[s_warnOnPHPArrays].toBoolean(),
    options.exists(s_ignoreLateInit) &&
      options[s_ignoreLateInit].toBoolean()
  );
}

String HHVM_FUNCTION(hhvm_intrinsics_serialize_keep_dvarrays,
                     const Variant& value) {
  return serialize_impl(value, true, false, false, false, false);
}

Variant HHVM_FUNCTION(unserialize, const String& str,
                                   const Array& options) {
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

void HHVM_FUNCTION(parse_str,
                   const String& str,
                   Array& arr) {
  arr = Array::Create();
  HttpProtocol::DecodeParameters(arr, str.data(), str.size());
}

/////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(hhvm_intrinsics_create_class_pointer, StringArg name) {
  auto const cls = Unit::loadClass(name.get());
  return cls ? Variant{cls} : init_null();
}

/////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(HH_is_late_init_prop_init,
                   const Object& obj,
                   const String& name) {
  auto const ctx = fromCaller(
    [] (const ActRec* fp, Offset) { return fp->func()->cls(); }
  );
  auto const val = obj->getPropIgnoreLateInit(ctx, name.get());
  if (!val) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat(
       "Unknown or inaccessible property '{}' on object of class {}",
       name.get(),
       obj->getVMClass()->name()
      )
    );
  }
  return type(val) != KindOfUninit;
}

bool HHVM_FUNCTION(HH_is_late_init_sprop_init,
                   const String& clsName,
                   const String& name) {
  auto const cls = Unit::loadClass(clsName.get());
  if (!cls) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat("Unknown class {}", clsName)
    );
  }
  auto const ctx = fromCaller(
    [] (const ActRec* fp, Offset) { return fp->func()->cls(); }
  );
  auto const lookup = cls->getSPropIgnoreLateInit(ctx, name.get());
  if (!lookup.val || !lookup.accessible) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat(
       "Unknown or inaccessible static property '{}' on class {}",
       name.get(),
       clsName.get()
      )
    );
  }
  return type(lookup.val) != KindOfUninit;
}

Array HHVM_FUNCTION(HH_global_keys) {
  return Array(get_global_variables()->keys());
}

bool HHVM_FUNCTION(HH_global_key_exists, StringArg key) {
  return g_context->m_globalVarEnv->lookup(key.get()) != nullptr;
}

/////////////////////////////////////////////////////////////////////////////

void StandardExtension::initVariable() {
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
  HHVM_FALIAS(HH\\is_list_like, HH_is_list_like);
  HHVM_FALIAS(HH\\is_meth_caller, HH_is_meth_caller);
  HHVM_FE(is_object);
  HHVM_FE(is_resource);
  HHVM_FE(boolval);
  HHVM_FE(intval);
  HHVM_FE(floatval);
  HHVM_FALIAS(doubleval, floatval);
  HHVM_FE(strval);
  HHVM_FE(gettype);
  HHVM_FE(get_resource_type);
  HHVM_FE(print_r);
  HHVM_FE(var_export);
  HHVM_FE(debug_zval_dump);
  HHVM_FE(var_dump);
  HHVM_FE(serialize);
  HHVM_FE(unserialize);
  HHVM_FE(parse_str);
  HHVM_FALIAS(HH\\object_prop_array, HH_object_prop_array);
  HHVM_FALIAS(HH\\serialize_with_options, HH_serialize_with_options);
  HHVM_FALIAS(HH\\Lib\\_Private\\Native\\first, HH_first);
  HHVM_FALIAS(HH\\Lib\\_Private\\Native\\last, HH_last);
  HHVM_FALIAS(HH\\Lib\\_Private\\Native\\first_key, HH_first_key);
  HHVM_FALIAS(HH\\Lib\\_Private\\Native\\last_key, HH_last_key);
  HHVM_FALIAS(HH\\is_late_init_prop_init, HH_is_late_init_prop_init);
  HHVM_FALIAS(HH\\is_late_init_sprop_init, HH_is_late_init_sprop_init);
  HHVM_FALIAS(HH\\global_keys, HH_global_keys);
  HHVM_FALIAS(HH\\global_key_exists, HH_global_key_exists);

  if (RuntimeOption::EnableIntrinsicsExtension) {
    HHVM_FALIAS(__hhvm_intrinsics\\serialize_keep_dvarrays,
                hhvm_intrinsics_serialize_keep_dvarrays);
    HHVM_FALIAS(__hhvm_intrinsics\\deserialize_keep_dvarrays,
                hhvm_intrinsics_unserialize_keep_dvarrays);
    HHVM_FALIAS(__hhvm_intrinsics\\create_class_pointer,
                hhvm_intrinsics_create_class_pointer);
  }

  loadSystemlib("std_variable");
}

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

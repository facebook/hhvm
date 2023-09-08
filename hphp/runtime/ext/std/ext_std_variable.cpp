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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"

#include "hphp/runtime/vm/jit/translator-inline.h"

#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/server/http-protocol.h"

#include "hphp/util/hphp-config.h"
#include "hphp/util/logger.h"

#include <folly/Likely.h>

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
  s_meth_caller_cls("__SystemLib\\MethCallerHelper"),
  s_dyn_meth_caller_cls("__SystemLib\\DynMethCallerHelper");

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
  if (RuntimeOption::EvalClassAsStringGetType &&
      (v.isLazyClass() || v.isClass())) {
    return s_string;
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

bool HHVM_FUNCTION(HH_is_php_array, const Variant& v) {
  return isArrayLikeType(v.getType()) && v.getArrayData()->isLegacyArray();
}

bool HHVM_FUNCTION(is_array, const Variant& v) {
  if (RO::EvalLogOnIsArrayFunction) {
    raise_notice("call to deprecated builtin is_array()");
  }
  return is_any_array(v.asTypedValue());
}

bool HHVM_FUNCTION(HH_is_vec, const Variant& v) {
  return is_vec(v.asTypedValue());
}

bool HHVM_FUNCTION(HH_is_dict, const Variant& v) {
  return is_dict(v.asTypedValue());
}

bool HHVM_FUNCTION(HH_is_keyset, const Variant& v) {
  return is_keyset(v.asTypedValue());
}

bool HHVM_FUNCTION(HH_is_varray, const Variant& val) {
  return is_vec(val.asTypedValue());
}

bool HHVM_FUNCTION(HH_is_darray, const Variant& val) {
  return is_dict(val.asTypedValue());
}

bool HHVM_FUNCTION(HH_is_vec_or_varray, const Variant& val) {
  return is_vec(val.asTypedValue());
}

bool HHVM_FUNCTION(HH_is_dict_or_darray, const Variant& val) {
  return is_dict(val.asTypedValue());
}

bool HHVM_FUNCTION(HH_is_any_array, const Variant& val) {
  return is_any_array(val.asTypedValue());
}

bool HHVM_FUNCTION(HH_is_list_like, const Variant& val) {
  auto const& ty = val.getType();
  if (!isArrayLikeType(ty)) return false;
  if (isVecType(ty)) return true;
  auto const& arr = val.asCArrRef();
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
    auto const mcCls = Class::lookup(s_meth_caller_cls.get());
    auto const dynMcCls = Class::lookup(s_dyn_meth_caller_cls.get());
    auto const cls = val(v).pobj->getVMClass();
    assertx(mcCls);
    assertx(dynMcCls);
    return mcCls == cls || dynMcCls == cls;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

Array HHVM_FUNCTION(HH_object_prop_array,
                    const Object& obj,
                    bool ignore_late_init /* = false */) {
  return obj.toArray(ignore_late_init).toDict();
}

///////////////////////////////////////////////////////////////////////////////
// input/output

ALWAYS_INLINE Variant print_r_impl(const Variant& expression,
                                   bool ret /* = false */,
                                   bool pure /* = false */) {
  Variant res;
  try {
    VariableSerializer vs(VariableSerializer::Type::PrintR);
    if (pure) {
      vs.setPure();
    }
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

Variant HHVM_FUNCTION(print_r, const Variant& expression,
                               bool ret /* = false */) {
  return print_r_impl(expression, ret, false);
}

Variant HHVM_FUNCTION(print_r_pure, const Variant& expression) {
  return print_r_impl(expression, true, true);
}

ALWAYS_INLINE Variant var_export_impl(const Variant& expression,
                                      bool ret /* = false */,
                                      bool pure /* = false */) {
  Variant res;
  try {
    VariableSerializer vs(VariableSerializer::Type::VarExport);
    if (pure) {
      vs.setPure();
    }
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

Variant HHVM_FUNCTION(var_export, const Variant& expression,
                                  bool ret /* = false */) {
  return var_export_impl(expression, ret, false);
}

Variant HHVM_FUNCTION(var_export_pure, const Variant& expression) {
  return var_export_impl(expression, true, true);
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
 * Intrinsic for Containers, i.e. the subset of \HH\Traversable including
 * 1. array:
 *   array, vec, dict, keyset
 * 2. collection: Vector, Map, Set
 * but not including Objects that implement e.g. \HH\Iterable or \HH\Iterator.
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

const StaticString s_Res("i:0;");

struct SerializeOptions {
  bool keepDVArrays = false;
  bool forcePHPArrays = false;
  bool warnOnHackArrays = false;
  bool warnOnPHPArrays = false;
  bool ignoreLateInit = false;
  bool serializeProvenanceAndLegacy = false;
  bool disallowObjects = false;
};

ALWAYS_INLINE String serialize_impl(const Variant& value,
                                    const SerializeOptions& opts,
                                    bool pure) {
  switch (value.getType()) {
    case KindOfClass:
    case KindOfLazyClass:
    case KindOfPersistentString:
    case KindOfString: {
      auto const str =
        isStringType(value.getType()) ? value.getStringData() :
        isClassType(value.getType()) ? classToStringHelper(value.toClassVal()) :
        lazyClassToStringHelper(value.toLazyClassVal());
      auto const size = str->size();
      if (size >= RuntimeOption::MaxSerializedStringSize) {
        throw Exception("Size of serialized string (%ld) exceeds max", size);
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

    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfFunc:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfDouble:
    case KindOfObject:
    case KindOfClsMeth:
    case KindOfRClsMeth:
    case KindOfRFunc:
    case KindOfEnumClassLabel:
      break;
  }
  VariableSerializer vs(VariableSerializer::Type::Serialize);
  if (opts.keepDVArrays)        vs.keepDVArrays();
  if (opts.forcePHPArrays)      vs.setForcePHPArrays();
  if (opts.warnOnHackArrays)    vs.setHackWarn();
  if (opts.warnOnPHPArrays)     vs.setPHPWarn();
  if (opts.ignoreLateInit)      vs.setIgnoreLateInit();
  if (opts.serializeProvenanceAndLegacy) vs.setSerializeProvenanceAndLegacy();
  if (opts.disallowObjects)      vs.setDisallowObjects();
  if (pure) vs.setPure();
  // Keep the count so recursive calls to serialize() embed references properly.
  return vs.serialize(value, true, true);
}

}

String HHVM_FUNCTION(serialize, const Variant& value) {
  return serialize_impl(value, SerializeOptions(), false);
}

String HHVM_FUNCTION(serialize_pure, const Variant& value) {
  return serialize_impl(value, SerializeOptions(), true);
}

const StaticString
  s_forcePHPArrays("forcePHPArrays"),
  s_keepDVArrays("keepDVArrays"),
  s_warnOnHackArrays("warnOnHackArrays"),
  s_warnOnPHPArrays("warnOnPHPArrays"),
  s_ignoreLateInit("ignoreLateInit"),
  s_disallowObjects("disallowObjects"),
  s_serializeProvenanceAndLegacy("serializeProvenanceAndLegacy");

String HHVM_FUNCTION(HH_serialize_with_options,
                     const Variant& value, const Array& options) {
  SerializeOptions opts;
  opts.keepDVArrays = options.exists(s_keepDVArrays) &&
    options[s_keepDVArrays].toBoolean();
  opts.forcePHPArrays = options.exists(s_forcePHPArrays) &&
    options[s_forcePHPArrays].toBoolean();
  opts.warnOnHackArrays = options.exists(s_warnOnHackArrays) &&
    options[s_warnOnHackArrays].toBoolean();
  opts.warnOnPHPArrays = options.exists(s_warnOnPHPArrays) &&
    options[s_warnOnPHPArrays].toBoolean();
  opts.ignoreLateInit = options.exists(s_ignoreLateInit) &&
    options[s_ignoreLateInit].toBoolean();
  opts.serializeProvenanceAndLegacy =
    options.exists(s_serializeProvenanceAndLegacy) &&
    options[s_serializeProvenanceAndLegacy].toBoolean();
  opts.disallowObjects = options.exists(s_disallowObjects) &&
    options[s_disallowObjects].toBoolean();
  return serialize_impl(value, opts, false);
}

String serialize_keep_dvarrays(const Variant& value) {
  SerializeOptions opts;
  opts.keepDVArrays = true;
  return serialize_impl(value, opts, false);
}

Variant HHVM_FUNCTION(unserialize, const String& str,
                                   const Array& options) {
  return unserialize_from_string(
    str,
    VariableUnserializer::Type::Serialize,
    options
  );
}

Variant HHVM_FUNCTION(unserialize_pure, const String& str,
                                        const Array& options) {
  return unserialize_from_string(
    str,
    VariableUnserializer::Type::Serialize,
    options,
    true
  );
}

///////////////////////////////////////////////////////////////////////////////
// variable table

void HHVM_FUNCTION(parse_str,
                   const String& str,
                   Array& arr) {
  arr = Array::CreateDict();
  HttpProtocol::DecodeParameters(arr, str.data(), str.size());
}

/////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(HH_is_late_init_prop_init,
                   const Object& obj,
                   const String& name) {
  auto const func = fromCaller(
    [] (const BTFrame& frm) { return frm.func(); }
  );
  auto const ctx = MemberLookupContext(func->cls(), func->moduleName());
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
  auto const cls = Class::load(clsName.get());
  if (!cls) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat("Unknown class {}", clsName)
    );
  }
  auto const func =fromCaller(
    [] (const BTFrame& frm) { return frm.func(); }
  );
  auto const ctx =  MemberLookupContext(func->cls(), func->unit()->moduleName());
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

bool HHVM_FUNCTION(HH_global_key_exists, StringArg key) {
  return g_context->m_globalNVTable->lookup(key.get()) != nullptr;
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
  HHVM_FALIAS(HH\\is_vec_or_varray, HH_is_vec_or_varray);
  HHVM_FALIAS(HH\\is_dict_or_darray, HH_is_dict_or_darray);
  HHVM_FALIAS(HH\\is_any_array, HH_is_any_array);
  HHVM_FALIAS(HH\\is_php_array, HH_is_php_array);
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
  HHVM_FE(print_r_pure);
  HHVM_FE(var_export);
  HHVM_FE(var_export_pure);
  HHVM_FE(debug_zval_dump);
  HHVM_FE(var_dump);
  HHVM_FE(serialize);
  HHVM_FE(serialize_pure);
  HHVM_FE(unserialize);
  HHVM_FE(unserialize_pure);
  HHVM_FE(parse_str);
  HHVM_FALIAS(HH\\object_prop_array, HH_object_prop_array);
  HHVM_FALIAS(HH\\serialize_with_options, HH_serialize_with_options);
  // Clang 15 doesn't like the HHVM_FALIAS macro with \\N
  HHVM_FALIAS_FE_STR("HH\\Lib\\_Private\\Native\\first", HH_first);
  HHVM_FALIAS_FE_STR("HH\\Lib\\_Private\\Native\\last", HH_last);
  HHVM_FALIAS_FE_STR("HH\\Lib\\_Private\\Native\\first_key", HH_first_key);
  HHVM_FALIAS_FE_STR("HH\\Lib\\_Private\\Native\\last_key", HH_last_key);
  HHVM_FALIAS(HH\\is_late_init_prop_init, HH_is_late_init_prop_init);
  HHVM_FALIAS(HH\\is_late_init_sprop_init, HH_is_late_init_sprop_init);
  HHVM_FALIAS(HH\\global_key_exists, HH_global_key_exists);
}

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/type-structure.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/util/text-util.h"

namespace HPHP {

struct String;
struct StaticString;

namespace {

struct TSEnv {
  bool allow_partial{};

  // Passed in by ref variables from the caller

  // Initial value true since something can only lose persistence
  bool persistent{true};
  // Initial value false since unless we have to resort to partial
  // evaluation, nothing is partial
  bool partial{};
  // Initial value false since unless proven there are no invalid types
  bool invalidType{};
  // Vector of typestructures that need to be put in for reified generics
  const req::vector<Array>* tsList;
};

/*
 * These static strings are the same as the ones in
 * hphp/compiler/type_annotation.cpp, where the typeAnnotArrays are
 * originally generated. See TypeAnnotation::getScalarArrayRep().
 */
const StaticString
  s_nullable("nullable"),
  s_exact("exact"),
  s_like("like"),
  s_name("name"),
  s_classname("classname"),
  s_kind("kind"),
  s_elem_types("elem_types"),
  s_return_type("return_type"),
  s_variadic_type("variadic_type"),
  s_param_types("param_types"),
  s_generic_types("generic_types"),
  s_root_name("root_name"),
  s_access_list("access_list"),
  s_fields("fields"),
  s_allows_unknown_fields("allows_unknown_fields"),
  s_is_cls_cns("is_cls_cns"),
  s_optional_shape_field("optional_shape_field"),
  s_value("value"),
  s_this("HH\\this"),
  s_self("self"),
  s_parent("parent"),
  s_callable("callable"),
  s_alias("alias"),
  s_typevars("typevars"),
  s_typevar_types("typevar_types"),
  s_id("id"),
  s_soft("soft")
;

const std::string
  s_null("null"),
  s_void("void"),
  s_int("int"),
  s_bool("bool"),
  s_float("float"),
  s_string("string"),
  s_resource("resource"),
  s_num("num"),
  s_arraykey("arraykey"),
  s_nothing("nothing"),
  s_noreturn("noreturn"),
  s_mixed("mixed"),
  s_dynamic("dynamic"),
  s_nonnull("nonnull"),
  s_array("array"),
  s_darray("HH\\darray"),
  s_varray("HH\\varray"),
  s_varray_or_darray("HH\\varray_or_darray"),
  s_shape("shape"),
  s_hh_vec("HH\\vec"),
  s_hh_dict("HH\\dict"),
  s_hh_keyset("HH\\keyset"),
  s_hh_vec_or_dict("HH\\vec_or_dict"),
  s_hh_arraylike("HH\\arraylike"),
  s_hh("HH\\")
;

std::string fullName(const Array& arr, bool forDisplay);

void functionTypeName(const Array& arr, std::string& name, bool forDisplay) {
  name += "(function (";

  assertx(arr.exists(s_return_type));
  auto const retType = arr[s_return_type].toCArrRef();

  assertx(arr.exists(s_param_types));
  auto const params = arr[s_param_types].toCArrRef();

  auto sep = "";
  auto const sz = params.size();
  for (auto i = 0; i < sz; i++) {
    auto const param = params[i].toCArrRef();
    folly::toAppend(sep, fullName(param, forDisplay), &name);
    sep = ", ";
  }

  if (arr.exists(s_variadic_type)) {
    auto const variadicType = arr[s_variadic_type].toCArrRef();
    folly::toAppend(sep, fullName(variadicType, forDisplay), "...", &name);
  }

  folly::toAppend("): ", fullName(retType, forDisplay), ")", &name);
}

void accessTypeName(const Array& arr, std::string& name) {
  assertx(arr.exists(s_root_name));
  auto const rootName = arr[s_root_name].toCStrRef();
  name += rootName.toCppString();

  assertx(arr.exists(s_access_list));
  auto const accessList = arr[s_access_list].toCArrRef();
  auto const sz = accessList.size();
  for (auto i = 0; i < sz; i++) {
    folly::toAppend("::",
                    accessList[i].toCStrRef().toCppString(),
                    &name);
  }
}

// xhp names are mangled so we get them back to their original definition
// see the mangling in ScannerToken::xhpLabel
void xhpTypeName(const Array& arr, std::string& name) {
  assertx(arr.exists(s_classname));
  std::string clsName = arr[s_classname].toCStrRef().toCppString();
  // remove prefix if any
  if (clsName.compare(0, sizeof("xhp_") - 1, "xhp_") == 0) {
    name += clsName.replace(0, sizeof("xhp_") - 1, ":");
  } else {
    name += clsName;
  }
  // un-mangle back
  replaceAll(name, "__", ":");
  replaceAll(name, "_", "-");
}

void tupleTypeName(const Array& arr, std::string& name, bool forDisplay) {
  name += "(";
  assertx(arr.exists(s_elem_types));
  auto const elems = arr[s_elem_types].toCArrRef();
  auto const sz = elems.size();
  auto sep = "";
  for (auto i = 0; i < sz; i++) {
    auto const elem = elems[i].toCArrRef();
    folly::toAppend(sep, fullName(elem, forDisplay), &name);
    sep = ", ";
  }

  name += ")";
}

void genericTypeName(const Array& arr, std::string& name, bool forDisplay) {
  name += "<";
  assertx(arr.exists(s_generic_types));
  auto const args = arr[s_generic_types].toCArrRef();
  auto const sz = args.size();
  auto sep = "";
  for (auto i = 0; i < sz; i++) {
    auto const arg = args[i].toCArrRef();
    folly::toAppend(sep, fullName(arg, forDisplay), &name);
    sep = ", ";
  }
  name += ">";
}

void shapeTypeName(const Array& arr, std::string& name, bool forDisplay) {
  // works for both resolved and unresolved TypeStructures
  name += "(";
  assertx(arr.exists(s_fields));
  auto const fields = arr[s_fields].toCArrRef();
  auto const sz = fields.size();
  auto sep = "";
  for (auto i = 0; i < sz; i++) {
    name += sep;
    auto const field = fields->getKey(i);
    auto value = fields->getValue(i).toCArrRef();
    auto quote = "'";
    if (value.exists(s_optional_shape_field)) {
      name += "?";
    }
    if (value.exists(s_value)) {
      // if unresolved, ignore wrapper
      if (value.exists(s_is_cls_cns)) quote = "";
      value = value[s_value].toCArrRef();
    }
    auto const fieldType = field.getType();
    if (isStringType(fieldType)) {
      folly::toAppend(quote, field.toCStrRef().data(), quote, &name);
    } else if (isIntType(fieldType)) {
      folly::toAppend(field.toInt64Val(), &name);
    }

    folly::toAppend(
      forDisplay ? " => " : "=>",
      fullName(value, forDisplay),
      &name
    );
    sep = ", ";
  }

  if (arr.exists(s_allows_unknown_fields)) {
    folly::toAppend(sep, "...", &name);
  }

  name += ")";
}

std::string fullName(const Array& arr, bool forDisplay) {
  std::string name;

  if (arr.exists(s_like)) {
    assertx(arr[s_like].toBoolean());
    name += '~';
  }

  if (arr.exists(s_nullable)) {
    assertx(arr[s_nullable].toBoolean());
    name += '?';
  }

  if (arr.exists(s_soft)) {
    assertx(arr[s_soft].toBoolean());
    name += '@';
  }

  assertx(arr.exists(s_kind));

  TypeStructure::Kind kind =
    TypeStructure::Kind(arr[s_kind].toInt64Val());
  switch (kind) {
    case TypeStructure::Kind::T_null:
      name += forDisplay ? s_null : s_hh + s_null;
      break;
    case TypeStructure::Kind::T_void:
      name += forDisplay ? s_void : s_hh + s_void;
      break;
    case TypeStructure::Kind::T_int:
      name += forDisplay ? s_int : s_hh + s_int;
      break;
    case TypeStructure::Kind::T_bool:
      name += forDisplay ? s_bool : s_hh + s_bool;
      break;
    case TypeStructure::Kind::T_float:
      name += forDisplay ? s_float : s_hh + s_float;
      break;
    case TypeStructure::Kind::T_string:
      name += forDisplay ? s_string : s_hh + s_string;
      break;
    case TypeStructure::Kind::T_resource:
      name += forDisplay ? s_resource : s_hh + s_resource;
      break;
    case TypeStructure::Kind::T_num:
      name += forDisplay ? s_num : s_hh + s_num;
      break;
    case TypeStructure::Kind::T_arraykey:
      name += forDisplay ? s_arraykey : s_hh + s_arraykey;
      break;
    case TypeStructure::Kind::T_nothing:
      name += forDisplay ? s_nothing : s_hh + s_nothing;
      break;
    case TypeStructure::Kind::T_noreturn:
      name += forDisplay ? s_noreturn : s_hh + s_noreturn;
      break;
    case TypeStructure::Kind::T_mixed:
      name += forDisplay ? s_mixed : s_hh + s_mixed;
      break;
    case TypeStructure::Kind::T_dynamic:
      name += forDisplay ? s_dynamic : s_hh + s_dynamic;
      break;
    case TypeStructure::Kind::T_nonnull:
      name += forDisplay ? s_nonnull : s_hh + s_nonnull;
      break;
    case TypeStructure::Kind::T_tuple:
      tupleTypeName(arr, name, forDisplay);
      break;
    case TypeStructure::Kind::T_fun:
      functionTypeName(arr, name, forDisplay);
      break;
    case TypeStructure::Kind::T_array:
      name += s_array;
      if (arr.exists(s_generic_types)) {
        genericTypeName(arr, name, forDisplay);
      }
      break;
    case TypeStructure::Kind::T_darray:
      name += s_darray;
      if (arr.exists(s_generic_types)) {
        genericTypeName(arr, name, forDisplay);
      }
      break;
    case TypeStructure::Kind::T_varray:
      name += s_varray;
      if (arr.exists(s_generic_types)) {
        genericTypeName(arr, name, forDisplay);
      }
      break;
    case TypeStructure::Kind::T_varray_or_darray:
      name += s_varray_or_darray;
      if (arr.exists(s_generic_types)) {
        genericTypeName(arr, name, forDisplay);
      }
      break;
    case TypeStructure::Kind::T_shape:
      name += forDisplay ? s_shape : s_hh + s_shape;
      shapeTypeName(arr, name, forDisplay);
      break;
    case TypeStructure::Kind::T_dict:
      name += s_hh_dict;
      if (arr.exists(s_generic_types)) {
        genericTypeName(arr, name, forDisplay);
      }
      break;
    case TypeStructure::Kind::T_vec:
      name += s_hh_vec;
      if (arr.exists(s_generic_types)) {
        genericTypeName(arr, name, forDisplay);
      }
      break;
    case TypeStructure::Kind::T_keyset:
      name += s_hh_keyset;
      if (arr.exists(s_generic_types)) {
        genericTypeName(arr, name, forDisplay);
      }
      break;
    case TypeStructure::Kind::T_vec_or_dict:
      name += s_hh_vec_or_dict;
      if (arr.exists(s_generic_types)) {
        genericTypeName(arr, name, forDisplay);
      }
      break;
    case TypeStructure::Kind::T_arraylike:
      name += s_hh_arraylike;
      if (arr.exists(s_generic_types)) {
        genericTypeName(arr, name, forDisplay);
      }
      break;
    case TypeStructure::Kind::T_typevar:
      assertx(arr.exists(s_name));
      name += arr[s_name].toCStrRef().toCppString();
      break;
    case TypeStructure::Kind::T_typeaccess:
      accessTypeName(arr, name);
      break;
    case TypeStructure::Kind::T_xhp:
      xhpTypeName(arr, name);
      break;
    case TypeStructure::Kind::T_reifiedtype:
      assertx(arr.exists(s_id));
      name += "reified ";
      name += arr[s_id].toCStrRef().data();
      break;
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_trait:
    case TypeStructure::Kind::T_enum:
    case TypeStructure::Kind::T_unresolved:
      assertx(arr.exists(s_classname));
      name += arr[s_classname].toCStrRef().toCppString();
      if (arr.exists(s_generic_types)) genericTypeName(arr, name, forDisplay);
      break;
  }

  return name;
}

Array resolveTS(TSEnv& env,
                const Array& arr,
                const Class::Const& typeCns,
                const Class* typeCnsCls,
                const Array& generics);

Array resolveList(TSEnv& env,
                  const Array& arr,
                  const Class::Const& typeCns,
                  const Class* typeCnsCls,
                  const Array& generics) {
  auto const sz = arr.size();

  VArrayInit newarr(sz);
  for (auto i = 0; i < sz; i++) {
    auto elemArr = arr[i].toArray();
    auto elem = resolveTS(env, elemArr, typeCns, typeCnsCls, generics);
    newarr.append(Variant(elem));
  }

  return newarr.toArray();
}

std::string resolveContextMsg(const Class::Const& typeCns,
                              const Class* typeCnsCls) {
  std::string msg("when resolving ");
  if (!typeCns.name) {
    folly::toAppend("anonymous type structure", &msg);
  } else if (typeCnsCls) {
    folly::toAppend("type constant ", typeCnsCls->name()->data(),
                    "::", typeCns.name->data(),
                    &msg);
  } else {
    folly::toAppend("type alias ", typeCns.name->data(), &msg);
  }
  return msg;
}

/* returns the unresolved TypeStructure; if aliasName is not an alias,
 * return an empty Array. */
Array getAlias(TSEnv& env, const String& aliasName) {
  if (aliasName.same(s_this) || Unit::lookupClass(aliasName.get())) {
    return Array::CreateDArray();
  }

  auto persistentTA = true;
  auto typeAliasReq = env.allow_partial
    ? Unit::lookupTypeAlias(aliasName.get(), &persistentTA)
    : Unit::loadTypeAlias(aliasName.get(), &persistentTA);
  if (!typeAliasReq) {
    env.partial = true;
    return Array::CreateDArray();
  }

  // this returned type structure is unresolved.
  assertx(typeAliasReq->typeStructure.isDictOrDArray());
  env.persistent &= persistentTA;
  return typeAliasReq->typeStructure;
}

const Class* getClass(TSEnv& env,
                      const String& clsName,
                      const Class::Const& typeCns,
                      const Class* typeCnsCls) {
  auto checkPersistent = [&](const Class* cls) {
    env.persistent &= classHasPersistentRDS(cls);
    return cls;
  };

  // the original unresolved type structure came from a type constant
  // (instead of a type alias), and may have this/self/parent.
  if (typeCnsCls) {
    // HH\this: late static binding
    if (clsName.same(s_this)) {
      return checkPersistent(typeCnsCls);
    }

    auto declCls = typeCns.cls;
    // self
    if (clsName.same(s_self)) {
      return checkPersistent(declCls);
    }
    // parent
    if (clsName.same(s_parent)) {
      auto parent = declCls->parent();
      if (!parent) {
        throw Exception(
          "%s, class %s does not have a parent",
          resolveContextMsg(typeCns, typeCnsCls).c_str(),
          declCls->name()->data());
      }
      return checkPersistent(parent);
    }
  }

  auto name = clsName;
  auto ts = getAlias(env, name);
  while (!ts.empty()) {
    assertx(ts.exists(s_kind));
    if (!ts.exists(s_classname)) {
      // not a class, interface, trait, enum, or alias
      throw Exception(
        "%s, alias %s does not resolve to a class",
        resolveContextMsg(typeCns, typeCnsCls).c_str(),
        name.data());
    }
    name = ts[s_classname].toCStrRef();
    ts = getAlias(env, name);
  }

  auto const cls = env.allow_partial ? Unit::lookupClass(name.get())
                                     : Unit::loadClass(name.get());
  if (!cls) {
    env.partial = true;
    if (env.allow_partial) return nullptr;
    throw Exception(
      "%s, class %s not found",
      resolveContextMsg(typeCns, typeCnsCls).c_str(),
      name.data());
  }

  return checkPersistent(cls);
}

/* Given an unresolved T_shape TypeStructure, returns the __fields__
 * portion of the array with all the field names resolved to string
 * literals. */
Array resolveShape(TSEnv& env,
                   const Array& arr,
                   const Class::Const& typeCns,
                   const Class* typeCnsCls,
                   const Array& generics) {
  assertx(arr.exists(s_kind));
  assertx(static_cast<TypeStructure::Kind>(arr[s_kind].toInt64Val())
         == TypeStructure::Kind::T_shape);
  assertx(arr.exists(s_fields));

  auto newfields = Array::CreateDArray();
  auto const fields = arr[s_fields].toCArrRef();
  auto const sz = fields.size();
  for (auto i = 0; i < sz; i++) {
    Variant key = fields->getKey(i);
    auto const wrapper = fields->getValue(i).toCArrRef();
    if (wrapper.exists(s_is_cls_cns)) {
      // if the shape field name is a class constant, its name is
      // double colon delimited clsName::cnsName
      auto const clsCns = key.toCStrRef().toCppString();
      std::string clsName, cnsName;
      folly::split("::", clsCns, clsName, cnsName);

      // look up clsName::cnsName
      auto cls = getClass(env, String(clsName), typeCns, typeCnsCls);
      if (!cls) throw Exception("failed to resolve shape classname");
      auto cnsValue = cls->clsCnsGet(String(cnsName).get());

      if (isStringType(cnsValue.m_type) || isIntType(cnsValue.m_type)) {
        key = tvAsVariant(&cnsValue);
      } else {
        throw Exception(
          "class constant %s::%s is not a string or an int "
          "and cannot be used as shape field names",
          clsName.c_str(), cnsName.c_str());
      }
    }

    // If the TS was resolved before then value field no longer exists, the
    // value is instead flattened.
    auto valueArr =
      wrapper.exists(s_value) ? wrapper[s_value].toArray() : wrapper;
    auto value = resolveTS(env, valueArr, typeCns, typeCnsCls, generics);

    if (wrapper.exists(s_optional_shape_field)) {
      value.set(s_optional_shape_field, make_tv<KindOfBoolean>(true));
    }

    newfields.set(key, Variant(value));
  }

  return newfields;
}

bool resolveClass(TSEnv& env,
                  Array& ret,
                  const String& clsName,
                  const Class::Const& typeCns,
                  const Class* typeCnsCls) {
  auto const cls = getClass(env, clsName, typeCns, typeCnsCls);
  if (!cls) return false;

  TypeStructure::Kind resolvedKind;
  if (isNormalClass(cls)) {
    resolvedKind = TypeStructure::Kind::T_class;
  } else if (isInterface(cls)) {
    resolvedKind = TypeStructure::Kind::T_interface;
  } else if (isTrait(cls)) {
    resolvedKind = TypeStructure::Kind::T_trait;
    env.invalidType = true;
  } else if (isEnum(cls)) {
    resolvedKind = TypeStructure::Kind::T_enum;
  } else {
    not_reached();
  }

  ret.set(s_kind, Variant(static_cast<uint8_t>(resolvedKind)));
  ret.set(s_classname, Variant(makeStaticString(cls->name())));
  if (clsName.same(s_this)) ret.set(s_exact, make_tv<KindOfBoolean>(true));

  return true;
}

Array resolveGenerics(TSEnv& env,
                      const Array& arr,
                      const Class::Const& typeCns,
                      const Class* typeCnsCls,
                      const Array& generics) {
  auto genericsArr = arr[s_generic_types].toArray();
  return resolveList(env, genericsArr, typeCns, typeCnsCls, generics);
}

/**
 * Copy modifiers, i.e. whether the type is nullable, soft, or a like-type.
 */
void copyTypeModifiers(const Array& from, Array& to) {
  if (from.exists(s_like))     to.set(s_like, make_tv<KindOfBoolean>(true));
  if (from.exists(s_nullable)) to.set(s_nullable, make_tv<KindOfBoolean>(true));
  if (from.exists(s_soft))     to.set(s_soft, make_tv<KindOfBoolean>(true));
}

Array resolveTS(TSEnv& env,
                const Array& arr,
                const Class::Const& typeCns,
                const Class* typeCnsCls,
                const Array& generics) {
  assertx(arr.exists(s_kind));
  auto const kind = static_cast<TypeStructure::Kind>(
    arr[s_kind].toInt64Val());

  auto newarr = Array::CreateDArray();
  copyTypeModifiers(arr, newarr);
  newarr.set(s_kind, Variant(static_cast<uint8_t>(kind)));

  if (arr.exists(s_allows_unknown_fields)) {
    newarr.set(s_allows_unknown_fields, make_tv<KindOfBoolean>(true));
  }

  switch (kind) {
    case TypeStructure::Kind::T_tuple: {
      assertx(arr.exists(s_elem_types));
      auto const elemsArr = arr[s_elem_types].toCArrRef();
      auto const elemTypes =
        resolveList(env, elemsArr, typeCns, typeCnsCls, generics);
      newarr.set(s_elem_types, Variant(elemTypes));
      break;
    }
    case TypeStructure::Kind::T_fun: {
      env.invalidType = true;
      assertx(arr.exists(s_return_type));
      auto const returnArr = arr[s_return_type].toCArrRef();
      auto const returnType =
        resolveTS(env, returnArr, typeCns, typeCnsCls, generics);
      newarr.set(s_return_type, Variant(returnType));

      assertx(arr.exists(s_param_types));
      auto const paramsArr = arr[s_param_types].toCArrRef();
      auto const paramTypes =
        resolveList(env, paramsArr, typeCns, typeCnsCls, generics);
      newarr.set(s_param_types, Variant(paramTypes));

      if (arr.exists(s_variadic_type)) {
        auto const variadicArr = arr[s_variadic_type].toCArrRef();
        auto const variadicType =
          resolveTS(env, variadicArr, typeCns, typeCnsCls, generics);
        newarr.set(s_variadic_type, Variant(variadicType));
      }

      break;
    }
    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
    case TypeStructure::Kind::T_dict:
    case TypeStructure::Kind::T_vec:
    case TypeStructure::Kind::T_keyset:
    case TypeStructure::Kind::T_vec_or_dict:
    case TypeStructure::Kind::T_arraylike: {
      if (kind == TypeStructure::Kind::T_array ||
        kind == TypeStructure::Kind::T_darray ||
        kind == TypeStructure::Kind::T_varray ||
        kind == TypeStructure::Kind::T_varray_or_darray
      ) {
        env.invalidType = true;
      }
      if (arr.exists(s_generic_types)) {
        newarr.set(s_generic_types,
                   Variant(resolveGenerics(env, arr, typeCns, typeCnsCls,
                                           generics)));
      }
      break;
    }
    case TypeStructure::Kind::T_shape: {
      auto const fields = resolveShape(env, arr, typeCns, typeCnsCls, generics);
      newarr.set(s_fields, Variant(fields));
      break;
    }
    case TypeStructure::Kind::T_unresolved: {
      assertx(arr.exists(s_classname));
      auto const clsName = arr[s_classname].toCStrRef();
      auto ts = getAlias(env, clsName);

      auto resolve = [&] (const Array& generics = Array()) {
        Class::Const typeCns;
        typeCns.name = clsName.get();
        auto resolved = resolveTS(env, ts, typeCns, nullptr, generics);
        resolved.set(s_alias, Variant(clsName));
        return resolved;
      };

      if (!ts.empty()) {
        if (ts.exists(s_typevars) && arr.exists(s_generic_types)) {
          std::vector<std::string> typevars;
          folly::split(",", ts[s_typevars].toCStrRef().data(), typevars);
          ts.remove(s_typevars);

          auto generic_types =
            resolveGenerics(env, arr, typeCns, typeCnsCls, generics);

          auto const sz = std::min(static_cast<ssize_t>(typevars.size()),
                                   generic_types.size());
          DArrayInit newarr(sz);
          for (auto i = 0; i < sz; i++) {
            newarr.add(String(typevars[i]), generic_types[i]);
          }
          auto generics = newarr.toArray();
          ts = resolve(generics);
          ts.set(s_typevar_types, Variant(generics));
        } else {
          ts = resolve();
        }
        copyTypeModifiers(arr, ts);
        return ts;
      }

      /* Special cases for 'callable': Hack typechecker throws a naming error
       * (unbound name), however, hhvm still supports this type hint to be
       * compatible with php. We simply return as a OF_CLASS with class name
       * set to 'callable'. */
      if (clsName.same(s_callable)) {
        newarr.set(s_kind,
                   Variant(static_cast<uint8_t>(TypeStructure::Kind::T_class)));
        newarr.set(s_classname, Variant(clsName));
        break;
      }
      if (!resolveClass(env, newarr, clsName, typeCns, typeCnsCls) &&
          env.allow_partial) {
        env.partial = true;
        newarr.set(s_kind,
                   Variant(static_cast<uint8_t>(
                           TypeStructure::Kind::T_unresolved)));
        newarr.set(s_classname, Variant(clsName));
        break;
      }
      if (arr.exists(s_generic_types)) {
        newarr.set(s_generic_types,
                   Variant(resolveGenerics(env, arr, typeCns, typeCnsCls,
                                           generics)));
      }
      break;
    }
    case TypeStructure::Kind::T_typeaccess: {
      /* type access is a root class (may be HH\this) followed by a series of
       * type constants, i.e., cls::TC1::TC2::TC3. Each type constant other
       * than the last one in the chain must refer to a class or an
       * interface. */
      assertx(arr.exists(s_root_name));
      auto clsName = arr[s_root_name].toCStrRef();
      assertx(arr.exists(s_access_list));
      auto const accList = arr[s_access_list].toCArrRef();
      auto const sz = accList.size();
      Array typeCnsVal;
      for (auto i = 0; i < sz; i++) {
        auto const cls = getClass(env, clsName, typeCns, typeCnsCls);
        if (!cls) throw Exception("failed to resolve type access");
        auto const cnsName = accList[i].toCStrRef();
        if (!cls->hasTypeConstant(cnsName.get())) {
          throw Exception(
            "%s, class %s does not have non-abstract "
            "type constant %s",
            resolveContextMsg(typeCns, typeCnsCls).c_str(),
            clsName.data(),
            cnsName.data());
        }
        auto tv = cls->clsCnsGet(
          cnsName.get(),
          env.allow_partial ?
          ClsCnsLookup::IncludeTypesPartial : ClsCnsLookup::IncludeTypes);
        if (tv.m_type == KindOfUninit) {
          assertx(env.allow_partial);
          throw Exception(
            "Failed to resolve a type constant: %s::%s",
            cls->name()->data(), cnsName.get()->data()
          );
        }
        assertx(isArrayLikeType(tv.m_type));
        typeCnsVal = Array(tv.m_data.parr);
        assertx(typeCnsVal.isDictOrDArray());
        if (i == sz - 1) break;

        // if there are more accesses, keep resolving
        assertx(typeCnsVal.exists(s_kind));
        auto kind = static_cast<TypeStructure::Kind>
          (typeCnsVal[s_kind].toInt64Val());
        if (kind != TypeStructure::Kind::T_class
            && kind != TypeStructure::Kind::T_interface) {
          throw Exception(
            "%s, %s::%s does not resolve to a class or "
            "an interface and cannot contain type constant %s",
            resolveContextMsg(typeCns, typeCnsCls).c_str(),
            clsName.data(),
            cnsName.data(),
            accList[i+1].toCStrRef().data());
        }
        assertx(typeCnsVal.exists(s_classname));
        clsName = typeCnsVal[s_classname].toCStrRef();
      }
      copyTypeModifiers(arr, typeCnsVal);
      return typeCnsVal;
    }
    case TypeStructure::Kind::T_typevar: {
      env.invalidType = true;
      assertx(arr.exists(s_name));
      auto const name = arr[s_name].toCStrRef();
      return generics.exists(name) ? generics[name].toDArray() : arr.toDArray();
    }
    case TypeStructure::Kind::T_reifiedtype: {
      assertx(env.tsList != nullptr);
      assertx(arr.exists(s_id));
      auto id = arr[s_id].toInt64Val();
      assertx(id < env.tsList->size());
      // We want the data in the reified generic array to write over the data
      // in newarr hence using merge
      newarr = newarr.merge(env.tsList->at(id)).toDArray();
      break;
    }
    case TypeStructure::Kind::T_xhp:
    default:
      return arr.toDArray();
  }

  if (arr.exists(s_typevars)) newarr.set(s_typevars, arr[s_typevars]);

  return newarr;
}

} // anonymous namespace

String TypeStructure::toString(const Array& arr) {
  if (arr.empty()) return String();

  return String(fullName(arr, false));
}

String TypeStructure::toStringForDisplay(const Array& arr) {
  if (arr.empty()) return String();

  return String(fullName(arr, true));
}

/*
 * Constructs a scalar array with all the shape field names, this/self/parent,
 * classes, type accesses, and type aliases resolved.
 */
Array TypeStructure::resolve(const Class::Const& typeCns,
                             const Class* typeCnsCls,
                             bool& persistent) {
  assertx(typeCns.isType());
  assertx(isArrayLikeType(typeCns.val.m_type));
  assertx(typeCns.name);
  assertx(typeCnsCls);

  TSEnv env;
  Array arr(typeCns.val.m_data.parr);
  auto resolved = resolveTS(env, arr, typeCns, typeCnsCls, Array());
  persistent = env.persistent;
  return resolved;
}

/*
 * Called by TypeAliasReq to get resolved TypeStructure for type aliases.
 */
Array TypeStructure::resolve(const String& aliasName,
                             const Array& arr,
                             bool& persistent,
                             const Array& generics) {
  // use a bogus constant to store the name
  Class::Const typeCns;
  typeCns.name = aliasName.get();
  TSEnv env;
  auto resolved = resolveTS(env, arr, typeCns, nullptr, generics);
  resolved.set(s_alias, Variant(aliasName));
  persistent = env.persistent;
  return resolved;
}

/*
 * Called when resolving anonymous type structures, e.g. from `is` expressions.
 */
Array TypeStructure::resolve(const Array& ts,
                             const Class* typeCnsCls,
                             const Class* declCls,
                             const req::vector<Array>& tsList,
                             bool& persistent) {
  // Use a bogus constant, because the type structure is anonymous.
  Class::Const typeCns;
  typeCns.name = nullptr;
  typeCns.cls = declCls;
  TSEnv env;
  env.tsList = &tsList;
  auto resolved = resolveTS(env, ts, typeCns, typeCnsCls, Array());
  persistent = env.persistent;
  return resolved;
}

/*
 * Called when resolving anonymous type structures, e.g. from `is` expressions
 * while allowing partial resolution.
 */
Array TypeStructure::resolvePartial(const Array& ts,
                                    const Class* typeCnsCls,
                                    const Class* declCls,
                                    bool& persistent,
                                    bool& partial,
                                    bool& invalidType) {
  // Use a bogus constant, because the type structure is anonymous.
  Class::Const typeCns;
  typeCns.name = nullptr;
  typeCns.cls = declCls;
  TSEnv env;
  env.allow_partial = true;
  auto resolved = resolveTS(env, ts, typeCns, typeCnsCls, Array());
  persistent = env.persistent;
  partial = env.partial;
  invalidType = env.invalidType;
  return resolved;
}

bool TypeStructure::isValidResolvedTypeStructureList(const Array& arr,
                                                     bool isShape /*= false*/) {
  if (!(RuntimeOption::EvalHackArrDVArrs
          ? arr.isVecArray() : (arr.isPHPArray() || !arr.isNull()))) {
    return false;
  }
  bool valid = true;
  IterateV(
    arr.get(),
    [&](TypedValue v) {
      if (!tvIsDictOrDArray(v)) {
        valid = false;
        return true;
      }
      auto const parr = [&] {
        if (isShape) {
          auto const value = arr.rvalAt(s_value);
          if (value.is_set()) {
            if (!isDictOrArrayType(value.type())) {
              valid = false;
            } else {
              return value.val().parr;
            }
          }
        }
        return v.m_data.parr;
      }();
      if (!valid) return true;
      valid &= TypeStructure::isValidResolvedTypeStructure(ArrNR(parr));
      return !valid; // if valid is false, let's short circuit
    }
  );
  return valid;
}

bool TypeStructure::isValidResolvedTypeStructure(const Array& arr) {
  if (!(RuntimeOption::EvalHackArrDVArrs
          ? arr.isDict() : (arr.isPHPArray() && !arr.isNull()))) {
    return false;
  }
  auto const kindfield = arr.rvalAt(s_kind);
  if (!kindfield.is_set() || !isIntType(kindfield.type()) ||
      kindfield.val().num > TypeStructure::kMaxResolvedKind) {
    return false;
  }
  auto const kind = static_cast<TypeStructure::Kind>(kindfield.val().num);
  switch (kind) {
    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
    case TypeStructure::Kind::T_dict:
    case TypeStructure::Kind::T_vec:
    case TypeStructure::Kind::T_keyset:
    case TypeStructure::Kind::T_vec_or_dict:
    case TypeStructure::Kind::T_arraylike:
    case TypeStructure::Kind::T_null:
    case TypeStructure::Kind::T_void:
    case TypeStructure::Kind::T_int:
    case TypeStructure::Kind::T_bool:
    case TypeStructure::Kind::T_float:
    case TypeStructure::Kind::T_string:
    case TypeStructure::Kind::T_resource:
    case TypeStructure::Kind::T_num:
    case TypeStructure::Kind::T_arraykey:
    case TypeStructure::Kind::T_nothing:
    case TypeStructure::Kind::T_noreturn:
    case TypeStructure::Kind::T_mixed:
    case TypeStructure::Kind::T_dynamic:
    case TypeStructure::Kind::T_nonnull:
      return true;
    case TypeStructure::Kind::T_fun: {
      auto const rtype = arr.rvalAt(s_return_type);
      if (!rtype.is_set() || !isDictOrArrayType(rtype.type())) return false;
      if (!TypeStructure::isValidResolvedTypeStructure(
            ArrNR(rtype.val().parr))) {
        return false;
      }
      auto const vtype = arr.rvalAt(s_variadic_type);
      if (vtype.is_set()) {
        if (!isDictOrArrayType(vtype.type())) return false;
        auto const varr = ArrNR(rtype.val().parr);
        if (!TypeStructure::isValidResolvedTypeStructure(varr)) return false;
      }
      auto const ptypes = arr.rvalAt(s_param_types);
      if (!ptypes.is_set() || !isVecOrArrayType(ptypes.type())) return false;
      return isValidResolvedTypeStructureList(ArrNR(ptypes.val().parr));
    }
    case TypeStructure::Kind::T_typevar: {
      auto const name = arr.rvalAt(s_name);
      return name.is_set() && isStringType(name.type());
    }
    case TypeStructure::Kind::T_shape: {
      auto const fields = arr.rvalAt(s_fields);
      if (!fields.is_set() || !isVecOrArrayType(fields.type())) return false;
      return isValidResolvedTypeStructureList(ArrNR(fields.val().parr), true);
    }
    case TypeStructure::Kind::T_tuple: {
      auto const elems = arr.rvalAt(s_elem_types);
      if (!elems.is_set() || !isVecOrArrayType(elems.type())) return false;
      return isValidResolvedTypeStructureList(ArrNR(elems.val().parr));
    }
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_trait:
    case TypeStructure::Kind::T_enum: {
      auto const clsname = arr.rvalAt(s_classname);
      if (!clsname.is_set() || !isStringType(clsname.type())) return false;
      auto const generics = arr.rvalAt(s_generic_types);
      if (generics.is_set()) {
        if (!isVecOrArrayType(generics.type())) return false;
        return isValidResolvedTypeStructureList(ArrNR(generics.val().parr));
      }
      return true;
    }
    case TypeStructure::Kind::T_unresolved:
    case TypeStructure::Kind::T_typeaccess:
    case TypeStructure::Kind::T_xhp:
    case TypeStructure::Kind::T_reifiedtype:
      // Kinds denoting unresolved types should not appear
      return false;
  }
  return false;
}

} // namespace HPHP

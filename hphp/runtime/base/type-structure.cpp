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
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/string-hash-set.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/bespoke/type-structure.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/util/text-util.h"

namespace HPHP {

struct String;
struct StaticString;

/*
 * NB: This resolution logic and HHBBC must always agree exactly. If
 * you change anything here, HHBBC must be changed to match.
 */

namespace {

/* TSEnv holds values that are propagated through the entire resolution process
 * and TSCtx holds values that change every time we traverse through a
 * T_UNRESOLVED during resolution. They are both bags of data that we need at
 * various points, so we package them up to make them easier to pass around. */
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
  // Set of type aliases or case types currently being resolved
  req::StringFastSet resolving;
};

struct TSCtx {
  // The name of the type constant or type alias we're resolving; will be null
  // when we're resolving an anonymous type structure.
  const StringData* name = nullptr;

  // If we're resolving a type constant, these are the class that the type
  // constant was requested from, and the class it is declared in, respectively.
  // You will either have neither or both of these.
  const Class* typeCnsCls = nullptr;
  const Class* declCls = nullptr;

  // Generics for the context we're resolving in, if available.
  const ArrayData* generics = nullptr;
};

/*
 * These static strings are the same as the ones in
 * hphp/compiler/type_annotation.cpp, where the typeAnnotArrays are
 * originally generated. See TypeAnnotation::getScalarArrayRep().
 */
const StaticString
  s_nullable("nullable"),
  s_exact("exact"),
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
  s_this(annotTypeName(AnnotType::This)),
  s_self("self"),
  s_parent("parent"),
  s_callable("callable"),
  s_alias("alias"),
  s_case_type("case_type"),
  s_typevars("typevars"),
  s_typevar_types("typevar_types"),
  s_id("id"),
  s_soft("soft"),
  s_opaque("opaque"),
  s_union_types("union_types")
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
  s_darray(kAnnotTypeDarrayStr),
  s_varray(kAnnotTypeVarrayStr),
  s_varray_or_darray(kAnnotTypeVarrayOrDarrayStr),
  s_shape("shape"),
  s_hh_vec(annotTypeName(AnnotType::Vec)),
  s_hh_dict(annotTypeName(AnnotType::Dict)),
  s_hh_keyset(annotTypeName(AnnotType::Keyset)),
  s_hh_vec_or_dict(annotTypeName(AnnotType::VecOrDict)),
  s_hh_any_array(annotTypeName(AnnotType::ArrayLike)),
  s_hh("HH\\")
;

std::string fullName(const Array& arr, TypeStructure::TSDisplayType type);

void functionTypeName(const Array& arr, std::string& name,
                      TypeStructure::TSDisplayType type) {
  name += "(function (";

  assertx(arr.exists(s_return_type));
  auto const retType = arr[s_return_type].asCArrRef();

  assertx(arr.exists(s_param_types));
  auto const params = arr[s_param_types].asCArrRef();

  auto sep = "";
  auto const sz = params.size();
  for (auto i = 0; i < sz; i++) {
    auto const param = params[i].asCArrRef();
    folly::toAppend(sep, fullName(param, type), &name);
    sep = ", ";
  }

  auto variadic_tv = arr.lookup(s_variadic_type);
  if (variadic_tv.is_init()) {
    auto const variadicType = tvAsVariant(variadic_tv).asCArrRef();
    folly::toAppend(sep, fullName(variadicType, type), "...", &name);
  }

  folly::toAppend("): ", fullName(retType, type), ")", &name);
}

void accessTypeName(const Array& arr, std::string& name) {
  assertx(arr.exists(s_root_name));
  auto const rootName = arr[s_root_name].asCStrRef();
  name += rootName.toCppString();

  assertx(arr.exists(s_access_list));
  auto const accessList = arr[s_access_list].asCArrRef();
  auto const sz = accessList.size();
  for (auto i = 0; i < sz; i++) {
    folly::toAppend("::",
                    accessList[i].asCStrRef().toCppString(),
                    &name);
  }
}

// xhp names are mangled so we get them back to their original definition
// see the mangling in ScannerToken::xhpLabel
void xhpTypeName(const Array& arr, std::string& name) {
  assertx(arr.exists(s_classname));
  std::string clsName = arr[s_classname].asCStrRef().toCppString();
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

void tupleTypeName(const Array& arr, std::string& name,
                   TypeStructure::TSDisplayType type) {
  name += "(";
  assertx(arr.exists(s_elem_types));
  auto const elems = arr[s_elem_types].asCArrRef();
  auto const sz = elems.size();
  auto sep = "";
  for (auto i = 0; i < sz; i++) {
    auto const elem = elems[i].asCArrRef();
    folly::toAppend(sep, fullName(elem, type), &name);
    sep = ", ";
  }

  name += ")";
}

void genericTypeName(const Array& arr, std::string& name,
                     TypeStructure::TSDisplayType type) {
  auto args_tv = arr.lookup(s_generic_types);
  if (!args_tv.is_init()) return;
  name += "<";
  auto const args = tvAsVariant(args_tv).asCArrRef();
  auto const sz = args.size();
  auto sep = "";
  for (auto i = 0; i < sz; i++) {
    auto const arg = args[i].asCArrRef();
    folly::toAppend(sep, fullName(arg, type), &name);
    sep = ", ";
  }
  name += ">";
}

void unionTypeName(const Array& arr, std::string& name,
                   TypeStructure::TSDisplayType type) {
  auto union_tv = arr.lookup(s_union_types);
  if (!union_tv.is_init()) return;
  auto const unions = tvAsVariant(union_tv).asCArrRef();
  auto const sz = unions.size();
  auto sep = "";
  for (auto i = 0; i < sz; i++) {
    auto const ty = unions[i].asCArrRef();
    folly::toAppend(sep, fullName(ty, type), &name);
    sep = " | ";
  }
}

bool forDisplay(TypeStructure::TSDisplayType type) {
  return type != TypeStructure::TSDisplayType::TSDisplayTypeReflection;
}

void shapeTypeName(const Array& arr, std::string& name,
                   TypeStructure::TSDisplayType type) {
  // works for both resolved and unresolved TypeStructures
  name += "(";
  assertx(arr.exists(s_fields));
  auto const fields = arr[s_fields].asCArrRef();
  auto const sz = fields.size();
  auto sep = "";
  for (auto i = 0; i < sz; i++) {
    name += sep;
    auto const field = fields->getKey(i);
    auto value = fields->getValue(i).asCArrRef();
    auto quote = "'";
    if (value.exists(s_optional_shape_field)) {
      name += "?";
    }
    if (value.exists(s_value)) {
      // if unresolved, ignore wrapper
      if (value.exists(s_is_cls_cns)) quote = "";
      value = value[s_value].asCArrRef();
    }
    auto const fieldType = field.getType();
    if (isStringType(fieldType)) {
      folly::toAppend(quote, field.asCStrRef().data(), quote, &name);
    } else if (isIntType(fieldType)) {
      folly::toAppend(field.toInt64Val(), &name);
    }

    folly::toAppend(
      forDisplay(type) ? " => " : "=>",
      fullName(value, type),
      &name
    );
    sep = ", ";
  }

  if (arr.exists(s_allows_unknown_fields)) {
    folly::toAppend(sep, "...", &name);
  }

  name += ")";
}

std::string fullName(const Array& arr, TypeStructure::TSDisplayType type) {
  std::string name;

  if (arr.exists(s_nullable)) {
    assertx(arr[s_nullable].toBoolean());
    name += '?';
  }

  if (arr.exists(s_soft)) {
    assertx(arr[s_soft].toBoolean());
    name += '@';
  }

  if (type == TypeStructure::TSDisplayType::TSDisplayTypeInternal) {
    if (arr.exists(s_opaque)) {
      assertx(arr[s_opaque].toBoolean());
      name += "opaque:";
    }
    auto tv = arr.lookup(s_alias);
    if (!tv.is_init()) tv = arr.lookup(s_case_type);
    if (tv.is_init()) {
      auto const alias = tvAsVariant(tv).asCStrRef();
      name += alias.toCppString() + ':';
    }

    tv = arr.lookup(s_typevar_types);
    if (tv.is_init()) {
      name += "typevar_types[";
      auto const types = tvAsVariant(tv).asCArrRef();
      bool first = true;
      for (ArrayIter iter(types); iter; ++iter, first=false) {
        auto key = iter.first();
        auto value = iter.second();
        if (!first) name += ",";
        name += key.asCStrRef().toCppString() + ":" + fullName(value.asCArrRef(), type);
      }
      name += "]:";
    }
  }

  assertx(arr.exists(s_kind));

  TypeStructure::Kind kind =
    TypeStructure::Kind(arr[s_kind].toInt64Val());
  switch (kind) {
    case TypeStructure::Kind::T_null:
      name += forDisplay(type) ? s_null : s_hh + s_null;
      break;
    case TypeStructure::Kind::T_void:
      name += forDisplay(type) ? s_void : s_hh + s_void;
      break;
    case TypeStructure::Kind::T_int:
      name += forDisplay(type) ? s_int : s_hh + s_int;
      break;
    case TypeStructure::Kind::T_bool:
      name += forDisplay(type) ? s_bool : s_hh + s_bool;
      break;
    case TypeStructure::Kind::T_float:
      name += forDisplay(type) ? s_float : s_hh + s_float;
      break;
    case TypeStructure::Kind::T_string:
      name += forDisplay(type) ? s_string : s_hh + s_string;
      break;
    case TypeStructure::Kind::T_resource:
      name += forDisplay(type) ? s_resource : s_hh + s_resource;
      break;
    case TypeStructure::Kind::T_num:
      name += forDisplay(type) ? s_num : s_hh + s_num;
      break;
    case TypeStructure::Kind::T_arraykey:
      name += forDisplay(type) ? s_arraykey : s_hh + s_arraykey;
      break;
    case TypeStructure::Kind::T_nothing:
      name += forDisplay(type) ? s_nothing : s_hh + s_nothing;
      break;
    case TypeStructure::Kind::T_noreturn:
      name += forDisplay(type) ? s_noreturn : s_hh + s_noreturn;
      break;
    case TypeStructure::Kind::T_mixed:
      name += forDisplay(type) ? s_mixed : s_hh + s_mixed;
      break;
    case TypeStructure::Kind::T_dynamic:
      name += forDisplay(type) ? s_dynamic : s_hh + s_dynamic;
      break;
    case TypeStructure::Kind::T_nonnull:
      name += forDisplay(type) ? s_nonnull : s_hh + s_nonnull;
      break;
    case TypeStructure::Kind::T_tuple:
      tupleTypeName(arr, name, type);
      break;
    case TypeStructure::Kind::T_fun:
      functionTypeName(arr, name, type);
      break;
    case TypeStructure::Kind::T_darray:
      name += s_darray;
      genericTypeName(arr, name, type);
      break;
    case TypeStructure::Kind::T_varray:
      name += s_varray;
      genericTypeName(arr, name, type);
      break;
    case TypeStructure::Kind::T_varray_or_darray:
      name += s_varray_or_darray;
      genericTypeName(arr, name, type);
      break;
    case TypeStructure::Kind::T_shape:
      name += forDisplay(type) ? s_shape : s_hh + s_shape;
      shapeTypeName(arr, name, type);
      break;
    case TypeStructure::Kind::T_dict:
      name += s_hh_dict;
      genericTypeName(arr, name, type);
      break;
    case TypeStructure::Kind::T_vec:
      name += s_hh_vec;
      genericTypeName(arr, name, type);
      break;
    case TypeStructure::Kind::T_keyset:
      name += s_hh_keyset;
      genericTypeName(arr, name, type);
      break;
    case TypeStructure::Kind::T_vec_or_dict:
      name += s_hh_vec_or_dict;
      genericTypeName(arr, name, type);
      break;
    case TypeStructure::Kind::T_any_array:
      name += s_hh_any_array;
      genericTypeName(arr, name, type);
      break;
    case TypeStructure::Kind::T_typevar:
      assertx(arr.exists(s_name));
      name += arr[s_name].asCStrRef().toCppString();
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
      name += arr[s_id].asCStrRef().data();
      break;
    case TypeStructure::Kind::T_union:
      assertx(arr.exists(s_union_types));
      unionTypeName(arr, name, type);
      break;
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_trait:
    case TypeStructure::Kind::T_enum:
    case TypeStructure::Kind::T_unresolved:
      assertx(arr.exists(s_classname));
      name += arr[s_classname].asCStrRef().toCppString();
      genericTypeName(arr, name, type);
      break;
  }

  return name;
}

Array resolveTS(TSEnv& env, const TSCtx& ctx, const Array& arr,
                ArrayData* typevarTypes);

Array resolveTSImpl(TSEnv& env, const TSCtx& ctx, const Array& arr);

Array maybeMakeBespoke(Array& ts) {
  if (allowBespokeArrayLikes() &&
      RO::EvalEmitBespokeTypeStructures &&
      bespoke::TypeStructure::isValidTypeStructure(ts.get())) {
    auto const bespokeTS = bespoke::TypeStructure::MakeFromVanilla(ts.get());
    if (bespokeTS) return Array::attach(bespokeTS);
  }
  return ts;
}

Array resolveList(TSEnv& env, const TSCtx& ctx, const Array& arr) {
  auto const sz = arr.size();

  VecInit newarr(sz);
  for (auto i = 0; i < sz; i++) {
    newarr.append(Variant(resolveTS(env, ctx, arr[i].toArray(), nullptr)));
  }

  return newarr.toArray();
}

std::string resolveContextMsg(const TSCtx& ctx) {
  std::string msg("when resolving ");
  if (!ctx.name) {
    folly::toAppend("anonymous type structure", &msg);
  } else if (ctx.typeCnsCls) {
    folly::toAppend("type constant ", ctx.typeCnsCls->name()->data(),
                    "::", ctx.name->data(), &msg);
  } else {
    folly::toAppend("type alias ", ctx.name->data(), &msg);
  }
  return msg;
}

/*
 * Returns the type-structure for the alias with the given name. If
 * the name is not an alias, return an empty Array. The bool is
 * whether the returned type-structure is already resolved or not.
*/
std::pair<Array, bool> getAlias(TSEnv& env, const String& aliasName,
                                bool usePreResolved) {

  if (aliasName.same(s_this) || Class::lookup(aliasName.get())) {
    return std::make_pair(Array::CreateDict(), false);
  }

  if (env.resolving.contains(aliasName)) {
    // The alias is currently being resolved and we are hitting a recursion.
    // give error
    throw Exception(
      "Alias %s is being used recursively",
      aliasName.data());
  }

  auto persistentTA = true;
  auto typeAlias = env.allow_partial
    ? TypeAlias::lookup(aliasName.get(), &persistentTA)
    : TypeAlias::load(aliasName.get(), &persistentTA);
  if (!typeAlias) {
    env.partial = true;
    return std::make_pair(Array::CreateDict(), false);
  }

  env.persistent &= persistentTA;

  if (usePreResolved) {
    auto const& preresolved = typeAlias->resolvedTypeStructure();
    if (!preresolved.isNull()) {
      assertx(preresolved.isDict());
      assertx(!preresolved.empty());
      return std::make_pair(preresolved, true);
    }
  }

  // this returned type structure is unresolved.
  assertx(typeAlias->typeStructure().isDict());
  assertx(!typeAlias->typeStructure().empty());
  return std::make_pair(typeAlias->typeStructure(), false);
}

const Class* getClass(TSEnv& env, const TSCtx& ctx, const String& clsName) {
  auto checkPersistent = [&](const Class* cls) {
    env.persistent &= classHasPersistentRDS(cls);
    return cls;
  };

  // the original unresolved type structure came from a type constant
  // (instead of a type alias), and may have this/self/parent.
  if (ctx.typeCnsCls) {
    // HH\this: late static binding
    if (clsName.same(s_this)) {
      return checkPersistent(ctx.typeCnsCls);
    }

    // self
    if (clsName.same(s_self)) {
      return checkPersistent(ctx.declCls);
    }
    // parent
    if (clsName.same(s_parent)) {
      auto parent = ctx.declCls->parent();
      if (!parent) {
        throw Exception(
          "%s, class %s does not have a parent",
          resolveContextMsg(ctx).c_str(),
          ctx.declCls->name()->data());
      }
      return checkPersistent(parent);
    }
  }

  auto name = clsName;
  // We can't use preresolved aliases here in order to keep the error
  // message the same.
  auto ts = getAlias(env, name, false).first;
  while (!ts.empty()) {
    assertx(ts.exists(s_kind));
    if (!ts.exists(s_classname)) {
      // not a class, interface, trait, enum, or alias
      throw Exception(
        "%s, alias %s does not resolve to a class",
        resolveContextMsg(ctx).c_str(),
        name.data());
    }
    name = ts[s_classname].asCStrRef();
    ts = getAlias(env, name, false).first;
  }

  auto const cls = env.allow_partial ? Class::lookup(name.get())
                                     : Class::load(name.get());
  if (!cls) {
    env.partial = true;
    if (env.allow_partial) return nullptr;
    throw Exception(
      "%s, class %s not found",
      resolveContextMsg(ctx).c_str(),
      name.data());
  }

  return checkPersistent(cls);
}

/* Given an unresolved T_shape TypeStructure, returns the __fields__
 * portion of the array with all the field names resolved to string
 * literals. */
Array resolveShape(TSEnv& env, const TSCtx& ctx, const Array& arr) {
  assertx(arr.exists(s_kind));
  assertx(static_cast<TypeStructure::Kind>(arr[s_kind].toInt64Val())
         == TypeStructure::Kind::T_shape);
  assertx(arr.exists(s_fields));

  auto newfields = Array::CreateDict();
  auto const fields = arr[s_fields].asCArrRef();
  auto const sz = fields.size();
  for (auto i = 0; i < sz; i++) {
    Variant key = fields->getKey(i);
    auto const wrapper = fields->getValue(i).asCArrRef();
    if (wrapper.exists(s_is_cls_cns)) {
      // if the shape field name is a class constant, its name is
      // double colon delimited clsName::cnsName
      auto const clsCns = key.asCStrRef().toCppString();
      std::string clsName, cnsName;
      folly::split("::", clsCns, clsName, cnsName);

      // look up clsName::cnsName
      auto cls = getClass(env, ctx, String(clsName));
      if (!cls) throw Exception("failed to resolve shape classname");
      auto cnsValue = tvClassToString(cls->clsCnsGet(String(cnsName).get()));

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

    auto tv = wrapper.lookup(s_value);
    auto valueArr = tv.is_init() ? tvAsVariant(tv).toArray() : wrapper;
    auto value = resolveTSImpl(env, ctx, valueArr);

    if (wrapper.exists(s_optional_shape_field)) {
      value.set(s_optional_shape_field, make_tv<KindOfBoolean>(true));
    }

    newfields.set(key, Variant(maybeMakeBespoke(value)));
  }

  return newfields;
}

bool resolveClass(TSEnv& env, const TSCtx& ctx, Array& ret,
                  const String& clsName) {
  auto const cls = getClass(env, ctx, clsName);
  if (!cls) return false;

  TypeStructure::Kind resolvedKind;
  if (isNormalClass(cls) || isEnumClass(cls)) {
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

Array resolveGenerics(TSEnv& env, const TSCtx& ctx, const Array& arr) {
  return resolveList(env, ctx, arr[s_generic_types].toArray());
}

Array resolveUnion(TSEnv& env, const TSCtx& ctx, const Array& arr) {
  return resolveList(env, ctx, arr[s_union_types].toArray());
}

/**
 * Copy modifiers, i.e. whether the type is nullable, soft, or a like-type.
 */
void copyTypeModifiers(const Array& from, Array& to) {
  if (from.exists(s_nullable)) to.set(s_nullable, make_tv<KindOfBoolean>(true));
  if (from.exists(s_soft))     to.set(s_soft, make_tv<KindOfBoolean>(true));
}

Array resolveTSImpl(TSEnv& env, const TSCtx& ctx, const Array& arr) {
  assertx(arr.exists(s_kind));
  auto const kind = static_cast<TypeStructure::Kind>(
    arr[s_kind].toInt64Val());

  auto newarr = Array::CreateDict();
  copyTypeModifiers(arr, newarr);
  newarr.set(s_kind, Variant(static_cast<uint8_t>(kind)));

  if (arr.exists(s_allows_unknown_fields)) {
    newarr.set(s_allows_unknown_fields, make_tv<KindOfBoolean>(true));
  }

  switch (kind) {
    case TypeStructure::Kind::T_tuple: {
      assertx(arr.exists(s_elem_types));
      auto const elemsArr = arr[s_elem_types].asCArrRef();
      newarr.set(s_elem_types, Variant(resolveList(env, ctx, elemsArr)));
      break;
    }
    case TypeStructure::Kind::T_fun: {
      env.invalidType = true;
      assertx(arr.exists(s_return_type));
      auto const returnArr = arr[s_return_type].asCArrRef();
      newarr.set(s_return_type, Variant(resolveTS(env, ctx, returnArr, nullptr)));

      assertx(arr.exists(s_param_types));
      auto const paramsArr = arr[s_param_types].asCArrRef();
      newarr.set(s_param_types, Variant(resolveList(env, ctx, paramsArr)));

      auto tv = arr.lookup(s_variadic_type);
      if (tv.is_init()) {
        newarr.set(s_variadic_type, Variant(resolveTS(env, ctx, tvAsVariant(tv).asCArrRef(), nullptr)));
      }

      break;
    }
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
    case TypeStructure::Kind::T_dict:
    case TypeStructure::Kind::T_vec:
    case TypeStructure::Kind::T_keyset:
    case TypeStructure::Kind::T_vec_or_dict:
    case TypeStructure::Kind::T_any_array: {
      if (kind == TypeStructure::Kind::T_darray ||
        kind == TypeStructure::Kind::T_varray ||
        kind == TypeStructure::Kind::T_varray_or_darray
      ) {
        env.invalidType = true;
      }
      if (arr.exists(s_generic_types)) {
        newarr.set(s_generic_types, Variant(resolveGenerics(env, ctx, arr)));
      }
      break;
    }
    case TypeStructure::Kind::T_shape: {
      newarr.set(s_fields, Variant(resolveShape(env, ctx, arr)));
      break;
    }
    case TypeStructure::Kind::T_unresolved: {
      assertx(arr.exists(s_classname));
      auto const clsName = arr[s_classname].asCStrRef();
      auto tsAndPreresolved = getAlias(env, clsName, true);
      auto ts = tsAndPreresolved.first;
      auto const preresolved = tsAndPreresolved.second;

      if (!ts.empty()) {
        auto resolveAlias = [&] (ArrayData* generics) {
          // If it's already resolved, don't do so again.
          if (preresolved) return ts;
          TSCtx newCtx;
          newCtx.name = clsName.get();
          newCtx.generics = generics;
          auto const [_, inserted] = env.resolving.insert(clsName);
          always_assert(inserted);
          auto resolved = resolveTS(env, newCtx, ts, generics);
          assertx(!resolved.isNull());
          env.resolving.erase(clsName);
          return resolved;
        };

        if (ts.exists(s_typevars) && arr.exists(s_generic_types)) {
          std::vector<std::string> typevars;
          folly::split(',', ts[s_typevars].asCStrRef().data(), typevars);
          ts.remove(s_typevars);

          auto generic_types = resolveGenerics(env, ctx, arr);

          auto const sz = std::min(static_cast<ssize_t>(typevars.size()),
                                   generic_types.size());
          DictInit newarr(sz);
          for (auto i = 0; i < sz; i++) {
            newarr.set(String(typevars[i]), generic_types[i]);
          }
          auto generics = newarr.toArray();
          ts = resolveAlias(generics.get());
        } else {
          ts = resolveAlias(nullptr);
        }
        copyTypeModifiers(arr, ts);
        newarr = ts;
        break;
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
      if (!resolveClass(env, ctx, newarr, clsName) && env.allow_partial) {
        env.partial = true;
        newarr.set(s_kind,
                   Variant(static_cast<uint8_t>(
                           TypeStructure::Kind::T_unresolved)));
        newarr.set(s_classname, Variant(clsName));
        break;
      }
      if (arr.exists(s_generic_types)) {
        newarr.set(s_generic_types, Variant(resolveGenerics(env, ctx, arr)));
      }
      break;
    }
    case TypeStructure::Kind::T_typeaccess: {
      /* type access is a root class (may be HH\this) followed by a series of
       * type constants, i.e., cls::TC1::TC2::TC3. Each type constant other
       * than the last one in the chain must refer to a class or an
       * interface. */
      assertx(arr.exists(s_root_name));
      auto clsName = arr[s_root_name].asCStrRef();
      assertx(arr.exists(s_access_list));
      auto const accList = arr[s_access_list].asCArrRef();
      auto const sz = accList.size();
      Array typeCnsVal;
      for (auto i = 0; i < sz; i++) {
        auto const cls = getClass(env, ctx, clsName);
        if (!cls) throw Exception("failed to resolve type access");
        auto const cnsName = accList[i].asCStrRef();
        if (!cls->hasTypeConstant(cnsName.get())) {
          throw Exception(
            "%s, class %s does not have non-abstract "
            "type constant %s",
            resolveContextMsg(ctx).c_str(),
            clsName.data(),
            cnsName.data());
        }
        auto tv = cls->clsCnsGet(cnsName.get(), ConstModifiers::Kind::Type,
                                 !env.allow_partial);
        if (!tv.is_init()) {
          assertx(env.allow_partial);
          throw Exception(
            "Failed to resolve a type constant: %s::%s",
            cls->name()->data(), cnsName.get()->data()
          );
        }
        assertx(isArrayLikeType(tv.m_type));
        typeCnsVal = Array(tv.m_data.parr);
        assertx(typeCnsVal.isDict());
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
            resolveContextMsg(ctx).c_str(),
            clsName.data(),
            cnsName.data(),
            accList[i+1].asCStrRef().data());
        }
        assertx(typeCnsVal.exists(s_classname));
        clsName = typeCnsVal[s_classname].asCStrRef();
      }
      copyTypeModifiers(arr, typeCnsVal);
      newarr = typeCnsVal;
      break;
    }
    case TypeStructure::Kind::T_typevar: {
      env.invalidType = true;
      if (!ctx.generics) return arr.toDict();
      assertx(arr.exists(s_name));
      auto const generic = ctx.generics->get(arr[s_name].asCStrRef().get());
      if (!generic.is_init()) return arr.toDict();
      newarr = Variant::wrap(generic).toDict();
      break;
    }
    case TypeStructure::Kind::T_reifiedtype: {
      assertx(env.tsList != nullptr);
      assertx(arr.exists(s_id));
      auto id = arr.lookup(s_id).val().num;
      assertx(id < env.tsList->size());
      // Overwrite data in newarr with fields in the reified generics type.
      IterateKV(env.tsList->at(id).get(), [&](auto key, auto val) {
        newarr.set(key, val);
      });
      break;
    }
    case TypeStructure::Kind::T_union:
      assertx(arr.exists(s_union_types));
      newarr.set(s_union_types, Variant(resolveUnion(env, ctx, arr)));
      break;

    case TypeStructure::Kind::T_xhp:
    case TypeStructure::Kind::T_void:
    case TypeStructure::Kind::T_int:
    case TypeStructure::Kind::T_bool:
    case TypeStructure::Kind::T_float:
    case TypeStructure::Kind::T_string:
    case TypeStructure::Kind::T_resource:
    case TypeStructure::Kind::T_num:
    case TypeStructure::Kind::T_arraykey:
    case TypeStructure::Kind::T_noreturn:
    case TypeStructure::Kind::T_mixed:
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_trait:
    case TypeStructure::Kind::T_enum:
    case TypeStructure::Kind::T_nonnull:
    case TypeStructure::Kind::T_null:
    case TypeStructure::Kind::T_nothing:
    case TypeStructure::Kind::T_dynamic:
      // Return the original type structure, no resolution needed
      return arr.toDict();
  }

  if (arr.exists(s_typevars)) newarr.set(s_typevars, arr[s_typevars]);
  if (arr.exists(s_alias)) {
    newarr.set(s_alias, Variant{arr[s_alias].asCStrRef()});
  }
  if (arr.exists(s_case_type)) {
    newarr.set(s_case_type, Variant{arr[s_case_type].asCStrRef()});
  }

  return newarr;
}

Array resolveTS(TSEnv& env, const TSCtx& ctx, const Array& arr,
                ArrayData* typevarTypes) {
  auto ts = resolveTSImpl(env, ctx, arr);
  if (typevarTypes) ts.set(s_typevar_types, Variant(typevarTypes));

  return maybeMakeBespoke(ts);
}

} // anonymous namespace

std::string xhpNameFromTS(const Array& arr) {
  std::string name;
  xhpTypeName(arr, name);
  return name;
}

String TypeStructure::toString(const Array& arr, TSDisplayType type) {
  if (arr.empty()) return String();
  return String(fullName(arr, type));
}

/*
 * Constructs a scalar array with all the shape field names, this/self/parent,
 * classes, type accesses, and type aliases resolved.
 */
Array TypeStructure::resolve(const ArrayData* ts,
                             const StringData* clsName,
                             const Class* declCls,
                             const Class* typeCnsCls,
                             bool& persistent) {
  TSEnv env;
  TSCtx ctx;
  ctx.name = clsName;
  ctx.declCls = declCls;
  ctx.typeCnsCls = typeCnsCls;
  auto resolved = resolveTS(env, ctx, ArrNR(ts), nullptr);
  persistent = env.persistent;
  return resolved;
}

/*
 * Called by TypeAlias to get resolved TypeStructure for type aliases.
 */
Array TypeStructure::resolve(const String& aliasName,
                             const Array& arr,
                             bool& persistent,
                             const Array& generics) {
  TSEnv env;
  env.resolving.insert(aliasName);
  TSCtx ctx;
  ctx.name = aliasName.get();
  ctx.generics = generics.get();
  auto resolved = resolveTS(env, ctx, arr, nullptr);
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
  TSEnv env;
  env.tsList = &tsList;
  TSCtx ctx;
  ctx.declCls = declCls;
  ctx.typeCnsCls = typeCnsCls;
  auto resolved = resolveTS(env, ctx, ts, nullptr);
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
  TSEnv env;
  env.allow_partial = true;
  TSCtx ctx;
  ctx.declCls = declCls;
  ctx.typeCnsCls = typeCnsCls;
  auto resolved = resolveTS(env, ctx, ts, nullptr);
  persistent = env.persistent;
  partial = env.partial;
  invalidType = env.invalidType;
  return resolved;
}

namespace {

// Coerces vector-like darrays / dicts to varrays / vecs. Returns true if the
// lval is now a varray (either coerced, or if it started off that way).
bool coerceToVecOrVArray(tv_lval lval) {
  if (tvIsVec(lval)) return true;

  // Must be a dict or darray with vector-like data.
  if (!tvIsDict(lval)) return false;
  auto const ad = val(lval).parr;
  if (!ad->isVectorData()) return false;

  // Do the coercion and replace the value.
  auto const varray = ad->toVec(/*copy=*/true);
  tvCopy(make_array_like_tv(varray), lval);
  assertx(ad != varray);
  decRefArr(ad);
  return true;
}

bool coerceToTypeStructure(Array& arr);
bool coerceToTypeStructureList(Array& arr, bool shape=false);

// Returns true and performs coercion if the given field of `arr` can be
// coerced to a valid, resolved TypeStructure.
bool coerceTSField(Array& arr, const String& name) {
  assertx(!arr->cowCheck());
  auto field = arr.lookup(name);
  if (!tvIsDict(field)) return false;
  return coerceToTypeStructure(ArrNR(val(field).parr).asArray());
}

// Returns true and performs coercion if the given field of `arr` can be
// coerced to a list of valid, resolved TypeStructures.
bool coerceTSListField(Array& arr, const String& name, bool shape=false) {
  assertx(!arr->cowCheck());
  if (!arr.exists(name)) return false;
  auto field = arr.lval(name);
  if (!coerceToVecOrVArray(field)) return false;
  assertx(tvIsVec(field));
  return coerceToTypeStructureList(ArrNR(val(field).parr).asArray(), shape);
}

// Same as above, except that they allow the field to be missing.
bool coerceOptTSField(Array& arr, const String& name) {
  assertx(!arr->cowCheck());
  auto field = arr.lookup(name);
  if (!field.is_init()) return true;
  if (!tvIsDict(field)) return false;
  return coerceToTypeStructure(ArrNR(val(field).parr).asArray());
}
bool coerceOptTSListField(Array& arr, const String& name, bool shape=false) {
  assertx(!arr->cowCheck());
  if (!arr.exists(name)) return true;
  auto field = arr.lval(name);
  if (!coerceToVecOrVArray(field)) return false;
  assertx(tvIsVec(field));
  return coerceToTypeStructureList(ArrNR(val(field).parr).asArray(), shape);
}

bool coerceToTypeStructureList(Array& arr, bool shape) {
  assertx(arr->empty() || !arr->cowCheck());
  if (!arr->isVecType()) return false;

  auto valid = true;
  IterateV(arr.get(), [&](TypedValue tv) {
    if (!tvIsDict(tv)) {
      valid = false;
      return true;
    }
    auto const ad = [&] {
      if (shape) {
        auto const value = arr.lookup(s_value);
        if (tvIsDict(value)) return val(value).parr;
        if (value.is_init()) valid = false;
      }
      return val(tv).parr;
    }();
    valid = valid && coerceToTypeStructure(ArrNR(ad).asArray());
    return !valid;
  });
  return valid;
}

bool coerceToTypeStructure(Array& arr) {
  assertx(arr->empty() || !arr->cowCheck());
  if (!arr->isDictType()) return false;

  auto const kindfield = arr.lookup(s_kind);
  if (!isIntType(kindfield.type()) ||
      kindfield.val().num > TypeStructure::kMaxResolvedKind) {
    return false;
  }
  auto const kind = static_cast<TypeStructure::Kind>(kindfield.val().num);
  switch (kind) {
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
    case TypeStructure::Kind::T_dict:
    case TypeStructure::Kind::T_vec:
    case TypeStructure::Kind::T_keyset:
    case TypeStructure::Kind::T_vec_or_dict:
    case TypeStructure::Kind::T_any_array:
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
    case TypeStructure::Kind::T_nonnull: {
      return true;
    }
    case TypeStructure::Kind::T_fun: {
      return coerceTSField(arr, s_return_type) &&
             coerceTSListField(arr, s_param_types) &&
             coerceOptTSField(arr, s_variadic_type);
    }
    case TypeStructure::Kind::T_typevar: {
      return tvIsString(arr.lookup(s_name));
    }
    case TypeStructure::Kind::T_shape: {
      return coerceTSListField(arr, s_fields, /*shape=*/true);
    }
    case TypeStructure::Kind::T_tuple: {
      return coerceTSListField(arr, s_elem_types);
    }
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_trait:
    case TypeStructure::Kind::T_enum: {
      return tvIsString(arr.lookup(s_classname)) &&
             coerceOptTSListField(arr, s_generic_types);
    }
    case TypeStructure::Kind::T_union: {
      return coerceTSListField(arr, s_union_types, /*shape=*/false);
    }
    case TypeStructure::Kind::T_unresolved:
    case TypeStructure::Kind::T_typeaccess:
    case TypeStructure::Kind::T_xhp:
    case TypeStructure::Kind::T_reifiedtype: {
      // Unresolved types should not appear in valid resolved type structures.
      return false;
    }
  }
  return false;
}

}

bool TypeStructure::coerceToTypeStructureList_SERDE_ONLY(tv_lval lval) {
  return coerceToVecOrVArray(lval) &&
         coerceToTypeStructureList(ArrNR(val(lval).parr).asArray());
}

} // namespace HPHP

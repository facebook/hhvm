/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

/*
 * These static strings are the same as the ones in
 * hphp/compiler/type_annotation.cpp, where the typeAnnotArrays are
 * originally generated. See TypeAnnotation::getScalarArrayRep().
 */
const StaticString
  s_nullable("nullable"),
  s_name("name"),
  s_classname("classname"),
  s_kind("kind"),
  s_elem_types("elem_types"),
  s_return_type("return_type"),
  s_param_types("param_types"),
  s_generic_types("generic_types"),
  s_root_name("root_name"),
  s_access_list("access_list"),
  s_fields("fields"),
  s_is_cls_cns("is_cls_cns"),
  s_value("value"),
  s_this("HH\\this"),
  s_self("self"),
  s_parent("parent"),
  s_callable("callable"),
  s_alias("alias"),
  s_typevars("typevars"),
  s_typevar_types("typevar_types")
;

const std::string
  s_void("HH\\void"),
  s_int("HH\\int"),
  s_bool("HH\\bool"),
  s_float("HH\\float"),
  s_string("HH\\string"),
  s_resource("HH\\resource"),
  s_num("HH\\num"),
  s_arraykey("HH\\arraykey"),
  s_noreturn("HH\\noreturn"),
  s_mixed("HH\\mixed"),
  s_array("array"),
  s_shape("HH\\shape")
;

std::string fullName(const Array& arr);

void functionTypeName(const Array& arr, std::string& name) {
  name += "(function (";

  assert(arr.exists(s_return_type));
  auto const retType = arr[s_return_type].toCArrRef();

  assert(arr.exists(s_param_types));
  auto const params = arr[s_param_types].toCArrRef();

  auto sep = "";
  auto const sz = params.size();
  for (auto i = 0; i < sz; i++) {
    auto const param = params[i].toCArrRef();
    folly::toAppend(sep, fullName(param), &name);
    sep = ", ";
  }

  // add funciton return type
  folly::toAppend("): ", fullName(retType), ")", &name);
}

void accessTypeName(const Array& arr, std::string& name) {
  assert(arr.exists(s_root_name));
  auto const rootName = arr[s_root_name].toCStrRef();
  name += rootName.toCppString();

  assert(arr.exists(s_access_list));
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
  assert(arr.exists(s_classname));
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

void tupleTypeName(const Array& arr, std::string& name) {
  name += "(";
  assert(arr.exists(s_elem_types));
  auto const elems = arr[s_elem_types].toCArrRef();
  auto const sz = elems.size();
  auto sep = "";
  for (auto i = 0; i < sz; i++) {
    auto const elem = elems[i].toCArrRef();
    folly::toAppend(sep, fullName(elem), &name);
    sep = ", ";
  }

  name += ")";
}

void genericTypeName(const Array& arr, std::string& name) {
  name += "<";
  assert(arr.exists(s_generic_types));
  auto const args = arr[s_generic_types].toCArrRef();
  auto const sz = args.size();
  auto sep = "";
  for (auto i = 0; i < sz; i++) {
    auto const arg = args[i].toCArrRef();
    folly::toAppend(sep, fullName(arg), &name);
    sep = ", ";
  }
  name += ">";
}

void shapeTypeName(const Array& arr, std::string& name) {
  // works for both resolved and unresolved TypeStructures
  name += "(";
  assert(arr.exists(s_fields));
  auto const fields = arr[s_fields].toCArrRef();
  auto const sz = fields.size();
  auto sep = "";
  for (auto i = 0; i < sz; i++) {
    name += sep;
    auto const field = fields->getKey(i);
    auto value = fields->getValue(i).toCArrRef();
    auto quote = "'";
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

    folly::toAppend("=>", fullName(value), &name);
    sep = ", ";
  }

  name += ")";
}

std::string fullName(const Array& arr) {
  std::string name;
  if (arr.exists(s_nullable)) {
    assert(arr[s_nullable].toBoolean());
    name += '?';
  }

  assert(arr.exists(s_kind));

  TypeStructure::Kind kind =
    TypeStructure::Kind(arr[s_kind].toInt64Val());
  switch (kind) {
    case TypeStructure::Kind::T_void:
      name += s_void;
      break;
    case TypeStructure::Kind::T_int:
      name += s_int;
      break;
    case TypeStructure::Kind::T_bool:
      name += s_bool;
      break;
    case TypeStructure::Kind::T_float:
      name += s_float;
      break;
    case TypeStructure::Kind::T_string:
      name += s_string;
      break;
    case TypeStructure::Kind::T_resource:
      name += s_resource;
      break;
    case TypeStructure::Kind::T_num:
      name += s_num;
      break;
    case TypeStructure::Kind::T_arraykey:
      name += s_arraykey;
      break;
    case TypeStructure::Kind::T_noreturn:
      name += s_noreturn;
      break;
    case TypeStructure::Kind::T_mixed:
      name += s_mixed;
      break;
    case TypeStructure::Kind::T_tuple:
      tupleTypeName(arr, name);
      break;
    case TypeStructure::Kind::T_fun:
      functionTypeName(arr, name);
      break;
    case TypeStructure::Kind::T_array:
      name += s_array;
      if (arr.exists(s_generic_types)) {
        genericTypeName(arr, name);
      }
      break;
    case TypeStructure::Kind::T_shape:
      name += s_shape;
      shapeTypeName(arr, name);
      break;
    case TypeStructure::Kind::T_typevar:
      assert(arr.exists(s_name));
      name += arr[s_name].toCStrRef().toCppString();
      break;
    case TypeStructure::Kind::T_typeaccess:
      accessTypeName(arr, name);
      break;
    case TypeStructure::Kind::T_xhp:
      xhpTypeName(arr, name);
      break;
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_trait:
    case TypeStructure::Kind::T_enum:
    case TypeStructure::Kind::T_unresolved:
      assert(arr.exists(s_classname));
      name += arr[s_classname].toCStrRef().toCppString();
      if (arr.exists(s_generic_types)) genericTypeName(arr, name);
      break;
  }

  return name;
}

Array resolveTS(const Array& arr,
                const Class::Const& typeCns,
                const Class* typeCnsCls,
                const Array& generics,
                bool& persistent);

Array resolveList(const Array& arr,
                  const Class::Const& typeCns,
                  const Class* typeCnsCls,
                  const Array& generics,
                  bool& persistent) {
  auto const sz = arr.size();

  PackedArrayInit newarr(sz);
  for (auto i = 0; i < sz; i++) {
    auto elemArr = arr[i].toArray();
    auto elem = resolveTS(elemArr, typeCns, typeCnsCls, generics, persistent);
    newarr.append(Variant(elem));
  }

  auto ret = newarr.toArray();
  return ret;
}

std::string resolveContextMsg(const Class::Const& typeCns,
                              const Class* typeCnsCls) {
  std::string msg("when resolving ");
  if (typeCnsCls) {
    folly::toAppend("type constant ", typeCnsCls->name()->data(),
                    "::", typeCns.name->data(),
                    &msg);
  } else {
    folly::toAppend("type alias ", typeCns.name->data(), &msg);
  }
  return msg;
}

/* returns the unresolved TypeStructure; if aliasName is not an alias,
 * return nullptr. */
Array getAlias(const String& aliasName) {
  if (aliasName.same(s_this) || Unit::lookupClass(aliasName.get())) {
    return Array::Create();
  }
  auto typeAliasReq = Unit::loadTypeAlias(aliasName.get());
  if (!typeAliasReq) return Array::Create();

  // this returned type structure is unresolved.
  return typeAliasReq->typeStructure;
}

const Class* getClass(const String& clsName,
                      const Class::Const& typeCns,
                      const Class* typeCnsCls,
                      bool& persistent) {
  auto checkPersistent = [&persistent](const Class* cls) {
    persistent &= classHasPersistentRDS(cls);
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
  auto ts = getAlias(name);
  while (!ts.empty()) {
    assert(ts.exists(s_kind));
    if (!ts.exists(s_classname)) {
      // not a class, interface, trait, enum, or alias
      throw Exception(
        "%s, alias %s does not resolve to a class",
        resolveContextMsg(typeCns, typeCnsCls).c_str(),
        name.data());
    }
    name = ts[s_classname].toCStrRef();
    ts = getAlias(name);
  }

  auto const cls = Unit::loadClass(name.get());
  if (!cls) {
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
Array resolveShape(const Array& arr,
                   const Class::Const& typeCns,
                   const Class* typeCnsCls,
                   const Array& generics,
                   bool& persistent) {
  assert(arr.exists(s_kind));
  assert(static_cast<TypeStructure::Kind>(arr[s_kind].toInt64Val())
         == TypeStructure::Kind::T_shape);
  assert(arr.exists(s_fields));

  auto newfields = Array::Create();
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
      auto cls = getClass(String(clsName), typeCns, typeCnsCls, persistent);
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
    assert(wrapper.exists(s_value));
    auto valueArr = wrapper[s_value].toArray();
    auto value =
      resolveTS(valueArr, typeCns, typeCnsCls, generics, persistent);
    newfields.add(key, Variant(value));
  }

  return newfields;
}

void resolveClass(Array& ret,
                  const String& clsName,
                  const Class::Const& typeCns,
                  const Class* typeCnsCls,
                  bool& persistent) {
  auto const cls = getClass(clsName, typeCns, typeCnsCls, persistent);

  TypeStructure::Kind resolvedKind;
  if (isNormalClass(cls)) {
    resolvedKind = TypeStructure::Kind::T_class;
  } else if (isInterface(cls)) {
    resolvedKind = TypeStructure::Kind::T_interface;
  } else if (isTrait(cls)) {
    resolvedKind = TypeStructure::Kind::T_trait;
  } else if (isEnum(cls)) {
    resolvedKind = TypeStructure::Kind::T_enum;
  } else {
    not_reached();
  }

  ret.set(s_kind, Variant(static_cast<uint8_t>(resolvedKind)));
  ret.add(s_classname, Variant(makeStaticString(cls->name())));
}

Array resolveGenerics(const Array& arr,
                      const Class::Const& typeCns,
                      const Class* typeCnsCls,
                      const Array& generics,
                      bool& persistent) {
  auto genericsArr = arr[s_generic_types].toArray();
  return resolveList(genericsArr, typeCns, typeCnsCls, generics, persistent);
}

Array resolveTS(const Array& arr,
                const Class::Const& typeCns,
                const Class* typeCnsCls,
                const Array& generics,
                bool& persistent) {
  assert(arr.exists(s_kind));
  auto const kind = static_cast<TypeStructure::Kind>(
    arr[s_kind].toInt64Val());

  auto newarr = Array::Create();
  if (arr.exists(s_nullable)) newarr.add(s_nullable, true_varNR);
  newarr.add(s_kind, Variant(static_cast<uint8_t>(kind)));

  switch (kind) {
    case TypeStructure::Kind::T_tuple: {
      assert(arr.exists(s_elem_types));
      auto const elemsArr = arr[s_elem_types].toCArrRef();
      auto const elemTypes =
        resolveList(elemsArr, typeCns, typeCnsCls, generics, persistent);
      newarr.add(s_elem_types, Variant(elemTypes));
      break;
    }
    case TypeStructure::Kind::T_fun: {
      assert(arr.exists(s_return_type));
      auto const returnArr = arr[s_return_type].toCArrRef();
      auto const returnType =
        resolveTS(returnArr, typeCns, typeCnsCls, generics, persistent);
      newarr.add(s_return_type, Variant(returnType));

      assert(arr.exists(s_param_types));
      auto const paramsArr = arr[s_param_types].toCArrRef();
      auto const paramTypes =
        resolveList(paramsArr, typeCns, typeCnsCls, generics, persistent);
      newarr.add(s_param_types, Variant(paramTypes));
      break;
    }
    case TypeStructure::Kind::T_array: {
      if (arr.exists(s_generic_types)) {
        newarr.add(s_generic_types,
                   Variant(resolveGenerics(arr, typeCns, typeCnsCls,
                                           generics, persistent)));
      }
      break;
    }
    case TypeStructure::Kind::T_shape: {
      auto const fields =
        resolveShape(arr, typeCns, typeCnsCls, generics, persistent);
      newarr.add(s_fields, Variant(fields));
      break;
    }
    case TypeStructure::Kind::T_unresolved: {
      assert(arr.exists(s_classname));
      auto const clsName = arr[s_classname].toCStrRef();
      auto ts = getAlias(clsName);
      if (!ts.empty()) {
        if (ts.exists(s_typevars) && arr.exists(s_generic_types)) {
          std::vector<std::string> typevars;
          folly::split(",", ts[s_typevars].toCStrRef().data(), typevars);
          ts.remove(s_typevars);

          auto generic_types =
            resolveGenerics(arr, typeCns, typeCnsCls, generics, persistent);

          auto const sz = std::min(static_cast<ssize_t>(typevars.size()),
                                   generic_types.size());
          ArrayInit newarr(sz, ArrayInit::Map{});
          for (auto i = 0; i < sz; i++) {
            newarr.add(String(typevars[i]), generic_types[i]);
          }
          auto generics = newarr.toArray();
          ts = TypeStructure::resolve(clsName, ts, persistent, generics);
          ts.add(s_typevar_types, Variant(generics));
        } else {
          ts = TypeStructure::resolve(clsName, ts, persistent);
        }
        if (arr.exists(s_nullable)) {
          ts.add(s_nullable, true_varNR);
        }

        return ts;
      }

      /* Special cases for 'callable': Hack typechecker throws a naming error
       * (unbound name), however, hhvm still supports this type hint to be
       * compatible with php. We simply return as a OF_CLASS with class name
       * set to 'callable'. */
      if (clsName.same(s_callable)) {
        newarr.add(s_kind,
                   Variant(static_cast<uint8_t>(TypeStructure::Kind::T_class)));
        newarr.add(s_classname, Variant(clsName));
        break;
      }
      resolveClass(newarr, clsName, typeCns, typeCnsCls, persistent);
      if (arr.exists(s_generic_types)) {
        newarr.add(s_generic_types,
                   Variant(resolveGenerics(arr, typeCns, typeCnsCls,
                                           generics, persistent)));
      }
      break;
    }
    case TypeStructure::Kind::T_typeaccess: {
      /* type access is a root class (may be HH\this) followed by a series of
       * type constants, i.e., cls::TC1::TC2::TC3. Each type constant other
       * than the last one in the chain must refer to a class or an
       * interface. */
      assert(arr.exists(s_root_name));
      auto clsName = arr[s_root_name].toCStrRef();
      assert(arr.exists(s_access_list));
      auto const accList = arr[s_access_list].toCArrRef();
      auto const sz = accList.size();
      Array typeCnsVal;
      for (auto i = 0; i < sz; i++) {
        auto const cls = getClass(clsName, typeCns, typeCnsCls, persistent);
        auto const cnsName = accList[i].toCStrRef();
        if (!cls->hasTypeConstant(cnsName.get())) {
          throw Exception(
            "%s, class %s does not have non-abstract "
            "type constant %s",
            resolveContextMsg(typeCns, typeCnsCls).c_str(),
            clsName.data(),
            cnsName.data());
        }
        auto tv = cls->clsCnsGet(cnsName.get(), /* includeTypeCns = */ true);
        assert(isArrayType(tv.m_type));
        typeCnsVal = Array(tv.m_data.parr);
        if (i == sz - 1) break;

        // if there are more accesses, keep resolving
        assert(typeCnsVal.exists(s_kind));
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
        assert(typeCnsVal.exists(s_classname));
        clsName = typeCnsVal[s_classname].toCStrRef();
      }
      return typeCnsVal;
    }
    case TypeStructure::Kind::T_typevar: {
      assert(arr.exists(s_name));
      auto const name = arr[s_name].toCStrRef();
      return generics.exists(name) ? generics[name].toArray() : Array(arr);
    }
    case TypeStructure::Kind::T_xhp:
    default:
      return Array(arr);
  }

  if(arr.exists(s_typevars)) newarr.add(s_typevars, arr[s_typevars]);

  return newarr;
}

} // anonymous namespace

bool TypeStructure::KindOfClass(TypeStructure::Kind kind) {
  return kind == TypeStructure::Kind::T_class
    || kind == TypeStructure::Kind::T_interface
    || kind == TypeStructure::Kind::T_trait
    || kind == TypeStructure::Kind::T_enum;
}

String TypeStructure::toString(const Array& arr) {
  if (arr.empty()) return String();

  return String(fullName(arr));
}

/*
 * Constructs a scalar array with all the shape field names, this/self/parent,
 * classes, type accesses, and type aliases resolved.
 */
Array TypeStructure::resolve(const Class::Const& typeCns,
                             const Class* typeCnsCls,
                             bool& persistent) {
  assert(typeCns.isType());
  assert(isArrayType(typeCns.val.m_type));
  assert(typeCns.name);
  assert(typeCnsCls);

  Array arr(typeCns.val.m_data.parr);
  return resolveTS(arr, typeCns, typeCnsCls, Array(), persistent);
}

/*
 * Called by TypeAliasReq to get resolved TypeStructure for type aliases.
 */
Array TypeStructure::resolve(const String& aliasName,
                             const Array& arr,
                             bool& persistent,
                             const Array& generics) {
  // use a bogus constant to store the name
  Class::Const cns;
  cns.name = aliasName.get();

  auto newarr = resolveTS(arr, cns, nullptr, generics, persistent);
  newarr.add(s_alias, Variant(aliasName));
  return newarr;
}

} // namespace HPHP

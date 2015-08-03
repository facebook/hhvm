/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
  s_alias("alias")
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

std::string fullName(const ArrayData* arr);

void functionTypeName(const ArrayData* arr, std::string& name) {
  name += "(function (";

  assert(arr->exists(s_return_type));
  auto const retType = arr->get(s_return_type).getArrayData();

  assert(arr->exists(s_param_types));
  auto const params = arr->get(s_param_types).getArrayData();

  auto sep = "";
  auto sz = params->getSize();
  for (auto i = 0; i < sz; i++) {
    folly::toAppend(sep, fullName(params->get(i).getArrayData()), &name);
    sep = ", ";
  }

  // add funciton return type
  folly::toAppend("): ", fullName(retType), ")", &name);
}

void accessTypeName(const ArrayData* arr, std::string& name) {
  assert(arr->exists(s_root_name));
  auto const rootName = arr->get(s_root_name).getStringData();
  name += rootName->toCppString();

  assert(arr->exists(s_access_list));
  auto const accessList = arr->get(s_access_list).getArrayData();
  auto sz = accessList->getSize();
  for (auto i = 0; i < sz; i++) {
    folly::toAppend("::",
                    accessList->get(i).getStringData()->toCppString(),
                    &name);
  }
}

// xhp names are mangled so we get them back to their original definition
// see the mangling in ScannerToken::xhpLabel
void xhpTypeName(const ArrayData* arr, std::string& name) {
  assert(arr->exists(s_classname));
  std::string clsName = arr->get(s_classname).getStringData()->toCppString();
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

void tupleTypeName(const ArrayData* arr, std::string& name) {
  name += "(";
  assert(arr->exists(s_elem_types));
  auto const elems = arr->get(s_elem_types).getArrayData();
  auto sz = elems->getSize();
  auto sep = "";
  for (auto i = 0; i < sz; i++) {
    folly::toAppend(sep, fullName(elems->get(i).getArrayData()), &name);
    sep = ", ";
  }

  name += ")";
}

void genericTypeName(const ArrayData* arr, std::string& name) {
  name += "<";
  assert(arr->exists(s_generic_types));
  auto const args = arr->get(s_generic_types).getArrayData();
  auto sz = args->getSize();
  auto sep = "";
  for (auto i = 0; i < sz; i++) {
    folly::toAppend(sep, fullName(args->get(i).getArrayData()), &name);
    sep = ", ";
  }
  name += ">";
}

void shapeTypeName(const ArrayData* arr, std::string& name) {
  // works for both resolved and unresolved TypeStructures
  name += "(";
  assert(arr->exists(s_fields));
  auto const fields = arr->get(s_fields).getArrayData();
  auto const sz = fields->getSize();
  auto sep = "";
  for (auto i = 0; i < sz; i++) {
    name += sep;
    auto field = fields->getKey(i);
    auto value = fields->getValue(i).getArrayData();
    auto quote = "'";
    if (value->exists(s_value)) {
      // if unresolved, ignore wrapper
      if (value->exists(s_is_cls_cns)) quote = "";
      value = value->get(s_value).getArrayData();
    }
    auto fieldType = field.getType();
    if (isStringType(fieldType)) {
      folly::toAppend(quote, field.getStringData()->data(), quote, &name);
    } else if (isIntType(fieldType)) {
      folly::toAppend(field.getInt64(), &name);
    }

    folly::toAppend("=>", fullName(value), &name);
    sep = ", ";
  }

  name += ")";
}

std::string fullName (const ArrayData* arr) {
  std::string name;
  if (arr->exists(s_nullable)) {
    assert(arr->get(s_nullable).getBoolean());
    name += '?';
  }

  assert(arr->exists(s_kind));

  TypeStructure::Kind kind =
    TypeStructure::Kind(arr->get(s_kind).getNumData());
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
      if (arr->exists(s_generic_types)) {
        genericTypeName(arr, name);
      }
      break;
    case TypeStructure::Kind::T_shape:
      name += s_shape;
      shapeTypeName(arr, name);
      break;
    case TypeStructure::Kind::T_typevar:
      assert(arr->exists(s_name));
      name += arr->get(s_name).getStringData()->toCppString();
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
      assert(arr->exists(s_classname));
      name += arr->get(s_classname).getStringData()->toCppString();
      if (arr->exists(s_generic_types)) genericTypeName(arr, name);
      break;
  }

  return name;
}

ArrayData* resolveTS(ArrayData* arr,
                     const Class::Const& typeCns,
                     const Class* typeCnsCls);

ArrayData* resolveList(ArrayData* arr,
                       const Class::Const& typeCns,
                       const Class* typeCnsCls) {
  auto const sz = arr->getSize();

  PackedArrayInit newarr(sz);
  for (auto i = 0; i < sz; i++) {
    auto elemArr = arr->get(i).getArrayData();
    auto elem = resolveTS(elemArr, typeCns, typeCnsCls);
    newarr.append(Variant(elem));
  }

  return ArrayData::GetScalarArray(newarr.create());
}

const std::string resolveContextMsg(const Class::Const& typeCns,
                                    const Class* typeCnsCls) {
  std::string msg("when resolving ");
  if (typeCnsCls) {
    folly::toAppend("type constant ", typeCnsCls->name()->data(),
                    "::", typeCns.m_name->data(),
                    &msg);
  } else {
    folly::toAppend("type alias ", typeCns.m_name->data(), &msg);
  }
  return msg;
}

/* returns the resolved TypeStructure; if aliasName is not an alias,
 * return nullptr. */
ArrayData* getAlias(const StringData* aliasName) {
  auto typeAliasReq = Unit::loadTypeAlias(aliasName);
  if (!typeAliasReq) return nullptr;

  // this returned type structure is resolved.
  return TypeStructure::resolve(aliasName, typeAliasReq->typeStructure);
}

const Class* getClass(const StringData* clsName,
                      const Class::Const& typeCns,
                      const Class* typeCnsCls) {
  // the original unresolved type structure came from a type constant
  // (instead of a type alias), and may have this/self/parent.
  if (typeCnsCls) {
    // HH\this: late static binding
    if (clsName->same(s_this.get())) {
      return typeCnsCls;
    }

    auto declCls = typeCns.m_class;
    // self
    if (clsName->same(s_self.get())) {
      return declCls;
    }
    // parent
    if (clsName->same(s_parent.get())) {
      auto parent = declCls->parent();
      if (!parent) {
        throw Exception(
          "%s, class %s does not have a parent",
          resolveContextMsg(typeCns, typeCnsCls).c_str(),
          declCls->name()->data());
      }
      return parent;
    }
  }

  auto name = const_cast<StringData*>(clsName);
  auto ts = getAlias(name);
  while (ts) {
    assert(ts->exists(s_kind));
    if (!ts->exists(s_classname)) {
      // not a class, interface, trait, enum, or alias
      throw Exception(
        "%s, alias %s does not resolve to a class",
        resolveContextMsg(typeCns, typeCnsCls).c_str(),
        name->data());
    }
    name = ts->get(s_classname).getStringData();
    ts = getAlias(name);
  }

  auto const cls = Unit::loadClass(name);
  if (!cls) {
    throw Exception(
      "%s, class %s not found",
      resolveContextMsg(typeCns, typeCnsCls).c_str(),
      name->data());
  }

  return cls;
}

/* Given an unresolved T_shape TypeStructure, returns the __fields__
 * portion of the array with all the field names resolved to string
 * literals. */
ArrayData* resolveShape(ArrayData* arr,
                        const Class::Const& typeCns,
                        const Class* typeCnsCls) {
  assert(arr->exists(s_kind));
  assert(static_cast<TypeStructure::Kind>(arr->get(s_kind).getNumData())
         == TypeStructure::Kind::T_shape);
  assert(arr->exists(s_fields));

  auto newfields = Array::Create();
  auto const fields = arr->get(s_fields).getArrayData();
  auto const sz = fields->getSize();
  for (auto i = 0; i < sz; i++) {
    Variant key = fields->getKey(i);
    auto wrapper = fields->getValue(i).getArrayData();
    if (wrapper->exists(s_is_cls_cns)) {
      // if the shape field name is a class constant, its name is
      // double colon delimited clsName::cnsName
      auto const clsCns = key.getStringData()->toCppString();
      std::string clsName, cnsName;
      folly::split("::", clsCns, clsName, cnsName);

      // look up clsName::cnsName
      auto cls = getClass(String(clsName).get(), typeCns, typeCnsCls);
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
    assert(wrapper->exists(s_value));
    auto valueArr = wrapper->get(s_value).getArrayData();
    auto value = resolveTS(valueArr, typeCns, typeCnsCls);
    newfields.add(key, Variant(value));
  }

  return ArrayData::GetScalarArray(newfields.get());
}

void resolveClass(Array& ret,
                  const StringData* clsName,
                  const Class::Const& typeCns,
                  const Class* typeCnsCls) {
  auto const cls = getClass(clsName, typeCns, typeCnsCls);

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

void resolveGenerics(Array& ret,
                     ArrayData* arr,
                     const Class::Const& typeCns,
                     const Class* typeCnsCls){
  if (arr->exists(s_generic_types)) {
    auto genericsArr = arr->get(s_generic_types).getArrayData();
    auto genericTypes = resolveList(genericsArr, typeCns, typeCnsCls);
    ret.add(s_generic_types, Variant(genericTypes));
  }
}

ArrayData* resolveTS(ArrayData* arr,
                     const Class::Const& typeCns,
                     const Class* typeCnsCls) {
  assert(arr->exists(s_kind));
  auto const kind = static_cast<TypeStructure::Kind>(
    arr->get(s_kind).getNumData());

  auto newarr = Array::Create();
  if (arr->exists(s_nullable)) newarr.add(s_nullable, true_varNR);
  newarr.add(s_kind, Variant(static_cast<uint8_t>(kind)));

  switch (kind) {
    case TypeStructure::Kind::T_tuple: {
      assert(arr->exists(s_elem_types));
      auto elemsArr = arr->get(s_elem_types).getArrayData();
      auto elemTypes = resolveList(elemsArr, typeCns, typeCnsCls);
      newarr.add(s_elem_types, Variant(elemTypes));
      break;
    }
    case TypeStructure::Kind::T_fun: {
      assert(arr->exists(s_return_type));
      auto returnArr = arr->get(s_return_type).getArrayData();
      auto returnType = resolveTS(returnArr, typeCns, typeCnsCls);
      newarr.add(s_return_type, Variant(returnType));

      assert(arr->exists(s_param_types));
      auto paramsArr = arr->get(s_param_types).getArrayData();
      auto paramTypes = resolveList(paramsArr, typeCns, typeCnsCls);
      newarr.add(s_param_types, Variant(paramTypes));
      break;
    }
    case TypeStructure::Kind::T_array: {
      resolveGenerics(newarr, arr, typeCns, typeCnsCls);
      break;
    }
    case TypeStructure::Kind::T_shape: {
      auto fields = resolveShape(arr, typeCns, typeCnsCls);
      newarr.add(s_fields, Variant(fields));
      break;
    }
    case TypeStructure::Kind::T_unresolved: {
      assert(arr->exists(s_classname));
      auto const clsName = arr->get(s_classname).getStringData();
      auto ts = getAlias(clsName);
      if (ts) return ts;

      /* Special cases for 'callable': Hack typechecker throws a naming
       * error (unbound name), however, hhvm still supports this type
       * hint to be compatible with php. We simply return as a
       * OF_CLASS with class name set to 'callable'. */
      if (clsName->same(s_callable.get())) {
        newarr.add(s_kind,
                   Variant(static_cast<uint8_t>(TypeStructure::Kind::T_class)));
        newarr.add(s_classname, Variant(clsName));
        break;
      }

      resolveClass(newarr, clsName, typeCns, typeCnsCls);
      resolveGenerics(newarr, arr, typeCns, typeCnsCls);
      break;
    }
    case TypeStructure::Kind::T_typeaccess: {
      /* type access is a root class (may be HH\this) followed by a
       * series of type constants, i.e., cls::TC1::TC2::TC3. Each type
       * constant other than the last one in the chain must refer to a
       * class or an interface. */
      assert(arr->exists(s_root_name));
      auto clsName = arr->get(s_root_name).getStringData();
      assert(arr->exists(s_access_list));
      auto accList = arr->get(s_access_list).getArrayData();
      auto sz = accList->getSize();
      ArrayData* typeCnsVal;
      for (auto i = 0; i < sz; i++) {
        auto const cls = getClass(clsName, typeCns, typeCnsCls);
        auto cnsName = accList->get(i).getStringData();
        if (!cls->hasTypeConstant(cnsName)) {
          throw Exception(
            "%s, class %s does not have non-abstract "
            "type constant %s",
            resolveContextMsg(typeCns, typeCnsCls).c_str(),
            clsName->data(),
            cnsName->data());
        }
        auto tv = cls->clsCnsGet(cnsName, /* includeTypeCns = */ true);
        assert(tv.m_type == KindOfArray);
        typeCnsVal = tv.m_data.parr;
        if (i == sz - 1) break;

        // if there are more accesses, keep resolving
        assert(typeCnsVal->exists(s_kind));
        auto kind = static_cast<TypeStructure::Kind>
          (typeCnsVal->get(s_kind).getInt64());
        if (kind != TypeStructure::Kind::T_class
            && kind != TypeStructure::Kind::T_interface) {
          throw Exception(
            "%s, %s::%s does not resolve to a class or "
            "an interface and cannot contain type constant %s",
            resolveContextMsg(typeCns, typeCnsCls).c_str(),
            clsName->data(),
            cnsName->data(),
            accList->get(i+1).getStringData()->data());
        }
        assert(typeCnsVal->exists(s_classname));
        clsName = typeCnsVal->get(s_classname).getStringData();
      }
      assert(typeCnsVal->isStatic());
      return typeCnsVal;
    }
    case TypeStructure::Kind::T_typevar:
    case TypeStructure::Kind::T_xhp:
    default:
      return arr;
  }

  return ArrayData::GetScalarArray(newarr.get());
}

} // anonymous namespace

bool TypeStructure::KindOfClass(TypeStructure::Kind kind) {
  return kind == TypeStructure::Kind::T_class
    || kind == TypeStructure::Kind::T_interface
    || kind == TypeStructure::Kind::T_trait
    || kind == TypeStructure::Kind::T_enum;
}

String TypeStructure::toString(const ArrayData* arr) {
  if (arr->empty()) return String();

  return String(fullName(arr));
}

/* Constructs a scalar array with all the shape field names,
 * this/self/parent, classes, type accesses, and type aliases
 * resolved. */
ArrayData* TypeStructure::resolve(const Class::Const& typeCns,
                                  const Class* typeCnsCls) {
  assert(typeCns.isType());
  assert(typeCns.m_val.m_type == KindOfArray);
  assert(typeCns.m_name);
  assert(typeCnsCls);

  return resolveTS(typeCns.m_val.m_data.parr, typeCns, typeCnsCls);
}

/* called by TypeAliasReq to get resolved TypeStructure for type aliases */
ArrayData* TypeStructure::resolve(const StringData* aliasName,
                                  const ArrayData* arr) {
  // use a bogus constant to store the name
  Class::Const cns;
  cns.m_name = aliasName;

  auto newarr = Array(resolveTS(const_cast<ArrayData*>(arr), cns, nullptr));
  newarr.add(s_alias, Variant(const_cast<StringData*>(aliasName)));
  return ArrayData::GetScalarArray(newarr.get());
}

} // namespace HPHP

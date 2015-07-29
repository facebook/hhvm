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
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/type-variant.h"
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
  s_this("HH\\this")
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
  // when toString is accessed, all class constant field names should
  // have been resolved.
  name += "(";
  assert(arr->exists(s_fields));
  auto const fields = arr->get(s_fields).getArrayData();
  auto const sz = fields->getSize();
  auto sep = "";
  for (auto i = 0; i < sz; i++) {
    name += sep;
    auto field = fields->getKey(i);
    auto fieldType = field.getType();
    if (isStringType(fieldType)) {
      folly::toAppend("'", field.getStringData()->data(), "'", &name);
    } else if (isIntType(fieldType)) {
      folly::toAppend(field.getInt64(), &name);
    }
    auto value = fields->getValue(i);
    folly::toAppend("=>", fullName(value.getArrayData()), &name);
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

ArrayData* resolveTS(ArrayData* arr, const Class* typeCnsCls);

ArrayData* resolveList(ArrayData* arr, const Class* typeCnsCls) {
  auto const sz = arr->getSize();

  auto newarr = Array::Create();
  for (auto i = 0; i < sz; i++) {
    auto elem = arr->get(i).getArrayData();
    newarr.add(i, Variant(resolveTS(elem, typeCnsCls)));
  }

  return ArrayData::GetScalarArray(newarr.get());
}

/* Given an unresolved T_shape TypeStructure, returns the __fields__
 * portion of the array with all the field names resolved to string
 * literals. */
ArrayData* resolveShape(ArrayData* arr, const Class* typeCnsCls) {
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
      auto cls = Unit::loadClass(String(clsName).get());
      if (!cls) raise_error("class %s not found", clsName.c_str());
      auto cnsValue = cls->clsCnsGet(String(cnsName).get());

      if (isStringType(cnsValue.m_type) || isIntType(cnsValue.m_type)) {
        key = tvAsVariant(&cnsValue);
      } else {
        raise_error("class constant %s::%s is not a string or an int "
                    "and cannot be used as shape field names",
                    clsName.c_str(), cnsName.c_str());
      }
    }
    assert(wrapper->exists(s_value));
    auto value = resolveTS(wrapper->get(s_value).getArrayData(), typeCnsCls);
    newfields.add(key, Variant(value));
  }

  return ArrayData::GetScalarArray(newfields.get());
}

void resolveClass(Array& ret, const StringData* clsName,
                  const Class* typeCnsCls) {
  bool isThis = clsName->same(s_this.get());
  auto const cls = isThis ? typeCnsCls : Unit::loadClass(clsName);
  if (!cls) raise_error("class %s does not exist", clsName->data());

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

  if (isThis && (isTrait(cls) || isEnum(cls))) {
    // traits or enums cannot contain class constants
    not_reached();
  }
  ret.set(s_kind, Variant(static_cast<uint8_t>(resolvedKind)));
  ret.add(s_classname,
          Variant(makeStaticString(cls->name())));
}

ArrayData* resolveTS(ArrayData* arr, const Class* typeCnsCls) {
  assert(arr->exists(s_kind));
  auto const kind = static_cast<TypeStructure::Kind>(
    arr->get(s_kind).getNumData());

  auto newarr = Array::Create();
  if (arr->exists(s_nullable)) newarr.add(s_nullable, true_varNR);
  newarr.add(s_kind, Variant(static_cast<uint8_t>(kind)));

  switch (kind) {
    case TypeStructure::Kind::T_tuple: {
      assert(arr->exists(s_elem_types));
      auto elemTypes = arr->get(s_elem_types).getArrayData();
      newarr.add(s_elem_types, Variant(resolveList(elemTypes, typeCnsCls)));
      break;
    }
    case TypeStructure::Kind::T_fun: {
      assert(arr->exists(s_return_type));
      auto returnType = arr->get(s_return_type).getArrayData();
      newarr.add(s_return_type, Variant(resolveTS(returnType, typeCnsCls)));
      assert(arr->exists(s_param_types));
      auto paramTypes = arr->get(s_param_types).getArrayData();
      newarr.add(s_param_types, Variant(resolveList(paramTypes, typeCnsCls)));
      break;
    }
    case TypeStructure::Kind::T_array: {
      if (arr->exists(s_generic_types)) {
        auto genericTypes = arr->get(s_generic_types).getArrayData();
        newarr.add(s_generic_types,
                   Variant(resolveList(genericTypes, typeCnsCls)));
      }
      break;
    }
    case TypeStructure::Kind::T_shape:
      newarr.add(s_fields, Variant(resolveShape(arr, typeCnsCls)));
      break;
    case TypeStructure::Kind::T_unresolved: {
      assert(arr->exists(s_classname));
      auto const clsName = arr->get(s_classname).getStringData();
      resolveClass(newarr, clsName, typeCnsCls);
      if (arr->exists(s_generic_types)) {
        auto genericTypes = arr->get(s_generic_types).getArrayData();
        newarr.add(s_generic_types,
                   Variant(resolveList(genericTypes, typeCnsCls)));
      }
      break;
    }
    case TypeStructure::Kind::T_typevar:
    case TypeStructure::Kind::T_typeaccess:
      // TODO(7650500): resolve this if it is the rootname.
    case TypeStructure::Kind::T_xhp:
    default:
      return arr;
  }

  return ArrayData::GetScalarArray(newarr.get());
}

} // anonymous namespace

String TypeStructure::toString(const ArrayData* arr) {
  if (arr->empty()) return String();

  return String(fullName(arr));
}

/* Constructs a scalar array with all the shape field names, 'this'
 * resolved. TODO(7657500): resolve type access and generic types. */
ArrayData* TypeStructure::resolve(ArrayData* arr,
                                  const Class* typeCnsCls) {
  assert(typeCnsCls);
  if (arr == nullptr) return arr;

  return resolveTS(arr, typeCnsCls);
}

} // namespace HPHP

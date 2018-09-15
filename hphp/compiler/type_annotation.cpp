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

#include "hphp/compiler/type_annotation.h"

#include <vector>

#include "hphp/compiler/code_generator.h"
#include "hphp/util/text-util.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-structure.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

TypeAnnotation::TypeAnnotation(const std::string &name,
  TypeAnnotationPtr typeArgs) : m_name(name),
                                m_typeArgs(typeArgs),
                                m_typeList(TypeAnnotationPtr()),
                                m_nullable(false),
                                m_soft(false),
                                m_tuple(false),
                                m_function(false),
                                m_xhp(false),
                                m_typevar(false),
                                m_typeaccess(false),
                                m_shape(false),
                                m_allowsUnknownFields(false),
                                m_clsCnsShapeField(false),
                                m_optionalShapeField(false),
                                m_reifiedtype(false) {
}

std::string TypeAnnotation::vanillaName() const {
  // filter out types that should not be exposed to the runtime
  if (m_nullable || m_soft || m_typevar || m_function || m_typeaccess) {
    return "";
  }
  if (!strcasecmp(m_name.c_str(), "HH\\mixed")) {
    return "";
  }
  return m_name;
}

std::string TypeAnnotation::fullName() const {
  std::string name;
  if (m_soft) {
    name += '@';
  }
  if (m_nullable) {
    name += '?';
  }

  if (m_function) {
    functionTypeName(name);
  } else if (m_typeaccess) {
    accessTypeName(name);
  } else if (m_xhp) {
    xhpTypeName(name);
  } else if (m_tuple) {
    tupleTypeName(name);
  } else if (m_shape) {
    shapeTypeName(name);
  } else if (m_typeArgs) {
    genericTypeName(name);
  } else {
    name += m_name;
  }
  return name;
}

MaybeDataType TypeAnnotation::dataType() const {
  return get_datatype(m_name, (bool)m_typeArgs, m_function,
    m_xhp, m_tuple, m_nullable, m_soft);
}

void TypeAnnotation::getAllSimpleNames(std::vector<std::string>& names) const {
  names.push_back(m_name);
  if (m_typeList) {
    m_typeList->getAllSimpleNames(names);
  } else if (m_typeArgs) {
    // do not return the shape fields and keys
    if (!m_shape) {
      m_typeArgs->getAllSimpleNames(names);
    }
  }
}

void TypeAnnotation::shapeTypeName(std::string& name) const {
  name += "HH\\shape(";
  TypeAnnotationPtr shapeField = m_typeArgs;
  auto sep = "";
  while (shapeField) {
    name += sep;

    if (shapeField->isOptionalShapeField()) {
      folly::toAppend("?", &name);
    }

    if (shapeField->isClsCnsShapeField()) {
      folly::toAppend(shapeField->m_name, &name);
    } else {
      folly::toAppend("'", shapeField->m_name, "'", &name);
    }
    auto fieldValue = shapeField->m_typeArgs;
    assert(fieldValue);
    folly::toAppend("=>", fieldValue->fullName(), &name);

    sep = ", ";
    shapeField = shapeField->m_typeList;
  }

  name += ")";
}

void TypeAnnotation::functionTypeName(std::string &name) const {
  name += "(function (";
  // return value of function types is the first element of type list
  TypeAnnotationPtr retType = m_typeArgs;
  TypeAnnotationPtr typeEl = m_typeArgs->m_typeList;
  auto sep = "";
  while (typeEl) {
    folly::toAppend(sep, typeEl->fullName(), &name);
    typeEl = typeEl->m_typeList;
    sep = ", ";
  }
  // add function return value
  folly::toAppend("): ", retType->fullName(), ")", &name);
}

// xhp names are mangled so we get them back to their original definition
// @see the mangling in ScannerToken::xhpLabel
void TypeAnnotation::xhpTypeName(std::string &name) const {
  // remove prefix if any
  if (m_name.compare(0, sizeof("xhp_") - 1, "xhp_") == 0) {
    name += std::string(m_name).replace(0, sizeof("xhp_") - 1, ":");
  } else {
    name += m_name;
  }
  // un-mangle back
  replaceAll(name, "__", ":");
  replaceAll(name, "_", "-");
}

void TypeAnnotation::tupleTypeName(std::string &name) const {
  name += "(";
  TypeAnnotationPtr typeEl = m_typeArgs;
  auto sep = "";
  while (typeEl) {
    folly::toAppend(sep, typeEl->fullName(), &name);
    typeEl = typeEl->m_typeList;
    sep = ", ";
  }
  name += ")";
}

void TypeAnnotation::genericTypeName(std::string &name) const {
  folly::toAppend(m_name, "<", &name);
  TypeAnnotationPtr typeEl = m_typeArgs;
  auto sep = "";
  while (typeEl) {
    folly::toAppend(sep, typeEl->fullName(), &name);
    typeEl = typeEl->m_typeList;
    sep = ", ";
  }
  name += ">";
}

void TypeAnnotation::accessTypeName(std::string &name) const {
  name += m_name;
  TypeAnnotationPtr typeEl = m_typeArgs;
  while (typeEl) {
    folly::toAppend("::", typeEl->fullName(), &name);
    typeEl = typeEl->m_typeList;
  }
}

void TypeAnnotation::appendToTypeList(TypeAnnotationPtr typeList) {
  if (m_typeList) {
    TypeAnnotationPtr current = m_typeList;
    while (current->m_typeList) {
      current = current->m_typeList;
    }
    current->m_typeList = typeList;
  } else {
    m_typeList = typeList;
  }
}

int TypeAnnotation::numTypeArgs() const {
  int n = 0;
  TypeAnnotationPtr typeEl = m_typeArgs;
  while (typeEl) {
    ++n;
    typeEl = typeEl->m_typeList;
  }
  return n;
}

TypeAnnotationPtr TypeAnnotation::getTypeArg(int n) const {
  int i = 0;
  TypeAnnotationPtr typeEl = m_typeArgs;
  while (typeEl) {
    if (i == n) {
      return typeEl;
    }
    ++i;
    typeEl = typeEl->m_typeList;
  }
  return TypeAnnotationPtr();
}

bool TypeAnnotation::isPrimType(const char* str) const{
  return !strcasecmp(m_name.c_str(), str);
}

TypeStructure::Kind TypeAnnotation::getKind() const {
  if (isVoid()) {
    return TypeStructure::Kind::T_void;
  }

  // Primitive types
  if (isPrimType("HH\\int")) {
    return TypeStructure::Kind::T_int;
  }
  if (isPrimType("HH\\bool")) {
    return TypeStructure::Kind::T_bool;
  }
  if (isPrimType("HH\\float")) {
    return TypeStructure::Kind::T_float;
  }
  if (isPrimType("HH\\string")) {
    return TypeStructure::Kind::T_string;
  }
  if (isPrimType("HH\\resource")) {
    return TypeStructure::Kind::T_resource;
  }
  if (isPrimType("HH\\num")) {
    return TypeStructure::Kind::T_num;
  }
  if (isPrimType("HH\\arraykey")) {
    return TypeStructure::Kind::T_arraykey;
  }
  if (isPrimType("HH\\noreturn")) {
    return TypeStructure::Kind::T_noreturn;
  }

  if (isMixed()) {
    return TypeStructure::Kind::T_mixed;
  }
  if (isNonnull()) {
    return TypeStructure::Kind::T_nonnull;
  }
  if (m_tuple) {
    return TypeStructure::Kind::T_tuple;
  }
  if (m_shape) {
    return TypeStructure::Kind::T_shape;
  }
  if (m_function) {
    return TypeStructure::Kind::T_fun;
  }
  if (!strcasecmp(m_name.c_str(), "array")) {
    return TypeStructure::Kind::T_array;
  }
  if (!strcasecmp(m_name.c_str(), "HH\\varray")) {
    return TypeStructure::Kind::T_varray;
  }
  if (!strcasecmp(m_name.c_str(), "HH\\darray")) {
    return TypeStructure::Kind::T_darray;
  }
  if (!strcasecmp(m_name.c_str(), "HH\\varray_or_darray")) {
    return TypeStructure::Kind::T_varray_or_darray;
  }
  if (!strcasecmp(m_name.c_str(), "HH\\vec_or_dict")) {
    return TypeStructure::Kind::T_vec_or_dict;
  }
  if (!strcasecmp(m_name.c_str(), "HH\\arraylike")) {
    return TypeStructure::Kind::T_arraylike;
  }
  if (!strcasecmp(m_name.c_str(), "HH\\dict")) {
    return TypeStructure::Kind::T_dict;
  }
  if (!strcasecmp(m_name.c_str(), "HH\\vec")) {
    return TypeStructure::Kind::T_vec;
  }
  if (!strcasecmp(m_name.c_str(), "HH\\keyset")) {
    return TypeStructure::Kind::T_keyset;
  }
  if (m_typevar) {
    return TypeStructure::Kind::T_typevar;
  }
  if (m_typeaccess) {
    return TypeStructure::Kind::T_typeaccess;
  }
  if (m_xhp) {
    // TODO(7657500): in the runtime, resolve this type to a class.
    return TypeStructure::Kind::T_xhp;
  }
  if (m_reifiedtype) {
    return TypeStructure::Kind::T_reifiedtype;
  }

  return TypeStructure::Kind::T_unresolved;
}

const StaticString
  s_nullable("nullable"),
  s_kind("kind"),
  s_name("name"),
  s_classname("classname"),
  s_elem_types("elem_types"),
  s_return_type("return_type"),
  s_param_types("param_types"),
  s_generic_types("generic_types"),
  s_root_name("root_name"),
  s_access_list("access_list"),
  s_fields("fields"),
  s_allows_unknown_fields("allows_unknown_fields"),
  s_is_cls_cns("is_cls_cns"),
  s_optional_shape_field("optional_shape_field"),
  s_value("value"),
  s_typevars("typevars"),
  s_id("id")
;

/* Turns the argsList linked list of TypeAnnotation into a positioned
 * static array. */
Array TypeAnnotation::argsListToScalarArray(TypeAnnotationPtr ta) const {
  auto typeargs = Array::CreateVArray();

  auto typeEl = ta;
  while (typeEl) {
    typeargs.append(Variant(typeEl->getScalarArrayRep()));
    typeEl = typeEl->m_typeList;
  }
  return typeargs;
}

void TypeAnnotation::shapeFieldsToScalarArray(Array& rep,
                                              TypeAnnotationPtr ta) const {
  auto fields = Array::CreateDArray();
  auto shapeField = ta;
  while (shapeField) {
    assert(shapeField->m_typeArgs);

    auto field = Array::CreateDArray();
    if (shapeField->isClsCnsShapeField()) {
      field.set(s_is_cls_cns, true_varNR.tv());
    }
    if (shapeField->isOptionalShapeField()) {
      field.set(s_optional_shape_field, true_varNR.tv());
    }
    field.set(s_value, Variant(shapeField->m_typeArgs->getScalarArrayRep()));
    fields.set(String(shapeField->m_name), Variant(field.get()));
    shapeField = shapeField->m_typeList;
  }
  rep.set(s_fields, Variant(fields));
}

Array TypeAnnotation::getScalarArrayRep() const {
  auto rep = Array::CreateDArray();

  bool nullable = (bool) m_nullable;
  if (nullable) {
    rep.set(s_nullable, true_varNR.tv());
  }

  bool allowsUnknownFields = (bool) m_allowsUnknownFields;
  if (allowsUnknownFields) {
    rep.set(s_allows_unknown_fields, true_varNR.tv());
  }

  TypeStructure::Kind kind = getKind();
  rep.set(s_kind, Variant(static_cast<uint8_t>(kind)));

  switch (kind) {
  case TypeStructure::Kind::T_tuple:
    assert(m_typeArgs);
    rep.set(s_elem_types, Variant(argsListToScalarArray(m_typeArgs)));
    break;
  case TypeStructure::Kind::T_fun:
    assert(m_typeArgs);
    // return type is the first of the typeArgs
    rep.set(s_return_type, Variant(m_typeArgs->getScalarArrayRep()));
    rep.set(s_param_types,
            Variant(argsListToScalarArray(m_typeArgs->m_typeList)));
    break;
  case TypeStructure::Kind::T_array:
  case TypeStructure::Kind::T_darray:
  case TypeStructure::Kind::T_varray:
  case TypeStructure::Kind::T_varray_or_darray:
  case TypeStructure::Kind::T_dict:
  case TypeStructure::Kind::T_vec:
  case TypeStructure::Kind::T_vec_or_dict:
  case TypeStructure::Kind::T_keyset:
    if (m_typeArgs) {
      rep.set(s_generic_types, Variant(argsListToScalarArray(m_typeArgs)));
    }
    break;
  case TypeStructure::Kind::T_shape:
    shapeFieldsToScalarArray(rep, m_typeArgs);
    break;
  case TypeStructure::Kind::T_typevar:
    rep.set(s_name, Variant(m_name));
    break;
  case TypeStructure::Kind::T_typeaccess: {
    // for now, only store the vanilla names (strings) as part of the
    // access list
    rep.set(s_root_name, Variant(m_name));
    auto accList = Array::CreateVArray();
    auto typeEl = m_typeArgs;
    while (typeEl) {
      accList.append(Variant(typeEl->vanillaName()));
      typeEl = typeEl->m_typeList;
    }
    rep.set(s_access_list, Variant(accList));
    break;
  }
  case TypeStructure::Kind::T_xhp:
    rep.set(s_classname, Variant(m_name));
    break;
  case TypeStructure::Kind::T_unresolved:
    rep.set(s_classname, Variant(m_name));
    if (m_typeArgs) {
      rep.set(s_generic_types, Variant(argsListToScalarArray(m_typeArgs)));
    }
    break;
  case TypeStructure::Kind::T_reifiedtype:
    rep.set(s_id, Variant(m_name));
    break;
  default:
    break;
  }

  if (!m_generics.empty()) {
    rep.set(s_typevars, Variant(m_generics));
  }

  rep.setEvalScalar();
  return rep;
}

}

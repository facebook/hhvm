/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/type-structure.h"

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
                                m_typeaccess(false) { }

std::string TypeAnnotation::vanillaName() const {
  // filter out types that should not be exposed to the runtime
  if (m_nullable || m_soft || m_typevar || m_function || m_typeaccess) {
    return "";
  }
  if (!strcasecmp(m_name.c_str(), "HH\\mixed") ||
      !strcasecmp(m_name.c_str(), "HH\\this")) {
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
  } else if (m_typeArgs) {
    genericTypeName(name);
  } else {
    name += m_name;
  }
  return name;
}

MaybeDataType TypeAnnotation::dataType() const {
  if (m_function || m_xhp || m_tuple) {
    return KindOfObject;
  }
  if (m_typeArgs) {
    return !strcasecmp(m_name.c_str(), "array") ? KindOfArray : KindOfObject;
  }
  if (m_nullable || m_soft) {
    return folly::none;
  }
  if (!strcasecmp(m_name.c_str(), "null") ||
      !strcasecmp(m_name.c_str(), "HH\\void")) {
    return KindOfNull;
  }
  if (!strcasecmp(m_name.c_str(), "HH\\bool"))     return KindOfBoolean;
  if (!strcasecmp(m_name.c_str(), "HH\\int"))      return KindOfInt64;
  if (!strcasecmp(m_name.c_str(), "HH\\float"))    return KindOfDouble;
  if (!strcasecmp(m_name.c_str(), "HH\\num"))      return folly::none;
  if (!strcasecmp(m_name.c_str(), "HH\\arraykey")) return folly::none;
  if (!strcasecmp(m_name.c_str(), "HH\\string"))   return KindOfString;
  if (!strcasecmp(m_name.c_str(), "array"))        return KindOfArray;
  if (!strcasecmp(m_name.c_str(), "HH\\resource")) return KindOfResource;
  if (!strcasecmp(m_name.c_str(), "HH\\mixed"))    return folly::none;

  return KindOfObject;
}

void TypeAnnotation::getAllSimpleNames(std::vector<std::string>& names) const {
  names.push_back(m_name);
  if (m_typeList) {
    m_typeList->getAllSimpleNames(names);
  } else if (m_typeArgs) {
    m_typeArgs->getAllSimpleNames(names);
  }
}

void TypeAnnotation::functionTypeName(std::string &name) const {
  name += "(function (";
  // return value of function types is the first element of type list
  TypeAnnotationPtr retType = m_typeArgs;
  TypeAnnotationPtr typeEl = m_typeArgs->m_typeList;
  bool hasArgs = (typeEl != nullptr);
  while (typeEl) {
    name += typeEl->fullName();
    typeEl = typeEl->m_typeList;
    name += ", ";
  }
  // replace the trailing ", " (if any) with "): "
  if (hasArgs) {
    name.replace(name.size() - 2, 2, "): ");
  } else {
    name += "): ";
  }
  // add function return value
  name += retType->fullName();
  name += ")";
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
  while (typeEl) {
    name += typeEl->fullName();
    typeEl = typeEl->m_typeList;
    name += ", ";
  }
  // replace the trailing ", " with ")"
  name.replace(name.size() - 2, 2, ")");
}

void TypeAnnotation::genericTypeName(std::string &name) const {
  name += m_name;
  name += "<";
  TypeAnnotationPtr typeEl = m_typeArgs;
  while (typeEl) {
    name += typeEl->fullName();
    typeEl = typeEl->m_typeList;
    name += ", ";
  }
  // replace the trailing ", " with ">"
  name.replace(name.size() - 2, 2, ">");
}

void TypeAnnotation::accessTypeName(std::string &name) const {
  name += m_name;
  TypeAnnotationPtr typeEl = m_typeArgs;
  while (typeEl) {
    name += "::";
    name += typeEl->fullName();
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

void TypeAnnotation::outputCodeModel(CodeGenerator& cg) {
  TypeAnnotationPtr typeArgsElem = m_typeArgs;
  auto numTypeArgs = this->numTypeArgs();
  auto numProps = 1;
  if (m_nullable) numProps++;
  if (m_soft) numProps++;
  if (m_function) {
    numProps++;
    // Since this is a function type, the first type argument is the return type
    // and no typeArguments property will be serialized unless there are at
    // least two type arguments.
    if (numTypeArgs > 1) numProps++;
  } else {
    if (numTypeArgs > 0) numProps++;
  }
  cg.printObjectHeader("TypeExpression", numProps);
  cg.printPropertyHeader("name");
  cg.printValue(m_tuple ? "tuple" : m_name);
  if (m_nullable) {
    cg.printPropertyHeader("isNullable");
    cg.printBool(true);
  }
  if (m_soft) {
    cg.printPropertyHeader("isSoft");
    cg.printBool(true);
  }
  if (m_function) {
    cg.printPropertyHeader("returnType");
    typeArgsElem->outputCodeModel(cg);
    typeArgsElem = typeArgsElem->m_typeList;
    // Since we've grabbed the first element of the list as the return
    // type, make sure that the logic for serializing type arguments gets
    // disabled unless there is at least one more type argument.
    numTypeArgs--;
  }
  if (numTypeArgs > 0) {
    cg.printPropertyHeader("typeArguments");
    cg.printf("V:9:\"HH\\Vector\":%d:{", numTypeArgs);
    while (typeArgsElem != nullptr) {
      typeArgsElem->outputCodeModel(cg);
      typeArgsElem = typeArgsElem->m_typeList;
    }
    cg.printf("}");
  }
  cg.printObjectFooter();
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
  if (m_tuple) {
    return TypeStructure::Kind::T_tuple;
  }
  if (m_function) {
    return TypeStructure::Kind::T_fun;
  }
  if (!strcasecmp(m_name.c_str(), "array")) {
    return TypeStructure::Kind::T_array;
  }
  // TODO(7657570): support shapes
  if (m_typevar) {
    return TypeStructure::Kind::T_typevar;
  }
  if (isThis()) {
    return TypeStructure::Kind::T_this;
  }
  if (m_typeaccess) {
    return TypeStructure::Kind::T_typeaccess;
  }
  if (m_xhp) {
    // TODO(7657500): in the runtime, resolve this type to a class.
    return TypeStructure::Kind::T_xhp;
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
  s_access_list("access_list")
;

/* Turns the argsList linked list of TypeAnnotation into a positioned
 * static array. */
ArrayData* TypeAnnotation::argsListToScalarArray(TypeAnnotationPtr ta) const {
  int i = 0;
  auto typeargs = Array::Create();

  auto typeEl = ta;
  while (typeEl) {
    typeargs.add(i, Variant(typeEl->getScalarArrayRep()));
    ++i;
    typeEl = typeEl->m_typeList;
  }
  return ArrayData::GetScalarArray(typeargs.get());
}

ArrayData* TypeAnnotation::getScalarArrayRep() const {
  auto rep = Array::Create();

  bool nullable = (bool) m_nullable;
  if (nullable) {
    rep.add(s_nullable, Variant(nullable));
  }

  TypeStructure::Kind kind = getKind();
  rep.add(s_kind, Variant(static_cast<uint8_t>(kind)));

  switch (kind) {
  case TypeStructure::Kind::T_tuple:
    assert(m_typeArgs);
    rep.add(s_elem_types, Variant(argsListToScalarArray(m_typeArgs)));
    break;
  case TypeStructure::Kind::T_fun:
    assert(m_typeArgs);
    // return type is the first of the typeArgs
    rep.add(s_return_type, Variant(m_typeArgs->getScalarArrayRep()));
    rep.add(s_param_types,
            Variant(argsListToScalarArray(m_typeArgs->m_typeList)));
    break;
  case TypeStructure::Kind::T_array:
    rep.add(s_generic_types, Variant(argsListToScalarArray(m_typeArgs)));
    break;
  case TypeStructure::Kind::T_typevar:
    rep.add(s_name, Variant(m_name));
    break;
  case TypeStructure::Kind::T_typeaccess: {
    // for now, only store the vanilla names (strings) as part of the
    // access list
    rep.add(s_root_name, Variant(m_name));
    auto accList = Array::Create();
    auto typeEl = m_typeArgs;
    int i = 0;
    while (typeEl) {
      accList.add(i, Variant(typeEl->vanillaName()));
      ++i;
      typeEl = typeEl->m_typeList;
    }
    accList = ArrayData::GetScalarArray(accList.get());
    rep.add(s_access_list, Variant(accList));
    break;
  }
  case TypeStructure::Kind::T_xhp:
    rep.add(s_classname, Variant(m_name));
    break;
  case TypeStructure::Kind::T_unresolved:
    rep.add(s_classname, Variant(m_name));
    rep.add(s_generic_types, Variant(argsListToScalarArray(m_typeArgs)));
    break;
  default:
    break;
  }

  return ArrayData::GetScalarArray(rep.get());
}

}

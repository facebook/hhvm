/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/util.h"

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
                                m_typevar(false) { }

std::string TypeAnnotation::vanillaName() const {
  // filter out types that should not be exposed to the runtime
  if (m_nullable || m_soft || m_typevar || m_function) {
    return "";
  }
  if (!m_name.compare("mixed") || !m_name.compare("this")) {
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

DataType TypeAnnotation::dataType(bool expectedType /*= false */) const {
  if (m_function || m_xhp || m_tuple) {
    return KindOfObject;
  }
  if (m_typeArgs) {
    return !m_name.compare("array") ? KindOfArray : KindOfObject;
  }
  if (!expectedType && (m_nullable || m_soft)) {
    return KindOfUnknown;
  }
  if (!m_name.compare("null") || !m_name.compare("void")) {
    return KindOfNull;
  }
  if (!m_name.compare("bool"))     return KindOfBoolean;
  if (!m_name.compare("int"))      return KindOfInt64;
  if (!m_name.compare("float"))    return KindOfDouble;
  if (!m_name.compare("string"))   return KindOfString;
  if (!m_name.compare("array"))    return KindOfArray;
  if (!m_name.compare("resource")) return KindOfResource;
  if (!m_name.compare("mixed"))    return KindOfUnknown;

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
  Util::replaceAll(name, "__", ":");
  Util::replaceAll(name, "_", "-");
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
  auto numTypeArgs = 0;
  while (typeArgsElem != nullptr) {
    numTypeArgs++;
    typeArgsElem = typeArgsElem->m_typeList;
  }
  typeArgsElem = m_typeArgs;

  auto numProps = 3;
  if (m_function) numProps++;
  if (numTypeArgs > 0) numProps++;
  cg.printObjectHeader("TypeExpression", numProps);
  cg.printPropertyHeader("name");
  cg.printValue(m_tuple ? "tuple" : m_name);
  cg.printPropertyHeader("isNullable");
  cg.printBool((bool)m_nullable);
  cg.printPropertyHeader("isSoft");
  cg.printBool((bool)m_soft);
  if (m_function) {
    cg.printPropertyHeader("returnType");
    typeArgsElem->outputCodeModel(cg);
    typeArgsElem = typeArgsElem->m_typeList;
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

}

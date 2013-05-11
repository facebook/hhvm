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

#include <compiler/type_annotation.h>
#include <util/util.h>

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
                                m_typevar(false) {}

const std::string TypeAnnotation::simpleName() const {
  // filter out types that should not be exposed to the runtime
  if (m_nullable || m_soft || m_typevar || m_function) {
    return "";
  }
  if (!m_name.compare("mixed") || !m_name.compare("this")) {
    return "";
  }
  return m_name;
}

const std::string TypeAnnotation::fullName() const {
  std::string name;
  if (m_nullable) {
    name += '?';
  } else if (m_soft) {
    name += '@';
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

void TypeAnnotation::functionTypeName(std::string &name) const {
  name += "(function (";
  // return value of function types is the first element of type list
  TypeAnnotationPtr retType = m_typeArgs;
  TypeAnnotationPtr typeEl = m_typeArgs->m_typeList;
  bool hasArgs = typeEl;
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
void TypeAnnotation::xhpTypeName(std::string &name) const {
  // remove prefix if any
  if (m_name.compare(0, sizeof("xhp_xhp__") - 1, "xhp_xhp__") == 0) {
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

}

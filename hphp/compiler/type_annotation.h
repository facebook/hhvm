/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __TYPE_ANNOTATION_H__
#define __TYPE_ANNOTATION_H__

#include <util/base.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(TypeAnnotation);

/**
 * A class that represents a type annotation. Type annotations are used
 * for arguments in function/method definitions and class fields.
 * For simple types (e.g. int, string, C, etc.) m_typeArgs and m_typeList are
 * null and the name is all we need.
 * For "composite" types the m_typeArgs is set to point to the type list.
 * The type list is a an instance of this class with m_typeList pointing to
 * the next type (linked list). So for the type
 * Map<int, string>
 * the representation would be
 *
 *  __________________
 * | m_name("Map")    |     __________________
 * | m_typeArgs       |--> | m_name("int")    |
 * | m_typeList(null) |    | m_typeArgs(null) |     ___________________
 * |__________________|    | m_typeList       |--> | m_name("string")  |
 *                         |__________________|    | m_typeArgs(null)  |
 *                                                 | m_typeList(null)  |
 *                                                 |___________________|
 *
 * Map<int, Vector<string>> is
 *
 *  __________________
 * | m_name("Map")    |     __________________
 * | m_typeArgs       |--> | m_name("int")    |
 * | m_typeList(null) |    | m_typeArgs(null) |     ___________________
 * |__________________|    | m_typeList       |--> | m_name("Vector")  |
 *                         |__________________|    | m_typeArgs        |---
 *                                                 | m_typeList(null)  |  |
 *                                                 |___________________|  |
 *                                                                        |
 *                                                                        |
 *                                                __________________      |
 *                                               | m_name(string)   |  <--
 *                                               | m_typeArgs(null) |
 *                                               | m_typeList(null) |
 *                                               |__________________|
 */
class TypeAnnotation {
public:
  TypeAnnotation(const std::string &name, TypeAnnotationPtr typeArgs);

  void setNullable() { m_nullable = true; }
  void setSoft() { m_soft = true; }
  void setTuple() { m_tuple = true; }
  void setFunction() { m_function = true; }
  void setXHP() { m_xhp = true; }
  void setTypeVar() { m_typevar = true; }

  const std::string simpleName() const;
  const std::string fullName() const;

  void appendToTypeList(TypeAnnotationPtr typeList);

private:
  void functionTypeName(std::string &name) const;
  void xhpTypeName(std::string &name) const;
  void tupleTypeName(std::string &name) const;
  void genericTypeName(std::string &name) const;

private:
  std::string m_name;
  TypeAnnotationPtr m_typeArgs;
  TypeAnnotationPtr m_typeList;
  unsigned m_nullable : 1;
  unsigned m_soft : 1;
  unsigned m_tuple : 1;
  unsigned m_function : 1;
  unsigned m_xhp : 1;
  unsigned m_typevar : 1;
};

}

#endif


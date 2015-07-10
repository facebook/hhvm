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

#ifndef incl_HPHP_TYPEANNOTATION_H_
#define incl_HPHP_TYPEANNOTATION_H_

#include <cstring>
#include <string>
#include <vector>

#include "hphp/runtime/base/datatype.h"

#include "hphp/util/deprecated/declare-boost-types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class CodeGenerator;

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
  void setTypeAccess() { m_typeaccess = true; }

  bool isNullable() const { return m_nullable; }
  bool isSoft() const { return m_soft; }
  bool isTuple() const { return m_tuple; }
  bool isFunction() const { return m_function; }
  bool isXHP() const { return m_xhp; }
  bool isTypeVar() const { return m_typevar; }
  bool isTypeAccess() const { return m_typeaccess; }

  /*
   * Return a shallow copy of this TypeAnnotation, except with
   * nullability stripped.
   */
  TypeAnnotation stripNullable() const {
    auto ret = *this;
    ret.m_nullable = false;
    return ret;
  }

  /*
   * Return a shallow copy of this TypeAnnotation, except with
   * softness stripped.
   */
  TypeAnnotation stripSoft() const {
    auto ret = *this;
    ret.m_soft = false;
    return ret;
  }

  bool isMixed() const { return !strcasecmp(m_name.c_str(), "HH\\mixed"); }

  bool isVoid() const { return !strcasecmp(m_name.c_str(), "HH\\void"); }

  bool isThis() const { return !strcasecmp(m_name.c_str(), "HH\\this"); }

  bool isAwaitable() const {
    return !strcasecmp(m_name.c_str(), "HH\\Awaitable");
  }

  bool isWaitHandle() const {
    return !strcasecmp(m_name.c_str(), "WaitHandle") ||
           !strcasecmp(m_name.c_str(), "HH\\WaitHandle");
  }

  /*
   * Returns whether this TypeAnnotation is "simple"---as described
   * above, this implies it has only one level of depth.  Both the
   * type list and type args are null.
   *
   * It may however be soft or nullable, or a function type, etc.
   */
  bool isSimple() const { return !m_typeList && !m_typeArgs; }

  /*
   * Return a string for this annotation that is a type hint for
   * normal "vanilla" php.  This means <?hh-specific annotations (such
   * as ?Foo or @Foo) are going to be stripped, as well as the deep
   * information about a type.  (E.g. for Vector<string> this will
   * return "Vector".)
   */
  std::string vanillaName() const;

  /*
   * Returns a complete string name of this type-annotation, including
   * <?hh-specific extensions, any type parameter list, etc.
   */
  std::string fullName() const;

  /*
   * Fill the vector in input with all the types used in this annotation
   * as simple names.
   * Vector<Map<string, int>> would return
   * [Vector, Map, string, int]
   */
  void getAllSimpleNames(std::vector<std::string>& names) const;

  /*
   * Add a new element to this type list for this TypeAnnotation.
   */
  void appendToTypeList(TypeAnnotationPtr typeList);

  /*
   * Root datatype; ignores inner types for generics.
   *
   * For nullable or soft types, folly::none will be returned since the
   * annotation could represent more than one type.
   */
  MaybeDataType dataType() const;

  /*
   *  Serializes the type annotation using the given CodeGenerator.
   */
  void outputCodeModel(CodeGenerator& cg);

  int numTypeArgs() const;
  TypeAnnotationPtr getTypeArg(int n) const;

private:
  void functionTypeName(std::string &name) const;
  void xhpTypeName(std::string &name) const;
  void tupleTypeName(std::string &name) const;
  void genericTypeName(std::string &name) const;
  void accessTypeName(std::string &name) const;

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
  unsigned m_typeaccess : 1;
};

}

#endif

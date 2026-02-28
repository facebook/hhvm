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

#pragma once

#include <hphp/runtime/base/string-data.h>
#include <hphp/runtime/vm/preclass.h>

#include <hphp/util/hash-map.h>

#include <functional>
#include <list>
#include <string>
#include <unordered_set>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Data used for trait method importing.
 *
 * Handles method order as well as trait alias and precedence resolution.
 *
 * The TraitMethod parameter class is expected to expose the following types
 * and members:
 *
 * TraitMethod {
 *    typename class_type;
 *    typename method_type;
 *    typename origin_type;
 *
 *    class_type trait;
 *    method_type method;
 *    Attr modifiers;
 * }
 *
 * The Ops parameter class is expected to expose a number of static member
 * functions:
 *
 * Ops {
 *    // Return the name for a trait class/method.
 *    String clsName(TraitMethod::class_type traitCls);
 *
 *    // Return the reference of the trait where the method was
 *    // originally defined in the bytecode.
 *    origin_type originalClass(TraitMethod::method_type meth);
 *
 *    // Is-a methods.
 *    bool isAbstract(Attr modifiers);
 *
 *    // Whether to exclude methods with name `methName' when adding.
 *    bool exclude(const String& methName);
 *
 *    // Errors.
 *    void errorDuplicateMethod(Context ctx, const String& methName,
 *                              const std::list<TraitMethod>& methods);
 * }
 */
template <class TraitMethod, class Ops>
struct TraitMethodImportData {
  /////////////////////////////////////////////////////////////////////////////
  // Types.

  using String = const StringData*;

  /*
   * Data associated with a given trait name.
   */
  struct NameData {
    // List of all declared trait methods with the name.
    std::list<TraitMethod> methods;
    // For error reporting, list of names of traits that declare
    // methods with the name Includes duplicates that might have been
    // removed from methods.
    std::vector<String> methodOriginsWithDuplicates;
  };

  /*
   * Information associated with an imported method---essentially a (name,
   * method) pair.
   */
  struct MethodData {
    String name;
    TraitMethod tm;
  };


  /////////////////////////////////////////////////////////////////////////////
  // Public API.

  /*
   * Add a trait method to the import data set.
   *
   * The add() methods should be called in the order in which the trait methods
   * were included in the importing class.
   */
  void add(const TraitMethod& tm, const String& name);

  /*
   * Declare that all imports have been added---and that all rules have been
   * applied---and return an ordered vector of all imported (name, TraitMethod)
   * pairs.
   *
   * Continued use of `this' after calling finish() is undefined.
   */
  std::vector<MethodData> finish(typename TraitMethod::class_type ctx,
                                 bool enableMethodTraitDiamond);

  /////////////////////////////////////////////////////////////////////////////
  // Internals.

private:
  void removeSpareTraitAbstractMethods();
  void removeDiamondDuplicates(bool enableMethodTraitDiamond);

  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  // Map from trait method name to NameData.
  hphp_fast_map<String, NameData,
                string_data_hash, string_data_same> m_dataForName;

  // Method names in order of first declaration.
  std::vector<String> m_orderedNames;
};

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/runtime/vm/trait-method-import-data-inl.h"

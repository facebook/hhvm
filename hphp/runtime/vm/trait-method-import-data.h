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

#ifndef incl_HPHP_TRAIT_METHOD_IMPORT_DATA_H_
#define incl_HPHP_TRAIT_METHOD_IMPORT_DATA_H_

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
 *    String methName(TraitMethod::method_type meth);
 *
 *    // Is-a methods.
 *    bool isTrait(TraitMethod::class_type traitCls);
 *    bool isAbstract(Attr modifiers);
 *
 *    bool isAsync(TraitMethod::method_type meth);
 *    bool isStatic(TraitMethod::method_type meth);
 *    bool isFinal(TraitMethod::method_type meth);
 *
 *    // Whether to exclude methods with name `methName' when adding.
 *    bool exclude(const String& methName);
 *
 *    // TraitMethod constructor.
 *    TraitMethod traitMethod(TraitMethod::class_type traitCls,
 *                            TraitMethod::method_type traitMeth,
 *                            alias_rule rule);
 *
 *    // Register a trait alias once the trait class is found.
 *    void addTraitAlias(Context ctx, const PreClass::TraitAliasRule& rule,
 *                       TraitMethod::class_type traitCls);
 *
 *    // Trait class/method finders.
 *    TraitMethod::class_type
 *      findSingleTraitWithMethod(Context ctx, const String& origMethName);
 *    TraitMethod::class_type
 *      findTraitClass(Context ctx, const String& traitName);
 *    TraitMethod::method_type
 *      findTraitMethod(TraitMethod::class_type traitCls,
 *                      const String& origMethName);
 *
 *    // Errors.
 *    void errorUnknownMethod(const String& methName);
 *    void errorUnknownTrait(const String& traitName);
 *    void errorDuplicateMethod(Context ctx, const String& methName,
 *                              const std::list<TraitMethod>& methods);
 *    void errorInconsistentInsteadOf(Context ctx, const String& methName);
 *    void errorMultiplyExcluded(const String& traitName,
 *                               const String& methName);
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
    // In-order aliases of the name.
    std::vector<String> aliases;
  };

  /*
   * Information associated with an imported method---essentially a (name,
   * method) pair.
   */
  struct MethodData {
    const String name;
    const TraitMethod tm;
  };


  /////////////////////////////////////////////////////////////////////////////
  // Public API.

  /*
   * Get a reference to the list of trait method names.
   */
  const std::vector<String>& methodNames() { return m_orderedNames; }

  /*
   * Add a trait method to the import data set.
   *
   * The add() methods should be called in the order in which the trait methods
   * were included in the importing class.
   */
  void add(const TraitMethod& tm, const String& name);
  void add(const TraitMethod& tm,
           const String& aliasedName,
           const String& origName);

  /*
   * Erase all records of a trait method called `name'.
   */
  void erase(const String& name);

  /*
   * Set rule modifiers on the trait declared on `trait' called `name' if it
   * exists.
   */
  void setModifiers(const String& name,
                    typename TraitMethod::class_type trait,
                    Attr mods);

  /*
   * Apply a precedence ("insteadof") rule to the import data set.
   */
  void applyPrecRule(const PreClass::TraitPrecRule& rule,
                     typename TraitMethod::class_type ctx);

  /*
   * Apply all alias ("as") rules to the import data set.
   */
  template<typename Iter>
  void applyAliasRules(Iter it_begin,
                       Iter it_end,
                       typename TraitMethod::class_type ctx);

  /*
   * Declare that all imports have been added---and that all rules have been
   * applied---and return an ordered vector of all imported (name, TraitMethod)
   * pairs.
   *
   * Continued use of `this' after calling finish() is undefined.
   */
  auto finish(typename TraitMethod::class_type ctx);


  /////////////////////////////////////////////////////////////////////////////
  // Internals.

private:
  void removeSpareTraitAbstractMethods();


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  // Map from trait method name to NameData.
  hphp_fast_map<String, NameData,
                string_data_hash, string_data_isame> m_dataForName;

  // Method names in order of first declaration.
  std::vector<String> m_orderedNames;
};

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/runtime/vm/trait-method-import-data-inl.h"

#endif

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

#ifndef incl_HPHP_CONSTANT_TABLE_H_
#define incl_HPHP_CONSTANT_TABLE_H_

#include "hphp/compiler/analysis/symbol_table.h"
#include "hphp/compiler/analysis/block_scope.h"

#include <vector>
#include <boost/container/flat_set.hpp>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(CodeError);
DECLARE_BOOST_TYPES(ConstantTable);
DECLARE_BOOST_TYPES(ClassScope);

/**
 * These are the only places that a new constant can be declared:
 *
 *   const T_STRING = static_scalar
 *   class { const T_STRING = static_scalar,...}
 *   define('NAME', static_scalar)
 */
struct ConstantTable : SymbolTable {
  explicit ConstantTable(BlockScope &blockScope);

  /**
   * Whether defining something to be non-scalar value or redeclared, or
   * marked up by "Dynamic" note.
   */
  bool isDynamic(const std::string &name) const {
    const Symbol *sym = getSymbol(name);
    return sym && sym->isDynamic();
  }

  bool hasDynamic() const { return m_hasDynamic; }

  /**
   * Explicitly setting a constant to be dynamic, mainly for "Dynamic" note.
   */
  void setDynamic(AnalysisResultConstRawPtr ar, const std::string &name);
  void setDynamic(AnalysisResultConstRawPtr ar, Symbol* sym);

  /**
   * Called when a constant is declared (l-value).
   */
  void add(const std::string &name, ExpressionPtr exp,
           AnalysisResultConstRawPtr ar, ConstructPtr construct);

  /**
   * Called after a constants value is determined
   */
  void setValue(AnalysisResultConstRawPtr ar, const std::string &name,
                ExpressionPtr value);

  /**
   * Generate all constant declarations for this symbol table.
   */
  void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);

  bool isRecursivelyDeclared(AnalysisResultConstRawPtr ar,
                             const std::string &name) const;
  ConstructPtr getValueRecur(AnalysisResultConstRawPtr ar,
                             const std::string &name,
                             ClassScopePtr &defClass) const;
  ConstructPtr getDeclarationRecur(AnalysisResultConstRawPtr ar,
                                   const std::string &name,
                                   ClassScopePtr &defClass) const;

  void cleanupForError(AnalysisResultConstRawPtr ar);

  using ClassConstantSet = boost::container::flat_set<
    std::pair<ClassScopePtr, std::string>
  >;
  using DependencyMap = std::map<Symbol*, ClassConstantSet>;

  void recordDependency(Symbol* sym, ClassScopePtr cls, std::string name);

  const ClassConstantSet& lookupDependencies(const std::string&);

  bool hasDependencies() const { return m_hasDependencies; }
  const DependencyMap& getDependencies() const { return m_dependencies; }
private:
  bool m_hasDynamic;
  bool m_hasDependencies;

  ClassScopePtr findParent(AnalysisResultConstRawPtr ar,
                           const std::string &name) const;
  ClassScopeRawPtr findBase(AnalysisResultConstRawPtr ar,
                            const std::string &name,
                            const std::vector<std::string> &bases) const;

  DependencyMap m_dependencies;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CONSTANT_TABLE_H_

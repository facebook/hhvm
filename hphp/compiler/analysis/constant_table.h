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

#ifndef incl_HPHP_CONSTANT_TABLE_H_
#define incl_HPHP_CONSTANT_TABLE_H_

#include "hphp/compiler/analysis/symbol_table.h"
#include "hphp/compiler/analysis/block_scope.h"

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
class ConstantTable : public SymbolTable {
public:
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
  void setDynamic(AnalysisResultConstPtr ar, const std::string &name,
                  bool forceVariant);

  /**
   * Called when a constant is declared (l-value).
   */
  TypePtr add(const std::string &name, TypePtr type, ExpressionPtr exp,
              AnalysisResultConstPtr ar, ConstructPtr construct);

  /**
   * Called after a constant is type-inferred
   */
  void setValue(AnalysisResultConstPtr ar, const std::string &name,
                ExpressionPtr value);

  /**
   * Called when a constant is used or being evaluated (r-value).
   */
  TypePtr check(BlockScopeRawPtr context,
                const std::string &name, TypePtr type, bool coerce,
                AnalysisResultConstPtr ar, ConstructPtr construct,
                const std::vector<std::string> &bases,
                BlockScope *&defScope);

  /**
   * Generate all constant declarations for this symbol table.
   */
  void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);

  bool isRecursivelyDeclared(AnalysisResultConstPtr ar,
                             const std::string &name) const;
  ConstructPtr getValueRecur(AnalysisResultConstPtr ar, const std::string &name,
                             ClassScopePtr &defClass) const;
  ConstructPtr getDeclarationRecur(AnalysisResultConstPtr ar,
                                   const std::string &name,
                                   ClassScopePtr &defClass) const;

  void cleanupForError(AnalysisResultConstPtr ar);
private:
  bool m_hasDynamic;

  ClassScopePtr findParent(AnalysisResultConstPtr ar,
                           const std::string &name) const;
  ClassScopeRawPtr findBase(AnalysisResultConstPtr ar,
                            const std::string &name,
                            const std::vector<std::string> &bases) const;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CONSTANT_TABLE_H_

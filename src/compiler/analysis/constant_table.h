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

#ifndef __CONSTANT_TABLE_H__
#define __CONSTANT_TABLE_H__

#include <compiler/analysis/symbol_table.h>
#include <compiler/analysis/block_scope.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(CodeError);
DECLARE_BOOST_TYPES(ConstantTable);
DECLARE_BOOST_TYPES(ClassScope);

/**
 * These are the only places that a new constant can be declared:
 *
 *   class_constant_declaration: class { const T_STRING = static_scalar,...}
 *   define('NAME', static_scalar)
 */
class ConstantTable : public SymbolTable {
public:
  ConstantTable(BlockScope &blockScope);

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
  void setDynamic(AnalysisResultConstPtr ar, const std::string &name);

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
  TypePtr check(const std::string &name, TypePtr type, bool coerce,
                AnalysisResultConstPtr ar, ConstructPtr construct,
                const std::vector<std::string> &bases,
                BlockScope *&defScope);

  /**
   * Generate all constant declarations for this symbol table.
   */
  void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                 bool newline = true) const;
  void outputCPPDynamicDecl(CodeGenerator &cg, AnalysisResultPtr ar,
                            Type2SymbolListMap &type2names);
  void outputCPPDynamicImpl(CodeGenerator &cg, AnalysisResultPtr ar);
  bool outputSingleConstant(CodeGenerator &cg, AnalysisResultPtr ar,
                            const std::string &name) const;

  void collectCPPGlobalSymbols(StringPairVec &symbols, CodeGenerator &cg,
                               AnalysisResultPtr ar);

  /**
   * Only used by redeclared classes
   */
  void outputCPPJumpTable(CodeGenerator &cg, AnalysisResultPtr ar,
                          bool needsGlobals, bool ret);

  /**
   * Generate all class constants in class info map.
   */
  void outputCPPClassMap(CodeGenerator &cg, AnalysisResultPtr ar,
                         bool last = true);

  /**
   * Whether or not we need to generate a jump table.
   */
  bool hasJumpTable() const { return !m_emptyJumpTable;}

  bool isRecursivelyDeclared(AnalysisResultConstPtr ar,
                             const std::string &name);
  ConstructPtr getValueRecur(AnalysisResultConstPtr ar, const std::string &name,
                             ClassScopePtr &defClass);
  ConstructPtr getDeclarationRecur(AnalysisResultConstPtr ar,
                                   const std::string &name,
                                   ClassScopePtr &defClass);

private:
  bool m_emptyJumpTable;
  bool m_hasDynamic;

  ClassScopePtr findParent(AnalysisResultConstPtr ar, const std::string &name);
  bool outputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                 const Symbol *sym) const;
  void outputCPPConstantSymbol(CodeGenerator &cg, AnalysisResultPtr ar,
                               Symbol *sym);

  TypePtr checkBases(const std::string &name, TypePtr type,
                     bool coerce, AnalysisResultConstPtr ar,
                     ConstructPtr construct,
                     const std::vector<std::string> &bases,
                     BlockScope *&defScope);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __CONSTANT_TABLE_H__

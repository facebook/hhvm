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

#ifndef incl_HPHP_VARIABLE_TABLE_H_
#define incl_HPHP_VARIABLE_TABLE_H_

#include "hphp/compiler/analysis/symbol_table.h"
#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/analysis/class_scope.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ModifierExpression);
DECLARE_BOOST_TYPES(CodeError);
DECLARE_BOOST_TYPES(VariableTable);
DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(FunctionScope);

/**
 * These are the only places that a new variable can be declared:
 *
 *   variable = expr|variable|new obj(...)
 *   static_var_list: T_STATIC T_VARIABLE = static_scalar,...
 *   class_variable_declaration: class { T_VARIABLE = static_scalar,...}
 *   T_LIST (variable, T_LIST(...), ...) = ...
 *   try {...} catch (T obj) {...}
 *   extract(name_value_pair)
 */
class VariableTable : public SymbolTable {
  friend class AssignmentExpression;
public:
  enum Attribute {
    ContainsDynamicVariable = 1,
    ContainsLDynamicVariable = ContainsDynamicVariable | 2,
    ContainsExtract = 4,
    ContainsCompact = 8,
    InsideStaticStatement = 16,
    InsideGlobalStatement = 32,
    ForceGlobal = 64,
    ContainsUnset = 128,
    NeedGlobalPointer = 256,
    ContainsDynamicStatic  = 512,
    ContainsGetDefinedVars = 1024,
    ContainsDynamicFunctionCall = 2048,
  };

  enum JumpTableType {
    JumpReturn,
    JumpSet,
    JumpInitialized,
    JumpInitializedString,
    JumpIndex,
    JumpReturnString
  };

  enum JumpTableName {
    JumpTableGlobalGetImpl,
    JumpTableGlobalExists,
    JumpTableGlobalGetIndex,

    JumpTableLocalGetImpl,
    JumpTableLocalExists,
  };

  enum AlteredVarClass {
    NonPrivateNonStaticVars = 1,
    NonPrivateStaticVars = 2,
    PrivateNonStaticVars = 4,
    PrivateStaticVars = 8,

    AnyNonStaticVars = NonPrivateNonStaticVars | PrivateNonStaticVars,
    AnyStaticVars = NonPrivateStaticVars | PrivateStaticVars,
    AnyNonPrivateVars = NonPrivateNonStaticVars | NonPrivateStaticVars,
    AnyPrivateVars = PrivateNonStaticVars | PrivateStaticVars,

    AnyVars = AnyStaticVars | AnyNonStaticVars
  };

  static int GetVarClassMask(bool privates, bool statics) {
    return (statics ? 2 : 1) << (privates ? 2 : 0);
  }

  static int GetVarClassMaskForSym(const Symbol *sym) {
    return GetVarClassMask(sym->isPrivate(), sym->isStatic());
  }

public:
  explicit VariableTable(BlockScope &blockScope);

  /**
   * Get/set attributes.
   */
  void setAttribute(Attribute attr) { m_attribute |= attr;}
  void clearAttribute(Attribute attr) { m_attribute &= ~attr;}
  bool getAttribute(Attribute attr) const {
    return (m_attribute & attr) == attr;
  }

  bool isParameter(const std::string &name) const;
  bool isPublic(const std::string &name) const;
  bool isProtected(const std::string &name) const;
  bool isPrivate(const std::string &name) const;
  bool isStatic(const std::string &name) const;
  bool isGlobal(const std::string &name) const;
  bool isSuperGlobal(const std::string &name) const;
  bool isLocal(const std::string &name) const;
  bool isLocal(const Symbol *sym) const;
  bool isRedeclared(const std::string &name) const;
  bool isLocalGlobal(const std::string &name) const;
  bool isNestedStatic(const std::string &name) const;
  bool isLvalParam(const std::string &name) const;
  bool isUsed(const std::string &name) const;
  bool isNeeded(const std::string &name) const;

  bool needLocalCopy(const Symbol *sym) const;
  bool needLocalCopy(const std::string &name) const;
  bool needGlobalPointer() const;
  bool isPseudoMainTable() const;
  bool hasPrivate() const;
  bool hasNonStaticPrivate() const;
  bool hasStatic() const { return m_hasStatic; }

  virtual bool isInherited(const std::string &name) const;

  void getLocalVariableNames(std::vector<std::string> &syms) const;

  /**
   * Get all variable's names.
   */
  void getNames(std::set<std::string> &names,
                bool collectPrivate = true) const;

  Symbol *addSymbol(const std::string &name) {
    return genSymbol(name, false);
  }

  Symbol *addDeclaredSymbol(const std::string &name, ConstructPtr construct) {
    return genSymbol(name, false, construct);
  }

  /**
   * Add a function's parameter to this table.
   */
  TypePtr addParam(const std::string &name, TypePtr type,
                   AnalysisResultConstPtr ar, ConstructPtr construct);

  TypePtr addParamLike(const std::string &name, TypePtr type,
                       AnalysisResultPtr ar, ConstructPtr construct,
                       bool firstPass);

  /**
   * Called when a variable is declared or being assigned (l-value).
   */
  TypePtr add(const std::string &name, TypePtr type, bool implicit,
              AnalysisResultConstPtr ar, ConstructPtr construct,
              ModifierExpressionPtr modifiers);
  TypePtr add(Symbol *sym, TypePtr type, bool implicit,
              AnalysisResultConstPtr ar, ConstructPtr construct,
              ModifierExpressionPtr modifiers);

  /**
   * Called to note whether a class variable overrides
   * a definition in a base class. Returns whether or not there
   * was an error in marking as override.
   */
  bool markOverride(AnalysisResultPtr ar, const std::string &name);

  /**
   * Called when a variable is used or being evaluated (r-value).
   */
  TypePtr checkVariable(const std::string &name, TypePtr type, bool coerce,
                        AnalysisResultConstPtr ar, ConstructPtr construct);
  TypePtr checkVariable(Symbol *sym, TypePtr type, bool coerce,
                        AnalysisResultConstPtr ar, ConstructPtr construct);
  /**
   * Find the class which contains the property, and return
   * its Symbol
   */
  Symbol *findProperty(ClassScopePtr &cls,
                       const std::string &name,
                       AnalysisResultConstPtr ar);

  /**
   * Caller is responsible for grabbing a lock on this class scope,
   * This function will be responsible for grabbing (and releasing)
   * a lock on the parent scope if necessary.
   */
  TypePtr checkProperty(BlockScopeRawPtr context,
                        Symbol *sym, TypePtr type,
                        bool coerce, AnalysisResultConstPtr ar);

  /**
   * Walk up to find first parent that has the specified symbol.
   */
  ClassScopePtr findParent(AnalysisResultConstPtr ar,
                           const std::string &name,
                           const Symbol *&sym) const;

  ClassScopePtr findParent(AnalysisResultConstPtr ar,
                           const std::string &name,
                           Symbol *&sym) {
    const Symbol *ss;
    ClassScopePtr p = findParent(ar, name, ss); // const version
    sym = const_cast<Symbol*>(ss);
    return p;
  }

  /**
   * Called when analyze global and static statement.
   */
  bool checkRedeclared(const std::string &name, Statement::KindOf kindOf);
  void addLocalGlobal(const std::string &name);
  void addNestedStatic(const std::string &name);

  /**
   * Helper for static variable default value
   */
  ConstructPtr getStaticInitVal(std::string varName);
  bool setStaticInitVal(std::string varName, ConstructPtr value);

  /**
   * Helper for class variable default value
   */
  ConstructPtr getClassInitVal(std::string varName);
  bool setClassInitVal(std::string varName, ConstructPtr value);

  /**
   * Called when analyze simple variable
   */
  void addLvalParam(const std::string &name);
  void addUsed(const std::string &name);
  bool checkUnused(Symbol *sym);
  void addNeeded(const std::string &name);
  void clearUsed();
  void addStaticVariable(Symbol *sym, AnalysisResultConstPtr ar,
                         bool member = false);
  void addStaticVariable(Symbol *sym, AnalysisResultPtr ar,
                         bool member = false);
  void cleanupForError(AnalysisResultConstPtr ar);

  /**
   * Set all matching variables to variants, since l-dynamic value was used.
   */
  void forceVariants(AnalysisResultConstPtr ar, int varClass,
                     bool recur = true);

  /**
   * Set one matching variable to be Type::Variant.
   */
  void forceVariant(AnalysisResultConstPtr ar, const std::string &name,
                    int varClass);

  /**
   * Keep track of $GLOBALS['var'].
   */
  void addSuperGlobal(const std::string &name);
  bool isConvertibleSuperGlobal(const std::string &name) const;

  /**
   * Canonicalize symbol order of static globals.
   */
  void canonicalizeStaticGlobals();

  /**
   * Generate all variable declarations for this symbol table.
   */
  void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);
  /**
   * Whether or not the specified jump table is empty.
   */
  bool hasAllJumpTables() const {
    return m_emptyJumpTables.empty();
  }
  bool hasJumpTable(JumpTableName name) const {
    return m_emptyJumpTables.find(name) == m_emptyJumpTables.end();
  }

  /**
   * These are static variables collected from different local scopes,
   * as they have to be turned into global variables defined in
   * GlobalVariables class to make ThreadLocal<GlobalVaribles> work.
   * This data structure is only needed by global scope.
   */
  DECLARE_BOOST_TYPES(StaticGlobalInfo);
  struct StaticGlobalInfo {
    Symbol *sym;
    VariableTable *variables; // where this variable was from
    ClassScopeRawPtr cls;     // these need to be raw to avoid reference cycles
    FunctionScopeRawPtr func;

    // get unique identifier for this variable
    static std::string GetId(ClassScopePtr cls,
                             FunctionScopePtr func, const std::string &name);
  };

  bool hasStaticLocals() const { return !m_staticLocalsVec.empty(); }

private:
  enum StaticSelection {
    NonStatic = 1,
    Static = 2,
    EitherStatic = 3
  };

  enum PrivateSelection {
    NonPrivate = 1,
    Private = 2,
    EitherPrivate = 3
  };

  int m_attribute;
  int m_nextParam;
  unsigned m_hasGlobal : 1;
  unsigned m_hasStatic : 1;
  unsigned m_hasPrivate : 1;
  unsigned m_hasNonStaticPrivate : 1;
  unsigned m_forcedVariants : 4;

  std::set<JumpTableName> m_emptyJumpTables;

  StaticGlobalInfoPtrVec m_staticGlobalsVec;
  StringToStaticGlobalInfoPtrMap m_staticGlobals;

  /** static symbols local to this variable table (ie for closures) */
  SymbolVec m_staticLocalsVec;

  bool isGlobalTable(AnalysisResultConstPtr ar) const;

  virtual TypePtr setType(AnalysisResultConstPtr ar, const std::string &name,
                          TypePtr type, bool coerce);
  virtual TypePtr setType(AnalysisResultConstPtr ar, Symbol *sym,
                          TypePtr type, bool coerce);
  virtual void dumpStats(std::map<std::string, int> &typeCounts);

  void checkSystemGVOrder(SymbolSet &variants, unsigned int max);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_VARIABLE_TABLE_H_

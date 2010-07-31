/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __VARIABLE_TABLE_H__
#define __VARIABLE_TABLE_H__

#include <compiler/analysis/symbol_table.h>
#include <compiler/statement/statement.h>
#include <compiler/hphp_unique.h>
#include <compiler/analysis/class_scope.h>

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
  friend class VariableTableHook;
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
  };

  enum VariableProperty {
    VariablePresent = 1,
    VariableStatic = 2,
    VariablePrivate = 4
  };

  enum JumpTableType {
    JumpReturn,
    JumpExists,
    JumpSet,
    JumpInitialized,
    JumpIndex,
    JumpReturnString,
    JumpReturnInit
  };

  enum JumpTableName {
    JumpTableGlobalGetImpl,
    JumpTableGlobalExists,
    JumpTableGlobalGetIndex,

    JumpTableLocalGetImpl,
    JumpTableLocalExists,

    JumpTableClassStaticGetInit,
    JumpTableClassStaticGet,
    JumpTableClassStaticLval,

    // this order is significant in outputCPPPropertyOp()
    JumpTableClassGetArray,
    JumpTableClassSetArray,
    JumpTableClassGet,
    JumpTableClassGetPublic,
    JumpTableClassGetPrivate,
    JumpTableClassExists,
    JumpTableClassExistsPublic,
    JumpTableClassExistsPrivate,
    JumpTableClassSet,
    JumpTableClassSetPublic,
    JumpTableClassSetPrivate,
    JumpTableClassLval,
    JumpTableClassLvalPublic,
    JumpTableClassLvalPrivate,
  };

public:
  VariableTable(BlockScope &blockScope);
  ~VariableTable();

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
  bool isRedeclared(const std::string &name) const;
  bool isLocalGlobal(const std::string &name) const;
  bool isNestedStatic(const std::string &name) const;
  bool isLvalParam(const std::string &name) const;
  bool isUsed(const std::string &name) const;
  bool isNeeded(const std::string &name) const;

  bool needLocalCopy(const std::string &name) const;
  bool needGlobalPointer() const;
  bool isPseudoMainTable() const;
  bool hasPrivate() const;

  virtual bool isInherited(const std::string &name) const;

  const char *getVariablePrefix(AnalysisResultPtr ar,
                                const std::string &name) const;
  std::string getVariableName(CodeGenerator &cg, AnalysisResultPtr ar,
                              const std::string &name) const;
  std::string getGlobalVariableName(CodeGenerator &cg, AnalysisResultPtr ar,
                                    const std::string &name) const;

  /**
   * Get all variable's names.
   */
  void getNames(std::set<std::string> &names,
                bool collectPrivate = true) const;

  /**
   * Add a function's parameter to this table.
   */
  TypePtr addParam(const std::string &name, TypePtr type,
                   AnalysisResultPtr ar, ConstructPtr construct);

  /**
   * Called when a variable is declared or being assigned (l-value).
   */
  TypePtr add(const std::string &name, TypePtr type, bool implicit,
              AnalysisResultPtr ar, ConstructPtr construct,
              ModifierExpressionPtr modifiers, bool checkError = true);

  /**
   * Called when a variable is used or being evaluated (r-value).
   */
  TypePtr checkVariable(const std::string &name, TypePtr type, bool coerce,
                        AnalysisResultPtr ar, ConstructPtr construct,
                        int &properties);

  /**
   * Called when a property is used or being evaluated (r-value).
   */
  TypePtr checkProperty(const std::string &name, TypePtr type, bool coerce,
                        AnalysisResultPtr ar, ConstructPtr construct,
                        int &properties);

  /**
   * Walk up to find first parent that has the specified symbol.
   */
  ClassScopePtr findParent(AnalysisResultPtr ar, const std::string &name);

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
  bool checkUnused(const std::string &name);
  void addNeeded(const std::string &name);
  void clearUsed();

  /**
   * Set all variables to variants, since l-dynamic value was used.
   */
  void forceVariants(AnalysisResultPtr ar);

  /**
   * Set one variable to be Type::Variant.
   */
  void forceVariant(AnalysisResultPtr ar, const std::string &name);

  /**
   * Keep track of $GLOBALS['var'].
   */
  void addSuperGlobal(const std::string &name);
  bool isConvertibleSuperGlobal(const std::string &name) const;

  /**
   * Generate all variable declarations for this symbol table.
   */
  void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPP(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPPropertyDecl(CodeGenerator &cg, AnalysisResultPtr ar,
      bool dynamicObject = false);
  void outputCPPPropertyClone(CodeGenerator &cg, AnalysisResultPtr ar,
                              bool dynamicObject = false);
  void outputCPPPropertyTable(CodeGenerator &cg, AnalysisResultPtr ar,
      const char *parent,
      ClassScope::Derivation dynamicObject = ClassScope::FromNormal);
  void outputCPPClassMap(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPStaticVariables(CodeGenerator &cg, AnalysisResultPtr ar);

  void outputCPPGlobalVariablesDtorIncludes(CodeGenerator &cg,
                                            AnalysisResultPtr ar);
  void outputCPPGlobalVariablesDtor(CodeGenerator &cg);
  void outputCPPGlobalVariablesGetImpl(CodeGenerator &cg,
                                       AnalysisResultPtr ar);
  void outputCPPGlobalVariablesExists(CodeGenerator &cg,
                                      AnalysisResultPtr ar);
  void outputCPPGlobalVariablesGetIndex(CodeGenerator &cg,
                                        AnalysisResultPtr ar);
  void outputCPPGlobalVariablesMethods(CodeGenerator &cg,
                                       AnalysisResultPtr ar);

  void collectCPPGlobalSymbols(StringPairVecVec &symbols, CodeGenerator &cg,
                               AnalysisResultPtr ar);

  void *getHookData() { return m_hookData;}
  static void setHookHandler(void (*hookHandler)(AnalysisResultPtr ar,
                                                 VariableTable *variables,
                                                 ExpressionPtr variable,
                                                 HphpHookUniqueId id)) {
    m_hookHandler = hookHandler;
  }

  /**
   * Whether or not the specified jump table is empty.
   */
  bool hasAllJumpTables() const {
    return m_emptyJumpTables.empty();
  }
  bool hasJumpTable(JumpTableName name) const {
    return m_emptyJumpTables.find(name) == m_emptyJumpTables.end();
  }

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
  std::map<std::string, int> m_parameters;
  std::set<std::string> m_protected;
  std::set<std::string> m_private;
  std::set<std::string> m_static;
  std::set<std::string> m_global;
  std::set<std::string> m_superGlobals;
  std::set<std::string> m_redeclared;
  std::set<std::string> m_localGlobal;  // the name occurred in a
                                        // global statement
  std::set<std::string> m_nestedStatic; // the name occurred in a
                                        // nested static statement
  std::set<std::string> m_lvalParam;    // the non-readonly parameters
  std::set<std::string> m_used;         // the used (referenced) variables
  std::set<std::string> m_needed;       // needed even though not referenced
  StringToConstructPtrMap m_staticInitVal; // static stmt variable init value
  StringToConstructPtrMap m_clsInitVal; // class variable init value

  std::set<JumpTableName> m_emptyJumpTables;

  /**
   * These are static variables collected from different local scopes,
   * as they have to be turned into global variables defined in
   * GlobalVariables class to make ThreadLocal<GlobalVaribles> work.
   * This data structure is only needed by global scope.
   */
  DECLARE_BOOST_TYPES(StaticGlobalInfo);
  struct StaticGlobalInfo {
    std::string name;
    VariableTable *variables; // where this variable was from
    ClassScopePtr cls;
    FunctionScopePtr func;

    // get unique identifier for this variable
    static std::string getName(ClassScopePtr cls,
                               FunctionScopePtr func, const std::string &name);
    static std::string getId(CodeGenerator &cg, ClassScopePtr cls,
                             FunctionScopePtr func, const std::string &name);
  };
  StringToStaticGlobalInfoPtrMap m_staticGlobals;

  bool m_allVariants;

  bool isGlobalTable(AnalysisResultPtr ar) const;

  void addStaticVariable(const std::string &name, AnalysisResultPtr ar,
                         bool member = false);

  virtual TypePtr setType(AnalysisResultPtr ar, const std::string &name,
                          TypePtr type, bool coerce);
  virtual void dumpStats(std::map<std::string, int> &typeCounts);

  bool definedByParent(AnalysisResultPtr ar, const std::string &name);

  void outputCPPGlobalVariablesHeader(CodeGenerator &cg,
                                      AnalysisResultPtr ar);
  void outputCPPGlobalVariablesImpl(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPVariableTable(CodeGenerator &cg, AnalysisResultPtr ar);
  bool outputCPPJumpTable(CodeGenerator &cg, AnalysisResultPtr ar,
                          const char *prefix, bool defineHash,
                          bool variantOnly, StaticSelection staticVar,
                          JumpTableType type = JumpReturn,
                          PrivateSelection privateVar = NonPrivate,
                          bool *declaredGlobals = NULL);
  bool outputCPPPrivateSelector(CodeGenerator &cg, AnalysisResultPtr ar,
                                const char *op, const char *args);
  void outputCPPPropertyOp(CodeGenerator &cg, AnalysisResultPtr ar,
      const char *cls, const char *parent, const char *op, const char *argsDec,
      const char *args, const char *ret, bool cnst, JumpTableType type,
      bool varOnly, ClassScope::Derivation dynamicObject,
      JumpTableName jtname);

  // hook
  static void (*m_hookHandler)(AnalysisResultPtr ar,
                               VariableTable *variables,
                               ExpressionPtr variable,
                               HphpHookUniqueId id);
  void *m_hookData;
};
}

///////////////////////////////////////////////////////////////////////////////

#endif // __VARIABLE_TABLE_H__

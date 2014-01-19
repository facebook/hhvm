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

#ifndef incl_HPHP_STATEMENT_H_
#define incl_HPHP_STATEMENT_H_

#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/analysis/label_scope.h"
#include <string>

#define STATEMENT_CONSTRUCTOR_BASE_PARAMETERS                           \
  BlockScopePtr scope, LabelScopePtr labelScope, LocationPtr loc, \
  Statement::KindOf kindOf
#define STATEMENT_CONSTRUCTOR_BASE_PARAMETER_VALUES                     \
  scope, labelScope, loc, kindOf
#define STATEMENT_CONSTRUCTOR_PARAMETERS                                \
  BlockScopePtr scope, LabelScopePtr labelScope, LocationPtr loc
#define STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(kindOf)                  \
  scope, labelScope, loc, Statement::KindOf##kindOf
#define DECLARE_BASE_STATEMENT_VIRTUAL_FUNCTIONS                        \
  virtual void analyzeProgram(AnalysisResultPtr ar);                    \
  virtual StatementPtr clone();                                         \
  virtual void inferTypes(AnalysisResultPtr ar);                        \
  virtual void outputCodeModel(CodeGenerator &cg);                      \
  virtual void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);
#define DECLARE_STATEMENT_VIRTUAL_FUNCTIONS                             \
  DECLARE_BASE_STATEMENT_VIRTUAL_FUNCTIONS;                             \
  virtual ConstructPtr getNthKid(int n) const;                          \
  virtual int getKidCount() const;                                      \
  virtual void setNthKid(int n, ConstructPtr cp)
#define NULL_STATEMENT()                                                \
  BlockStatementPtr(new BlockStatement(getScope(),          \
                                       getLabelScope(),     \
                                       getLocation(),       \
                                       StatementListPtr()))

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(LabelScope);

#define DECLARE_STATEMENT_TYPES(x)              \
    x(FunctionStatement),                       \
    x(ClassStatement),                          \
    x(InterfaceStatement),                      \
    x(ClassVariable),                           \
    x(ClassConstant),                           \
    x(MethodStatement),                         \
    x(StatementList),                           \
    x(BlockStatement),                          \
    x(IfBranchStatement),                       \
    x(IfStatement),                             \
    x(WhileStatement),                          \
    x(DoStatement),                             \
    x(ForStatement),                            \
    x(SwitchStatement),                         \
    x(CaseStatement),                           \
    x(BreakStatement),                          \
    x(ContinueStatement),                       \
    x(ReturnStatement),                         \
    x(GlobalStatement),                         \
    x(StaticStatement),                         \
    x(EchoStatement),                           \
    x(UnsetStatement),                          \
    x(ExpStatement),                            \
    x(ForEachStatement),                        \
    x(FinallyStatement),                        \
    x(CatchStatement),                          \
    x(TryStatement),                            \
    x(ThrowStatement),                          \
    x(GotoStatement),                           \
    x(LabelStatement),                          \
    x(UseTraitStatement),                       \
    x(TraitRequireStatement),                   \
    x(TraitPrecStatement),                      \
    x(TraitAliasStatement),                     \
    x(TypedefStatement)

class Statement : public Construct {
public:
#define DEC_STMT_ENUM(x) KindOf##x
  enum KindOf {
    DECLARE_STATEMENT_TYPES(DEC_STMT_ENUM)
    /* KindOfCount = 29 */
  };
  static const char *Names[];

protected:
  Statement(STATEMENT_CONSTRUCTOR_BASE_PARAMETERS);

public:

  /**
   * Type checking without RTTI.
   */
  bool is(KindOf type) const { return m_kindOf == type;}
  KindOf getKindOf() const { return m_kindOf;}

  /**
   * This is to avoid dynamic casting to StatementList in Parser.
   */
  virtual void addElement(StatementPtr stmt);
  virtual void insertElement(StatementPtr stmt, int index = 0);

  /**
   * Return a name for stats purpose.
   */
  virtual std::string getName() const { return "";}

  StatementPtr getNthStmt(int n) const {
    return dynamic_pointer_cast<Statement>(getNthKid(n));
  }

 /**
   * Called before type inference.
   */
  virtual StatementPtr preOptimize(AnalysisResultConstPtr ar) {
    return StatementPtr();
  }

  /**
   * Called after type inference.
   */
  virtual StatementPtr postOptimize(AnalysisResultConstPtr ar) {
    return StatementPtr();
  }

  /**
   * Called when types need to be inferred inside this statement.
   */
  virtual void inferTypes(AnalysisResultPtr ar) = 0;

  bool hasReachableLabel() const;

  virtual bool hasDecl() const { return false; }
  virtual bool hasImpl() const { return hasEffect(); }
  virtual bool hasBody() const { return true;}
  virtual bool hasRetExp() const { return false; }

  virtual StatementPtr clone() {
    assert(false);
    return StatementPtr();
  }

  virtual int getRecursiveCount() const { return 1; }

  LabelScopePtr getLabelScope() { return m_labelScope; }
  void setLabelScope(LabelScopePtr labelScope) { m_labelScope = labelScope; }

protected:
  KindOf m_kindOf;
  int m_silencerCountMax;
  int m_silencerCountCurrent;
  LabelScopePtr m_labelScope;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_STATEMENT_H_

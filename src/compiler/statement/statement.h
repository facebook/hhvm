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

#ifndef __STATEMENT_H__
#define __STATEMENT_H__

#include <compiler/expression/expression.h>

#define STATEMENT_CONSTRUCTOR_PARAMETERS        \
  LocationPtr loc, Statement::KindOf kindOf
#define STATEMENT_CONSTRUCTOR_PARAMETER_VALUES  \
  loc, kindOf
#define DECLARE_BASE_STATEMENT_VIRTUAL_FUNCTIONS                        \
  virtual void analyzeProgramImpl(AnalysisResultPtr ar);                \
  virtual StatementPtr clone();                                         \
  virtual StatementPtr preOptimize(AnalysisResultPtr ar);               \
  virtual StatementPtr postOptimize(AnalysisResultPtr ar);              \
  virtual void inferTypes(AnalysisResultPtr ar);                        \
  virtual void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);      \
  virtual void outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar);
#define DECLARE_STATEMENT_VIRTUAL_FUNCTIONS                             \
  DECLARE_BASE_STATEMENT_VIRTUAL_FUNCTIONS;                             \
  virtual ConstructPtr getNthKid(int n) const;                          \
  virtual int getKidCount() const;                                      \
  virtual void setNthKid(int n, ConstructPtr cp)
#define NULL_STATEMENT()                                                \
  BlockStatementPtr(new BlockStatement(getLocation(), KindOfBlockStatement, \
                                       StatementListPtr()))

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
DECLARE_BOOST_TYPES(Statement);

class Statement : public Construct {
public:
  enum KindOf {
    KindOfFunctionStatement,
    KindOfClassStatement,
    KindOfInterfaceStatement,
    KindOfClassVariable,
    KindOfClassConstant,
    KindOfMethodStatement,
    KindOfStatementList,
    KindOfBlockStatement,
    KindOfIfBranchStatement,
    KindOfIfStatement,
    KindOfWhileStatement,
    KindOfDoStatement,
    KindOfForStatement,
    KindOfSwitchStatement,
    KindOfCaseStatement,
    KindOfBreakStatement,
    KindOfContinueStatement,
    KindOfReturnStatement,
    KindOfGlobalStatement,
    KindOfStaticStatement,
    KindOfEchoStatement,
    KindOfUnsetStatement,
    KindOfExpStatement,
    KindOfForEachStatement,
    KindOfCatchStatement,
    KindOfTryStatement,
    KindOfThrowStatement,
    /* KindOfCount = 27 */
  };

public:
  Statement(STATEMENT_CONSTRUCTOR_PARAMETERS);

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

  StatementPtr getNthStmt(int n) {
    return boost::static_pointer_cast<Statement>(getNthKid(n));
  }

  virtual void analyzeProgram(AnalysisResultPtr ar);
  virtual void analyzeProgramImpl(AnalysisResultPtr ar) = 0;

  virtual void outputCPP(CodeGenerator &cg, AnalysisResultPtr ar);
  virtual void outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) = 0;

 /**
   * Called before type inference.
   */
  virtual StatementPtr preOptimize(AnalysisResultPtr ar) = 0;

  /**
   * Called after type inference.
   */
  virtual StatementPtr postOptimize(AnalysisResultPtr ar) = 0;

  /**
   * Called when types need to be inferred inside this statement.
   */
  virtual void inferTypes(AnalysisResultPtr ar) = 0;

  virtual bool hasDecl() const { return false; }
  virtual bool hasImpl() const { return hasEffect(); }
  virtual bool hasRetExp() const { return false; }

  virtual StatementPtr clone() {
    ASSERT(false);
    return StatementPtr();
  }

  virtual int getRecursiveCount() const { return 1; }

  int requireSilencers(int count);
  void endRequireSilencers(int old);
  int getSilencerCount();

protected:
  KindOf m_kindOf;
  int m_silencerCountMax;
  int m_silencerCountCurrent;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __STATEMENT_H__

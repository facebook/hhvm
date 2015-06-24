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

#ifndef incl_HPHP_STATEMENT_H_
#define incl_HPHP_STATEMENT_H_

#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/analysis/label_scope.h"
#include <string>

#define STATEMENT_CONSTRUCTOR_BASE_PARAMETERS                           \
  BlockScopePtr scope, LabelScopePtr labelScope,                        \
  const Location::Range& r, Construct::KindOf kindOf
#define STATEMENT_CONSTRUCTOR_BASE_PARAMETER_VALUES                     \
  scope, labelScope, r, kindOf
#define STATEMENT_CONSTRUCTOR_PARAMETERS                                \
  BlockScopePtr scope, LabelScopePtr labelScope, const Location::Range& r
#define STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(kindOf)                  \
  scope, labelScope, r, Statement::KindOf##kindOf
#define DECLARE_BASE_STATEMENT_VIRTUAL_FUNCTIONS                        \
  void analyzeProgram(AnalysisResultPtr ar) override;                   \
  StatementPtr clone() override;                                        \
  void outputCodeModel(CodeGenerator &cg) override;                     \
  void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) override;
#define DECLARE_STATEMENT_VIRTUAL_FUNCTIONS                             \
  DECLARE_BASE_STATEMENT_VIRTUAL_FUNCTIONS;                             \
  ConstructPtr getNthKid(int n) const override;                         \
  int getKidCount() const override;                                     \
  void setNthKid(int n, ConstructPtr cp) override
#define NULL_STATEMENT()                                    \
  BlockStatementPtr(new BlockStatement(getScope(),          \
                                       getLabelScope(),     \
                                       getRange(),          \
                                       StatementListPtr()))

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(LabelScope);

class Statement : public Construct {
private:
  static const char *Names[];

public:
  static const char* nameOfKind(Construct::KindOf);

protected:
  Statement(STATEMENT_CONSTRUCTOR_BASE_PARAMETERS);

public:
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
  int m_silencerCountMax;
  int m_silencerCountCurrent;
  LabelScopePtr m_labelScope;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_STATEMENT_H_

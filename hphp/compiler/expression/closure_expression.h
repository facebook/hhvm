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

#ifndef incl_HPHP_CLOSURE_EXPRESSION_H_
#define incl_HPHP_CLOSURE_EXPRESSION_H_

#include "hphp/compiler/expression/expression.h"
#include <set>
#include "hphp/parser/parser.h"
#include "hphp/util/compact-vector.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ClosureExpression);
DECLARE_BOOST_TYPES(FunctionStatement);
DECLARE_BOOST_TYPES(ExpressionList);

struct ClosureExpression : Expression {
  ClosureExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                    ClosureType type,
                    FunctionStatementPtr func,
                    ExpressionListPtr vars);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;

  // Flag for whether we have already determined the capture list for
  // this lambda.
  enum class CaptureState {
    Unknown,
    Known,
  };
  CaptureState captureState() const { return m_captureState; }

  ConstructPtr getNthKid(int n) const override;
  void setNthKid(int n, ConstructPtr cp) override;
  int getKidCount() const override;
  void analyzeProgram(AnalysisResultConstRawPtr ar) override;

  FunctionStatementPtr getClosureFunction() { return m_func; }
  ExpressionListPtr getClosureVariables() { return m_vars; }
  ExpressionListPtr getClosureValues() { return m_values; }
  bool hasStaticLocals();
  ClosureType type() const { return m_type; }
  std::set<std::string> collectParamNames() const;

  static void processLambdas(AnalysisResultConstRawPtr ar,
                             CompactVector<ClosureExpressionRawPtr>&& lambdas);

  /*
   * Initialize the capture list for a closure that uses automatic
   * captures.
   *
   * Pre: captureState() == CaptureState::Unknown.
   */
  void setCaptureList(AnalysisResultConstRawPtr ar,
                      const std::set<std::string>&);

private:
  void initializeFromUseList(ExpressionListPtr vars);
  void initializeValuesFromVars();
  void analyzeVarsForClosure(AnalysisResultConstRawPtr);
  void analyzeVarsForClosureExpression(AnalysisResultConstRawPtr);
  bool hasStaticLocalsImpl(ConstructPtr root);
  void processLambda(AnalysisResultConstRawPtr ar);

private:
  ClosureType m_type;
  FunctionStatementPtr m_func;
  ExpressionListPtr m_vars;
  ExpressionListPtr m_values;
  std::set<std::string> m_unboundNames;
  CaptureState m_captureState;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CLOSURE_EXPRESSION_H_

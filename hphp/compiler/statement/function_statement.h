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

#ifndef incl_HPHP_FUNCTION_STATEMENT_H_
#define incl_HPHP_FUNCTION_STATEMENT_H_

#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/expression/unary_op_expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(FunctionStatement);

class FunctionStatement : public MethodStatement {
public:
  FunctionStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                    ModifierExpressionPtr modifiers, bool ref,
                    const std::string &name, ExpressionListPtr params,
                    TypeAnnotationPtr retTypeAnnotation,
                    StatementListPtr stmt, int attr,
                    const std::string &docComment,
                    ExpressionListPtr attrList);

  DECLARE_BASE_STATEMENT_VIRTUAL_FUNCTIONS;
  bool hasDecl() const override { return true; }
  bool hasImpl() const override;

  std::string getName() const override;

  // implementing IParseHandler
  void onParse(AnalysisResultConstPtr ar, FileScopePtr scope) override;
  bool ignored() const { return m_ignored;}

  void outputPHPHeader(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputPHPBody(CodeGenerator &cg, AnalysisResultPtr ar);

private:
  bool m_ignored;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_FUNCTION_STATEMENT_H_

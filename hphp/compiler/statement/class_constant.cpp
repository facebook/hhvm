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

#include "hphp/compiler/statement/class_constant.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/class_constant_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/type_annotation.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ClassConstant::ClassConstant
(STATEMENT_CONSTRUCTOR_PARAMETERS, std::string typeConstraint,
 ExpressionListPtr exp, bool abstract,
 bool typeconst, TypeAnnotationPtr typeAnnot)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(ClassConstant)),
    m_typeConstraint(typeConstraint), m_exp(exp), m_abstract(abstract),
    m_typeconst(typeconst) {
  // for now only store TypeAnnotation info for type constants
  if (typeconst && typeAnnot) {
    m_typeStructure = Array(typeAnnot->getScalarArrayRep());
    assertx(m_typeStructure.isDictOrDArray());
  }
}

StatementPtr ClassConstant::clone() {
  ClassConstantPtr stmt(new ClassConstant(*this));
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ClassConstant::onParseRecur(AnalysisResultConstRawPtr /*ar*/,
                                 FileScopeRawPtr fs, ClassScopePtr scope) {
  if (scope->isTrait()) {
    parseTimeFatal(fs,
                   "Traits cannot have constants");
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

ConstructPtr ClassConstant::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int ClassConstant::getKidCount() const {
  return 1;
}

void ClassConstant::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ClassConstant::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (isAbstract()) {
    cg_printf("abstract ");
  }
  cg_printf("const ");
  if (isTypeconst()) {
    cg_printf("type ");
  }
  m_exp->outputPHP(cg, ar);
  cg_printf(";\n");
}

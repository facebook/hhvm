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

#include "hphp/compiler/expression/function_call.h"
#include "hphp/util/text-util.h"
#include "hphp/util/logger.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/exp_statement.h"
#include "hphp/compiler/statement/return_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/array_pair_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/parser/hphp.tab.hpp"
#include <folly/Conv.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

FunctionCall::FunctionCall
(EXPRESSION_CONSTRUCTOR_BASE_PARAMETERS,
 ExpressionPtr nameExp, const std::string &name, bool hadBackslash,
 ExpressionListPtr params, ExpressionPtr classExp)
  : Expression(EXPRESSION_CONSTRUCTOR_BASE_PARAMETER_VALUES),
    StaticClassName(classExp),
    m_nameExp(nameExp),
    m_params(params),
    m_valid(false),
    m_variableArgument(false),
    m_redeclared(false),
    m_arrayParams(false),
    m_hadBackslash(hadBackslash) {

  if (m_nameExp &&
      m_nameExp->getKindOf() == Expression::KindOfScalarExpression) {
    assert(m_origName.empty());
    auto c = dynamic_pointer_cast<ScalarExpression>(m_nameExp);
    m_origName = c->getOriginalLiteralString();
    c->toLower(true /* func call*/);
  } else {
    m_origName = name;
  }
}

void FunctionCall::reset() {
  m_valid = false;
  m_variableArgument = false;
}

bool FunctionCall::isNamed(folly::StringPiece name) const {
  return bstrcasecmp(m_origName, name) == 0;
}

void FunctionCall::deepCopy(FunctionCallPtr exp) {
  Expression::deepCopy(exp);
  exp->m_class = Clone(m_class);
  exp->m_params = Clone(m_params);
  exp->m_nameExp = Clone(m_nameExp);
}

ConstructPtr FunctionCall::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_class;
    case 1:
      return m_nameExp;
    case 2:
      return m_params;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int FunctionCall::getKidCount() const {
  return 3;
}

bool FunctionCall::hasUnpack() const {
  return m_params && m_params->containsUnpack();
}

void FunctionCall::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_class = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_nameExp = dynamic_pointer_cast<Expression>(cp);
      break;
    case 2:
      m_params = dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

void FunctionCall::onParse(AnalysisResultConstRawPtr ar, FileScopePtr fs) {
  StaticClassName::onParse(ar, fs);
  if (!checkUnpackParams()) {
    parseTimeFatal(
      fs,
      "Only the last parameter in a function call is allowed to use ...");
  }
}

bool FunctionCall::checkUnpackParams() {
  if (!m_params) { return true; }
  ExpressionList &params = *m_params;
  const auto numParams = params.getCount();
  if (!numParams) return true;

  // when supporting multiple unpacks at the end of the param list, this
  // will need to disallow transitions from unpack to non-unpack.
  for (int i = 0; i < (numParams - 1); ++i) {
    ExpressionPtr p = params[i];
    if (p->isUnpack()) {
      return false;
    }
  }

  // we don't get here if any parameter before the last has isUnpack()
  // set, so the last one had better match containsUnpack().
  assert(params.containsUnpack() == params[numParams - 1]->isUnpack());
  return true;
}

void FunctionCall::analyzeProgram(AnalysisResultConstRawPtr /*ar*/) {
  if (isParent()) {
    getFunctionScope()->setContainsThis();
  }
}

///////////////////////////////////////////////////////////////////////////////

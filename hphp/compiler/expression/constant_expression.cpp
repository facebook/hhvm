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

#include "hphp/compiler/expression/constant_expression.h"
#include <vector>
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/util/hash.h"
#include "hphp/util/text-util.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/runtime/ext/std/ext_std_misc.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ConstantExpression::ConstantExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 const std::string &name, bool hadBackslash, const std::string &docComment)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ConstantExpression)),
    m_name(name), m_origName(name), m_hadBackslash(hadBackslash),
    m_docComment(docComment), m_valid(false), m_dynamic(false),
    m_visited(false) {
}

void ConstantExpression::onParse(AnalysisResultConstRawPtr ar,
                                 FileScopePtr /*scope*/) {
  ar->parseOnDemandByConstant(m_name);
}

ExpressionPtr ConstantExpression::clone() {
  ConstantExpressionPtr exp(new ConstantExpression(*this));
  Expression::deepCopy(exp);
  return exp;
}

bool ConstantExpression::isScalar() const {
  if (m_name == "INF" || m_name == "NAN") return true;
  auto const lower = toLower(m_name);
  return lower == "true" || lower == "false" || lower == "null";
}

bool ConstantExpression::isLiteralNull() const {
  return isNull();
}

bool ConstantExpression::isNull() const {
  auto const lower = toLower(m_name);
  return (lower == "null");
}

bool ConstantExpression::isBoolean() const {
  auto const lower = toLower(m_name);
  return (lower == "true" || lower == "false");
}

bool ConstantExpression::isDouble() const {
  return (m_name == "INF" || m_name == "NAN");
}

bool ConstantExpression::getBooleanValue() const {
  auto const lower = toLower(m_name);
  assert(lower == "true" || lower == "false");
  return lower == "true";
}

bool ConstantExpression::getScalarValue(Variant &value) {
  if (!isScalar()) return false;
  if (isBoolean()) {
    value = getBooleanValue();
  } else if (m_name == "INF") {
    value = k_INF;
  } else if (m_name == "NAN") {
    value = k_NAN;
  } else {
    value.unset();
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ConstantExpression::outputPHP(CodeGenerator& cg,
                                   AnalysisResultPtr /*ar*/) {
  if (hadBackslash()) {
    cg_printf("\\%s", m_name.c_str());
  } else {
    cg_printf("%s", getNonNSOriginalName().c_str());
  }
}

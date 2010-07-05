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

#include <compiler/expression/static_class_name.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/analysis/class_scope.h>
#include <util/util.h>

using namespace std;
using namespace boost;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

StaticClassName::StaticClassName(ExpressionPtr classExp)
    : m_class(classExp) {
  updateClassName();
}

void StaticClassName::updateClassName() {
  if (m_class && m_class->is(Expression::KindOfScalarExpression)) {
    ScalarExpressionPtr s(dynamic_pointer_cast<ScalarExpression>(m_class));
    const string &className = s->getString();
    m_className = Util::toLower(className);
    if (m_className == "static") {
      m_className.clear();
    } else {
      m_origClassName = className;
      m_class.reset();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void StaticClassName::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_class) {
    m_class->outputPHP(cg, ar);
  } else {
    if (ar && cg.getOutput() == CodeGenerator::PickledPHP) {
      ClassScopePtr cls = ar->resolveClass(m_className);
      if (cls) {
        cg_printf("%s", cls->getName().c_str());
        return;
      }
    }
    cg_printf("%s", m_className.c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////
}

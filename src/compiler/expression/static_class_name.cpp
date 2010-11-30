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
#include <compiler/statement/statement_list.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/file_scope.h>
#include <util/util.h>

using namespace std;
using namespace boost;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

StaticClassName::StaticClassName(ExpressionPtr classExp)
    : m_class(classExp),
      m_isSelf(false), m_isParent(false), m_isStatic(false),
      m_redeclared(false), m_present(false) {
  updateClassName();
  if (m_origClassName == "parent") {
    m_isParent = true;
  } else if (m_origClassName == "self") {
    m_isSelf = true;
  } else if (m_origClassName == "static") {
    m_isStatic = true;
    m_present = true;
    m_class = classExp;
    m_className = m_origClassName = "";
  }
}

void StaticClassName::updateClassName() {
  if (m_class && m_class->is(Expression::KindOfScalarExpression) &&
      !m_isStatic) {
    ScalarExpressionPtr s(dynamic_pointer_cast<ScalarExpression>(m_class));
    const string &className = s->getString();
    m_className = Util::toLower(className);
    m_origClassName = className;
    m_class.reset();
  }
}

ClassScopePtr StaticClassName::resolveClass(BlockScopeRawPtr scope) {
  m_present = false;
  if (m_class) return ClassScopePtr();
  if (m_isSelf) {
    if (ClassScopePtr self = scope->getContainingClass()) {
      m_className = self->getName();
      m_present = true;
      return self;
    }
  } else if (m_isParent) {
    if (ClassScopePtr self = scope->getContainingClass()) {
      if (!self->getParent().empty()) {
        m_className = self->getParent();
        m_present = true;
      }
    } else {
      m_isParent = false;
    }
  }
  ClassScopePtr cls = scope->getContainingProgram()->findClass(m_className);
  if (cls) {
    if (cls->isRedeclaring()) {
      cls = scope->findExactClass(m_className);
      if (!cls) {
        m_redeclared = true;
      } else {
        m_present = true;
      }
    } else if (cls->isVolatile()) {
      m_present = checkPresent(scope);
    } else {
      m_present = true;
    }
  }
  return cls;
}

bool StaticClassName::checkPresent(BlockScopeRawPtr scope) {
  if (m_isSelf || m_isParent || m_isStatic) return true;
  if (ClassScopePtr self = scope->getContainingClass()) {
    if (m_className == self->getName() ||
        self->derivesFrom(scope->getContainingProgram(), m_className,
                          true, false)) {
      return true;
    }
  }
  if (FileScopePtr currentFile = scope->getContainingFile()) {
    StatementList &stmts = *currentFile->getStmt();
    for (int i = stmts.getCount(); i--; ) {
      StatementPtr s = stmts[i];
      if (s && s->is(Statement::KindOfClassStatement)) {
        ClassScopePtr cls =
          static_pointer_cast<ClassStatement>(s)->getClassScope();
        if (m_className == cls->getName()) {
          return true;
        }
        if (cls->derivesFrom(scope->getContainingProgram(), m_className,
                             true, false)) {
          return true;
        }
      }
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

void StaticClassName::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_class) {
    m_class->outputPHP(cg, ar);
  } else {
    cg_printf("%s", m_origClassName.c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////
}

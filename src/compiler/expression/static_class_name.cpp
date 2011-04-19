/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
      m_self(false), m_parent(false), m_static(false),
      m_redeclared(false), m_present(false), m_forcePresent(false),
      m_unknown(true) {
  updateClassName();
  if (m_origClassName == "parent") {
    m_parent = true;
  } else if (m_origClassName == "self") {
    m_self = true;
  } else if (m_origClassName == "static") {
    m_static = true;
    m_present = true;
    m_class = classExp;
    m_className = m_origClassName = "";
  }
}

void StaticClassName::updateClassName() {
  if (m_class && m_class->is(Expression::KindOfScalarExpression) &&
      !m_static) {
    ScalarExpressionPtr s(dynamic_pointer_cast<ScalarExpression>(m_class));
    const string &className = s->getString();
    m_className = Util::toLower(className);
    m_origClassName = className;
    m_class.reset();
  } else {
    m_className = "";
  }
}

void StaticClassName::resolveStatic(const string &name) {
  assert(isStatic());
  m_static = m_self = m_parent = false;
  m_present = false;
  m_class.reset();
  m_origClassName = name;
  m_className = Util::toLower(name);
}

ClassScopePtr StaticClassName::resolveClass() {
  m_present = false;
  m_unknown = true;
  if (m_class) return ClassScopePtr();
  BlockScopeRawPtr scope = dynamic_cast<Expression*>(this)->getOriginalScope();
  if (m_self) {
    if (ClassScopePtr self = scope->getContainingClass()) {
      m_className = self->getName();
      m_present = true;
      m_unknown = false;
      return self;
    }
  } else if (m_parent) {
    if (ClassScopePtr self = scope->getContainingClass()) {
      if (!self->getParent().empty()) {
        m_className = self->getParent();
        m_present = true;
      }
    } else {
      m_parent = false;
    }
  }
  ClassScopePtr cls = scope->getContainingProgram()->findClass(m_className);
  if (cls) {
    m_unknown = false;
    if (cls->isRedeclaring()) {
      cls = scope->findExactClass(m_className);
      if (!cls) {
        m_redeclared = true;
      } else {
        m_present = true;
      }
    } else if (cls->isVolatile()) {
      m_present = checkPresent();
    } else {
      m_present = true;
    }
  }
  return cls;
}

bool StaticClassName::checkPresent() {
  if (m_self || m_parent || m_static) return true;
  BlockScopeRawPtr scope = dynamic_cast<Expression*>(this)->getOriginalScope();
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

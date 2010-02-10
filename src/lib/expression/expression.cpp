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

#include <lib/expression/expression.h>
#include <lib/analysis/code_error.h>
#include <lib/parser/parser.h>
#include <lib/parser/hphp.tab.hpp>
#include <util/util.h>
#include <lib/analysis/class_scope.h>
#include <lib/analysis/function_scope.h>
#include <lib/expression/scalar_expression.h>
#include <lib/expression/constant_expression.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

Expression::Expression(LocationPtr loc, KindOf kindOf)
  : Construct(loc), m_kindOf(kindOf), m_context(RValue) {
}

void Expression::deepCopy(ExpressionPtr exp) {
  exp->m_actualType = m_actualType;
  exp->m_expectedType = m_expectedType;
};

void Expression::addElement(ExpressionPtr exp) {
  ASSERT(false);
}

void Expression::insertElement(ExpressionPtr exp, int index /* = 0 */) {
  ASSERT(false);
}

///////////////////////////////////////////////////////////////////////////////

bool Expression::isIdentifier(const string &value) {
  for (unsigned int i = 0; i < value.size(); i++) {
    char ch = value[i];
    if ((i == 0 && ch >= '0' && ch <= '9') ||
        ((ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z') &&
         (ch < '0' || ch > '9') && (ch != '_'))) {
      return false;
    }
  }
  return true;
}

TypePtr Expression::getType() {
  if (m_expectedType) return m_expectedType;
  if (m_actualType) return m_actualType;
  return NEW_TYPE(Any);
}

TypePtr Expression::inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                  bool coerce) {
  TypePtr actualType = inferTypes(ar, type, coerce);
  if (type->is(Type::KindOfSome) || type->is(Type::KindOfAny)) {
    m_actualType = actualType;
    return actualType;
  }
  return checkTypesImpl(ar, type, actualType, coerce);
}

TypePtr Expression::checkTypesImpl(AnalysisResultPtr ar, TypePtr expectedType,
                                   TypePtr actualType, bool coerce) {
  TypePtr ret;
  if (coerce) {
    ret = Type::Coerce(ar, expectedType, actualType);
    setTypes(actualType, expectedType);
  } else {
    if (!Type::IsLegalCast(ar, actualType, expectedType)) {
      ar->getCodeError()->record(shared_from_this(), expectedType->getKindOf(),
                                 actualType->getKindOf());
    }
    ret = Type::Cast(ar, actualType, expectedType);
    setTypes(actualType, ret);
  }
  return ret;
}

void Expression::setTypes(TypePtr actualType, TypePtr expectedType) {
  m_actualType = actualType;
  if (!Type::SameType(expectedType, actualType)) {
    m_expectedType = expectedType;
  } else {
    // Clear expected type since expectedType == actualType
    m_expectedType.reset();
  }

  // This is a special case where Type::KindOfObject means any object.
  if (m_expectedType && m_expectedType->is(Type::KindOfObject) &&
      !m_expectedType->isSpecificObject() &&
      m_actualType->isSpecificObject()) {
    m_expectedType.reset();
  }
}

void Expression::setDynamicByIdentifier(AnalysisResultPtr ar,
                                        const std::string &value) {
  string id = Util::toLower(value);
  size_t c = id.find("::");
  FunctionScopePtr fi;
  ClassScopePtr ci;
  if (c != 0 && c != string::npos && c+2 < id.size()) {
    string cl = id.substr(0, c);
    string fn = id.substr(c+2);
    if (isIdentifier(cl) && isIdentifier(fn)) {
      ci = ar->findClass(cl);
      if (ci) {
        fi = ci->findFunction(ar, fn, false);
        if (fi) fi->setDynamic();
      }
    }
  } else if (isIdentifier(id)) {
    fi = ar->findFunction(id);
    if (fi) {
      fi->setDynamic();
    }
    ClassScopePtr ci = ar->findClass(id, AnalysisResult::MethodName);
    if (ci) {
      fi = ci->findFunction(ar, id, false);
      if (fi) {
        fi->setDynamic();
      }
    }
  }
}

ExpressionPtr Expression::makeConstant(AnalysisResultPtr ar,
                                       LocationPtr loc,
                                       const std::string &value) {
  ConstantExpressionPtr exp(new ConstantExpression(loc,
                            Expression::KindOfConstantExpression,
                            value));
  if (value == "true" || value == "false") {
    if (ar->getPhase() >= AnalysisResult::PostOptimize) {
      exp->m_actualType = Type::Boolean;
    }
  } else if (value == "null") {
    if (ar->getPhase() >= AnalysisResult::PostOptimize) {
      exp->m_actualType = Type::Variant;
    }
  } else {
    ASSERT(false);
  }
  return exp;
}

///////////////////////////////////////////////////////////////////////////////

bool Expression::outputLineMap(CodeGenerator &cg, AnalysisResultPtr ar) {
  switch (cg.getOutput()) {
  case CodeGenerator::TrimmedPHP:
    if (cg.getStream(CodeGenerator::MapFile) &&
        cg.usingStream(CodeGenerator::PrimaryStream)) {
      cg.useStream(CodeGenerator::MapFile);
      cg.printf("%d => '%s:%d',", cg.getLineNo(CodeGenerator::PrimaryStream),
                getLocation()->file, getLocation()->line1);
      cg.useStream(CodeGenerator::PrimaryStream);
    }
    break;
  case CodeGenerator::ClusterCPP:
    {
      int line = cg.getLineNo(CodeGenerator::PrimaryStream);
      string fileline = cg.getFileName() + ":" + lexical_cast<string>(line);
      LocationPtr loc = getLocation();
      ar->recordSourceInfo(fileline, loc);
      if (cg.getPHPLineNo() != loc->line1) {
        cg.setPHPLineNo(loc->line1);
        cg.printf("LINE(%d,", loc->line1);
        return true;
      }
    }
    break;
  default:
    break;
  }
  return false;
}

void Expression::outputCPPCast(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_expectedType) {
    m_expectedType->outputCPPCast(cg, ar);
  }
}

void Expression::outputCPPDecl(CodeGenerator &cg, AnalysisResultPtr ar) {
  TypePtr type = m_actualType;
  if (!type) type = Type::Variant;
  type->outputCPPDecl(cg, ar);
}

void Expression::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  // When p_ types are used as r-value, we cast it to Object so not to define
  // every function with SmartObject<T> template functions
  bool castToObject = false;
  if ((m_context & (LValue|RefValue|ObjectContext)) == 0 &&
      m_actualType && m_actualType->isSpecificObject() &&
      !is(Expression::KindOfParameterExpression)) {
    castToObject = true;
  }

  if (m_expectedType && m_actualType && ((m_context & LValue) == 0) &&
      Type::IsCastNeeded(ar, m_actualType, m_expectedType)) {
    m_expectedType->outputCPPCast(cg, ar);
    if (m_expectedType->isSpecificObject()) castToObject = false;
    cg.printf("(");
    if (castToObject) {
      cg.printf("((Object)(");
      outputCPPImpl(cg, ar);
      cg.printf("))");
    } else {
      outputCPPImpl(cg, ar);
    }
    cg.printf(")");
  } else {
    int closeParan = 0;
    if (((m_context & RefValue) != 0) && ((m_context & NoRefWrapper) == 0) &&
        isRefable()) {
      cg.printf("ref(");
      closeParan++;
    }
    if (((((m_context & LValue) != 0) &&
          ((m_context & NoLValueWrapper) == 0)) ||
         ((m_context & RefValue) != 0) &&
         ((m_context & InvokeArgument) == 0)) &&
        (is(Expression::KindOfArrayElementExpression) ||
         is(Expression::KindOfObjectPropertyExpression))) {
      cg.printf("lval(");
      closeParan++;
    }
    if (castToObject) {
      cg.printf("((Object)(");
      outputCPPImpl(cg, ar);
      cg.printf("))");
    } else {
      outputCPPImpl(cg, ar);
    }
    for (int i = 0; i < closeParan; i++) {
      cg.printf(")");
    }
  }

  string comment;
  if (is(Expression::KindOfScalarExpression)) {
    ScalarExpressionPtr exp =
      dynamic_pointer_cast<ScalarExpression>(shared_from_this());
      comment = exp->getComment();
  } else if (is(Expression::KindOfConstantExpression)) {
    ConstantExpressionPtr exp =
      dynamic_pointer_cast<ConstantExpression>(shared_from_this());
      comment = exp->getComment();
  }

  if (!comment.empty()) {
    if (cg.inComments()) {
      cg.printf(" (%s)", comment.c_str());
    } else {
      cg.printf(" /* %s */", comment.c_str());
    }
  }
}

void Expression::outputCPPExistTest(CodeGenerator &cg, AnalysisResultPtr ar,
                                    int op) {
  switch (op) {
  case T_ISSET:  cg.printf("isset("); break;
  case T_EMPTY:  cg.printf("empty("); break;
  default: ASSERT(false);
  }
  outputCPP(cg, ar);
  cg.printf(")");
}
void Expression::outputCPPUnset(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.printf("unset(");
  outputCPP(cg, ar);
  cg.printf(");\n");
}

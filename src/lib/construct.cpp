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

#include <lib/construct.h>
#include <lib/parser/parser.h>
#include <util/util.h>
#include <lib/analysis/file_scope.h>
#include <lib/analysis/function_scope.h>
#include <lib/analysis/class_scope.h>
#include <lib/analysis/analysis_result.h>
#include <lib/analysis/dependency_graph.h>

#include <lib/expression/simple_function_call.h>
#include <lib/expression/simple_variable.h>

using namespace HPHP;
using namespace std;

///////////////////////////////////////////////////////////////////////////////

Construct::Construct(LocationPtr loc)
 : m_extra(NULL), m_loc(loc), m_fileLevel(false), m_topLevel(false) {
}

std::string Construct::getText(bool useCache /* = false */,
                               bool translate /* = false */) {
  std::string &text = m_text;
  if (useCache && !text.empty()) return text;
  ostringstream o;
  CodeGenerator cg(&o, CodeGenerator::PickledPHP);
  cg.translatePredefined(translate);
  outputPHP(cg, AnalysisResultPtr()); // we knew PickledPHP won't use ar
  text = o.str();
  return text;
}

void Construct::serialize(JSON::OutputStream &out) const {
  out.raw() << "[\"" << m_loc->file << "\"," <<
    m_loc->line0 << "," << m_loc->char0 << "," <<
    m_loc->line1 << "," << m_loc->char1 << "]";
}

void Construct::addUserFunction(AnalysisResultPtr ar,
                                const std::string &name,
                                bool strong /* = true */) {
  if (!name.empty()) {
    FunctionScopePtr func = ar->findFunction(name);
    if (func && func->isUserFunction()) {
      ar->getDependencyGraph()->add
        (DependencyGraph::KindOfProgramUserFunction,
         ar->getName(), func->getName(), func->getStmt());
      ar->addCallee(func->getStmt());
    }
    if (strong && ar->getPhase() == AnalysisResult::AnalyzeAll) {
      FunctionScopePtr func = ar->getFunctionScope();
      ar->getFileScope()->addFunctionDependency(ar, name, func &&
                                                func->isInlined());
    }
  }
}

void Construct::addUserClass(AnalysisResultPtr ar,
                             const std::string &name,
                             bool strong /* = true */) {
  if (!name.empty()) {
    ClassScopePtr cls = ar->findClass(name);
    if (cls && cls->isUserClass()) {
      ar->getDependencyGraph()->add(DependencyGraph::KindOfProgramUserClass,
                                    ar->getName(),
                                    cls->getName(), cls->getStmt());
      ar->addCallee(cls->getStmt());
    }
    if (strong && !ar->isFirstPass()) {
      ar->getFileScope()->addClassDependency(ar, name);
    }
  }
}

bool Construct::isErrorSuppressed(CodeError::ErrorType e) const {
  std::vector<CodeError::ErrorType> &suppressedErrors =
    getExtra()->suppressedErrors;
  for (unsigned int i = 0; i < suppressedErrors.size(); i++) {
    if (suppressedErrors[i] == e) {
      return true;
    }
  }
  return false;
}

void Construct::addHphpNote(const std::string &s) {
  ExtraData *extra = getExtra();
  if (s.find("C++") == 0) {
    extra->embedded += s.substr(3);
    extra->hphpNotes.insert("C++");
  } else {
    extra->hphpNotes.insert(s);
  }
}

void Construct::printSource(CodeGenerator &cg) {
  if (m_loc) {
    cg.printf("/* SRC: %s line %d */\n", m_loc->file, m_loc->line1);
  }
}

void Construct::dump(int spc, AnalysisResultPtr ar) {
  int nkid = getKidCount();
  std::string name;
  int type = 0;
  std::string scontext = "";
  std::string value = "";
  if (Statement *s = dynamic_cast<Statement*>(this)) {
    Statement::KindOf stype = s->getKindOf();
    switch (stype) {
    case Statement::KindOfFunctionStatement:
      name="FunctionStatement";
      break;
    case Statement::KindOfClassStatement:
      name="ClassStatement";
      break;
    case Statement::KindOfInterfaceStatement:
      name="InterfaceStatement";
      break;
    case Statement::KindOfClassVariable:
      name="ClassVariable";
      break;
    case Statement::KindOfClassConstant:
      name="ClassConstant";
      break;
    case Statement::KindOfMethodStatement:
      name="MethodStatement";
      break;
    case Statement::KindOfStatementList:
      name="StatementList";
      break;
    case Statement::KindOfBlockStatement:
      name="BlockStatement";
      break;
    case Statement::KindOfIfBranchStatement:
      name="IfBranchStatement";
      break;
    case Statement::KindOfIfStatement:
      name="IfStatement";
      break;
    case Statement::KindOfWhileStatement:
      name="WhileStatement";
      break;
    case Statement::KindOfDoStatement:
      name="DoStatement";
      break;
    case Statement::KindOfForStatement:
      name="ForStatement";
      break;
    case Statement::KindOfSwitchStatement:
      name="SwitchStatement";
      break;
    case Statement::KindOfCaseStatement:
      name="CaseStatement";
      break;
    case Statement::KindOfBreakStatement:
      name="BreakStatement";
      break;
    case Statement::KindOfContinueStatement:
      name="ContinueStatement";
      break;
    case Statement::KindOfReturnStatement:
      name="ReturnStatement";
      break;
    case Statement::KindOfGlobalStatement:
      name="GlobalStatement";
      break;
    case Statement::KindOfStaticStatement:
      name="StaticStatement";
      break;
    case Statement::KindOfEchoStatement:
      name="EchoStatement";
      break;
    case Statement::KindOfUnsetStatement:
      name="UnsetStatement";
      break;
    case Statement::KindOfExpStatement:
      name="ExpStatement";
      break;
    case Statement::KindOfForEachStatement:
      name="ForEachStatement";
      break;
    case Statement::KindOfCatchStatement:
      name="CatchStatement";
      break;
    case Statement::KindOfTryStatement:
      name="TryStatement";
      break;
    case Statement::KindOfThrowStatement:
      name="ThrowStatement";
      break;
    }
    type = (int)stype;
  } else if (Expression *e = dynamic_cast<Expression*>(this)) {
    Expression::KindOf etype = e->getKindOf();
    switch (etype) {
    case Expression::KindOfSimpleFunctionCall:
      name="SimpleFunctionCall";
      value = static_cast<SimpleFunctionCall*>(e)->getName();
      break;
    case Expression::KindOfSimpleVariable:
      name="SimpleVariable";
      value = static_cast<SimpleVariable*>(e)->getName();
      break;
    case Expression::KindOfConstantExpression:
      name="ConstantExpression";
      value = e->getText();
      break;
    case Expression::KindOfScalarExpression:
      name="ScalarExpression";
      value = e->getText();
      break;
    case Expression::KindOfExpressionList:
      name="ExpressionList";
      break;
    case Expression::KindOfAssignmentExpression:
      name="AssignmentExpression";
      break;
    case Expression::KindOfDynamicVariable:
      name="DynamicVariable";
      break;
    case Expression::KindOfStaticMemberExpression:
      name="StaticMemberExpression";
      break;
    case Expression::KindOfArrayElementExpression:
      name="ArrayElementExpression";
      break;
    case Expression::KindOfDynamicFunctionCall:
      name="DynamicFunctionCall";
      break;
    case Expression::KindOfObjectPropertyExpression:
      name="ObjectPropertyExpression";
      break;
    case Expression::KindOfObjectMethodExpression:
      name="ObjectMethodExpression";
      break;
    case Expression::KindOfListAssignment:
      name="ListAssignment";
      break;
    case Expression::KindOfNewObjectExpression:
      name="NewObjectExpression";
      break;
    case Expression::KindOfUnaryOpExpression:
      name="UnaryOpExpression";
      break;
    case Expression::KindOfIncludeExpression:
      name="IncludeExpression";
      break;
    case Expression::KindOfBinaryOpExpression:
      name="BinaryOpExpression";
      break;
    case Expression::KindOfQOpExpression:
      name="QOpExpression";
      break;
    case Expression::KindOfArrayPairExpression:
      name="ArrayPairExpression";
      break;
    case Expression::KindOfClassConstantExpression:
      name="ClassConstantExpression";
      break;
    case Expression::KindOfParameterExpression:
      name="ParameterExpression";
      break;
    case Expression::KindOfModifierExpression:
      name="ModifierExpression";
      break;
    case Expression::KindOfEncapsListExpression:
      name="EncapsListExpression";
      break;
    }

    int c = e->getContext();
    if ((c & Expression::Declaration) == Expression::Declaration) {
      scontext += "|Declaration";
    } else if (c & Expression::LValue) {
      scontext += "|LValue";
    }
    if (c & Expression::NoLValueWrapper) {
      scontext += "|NoLValueWrapper";
    }
    if (c & Expression::RefValue) {
      scontext += "|RefValue";
    }
    if (c & Expression::NoRefWrapper) {
      scontext += "|NoRefWrapper";
    }
    if (c & Expression::ObjectContext) {
      scontext += "|ObjectContext";
    }
    if (c & Expression::InParameterExpression) {
      scontext += "|InParameterExpression";
    }
    if (c & Expression::IssetContext) {
      scontext += "|IssetContext";
    }
    if (c & Expression::AssignmentLHS) {
      scontext += "|AssignmentLHS";
    }
    if (c & Expression::DeepAssignmentLHS) {
      scontext += "|DeepAssignmentLHS";
    }
    if (c & Expression::UnsetContext) {
      scontext += "|UnsetContext";
    }
    if (c & Expression::InvokeArgument) {
      scontext += "|InvokeArgument";
    }
    if (c & Expression::RefParameter) {
      scontext += "|RefParameter";
    }

    if (scontext != "") {
      scontext = " (" + scontext.substr(1) + ")";
    }
    type = (int)etype;
  } else {
    ASSERT(FALSE);
  }

  int s = spc;
  while (s > 0) {
    int n = s > 10 ? 10 : s;
    std::cout << ("          "+10-n);
    s -= n;
  }

  std::cout << "-> 0x";
  std::cout.setf(std::ios::hex);
  std::cout << (long long)this;
  std::cout.setf(std::ios::dec);

  std::cout << " " << name << "(" << type << ") ";
  if (value != "") {
    std::cout << "[" << value << "] ";
  }
  std::cout << nkid << scontext;
  if (m_loc) {
    std::cout << " " << m_loc->file << ":" <<
      m_loc->line1 << "@" << m_loc->char1;
  }
  std::cout << "\n";

  for (int i = 0; i < nkid; i++) {
    ConstructPtr kid = getNthKid(i);
    if (kid) {
      kid->dump(spc+2, ar);
    }
  }
}

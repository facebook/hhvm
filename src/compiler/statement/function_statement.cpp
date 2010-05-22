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

#include <compiler/statement/function_statement.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/expression/expression_list.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/statement/statement_list.h>
#include <util/util.h>
#include <compiler/expression/parameter_expression.h>
#include <compiler/expression/modifier_expression.h>
#include <compiler/option.h>
#include <compiler/analysis/dependency_graph.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/class_scope.h>
#include <sstream>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

FunctionStatement::FunctionStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 bool ref, const std::string &name, ExpressionListPtr params,
 StatementListPtr stmt, int attr, const std::string &docComment)
  : MethodStatement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES,
                    ModifierExpressionPtr(), ref, name, params, stmt, attr,
                    docComment, false) {
}

StatementPtr FunctionStatement::clone() {
  FunctionStatementPtr stmt(new FunctionStatement(*this));
  stmt->m_stmt = Clone(m_stmt);
  stmt->m_params = Clone(m_params);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void FunctionStatement::onParse(AnalysisResultPtr ar) {
   // note it's important to add to file scope, not a pushed FunctionContainer,
  // as a function may be declared inside a class's method, yet this function
  // is a global function, not a class method.
  FileScopePtr fileScope = ar->getFileScope();
  fileScope->addFunction(ar, onParseImpl(ar));
  ar->recordFunctionSource(m_name, fileScope->getName());
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

std::string FunctionStatement::getName() const {
  return string("Function ") + m_funcScope.lock()->getName();
}

void FunctionStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  // registering myself as a parent in dependency graph, so that
  // (1) we can tell orphaned parents
  // (2) overwrite non-master copy of function declarations
  if (ar->isFirstPass()) {
    if (m_loc) {
      ar->getDependencyGraph()->addParent(DependencyGraph::KindOfFunctionCall,
                                          "", m_name, shared_from_this());
    } // else it's pseudoMain or artificial functions we added
    FunctionScopePtr fs = m_funcScope.lock();
    if (!isFileLevel() && !fs->isIgnored() &&
        !fs->isRedeclaring()) {
      // Dynamic function declaration
      // fs->setRedeclaring(0);
    }
  }
  FunctionScopePtr func = ar->getFunctionScope(); // containing function scope
  FunctionScopePtr fs = m_funcScope.lock();
  // redeclared functions are automatically volatile
  if (func && fs->isVolatile()) {
    func->getVariables()->setAttribute(VariableTable::NeedGlobalPointer);
  }
  MethodStatement::analyzeProgramImpl(ar);
}

StatementPtr FunctionStatement::preOptimize(AnalysisResultPtr ar) {
  return MethodStatement::preOptimize(ar);
}

StatementPtr FunctionStatement::postOptimize(AnalysisResultPtr ar) {
  return MethodStatement::postOptimize(ar);
}

void FunctionStatement::inferTypes(AnalysisResultPtr ar) {
  MethodStatement::inferTypes(ar);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void FunctionStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  FunctionScopePtr funcScope = m_funcScope.lock();
  if (!funcScope->isUserFunction()) return;
  if (ar) ar->pushScope(funcScope);

  cg.printf("function ");
  if (m_ref) cg.printf("&");
  cg.printf("%s(", m_name.c_str());
  if (m_params) m_params->outputPHP(cg, ar);
  cg.indentBegin(") {\n");

  m_funcScope.lock()->outputPHP(cg, ar);
  if (m_stmt) m_stmt->outputPHP(cg, ar);
  cg.indentEnd("}\n");

  if (ar) ar->popScope();
}

bool FunctionStatement::hasImpl() const {
  return m_funcScope.lock()->isVolatile();
}

void FunctionStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  FunctionScopePtr funcScope = m_funcScope.lock();
  string fname = funcScope->getId().c_str();
  bool pseudoMain = funcScope->inPseudoMain();

  if (outputFFI(cg, ar)) return;

  if (cg.getContext() == CodeGenerator::NoContext) {
    if (funcScope->isRedeclaring()) {
      cg.printf("g->%s%s = %s%s;\n",
                Option::InvokePrefix, m_name.c_str(),
                Option::InvokePrefix, fname.c_str());
      cg.printf("g->%s%s_few_args = %s%s_few_args;\n",
                Option::InvokePrefix, m_name.c_str(),
                Option::InvokePrefix, fname.c_str());
    }
    if (funcScope->isVolatile()) {
      cg.printf("g->declareFunction(\"%s\");\n",
                m_name.c_str());
    }
    return;
  }

  if (cg.getContext() == CodeGenerator::CppDeclaration &&
      !funcScope->isInlined()) return;

  if (cg.getContext() == CodeGenerator::CppPseudoMain &&
      !pseudoMain) return;
  if (cg.getContext() == CodeGenerator::CppImplementation &&
      (funcScope->isInlined() || pseudoMain)) return;
  ar->pushScope(funcScope);

  cg.setPHPLineNo(-1);

  if (pseudoMain && !Option::GenerateCPPMain) {
    if (cg.getContext() == CodeGenerator::CppPseudoMain) {
      if (cg.getOutput() != CodeGenerator::SystemCPP) {
        cg.setContext(CodeGenerator::NoContext); // no inner functions/classes
        funcScope->getVariables()->setAttribute(VariableTable::ForceGlobal);
        outputCPPStmt(cg, ar);
        funcScope->getVariables()->clearAttribute(VariableTable::ForceGlobal);
        cg.setContext(CodeGenerator::CppPseudoMain);
        ar->popScope();
        return;
      }
    } else if (cg.getContext() == CodeGenerator::CppForwardDeclaration &&
               cg.getOutput() != CodeGenerator::SystemCPP) {
      return;
    }
  }

  if (cg.getContext() == CodeGenerator::CppImplementation) {
    printSource(cg);
  }

  if (funcScope->isInlined()) cg.printf("inline ");

  TypePtr type = funcScope->getReturnType();
  if (type) {
    type->outputCPPDecl(cg, ar);
  } else {
    cg.printf("void");
  }

  if (pseudoMain) {
    cg.printf(" %s%s(", Option::PseudoMainPrefix,
              funcScope->getFileScope()->pseudoMainName().c_str());
  } else {
    cg.printf(" %s%s(", Option::FunctionPrefix, fname.c_str());
  }

  CodeGenerator::Context context = cg.getContext();
  switch (context) {
  case CodeGenerator::CppForwardDeclaration:
    funcScope->outputCPPParamsDecl(cg, ar, m_params, true);
    cg.printf(");\n");
    break;
  case CodeGenerator::CppDeclaration:
  case CodeGenerator::CppImplementation:
  case CodeGenerator::CppPseudoMain:
    {
      funcScope->outputCPPParamsDecl(cg, ar, m_params, false);
      cg.indentBegin(") {\n");
      if (pseudoMain) {
        cg.indentBegin("{\n");
        cg.printDeclareGlobals();
        cg.printf("bool &alreadyRun = g->run_%s%s;\n",
                  Option::PseudoMainPrefix,
                  funcScope->getFileScope()->pseudoMainName().c_str());
        cg.printf("if (alreadyRun) { if (incOnce) return true;}\n");
        cg.printf("else alreadyRun = true;\n");
        cg.printf("if (!variables) variables = g;\n");
        cg.indentEnd("}\n");
        cg.printf("PSEUDOMAIN_INJECTION(run_init::%s);\n",
                  funcScope->getFileScope()->getName().c_str());
      } else {
        cg.printf("FUNCTION_INJECTION(%s);\n",
                  funcScope->getOriginalName().c_str());
        if (Option::GenRTTIProfileData && m_params) {
          for (int i = 0; i < m_params->getCount(); i++) {
            ParameterExpressionPtr param =
              dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
            if (param->hasRTTI()) {
              const string &paramName = param->getName();
              int id = ar->getParamRTTIEntryId(ClassScopePtr(),
                                               funcScope, paramName);
              if (id != -1) {
                cg.printf("RTTI_INJECTION(%s%s, %d);\n",
                          Option::VariablePrefix, paramName.c_str(), id);
              }
            }
          }
        }
      }
      funcScope->outputCPP(cg, ar);
      cg.setContext(CodeGenerator::NoContext); // no inner functions/classes
      outputCPPStmt(cg, ar);
      cg.setContext(context);
      cg.indentEnd("} /* function */\n");
    }
    break;
  default:
    ASSERT(false);
  }

  ar->popScope();
}

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
                    docComment, false), m_ignored(false) {
}

StatementPtr FunctionStatement::clone() {
  FunctionStatementPtr stmt(new FunctionStatement(*this));
  stmt->m_stmt = Clone(m_stmt);
  stmt->m_params = Clone(m_params);
  stmt->m_modifiers = Clone(m_modifiers);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void FunctionStatement::onParse(AnalysisResultConstPtr ar, FileScopePtr scope) {
  // note it's important to add to scope, not a pushed FunctionContainer,
  // as a function may be declared inside a class's method, yet this function
  // is a global function, not a class method.
  FunctionScopePtr fs = onInitialParse(ar, scope);
  if (m_params) {
    for (int i = 0; i < m_params->getCount(); i++) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
      param->parseHandler(ar, fs, ClassScopePtr());
    }
  }
  FunctionScope::RecordFunctionInfo(m_name, fs);
  if (!scope->addFunction(ar, fs)) {
    m_ignored = true;
    return;
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

std::string FunctionStatement::getName() const {
  return string("Function ") + getScope()->getName();
}

void FunctionStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  FunctionScopePtr func = getScope()->getOuterScope()->getContainingFunction();
  FunctionScopeRawPtr fs = getFunctionScope();
  // redeclared functions are automatically volatile
  if (func && fs->isVolatile()) {
    func->getVariables()->setAttribute(VariableTable::NeedGlobalPointer);
  }
  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    ar->recordFunctionSource(m_name, m_loc, getFileScope()->getName());
  }
  MethodStatement::analyzeProgramImpl(ar);
}

void FunctionStatement::inferTypes(AnalysisResultPtr ar) {
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void FunctionStatement::outputPHPHeader(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  cg_printf("function ");
  if (m_ref) cg_printf("&");
  if (m_name[0] != '0') {
    cg_printf("%s", m_name.c_str());
  }
  cg_printf("(");
  if (m_params) m_params->outputPHP(cg, ar);
  cg_printf(")");
}

void FunctionStatement::outputPHPBody(CodeGenerator &cg,
                                      AnalysisResultPtr ar) {
  FunctionScopeRawPtr funcScope = getFunctionScope();
  cg_indentBegin(" {\n");
  funcScope->outputPHP(cg, ar);
  if (m_stmt) m_stmt->outputPHP(cg, ar);
  cg_indentEnd("}\n");
}

void FunctionStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  FunctionScopeRawPtr funcScope = getFunctionScope();
  if (funcScope->isUserFunction()) {
    outputPHPHeader(cg, ar);
    outputPHPBody(cg, ar);
  }
}

bool FunctionStatement::hasImpl() const {
  return getFunctionScope()->isVolatile();
}

void FunctionStatement::outputCPPImpl(CodeGenerator &cg,
                                      AnalysisResultPtr ar) {
  CodeGenerator::Context context = cg.getContext();

  FunctionScopeRawPtr funcScope = getFunctionScope();
  string fname = funcScope->getId(cg);
  bool pseudoMain = funcScope->inPseudoMain();
  string origFuncName = !pseudoMain ? funcScope->getOriginalName() :
          ("run_init::" + funcScope->getContainingFile()->getName());

  if (outputFFI(cg, ar)) return;

  if (context == CodeGenerator::NoContext) {
    if (funcScope->isVolatile()) {
      string rname = cg.formatLabel(m_name);
      if (funcScope->isRedeclaring()) {
        cg.printf("g->GCI(%s) = &%s%s;\n", rname.c_str(),
                  Option::CallInfoPrefix, fname.c_str());
      }
      cg_printf("g->declareFunctionLit(");
      cg_printString(m_name, ar, shared_from_this());
      cg_printf(");\n");
      cg_printf("g->FVF(%s) = true;\n", rname.c_str());
    }
    return;
  }

  if (context == CodeGenerator::CppDeclaration &&
      !funcScope->isInlined()) return;

  if (context == CodeGenerator::CppPseudoMain &&
      !pseudoMain) return;
  if (context == CodeGenerator::CppImplementation &&
      (funcScope->isInlined() || pseudoMain)) return;

  cg.setPHPLineNo(-1);

  if (pseudoMain && !Option::GenerateCPPMain) {
    if (context == CodeGenerator::CppPseudoMain) {
      if (cg.getOutput() != CodeGenerator::SystemCPP) {
        cg.setContext(CodeGenerator::NoContext); // no inner functions/classes
        funcScope->getVariables()->setAttribute(VariableTable::ForceGlobal);
        outputCPPStmt(cg, ar);
        funcScope->getVariables()->clearAttribute(VariableTable::ForceGlobal);
        cg.setContext(CodeGenerator::CppPseudoMain);
        return;
      }
    } else if (context == CodeGenerator::CppForwardDeclaration &&
               cg.getOutput() != CodeGenerator::SystemCPP) {
      return;
    }
  }

  if (context == CodeGenerator::CppImplementation) {
    printSource(cg);
  } else if (context == CodeGenerator::CppForwardDeclaration &&
             Option::GenerateCppLibCode) {
    cg_printf("\n");
    printSource(cg);
    cg.printDocComment(funcScope->getDocComment());
  }

  bool isWrapper = context == CodeGenerator::CppTypedParamsWrapperDecl ||
    context == CodeGenerator::CppTypedParamsWrapperImpl;

  bool needsWrapper = isWrapper ||
    (Option::HardTypeHints && funcScope->needsTypeCheckWrapper());

  int startLineImplementation = -1;
  if (context == CodeGenerator::CppDeclaration ||
      context == CodeGenerator::CppImplementation ||
      context == CodeGenerator::CppPseudoMain) {
    startLineImplementation = cg.getLineNo(CodeGenerator::PrimaryStream);
  }

  if (funcScope->isInlined()) cg_printf("inline ");

  TypePtr type = funcScope->getReturnType();
  if (type) {
    type->outputCPPDecl(cg, ar, getScope());
  } else {
    cg_printf("void");
  }

  if (Option::FunctionSections.find(origFuncName) !=
      Option::FunctionSections.end()) {
    string funcSection = Option::FunctionSections[origFuncName];
    if (!funcSection.empty()) {
      cg_printf(" __attribute__ ((section (\".text.%s\")))",
                funcSection.c_str());
    }
  }

  if (pseudoMain) {
    cg_printf(" %s%s(", Option::PseudoMainPrefix,
              funcScope->getContainingFile()->pseudoMainName().c_str());
  } else {
    cg_printf(" %s%s(",
              needsWrapper && !isWrapper ?
              Option::TypedFunctionPrefix : Option::FunctionPrefix,
              fname.c_str());
  }

  switch (context) {
    case CodeGenerator::CppForwardDeclaration:
    case CodeGenerator::CppTypedParamsWrapperDecl:
      funcScope->outputCPPParamsDecl(cg, ar, m_params, true);
      if (!isWrapper) {
        int opt = Option::GetOptimizationLevel(m_cppLength);
        if (opt < 3) cg_printf(") __attribute__((optimize(%d))", opt);
      }
      cg_printf(");\n");
      if (!isWrapper) {
        if (funcScope->hasDirectInvoke()) {
          cg_printf("Variant %s%s(void *extra, CArrRef params);\n",
                    Option::InvokePrefix, fname.c_str());
        }
        if (needsWrapper) {
          cg.setContext(CodeGenerator::CppTypedParamsWrapperDecl);
          outputCPPImpl(cg, ar);
          cg.setContext(context);
        }
      }
      break;
    case CodeGenerator::CppDeclaration:
    case CodeGenerator::CppImplementation:
    case CodeGenerator::CppPseudoMain:
    case CodeGenerator::CppTypedParamsWrapperImpl:
    {
      funcScope->outputCPPParamsDecl(cg, ar, m_params, false);
      cg_indentBegin(") {\n");
      if (!isWrapper) {
        const char *suffix =
          (cg.getOutput() == CodeGenerator::SystemCPP ? "_BUILTIN" : "");
        if (pseudoMain) {
          cg_printf("PSEUDOMAIN_INJECTION%s(%s, %s%s);\n",
                    suffix, origFuncName.c_str(), Option::PseudoMainPrefix,
                    funcScope->getContainingFile()->pseudoMainName().c_str());
        } else {
          if (m_stmt->hasBody()) {
            if (suffix[0] == '\0' && !funcScope->needsCheckMem()) {
              suffix = "_NOMEM";
            }
            cg_printf("FUNCTION_INJECTION%s(%s);\n", suffix,
                      origFuncName.c_str());
          }
          outputCPPArgInjections(cg, ar, origFuncName.c_str(),
                                 ClassScopePtr(), funcScope);
        }
        funcScope->outputCPP(cg, ar);
        if (funcScope->needsRefTemp()) cg.genReferenceTemp(shared_from_this());
        cg.setContext(CodeGenerator::NoContext); // no inner functions/classes
        outputCPPStmt(cg, ar);
        if (funcScope->needsRefTemp()) cg.clearRefereceTemp();
        cg_indentEnd("}\n");
        ASSERT(startLineImplementation >= 0);
        m_cppLength = cg.getLineNo(CodeGenerator::PrimaryStream)
                      - startLineImplementation;
        if (needsWrapper) {
          cg.setContext(CodeGenerator::CppTypedParamsWrapperImpl);
          outputCPPImpl(cg, ar);
        }
        cg.setContext(context);
      } else {
        outputCPPTypeCheckWrapper(cg, ar);
        cg_indentEnd("}\n");
      }
      cg.printImplSplitter();
    }
    break;
    default:
      ASSERT(false);
  }
}


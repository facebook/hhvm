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

#include <lib/analysis/function_scope.h>
#include <lib/analysis/analysis_result.h>
#include <lib/expression/modifier_expression.h>
#include <lib/expression/expression_list.h>
#include <lib/analysis/code_error.h>
#include <lib/statement/statement_list.h>
#include <lib/analysis/file_scope.h>
#include <lib/analysis/variable_table.h>
#include <util/logger.h>
#include <lib/option.h>
#include <lib/statement/method_statement.h>
#include <lib/statement/exp_statement.h>
#include <lib/expression/parameter_expression.h>
#include <lib/analysis/class_scope.h>
#include <util/util.h>
#include <cpp/base/class_info.h>
#include <lib/parser/hphp.tab.hpp>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

FunctionScope::FunctionScope(AnalysisResultPtr ar, bool method,
                             const std::string &name, StatementPtr stmt,
                             bool reference, int minParam, int maxParam,
                             ModifierExpressionPtr modifiers,
                             int attribute, FileScopePtr file,
                             bool inPseudoMain /* = false */)
  : BlockScope(name, stmt, BlockScope::FunctionScope),
    m_method(method), m_file(file), m_minParam(0), m_maxParam(0),
    m_attribute(attribute), m_refReturn(reference), m_modifiers(modifiers),
    m_virtual(false), m_overriding(false), m_redeclaring(-1),
    m_volatile(false), m_ignored(false), m_pseudoMain(inPseudoMain),
    m_magicMethod(false), m_system(false), m_inlineable(false),
    m_containsThis(false), m_callTempCountMax(0), m_callTempCountCurrent(0) {
  bool canInline = true;
  if (inPseudoMain) {
    canInline = false;
    m_variables->forceVariants(ar);
    setReturnType(ar, Type::Variant);
  }
  setParamCounts(minParam, maxParam);

  if (m_refReturn) {
    m_returnType = Type::Variant;
  }

  // FileScope's flags are from parser, but VariableTable has more flags
  // coming from type inference phase. So we are tranferring these two
  // flags just for better modularization between FileScope and VariableTable.
  if (m_attribute & FileScope::ContainsDynamicVariable) {
    m_variables->setAttribute(VariableTable::ContainsDynamicVariable);
  }
  if (m_attribute & FileScope::ContainsLDynamicVariable) {
    m_variables->setAttribute(VariableTable::ContainsLDynamicVariable);
  }
  if (m_attribute & FileScope::ContainsExtract) {
    m_variables->setAttribute(VariableTable::ContainsExtract);
  }
  if (m_attribute & FileScope::ContainsCompact) {
    m_variables->setAttribute(VariableTable::ContainsCompact);
  }
  if (m_attribute & FileScope::ContainsUnset) {
    m_variables->setAttribute(VariableTable::ContainsUnset);
  }
  if (m_attribute & FileScope::ContainsGetDefinedVars) {
    m_variables->setAttribute(VariableTable::ContainsGetDefinedVars);
  }

  if (m_stmt && Option::AllVolatile) {
    m_volatile = true;
  }

  m_dynamic = Option::isDynamicFunction(method, m_name) ||
    Option::EnableEval == Option::FullEval;
  if (modifiers) {
    m_virtual = modifiers->isAbstract();
  }

  if (m_stmt) {
    MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    StatementListPtr stmts = stmt->getStmts();
    if (stmts) {
      if (stmts->getRecursiveCount() > Option::InlineFunctionThreshold)
        canInline = false;
      for (int i = 0; i < stmts->getCount(); i++) {
        StatementPtr stmt = (*stmts)[i];
        stmt->setFileLevel();
        if (stmt->is(Statement::KindOfExpStatement)) {
          ExpStatementPtr expStmt = dynamic_pointer_cast<ExpStatement>(stmt);
          ExpressionPtr exp = expStmt->getExpression();
          exp->setTopLevel();
        }
      }
    }
  } else {
    canInline = false;
  }
  m_inlineable = canInline;
}

FunctionScope::FunctionScope(bool method, const std::string &name,
                             bool reference)
  : BlockScope(name, StatementPtr(), BlockScope::FunctionScope),
    m_method(method), m_minParam(0), m_maxParam(0),
    m_attribute(0), m_refReturn(reference),
    m_modifiers(ModifierExpressionPtr()),
    m_virtual(false), m_overriding(false), m_redeclaring(-1),
    m_volatile(false), m_ignored(false),
    m_pseudoMain(false), m_magicMethod(false), m_system(true),
    m_inlineable(false), m_callTempCountMax(0), m_callTempCountCurrent(0) {
  m_dynamic = Option::isDynamicFunction(method, m_name);
}

void FunctionScope::setParamCounts(int minParam, int maxParam) {
  m_minParam = minParam;
  m_maxParam = maxParam;
  ASSERT(m_minParam >= 0 && m_maxParam >= m_minParam);
  if (m_maxParam > 0) {
    m_paramNames.resize(m_maxParam);
    m_paramTypes.resize(m_maxParam);
    m_refs.resize(m_maxParam);

    if (m_stmt) {
      MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
      ExpressionListPtr params = stmt->getParams();

      for (int i = 0; i < m_maxParam; i++) {
        if (stmt->isRef(i)) m_refs[i] = true;

        ParameterExpressionPtr param =
          dynamic_pointer_cast<ParameterExpression>((*params)[i]);
        m_paramNames[i] = param->getName();
      }
    }
  }
}

bool FunctionScope::isPublic() const {
  return m_modifiers && m_modifiers->isPublic();
}

bool FunctionScope::isProtected() const {
  return m_modifiers && m_modifiers->isProtected();
}

bool FunctionScope::isPrivate() const {
  return m_modifiers && m_modifiers->isPrivate();
}

bool FunctionScope::isStatic() const {
  return m_modifiers && m_modifiers->isStatic();
}

bool FunctionScope::isAbstract() const {
  return m_modifiers && m_modifiers->isAbstract();
}

bool FunctionScope::isFinal() const {
  return m_modifiers && m_modifiers->isFinal();
}

bool FunctionScope::isVariableArgument() const {
  return m_attribute & FileScope::VariableArgument;
}

bool FunctionScope::isReferenceVariableArgument() const {
  return m_attribute & FileScope::ReferenceVariableArgument;
}

void FunctionScope::setVariableArgument(bool reference) {
  m_attribute |= FileScope::VariableArgument;
  if (reference) {
    m_attribute |= FileScope::ReferenceVariableArgument;
  }
}

bool FunctionScope::hasEffect() const {
  return (m_attribute & FileScope::NoEffect) == 0;
}

void FunctionScope::setNoEffect() {
  m_attribute |= FileScope::NoEffect;
}

bool FunctionScope::isHelperFunction() const {
  return m_attribute & FileScope::HelperFunction;
}

void FunctionScope::setHelperFunction() {
  m_attribute |= FileScope::HelperFunction;
}

bool FunctionScope::containsReference() const {
  return m_attribute & FileScope::ContainsReference;
}

bool FunctionScope::hasImpl() const {
  if (!isUserFunction()) {
    return !isAbstract();
  }
  if (m_stmt) {
    MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    return stmt->getStmts();
  }
  return false;
}

bool FunctionScope::isConstructor(ClassScopePtr cls) const {
  return m_stmt && cls
    && (getName() == "__construct"
     || cls->classNameCtor() && getName() == cls->getName());
}

std::string FunctionScope::getOriginalName() const {
  if (m_pseudoMain) return "";
  if (m_stmt) {
    MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    return stmt->getOriginalName();
  }
  return m_name;
}

std::string FunctionScope::getFullName() const {
  if (m_stmt) {
    MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    return stmt->getFullName();
  }
  return m_name;
}

///////////////////////////////////////////////////////////////////////////////


int FunctionScope::inferParamTypes(AnalysisResultPtr ar, ConstructPtr exp,
                                   ExpressionListPtr params, bool &valid) {
  valid = true;

  if (!params) {
    if (m_minParam > 0) {
      if (ar->isFirstPass()) {
        ar->getCodeError()->record(CodeError::TooFewArgument, exp, m_stmt);
      }
      valid = false;
    }
    return 0;
  }

  int ret = 0;
  if (params->getCount() < m_minParam) {
    if (ar->isFirstPass()) {
      ar->getCodeError()->record(CodeError::TooFewArgument, exp, m_stmt);
    }
    valid = false;
  }
  if (params->getCount() > m_maxParam) {
    if (isVariableArgument()) {
      ret = params->getCount() - m_maxParam;
    } else {
      if (ar->isFirstPass()) {
        ar->getCodeError()->record(CodeError::TooManyArgument, exp, m_stmt);
      }
      params->setOutputCount(m_maxParam);
    }
  }

  bool canSetParamType = isUserFunction() && !m_overriding;
  for (int i = 0; i < params->getCount(); i++) {
    ExpressionPtr param = (*params)[i];
    param->clearContext(Expression::InvokeArgument);
    TypePtr expType;
    if (!canSetParamType && i < m_maxParam) {
      expType = param->inferAndCheck(ar, getParamType(i), false);
    } else {
      expType = param->inferAndCheck(ar, NEW_TYPE(Some), false);
    }
    if ((i < m_maxParam && isRefParam(i)) ||
        (i >= m_maxParam && isReferenceVariableArgument())) {
      param->setContext(Expression::LValue);
      param->setContext(Expression::RefValue);
      param->inferAndCheck(ar, Type::Variant, true);
    } else if (!(param->getContext() & Expression::RefParameter)) {
      param->clearContext(Expression::LValue);
      param->clearContext(Expression::RefValue);
    }
    if (i < m_maxParam) {
      TypePtr paramType = getParamType(i);
      if (canSetParamType) {
        paramType = setParamType(ar, i, expType);
      }
      if (!Type::IsLegalCast(ar, expType, paramType) &&
          paramType->isNonConvertibleType()) {
        param->inferAndCheck(ar, paramType, true);
      }
      param->setExpectedType(paramType);
    }
  }
  return ret;
}

TypePtr FunctionScope::setParamType(AnalysisResultPtr ar, int index,
                                    TypePtr type) {
  ASSERT(index >= 0 && index < (int)m_paramTypes.size());
  TypePtr paramType = m_paramTypes[index];
  if (!paramType) paramType = NEW_TYPE(Some);
  type = Type::Coerce(ar, paramType, type);
  if (type && !Type::SameType(paramType, type)) {
    ar->incNewlyInferred();
    if (!ar->isFirstPass()) {
      Logger::Verbose("Corrected paramter type %s -> %s",
                      paramType->toString().c_str(), type->toString().c_str());
    }
  }
  m_paramTypes[index] = type;
  return type;
}

TypePtr FunctionScope::getParamType(int index) {
  ASSERT(index >= 0 && index < (int)m_paramTypes.size());
  TypePtr paramType = m_paramTypes[index];
  if (!paramType) {
    paramType = NEW_TYPE(Some);
    m_paramTypes[index] = paramType;
  }
  return paramType;
}

bool FunctionScope::isRefParam(int index) const {
  ASSERT(index >= 0 && index < (int)m_refs.size());
  return m_refs[index];
}

void FunctionScope::setRefParam(int index) {
  ASSERT(index >= 0 && index < (int)m_refs.size());
  m_refs[index] = true;
}

const std::string &FunctionScope::getParamName(int index) const {
  ASSERT(index >= 0 && index < (int)m_paramNames.size());
  return m_paramNames[index];
}

void FunctionScope::setParamName(int index, const std::string &name) {
  ASSERT(index >= 0 && index < (int)m_paramNames.size());
  m_paramNames[index] = name;
}

void FunctionScope::addModifier(int mod) {
  if (!m_modifiers) {
    m_modifiers =
      ModifierExpressionPtr(new ModifierExpression(LocationPtr(),
                              Expression::KindOfModifierExpression));
  }
  m_modifiers->add(mod);
}

void FunctionScope::setStatic() {
  if (!isStatic()) {
    addModifier(T_STATIC);
  }
}

void FunctionScope::setReturnType(AnalysisResultPtr ar, TypePtr type) {
  // no change can be made to virtual function's prototype
  if (m_overriding) return;

  if (m_returnType) {
    type = Type::Coerce(ar, m_returnType, type);
    if (type && !Type::SameType(m_returnType, type)) {
      ar->incNewlyInferred();
      if (!ar->isFirstPass()) {
        Logger::Verbose("Corrected function return type %s -> %s",
                        m_returnType->toString().c_str(),
                        type->toString().c_str());
      }
    }
  }
  if (!type->getName().empty()) {
    FileScopePtr fs = getFileScope();
    if (fs) fs->addClassDependency(ar, type->getName());
  }
  m_returnType = type;
}

void FunctionScope::setOverriding(TypePtr returnType,
                                  TypePtr param1 /* = TypePtr() */,
                                  TypePtr param2 /* = TypePtr() */) {
  m_returnType = returnType;
  m_overriding = true;

  if (param1 && m_paramTypes.size() >= 1) m_paramTypes[0] = param1;
  if (param2 && m_paramTypes.size() >= 2) m_paramTypes[1] = param2;

  // TODO: remove this block and replace with stronger typing
  // Right now, we have to avoid a situation where a parameter is assigned
  // with different values, making them a Variant.
  for (unsigned int i = 0; i < m_paramTypes.size(); i++) {
    m_paramTypes[i] = Type::Variant;
  }
}

///////////////////////////////////////////////////////////////////////////////

void FunctionScope::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (Option::GenerateInferredTypes && m_returnType) {
    cg.printf("// @return %s\n", m_returnType->toString().c_str());
  }

  BlockScope::outputPHP(cg, ar);
}

void FunctionScope::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  for (int i = 0; i < m_callTempCountMax; i++) {
    cg.printf("Variant %s%d;\n", Option::EvalOrderTempPrefix, i);
  }
  BlockScope::outputCPP(cg, ar);
}

void FunctionScope::outputCPPParamsDecl(CodeGenerator &cg,
                                        AnalysisResultPtr ar,
                                        ExpressionListPtr params,
                                        bool showDefault) {
  if (isVariableArgument()) {
    cg.printf("int num_args, ");
    if (params) {
      params->outputCPP(cg, ar);
      cg.printf(", ");
    }
    if (showDefault) {
      cg.printf("Array args = Array()");
    } else {
      cg.printf("Array args /* = Array() */");
    }
  } else if (m_pseudoMain) {
    if (showDefault) {
      cg.printf("bool incOnce = false, LVariableTable* variables = NULL");
    } else {
      cg.printf("bool incOnce /* = false */, "
                "LVariableTable* variables /* = NULL */");
    }
  } else if (params) {
    params->outputCPP(cg, ar);
  }
}


void FunctionScope::outputCPPParamsImpl(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  int paramcount = getMaxParamCount();
  if (isUserFunction()) {
    outputCPPParamsDecl(cg, ar, dynamic_pointer_cast<MethodStatement>(getStmt())
                        ->getParams(), false);
    return;
  }
  bool first = true;
  if (isVariableArgument()) {
    cg.printf("int num_args");
    first = false;
  }

  for (int i = 0; i < paramcount; i++) {
    if (first) {
      first = false;
    } else {
      cg.printf(", ");
    }
    TypePtr type = getParamType(i);
    type->outputCPPDecl(cg, ar);
    cg.printf(" a%d", i);
  }

  if (isVariableArgument()) {
    cg.printf(", Array args /* = Array() */");
  }
}

void FunctionScope::outputCPPParamsCall(CodeGenerator &cg,
                                        AnalysisResultPtr ar,
                                        bool aggregateParams) {
  if (isVariableArgument()) {
    cg.printf("num_args, ");
  }
  bool userFunc = isUserFunction();
  ExpressionListPtr params;
  if (userFunc) {
    MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    params = stmt->getParams();
  }
  if (aggregateParams) {
    cg.printf("Array(");
    if (m_maxParam) cg.printf("ArrayInit(%d).", m_maxParam);
  }
  for (int i = 0; i < m_maxParam; i++) {
    if (i > 0) cg.printf(aggregateParams ? "." : ", ");
    if (aggregateParams) {
      cg.printf("set(%d, ", i);
    }
    bool isRef;
    if (userFunc) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*params)[i]);
      isRef = param->isRef();
      cg.printf(aggregateParams ? "ArrayElement(%sv_%s%s)" : "%sv_%s%s",
                isRef ? "ref(" : "", param->getName().c_str(),
                isRef ? ")" : "");
    } else {
      isRef = isRefParam(i);
      cg.printf(aggregateParams ? "ArrayElement(%sa%d%s)" : "%sa%d%s",
                isRef ? "ref(" : "", i, isRef ? ")" : "");
    }
    if (aggregateParams) cg.printf(")");
  }
  if (aggregateParams) {
    if (m_maxParam) cg.printf(".create()");
    cg.printf(")");
  }
  if (isVariableArgument()) {
    if (aggregateParams || m_maxParam > 0) cg.printf(",");
    cg.printf("args");
  }
}

void FunctionScope::outputCPPArguments(ExpressionListPtr params,
                                       CodeGenerator &cg,
                                       AnalysisResultPtr ar, int extraArg,
                                       bool variableArgument) {
  int paramCount = params ? params->getOutputCount() : 0;
  ASSERT(extraArg <= paramCount);
  int iMax = paramCount - extraArg;
  bool extra = false;

  if (variableArgument) {
    if (paramCount == 0) {
      cg.printf("0");
    } else {
      cg.printf("%d, ", paramCount);
    }
  }
  bool hasEffect = params && params->controllingOrder();
  int tempOffset = params ? params->tempOffset() : 0;
  int firstExtra = 0;
  for (int i = 0; i < paramCount; i++) {
    ExpressionPtr param = (*params)[i];
    cg.setItemIndex(i);
    if (i > 0) cg.printf(extra ? "." : ", ");
    if (!extra && (i == iMax || extraArg < 0)) {
      extra = true;
      cg.printf("Array(ArrayInit(%d).", paramCount - i);
      firstExtra = i;
    }
    if (extra) {
      cg.printf("set(%d, ArrayElement(", i - firstExtra);
      if (hasEffect && !param->isScalar()) {
        cg.printf("%s%d", Option::EvalOrderTempPrefix, tempOffset + i);
      } else {
        param->outputCPP(cg, ar);
      }
      cg.printf("))");
    } else {
      if (hasEffect && !param->isScalar()) {
        cg.printf("%s%d", Option::EvalOrderTempPrefix, tempOffset + i);
      } else {
        param->outputCPP(cg, ar);
      }
    }
  }
  if (extra) {
    cg.printf(".create())");
  }
}

void FunctionScope::outputCPPEffectiveArguments(ExpressionListPtr params,
                                                CodeGenerator &cg,
                                                AnalysisResultPtr ar) {
  int paramCount = params ? params->getCount() : 0;
  for (int i = 0; i < paramCount; i++) {
    ExpressionPtr param = (*params)[i];
    if (param->hasEffect()) {
      param->outputCPP(cg, ar);
      cg.printf(", ");
    }
  }
}

void FunctionScope::outputCPPDynamicInvoke(CodeGenerator &cg,
                                           AnalysisResultPtr ar,
                                           const char *funcPrefix,
                                           const char *name,
                                           bool voidWrapperOff /* = false */,
                                           bool fewArgs /* = false */,
                                           bool ret /* = true */,
                                           const char *extraArg /* = NULL */) {
  const char *voidWrapper = (m_returnType || voidWrapperOff) ? "" : ", null";
  const char *retrn = ret ? "return " : "";
  int maxParam = fewArgs ? (m_maxParam > Option::InvokeFewArgsCount ?
                            Option::InvokeFewArgsCount : m_maxParam)
    : m_maxParam;
  bool variable = isVariableArgument();
  if (variable || getOptionalParamCount()) {
    if (!fewArgs) {
      cg.printf("int count = params.size();\n");
    }
    cg.printf("if (count <= %d) ", m_minParam);
  }

  stringstream callss;
  callss << retrn << (m_refReturn ? "ref(" : "(") << funcPrefix << name << "(";
  if (extraArg) {
    callss << extraArg;
    if (variable) {
      callss << ",";
    }
  }
  if (variable) {
    callss << "count";
  }
  bool preArgs = variable || extraArg;
  string call = callss.str();

  cg.printf("%s", call.c_str());
  for (int i = 0; i < m_minParam; i++) {
    if (preArgs || i > 0) cg.printf(", ");
    if (isRefParam(i)) {
      if (fewArgs) {
        cg.printf("ref(a%d)", i);
      } else {
        cg.printf("ref(const_cast<Array&>(params).lvalAt(%d))", i);
      }
    } else {
      if (fewArgs) {
        cg.printf("a%d", i);
      } else {
        cg.printf("params.rvalAt(%d)", i);
      }
    }
  }
  cg.printf(")%s);\n", voidWrapper);

  for (int iMax = m_minParam + 1; iMax <= maxParam; iMax++) {
    if (iMax < maxParam || variable) {
      cg.printf("if (count == %d) ", iMax);
    }
    cg.printf("%s", call.c_str());
    for (int i = 0; i < iMax; i++) {
      if (preArgs || i > 0) cg.printf(", ");
      if (isRefParam(i)) {
        if (fewArgs) {
          cg.printf("ref(a%d)", i);
        } else {
          cg.printf("ref(const_cast<Array&>(params).lvalAt(%d))", i);
        }
      } else {
        if (fewArgs) {
          cg.printf("a%d", i);
        } else {
          cg.printf("params.rvalAt(%d)", i);
        }
      }
    }
    cg.printf(")%s);\n", voidWrapper);
  }

  if (variable) {
    if (fewArgs) {
      if (maxParam == Option::InvokeFewArgsCount) return;
      cg.printf("Array params;\n");
      for (int i = maxParam; i < Option::InvokeFewArgsCount; i++) {
        cg.printf(
          "if (count >= %d) params.append(a%d);\n", i + 1, i);
      }
    }
    cg.printf("%s,", call.c_str());
    for (int i = 0; i < maxParam; i++) {
      if (isRefParam(i)) {
        if (fewArgs) {
          cg.printf("ref(a%d), ", i);
        } else {
          cg.printf("ref(const_cast<Array&>(params).lvalAt(%d)), ", i);
        }
      } else {
        if (fewArgs) {
          cg.printf("a%d, ", i);
        } else {
          cg.printf("params.rvalAt(%d), ", i);
        }
      }
    }
    if (fewArgs) {
      cg.printf("params)%s);\n", voidWrapper);
    }
    else {
      cg.printf("params.slice(%d, count - %d, false))%s);\n",
                maxParam, maxParam, voidWrapper);
    }
  }
}

void FunctionScope::outputCPPEvalInvoke(CodeGenerator &cg,
                                        AnalysisResultPtr ar,
                                        const char *funcPrefix,
                                        const char *name,
                                        bool profile,
                                        const char *extraArg /* = NULL */) {
  const char *voidWrapper = m_returnType  ? "" : ", null";
  const char *retrn = "return ";
  int maxParam = m_maxParam;
  bool variable = isVariableArgument();

  stringstream callss;
  callss << retrn << (m_refReturn ? "ref(" : "(") << funcPrefix << name << "(";
  if (extraArg) {
    callss << extraArg;
    if (variable) {
      callss << ",";
    }
  }
  if (variable) {
    callss << "count";
  }
  bool preArgs = variable || extraArg;
  // Build temps
  for (int i = 0; i < m_maxParam; i++) {
    cg.printf("Variant a%d;\n", i);
  }
  cg.printf("const std::vector<Eval::ExpressionPtr> &params = "
            "caller->params();\n");
  cg.printf("std::vector<Eval::ExpressionPtr>::const_iterator it = "
            "params.begin();\n");
  cg.indentBegin("do {\n");
  for (int i = 0; i < m_maxParam; i++) {
    cg.printf("if (it == params.end()) break;\n");
    if (i < m_minParam && (preArgs || i > 0)) {
      callss << ", ";
    }
    if (isRefParam(i)) {
      if (i < m_minParam) callss << "ref(a" << i << ")";
      cg.printf("a%d = ref((*it)->refval(env));\n", i);
    } else {
      if (i < m_minParam) callss << "a" << i;
      cg.printf("a%d = (*it)->eval(env);\n", i);
    }
    cg.printf("it++;\n");
  }
  cg.indentEnd("} while(false);\n");
  // Put extra args into vargs or just eval them
  if (variable) {
    cg.printf("Array vargs;\n");
  }
  cg.indentBegin("for (; it != params.end(); ++it) {\n");
  const char *paramEval = "(*it)->eval(env)";
  if (isReferenceVariableArgument()) {
    paramEval = "ref((*it)->refval(env))";
  }
  if (variable) cg.printf("vargs.append(");
  cg.printf(paramEval);
  if (variable) cg.printf(")");
  cg.printf(";\n");
  cg.indentEnd("}\n");

  if (profile) {
    cg.printf("FUNCTION_INJECTION(%s);\n", name);
  }

  if (variable || getOptionalParamCount()) {
    cg.printf("int count = params.size();\n");
    cg.printf("if (count <= %d) ", m_minParam);
  }

  // No optional args
  string call = callss.str();
  cg.printf("%s", call.c_str());
  cg.printf(")%s);\n", voidWrapper);

  // Optional args
  for (int iMax = m_minParam + 1; iMax <= maxParam; iMax++) {
    if (iMax < maxParam || variable) {
      cg.printf("if (count == %d) ", iMax);
    }
    cg.printf("%s", call.c_str());
    for (int i = m_minParam; i < iMax; i++) {
      if (i > 0 || preArgs) cg.printf(", ");
      if (isRefParam(i)) {
        cg.printf("ref(a%d)", i);
      } else {
        cg.printf("a%d", i);
      }
    }
    cg.printf(")%s);\n", voidWrapper);
  }

  // Var args
  if (variable) {
    cg.printf("%s,", call.c_str());
    for (int i = m_minParam; i < maxParam; i++) {
      if (isRefParam(i)) {
        cg.printf("ref(a%d), ", i);
      } else {
        cg.printf("a%d, ", i);
      }
    }
    cg.printf("vargs)%s);\n", voidWrapper);
  }
}

void FunctionScope::serialize(JSON::OutputStream &out) const {
  JSON::MapStream ms(out);
  int vis = 0;
  if (isPublic()) vis = ClassScope::Public;
  else if (isProtected()) vis = ClassScope::Protected;
  else if (isPrivate()) vis = ClassScope::Protected;

  int mod = 0;
  if (isAbstract()) mod = ClassScope::Abstract;
  else if (isFinal()) mod = ClassScope::Final;

  if (!m_returnType) {
    ms.add("retTp", -1);
  } else if (m_returnType->isSpecificObject()) {
    ms.add("retTp", m_returnType->getName());
  } else {
    ms.add("retTp", m_returnType->getKindOf());
  }
  ms.add("minArgs", m_minParam)
    .add("maxArgs", m_maxParam)
    .add("varArgs", isVariableArgument())
    .add("static", isStatic())
    .add("modifier", mod)
    .add("visibility", vis)
    .add("argIsRef", m_refs)
    .done();
}



void FunctionScope::outputCPPCreateDecl(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  ClassScopePtr scope = ar->getClassScope();

  cg.printf("public: ObjectData *create(");
  outputCPPParamsDecl(cg, ar,
                      dynamic_pointer_cast<MethodStatement>(getStmt())
                      ->getParams(), true);
  cg.printf(");\n");
  cg.printf("public: ObjectData *dynCreate(CArrRef params, bool init = true);\n");
  if (isDynamic())
    cg.printf("public: void dynConstruct(CArrRef params);\n");
}

void FunctionScope::outputCPPCreateImpl(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  ClassScopePtr scope = ar->getClassScope();
  string clsNameStr = scope->getId();
  const char *clsName = clsNameStr.c_str();
  const char *consName = scope->classNameCtor() ? scope->getName().c_str()
                                                : "__construct";

  cg.printf("ObjectData *%s%s::create(",
            Option::ClassPrefix, clsName);
  outputCPPParamsImpl(cg, ar);
  cg.indentBegin(") {\n");
  cg.printf("init();\n");
  cg.printf("%s%s(", Option::MethodPrefix, consName);
  outputCPPParamsCall(cg, ar, false);
  cg.printf(");\n");
  cg.printf("return this;\n");
  cg.indentEnd("}\n");
  cg.indentBegin("ObjectData *%s%s::dynCreate(CArrRef params, "
                 "bool init /* = true */) {\n",
                 Option::ClassPrefix, clsName);
  cg.indentBegin("if (init) {\n");
  outputCPPDynamicInvoke(cg, ar, "","create",
                         true);
  cg.indentEnd("} else return this;\n");
  cg.indentEnd("}\n");
  if (isDynamic()) {
    cg.indentBegin("void %s%s::dynConstruct(CArrRef params) {\n",
                   Option::ClassPrefix, clsName);
    outputCPPDynamicInvoke(cg, ar, Option::MethodPrefix,
                           consName, true, false, false);
    cg.indentEnd("}\n");
  }
}

void FunctionScope::outputCPPClassMap(CodeGenerator &cg, AnalysisResultPtr ar){
  int attribute = ClassInfo::IsNothing;
  if (!isUserFunction()) attribute |= ClassInfo::IsSystem;
  if (isRedeclaring()) attribute |= ClassInfo::IsRedeclared;
  if (isVolatile()) attribute |= ClassInfo::IsVolatile;
  if (isRefReturn()) attribute |= ClassInfo::IsReference;

  if (isProtected()) {
    attribute |= ClassInfo::IsProtected;
  } else if (isPrivate()) {
    attribute |= ClassInfo::IsPrivate;
  } else {
    attribute |= ClassInfo::IsPublic;
  }
  if (isAbstract()) attribute |= ClassInfo::IsAbstract;
  if (isStatic()) attribute |= ClassInfo::IsStatic;
  if (isFinal()) attribute |= ClassInfo::IsFinal;

  cg.printf("(const char *)0x%04X, \"%s\", NULL, NULL,\n", attribute,
            m_name.c_str());

  for (int i = 0; i < m_maxParam; i++) {
    int attr = ClassInfo::IsNothing;
    if (i >= m_minParam) attr |= ClassInfo::IsOptional;
    if (isRefParam(i)) attr |= ClassInfo::IsReference;

    cg.printf("(const char *)0x%04X, \"%s\", \"%s\", \"%s\",\n",
              attr, m_paramNames[i].c_str(),
              Util::toLower(m_paramTypes[i]->getPHPName()).c_str(),
              (i >= m_minParam ? "1" : ""));

  }
  cg.printf("NULL,\n");

  m_variables->outputCPPStaticVariables(cg, ar);
}

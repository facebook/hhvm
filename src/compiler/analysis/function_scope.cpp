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

#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/expression/modifier_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/analysis/code_error.h>
#include <compiler/statement/statement_list.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/parser/parser.h>
#include <util/logger.h>
#include <compiler/option.h>
#include <compiler/statement/method_statement.h>
#include <compiler/statement/exp_statement.h>
#include <compiler/expression/parameter_expression.h>
#include <compiler/analysis/class_scope.h>
#include <util/util.h>
#include <runtime/base/class_info.h>
#include <runtime/base/builtin_functions.h>
#include <compiler/parser/hphp.tab.hpp>
#include <runtime/base/zend/zend_string.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

FunctionScope::FunctionScope(AnalysisResultPtr ar, bool method,
                             const std::string &name, StatementPtr stmt,
                             bool reference, int minParam, int maxParam,
                             ModifierExpressionPtr modifiers,
                             int attribute, const std::string &docComment,
                             FileScopePtr file,
                             bool inPseudoMain /* = false */)
    : BlockScope(name, docComment, stmt, BlockScope::FunctionScope),
    m_method(method), m_file(file), m_minParam(0), m_maxParam(0),
    m_attribute(attribute), m_refReturn(reference), m_modifiers(modifiers),
    m_virtual(false), m_overriding(false), m_redeclaring(-1),
    m_volatile(false), m_pseudoMain(inPseudoMain),
    m_magicMethod(false), m_system(false), m_inlineable(false), m_sep(false),
    m_containsThis(false), m_staticMethodAutoFixed(false),
    m_callTempCountMax(0), m_callTempCountCurrent(0),
    m_nrvoFix(true) {
  bool canInline = true;
  if (inPseudoMain) {
    canInline = false;
    m_variables->forceVariants(ar);
    setReturnType(ar, Type::Variant);
  }
  setParamCounts(ar, minParam, maxParam);

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

  m_dynamic = Option::IsDynamicFunction(method, m_name) ||
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
  : BlockScope(name, "", StatementPtr(), BlockScope::FunctionScope),
    m_method(method), m_minParam(0), m_maxParam(0),
    m_attribute(0), m_refReturn(reference),
    m_modifiers(ModifierExpressionPtr()),
    m_virtual(false), m_overriding(false), m_redeclaring(-1),
    m_volatile(false), m_pseudoMain(false), m_magicMethod(false),
    m_system(true), m_inlineable(false), m_sep(false),
    m_containsThis(false), m_staticMethodAutoFixed(false),
    m_callTempCountMax(0), m_callTempCountCurrent(0),
    m_nrvoFix(true) {
  m_dynamic = Option::IsDynamicFunction(method, m_name);
}

void FunctionScope::setParamCounts(AnalysisResultPtr ar, int minParam,
                                   int maxParam) {
  m_minParam = minParam;
  m_maxParam = maxParam;
  ASSERT(m_minParam >= 0 && m_maxParam >= m_minParam);
  if (m_maxParam > 0) {
    m_paramNames.resize(m_maxParam);
    m_paramTypes.resize(m_maxParam);
    m_paramTypeSpecs.resize(m_maxParam);
    m_refs.resize(m_maxParam);

    if (m_stmt) {
      MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
      ExpressionListPtr params = stmt->getParams();

      for (int i = 0; i < m_maxParam; i++) {
        if (stmt->isRef(i)) m_refs[i] = true;

        ParameterExpressionPtr param =
          dynamic_pointer_cast<ParameterExpression>((*params)[i]);
        m_paramNames[i] = param->getName();
        m_paramTypeSpecs[i] = param->getTypeSpec(ar);
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

bool FunctionScope::isMixedVariableArgument() const {
  return (m_name == "array_multisort");
}

bool FunctionScope::hasEffect() const {
  return (m_attribute & FileScope::NoEffect) == 0;
}

void FunctionScope::setNoEffect() {
  if (!(m_attribute & FileScope::NoEffect)) {
    m_attribute |= FileScope::NoEffect;
    Construct::recomputeEffects();
  }
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
  if (!params) {
    if (m_minParam > 0) {
      if (ar->isFirstPass()) {
        ar->getCodeError()->record(CodeError::TooFewArgument, exp, m_stmt);
      }
      valid = false;
      setDynamic();
    }
    return 0;
  }

  int ret = 0;
  if (params->getCount() < m_minParam) {
    if (ar->isFirstPass()) {
      ar->getCodeError()->record(CodeError::TooFewArgument, exp, m_stmt);
    }
    valid = false;
    setDynamic();
  }
  if (params->getCount() > m_maxParam) {
    if (isVariableArgument()) {
      ret = params->getCount() - m_maxParam;
    } else {
      if (ar->isFirstPass()) {
        ar->getCodeError()->record(CodeError::TooManyArgument, exp, m_stmt);
      }
      valid = false;
      setDynamic();
    }
  }

  bool canSetParamType = isUserFunction() && !m_overriding;
  for (int i = 0; i < params->getCount(); i++) {
    ExpressionPtr param = (*params)[i];
    if (valid && param->hasContext(Expression::InvokeArgument)) {
      param->clearContext(Expression::InvokeArgument);
      param->clearContext(Expression::RefValue);
      param->clearContext(Expression::NoRefWrapper);
    }
    TypePtr expType;
    if (!canSetParamType && i < m_maxParam) {
      expType = param->inferAndCheck(ar, getParamType(i), false);
    } else {
      expType = param->inferAndCheck(ar, NEW_TYPE(Some), false);
    }
    bool isRefVararg = (i >= m_maxParam && isReferenceVariableArgument());
    if ((i < m_maxParam && isRefParam(i)) || isRefVararg) {
      param->setContext(Expression::LValue);
      param->setContext(Expression::RefValue);
      param->inferAndCheck(ar, Type::Variant, true);
    } else if (!(param->getContext() & Expression::RefParameter)) {
      param->clearContext(Expression::LValue);
      param->clearContext(Expression::RefValue);
      param->clearContext(Expression::InvokeArgument);
      param->clearContext(Expression::NoRefWrapper);
    }
    if (i < m_maxParam) {
      if (m_paramTypeSpecs[i] && ar->isFirstPass()) {
        if (!Type::Inferred(ar, expType, m_paramTypeSpecs[i])) {
          const char *file = m_stmt->getLocation()->file;
          Logger::Error("%s: parameter %d of %s requires %s, called with %s",
                        file, i, m_name.c_str(),
                        m_paramTypeSpecs[i]->toString().c_str(),
                        expType->toString().c_str());
          ar->getCodeError()->record(CodeError::BadArgumentType, m_stmt);
        }
      }
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
    // we do a best-effort check for bad pass-by-reference and do not report
    // error for some vararg case (e.g., array_multisort can have either ref
    // or value for the same vararg).
    if (!isRefVararg || !isMixedVariableArgument()) {
      Expression::checkPassByReference(ar, param);
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
      Logger::Verbose("Corrected type of parameter %d of %s: %s -> %s",
                      index, m_name.c_str(),
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

void FunctionScope::setStaticMethodAutoFixed() {
  m_staticMethodAutoFixed = true;
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

  ExpressionListPtr params =
    dynamic_pointer_cast<MethodStatement>(getStmt())->getParams();

  /* Typecheck parameters */
  for (size_t i = 0; i < m_paramTypes.size(); i++) {
    TypePtr specType = m_paramTypeSpecs[i];
    if (!specType)
      continue;
    if (specType->is(Type::KindOfSome) || specType->is(Type::KindOfAny)
      || specType->is(Type::KindOfVariant))
      continue;

    /* If there's any possible runtime conflict, this will be turned into
     * a Variant; if it hasn't been, then we don't need to worry */
    if (!getParamType(i)->is(Type::KindOfVariant))
      continue;

    ParameterExpressionPtr param =
      dynamic_pointer_cast<ParameterExpression>((*params)[i]);

    /* Insert runtime checks. */
    if (specType->is(Type::KindOfArray)) {
      cg.printf("if(!%s%s.isArray())\n",
                Option::VariablePrefix, param->getName().c_str());
      cg.printf("  throw_unexpected_argument_type(%d,\"%s\",\"array\",%s%s);\n",
                i, m_name.c_str(),
                Option::VariablePrefix, param->getName().c_str());
    } else if (specType->is(Type::KindOfObject)) {
      cg.printf("if(!%s%s.instanceof(\"%s\"))\n", Option::VariablePrefix,
                param->getName().c_str(), specType->getName().c_str());
      cg.printf("  throw_unexpected_argument_type(%d,\"%s\",\"%s\",%s%s);\n",
                i, m_name.c_str(), specType->getName().c_str(),
                Option::VariablePrefix, param->getName().c_str());
    } else {
      Logger::Error("parameter %d of %s: improper type hint %s",
                    i, m_name.c_str(), specType->toString().c_str());
      return;
    }
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
      ar->setInExpression(true);
      params->outputCPP(cg, ar);
      ar->setInExpression(false);
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
    ar->setInExpression(true);
    params->outputCPP(cg, ar);
    ar->setInExpression(false);
  }
}


void FunctionScope::outputCPPParamsImpl(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  int paramcount = getMaxParamCount();
  if (isUserFunction()) {
    MethodStatementPtr m = dynamic_pointer_cast<MethodStatement>(getStmt());
    outputCPPParamsDecl(cg, ar, m->getParams(), false);
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
    if (m_maxParam) {
      // param arrays are always vectors
      cg.printf("ArrayInit(%d, true).", m_maxParam);
    }
  }
  for (int i = 0; i < m_maxParam; i++) {
    if (i > 0) cg.printf(aggregateParams ? "." : ", ");
    bool isRef;
    if (userFunc) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*params)[i]);
      isRef = param->isRef();
      if (aggregateParams) {
        cg.printf("set%s(%d, v_%s", isRef ? "Ref" : "", i,
                                    param->getName().c_str());
      } else {
        cg.printf("%sv_%s%s",
                  isRef ? "ref(" : "", param->getName().c_str(),
                  isRef ? ")" : "");
      }
    } else {
      isRef = isRefParam(i);
      if (aggregateParams) {
        cg.printf("set%s(%d, a%d", isRef ? "Ref" : "", i, i);
      } else {
        cg.printf("%sa%d%s",
                  isRef ? "ref(" : "", i, isRef ? ")" : "");
      }
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
                                       bool variableArgument,
                                       int extraArgArrayId /* = -1 */) {
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
  int firstExtra = 0;
  for (int i = 0; i < paramCount; i++) {
    ExpressionPtr param = (*params)[i];
    cg.setItemIndex(i);
    if (i > 0) cg.printf(extra ? "." : ", ");
    if (!extra && (i == iMax || extraArg < 0)) {
      if (extraArgArrayId != -1) {
        if (cg.getOutput() == CodeGenerator::SystemCPP) {
          cg.printf("SystemScalarArrays::%s[%d]",
                    Option::SystemScalarArrayName, extraArgArrayId);
        } else {
          cg.printf("ScalarArrays::%s[%d]",
                    Option::ScalarArrayName, extraArgArrayId);
        }
        break;
      }
      extra = true;
      // Parameter arrays are always vectors.
      cg.printf("Array(ArrayInit(%d, true).", paramCount - i);
      firstExtra = i;
    }
    if (extra) {
      bool needRef = param->hasContext(Expression::RefValue) &&
                     !param->hasContext(Expression::NoRefWrapper) &&
                     param->isRefable();
      cg.printf("set%s(%d, ", needRef ? "Ref" : "", i - firstExtra);
      if (needRef) {
        // The parameter itself shouldn't be wrapped with ref() any more.
        param->setContext(Expression::NoRefWrapper);
      }
      param->outputCPP(cg, ar);
      cg.printf(")");
    } else {
      param->outputCPP(cg, ar);
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

void FunctionScope::OutputCPPDynamicInvokeCount(CodeGenerator &cg) {
  cg.printf("int count __attribute__((__unused__)) = params.size();\n");
}

void FunctionScope::outputCPPInvokeArgCountCheck(CodeGenerator &cg,
    AnalysisResultPtr ar, bool ret, bool constructor) {
  bool variable = isVariableArgument();
  // system function has different handling of argument counts
  bool system = (m_system || m_sep ||
                 cg.getOutput() == CodeGenerator::SystemCPP);
  if (!system) {
    ClassScopePtr scope = ar->getClassScope();
    if (scope) system = (!scope->isUserClass() || scope->isExtensionClass() ||
                         scope->isSepExtension());
  }

  bool checkMissing = (m_minParam > 0);
  bool checkTooMany = (!variable && (system ||
                                     RuntimeOption::ThrowTooManyArguments));
  const char *sysret = (system && ret) ? "return " : "";
  const char *level = (system ? (constructor ? ", 2" : ", 1") : "");
  string fullname = getFullName();
  if (checkMissing && checkTooMany) {
    if (!variable && m_minParam == m_maxParam) {
      cg.printf("if (count != %d)"
                " %sthrow_wrong_arguments(\"%s\", count, %d, %d%s);\n",
                m_minParam, sysret, fullname.c_str(), m_minParam, m_maxParam,
                level);
    } else {
      cg.printf("if (count < %d || count > %d)"
                " %sthrow_wrong_arguments(\"%s\", count, %d, %d%s);\n",
                m_minParam, m_maxParam, sysret, fullname.c_str(),
                m_minParam, variable ? -1 : m_maxParam, level);
    }
  } else if (checkMissing) {
    cg.printf("if (count < %d)"
              " %sthrow_missing_arguments(\"%s\", count+1%s);\n",
              m_minParam, sysret, fullname.c_str(), level);
  } else if (checkTooMany) {
    cg.printf("if (count > %d)"
              " %sthrow_toomany_arguments(\"%s\", %d%s);\n",
              m_maxParam, sysret, fullname.c_str(), m_maxParam, level);
  }
}

void FunctionScope::outputCPPDynamicInvoke(CodeGenerator &cg,
                                           AnalysisResultPtr ar,
                                           const char *funcPrefix,
                                           const char *name,
                                           bool voidWrapperOff /* = false */,
                                           bool fewArgs /* = false */,
                                           bool ret /* = true */,
                                           const char *extraArg /* = NULL */,
                                           bool constructor /* = false */) {
  const char *voidWrapper = (m_returnType || voidWrapperOff) ? "" : ", null";
  const char *retrn = ret ? "return " : "";
  int maxParam = fewArgs ? (m_maxParam > Option::InvokeFewArgsCount ?
                            Option::InvokeFewArgsCount : m_maxParam)
    : m_maxParam;
  bool variable = isVariableArgument();

  ASSERT(m_minParam >= 0);

  outputCPPInvokeArgCountCheck(cg, ar, ret, constructor);

  if (variable || getOptionalParamCount()) {
    if (!fewArgs || m_minParam < Option::InvokeFewArgsCount) {
      cg.printf("if (count <= %d) ", m_minParam);
    }
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
        if (i < Option::InvokeFewArgsCount) {
          cg.printf("ref(a%d)", i);
        } else {
          cg.printf("null");
        }
      } else {
        cg.printf("ref(const_cast<Array&>(params).lvalAt(%d))", i);
      }
    } else {
      if (fewArgs) {
        if (i < Option::InvokeFewArgsCount) {
          cg.printf("a%d", i);
        } else {
          cg.printf("null");
        }
      } else {
        cg.printf("params[%d]", i);
      }
    }
  }
  cg.printf(")%s);\n", voidWrapper);

  for (int iMax = m_minParam + 1; iMax <= maxParam; iMax++) {
    if (!ret) cg.printf("else ");
    if (iMax < maxParam || variable) {
      cg.printf("if (count == %d) ", iMax);
    }
    cg.printf("%s", call.c_str());
    for (int i = 0; i < iMax; i++) {
      if (preArgs || i > 0) cg.printf(", ");
      if (isRefParam(i)) {
        if (fewArgs) {
          if (i < Option::InvokeFewArgsCount) {
            cg.printf("ref(a%d)", i);
          } else {
            cg.printf("null");
          }
        } else {
          cg.printf("ref(const_cast<Array&>(params).lvalAt(%d))", i);
        }
      } else {
        if (fewArgs) {
          if (i < Option::InvokeFewArgsCount) {
            cg.printf("a%d", i);
          } else {
            cg.printf("null");
          }
        } else {
          cg.printf("params[%d]", i);
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
        cg.printf("if (count >= %d) params.append(a%d);\n", i + 1, i);
      }
    }
    cg.printf("%s,", call.c_str());
    for (int i = 0; i < maxParam; i++) {
      if (isRefParam(i)) {
        if (fewArgs) {
          if (i < Option::InvokeFewArgsCount) {
            cg.printf("ref(a%d), ", i);
          } else {
            cg.printf("null");
          }
        } else {
          cg.printf("ref(const_cast<Array&>(params).lvalAt(%d)), ", i);
        }
      } else {
        if (fewArgs) {
          if (i < Option::InvokeFewArgsCount) {
            cg.printf("a%d, ", i);
          } else {
            cg.printf("null");
          }
        } else {
          cg.printf("params[%d], ", i);
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
    AnalysisResultPtr ar, const char *funcPrefix, const char *name,
    const char *extraArg /* = NULL */, bool ret /* = true */,
    bool constructor /* = false */) {
  const char *voidWrapper = m_returnType  ? "" : ", null";
  const char *retrn = ret ? "return " : "";
  int maxParam = m_maxParam;
  bool variable = isVariableArgument();
  stringstream callss;
  callss << retrn << ((ret && m_refReturn) ? "ref(" : "(") << funcPrefix <<
    name << "(";
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
  FunctionScope::OutputCPPDynamicInvokeCount(cg);
  outputCPPInvokeArgCountCheck(cg, ar, ret, constructor);
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
    paramEval = "ref((*it)->refval(env, false))";
  }
  if (variable) cg.printf("vargs.append(");
  cg.printf(paramEval);
  if (variable) cg.printf(")");
  cg.printf(";\n");
  cg.indentEnd("}\n");

  if (variable || getOptionalParamCount()) {
    cg.printf("if (count <= %d) ", m_minParam);
  }

  // No optional args
  string call = callss.str();
  cg.printf("%s", call.c_str());
  cg.printf(")%s);\n", voidWrapper);

  // Optional args
  for (int iMax = m_minParam + 1; iMax <= maxParam; iMax++) {
    cg.printf("else ");
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
    .add("static", isStatic() && !isStaticMethodAutoFixed())
    .add("modifier", mod)
    .add("visibility", vis)
    .add("argIsRef", m_refs)
    .done();
}



void FunctionScope::outputCPPCreateDecl(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  ClassScopePtr scope = ar->getClassScope();

  cg.printf("public: %s%s *create(",
            Option::ClassPrefix, scope->getId().c_str());
  outputCPPParamsDecl(cg, ar,
                      dynamic_pointer_cast<MethodStatement>(getStmt())
                      ->getParams(), true);
  cg.printf(");\n");
  cg.printf("public: ObjectData *dynCreate(CArrRef params, bool init = true);"
            "\n");
  if (isDynamic()) {
    cg.printf("public: void dynConstruct(CArrRef params);\n");
    if (cg.getOutput() == CodeGenerator::SystemCPP ||
        Option::EnableEval >= Option::LimitedEval) {
      cg.printf("public: void dynConstructFromEval(Eval::VariableEnvironment "
                "&env, const Eval::FunctionCallExpression *call);\n");
    }
  }
}

void FunctionScope::outputCPPCreateImpl(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  ClassScopePtr scope = ar->getClassScope();
  string clsNameStr = scope->getId();
  const char *clsName = clsNameStr.c_str();
  const char *consName = scope->classNameCtor() ? scope->getName().c_str()
                                                : "__construct";

  cg.printf("%s%s *%s%s::create(",
            Option::ClassPrefix, clsName, Option::ClassPrefix, clsName);
  outputCPPParamsImpl(cg, ar);
  cg.indentBegin(") {\n");
  cg.printf("CountableHelper h(this);\n");
  cg.printf("init();\n");
  cg.printf("%s%s(", Option::MethodPrefix, consName);
  outputCPPParamsCall(cg, ar, false);
  cg.printf(");\n");
  cg.printf("return this;\n");
  cg.indentEnd("}\n");
  cg.indentBegin("ObjectData *%s%s::dynCreate(CArrRef params, "
                 "bool construct /* = true */) {\n",
                 Option::ClassPrefix, clsName);
  cg.printf("init();\n");
  cg.indentBegin("if (construct) {\n");
  cg.printf("CountableHelper h(this);\n");
  OutputCPPDynamicInvokeCount(cg);
  outputCPPDynamicInvoke(cg, ar, Option::MethodPrefix,consName,
                         true, false, false, NULL, true);
  cg.indentEnd("}\n");
  cg.printf("return this;\n");
  cg.indentEnd("}\n");
  if (isDynamic() || isSepExtension()) {
    cg.indentBegin("void %s%s::dynConstruct(CArrRef params) {\n",
                   Option::ClassPrefix, clsName);
    OutputCPPDynamicInvokeCount(cg);
    outputCPPDynamicInvoke(cg, ar, Option::MethodPrefix,
                           consName, true, false, false, NULL, true);
    cg.indentEnd("}\n");
    if (cg.getOutput() == CodeGenerator::SystemCPP ||
        Option::EnableEval >= Option::LimitedEval) {
      cg.indentBegin("void %s%s::dynConstructFromEval("
                     "Eval::VariableEnvironment &env, "
                     "const Eval::FunctionCallExpression *caller) {\n",
                     Option::ClassPrefix, clsName);
      outputCPPEvalInvoke(cg, ar, Option::MethodPrefix,
                          consName, NULL, false);
      cg.indentEnd("}\n");
    }
  }
}

void FunctionScope::outputCPPClassMap(CodeGenerator &cg, AnalysisResultPtr ar) {
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
  if (isStatic() && !isStaticMethodAutoFixed()) {
    attribute |= ClassInfo::IsStatic;
  }
  if (isFinal()) attribute |= ClassInfo::IsFinal;
  if (!m_docComment.empty()) attribute |= ClassInfo::HasDocComment;

  // Use the original cased name, for reflection to work correctly.
  cg.printf("(const char *)0x%04X, \"%s\", NULL, NULL,\n", attribute,
            getOriginalName().c_str());

  if (!m_docComment.empty()) {
    char *dc = string_cplus_escape(m_docComment.c_str());
    cg.printf("\"%s\",\n", dc);
    free(dc);
  }

  Variant defArg;
  for (int i = 0; i < m_maxParam; i++) {
    int attr = ClassInfo::IsNothing;
    if (i >= m_minParam) attr |= ClassInfo::IsOptional;
    if (isRefParam(i)) attr |= ClassInfo::IsReference;

    cg.printf("(const char *)0x%04X, \"%s\", \"%s\", ",
              attr, m_paramNames[i].c_str(),
              Util::toLower(m_paramTypes[i]->getPHPName()).c_str());

    if (i >= m_minParam) {
      MethodStatementPtr m =
        dynamic_pointer_cast<MethodStatement>(getStmt());
      if (m) {
        ExpressionListPtr params = m->getParams();
        assert(i < params->getCount());
        ParameterExpressionPtr param =
          dynamic_pointer_cast<ParameterExpression>((*params)[i]);
        assert(param);
        ExpressionPtr def = param->defaultValue();
        if (!def->isScalar() || !def->getScalarValue(defArg)) {
          defArg = "1";
        }
      } else {
        defArg = "1";
      }
      char *s = string_cplus_escape(f_serialize(defArg).data());
      cg.printf("\"%s\",\n", s);
      free(s);
    } else {
      cg.printf("\"\",\n");
    }
  }
  cg.printf("NULL,\n");

  m_variables->outputCPPStaticVariables(cg, ar);
}

FunctionScope::StringToRefParamInfoPtrMap FunctionScope::s_refParamInfo;

void FunctionScope::RecordRefParamInfo(string fname, FunctionScopePtr func) {
  RefParamInfoPtr info = s_refParamInfo[fname];
  if (!info) {
    info = RefParamInfoPtr(new RefParamInfo());
    s_refParamInfo[fname] = info;
  }
  if (func->isReferenceVariableArgument()) {
    info->setRefVarArg(func->getMaxParamCount());
  }
  for (int i = 0; i < func->getMaxParamCount(); i++) {
    if (func->isRefParam(i)) info->setRefParam(i);
  }
}

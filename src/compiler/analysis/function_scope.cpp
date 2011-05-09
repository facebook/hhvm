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

#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/expression/constant_expression.h>
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
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <util/parser/hphp.tab.hpp>
#include <runtime/base/zend/zend_string.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

FunctionScope::FunctionScope(AnalysisResultConstPtr ar, bool method,
                             const std::string &name, StatementPtr stmt,
                             bool reference, int minParam, int maxParam,
                             ModifierExpressionPtr modifiers,
                             int attribute, const std::string &docComment,
                             FileScopePtr file,
                             bool inPseudoMain /* = false */)
    : BlockScope(name, docComment, stmt, BlockScope::FunctionScope),
      m_minParam(minParam), m_maxParam(maxParam), m_attribute(attribute),
      m_modifiers(modifiers), m_hasVoid(false),
      m_method(method), m_refReturn(reference), m_virtual(false),
      m_hasOverride(false), m_perfectVirtual(false), m_overriding(false),
      m_volatile(false), m_pseudoMain(inPseudoMain),
      m_magicMethod(false), m_system(false), m_inlineable(false), m_sep(false),
      m_containsThis(false), m_containsBareThis(false), m_nrvoFix(true),
      m_inlineAsExpr(false), m_inlineSameContext(false),
      m_contextSensitive(false),
      m_directInvoke(false), m_needsRefTemp(false), m_needsCheckMem(false),
      m_closureGenerator(false),
      m_redeclaring(-1), m_inlineIndex(0), m_optFunction(0) {
  bool canInline = true;
  if (inPseudoMain) {
    canInline = false;
    m_variables->forceVariants(ar, VariableTable::AnyVars);
    setReturnType(ar, Type::Variant);
  }

  if (m_refReturn) {
    m_returnType = Type::Variant;
  }

  if (!strcasecmp(name.c_str(), "__autoload")) {
    setVolatile();
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
    Option::EnableEval == Option::FullEval || Option::AllDynamic;
  m_dynamicInvoke = Option::DynamicInvokeFunctions.find(m_name) !=
    Option::DynamicInvokeFunctions.end();
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
      m_minParam(0), m_maxParam(0), m_attribute(0),
      m_modifiers(ModifierExpressionPtr()), m_hasVoid(false),
      m_method(method), m_refReturn(reference), m_virtual(false),
      m_hasOverride(false), m_perfectVirtual(false), m_overriding(false),
      m_volatile(false), m_pseudoMain(false),
      m_magicMethod(false), m_system(true), m_inlineable(false), m_sep(false),
      m_containsThis(false), m_containsBareThis(false), m_nrvoFix(true),
      m_inlineAsExpr(false), m_inlineSameContext(false),
      m_contextSensitive(false),
      m_directInvoke(false), m_needsRefTemp(false),
      m_closureGenerator(false),
      m_redeclaring(-1), m_inlineIndex(0), m_optFunction(0) {
  m_dynamic = Option::IsDynamicFunction(method, m_name);
  m_dynamicInvoke = Option::DynamicInvokeFunctions.find(m_name) !=
    Option::DynamicInvokeFunctions.end();
}

void FunctionScope::setParamCounts(AnalysisResultConstPtr ar, int minParam,
                                   int maxParam) {
  if (minParam >= 0) {
    m_minParam = minParam;
    m_maxParam = maxParam;
  } else {
    ASSERT(maxParam == minParam);
  }
  ASSERT(m_minParam >= 0 && m_maxParam >= m_minParam);
  if (m_maxParam > 0) {
    m_paramNames.resize(m_maxParam);
    m_paramTypes.resize(m_maxParam);
    m_paramTypeSpecs.resize(m_maxParam);
    m_paramDefaults.resize(m_maxParam);
    m_paramDefaultTexts.resize(m_maxParam);
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

void FunctionScope::setParamSpecs(AnalysisResultPtr ar) {
  if (m_maxParam > 0 && m_stmt) {
    MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    ExpressionListPtr params = stmt->getParams();

    for (int i = 0; i < m_maxParam; i++) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*params)[i]);
      TypePtr specType = param->getTypeSpec(ar, false);
      if (specType &&
          !specType->is(Type::KindOfSome) &&
          !specType->is(Type::KindOfVariant)) {
        m_paramTypeSpecs[i] = specType;
      }
      ExpressionPtr exp = param->defaultValue();
      if (exp) {
        m_paramDefaults[i] = exp->getText(false, false, ar);
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
  bool res = (m_attribute & FileScope::VariableArgument) && !m_overriding;
  return res;
}

bool FunctionScope::isReferenceVariableArgument() const {
  bool res = (m_attribute & FileScope::ReferenceVariableArgument) &&
             !m_overriding;
  // If this method returns true, then isVariableArgument() must also
  // return true.
  ASSERT(!res || isVariableArgument());
  return res;
}

bool FunctionScope::isMixedVariableArgument() const {
  bool res = (m_attribute & FileScope::MixedVariableArgument) && !m_overriding;
  // If this method returns true, then isReferenceVariableArgument()
  // must also return true.
  ASSERT(!res || isReferenceVariableArgument());
  return res;
}

bool FunctionScope::isGenerator() const {
  return name()[0] == '0' && m_paramNames.size() == 1
      && m_paramNames[0] == CONTINUATION_OBJECT_NAME;
}

void FunctionScope::setVariableArgument(int reference) {
  m_attribute |= FileScope::VariableArgument;
  if (reference) {
    m_attribute |= FileScope::ReferenceVariableArgument;
    if (reference < 0) {
      m_attribute |= FileScope::MixedVariableArgument;
    }
  }
}

bool FunctionScope::hasEffect() const {
  return (m_attribute & FileScope::NoEffect) == 0;
}

void FunctionScope::setNoEffect() {
  if (!(m_attribute & FileScope::NoEffect)) {
    m_attribute |= FileScope::NoEffect;
  }
}

bool FunctionScope::isFoldable() const {
  return m_attribute & FileScope::IsFoldable;
}

void FunctionScope::setIsFoldable() {
  m_attribute |= FileScope::IsFoldable;
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

bool FunctionScope::isMagic() const {
  return m_name.size() >= 2 && m_name[0] == '_' && m_name[1] == '_';
}

static std::string s_empty;
const string &FunctionScope::getOriginalName() const {
  if (m_pseudoMain) return s_empty;
  if (m_stmt) {
    MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    return stmt->getOriginalName();
  }
  return m_originalName;
}

string FunctionScope::getFullName() const {
  if (m_stmt) {
    MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    return stmt->getFullName();
  }
  return m_name;
}

string FunctionScope::getOriginalFullName() const {
  if (m_stmt) {
    MethodStatementPtr stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    return stmt->getOriginalFullName();
  }
  return m_name;
}

///////////////////////////////////////////////////////////////////////////////

void FunctionScope::addCaller(BlockScopePtr caller) {
  addUse(caller, UseKindCaller);
}

bool FunctionScope::matchParams(FunctionScopePtr func) {
  // leaving them alone for now
  if (m_overriding || func->m_overriding) return false;
  if (isStatic() || func->isStatic()) return false;

  // conservative here, as we could normalize them into same counts.
  if (m_minParam != func->m_minParam || m_maxParam != func->m_maxParam) {
    return false;
  }
  if (isVariableArgument() != func->isVariableArgument() ||
      isReferenceVariableArgument() != func->isReferenceVariableArgument() ||
      isMixedVariableArgument() != func->isMixedVariableArgument()) {
    return false;
  }

  // needs perfect match for ref, hint and defaults
  for (int i = 0; i < m_maxParam; i++) {
    if (m_refs[i] != func->m_refs[i]) return false;

    TypePtr type1 = m_paramTypeSpecs[i];
    TypePtr type2 = func->m_paramTypeSpecs[i];
    if ((type1 && !type2) || (!type1 && type2) ||
        (type1 && type2 && !Type::SameType(type1, type2))) return false;

    if (m_paramDefaults[i] != func->m_paramDefaults[i]) return false;
  }

  return true;
}

void FunctionScope::setPerfectVirtual() {
  m_virtual = true;
  m_perfectVirtual = true;

  // conservative here, as we could still try to infer types THEN only
  // force variants on non-matching parameters
  m_returnType = Type::Variant;
  for (unsigned int i = 0; i < m_paramTypes.size(); i++) {
    m_paramTypes[i] = Type::Variant;
    m_variables->addLvalParam(m_paramNames[i]);
  }
}

bool FunctionScope::needsTypeCheckWrapper() const {
  for (int i = 0; i < m_maxParam; i++) {
    if (isRefParam(i)) continue;
    if (TypePtr spec = m_paramTypeSpecs[i]) {
      if (Type::SameType(spec, m_paramTypes[i])) {
        return true;
      }
    }
  }
  return false;
}

const char *FunctionScope::getPrefix(ExpressionListPtr params) {
  bool isMethod = getContainingClass();
  bool callInner = false;
  if (Option::HardTypeHints && !Option::SystemGen &&
      !m_system && !m_sep && params) {
    int count = params->getCount();
    if (count >= m_minParam) {
      for (int i = 0; i < count; i++) {
        if (i == m_maxParam) break;
        if (isRefParam(i)) continue;
        if (TypePtr spec = m_paramTypeSpecs[i]) {
          if (Type::SameType(spec, m_paramTypes[i])) {
            ExpressionPtr p = (*params)[i];
            TypePtr at = p->getActualType();
            if (!Type::SameType(spec, at)) {
              callInner = false;
              break;
            }
            callInner = true;
          }
        }
      }
    }
  }
  if (callInner) {
    return isMethod ? Option::TypedMethodPrefix : Option::TypedFunctionPrefix;
  }

  return isMethod ? Option::MethodPrefix : Option::FunctionPrefix;
}

int FunctionScope::inferParamTypes(AnalysisResultPtr ar, ConstructPtr exp,
                                   ExpressionListPtr params, bool &valid) {
  if (!params) {
    if (m_minParam > 0) {
      if (exp->getScope()->isFirstPass()) {
        Compiler::Error(Compiler::TooFewArgument, exp, m_stmt);
      }
      valid = false;
      setDynamic();
    }
    return 0;
  }

  int ret = 0;
  if (params->getCount() < m_minParam) {
    if (exp->getScope()->isFirstPass()) {
      Compiler::Error(Compiler::TooFewArgument, exp, m_stmt);
    }
    valid = false;
    setDynamic();
  }
  if (params->getCount() > m_maxParam) {
    if (isVariableArgument()) {
      ret = params->getCount() - m_maxParam;
    } else {
      if (exp->getScope()->isFirstPass()) {
        Compiler::Error(Compiler::TooManyArgument, exp, m_stmt);
      }
      valid = false;
      setDynamic();
    }
  }

  bool canSetParamType = isUserFunction() && !m_overriding && !m_perfectVirtual;
  for (int i = 0; i < params->getCount(); i++) {
    ExpressionPtr param = (*params)[i];
    if (param->hasContext(Expression::RefParameter)) {
      Symbol *sym = getVariables()->addSymbol(m_paramNames[i]);
      sym->setLvalParam();
      sym->setCallTimeRef();
    }
    if (valid && param->hasContext(Expression::InvokeArgument)) {
      param->clearContext(Expression::InvokeArgument);
      param->clearContext(Expression::RefValue);
      param->clearContext(Expression::NoRefWrapper);
    }
    TypePtr expType;
    if (valid && !canSetParamType && i < m_maxParam &&
        (!Option::HardTypeHints || !m_paramTypeSpecs[i])) {
      expType = param->inferAndCheck(ar, getParamType(i), false);
    } else {
      expType = param->inferAndCheck(ar, Type::Some, false);
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
      if (!Option::HardTypeHints || !m_paramTypeSpecs[i]) {
        TypePtr paramType = getParamType(i);
        if (canSetParamType) {
          paramType = setParamType(ar, i, expType);
        }
        if (valid) {
          if (!Type::IsLegalCast(ar, expType, paramType) &&
              paramType->isNonConvertibleType()) {
            param->inferAndCheck(ar, paramType, true);
          }
          param->setExpectedType(paramType);
        }
      }
    }
    // we do a best-effort check for bad pass-by-reference and do not report
    // error for some vararg case (e.g., array_multisort can have either ref
    // or value for the same vararg).
    if (!isRefVararg || !isMixedVariableArgument()) {
      Expression::CheckPassByReference(ar, param);
    }
  }
  return ret;
}

TypePtr FunctionScope::setParamType(AnalysisResultConstPtr ar, int index,
                                    TypePtr type) {
  ASSERT(index >= 0 && index < (int)m_paramTypes.size());
  TypePtr paramType = m_paramTypes[index];

  if (!paramType) paramType = Type::Some;
  type = Type::Coerce(ar, paramType, type);
  if (type && !Type::SameType(paramType, type)) {
    addUpdates(UseKindNonStaticRef);
    if (!isFirstPass()) {
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
    paramType = Type::Some;
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

bool FunctionScope::hasRefParam(int max) const {
  ASSERT(max >= 0 && max < (int)m_refs.size());
  for (int i = 0; i < max; i++) {
    if (m_refs[i]) return true;
  }
  return false;
}

const std::string &FunctionScope::getParamName(int index) const {
  ASSERT(index >= 0 && index < (int)m_paramNames.size());
  return m_paramNames[index];
}

void FunctionScope::setParamName(int index, const std::string &name) {
  ASSERT(index >= 0 && index < (int)m_paramNames.size());
  m_paramNames[index] = name;
}

void FunctionScope::setParamDefault(int index, const std::string &value,
                                    const std::string &text) {
  ASSERT(index >= 0 && index < (int)m_paramNames.size());
  m_paramDefaults[index] = value;
  m_paramDefaultTexts[index] = text;
}

void FunctionScope::addModifier(int mod) {
  if (!m_modifiers) {
    m_modifiers =
      ModifierExpressionPtr(new ModifierExpression(
                              shared_from_this(), LocationPtr(),
                              Expression::KindOfModifierExpression));
  }
  m_modifiers->add(mod);
}

void FunctionScope::setReturnType(AnalysisResultConstPtr ar, TypePtr type) {
  // no change can be made to virtual function's prototype
  if (m_overriding) return;

  if (!type) {
    m_hasVoid = true;
    if (!m_returnType) return;
  }

  if (m_hasVoid) {
    type = Type::Variant;
  } else if (m_returnType) {
    type = Type::Coerce(ar, m_returnType, type);
  }
  m_returnType = type;
}

void FunctionScope::pushReturnType() {
  m_prevReturn = m_returnType;
  m_hasVoid = false;
  if (m_overriding || m_perfectVirtual || m_pseudoMain) return;
  m_returnType.reset();
}

void FunctionScope::popReturnType() {
  if (m_overriding || m_perfectVirtual || m_pseudoMain) return;

  if (m_returnType) {
    if (m_prevReturn) {
      if (Type::SameType(m_returnType, m_prevReturn)) {
        m_prevReturn.reset();
        return;
      }
      if (!isFirstPass()) {
        Logger::Verbose("Corrected function return type %s -> %s",
                        m_prevReturn->toString().c_str(),
                        m_returnType->toString().c_str());
      }
    }
  } else if (!m_prevReturn) {
    return;
  }

  m_prevReturn.reset();
  addUpdates(UseKindCaller);
}

void FunctionScope::addRetExprToFix(ExpressionPtr e) {
  m_retExprsToFix.push_back(e);
}

void FunctionScope::clearRetExprs() {
  m_retExprsToFix.clear();
}

void FunctionScope::fixRetExprs() {
  for (ExpressionPtrVec::iterator it = m_retExprsToFix.begin(),
         end = m_retExprsToFix.end(); it != end; ++it) {
    (*it)->setExpectedType(m_returnType);
  }
  m_retExprsToFix.clear();
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

std::string FunctionScope::getId(CodeGenerator &cg) const {
  string name = cg.formatLabel(getOriginalName());
  if (m_redeclaring < 0) {
    return name;
  }
  return name + Option::IdPrefix +
    boost::lexical_cast<std::string>(m_redeclaring);
}

///////////////////////////////////////////////////////////////////////////////

void FunctionScope::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (Option::GenerateInferredTypes && m_returnType) {
    cg_printf("// @return %s\n", m_returnType->toString().c_str());
  }

  BlockScope::outputPHP(cg, ar);
}

void FunctionScope::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  ExpressionListPtr params =
    dynamic_pointer_cast<MethodStatement>(getStmt())->getParams();

  int inTypedWrapper = Option::HardTypeHints ?
    cg.getContext() == CodeGenerator::CppTypedParamsWrapperImpl : -1;

  string funcName = cg.escapeLabel(getOriginalName());
  if (ClassScopePtr cls = getContainingClass()) {
    funcName = cg.escapeLabel(cls->getOriginalName()) + "::" + funcName;
  }
  funcName += "()";

  /* Typecheck parameters */
  for (int i = 0; i < m_maxParam; i++) {
    if (inTypedWrapper <= 0 && isRefParam(i)) {
      const string &name = getParamName(i);
      string vname = Option::VariablePrefix + cg.formatLabel(name);
      cg_printf("Variant &%s ATTRIBUTE_UNUSED = r%s;\n",
                vname.c_str(), vname.c_str());
    }
    TypePtr specType = m_paramTypeSpecs[i];
    if (!specType) continue;

    ParameterExpressionPtr param =
      dynamic_pointer_cast<ParameterExpression>((*params)[i]);

    ConstantExpressionPtr constPtr;
    bool isDefaultNull =
      param->defaultValue() &&
      (constPtr = dynamic_pointer_cast<ConstantExpression>(
                    param->defaultValue())) &&
      constPtr->isNull();
    TypePtr infType = m_paramTypes[i];

    if (inTypedWrapper >= 0) {
      bool typed = !isRefParam(i) &&
          (Type::SameType(specType, infType) ||
          (infType && infType->isStandardObject() && isDefaultNull));
      // if the infType is a standard object +
      // we have a default null parameter, then we need to
      // do some special runtime checks
      if (!typed == inTypedWrapper) continue;
    }

    /* Insert runtime checks. */
    if (infType && infType->isStandardObject() && isDefaultNull) {
      cg_indentBegin("if(!%s%s.isNull() && !%s%s.isObject()) {\n",
                     Option::VariablePrefix, param->getName().c_str(),
                     Option::VariablePrefix, param->getName().c_str());
      cg_printf("throw_unexpected_argument_type(%d,\"%s\",\"%s\",%s%s);\n",
                i + 1, funcName.c_str(), specType->getName().c_str(),
                Option::VariablePrefix, param->getName().c_str());
      if (Option::HardTypeHints) {
        cg_printf("return%s;\n", getReturnType() ? " null" : "");
      }
      cg_indentEnd("}\n");
    } else if (specType->is(Type::KindOfObject)) {
      cg_printf("if(");
      if (!m_paramDefaults[i].empty()) {
        cg_printf("!f_is_null(%s%s) && ",
                   Option::VariablePrefix, param->getName().c_str());
      }
      cg_printf("!%s%s.instanceof(", Option::VariablePrefix,
                param->getName().c_str());
      cg_printString(specType->getName(), ar, shared_from_this());
      cg_indentBegin(")) {\n");
      cg_printf("throw_unexpected_argument_type(%d,\"%s\",\"%s\",%s%s);\n",
                i + 1, funcName.c_str(), specType->getName().c_str(),
                Option::VariablePrefix, param->getName().c_str());
      if (Option::HardTypeHints) {
        cg_printf("return%s;\n", getReturnType() ? " null" : "");
      }
      cg_indentEnd("}\n");
    } else {
      cg_printf("if(");
      if (!m_paramDefaults[i].empty() &&
          (isRefParam(i) || isDefaultNull)) {
        cg_printf("!f_is_null(%s%s) && ",
                  Option::VariablePrefix, param->getName().c_str());
      }
      const char *p = 0;
      switch (specType->getKindOf()) {
        case Type::KindOfArray: p = "Array"; break;
        case Type::KindOfBoolean: p = "Boolean"; break;
        case Type::KindOfInt64: p = "Integer"; break;
        case Type::KindOfDouble: p = "Double"; break;
        case Type::KindOfString: p = "String"; break;
        default: assert(false);
      }
      cg_indentBegin("!%s%s.is%s()) {\n",
                     Option::VariablePrefix, param->getName().c_str(), p);
      cg_printf("throw_unexpected_argument_type"
                "(%d,\"%s\",\"%s\",%s%s);\n",
                i + 1, funcName.c_str(), p,
                Option::VariablePrefix, param->getName().c_str());
      if (Option::HardTypeHints) {
        cg_printf("return%s;\n", getReturnType() ? " null" : "");
      }
      cg_indentEnd("}\n");
    }
  }

  if (inTypedWrapper <= 0) {
    BlockScope::outputCPP(cg, ar);

    if (isVariableArgument()) {
      VariableTablePtr variables = getVariables();
      for (int i = 0; i < m_maxParam; i++) {
        const string &name = getParamName(i);
        Symbol *sym = variables->getSymbol(name);
        bool ref = isRefParam(i);
        if (ref ? sym->isReseated() : sym->isLvalParam()) {
          sym->setStashedVal();
          TypePtr paramType = getParamType(i);
          paramType->outputCPPDecl(cg, ar, shared_from_this());
          string vname = Option::VariablePrefix + cg.formatLabel(name);
          cg_printf(ref ? " v%s = ref(%s);\n" : " v%s = %s;\n",
                    vname.c_str(), vname.c_str());
        }
      }
    }

    if (m_closureVars) {
      VariableTablePtr variables = getVariables();
      cg_printf("c_Closure *closure ATTRIBUTE_UNUSED = "
                "(c_Closure*)extra;\n");
      if (m_closureGenerator) {
        cg_printf("extract(variables, closure->t_getvars(), 256);\n");
      } else {
        VariableTablePtr variables = getVariables();
        for (int i = 0; i < m_closureVars->getCount(); i++) {
          ParameterExpressionPtr param =
            dynamic_pointer_cast<ParameterExpression>((*m_closureVars)[i]);
          string name = param->getName();
          if (variables->isPresent(name)) {
            if (param->isRef()) {
              cg_printf("%s%s.assignRef(closure->m_vars.lvalAt(\"%s\"));\n",
                        Option::VariablePrefix, name.c_str(), name.c_str());
            } else {
              cg_printf("%s%s.assignVal(closure->m_vars[\"%s\"]);\n",
                        Option::VariablePrefix, name.c_str(), name.c_str());
            }
          }
        }
      }
    }
  }
}

void FunctionScope::outputCPPParamsDecl(CodeGenerator &cg,
                                        AnalysisResultPtr ar,
                                        ExpressionListPtr params,
                                        bool showDefault) {
  if (isVariableArgument()) {
    if (m_name[0] == '0' && !m_method) {
      cg_printf("void *extra, ");
    }
    cg_printf("int num_args, ");
    if (params) {
      cg.setInExpression(true);
      params->outputCPP(cg, ar);
      cg.setInExpression(false);
      cg_printf(", ");
    }
    if (showDefault) {
      cg_printf("Array args = Array()");
    } else {
      cg_printf("Array args /* = Array() */");
    }
  } else if (m_pseudoMain) {
    if (showDefault) {
      cg_printf("bool incOnce = false, LVariableTable* variables = NULL, "
                "Globals *globals = get_globals()");
    } else {
      cg_printf("bool incOnce /* = false */, "
                "LVariableTable* variables /* = NULL */, "
                "Globals *globals /* = get_globals() */");
    }
  } else if (params) {
    if (m_name[0] == '0' && !m_method) {
      cg_printf("void *extra, ");
    }
    cg.setInExpression(true);
    params->outputCPP(cg, ar);
    cg.setInExpression(false);
  } else if (m_name[0] == '0' && !m_method) {
    cg_printf("void *extra");
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
    cg_printf("int num_args");
    first = false;
  }

  for (int i = 0; i < paramcount; i++) {
    if (first) {
      first = false;
    } else {
      cg_printf(", ");
    }
    TypePtr type = getParamType(i);
    type->outputCPPDecl(cg, ar, BlockScopeRawPtr(this));
    cg_printf(" a%d", i);
  }

  if (isVariableArgument()) {
    cg_printf(", Array args /* = Array() */");
  }
}

bool FunctionScope::outputCPPArrayCreate(CodeGenerator &cg,
                                         AnalysisResultPtr ar,
                                         int m_maxParam) {
  ASSERT(m_maxParam >= 0);
  ASSERT(m_attribute & FileScope::VariableArgument);
  if (!Option::GenArrayCreate ||
      cg.getOutput() == CodeGenerator::SystemCPP ||
      m_maxParam == 0) {
    return false;
  }
  if (isUserFunction()) {
    MethodStatementPtr stmt;
    stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    if (stmt->hasRefParam()) return false;
    cg_printf("Array(");
    stmt->outputParamArrayCreate(cg, false);
  } else if (hasRefParam(m_maxParam)) {
    ASSERT(false);
    return false;
  } else {
    ASSERT(false);
    cg_printf("Array(");
    for (int i = 0; i < m_maxParam; i++) {
      cg_printf("%d, a%d", i, i);
      if (i < m_maxParam - 1) {
        cg_printf(", ");
      } else {
        cg_printf(")");
      }
    }
  }
  cg_printf(")");
  cg_printf(isVariableArgument() ? ",args" : ",Array()");
  return true;
}

void FunctionScope::outputCPPParamsCall(CodeGenerator &cg,
                                        AnalysisResultPtr ar,
                                        bool aggregateParams) {
  if (isVariableArgument()) {
    cg_printf("num_args, ");
  } else if (aggregateParams) {
    cg_printf("%d, ", m_maxParam);
  }
  if (aggregateParams && outputCPPArrayCreate(cg, ar, m_maxParam)) {
    return;
  }
  bool userFunc = isUserFunction();
  CodeGenerator::Context context = cg.getContext();
  bool isWrapper = !aggregateParams &&
    (context == CodeGenerator::CppTypedParamsWrapperImpl ||
     context == CodeGenerator::CppTypedParamsWrapperDecl ||
     context == CodeGenerator::CppFunctionWrapperImpl ||
     context == CodeGenerator::CppFunctionWrapperDecl);

  MethodStatementPtr stmt;
  ExpressionListPtr params;
  if (userFunc) {
    stmt = dynamic_pointer_cast<MethodStatement>(m_stmt);
    params = stmt->getParams();
  }

  VariableTablePtr variables = getVariables();
  if (aggregateParams) {
    cg_printf("Array(");
    if (m_maxParam) {
      // param arrays are always vectors
      cg_printf("ArrayInit(%d, true).", m_maxParam);
    }
  }
  for (int i = 0; i < m_maxParam; i++) {
    if (i > 0) cg_printf(aggregateParams ? "." : ", ");
    bool isRef;
    if (userFunc) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*params)[i]);
      isRef = !isWrapper && param->isRef();
      if (aggregateParams) {
        cg_printf("set(%s%s%s",
                  variables->getSymbol(param->getName())->isStashedVal() ?
                  "v" : "",
                  Option::VariablePrefix, param->getName().c_str());
      } else {
        cg_printf("%s%s%s%s",
                  isRef ? "ref(" : "",
                  Option::VariablePrefix, param->getName().c_str(),
                  isRef ? ")" : "");
      }
    } else {
      isRef = !isWrapper && isRefParam(i);
      if (aggregateParams) {
        ASSERT(false);
        cg_printf("set(a%d", i);
      } else {
        cg_printf("%sa%d%s",
                  isRef ? "ref(" : "", i, isRef ? ")" : "");
      }
    }
    if (aggregateParams) cg_printf(")");
  }
  if (aggregateParams) {
    if (m_maxParam) cg_printf(".create()");
    cg_printf(")");
  }
  if (isVariableArgument()) {
    if (aggregateParams || m_maxParam > 0) cg_printf(",");
    cg_printf("args");
  } else if (aggregateParams) {
    cg_printf(", Array()");
  }
}

void FunctionScope::OutputCPPArguments(ExpressionListPtr params,
                                       FunctionScopePtr func,
                                       CodeGenerator &cg,
                                       AnalysisResultPtr ar, int extraArg,
                                       bool variableArgument,
                                       int extraArgArrayId /* = -1 */,
                                       int extraArgArrayHash /* = -1 */,
                                       int extraArgArrayIndex /* = -1 */) {
  int paramCount = params ? params->getOutputCount() : 0;
  ASSERT(extraArg <= paramCount);
  int iMax = paramCount - extraArg;
  bool extra = false;

  if (variableArgument) {
    if (paramCount == 0) {
      cg_printf("0");
    } else {
      cg_printf("%d, ", paramCount);
    }
  }
  for (int i = 0; i < paramCount; i++) {
    ExpressionPtr param = (*params)[i];
    if (i > 0) cg_printf(extra ? "." : ", ");
    if (!extra && (i == iMax || extraArg < 0)) {
      if (extraArgArrayId != -1) {
        assert(extraArgArrayHash != -1 && extraArgArrayIndex != -1);
        ar->outputCPPScalarArrayId(cg, extraArgArrayId, extraArgArrayHash,
                                   extraArgArrayIndex);
        break;
      }
      extra = true;
      // Parameter arrays are always vectors.
      if (Option::GenArrayCreate &&
          cg.getOutput() != CodeGenerator::SystemCPP) {
        if (!params->hasNonArrayCreateValue(false, i)) {
          if (ar->m_arrayIntegerKeyMaxSize < paramCount - i) {
            ar->m_arrayIntegerKeyMaxSize  = paramCount - i;
          }
          cg_printf("Array(");
          params->outputCPPUniqLitKeyArrayInit(cg, ar, false,
                                               paramCount - i, false, i);
          cg_printf(")");
          return;
        }
      }
      cg_printf("Array(ArrayInit(%d, true).", paramCount - i);
    }
    if (extra) {
      bool needRef = param->hasContext(Expression::RefValue) &&
        !param->hasContext(Expression::NoRefWrapper) &&
        param->isRefable();
      cg_printf("set%s(", needRef ? "Ref" : "");
      if (needRef) {
        // The parameter itself shouldn't be wrapped with ref() any more.
        param->setContext(Expression::NoRefWrapper);
      }
      param->outputCPP(cg, ar);
      cg_printf(")");
    } else {
      // If the implemented type is ref-counted and expected type is variant,
      // use VarNR to avoid unnecessary ref-counting because we know
      // the actual argument will always has a ref in the callee.
      bool wrap = false;
      if (!param->hasContext(Expression::RefValue) &&
          param->getExpectedType() &&
          param->getExpectedType()->is(Type::KindOfVariant) &&
          (param->getCPPType()->is(Type::KindOfArray) ||
           param->getCPPType()->is(Type::KindOfString) ||
           param->getCPPType()->is(Type::KindOfObject) ||
           param->getCPPType()->isPrimitive())) {
        wrap = true;
        if (func && i < func->getMaxParamCount()) {
          VariableTablePtr variables = func->getVariables();
          if (variables->isLvalParam(func->getParamName(i))) {
            // Callee expects a Variant instead of CVarRef
            wrap = false;
          }
        }
      }
      if (wrap) {
        cg_printf("VarNR(");
      }
      param->outputCPP(cg, ar);
      if (wrap) {
        cg_printf(")");
      }
    }
  }
  if (extra) {
    cg_printf(".create())");
  }
}

void FunctionScope::OutputCPPEffectiveArguments(ExpressionListPtr params,
                                                CodeGenerator &cg,
                                                AnalysisResultPtr ar) {
  int paramCount = params ? params->getCount() : 0;
  for (int i = 0; i < paramCount; i++) {
    ExpressionPtr param = (*params)[i];
    if (param->hasEffect()) {
      param->outputCPP(cg, ar);
      cg_printf(", ");
    }
  }
}

void FunctionScope::OutputCPPDynamicInvokeCount(CodeGenerator &cg) {
  cg_printf("int count ATTRIBUTE_UNUSED = params.size();\n");
}

int FunctionScope::outputCPPInvokeArgCountCheck(
  CodeGenerator &cg, AnalysisResultPtr ar,
  bool ret, bool constructor, int maxCount) {
  bool variable = isVariableArgument();
  // system function has different handling of argument counts
  bool system = (m_system || m_sep ||
                 cg.getOutput() == CodeGenerator::SystemCPP);
  if (!system) {
    ClassScopePtr scope = getContainingClass();
    if (scope) system = (!scope->isUserClass() || scope->isExtensionClass() ||
                         scope->isSepExtension());
  }

  bool checkMissing = (m_minParam > 0);
  bool checkTooMany = (!variable && (system ||
                                     RuntimeOption::ThrowTooManyArguments));
  const char *sysret = (system && ret) ? "return " : "";
  const char *level = (system ? (constructor ? ", 2" : ", 1") : "");
  int guarded = system && (ret || constructor) ? m_minParam : 0;
  string fullname = getOriginalFullName();
  if (checkMissing) {
    bool fullGuard = ret && (system || Option::HardTypeHints);
    for (int i = 0; i < m_minParam; i++) {
      if (TypePtr t = m_paramTypeSpecs[i]) {
        if (m_paramDefaults[i].empty()) {
          if (i < maxCount) cg_printf("if (UNLIKELY(count < %d)) ", i + 1);
          cg_printf("%sthrow_missing_typed_argument(\"%s\", ",
                    fullGuard ? "return " : "", fullname.c_str());
          cg_printf(t->is(Type::KindOfArray) ?
                    "0" : "\"%s\"", cg.escapeLabel(t->getName()).c_str());
          cg_printf(", %d);\n", i + 1);
          if (fullGuard) {
            if (i >= maxCount) return m_minParam;
            if (guarded <= i) guarded = i + 1;
            if (i + 1 == m_minParam) {
              checkMissing = false;
            }
          }
        }
      }
    }
  }

  if (checkMissing && checkTooMany) {
    if (!variable && m_minParam == m_maxParam) {
      if (maxCount >= m_minParam) {
        cg_printf("if (UNLIKELY(count != %d))", m_minParam);
      }
      cg_printf(" %sthrow_wrong_arguments(\"%s\", count, %d, %d%s);\n",
                sysret, fullname.c_str(), m_minParam, m_maxParam, level);
    } else {
      if (maxCount >= m_minParam) {
        if (maxCount <= m_maxParam) {
          cg_printf("if (UNLIKELY(count < %d))", m_minParam, m_maxParam);
        } else {
          cg_printf("if (UNLIKELY(count < %d || count > %d))",
                    m_minParam, m_maxParam);
        }
      }
      cg_printf(" %sthrow_wrong_arguments(\"%s\", count, %d, %d%s);\n",
                sysret, fullname.c_str(),
                m_minParam, variable ? -1 : m_maxParam, level);
    }
  } else if (checkMissing) {
    if (maxCount >= m_minParam) {
      cg_printf("if (UNLIKELY(count < %d))", m_minParam);
    }
    cg_printf(" %sthrow_missing_arguments(\"%s\", count+1%s);\n",
              sysret, fullname.c_str(), level);
  } else if (checkTooMany && maxCount > m_maxParam) {
    cg_printf("if (UNLIKELY(count > %d))"
              " %sthrow_toomany_arguments(\"%s\", %d%s);\n",
              m_maxParam, sysret, fullname.c_str(), m_maxParam, level);
  }
  return guarded;
}

void FunctionScope::outputCPPDynamicInvoke(CodeGenerator &cg,
                                           AnalysisResultPtr ar,
                                           const char *funcPrefix,
                                           const char *name,
                                           bool voidWrapperOff /* = false */,
                                           bool fewArgs /* = false */,
                                           bool ret /* = true */,
                                           const char *extraArg /* = NULL */,
                                           bool constructor /* = false */,
                                           const char *instance /* = NULL */,
                                           const char *class_name /* = "" */) {
  const char *voidWrapper = (m_returnType || voidWrapperOff) ? "" : ", null";
  const char *retrn = ret ? "return " : "";
  bool variable = isVariableArgument();
  bool refVariable = isReferenceVariableArgument();
  int maxCount = fewArgs ? Option::InvokeFewArgsCount : INT_MAX;
  bool useDefaults = !m_stmt || ar->isSystem();

  ASSERT(m_minParam >= 0);

  int guarded = outputCPPInvokeArgCountCheck(cg, ar, ret,
                                             constructor, maxCount);
  if (guarded > maxCount) return;

  bool do_while = !ret && !fewArgs && m_maxParam > m_minParam && useDefaults;
  cg_printf("%s", class_name);
  if (!fewArgs && m_maxParam) {
    for (int i = 0; i < m_maxParam; i++) {
      if (isRefParam(i)) {
        cg_printf("const_cast<Array&>(params).escalate(true);\n");
        break;
      }
    }
    if (do_while) cg_printf("do ");
    cg_indentBegin("{\n");
    cg_printf("ArrayData *ad(params.get());\n");
    cg_printf("ssize_t pos = ad ? ad->iter_begin() : "
              "ArrayData::invalid_index;\n");
  }

  CodeGenerator::Context context = cg.getContext();
  std::vector<Variant> defs;

  int i = !m_minParam ? -1 : 0;
  while (true) {
    if (i >= 0 && i < m_maxParam) {
      bool ref = isRefParam(i);
      int defIndex = -1;
      bool wantNullVariantNotNull = fewArgs ? i < maxCount : ref;
      bool dftNull = false;
      if (!m_paramDefaults[i].empty() && !useDefaults) {
        Variant tmp;
        MethodStatementPtr m(dynamic_pointer_cast<MethodStatement>(m_stmt));
        ExpressionListPtr params = m->getParams();
        assert(params && params->getCount() > i);
        ParameterExpressionPtr p(
          dynamic_pointer_cast<ParameterExpression>((*params)[i]));
        dftNull = p->defaultValue()->isScalar() &&
          p->defaultValue()->getScalarValue(tmp) && tmp.isNull();
        if (wantNullVariantNotNull && !dftNull && !ref) {
          cg_printf("TypedValue def%d;\n", i);
          defIndex = i;
        }
      }
      cg_printf("%s arg%d(", ref ? "VRefParam" : "CVarRef", i);
      if (m_paramDefaults[i].empty()) {
        if (i >= guarded) {
          if (i < maxCount) cg_printf("UNLIKELY(count <= %d) ? ", i);
          cg_printf(ref ? "(VRefParamValue())" :
              (wantNullVariantNotNull ? "null_variant" : "null"));
          if (i < maxCount) cg_printf(" : ");
        }
      } else if (!useDefaults) {
        bool close = false;
        if (i < maxCount) {
          cg_printf("count <= %d ? ", i);
        } else if (defIndex < 0) {
          cg_printf("(");
          close = true;
        }
        if (dftNull) {
          cg_printf(ref ? "(VRefParamValue())" :
              (wantNullVariantNotNull ? "null_variant" : "null"));
        } else {
          MethodStatementPtr m(dynamic_pointer_cast<MethodStatement>(m_stmt));
          ExpressionListPtr params = m->getParams();
          assert(params && params->getCount() > i);
          ParameterExpressionPtr p(
            dynamic_pointer_cast<ParameterExpression>((*params)[i]));

          if (defIndex >= 0) {
            cg_printf("*new (&def%d) Variant(", defIndex);
          } else {
            TypePtr type = p->defaultValue()->getCPPType();
            bool isVariant = type && type->is(Type::KindOfVariant);
            cg_printf("%s(", ref ? "VRefParamValue" :
                      (!fewArgs || i >= maxCount) && !isVariant ?
                      "Variant" : "");
          }
          cg.setContext(CodeGenerator::CppParameterDefaultValueDecl);
          p->defaultValue()->outputCPP(cg, ar);
          cg.setContext(context);
          cg_printf(")");
        }
        if (close) cg_printf(")");
        if (i < maxCount) cg_printf(" : ");
      }
      if (fewArgs) {
        if (i < maxCount) cg_printf(ref ? "vref(a%d)" : "a%d", i);
      } else {
        cg_printf("%s(ad->getValue%s(pos%s))",
                  ref ? "vref" : "",
                  ref ? "Ref" : "",
                  i ? " = ad->iter_advance(pos)" : "");
      }
      cg_printf(");\n");
    }
    if (++i < m_minParam) continue;
    const char *extra = "";
    bool last = i >= maxCount || i == m_maxParam;
    if (!useDefaults && !last) continue;

    if (variable && last) {
      if (fewArgs) {
        if (i < Option::InvokeFewArgsCount) {
          cg_printf("Array p;\n");
          for (int j = i; j < Option::InvokeFewArgsCount; j++) {
            if (refVariable) {
              cg_printf("if (count >= %d) p.append(ref(a%d));\n", j + 1, j);
            } else {
              cg_printf("if (count >= %d) p.append(a%d);\n", j + 1, j);
            }
          }
          extra = ", p";
        }
      } else {
        cg_printf("const Array &p(count > %d ? "
                  "params.slice(%d, count - %d, false) : Array());\n",
                  m_maxParam, m_maxParam, m_maxParam);
        extra = ", p";
      }
    }

    if (!last) {
      cg_printf("if (count <= %d) ", i);
      if (do_while) {
        cg_indentBegin("{\n");
      }
    }
    cg_printf("%s%s%s", retrn, (m_refReturn ? "ref(" : "("),
              instance ? instance : "");
    if (m_perfectVirtual) {
      ClassScopePtr cls = getContainingClass();
      cg_printf("%s%s::", Option::ClassPrefix, cls->getId(cg).c_str());
    }
    cg_printf("%s%s(", funcPrefix, name);

    bool preArgs = false;
    if (name[0] == '0' && !m_method) {
      cg_printf("extra"); // closure
      preArgs = true;
    }
    if (extraArg) {
      if (preArgs) cg_printf(", ");
      cg_printf("%s", extraArg);
      preArgs = true;
    }
    if (variable) {
      if (preArgs) cg_printf(", ");
      cg_printf("count");
      preArgs = true;
    }
    for (int j = 0; j < i; j++) {
      if (j >= m_minParam && j >= maxCount) break;
      if (preArgs || j > 0) cg_printf(", ");
      cg_printf("arg%d", j);
    }
    cg_printf("%s)%s);\n", extra, voidWrapper);
    if (last) break;
    if (do_while) {
      cg_printf("break;\n");
      cg_indentEnd("}\n");
    }
  }

  if (!fewArgs && m_maxParam) {
    if (do_while) {
      cg_indentEnd("} while (false);\n");
    } else {
      cg_indentEnd("}\n");
    }
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
  ClassScopePtr scope = getContainingClass();
  CodeGenerator::Context context = cg.getContext();
  bool setWrapper = Option::HardTypeHints && needsTypeCheckWrapper();

  cg_printf("public: %s%s *create(",
            Option::ClassPrefix, scope->getId(cg).c_str());
  cg.setContext(setWrapper ?
                CodeGenerator::CppTypedParamsWrapperDecl :
                CodeGenerator::CppFunctionWrapperDecl);
  outputCPPParamsDecl(cg, ar,
                      dynamic_pointer_cast<MethodStatement>(getStmt())
                      ->getParams(), true);
  cg.setContext(context);
  cg_printf(");\n");
  cg_printf("public: void dynConstruct(CArrRef params);\n");
  if (isDynamic()) {
    cg_printf("public: void getConstructor(MethodCallPackage &mcp);\n");
  }
}

void FunctionScope::outputCPPCreateImpl(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  ClassScopePtr scope = getContainingClass();
  string clsNameStr = scope->getId(cg);
  const char *clsName = clsNameStr.c_str();
  const string &funcNameStr = this->getName();
  const char *consName = funcNameStr.c_str();
  CodeGenerator::Context context = cg.getContext();
  bool setWrapper = Option::HardTypeHints && needsTypeCheckWrapper();

  cg_printf("%s%s *%s%s::create(",
            Option::ClassPrefix, clsName, Option::ClassPrefix, clsName);

  cg.setContext(setWrapper ?
                CodeGenerator::CppTypedParamsWrapperImpl :
                CodeGenerator::CppFunctionWrapperImpl);
  outputCPPParamsImpl(cg, ar);
  cg_indentBegin(") {\n");
  cg_printf("CountableHelper h(this);\n");
  cg_printf("init();\n");
  cg_printf("%s%s(", Option::MethodPrefix, consName);
  outputCPPParamsCall(cg, ar, false);
  cg.setContext(context);
  cg_printf(");\n");
  cg_printf("return this;\n");
  cg_indentEnd("}\n");
  cg_indentBegin("void %s%s::dynConstruct(CArrRef params) {\n",
                 Option::ClassPrefix, clsName);
  OutputCPPDynamicInvokeCount(cg);
  outputCPPDynamicInvoke(cg, ar, Option::MethodPrefix,
                         cg.formatLabel(consName).c_str(),
                         true, false, false, NULL, true);
  cg_indentEnd("}\n");
  if (isDynamic() || isSepExtension()) {
    cg_indentBegin("void %s%s::getConstructor(MethodCallPackage &mcp) {\n",
                   Option::ClassPrefix, clsName);
    cg_printf("mcp.ci = &%s%s::%s%s;\n", Option::ClassPrefix, clsName,
              Option::CallInfoPrefix, cg.formatLabel(consName).c_str());
    cg_printf("mcp.obj = this;\n");
    cg_indentEnd("}\n");
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
  if (isStatic()) {
    attribute |= ClassInfo::IsStatic;
  }
  if (isFinal()) attribute |= ClassInfo::IsFinal;

  if (isVariableArgument()) {
    attribute |= ClassInfo::VariableArguments;
  }
  if (isReferenceVariableArgument()) {
    attribute |= ClassInfo::RefVariableArguments;
  }
  if (isMixedVariableArgument()) {
    attribute |= ClassInfo::MixedVariableArguments;
  }

  attribute |= m_attributeClassInfo;
  if (!m_docComment.empty() && Option::GenerateDocComments) {
    attribute |= ClassInfo::HasDocComment;
  } else {
    attribute &= ~ClassInfo::HasDocComment;
  }

  // Use the original cased name, for reflection to work correctly.
  cg_printf("(const char *)0x%04X, \"%s\", \"%s\", (const char *)%d, "
            "(const char *)%d, NULL, NULL,\n", attribute,
            cg.escapeLabel(getOriginalName()).c_str(),
            m_stmt ? m_stmt->getLocation()->file : "",
            m_stmt ? m_stmt->getLocation()->line0 : 0,
            m_stmt ? m_stmt->getLocation()->line1 : 0);

  if (!m_docComment.empty() && Option::GenerateDocComments) {
    char *dc = string_cplus_escape(m_docComment.c_str(), m_docComment.size());
    cg_printf("\"%s\",\n", dc);
    free(dc);
  }

  Variant defArg;
  for (int i = 0; i < m_maxParam; i++) {
    int attr = ClassInfo::IsNothing;
    if (i >= m_minParam) attr |= ClassInfo::IsOptional;
    if (isRefParam(i)) attr |= ClassInfo::IsReference;

    const std::string &tname = m_paramTypeSpecs[i] ?
      m_paramTypeSpecs[i]->getPHPName() : "";
    cg_printf("(const char *)0x%04X, \"%s\", \"%s\", ",
              attr, m_paramNames[i].c_str(),
              Util::toLower(tname).c_str());

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
        string sdef = def->getText();
        char *esdef = string_cplus_escape(sdef.data(), sdef.size());
        if (!def->isScalar() || !def->getScalarValue(defArg)) {
          /**
           * Special value runtime/ext/ext_reflection.cpp can check and throw.
           * If we want to avoid seeing this so to make getDefaultValue()
           * work better for reflections, we will have to implement
           * getScalarValue() to greater extent under compiler/expressions.
           */
          cg_printf("\"\x01\", \"%s\",\n", esdef);
        } else {
          String str = f_serialize(defArg);
          char *s = string_cplus_escape(str.data(), str.size());
          cg_printf("\"%s\", \"%s\",\n", s, esdef);
          free(s);
        }
        free(esdef);
      } else {
        char *def = string_cplus_escape(m_paramDefaults[i].data(),
                                        m_paramDefaults[i].size());
        char *defText = string_cplus_escape(m_paramDefaultTexts[i].data(),
                                            m_paramDefaultTexts[i].size());
        cg_printf("\"%s\", \"%s\",\n", def, defText);
        free(def);
        free(defText);
      }
    } else {
      cg_printf("\"\", \"\",\n");
    }
  }
  cg_printf("NULL,\n");

  m_variables->outputCPPStaticVariables(cg, ar);
}

void FunctionScope::outputCPPCallInfo(CodeGenerator &cg,
    AnalysisResultPtr ar) {
  if (isAbstract()) return;
  string id;
  if (m_method) {
    id = cg.formatLabel(m_name).c_str();
  } else {
    id = getId(cg);
  }
  int64 refflags = 0;
  for (int i = 0; i < m_maxParam; ++i) {
    if (isRefParam(i)) {
      refflags |= 1 << i;
    }
  }
  int flags = 0;
  if (isMixedVariableArgument()) {
    flags |= CallInfo::MixedVarArgs;
  } else if (isReferenceVariableArgument()) {
    flags |= CallInfo::RefVarArgs;
  } else if (isVariableArgument()) {
    flags |= CallInfo::VarArgs;
  }
  if (m_method) {
    flags |= CallInfo::Method;
    if (isStatic()) {
      flags |= CallInfo::StaticMethod;
    }
    ClassScopePtr scope = getContainingClass();
    string clsName = scope->getId(cg);
    cg.printf("CallInfo %s%s::%s%s((void*)&%s%s::%s%s, ", Option::ClassPrefix,
              clsName.c_str(), Option::CallInfoPrefix, id.c_str(),
              Option::ClassPrefix, clsName.c_str(), Option::InvokePrefix,
              id.c_str());
    cg.printf("(void*)&%s%s::%s%s", Option::ClassPrefix, clsName.c_str(),
              Option::InvokeFewArgsPrefix, id.c_str());
  } else {
    cg.printf("CallInfo %s%s((void*)&%s%s, ", Option::CallInfoPrefix,
              id.c_str(), Option::InvokePrefix, id.c_str());
    cg.printf("(void*)&%s%s", Option::InvokeFewArgsPrefix, id.c_str());
  }
  cg.printf(", %d, %d, 0x%.16lXLL);\n", m_maxParam, flags, refflags);
}

FunctionScope::StringToFunctionInfoPtrMap FunctionScope::s_refParamInfo;
static Mutex s_refParamInfoLock;

void FunctionScope::RecordFunctionInfo(string fname, FunctionScopePtr func) {
  Lock lock(s_refParamInfoLock);
  FunctionInfoPtr info = s_refParamInfo[fname];
  if (!info) {
    info = FunctionInfoPtr(new FunctionInfo());
    s_refParamInfo[fname] = info;
  }
  if (func->isStatic()) {
    info->setMaybeStatic();
  }
  if (func->isReferenceVariableArgument()) {
    info->setRefVarArg(func->getMaxParamCount());
  }
  VariableTablePtr variables = func->getVariables();
  for (int i = 0; i < func->getMaxParamCount(); i++) {
    if (func->isRefParam(i)) info->setRefParam(i);
    variables->addParam(func->getParamName(i),
                        TypePtr(), AnalysisResultPtr(), ConstructPtr());
  }
}

FunctionScope::FunctionInfoPtr FunctionScope::GetFunctionInfo(string fname) {
  StringToFunctionInfoPtrMap::iterator it = s_refParamInfo.find(fname);
  if (it == s_refParamInfo.end()) {
    return FunctionInfoPtr();
  }
  return it->second;
}

void FunctionScope::outputMethodWrapper(CodeGenerator &cg,
                                        AnalysisResultPtr ar,
                                        const char *clsToConstruct) {
  cg_printf("\n");
  if (m_stmt) {
    m_stmt->printSource(cg);
  }
  cg.printDocComment(m_docComment);

  int minCount = getMinParamCount();
  int maxCount = getMaxParamCount();

  for (int count = minCount; count <= maxCount; count++) {
    TypePtr type = getReturnType();

    if (clsToConstruct) {
      cg_printf("static %s%s Create(", Option::SmartPtrPrefix, clsToConstruct);
    } else {
      if (isStatic()) cg_printf("static ");
      if (type) {
        type->outputCPPDecl(cg, ar, BlockScopeRawPtr(this));
      } else {
        cg_printf("void");
      }
      cg_printf(" %s%s(", Option::MethodWrapperPrefix, getId(cg).c_str());
    }

    for (int i = 0; i < count; i++) {
      if (i > 0) cg_printf(", ");
      getParamType(i)->outputCPPDecl(cg, ar, BlockScopeRawPtr(this));
      cg_printf(" %s%s", Option::VariablePrefix, getParamName(i).c_str());
    }
    if (isVariableArgument()) {
      if (count) cg_printf(", ");
      cg_printf("Array args = Array()");
    }
    cg_indentBegin(") {\n");

    if (clsToConstruct) {
      cg_printf("%s%s ret(NEWOBJ(%s%s)());\n",
                Option::SmartPtrPrefix, clsToConstruct,
                Option::ClassPrefix, clsToConstruct);
    }

    if ((isStatic() || isPerfectVirtual() || !isVirtual() || clsToConstruct) &&
        !isRedeclaring()) {

      if (clsToConstruct) {
        cg_printf("ret->");
      } else if (type) {
        cg_printf("return ");
      }

      cg_printf("%s%s(", Option::MethodPrefix, cg.formatLabel(m_name).c_str());
      if (isVariableArgument()) {
        cg_printf("args.size() + %d, ", count);
      }
      for (int i = 0; i < count; i++) {
        if (i > 0) cg_printf(", ");
        bool isRef = isRefParam(i);
        if (isRef) cg_printf("ref(");
        cg_printf("%s%s", Option::VariablePrefix, getParamName(i).c_str());
        if (isRef) cg_printf(")");
      }
      if (isVariableArgument()) {
        if (count) cg_printf(", ");
        cg_printf("args");
      }
      cg_printf(");\n");

    } else {
      cg_printf("Array params;\n");
      for (int i = 0; i < count; i++) {
        cg_printf("params.append(");

        bool isRef = isRefParam(i);
        if (isRef) cg_printf("ref(");
        cg_printf("%s%s", Option::VariablePrefix, getParamName(i).c_str());
        if (isRef) cg_printf(")");

        cg_printf(");\n");
      }
      if (isVariableArgument()) {
        cg_printf("params.merge(args);\n");
      }

      if (clsToConstruct) {
        cg_printf("ret->");
      } else if (type) {
        cg_printf("return ");
      }

      if (isStatic()) {
        cg_printf("%sinvoke(NULL, \"%s\", params, -1);\n",
                  Option::ObjectStaticPrefix, m_name.c_str());
      } else {
        cg_printf("%sinvoke(\"%s\", params, -1);\n",
                  Option::ObjectPrefix, m_name.c_str());
      }
    }

    if (clsToConstruct) {
      cg_printf("return ret;\n");
    }
    cg_indentEnd("}\n");
  }
}

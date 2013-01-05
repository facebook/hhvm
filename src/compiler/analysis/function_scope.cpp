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
#include <compiler/expression/function_call.h>
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
#include <util/atomic.h>
#include <util/util.h>
#include <runtime/base/class_info.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <util/parser/hphp.tab.hpp>
#include <runtime/base/zend/zend_string.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

FunctionScope::FunctionScope(AnalysisResultConstPtr ar, bool method,
                             const std::string &name, StatementPtr stmt,
                             bool reference, int minParam, int maxParam,
                             ModifierExpressionPtr modifiers,
                             int attribute, const std::string &docComment,
                             FileScopePtr file,
                             const std::vector<UserAttributePtr> &attrs,
                             bool inPseudoMain /* = false */)
    : BlockScope(name, docComment, stmt, BlockScope::FunctionScope),
      m_minParam(minParam), m_maxParam(maxParam), m_attribute(attribute),
      m_modifiers(modifiers), m_hasVoid(false),
      m_method(method), m_refReturn(reference), m_virtual(false),
      m_hasOverride(false), m_perfectVirtual(false), m_overriding(false),
      m_volatile(false), m_persistent(false), m_pseudoMain(inPseudoMain),
      m_magicMethod(false), m_system(false), m_inlineable(false), m_sep(false),
      m_containsThis(false), m_containsBareThis(0), m_nrvoFix(true),
      m_inlineAsExpr(false), m_inlineSameContext(false),
      m_contextSensitive(false),
      m_directInvoke(false), m_needsRefTemp(false),
      m_needsObjTemp(false), m_needsCheckMem(false),
      m_closureGenerator(false), m_noLSB(false), m_nextLSB(false),
      m_hasTry(false), m_hasGoto(false), m_localRedeclaring(false),
      m_redeclaring(-1), m_inlineIndex(0), m_optFunction(0), m_nextID(0) {
  init(ar);
  for (unsigned i = 0; i < attrs.size(); ++i) {
    if (m_userAttributes.find(attrs[i]->getName()) != m_userAttributes.end()) {
      attrs[i]->parseTimeFatal(Compiler::DeclaredAttributeTwice,
                               "Redeclared attribute %s",
                               attrs[i]->getName().c_str());
    }
    m_userAttributes[attrs[i]->getName()] = attrs[i]->getExp();
  }
}

FunctionScope::FunctionScope(FunctionScopePtr orig,
                             AnalysisResultConstPtr ar,
                             const string &name,
                             const string &originalName,
                             StatementPtr stmt,
                             ModifierExpressionPtr modifiers)
    : BlockScope(name, orig->m_docComment, stmt,
                 BlockScope::FunctionScope),
      m_minParam(orig->m_minParam), m_maxParam(orig->m_maxParam),
      m_attribute(orig->m_attribute), m_modifiers(modifiers),
      m_userAttributes(orig->m_userAttributes), m_hasVoid(orig->m_hasVoid),
      m_method(orig->m_method), m_refReturn(orig->m_refReturn),
      m_virtual(orig->m_virtual), m_hasOverride(orig->m_hasOverride),
      m_perfectVirtual(orig->m_perfectVirtual),
      m_overriding(orig->m_overriding), m_volatile(orig->m_volatile),
      m_persistent(orig->m_persistent),
      m_pseudoMain(orig->m_pseudoMain), m_magicMethod(orig->m_magicMethod),
      m_system(orig->m_system), m_inlineable(orig->m_inlineable),
      m_sep(orig->m_sep), m_containsThis(orig->m_containsThis),
      m_containsBareThis(orig->m_containsBareThis), m_nrvoFix(orig->m_nrvoFix),
      m_inlineAsExpr(orig->m_inlineAsExpr),
      m_inlineSameContext(orig->m_inlineSameContext),
      m_contextSensitive(orig->m_contextSensitive),
      m_directInvoke(orig->m_directInvoke),
      m_needsRefTemp(orig->m_needsRefTemp),
      m_needsObjTemp(orig->m_needsObjTemp),
      m_needsCheckMem(orig->m_needsCheckMem),
      m_closureGenerator(orig->m_closureGenerator), m_noLSB(orig->m_noLSB),
      m_nextLSB(orig->m_nextLSB), m_hasTry(orig->m_hasTry),
      m_hasGoto(orig->m_hasGoto), m_localRedeclaring(orig->m_localRedeclaring),
      m_redeclaring(orig->m_redeclaring),
      m_inlineIndex(orig->m_inlineIndex), m_optFunction(orig->m_optFunction),
      m_nextID(0) {
  init(ar);
  m_originalName = originalName;
  setParamCounts(ar, m_minParam, m_maxParam);
}

void FunctionScope::init(AnalysisResultConstPtr ar) {
  m_dynamicInvoke = false;
  bool canInline = true;
  if (m_pseudoMain) {
    canInline = false;
    m_variables->forceVariants(ar, VariableTable::AnyVars);
    setReturnType(ar, Type::Variant);
  }

  if (m_refReturn) {
    m_returnType = Type::Variant;
  }

  if (!strcasecmp(m_name.c_str(), "__autoload")) {
    setVolatile();
  }

  // FileScope's flags are from parser, but VariableTable has more flags
  // coming from type inference phase. So we are tranferring these flags
  // just for better modularization between FileScope and VariableTable.
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

  if (m_stmt && Option::AllVolatile && !m_pseudoMain && !m_method) {
    m_volatile = true;
  }

  m_dynamic = Option::IsDynamicFunction(m_method, m_name) ||
    Option::EnableEval == Option::FullEval || Option::AllDynamic;
  if (!m_method && Option::DynamicInvokeFunctions.find(m_name) !=
      Option::DynamicInvokeFunctions.end()) {
    setDynamicInvoke();
  }
  if (m_modifiers) {
    m_virtual = m_modifiers->isAbstract();
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
      m_volatile(false), m_persistent(false), m_pseudoMain(false),
      m_magicMethod(false), m_system(true), m_inlineable(false), m_sep(false),
      m_containsThis(false), m_containsBareThis(0), m_nrvoFix(true),
      m_inlineAsExpr(false), m_inlineSameContext(false),
      m_contextSensitive(false),
      m_directInvoke(false), m_needsRefTemp(false), m_needsObjTemp(false),
      m_closureGenerator(false), m_noLSB(false), m_nextLSB(false),
      m_hasTry(false), m_hasGoto(false), m_localRedeclaring(false),
      m_redeclaring(-1), m_inlineIndex(0),
      m_optFunction(0) {
  m_dynamic = Option::IsDynamicFunction(method, m_name) ||
    Option::EnableEval == Option::FullEval || Option::AllDynamic;
  m_dynamicInvoke = false;
  if (!method && Option::DynamicInvokeFunctions.find(m_name) !=
      Option::DynamicInvokeFunctions.end()) {
    setDynamicInvoke();
  }
}

void FunctionScope::setDynamicInvoke() {
  m_returnType = Type::Variant;
  m_dynamicInvoke = true;
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

bool FunctionScope::ignoreRedefinition() const {
  return m_attribute & FileScope::IgnoreRedefinition;
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

bool FunctionScope::needsActRec() const {
  bool res = (m_attribute & FileScope::NeedsActRec);
  return res;
}

bool FunctionScope::isClosure() const {
  return ParserBase::IsClosureName(name());
}

bool FunctionScope::isGenerator() const {
  ASSERT(!getOrigGenStmt() ||
         (ParserBase::IsContinuationName(name()) &&
          m_paramNames.size() == 1 &&
          m_paramNames[0] == CONTINUATION_OBJECT_NAME));
  return getOrigGenStmt();
}

bool FunctionScope::isGeneratorFromClosure() const {
  bool res = isGenerator() && getOrigGenFS()->isClosure();
  ASSERT(!res || getOrigGenFS()->isClosureGenerator());
  return res;
}

MethodStatementRawPtr FunctionScope::getOrigGenStmt() const {
  if (!getStmt()) return MethodStatementRawPtr();
  MethodStatementPtr m =
    dynamic_pointer_cast<MethodStatement>(getStmt());
  return m ? m->getOrigGeneratorFunc() : MethodStatementRawPtr();
}

FunctionScopeRawPtr FunctionScope::getOrigGenFS() const {
  MethodStatementRawPtr origStmt = getOrigGenStmt();
  return origStmt ? origStmt->getFunctionScope() : FunctionScopeRawPtr();
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

void FunctionScope::setIgnoreRedefinition() {
  m_attribute |= FileScope::IgnoreRedefinition;
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

void FunctionScope::setNeedsActRec() {
  m_attribute |= FileScope::NeedsActRec;
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
     || (cls->classNameCtor() && getName() == cls->getName()));
}

bool FunctionScope::isMagic() const {
  return m_name.size() >= 2 && m_name[0] == '_' && m_name[1] == '_';
}

bool FunctionScope::needsLocalThis() const {
  return containsBareThis() &&
    (inPseudoMain() ||
     containsRefThis() ||
     isStatic() ||
     getVariables()->getAttribute(
       VariableTable::ContainsDynamicVariable));
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

void FunctionScope::addCaller(BlockScopePtr caller,
                              bool careAboutReturn /* = true */) {
  addUse(caller, UseKindCaller);
}

void FunctionScope::addNewObjCaller(BlockScopePtr caller) {
  addUse(caller, UseKindCaller & ~UseKindCallerReturn);
}

bool FunctionScope::mayUseVV() const {
  VariableTableConstPtr variables = getVariables();
  return (inPseudoMain() ||
          isVariableArgument() ||
          isGenerator() ||
          variables->getAttribute(VariableTable::ContainsDynamicVariable) ||
          variables->getAttribute(VariableTable::ContainsExtract) ||
          variables->getAttribute(VariableTable::ContainsCompact) ||
          variables->getAttribute(VariableTable::ContainsGetDefinedVars) ||
          variables->getAttribute(VariableTable::ContainsDynamicFunctionCall));
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

bool FunctionScope::needsClassParam() {
  if (!isStatic()) return false;
  ClassScopeRawPtr cls = getContainingClass();
  if (!ClassScope::NeedStaticArray(cls, FunctionScopeRawPtr(this))) {
    return false;
  }
  return getVariables()->hasStatic();
}

const char *FunctionScope::getPrefix(AnalysisResultPtr ar, ExpressionListPtr params) {
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
            if (!Type::SubType(ar, at, spec)) {
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
      if (!Option::AllDynamic) setDynamic();
    }
    return 0;
  }

  int ret = 0;
  if (params->getCount() < m_minParam) {
    if (exp->getScope()->isFirstPass()) {
      Compiler::Error(Compiler::TooFewArgument, exp, m_stmt);
    }
    valid = false;
    if (!Option::AllDynamic) setDynamic();
  }
  if (params->getCount() > m_maxParam) {
    if (isVariableArgument()) {
      ret = params->getCount() - m_maxParam;
    } else {
      if (exp->getScope()->isFirstPass()) {
        Compiler::Error(Compiler::TooManyArgument, exp, m_stmt);
      }
      valid = false;
      if (!Option::AllDynamic) setDynamic();
    }
  }

  bool canSetParamType = isUserFunction() && !m_overriding && !m_perfectVirtual;
  for (int i = 0; i < params->getCount(); i++) {
    ExpressionPtr param = (*params)[i];
    if (i < m_maxParam && param->hasContext(Expression::RefParameter)) {
      /**
       * This should be very un-likely, since call time pass by ref is a
       * deprecated, not very widely used (at least in FB codebase) feature.
       */
      TRY_LOCK_THIS();
      Symbol *sym = getVariables()->addSymbol(m_paramNames[i]);
      sym->setLvalParam();
      sym->setCallTimeRef();
    }
    if (valid && param->hasContext(Expression::InvokeArgument)) {
      param->clearContext(Expression::InvokeArgument);
      param->clearContext(Expression::RefValue);
      param->clearContext(Expression::NoRefWrapper);
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
    TypePtr expType;
    /**
     * Duplicate the logic of getParamType(i), w/o the mutation
     */
    TypePtr paramType(i < m_maxParam ? m_paramTypes[i] : TypePtr());
    if (!paramType) paramType = Type::Some;
    if (valid && !canSetParamType && i < m_maxParam &&
        (!Option::HardTypeHints || !m_paramTypeSpecs[i])) {
      /**
       * What is this magic, you might ask?
       *
       * Here, we take advantage of implicit conversion from every type to
       * Variant. Essentially, we don't really care what type comes out of this
       * expression since it'll just get converted anyways. Doing it this way
       * allows us to generate less temporaries along the way.
       */
      TypePtr optParamType(
          paramType->is(Type::KindOfVariant) ? Type::Some : paramType);
      expType = param->inferAndCheck(ar, optParamType, false);
    } else {
      expType = param->inferAndCheck(ar, Type::Some, false);
    }
    if (i < m_maxParam) {
      if (!Option::HardTypeHints || !m_paramTypeSpecs[i]) {
        if (canSetParamType) {
          if (!Type::SameType(paramType, expType) &&
              !paramType->is(Type::KindOfVariant)) {
            TRY_LOCK_THIS();
            paramType = setParamType(ar, i, expType);
          } else {
            // do nothing - how is this safe?  well, if we ever observe
            // paramType == expType, then this means at some point in the past,
            // somebody called setParamType() with expType.  thus, by calling
            // setParamType() again with expType, we contribute no "new"
            // information. this argument also still applies in the face of
            // concurrency
          }
        }
        // See note above. If we have an implemented type, however, we
        // should set the paramType to the implemented type to avoid an
        // un-necessary cast
        if (paramType->is(Type::KindOfVariant)) {
          TypePtr it(param->getImplementedType());
          paramType = it ? it : expType;
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
    addUpdates(UseKindCallerParam);
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

void FunctionScope::setParamDefault(int index, const char* value, int64_t len,
                                    const std::string &text) {
  ASSERT(index >= 0 && index < (int)m_paramNames.size());
  StringData* sd = new StringData(value, len, AttachLiteral);
  sd->setStatic();
  m_paramDefaults[index] = String(sd);
  m_paramDefaultTexts[index] = text;
}

CStrRef FunctionScope::getParamDefault(int index) {
  return m_paramDefaults[index];
}

void FunctionScope::addModifier(int mod) {
  if (!m_modifiers) {
    m_modifiers =
      ModifierExpressionPtr(new ModifierExpression(
                              shared_from_this(), LocationPtr()));
  }
  m_modifiers->add(mod);
}

void FunctionScope::setReturnType(AnalysisResultConstPtr ar, TypePtr type) {
  if (inTypeInference()) {
    getInferTypesMutex().assertOwnedBySelf();
  }
  // no change can be made to virtual function's prototype
  if (m_overriding || m_dynamicInvoke) return;

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
  ASSERT(m_returnType);
}

void FunctionScope::pushReturnType() {
  getInferTypesMutex().assertOwnedBySelf();
  m_prevReturn = m_returnType;
  m_hasVoid = false;
  if (m_overriding || m_dynamicInvoke || m_perfectVirtual || m_pseudoMain) {
    return;
  }
  m_returnType.reset();
}

bool FunctionScope::popReturnType() {
  getInferTypesMutex().assertOwnedBySelf();
  if (m_overriding || m_dynamicInvoke || m_perfectVirtual || m_pseudoMain) {
    return false;
  }

  if (m_returnType) {
    if (m_prevReturn) {
      if (Type::SameType(m_returnType, m_prevReturn)) {
        m_prevReturn.reset();
        return false;
      }
      if (!isFirstPass()) {
        Logger::Verbose("Corrected function return type %s -> %s",
                        m_prevReturn->toString().c_str(),
                        m_returnType->toString().c_str());
      }
    }
  } else if (!m_prevReturn) {
    return false;
  }

  m_prevReturn.reset();
  addUpdates(UseKindCallerReturn);
#ifdef HPHP_INSTRUMENT_TYPE_INF
  atomic_inc(RescheduleException::s_NumRetTypesChanged);
#endif /* HPHP_INSTRUMENT_TYPE_INF */
  return true;
}

void FunctionScope::resetReturnType() {
  getInferTypesMutex().assertOwnedBySelf();
  if (m_overriding || m_dynamicInvoke || m_perfectVirtual || m_pseudoMain) {
    return;
  }
  m_returnType = m_prevReturn;
  m_prevReturn.reset();
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

std::string FunctionScope::getId() const {
  string name = CodeGenerator::FormatLabel(getOriginalName());
  if (m_redeclaring < 0) {
    return name;
  }
  return name + Option::IdPrefix +
    boost::lexical_cast<std::string>(m_redeclaring);
}

std::string FunctionScope::getDocName() const {
  string name = getOriginalName();
  if (m_redeclaring < 0) {
    return name;
  }
  return name + Option::IdPrefix +
    boost::lexical_cast<std::string>(m_redeclaring);
}

std::string FunctionScope::getDocFullName() const {
  FunctionScope *self = const_cast<FunctionScope*>(this);
  const string &docName = getDocName();
  if (ClassScopeRawPtr cls = self->getContainingClass()) {
    return cls->getDocName() + string("::") + docName;
  }
  return docName;
}

std::string FunctionScope::getInjectionId() const {
  string injectionName = CodeGenerator::FormatLabel(getOriginalName());
  MethodStatementPtr stmt =
    dynamic_pointer_cast<MethodStatement>(getStmt());
  ASSERT(stmt);
  if (stmt->getGeneratorFunc()) {
    injectionName = isClosureGenerator() ?
      injectionName :
      injectionName + "{continuation}";
  } else if (stmt->getOrigGeneratorFunc() &&
             !getOrigGenFS()->isClosure()) {
    injectionName = CodeGenerator::FormatLabel(
      stmt->getOrigGeneratorFunc()->getOriginalName());
  }
  if (m_redeclaring < 0) {
    return injectionName;
  }
  const string &redecSuffix = string(Option::IdPrefix) +
    boost::lexical_cast<std::string>(m_redeclaring);
  return injectionName + redecSuffix;
}

///////////////////////////////////////////////////////////////////////////////

void FunctionScope::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (Option::GenerateInferredTypes && m_returnType) {
    cg_printf("// @return %s\n", m_returnType->toString().c_str());
  }

  BlockScope::outputPHP(cg, ar);
}

void FunctionScope::outputCPPDef(CodeGenerator &cg) {
  if (isVolatile()) {
    string name = CodeGenerator::FormatLabel(m_name);
    if (isRedeclaring()) {
      cg_printf("extern const %sCallInfo %s%s;\n",
                isRedeclaring() ? "Redeclared" : "",
                Option::CallInfoPrefix, getId().c_str());
      cg_printf("g->GCI(%s) = &%s%s;\n",
                name.c_str(),
                Option::CallInfoPrefix,
                getId().c_str());
    }
    cg_printf("g->FVF(%s) = true;\n", name.c_str());
  }
}

void FunctionScope::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  ExpressionListPtr params =
    dynamic_pointer_cast<MethodStatement>(getStmt())->getParams();

  int inTypedWrapper = Option::HardTypeHints ?
    cg.getContext() == CodeGenerator::CppTypedParamsWrapperImpl : -1;

  string funcName = CodeGenerator::EscapeLabel(getOriginalName());
  if (ClassScopePtr cls = getContainingClass()) {
    funcName = CodeGenerator::EscapeLabel(cls->getOriginalName()) + "::" +
               funcName;
  }
  funcName += "()";

  /* Typecheck parameters */
  for (int i = 0; i < m_maxParam; i++) {
    if (inTypedWrapper <= 0 && isRefParam(i)) {
      const string &name = getParamName(i);
      string vname = Option::VariablePrefix + CodeGenerator::FormatLabel(name);
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
        default: not_reached();
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
          string vname = Option::VariablePrefix +
                         CodeGenerator::FormatLabel(name);
          if (!ref || paramType->isExactType()) {
            cg_printf(" v%s = %s;\n", vname.c_str(), vname.c_str());
          } else {
            cg_printf(" v%s = strongBind(%s);\n",
                      vname.c_str(), vname.c_str());
          }
        }
      }
    }

    ParameterExpressionPtrVec useVars;
    if (needsAnonClosureClass(useVars)) {
      if (!m_closureGenerator) {
        VariableTablePtr variables = getVariables();
        BOOST_FOREACH(ParameterExpressionPtr param, useVars) {
          const string &name = param->getName();
          Symbol *sym = variables->getSymbol(name);
          ASSERT(sym->isUsed());
          TypePtr t(sym->getFinalType());
          ASSERT(!param->isRef() || t->is(Type::KindOfVariant));
          if (t->is(Type::KindOfVariant)) {
            const char *s = param->isRef() ? "Ref" : "Val";
            cg_printf("%s%s.assign%s(closure->%s%s);\n",
                      Option::VariablePrefix, name.c_str(), s,
                      Option::VariablePrefix, name.c_str());
          } else {
            cg_printf("%s%s = closure->%s%s;\n",
                      Option::VariablePrefix, name.c_str(),
                      Option::VariablePrefix, name.c_str());
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
    cg_printf("bool incOnce, LVariableTable* variables, Globals *globals");
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
    if (isRefParam(i)) {
      cg_printf("VRefParam");
    } else {
      TypePtr type = getParamType(i);
      type->outputCPPDecl(cg, ar, BlockScopeRawPtr(this));
    }
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
      cg_printf("ArrayInit(%d, ArrayInit::vectorInit).", m_maxParam);
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
                                       int extraArgArrayIndex /* = -1 */,
                                       bool ignoreFuncParamTypes /* = false */)
{
  int paramCount = params ? params->getOutputCount() : 0;
  ASSERT(extraArg <= paramCount);
  int iMax = paramCount - extraArg;
  bool extra = false;
  bool callUserFuncFewArgs = false;

  if (variableArgument) {
    callUserFuncFewArgs =
      Option::UseCallUserFuncFewArgs &&
      func->getName() == "call_user_func" &&
      (paramCount <= CALL_USER_FUNC_FEW_ARGS_COUNT + 1) &&
      extraArgArrayId == -1;

    if (!callUserFuncFewArgs) {
      if (paramCount == 0) {
        cg_printf("0");
      } else {
        cg_printf("%d, ", paramCount);
      }
    }
  }
  for (int i = 0; i < paramCount; i++) {
    ExpressionPtr param = (*params)[i];
    if (i > 0) cg_printf(extra && !callUserFuncFewArgs ? "." : ", ");
    if (!extra && (i == iMax || extraArg < 0)) {
      if (extraArgArrayId != -1) {
        always_assert(extraArgArrayHash != -1 && extraArgArrayIndex != -1);
        ar->outputCPPScalarArrayId(cg, extraArgArrayId, extraArgArrayHash,
                                   extraArgArrayIndex);
        break;
      }
      extra = true;
      if (!callUserFuncFewArgs) {
        // Parameter arrays are always vectors.
        if (Option::GenArrayCreate &&
            cg.getOutput() != CodeGenerator::SystemCPP) {
          if (!params->hasNonArrayCreateValue(false, i)) {
            always_assert(!callUserFuncFewArgs);
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
        cg_printf("Array(ArrayInit(%d, ArrayInit::vectorInit).",
                  paramCount - i);
      }
    }
    if (extra) {
      bool needRef = param->hasContext(Expression::RefValue) &&
        !param->hasContext(Expression::NoRefWrapper) &&
        param->isRefable();
      if (!callUserFuncFewArgs) {
        cg_printf("set%s(", needRef ? "Ref" : "");
      }
      if (needRef) {
        // The parameter itself shouldn't be wrapped with ref() any more.
        param->setContext(Expression::NoRefWrapper);
      }
      param->outputCPP(cg, ar);
      if (!callUserFuncFewArgs) cg_printf(")");
    } else {
      // If the implemented type is ref-counted and expected type is variant,
      // use VarNR to avoid unnecessary ref-counting because we know
      // the actual argument will always has a ref in the callee.
      bool wrap = false, wrapv = false;
      bool scalar = param->isScalar();
      bool isValidFuncIdx = func && i < func->getMaxParamCount();
      TypePtr expType(
          isValidFuncIdx && !ignoreFuncParamTypes ?
            // the common case, where we read the param type from the function
            // scope
            func->getParamType(i) :

            // the uncommon case
            !func && !ignoreFuncParamTypes ?

              // if we don't have a function, then we can assume its param type
              // is Variant.  however, we only want to do this if we aren't
              // ignoring function param types
              Type::Variant :

              // in this case, just use the expected type from the parameter
              // ast node (that's the default behavior of CheckVarNR)
              TypePtr());
      if (Expression::CheckVarNR(param, expType)) {
        if (scalar) {
          ASSERT(!cg.hasScalarVariant());
          cg.setScalarVariant();
        }
        if (!scalar ||
            (!Option::UseScalarVariant && !param->isLiteralNull())) {
          wrap = true;
        }
        if (isValidFuncIdx) {
          VariableTablePtr variables = func->getVariables();
          if (variables->isLvalParam(func->getParamName(i))) {
            // Callee expects a Variant instead of CVarRef
            wrap = false;
            if (scalar) cg.clearScalarVariant();
          }
        }
      } else if (isValidFuncIdx &&
                 !param->hasCPPTemp() &&
                 func->getVariables()->isLvalParam(func->getParamName(i)) &&
                 !param->hasAnyContext(Expression::RefValue|
                                       Expression::RefParameter|
                                       Expression::InvokeArgument)) {
        FunctionCallPtr f = dynamic_pointer_cast<FunctionCall>(param);
        if (f) {
          if (f->getFuncScope()) {
            wrapv = f->getFuncScope()->isRefReturn();
          } else if (!f->getName().empty()) {
            FunctionInfoPtr info = GetFunctionInfo(f->getName());
            wrapv = info && info->getMaybeRefReturn();
          } else {
            wrapv = true;
          }
          if (wrapv) cg_printf("wrap_variant(");
        }
      }
      if (wrap) cg_printf("VarNR(");
      param->outputCPP(cg, ar);
      if (scalar) cg.clearScalarVariant();
      ASSERT(!cg.hasScalarVariant());
      if (wrap || wrapv) {
        cg_printf(")");
      }
    }
  }
  if (extra) {
    if (!callUserFuncFewArgs) cg_printf(".create())");
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
  string fullname = CodeGenerator::EscapeLabel(getOriginalFullName());
  if (checkMissing) {
    bool fullGuard = ret && (system || Option::HardTypeHints);
    for (int i = 0; i < m_minParam; i++) {
      if (TypePtr t = m_paramTypeSpecs[i]) {
        if (m_paramDefaults[i].empty()) {
          if (i < maxCount) cg_printf("if (UNLIKELY(count < %d)) ", i + 1);
          cg_printf("%sthrow_missing_typed_argument(\"%s\", ",
                    fullGuard ? "return " : "", fullname.c_str());
          cg_printf(t->is(Type::KindOfArray) ?
                    "0" : "\"%s\"",
                    CodeGenerator::EscapeLabel(t->getName()).c_str());
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
          cg_printf("if (UNLIKELY(count < %d))", m_minParam);
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
      bool isCVarRef = false;
      if (!m_paramDefaults[i].empty() && !useDefaults) {
        Variant tmp;
        MethodStatementPtr m(dynamic_pointer_cast<MethodStatement>(m_stmt));
        ExpressionListPtr params = m->getParams();
        always_assert(params && params->getCount() > i);
        ParameterExpressionPtr p(
          dynamic_pointer_cast<ParameterExpression>((*params)[i]));
        ExpressionPtr defVal = p->defaultValue();
        bool isScalar = defVal->isScalar();
        if (Option::UseScalarVariant && isScalar && !ref && fewArgs) isCVarRef = true;
        dftNull = isScalar && defVal->getScalarValue(tmp) && tmp.isNull();
        if (!isCVarRef && wantNullVariantNotNull && !dftNull && !ref) {
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
          always_assert(params && params->getCount() > i);
          ParameterExpressionPtr p(
            dynamic_pointer_cast<ParameterExpression>((*params)[i]));

          if (defIndex >= 0) {
            if (!isCVarRef) cg_printf("*new (&def%d) Variant(", defIndex);
          } else {
            ExpressionPtr defVal = p->defaultValue();
            bool isScalar = defVal->isScalar();
            if (Option::UseScalarVariant && isScalar && !ref && fewArgs) isCVarRef = true;
            TypePtr type = p->defaultValue()->getCPPType();
            bool isVariant = type && Type::IsMappedToVariant(type);
            cg_printf("%s(", ref ? "VRefParamValue" :
                      (!fewArgs || i >= maxCount) && !isVariant && !isCVarRef ?
                      "Variant" : "");
          }
          cg.setContext(CodeGenerator::CppParameterDefaultValueDecl);
          ExpressionPtr defVal = p->defaultValue();
          if (isCVarRef) {
            ASSERT(!cg.hasScalarVariant());
            cg.setScalarVariant();
          }

          TypePtr exp = defVal->getExpectedType();
          defVal->setExpectedType(TypePtr());
          defVal->outputCPP(cg, ar);
          defVal->setExpectedType(exp);

          if (isCVarRef) cg.clearScalarVariant();
          ASSERT(!cg.hasScalarVariant());
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
        if (m_maxParam) {
          cg_printf("const Array &p(count > %d ? "
                    "params.slice(%d, count - %d, false) : Array());\n",
                    m_maxParam, m_maxParam, m_maxParam);
        } else {
          // in this case, avoid the un-necessary call to params.slice(),
          // since we want the entire params array anyways
          ASSERT(m_maxParam == 0);
          cg_printf("const Array &p(count > 0 ? "
                    "ArrayUtil::EnsureIntKeys(params) : Array());\n");
        }
        extra = ", p";
      }
    }

    if (!last) {
      cg_printf("if (count <= %d) ", i);
      if (do_while) {
        cg_indentBegin("{\n");
      }
    }
    cg_printf("%s%s%s", retrn, (m_refReturn ? "strongBind(" : "("),
              instance ? instance : "");
    if (m_perfectVirtual) {
      ClassScopePtr cls = getContainingClass();
      cg_printf("%s%s::", Option::ClassPrefix, cls->getId().c_str());
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

void FunctionScope::serialize(JSON::CodeError::OutputStream &out) const {
  JSON::CodeError::MapStream ms(out);
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

void FunctionScope::serialize(JSON::DocTarget::OutputStream &out) const {
  JSON::DocTarget::MapStream ms(out);

  ms.add("name", getDocName());
  ms.add("line", getStmt() ? getStmt()->getLocation()->line0 : 0);
  ms.add("docs", m_docComment);

  int mods = 0;
  if (isPublic())    mods |= ClassInfo::IsPublic;
  if (isProtected()) mods |= ClassInfo::IsProtected;
  if (isPrivate())   mods |= ClassInfo::IsPrivate;
  if (isStatic())    mods |= ClassInfo::IsStatic;
  if (isFinal())     mods |= ClassInfo::IsFinal;
  if (isAbstract())  mods |= ClassInfo::IsAbstract;
  ms.add("modifiers", mods);

  ms.add("refreturn", isRefReturn());
  ms.add("return",    getReturnType());

  vector<SymParamWrapper> paramSymbols;
  for (int i = 0; i < m_maxParam; i++) {
    const string &name = getParamName(i);
    const Symbol *sym = getVariables()->getSymbol(name);
    ASSERT(sym && sym->isParameter());
    paramSymbols.push_back(SymParamWrapper(sym));
  }
  ms.add("parameters", paramSymbols);

  // scopes that call this scope (callers)
  vector<string> callers;
  const BlockScopeRawPtrFlagsPtrVec &deps = getDeps();
  for (BlockScopeRawPtrFlagsPtrVec::const_iterator it = deps.begin();
       it != deps.end(); ++it) {
    const BlockScopeRawPtrFlagsPtrPair &p(*it);
    if ((*p.second & BlockScope::UseKindCaller) &&
        p.first->is(BlockScope::FunctionScope)) {
      FunctionScopeRawPtr f(
          static_pointer_cast<FunctionScope>(p.first));
      callers.push_back(f->getDocFullName());
    }
  }
  ms.add("callers", callers);

  // scopes that this scope calls (callees)
  // TODO(stephentu): this list only contains *user* functions,
  // we should also include builtins
  vector<string> callees;
  const BlockScopeRawPtrFlagsVec &users = getOrderedUsers();
  for (BlockScopeRawPtrFlagsVec::const_iterator uit = users.begin();
       uit != users.end(); ++uit) {
    BlockScopeRawPtrFlagsVec::value_type pf = *uit;
    if ((pf->second & BlockScope::UseKindCaller) &&
        pf->first->is(BlockScope::FunctionScope)) {
      FunctionScopeRawPtr f(
          static_pointer_cast<FunctionScope>(pf->first));
      callees.push_back(f->getDocFullName());
    }
  }
  ms.add("callees", callees);

  ms.done();
}

void FunctionScope::outputCPPCreateDecl(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  ClassScopePtr scope = getContainingClass();
  CodeGenerator::Context context = cg.getContext();
  bool setWrapper = Option::HardTypeHints && needsTypeCheckWrapper();

  cg_printf("public: %s%s *create(",
            Option::ClassPrefix, scope->getId().c_str());
  cg.setContext(setWrapper ?
                CodeGenerator::CppTypedParamsWrapperDecl :
                CodeGenerator::CppFunctionWrapperDecl);
  outputCPPParamsDecl(cg, ar,
                      dynamic_pointer_cast<MethodStatement>(getStmt())
                      ->getParams(), true);
  cg.setContext(context);
  cg_printf(");\n");
}

void FunctionScope::outputCPPCreateImpl(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  ClassScopePtr scope = getContainingClass();
  string clsNameStr = scope->getId();
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
  if (scope->derivesFromRedeclaring() ||
      scope->hasAttribute(ClassScope::HasDestructor, ar)) {
    cg_printf("clearNoDestruct();\n");
  }
  cg_printf("return this;\n");
  cg_indentEnd("}\n");
}

void FunctionScope::outputCPPClassMap(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_method && ParserBase::IsAnonFunctionName(m_name)) {
    return;
  }
  int attribute = ClassInfo::IsNothing;
  if (!isUserFunction()) attribute |= ClassInfo::IsSystem;
  if (!m_method && isSepExtension()) attribute |= ClassInfo::IsPrivate;
  if (isVolatile() && !isRedeclaring()) attribute |= ClassInfo::IsVolatile;
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

  if (m_method && isConstructor(getContainingClass())) {
    attribute |= ClassInfo::IsConstructor;
  }
  if (needsActRec()) {
    attribute |= ClassInfo::NeedsActRec;
  }

  // Use the original cased name, for reflection to work correctly.
  cg_printf("(const char *)0x%04X, \"%s\", \"%s\", (const char *)%d, "
            "(const char *)%d,\n", attribute,
            CodeGenerator::EscapeLabel(getOriginalName()).c_str(),
            m_stmt ? m_stmt->getLocation()->file : "",
            m_stmt ? m_stmt->getLocation()->line0 : 0,
            m_stmt ? m_stmt->getLocation()->line1 : 0);


  if (attribute & ClassInfo::IsVolatile) {
    cg_printf("(const char*)offsetof(GlobalVariables, FVF(%s)),\n",
              CodeGenerator::FormatLabel(m_name).c_str());
  }

  if (!m_docComment.empty() && Option::GenerateDocComments) {
    std::string dc = string_cplus_escape(m_docComment.c_str(), m_docComment.size());
    cg_printf("\"%s\",\n", dc.c_str());
  }

  if (attribute & ClassInfo::IsSystem) {
    if (m_returnType) {
      cg_printf("(const char *)0x%x, ", m_returnType->getDataType());
    } else {
      cg_printf("(const char *)-1, ");
    }
  }

  Variant defArg;
  for (int i = 0; i < m_maxParam; i++) {
    int attr = ClassInfo::IsNothing;
    if (isRefParam(i)) attr |= ClassInfo::IsReference;

    cg_printf("(const char *)0x%04X, \"%s\", ",
              attr, m_paramNames[i].c_str());

    MethodStatementPtr m =
      dynamic_pointer_cast<MethodStatement>(getStmt());
    if (m) {
      ExpressionListPtr params = m->getParams();
      always_assert(i < params->getCount());
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*params)[i]);
      always_assert(param);
      cg_printf("\"%s\", ", param->hasTypeHint() ?
                            param->getOriginalTypeHint().c_str() : "");
      if (attribute & ClassInfo::IsSystem) {
        cg_printf("(const char *)-1, ");
      }
      ExpressionPtr def = param->defaultValue();
      if (def) {
        std::string sdef = def->getText();
        std::string esdef = string_cplus_escape(sdef.data(), sdef.size());
        ASSERT(!esdef.empty());
        if (!def->isScalar() || !def->getScalarValue(defArg)) {
          /**
           * Special value runtime/ext/ext_reflection.cpp can check and throw.
           * If we want to avoid seeing this so to make getDefaultValue()
           * work better for reflections, we will have to implement
           * getScalarValue() to greater extent under compiler/expressions.
           */
          cg_printf("\"\x01\", \"%s\",\n", esdef.c_str());
        } else {
          String str = f_serialize(defArg);
          std::string s = string_cplus_escape(str.data(), str.size());
          cg_printf("\"%s\", \"%s\",\n", s.c_str(), esdef.c_str());
        }
      } else {
        cg_printf("\"\", \"\",\n");
      }
      // user attributes
      ExpressionListPtr paramUserAttrs =
        dynamic_pointer_cast<ExpressionList>(param->userAttributeList());
      if (paramUserAttrs) {
        for (int j = 0; j < paramUserAttrs->getCount(); ++j) {
          UserAttributePtr a = dynamic_pointer_cast<UserAttribute>(
            (*paramUserAttrs)[j]);
          ExpressionPtr expr = a->getExp();
          Variant v;
          bool isScalar UNUSED = expr->getScalarValue(v);
          ASSERT(isScalar);
          int valueLen = 0;
          string valueText = SymbolTable::getEscapedText(v, valueLen);
          cg_printf("\"%s\", (const char *)%d, \"%s\",\n",
                    CodeGenerator::EscapeLabel(a->getName()).c_str(),
                    valueLen, valueText.c_str());
        }
      }
      cg_printf("NULL,\n");
    } else {
      cg_printf("\"\", ");
      if (attribute & ClassInfo::IsSystem) {
        cg_printf("(const char *)0x%x, ", m_paramTypes[i]->getDataType());
      }
      std::string def = string_cplus_escape(m_paramDefaults[i].data(),
                                            m_paramDefaults[i].size());
      std::string defText = string_cplus_escape(m_paramDefaultTexts[i].data(),
                                                m_paramDefaultTexts[i].size());
      cg_printf("\"%s\", \"%s\", NULL,\n", def.c_str(), defText.c_str());
    }
  }
  cg_printf("NULL,\n");

  m_variables->outputCPPStaticVariables(cg, ar);

  // user attributes
  UserAttributeMap::const_iterator it = m_userAttributes.begin();
  for (; it != m_userAttributes.end(); ++it) {
    ExpressionPtr expr = it->second;
    Variant v;
    bool isScalar UNUSED = expr->getScalarValue(v);
    ASSERT(isScalar);
    int valueLen = 0;
    string valueText = SymbolTable::getEscapedText(v, valueLen);
    cg_printf("\"%s\", (const char *)%d, \"%s\",\n",
              CodeGenerator::EscapeLabel(it->first).c_str(),
              valueLen, valueText.c_str());
  }
  cg_printf("NULL,\n");
}

void FunctionScope::outputCPPHelperClassAlloc(CodeGenerator &cg,
                                              AnalysisResultPtr ar) {

  const string &funcName = CodeGenerator::FormatLabel(m_name);
  ParameterExpressionPtrVec useVars;
  if (needsAnonClosureClass(useVars)) {
    cg_printf("IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(%sClosure$%s)\n",
              Option::ClassPrefix, funcName.c_str());
  }
  if (isGenerator()) {
    cg_printf(
        "IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(%sContinuation$%s)\n",
        Option::ClassPrefix, funcName.c_str());
  }
}

void FunctionScope::outputCPPCallInfo(CodeGenerator &cg,
    AnalysisResultPtr ar) {
  if (isAbstract()) return;
  string id;
  if (m_method) {
    id = CodeGenerator::FormatLabel(m_name);
  } else {
    id = getId();
  }
  uint64 refflags = 0;
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
    if (isProtected()) {
      flags |= CallInfo::Protected;
    } else if (isPrivate()) {
      flags |= CallInfo::Private;
    }
    if (isStatic()) {
      flags |= CallInfo::StaticMethod;
    }
    ClassScopePtr scope = getContainingClass();
    string clsName = scope->getId();

    cg.printf("extern const CallInfo %s%s%s%s = { (void*)&%s%s::%s%s, ",
              Option::CallInfoPrefix, clsName.c_str(),
              Option::IdPrefix.c_str(), id.c_str(),
              Option::ClassPrefix, clsName.c_str(), Option::InvokePrefix,
              id.c_str());
    cg.printf("(void*)&%s%s::%s%s", Option::ClassPrefix, clsName.c_str(),
              Option::InvokeFewArgsPrefix, id.c_str());
    if (m_name == "__invoke" &&
        strcasecmp(clsName.c_str(), "closure")) {
      cg.printf(", %d, %d, 0x%.16llXLL};\n",
                m_maxParam, flags, refflags);

      // need to generate an extra call info for an extra wrapper
      cg.printf("const CallInfo %s%s%s%s = { (void*)&%s%s::%s%s, ",
                Option::CallInfoWrapperPrefix, clsName.c_str(),
                Option::IdPrefix.c_str(), id.c_str(),
                Option::ClassPrefix, clsName.c_str(),
                Option::InvokeWrapperPrefix, id.c_str());
      cg.printf("(void*)&%s%s::%s%s", Option::ClassPrefix, clsName.c_str(),
                Option::InvokeWrapperFewArgsPrefix, id.c_str());
    }
  } else {
    cg.printf("extern const %sCallInfo %s%s = {",
              isRedeclaring() ? "Redeclared" : "",
              Option::CallInfoPrefix, id.c_str());
    if (isRedeclaring()) cg_printf("{");
    cg_printf("(void*)&%s%s, (void*)&%s%s",
              Option::InvokePrefix, id.c_str(),
              Option::InvokeFewArgsPrefix, id.c_str());
  }
  cg.printf(", %d, %d, 0x%.16llXLL", m_maxParam, flags, refflags);
  if (isRedeclaring()) {
    cg_printf("}, %d", getRedeclaringId());
  }
  cg_printf("};\n");
}

void FunctionScope::getClosureUseVars(
    ParameterExpressionPtrIdxPairVec &useVars,
    bool filterUsed /* = true */) {
  useVars.clear();
  if (!m_closureVars) return;
  ASSERT(isClosure());
  VariableTablePtr variables = getVariables();
  for (int i = 0; i < m_closureVars->getCount(); i++) {
    ParameterExpressionPtr param =
      dynamic_pointer_cast<ParameterExpression>((*m_closureVars)[i]);
    const string &name = param->getName();
    if (!filterUsed || variables->isUsed(name)) {
      useVars.push_back(ParameterExpressionPtrIdxPair(param, i));
    }
  }
}

template <class U, class V>
static U pair_first_elem(std::pair<U, V> p) { return p.first; }

bool FunctionScope::needsAnonClosureClass(ParameterExpressionPtrVec &useVars) {
  useVars.clear();
  if (!isClosure()) return false;
  ParameterExpressionPtrIdxPairVec useVars0;
  getClosureUseVars(useVars0, !m_closureGenerator);
  useVars.resize(useVars0.size());
  // C++ seems to be unable to infer the type here on pair_first_elem
  transform(useVars0.begin(),
            useVars0.end(),
            useVars.begin(),
            pair_first_elem<ParameterExpressionPtr, int>);
  return useVars.size() > 0 || getVariables()->hasStaticLocals();
}

bool FunctionScope::needsAnonClosureClass(
    ParameterExpressionPtrIdxPairVec &useVars) {
  useVars.clear();
  if (!isClosure()) return false;
  getClosureUseVars(useVars, !m_closureGenerator);
  return useVars.size() > 0 || getVariables()->hasStaticLocals();
}

void FunctionScope::outputCPPSubClassParam(CodeGenerator &cg,
                                           AnalysisResultPtr ar,
                                           ParameterExpressionPtr param) {
  VariableTablePtr variables = getVariables();
  const string &name = param->getName();
  Symbol *sym = variables->getSymbol(name);
  ASSERT(sym);
  TypePtr t(sym->getFinalType());
  if (!param->isRef()) {
    if (t->is(Type::KindOfVariant) || t->is(Type::KindOfSome)) {
      cg_printf("CVarRef");
    } else if (t->is(Type::KindOfArray)) cg_printf("CArrRef");
    else if (t->is(Type::KindOfString))  cg_printf("CStrRef");
    else if (t->isStandardObject())      cg_printf("CObjRef");
    else t->outputCPPDecl(cg, ar, shared_from_this());
  } else {
    t->outputCPPDecl(cg, ar, shared_from_this());
  }
  cg_printf(" %s%s",
            Option::TempVariablePrefix,
            CodeGenerator::FormatLabel(name).c_str());
}

void FunctionScope::outputCPPPreface(CodeGenerator &cg, AnalysisResultPtr ar) {
  ClassScopeRawPtr cls = getContainingClass();
  if (!cls) {
    if (!inPseudoMain()) {
      // spit out extern CallInfo decl
      cg_printf("extern const %sCallInfo %s%s;\n",
                isRedeclaring() ? "Redeclared" : "",
                Option::CallInfoPrefix, getId().c_str());
    }
  } else {
    if (isGenerator()) {
      cg_printf("extern const CallInfo %s%s%s%s;\n",
                Option::CallInfoPrefix, getContainingClass()->getId().c_str(),
                Option::IdPrefix.c_str(), getId().c_str());
    }
  }

  const string &funcName = CodeGenerator::FormatLabel(m_name);
  ParameterExpressionPtrVec useVars;
  if (needsAnonClosureClass(useVars) &&
      cg.insertDeclaredClosure(this)) {
    cg_printf("FORWARD_DECLARE_CLASS(Closure$%s);\n", funcName.c_str());
    cg_indentBegin("class %sClosure$%s : public %sClosure {\n",
                   Option::ClassPrefix, funcName.c_str(),
                   Option::ClassPrefix);

    cg_printf("public:\n");

    cg_printf("DECLARE_OBJECT_ALLOCATION_NO_SWEEP(%sClosure$%s)\n",
              Option::ClassPrefix, funcName.c_str());

    VariableTablePtr variables = getVariables();
    if (!useVars.empty()) {
      BOOST_FOREACH(ParameterExpressionPtr param, useVars) {
        const string &name = param->getName();
        Symbol *sym = variables->getSymbol(name);
        TypePtr t(sym->getFinalType());
        t->outputCPPDecl(cg, ar, shared_from_this());
        const string &varName = variables->getVariableName(ar, sym);
        cg_printf(" %s;\n", varName.c_str());
      }
      cg_printf("\n");
    }

    if (variables->hasStaticLocals()) {
      variables->outputCPPStaticLocals(cg, ar, false);
      cg_printf("\n");
    }

    cg_printf("%sClosure$%s(const CallInfo *func, const char *name",
              Option::ClassPrefix, funcName.c_str());

    if (!useVars.empty()) cg_printf(", ");
    bool hasEmit = false;
    BOOST_FOREACH(ParameterExpressionPtr param, useVars) {
      if (!hasEmit) hasEmit = true;
      else          cg_printf(", ");
      outputCPPSubClassParam(cg, ar, param);
    }

    // TODO: non ref variants can be directly assigned to the member
    // variable in the initialization list, giving an ever-so-slight
    // gain in performance
    cg_printf(") : %sClosure(func, name)", Option::ClassPrefix);
    if (variables->hasStaticLocals()) {
      cg_printf(", ");
      variables->outputCPPStaticLocals(cg, ar, true);
    }
    cg_indentBegin(" {\n");
    BOOST_FOREACH(ParameterExpressionPtr param, useVars) {
      const string &name = param->getName();
      Symbol *sym = variables->getSymbol(name);
      ASSERT(sym);
      TypePtr t(sym->getFinalType());
      ASSERT(!param->isRef() || t->is(Type::KindOfVariant));
      const string &varName = variables->getVariableName(ar, sym);
      const string &tmpName =
        string(Option::TempVariablePrefix) + CodeGenerator::FormatLabel(name);
      if (t->is(Type::KindOfVariant)) {
        const char *s = param->isRef() ? "Ref" : "Val";
        cg_printf("%s.assign%s(%s);\n", varName.c_str(), s, tmpName.c_str());
      } else {
        ASSERT(!param->isRef());
        cg_printf("%s = %s;\n", varName.c_str(), tmpName.c_str());
      }
    }
    cg_indentEnd("}\n");
    cg_indentEnd("};\n");
  }

  if (isGenerator()) {
    cg_printf("FORWARD_DECLARE_CLASS(Continuation$%s);\n", funcName.c_str());
    cg_indentBegin("class %sContinuation$%s : public %sContinuation {\n",
                   Option::ClassPrefix, funcName.c_str(), Option::ClassPrefix);

    if (cls && cls->derivedByDynamic()) {
      /*
       * The m_obj property is potentially a redeclared-parent. We
       * need to keep a reference to the root object too, to ensure
       * that the root object doesnt get destroyed before m_obj
       */
      cg_printf("Object o_rootObj;\n");
    }
    cg_printf("public:\n");

    cg_printf("DECLARE_OBJECT_ALLOCATION_NO_SWEEP(%sContinuation$%s)\n",
              Option::ClassPrefix, funcName.c_str());

    VariableTablePtr variables = getVariables();
    vector<Symbol*> symbols;
    variables->getSymbols(symbols, true);

    if (!symbols.empty()) {
      BOOST_FOREACH(Symbol *sym, symbols) {
        const string &varName = variables->getVariableName(ar, sym);
        TypePtr type = sym->getFinalType();
        type->outputCPPDecl(cg, ar, shared_from_this());
        cg_printf(" %s;\n", varName.c_str());
      }
      cg_printf("\n");
    }

    if (variables->hasStaticLocals()) {
      variables->outputCPPStaticLocals(cg, ar, false);
      cg_printf("\n");

      // no arg ctor w/ init list of static locals
      cg_printf("%sContinuation$%s() : ",
                Option::ClassPrefix,
                funcName.c_str());
      variables->outputCPPStaticLocals(cg, ar, true);
      cg_printf(" {}\n");
    }

    // constructor is bootstrapped with all the parameters
    // of the original generator function + use vars for any
    // surrounding closure generator
    cg_printf("static %sContinuation$%s Build("
              "int64 func, int64 extra, bool isMethod, CStrRef origFuncName, ",
              Option::SmartPtrPrefix, funcName.c_str());

    MethodStatementRawPtr orig = getOrigGenStmt();
    ASSERT(orig);
    ExpressionListPtr params = orig->getParams();

    vector<ParameterExpressionPtr> ctorParams;
    if (params) {
      for (int i = 0; i < params->getCount(); i++) {
        ParameterExpressionPtr param =
          dynamic_pointer_cast<ParameterExpression>((*params)[i]);
        ctorParams.push_back(param);
      }
    }
    useVars.clear();
    bool needsClosureCls =
      orig->getFunctionScope()->needsAnonClosureClass(useVars);
    if (needsClosureCls) {
      ASSERT(useVars.size() > 0);
      ctorParams.insert(ctorParams.end(), useVars.begin(), useVars.end());
    }

    bool hasEmit = false;
    BOOST_FOREACH(ParameterExpressionPtr param, ctorParams) {
      const string& name = param->getName();
      Symbol *sym = variables->getSymbol(name);
      if (sym) {
        if (!hasEmit) hasEmit = true;
        else          cg_printf(", ");
        outputCPPSubClassParam(cg, ar, param);
      }
    }

    if (hasEmit) cg_printf(", ");
    if (cls && cls->derivedByDynamic()) {
      cg_printf("CObjRef rootObj, ");
    }
    cg_indentBegin("CVarRef obj, CArrRef args) {\n");
    cg_printf("%sContinuation$%s cont = "
              "%sContinuation$%s(NEWOBJ(%sContinuation$%s)());\n",
              Option::SmartPtrPrefix, funcName.c_str(),
              Option::SmartPtrPrefix, funcName.c_str(),
              Option::ClassPrefix,    funcName.c_str());
    cg_printf("cont->%s__construct"
              "(func, extra, isMethod, origFuncName, obj, args);\n",
              Option::MethodPrefix);
    if (cls && cls->derivedByDynamic()) {
      cg_printf("cont->o_rootObj = rootObj;\n");
    }

    BOOST_FOREACH(ParameterExpressionPtr param, ctorParams) {
      const string& name = param->getName();
      Symbol *sym = variables->getSymbol(name);
      if (sym) {
        const string &varName = variables->getVariableName(ar, sym);
        const string &tmpName =
          string(Option::TempVariablePrefix) +
          CodeGenerator::FormatLabel(name);
        TypePtr t(sym->getFinalType());
        if (t->is(Type::KindOfVariant)) {
          const char *s = param->isRef() ? "Ref" : "Val";
          cg_printf("cont->%s.assign%s(%s);\n",
                    varName.c_str(), s, tmpName.c_str());
        } else {
          ASSERT(!param->isRef());
          cg_printf("cont->%s = %s;\n", varName.c_str(), tmpName.c_str());
        }
      }
    }
    if (usesLSB()) {
      cg_printf("cont->setCalledClass(f_get_called_class());\n");
    }
    cg_printf("return cont;\n");
    cg_indentEnd("}\n");
    cg_indentEnd("};\n");
  }
}

FunctionScope::StringToFunctionInfoPtrMap FunctionScope::s_refParamInfo;
static Mutex s_refParamInfoLock;

void FunctionScope::RecordFunctionInfo(string fname, FunctionScopePtr func) {
  Lock lock(s_refParamInfoLock);
  FunctionInfoPtr &info = s_refParamInfo[fname];
  if (!info) {
    info = FunctionInfoPtr(new FunctionInfo());
  }
  if (func->isStatic()) {
    info->setMaybeStatic();
  }
  if (func->isRefReturn()) {
    info->setMaybeRefReturn();
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
      cg_printf(" %s%s(", Option::MethodWrapperPrefix, getId().c_str());
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

      cg_printf("%s%s(", Option::MethodPrefix,
                         CodeGenerator::FormatLabel(m_name).c_str());
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

      always_assert(!isStatic());
      cg_printf("%sinvoke(\"%s\", params, -1);\n",
                Option::ObjectPrefix, m_name.c_str());
    }

    if (clsToConstruct) {
      cg_printf("return ret;\n");
    }
    cg_indentEnd("}\n");
  }
}

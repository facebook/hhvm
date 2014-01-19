/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/expression/function_call.h"
#include "hphp/util/util.h"
#include "hphp/util/logger.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/exp_statement.h"
#include "hphp/compiler/statement/return_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/array_pair_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/parser/hphp.tab.hpp"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

FunctionCall::FunctionCall
(EXPRESSION_CONSTRUCTOR_BASE_PARAMETERS,
 ExpressionPtr nameExp, const std::string &name, bool hadBackslash,
 ExpressionListPtr params, ExpressionPtr classExp)
  : Expression(EXPRESSION_CONSTRUCTOR_BASE_PARAMETER_VALUES),
    StaticClassName(classExp), m_nameExp(nameExp),
    m_ciTemp(-1), m_params(params), m_valid(false),
    m_extraArg(0), m_variableArgument(false), m_voidReturn(false),
    m_voidWrapper(false), m_redeclared(false),
    m_noStatic(false), m_noInline(false), m_invokeFewArgsDecision(true),
    m_arrayParams(false), m_hadBackslash(hadBackslash),
    m_argArrayId(-1), m_argArrayHash(-1), m_argArrayIndex(-1) {

  if (m_nameExp &&
      m_nameExp->getKindOf() == Expression::KindOfScalarExpression) {
    assert(m_name.empty());
    ScalarExpressionPtr c = dynamic_pointer_cast<ScalarExpression>(m_nameExp);
    m_origName = c->getOriginalLiteralString();
    c->toLower(true /* func call*/);
    m_name = c->getLiteralString();
  } else {
    m_origName = name;
    m_name = Util::toLower(name);
  }
  m_clsNameTemp = -1;
}

void FunctionCall::reset() {
  m_valid = false;
  m_extraArg = 0;
  m_variableArgument = false;
  m_voidWrapper = false;
}

bool FunctionCall::isTemporary() const {
  return m_funcScope && !m_funcScope->isRefReturn();
}

void FunctionCall::deepCopy(FunctionCallPtr exp) {
  Expression::deepCopy(exp);
  exp->m_class = Clone(m_class);
  exp->m_params = Clone(m_params);
  exp->m_nameExp = Clone(m_nameExp);
}

bool FunctionCall::canInvokeFewArgs() {
  // We can always change out minds about saying yes, but once we say
  // no, it sticks.
  if (m_invokeFewArgsDecision &&
      ((m_params && m_params->getCount() > Option::InvokeFewArgsCount) ||
       m_arrayParams)) {
    m_invokeFewArgsDecision = false;
  }
  return m_invokeFewArgsDecision;
}

ConstructPtr FunctionCall::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_class;
    case 1:
      return m_nameExp;
    case 2:
      return m_params;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int FunctionCall::getKidCount() const {
  return 3;
}

void FunctionCall::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_class = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_nameExp = dynamic_pointer_cast<Expression>(cp);
      break;
    case 2:
      m_params = dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

void FunctionCall::markRefParams(FunctionScopePtr func,
                                 const std::string &name,
                                 bool canInvokeFewArgs) {
  ExpressionList &params = *m_params;
  if (func) {
    int mpc = func->getMaxParamCount();
    for (int i = params.getCount(); i--; ) {
      ExpressionPtr p = params[i];
      if (i < mpc ? func->isRefParam(i) :
          func->isReferenceVariableArgument()) {
        p->setContext(Expression::RefValue);
      } else if (i < mpc && p->hasContext(RefParameter)) {
        Symbol *sym = func->getVariables()->addSymbol(func->getParamName(i));
        sym->setLvalParam();
        sym->setCallTimeRef();
      }
    }
  } else if (Option::WholeProgram && !m_name.empty()) {
    FunctionScope::FunctionInfoPtr info =
      FunctionScope::GetFunctionInfo(m_name);
    if (info) {
      for (int i = params.getCount(); i--; ) {
        if (info->isRefParam(i)) {
          m_params->markParam(i, canInvokeFewArgs);
        }
      }
    }
    // If we cannot find information of the so-named function, it might not
    // exist, or it might go through __call(), either of which cannot have
    // reference parameters.
  } else {
    for (int i = params.getCount(); i--; ) {
      m_params->markParam(i, canInvokeFewArgs);
    }
  }
}

void FunctionCall::checkParamTypeCodeErrors(AnalysisResultPtr ar) {
  if (!m_funcScope || m_arrayParams) return;
  for (int i = 0, n = m_funcScope->getMaxParamCount(); i < n; ++i) {
    TypePtr specType = m_funcScope->getParamTypeSpec(i);
    if (!specType) continue;

    const char *fmt = 0;
    string ptype;
    if (!m_params || m_params->getCount() <= i) {
      if (i >= m_funcScope->getMinParamCount()) break;
      fmt = "parameter %d of %s() requires %s, none given";
    } else {
      ExpressionPtr param = (*m_params)[i];
      if (!Type::Inferred(ar, param->getType(), specType)) {
        fmt = "parameter %d of %s() requires %s, called with %s";
      }
      ptype = param->getType()->toString();
    }
    if (fmt) {
      string msg;
      Util::string_printf
        (msg, fmt,
         i + 1,
         Util::escapeStringForCPP(m_funcScope->getOriginalName()).c_str(),
         specType->toString().c_str(), ptype.c_str());
      Compiler::Error(Compiler::BadArgumentType,
                      shared_from_this(), msg);
    }
  }
}

void FunctionCall::analyzeProgram(AnalysisResultPtr ar) {
  if (m_class) m_class->analyzeProgram(ar);
  if (m_nameExp) m_nameExp->analyzeProgram(ar);
  if (m_params) m_params->analyzeProgram(ar);
  if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    checkParamTypeCodeErrors(ar);
  }
}

struct InlineCloneInfo {
  explicit InlineCloneInfo(FunctionScopePtr fs)
    : func(fs)
    , callWithThis(false)
  {}

  FunctionScopePtr              func;
  StringToExpressionPtrMap      sepm;
  ExpressionListPtr             elist;
  bool                          callWithThis;
  string                        localThis;
  string                        staticClass;
};

//typedef std::map<std::string, ExpressionPtr> StringToExpressionPtrMap;

static ExpressionPtr cloneForInlineRecur(InlineCloneInfo &info,
                                         ExpressionPtr exp,
                                         const std::string &prefix,
                                         AnalysisResultConstPtr ar,
                                         FunctionScopePtr scope) {
  exp->getOriginalScope(); // make sure to cache the original scope
  exp->setBlockScope(scope);
  for (int i = 0, n = exp->getKidCount(); i < n; i++) {
    if (ExpressionPtr k = exp->getNthExpr(i)) {
      exp->setNthKid(i, cloneForInlineRecur(info, k, prefix, ar, scope));
    }
  }
  StaticClassName *scn = dynamic_cast<StaticClassName*>(exp.get());
  if (scn && scn->isStatic() && !info.staticClass.empty()) {
    scn->resolveStatic(info.staticClass);
  }
  switch (exp->getKindOf()) {
  case Expression::KindOfSimpleVariable:
    {
      SimpleVariablePtr sv(dynamic_pointer_cast<SimpleVariable>(exp));
      if (sv->isSuperGlobal()) break;
      string name;
      if (sv->isThis()) {
        if (!info.callWithThis) {
          if (!sv->hasContext(Expression::ObjectContext)) {
            exp = sv->makeConstant(ar, "null");
          } else {
            // This will produce the wrong error
            // we really want a "throw_fatal" ast node.
            exp = sv->makeConstant(ar, "null");
          }
          break;
        }
        if (info.localThis.empty()) break;
        name = info.localThis;
      } else {
        name = prefix + sv->getName();
      }
      SimpleVariablePtr rep(new SimpleVariable(
                              exp->getScope(), exp->getLocation(), name));
      rep->copyContext(sv);
      rep->updateSymbol(SimpleVariablePtr());
      rep->getSymbol()->setHidden();
      // Conservatively set flags to prevent
      // the alias manager from getting confused.
      // On the next pass, it will correct the values,
      // and optimize appropriately.
      rep->getSymbol()->setUsed();
      rep->getSymbol()->setReferenced();
      if (exp->getContext() & (Expression::LValue|
                               Expression::RefValue|
                               Expression::RefParameter)) {
        info.sepm[name] = rep;
      }
      exp = rep;
    }
    break;
  case Expression::KindOfObjectMethodExpression:
  {
    FunctionCallPtr call(
      static_pointer_cast<FunctionCall>(exp));
    if (call->getFuncScope() == info.func) {
      call->setNoInline();
    }
    break;
  }
  case Expression::KindOfSimpleFunctionCall:
    {
      SimpleFunctionCallPtr call(static_pointer_cast<SimpleFunctionCall>(exp));
      call->addLateDependencies(ar);
      call->setLocalThis(info.localThis);
      if (call->getFuncScope() == info.func) {
        call->setNoInline();
      }
    }
  default:
    break;
  }
  return exp;
}

static ExpressionPtr cloneForInline(InlineCloneInfo &info,
                                    ExpressionPtr exp,
                                    const std::string &prefix,
                                    AnalysisResultConstPtr ar,
                                    FunctionScopePtr scope) {
  return cloneForInlineRecur(info, exp->clone(), prefix, ar, scope);
}

static int cloneStmtsForInline(InlineCloneInfo &info, StatementPtr s,
                               const std::string &prefix,
                               AnalysisResultConstPtr ar,
                               FunctionScopePtr scope) {
  switch (s->getKindOf()) {
  case Statement::KindOfStatementList:
    {
      for (int i = 0, n = s->getKidCount(); i < n; ++i) {
        if (int ret = cloneStmtsForInline(info, s->getNthStmt(i),
                                          prefix, ar, scope)) {
          return ret;
        }
      }
      return 0;
    }
  case Statement::KindOfExpStatement:
    info.elist->addElement(cloneForInline(
                             info, dynamic_pointer_cast<ExpStatement>(s)->
                             getExpression(), prefix, ar, scope));
    return 0;
  case Statement::KindOfReturnStatement:
    {
      ExpressionPtr exp =
        dynamic_pointer_cast<ReturnStatement>(s)->getRetExp();

      if (exp) {
        exp = cloneForInline(info, exp, prefix, ar, scope);
        if (exp->hasContext(Expression::RefValue)) {
          exp->clearContext(Expression::RefValue);
          if (exp->isRefable()) exp->setContext(Expression::LValue);
        }
        info.elist->addElement(exp);
        return 1;
      }
      return -1;
    }
  default:
    not_reached();
  }
  return 1;
}

ExpressionPtr FunctionCall::inliner(AnalysisResultConstPtr ar,
                                    ExpressionPtr obj, std::string localThis) {
  FunctionScopePtr fs = getFunctionScope();
  if (m_noInline || !fs || fs == m_funcScope || !m_funcScope->getStmt()) {
    return ExpressionPtr();
  }

  BlockScope::s_jobStateMutex.lock();
  if (m_funcScope->getMark() == BlockScope::MarkProcessing) {
    fs->setForceRerun(true);
    BlockScope::s_jobStateMutex.unlock();
    return ExpressionPtr();
  }
  ReadLock lock(m_funcScope->getInlineMutex());
  BlockScope::s_jobStateMutex.unlock();

  if (!m_funcScope->getInlineAsExpr()) {
    return ExpressionPtr();
  }

  if (m_funcScope->getInlineSameContext() &&
      m_funcScope->getContainingClass() &&
      m_funcScope->getContainingClass() != getClassScope()) {
    /*
      The function contains a context sensitive construct such as
      call_user_func (context sensitive because it could call
      array('parent', 'foo')) so its not safe to inline it
      into a different context.
    */
    return ExpressionPtr();
  }

  MethodStatementPtr m
    (dynamic_pointer_cast<MethodStatement>(m_funcScope->getStmt()));

  VariableTablePtr vt = fs->getVariables();
  int nAct = m_params ? m_params->getCount() : 0;
  int nMax = m_funcScope->getMaxParamCount();
  if (nAct < m_funcScope->getMinParamCount() || !m->getStmts()) {
    return ExpressionPtr();
  }

  InlineCloneInfo info(m_funcScope);
  info.elist = ExpressionListPtr(new ExpressionList(
                                   getScope(), getLocation(),
                                   ExpressionList::ListKindWrapped));
  std::ostringstream oss;
  oss << fs->nextInlineIndex() << "_" << m_name << "_";
  std::string prefix = oss.str();

  if (obj) {
    info.callWithThis = true;
    if (!obj->isThis()) {
      SimpleVariablePtr var
        (new SimpleVariable(getScope(),
                            obj->getLocation(),
                            prefix + "this"));
      var->updateSymbol(SimpleVariablePtr());
      var->getSymbol()->setHidden();
      var->getSymbol()->setUsed();
      var->getSymbol()->setReferenced();
      AssignmentExpressionPtr ae
        (new AssignmentExpression(getScope(),
                                  obj->getLocation(),
                                  var, obj, false));
      info.elist->addElement(ae);
      info.sepm[var->getName()] = var;
      info.localThis = var->getName();
    }
  } else {
    if (m_classScope) {
      if (!m_funcScope->isStatic()) {
        ClassScopeRawPtr oCls = getOriginalClass();
        FunctionScopeRawPtr oFunc = getOriginalFunction();
        if (oCls && !oFunc->isStatic() &&
            (oCls == m_classScope ||
             oCls->derivesFrom(ar, m_className, true, false))) {
          info.callWithThis = true;
          info.localThis = localThis;
        }
      }
      if (!isSelf() && !isParent() && !isStatic()) {
        info.staticClass = m_className;
      }
    }
  }

  ExpressionListPtr plist = m->getParams();

  int i;

  for (i = 0; i < nMax || i < nAct; i++) {
    ParameterExpressionPtr param
      (i < nMax ?
       dynamic_pointer_cast<ParameterExpression>((*plist)[i]) :
       ParameterExpressionPtr());
    ExpressionPtr arg = i < nAct ? (*m_params)[i] :
      Clone(param->defaultValue(), getScope());
    SimpleVariablePtr var
      (new SimpleVariable(getScope(),
                          (i < nAct ? arg.get() : this)->getLocation(),
                          prefix + (param ?
                                    param->getName() :
                                    lexical_cast<string>(i))));
    var->updateSymbol(SimpleVariablePtr());
    var->getSymbol()->setHidden();
    var->getSymbol()->setUsed();
    var->getSymbol()->setReferenced();
    bool ref =
      (i < nMax && m_funcScope->isRefParam(i)) ||
      arg->hasContext(RefParameter);
    arg->clearContext(RefParameter);
    AssignmentExpressionPtr ae
      (new AssignmentExpression(getScope(),
                                arg->getLocation(),
                                var, arg, ref));
    info.elist->addElement(ae);
    if (i < nAct && (ref || !arg->isScalar())) {
      info.sepm[var->getName()] = var;
    }
  }

  if (cloneStmtsForInline(info, m->getStmts(), prefix, ar,
                          getFunctionScope()) <= 0) {
    info.elist->addElement(makeConstant(ar, "null"));
  }

  if (info.sepm.size()) {
    ExpressionListPtr unset_list
      (new ExpressionList(getScope(), getLocation()));

    for (StringToExpressionPtrMap::iterator it = info.sepm.begin(),
           end = info.sepm.end(); it != end; ++it) {
      ExpressionPtr var = it->second->clone();
      var->clearContext((Context)(unsigned)-1);
      unset_list->addElement(var);
    }

    ExpressionPtr unset(
      new UnaryOpExpression(getScope(), getLocation(),
                            unset_list, T_UNSET, true));
    i = info.elist->getCount();
    ExpressionPtr ret = (*info.elist)[--i];
    if (ret->isScalar()) {
      info.elist->insertElement(unset, i);
    } else {
      ExpressionListPtr result_list
        (new ExpressionList(getScope(), getLocation(),
                            ExpressionList::ListKindLeft));
      if (ret->hasContext(LValue)) {
        result_list->setContext(LValue);
        result_list->setContext(ReturnContext);
      }
      result_list->addElement(ret);
      result_list->addElement(unset);
      (*info.elist)[i] = result_list;
    }
  }

  recomputeEffects();

  return replaceValue(info.elist);
}

ExpressionPtr FunctionCall::preOptimize(AnalysisResultConstPtr ar) {
  if (m_class) updateClassName();
  return ExpressionPtr();
}

ExpressionPtr FunctionCall::postOptimize(AnalysisResultConstPtr ar) {
  if (m_class) updateClassName();
  return ExpressionPtr();
}

///////////////////////////////////////////////////////////////////////////////

TypePtr FunctionCall::checkParamsAndReturn(AnalysisResultPtr ar,
                                           TypePtr type, bool coerce,
                                           FunctionScopePtr func,
                                           bool arrayParams) {
#ifdef HPHP_DETAILED_TYPE_INF_ASSERT
  assert(func->hasUser(getScope(), BlockScope::UseKindCaller));
#endif /* HPHP_DETAILED_TYPE_INF_ASSERT */
  ConstructPtr self = shared_from_this();
  TypePtr frt;
  {
    TRY_LOCK(func);
    func->getInferTypesMutex().assertOwnedBySelf();
    assert(!func->inVisitScopes() || getScope() == func);
    frt = func->getReturnType();
  }

  // fix return type for generators and async functions here, keep the
  // infered return type in function scope to allow further optimizations
  if (func->isGenerator()) {
    frt = Type::GetType(Type::KindOfObject, "Continuation");
  } else if (func->isAsync()) {
    frt = Type::GetType(Type::KindOfObject, "WaitHandle");
  }

  if (!frt) {
    m_voidReturn = true;
    setActualType(TypePtr());
    if (!isUnused() && !type->is(Type::KindOfAny)) {
      if (!hasContext(ReturnContext) &&
          !func->isFirstPass() && !func->isAbstract()) {
        if (Option::WholeProgram || !func->getContainingClass() ||
            func->isStatic() || func->isFinal() || func->isPrivate()) {
          Compiler::Error(Compiler::UseVoidReturn, self);
        }
      }
      if (!Type::IsMappedToVariant(type)) {
        setExpectedType(type);
      }
      m_voidWrapper = true;
    }
  } else {
    m_voidReturn = false;
    m_voidWrapper = false;
    type = checkTypesImpl(ar, type, frt, coerce);
    assert(m_actualType);
  }
  if (arrayParams) {
    m_extraArg = 0;
    (*m_params)[0]->inferAndCheck(ar, Type::Array, false);
  } else {
    m_extraArg = func->inferParamTypes(ar, self, m_params, m_valid);
  }
  m_variableArgument = func->isVariableArgument();
  if (m_valid) {
    m_implementedType.reset();
  } else {
    m_implementedType = Type::Variant;
  }
  assert(type);

  return type;
}

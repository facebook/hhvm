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

#include <compiler/expression/function_call.h>
#include <util/util.h>
#include <util/logger.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/statement/statement.h>
#include <compiler/statement/method_statement.h>
#include <compiler/statement/exp_statement.h>
#include <compiler/statement/return_statement.h>
#include <compiler/statement/statement_list.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/array_pair_expression.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/expression/simple_function_call.h>
#include <compiler/expression/parameter_expression.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/expression/unary_op_expression.h>
#include <util/parser/hphp.tab.hpp>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

FunctionCall::FunctionCall
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr nameExp, const std::string &name, ExpressionListPtr params,
 ExpressionPtr classExp)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    StaticClassName(classExp), m_nameExp(nameExp),
    m_ciTemp(-1), m_params(params), m_valid(false),
    m_extraArg(0), m_variableArgument(false), m_voidReturn(false),
    m_voidWrapper(false), m_allowVoidReturn(false), m_redeclared(false),
    m_noStatic(false), m_noInline(false), m_invokeFewArgsDecision(true),
    m_arrayParams(false),
    m_argArrayId(-1), m_argArrayHash(-1), m_argArrayIndex(-1) {

  if (m_nameExp &&
      m_nameExp->getKindOf() == Expression::KindOfScalarExpression) {
    ASSERT(m_name.empty());
    ScalarExpressionPtr c = dynamic_pointer_cast<ScalarExpression>(m_nameExp);
    m_origName = c->getString();
    c->toLower(true /* func call*/);
    m_name = c->getString();
    ASSERT(!m_name.empty());
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
      ASSERT(false);
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
      m_class = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_nameExp = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 2:
      m_params = boost::dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      ASSERT(false);
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
  } else if (!m_name.empty()) {
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

void FunctionCall::analyzeProgram(AnalysisResultPtr ar) {
  if (m_class) m_class->analyzeProgram(ar);
  if (m_nameExp) m_nameExp->analyzeProgram(ar);
  if (m_params) m_params->analyzeProgram(ar);
  if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    if (m_funcScope && !m_arrayParams) {
      for (int i = 0, n = m_funcScope->getMaxParamCount(); i < n; ++i) {
        if (TypePtr specType = m_funcScope->getParamTypeSpec(i)) {
          const char *fmt = 0;
          string ptype;
          if (!m_params || m_params->getCount() <= i) {
            if (i >= m_funcScope->getMinParamCount()) break;
            fmt = "%s: parameter %d of %s() requires %s, none given";
          } else {
            ExpressionPtr param = (*m_params)[i];
            if (!Type::Inferred(ar, param->getType(), specType)) {
              fmt = "%s: parameter %d of %s() requires %s, called with %s";
            }
            ptype = param->getType()->toString();
          }
          if (fmt) {
            string msg;
            Util::string_printf
              (msg, fmt,
               Util::escapeStringForCPP(
                 m_funcScope->getContainingFile()->getName()).c_str(),
               i + 1,
               Util::escapeStringForCPP(m_funcScope->getOriginalName()).c_str(),
               specType->toString().c_str(), ptype.c_str());
            Compiler::Error(Compiler::BadArgumentType,
                            shared_from_this(), msg);
          }
        }
      }
    }
    if (getContext() & RefValue) {
      FunctionScopePtr fs = getFunctionScope();
      if (fs) fs->setNeedsCheckMem();
    }
  }
}

struct InlineCloneInfo {
  InlineCloneInfo(FunctionScopePtr fs) : func(fs), callWithThis(false) {}

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
                              exp->getScope(), exp->getLocation(),
                              exp->getKindOf(), name));
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
    assert(false);
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
  if (unsigned(nAct - m_funcScope->getMinParamCount()) > (unsigned)nMax ||
      !m->getStmts()) {
    return ExpressionPtr();
  }

  InlineCloneInfo info(m_funcScope);
  info.elist = ExpressionListPtr(new ExpressionList(
                                   getScope(), getLocation(),
                                   KindOfExpressionList,
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
                            KindOfSimpleVariable,
                            prefix + "this"));
      var->updateSymbol(SimpleVariablePtr());
      var->getSymbol()->setHidden();
      var->getSymbol()->setUsed();
      var->getSymbol()->setReferenced();
      AssignmentExpressionPtr ae
        (new AssignmentExpression(getScope(),
                                  obj->getLocation(),
                                  KindOfAssignmentExpression,
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

  for (i = 0; i < nMax; i++) {
    ParameterExpressionPtr param
      (dynamic_pointer_cast<ParameterExpression>((*plist)[i]));
    ExpressionPtr arg = i < nAct ? (*m_params)[i] :
      param->defaultValue()->clone();
    SimpleVariablePtr var
      (new SimpleVariable(getScope(),
                          (i < nAct ? arg.get() : this)->getLocation(),
                          KindOfSimpleVariable,
                          prefix + param->getName()));
    var->updateSymbol(SimpleVariablePtr());
    var->getSymbol()->setHidden();
    var->getSymbol()->setUsed();
    var->getSymbol()->setReferenced();
    bool ref = m_funcScope->isRefParam(i);
    AssignmentExpressionPtr ae
      (new AssignmentExpression(getScope(),
                                arg->getLocation(), KindOfAssignmentExpression,
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
      (new ExpressionList(getScope(), getLocation(), KindOfExpressionList));

    for (StringToExpressionPtrMap::iterator it = info.sepm.begin(),
           end = info.sepm.end(); it != end; ++it) {
      ExpressionPtr var = it->second->clone();
      var->clearContext((Context)(unsigned)-1);
      unset_list->addElement(var);
    }

    ExpressionPtr unset(
      new UnaryOpExpression(getScope(), getLocation(), KindOfUnaryOpExpression,
                            unset_list, T_UNSET, true));
    i = info.elist->getCount();
    ExpressionPtr ret = (*info.elist)[--i];
    if (ret->isScalar()) {
      info.elist->insertElement(unset, i);
    } else {
      ExpressionListPtr result_list
        (new ExpressionList(getScope(), getLocation(), KindOfExpressionList,
                            ExpressionList::ListKindLeft));
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
  ConstructPtr self = shared_from_this();
  TypePtr frt = func->getReturnType();
  if (!frt) {
    m_voidReturn = true;
    setActualType(TypePtr());
    if (!type->is(Type::KindOfAny)) {
      if (!m_allowVoidReturn && !func->isFirstPass() && !func->isAbstract()) {
        Compiler::Error(Compiler::UseVoidReturn, self);
      }
      m_voidWrapper = true;
    }
  } else {
    m_voidReturn = false;
    m_voidWrapper = false;
    type = checkTypesImpl(ar, type, frt, coerce);
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

  return type;
}

bool FunctionCall::preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                                int state) {
  optimizeArgArray(ar);
  if (m_noStatic || isUnused() || m_className.empty() ||
      isSelf() || isParent()) {
    return Expression::preOutputCPP(cg, ar, state);
  }

  if (!cg.inExpression()) {
    return true;
  }
  Expression::preOutputCPP(cg, ar, state & ~FixOrder);
  cg.wrapExpressionBegin();
  if (m_classScope) {
    string className = m_classScope->getId(cg);
#ifdef ENABLE_LATE_STATIC_BINDING
    cg_printf("fi.setStaticClassName(%s%s::s_class_name);\n",
              Option::ClassPrefix, className.c_str());
#endif
  } else {
    m_clsNameTemp = cg.createNewLocalId(shared_from_this());
    cg_printf("CStrRef clsName%d(", m_clsNameTemp);
    cg_printString(m_origClassName, ar, shared_from_this());
    cg_printf(");\n");
#ifdef ENABLE_LATE_STATIC_BINDING
    cg_printf("fi.setStaticClassName(clsName%d);\n", m_clsNameTemp);
#endif
  }
  m_noStatic = true;
  preOutputStash(cg, ar, FixOrder);
  m_noStatic = false;
#ifdef ENABLE_LATE_STATIC_BINDING
  cg_printf("fi.resetStaticClassName();\n");
#endif

  if (!(state & FixOrder)) {
    cg_printf("id(%s);\n", cppTemp().c_str());
  }

  return true;
}

void FunctionCall::outputDynamicCall(CodeGenerator &cg,
                                     AnalysisResultPtr ar, bool method) {
  const char *kind = method ? "Meth" : "Func";
  const char *var = method ? "mcp" : "vt";

  int pcount = m_params ? m_params->getCount() : 0;
  if (canInvokeFewArgs()) {
    if (Option::InvokeWithSpecificArgs) {
      cg_printf("get%s%dArgs())(%s%d, ", kind, pcount, var, m_ciTemp);
    } else {
      cg_printf("get%sFewArgs())(%s%d, ", kind, var, m_ciTemp);
    }
    if (pcount) {
      for (int i = 0; i < pcount; i++) {
        (*m_params)[i]->setContext(NoRefWrapper);
      }
      cg_printf("%d, ", pcount);
      cg.pushCallInfo(m_ciTemp);
      FunctionScope::OutputCPPArguments(m_params, FunctionScopePtr(),
                                        cg, ar, 0, false);
      cg.popCallInfo();
    } else {
      cg_printf("0");
    }
    if (!Option::InvokeWithSpecificArgs) {
      for (int i = pcount; i < Option::InvokeFewArgsCount; i++) {
        cg_printf(", null_variant");
      }
    }
  } else {
    cg_printf("get%s())(%s%d, ", kind, var, m_ciTemp);

    if (pcount) {
      cg.pushCallInfo(m_ciTemp);
      FunctionScope::OutputCPPArguments(m_params, FunctionScopePtr(),
                                        cg, ar, -1, false);
      cg.popCallInfo();
    } else {
      cg_printf("Array()");
    }
  }

  cg_printf(")");
}

void FunctionCall::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  optimizeArgArray(ar);
  bool staticClassName = false;
  if (!m_noStatic && (m_class || !m_className.empty()) && m_cppTemp.empty() &&
      !isSelf() && !isParent() && !isStatic()) {
    if (!m_className.empty()) {
      cg_printf("STATIC_CLASS_NAME_CALL(");
      if (m_classScope) {
        string className = m_classScope->getId(cg);
        cg_printf("%s%s::s_class_name, ",
                  Option::ClassPrefix, className.c_str());
      } else {
        ASSERT(m_clsNameTemp >= 0);
        cg_printf("clsName%d, ", m_clsNameTemp);
      }
    } else {
      cg_printf("STATIC_CLASS_INVOKE_CALL(mcp%d.getClassName(), ",
          m_ciTemp);
    }
    if (m_voidReturn) m_voidWrapper = true;
    staticClassName = true;
  }

  if (m_voidReturn) clearContext(RefValue);
  bool wrap = m_voidWrapper && m_cppTemp.empty() && !isUnused();
  if (wrap) {
    cg_printf("(");
  }
  Expression::outputCPP(cg, ar);
  if (wrap) {
    cg_printf(", null)");
  }

  if (staticClassName) {
    cg_printf(")");
  }
}

void FunctionCall::setFunctionAndClassScope(FunctionScopePtr fsp,
                                            ClassScopePtr csp) {
  m_funcScope = fsp;
  m_classScope = csp;
}

void FunctionCall::optimizeArgArray(AnalysisResultPtr ar) {
  if (m_extraArg <= 0 || m_argArrayId >= 0) return;
  int paramCount = m_params->getOutputCount();
  int iMax = paramCount - m_extraArg;
  bool isScalar = true;
  for (int i = iMax; i < paramCount; i++) {
    ExpressionPtr param = (*m_params)[i];
    if (!param->isScalar()) {
      isScalar = false;
      break;
    }
  }
  if (isScalar) {
    ExpressionPtr argArrayPairs =
      ExpressionListPtr(new ExpressionList(getScope(), getLocation(),
                                           Expression::KindOfExpressionList));
    for (int i = iMax; i < paramCount; i++) {
      ExpressionPtr param = (*m_params)[i];
      argArrayPairs->addElement(
        ArrayPairExpressionPtr(new ArrayPairExpression(
                                 getScope(), param->getLocation(),
                                 Expression::KindOfArrayPairExpression,
                                 ExpressionPtr(), param, false)));
    }
    string text;
    m_argArrayId =
      ar->registerScalarArray(false, getFileScope(), argArrayPairs,
                              m_argArrayHash, m_argArrayIndex, text);
  }
}

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

#include <compiler/expression/simple_function_call.h>
#include <compiler/analysis/dependency_graph.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/code_error.h>
#include <compiler/expression/expression_list.h>
#include <compiler/statement/statement_list.h>
#include <compiler/statement/exp_statement.h>
#include <compiler/statement/return_statement.h>
#include <compiler/statement/method_statement.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/constant_expression.h>
#include <compiler/expression/parameter_expression.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/expression/unary_op_expression.h>
#include <compiler/analysis/constant_table.h>
#include <compiler/analysis/variable_table.h>
#include <util/util.h>
#include <compiler/option.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/parser/parser.h>
#include <util/parser/hphp.tab.hpp>
#include <runtime/base/complex_types.h>
#include <runtime/base/externals.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/string_util.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// statics

std::map<std::string, int> SimpleFunctionCall::FunctionTypeMap;

void SimpleFunctionCall::InitFunctionTypeMap() {
  if (FunctionTypeMap.empty()) {
    FunctionTypeMap["define"]               = DefineFunction;
    FunctionTypeMap["create_function"]      = CreateFunction;

    FunctionTypeMap["func_get_arg"]         = VariableArgumentFunction;
    FunctionTypeMap["func_get_args"]        = VariableArgumentFunction;
    FunctionTypeMap["func_num_args"]        = VariableArgumentFunction;

    FunctionTypeMap["extract"]              = ExtractFunction;
    FunctionTypeMap["compact"]              = CompactFunction;

    FunctionTypeMap["shell_exec"]           = ShellExecFunction;
    FunctionTypeMap["exec"]                 = ShellExecFunction;
    FunctionTypeMap["passthru"]             = ShellExecFunction;
    FunctionTypeMap["system"]               = ShellExecFunction;

    FunctionTypeMap["defined"]              = DefinedFunction;
    FunctionTypeMap["function_exists"]      = FunctionExistsFunction;
    FunctionTypeMap["class_exists"]         = ClassExistsFunction;
    FunctionTypeMap["interface_exists"]     = InterfaceExistsFunction;
    FunctionTypeMap["constant"]             = ConstantFunction;

    FunctionTypeMap["unserialize"]          = UnserializeFunction;
    FunctionTypeMap["apc_fetch"]            = UnserializeFunction;

    FunctionTypeMap["get_defined_vars"]     = GetDefinedVarsFunction;
  }
}

static class FunctionTypeMapInitializer {
public:
  FunctionTypeMapInitializer() {
    SimpleFunctionCall::InitFunctionTypeMap();
  }
} s_function_type_map_initializer;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

SimpleFunctionCall::SimpleFunctionCall
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 const std::string &name, ExpressionListPtr params, ExpressionPtr cls)
  : FunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES,
                 ExpressionPtr(), name, params, cls),
    m_type(UnknownType), m_dynamicConstant(false),
    m_parentClass(false), m_builtinFunction(false), m_noPrefix(false),
    m_invokeFewArgsDecision(true),
    m_dynamicInvoke(false), m_safe(0), m_arrayParams(false) {

  if (!m_class && m_className.empty()) {
    m_dynamicInvoke = Option::DynamicInvokeFunctions.find(m_name) !=
      Option::DynamicInvokeFunctions.end();
    map<string, int>::const_iterator iter =
      FunctionTypeMap.find(m_name);
    if (iter != FunctionTypeMap.end()) {
      m_type = iter->second;
    }
  }
}

ExpressionPtr SimpleFunctionCall::clone() {
  SimpleFunctionCallPtr exp(new SimpleFunctionCall(*this));
  deepCopy(exp);
  return exp;
}

void SimpleFunctionCall::deepCopy(SimpleFunctionCallPtr exp) {
  FunctionCall::deepCopy(exp);
  exp->m_safeDef = Clone(m_safeDef);
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void SimpleFunctionCall::onParse(AnalysisResultPtr ar, BlockScopePtr scope) {
  if (m_class) return;

  FileScopePtr fs = dynamic_pointer_cast<FileScope>(scope);
  assert(fs);

  ConstructPtr self = shared_from_this();
  if (m_className.empty()) {
    switch (m_type) {
    case CreateFunction:
      if (m_params->getCount() == 2 &&
          (*m_params)[0]->isLiteralString() &&
          (*m_params)[1]->isLiteralString()) {
        string params = (*m_params)[0]->getLiteralString();
        string body = (*m_params)[1]->getLiteralString();
        m_lambda = CodeGenerator::GetNewLambda();
        string code = "function " + m_lambda + "(" + params + ") "
          "{" + body + "}";
        ar->appendExtraCode(code);
      }
      break;
    case VariableArgumentFunction:
      /*
        Note:
        At this point, we dont have a function scope, so we set
        the flags on the FileScope.
        The FileScope maintains a stack of attributes, so that
        it correctly handles each function.
        But note that later phases should set/get the attribute
        directly on the FunctionScope, rather than on the FileScope
      */
      fs->setAttribute(FileScope::VariableArgument);
      break;
    case ExtractFunction:
      fs->setAttribute(FileScope::ContainsLDynamicVariable);
      fs->setAttribute(FileScope::ContainsExtract);
      break;
    case CompactFunction:
      fs->setAttribute(FileScope::ContainsDynamicVariable);
      fs->setAttribute(FileScope::ContainsCompact);
      break;
    case GetDefinedVarsFunction:
      fs->setAttribute(FileScope::ContainsDynamicVariable);
      fs->setAttribute(FileScope::ContainsGetDefinedVars);
      fs->setAttribute(FileScope::ContainsCompact);
      break;
    default:
      break;
    }
  }

  string call = getText();
  string name = m_name;
  if (!m_className.empty()) {
    name = m_className + "::" + name;
  }
  ar->getDependencyGraph()->add(DependencyGraph::KindOfFunctionCall, call,
                                shared_from_this(), name);
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void SimpleFunctionCall::addDependencies(AnalysisResultPtr ar) {
  if (!m_class) {
    if (m_className.empty()) {
      addUserFunction(ar, m_name);
    } else if (m_origClassName != "parent") {
      addUserClass(ar, m_className);
    } else {
      m_parentClass = true;
    }
  }
}

void SimpleFunctionCall::setupScopes(AnalysisResultPtr ar) {
  FunctionScopePtr func;
  ClassScopePtr cls;
  if (!m_class && m_className.empty()) {
    if (!m_dynamicInvoke) {
      func = ar->findFunction(m_name);
    }
  } else {
    cls = ar->resolveClass(shared_from_this(), m_className);
    if (cls && cls->isRedeclaring()) {
      cls = ar->findExactClass(shared_from_this(), m_className);
    }
    if (cls) {
      m_classScope = cls;
      if (m_name == "__construct") {
        func = cls->findConstructor(ar, true);
      } else {
        func = cls->findFunction(ar, m_name, true, true);
      }
    }
  }
  if (func && !func->isRedeclaring()) {
    if (m_funcScope != func) {
      m_funcScope = func;
      Construct::recomputeEffects();
      m_funcScope->addCaller(getScope());
    }
  }
}

void SimpleFunctionCall::addLateDependencies(AnalysisResultPtr ar) {
  AnalysisResult::Phase phase = ar->getPhase();
  ar->setPhase(AnalysisResult::AnalyzeAll);
  addDependencies(ar);
  ar->setPhase(phase);
  m_funcScope.reset();
  m_classScope.reset();
  setupScopes(ar);
}

ConstructPtr SimpleFunctionCall::getNthKid(int n) const {
  if (!n) return m_safeDef;
  return FunctionCall::getNthKid(n);
}

void SimpleFunctionCall::setNthKid(int n, ConstructPtr cp) {
  if (!n) {
    m_safeDef = boost::dynamic_pointer_cast<Expression>(cp);
  } else {
    FunctionCall::setNthKid(n, cp);
  }
}

void SimpleFunctionCall::analyzeProgram(AnalysisResultPtr ar) {
  if (m_class) {
    m_class->analyzeProgram(ar);
    setDynamicByIdentifier(ar, m_name);
  } else {
    addDependencies(ar);
  }

  if (m_safeDef) m_safeDef->analyzeProgram(ar);
  if (m_params) m_params->analyzeProgram(ar);

  if (ar->getPhase() == AnalysisResult::AnalyzeInclude) {
    onAnalyzeInclude(ar);

    ConstructPtr self = shared_from_this();

    // We need to know the name of the constant so that we can associate it
    // with this file before we do type inference.
    if (!m_class && m_className.empty() && m_type == DefineFunction &&
        m_params && m_params->getCount() >= 2) {
      ExpressionPtr ename = (*m_params)[0];
      if (ConstantExpressionPtr cname =
          dynamic_pointer_cast<ConstantExpression>(ename)) {

        ename = ExpressionPtr(
          new ScalarExpression(cname->getScope(), cname->getLocation(),
                               KindOfScalarExpression,
                               T_STRING, cname->getName(), true));
        m_params->removeElement(0);
        m_params->insertElement(ename);
      }
      ScalarExpressionPtr name =
        dynamic_pointer_cast<ScalarExpression>(ename);
      if (name) {
        string varName = name->getIdentifier();
        if (!varName.empty()) {
          getFileScope()->declareConstant(ar, varName);

          // handling define("CONSTANT", ...);
          ExpressionPtr value = (*m_params)[1];
          BlockScopePtr block = ar->findConstantDeclarer(varName);
          ConstantTablePtr constants = block->getConstants();
          if (constants != ar->getConstants()) {
            constants->add(varName, Type::Some, value, ar, self);
          }
        }
      }
    }

    if (m_type == UnserializeFunction) {
      ar->forceClassVariants(getOriginalScope(), false);
    }
  }

  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    // Look up the corresponding FunctionScope and ClassScope
    // for this function call
    {
      setupScopes(ar);
    }
    // check for dynamic constant and volatile function/class
    if (!m_class && m_className.empty() &&
      (m_type == DefinedFunction ||
       m_type == FunctionExistsFunction ||
       m_type == ClassExistsFunction ||
       m_type == InterfaceExistsFunction) &&
      m_params && m_params->getCount() >= 1) {
      ExpressionPtr value = (*m_params)[0];
      if (value->isScalar()) {
        ScalarExpressionPtr name =
          dynamic_pointer_cast<ScalarExpression>(value);
        if (name && name->isLiteralString()) {
          string symbol = name->getLiteralString();
          switch (m_type) {
          case DefinedFunction: {
            ConstantTablePtr constants = ar->getConstants();
            if (!constants->isPresent(symbol)) {
              // user constant
              BlockScopePtr block = ar->findConstantDeclarer(symbol);
              if (block) { // found the constant
                constants = block->getConstants();
                // set to be dynamic
                constants->setDynamic(ar, symbol);
              }
            }
            break;
          }
          case FunctionExistsFunction: {
            FunctionScopePtr func = ar->findFunction(Util::toLower(symbol));
            if (func && func->isUserFunction()) {
              func->setVolatile();
            }
            break;
          }
          case InterfaceExistsFunction:
          case ClassExistsFunction: {
            ClassScopePtr cls = ar->findClass(Util::toLower(symbol));
            if (cls && cls->isUserClass()) {
              cls->setVolatile();
            }
            break;
          }
          default:
            ASSERT(false);
          }
        }
      }
    }
  }

  if (m_params) {
    if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
      markRefParams(m_funcScope, m_name, canInvokeFewArgs());
    }
  }
}

bool SimpleFunctionCall::isDefineWithoutImpl(AnalysisResultPtr ar) {
  if (m_class || !m_className.empty()) return false;
  if (m_type == DefineFunction && m_params && m_params->getCount() >= 2) {
    if (m_dynamicConstant) return false;
    ScalarExpressionPtr name =
      dynamic_pointer_cast<ScalarExpression>((*m_params)[0]);
    if (!name) return false;
    string varName = name->getIdentifier();
    if (varName.empty()) return false;
    if (ar->getConstants()->isSystem(varName)) return true;
    ExpressionPtr value = (*m_params)[1];
    return (!ar->isConstantRedeclared(varName)) && value->isScalar();
  } else {
    return false;
  }
}

//typedef std::map<std::string, ExpressionPtr> StringToExpressionPtrMap;

static ExpressionPtr cloneForInlineRecur(ExpressionPtr exp,
                                         const std::string &prefix,
                                         StringToExpressionPtrMap &sepm,
                                         AnalysisResultPtr ar,
                                         FunctionScopePtr scope) {
  exp->getOriginalScope(); // make sure to cache the original scope
  exp->setBlockScope(scope);
  for (int i = 0, n = exp->getKidCount(); i < n; i++) {
    if (ExpressionPtr k = exp->getNthExpr(i)) {
      exp->setNthKid(i, cloneForInlineRecur(k, prefix, sepm, ar, scope));
    }
  }
  switch (exp->getKindOf()) {
  case Expression::KindOfSimpleVariable:
    {
      SimpleVariablePtr sv(dynamic_pointer_cast<SimpleVariable>(exp));
      if (!sv->isThis() && !sv->isSuperGlobal()) {
        string name = prefix + sv->getName();
        ExpressionPtr rep(new SimpleVariable(
                            exp->getScope(), exp->getLocation(),
                            exp->getKindOf(), name));
        rep->copyContext(exp);
        sepm[name] = rep;
        exp = rep;
      }
    }
    break;
  case Expression::KindOfSimpleFunctionCall:
    {
      static_pointer_cast<SimpleFunctionCall>(exp)->addLateDependencies(ar);
    }
  default:
    break;
  }
  return exp;
}

static ExpressionPtr cloneForInline(ExpressionPtr exp,
                                    const std::string &prefix,
                                    StringToExpressionPtrMap &sepm,
                                    AnalysisResultPtr ar,
                                    FunctionScopePtr scope) {
  return cloneForInlineRecur(exp->clone(), prefix, sepm, ar, scope);
}

static int cloneStmtsForInline(ExpressionListPtr elist, StatementPtr s,
                               const std::string &prefix,
                               StringToExpressionPtrMap &sepm,
                               AnalysisResultPtr ar,
                               FunctionScopePtr scope) {
  switch (s->getKindOf()) {
  case Statement::KindOfStatementList:
    {
      for (int i = 0, n = s->getKidCount(); i < n; ++i) {
        if (int ret = cloneStmtsForInline(elist, s->getNthStmt(i),
                                          prefix, sepm, ar, scope)) {
          return ret;
        }
      }
      return 0;
    }
  case Statement::KindOfExpStatement:
    elist->addElement(cloneForInline(dynamic_pointer_cast<ExpStatement>(s)->
                                     getExpression(), prefix, sepm, ar, scope));
    return 0;
  case Statement::KindOfReturnStatement:
    {
      ExpressionPtr exp =
        dynamic_pointer_cast<ReturnStatement>(s)->getRetExp();

      if (exp) {
        elist->addElement(cloneForInline(exp, prefix, sepm, ar, scope));
        return 1;
      }
      return -1;
    }
  default:
    assert(false);
  }
  return 1;
}

ExpressionPtr SimpleFunctionCall::optimize(AnalysisResultPtr ar) {
  if (m_class || !m_className.empty() || !m_funcScope) return ExpressionPtr();

  if (!m_funcScope->isUserFunction()) {
    if (m_type == UnknownType && m_funcScope->isFoldable()) {
      Array arr;
      if (m_params) {
        if (!m_params->isScalar()) return ExpressionPtr();
        for (int i = 0, n = m_params->getCount(); i < n; ++i) {
          Variant v;
          if (!(*m_params)[i]->getScalarValue(v)) return ExpressionPtr();
          arr.set(i, v);
        }
      }
      try {
        g_context->setThrowAllErrors(true);
        Variant v = invoke_builtin(m_funcScope->getName().c_str(),
                                   arr, -1, true);
        g_context->setThrowAllErrors(false);
        return makeScalarExpression(ar, v);
      } catch (...) {
        g_context->setThrowAllErrors(false);
      }
      return ExpressionPtr();
    }
    if (m_funcScope->getOptFunction()) {
      SimpleFunctionCallPtr self(
        static_pointer_cast<SimpleFunctionCall>(shared_from_this()));
      ExpressionPtr e = (m_funcScope->getOptFunction())(0, ar, self, 0);
      if (e) return e;
    }
  }

  if (m_type != UnknownType || m_safe ||
      !m_funcScope->getInlineAsExpr() ||
      !m_funcScope->getStmt() ||
      m_funcScope->getStmt()->getRecursiveCount() > 10) {
    return ExpressionPtr();
  }

  FunctionScopePtr fs = getFunctionScope();
  if (!fs || fs->inPseudoMain()) return ExpressionPtr();
  VariableTablePtr vt = fs->getVariables();
  int nAct = m_params ? m_params->getCount() : 0;
  int nMax = m_funcScope->getMaxParamCount();
  if (unsigned(nAct - m_funcScope->getMinParamCount()) > (unsigned)nMax ||
      vt->getAttribute(VariableTable::ContainsDynamicVariable) ||
      vt->getAttribute(VariableTable::ContainsExtract) ||
      vt->getAttribute(VariableTable::ContainsCompact) ||
      vt->getAttribute(VariableTable::ContainsGetDefinedVars)) {
    return ExpressionPtr();
  }

  ExpressionListPtr elist(new ExpressionList(getScope(), getLocation(),
                                             KindOfExpressionList,
                                             ExpressionList::ListKindWrapped));

  std::ostringstream oss;
  oss << "inl" << m_funcScope->nextInlineIndex() << "_" << m_name << "_";
  std::string prefix = oss.str();

  MethodStatementPtr m
    (dynamic_pointer_cast<MethodStatement>(m_funcScope->getStmt()));
  ExpressionListPtr plist = m->getParams();

  int i;
  StringToExpressionPtrMap sepm;

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
    bool ref = m_funcScope->isRefParam(i);
    AssignmentExpressionPtr ae
      (new AssignmentExpression(getScope(),
                                arg->getLocation(), KindOfAssignmentExpression,
                                var, arg, ref));
    elist->addElement(ae);
    if (i < nAct && (ref || !arg->isScalar())) {
      sepm[var->getName()] = var;
    }
  }

  if (cloneStmtsForInline(elist, m->getStmts(), prefix, sepm, ar,
                          getFunctionScope()) <= 0) {
    elist->addElement(CONSTANT("null"));
  }

  if (sepm.size()) {
    ExpressionListPtr unset_list
      (new ExpressionList(getScope(), getLocation(), KindOfExpressionList));

    for (StringToExpressionPtrMap::iterator it = sepm.begin(), end = sepm.end();
         it != end; ++it) {
      ExpressionPtr var = it->second->clone();
      var->clearContext((Context)(unsigned)-1);
      unset_list->addElement(var);
    }

    ExpressionPtr unset(
      new UnaryOpExpression(getScope(), getLocation(), KindOfUnaryOpExpression,
                            unset_list, T_UNSET, true));
    i = elist->getCount();
    ExpressionPtr ret = (*elist)[--i];
    if (ret->isScalar()) {
      elist->insertElement(unset, i);
    } else {
      ExpressionListPtr result_list
        (new ExpressionList(getScope(), getLocation(), KindOfExpressionList,
                            ExpressionList::ListKindLeft));
      result_list->addElement(ret);
      result_list->addElement(unset);
      (*elist)[i] = result_list;
    }
  }

  return replaceValue(elist);
}

ExpressionPtr SimpleFunctionCall::preOptimize(AnalysisResultPtr ar) {
  if (m_class) updateClassName();

  if (ar->getPhase() < AnalysisResult::FirstPreOptimize) {
    return ExpressionPtr();
  }

  if (ExpressionPtr rep = onPreOptimize(ar)) {
    return rep;
  }

  if (ExpressionPtr rep = optimize(ar)) {
    return rep;
  }

  if (!m_class && m_className.empty() &&
      (m_type == DefineFunction ||
       m_type == DefinedFunction ||
       m_type == FunctionExistsFunction ||
       m_type == ClassExistsFunction ||
       m_type == InterfaceExistsFunction) &&
      m_params &&
      m_params->getCount() == (m_type == DefineFunction) + 1) {
    ExpressionPtr value = (*m_params)[0];
    if (value->isScalar()) {
      ScalarExpressionPtr name = dynamic_pointer_cast<ScalarExpression>(value);
      if (name && name->isLiteralString()) {
        string symbol = name->getLiteralString();
        switch (m_type) {
        case DefineFunction: {
          ConstantTablePtr constants = ar->getConstants();
          // system constant
          if (constants->isPresent(symbol)) {
            break;
          }
          // user constant
          BlockScopePtr block = ar->findConstantDeclarer(symbol);
          // not found (i.e., undefined)
          if (!block) break;
          constants = block->getConstants();
          // already set to be dynamic
          Symbol *sym = constants->getSymbol(symbol);
          assert(sym);
          if (!sym->isDynamic() && sym->getValue() != (*m_params)[1]) {
            sym->setValue((*m_params)[1]);
            ar->incOptCounter();
          }
          break;
        }
        case DefinedFunction: {
          ConstantTablePtr constants = ar->getConstants();
          // system constant
          if (constants->isPresent(symbol) && !constants->isDynamic(symbol)) {
            return CONSTANT("true");
          }
          // user constant
          BlockScopePtr block = ar->findConstantDeclarer(symbol);
          // not found (i.e., undefined)
          if (!block) {
            if (symbol.find("::") == std::string::npos) {
              return CONSTANT("false");
            } else {
              // e.g., defined("self::ZERO")
              return ExpressionPtr();
            }
          }
          constants = block->getConstants();
          // already set to be dynamic
          if (constants->isDynamic(symbol)) return ExpressionPtr();
          ConstructPtr decl = constants->getValue(symbol);
          ExpressionPtr constValue = dynamic_pointer_cast<Expression>(decl);
          if (constValue->isScalar()) {
            return CONSTANT("true");
          } else {
            return ExpressionPtr();
          }
          break;
        }
        case FunctionExistsFunction: {
          const std::string &lname = Util::toLower(symbol);
          if (Option::DynamicInvokeFunctions.find(lname) ==
              Option::DynamicInvokeFunctions.end()) {
            FunctionScopePtr func = ar->findFunction(lname);
            if (!func) {
              return CONSTANT("false");
            }
            if (func->isUserFunction()) {
              func->setVolatile();
            }
            if (!func->isVolatile()) {
              return CONSTANT("true");
            }
          }
          break;
        }
        case InterfaceExistsFunction: {
          ClassScopePtrVec classes = ar->findClasses(Util::toLower(symbol));
          bool interfaceFound = false;
          for (ClassScopePtrVec::const_iterator it = classes.begin();
               it != classes.end(); ++it) {
            ClassScopePtr cls = *it;
            if (cls->isUserClass()) cls->setVolatile();
            if (cls->isInterface()) {
              interfaceFound = true;
            }
          }
          if (!interfaceFound) return CONSTANT("false");
          if (classes.size() == 1 && !classes.back()->isVolatile()) {
            return CONSTANT("true");
          }
          break;
        }
        case ClassExistsFunction: {
          ClassScopePtrVec classes = ar->findClasses(Util::toLower(symbol));
          bool classFound = false;
          for (ClassScopePtrVec::const_iterator it = classes.begin();
               it != classes.end(); ++it) {
            ClassScopePtr cls = *it;
            if (cls->isUserClass()) cls->setVolatile();
            if (!cls->isInterface()) {
              classFound = true;
            }
          }
          if (!classFound) return CONSTANT("false");
          if (classes.size() == 1 && !classes.back()->isVolatile()) {
            return CONSTANT("true");
          }
          break;
        }
        default:
          ASSERT(false);
        }
      }
    }
  }
  return ExpressionPtr();
}

ExpressionPtr SimpleFunctionCall::postOptimize(AnalysisResultPtr ar) {
  if (!Option::KeepStatementsWithNoEffect && isDefineWithoutImpl(ar)) {
    Construct::recomputeEffects();
    return CONSTANT("true");
  }
  /*
    Dont do this for now. Need to take account of newly created
    variables etc (which would normally be handled by inferTypes).

    if (ExpressionPtr rep = optimize(ar)) {
      return rep;
    }
  */
  return FunctionCall::postOptimize(ar);
}

int SimpleFunctionCall::getLocalEffects() const {
  if (m_class) return UnknownEffect;

  if (m_funcScope && !m_funcScope->hasEffect()) {
    return 0;
  }

  return UnknownEffect;
}

TypePtr SimpleFunctionCall::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                       bool coerce) {
  ASSERT(false);
  return TypePtr();
}

TypePtr SimpleFunctionCall::inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                          bool coerce) {
  reset();

  if (m_class) {
    m_class->inferAndCheck(ar, Type::Any, false);
  }

  if (m_safeDef) {
    m_safeDef->inferAndCheck(ar, Type::Any, false);
  }

  if (m_safe) {
    getScope()->getVariables()->
      setAttribute(VariableTable::NeedGlobalPointer);
  }

  ConstructPtr self = shared_from_this();

  // handling define("CONSTANT", ...);
  if (!m_class && m_className.empty()) {
    if (m_type == DefineFunction && m_params && m_params->getCount() >= 2) {
      ScalarExpressionPtr name =
        dynamic_pointer_cast<ScalarExpression>((*m_params)[0]);
      string varName;
      if (name) {
        varName = name->getIdentifier();
        if (!varName.empty()) {
          ExpressionPtr value = (*m_params)[1];
          TypePtr varType = value->inferAndCheck(ar, Type::Some, false);
          ar->getDependencyGraph()->
            addParent(DependencyGraph::KindOfConstant,
                      ar->getName(), varName, self);
          BlockScopePtr block = ar->findConstantDeclarer(varName);
          if (!block) {
            getFileScope()->declareConstant(ar, varName);
            block = ar->findConstantDeclarer(varName);
            ASSERT(block);
          }
          ConstantTablePtr constants = block->getConstants();
          if (constants != ar->getConstants()) {
            if (value && !value->isScalar()) {
              constants->setDynamic(ar, varName);
              varType = Type::Variant;
            }
            if (constants->isDynamic(varName)) {
              m_dynamicConstant = true;
              getScope()->getVariables()->
                setAttribute(VariableTable::NeedGlobalPointer);
            } else {
              constants->setType(ar, varName, varType, true);
            }
            // in case the old 'value' has been optimized
            constants->setValue(ar, varName, value);
          }
          return checkTypesImpl(ar, type, Type::Boolean, coerce);
        }
      }
      if (varName.empty() && ar->isFirstPass()) {
        Compiler::Error(Compiler::BadDefine, self);
      }
    } else if (m_type == ExtractFunction) {
      getScope()->getVariables()->forceVariants(ar, VariableTable::AnyVars);
    }
  }

  FunctionScopePtr func;

  // avoid raising both MissingObjectContext and UnknownFunction
  bool errorFlagged = false;

  if (!m_class && m_className.empty()) {
    if (!m_dynamicInvoke) {
      func = ar->findFunction(m_name);
    }
  } else {
    ClassScopePtr cls = ar->resolveClass(shared_from_this(), m_className);
    if (cls && cls->isVolatile()) {
      getScope()->getVariables()
        ->setAttribute(VariableTable::NeedGlobalPointer);

      if (cls->isRedeclaring()) {
        cls = ar->findExactClass(shared_from_this(), m_className);
        if (!cls) {
          m_redeclaredClass = true;
        }
      }
    }
    if (!cls) {
      if (ar->isFirstPass()) {
        Compiler::Error(Compiler::UnknownClass, self);
      }
      if (m_params) {
        m_params->inferAndCheck(ar, Type::Some, false);
        m_params->markParams(canInvokeFewArgs());
      }
      return checkTypesImpl(ar, type, Type::Variant, coerce);
    }
    m_classScope = cls;
    m_derivedFromRedeclaring = cls->derivesFromRedeclaring();
    m_validClass = true;

    if (m_name == "__construct") {
      // if the class is known, php will try to identify class-name ctor
      func = cls->findConstructor(ar, true);
    } else {
      func = cls->findFunction(ar, m_name, true, true);
    }

    if (func && !func->isStatic()) {
      ClassScopePtr clsThis = getClassScope();
      FunctionScopePtr funcThis = getFunctionScope();
      if (!clsThis ||
          (clsThis->getName() != m_className &&
           !clsThis->derivesFrom(ar, m_className, true, false)) ||
          funcThis->isStatic()) {
        func->setDynamic();
        if (ar->isFirstPass()) {
          Compiler::Error(Compiler::MissingObjectContext, self);
          errorFlagged = true;
        }
        func.reset();
      }
    }
  }
  if (!func || func->isRedeclaring()) {
    if (m_funcScope) {
      m_funcScope.reset();
      Construct::recomputeEffects();
    }
    if (func) {
      m_redeclared = true;
      getScope()->getVariables()->
        setAttribute(VariableTable::NeedGlobalPointer);
    }
    if (!func && !errorFlagged && ar->isFirstPass()) {
      Compiler::Error(Compiler::UnknownFunction, self);
    }
    if (m_params) {
      if (func) {
        FunctionScope::RefParamInfoPtr info =
          FunctionScope::GetRefParamInfo(m_name);
        ASSERT(info);
        for (int i = m_params->getCount(); i--; ) {
          if (info->isRefParam(i)) {
            m_params->markParam(i, canInvokeFewArgs());
          }
        }
      }
      if (m_arrayParams) {
        (*m_params)[0]->inferAndCheck(ar, Type::Array, false);
      } else {
        m_params->inferAndCheck(ar, Type::Some, false);
      }
    }
    return checkTypesImpl(ar, type, Type::Variant, coerce);
  } else if (func != m_funcScope) {
    m_funcScope = func;
    m_funcScope->addCaller(getScope());
    Construct::recomputeEffects();
  }

  m_builtinFunction = (!func->isUserFunction() || func->isSepExtension());

  beforeCheck(ar);

  m_valid = true;
  TypePtr rtype = checkParamsAndReturn(ar, type, coerce, func, m_arrayParams);

  if (m_arrayParams && func && !m_builtinFunction) func->setDirectInvoke();

  if (!m_valid && m_params) {
    m_params->markParams(canInvokeFewArgs());
  }

  if (m_safe) {
    TypePtr atype = getActualType();
    if (m_safe > 0 && !m_safeDef) {
      atype = Type::Array;
    } else if (!m_safeDef) {
      atype = Type::Variant;
    } else {
      TypePtr t = m_safeDef->getActualType();
      if (!t || !atype || !Type::SameType(t, atype)) {
        atype = Type::Variant;
      }
    }
    rtype = checkTypesImpl(ar, type, atype, coerce);
    m_voidReturn = m_voidWrapper = false;
  }

  return rtype;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void SimpleFunctionCall::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  outputLineMap(cg, ar);

  if (m_class || !m_className.empty()) {
    StaticClassName::outputPHP(cg, ar);
    cg_printf("::%s(", m_name.c_str());
  } else {

    if (cg.getOutput() == CodeGenerator::InlinedPHP ||
        cg.getOutput() == CodeGenerator::TrimmedPHP) {

      if (cg.getOutput() == CodeGenerator::TrimmedPHP &&
          cg.usingStream(CodeGenerator::PrimaryStream) &&
          Option::DynamicFunctionCalls.find(m_name) !=
          Option::DynamicFunctionCalls.end()) {
        int funcNamePos = Option::DynamicFunctionCalls[m_name];
        if (m_params && m_params->getCount() &&
            m_params->getCount() >= funcNamePos + 1) {
          if (funcNamePos == -1) funcNamePos = m_params->getCount() - 1;
          ExpressionPtr funcName = (*m_params)[funcNamePos];
          if (!funcName->is(Expression::KindOfScalarExpression)) {

            cg_printf("%s(", m_name.c_str());
            for (int i = 0; i < m_params->getCount(); i++) {
              if (i > 0) cg_printf(", ");
              if (i == funcNamePos) {
                cg_printf("%sdynamic_load(", Option::IdPrefix.c_str());
                funcName->outputPHP(cg, ar);
                cg_printf(")");
              } else {
                ExpressionPtr param = (*m_params)[i];
                if (param) param->outputPHP(cg, ar);
              }
            }
            cg_printf(")");
            return;
          }
        }
      }
      cg_printf("%s(", m_name.c_str());
    } else {
      cg_printf("%s(", m_name.c_str());
    }
  }

  if (m_params) m_params->outputPHP(cg, ar);
  cg_printf(")");
}

bool SimpleFunctionCall::preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                                      int state) {
  if (m_validClass && m_classScope && m_classScope->isRedeclaring() &&
      (!m_funcScope || !m_funcScope->isStatic())) {
    /*
      In this case, even though its redeclaring, we were able
      to figure out the actual ClassScope for cls. But for a non-static
      call in a method of class D derived from cls, we cant
      use the "cls$$n::func()" syntax, because, cls$$n wont actually
      be a base class of D (D will derive from DynamicObjectData).

      We should improve class derivation so that if D knows which cls
      its derived from, it should use it.

      And we should improve code gen here, so that we can use
      p_cls$$n(parent)->cls$$n::func(), rather than resorting to
      fully dynamic dispatch.
    */
    m_valid = false;
    m_redeclaredClass = true;
    m_validClass = false;
  }
  if (m_valid && (!m_arrayParams ||
        (!m_class && m_className.empty() && m_funcScope->isUserFunction())))
    return FunctionCall::preOutputCPP(cg, ar, state);
  // Short circuit out if inExpression() returns false
  if (!ar->inExpression()) return true;
  m_ciTemp = cg.createNewId(shared_from_this());
  bool needHash = true;
  string escapedName(cg.escapeLabel(m_origName));
  string escapedClass;
  ClassScopePtr cls;
  if (!m_className.empty()) {
    cls = ar->findClass(m_className);
    if (!m_safe && cls &&
        !ar->checkClassPresent(shared_from_this(), m_origClassName) &&
        cls->isVolatile()) {
      ClassScope::OutputVolatileCheck(cg, ar, getScope(),
                                      cls->getOriginalName(), false);
      cg_printf(";\n");
    }
    escapedClass = cg.escapeLabel(m_className);
  }
  ar->wrapExpressionBegin(cg);
  cg_printf("const CallInfo *cit%d = NULL;\n", m_ciTemp);
  if (!m_class && m_className.empty()) {
    cg_printf("void *vt%d = NULL;\n", m_ciTemp);
  } else {
    cg_printf("MethodCallPackage mcp%d;\n", m_ciTemp);
  }
  bool safeCheck = false;
  if (m_safe) {
    if (!m_className.empty()) {
      if ((!cls || cls->isVolatile()) &&
          !ar->checkClassPresent(shared_from_this(), m_origClassName)) {
        cg_indentBegin("if (");
        ClassScope::OutputVolatileCheck(cg, ar, getScope(), m_className, true);
        safeCheck = true;
      }
    } else if (!m_funcScope || m_funcScope->isVolatile()) {
      cg_indentBegin("if (");
      cg_printf("%s->FVF(%s)",
          cg.getGlobals(ar),
          cg.formatLabel(m_name).c_str());
      safeCheck = true;
    }
  }
  if (safeCheck) {
    cg_printf(") {\n");
  }
  if (!m_class && m_className.empty()) {
    if (m_redeclared && !m_dynamicInvoke) {
      needHash = false;
      cg_printf("cit%d = %s->GCI(%s);\n", m_ciTemp, cg.getGlobals(ar),
                cg.formatLabel(m_name).c_str());
      if (!safeCheck) {
        // If m_safe, check cit later, if null then yield null or safeDef
        cg_printf("if (!cit%d) invoke_failed(\"%s\", null_array, -1);\n",
                  m_ciTemp, escapedName.c_str());
      }
    } else {
      cg_printf("get_call_info_or_fail(cit%d, vt%d, ", m_ciTemp, m_ciTemp);
      cg_printString(m_name, ar, shared_from_this());
      cg_printf(");\n");
      needHash = false;
    }
  } else {
    const MethodSlot *ms = NULL;
    if (!m_name.empty()) {
      ConstructPtr self = shared_from_this();
      ms = ar->getOrAddMethodSlot(m_name, self);
    }
    if (safeCheck) {
      cg_printf("mcp%d.noFatal();\n", m_ciTemp);
    }
    ClassScopePtr cscope = getClassScope();
    // The call is happening in an instance method
    bool inObj = cscope && !getFunctionScope()->isStatic();
    // The call was like parent::
    bool parentCall = m_parentClass && inObj;
    string className;
    if (m_classScope) {
      if (m_redeclaredClass) {
        className = cg.formatLabel(m_classScope->getName());
      } else {
        className = m_classScope->getId(cg);
      }
    } else {
      className = cg.formatLabel(m_className);
      if (!m_className.empty() && m_cppTemp.empty() &&
          m_origClassName != "self" && m_origClassName != "parent" &&
          m_origClassName != "static") {
        // Create a temporary to hold the class name, in case it is not a
        // StaticString.
        m_clsNameTemp = cg.createNewId(shared_from_this());
        cg_printf("CStrRef clsName%d(", m_clsNameTemp);
        cg_printString(m_origClassName, ar, shared_from_this());
        cg_printf(");\n");
      }
    }

    if (cscope && cscope->derivesFromRedeclaring()) {
      // In a derived from redeclaring class
      cg_printf("mcp%d.dynamicNamedCall%s(\"%s\", \"%s\"", m_ciTemp,
                ms ? "WithIndex" : "",
                escapedClass.c_str(), escapedName.c_str());
    } else if (m_redeclaredClass) {
      if (parentCall) {
        cg_printf("mcp%d.methodCallEx(this, \"%s\");\n", m_ciTemp,
                  escapedName.c_str());
        cg_printf("parent->%sget_call_info%s(mcp%d", Option::ClassPrefix,
                  ms ? "_with_index" : "", m_ciTemp);
      } else {
        cg_printf("mcp%d.staticMethodCall(\"%s\", \"%s\");\n", m_ciTemp,
                  escapedClass.c_str(), escapedName.c_str());
        cg_printf("%s->%s%s->%sget_call_info%s(mcp%d",
                  cg.getGlobals(ar), Option::ClassStaticsObjectPrefix,
                  className.c_str(), Option::ObjectStaticPrefix,
                  ms ? "with_index" : "", m_ciTemp);
      }
    } else if (m_validClass) {
      // In an object, calling a superclass's method
      bool exCall = inObj && getClassScope()->derivesFrom(ar, m_className,
                                                          true, false);
      if (exCall) {
        cg_printf("mcp%d.methodCallEx(this, \"%s\");\n", m_ciTemp,
                  escapedName.c_str());
        cg_printf("%s%s::%sget_call_info%s(mcp%d",
                  Option::ClassPrefix, className.c_str(), Option::ObjectPrefix,
                  ms ? "_with_index" : "", m_ciTemp);
      } else {
        cg_printf("mcp%d.staticMethodCall(\"%s\", \"%s\");\n", m_ciTemp,
                  escapedClass.c_str(), escapedName.c_str());
        cg_printf("%s%s::%sget_call_info%s(mcp%d",
                  Option::ClassPrefix, className.c_str(),
                  Option::ObjectStaticPrefix,
                  ms ? "_with_index" : "", m_ciTemp);
      }
    } else {
      if (m_class) {
        needHash = false;
        bool lsb = false;
        if (m_class->is(KindOfScalarExpression)) {
          cg_printf("mcp%d.staticMethodCall(", m_ciTemp);
          ASSERT(strcasecmp(dynamic_pointer_cast<ScalarExpression>(m_class)->
                getString().c_str(), "static") == 0);
          cg_printf("\"static\"");
          lsb = true;
        } else {
          cg_printf("mcp%d.dynamicNamedCall%s(", m_ciTemp,
              ms ? "_with_index" : "");
          m_class->outputCPP(cg, ar);
        }
        cg_printf(", \"%s\");\n", escapedName.c_str());
        if (lsb) cg_printf("mcp%d.lateStaticBind(info);\n", m_ciTemp);
      } else {
        // Nonexistent method
        cg_printf("mcp%d.dynamicNamedCall%s(\"%s\", \"%s\"", m_ciTemp,
                  ms ? "_with_index" : "", escapedClass.c_str(),
                  escapedName.c_str());
      }
    }
    if (ms) {
      cg_printf(", %s", ms->runObjParam().c_str());
    }
  }
  if (needHash) {
    cg_printf(", 0x%016llXLL);\n", hash_string_i(m_name.data(), m_name.size()));
  }
  if (m_class || !m_className.empty()) {
    cg_printf("cit%d = mcp%d.ci;\n", m_ciTemp, m_ciTemp);
  }
  if (safeCheck) {
    cg_indentEnd("}\n");

  }

  if (m_class) m_class->preOutputCPP(cg, ar, state);
  if (m_safeDef) m_safeDef->preOutputCPP(cg, ar, state);

  if (m_params && m_params->getCount() > 0) {
    ar->pushCallInfo(m_ciTemp);
    m_params->preOutputCPP(cg, ar, state);
    ar->popCallInfo();
  }
  return true;
}

void SimpleFunctionCall::outputCPPParamOrderControlled(CodeGenerator &cg,
                                                       AnalysisResultPtr ar) {
  if (!m_class && m_className.empty()) {
    switch (m_type) {
    case ExtractFunction:
      cg_printf("extract(variables, ");
      FunctionScope::outputCPPArguments(m_params, cg, ar, 0, false);
      cg_printf(")");
      return;
    case CompactFunction:
      cg_printf("compact(variables, ");
      FunctionScope::outputCPPArguments(m_params, cg, ar, -1, true);
      cg_printf(")");
      return;
    default:
      break;
    }
  }
  bool volatileCheck = false;
  ClassScopePtr cls = m_classScope;

  TypePtr safeCast;
  if (m_safe) {
    if (!m_className.empty()) {
      if ((!cls || cls->isVolatile()) &&
          !ar->checkClassPresent(shared_from_this(), m_origClassName)) {
        if (m_valid) {
          cg_printf("(");
          ClassScope::OutputVolatileCheck(cg, ar, getScope(),
                                          m_className, true);
        }
        volatileCheck = true;
      }
    }  else if (!m_funcScope || m_funcScope->isVolatile()) {
      if (m_valid) {
        cg_printf("(%s->FVF(%s)",
            cg.getGlobals(ar),
            cg.formatLabel(m_name).c_str());
      }
      volatileCheck = true;
    }

    if (volatileCheck) {
      if (!m_valid) {
        // ci will be null if fn/cl not defined. Set in preoutput
        cg_printf("(cit%d", m_ciTemp);
      }
      if (isUnused()) {
        cg_printf(" && (");
      } else {
        cg_printf(" ? ");
      }
    }
    if (!isUnused()) {
      if (m_safe > 0 && !m_safeDef) {
        cg_printf("Array(ArrayInit(2, true).set(0, true).set(1, ");

        Array ret(Array::Create(0, false));
        ret.set(1, Variant());
      } else if (m_funcScope && m_funcScope->getReturnType() &&
                 !Type::SameType(m_funcScope->getReturnType(),
                                 getActualType())) {
        safeCast = getActualType();
        safeCast->outputCPPCast(cg, ar);
        cg_printf("(");
      }
      if (m_funcScope && !m_funcScope->getReturnType()) {
        cg_printf("(");
      }
    }
  } else if (!m_className.empty()) {
    if ((!cls || cls->isVolatile()) &&
        !ar->checkClassPresent(shared_from_this(), m_origClassName)) {
      volatileCheck = true;
      ClassScope::OutputVolatileCheckBegin(cg, ar, getScope(), m_origClassName);
    }
  }


  if (m_valid && !m_arrayParams) {
    if (!m_className.empty()) {
      assert(cls);
      cg_printf("%s%s::", Option::ClassPrefix, cls->getId(cg).c_str());
      std::string name = m_name == "__construct" ?
        cls->findConstructor(ar, true)->getName() :
        cg.formatLabel(m_name);

      cg_printf("%s%s(", Option::MethodPrefix, name.c_str());
    } else {
      int paramCount = m_params ? m_params->getCount() : 0;
      if (m_name == "get_class" && getClassScope() && paramCount == 0) {
        cg_printf("(");
        cg_printString(getClassScope()->getOriginalName(), ar,
                       shared_from_this());
      } else if (m_name == "get_parent_class" && getClassScope() &&
                 paramCount == 0) {
        const std::string parentClass = getClassScope()->getParent();
        cg_printf("(");
        if (!parentClass.empty()) {
          cg_printString(getClassScope()->getParent(), ar, shared_from_this());
        } else {
          cg_printf("false");
        }
      } else {
        if (m_noPrefix) {
          cg_printf("%s(", cg.formatLabel(m_name).c_str());
        } else {
          cg_printf("%s%s(",
                    m_builtinFunction ? Option::BuiltinFunctionPrefix :
                    Option::FunctionPrefix, m_funcScope->getId(cg).c_str());
        }
      }
    }
    FunctionScope::outputCPPArguments(m_params, cg, ar, m_extraArg,
                                      m_variableArgument, m_argArrayId,
                                      m_argArrayHash, m_argArrayIndex);
  } else {
    if (!m_class && m_className.empty()) {
      if (m_valid && m_funcScope->isUserFunction()) {
        cg_printf("HPHP::%s%s(NULL, ", Option::InvokePrefix,
            m_funcScope->getId(cg).c_str());
      } else if (canInvokeFewArgs() && !m_arrayParams) {
        cg_printf("(cit%d->getFuncFewArgs())(vt%d, ", m_ciTemp, m_ciTemp);
      } else {
        cg_printf("(cit%d->getFunc())(vt%d, ", m_ciTemp, m_ciTemp);
      }
    } else {
      if (canInvokeFewArgs() && !m_arrayParams) {
        cg_printf("(cit%d->getMethFewArgs())(mcp%d, ", m_ciTemp, m_ciTemp);
      } else {
        cg_printf("(cit%d->getMeth())(mcp%d, ", m_ciTemp, m_ciTemp);
      }
    }
    if (canInvokeFewArgs() && !m_arrayParams) {
      int left = Option::InvokeFewArgsCount;
      if (m_params && m_params->getCount()) {
        left -= m_params->getCount();
        cg_printf("%d, ", m_params->getCount());
        ar->pushCallInfo(m_ciTemp);
        FunctionScope::outputCPPArguments(m_params, cg, ar, 0, false);
        ar->popCallInfo();
      } else {
        cg_printf("0");
      }
      for (int i = 0; i < left; i++) {
        cg_printf(", null");
      }
    } else {
      if ((!m_params) || (m_params->getCount() == 0)) {
        cg_printf("Array()");
      } else {
        ar->pushCallInfo(m_ciTemp);
        FunctionScope::outputCPPArguments(m_params, cg, ar,
                                          m_arrayParams ? 0 : -1, false);
        ar->popCallInfo();
      }
    }
  }
  cg_printf(")");

  if (m_safe) {
    if (isUnused()) {
    } else {
      if (m_funcScope && !m_funcScope->getReturnType()) {
        cg_printf(", null)");
      }
      if (safeCast) {
        cg_printf(")");
      }
      if (m_safe > 0 && !m_safeDef) {
        cg_printf(").create())");
      }
    }
  }
  if (volatileCheck) {
    if (m_safe) {
      if (isUnused()) {
        cg_printf(", false)");
      } else {
        cg_printf(" : ");

        if (m_safeDef && m_safeDef->getActualType() &&
            !Type::SameType(m_safeDef->getActualType(), getActualType())) {
          safeCast = getActualType();
          safeCast->outputCPPCast(cg, ar);
          cg_printf("(");
        } else {
          safeCast.reset();
        }

        if (m_safeDef) {
          m_safeDef->outputCPP(cg, ar);
        } else {
          if (m_safe < 0) {
            cg_printf("null");
          } else {
            Array ret(Array::Create(0, false));
            ret.set(1, Variant());
            ExpressionPtr t = makeScalarExpression(ar, ret);
            t->outputCPP(cg, ar);
          }
        }
        if (safeCast) {
          cg_printf(")");
        }
      }
      cg_printf(")");
    } else {
      ClassScope::OutputVolatileCheckEnd(cg);
    }
  }
}

void SimpleFunctionCall::outputCPPImpl(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  if (!m_lambda.empty()) {
    cg_printf("\"%s\"", m_lambda.c_str());
    return;
  }

  if (!m_class && m_className.empty()) {
    if (m_type == DefineFunction && m_params && m_params->getCount() >= 2) {
      ScalarExpressionPtr name =
        dynamic_pointer_cast<ScalarExpression>((*m_params)[0]);
      string varName;
      ExpressionPtr value = (*m_params)[1];
      if (name) {
        varName = name->getIdentifier();
        if (varName.empty()) {
          cg_printf("throw_fatal(\"bad define\")");
        } else if (m_dynamicConstant) {
          cg_printf("g->declareConstant(\"%s\", g->%s%s, ",
                    varName.c_str(), Option::ConstantPrefix,
                    varName.c_str());
          value->outputCPP(cg, ar);
          cg_printf(")");
        } else {
          bool needAssignment = true;
          bool isSystem = ar->getConstants()->isSystem(varName);
          if (isSystem ||
              ((!ar->isConstantRedeclared(varName)) && value->isScalar())) {
            needAssignment = false;
          }
          if (needAssignment) {
            cg_printf("%s%s = ", Option::ConstantPrefix, varName.c_str());
            value->outputCPP(cg, ar);
          }
        }
      } else {
        bool close = false;
        if (value->hasEffect()) {
          cg_printf("(id(");
          value->outputCPP(cg, ar);
          cg_printf("),");
          close = true;
        }
        cg_printf("throw_fatal(\"bad define\")");
        if (close) cg_printf(")");
      }
      return;
    }
    if (m_name == "func_num_args") {
      cg_printf("num_args");
      return;
    }

    switch (m_type) {
    case VariableArgumentFunction:
      {
        FunctionScopePtr func = getFunctionScope();
        if (func) {
          cg_printf("%s(", m_name.c_str());
          func->outputCPPParamsCall(cg, ar, true);
          if (m_params) {
            cg_printf(",");
            m_params->outputCPP(cg, ar);
          }
          cg_printf(")");
          return;
        }
      }
      break;
    case FunctionExistsFunction:
    case ClassExistsFunction:
    case InterfaceExistsFunction:
      {
        bool literalString = false;
        string symbol;
        if (m_params && m_params->getCount() == 1) {
          ExpressionPtr value = (*m_params)[0];
          if (value->isScalar()) {
            ScalarExpressionPtr name =
              dynamic_pointer_cast<ScalarExpression>(value);
            if (name && name->isLiteralString()) {
              literalString = true;
              symbol = name->getLiteralString();
            }
          }
        }
        if (literalString) {
          const std::string &lname = Util::toLower(symbol);
          switch (m_type) {
          case FunctionExistsFunction:
            {
              bool dynInvoke = Option::DynamicInvokeFunctions.find(lname) !=
                Option::DynamicInvokeFunctions.end();
              if (!dynInvoke) {
                FunctionScopePtr func = ar->findFunction(lname);
                if (func) {
                  if (func->isVolatile()) {
                    cg_printf("%s->FVF(%s)",
                              cg.getGlobals(ar),
                              cg.formatLabel(lname).c_str());
                    break;
                  }
                  cg_printf("true");
                  break;
                } else {
                  cg_printf("false");
                  break;
                }
              }
              cg_printf("f_function_exists(");
              cg_printString(symbol, ar, shared_from_this());
              cg_printf(")");
            }
            break;
          case ClassExistsFunction:
          case InterfaceExistsFunction:
            {
              ClassScopePtrVec classes = ar->findClasses(Util::toLower(symbol));
              int found = 0;
              bool foundOther = false;
              for (ClassScopePtrVec::const_iterator it = classes.begin();
                   it != classes.end(); ++it) {
                ClassScopePtr cls = *it;
                if (cls->isInterface() == (m_type == InterfaceExistsFunction)) {
                  found += cls->isVolatile() ? 2 : 1;
                } else {
                  foundOther = true;
                }
              }

              if (!found) {
                cg_printf("false");
              } else if (!foundOther) {
                if (found == 1) {
                  cg_printf("true");
                } else {
                  if (m_type == ClassExistsFunction) {
                    cg_printf("checkClassExists(");
                  } else {
                    cg_printf("checkInterfaceExists(");
                  }
                  cg_printString(symbol, ar, shared_from_this());
                  cg_printf(", &%s->CDEC(%s), %s->FVF(__autoload), true)",
                            cg.getGlobals(ar),
                            cg.formatLabel(lname).c_str(),
                            cg.getGlobals(ar));
                }
              } else {
                if (m_type == ClassExistsFunction) {
                  cg_printf("f_class_exists(");
                } else {
                  cg_printf("f_interface_exists(");
                }
                cg_printString(symbol, ar, shared_from_this());
                cg_printf(")");
              }
            }
            break;
          default:
            break;
          }
          return;
        }
      }
      break;
    case GetDefinedVarsFunction:
      cg_printf("get_defined_vars(variables)");
      return;
    default:
      break;
    }
  }

  outputCPPParamOrderControlled(cg, ar);
}

bool SimpleFunctionCall::canInvokeFewArgs() {
  // We can always change out minds about saying yes, but once we say
  // no, it sticks.
  if (m_dynamicInvoke ||
      (m_invokeFewArgsDecision && m_params &&
       m_params->getCount() > Option::InvokeFewArgsCount)) {
    m_invokeFewArgsDecision = false;
  }
  return m_invokeFewArgsDecision;
}

SimpleFunctionCallPtr SimpleFunctionCall::getFunctionCallForCallUserFunc(
  AnalysisResultPtr ar, SimpleFunctionCallPtr call, bool testOnly,
  int firstParam, bool &error) {
  error = false;
  ExpressionListPtr params = call->getParams();
  if (params && params->getCount() >= firstParam) {
    ExpressionPtr p0 = (*params)[0];
    Variant v;
    if (p0->isScalar() && p0->getScalarValue(v)) {
      if (v.isString()) {
        Variant t = StringUtil::Explode(v.toString(), "::", 3);
        if (!t.isArray() || t.toArray().size() != 2) {
          std::string name = Util::toLower(v.toString().data());
          FunctionScopePtr func = ar->findFunction(name);
          if (!func || func->isDynamicInvoke()) {
            error = !func;
            return SimpleFunctionCallPtr();
          }
          if (func->isUserFunction()) func->setVolatile();
          ExpressionListPtr p2;
          if (testOnly) {
            p2 = ExpressionListPtr(
              new ExpressionList(call->getScope(), call->getLocation(),
                                 Expression::KindOfExpressionList));
            p2->addElement(call->makeScalarExpression(ar, v));
            name = "function_exists";
          } else {
            p2 = static_pointer_cast<ExpressionList>(params->clone());
            while (firstParam--) {
              p2->removeElement(0);
            }
          }
          SimpleFunctionCallPtr rep(
            NewSimpleFunctionCall(call->getScope(), call->getLocation(),
                                  Expression::KindOfSimpleFunctionCall,
                                  name, p2, ExpressionPtr()));
          return rep;
        }
        v = t;
      }
      if (v.isArray()) {
        Array arr = v.toArray();
        if (arr.size() != 2 || !arr.exists(0) || !arr.exists(1)) {
          error = true;
          return SimpleFunctionCallPtr();
        }
        Variant classname = arr.rvalAt(0LL);
        Variant methodname = arr.rvalAt(1LL);
        if (!methodname.isString()) {
          error = true;
          return SimpleFunctionCallPtr();
        }
        if (!classname.isString()) {
          return SimpleFunctionCallPtr();
        }
        std::string sclass = classname.toString().data();
        std::string smethod = Util::toLower(methodname.toString().data());

        ClassScopePtr cls = ar->findClass(sclass);
        if (!cls) {
          error = true;
          return SimpleFunctionCallPtr();
        }
        if (cls->isRedeclaring()) {
          cls = ar->findExactClass(call->shared_from_this(), sclass);
        } else if (!cls->isVolatile() && cls->isUserClass() &&
                   !ar->checkClassPresent(call->shared_from_this(), sclass)) {
          cls->setVolatile();
        }
        if (!cls) {
          return SimpleFunctionCallPtr();
        }

        size_t c = smethod.find("::");
        if (c != 0 && c != string::npos && c+2 < smethod.size()) {
          string name = smethod.substr(0, c);
          if (cls->getName() != name) {
            if (!cls->derivesFrom(ar, name, true, false)) {
              error = cls->derivesFromRedeclaring() == ClassScope::FromNormal;
              return SimpleFunctionCallPtr();
            }
          }
          smethod = smethod.substr(c+2);
        }

        FunctionScopePtr func = cls->findFunction(ar, smethod, true);
        if (!func) {
          error = true;
          return SimpleFunctionCallPtr();
        }
        if (func->isPrivate() ?
            (cls != call->getOriginalScope() ||
             !cls->findFunction(ar, smethod, false)) :
            (func->isProtected() &&
             (!call->getOriginalScope() ||
              !call->getOriginalScope()->derivesFrom(ar, sclass,
                                                     true, false)))) {
          error = true;
          return SimpleFunctionCallPtr();
        }
        ExpressionPtr cl(call->makeScalarExpression(ar, classname));
        ExpressionListPtr p2;
        if (testOnly) {
          p2 = ExpressionListPtr(
            new ExpressionList(call->getScope(), call->getLocation(),
                               Expression::KindOfExpressionList));
          p2->addElement(cl);
          cl.reset();
          smethod = "class_exists";
        } else {
          p2 = static_pointer_cast<ExpressionList>(params->clone());
          while (firstParam--) {
            p2->removeElement(0);
          }
        }
        SimpleFunctionCallPtr rep(
          NewSimpleFunctionCall(call->getScope(), call->getLocation(),
                                Expression::KindOfSimpleFunctionCall,
                                smethod, p2, cl));
        return rep;
      }
    }
  }
  return SimpleFunctionCallPtr();
}

namespace HPHP {

ExpressionPtr hphp_opt_call_user_func(CodeGenerator *cg,
                                      AnalysisResultPtr ar,
                                      SimpleFunctionCallPtr call, int mode) {
  bool error = false;
  if (!cg && !mode && !ar->isSystem()) {
    const std::string &name = call->getName();
    bool isArray = name == "call_user_func_array";
    if (name == "call_user_func" || isArray) {
      SimpleFunctionCallPtr rep(
        SimpleFunctionCall::getFunctionCallForCallUserFunc(ar, call, false,
                                                           1, error));
      if (error) {
        rep.reset();
      } else if (rep) {
        rep->setSafeCall(-1);
        rep->addLateDependencies(ar);
        if (isArray) rep->setArrayParams();
      }
      return rep;
    }
  }
  return ExpressionPtr();
}

ExpressionPtr hphp_opt_fb_call_user_func(CodeGenerator *cg,
                                         AnalysisResultPtr ar,
                                         SimpleFunctionCallPtr call, int mode) {
  bool error = false;
  if (!cg && !mode && !ar->isSystem()) {
    const std::string &name = call->getName();
    bool isArray = name == "fb_call_user_func_array_safe";
    bool safe_ret = name == "fb_call_user_func_safe_return";
    if (isArray || safe_ret || name == "fb_call_user_func_safe") {
      SimpleFunctionCallPtr rep(
        SimpleFunctionCall::getFunctionCallForCallUserFunc(
          ar, call, false, safe_ret ? 2 : 1, error));
      if (error) {
        if (safe_ret) {
          return (*call->getParams())[1];
        } else {
          Array ret(Array::Create(0, false));
          ret.set(1, Variant());
          return call->makeScalarExpression(ar, ret);
        }
      }
      if (rep) {
        if (isArray) rep->setArrayParams();
        rep->addLateDependencies(ar);
        rep->setSafeCall(1);
        if (safe_ret) {
          ExpressionPtr def = (*call->getParams())[1];
          rep->setSafeDefault(def);
        }
        return rep;
      }
    }
  }
  return ExpressionPtr();
}

ExpressionPtr hphp_opt_is_callable(CodeGenerator *cg,
                                   AnalysisResultPtr ar,
                                   SimpleFunctionCallPtr call, int mode) {
  if (!cg && !mode && !ar->isSystem()) {
    bool error = false;
    SimpleFunctionCallPtr rep(
      SimpleFunctionCall::getFunctionCallForCallUserFunc(
        ar, call, true, 1, error));
    if (error) {
      return call->makeConstant(ar, "false");
    }
    return rep;
  }

  return ExpressionPtr();
}

}


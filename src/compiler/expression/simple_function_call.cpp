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

#include <compiler/expression/simple_function_call.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/code_error.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/constant_expression.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/expression/array_pair_expression.h>
#include <compiler/expression/array_element_expression.h>
#include <compiler/expression/unary_op_expression.h>
#include <compiler/analysis/constant_table.h>
#include <compiler/analysis/variable_table.h>
#include <util/util.h>
#include <compiler/option.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/parser/parser.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/externals.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/string_util.h>
#include <runtime/ext/ext_variable.h>

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

    FunctionTypeMap["fb_call_user_func_safe"] = FBCallUserFuncSafeFunction;
    FunctionTypeMap["fb_call_user_func_array_safe"] =
      FBCallUserFuncSafeFunction;
    FunctionTypeMap["fb_call_user_func_safe_return"] =
      FBCallUserFuncSafeFunction;
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
    m_builtinFunction(false), m_noPrefix(false), m_dynamicInvoke(false),
    m_safe(0), m_extra(NULL) {

  if (Option::ParseTimeOpts && !m_class && m_className.empty()) {
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

void SimpleFunctionCall::onParse(AnalysisResultConstPtr ar, FileScopePtr fs) {
  if (m_class) return;

  ConstructPtr self = shared_from_this();
  if (m_className.empty()) {
    switch (m_type) {
      case DefineFunction:
        if (m_params && unsigned(m_params->getCount() - 2) <= 1u) {
          // need to register the constant before AnalyzeAll, so that
          // DefinedFunction can mark this volatile
          ExpressionPtr ename = (*m_params)[0];
          if (ConstantExpressionPtr cname =
              dynamic_pointer_cast<ConstantExpression>(ename)) {
            /*
              Hack: If the name of the constant being defined is itself
              a constant expression, assume that its not yet defined.
              So define(FOO, 'bar') is equivalent to define('FOO', 'bar').
            */
            ename = makeScalarExpression(ar, cname->getName());
            m_params->removeElement(0);
            m_params->insertElement(ename);
          }
          ScalarExpressionPtr name =
            dynamic_pointer_cast<ScalarExpression>(ename);
          if (name) {
            string varName = name->getIdentifier();
            if (varName.empty()) break;
            AnalysisResult::Locker lock(ar);
            fs->declareConstant(lock.get(), varName);
            // handling define("CONSTANT", ...);
            ExpressionPtr value = (*m_params)[1];
            BlockScopePtr block = lock->findConstantDeclarer(varName);
            ConstantTablePtr constants = block->getConstants();
            if (constants != ar->getConstants()) {
              constants->add(varName, Type::Some, value, ar, self);
            }
          }
        }
        break;
      case CreateFunction:
        if (m_params->getCount() == 2 &&
            (*m_params)[0]->isLiteralString() &&
            (*m_params)[1]->isLiteralString()) {
          string params = (*m_params)[0]->getLiteralString();
          string body = (*m_params)[1]->getLiteralString();
          m_lambda = CodeGenerator::GetNewLambda();
          string code = "function " + m_lambda + "(" + params + ") "
            "{" + body + "}";
          m_lambda = "1_" + m_lambda;
          ar->appendExtraCode(fs->getName(), code);
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
      case CompactFunction: {
        // If all the parameters in the compact() call are statically known,
        // there is no need to create a variable table.
        vector<ExpressionPtr> literals;
        if (m_params->flattenLiteralStrings(literals)) {
          m_type = StaticCompactFunction;
          m_params->clearElements();
          for (unsigned i = 0; i < literals.size(); i++) {
            m_params->addElement(literals[i]);
          }
        } else {
          fs->setAttribute(FileScope::ContainsDynamicVariable);
        }
        fs->setAttribute(FileScope::ContainsCompact);
        break;
      }
      case GetDefinedVarsFunction:
        fs->setAttribute(FileScope::ContainsDynamicVariable);
        fs->setAttribute(FileScope::ContainsGetDefinedVars);
        fs->setAttribute(FileScope::ContainsCompact);
        break;
      default:
        break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void SimpleFunctionCall::addDependencies(AnalysisResultPtr ar) {
  if (!m_class) {
    if (m_className.empty()) {
      addUserFunction(ar, m_name);
    } else if ((!isParent() && !isSelf()) ||
               getOriginalScope() != getScope()) {
      addUserClass(ar, m_className);
    }
  }
}

void SimpleFunctionCall::setupScopes(AnalysisResultConstPtr ar) {
  FunctionScopePtr func;
  if (!m_class && m_className.empty()) {
    if (!m_dynamicInvoke) {
      bool namespaced = (m_name[0] == '\\');
      if (namespaced) {
        m_name = m_name.substr(1);
      }
      func = ar->findFunction(m_name);
      if (!func && namespaced) {
        int pos = m_name.rfind('\\');
        m_name = m_name.substr(pos + 1);
        func = ar->findFunction(m_name);
      }
    }
  } else {
    ClassScopePtr cls = resolveClass();
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

void SimpleFunctionCall::addLateDependencies(AnalysisResultConstPtr ar) {
  m_funcScope.reset();
  m_classScope.reset();
  setupScopes(ar);
}

ConstructPtr SimpleFunctionCall::getNthKid(int n) const {
  if (n == 1) return m_safeDef;
  return FunctionCall::getNthKid(n);
}

void SimpleFunctionCall::setNthKid(int n, ConstructPtr cp) {
  if (n == 1) {
    m_safeDef = boost::dynamic_pointer_cast<Expression>(cp);
  } else {
    FunctionCall::setNthKid(n, cp);
  }
}

void SimpleFunctionCall::analyzeProgram(AnalysisResultPtr ar) {
  FunctionCall::analyzeProgram(ar);
  if (m_class) {
    setDynamicByIdentifier(ar, m_name);
  } else if (ar->getPhase() >= AnalysisResult::AnalyzeAll) {
    addDependencies(ar);
  }

  if (m_safeDef) m_safeDef->analyzeProgram(ar);

  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    ConstructPtr self = shared_from_this();
    // Look up the corresponding FunctionScope and ClassScope
    // for this function call
    setupScopes(ar);
    if (m_funcScope && m_funcScope->getOptFunction()) {
      SimpleFunctionCallPtr self(
        static_pointer_cast<SimpleFunctionCall>(shared_from_this()));
      (m_funcScope->getOptFunction())(0, ar, self, 1);
    }

    // check for dynamic constant and volatile function/class
    if (!m_class && m_className.empty() &&
        (m_type == DefinedFunction ||
         m_type == FunctionExistsFunction ||
         m_type == FBCallUserFuncSafeFunction ||
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
                  if (m_type == DefinedFunction) {
                    constants->setDynamic(ar, symbol);
                  }
                }
              }
              break;
            }
            case FBCallUserFuncSafeFunction:
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
      } else if ((m_type == InterfaceExistsFunction ||
                  m_type == ClassExistsFunction) &&
                 value->is(KindOfSimpleVariable)) {
        SimpleVariablePtr name = dynamic_pointer_cast<SimpleVariable>(value);
        if (name && name->getSymbol()) {
          // name is checked as class name
          name->getSymbol()->setClassName();
        }
      }
    }

    if (m_type == StaticCompactFunction) {
      FunctionScopePtr fs = getFunctionScope();
      VariableTablePtr vt = fs->getVariables();
      if (vt->isPseudoMainTable() ||
          vt->getAttribute(VariableTable::ContainsDynamicVariable)) {
        // When there is a variable table already, we will keep the ordinary
        // compact() call.
        m_type = CompactFunction;
      } else {
        // compact('a', 'b', 'c') becomes compact('a', $a, 'b', $b, 'c', $c),
        // but only for variables that exist in the variable table.
        vector<ExpressionPtr> new_params;
        vector<string> strs;
        for (int i = 0; i < m_params->getCount(); i++) {
          ExpressionPtr e = (*m_params)[i];
          assert(e->isLiteralString());
          string name = e->getLiteralString();

          // no need to record duplicate names
          bool found = false;
          for (unsigned j = 0; j < strs.size(); j++) {
            if (strcasecmp(name.data(), strs[j].data()) == 0) {
              found = true;
              break;
            }
          }
          if (found) continue;
          strs.push_back(name);

          if (vt->getSymbol(name)) {
            SimpleVariablePtr var(new SimpleVariable(
                                    e->getScope(), e->getLocation(),
                                    KindOfSimpleVariable, name));
            var->copyContext(e);
            var->updateSymbol(SimpleVariablePtr());
            new_params.push_back(e);
            new_params.push_back(var);
          }
        }
        m_params->clearElements();
        for (unsigned i = 0; i < new_params.size(); i++) {
          m_params->addElement(new_params[i]);
        }
      }
    }

    if (m_type == UnserializeFunction) {
      ar->forceClassVariants(getOriginalClass(), false);
    }
    if (m_params) {
      markRefParams(m_funcScope, m_name, canInvokeFewArgs());
    }
  } else if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    if (!m_class && !m_redeclared && !m_dynamicInvoke && !m_funcScope &&
        (m_className.empty() ||
         (m_classScope &&
          !m_classScope->derivesFromRedeclaring() &&
          !m_classScope->getAttribute(
            ClassScope::HasUnknownStaticMethodHandler) &&
          !m_classScope->getAttribute(
            ClassScope::InheritsUnknownStaticMethodHandler)))) {
      Compiler::Error(Compiler::UnknownFunction, shared_from_this());
    }
  }
}

bool SimpleFunctionCall::readsLocals() const {
  return m_type == GetDefinedVarsFunction ||
    m_type == CompactFunction;
}

bool SimpleFunctionCall::writesLocals() const {
  return m_type == ExtractFunction;
}

void SimpleFunctionCall::updateVtFlags() {
  FunctionScopeRawPtr f = getFunctionScope();
  if (f) {
    if (m_funcScope) {
      if (m_funcScope->getContextSensitive()) {
        f->setInlineSameContext(true);
      }
      if ((m_classScope && (isSelf() || isParent()) &&
           m_funcScope->usesLSB()) ||
          isStatic() ||
          m_type == FBCallUserFuncSafeFunction ||
          m_name == "call_user_func" ||
          m_name == "call_user_func_array" ||
          m_name == "forward_static_call" ||
          m_name == "forward_static_call_array" ||
          m_name == "get_called_class") {
        f->setNextLSB(true);
      }
    }
  }
  if (m_type != UnknownType) {
    VariableTablePtr vt = getScope()->getVariables();
    switch (m_type) {
      case ExtractFunction:
        vt->setAttribute(VariableTable::ContainsLDynamicVariable);
        vt->setAttribute(VariableTable::ContainsExtract);
        break;
      case CompactFunction:
        vt->setAttribute(VariableTable::ContainsDynamicVariable);
      case StaticCompactFunction:
        vt->setAttribute(VariableTable::ContainsCompact);
        break;
      case GetDefinedVarsFunction:
        vt->setAttribute(VariableTable::ContainsDynamicVariable);
        vt->setAttribute(VariableTable::ContainsGetDefinedVars);
        vt->setAttribute(VariableTable::ContainsCompact);
        break;
    }
  }
}

bool SimpleFunctionCall::isDefineWithoutImpl(AnalysisResultConstPtr ar) {
  if (m_class || !m_className.empty()) return false;
  if (m_type == DefineFunction && m_params &&
      unsigned(m_params->getCount() - 2) <= 1u) {
    if (m_dynamicConstant) return false;
    ScalarExpressionPtr name =
      dynamic_pointer_cast<ScalarExpression>((*m_params)[0]);
    if (!name) return false;
    string varName = name->getIdentifier();
    if (varName.empty()) return false;
    if (ar->isSystemConstant(varName)) {
      assert(!m_extra);
      return true;
    }
    ExpressionPtr value = (*m_params)[1];
    return (!ar->isConstantRedeclared(varName)) && value->isScalar();
  } else {
    return false;
  }
}

ExpressionPtr SimpleFunctionCall::optimize(AnalysisResultConstPtr ar) {
  if (m_class || !m_funcScope ||
      (!m_className.empty() && (!m_classScope || !isPresent()))) {
    return ExpressionPtr();
  }

  if (!m_funcScope->isUserFunction()) {
    if (m_type == ExtractFunction && m_params && m_params->getCount() >= 1) {
      ExpressionPtr vars = (*m_params)[0];
      while (vars) {
        if (vars->is(KindOfUnaryOpExpression) &&
            static_pointer_cast<UnaryOpExpression>(vars)->getOp() == T_ARRAY) {
          break;
        }
        if (vars->is(KindOfExpressionList)) {
          vars = static_pointer_cast<ExpressionList>(vars)->listValue();
        } else {
          vars = vars->getCanonPtr();
        }
      }
      if (vars) {
        bool svar = vars->isScalar();
        if (!svar && getScope()->getUpdated()) {
          /*
           * kind of a hack. If the extract param is non-scalar,
           * and we've made changes already, dont try to optimize yet.
           * this gives us a better chance of getting a scalar result.
           * Later, we should add more array optimizations, which would
           * allow us to optimize the generated code once the scalar
           * expressions are resolved
           */
          return ExpressionPtr();
        }
        int n = m_params->getCount();
        String prefix;
        int mode = EXTR_OVERWRITE;
        if (n >= 2) {
          Variant v;
          ExpressionPtr m = (*m_params)[1];
          if (m->isScalar() && m->getScalarValue(v)) {
            mode = v.toInt64();
          }
          if (n >= 3) {
            ExpressionPtr p = (*m_params)[2];
            if (p->isScalar() && p->getScalarValue(v)) {
              prefix = v.toString();
            }
          }
        }
        bool ref = mode & EXTR_REFS;
        mode &= ~EXTR_REFS;
        switch (mode) {
          case EXTR_PREFIX_ALL:
          case EXTR_PREFIX_INVALID:
          case EXTR_OVERWRITE: {
            ExpressionListPtr arr(
              static_pointer_cast<ExpressionList>(
                static_pointer_cast<UnaryOpExpression>(vars)->getExpression()));
            ExpressionListPtr rep(
              new ExpressionList(getScope(), getLocation(),
                                 Expression::KindOfExpressionList,
                                 ExpressionList::ListKindWrapped));
            string root_name;
            int n = arr ? arr->getCount() : 0;
            int i, j, k;
            for (i = j = k = 0; i < n; i++) {
              ArrayPairExpressionPtr ap(
                dynamic_pointer_cast<ArrayPairExpression>((*arr)[i]));
              assert(ap);
              String name;
              Variant voff;
              if (!ap->getName()) {
                voff = j++;
              } else {
                if (!ap->getName()->isScalar() ||
                    !ap->getName()->getScalarValue(voff)) {
                  return ExpressionPtr();
                }
              }
              name = voff.toString();
              if (mode == EXTR_PREFIX_ALL ||
                  (mode == EXTR_PREFIX_INVALID &&
                   !name.isValidVariableName())) {
                name = prefix + "_" + name;
              }
              if (!name.isValidVariableName()) continue;
              SimpleVariablePtr var(
                new SimpleVariable(getScope(), getLocation(),
                                   KindOfSimpleVariable, name.data()));
              var->updateSymbol(SimpleVariablePtr());
              ExpressionPtr val(ap->getValue());
              if (!val->isScalar()) {
                if (root_name.empty()) {
                  root_name = "t" + lexical_cast<string>(
                    getFunctionScope()->nextInlineIndex());
                  SimpleVariablePtr rv(
                    new SimpleVariable(getScope(), getLocation(),
                                       KindOfSimpleVariable, root_name));
                  rv->updateSymbol(SimpleVariablePtr());
                  rv->getSymbol()->setHidden();
                  ExpressionPtr root(
                    new AssignmentExpression(getScope(), getLocation(),
                                             KindOfAssignmentExpression,
                                             rv, (*m_params)[0], false));
                  rep->insertElement(root);
                }

                SimpleVariablePtr rv(
                  new SimpleVariable(getScope(), getLocation(),
                                     KindOfSimpleVariable, root_name));
                rv->updateSymbol(SimpleVariablePtr());
                rv->getSymbol()->setHidden();
                ExpressionPtr offset(makeScalarExpression(ar, voff));
                val = ExpressionPtr(
                  new ArrayElementExpression(getScope(), getLocation(),
                                             KindOfArrayElementExpression,
                                             rv, offset));
              }
              ExpressionPtr a(
                new AssignmentExpression(getScope(), getLocation(),
                                         KindOfAssignmentExpression,
                                         var, val, ref));
              rep->addElement(a);
              k++;
            }
            if (root_name.empty()) {
              if ((*m_params)[0]->hasEffect()) {
                rep->insertElement((*m_params)[0]);
              }
            } else {
              ExpressionListPtr unset_list
                (new ExpressionList(getScope(), getLocation(),
                                    KindOfExpressionList));

              SimpleVariablePtr rv(
                new SimpleVariable(getScope(), getLocation(),
                                   KindOfSimpleVariable, root_name));
              rv->updateSymbol(SimpleVariablePtr());
              unset_list->addElement(rv);

              ExpressionPtr unset(
                new UnaryOpExpression(getScope(), getLocation(),
                                      KindOfUnaryOpExpression,
                                      unset_list, T_UNSET, true));
              rep->addElement(unset);
            }
            rep->addElement(makeScalarExpression(ar, k));
            return replaceValue(rep);
          }
          default: break;
        }
      }
    }
  }

  if (!m_classScope && !m_funcScope->isUserFunction()) {
    if (m_type == UnknownType && m_funcScope->isFoldable()) {
      Array arr;
      if (m_params) {
        if (!m_params->isScalar()) return ExpressionPtr();
        for (int i = 0, n = m_params->getCount(); i < n; ++i) {
          Variant v;
          if (!(*m_params)[i]->getScalarValue(v)) return ExpressionPtr();
          arr.set(i, v);
        }
        if (m_arrayParams) {
          arr = arr[0];
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

  if (m_type != UnknownType || m_safe) {
    return ExpressionPtr();
  }

  return inliner(ar, ExpressionPtr(), m_localThis);
}

ExpressionPtr SimpleFunctionCall::preOptimize(AnalysisResultConstPtr ar) {
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
       m_type == FBCallUserFuncSafeFunction ||
       m_type == FunctionExistsFunction ||
       m_type == ClassExistsFunction ||
       m_type == InterfaceExistsFunction) &&
      m_params &&
      (m_type == DefineFunction ?
       unsigned(m_params->getCount() - 2) <= 1u :
       m_type == FBCallUserFuncSafeFunction ? m_params->getCount() >= 1 :
       m_params->getCount() == 1)) {
    ExpressionPtr value = (*m_params)[0];
    if (value->isScalar()) {
      ScalarExpressionPtr name = dynamic_pointer_cast<ScalarExpression>(value);
      if (name && name->isLiteralString()) {
        string symbol = name->getLiteralString();
        switch (m_type) {
          case DefineFunction: {
            ConstantTableConstPtr constants = ar->getConstants();
            // system constant
            if (constants->isPresent(symbol)) {
              break;
            }
            // user constant
            BlockScopeConstPtr block = ar->findConstantDeclarer(symbol);
            // not found (i.e., undefined)
            if (!block) break;
            constants = block->getConstants();
            const Symbol *sym = constants->getSymbol(symbol);
            assert(sym);
            m_extra = (void *)sym;
            Lock lock(BlockScope::s_constMutex);
            if (!sym->isDynamic() && sym->getValue() != (*m_params)[1]) {
              if (sym->getDeclaration() != shared_from_this()) {
                // redeclared
                const_cast<Symbol*>(sym)->setDynamic();
              }
              const_cast<Symbol*>(sym)->setValue((*m_params)[1]);
              getScope()->addUpdates(BlockScope::UseKindConstRef);
            }
            break;
          }
          case DefinedFunction: {
            if (symbol == "false" ||
                symbol == "true" ||
                symbol == "null") {
              return CONSTANT("true");
            }
            ConstantTableConstPtr constants = ar->getConstants();
            // system constant
            if (constants->isPresent(symbol) && !constants->isDynamic(symbol)) {
              return CONSTANT("true");
            }
            // user constant
            BlockScopeConstPtr block = ar->findConstantDeclarer(symbol);
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
            Lock lock(BlockScope::s_constMutex);
            ConstructPtr decl = constants->getValue(symbol);
            ExpressionPtr constValue = dynamic_pointer_cast<Expression>(decl);
            if (constValue->isScalar()) {
              return CONSTANT("true");
            } else {
              return ExpressionPtr();
            }
            break;
          }
          case FBCallUserFuncSafeFunction:
          case FunctionExistsFunction: {
            const std::string &lname = Util::toLower(symbol);
            if (Option::DynamicInvokeFunctions.find(lname) ==
                Option::DynamicInvokeFunctions.end()) {
              FunctionScopePtr func = ar->findFunction(lname);
              if (!func && m_type == FunctionExistsFunction) {
                return CONSTANT("false");
              }
              if (func->isUserFunction()) {
                func->setVolatile();
              }
              if (!func->isVolatile() && m_type == FunctionExistsFunction) {
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

ExpressionPtr SimpleFunctionCall::postOptimize(AnalysisResultConstPtr ar) {
  if (!Option::KeepStatementsWithNoEffect && isDefineWithoutImpl(ar)) {
    Construct::recomputeEffects();
    if (m_extra) {
      Symbol *sym = (Symbol *)m_extra;
      Lock lock(BlockScope::s_constMutex);
      sym->setReplaced();
    }
    return m_extra ? CONSTANT("true") : CONSTANT("false");
  }
  if (m_type == StaticCompactFunction) {
    for (int i = 0; i < m_params->getCount(); i += 2) {
      ExpressionPtr e = (*m_params)[i + 1];
      if (e->is(KindOfUnaryOpExpression) &&
          static_pointer_cast<UnaryOpExpression>(e)->getOp() == T_UNSET_CAST) {
        m_params->removeElement(i);
        m_params->removeElement(i);
        i -= 2;
        m_extraArg -= 2;
        if (m_extraArg < 0) m_extraArg = 0;
      }
    }
    if (!m_params->getCount()) {
      ExpressionPtr rep(new UnaryOpExpression(getScope(), getLocation(),
                                              KindOfUnaryOpExpression,
                                              ExpressionPtr(), T_ARRAY, true));
      return replaceValue(rep);
    }
    m_params->resetOutputCount();
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
  m_implementedType.reset();

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
    if (m_type == DefineFunction && m_params &&
        unsigned(m_params->getCount() - 2) <= 1u) {
      ScalarExpressionPtr name =
        dynamic_pointer_cast<ScalarExpression>((*m_params)[0]);
      if (name) {
        string varName = name->getIdentifier();
        if (!varName.empty()) {
          ExpressionPtr value = (*m_params)[1];
          TypePtr varType = value->inferAndCheck(ar, Type::Some, false);
          BlockScopePtr block = ar->findConstantDeclarer(varName);
          bool newlyDeclared = false;
          if (!block) {
            getFileScope()->declareConstant(ar, varName);
            block = ar->findConstantDeclarer(varName);
            ASSERT(block);
            newlyDeclared = true;
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
              if (newlyDeclared) {
                constants->add(varName, varType, value, ar, self);
                const Symbol *sym = constants->getSymbol(varName);
                assert(sym);
                m_extra = (void *)sym;
              } else {
                constants->setType(ar, varName, varType, true);
              }
            }
            // in case the old 'value' has been optimized
            constants->setValue(ar, varName, value);
          } else {
            assert(!newlyDeclared);
          }
          m_valid = true;
          return checkTypesImpl(ar, type, Type::Boolean, coerce);
        }
      }
      if (getScope()->isFirstPass()) {
        Compiler::Error(Compiler::BadDefine, self);
      }
    } else if (m_type == ExtractFunction || m_type == GetDefinedVarsFunction) {
      getScope()->getVariables()->forceVariants(ar, VariableTable::AnyVars);
    }
  }

  FunctionScopePtr func;

  if (!m_class && m_className.empty()) {
    if (!m_dynamicInvoke) {
      func = ar->findFunction(m_name);
    }
  } else {
    ClassScopePtr cls = resolveClass();
    if (cls && !isPresent()) {
      getScope()->getVariables()
        ->setAttribute(VariableTable::NeedGlobalPointer);
    }
    if (!cls) {
      if (!m_class && !isRedeclared() && getScope()->isFirstPass()) {
        Compiler::Error(Compiler::UnknownClass, self);
      }
      if (m_params) {
        m_params->inferAndCheck(ar, Type::Some, false);
        m_params->markParams(canInvokeFewArgs());
      }
      return checkTypesImpl(ar, type, Type::Variant, coerce);
    }
    m_classScope = cls;

    if (m_name == "__construct") {
      // if the class is known, php will try to identify class-name ctor
      func = cls->findConstructor(ar, true);
    } else {
      func = cls->findFunction(ar, m_name, true, true);
    }

    if (func && !func->isStatic()) {
      ClassScopePtr clsThis = getOriginalClass();
      FunctionScopePtr funcThis = getOriginalFunction();
      if (!clsThis ||
          (clsThis != m_classScope &&
           !clsThis->derivesFrom(ar, m_className, true, false)) ||
          funcThis->isStatic()) {
        func->setDynamic();
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
    if (m_params) {
      if (func) {
        FunctionScope::FunctionInfoPtr info =
          FunctionScope::GetFunctionInfo(m_name);
        assert(info);
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

  if (m_valid && !m_className.empty() &&
      (!m_funcScope || !m_funcScope->isStatic())) {
    int objCall = checkObjCall(ar);

    if (objCall < 0 || (objCall > 0 && !m_localThis.empty())) {
      m_implementedType = Type::Variant;
    }
  }

  return rtype;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void SimpleFunctionCall::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  outputLineMap(cg, ar);

  if (m_class || !m_className.empty()) {
    StaticClassName::outputPHP(cg, ar);
    cg_printf("::%s(", m_origName.c_str());
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
      cg_printf("%s(", m_origName.c_str());
    } else {
      cg_printf("%s(", m_origName.c_str());
    }
  }

  if (m_params) m_params->outputPHP(cg, ar);
  cg_printf(")");
}

/*
 * returns: 1 - if the call is dynamic
 *         -1 - if the call may be dynamic, depending on redeclared derivation
 *          0 - if the call is static (ie with an "empty" this).
 */
static int isObjCall(AnalysisResultPtr ar,
                     ClassScopeRawPtr thisCls, FunctionScopeRawPtr thisFunc,
                     ClassScopeRawPtr methCls, const std::string &methClsName) {
  if (!thisCls || !thisFunc || thisFunc->isStatic()) return 0;
  if (thisCls == methCls) return 1;
  if (thisCls->derivesFrom(ar, methClsName, true, false)) return 1;
  if (thisCls->derivesFromRedeclaring() &&
      thisCls->derivesFrom(ar, methClsName, true, true)) {
    return -1;
  }
  return 0;
}

int SimpleFunctionCall::checkObjCall(AnalysisResultPtr ar) {
  ClassScopeRawPtr orig = getOriginalClass();
  int objCall = isObjCall(ar, orig, getOriginalFunction(),
                          m_classScope, m_className);
  if (objCall > 0 && m_localThis.empty() &&
      (getClassScope() != orig || getFunctionScope()->isStatic())) {
    int o = isObjCall(ar, getClassScope(), getFunctionScope(),
                      orig, orig->getName());
    if (o <= 0) objCall = o;
  }
  return objCall;
}

bool SimpleFunctionCall::preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                                      int state) {
  if (m_type == StaticCompactFunction) {
    if (!cg.inExpression()) return true;
    cg.wrapExpressionBegin();
    m_ciTemp = cg.createNewLocalId(shared_from_this());
    cg_printf("ArrayInit compact%d(%d, false);\n",
              m_ciTemp, m_params->getCount() / 2);
    for (int i = 0; i < m_params->getCount(); i += 2) {
      assert((*m_params)[i]->isLiteralString());
      string p = (*m_params)[i]->getLiteralString();
      ExpressionPtr e = (*m_params)[i + 1];
      if (e->is(KindOfSimpleVariable)
       && Type::SameType(e->getCPPType(), Type::Variant)) {
        SimpleVariablePtr sv = dynamic_pointer_cast<SimpleVariable>(e);
        cg_printf("if (%s%s.isInitialized()) ",
                  Option::VariablePrefix, sv->getName().c_str());
      }
      e->preOutputCPP(cg, ar, 0);
      cg_printf("compact%d.add(", m_ciTemp);
      cg_printString(p, ar, shared_from_this());
      cg_printf(", ");
      e->outputCPP(cg, ar);
      cg_printf(");\n");
    }
    return true;
  }

  int objCall = 0;
  if (!m_className.empty() && (!m_funcScope || !m_funcScope->isStatic())) {
    objCall = checkObjCall(ar);

    if (objCall < 0) {
      /*
        We have X::foo (which is non-static) inside a non-static
        method of Y, where Y may or may not be derived from X, depending
        on redeclared classes.
        Revert to dynamic dispatch for this case.
      */
      m_valid = false;
    } else if (objCall > 0 && !m_localThis.empty()) {
      m_valid = false;
    }
  }

  if (m_valid) {
    bool ret = false;
    if (m_classScope &&
        (m_arrayParams || !m_funcScope->isStatic())) {
      ret = true;
      if (cg.inExpression()) {
        if (m_funcScope->isStatic()) {
          cg.wrapExpressionBegin();
          m_ciTemp = cg.createNewLocalId(shared_from_this());
          cg_printf("MethodCallPackage mcp%d;\n", m_ciTemp);
          // mcp.isObj is by default false
          cg_printf("mcp%d.rootCls = %s%s::s_class_name.get();\n",
                    m_ciTemp,
                    Option::ClassPrefix, m_classScope->getId(cg).c_str());
        } else {
          if (!objCall || m_arrayParams) {
            cg.wrapExpressionBegin();
            m_ciTemp = cg.createNewLocalId(shared_from_this());
            cg_printf("MethodCallPackage mcp%d;\n", m_ciTemp);
          }
          if (objCall && m_arrayParams) {
            cg_printf("mcp%d.obj = %s;\n",
                      m_ciTemp, getThisString(false).c_str());
          }
        }
      }
    } else if (m_arrayParams && !m_funcScope->isUserFunction()) {
      ret = true;
      if (cg.inExpression()) {
        cg.wrapExpressionBegin();
        cg_printf("extern Variant %s%s(void*,CArrRef);\n",
                  Option::InvokePrefix, m_funcScope->getId(cg).c_str());
      }
    }

    return FunctionCall::preOutputCPP(cg, ar, state) || ret;
  }

  // Short circuit out if inExpression() returns false
  if (!cg.inExpression()) return true;
  cg.wrapExpressionBegin();
  m_ciTemp = cg.createNewLocalId(shared_from_this());
  bool needHash = true;
  string escapedName(cg.escapeLabel(m_origName));
  string escapedClass;
  ClassScopePtr cls = m_classScope;
  if (!m_className.empty()) {
    if (!m_safe && !isPresent()) {
      ClassScope::OutputVolatileCheck(
        cg, ar, getScope(), m_origClassName, false);
      cg_printf(";\n");
    }
    escapedClass = cg.escapeLabel(m_className);
  }
  cg_printf("const CallInfo *cit%d = NULL;\n", m_ciTemp);
  if (!m_class && m_className.empty()) {
    cg_printf("void *vt%d = NULL;\n", m_ciTemp);
  } else {
    cg_printf("MethodCallPackage mcp%d;\n", m_ciTemp);
  }
  bool safeCheck = false;
  if (m_safe) {
    if (!m_className.empty()) {
      if (!isPresent()) {
        cg_indentBegin("if (");
        ClassScope::OutputVolatileCheck(cg, ar, getScope(),
                                        m_origClassName, true);
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
    ClassScopePtr cscope = getOriginalClass();

    // The call was like parent::
    string className;
    if (m_classScope) {
      if (isRedeclared()) {
        className = cg.formatLabel(m_className);
      } else {
        className = m_classScope->getId(cg);
      }
    } else {
      className = cg.formatLabel(m_className);
      if (!m_className.empty() && m_cppTemp.empty() &&
          !isSelf() && !isParent() && !isStatic()) {
        // Create a temporary to hold the class name, in case it is not a
        // StaticString.
        m_clsNameTemp = cg.createNewLocalId(shared_from_this());
        cg_printf("CStrRef clsName%d(", m_clsNameTemp);
        cg_printString(m_origClassName, ar, shared_from_this());
        cg_printf(");\n");
        cg_printf("id(clsName%d);\n", m_clsNameTemp);
      }
    }

    if (objCall && !isUnknown() && !m_classScope) {
      // class must be redeclaring, and cant be the originalClass
      // (because then m_classScope would not be null).
      // so we can start the search by following the redeclared parent.
      cg_printf("mcp%d.methodCallEx(%s, ",
                m_ciTemp, getThisString(false).c_str());
      cg_printString(escapedName, ar, shared_from_this());
      cg_printf(");\n");
      cg_printf("%sparent->%sget_call_info%s(mcp%d",
                getThisString(true).c_str(),
                Option::ObjectPrefix,
                ms ? "_with_index" : "", m_ciTemp);
    } else if (objCall < 0) {
      // Dont know if going to get an object or not
      cg_printf("mcp%d.dynamicNamedCall%s(", m_ciTemp,
                ms ? "WithIndex" : "");
      cg_printString(escapedClass, ar, shared_from_this());
      cg_printf(", ");
      cg_printString(escapedName, ar, shared_from_this());
    } else if (isRedeclared()) {
      cg_printf("mcp%d.staticMethodCall(", m_ciTemp);
      cg_printString(escapedClass, ar, shared_from_this());
      cg_printf(", ");
      cg_printString(escapedName, ar, shared_from_this());
      cg_printf(");\n");
      cg_printf("%s->%s%s->%sget_call_info%s(mcp%d",
                cg.getGlobals(ar), Option::ClassStaticsObjectPrefix,
                className.c_str(), Option::ObjectStaticPrefix,
                ms ? "with_index" : "", m_ciTemp);
    } else if (m_classScope) {
      // In an object, calling a superclass's method
      if (objCall > 0) {
        cg_printf("mcp%d.methodCallEx(%s, ",
                  m_ciTemp, getThisString(false).c_str());
        cg_printString(escapedName, ar, shared_from_this());
        cg_printf(");\n");
        cg_printf("%s%s::%sget_call_info%s(mcp%d",
                  Option::ClassPrefix, className.c_str(), Option::ObjectPrefix,
                  ms ? "_with_index" : "", m_ciTemp);
      } else {
        cg_printf("mcp%d.staticMethodCall(", m_ciTemp);
        cg_printString(escapedClass, ar, shared_from_this());
        cg_printf(", ");
        cg_printString(escapedName, ar, shared_from_this());
        cg_printf(");\n");
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
          cg_printString("static", ar, shared_from_this());
          lsb = true;
        } else {
          m_class->preOutputCPP(cg, ar, 0);
          cg_printf("mcp%d.dynamicNamedCall%s(", m_ciTemp,
              ms ? "_with_index" : "");
          m_class->outputCPP(cg, ar);
        }
        cg_printf(", ");
        cg_printString(escapedName, ar, shared_from_this());
        cg_printf(");\n");
        if (lsb) {
          cg_printf("mcp%d.lateStaticBind(fi.getThreadInfo());\n", m_ciTemp);
        }
      } else {
        // Nonexistent method
        cg_printf("mcp%d.dynamicNamedCall%s(", m_ciTemp,
                  ms ? "_with_index" : "");
        cg_printString(escapedClass, ar, shared_from_this());
        cg_printf(", ");
        cg_printString(escapedName, ar, shared_from_this());
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

  int s = m_safeDef && m_params &&
    m_safeDef->hasEffect() && m_params->hasEffect() ? FixOrder | StashVars : 0;

  if (m_safeDef) m_safeDef->preOutputCPP(cg, ar, s);

  if (m_params && m_params->getCount() > 0) {
    cg.pushCallInfo(m_ciTemp);
    m_params->preOutputCPP(cg, ar, s & ~StashVars);
    cg.popCallInfo();
  }

  if (state & FixOrder) {
    if (cg.inExpression()) {
      preOutputStash(cg, ar, state);
    }
  }

  return true;
}

string SimpleFunctionCall::getThisString(bool withArrow) {
  if (m_localThis.empty() || getOriginalClass() == getClassScope()) {
    return withArrow ? "" : "this";
  }
  assert(!m_localThis.empty());
  if (withArrow) return m_localThis + "->";
  return m_localThis;
}

void SimpleFunctionCall::outputCPPParamOrderControlled(CodeGenerator &cg,
                                                       AnalysisResultPtr ar) {
  if (!m_class && m_className.empty()) {
    switch (m_type) {
      case ExtractFunction:
        cg_printf("extract(variables, ");
        FunctionScope::OutputCPPArguments(m_params, m_funcScope, cg, ar, 0,
                                          false);
        cg_printf(")");
        return;
      case CompactFunction:
        cg_printf("compact(variables, ");
        FunctionScope::OutputCPPArguments(m_params, m_funcScope, cg, ar, -1,
                                          true);
        cg_printf(")");
        return;
      case StaticCompactFunction:
        cg_printf("Array(compact%d.create())", m_ciTemp);
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
      if (!isPresent()) {
        if (m_valid) {
          cg_printf("(");
          ClassScope::OutputVolatileCheck(cg, ar, getScope(),
                                          m_origClassName, true);
        }
        volatileCheck = true;
      }
    } else if (!m_funcScope || m_funcScope->isVolatile()) {
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
        safeCast->outputCPPCast(cg, ar, getScope());
        cg_printf("(");
      }
      if (m_funcScope && !m_funcScope->getReturnType()) {
        cg_printf("(");
      }
    }
  } else if (!m_className.empty()) {
    if (!isPresent()) {
      volatileCheck = true;
      ClassScope::OutputVolatileCheckBegin(cg, ar, getScope(), m_origClassName);
    }
  }

  if (m_valid && m_ciTemp < 0 && !m_arrayParams) {
    if (!m_className.empty()) {
      assert(cls);
      if (!m_funcScope->isStatic()) {
        if (m_localThis.empty() && getOriginalClass()->isRedeclaring() &&
            getOriginalClass() != getClassScope()) {
          cg_printf("((%s%s*)parent.get())->",
                    Option::ClassPrefix, getOriginalClass()->getId(cg).c_str());
        }
        if (m_classScope->isRedeclaring() &&
            m_classScope != getOriginalClass()) {
          cg_printf("((%s%s*)%sparent.get())->",
                    Option::ClassPrefix, cls->getId(cg).c_str(),
                    getThisString(true).c_str());
        } else if (getOriginalClass() != getClassScope()) {
          cg_printf("%s", getThisString(true).c_str());
        }
      }
      cg_printf("%s%s::%s%s(", Option::ClassPrefix, cls->getId(cg).c_str(),
                m_funcScope->getPrefix(m_params),
                cg.formatLabel(m_funcScope->getName()).c_str());
    } else {
      int paramCount = m_params ? m_params->getCount() : 0;
      if (m_name == "get_class" && getOriginalClass() && paramCount == 0) {
        cg_printf("(");
        cg_printString(getOriginalClass()->getOriginalName(), ar,
                       shared_from_this());
      } else if (m_name == "get_parent_class" && getOriginalClass() &&
                 paramCount == 0) {
        const std::string parentClass = getOriginalClass()->getParent();
        cg_printf("(");
        if (!parentClass.empty()) {
          cg_printString(parentClass, ar, shared_from_this());
        } else {
          cg_printf("false");
        }
      } else {
        if (m_noPrefix) {
          cg_printf("%s(", cg.formatLabel(m_name).c_str());
        } else {
          cg_printf("%s%s(",
                    m_builtinFunction ? Option::BuiltinFunctionPrefix :
                    m_funcScope->getPrefix(m_params),
                    m_funcScope->getId(cg).c_str());
        }
      }
    }
    FunctionScope::OutputCPPArguments(m_params, m_funcScope, cg, ar,
                                      m_extraArg, m_variableArgument,
                                      m_argArrayId,
                                      m_argArrayHash, m_argArrayIndex);
  } else {
    int pcount = m_params ? m_params->getCount() : 0;
    bool outputExtraArgs = true;
    if (!m_class && m_className.empty()) {
      if (m_valid) {
        assert(m_arrayParams && m_ciTemp < 0);
        cg_printf("%s%s(NULL, ", Option::InvokePrefix,
                  m_funcScope->getId(cg).c_str());
      } else if (canInvokeFewArgs() && !m_arrayParams) {
        if (Option::InvokeWithSpecificArgs) {
          cg_printf("(cit%d->getFunc%dArgs())(vt%d, ",
                    m_ciTemp, pcount, m_ciTemp);
          outputExtraArgs = false;
        } else {
          cg_printf("(cit%d->getFuncFewArgs())(vt%d, ", m_ciTemp, m_ciTemp);
        }
      } else {
        cg_printf("(cit%d->getFunc())(vt%d, ", m_ciTemp, m_ciTemp);
      }
    } else {
      if (m_valid) {
        const char *prefix = m_arrayParams || !canInvokeFewArgs() ?
          Option::InvokePrefix : Option::InvokeFewArgsPrefix;

        cg_printf("%s%s%s::%s%s(mcp%d, ",
                  getThisString(true).c_str(),
                  Option::ClassPrefix, cls->getId(cg).c_str(),
                  prefix,
                  cg.formatLabel(m_funcScope->getName()).c_str(),
                  m_ciTemp);
      } else if (canInvokeFewArgs() && !m_arrayParams) {
        if (Option::InvokeWithSpecificArgs) {
          cg_printf("(cit%d->getMeth%dArgs())(mcp%d, ",
                    m_ciTemp, pcount, m_ciTemp);
          outputExtraArgs = false;
        } else {
          cg_printf("(cit%d->getMethFewArgs())(mcp%d, ", m_ciTemp, m_ciTemp);
        }
      } else {
        cg_printf("(cit%d->getMeth())(mcp%d, ", m_ciTemp, m_ciTemp);
      }
    }
    if (canInvokeFewArgs() && !m_arrayParams) {
      if (pcount) {
        cg_printf("%d, ", pcount);
        cg.pushCallInfo(m_ciTemp);
        for (int i = 0; i < pcount; i++) {
          (*m_params)[i]->setContext(NoRefWrapper);
        }
        FunctionScope::OutputCPPArguments(m_params, m_funcScope, cg, ar, 0,
                                          false);
        cg.popCallInfo();
      } else {
        cg_printf("0");
      }
      if (outputExtraArgs) {
        for (int i = pcount; i < Option::InvokeFewArgsCount; i++) {
          cg_printf(", null_variant");
        }
      }
    } else {
      if (!pcount) {
        cg_printf("Array()");
      } else {
        cg.pushCallInfo(m_ciTemp);
        FunctionScope::OutputCPPArguments(m_params, m_funcScope, cg, ar,
                                          m_arrayParams ? 0 : -1, false);
        cg.popCallInfo();
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
          safeCast->outputCPPCast(cg, ar, getScope());
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
    if (m_type == DefineFunction && m_params &&
        unsigned(m_params->getCount() - 2) <= 1u) {
      ScalarExpressionPtr name =
        dynamic_pointer_cast<ScalarExpression>((*m_params)[0]);
      string varName;
      ExpressionPtr value = (*m_params)[1];
      if (name) {
        varName = name->getIdentifier();
        if (varName.empty()) {
          cg_printf("throw_fatal(\"bad define\")");
        } else if (m_dynamicConstant) {
          cg_printf("g->declareConstant(");
          cg_printString(varName, ar, shared_from_this());
          cg_printf(", g->%s%s, ", Option::ConstantPrefix,
                    cg.formatLabel(varName).c_str());
          value->outputCPP(cg, ar);
          cg_printf(")");
        } else {
          bool needAssignment = true;
          bool isSystem = ar->isSystemConstant(varName);
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
      FunctionScopePtr func = getFunctionScope();
      if (func && func->isGenerator()) {
        cg_printf("%s%s->%snum_args()",
                  Option::VariablePrefix, CONTINUATION_OBJECT_NAME,
                  Option::MethodPrefix);
        return;
      }
      if (!func || func->isVariableArgument()) {
        cg_printf("num_args");
      } else {
        cg_printf("%d", func->getMaxParamCount());
      }
      return;
    }
    if (m_name == "hphp_get_call_info" &&
        m_params && m_params->getCount() == 2) {
      ScalarExpressionPtr clsName(
          dynamic_pointer_cast<ScalarExpression>((*m_params)[0]));
      ScalarExpressionPtr funcName(
          dynamic_pointer_cast<ScalarExpression>((*m_params)[1]));
      if (clsName && funcName) {
        string cname = clsName->getLiteralString();
        string fname = funcName->getLiteralString();
        if (!fname.empty()) {
          if (!cname.empty()) {
            ClassScopePtr cscope(ar->findClass(cname));
            if (cscope && !cscope->isRedeclaring()) {
              FunctionScopePtr fscope(cscope->findFunction(ar, fname, true));
              if (fscope) {
                cg_printf("(int64)&%s%s::%s%s",
                          Option::ClassPrefix,
                          cg.formatLabel(cname).c_str(),
                          Option::CallInfoPrefix,
                          fname.c_str());
                return;
              }
            }
          } else {
            FunctionScopePtr fscope(ar->findFunction(fname));
            if (fscope) {
              cg_printf("(int64)&%s%s",
                        Option::CallInfoPrefix,
                        fname.c_str());
              return;
            }
          }
        }
      }
    }

    switch (m_type) {
      case VariableArgumentFunction:
      {
        FunctionScopePtr func = getFunctionScope();
        if (func) {
          bool isGetArgs = m_name == "func_get_args";
          if (func->isGenerator()) {
            cg_printf("%s%s->%s%s(",
                      Option::VariablePrefix, CONTINUATION_OBJECT_NAME,
                      Option::MethodPrefix,
                      isGetArgs ? "get_args" : "get_arg");
            if (m_params) {
              for (int i = 0; i < m_params->getCount(); i++) {
                if (i) cg_printf(",");
                (*m_params)[i]->outputCPP(cg, ar);
              }
            }
            cg_printf(")");
            return;
          }
          if (isGetArgs &&
              func->getMaxParamCount() == 0 &&
              (!m_params || m_params->getCount() == 0)) {
            // in the special case of calling func_get_args() for
            // a function with no explicit params, bypass the call
            // to func_get_args() and simply use the passed in args
            // array or the empty array
            if (func->isVariableArgument()) {
              cg_printf("(args.isNull() ? Array::Create() : args)");
            } else {
              cg_printf("Array::Create()");
            }
            return;
          } else if (m_name == "func_get_arg" &&
                     m_params &&
                     m_params->getCount() == 1) {
            Variant v;
            ExpressionPtr p((*m_params)[0]);
            if (p->getScalarValue(v) && v.isInteger()) {
              // if func_get_arg is called with a scalar int, then
              // optimize the call to func_get_arg away
              int64 idx = v.toInt64();
              if (idx < 0) {
                cg_printf("Variant(false)");
                return;
              }
              if (idx >= func->getMaxParamCount()) {
                if (func->isVariableArgument()) {
                  int64 idx0 = idx - func->getMaxParamCount();
                  cg_printf("(%lldLL < num_args ? "
                            "args.rvalAt(%lldLL) : Variant(false))",
                            idx, idx0);
                } else {
                  cg_printf("Variant(false)");
                }
                return;
              }
              const string& funcName = func->getParamName(idx);
              bool needsCast =
                !func->getParamType(idx)->is(Type::KindOfVariant) &&
                !func->getParamType(idx)->is(Type::KindOfSome);
              bool isStashed =
                func->getVariables()->getSymbol(funcName)->isStashedVal();
              if (func->isVariableArgument()) {
                cg_printf("(%lldLL < num_args ? ", idx);
              }
              if (needsCast) cg_printf("Variant(");
              cg_printf("%s%s%s",
                        isStashed ? "v" : "",
                        Option::VariablePrefix,
                        funcName.c_str());
              if (needsCast) cg_printf(")");
              if (func->isVariableArgument()) {
                cg_printf(" : Variant(false))");
              }
              return;
            }
          }
          cg_printf("%s(", m_name.c_str());
          func->outputCPPParamsCall(cg, ar, true);
          if (m_params) {
            cg_printf(", ");
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
                    cg_printf("checkClassExistsNoThrow(");
                  } else {
                    cg_printf("checkInterfaceExistsNoThrow(");
                  }
                  cg_printString(symbol, ar, shared_from_this());
                  cg_printf(", &%s->CDEC(%s))",
                            cg.getGlobals(ar),
                            cg.formatLabel(lname).c_str());
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

SimpleFunctionCallPtr SimpleFunctionCall::GetFunctionCallForCallUserFunc(
  AnalysisResultConstPtr ar, SimpleFunctionCallPtr call, int testOnly,
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
          if (testOnly < 0) return SimpleFunctionCallPtr();
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

        ClassScopePtr cls;
        if (sclass == "self") {
          cls = call->getClassScope();
        }
        if (!cls) {
          if (sclass == "parent") {
            cls = call->getClassScope();
            if (cls && !cls->getParent().empty()) {
              sclass = cls->getParent();
            }
          }
          cls = ar->findClass(sclass);
          if (!cls) {
            error = true;
            return SimpleFunctionCallPtr();
          }
          if (cls->isRedeclaring()) {
            cls = ar->findExactClass(call, sclass);
          } else if (!cls->isVolatile() && cls->isUserClass() &&
                     !ar->checkClassPresent(call, sclass)) {
            cls->setVolatile();
          }
          if (!cls) {
            return SimpleFunctionCallPtr();
          }
        }

        if (testOnly < 0) return SimpleFunctionCallPtr();

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
            (cls != call->getOriginalClass() ||
             !cls->findFunction(ar, smethod, false)) :
            (func->isProtected() &&
             (!call->getOriginalClass() ||
              !call->getOriginalClass()->derivesFrom(ar, sclass,
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
                                      AnalysisResultConstPtr ar,
                                      SimpleFunctionCallPtr call, int mode) {
  bool error = false;
  if (!cg && mode <= 1 && !ar->isSystem()) {
    const std::string &name = call->getName();
    bool isArray = name == "call_user_func_array";
    if (name == "call_user_func" || isArray) {
      SimpleFunctionCallPtr rep(
        SimpleFunctionCall::GetFunctionCallForCallUserFunc(ar, call,
                                                           mode ? -1 : 0,
                                                           1, error));
      if (!mode) {
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
  }
  return ExpressionPtr();
}

ExpressionPtr hphp_opt_fb_call_user_func(CodeGenerator *cg,
                                         AnalysisResultConstPtr ar,
                                         SimpleFunctionCallPtr call, int mode) {
  bool error = false;
  if (!cg && mode <= 1 && !ar->isSystem()) {
    const std::string &name = call->getName();
    bool isArray = name == "fb_call_user_func_array_safe";
    bool safe_ret = name == "fb_call_user_func_safe_return";
    if (isArray || safe_ret || name == "fb_call_user_func_safe") {
      SimpleFunctionCallPtr rep(
        SimpleFunctionCall::GetFunctionCallForCallUserFunc(
          ar, call, mode ? -1 : 0, safe_ret ? 2 : 1, error));
      if (!mode) {
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
  }
  return ExpressionPtr();
}

ExpressionPtr hphp_opt_is_callable(CodeGenerator *cg,
                                   AnalysisResultConstPtr ar,
                                   SimpleFunctionCallPtr call, int mode) {
  if (!cg && mode <= 1 && !ar->isSystem()) {
    bool error = false;
    SimpleFunctionCallPtr rep(
      SimpleFunctionCall::GetFunctionCallForCallUserFunc(
        ar, call, mode ? 1 : -1, 1, error));
    if (error && !mode) {
      return call->makeConstant(ar, "false");
    }
    return rep;
  }

  return ExpressionPtr();
}

}


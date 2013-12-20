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

#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/array_pair_expression.h"
#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/analysis/constant_table.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/util/util.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/ext/ext_variable.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// statics

std::map<std::string,SimpleFunctionCall::FunType>
  SimpleFunctionCall::FunctionTypeMap;

void SimpleFunctionCall::InitFunctionTypeMap() {
  if (FunctionTypeMap.empty()) {
    FunctionTypeMap["define"]               = FunType::Define;
    FunctionTypeMap["create_function"]      = FunType::Create;
    FunctionTypeMap["class_alias"]          = FunType::ClassAlias;

    FunctionTypeMap["func_get_arg"]         = FunType::VariableArgument;
    FunctionTypeMap["func_get_args"]        = FunType::VariableArgument;
    FunctionTypeMap["func_num_args"]        = FunType::VariableArgument;

    FunctionTypeMap["extract"]              = FunType::Extract;
    FunctionTypeMap["compact"]              = FunType::Compact;

    FunctionTypeMap["shell_exec"]           = FunType::ShellExec;
    FunctionTypeMap["exec"]                 = FunType::ShellExec;
    FunctionTypeMap["passthru"]             = FunType::ShellExec;
    FunctionTypeMap["system"]               = FunType::ShellExec;

    FunctionTypeMap["defined"]              = FunType::Defined;
    FunctionTypeMap["function_exists"]      = FunType::FunctionExists;
    FunctionTypeMap["class_exists"]         = FunType::ClassExists;
    FunctionTypeMap["interface_exists"]     = FunType::InterfaceExists;
    FunctionTypeMap["constant"]             = FunType::Constant;

    FunctionTypeMap["unserialize"]          = FunType::Unserialize;
    FunctionTypeMap["apc_fetch"]            = FunType::Unserialize;

    FunctionTypeMap["get_defined_vars"]     = FunType::GetDefinedVars;

    FunctionTypeMap["fb_call_user_func_safe"] = FunType::FBCallUserFuncSafe;
    FunctionTypeMap["fb_call_user_func_array_safe"] =
      FunType::FBCallUserFuncSafe;
    FunctionTypeMap["fb_call_user_func_safe_return"] =
      FunType::FBCallUserFuncSafe;
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
 const std::string &name, bool hadBackslash, ExpressionListPtr params,
 ExpressionPtr cls)
  : FunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(SimpleFunctionCall),
                 ExpressionPtr(), name, hadBackslash, params, cls)
  , m_type(FunType::Unknown)
  , m_dynamicConstant(false)
  , m_builtinFunction(false)
  , m_dynamicInvoke(false)
  , m_transformed(false)
  , m_changedToBytecode(false)
  , m_optimizable(false)
  , m_safe(0)
  , m_extra(nullptr)
{
  if (!m_class && m_className.empty()) {
    m_dynamicInvoke = Option::DynamicInvokeFunctions.find(m_name) !=
      Option::DynamicInvokeFunctions.end();
    auto iter = FunctionTypeMap.find(m_name);
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
  StaticClassName::onParse(ar, fs);
  mungeIfSpecialFunction(ar, fs);

  if (m_type == FunType::Unknown && !m_class && m_className.empty()) {
    ar->parseOnDemandByFunction(m_name);
    int pos = m_name.rfind('\\');
    std::string short_name = m_name.substr(pos + 1);
    auto iter = FunctionTypeMap.find(short_name);
    if (iter != FunctionTypeMap.end()) {
      ar->lock()->addNSFallbackFunc(shared_from_this(), fs);
    }
  }
}

void SimpleFunctionCall::mungeIfSpecialFunction(AnalysisResultConstPtr ar,
                                                FileScopePtr fs) {
  ConstructPtr self = shared_from_this();
  switch (m_type) {
    case FunType::Define:
      if (Option::ParseTimeOpts && m_params &&
          unsigned(m_params->getCount() - 2) <= 1u) {
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

    case FunType::Create:
      if (Option::ParseTimeOpts &&
          m_params->getCount() == 2 &&
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

    // The class_alias builtin can create new names for other classes;
    // we need to mark some of these classes redeclaring to avoid
    // making incorrect assumptions during WholeProgram mode.  See
    // AnalysisResult::collectFunctionsAndClasses.
    case FunType::ClassAlias:
      if (m_params &&
          (m_params->getCount() == 2 || m_params->getCount() == 3) &&
          Option::WholeProgram) {
        if (!(*m_params)[0]->isLiteralString() ||
            !(*m_params)[1]->isLiteralString()) {
          parseTimeFatal(Compiler::NoError,
            "class_alias with non-literal parameters is not allowed when "
            "WholeProgram optimizations are turned on");
        }
        fs->addClassAlias((*m_params)[0]->getLiteralString(),
                          (*m_params)[1]->getLiteralString());
      }
      break;

    case FunType::VariableArgument:
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

    case FunType::Extract:
      fs->setAttribute(FileScope::ContainsLDynamicVariable);
      fs->setAttribute(FileScope::ContainsExtract);
      break;

    case FunType::Compact: {
      // If all the parameters in the compact() call are statically known,
      // there is no need to create a variable table.
      vector<ExpressionPtr> literals;
      if (false && m_params->flattenLiteralStrings(literals)) {
        m_type = FunType::StaticCompact;
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

    case FunType::GetDefinedVars:
      fs->setAttribute(FileScope::ContainsDynamicVariable);
      fs->setAttribute(FileScope::ContainsGetDefinedVars);
      fs->setAttribute(FileScope::ContainsCompact);
      break;

    case FunType::Unknown:
      break;

    default:
      break;
  }
}

void SimpleFunctionCall::resolveNSFallbackFunc(
    AnalysisResultConstPtr ar, FileScopePtr fs) {
  if (ar->findFunction(m_name)) {
    // the fully qualified name for this function exists, nothing to do
    return;
  }

  int pos = m_name.rfind('\\');
  std::string short_name = m_name.substr(pos + 1);
  auto iter = FunctionTypeMap.find(short_name);
  assert(iter != FunctionTypeMap.end());
  m_type = iter->second;
  mungeIfSpecialFunction(ar, fs);
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
      func = ar->findFunction(m_name);
      if (!func && !hadBackslash() && Option::WholeProgram) {
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
      assert(ar->getPhase() != AnalysisResult::FirstInference);
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
    m_safeDef = dynamic_pointer_cast<Expression>(cp);
  } else {
    FunctionCall::setNthKid(n, cp);
  }
}

void SimpleFunctionCall::analyzeProgram(AnalysisResultPtr ar) {
  FunctionCall::analyzeProgram(ar);
  if (m_class) {
    if (!Option::AllDynamic) {
      setDynamicByIdentifier(ar, m_name);
    }
  } else if (ar->getPhase() >= AnalysisResult::AnalyzeAll) {
    addDependencies(ar);
  }

  if (m_safeDef) m_safeDef->analyzeProgram(ar);

  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    ConstructPtr self = shared_from_this();
    // Look up the corresponding FunctionScope and ClassScope
    // for this function call
    m_funcScope.reset();
    m_classScope.reset();
    setupScopes(ar);
    if (m_funcScope && m_funcScope->getOptFunction()) {
      SimpleFunctionCallPtr self(
        static_pointer_cast<SimpleFunctionCall>(shared_from_this()));
      (m_funcScope->getOptFunction())(0, ar, self, 1);
    }

    if (!m_class && !m_className.empty()) {
      if (Option::DynamicInvokeFunctions.find(
            Util::toLower(m_className + "::" + m_name)) !=
          Option::DynamicInvokeFunctions.end()) {
        setNoInline();
      }
    }

    // check for dynamic constant and volatile function/class
    if (!m_class && m_className.empty() &&
        (m_type == FunType::Define ||
         m_type == FunType::Defined ||
         m_type == FunType::FunctionExists ||
         m_type == FunType::FBCallUserFuncSafe ||
         m_type == FunType::ClassExists ||
         m_type == FunType::InterfaceExists) &&
        m_params && m_params->getCount() >= 1) {
      ExpressionPtr value = (*m_params)[0];
      if (value->isScalar()) {
        ScalarExpressionPtr name =
          dynamic_pointer_cast<ScalarExpression>(value);
        if (name && name->isLiteralString()) {
          string symbol = name->getLiteralString();
          switch (m_type) {
            case FunType::Define: {
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
              always_assert(sym);
              if (!sym->isDynamic()) {
                if (FunctionScopeRawPtr fsc = getFunctionScope()) {
                  if (!fsc->inPseudoMain()) {
                    const_cast<Symbol*>(sym)->setDynamic();
                  }
                }
              }
              break;
            }
            case FunType::Defined: {
              ConstantTablePtr constants = ar->getConstants();
              if (!constants->isPresent(symbol)) {
                // user constant
                BlockScopePtr block = ar->findConstantDeclarer(symbol);
                if (block) { // found the constant
                  constants = block->getConstants();
                  // set to be dynamic
                  if (m_type == FunType::Defined) {
                    constants->setDynamic(ar, symbol, true);
                  }
                }
              }
              break;
            }
            case FunType::FBCallUserFuncSafe:
            case FunType::FunctionExists:
              {
                FunctionScopePtr func = ar->findFunction(Util::toLower(symbol));
                if (func && func->isUserFunction()) {
                  func->setVolatile();
                }
                break;
              }
            case FunType::InterfaceExists:
            case FunType::ClassExists:
              {
                ClassScopePtr cls = ar->findClass(Util::toLower(symbol));
                if (cls && cls->isUserClass()) {
                  cls->setVolatile();
                }
                break;
              }
            default:
              assert(false);
          }
        }
      } else if ((m_type == FunType::InterfaceExists ||
                  m_type == FunType::ClassExists) &&
                 value->is(KindOfSimpleVariable)) {
        SimpleVariablePtr name = dynamic_pointer_cast<SimpleVariable>(value);
        if (name && name->getSymbol()) {
          // name is checked as class name
          name->getSymbol()->setClassName();
        }
      }
    }

    if (m_type == FunType::StaticCompact) {
      FunctionScopePtr fs = getFunctionScope();
      VariableTablePtr vt = fs->getVariables();
      if (vt->isPseudoMainTable() ||
          vt->getAttribute(VariableTable::ContainsDynamicVariable)) {
        // When there is a variable table already, we will keep the ordinary
        // compact() call.
        m_type = FunType::Compact;
      } else {
        // compact('a', 'b', 'c') becomes compact('a', $a, 'b', $b, 'c', $c)
        vector<ExpressionPtr> new_params;
        vector<string> strs;
        for (int i = 0; i < m_params->getCount(); i++) {
          ExpressionPtr e = (*m_params)[i];
          always_assert(e->isLiteralString());
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

          SimpleVariablePtr var(new SimpleVariable(
                                  e->getScope(), e->getLocation(), name));
          var->copyContext(e);
          var->updateSymbol(SimpleVariablePtr());
          new_params.push_back(e);
          new_params.push_back(var);
        }
        m_params->clearElements();
        for (unsigned i = 0; i < new_params.size(); i++) {
          m_params->addElement(new_params[i]);
        }
      }
    }

    if (m_type == FunType::Unserialize) {
      ar->forceClassVariants(getOriginalClass(), false);
    }
    if (m_params) {
      markRefParams(m_funcScope, m_name, canInvokeFewArgs());
    }
  } else if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    if (m_type == FunType::Unknown &&
        !m_class && !m_redeclared && !m_dynamicInvoke && !m_funcScope &&
        (m_className.empty() ||
         (m_classScope &&
          !m_classScope->isTrait() &&
          !m_classScope->derivesFromRedeclaring() &&
          !m_classScope->getAttribute(
            ClassScope::HasUnknownStaticMethodHandler) &&
          !m_classScope->getAttribute(
            ClassScope::InheritsUnknownStaticMethodHandler)))) {
      bool ok = false;
      if (m_classScope && getOriginalClass()) {
        FunctionScopeRawPtr fs = getOriginalFunction();
        if (fs && !fs->isStatic() &&
            (m_classScope->getAttribute(
              ClassScope::HasUnknownMethodHandler) ||
             m_classScope->getAttribute(
               ClassScope::InheritsUnknownMethodHandler))) {
          ok = true;
        }
      }
      if (!ok) {
        if (m_classScope || !Unit::lookupFunc(String(m_name).get())) {
          Compiler::Error(Compiler::UnknownFunction, shared_from_this());
        }
      }
    }
  }
}

bool SimpleFunctionCall::readsLocals() const {
  return m_type == FunType::GetDefinedVars ||
    m_type == FunType::Compact;
}

bool SimpleFunctionCall::writesLocals() const {
  return m_type == FunType::Extract;
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
          m_type == FunType::FBCallUserFuncSafe ||
          m_name == "call_user_func" ||
          m_name == "call_user_func_array" ||
          m_name == "forward_static_call" ||
          m_name == "forward_static_call_array" ||
          m_name == "get_called_class") {
        f->setNextLSB(true);
      }
    }
  }
  if (m_type != FunType::Unknown) {
    VariableTablePtr vt = getScope()->getVariables();
    switch (m_type) {
      case FunType::Extract:
        vt->setAttribute(VariableTable::ContainsLDynamicVariable);
        vt->setAttribute(VariableTable::ContainsExtract);
        break;
      case FunType::Compact:
        vt->setAttribute(VariableTable::ContainsDynamicVariable);
      case FunType::StaticCompact:
        vt->setAttribute(VariableTable::ContainsCompact);
        break;
      case FunType::GetDefinedVars:
        vt->setAttribute(VariableTable::ContainsDynamicVariable);
        vt->setAttribute(VariableTable::ContainsGetDefinedVars);
        vt->setAttribute(VariableTable::ContainsCompact);
        break;
      default:
        break;
    }
  }
}

bool SimpleFunctionCall::isCallToFunction(const char *name) const {
  return !strcasecmp(getName().c_str(), name) &&
    !getClass() && getClassName().empty();
}

bool SimpleFunctionCall::isSimpleDefine(StringData **outName,
                                        TypedValue *outValue) const {
  if (!isCallToFunction("define")) return false;
  if (!m_params || m_params->getCount() != 2) return false;
  Variant v;
  if (!(*m_params)[0]->getScalarValue(v) || !v.isString()) return false;
  if (outName) {
    *outName = makeStaticString(v.toCStrRef().get());
  }
  if (!(*m_params)[1]->getScalarValue(v) || v.isArray()) return false;
  if (outValue) {
    if (v.isString()) {
      v = makeStaticString(v.toCStrRef().get());
    }
    *outValue = *v.asTypedValue();
  }
  return true;
}

bool SimpleFunctionCall::isDefineWithoutImpl(AnalysisResultConstPtr ar) {
  if (m_class || !m_className.empty()) return false;
  if (m_type == FunType::Define && m_params &&
      unsigned(m_params->getCount() - 2) <= 1u) {
    if (m_dynamicConstant) return false;
    ScalarExpressionPtr name =
      dynamic_pointer_cast<ScalarExpression>((*m_params)[0]);
    if (!name) return false;
    string varName = name->getIdentifier();
    if (varName.empty()) return false;
    if (!SystemLib::s_inited || ar->isSystemConstant(varName)) {
      always_assert(!m_extra);
      return true;
    }
    ExpressionPtr value = (*m_params)[1];
    if (ar->isConstantRedeclared(varName)) {
      return false;
    }
    Variant scalarValue;
    return (value->isScalar() &&
            value->getScalarValue(scalarValue) &&
            scalarValue.isAllowedAsConstantValue());
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
    if (m_type == FunType::Extract && m_params && m_params->getCount() >= 1) {
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
          } else {
            mode = -1;
          }
          if (n >= 3) {
            ExpressionPtr p = (*m_params)[2];
            if (p->isScalar() && p->getScalarValue(v)) {
              prefix = v.toString();
            } else {
              mode = -1;
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
                                 ExpressionList::ListKindWrapped));
            string root_name;
            int n = arr ? arr->getCount() : 0;
            int i, j, k;
            for (i = j = k = 0; i < n; i++) {
              ArrayPairExpressionPtr ap(
                dynamic_pointer_cast<ArrayPairExpression>((*arr)[i]));
              always_assert(ap);
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
                   !is_valid_var_name(name.c_str(), name.size()))) {
                name = prefix + "_" + name;
              }
              if (!is_valid_var_name(name.c_str(), name.size())) continue;
              SimpleVariablePtr var(
                new SimpleVariable(getScope(), getLocation(), name.data()));
              var->updateSymbol(SimpleVariablePtr());
              ExpressionPtr val(ap->getValue());
              if (!val->isScalar()) {
                if (root_name.empty()) {
                  root_name = "t" + lexical_cast<string>(
                    getFunctionScope()->nextInlineIndex());
                  SimpleVariablePtr rv(
                    new SimpleVariable(getScope(), getLocation(), root_name));
                  rv->updateSymbol(SimpleVariablePtr());
                  rv->getSymbol()->setHidden();
                  ExpressionPtr root(
                    new AssignmentExpression(getScope(), getLocation(),
                                             rv, (*m_params)[0], false));
                  rep->insertElement(root);
                }

                SimpleVariablePtr rv(
                  new SimpleVariable(getScope(), getLocation(), root_name));
                rv->updateSymbol(SimpleVariablePtr());
                rv->getSymbol()->setHidden();
                ExpressionPtr offset(makeScalarExpression(ar, voff));
                val = ExpressionPtr(
                  new ArrayElementExpression(getScope(), getLocation(),
                                             rv, offset));
              }
              ExpressionPtr a(
                new AssignmentExpression(getScope(), getLocation(),
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
                (new ExpressionList(getScope(), getLocation()));

              SimpleVariablePtr rv(
                new SimpleVariable(getScope(), getLocation(), root_name));
              rv->updateSymbol(SimpleVariablePtr());
              unset_list->addElement(rv);

              ExpressionPtr unset(
                new UnaryOpExpression(getScope(), getLocation(),
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
    if (m_type == FunType::Unknown && m_funcScope->isFoldable()) {
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
        Variant v = invoke(m_funcScope->getName().c_str(),
                           arr, -1, true, true);
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

  if (m_type != FunType::Unknown || m_safe) {
    return ExpressionPtr();
  }

  return inliner(ar, ExpressionPtr(), m_localThis);
}

ExpressionPtr SimpleFunctionCall::preOptimize(AnalysisResultConstPtr ar) {
  if (!Option::ParseTimeOpts) return ExpressionPtr();

  if (m_class) updateClassName();

  if (ar->getPhase() < AnalysisResult::FirstPreOptimize) {
    return ExpressionPtr();
  }

  if (ExpressionPtr rep = optimize(ar)) {
    return rep;
  }

  if (!m_class && m_className.empty() &&
      (m_type == FunType::Define ||
       m_type == FunType::Defined ||
       m_type == FunType::FBCallUserFuncSafe ||
       m_type == FunType::FunctionExists ||
       m_type == FunType::ClassExists ||
       m_type == FunType::InterfaceExists) &&
      m_params &&
      (m_type == FunType::Define ?
       unsigned(m_params->getCount() - 2) <= 1u :
       m_type == FunType::FBCallUserFuncSafe ? m_params->getCount() >= 1 :
       m_params->getCount() == 1)) {
    ExpressionPtr value = (*m_params)[0];
    if (value->isScalar()) {
      ScalarExpressionPtr name = dynamic_pointer_cast<ScalarExpression>(value);
      if (name && name->isLiteralString()) {
        string symbol = name->getLiteralString();
        switch (m_type) {
          case FunType::Define: {
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
            always_assert(sym);
            m_extra = (void *)sym;
            Lock lock(BlockScope::s_constMutex);
            if (!sym->isDynamic()) {
              if (sym->getValue() != (*m_params)[1]) {
                if (sym->getDeclaration() != shared_from_this()) {
                  // redeclared
                  const_cast<Symbol*>(sym)->setDynamic();
                }
                const_cast<Symbol*>(sym)->setValue((*m_params)[1]);
                getScope()->addUpdates(BlockScope::UseKindConstRef);
              }
              Variant v;
              ExpressionPtr value =
                static_pointer_cast<Expression>(sym->getValue());
              if (value->getScalarValue(v)) {
                if (!v.isAllowedAsConstantValue()) {
                  const_cast<Symbol*>(sym)->setDynamic();
                }
              }
            }
            break;
          }
          case FunType::Defined: {
            if (symbol == "false" ||
                symbol == "true" ||
                symbol == "null") {
              return CONSTANT("true");
            }
            ConstantTableConstPtr constants = ar->getConstants();
            if (symbol[0] == '\\') symbol = symbol.substr(1);
            // system constant
            if (constants->isPresent(symbol) && !constants->isDynamic(symbol)) {
              return CONSTANT("true");
            }
            // user constant
            BlockScopeConstPtr block = ar->findConstantDeclarer(symbol);
            // not found (i.e., undefined)
            if (!block) {
              if (symbol.find("::") == std::string::npos &&
                  Option::WholeProgram) {
                return CONSTANT("false");
              } else {
                // e.g., defined("self::ZERO")
                break;
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
            }
            break;
          }
          case FunType::FBCallUserFuncSafe:
          case FunType::FunctionExists: {
            const std::string &lname = Util::toLower(symbol);
            if (Option::DynamicInvokeFunctions.find(lname) ==
                Option::DynamicInvokeFunctions.end()) {
              FunctionScopePtr func = ar->findFunction(lname);
              if (!func) {
                if (m_type == FunType::FunctionExists &&
                    Option::WholeProgram) {
                  return CONSTANT("false");
                }
                break;
              }
              if (func->isUserFunction()) {
                func->setVolatile();
              }
              if (!func->isVolatile() && m_type == FunType::FunctionExists) {
                return CONSTANT("true");
              }
            }
            break;
          }
          case FunType::InterfaceExists: {
            ClassScopePtrVec classes = ar->findClasses(Util::toLower(symbol));
            bool interfaceFound = false;
            for (ClassScopePtrVec::const_iterator it = classes.begin();
                 it != classes.end(); ++it) {
              ClassScopePtr cls = *it;
              if (cls->isUserClass()) {
                cls->setVolatile();
              }
              if (cls->isInterface()) {
                interfaceFound = true;
              }
            }
            if (!interfaceFound) {
              if (Option::WholeProgram) {
                return CONSTANT("false");
              }
              break;
            }
            if (classes.size() == 1 && !classes.back()->isVolatile()) {
              return CONSTANT("true");
            }
            break;
          }
          case FunType::ClassExists: {
            ClassScopePtrVec classes = ar->findClasses(Util::toLower(symbol));
            bool classFound = false;
            for (ClassScopePtrVec::const_iterator it = classes.begin();
                 it != classes.end(); ++it) {
              ClassScopePtr cls = *it;
              if (cls->isUserClass()) {
                cls->setVolatile();
              }
              if (!cls->isInterface() && !cls->isTrait()) {
                classFound = true;
              }
            }
            if (!classFound) {
              if (Option::WholeProgram) {
                return CONSTANT("false");
              }
              break;
            }
            if (classes.size() == 1 && !classes.back()->isVolatile()) {
              return CONSTANT("true");
            }
            break;
          }
          default:
            assert(false);
        }
      }
    }
  }
  return ExpressionPtr();
}

ExpressionPtr SimpleFunctionCall::postOptimize(AnalysisResultConstPtr ar) {
  if (m_type == FunType::StaticCompact) {
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
                                              ExpressionPtr(), T_ARRAY, true));
      return replaceValue(rep);
    }
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
  assert(false);
  return TypePtr();
}

TypePtr SimpleFunctionCall::inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                          bool coerce) {
  assert(type);
  IMPLEMENT_INFER_AND_CHECK_ASSERT(getScope());

  resetTypes();
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
    if (m_type == FunType::Define && m_params &&
        unsigned(m_params->getCount() - 2) <= 1u) {
      ScalarExpressionPtr name =
        dynamic_pointer_cast<ScalarExpression>((*m_params)[0]);
      if (name) {
        string varName = name->getIdentifier();
        if (!varName.empty()) {
          ExpressionPtr value = (*m_params)[1];
          TypePtr varType = value->inferAndCheck(ar, Type::Some, false);

          BlockScopePtr block;
          bool newlyDeclared = false;
          {
            Lock lock(ar->getMutex());
            block = ar->findConstantDeclarer(varName);
            if (!block) {
              FileScopeRawPtr fs(getFileScope());
              GET_LOCK(fs); // file scope cannot depend on a function scope
              fs->declareConstant(ar, varName);
              block = ar->findConstantDeclarer(varName);
              newlyDeclared = true;
            }
          }

          assert(block);
          ConstantTablePtr constants = block->getConstants();
          if (constants != ar->getConstants()) {
            TRY_LOCK(block);
            if (value && !value->isScalar()) {
              constants->setDynamic(ar, varName, true);
              varType = Type::Variant;
            }
            if (constants->isDynamic(varName)) {
              m_dynamicConstant = true;
              getScope()->getVariables()->
                setAttribute(VariableTable::NeedGlobalPointer);
            } else {
              if (newlyDeclared) {
                const Symbol *sym = constants->getSymbol(varName);
                assert(!sym || !sym->declarationSet());
                constants->add(varName, varType, value, ar, self);
                sym = constants->getSymbol(varName);
                always_assert(sym);
                m_extra = (void *)sym;
              } else {
                constants->setType(ar, varName, varType, true);
              }
            }
            // in case the old 'value' has been optimized
            constants->setValue(ar, varName, value);
          } else {
            always_assert(!newlyDeclared);
          }
          m_valid = true;
          return checkTypesImpl(ar, type, Type::Boolean, coerce);
        }
      }
      if (getScope()->isFirstPass()) {
        Compiler::Error(Compiler::BadDefine, self);
      }
    } else if (m_type == FunType::Extract || m_type == FunType::GetDefinedVars) {
      getScope()->getVariables()->forceVariants(ar, VariableTable::AnyVars);
    }
  }

  FunctionScopePtr func;

  if (!m_class && m_className.empty()) {
    if (!m_dynamicInvoke) {
      func = ar->findFunction(m_name);
    }
  } else {
    ClassScopePtr cls = resolveClassWithChecks();
    if (!cls) {
      if (m_params) {
        m_params->inferAndCheck(ar, Type::Some, false);
        markRefParams(FunctionScopePtr(), m_name, canInvokeFewArgs());
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
      if (!Option::AllDynamic &&
          (!clsThis ||
          (clsThis != m_classScope &&
           !clsThis->derivesFrom(ar, m_className, true, false)) ||
          funcThis->isStatic())) {
        func->setDynamic();
      }
    }
  }
  if (!func || func->isRedeclaring() || func->isAbstract()) {
    if (m_funcScope) {
      m_funcScope.reset();
      Construct::recomputeEffects();
    }
    if (func && func->isRedeclaring()) {
      m_redeclared = true;
      getScope()->getVariables()->
        setAttribute(VariableTable::NeedGlobalPointer);
    }
    if (m_params) {
      if (Option::WholeProgram && func && func->isRedeclaring()) {
        FunctionScope::FunctionInfoPtr info =
          FunctionScope::GetFunctionInfo(m_name);
        always_assert(info);
        for (int i = m_params->getCount(); i--; ) {
          if (!Option::WholeProgram || info->isRefParam(i)) {
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
    assert(!m_funcScope ||
           !func->hasUser(getScope(), BlockScope::UseKindCaller));
    m_funcScope = func;
    m_funcScope->addCaller(getScope(), !type->is(Type::KindOfAny));
    Construct::recomputeEffects();
  }

  m_builtinFunction = !func->isUserFunction();

  beforeCheck(ar);

  m_valid = true;
  TypePtr rtype = checkParamsAndReturn(ar, type, coerce,
                                       func, m_arrayParams);

  // this is ok un-guarded b/c this value never gets un-set (once its
  // true its always true) and the value itself doesn't get read
  // until later phases
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

    if (objCall <= 0 || !m_localThis.empty()) {
      m_implementedType = Type::Variant;
    }
  }

  assert(rtype);
  return rtype;
}

///////////////////////////////////////////////////////////////////////////////

void SimpleFunctionCall::outputCodeModel(CodeGenerator &cg) {
  if (m_class || !m_className.empty()) {
    cg.printObjectHeader("ClassMethodCallExpression", 4);
    cg.printPropertyHeader("className");
    StaticClassName::outputCodeModel(cg);
    cg.printPropertyHeader("methodName");
  } else {
    cg.printObjectHeader("SimpleFunctionCallExpression", 3);
    cg.printPropertyHeader("functionName");
  }
  cg.printValue(m_origName);
  cg.printPropertyHeader("arguments");
  cg.printExpressionVector(m_params);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void SimpleFunctionCall::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
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

FunctionScopePtr
SimpleFunctionCall::getFuncScopeFromParams(AnalysisResultPtr ar,
                                           BlockScopeRawPtr scope,
                                           ExpressionPtr clsName,
                                           ExpressionPtr funcName,
                                           ClassScopePtr &clsScope) {
  clsScope.reset();
  ScalarExpressionPtr clsName0(
      dynamic_pointer_cast<ScalarExpression>(clsName));
  ScalarExpressionPtr funcName0(
      dynamic_pointer_cast<ScalarExpression>(funcName));
  if (clsName0 && funcName0) {
    string cname = clsName0->getLiteralString();
    string fname = funcName0->getLiteralString();
    if (!fname.empty()) {
      if (!cname.empty()) {
        ClassScopePtr cscope(ar->findClass(cname));
        if (cscope && cscope->isRedeclaring()) {
          cscope = scope->findExactClass(cscope);
        }
        if (cscope) {
          FunctionScopePtr fscope(cscope->findFunction(ar, fname, true));
          if (fscope) {
            clsScope = cscope;
          }
          return fscope;
        }
      } else {
        FunctionScopePtr fscope(ar->findFunction(fname));
        return fscope;
      }
    }
  }
  return FunctionScopePtr();
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
              new ExpressionList(call->getScope(), call->getLocation()));
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
                                  name, false, p2, ExpressionPtr()));
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
        Variant classname = arr.rvalAt(int64_t(0));
        Variant methodname = arr.rvalAt(int64_t(1));
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
            if (cls && !cls->getOriginalParent().empty()) {
              sclass = cls->getOriginalParent();
            }
          }
          cls = ar->findClass(sclass);
          if (!cls) {
            error = true;
            return SimpleFunctionCallPtr();
          }
          if (cls->isRedeclaring()) {
            cls = call->getScope()->findExactClass(cls);
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
            new ExpressionList(call->getScope(), call->getLocation()));
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
                                smethod, false, p2, cl));
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
  if (!cg && mode <= 1 && Option::WholeProgram) {
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
          if (!isArray) {
            rep->setSafeCall(-1);
            rep->addLateDependencies(ar);
            if (isArray) rep->setArrayParams();
          } else {
            rep.reset();
          }
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
  if (!cg && mode <= 1 && Option::WholeProgram) {
    const std::string &name = call->getName();
    bool isArray = name == "fb_call_user_func_array_safe";
    bool safe_ret = name == "fb_call_user_func_safe_return";
    if (isArray || safe_ret || name == "fb_call_user_func_safe") {
      SimpleFunctionCallPtr rep(
        SimpleFunctionCall::GetFunctionCallForCallUserFunc(
          ar, call, mode ? -1 : 0, safe_ret ? 2 : 1, error));
      if (!mode) {
        if (error) {
          if (!Option::WholeProgram) {
            rep.reset();
          } else if (safe_ret) {
            return (*call->getParams())[1];
          } else {
            Array ret(Array::Create(0, false));
            ret.set(1, Variant());
            return call->makeScalarExpression(ar, ret);
          }
        }
      }
    }
  }
  return ExpressionPtr();
}

ExpressionPtr hphp_opt_is_callable(CodeGenerator *cg,
                                   AnalysisResultConstPtr ar,
                                   SimpleFunctionCallPtr call, int mode) {
  if (!cg && mode <= 1 && Option::WholeProgram) {
    bool error = false;
    SimpleFunctionCallPtr rep(
      SimpleFunctionCall::GetFunctionCallForCallUserFunc(
        ar, call, mode ? 1 : -1, 1, error));
    if (error && !mode) {
      if (!Option::WholeProgram) {
        rep.reset();
      } else {
        return call->makeConstant(ar, "false");
      }
    }
    return rep;
  }

  return ExpressionPtr();
}

}

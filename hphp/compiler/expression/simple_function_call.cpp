/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include <folly/Conv.h>
#include <map>
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
#include "hphp/util/text-util.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/ext/std/ext_std_variable.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// statics

std::map<std::string,SimpleFunctionCall::FunType,stdltistr>
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
    FunctionTypeMap["parse_str"]            = FunType::Extract;
    FunctionTypeMap["compact"]              = FunType::Compact;

    FunctionTypeMap["assert"]               = FunType::Assert;

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

static struct FunctionTypeMapInitializer {
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
{
  if (!m_class && !hasStaticClass()) {
    m_dynamicInvoke =
      RuntimeOption::DynamicInvokeFunctions.count(m_origName) != 0;
    auto iter = FunctionTypeMap.find(m_origName);
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

void SimpleFunctionCall::onParse(AnalysisResultConstRawPtr ar,
                                 FileScopePtr fs) {
  FunctionCall::onParse(ar, fs);
  mungeIfSpecialFunction(ar, fs);

  if (m_type == FunType::Unknown && !m_class && !hasStaticClass()) {
    ar->parseOnDemandByFunction(m_origName);
    int pos = m_origName.rfind('\\');
    std::string short_name = m_origName.substr(pos + 1);
    auto iter = FunctionTypeMap.find(short_name);
    if (iter != FunctionTypeMap.end()) {
      ar->lock()->addNSFallbackFunc(shared_from_this(), fs);
    }
  }
}

void SimpleFunctionCall::mungeIfSpecialFunction(AnalysisResultConstRawPtr ar,
                                                FileScopePtr fs) {
  ConstructPtr self = shared_from_this();
  switch (m_type) {
    case FunType::Define:
      if (Option::ParseTimeOpts && m_params &&
          unsigned(m_params->getCount() - 2) <= 1u) {
        // need to register the constant before AnalyzeAll, so that
        // DefinedFunction can mark this volatile
        ExpressionPtr ename = (*m_params)[0];
        if (auto cname = dynamic_pointer_cast<ConstantExpression>(ename)) {
          /*
            Hack: If the name of the constant being defined is itself
            a constant expression, assume that its not yet defined.
            So define(FOO, 'bar') is equivalent to define('FOO', 'bar').
          */
          ename = makeScalarExpression(ar, cname->getName());
          m_params->removeElement(0);
          m_params->insertElement(ename);
        }
        auto name = dynamic_pointer_cast<ScalarExpression>(ename);
        if (name) {
          auto const varName = name->getIdentifier();
          if (varName.empty()) break;
          AnalysisResult::Locker lock(ar);
          fs->declareConstant(lock.get(), varName);
          // handling define("CONSTANT", ...);
          ExpressionPtr value = (*m_params)[1];
          BlockScopePtr block = lock->findConstantDeclarer(varName);
          ConstantTablePtr constants = block->getConstants();
          if (constants != ar->getConstants()) {
            constants->add(varName, value, ar, self);
          }
        }
      }
      break;

    case FunType::Create:
      if (Option::ParseTimeOpts &&
          m_params->getCount() == 2 &&
          (*m_params)[0]->isLiteralString() &&
          (*m_params)[1]->isLiteralString()) {
        auto const params = (*m_params)[0]->getLiteralString();
        auto const body = (*m_params)[1]->getLiteralString();
        m_lambda = CodeGenerator::GetNewLambda();
        auto const code = "function " + m_lambda + "(" + params + ") "
          "{" + body + "}";
        m_lambda = "1_" + m_lambda;
        ar->appendExtraCode(fs->getName(), code);
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
      break;

    case FunType::Assert:
      fs->setAttribute(FileScope::ContainsLDynamicVariable);
      break;

    case FunType::Compact:
      fs->setAttribute(FileScope::ContainsDynamicVariable);
      break;

    case FunType::GetDefinedVars:
      fs->setAttribute(FileScope::ContainsDynamicVariable);
      break;

    case FunType::Unknown:
      break;

    default:
      break;
  }
}

void SimpleFunctionCall::resolveNSFallbackFunc(
    AnalysisResultConstRawPtr ar, FileScopePtr fs) {
  if (ar->findFunction(m_origName)) {
    // the fully qualified name for this function exists, nothing to do
    return;
  }

  int pos = m_origName.rfind('\\');
  m_origName = m_origName.substr(pos + 1);
  auto iter = FunctionTypeMap.find(m_origName);
  assert(iter != FunctionTypeMap.end());
  m_type = iter->second;
  mungeIfSpecialFunction(ar, fs);
  updateVtFlags();
}


///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void SimpleFunctionCall::setupScopes(AnalysisResultConstRawPtr ar) {
  FunctionScopePtr func;
  if (!m_class && !hasStaticClass()) {
    if (!m_dynamicInvoke) {
      func = ar->findFunction(m_origName);
      if (!func && !hadBackslash() && Option::WholeProgram) {
        int pos = m_origName.rfind('\\');
        auto short_name = m_origName.substr(pos + 1);
        func = ar->findFunction(m_origName);
        if (func) m_origName = short_name;
      }
    }
  } else {
    ClassScopePtr cls = resolveClass();
    if (cls) {
      m_classScope = cls;
      if (isNamed("__construct")) {
        func = cls->findConstructor(ar, true);
      } else {
        func = cls->findFunction(ar, m_origName, true, true);
      }
    }
  }
  if (func && !func->isRedeclaring()) {
    if (m_funcScope != func) {
      m_funcScope = func;
    }
  }
}

void SimpleFunctionCall::addLateDependencies(AnalysisResultConstRawPtr ar) {
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

void SimpleFunctionCall::analyzeProgram(AnalysisResultConstRawPtr ar) {
  FunctionCall::analyzeProgram(ar);
  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    ConstructPtr self = shared_from_this();
    // Look up the corresponding FunctionScope and ClassScope
    // for this function call
    m_funcScope.reset();
    m_classScope.reset();
    setupScopes(ar);
    if (m_funcScope && m_funcScope->getOptFunction()) {
      auto self = static_pointer_cast<SimpleFunctionCall>(shared_from_this());
      (m_funcScope->getOptFunction())(0, ar, self, 1);
    }

    // check for dynamic constant and volatile function/class
    if (!m_class && !hasStaticClass() &&
        (m_type == FunType::Define ||
         m_type == FunType::Defined ||
         m_type == FunType::FunctionExists ||
         m_type == FunType::ClassExists ||
         m_type == FunType::InterfaceExists) &&
        m_params && m_params->getCount() >= 1) {
      ExpressionPtr value = (*m_params)[0];
      if (value->isScalar()) {
        auto name = dynamic_pointer_cast<ScalarExpression>(value);
        if (name && name->isLiteralString()) {
          auto const symbol = name->getLiteralString();
          switch (m_type) {
            case FunType::Define: {
              // system constant
              if (ar->getConstants()->isPresent(symbol)) {
                break;
              }
              // user constant
              auto const arv = ar->lock();
              auto const block = ar->findConstantDeclarer(symbol);
              // not found (i.e., undefined)
              if (!block) break;
              auto const constants = block->getConstants();
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
              if (!ar->getConstants()->isPresent(symbol)) {
                auto const arv = ar->lock();
                // user constant
                auto const block = arv->findConstantDeclarer(symbol);
                if (block) { // found the constant
                  auto const constants = block->getConstants();
                  // set to be dynamic
                  if (m_type == FunType::Defined) {
                    constants->setDynamic(ar, symbol);
                  }
                }
              }
              break;
            }
            case FunType::FunctionExists:
              {
                FunctionScopePtr func = ar->findFunction(toLower(symbol));
                if (func && func->isUserFunction()) {
                  func->setVolatile();
                }
                break;
              }
            case FunType::InterfaceExists:
            case FunType::ClassExists:
              {
                ClassScopePtr cls = ar->findClass(toLower(symbol));
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
        auto name = dynamic_pointer_cast<SimpleVariable>(value);
        if (name && name->getSymbol()) {
          // name is checked as class name
          name->getSymbol()->setClassName();
        }
      }
    }

    if (m_params) {
      markRefParams(m_funcScope, m_origName);
    }
  } else if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    if (m_type == FunType::Unknown &&
        !m_class && !m_redeclared && !m_dynamicInvoke && !m_funcScope &&
        (!hasStaticClass() ||
         (m_classScope &&
          !m_classScope->isTrait() &&
          m_classScope->derivesFromRedeclaring() == Derivation::Normal &&
          !m_classScope->getAttribute(
            ClassScope::HasUnknownStaticMethodHandler) &&
          !m_classScope->getAttribute(
            ClassScope::InheritsUnknownStaticMethodHandler)))) {
      bool ok = false;
      if (m_classScope && getClassScope()) {
        FunctionScopeRawPtr fs = getFunctionScope();
        if (fs && !fs->isStatic() &&
            (m_classScope->getAttribute(
              ClassScope::HasUnknownMethodHandler) ||
             m_classScope->getAttribute(
               ClassScope::InheritsUnknownMethodHandler))) {
          ok = true;
        }
      }
      if (!ok) {
        if (m_classScope || !Unit::lookupFunc(String(m_origName).get())) {
          Compiler::Error(Compiler::UnknownFunction, shared_from_this());
        }
      }
    }
  }
}

bool SimpleFunctionCall::readsLocals() const {
  return m_type == FunType::GetDefinedVars ||
    m_type == FunType::Compact || m_type == FunType::Assert;
}

bool SimpleFunctionCall::writesLocals() const {
  return m_type == FunType::Extract;
}

void SimpleFunctionCall::updateVtFlags() {
  FunctionScopeRawPtr f = getFunctionScope();
  if (f) {
    if (m_funcScope) {
      if ((m_classScope && (isSelf() || isParent()) &&
           m_funcScope->usesLSB()) ||
          isStatic() ||
          m_type == FunType::FBCallUserFuncSafe ||
          isNamed("call_user_func") ||
          isNamed("call_user_func_array") ||
          isNamed("forward_static_call") ||
          isNamed("forward_static_call_array") ||
          isNamed("get_called_class")) {
        f->setNextLSB(true);
      }
    }
  }
  if (m_type != FunType::Unknown) {
    VariableTablePtr vt = getScope()->getVariables();
    switch (m_type) {
      case FunType::Extract:
        vt->setAttribute(VariableTable::ContainsLDynamicVariable);
        break;
      case FunType::Assert:
        vt->setAttribute(VariableTable::ContainsLDynamicVariable);
      case FunType::Compact:
        vt->setAttribute(VariableTable::ContainsDynamicVariable);
        break;
      case FunType::GetDefinedVars:
        vt->setAttribute(VariableTable::ContainsDynamicVariable);
        break;
      default:
        break;
    }
  }
}

bool SimpleFunctionCall::isCallToFunction(folly::StringPiece name) const {
  return isNamed(name) && !getClass() && !hasStaticClass();
}

std::string SimpleFunctionCall::getFullName() const {
  std::string name;
  if (isStatic()) {
    name = "static::";
  } else if (hasStaticClass()) {
    name += getOriginalClassName() + "::";
  }
  name += getOriginalName();

  return name;
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
  if (!(*m_params)[1]->getScalarValue(v)) return false;
  if (outValue) {
    v.setEvalScalar();
    *outValue = *v.asTypedValue();
  }
  return true;
}

bool SimpleFunctionCall::isDefineWithoutImpl(AnalysisResultConstRawPtr ar) {
  if (m_class || hasStaticClass()) return false;
  if (m_type == FunType::Define && m_params &&
      unsigned(m_params->getCount() - 2) <= 1u) {
    if (m_dynamicConstant) return false;
    auto name = dynamic_pointer_cast<ScalarExpression>((*m_params)[0]);
    if (!name) return false;
    auto const varName = name->getIdentifier();
    if (varName.empty()) return false;
    if (!SystemLib::s_inited || ar->isSystemConstant(varName)) {
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

const StaticString
  s_GLOBALS("GLOBALS"),
  s_this("this");

// Certain simple function calls, like get_class(), can sometimes be evaluated
// statically. Implementing the virtual isScalar() and getScalarValue() here
// allows the statically knowable return values of those sorts of calls to be
// "maximally" inlined early on if desired.
bool SimpleFunctionCall::isScalar() const {
  return
    getScope() &&
    isCallToFunction("get_class") &&
    !getParams() &&
    getClassScope() &&
    !getClassScope()->isTrait();
}

bool SimpleFunctionCall::getScalarValue(Variant &value) {
  if (isScalar()) {
    value = getClassScope()->getScopeName();
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void SimpleFunctionCall::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_class || hasStaticClass()) {
    StaticClassName::outputPHP(cg, ar);
    cg_printf("::%s(", m_origName.c_str());
  } else {
    cg_printf("%s(", m_origName.c_str());
  }

  if (m_params) m_params->outputPHP(cg, ar);
  cg_printf(")");
}

FunctionScopePtr
SimpleFunctionCall::getFuncScopeFromParams(AnalysisResultPtr ar,
                                           BlockScopeRawPtr scope,
                                           ExpressionPtr clsName,
                                           ExpressionPtr funcName,
                                           ClassScopePtr &clsScope) {
  clsScope.reset();
  auto clsName0 = dynamic_pointer_cast<ScalarExpression>(clsName);
  auto funcName0 = dynamic_pointer_cast<ScalarExpression>(funcName);
  if (clsName0 && funcName0) {
    auto const cname = clsName0->getLiteralString();
    auto const fname = funcName0->getLiteralString();
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
  AnalysisResultConstRawPtr ar, SimpleFunctionCallPtr call, int testOnly,
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
          auto name = toLower(v.toString().slice());
          FunctionScopePtr func = ar->findFunction(name);
          if (!func || func->isDynamicInvoke()) {
            error = !func;
            return SimpleFunctionCallPtr();
          }
          if (testOnly < 0) return SimpleFunctionCallPtr();
          ExpressionListPtr p2;
          if (testOnly) {
            p2 = ExpressionListPtr(
              new ExpressionList(call->getScope(), call->getRange()));
            p2->addElement(call->makeScalarExpression(ar, v));
            name = "function_exists";
          } else {
            p2 = static_pointer_cast<ExpressionList>(params->clone());
            while (firstParam--) {
              p2->removeElement(0);
            }
          }
          SimpleFunctionCallPtr rep(
            NewSimpleFunctionCall(call->getScope(), call->getRange(),
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
        Variant classname = arr[0];
        Variant methodname = arr[1];
        if (!methodname.isString()) {
          error = true;
          return SimpleFunctionCallPtr();
        }
        if (!classname.isString()) {
          return SimpleFunctionCallPtr();
        }
        std::string sclass = classname.toString().data();
        auto smethod = toLower(methodname.toString().slice());

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
          }
          if (!cls) {
            return SimpleFunctionCallPtr();
          }
        }

        if (testOnly < 0) return SimpleFunctionCallPtr();

        size_t c = smethod.find("::");
        if (c != 0 && c != std::string::npos && c+2 < smethod.size()) {
          auto const name = smethod.substr(0, c);
          if (!cls->isNamed(name)) {
            if (!cls->derivesFrom(ar, name, true, false)) {
              error = cls->derivesFromRedeclaring() == Derivation::Normal;
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
            (cls != call->getClassScope() ||
             !cls->findFunction(ar, smethod, false)) :
            (func->isProtected() &&
             (!call->getClassScope() ||
              !call->getClassScope()->derivesFrom(ar, sclass,
                                                     true, false)))) {
          error = true;
          return SimpleFunctionCallPtr();
        }
        ExpressionPtr cl(call->makeScalarExpression(ar, classname));
        ExpressionListPtr p2;
        if (testOnly) {
          p2 = ExpressionListPtr(
            new ExpressionList(call->getScope(), call->getRange()));
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
          NewSimpleFunctionCall(call->getScope(), call->getRange(),
                                smethod, false, p2, cl));
        return rep;
      }
    }
  }
  return SimpleFunctionCallPtr();
}

namespace HPHP {

ExpressionPtr hphp_opt_call_user_func(CodeGenerator *cg,
                                      AnalysisResultConstRawPtr ar,
                                      SimpleFunctionCallPtr call, int mode) {
  bool error = false;
  if (!cg && mode <= 1 && Option::WholeProgram) {
    bool isArray = call->isNamed("call_user_func_array");
    if (call->isNamed("call_user_func") || isArray) {
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
                                         AnalysisResultConstRawPtr ar,
                                         SimpleFunctionCallPtr call, int mode) {
  bool error = false;
  if (!cg && mode <= 1 && Option::WholeProgram) {
    bool isArray = call->isNamed("fb_call_user_func_array_safe");
    bool safe_ret = call->isNamed("fb_call_user_func_safe_return");
    if (isArray || safe_ret || call->isNamed("fb_call_user_func_safe")) {
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
            ret.set(1, init_null());
            return call->makeScalarExpression(ar, ret);
          }
        }
      }
    }
  }
  return ExpressionPtr();
}

ExpressionPtr hphp_opt_is_callable(CodeGenerator *cg,
                                   AnalysisResultConstRawPtr ar,
                                   SimpleFunctionCallPtr call, int mode) {
  if (!cg && mode <= 1 && Option::WholeProgram) {
    ExpressionListPtr params = call->getParams();
    if (params && params->getCount() == 1) {
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
  }

  return ExpressionPtr();
}

}

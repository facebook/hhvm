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

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void SimpleFunctionCall::setupScopes(AnalysisResultConstRawPtr ar) {
  if (m_class || hasStaticClass()) {
    resolveClass();
  }
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
    setupScopes(ar);

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
              break;
            case FunType::InterfaceExists:
            case FunType::ClassExists:
              break;
            default:
              assert(false);
          }
        }
      }
    }

    if (m_params) m_params->markParams();
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

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/statement/method_statement.h"
#include <folly/Conv.h>
#include <map>
#include <set>
#include "hphp/compiler/statement/return_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/statement/try_statement.h"
#include "hphp/compiler/statement/label_statement.h"
#include "hphp/compiler/statement/goto_statement.h"
#include "hphp/compiler/statement/exp_statement.h"
#include "hphp/compiler/statement/switch_statement.h"
#include "hphp/compiler/statement/case_statement.h"
#include "hphp/compiler/statement/catch_statement.h"

#include "hphp/compiler/expression/modifier_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/closure_expression.h"

#include "hphp/compiler/analysis/ast_walker.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/function_scope.h"

#include "hphp/compiler/option.h"
#include "hphp/compiler/builtin_symbols.h"

#include "hphp/parser/parser.h"
#include "hphp/util/text-util.h"

using namespace HPHP;
using std::map;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

MethodStatement::MethodStatement
(STATEMENT_CONSTRUCTOR_BASE_PARAMETERS,
 ModifierExpressionPtr modifiers, bool ref, const string &name,
 ExpressionListPtr params, TypeAnnotationPtr retTypeAnnotation,
 StatementListPtr stmt, int attr, const string &docComment,
 ExpressionListPtr attrList, bool method /* = true */)
  : Statement(STATEMENT_CONSTRUCTOR_BASE_PARAMETER_VALUES)
  , m_method(method)
  , m_ref(ref)
  , m_hasCallToGetArgs(false)
  , m_mayCallSetFrameMetadata(false)
  , m_attribute(attr)
  , m_cppLength(-1)
  , m_autoPropCount(0)
  , m_modifiers(modifiers)
  , m_originalName(name)
  , m_params(params)
  , m_retTypeAnnotation(retTypeAnnotation)
  , m_stmt(stmt)
  , m_docComment(docComment)
  , m_attrList(attrList)
{
}

StatementPtr MethodStatement::clone() {
  MethodStatementPtr stmt(new MethodStatement(*this));
  stmt->m_stmt = Clone(m_stmt);
  stmt->m_params = Clone(m_params);
  stmt->m_modifiers = Clone(m_modifiers);
  return stmt;
}

bool MethodStatement::isNamed(const char* name) const {
  return !strcasecmp(m_originalName.c_str(), name);
}

string MethodStatement::getOriginalFullName() const {
  if (!m_method) return m_originalName;
  return getClassScope()->getOriginalName() + "::" + m_originalName;
}

bool MethodStatement::isRef(int index /* = -1 */) const {
  if (index == -1) return m_ref;
  assert(index >= 0 && index < m_params->getCount());
  ParameterExpressionPtr param =
    dynamic_pointer_cast<ParameterExpression>((*m_params)[index]);
  return param->isRef();
}

bool MethodStatement::isSystem() const {
  return getFunctionScope()->isSystem();
}

int MethodStatement::getRecursiveCount() const {
  return m_stmt ? m_stmt->getRecursiveCount() : 0;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

FunctionScopePtr MethodStatement::onInitialParse(AnalysisResultConstPtr ar,
                                                 FileScopePtr fs) {
  ConstructPtr self = shared_from_this();
  int minParam = 0, numDeclParam = 0;
  bool hasRef = false;
  bool hasVariadicParam = false;
  if (m_params) {
    std::set<string> names, allDeclNames;
    int i = 0;
    numDeclParam = m_params->getCount();
    ParameterExpressionPtr lastParam =
      dynamic_pointer_cast<ParameterExpression>(
        (*m_params)[numDeclParam - 1]);
    hasVariadicParam = lastParam->isVariadic();
    if (hasVariadicParam) {
      allDeclNames.insert(lastParam->getName());
      // prevent the next loop from visiting the variadic param and testing
      // its optionality. parsing ensures that the variadic capture param
      // can *only* be the last param.
      i = numDeclParam - 2;
    } else {
      i = numDeclParam - 1;
    }
    for (; i >= 0; --i) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
      assert(!param->isVariadic());
      if (param->isRef()) { hasRef = true; }
      if (!param->isOptional()) {
        if (!minParam) minParam = i + 1;
      } else if (minParam && !param->hasTypeHint()) {
        Compiler::Error(Compiler::RequiredAfterOptionalParam, param);
      }
      allDeclNames.insert(param->getName());
    }

    // For the purpose of naming (having entered the the function body), a
    // variadic capture param acts as any other variable.
    for (i = (numDeclParam - 1); i >= 0; --i) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
      if (names.find(param->getName()) != names.end()) {
        Compiler::Error(Compiler::RedundantParameter, param);
        for (int j = 0; j < 1000; j++) {
          string name = param->getName() + folly::to<string>(j);
          if (names.find(name) == names.end() &&
              allDeclNames.find(name) == allDeclNames.end()) {
            param->rename(name);
            break;
          }
        }
      }
      names.insert(param->getName());
    }
  }

  if (hasRef || m_ref) {
    m_attribute |= FileScope::ContainsReference;
  }
  if (hasVariadicParam) {
    m_attribute |= FileScope::VariadicArgumentParam;
  }

  vector<UserAttributePtr> attrs;
  if (m_attrList) {
    for (int i = 0; i < m_attrList->getCount(); ++i) {
      UserAttributePtr a =
        dynamic_pointer_cast<UserAttribute>((*m_attrList)[i]);
      attrs.push_back(a);
    }
  }

  StatementPtr stmt = dynamic_pointer_cast<Statement>(shared_from_this());
  auto funcScope =
    std::make_shared<FunctionScope>(ar, m_method, m_originalName,
                                    stmt, m_ref, minParam, numDeclParam,
                                    m_modifiers, m_attribute, m_docComment,
                                    fs, attrs);
  if (!m_stmt) {
    funcScope->setVirtual();
  }
  setBlockScope(funcScope);

  funcScope->setParamCounts(ar, -1, -1);

  return funcScope;
}

void MethodStatement::onParseRecur(AnalysisResultConstPtr ar,
                                   FileScopeRawPtr fileScope,
                                   ClassScopePtr classScope) {
  checkParameters(fileScope);

  FunctionScopeRawPtr funcScope = getFunctionScope();
  funcScope->setOuterScope(classScope);
  const bool isNative = funcScope->isNative();
  if (m_modifiers) {
    if ((m_modifiers->isExplicitlyPublic() +
         m_modifiers->isProtected() +
         m_modifiers->isPrivate()) > 1) {
      m_modifiers->parseTimeFatal(
        fileScope,
        Compiler::InvalidAttribute,
        Strings::PICK_ACCESS_MODIFIER
      );
    }

    if (m_modifiers->hasDuplicates()) {
      m_modifiers->parseTimeFatal(
        fileScope,
        Compiler::InvalidAttribute,
        Strings::PICK_ACCESS_MODIFIER);
    }

    if (classScope->isInterface()) {
      if (m_modifiers->isProtected() || m_modifiers->isPrivate() ||
          m_modifiers->isAbstract()  || m_modifiers->isFinal() ||
          isNative) {
        m_modifiers->parseTimeFatal(
          fileScope,
          Compiler::InvalidAttribute,
          "Access type for interface method %s::%s() must be omitted",
          classScope->getOriginalName().c_str(), getOriginalName().c_str());
      }
      if (m_modifiers->isAsync()) {
        m_modifiers->parseTimeFatal(
          fileScope,
          Compiler::InvalidAttribute,
          Strings::ASYNC_WITHOUT_BODY,
          "interface", classScope->getOriginalName().c_str(),
          getOriginalName().c_str()
        );
      }
      if (getStmts()) {
        getStmts()->parseTimeFatal(
          fileScope,
          Compiler::InvalidMethodDefinition,
          "Interface method %s::%s() cannot contain body",
          classScope->getOriginalName().c_str(),
          getOriginalName().c_str());
      }
    }
    if (m_modifiers->isAbstract()) {
      if (funcScope->userAttributes().count("__Memoize")) {
        m_modifiers->parseTimeFatal(
          fileScope,
          Compiler::InvalidAttribute,
          "Abstract method %s::%s cannot be memoized",
          classScope->getOriginalName().c_str(),
          getOriginalName().c_str());
      }
      if (m_modifiers->isPrivate() || m_modifiers->isFinal() || isNative) {
        m_modifiers->parseTimeFatal(
          fileScope,
          Compiler::InvalidAttribute,
          "Cannot declare abstract method %s::%s() %s",
          classScope->getOriginalName().c_str(),
          getOriginalName().c_str(),
          m_modifiers->isPrivate() ? "private" :
           (m_modifiers->isFinal() ? "final" : "native"));
      }
      if (!classScope->isInterface() && !classScope->isAbstract()) {
        /* note that classScope->isAbstract() returns true for traits */
        m_modifiers->parseTimeFatal(fileScope,
                                    Compiler::InvalidAttribute,
                                    "Class %s contains abstract method %s and "
                                    "must therefore be declared abstract",
                                    classScope->getOriginalName().c_str(),
                                    getOriginalName().c_str());
      }
      if (getStmts()) {
        parseTimeFatal(fileScope,
                       Compiler::InvalidAttribute,
                       "Abstract method %s::%s() cannot contain body",
                       classScope->getOriginalName().c_str(),
                       getOriginalName().c_str());
      }
      if (m_modifiers->isAsync()) {
        m_modifiers->parseTimeFatal(
          fileScope,
          Compiler::InvalidAttribute,
          Strings::ASYNC_WITHOUT_BODY,
          "abstract", classScope->getOriginalName().c_str(),
          getOriginalName().c_str()
        );
      }
    }
    if (!m_modifiers->isStatic() && classScope->isStaticUtil()) {
      m_modifiers->parseTimeFatal(
        fileScope,
        Compiler::InvalidAttribute,
        "Class %s contains non-static method %s and "
        "therefore cannot be declared 'abstract final'",
        classScope->getOriginalName().c_str(),
        getOriginalName().c_str()
      );
    }

    if (isNative) {
      if (getStmts()) {
        parseTimeFatal(fileScope,
                       Compiler::InvalidAttribute,
                       "Native method %s::%s() cannot contain body",
                       classScope->getOriginalName().c_str(),
                       getOriginalName().c_str());
      }
      auto is_ctordtor = isNamed("__construct") || isNamed("__destruct");
      if (!m_retTypeAnnotation && !is_ctordtor) {
        parseTimeFatal(fileScope,
                       Compiler::InvalidAttribute,
                       "Native method %s::%s() must have a return type hint",
                       classScope->getOriginalName().c_str(),
                       getOriginalName().c_str());
      } else if (m_retTypeAnnotation &&
                 is_ctordtor &&
                (m_retTypeAnnotation->dataType() != KindOfNull)) {
        parseTimeFatal(fileScope,
                       Compiler::InvalidAttribute,
                       "Native method %s::%s() must return void",
                       classScope->getOriginalName().c_str(),
                       getOriginalName().c_str());
      }
    }
  }
  if ((!m_modifiers || !m_modifiers->isAbstract()) &&
      !getStmts() && !classScope->isInterface() && !isNative) {
    parseTimeFatal(fileScope,
                   Compiler::InvalidAttribute,
                   "Non-abstract method %s::%s() must contain body",
                   classScope->getOriginalName().c_str(),
                   getOriginalName().c_str());
  }

  classScope->addFunction(ar, fileScope, funcScope);

  setSpecialMethod(fileScope, classScope);

  if (Option::DynamicInvokeFunctions.count(getOriginalFullName())) {
    funcScope->setDynamicInvoke();
  }
  if (m_params) {
    auto nParams = m_params->getCount();
    for (int i = 0; i < nParams; i++) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
      param->parseHandler(fileScope, classScope);
      // Variadic capture params don't need types because they'll
      // be treated as Arrays as far as HNI is concerned.
      if (isNative && !param->hasUserType() && !param->isVariadic()) {
        parseTimeFatal(fileScope,
                       Compiler::InvalidAttribute,
                       "Native method calls must have type hints on all args");
      }
    }
  }
  FunctionScope::RecordFunctionInfo(m_originalName, funcScope);
}

void MethodStatement::fixupSelfAndParentTypehints(ClassScopePtr scope) {
  if (m_params) {
    for (int i = 0; i < m_params->getCount(); i++) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
      param->fixupSelfAndParentTypehints(scope);
    }
  }
}

void MethodStatement::setSpecialMethod(FileScopeRawPtr fileScope,
                                       ClassScopePtr classScope) {
  if (m_originalName.size() < 2 || m_originalName.substr(0,2) != "__") {
    return;
  }
  auto name = toLower(m_originalName);
  int numArgs = -1;
  bool isStatic = false;
  if (name == "__construct") {
    classScope->setAttribute(ClassScope::HasConstructor);
  } else if (name == "__destruct") {
    classScope->setAttribute(ClassScope::HasDestructor);
    if (m_params && m_params->getCount()) {
      parseTimeFatal(fileScope,
                     Compiler::InvalidMagicMethod,
                     "Method %s cannot take any arguments",
                     getOriginalFullName().c_str());
    }
  } else if (name == "__get") {
    classScope->setAttribute(ClassScope::HasUnknownPropGetter);
    numArgs = 1;
  } else if (name == "__set") {
    classScope->setAttribute(ClassScope::HasUnknownPropSetter);
    numArgs = 2;
  } else if (name == "__isset") {
    classScope->setAttribute(ClassScope::HasUnknownPropTester);
    numArgs = 1;
  } else if (name == "__unset") {
    classScope->setAttribute(ClassScope::HasPropUnsetter);
    numArgs = 1;
  } else if (name == "__call") {
    classScope->setAttribute(ClassScope::HasUnknownMethodHandler);
    numArgs = 2;
  } else if (name == "__callstatic") {
    classScope->setAttribute(ClassScope::HasUnknownStaticMethodHandler);
    numArgs = 2;
    isStatic = true;
  } else if (name == "__invoke") {
    classScope->setAttribute(ClassScope::HasInvokeMethod);
  } else if (name == "__tostring") {
    numArgs = 0;
  } else if (name == "__clone") {
    if (m_params && m_params->getCount()) {
      parseTimeFatal(fileScope,
                     Compiler::InvalidMagicMethod,
                     "Method %s cannot accept any arguments",
                     getOriginalFullName().c_str());
    }
  }
  if (numArgs >= 0) {
    // Fatal if the number of arguments is wrong
    int n = m_params ? m_params->getCount() : 0;
    if (numArgs != n) {
      parseTimeFatal(fileScope,
                     Compiler::InvalidMagicMethod,
                     "Method %s() must take exactly %d argument%s",
                     getOriginalFullName().c_str(),
                     numArgs, (numArgs == 1) ? "" : "s");
    }
    // Fatal if any arguments are pass by reference
    if (m_params && hasRefParam()) {
      parseTimeFatal(fileScope,
                     Compiler::InvalidMagicMethod,
                     "Method %s() cannot take arguments by reference",
                     getOriginalFullName().c_str());
    }
    // Fatal if any arguments are variadic
    if (m_params && getFunctionScope()->hasVariadicParam()) {
      parseTimeFatal(fileScope,
                     Compiler::InvalidMagicMethod,
                     "Method %s() cannot take a variadic argument",
                     getOriginalFullName().c_str());
    }
    // Fatal if protected/private or if the staticness is wrong
    if (m_modifiers->isProtected() || m_modifiers->isPrivate() ||
        m_modifiers->isStatic() != isStatic) {
      parseTimeFatal(
        fileScope,
        Compiler::InvalidMagicMethod,
        "Method %s() must have public visibility and %sbe static",
        getOriginalFullName().c_str(),
        isStatic ? "" : "cannot ");
    }
  }
}

void MethodStatement::addTraitMethodToScope(AnalysisResultConstPtr ar,
                                            ClassScopePtr classScope) {
  FunctionScopeRawPtr funcScope = getFunctionScope();
  FileScopeRawPtr fileScope = getFileScope();
  classScope->addFunction(ar, fileScope, funcScope);
  setSpecialMethod(fileScope, classScope);
  FunctionScope::RecordFunctionInfo(m_originalName, funcScope);
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

int MethodStatement::getLocalEffects() const {
  if (m_method) return NoEffect;
  FunctionScopeRawPtr scope = getFunctionScope();
  return scope->isVolatile() ? OtherEffect | CanThrow : NoEffect;
}

void MethodStatement::analyzeProgram(AnalysisResultPtr ar) {
  FunctionScopeRawPtr funcScope = getFunctionScope();

  if (m_params) {
    m_params->analyzeProgram(ar);
  }

  if (m_stmt) m_stmt->analyzeProgram(ar);

  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    if (Option::AllDynamic ||
        Option::IsDynamicFunction(m_method, m_originalName)) {
      funcScope->setDynamic();
    }
  }
}

ConstructPtr MethodStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_modifiers;
    case 1:
      return m_params;
    case 2:
      return m_stmt;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int MethodStatement::getKidCount() const {
  return 3;
}

void MethodStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_modifiers = dynamic_pointer_cast<ModifierExpression>(cp);
      break;
    case 1:
      m_params = dynamic_pointer_cast<ExpressionList>(cp);
      break;
    case 2:
      m_stmt = dynamic_pointer_cast<StatementList>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

void MethodStatement::outputCodeModel(CodeGenerator &cg) {
  auto isAnonymous = ParserBase::IsClosureName(m_originalName);
  auto numProps = 4;
  if (m_attrList != nullptr) numProps++;
  if (m_ref) numProps++;
  if (m_params != nullptr) numProps++;
  if (m_retTypeAnnotation != nullptr) numProps++;
  if (!m_docComment.empty()) numProps++;
  cg.printObjectHeader("FunctionStatement", numProps);
  if (m_attrList != nullptr) {
    cg.printPropertyHeader("attributes");
    cg.printExpressionVector(m_attrList);
  }
  cg.printPropertyHeader("modifiers");
  m_modifiers->outputCodeModel(cg);
  if (m_ref) {
    cg.printPropertyHeader("returnsReference");
    cg.printBool(true);
  }
  cg.printPropertyHeader("name");
  cg.printValue(isAnonymous ? "" : m_originalName);
  //TODO: type parameters (task 3262469)
  if (m_params != nullptr) {
    cg.printPropertyHeader("parameters");
    cg.printExpressionVector(m_params);
  }
  if (m_retTypeAnnotation != nullptr) {
    cg.printPropertyHeader("returnType");
    m_retTypeAnnotation->outputCodeModel(cg);
  }
  cg.printPropertyHeader("block");
  if (m_stmt != nullptr) {
    auto stmt = m_stmt;
    if (m_autoPropCount > 0) {
      stmt = static_pointer_cast<StatementList>(stmt->clone());
      for (int i = m_autoPropCount; i > 0; i--) {
        stmt->removeElement(0);
      }
    }
    cg.printAsEnclosedBlock(stmt);
  } else {
    cg.printAsBlock(nullptr);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  if (!m_docComment.empty()) {
    cg.printPropertyHeader("comments");
    cg.printValue(m_docComment);
  }
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void MethodStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  FunctionScopeRawPtr funcScope = getFunctionScope();

  m_modifiers->outputPHP(cg, ar);
  cg_printf("function ");
  if (m_ref) cg_printf("&");
  if (!ParserBase::IsClosureName(m_originalName)) {
    cg_printf("%s", m_originalName.c_str());
  }
  cg_printf("(");
  if (m_params) m_params->outputPHP(cg, ar);
  if (m_stmt) {
    cg_indentBegin(") {\n");
    funcScope->outputPHP(cg, ar);
    m_stmt->outputPHP(cg, ar);
    cg_indentEnd("}\n");
  } else {
    cg_printf(");\n");
  }
}

bool MethodStatement::hasRefParam() {
  for (int i = 0; i < m_params->getCount(); i++) {
    ParameterExpressionPtr param =
      dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
    if (param->isRef()) return true;
  }
  return false;
}

void MethodStatement::checkParameters(FileScopeRawPtr scope) {
  // only allow parameter modifiers (public, private, protected)
  // on constructor for promotion
  if (!m_params) {
    return;
  }
  bool isCtor = isNamed("__construct");
  for (int i = 0; i < m_params->getCount(); i++) {
    auto param =
      dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
    switch (param->getModifier()) {
    case 0:
      continue;
    case T_PUBLIC:
    case T_PRIVATE:
    case T_PROTECTED:
      if (isCtor) {
        m_autoPropCount++;
        continue;
      }
    default:
      if (isCtor) {
        param->parseTimeFatal(scope,
                              Compiler::InvalidAttribute,
                              "Invalid modifier on __construct, only public, "
                              "private or protected allowed");
      } else {
        param->parseTimeFatal(scope,
                              Compiler::InvalidAttribute,
                              "Parameters modifiers not allowed on methods");
      }
    }
  }
}

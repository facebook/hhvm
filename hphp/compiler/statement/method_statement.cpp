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
#include "hphp/compiler/analysis/alias_manager.h"

#include "hphp/runtime/base/complex-types.h"

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
  : Statement(STATEMENT_CONSTRUCTOR_BASE_PARAMETER_VALUES),
    m_method(method), m_ref(ref), m_hasCallToGetArgs(false), m_attribute(attr),
    m_cppLength(-1), m_autoPropCount(0), m_modifiers(modifiers),
    m_originalName(name), m_params(params),
    m_retTypeAnnotation(retTypeAnnotation), m_stmt(stmt),
    m_docComment(docComment), m_attrList(attrList) {
  m_name = toLower(name);
  checkParameters();
}

MethodStatement::MethodStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 ModifierExpressionPtr modifiers, bool ref, const string &name,
 ExpressionListPtr params, TypeAnnotationPtr retTypeAnnotation,
 StatementListPtr stmt, int attr, const string &docComment,
 ExpressionListPtr attrList, bool method /* = true */)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(MethodStatement)),
    m_method(method), m_ref(ref), m_hasCallToGetArgs(false), m_attribute(attr),
    m_cppLength(-1), m_autoPropCount(0), m_modifiers(modifiers),
    m_originalName(name), m_params(params),
    m_retTypeAnnotation(retTypeAnnotation), m_stmt(stmt),
    m_docComment(docComment), m_attrList(attrList) {
  m_name = toLower(name);
  checkParameters();
}

StatementPtr MethodStatement::clone() {
  MethodStatementPtr stmt(new MethodStatement(*this));
  stmt->m_stmt = Clone(m_stmt);
  stmt->m_params = Clone(m_params);
  stmt->m_modifiers = Clone(m_modifiers);
  return stmt;
}

string MethodStatement::getFullName() const {
  if (m_className.empty()) return m_name;
  return m_className + "::" + m_name;
}

string MethodStatement::getOriginalFullName() const {
  if (m_originalClassName.empty()) return m_originalName;
  return m_originalClassName + "::" + m_originalName;
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
  int minParam, maxParam;
  ConstructPtr self = shared_from_this();
  minParam = maxParam = 0;
  bool hasRef = false;
  if (m_params) {
    std::set<string> names, allDeclNames;
    int i = 0;
    maxParam = m_params->getCount();
    for (i = maxParam; i--; ) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
      if (param->isRef()) hasRef = true;
      if (!param->isOptional()) {
        if (!minParam) minParam = i + 1;
      } else if (minParam && !param->hasTypeHint()) {
        Compiler::Error(Compiler::RequiredAfterOptionalParam, param);
      }
      allDeclNames.insert(param->getName());
    }

    for (i = maxParam-1; i >= 0; i--) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
      if (names.find(param->getName()) != names.end()) {
        Compiler::Error(Compiler::RedundantParameter, param);
        for (int j = 0; j < 1000; j++) {
          string name = param->getName() + lexical_cast<string>(j);
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

  vector<UserAttributePtr> attrs;
  if (m_attrList) {
    for (int i = 0; i < m_attrList->getCount(); ++i) {
      UserAttributePtr a =
        dynamic_pointer_cast<UserAttribute>((*m_attrList)[i]);
      attrs.push_back(a);
    }
  }

  StatementPtr stmt = dynamic_pointer_cast<Statement>(shared_from_this());
  FunctionScopePtr funcScope
    (new FunctionScope(ar, m_method, m_name, stmt, m_ref, minParam, maxParam,
                       m_modifiers, m_attribute, m_docComment, fs, attrs));
  if (!m_stmt) {
    funcScope->setVirtual();
  }
  setBlockScope(funcScope);

  funcScope->setParamCounts(ar, -1, -1);

  if (funcScope->isNative()) {
    funcScope->setReturnType(
      ar, Type::FromDataType(m_retTypeAnnotation->dataType(), Type::Variant));
  }

  return funcScope;
}

void MethodStatement::onParseRecur(AnalysisResultConstPtr ar,
                                   ClassScopePtr classScope) {

  FunctionScopeRawPtr fs = getFunctionScope();
  const bool isNative = fs->isNative();
  if (m_modifiers) {
    if ((m_modifiers->isExplicitlyPublic() +
         m_modifiers->isProtected() +
         m_modifiers->isPrivate()) > 1) {
      m_modifiers->parseTimeFatal(
        Compiler::InvalidAttribute,
        "%s: method %s::%s()",
        Strings::PICK_ACCESS_MODIFIER,
        classScope->getOriginalName().c_str(),
        getOriginalName().c_str()
      );
    }

    if (classScope->isInterface()) {
      if (m_modifiers->isProtected() || m_modifiers->isPrivate() ||
          m_modifiers->isAbstract()  || m_modifiers->isFinal() ||
          isNative) {
        m_modifiers->parseTimeFatal(
          Compiler::InvalidAttribute,
          "Access type for interface method %s::%s() must be omitted",
          classScope->getOriginalName().c_str(), getOriginalName().c_str());
      }
      if (m_modifiers->isAsync()) {
        m_modifiers->parseTimeFatal(
          Compiler::InvalidAttribute,
          Strings::ASYNC_WITHOUT_BODY,
          "interface", classScope->getOriginalName().c_str(),
          getOriginalName().c_str()
        );
      }
      if (getStmts()) {
        getStmts()->parseTimeFatal(
          Compiler::InvalidMethodDefinition,
          "Interface method %s::%s() cannot contain body",
          classScope->getOriginalName().c_str(),
          getOriginalName().c_str());
      }
    }
    if (m_modifiers->isAbstract()) {
      if (m_modifiers->isPrivate() || m_modifiers->isFinal() || isNative) {
        m_modifiers->parseTimeFatal(
          Compiler::InvalidAttribute,
          "Cannot declare abstract method %s::%s() %s",
          classScope->getOriginalName().c_str(),
          getOriginalName().c_str(),
          m_modifiers->isPrivate() ? "private" :
           (m_modifiers->isFinal() ? "final" : "native"));
      }
      if (!classScope->isInterface() && !classScope->isAbstract()) {
        /* note that classScope->isAbstract() returns true for traits */
        m_modifiers->parseTimeFatal(Compiler::InvalidAttribute,
                                    "Class %s contains abstract method %s and "
                                    "must therefore be declared abstract",
                                    classScope->getOriginalName().c_str(),
                                    getOriginalName().c_str());
      }
      if (getStmts()) {
        parseTimeFatal(Compiler::InvalidAttribute,
                       "Abstract method %s::%s() cannot contain body",
                       classScope->getOriginalName().c_str(),
                       getOriginalName().c_str());
      }
      if (m_modifiers->isAsync()) {
        m_modifiers->parseTimeFatal(
          Compiler::InvalidAttribute,
          Strings::ASYNC_WITHOUT_BODY,
          "abstract", classScope->getOriginalName().c_str(),
          getOriginalName().c_str()
        );
      }
    }
    if (isNative) {
      if (getStmts()) {
        parseTimeFatal(Compiler::InvalidAttribute,
                       "Native method %s::%s() cannot contain body",
                       classScope->getOriginalName().c_str(),
                       getOriginalName().c_str());
      }
      if (!m_retTypeAnnotation) {
        parseTimeFatal(Compiler::InvalidAttribute,
                       "Native method %s::%s() must have a return type hint",
                       classScope->getOriginalName().c_str(),
                       getOriginalName().c_str());
      }
    }
  }
  if ((!m_modifiers || !m_modifiers->isAbstract()) &&
      !getStmts() && !classScope->isInterface() && !isNative) {
    parseTimeFatal(Compiler::InvalidAttribute,
                   "Non-abstract method %s::%s() must contain body",
                   classScope->getOriginalName().c_str(),
                   getOriginalName().c_str());
  }

  classScope->addFunction(ar, fs);

  m_className = classScope->getName();
  m_originalClassName = classScope->getOriginalName();

  setSpecialMethod(classScope);

  if (Option::DynamicInvokeFunctions.find(getFullName()) !=
      Option::DynamicInvokeFunctions.end()) {
    fs->setDynamicInvoke();
  }
  if (m_params) {
    for (int i = 0; i < m_params->getCount(); i++) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
      param->parseHandler(classScope);
      if (isNative && !param->hasUserType()) {
        parseTimeFatal(Compiler::InvalidAttribute,
                       "Native method calls must have type hints on all args");
      }
    }
  }
  FunctionScope::RecordFunctionInfo(m_name, fs);
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

void MethodStatement::setSpecialMethod(ClassScopePtr classScope) {
  if (m_name.size() < 2 || m_name.substr(0,2) != "__") {
    return;
  }
  int numArgs = -1;
  bool isStatic = false;
  if (m_name == "__construct") {
    classScope->setAttribute(ClassScope::HasConstructor);
  } else if (m_name == "__destruct") {
    classScope->setAttribute(ClassScope::HasDestructor);
  } else if (m_name == "__call") {
    classScope->setAttribute(ClassScope::HasUnknownMethodHandler);
    numArgs = 2;
  } else if (m_name == "__get") {
    classScope->setAttribute(ClassScope::HasUnknownPropGetter);
    numArgs = 1;
  } else if (m_name == "__set") {
    classScope->setAttribute(ClassScope::HasUnknownPropSetter);
    numArgs = 2;
  } else if (m_name == "__isset") {
    classScope->setAttribute(ClassScope::HasUnknownPropTester);
    numArgs = 1;
  } else if (m_name == "__unset") {
    classScope->setAttribute(ClassScope::HasPropUnsetter);
    numArgs = 1;
  } else if (m_name == "__call") {
    classScope->setAttribute(ClassScope::HasUnknownMethodHandler);
    numArgs = 2;
  } else if (m_name == "__callstatic") {
    classScope->setAttribute(ClassScope::HasUnknownStaticMethodHandler);
    numArgs = 2;
    isStatic = true;
  } else if (m_name == "__invoke") {
    classScope->setAttribute(ClassScope::HasInvokeMethod);
  } else if (m_name == "__tostring") {
    numArgs = 0;
  }
  if (numArgs >= 0) {
    // Fatal if the number of arguments is wrong
    int n = m_params ? m_params->getCount() : 0;
    if (numArgs != n) {
      parseTimeFatal(Compiler::InvalidMagicMethod,
        "Method %s::%s() must take exactly %d argument%s",
        m_originalClassName.c_str(), m_originalName.c_str(),
        numArgs, (numArgs == 1) ? "" : "s");
    }
    // Fatal if any arguments are pass by reference
    if (m_params && hasRefParam()) {
      parseTimeFatal(Compiler::InvalidMagicMethod,
        "Method %s::%s() cannot take arguments by reference",
        m_originalClassName.c_str(), m_originalName.c_str());
    }
    // Fatal if protected/private or if the staticness is wrong
    if (m_modifiers->isProtected() || m_modifiers->isPrivate() ||
        m_modifiers->isStatic() != isStatic) {
      parseTimeFatal(Compiler::InvalidMagicMethod,
        "Method %s::%s() must have public visibility and %sbe static",
        m_originalClassName.c_str(), m_originalName.c_str(),
        isStatic ? "" : "cannot ");
    }
  }
}

void MethodStatement::addTraitMethodToScope(AnalysisResultConstPtr ar,
                                            ClassScopePtr classScope) {
  FunctionScopeRawPtr funcScope = getFunctionScope();
  classScope->addFunction(ar, funcScope);
  setSpecialMethod(classScope);
  FunctionScope::RecordFunctionInfo(m_name, funcScope);
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
    funcScope->setParamSpecs(ar);

    if (Option::IsDynamicFunction(m_method, m_name) || Option::AllDynamic) {
      funcScope->setDynamic();
    }
    // TODO: this may have to expand to a concept of "virtual" functions...
    if (m_method) {
      funcScope->disableInline();
      if (m_name.length() > 2 && m_name.substr(0,2) == "__") {
        bool magic = true;
        int paramCount = 0;
        if (m_name == "__destruct") {
          funcScope->setOverriding(Type::Variant);
        } else if (m_name == "__call") {
          funcScope->setOverriding(Type::Variant, Type::String, Type::Array);
          paramCount = 2;
        } else if (m_name == "__set") {
          funcScope->setOverriding(Type::Variant, Type::String, Type::Variant);
          paramCount = 2;
        } else if (m_name == "__get") {
          funcScope->setOverriding(Type::Variant, Type::String);
          paramCount = 1;
        } else if (m_name == "__isset") {
          funcScope->setOverriding(Type::Boolean, Type::String);
          paramCount = 1;
        } else if (m_name == "__unset") {
          funcScope->setOverriding(Type::Variant, Type::String);
          paramCount = 1;
        } else if (m_name == "__sleep") {
          funcScope->setOverriding(Type::Variant);
        } else if (m_name == "__wakeup") {
          funcScope->setOverriding(Type::Variant);
        } else if (m_name == "__set_state") {
          funcScope->setOverriding(Type::Variant, Type::Variant);
          paramCount = 1;
        } else if (m_name == "__tostring") {
          // do nothing
        } else if (m_name == "__clone") {
          funcScope->setOverriding(Type::Variant);
        } else {
          paramCount = -1;
          if (m_name != "__construct") {
            magic = false;
          }
        }
        if (paramCount >= 0 && paramCount != funcScope->getMaxParamCount()) {
          Compiler::Error(Compiler::InvalidMagicMethod, shared_from_this());
          magic = false;
        }
        if (magic) funcScope->setMagicMethod();
      }
      // ArrayAccess methods
      else if (m_name.length() > 6 && m_name.substr(0, 6) == "offset") {
        if (m_name == "offsetexists") {
          funcScope->setOverriding(Type::Boolean, Type::Variant);
        } else if (m_name == "offsetget") {
          funcScope->setOverriding(Type::Variant, Type::Variant);
        } else if (m_name == "offsetset") {
          funcScope->setOverriding(Type::Variant, Type::Variant, Type::Variant);
        } else if (m_name == "offsetunset") {
          funcScope->setOverriding(Type::Variant, Type::Variant);
        }
      }
    }
  } else if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    TypePtr ret = funcScope->getReturnType();
    if (ret && ret->isSpecificObject()) {
      FileScopePtr fs = getFileScope();
      if (fs) fs->addClassDependency(ar, ret->getName());
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

void MethodStatement::inferTypes(AnalysisResultPtr ar) {
}

void MethodStatement::inferFunctionTypes(AnalysisResultPtr ar) {
  IMPLEMENT_INFER_AND_CHECK_ASSERT(getFunctionScope());

  FunctionScopeRawPtr funcScope = getFunctionScope();
  bool pseudoMain = funcScope->inPseudoMain();

  if (m_stmt && funcScope->isFirstPass()) {
    if (pseudoMain ||
        funcScope->getReturnType() ||
        m_stmt->hasRetExp()) {
      bool lastIsReturn = false;
      if (m_stmt->getCount()) {
        StatementPtr lastStmt = (*m_stmt)[m_stmt->getCount()-1];
        if (lastStmt->is(Statement::KindOfReturnStatement)) {
          lastIsReturn = true;
        }
      }
      if (!lastIsReturn) {
        ExpressionPtr constant =
          makeScalarExpression(ar, funcScope->inPseudoMain() ?
                               Variant(1) :
                               Variant(Variant::NullInit()));
        ReturnStatementPtr returnStmt =
          ReturnStatementPtr(
            new ReturnStatement(getScope(), getLabelScope(),
                                getLocation(), constant));
        m_stmt->addElement(returnStmt);
      }
    }
  }

  if (m_params) {
    m_params->inferAndCheck(ar, Type::Any, false);
  }

  if (m_stmt) {
    m_stmt->inferTypes(ar);
  }
}

///////////////////////////////////////////////////////////////////////////////

void MethodStatement::outputCodeModel(CodeGenerator &cg) {
  auto isAnonymous = ParserBase::IsClosureName(m_name);
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
  cg.printLocation(this->getLocation());
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
  if (!ParserBase::IsClosureName(m_name)) {
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

void MethodStatement::checkParameters() {
  // only allow parameter modifiers (public, private, protected)
  // on constructor for promotion
  if (!m_params) {
    return;
  }
  bool isCtor = m_name == "__construct";
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
        param->parseTimeFatal(Compiler::InvalidAttribute,
                              "Invalid modifier on __construct, only public, "
                              "private or protected allowed");
      } else {
        param->parseTimeFatal(Compiler::InvalidAttribute,
                              "Parameters modifiers not allowed on methods");
      }
    }
  }
}

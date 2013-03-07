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

#include <runtime/base/complex_types.h>

#include <compiler/statement/method_statement.h>
#include <compiler/statement/return_statement.h>
#include <compiler/statement/statement_list.h>
#include <compiler/statement/try_statement.h>
#include <compiler/statement/label_statement.h>
#include <compiler/statement/goto_statement.h>
#include <compiler/statement/exp_statement.h>
#include <compiler/statement/switch_statement.h>
#include <compiler/statement/case_statement.h>
#include <compiler/statement/catch_statement.h>

#include <compiler/expression/modifier_expression.h>
#include <compiler/expression/expression_list.h>
#include <compiler/expression/constant_expression.h>
#include <compiler/expression/parameter_expression.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/expression/simple_variable.h>

#include <compiler/analysis/ast_walker.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/function_scope.h>

#include <compiler/option.h>
#include <compiler/builtin_symbols.h>
#include <compiler/analysis/alias_manager.h>

#include <util/parser/parser.h>
#include <util/util.h>

using namespace HPHP;
using std::map;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

MethodStatement::MethodStatement
(STATEMENT_CONSTRUCTOR_BASE_PARAMETERS,
 ModifierExpressionPtr modifiers, bool ref, const string &name,
 ExpressionListPtr params, StatementListPtr stmt, int attr,
 const string &docComment, ExpressionListPtr attrList,
 bool method /* = true */)
  : Statement(STATEMENT_CONSTRUCTOR_BASE_PARAMETER_VALUES),
    m_method(method), m_ref(ref), m_attribute(attr),
    m_cppLength(-1), m_modifiers(modifiers),
    m_originalName(name), m_params(params), m_stmt(stmt),
    m_docComment(docComment), m_attrList(attrList) {
  m_name = Util::toLower(name);
}

MethodStatement::MethodStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 ModifierExpressionPtr modifiers, bool ref, const string &name,
 ExpressionListPtr params, StatementListPtr stmt, int attr,
 const string &docComment, ExpressionListPtr attrList,
 bool method /* = true */)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(MethodStatement)),
    m_method(method), m_ref(ref), m_attribute(attr), m_cppLength(-1),
    m_modifiers(modifiers), m_originalName(name),
    m_params(params), m_stmt(stmt),
    m_docComment(docComment), m_attrList(attrList) {
  m_name = Util::toLower(name);
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

string MethodStatement::getOriginalFullNameForInjection() const {
  FunctionScopeRawPtr funcScope = getFunctionScope();
  string injectionName;
  if (getGeneratorFunc()) {
    injectionName = funcScope->isClosureGenerator() ?
      m_originalName :
      m_originalName + "{continuation}";
  } else if (getOrigGeneratorFunc()) {
    bool needsOrig = !funcScope->getOrigGenFS()->isClosure();
    injectionName = needsOrig ?
      getOrigGeneratorFunc()->getOriginalName() :
      m_originalName;
  } else {
    injectionName = m_originalName;
  }
  return m_originalClassName.empty() ?
    injectionName :
    m_originalClassName + "::" + injectionName;
}

bool MethodStatement::isRef(int index /* = -1 */) const {
  if (index == -1) return m_ref;
  assert(index >= 0 && index < m_params->getCount());
  ParameterExpressionPtr param =
    dynamic_pointer_cast<ParameterExpression>((*m_params)[index]);
  return param->isRef();
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
  return funcScope;
}

void MethodStatement::onParseRecur(AnalysisResultConstPtr ar,
                                   ClassScopePtr classScope) {

  if (m_modifiers) {
    if (classScope->isInterface()) {
      if (m_modifiers->isProtected() || m_modifiers->isPrivate() ||
          m_modifiers->isAbstract()  || m_modifiers->isFinal()) {
        m_modifiers->parseTimeFatal(
          Compiler::InvalidAttribute,
          "Access type for interface method %s::%s() must be omitted",
          classScope->getOriginalName().c_str(), getOriginalName().c_str());
      }
    }
    if (m_modifiers->isAbstract()) {
      if (m_modifiers->isPrivate() || m_modifiers->isFinal()) {
        m_modifiers->parseTimeFatal(
          Compiler::InvalidAttribute,
          "Cannot declare abstract method %s::%s() %s",
          classScope->getOriginalName().c_str(),
          getOriginalName().c_str(),
          m_modifiers->isPrivate() ? "private" : "final");
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
    }
  }
  if ((!m_modifiers || !m_modifiers->isAbstract()) &&
      !getStmts() && !classScope->isInterface()) {
    parseTimeFatal(Compiler::InvalidAttribute,
                   "Non-abstract method %s::%s() must contain body",
                   classScope->getOriginalName().c_str(),
                   getOriginalName().c_str());
  }

  FunctionScopeRawPtr fs = getFunctionScope();

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
    if (funcScope->isGenerator()) {
      VariableTablePtr variables = funcScope->getVariables();
      Symbol *cont = variables->getSymbol(CONTINUATION_OBJECT_NAME);
      cont->setHidden();
      getOrigGeneratorFunc()->getFunctionScope()->addUse(
        funcScope, BlockScope::UseKindClosure);
      getOrigGeneratorFunc()->getFunctionScope()->setContainsBareThis(
        funcScope->containsBareThis(), funcScope->containsRefThis());
      getOrigGeneratorFunc()->getFunctionScope()->setContainsThis(
        funcScope->containsThis());
    }
    if (funcScope->isSepExtension() ||
        Option::IsDynamicFunction(m_method, m_name) || Option::AllDynamic) {
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
          funcScope->setOverriding(Type::String);
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
    if (!getFunctionScope()->usesLSB()) {
      if (StatementPtr orig = getOrigGeneratorFunc()) {
        orig->getFunctionScope()->clearUsesLSB();
      }
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
      m_modifiers = boost::dynamic_pointer_cast<ModifierExpression>(cp);
      break;
    case 1:
      m_params = boost::dynamic_pointer_cast<ExpressionList>(cp);
      break;
    case 2:
      m_stmt = boost::dynamic_pointer_cast<StatementList>(cp);
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
          makeConstant(ar, funcScope->inPseudoMain() ? "true" : "null");
        ReturnStatementPtr returnStmt =
          ReturnStatementPtr(
            new ReturnStatement(getScope(), getLocation(), constant));
        m_stmt->addElement(returnStmt);
      }
    }
  }

  if (m_params) {
    m_params->inferAndCheck(ar, Type::Any, false);
  }

  // must also include params and use vars if this is a generator. note: we are
  // OK reading the params from the AST nodes of the original generator
  // function, since we have the dependency links set up
  if (funcScope->isGenerator()) {
    // orig function params
    MethodStatementRawPtr m = getOrigGeneratorFunc();
    assert(m);

    VariableTablePtr variables = funcScope->getVariables();
    ExpressionListPtr params = m->getParams();
    if (params) {
      for (int i = 0; i < params->getCount(); i++) {
        ParameterExpressionPtr param =
          dynamic_pointer_cast<ParameterExpression>((*params)[i]);
        const string &name = param->getName();
        assert(!param->isRef() || param->getType()->is(Type::KindOfVariant));
        variables->addParamLike(name, param->getType(), ar, param,
                                funcScope->isFirstPass());
      }
    }

    // use vars
    ExpressionListPtr useVars = m->getFunctionScope()->getClosureVars();
    if (useVars) {
      for (int i = 0; i < useVars->getCount(); i++) {
        ParameterExpressionPtr param =
          dynamic_pointer_cast<ParameterExpression>((*useVars)[i]);
        const string &name = param->getName();
        assert(!param->isRef() || param->getType()->is(Type::KindOfVariant));
        variables->addParamLike(name, param->getType(), ar, param,
                                funcScope->isFirstPass());
      }
    }
  }

  if (m_stmt) {
    m_stmt->inferTypes(ar);
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void MethodStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  FunctionScopeRawPtr funcScope = getFunctionScope();

  m_modifiers->outputPHP(cg, ar);
  cg_printf(" function ");
  if (m_ref) cg_printf("&");
  if (!ParserBase::IsClosureOrContinuationName(m_name)) {
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

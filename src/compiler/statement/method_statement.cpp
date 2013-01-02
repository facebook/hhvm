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
  ASSERT(index >= 0 && index < m_params->getCount());
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

void MethodStatement::addParamRTTI(AnalysisResultPtr ar) {
  FunctionScopeRawPtr func = getFunctionScope();

  VariableTablePtr variables = func->getVariables();
  if (variables->getAttribute(VariableTable::ContainsDynamicVariable) ||
      variables->getAttribute(VariableTable::ContainsExtract)) {
    return;
  }
  for (int i = 0; i < m_params->getCount(); i++) {
    ParameterExpressionPtr param =
      dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
    const string &paramName = param->getName();
    if (variables->isLvalParam(paramName)) continue;
    TypePtr paramType = param->getActualType();
    if ((paramType->is(Type::KindOfVariant) ||
         paramType->is(Type::KindOfSome)) &&
        !param->isRef()) {
      param->setHasRTTI();
      ClassScopePtr cls = getClassScope();
      ar->addParamRTTIEntry(cls, func, paramName);
      const string funcId = ar->getFuncId(cls, func);
      ar->addRTTIFunction(funcId);
    }
  }
}

class TryingGotoFixer {
public:
  TryingGotoFixer(AnalysisResultPtr ar) : id(0), m_ar(ar) {}

  void fixTryingGotos(StatementPtr s) {
    findTryingGotosRecur(s);
    while (labelMap.size()) {
      LabelInfoMap::iterator it = labelMap.begin(), end = labelMap.end();
      while (it != end) {
        if (it->second.trys.size()) {
          TryStatementRawPtr t = it->second.trys.back();
          for (int i = it->second.gotos.size(); i--; ) {
            GotoInfo &gi = it->second.gotos[i];
            if (gi.trys.size() < it->second.trys.size() ||
                gi.trys[it->second.trys.size() - 1] != t) {
              // the goto is not nested in the same try as the label
              TryInfo & ti = tryMap[t];
              if (!ti.label) {
                ti.label = ++id;
              }
              string labelStr = lexical_cast<string>(ti.label);
              if (it->second.trys.size() > 1) {
                LabelInfo &li = labelMap[labelStr];
                li.trys = it->second.trys;
                li.trys.pop_back();
                li.gotos.push_back(gi);
              }

              int lid = gi.goto_stmt->getId();
              if (!lid) {
                int &id = labelIdMap[gi.goto_stmt->label()];
                if (!id) id = labelIdMap.size();
                lid = id;
                gi.goto_stmt->setId(lid);
                ti.targets[-lid] = it->first;
              } else {
                ti.targets[lid] = it->first;
              }

              gi.goto_stmt->setLabel(labelStr);
            }
          }
        }
        labelMap.erase(it++);
      }
    }
    fixTryingGotosRecur(s);
  }

  void findTryingGotosRecur(StatementPtr s) {
    if (!s || FunctionWalker::SkipRecurse(s)) return;
    switch (s->getKindOf()) {
      case Statement::KindOfLabelStatement:
        if (trys.size()) {
          LabelStatementRawPtr label_stmt(
            static_pointer_cast<LabelStatement>(s));
          LabelInfo &li = labelMap[label_stmt->label()];
          always_assert(!li.trys.size());
          li.trys = trys;
        }
        break;
      case Statement::KindOfGotoStatement: {
        GotoStatementRawPtr goto_stmt(
          static_pointer_cast<GotoStatement>(s));
        LabelInfo &li = labelMap[goto_stmt->label()];
        li.gotos.push_back(GotoInfo(goto_stmt, trys));
        break;
      }
      case Statement::KindOfTryStatement: {
        TryStatementRawPtr t(static_pointer_cast<TryStatement>(s));
        trys.push_back(t);
        findTryingGotosRecur(t->getBody());
        trys.pop_back();
        findTryingGotosRecur(t->getCatches());
        return;
      }
      default:
        break;
    }
    for (int i = s->getKidCount(); i--; ) {
      StatementPtr child(s->getNthStmt(i));
      if (child) {
        findTryingGotosRecur(child);
      }
    }
    return;
  }

  ExpressionPtr gotoVar(StatementPtr s) {
    SimpleVariablePtr sv(
      new SimpleVariable(
        s->getScope(), s->getLocation(), "0_gtid"));
    sv->updateSymbol(SimpleVariablePtr());
    sv->getSymbol()->setHidden();
    return sv;
  }

  StatementPtr replaceGoto(GotoStatementRawPtr goto_stmt, int id) {
    StatementListPtr sl(new StatementList(
                          goto_stmt->getScope(), goto_stmt->getLocation()));
    AssignmentExpressionPtr ae(
      new AssignmentExpression(
        goto_stmt->getScope(), goto_stmt->getLocation(),
        gotoVar(goto_stmt),
        goto_stmt->makeScalarExpression(m_ar, Variant(id > 0 ? id : 0)),
        false));
    ExpStatementPtr exp(
      new ExpStatement(goto_stmt->getScope(), goto_stmt->getLocation(), ae));
    sl->addElement(exp);
    sl->addElement(goto_stmt);
    return sl;
  }

  StatementPtr fixTryingGotosRecur(StatementPtr s) {
    if (FunctionWalker::SkipRecurse(s)) return StatementPtr();

    for (int i = s->getKidCount(); i--; ) {
      StatementPtr child(s->getNthStmt(i));
      if (child) {
        StatementPtr r = fixTryingGotosRecur(child);
        if (r) s->setNthKid(i, r);
      }
    }

    switch (s->getKindOf()) {
      case Statement::KindOfGotoStatement: {
        GotoStatementRawPtr goto_stmt(
          static_pointer_cast<GotoStatement>(s));
        if (int id = goto_stmt->getId()) {
          return replaceGoto(goto_stmt, id);
        }
        break;
      }
      case Statement::KindOfTryStatement: {
        TryStatementRawPtr t(static_pointer_cast<TryStatement>(s));
        TryInfo &ti = tryMap[t];
        StatementListPtr catches = t->getCatches();
        bool doTry = ti.targets.size();
        bool doCatch = catches->hasReachableLabel();
        if (!doTry && !doCatch) break;
        StatementListPtr sl(new StatementList(
                              s->getScope(), s->getLocation()));
        StatementListPtr newBody(new StatementList(
                                   s->getScope(), s->getLocation()));
        if (doTry) {
          StatementListPtr cases(new StatementList(
                                   s->getScope(), s->getLocation()));
          for (map<int,string>::iterator it = ti.targets.begin(),
                 end = ti.targets.end(); it != end; ++it) {
            StatementPtr g(new GotoStatement(
                             s->getScope(), s->getLocation(), it->second));
            if (it->first < 0) {
              g = replaceGoto(static_pointer_cast<GotoStatement>(g), 0);
            }
            CaseStatementPtr c(new CaseStatement(
                                 s->getScope(), s->getLocation(),
                                 s->makeScalarExpression(m_ar, abs(it->first)),
                                 g));
            cases->addElement(c);
          }
          SwitchStatementPtr sw(new SwitchStatement(
                                  s->getScope(), s->getLocation(),
                                  gotoVar(s), cases));

          newBody->addElement(sw);

          LabelStatementPtr lab(new LabelStatement(
                                  s->getScope(), s->getLocation(),
                                  lexical_cast<string>(ti.label)));
          sl->addElement(lab);
        }
        newBody->addElement(t->getBody());
        sl->addElement(StatementPtr(new TryStatement(
                                      s->getScope(), s->getLocation(),
                                      newBody, catches)));
        if (doCatch) {
          if (!ti.label) ti.label = ++id;
          string afterLab = lexical_cast<string>(ti.label) + "_a";
          newBody->addElement(StatementPtr(
                                new GotoStatement(
                                  s->getScope(), s->getLocation(), afterLab)));

          for (int i = 0, n = catches->getCount(); i < n; i++) {
            CatchStatementPtr c =
              static_pointer_cast<CatchStatement>((*catches)[i]);
            StatementPtr body = c->getStmt();
            string lab = lexical_cast<string>(ti.label) + "_c" +
              lexical_cast<string>(i);
            c->setStmt(StatementPtr(
                         new GotoStatement(
                           c->getScope(), c->getLocation(), lab)));
            StatementListPtr newBody(new StatementList(
                                       c->getScope(), c->getLocation()));
            newBody->addElement(StatementPtr(
                                  new LabelStatement(
                                    c->getScope(), c->getLocation(),
                                    lab)));
            if (body) newBody->addElement(body);
            if (i + 1 < n) {
              newBody->addElement(
                StatementPtr(new GotoStatement(
                               c->getScope(), c->getLocation(),
                               afterLab)));
            }
            sl->addElement(newBody);
          }

          sl->addElement(StatementPtr(
                           new LabelStatement(
                             s->getScope(), s->getLocation(), afterLab)));
        }
        return sl;
        break;
      }
      default:
        break;
    }
    return StatementPtr();
  }

private:
  typedef std::vector<TryStatementRawPtr> TryVector;
  struct GotoInfo {
    GotoInfo(GotoStatementRawPtr g, const TryVector &t) :
        goto_stmt(g), trys(t) {}
    GotoStatementRawPtr goto_stmt;
    TryVector trys;
  };
  typedef std::vector<GotoInfo> GotoVector;
  struct LabelInfo {
    TryVector   trys;
    GotoVector  gotos;
  };
  struct TryInfo {
    int label;
    std::map<int,string> targets;
  };
  TryVector trys;
  typedef map<string,LabelInfo> LabelInfoMap;
  LabelInfoMap labelMap;
  map<TryStatementRawPtr,TryInfo> tryMap;
  map<string,int> labelIdMap;
  int id;
  AnalysisResultPtr m_ar;
};

void MethodStatement::analyzeProgram(AnalysisResultPtr ar) {
  FunctionScopeRawPtr funcScope = getFunctionScope();

  if (m_params) {
    m_params->analyzeProgram(ar);
    if (Option::GenRTTIProfileData &&
        ar->getPhase() == AnalysisResult::AnalyzeFinal) {
      addParamRTTI(ar);
    }
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
    }
    if (funcScope->isSepExtension() ||
        Option::IsDynamicFunction(m_method, m_name) || Option::AllDynamic) {
      funcScope->setDynamic();
    }
#ifndef HHVM
    // This is an hphpc-only transformation to deal with gotos that jump into
    // try/catch blocks. The VM does not need this transformation, and it does
    // not support this transformation.
    if (funcScope->hasGoto() && funcScope->hasTry()) {
      TryingGotoFixer tgf(ar);
      tgf.fixTryingGotos(m_stmt);
    }
#endif
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
      ASSERT(false);
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
      ASSERT(false);
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
      if (!lastIsReturn && (!pseudoMain || Option::GenerateCPPMain)) {
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
    ASSERT(m);

    VariableTablePtr variables = funcScope->getVariables();
    ExpressionListPtr params = m->getParams();
    if (params) {
      for (int i = 0; i < params->getCount(); i++) {
        ParameterExpressionPtr param =
          dynamic_pointer_cast<ParameterExpression>((*params)[i]);
        const string &name = param->getName();
        ASSERT(!param->isRef() || param->getType()->is(Type::KindOfVariant));
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
        ASSERT(!param->isRef() || param->getType()->is(Type::KindOfVariant));
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

void MethodStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  FunctionScopeRawPtr funcScope = getFunctionScope();
  ClassScopePtr scope = getClassScope();

  if (outputFFI(cg, ar)) return;

  cg.setPHPLineNo(-1);

  CodeGenerator::Context context = cg.getContext();

  if (context == CodeGenerator::CppImplementation) {
    printSource(cg);
  }

  bool isWrapper = context == CodeGenerator::CppTypedParamsWrapperDecl ||
    context == CodeGenerator::CppTypedParamsWrapperImpl;

  bool needsWrapper = isWrapper ||
    (Option::HardTypeHints && funcScope->needsTypeCheckWrapper());

  const char *prefix = needsWrapper && !isWrapper ?
    Option::TypedMethodPrefix : Option::MethodPrefix;

  switch (context) {
    case CodeGenerator::CppDeclaration:
    case CodeGenerator::CppTypedParamsWrapperDecl:
    {
      if (!m_stmt && !funcScope->isPerfectVirtual()) {
        cg_printf("// ");
      }

      m_modifiers->outputCPP(cg, ar);

      if (!m_stmt || m_name == "__offsetget_lval" ||
          funcScope->isPerfectVirtual()) {
        cg_printf("virtual ");
      }
      TypePtr type = funcScope->getReturnType();
      if (type) {
        type->outputCPPDecl(cg, ar, getScope());
      } else {
        cg_printf("void");
      }
      if (m_name == "__offsetget_lval") {
        cg_printf(" &___offsetget_lval(");
      } else if (m_modifiers->isStatic() && m_stmt) {
        // Static method wrappers get generated as support methods
        bool needsClassParam = funcScope->needsClassParam();
        const char *prefix = needsWrapper && !isWrapper ?
          (needsClassParam ?
           Option::TypedMethodImplPrefix : Option::TypedMethodPrefix) :
          (needsClassParam ? Option::MethodImplPrefix : Option::MethodPrefix);
        cg_printf(" %s%s(", prefix,
                  CodeGenerator::FormatLabel(m_name).c_str());
        if (needsClassParam) {
          cg_printf("CStrRef cls%s",
                    funcScope->isVariableArgument() ||
                    (m_params && m_params->getCount()) ? ", " : "");
        }
      } else {
        cg_printf(" %s%s(", prefix,
                  CodeGenerator::FormatLabel(m_name).c_str());
      }
      funcScope->outputCPPParamsDecl(cg, ar, m_params, true);
      if (m_stmt) {
        int opt = Option::GetOptimizationLevel(m_cppLength);
        if (opt < 3) cg_printf(") __attribute__((optimize(%d)));\n", opt);
        else cg_printf(");\n");
      } else if (funcScope->isPerfectVirtual()) {
        cg_printf(");\n");
      } else {
        cg_printf(") = 0;\n");
      }

      if (context != CodeGenerator::CppTypedParamsWrapperDecl) {
        if (funcScope->isConstructor(scope)
            && !funcScope->isAbstract() && !scope->isInterface()) {
          funcScope->outputCPPCreateDecl(cg, ar);
        }
        if (Option::HardTypeHints && funcScope->needsTypeCheckWrapper()) {
          cg.setContext(CodeGenerator::CppTypedParamsWrapperDecl);
          outputCPPImpl(cg, ar);
          cg.setContext(context);
        }
      }
    }
    break;
    case CodeGenerator::CppImplementation:
    case CodeGenerator::CppTypedParamsWrapperImpl:
      if (m_stmt || funcScope->isPerfectVirtual()) {
        int startLineImplementation = -1;
        if (context == CodeGenerator::CppImplementation) {
          startLineImplementation = cg.getLineNo(CodeGenerator::PrimaryStream);
        }
        TypePtr type = funcScope->getReturnType();
        if (type) {
          bool isHeader = cg.isFileOrClassHeader();
          cg.setFileOrClassHeader(true);
          type->outputCPPDecl(cg, ar, getScope());
          cg.setFileOrClassHeader(isHeader);
        } else {
          cg_printf("void");
        }
        string origFuncName = getOriginalFullName();
        if (Option::FunctionSections.find(origFuncName) !=
            Option::FunctionSections.end()) {
          string funcSection = Option::FunctionSections[origFuncName];
          if (!funcSection.empty()) {
            cg_printf(" __attribute__ ((section (\".text.%s\")))",
                      funcSection.c_str());
          }
        }
        origFuncName = CodeGenerator::EscapeLabel(origFuncName);

        if (m_name == "__offsetget_lval") {
          cg_printf(" &%s%s::___offsetget_lval(",
                    Option::ClassPrefix, scope->getId().c_str());
        } else if (m_modifiers->isStatic()) {
          bool needsClassParam = funcScope->needsClassParam();
          const char *prefix = needsWrapper && !isWrapper ?
            (needsClassParam ?
             Option::TypedMethodImplPrefix : Option::TypedMethodPrefix) :
            (needsClassParam ? Option::MethodImplPrefix : Option::MethodPrefix);
          cg_printf(" %s%s::%s%s(", Option::ClassPrefix,
                    scope->getId().c_str(), prefix,
                    CodeGenerator::FormatLabel(m_name).c_str());
          if (needsClassParam) {
            cg_printf("CStrRef cls%s",
                      funcScope->isVariableArgument() ||
                      (m_params && m_params->getCount()) ? ", " : "");
          }
        } else {
          cg_printf(" %s%s::%s%s(", Option::ClassPrefix,
                    scope->getId().c_str(),
                    prefix, CodeGenerator::FormatLabel(m_name).c_str());
        }
        funcScope->outputCPPParamsDecl(cg, ar, m_params, false);
        cg_indentBegin(") {\n");
        if (!m_stmt) {
          cg_printf("return throw_fatal(\"pure virtual\");\n");
          cg_indentEnd("}\n");
        } else if (context != CodeGenerator::CppTypedParamsWrapperImpl) {
          if (m_stmt->hasBody()) {
            const char *suffix =
              (cg.getOutput() == CodeGenerator::SystemCPP ? "_BUILTIN" : "");
            if (suffix[0] == '\0' && !funcScope->needsCheckMem()) {
              suffix = "_NOMEM";
            }
            const string &injectionName =
              CodeGenerator::EscapeLabel(getOriginalFullNameForInjection());
            const string &scopeName =
              CodeGenerator::EscapeLabel(scope->getOriginalName());
            if (m_modifiers->isStatic()) {
              cg_printf("STATIC_METHOD_INJECTION%s(%s, %s);\n", suffix,
                        scopeName.c_str(), injectionName.c_str());
            } else if (cg.getOutput() != CodeGenerator::SystemCPP &&
                       !scope->isRedeclaring() && !scope->derivedByDynamic()) {
              cg_printf("INSTANCE_METHOD_INJECTION_ROOTLESS%s(%s, %s);\n",
                        suffix, scopeName.c_str(), injectionName.c_str());
            } else if (scope->getOriginalName() != "XhprofFrame") {
              cg_printf("INSTANCE_METHOD_INJECTION%s(%s, %s);\n", suffix,
                        scopeName.c_str(), injectionName.c_str());
            }
          }
          if (m_name == "__offsetget_lval") {
            ParameterExpressionPtr param =
              dynamic_pointer_cast<ParameterExpression>((*m_params)[0]);
            cg_printf("Variant &v = %s->__lvalProxy;\n", cg.getGlobals(ar));
            string lowered = Util::toLower(m_originalName);
            cg_printf("v = %s%s(%s%s);\n",
                      prefix, lowered.c_str(),
                      Option::VariablePrefix, param->getName().c_str());
            cg_printf("return v;\n");
          } else {
            outputCPPArgInjections(cg, ar, origFuncName.c_str(),
                                   scope, funcScope);
            funcScope->outputCPP(cg, ar);
            if (funcScope->needsRefTemp()) {
              cg.genReferenceTemp(shared_from_this());
            }
            if (funcScope->needsObjTemp()) {
              cg_printf("ObjectData *obj_tmp UNUSED;\n");
            }
            cg.setContext(
              CodeGenerator::NoContext); // no inner functions/classes
            if (!funcScope->isStatic() && funcScope->getVariables()->
                getAttribute(VariableTable::ContainsDynamicVariable)) {
              Symbol *sym = funcScope->getVariables()->getSymbol("this");
              if (sym && sym->declarationSet()) {
                string namePrefix;
                if (funcScope->isGenerator()) {
                  namePrefix = string(TYPED_CONTINUATION_OBJECT_NAME) + "->";
                }
                cg_printf("%s%sthis = this;\n", namePrefix.c_str(),
                          Option::VariablePrefix);
              }
            }
            outputCPPStmt(cg, ar);
            if (funcScope->needsRefTemp()) cg.clearRefereceTemp();
          }
          cg_indentEnd("}\n");
          ASSERT(startLineImplementation >= 0);
          m_cppLength = cg.getLineNo(CodeGenerator::PrimaryStream)
                        - startLineImplementation;
          if (Option::HardTypeHints && funcScope->needsTypeCheckWrapper()) {
            cg.setContext(CodeGenerator::CppTypedParamsWrapperImpl);
            outputCPPImpl(cg, ar);
          }
        } else {
          outputCPPTypeCheckWrapper(cg, ar);
          cg_indentEnd("}\n");
        }
        cg.setContext(context);
        cg.printImplSplitter();
      }
      break;
    default:
      break;
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

void MethodStatement::outputParamArrayCreate(CodeGenerator &cg, bool checkRef) {
  int n = m_params->getCount();
  ASSERT(n > 0);
  cg_printf("array_createvi(%d, ", n);
  VariableTablePtr variables = getFunctionScope()->getVariables();
  for (int i = 0; i < n; i++) {
    ParameterExpressionPtr param =
      dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
    const string &paramName = param->getName();
    cg_printf("toVPOD(");
    const char *pre = "";
    if (!checkRef && variables->getSymbol(paramName)->isStashedVal()) {
      pre = "v";
    }
    string name = pre + (Option::VariablePrefix +
                         CodeGenerator::FormatLabel(paramName));
    if (checkRef && param->isRef()) {
      not_reached();
    } else {
      cg_printf("%s", name.c_str());
    }
    cg_printf(")");
    if (i < n - 1) {
      cg_printf(", ");
    } else {
      cg_printf(")");
    }
  }
}

void MethodStatement::outputCPPArgInjections(CodeGenerator &cg,
                                             AnalysisResultPtr ar,
                                             const char *name,
                                             ClassScopePtr cls,
                                             FunctionScopePtr funcScope) {
  if (cg.getOutput() != CodeGenerator::SystemCPP) {
    if (funcScope->isDynamicInvoke()) {
      cg_printf("INTERCEPT_INJECTION_ALWAYS(\"%s\", \"%s\", ", name, name);
    } else {
      cg_printf("INTERCEPT_INJECTION(\"%s\", ", name);
    }
    if (m_params) {
      int n = m_params->getCount();
      ASSERT(n >= 0);
      if (Option::GenArrayCreate && !hasRefParam()) {
        if (ar->m_arrayIntegerKeyMaxSize < n) ar->m_arrayIntegerKeyMaxSize = n;
        outputParamArrayCreate(cg, true);
      } else {
        cg_printf("(Array(ArrayInit(%d, ArrayInit::vectorInit)", n);
        for (int i = 0; i < n; i++) {
          ParameterExpressionPtr param =
            dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
          const string &paramName = param->getName();
          cg_printf(".set%s(%d, %s%s%s)", param->isRef() ? "Ref" : "",
                    i, param->isRef() ? "r" : "",
                    Option::VariablePrefix, paramName.c_str());
        }
        cg_printf(".create()))");
      }
    } else {
      cg_printf("null_array");
    }
    TypePtr t = funcScope->getReturnType();
    bool refRet = funcScope->isRefReturn() && t && Type::IsMappedToVariant(t);
    cg_printf(", %s);\n",
              !funcScope->getReturnType() ? "" :
              refRet ? "strongBind(r)" : "r");
  }

  if (Option::GenRTTIProfileData && m_params) {
    for (int i = 0; i < m_params->getCount(); i++) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
      if (param->hasRTTI()) {
        const string &paramName = param->getName();
        int id = ar->getParamRTTIEntryId(cls, funcScope, paramName);
        if (id != -1) {
          cg_printf("RTTI_INJECTION(%s%s, %d);\n",
                    Option::VariablePrefix, paramName.c_str(), id);
        }
      }
    }
  }
}

void MethodStatement::outputCPPStmt(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_stmt) {
    FunctionScopeRawPtr funcScope = getFunctionScope();
    if (funcScope->inPseudoMain()) {
      cg.beginHoistedClasses();
      int i, n = m_stmt->getCount();
      for (i = 0; i < n; ++i) {
        StatementPtr s((*m_stmt)[i]);
        if (s->is(Statement::KindOfClassStatement) ||
            s->is(Statement::KindOfInterfaceStatement)) {
          cg.addHoistedClass(s->getClassScope()->getName());
        }
      }
      bool seenOther = false;
      for (i = 0; i < n; ++i) {
        StatementPtr s((*m_stmt)[i]);
        if (!s || !s->hasImpl()) continue;
        if (s->is(Statement::KindOfFunctionStatement)) {
          s->outputCPP(cg, ar);
          continue;
        }
        if (s->is(Statement::KindOfClassStatement) ||
            s->is(Statement::KindOfInterfaceStatement)) {
          ClassScopeRawPtr cls = s->getClassScope();
          if (cls->isBaseClass() ||
              !cls->isVolatile() ||
              !cls->hasUnknownBases()) {
            cls->outputCPPDef(cg);
            continue;
          }
          const string &parent = cls->getOriginalParent();
          if (cls->getBases().size() > (parent.empty() ? 0 : 1)) {
            // the class is not hoistable because it implements
            // an interface
            continue;
          }
          always_assert(!parent.empty());
          if (seenOther) {
            ClassScopePtr pcls = ar->findClass(parent);
            if (pcls) {
              if (pcls->isVolatile()) {
                cg_indentBegin("if (%s->CDEC(%s)) {\n",
                               cg.getGlobals(ar),
                               CodeGenerator::FormatLabel(
                                 pcls->getName()).c_str());
              }
              cls->outputCPPDef(cg);
              if (pcls->isVolatile()) {
                cg_indentEnd("}\n");
              }
              continue;
            }
          }
        } else {
          seenOther = true;
        }
      }
      cg.collectHoistedClasses(false);
      for (i = 0; i < n; ++i) {
        StatementPtr s((*m_stmt)[i]);
        if (s->is(Statement::KindOfFunctionStatement)) continue;
        if (s->is(Statement::KindOfClassStatement) ||
            s->is(Statement::KindOfInterfaceStatement)) {
          ClassScopeRawPtr cls = s->getClassScope();
          if (cls->isBaseClass() ||
              !cls->isVolatile() ||
              !cls->hasUnknownBases()) {
            continue;
          }
          cg.collectHoistedClasses(true);
          s->outputCPP(cg, ar);
          cg.collectHoistedClasses(false);
        } else {
          s->outputCPP(cg, ar);
        }
      }
      cg.endHoistedClasses();
    } else {
      m_stmt->outputCPP(cg, ar);
    }
  }
}

void MethodStatement::outputCPPStaticMethodWrapper(CodeGenerator &cg,
                                                   AnalysisResultPtr ar,
                                                   const char *cls) {
  if (!m_modifiers->isStatic() || !m_stmt) return;

  CodeGenerator::Context context = cg.getContext();
  FunctionScopeRawPtr funcScope = getFunctionScope();
  if (!funcScope->needsClassParam()) return;

  bool isWrapper = context == CodeGenerator::CppTypedParamsWrapperDecl ||
    context == CodeGenerator::CppTypedParamsWrapperImpl;

  bool needsWrapper = isWrapper ||
    (Option::HardTypeHints && funcScope->needsTypeCheckWrapper());

  m_modifiers->outputCPP(cg, ar);
  TypePtr type = funcScope->getReturnType();
  if (type) {
    type->outputCPPDecl(cg, ar, getScope());
  } else {
    cg_printf("void");
  }
  cg_printf(" %s%s(", needsWrapper && !isWrapper ?
            Option::TypedMethodPrefix : Option::MethodPrefix,
            CodeGenerator::FormatLabel(m_name).c_str());
  if (!isWrapper) cg.setContext(CodeGenerator::CppFunctionWrapperDecl);
  funcScope->outputCPPParamsDecl(cg, ar, m_params, true);
  cg_printf(") { %s%s%s(", type ? "return " : "",
            needsWrapper && !isWrapper ?
            Option::TypedMethodImplPrefix : Option::MethodImplPrefix,
            CodeGenerator::FormatLabel(m_name).c_str());
  cg_printf("%s%s::s_class_name", Option::ClassPrefix, cls);
  cg.setContext(context);
  if (funcScope->isVariableArgument()) {
    cg_printf(", num_args");
  }
  if (m_params) {
    for (int i = 0; i < m_params->getCount(); i++) {
      ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
      ASSERT(param);
      cg_printf(", %s%s", Option::VariablePrefix, param->getName().c_str());
    }
  }
  if (funcScope->isVariableArgument()) {
    cg_printf(", args");
  }
  cg_printf("); }\n");
  if (!isWrapper && needsWrapper) {
    cg.setContext(CodeGenerator::CppTypedParamsWrapperDecl);
    outputCPPStaticMethodWrapper(cg, ar, cls);
    cg.setContext(context);
  }
}

void MethodStatement::outputCPPTypeCheckWrapper(CodeGenerator &cg,
                                                AnalysisResultPtr ar) {
  FunctionScopeRawPtr funcScope = getFunctionScope();
  TypePtr type = funcScope->getReturnType();
  bool isMethod = getClassScope();
  string fname = isMethod ? funcScope->getName() : funcScope->getId();

  funcScope->outputCPP(cg, ar);
  cg_printf("%s%s%s(", type ? "return " : "",
            (isMethod ?
             (m_modifiers->isStatic() && funcScope->needsClassParam() ?
              Option::TypedMethodImplPrefix :
              Option::TypedMethodPrefix) :
             Option::TypedFunctionPrefix),
            fname.c_str());
  if (!isMethod) {
    if (fname[0] == '0') {
      cg_printf("extra, ");
    }
  } else if (m_modifiers->isStatic() && funcScope->needsClassParam()) {
    cg_printf("cls, ");
  }
  if (funcScope->isVariableArgument()) {
    cg_printf("num_args, ");
  }

  always_assert(m_params);

  for (int i = 0; i < m_params->getCount(); i++) {
    ParameterExpressionPtr param =
      dynamic_pointer_cast<ParameterExpression>((*m_params)[i]);
    ASSERT(param);
    ConstantExpressionPtr con =
      dynamic_pointer_cast<ConstantExpression>(param->defaultValue());
    bool needsNullGet = con && con->isNull();
    if (i) cg_printf(", ");

    const string &paramName =
      string(Option::VariablePrefix) + param->getName();
    if (TypePtr spec = funcScope->getParamTypeSpec(i)) {
      if (Type::SameType(spec, funcScope->getParamType(i))) {
        if (spec->isSpecificObject() && !needsNullGet) {
          ClassScopePtr cls = ar->findClass(spec->getName());
          ASSERT(cls && !cls->isRedeclaring());
          spec->outputCPPFastObjectCast(cg, ar, getScope(), true);
        }

        ASSERT(Type::HasFastCastMethod(spec));

        const char *nullGetMethod = NULL;
        switch (spec->getKindOf()) {
        case Type::KindOfArray:
          nullGetMethod = "getArrayDataOrNull";
          break;
        case Type::KindOfString:
          nullGetMethod = "getStringDataOrNull";
          break;
        case Type::KindOfObject:
          ASSERT(spec->isSpecificObject());
          nullGetMethod = "getObjectDataOrNull";
          break;
        default:
          break;
        }

        ASSERT(!needsNullGet || nullGetMethod != NULL);

        cg_printf("%s", paramName.c_str());
        if (needsNullGet) {
          cg_printf(".%s()", nullGetMethod);
        } else {
          const string &fastCast = Type::GetFastCastMethod(spec, true, true);
          ASSERT(!fastCast.empty());
          cg_printf(".%s()", fastCast.c_str());
        }
      } else if (needsNullGet &&
                 funcScope->getParamType(i)->isStandardObject()) {
        // a standard object can only be inferred if the spec type is
        // a specific object - also, we only do this if we have
        // a default value of null specified since otherwise,
        // the passed in parameter cannot be null anyways
        // and we do not have to generate a special type
        // check in the wrapper
        ASSERT(spec->isSpecificObject());
        cg_printf("%s.getObjectDataOrNull()", paramName.c_str());
      } else {
        cg_printf("%s", paramName.c_str());
      }
    } else {
      cg_printf("%s", paramName.c_str());
    }
  }

  if (funcScope->isVariableArgument()) {
    cg_printf(", args");
  }
  cg_printf(");\n");
}

bool MethodStatement::outputFFI(CodeGenerator &cg, AnalysisResultPtr ar) {
  FunctionScopeRawPtr funcScope = getFunctionScope();
  ClassScopePtr clsScope = getClassScope();
  bool pseudoMain = funcScope->inPseudoMain();
  bool inClass = !m_className.empty();
  // only expose public methods, and ignore those in redeclared classes
  bool inaccessible =
    inClass && (!m_modifiers->isPublic() || clsScope->isRedeclaring());
  // skip constructors
  bool isConstructor = inClass && funcScope->isConstructor(clsScope);
  bool valid = !pseudoMain && !inaccessible && !isConstructor &&
    (inClass || !ParserBase::IsAnonFunctionName(m_originalName));

  if (cg.getContext() == CodeGenerator::CppFFIDecl ||
      cg.getContext() == CodeGenerator::CppFFIImpl) {
    if (valid) outputCPPFFIStub(cg, ar);
    return true;
  }

  if (cg.getContext() == CodeGenerator::HsFFI) {
    if (valid) outputHSFFIStub(cg, ar);
    return true;
  }

  if (cg.getContext() == CodeGenerator::JavaFFI
   || cg.getContext() == CodeGenerator::JavaFFIInterface) {
    if (valid) outputJavaFFIStub(cg, ar);
    return true;
  }

  if (cg.getContext() == CodeGenerator::JavaFFICppDecl
   || cg.getContext() == CodeGenerator::JavaFFICppImpl) {
    if (valid) outputJavaFFICPPStub(cg, ar);
    return true;
  }

  if (cg.getContext() == CodeGenerator::SwigFFIDecl
   || cg.getContext() == CodeGenerator::SwigFFIImpl) {
    if (valid) outputSwigFFIStub(cg, ar);
    return true;
  }

  return false;
}

void MethodStatement::outputCPPFFIStub(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  FunctionScopeRawPtr funcScope = getFunctionScope();
  ClassScopePtr clsScope = getClassScope();
  bool varArgs = funcScope->isVariableArgument();
  bool ret = funcScope->getReturnType();
  string fname = funcScope->getId();
  bool inClass = !m_className.empty();
  bool isStatic = !inClass || m_modifiers->isStatic();

  if (inClass && m_modifiers->isAbstract()) {
    return;
  }

  if (funcScope->getName() == "__offsetget_lval") {
    return;
  }

  if (ret) {
    cg_printf("int");
  } else {
    cg_printf("void");
  }
  cg_printf(" %s%s%s(", Option::FFIFnPrefix,
            (inClass ? (m_className + "_cls_").c_str() : ""), fname.c_str());
  if (ret) {
    cg_printf("void** res");
  }

  bool first = !ret;

  if (!isStatic) {
    // instance methods need one additional parameter for the target
    if (first) {
      first = false;
    }
    else {
      cg_printf(", ");
    }
    cg_printf("Variant *target");
  }

  int ac = funcScope->getMaxParamCount();
  for (int i = 0; i < ac; ++i) {
    if (first) {
      first = false;
    } else {
      cg_printf(", ");
    }
    cg_printf("Variant *a%d", i);
  }
  if (varArgs) {
    if (!first) {
      cg_printf(", ");
    }
    cg_printf("Variant *va");
  }
  cg_printf(")");
  if (cg.getContext() == CodeGenerator::CppFFIDecl) {
    cg_printf(";\n");
  } else {
    cg_indentBegin(" {\n");
    if (ret) {
      cg_printf("return hphp_ffi_exportVariant(");
    }

    if (!inClass) {
      // simple function call
      cg_printf("%s%s(", Option::FunctionPrefix, fname.c_str());
    } else if (isStatic) {
      // static method call
      cg_printf("%s%s::%s%s(", Option::ClassPrefix,
                clsScope->getId().c_str(),
                Option::MethodPrefix, funcScope->getName().c_str());
    } else {
      // instance method call
      cg_printf("dynamic_cast<%s%s *>(target->getObjectData())->",
                Option::ClassPrefix, clsScope->getId().c_str());
      cg_printf("%s%s(", Option::MethodPrefix, funcScope->getName().c_str());
    }

    first = true;
    if (varArgs) {
      cg_printf("%d + (va->isNull() ? 0 : va->getArrayData()->size())", ac);
      first = false;
    }

    for (int i = 0; i < ac; ++i) {
      if (first) {
        first = false;
      } else {
        cg_printf(", ");
      }
      cg_printf("*a%d", i);
    }
    if (varArgs) {
      cg_printf(", va->toArray()");
    }
    if (ret) {
      cg_printf("), res");
    }
    cg_printf(");\n");
    cg_indentEnd("}\n");
    cg.printImplSplitter();
  }
  return;
}

void MethodStatement::outputHSFFIStub(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (!m_className.empty()) {
    // Haskell currently doesn't support FFI for class methods.
    return;
  }

  FunctionScopeRawPtr funcScope = getFunctionScope();
  bool varArgs = funcScope->isVariableArgument();
  bool ret = funcScope->getReturnType();
  string fname = funcScope->getId().c_str();
  cg_indentBegin("foreign import ccall \"stubs.h %s%s\" %s%s\n",
                 Option::FFIFnPrefix, fname.c_str(),
                 Option::FFIFnPrefix, fname.c_str());
  cg_printf(":: ");
  if (ret) {
    cg_printf("PtrPtr a -> ");
  }
  int ac = funcScope->getMaxParamCount();
  for (int i = 0; i < ac; ++i) {
    cg_printf("HphpVariantPtr -> ");
  }
  if (varArgs) {
    cg_printf("HphpVariantPtr -> ");
  }
  if (ret) {
    cg_printf("IO CInt");
  } else {
    cg_printf("IO ()");
  }
  cg_indentEnd("\n");

  cg_printf("f_%s :: ", fname.c_str());
  bool first = true;
  if (ac > 0) {
    cg_printf("(");
  }
  for (int i = 0; i < ac; ++i) {
    if (first) {
      first = false;
    } else {
      cg_printf(", ");
    }
    cg_printf("VariantAble a%d", i);
  }
  if (ac > 0) {
    cg_printf(") => ");
  }
  for (int i = 0; i < ac; ++i) {
    cg_printf("a%d -> ", i);
  }
  if (varArgs) {
    cg_printf("[Variant] -> ");
  }
  if (ret) {
    cg_printf("IO Variant");
  } else {
    cg_printf("IO ()");
  }
  cg_printf("\n");
  cg_printf("f_%s ", fname.c_str());
  for (int i = 0; i < ac; ++i) {
    cg_printf("v%d ", i);
  }
  if (varArgs) {
    cg_printf("va ");
  }
  cg_indentBegin("=%s\n", ret ? " alloca (\\pres ->" : "");
  for (int i = 0; i < ac; ++i) {
    cg_indentBegin("withExportedVariant (toVariant v%d) (\\p%d ->\n", i, i);
  }
  if (varArgs) {
    cg_indentBegin("withVParamList va (\\pva ->\n");
  }
  cg_indentBegin("do\n");
  cg_printf("%sffi_%s", ret ? "t <- " : "", fname.c_str());
  if (ret) {
    cg_printf(" pres");
  }
  for (int i = 0; i < ac; ++i) {
    cg_printf(" p%d", i);
  }
  if (varArgs) {
    cg_printf(" pva");
  }
  if (ret) {
    cg_printf("\n");
    cg_printf("ppres <- peek pres\n");
    cg_printf("buildVariant (fromIntegral t) ppres");
  }
  cg_indentEnd(); // end do
  if (varArgs) {
    cg_indentEnd(")"); // end varargs
  }
  for (int i = 0; i < ac; ++i) {
    cg_indentEnd(")"); // end wEV i
  }
  if (ret) {
    cg_indentEnd(")"); // end alloca
  } else {
    cg_indentEnd();
  }
  cg_printf("\n");
  return;
}

/**
 * Generates the Java stub method for a PHP toplevel function.
 *
 * @author qixin
 */
void MethodStatement::outputJavaFFIStub(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  FunctionScopeRawPtr funcScope = getFunctionScope();
  bool varArgs = funcScope->isVariableArgument();
  bool ret = funcScope->getReturnType();
  bool inClass = !m_className.empty();
  bool isStatic = !inClass || m_modifiers->isStatic();
  string fname = funcScope->getId();
  string originalName = funcScope->getOriginalName();
  if (originalName.length() < fname.length()) {
    // if there are functions of the same name, fname may contain "$$..."
    // in the end
    originalName += fname.substr(originalName.length());
  }

  if (originalName == "clone"     || originalName == "equals"
   || originalName == "finalize"  || originalName == "getClass"
   || originalName == "hashCode"  || originalName == "notify"
   || originalName == "notifyAll" || originalName == "toString"
   || originalName == "wait") {
    // not to clash with Java method names
    originalName = "_" + originalName;
  }

  if (cg.getContext() == CodeGenerator::JavaFFIInterface
   || (inClass && m_modifiers->isAbstract())) {
    // skip all the abstract methods, because php overriding is not very
    // compatible with Java
    return;
  }

  if (!inClass) printSource(cg);

  // This Java method extracts the Variant pointer from the HphpVariant
  // argument as a 64-bit integer, and then calls the native version.
  bool exposeNative = false;
  int ac = funcScope->getMaxParamCount();
  if (ac > 0 || varArgs || !isStatic || (!ret && inClass)
   || cg.getContext() == CodeGenerator::JavaFFIInterface) {
    // make methods always return something, so that they can override
    // each other
    cg_printf("public %s%s %s(",
              (isStatic ? "static " : ""),
              (!ret && !inClass ? "void" : "HphpVariant"),
              originalName.c_str());
    std::ostringstream args;
    bool first = true;
    if (!isStatic) {
      // instance method has an additional parameter
      args << "this.getVariantPtr()";
    }
    for (int i = 0; i < ac; i++) {
      if (first) {
        first = false;
        if (!isStatic) args << ", ";
      }
      else {
        cg_printf(", ");
        args << ", ";
      }
      cg_printf("HphpVariant a%d", i);
      args << "a" << i << ".getVariantPtr()";
    }
    if (varArgs) {
      if (!first) {
        cg_printf(", ");
        args << ", ";
      }
      else if (!isStatic) {
        args << ", ";
      }
      cg_printf("HphpVariant va");
      args << "va.getVariantPtr()";
    }

    if (cg.getContext() == CodeGenerator::JavaFFIInterface) {
      cg_printf(");\n\n");
      return;
    }

    cg_indentBegin(") {\n");
    cg_printf("%s%s_native(%s);\n", (ret ? "return " : ""),
              originalName.c_str(),
              args.str().c_str());
    if (!ret && inClass) {
      cg_printf("return HphpNull.phpNull();\n");
    }
    cg_indentEnd("}\n\n");
  }
  else {
    exposeNative = true;
  }

  // the native method stub
  cg_printf("%s %snative %s %s%s(",
            (exposeNative ? "public" : "private"),
            (isStatic ? "static " : ""), (ret ? "HphpVariant" : "void"),
            originalName.c_str(),
            (exposeNative ? "" : "_native"));
  bool first = true;
  if (!isStatic) {
    // instance method has an additional parameter
    cg_printf("long targetPtr");
    first = false;
  }
  for (int i = 0; i < ac; i++) {
    if (first) first = false;
    else cg_printf(", ");
    cg_printf("long a%d", i);
  }
  if (varArgs) {
    if (!first) cg_printf(", ");
    cg_printf("long va");
  }
  cg_printf(");\n\n");
}

void MethodStatement::outputJavaFFICPPStub(CodeGenerator &cg,
                                           AnalysisResultPtr ar) {
  // TODO translate PHP namespace once that is supported
  string packageName = Option::JavaFFIRootPackage;

  FunctionScopeRawPtr funcScope = getFunctionScope();
  bool varArgs = funcScope->isVariableArgument();
  bool ret = funcScope->getReturnType();
  bool inClass = !m_className.empty();
  bool isStatic = !inClass || m_modifiers->isStatic();
  string fname = funcScope->getId();
  int ac = funcScope->getMaxParamCount();
  bool exposeNative = !(ac > 0 || varArgs || !isStatic || (!ret && inClass));

  if (inClass && m_modifiers->isAbstract()) {
    // skip all the abstract methods, because hphp doesn't generate code
    // for them
    return;
  }

  if (funcScope->getName() == "__offsetget_lval") return;

  const char *clsName;
  if (inClass) {
    // uses capitalized original class name
    ClassScopePtr cls = ar->findClass(m_className);
    clsName = cls->getOriginalName().c_str();
  } else {
    clsName = "HphpMain";
  }
  string mangledName = "Java." + packageName + "." + clsName + "." + fname
    + (exposeNative ? "" : "_native");
  // all the existing "_" are replaced as "_1"
  Util::replaceAll(mangledName, "_", "_1");
  Util::replaceAll(mangledName, ".", "_");

  cg_printf("JNIEXPORT %s JNICALL\n", ret ? "jobject" : "void");
  cg_printf("%s(JNIEnv *env, %s target", mangledName.c_str(),
            (isStatic ? "jclass" : "jobject"));

  std::ostringstream args;
  bool first = true;
  if (!isStatic) {
    // instance method also gets an additional argument, which is a Variant
    // pointer to the target, encoded in int64
    first = false;
    cg_printf(", jlong targetPtr");
    args << "(Variant *)targetPtr";
  }
  for (int i = 0; i < ac; i++) {
    cg_printf(", jlong a%d", i);
    if (first) first = false;
    else args << ", ";
    args << "(Variant *)a" << i;
  }
  if (varArgs) {
    cg_printf(", jlong va");
    if (!first) args << ", ";
    args << "(Variant *)va";
  }

  if (cg.getContext() == CodeGenerator::JavaFFICppDecl) {
    // java_stubs.h
    cg_printf(");\n\n");
    return;
  }

  cg_indentBegin(") {\n");

  // support static/instance methods
  if (ret) {
    cg_printf("void *result;\n");
    cg_printf("int kind = ");
    cg_printf("%s%s%s(&result",
              Option::FFIFnPrefix,
              (inClass ? (m_className + "_cls_").c_str() : ""), fname.c_str());
    if (!isStatic || ac > 0 || varArgs) cg_printf(", ");
  } else {
    cg_printf("%s%s%s(", Option::FFIFnPrefix,
                         (inClass ? (m_className + "_cls_").c_str() : ""),
                         fname.c_str());
  }
  cg_printf("%s);\n", args.str().c_str());
  if (ret) {
    if (!inClass) {
      // HphpMain extends hphp.Hphp.
      cg_printf("jclass hphp = env->GetSuperclass(target);\n");
    }
    else {
      cg_printf("jclass hphp = env->FindClass(\"hphp/Hphp\");\n");
    }
    cg_printf("return exportVariantToJava(env, hphp, result, kind);\n");
  }

  cg_indentEnd("}\n");
  cg.printImplSplitter();
}

void MethodStatement::outputSwigFFIStub(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  FunctionScopeRawPtr funcScope = getFunctionScope();
  bool varArgs = funcScope->isVariableArgument();
  bool ret = funcScope->getReturnType();
  string fname = funcScope->getId();
  string originalName = funcScope->getOriginalName();
  int ac = funcScope->getMaxParamCount();

  if (cg.getContext() == CodeGenerator::SwigFFIImpl) {
    printSource(cg);
  }

  cg_printf("Variant *%s(HphpSession *s", originalName.c_str());
  std::ostringstream args;
  bool first = true;
  for (int i = 0; i < ac; i++) {
    cg_printf(", Variant *a%d", i);
    if (first) first = false;
    else args << ", ";
    args << "a" << i;
  }
  if (varArgs) {
    cg_printf(", Variant *va");
    if (!first) args << ", ";
    args << "va";
  }

  if (cg.getContext() == CodeGenerator::SwigFFIDecl) {
    cg_printf(");\n\n");
    return;
  }

  cg_indentBegin(") {\n");
  if (ret) {
    cg_printf("void *result;\n");
    cg_printf("int kind = ");
    cg_printf("%s%s(&result", Option::FFIFnPrefix, fname.c_str());
    if (ac > 0 || varArgs) cg_printf(", ");
  } else {
    cg_printf("%s%s(", Option::FFIFnPrefix, fname.c_str());
  }
  cg_printf("%s);\n", args.str().c_str());
  cg_printf("Variant *ret = ");
  if (ret) {
    cg_printf("hphpBuildVariant(kind, result);\n");
    cg_printf("s->addVariant(ret);\n");
  } else {
    cg_printf("hphpBuildVariant(0, 0);\n");
    cg_printf("s->addVariant(ret);\n");
  }
  cg_printf("return ret;\n");
  cg_indentEnd("}\n\n");
}

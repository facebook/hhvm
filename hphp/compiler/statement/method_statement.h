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

#ifndef incl_HPHP_METHOD_STATEMENT_H_
#define incl_HPHP_METHOD_STATEMENT_H_

#include "hphp/compiler/statement/statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ClosureExpression);
DECLARE_BOOST_TYPES(ModifierExpression);
DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(MethodStatement);

class MethodStatement : public Statement, public IParseHandler {
protected:
  MethodStatement(STATEMENT_CONSTRUCTOR_BASE_PARAMETERS,
                  ModifierExpressionPtr modifiers, bool ref,
                  const std::string &name, ExpressionListPtr params,
                  const std::string &retTypeConstraint, StatementListPtr stmt,
                  int attr, const std::string &docComment,
                  ExpressionListPtr attrList, bool method = true);
public:
  MethodStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                  ModifierExpressionPtr modifiers, bool ref,
                  const std::string &name, ExpressionListPtr params,
                  const std::string &retTypeConstraint, StatementListPtr stmt,
                  int attr, const std::string &docComment,
                  ExpressionListPtr attrList, bool method = true);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  void inferFunctionTypes(AnalysisResultPtr ar);
  virtual bool hasDecl() const { return true; }
  virtual bool hasImpl() const { return false; }
  virtual int getRecursiveCount() const;
  // implementing IParseHandler
  virtual void onParseRecur(AnalysisResultConstPtr ar, ClassScopePtr scope);

  void fixupSelfAndParentTypehints(ClassScopePtr scope);

  const std::string &getOriginalName() const { return m_originalName;}
  std::string getName() const { return m_name;}
  void setName(const std::string name) { m_name = name; }
  void setOriginalName(const std::string name) { m_originalName = name; }
  std::string getFullName() const;
  std::string getOriginalFullName() const;
  std::string getOriginalFullNameForInjection() const;
  std::string getOriginalFilename() const { return m_originalFilename; }
  ExpressionListPtr getParams() { return m_params;}
  const std::string getReturnTypeConstraint() const {
    return m_retTypeConstraint;
  }
  StatementListPtr getStmts() { return m_stmt;}
  bool isRef(int index = -1) const;

  int getLocalEffects() const;

  ModifierExpressionPtr getModifiers() {
    return m_modifiers;
  }

  void setModifiers(ModifierExpressionPtr newModifiers) {
    m_modifiers = newModifiers;
  }

  bool hasRefParam();
  void outputParamArrayCreate(CodeGenerator &cg, bool checkRef);
  FunctionScopePtr onInitialParse(AnalysisResultConstPtr ar, FileScopePtr fs);

  FunctionScopeRawPtr getFunctionScope() const {
    BlockScopeRawPtr b = getScope();
    assert(b->is(BlockScope::FunctionScope));
    return FunctionScopeRawPtr((FunctionScope*)b.get());
  }

  const std::string &getDocComment() const { return m_docComment; }

  // these pointers must be raw (weak) pointers to prevent cycles
  // in the reference graph

  void setOrigGeneratorFunc(MethodStatementRawPtr stmt) {
    m_origGeneratorFunc = stmt;
  }
  MethodStatementRawPtr getOrigGeneratorFunc() const {
    return m_origGeneratorFunc;
  }

  void setGeneratorFunc(MethodStatementRawPtr stmt) {
    m_generatorFunc = stmt;
  }
  MethodStatementRawPtr getGeneratorFunc() const {
    return m_generatorFunc;
  }

  void setContainingClosure(ClosureExpressionRawPtr exp) {
    m_containingClosure = exp;
  }
  ClosureExpressionRawPtr getContainingClosure() const {
    return m_containingClosure;
  }

  void setClassName(const std::string &name) { m_className = name; }
  void setOriginalClassName(const std::string &name) {
    m_originalClassName = name;
  }

  // for flattened traits
  void setOriginalFilename(const std::string &name) {
    assert(m_method);
    m_originalFilename = name;
  }

  void addTraitMethodToScope(AnalysisResultConstPtr ar,
                             ClassScopePtr classScope);

protected:
  bool m_method;
  bool m_ref;
  int m_attribute;
  int m_cppLength;
  ModifierExpressionPtr m_modifiers;
  std::string m_name;
  std::string m_originalName;
  std::string m_className;
  std::string m_originalClassName;
  std::string m_originalFilename;
  ExpressionListPtr m_params;
  std::string m_retTypeConstraint;
  StatementListPtr m_stmt;
  std::string m_docComment;
  MethodStatementRawPtr m_origGeneratorFunc;
  MethodStatementRawPtr m_generatorFunc;
  ClosureExpressionRawPtr m_containingClosure;
  ExpressionListPtr m_attrList;

  void setSpecialMethod(ClassScopePtr classScope);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_METHOD_STATEMENT_H_

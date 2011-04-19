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

#ifndef __METHOD_STATEMENT_H__
#define __METHOD_STATEMENT_H__

#include <compiler/statement/statement.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ModifierExpression);
DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(MethodStatement);

class MethodStatement : public Statement, public IParseHandler {
public:
  MethodStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                  ModifierExpressionPtr modifiers, bool ref,
                  const std::string &name,
                  ExpressionListPtr params, StatementListPtr stmt, int attr,
                  const std::string &docComment,
                  bool method = true);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  void inferFunctionTypes(AnalysisResultPtr ar);
  virtual bool hasDecl() const { return true; }
  virtual bool hasImpl() const { return false; }
  virtual int getRecursiveCount() const;
  // implementing IParseHandler
  virtual void onParseRecur(AnalysisResultConstPtr ar, ClassScopePtr scope);

  const std::string &getOriginalName() const { return m_originalName;}
  std::string getName() const { return m_name;}
  void setName(const std::string name) { m_name = name; }
  std::string getFullName() const;
  std::string getOriginalFullName() const;
  ExpressionListPtr getParams() { return m_params;}
  StatementListPtr getStmts() { return m_stmt;}
  bool isRef(int index = -1) const;

  int getLocalEffects() const;

  ModifierExpressionPtr getModifiers() {
    return m_modifiers;
  }

  void outputCPPStaticMethodWrapper(CodeGenerator &cg,
                                    AnalysisResultPtr ar,
                                    const char *cls);
  void outputCPPTypeCheckWrapper(CodeGenerator &cg,
                                 AnalysisResultPtr ar);
  bool hasRefParam();
  void outputParamArrayCreate(CodeGenerator &cg, bool checkRef);
  FunctionScopePtr onInitialParse(AnalysisResultConstPtr ar, FileScopePtr fs);

  FunctionScopeRawPtr getFunctionScope() const {
    BlockScopeRawPtr b = getScope();
    ASSERT(b->is(BlockScope::FunctionScope));
    return FunctionScopeRawPtr((FunctionScope*)b.get());
  }

  const std::string &getDocComment() const { return m_docComment; }
protected:
  bool m_method;
  ModifierExpressionPtr m_modifiers;
  bool m_ref;
  std::string m_name;
  std::string m_originalName;
  std::string m_className;
  std::string m_originalClassName;
  ExpressionListPtr m_params;
  StatementListPtr m_stmt;
  int m_attribute;
  std::string m_docComment;
  int m_cppLength;

  void outputCPPArgInjections(CodeGenerator &cg, AnalysisResultPtr ar,
                              const char *name, ClassScopePtr cls,
                              FunctionScopePtr funcScope);
  void outputCPPStmt(CodeGenerator &cg, AnalysisResultPtr ar);
  void addParamRTTI(AnalysisResultPtr ar);

  /**
   * The one FFI generation master that rules them all!
   */
  bool outputFFI(CodeGenerator &cg, AnalysisResultPtr ar);

  void outputHSFFIStub(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPFFIStub(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputJavaFFIStub(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputJavaFFICPPStub(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputSwigFFIStub(CodeGenerator &cg, AnalysisResultPtr ar);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __METHOD_STATEMENT_H__

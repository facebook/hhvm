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

#ifndef incl_HPHP_METHOD_STATEMENT_H_
#define incl_HPHP_METHOD_STATEMENT_H_

#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/type_annotation.h"

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
                  TypeAnnotationPtr retTypeAnnotation, StatementListPtr stmt,
                  int attr, const std::string &docComment,
                  ExpressionListPtr attrList, bool method = true);
public:
  MethodStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                  ModifierExpressionPtr modifiers, bool ref,
                  const std::string &name, ExpressionListPtr params,
                  TypeAnnotationPtr retTypeAnnotation, StatementListPtr stmt,
                  int attr, const std::string &docComment,
                  ExpressionListPtr attrList, bool method = true) :
      MethodStatement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(MethodStatement),
                      modifiers, ref, name, params, retTypeAnnotation, stmt,
                      attr, docComment, attrList, method) {}

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  bool hasDecl() const override { return true; }
  bool hasImpl() const override { return false; }
  int getRecursiveCount() const override;
  // implementing IParseHandler
  void onParseRecur(AnalysisResultConstPtr ar, FileScopeRawPtr fs,
                    ClassScopePtr scope) override;

  void fixupSelfAndParentTypehints(ClassScopePtr scope);

  bool isNamed(const char* name) const;
  bool isNamed(const std::string& name) const { return isNamed(name.c_str()); }
  const std::string &getOriginalName() const { return m_originalName;}
  std::string getName() const override { return m_originalName; }
  void setOriginalName(const std::string name) { m_originalName = name; }
  std::string getOriginalFullName() const;
  std::string getOriginalFilename() const { return m_originalFilename; }
  ExpressionListPtr getParams() { return m_params;}
  const std::string getReturnTypeConstraint() const {
    return m_retTypeAnnotation.get() ? m_retTypeAnnotation->fullName() : "";
  }
  const TypeAnnotationPtr retTypeAnnotation() const {
    return m_retTypeAnnotation;
  }
  StatementListPtr getStmts() { return m_stmt;}
  bool isRef(int index = -1) const;
  bool isSystem() const;

  int getLocalEffects() const override;

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

  void setContainingClosure(ClosureExpressionRawPtr exp) {
    m_containingClosure = exp;
  }
  ClosureExpressionRawPtr getContainingClosure() const {
    return m_containingClosure;
  }

  // for flattened traits
  void setOriginalFilename(const std::string &name) {
    assert(m_method);
    m_originalFilename = name;
  }

  void addTraitMethodToScope(AnalysisResultConstPtr ar,
                             ClassScopePtr classScope);

  void setHasCallToGetArgs(bool f) { m_hasCallToGetArgs = f; }
  bool hasCallToGetArgs() const { return m_hasCallToGetArgs; }

  void setMayCallSetFrameMetadata(bool f) { m_mayCallSetFrameMetadata = f; }
  bool mayCallSetFrameMetadata() const { return m_mayCallSetFrameMetadata; }

protected:
  bool m_method;
  bool m_ref;
  bool m_hasCallToGetArgs;
  bool m_mayCallSetFrameMetadata;
  int m_attribute;
  int m_cppLength;
  int m_autoPropCount;
  ModifierExpressionPtr m_modifiers;
  std::string m_originalName;
  std::string m_originalFilename;
  ExpressionListPtr m_params;
  TypeAnnotationPtr m_retTypeAnnotation;
  StatementListPtr m_stmt;
  std::string m_docComment;
  ClosureExpressionRawPtr m_containingClosure;
  ExpressionListPtr m_attrList;

  void setSpecialMethod(FileScopeRawPtr fileScope, ClassScopePtr classScope);
  void checkParameters(FileScopeRawPtr scope);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_METHOD_STATEMENT_H_

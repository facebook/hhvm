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

#ifndef incl_HPHP_INTERFACE_STATEMENT_H_
#define incl_HPHP_INTERFACE_STATEMENT_H_

#include "hphp/compiler/statement/statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(InterfaceStatement);

class InterfaceStatement : public Statement, public IParseHandler {
protected:
  InterfaceStatement(STATEMENT_CONSTRUCTOR_BASE_PARAMETERS,
                     const std::string &name, ExpressionListPtr base,
                     const std::string &docComment, StatementListPtr stmt,
                     ExpressionListPtr attrList);
public:
  InterfaceStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                     const std::string &name, ExpressionListPtr base,
                     const std::string &docComment, StatementListPtr stmt,
                     ExpressionListPtr attrList);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  StatementPtr preOptimize(AnalysisResultConstPtr ar);
  virtual bool hasDecl() const { return true; }
  virtual bool hasImpl() const;
  virtual int getRecursiveCount() const;
  // implementing IParseHandler
  virtual void onParse(AnalysisResultConstPtr ar, FileScopePtr scope);

  int getLocalEffects() const;

  virtual std::string getName() const;
  const std::string &getOriginalName() const { return m_originalName;}
  virtual void getAllParents(AnalysisResultConstPtr ar,
                             std::vector<std::string> &names);
  ClassScopeRawPtr getClassScope() const {
    BlockScopeRawPtr b = getScope();
    assert(b->is(BlockScope::ClassScope));
    return ClassScopeRawPtr((ClassScope*)b.get());
  }

  StatementListPtr getStmts() { return m_stmt; }
  void checkArgumentsToPromote(ExpressionListPtr params, int type);
protected:
  std::string m_originalName;
  std::string m_name;
  ExpressionListPtr m_base;
  std::string m_docComment;
  StatementListPtr m_stmt;
  ExpressionListPtr m_attrList;
  void checkVolatile(AnalysisResultConstPtr ar);
private:
  bool checkVolatileBases(AnalysisResultConstPtr ar);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_INTERFACE_STATEMENT_H_

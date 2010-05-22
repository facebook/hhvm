/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __INTERFACE_STATEMENT_H__
#define __INTERFACE_STATEMENT_H__

#include <compiler/statement/scope_statement.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(InterfaceStatement);

class InterfaceStatement : public Statement, public IParseHandler,
                           public IScopeStatement {
public:
  InterfaceStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                     const std::string &name, ExpressionListPtr base,
                     const std::string &docComment, StatementListPtr stmt);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  virtual bool hasDecl() const { return true; }
  virtual bool hasImpl() const;
  virtual int getRecursiveCount() const;
  // implementing IParseHandler
  virtual void onParse(AnalysisResultPtr ar);

  virtual BlockScopePtr getScope();
  ClassScopePtr getClassScope() { return m_classScope.lock();}
  virtual std::string getName() const;
  std::string getOriginalName() const { return m_originalName;}
  virtual void getAllParents(AnalysisResultPtr ar,
                             std::vector<std::string> &names);

protected:
  std::string m_originalName;
  std::string m_name;
  ExpressionListPtr m_base;
  std::string m_docComment;
  StatementListPtr m_stmt;
  boost::weak_ptr<ClassScope> m_classScope;
  void checkVolatile(AnalysisResultPtr ar);
private:
  bool checkVolatileBases(AnalysisResultPtr ar);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __INTERFACE_STATEMENT_H__

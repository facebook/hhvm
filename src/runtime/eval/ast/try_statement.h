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

#ifndef __EVAL_TRY_STATEMENT_H__
#define __EVAL_TRY_STATEMENT_H__

#include <runtime/eval/ast/statement.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(CatchBlock);
DECLARE_AST_PTR(TryStatement);

class CatchBlock : public Construct {
public:
  CatchBlock(CONSTRUCT_ARGS, const std::string &ename, const std::string &vname,
             StatementPtr body);
  bool proc(CObjRef exn, VariableEnvironment &env) const;
  bool match(CObjRef exn) const;
  const StatementPtr &body() const { return m_body; }
  const std::string &vname() const { return m_vname; }
  virtual void dump() const;
private:
  std::string m_ename;
  std::string m_vname;
  StatementPtr m_body;
};

class TryStatement : public Statement {
public:
  TryStatement(STATEMENT_ARGS, StatementPtr body,
               const std::vector<CatchBlockPtr> &catches);
  virtual void eval(VariableEnvironment &env) const;
  virtual void dump() const;
private:
  std::vector<CatchBlockPtr> m_catches;
  StatementPtr m_body;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_TRY_STATEMENT_H__ */

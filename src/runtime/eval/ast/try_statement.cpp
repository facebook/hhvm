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

#include <runtime/eval/ast/try_statement.h>
#include <runtime/eval/runtime/variable_environment.h>

using namespace std;

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

CatchBlock::CatchBlock(CONSTRUCT_ARGS, const string &ename,
                       const string &vname, StatementPtr body)
  : Construct(CONSTRUCT_PASS), m_ename(ename),
    m_vname(vname), m_body(body) {}

bool CatchBlock::match(CObjRef exn) const {
  return exn.instanceof(m_ename.c_str());
}

bool CatchBlock::proc(CObjRef exn, VariableEnvironment &env) const {
  if (exn.instanceof(m_ename.c_str())) {
    if (m_body) {
      env.get(m_vname) = exn;
      m_body->eval(env);
    }
    return true;
  }
  return false;
}

void CatchBlock::dump() const {
  printf("catch (%s %s) {", m_ename.c_str(), m_vname.c_str());
  if (m_body) m_body->dump();
  printf("}");
}

TryStatement::TryStatement(STATEMENT_ARGS, StatementPtr body,
                           const std::vector<CatchBlockPtr> &catches)
  : Statement(STATEMENT_PASS), m_catches(catches), m_body(body) {}

void TryStatement::eval(VariableEnvironment &env) const {
  ENTER_STMT;
  try {
    m_body->eval(env);
  } catch (Object e) {
    for (vector<CatchBlockPtr>::const_iterator it = m_catches.begin();
         it != m_catches.end(); ++it) {
      if ((*it)->match(e)) {
        if ((*it)->body()) {
          env.get((*it)->vname()) = e;
          EVAL_STMT((*it)->body(), env);
        }
        return;
      }
    }
    throw e;
  }
}

void TryStatement::dump() const {
  printf("try {");
  m_body->dump();
  printf("}");
  dumpVector(m_catches, " ");
}

///////////////////////////////////////////////////////////////////////////////
}
}


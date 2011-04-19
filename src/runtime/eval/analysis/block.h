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

#ifndef __EVAL_BLOCK_H__
#define __EVAL_BLOCK_H__

#include <runtime/eval/base/eval_base.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(Expression);
DECLARE_AST_PTR(StaticStatement);
class VariableEnvironment;

class VariableIndex {
public:
  enum SuperGlobal {
    Server = 0,
    Get,
    Post,
    Files,
    Cookie,
    Session,
    Request,
    Env,
    Globals,
    HttpResponseHeader,
    Normal
  };
  void set(CStrRef name, int idx);
  int idx() const { return m_idx; }
  SuperGlobal superGlobal() const { return m_sg; }
  static SuperGlobal isSuperGlobal(CStrRef name);
private:
  int m_idx;
  SuperGlobal m_sg;
};

class Block {
public:
  // Varname -> idx
  typedef std::map<std::string, VariableIndex> VariableIndices;

  Block();
  ~Block();
  Block(const std::vector<StaticStatementPtr> &stat,
        const Block::VariableIndices &variableIndices);
  void declareStaticStatement(StaticStatementPtr stat);
  Variant getStaticValue(VariableEnvironment &env, CStrRef name) const;
  int declareVariable(CStrRef var);
  const VariableIndices &varIndices() const;
  const std::vector<std::string> &variables() const { return m_variables;}
protected:
  StringMap<ExpressionPtr> m_staticStmts;
  VariableIndices m_variableIndices;
  std::vector<std::string> m_variables; // in declaration order
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_BLOCK_H__ */

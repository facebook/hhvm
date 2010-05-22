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

#ifndef __EVAL_STATEMENT_H__
#define __EVAL_STATEMENT_H__

#include <runtime/eval/ast/construct.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(Statement);

class ByteCodeProgram;

#define STATEMENT_ARGS CONSTRUCT_ARGS
#define STATEMENT_PASS CONSTRUCT_PASS

class Statement : public Construct {
public:
  Statement(STATEMENT_ARGS) : Construct(CONSTRUCT_PASS) {};
  virtual void eval(VariableEnvironment &env) const = 0;
  virtual void byteCode(ByteCodeProgram &code) const;
};

#define EVAL_STMT(stmt, env)                                                  \
  {                                                                           \
    (stmt)->eval(env);                                                        \
    if (env.isEscaping()) return;                                             \
  }
#define EVAL_STMT_HANDLE_BREAK(stmt, env)                                     \
  {                                                                           \
    (stmt)->eval(env);                                                        \
    int hb = env.handleBreak();                                               \
    if (hb == 1 || env.isReturning()) return;                                 \
    if (hb == 2) break;                                                       \
  }
#define EVAL_STMT_HANDLE_BREAK_CONT(stmt, env)                                \
  {                                                                           \
    (stmt)->eval(env);                                                        \
    int hb = env.handleBreak();                                               \
    if (hb == 1 || env.isReturning()) return;                                 \
    if (hb == 2 || hb == 3) break;                                            \
  }

#define ENTER_STMT \
  SET_LINE

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_STATEMENT_H__ */

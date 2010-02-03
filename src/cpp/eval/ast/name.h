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

#ifndef __EVAL_NAME_H__
#define __EVAL_NAME_H__

#include <cpp/eval/ast/construct.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(Name);
DECLARE_AST_PTR(Expression);
class VariableEnvironment;

class Name : public Construct {
public:
  Name(CONSTRUCT_ARGS);
  virtual String get(VariableEnvironment &env) const = 0;
  virtual int64 hash() const;
  virtual int64 hashLwr() const;
  virtual String getStatic() const { return String(); }
  static NamePtr fromString(CONSTRUCT_ARGS, const std::string &name);
  static NamePtr fromExp(CONSTRUCT_ARGS, ExpressionPtr e);
};

class StringName : public Name {
public:
  StringName(CONSTRUCT_ARGS, const std::string &name);
  virtual String get(VariableEnvironment &env) const;
  virtual int64 hash() const;
  virtual int64 hashLwr() const;
  virtual String getStatic() const;
  virtual void dump() const;
private:
  std::string m_name;
  int64 m_hash;
  int64 m_hashLwr;
};

class ExprName : public Name {
public:
  ExprName(CONSTRUCT_ARGS, ExpressionPtr name);
  virtual String get(VariableEnvironment &env) const;
  virtual void dump() const;
private:
  ExpressionPtr m_name;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_NAME_H__ */

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

#ifndef __EVAL_NAME_H__
#define __EVAL_NAME_H__

#include <runtime/eval/ast/construct.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(Name);
DECLARE_AST_PTR(Expression);
class VariableEnvironment;
class ByteCodeProgram;

class Name : public Construct {
public:
  Name(CONSTRUCT_ARGS);
  virtual String get(VariableEnvironment &env) const = 0;
	virtual Variant getAsVariant(VariableEnvironment &env) const {
	 return get(env);
	}
  virtual int64 hash() const;
  virtual String get() const { return String(); }
  virtual bool isSp() const;
  static NamePtr fromString(CONSTRUCT_ARGS, const std::string &name,
      bool isSp = false);
  static NamePtr fromString(CONSTRUCT_ARGS, CStrRef name, bool isSp = false);
  static NamePtr fromExp(CONSTRUCT_ARGS, ExpressionPtr e);
  static NamePtr fromStaticClassExp(CONSTRUCT_ARGS, ExpressionPtr e);
#ifdef ENABLE_LATE_STATIC_BINDING
  static NamePtr LateStatic(CONSTRUCT_ARGS);
#endif
  void setOriginalText(const std::string &text) { m_originalText = text;}
  const std::string &getOriginalText() const { return m_originalText;}

private:
  std::string m_originalText;
};

class StringName : public Name {
public:
  StringName(CONSTRUCT_ARGS, const std::string &name, bool isSp = false);
  StringName(CONSTRUCT_ARGS, CStrRef name, bool isSp = false);
  virtual String get(VariableEnvironment &env) const;
  virtual int64 hash() const;
  virtual String get() const;
  virtual bool isSp() const;
  virtual void dump(std::ostream &out) const;
private:
  AtomicString m_name;
  bool m_isSp;
};

class ExprName : public Name {
public:
  ExprName(CONSTRUCT_ARGS, ExpressionPtr name);
  virtual String get(VariableEnvironment &env) const {
		return getAsVariant(env).toString();
	}
  virtual Variant getAsVariant(VariableEnvironment &env) const;
  virtual String get() const { return Name::get(); }
  virtual void dump(std::ostream &out) const;

  ExpressionPtr getExp();
  void setExp(ExpressionPtr name);

protected:
  ExpressionPtr m_name;
};

class StaticClassExprName : public ExprName {
public:
  StaticClassExprName(CONSTRUCT_ARGS, ExpressionPtr name);
  virtual String get(VariableEnvironment &env) const;
  virtual String get() const { return ExprName::get(); }
};

#ifdef ENABLE_LATE_STATIC_BINDING
class LateStaticName : public Name {
public:
  LateStaticName(CONSTRUCT_ARGS);
  virtual String get(VariableEnvironment &env) const;
  virtual String get() const { return Name::get(); }
  virtual void dump(std::ostream &out) const;
};
#endif

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_NAME_H__ */

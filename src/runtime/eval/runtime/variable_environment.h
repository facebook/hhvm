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

#ifndef __EVAL_RUNTIME_VARIABLE_ENVIRONMENT_H__
#define __EVAL_RUNTIME_VARIABLE_ENVIRONMENT_H__

#include <runtime/eval/runtime/assoc_list.h>
#include <runtime/eval/base/eval_base.h>
#include <runtime/eval/ast/class_statement.h>
#include <stack>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class FunctionStatement;
class Block;
class ClassStatement;

class GotoException {};
class UnlimitedGotoException {};

class VariableEnvironment : public LVariableTable {
public:
  VariableEnvironment();
  void setCurrentObject(CObjRef co);
  void setCurrentClass(const char* cls);
  virtual void flagStatic(CStrRef name, int64 hash = -1) = 0;
  virtual void flagGlobal(CStrRef name, int64 hash = -1);
  virtual void unset(CStrRef name, int64 hash = -1);
  virtual Variant *getIdx(int idx);
  virtual void setIdx(int idx, Variant *v);
  Variant &currentObject() { return m_currentObject; }
  virtual const char* currentClass() const;
  virtual const ClassStatement *currentClassStatement() const;
  virtual String currentContext() const;
  virtual Array getParams() const = 0;
  virtual bool refReturn() const { return false; }
  virtual Array getDefinedVariables() const;

  /**
   * Return the continuation object if it is in a generator function,
   * NULL otherwise.
   */
  virtual ObjectData *getContinuation() const { return NULL; }

  void setBreak(int n) { m_breakLevel = n; }
  void decBreak() {
    if (m_breakLevel > 0) {
      m_breakLevel--;
    } else if (m_breakLevel < 0) {
      m_breakLevel++;
    }
  }
  int handleBreak() {
    int r = m_breakLevel;
    decBreak();
    if (m_breakLevel == 0) {
      if (r == 1) return 2;
      if (r == -1) return 3;
    }
    if (m_breakLevel != 0) return 1;
    return 0;
  }
  Variant &getRet() { return m_ret; }
  void setRet(CVarRef ret) { m_ret = ret; m_returning = true; }
  void setRet() { m_returning = true; }
  bool isReturning() const { return m_returning; }
  bool isBreaking() const {
    if (m_breakLevel != 0) {
      return true;
    }
    return false;
  }
  bool isEscaping() const { return isBreaking() || m_returning; }
  void *getClosure() { return m_closure;}
  void setClosure(void *closure) { m_closure = closure;}
  bool isGotoing() const { return !m_label.empty();}
  bool isLimitedGoto() const { ASSERT(isGotoing()); return m_limitedGoto;}
  void setGoto(const std::string &label, bool limited) {
    m_label = label;
    m_limitedGoto = limited;
  }
  void resetGoto() { m_label.clear();}
  const std::string &getGoto() const { return m_label;}

  /**
   * Storing temporary variables for TempExpressionList.
   */
  std::vector<Variant> &createTempVariables();
  Variant getTempVariable(int index);
  void releaseTempVariables();

protected:
  Variant m_currentObject;
  const char* m_currentClass;
  int m_breakLevel;
  bool m_returning;
  void *m_closure;
  std::string m_label;
  bool m_limitedGoto;
  Variant m_ret;

  std::list<std::vector<Variant> > m_tempStack;
};

/**
 * This is gross but I need it to eval statics sometimes.
 */
class DummyVariableEnvironment : public VariableEnvironment {
public:
  DummyVariableEnvironment();
  virtual void flagStatic(CStrRef name, int64 hash = -1);
  virtual void flagGlobal(CStrRef name, int64 hash = -1);
  virtual void unset(CStrRef name, int64 hash = -1);
  virtual bool exists(CStrRef name) const;
  virtual Variant &getImpl(CStrRef s);
  virtual Array getParams() const;
};

/**
 * Used by functions and methods. Pass in an env for statics.
 */
class FuncScopeVariableEnvironment : public VariableEnvironment {
public:
  FuncScopeVariableEnvironment(const FunctionStatement *func);
  ~FuncScopeVariableEnvironment();
  virtual void flagStatic(CStrRef name, int64 hash = -1);
  virtual Variant *getIdx(int idx);
  virtual void setIdx(int idx, Variant *v);
  virtual bool refReturn() const;
  virtual Array getParams() const;
  virtual bool exists(CStrRef name) const;
  virtual Variant &getImpl(CStrRef s);
  void incArgc() { m_argc++; }
  virtual Array getDefinedVariables() const;
  virtual ObjectData *getContinuation() const;
private:
  Array m_statics;
  const FunctionStatement *m_func;
  LVariableTable *m_staticEnv;
  std::vector<Variant*> m_byIdx;
  AssocList m_alist;
  int m_argc;
  uint m_argStart;
};

class MethScopeVariableEnvironment : public FuncScopeVariableEnvironment {
public:
  MethScopeVariableEnvironment(const MethodStatement *meth);
  virtual String currentContext() const;
  const ClassStatement *currentClassStatement() const;
private:
  const ClassStatement *m_cls;
};

/**
 * Used to wrap a variable table for eval calls.
 */
class NestedVariableEnvironment : public VariableEnvironment {
public:
  NestedVariableEnvironment(LVariableTable *ext,
                            const Block &blk,
                            CArrRef params = Array(),
                            CObjRef current_object = Object());
  virtual void flagStatic(CStrRef name, int64 hash = -1);
  virtual Variant *getIdx(int idx);
  virtual void setIdx(int idx, Variant *v);
  virtual bool exists(CStrRef s) const;
  virtual Variant &getImpl(CStrRef s);
  virtual Array getParams() const;
  virtual Array getDefinedVariables() const;
private:
  LVariableTable *m_ext;
  std::vector<Variant*> m_byIdx;
  const Block &m_block;
  Array m_params;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_RUNTIME_VARIABLE_ENVIRONMENT_H__ */

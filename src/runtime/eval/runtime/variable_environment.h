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

#ifndef __EVAL_RUNTIME_VARIABLE_ENVIRONMENT_H__
#define __EVAL_RUNTIME_VARIABLE_ENVIRONMENT_H__

#include <runtime/eval/runtime/assoc_list.h>
#include <runtime/eval/base/eval_base.h>
#include <stack>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class FunctionStatement;
class Block;
class ClassStatement;

class VariableEnvironment : public LVariableTable {
public:
  VariableEnvironment();
  void setCurrentObject(CObjRef co);
  void setCurrentClass(const char* cls);
  virtual void flagStatic(CStrRef name, int64 hash) = 0;
  virtual void flagGlobal(CStrRef name, int64 hash);
  virtual void unset(CStrRef name, int64 hash);
  virtual Variant &getIdx(int idx);
  Variant &currentObject() { return m_currentObject; }
  virtual const char* currentClass() const;
  virtual const ClassStatement *currentClassStatement() const;
  virtual const char* currentContext() const;
  virtual Array getParams() const = 0;
  virtual bool refReturn() const { return false; }
  virtual Array getDefinedVariables() const;

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
protected:
  Variant m_currentObject;
  const char* m_currentClass;
  int m_breakLevel;
  bool m_returning;
  Variant m_ret;
};

/**
 * This is gross but I need it to eval statics sometimes.
 */
class DummyVariableEnvironment : public VariableEnvironment {
public:
  DummyVariableEnvironment();
  virtual void flagStatic(CStrRef name, int64 hash);
  virtual void flagGlobal(CStrRef name, int64 hash);
  virtual void unset(CStrRef name, int64 hash);
  virtual bool exists(const char *name, int64 hash = -1) const;
  virtual Variant &getImpl(CStrRef s, int64 hash);
  virtual Array getParams() const;
};

/**
 * Used by functions and methods. Pass in an env for statics.
 */
class FuncScopeVariableEnvironment : public VariableEnvironment {
public:
  FuncScopeVariableEnvironment(const FunctionStatement *func, int argc);
  ~FuncScopeVariableEnvironment();
  virtual void flagStatic(CStrRef name, int64 hash);
  virtual Variant &getIdx(int idx);
  virtual bool refReturn() const;
  virtual Array getParams() const;
  virtual bool exists(const char *name, int64 hash = -1) const;
  virtual Variant &getImpl(CStrRef s, int64 hash);
  void incArgc() { m_argc++; }
  virtual Array getDefinedVariables() const;
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
  MethScopeVariableEnvironment(const MethodStatement *meth, int argc);
  virtual const char* currentContext() const;
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
  virtual void flagStatic(CStrRef name, int64 hash);
  virtual bool exists(const char *s, int64 hash = 1) const;
  virtual Variant &getImpl(CStrRef s, int64 hash);
  virtual Array getParams() const;
  virtual Array getDefinedVariables() const;
private:
  LVariableTable *m_ext;
  const Block &m_block;
  Array m_params;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_RUNTIME_VARIABLE_ENVIRONMENT_H__ */

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

#ifndef __EVAL_CLASS_STATEMENT_H__
#define __EVAL_CLASS_STATEMENT_H__

#include <runtime/eval/ast/statement.h>
#include <runtime/base/class_info.h>
#include <util/case_insensitive.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(Expression);
DECLARE_AST_PTR(ClassVariable);
DECLARE_AST_PTR(ClassStatement);
DECLARE_AST_PTR(MethodStatement);
DECLARE_AST_PTR(ScalarExpression);
DECLARE_AST_PTR(ClassStatementMarker);
class EvalObjectData;
class ClassInfoEvaled;
class ClassEvalState;

class ClassVariable : public Construct {
public:
  ClassVariable(CONSTRUCT_ARGS, const std::string &name, int modifiers,
      ExpressionPtr value, const std::string &doc, ClassStatement *cls);
  void set(VariableEnvironment &env, EvalObjectData *self) const;
  void setStatic(VariableEnvironment &env, LVariableTable &statics) const;
  virtual void dump(std::ostream &out) const;
  String name() const { return m_name; }
  void getInfo(ClassInfo::PropertyInfo &info) const;
  int getModifiers() const { return m_modifiers; }
  void eval(VariableEnvironment &env, Variant &res) const;
  int64 getHash() const { return m_name->hash(); }
  bool hasInitialValue() const { return m_value; }
private:
  AtomicString m_name;
  int m_modifiers;
  ExpressionPtr m_value;
  std::string m_docComment;
  ClassStatement *m_cls;
};

class ClassStatement : public Statement {
public:
  enum Modifier {
    Public = 1,
    Protected = 2,
    Private = 4,
    AccessMask = Public|Protected|Private,
    Static = 8,
    Abstract = 16,
    Final = 32,
    Interface = 64
  };

  ClassStatement(STATEMENT_ARGS, const std::string &name,
                 const std::string &parent, const std::string &doc);
  void finish();
  String name() const { return m_name; }
  String parent() const { return m_parent; }
  const ClassStatement *parentStatement() const;
  void loadInterfaceStatements() const;
  void setModifiers(int m) { m_modifiers = m; }
  int getModifiers() const { return m_modifiers; }
  void addBases(const std::vector<String> &bases);
  void addVariable(ClassVariablePtr v);
  void addMethod(MethodStatementPtr m);
  void addConstant(const std::string &name, ExpressionPtr v);

  bool instanceOf(const char *c) const;
  bool subclassOf(const char *c) const;
  bool isBaseClass() const {
    return m_parent.empty() && m_bases.empty();
  }

  // Eval is called at declaration, not invocation
  virtual void eval(VariableEnvironment &env) const;
  void evalImpl(VariableEnvironment &env) const;
  // Called by create_class
  Object create(ClassEvalState &ce, ObjectData* root = NULL) const;
  void initializeObject(EvalObjectData *obj) const;
  void initializeStatics(LVariableTable &statics) const;

  const MethodStatement* findMethod(const char* name,
                                    bool recursive = false,
                                    bool interfaces = false) const;
  const ClassVariable* findVariable(CStrRef name,
                                    bool recursive = false) const;
  bool getConstant(Variant &res, const char *c,
                   bool recursive = false) const;

  virtual void dump(std::ostream &out) const;
  static void dumpModifiers(std::ostream &out, int m, bool variable);

  void getPropertyInfo(ClassInfoEvaled &owner) const;
  void getInfo(ClassInfoEvaled &info) const;

  bool hasAccess(const char *context, Modifier level) const;
  bool attemptPropertyAccess(CStrRef prop, const char *context,
      int &mods, bool rec = false) const;
  void failPropertyAccess(CStrRef prop, const char *context,
      int mods) const;
  void toArray(Array &props, Array &vals) const;
  void loadMethodTable(ClassEvalState &ce) const;
  void semanticCheck(const ClassStatement *cls) const;
  ClassStatementMarkerPtr getMarker() const;
  void delayDeclaration() { m_delayDeclaration = true; }
protected:
  AtomicString m_name;
  int m_modifiers;

  AtomicString m_parent;
  std::vector<AtomicString> m_bases;

  StringMap<ClassVariablePtr> m_variables;
  std::vector<ClassVariablePtr> m_variablesVec;
  hphp_const_char_imap<MethodStatementPtr> m_methods;
  std::vector<MethodStatementPtr> m_methodsVec;
  std::vector<AtomicString> m_constantNames;
  StringMap<ExpressionPtr> m_constants;

  std::string m_docComment;
  ClassStatementMarkerPtr m_marker;
  bool m_delayDeclaration;

  void loadProperties(ClassInfoEvaled &info) const;
  const MethodStatement* findParentMethod(const char* name,
      bool interface) const;
  const ClassInfo *getBuiltinParentInfo() const;
  void abstractMethodCheck(hphp_const_char_imap<const char*> &abstracts,
      bool ifaces) const;
  void recursiveParentCheck(std::set<const ClassStatement*> &seen) const;
};

class ClassStatementMarker : public Statement {
public:
  ClassStatementMarker(STATEMENT_ARGS, ClassStatement *cls);
  virtual void eval(VariableEnvironment &env) const;
  virtual bool skipDump() const { return true;}
  virtual void dump(std::ostream &out) const;
protected:
  ClassStatement *m_class;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_CLASS_STATEMENT_H__ */

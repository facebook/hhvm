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
#include <runtime/base/string_data.h>
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
DECLARE_AST_PTR(UseTraitStatement);
class EvalObjectData;
class ClassInfoEvaled;
class ClassEvalState;
class Name;

struct SemanticExtractor;

class ClassVariable : public Construct {
public:
  ClassVariable(CONSTRUCT_ARGS, const std::string &name, int modifiers,
      ExpressionPtr value, const std::string &doc, ClassStatement *cls);
  virtual ClassVariable *optimize(VariableEnvironment &env);
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
  StringData *m_name;
  int m_modifiers;
  ExpressionPtr m_value;
  std::string m_docComment;
};

class ClassStatement : public Statement {
  friend struct SemanticExtractor;
public:
  enum Modifier {
    Public = 1,
    Protected = 2,
    Private = 4,
    AccessMask = Public|Protected|Private,
    Static = 8,
    Abstract = 16,
    Final = 32,
    Interface = 64,
    Trait = 128,
    Constructor = 256
  };

  ClassStatement(STATEMENT_ARGS, const std::string &name,
                 const std::string &parent, const std::string &doc);
  void finish();
  String name() const { return m_name; }
  String parent() const { return m_parent; }
  bool isTrait() const { return m_modifiers & Trait; }
  const ClassStatement *parentStatement() const;
  void loadInterfaceStatements() const;
  void setModifiers(int m) { m_modifiers = m; }
  int getModifiers() const { return m_modifiers; }
  int getAttributes() const { return m_attributes; }
  const std::vector<MethodStatementPtr> &getMethods() const {
    return m_methodsVec;
  }
  const std::vector<UseTraitStatementPtr> &getUseTraits() const {
    return m_useTraitsVec;
  }
  void addBases(const std::vector<String> &bases);
  void addVariable(ClassVariablePtr v);
  void addMethod(MethodStatementPtr m);
  void addUseTrait(UseTraitStatementPtr u);
  void addConstant(const std::string &name, ExpressionPtr v);

  bool instanceOf(CStrRef c) const;
  bool subclassOf(CStrRef c) const;
  bool isBaseClass() const {
    return m_parent->empty() && m_bases.empty();
  }
  bool isHoistable(std::set<StringData *> &seen);
  bool checkHoist() const;

  virtual Statement *optimize(VariableEnvironment &env);
  // Eval is called at declaration, not invocation
  virtual void eval(VariableEnvironment &env) const;
  void evalImpl(VariableEnvironment &env, bool fromMarker = false) const;
  // Called by create_class
  Object create(ClassEvalState &ce, ObjectData* root = NULL) const;
  void initializeObject(ClassEvalState &ce, EvalObjectData *obj) const;
  void initializeStatics(ClassEvalState &ce, LVariableTable &statics) const;

  const MethodStatement* findMethod(CStrRef name,
                                    bool recursive = false,
                                    bool interfaces = false,
                                    bool trait = true) const;
  const ClassVariable* findVariable(CStrRef name,
                                    bool recursive = false) const;
  bool getConstant(Variant &res, const char *c,
                   bool recursive = false) const;

  virtual void dump(std::ostream &out) const;
  static void dumpModifiers(std::ostream &out, int m, bool variable);
  static String resolveSpInTrait(VariableEnvironment &env, CObjRef currentObj,
                                 Name *clsName);

  void getPropertyInfo(ClassInfoEvaled &owner) const;
  void getInfo(ClassInfoEvaled &info) const;

  bool hasAccess(CStrRef context, Modifier level) const;
  bool attemptPropertyAccess(CStrRef prop, CStrRef context,
      int &mods, bool rec = false) const;
  void failPropertyAccess(CStrRef prop, CStrRef context,
      int mods) const;
  void toArray(Array &props, Array &vals) const;
  void addTrait(ClassEvalState &ce,
                const UseTraitStatement *useTraitStmt) const;
  void addTraits(ClassEvalState &ce) const;
  void bindTraits(ClassEvalState &ce) const;
  void loadMethodTable(ClassEvalState &ce) const;
  void semanticCheck(const ClassStatement *cls) const;
  ClassStatementMarkerPtr getMarker();
protected:
  StringData *m_name;
  int m_modifiers;
  int m_attributes;
  int m_needCheckHoist;
  StringData *m_parent;
  std::vector<StringData *> m_bases;

  StringMap<ClassVariablePtr> m_variables;
  std::vector<ClassVariablePtr> m_variablesVec;
  StringIMap<MethodStatementPtr> m_methods;
  std::vector<MethodStatementPtr> m_methodsVec;
  std::vector<UseTraitStatementPtr> m_useTraitsVec;
  std::vector<StringData *> m_constantNames;
  StringMap<ExpressionPtr> m_constants;

  std::string m_docComment;
  ClassStatementMarkerPtr m_marker;

  void loadProperties(ClassInfoEvaled &info) const;
  const MethodStatement* findParentMethod(CStrRef name, bool interface) const;
  const ClassInfo *getBuiltinParentInfo() const;
  void collectBuiltinInterfaceInfos(
      std::vector<const ClassInfo*>& infos,
      bool excludeParent) const;
  void abstractMethodCheck(StringIMap<String> &abstracts,
                           bool ifaces) const;
private:
  void initTraitStructures(ClassEvalState &ce) const;
  void bindMethods(ClassEvalState &ce) const;
  void bindProperties(ClassEvalState &ce) const;

  template <typename ParentClass, typename ChildClass>
  static void ClassLevelMethodOverrideCheck(ParentClass parent,
                                            ChildClass  child);

  template <typename ParentClass, typename ChildClass>
  static void ClassLevelPropertyOverrideCheck(ParentClass parent,
                                              ChildClass  child);

  template <typename ParentClass, typename ChildClass>
  static void ClassLevelMethodAccessLevelCheck(ParentClass parent,
                                               ChildClass  child);


  static void BuiltinSemanticCheck(const ClassInfo      *parent,
                                   const ClassStatement *child);
};

class ClassStatementMarker : public Statement {
public:
  ClassStatementMarker(ClassStatement *cls, const Location *loc);
  virtual void eval(VariableEnvironment &env) const;
  virtual bool skipDump() const { return true;}
  virtual void dump(std::ostream &out) const;
protected:
  ClassStatement *m_class;
};

void optimize(VariableEnvironment &env, ClassVariablePtr &before);

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_CLASS_STATEMENT_H__ */

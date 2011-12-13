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

#ifndef __EVAL_STATE_H__
#define __EVAL_STATE_H__

#include <runtime/eval/base/eval_base.h>
#include <runtime/base/class_info.h>
#include <runtime/eval/runtime/variant_stack.h>
#include <util/case_insensitive.h>
#include <util/atomic.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(ClassStatement);
DECLARE_AST_PTR(ClassVariable);
DECLARE_AST_PTR(FunctionStatement);
DECLARE_AST_PTR(MethodStatement);
DECLARE_AST_PTR(Statement);
DECLARE_AST_PTR(TraitPrecStatement);
DECLARE_AST_PTR(TraitAliasStatement);
class PhpFile;
class Function;
class EvalObjectData;

class ClassEvalState {
public:
  typedef StringIMap<std::pair<const MethodStatement*, int> >
    MethodTable;
  struct TraitPrecedence {
  public:
    TraitPrecedence(CStrRef trait, CStrRef method, TraitPrecStatement *prec) :
      m_trait(trait), m_method(method), m_prec(prec), m_classEvalState(NULL) {}
    String m_trait;
    String m_method;
    TraitPrecStatement *m_prec;
    ClassEvalState *m_classEvalState;
    std::vector<ClassEvalState *> m_excludeFromClasses;
  };
  class TraitAlias {
  public:
    TraitAlias(CStrRef trait, CStrRef method, CStrRef alias, int modifiers) :
               m_trait(trait), m_method(method),
               m_alias(alias), m_modifiers(modifiers),
               m_classEvalState(NULL) {}
    String getFullName() const {
      String trait = (m_trait.empty() ? String("(null)") : m_trait);
      return trait + "::" + m_method;
    }
    String m_trait;
    String m_method;
    String m_alias;
    int m_modifiers;
    ClassEvalState *m_classEvalState;
  };

  ClassEvalState() : m_class(NULL), m_parentClassEvalState(NULL), m_constructor(NULL),
                     m_attributes(0),
                     m_initializedInstance(false),
                     m_initializedStatics(false),
                     m_doneSemanticCheck(false)
  {}
  void init(const ClassStatement *cls);
  const ClassStatement *getClass() const {
    return m_class;
  }
  ClassEvalState *getParentClassEvalState() {
    return m_parentClassEvalState;
  }
  void setParentClassEvalState(ClassEvalState *pce) {
    ASSERT(pce);
    m_parentClassEvalState = pce;
  }
  const MethodStatement *getMethod(CStrRef m, int &access);
  const MethodStatement *getTraitMethod(CStrRef m, int &access);
  MethodTable &getMethodTable() {
    return m_methodTable;
  }
  MethodTable &getTraitMethodTable() {
    return m_traitMethodTable;
  }
  const std::vector<ClassVariable *> &getTraitVariables() const {
    return m_traitVariables;
  }
  void setTraitVariables(const std::vector<ClassVariable *> &traitVariables) {
    m_traitVariables = traitVariables;
  }
  std::vector<const ClassStatement *> &getTraits() {
    return m_traits;
  }
  std::vector<TraitPrecedence> &getTraitPrecedences() {
    return m_traitPrecedences;
  }
  std::vector<TraitAlias> &getTraitAliases() {
    return m_traitAliases;
  }
  void initTraits() {
    m_traits.clear();
    m_traitPrecedences.clear();
    m_traitAliases.clear();
  }
  const MethodStatement* &getConstructor() {
    return m_constructor;
  }
  LVariableTable &getStatics() {
    return m_statics;
  }
  void setAttributes(int attrs) { m_attributes |= attrs; }
  int getAttributes() const { return m_attributes; }
  void initializeInstance();
  void initializeStatics();
  void semanticCheck();
  void fiberInit(ClassEvalState &oces, FiberReferenceMap &refMap);
  void fiberInitStatics(ClassEvalState &oces, FiberReferenceMap &refMap);
  void fiberExit(ClassEvalState &oces, FiberReferenceMap &refMap,
                 FiberAsyncFunc::Strategy default_strategy);
  void fiberExitStatics(ClassEvalState &oces, FiberReferenceMap &refMap,
                        FiberAsyncFunc::Strategy default_strategy);
  void implementTrait(const ClassStatement *trait);
  void compileExcludeTable(
    StringISet &excludeTable,
    const std::vector<ClassEvalState::TraitPrecedence> &traitPrecedences,
    const ClassStatement *trait);
  void copyTraitMethodTable(
    MethodTable &methodTable,
    const ClassStatement *trait,
    const std::vector<TraitAlias> &traitAliases,
    const StringISet &excludeTable);
  void mergeTraitMethods(const MethodTable &currentTable,
    unsigned int current, unsigned count,
    std::vector<MethodTable> &methodTables,
    MethodTable &resultTable);
  void mergeTraitMethodsToClass(const MethodTable &resultTable,
                                const ClassStatement *cls);

  std::set<const ClassStatement*> &getSeen() { return m_seen; }
private:
  const ClassStatement *m_class;
  ClassEvalState *m_parentClassEvalState;
  MethodTable m_methodTable;
  std::vector<ClassVariable *> m_traitVariables;
  const MethodStatement *m_constructor;
  LVariableTable m_statics;
  int m_attributes;
  bool m_initializedInstance;
  bool m_initializedStatics;
  bool m_doneSemanticCheck;
  std::vector<const ClassStatement *> m_traits;
  std::vector<TraitPrecedence> m_traitPrecedences;
  std::vector<TraitAlias> m_traitAliases;
  MethodTable m_traitMethodTable;
  std::set<const ClassStatement*> m_seen;
};


/**
 * Containers to live in Globals and do the garbage collection.
 */
class CodeContainer : public AtomicCountable {
public:
  virtual ~CodeContainer() {}
  void release();
};

class StringCodeContainer : public CodeContainer {
public:
  StringCodeContainer(StatementPtr s);
  ~StringCodeContainer();
private:
  StatementPtr m_s;
};

class EvalConstantInfo : public ClassInfo::ConstantInfo,
  public AtomicCountable {
public:
  void release() { delete this; }
};

class EvalMethodInfo :  public ClassInfo::MethodInfo, public AtomicCountable {
public:
  void release() { delete this; }
};

class ClassInfoEvaled : public ClassInfo, public AtomicCountable {
 public:
  ~ClassInfoEvaled();
  void release() { delete this; }
  virtual CStrRef getParentClass() const { return m_parentClass; }

  // implementing ClassInfo
  const InterfaceSet  &getInterfaces()      const { return m_interfaces;}
  const InterfaceVec  &getInterfacesVec()   const { return m_interfacesVec;}
  const TraitSet      &getTraits()          const { return m_traits;}
  const TraitVec      &getTraitsVec()       const { return m_traitsVec;}
  const TraitAliasVec &getTraitAliasesVec() const { return m_traitAliasesVec;}
  const MethodMap     &getMethods()         const { return m_methods;}
  const MethodVec     &getMethodsVec()      const { return m_methodsVec;}
  const PropertyMap   &getProperties()      const { return m_properties;}
  const PropertyVec   &getPropertiesVec()   const { return m_propertiesVec;}
  const ConstantMap   &getConstants()       const { return m_constants;}
  const ConstantVec   &getConstantsVec()    const { return m_constantsVec;}

 private:
  friend class ClassStatement;
  String        m_parentClass;
  InterfaceSet  m_interfaces;      // all interfaces
  InterfaceVec  m_interfacesVec;   // all interfaces
  TraitSet      m_traits;          // all traits
  TraitVec      m_traitsVec;       // all traits
  TraitAliasVec m_traitAliasesVec; // trait aliases
  MethodMap     m_methods;         // all methods
  MethodVec     m_methodsVec;      // in source order
  PropertyMap   m_properties;      // all properties
  PropertyVec   m_propertiesVec;   // in source order
  ConstantMap   m_constants;       // all constants
  ConstantVec   m_constantsVec;    // in source order
};

class RequestEvalState {
public:
  static void Reset();
  static void DestructObjects();
  static void addCodeContainer(SmartPtr<CodeContainer> &cc);
  static ClassEvalState &declareClass(const ClassStatement *cls);
  static void declareFunction(const FunctionStatement *cls);
  static bool declareConstant(CStrRef name, CVarRef value);
  static const ClassStatement *findClass(CStrRef name, bool autoload = false);
  static ClassEvalState *findClassState(CStrRef name, bool autoload = false);
  static const MethodStatement *findMethod(CStrRef cname, CStrRef name,
                                           ClassEvalState *&ce,
                                           bool autoload = false);
  static const FunctionStatement *findUserFunction(CStrRef name);
  static const Function *findFunction(CStrRef name);
  static bool findConstant(CStrRef name, Variant &ret);
  static Variant findUserConstant(CStrRef name, bool error = true);
  static bool includeFile(Variant &res, CStrRef path, bool once,
                          LVariableTable* variables,
                          const char *currentDir);
  static LVariableTable &getFunctionStatics(const FunctionStatement* func);
  static LVariableTable &getMethodStatics(const MethodStatement* func,
                                          CStrRef cls);
  static LVariableTable *getClassStatics(const ClassStatement* cls);

  // Class info hook methods
  static Array getUserFunctionsInfo();
  static Array getClassesInfo();
  static Array getInterfacesInfo();
  static Array getTraitsInfo();
  static Array getConstants();
  static const ClassInfo::MethodInfo *findFunctionInfo(CStrRef name);
  static const ClassInfo *findClassLikeInfo(CStrRef name);
  static const ClassInfo::ConstantInfo *findConstantInfo(CStrRef name);

  // Global state getters
  static void GetMethodStaticVariables(Array &arr);
  static void GetClassStaticVariables(Array &arr);
  static void GetDynamicConstants(Array &arr);
  static Array &GetIncludes() { return Get()->m_includes;}

  // Misc
  static std::string unique();
  static void info();

  static VariantStack &argStack();
  static VariantStack &bytecodeStack();

  static void registerObject(EvalObjectData *obj);
  static void deregisterObject(EvalObjectData *obj);

  static RequestEvalState *Get();
  void fiberInit(RequestEvalState *res, FiberReferenceMap &refMap);
  void fiberExit(RequestEvalState *res, FiberReferenceMap &refMap,
                 FiberAsyncFunc::Strategy default_strategy);
private:
  std::map<std::string, PhpFile*> m_evaledFiles;
  std::list<SmartPtr<CodeContainer> > m_codeContainers;

  StringIMap<ClassEvalState> m_classes;
  StringIMap<const FunctionStatement*> m_functions;
  std::map<const FunctionStatement*, LVariableTable> m_functionStatics;
  typedef std::map<const MethodStatement*,
    StringIMap<LVariableTable> > MethodStatics;
  MethodStatics m_methodStatics;
  Array m_constants;
  StringMap<SmartPtr<EvalConstantInfo> > m_constantInfos;
  StringIMap<SmartPtr<EvalMethodInfo> > m_methodInfos;
  StringMap<SmartPtr<ClassInfoEvaled> > m_classInfos;
  std::set<EvalObjectData*> m_livingObjects;
  int64 m_ids;
  VariantStack m_argStack;
  VariantStack m_bytecodeStack;
  Array m_includes;

  void reset();
  void destructObjects();
  void destructObject(EvalObjectData *eo);
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_STATE_H__ */

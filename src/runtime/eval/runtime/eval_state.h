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
DECLARE_AST_PTR(FunctionStatement);
DECLARE_AST_PTR(MethodStatement);
DECLARE_AST_PTR(Statement);
class PhpFile;
class Function;
class EvalObjectData;

class ClassEvalState {
public:
  typedef hphp_const_char_imap<std::pair<const MethodStatement*, int> >
    MethodTable;
  ClassEvalState() : m_class(NULL), m_constructor(NULL),
                     m_initializedInstance(false),
                     m_initializedStatics(false),
                     m_doneSemanticCheck(false)
  {}
  void init(const ClassStatement *cls);
  const ClassStatement *getClass() const {
    return m_class;
  }
  const MethodStatement *getMethod(const char *m);
  MethodTable &getMethodTable() {
    return m_methodTable;
  }
  const MethodStatement* &getConstructor() {
    return m_constructor;
  }
  LVariableTable &getStatics() {
    return m_statics;
  }
  void initializeInstance();
  void initializeStatics();
  void semanticCheck();
  void fiberInit(ClassEvalState &oces, FiberReferenceMap &refMap);
  void fiberInitStatics(ClassEvalState &oces, FiberReferenceMap &refMap);
  void fiberExit(ClassEvalState &oces, FiberReferenceMap &refMap,
                 FiberAsyncFunc::Strategy default_strategy);
  void fiberExitStatics(ClassEvalState &oces, FiberReferenceMap &refMap,
                        FiberAsyncFunc::Strategy default_strategy);
private:
  const ClassStatement *m_class;
  MethodTable m_methodTable;
  const MethodStatement *m_constructor;
  LVariableTable m_statics;
  bool m_initializedInstance;
  bool m_initializedStatics;
  bool m_doneSemanticCheck;
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
  const InterfaceSet &getInterfaces()    const { return m_interfaces;}
  const InterfaceVec &getInterfacesVec() const { return m_interfacesVec;}
  const MethodMap    &getMethods()       const { return m_methods;}
  const MethodVec    &getMethodsVec()    const { return m_methodsVec;}
  const PropertyMap  &getProperties()    const { return m_properties;}
  const PropertyVec  &getPropertiesVec() const { return m_propertiesVec;}
  const ConstantMap  &getConstants()     const { return m_constants;}
  const ConstantVec  &getConstantsVec()  const { return m_constantsVec;}

 private:
  friend class ClassStatement;
  String       m_parentClass;
  InterfaceSet m_interfaces;    // all interfaces
  InterfaceVec m_interfacesVec; // all interfaces
  MethodMap    m_methods;       // all methods
  MethodVec    m_methodsVec;    // in source order
  PropertyMap  m_properties;    // all properties
  PropertyVec  m_propertiesVec; // in source order
  ConstantMap  m_constants;     // all constants
  ConstantVec  m_constantsVec;  // in source order
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
  static const MethodStatement *findMethod(const char *cname, const char *name,
                                           bool &foundClass,
                                           bool autoload = false);
  static const FunctionStatement *findUserFunction(const char *name);
  static const Function *findFunction(const char *name);
  static bool findConstant(CStrRef name, Variant &ret);
  static bool includeFile(Variant &res, CStrRef path, bool once,
                          LVariableTable* variables,
                          const char *currentDir);
  static LVariableTable &getFunctionStatics(const FunctionStatement* func);
  static LVariableTable &getMethodStatics(const MethodStatement* func,
                                          const char* cls);
  static LVariableTable *getClassStatics(const ClassStatement* cls);

  // Class info hook methods
  static Array getUserFunctionsInfo();
  static Array getClassesInfo();
  static Array getInterfacesInfo();
  static Array getConstants();
  static const ClassInfo::MethodInfo *findFunctionInfo(CStrRef name);
  static const ClassInfo *findClassInfo(const char *name);
  static const ClassInfo *findInterfaceInfo(const char *name);
  static const ClassInfo::ConstantInfo *findConstantInfo(const char *name);

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
  hphp_const_char_imap<const FunctionStatement*> m_functions;
  std::map<const FunctionStatement*, LVariableTable> m_functionStatics;
  typedef std::map<const MethodStatement*,
    std::map<std::string, LVariableTable, string_lessi> > MethodStatics;
  MethodStatics m_methodStatics;
  Array m_constants;
  std::map<std::string, SmartPtr<EvalConstantInfo> > m_constantInfos;
  StringIMap<SmartPtr<EvalMethodInfo> > m_methodInfos;
  std::map<std::string, SmartPtr<ClassInfoEvaled> > m_classInfos;
  std::map<std::string, SmartPtr<ClassInfoEvaled> > m_interfaceInfos;
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

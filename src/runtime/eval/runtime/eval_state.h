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

#ifndef __EVAL_STATE_H__
#define __EVAL_STATE_H__

#include <runtime/eval/base/eval_base.h>
#include <runtime/base/class_info.h>
#include <runtime/eval/runtime/variant_stack.h>
#include <util/case_insensitive.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(ClassStatement);
DECLARE_AST_PTR(FunctionStatement);
DECLARE_AST_PTR(MethodStatement);
class PhpFile;
class Function;
class EvalObjectData;

class ClassEvalState {
public:
  ClassEvalState() : m_constructor(NULL),
                     m_initializedInstance(false),
                     m_initializedStatics(false),
                     m_doneSemanticCheck(false)
  {}
  void init(const ClassStatement *cls);
  const ClassStatement *getClass() const {
    return m_class;
  }
  const MethodStatement *getMethod(const char *m);
  hphp_const_char_imap<const MethodStatement*> &getMethodTable() {
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
private:
  const ClassStatement *m_class;
  hphp_const_char_imap<const MethodStatement*> m_methodTable;
  const MethodStatement *m_constructor;
  LVariableTable m_statics;
  bool m_initializedInstance;
  bool m_initializedStatics;
  bool m_doneSemanticCheck;
};

/**
 * Containers to live in Globals and do the garbage collection.
 */
class CodeContainer {
public:
  virtual ~CodeContainer() {}
  virtual void addDeclarations(hphp_const_char_imap<ClassEvalState>
                               &classes,
                               hphp_const_char_imap<const FunctionStatement*>
                               &m_functions) = 0;
};

class StringCodeContainer : public CodeContainer {
public:
  StringCodeContainer(const std::vector<ClassStatementPtr> &classes,
                      const std::vector<FunctionStatementPtr> &functions);
  virtual void addDeclarations(hphp_const_char_imap<ClassEvalState>
                               &classes,
                               hphp_const_char_imap<const FunctionStatement*>
                               &functions);
private:
  std::vector<ClassStatementPtr> m_classes;
  std::vector<FunctionStatementPtr> m_functions;
};

 class ClassInfoEvaled : public ClassInfo {
 public:
  ~ClassInfoEvaled();
  virtual const char *getParentClass() const { return m_parentClass; }
  const InterfaceMap &getInterfaces() const { return m_interfaces;}
  const InterfaceVec &getInterfacesVec() const { return m_interfacesVec;}
  const MethodMap &getMethods() const { return m_methods;}
  const MethodVec &getMethodsVec() const { return m_methodsVec;}
  const PropertyMap &getProperties() const { return m_properties;}
  const PropertyVec &getPropertiesVec() const { return m_propertiesVec;}
  const ConstantMap &getConstants() const { return m_constants;}
 private:
  friend class ClassStatement;
  const char* m_parentClass;
  InterfaceMap m_interfaces; // all interfaces
  InterfaceVec m_interfacesVec; // all interfaces
  MethodMap    m_methods;    // all methods
  MethodVec    m_methodsVec; // in source order
  PropertyMap  m_properties; // all properties
  PropertyVec  m_propertiesVec; // in source order
  ConstantMap  m_constants;  // all constants
};

class RequestEvalState {
public:
  static void Reset();
  static void DestructObjects();
  static void addCodeContainer(CodeContainer *cc);
  static ClassEvalState &declareClass(const ClassStatement *cls);
  static void declareFunction(const FunctionStatement *cls);
  static bool declareConstant(CStrRef name, CVarRef value);
  static const ClassStatement *findClass(const char *name,
                                         bool autoload = false);
  static ClassEvalState *findClassState(const char *name,
                                        bool autoload = false);
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
  static const ClassInfo::MethodInfo *findFunctionInfo(const char *name);
  static const ClassInfo *findClassInfo(const char *name);
  static const ClassInfo *findInterfaceInfo(const char *name);
  static const ClassInfo::ConstantInfo *findConstantInfo(const char *name);

  // Misc
  static int64 unique();
  static void info();

  static VariantStack &argStack();
  static VariantStack &bytecodeStack();

  static void registerObject(EvalObjectData *obj);
  static void deregisterObject(EvalObjectData *obj);
private:
  std::map<std::string, PhpFile*> m_evaledFiles;
  std::vector<Eval::CodeContainer*> m_codeContainers;

  hphp_const_char_imap<ClassEvalState> m_classes;
  hphp_const_char_imap<const FunctionStatement*> m_functions;
  std::map<const FunctionStatement*, LVariableTable> m_functionStatics;
  std::map<const MethodStatement*, std::map<std::string, LVariableTable> >
    m_methodStatics;
  Array m_constants;
  std::map<std::string, ClassInfo::ConstantInfo> m_constantInfos;
  std::map<std::string, ClassInfo::MethodInfo> m_methodInfos;
  std::map<std::string, ClassInfoEvaled> m_classInfos;
  std::map<std::string, ClassInfoEvaled> m_interfaceInfos;
  std::set<EvalObjectData*> m_livingObjects;
  int64 m_ids;
  VariantStack m_argStack;
  VariantStack m_bytecodeStack;
  void reset();
  void destructObjects();
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_STATE_H__ */

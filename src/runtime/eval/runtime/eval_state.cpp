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
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/base/function.h>
#include <runtime/eval/ast/function_statement.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/runtime/file_repository.h>
#include <runtime/base/util/request_local.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/eval/ext/ext.h>
#include <util/util.h>
#include <runtime/base/source_info.h>
#include <runtime/eval/parser/parser.h>
#include <runtime/eval/runtime/eval_object_data.h>
#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/eval.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

StringCodeContainer::StringCodeContainer(StatementPtr s) : m_s(s) {}

void ClassEvalState::init(const ClassStatement *cls) {
  m_class = cls;
}

const MethodStatement *ClassEvalState::getMethod(const char *m) {
  hphp_const_char_imap<const MethodStatement*>::const_iterator it =
    m_methodTable.find(m);
  if (it == m_methodTable.end()) return NULL;
  return it->second;
}

void ClassEvalState::initializeInstance() {
  if (!m_initializedInstance) {
    semanticCheck();
    m_class->loadMethodTable(*this);
    m_initializedInstance = true;
  }
}

void ClassEvalState::initializeStatics() {
  if (!m_initializedStatics) {
    semanticCheck();
    m_class->initializeStatics(m_statics);
    m_initializedStatics = true;
  }
}

void ClassEvalState::semanticCheck() {
  if (!m_doneSemanticCheck) {
    m_class->semanticCheck(NULL);
    m_doneSemanticCheck = true;
  }
}

IMPLEMENT_THREAD_LOCAL(RequestEvalState, s_res);
void RequestEvalState::Reset() {
  s_res->reset();
}

void RequestEvalState::reset() {
  m_functionStatics.clear();
  m_methodStatics.clear();

  m_classes.clear();
  m_functions.clear();
  m_constants = Array();
  m_constantInfos.clear();
  m_methodInfos.clear();
  m_classInfos.clear();
  m_interfaceInfos.clear();
  m_ids = 0;
  m_argStack.clear();

  for (vector<CodeContainer*>::const_iterator it =
         m_codeContainers.begin(); it != m_codeContainers.end();
       ++it) {
    delete *it;
  }
  for (map<string, PhpFile*>::const_iterator it =
         m_evaledFiles.begin(); it != m_evaledFiles.end(); ++it) {
    it->second->decRef();
  }
  m_codeContainers.clear();
  m_evaledFiles.clear();
}

void RequestEvalState::DestructObjects() {
  s_res->destructObjects();
}

void RequestEvalState::destructObjects() {
  while (!m_livingObjects.empty()) {
    EvalObjectData *eo = *m_livingObjects.begin();
    m_livingObjects.erase(eo);
    const MethodStatement *des = eo->getMethodStatement("__destruct");
    if (des) {
      try {
        des->invokeInstance(Object(eo), Array());
      } catch (...) {
        handle_destructor_exception();
      }
    }
    eo->setInDtor();
  }
}

void RequestEvalState::addCodeContainer(CodeContainer *cc) {
  RequestEvalState *self = s_res.get();
  self->m_codeContainers.push_back(cc);
}

ClassEvalState &RequestEvalState::declareClass(const ClassStatement *cls) {
  RequestEvalState *self = s_res.get();
  ClassEvalState &ce = self->m_classes[cls->lname().c_str()];
  ce.init(cls);
  return ce;
}
void RequestEvalState::declareFunction(const FunctionStatement *fn) {
  RequestEvalState *self = s_res.get();
  string lname = fn->lname();
  if (self->m_functions.find(lname.c_str()) != self->m_functions.end()) {
    raise_error("Cannot redeclare %s()", fn->name().c_str());
  }
  self->m_functions[lname.c_str()] = fn;
}

bool RequestEvalState::declareConstant(CStrRef name, CVarRef val) {
  RequestEvalState *self = s_res.get();
  if (self->m_constants.exists(name)) return false;
  self->m_constants.set(name, val);
  ClassInfo::ConstantInfo &ci = self->m_constantInfos[name.c_str()];
  // Only need to set value really.
  ci.name = NULL;
  ci.valueLen = 0;
  ci.valueText = NULL;
  ci.value = val;
  return true;
}

ClassInfoEvaled::~ClassInfoEvaled() {
  for (MethodVec::iterator it = m_methodsVec.begin(); it != m_methodsVec.end();
       ++it) {
    MethodInfo *mi = *it;
    for (std::vector<const ParameterInfo*>::iterator mit = mi->parameters.begin();
         mit != mi->parameters.end(); ++mit) {
      free((void*)(*mit)->value);
    }
    delete mi;
  }
  for (PropertyMap::iterator it = m_properties.begin();
       it != m_properties.end(); ++it) {
    delete it->second;
  }
  for (ConstantMap::iterator it = m_constants.begin(); it != m_constants.end();
       ++it) {
    delete [] it->second->valueText;
    delete it->second;
  }
}

const ClassStatement *RequestEvalState::findClass(const char *name,
                                                  bool autoload /* = false */)
{
  RequestEvalState *self = s_res.get();
  hphp_const_char_imap<ClassEvalState>::const_iterator it =
    self->m_classes.find(name);
  if (it != self->m_classes.end()) {
    return it->second.getClass();
  }
  if (autoload &&
      (!ClassInfo::HasClass(name) && !ClassInfo::HasInterface(name)) &&
      eval_try_autoload(name)) {
    return findClass(name, false);
  }
  return NULL;
}

ClassEvalState *RequestEvalState::findClassState(const char *name,
                                                 bool autoload /* = false */) {
  RequestEvalState *self = s_res.get();
  hphp_const_char_imap<ClassEvalState>::iterator it =
    self->m_classes.find(name);
  if (it != self->m_classes.end()) {
    return &it->second;
  }
  if (autoload && !ClassInfo::HasClass(name) && eval_try_autoload(name)) {
    return findClassState(name, false);
  }
  return NULL;
}

const MethodStatement *RequestEvalState::findMethod(const char *cname,
                                                    const char *name,
                                                    bool &foundClass,
                                                    bool autoload /* = false */)
{
  const Eval::ClassStatement *cls =
    Eval::RequestEvalState::findClass(cname, autoload);
  if (cls) {
    foundClass = true;
  }
  while (cls) {
    const Eval::MethodStatement *meth = cls->findMethod(name);
    if (meth) {
      return meth;
    }
    cls = Eval::RequestEvalState::findClass(cls->parent().c_str());
  }
  return NULL;
}

const Function *RequestEvalState::findFunction(const char *name) {
  const Function *f = findUserFunction(name);
  if (f) return f;
  return evalOverrides.findFunction(name);
}
const FunctionStatement *RequestEvalState::findUserFunction(const char *name) {
  RequestEvalState *self = s_res.get();
  hphp_const_char_imap<const FunctionStatement*>::const_iterator it =
    self->m_functions.find(name);
  if (it != self->m_functions.end()) {
    return it->second;
  }
  return NULL;
}

bool RequestEvalState::findConstant(CStrRef name, Variant &ret) {
  RequestEvalState *self = s_res.get();
  if (self->m_constants.exists(name)) {
    ret = self->m_constants.rvalAt(name);
    return true;
  }
  return false;
}

bool RequestEvalState::includeFile(Variant &res, CStrRef path, bool once,
                                   LVariableTable* variables,
                                   const char *currentDir) {
  RequestEvalState *self = s_res.get();
  string spath(path.data());
  struct stat s;
  if (!FileRepository::findFile(spath, s, currentDir)) return false;
  map<string, PhpFile*>::const_iterator it = self->m_evaledFiles.find(spath);
  Eval::PhpFile *efile = NULL;
  if (it != self->m_evaledFiles.end()) {
    if (once) {
      res = true;
      return true;
    }
    efile = it->second;
  } else {
    char *rpath = realpath(spath.c_str(), 0);
    if (rpath && rpath != spath) {
      it = self->m_evaledFiles.find(rpath);
      if (it != self->m_evaledFiles.end()) {
        self->m_evaledFiles[spath] = efile = it->second;
        efile->incRef();
        if (once) {
          free(rpath);
          res = true;
          return true;
        }
      }
    } else {
      free(rpath);
      rpath = 0;
    }
    if (!efile) {
      efile = FileRepository::checkoutFile(rpath ? rpath : spath, s);
      if (efile) {
        self->m_evaledFiles[spath] = efile;
        if (rpath) {
          self->m_evaledFiles[rpath] = efile;
          efile->incRef();
        }
      }
    }
    free(rpath);
  }
  if (efile) {
    res = efile->eval(variables);
    return true;
  }
  return false;
}

LVariableTable &RequestEvalState::
getFunctionStatics(const FunctionStatement* func) {
  RequestEvalState *self = s_res.get();
  return self->m_functionStatics[func];
}

LVariableTable &RequestEvalState::
getMethodStatics(const MethodStatement* func, const char* cls) {
  RequestEvalState *self = s_res.get();
  return self->m_methodStatics[func][cls];
}

LVariableTable *RequestEvalState::getClassStatics(const ClassStatement* cls) {
  RequestEvalState *self = s_res.get();
  hphp_const_char_imap<ClassEvalState>::iterator it =
    self->m_classes.find(cls->name().c_str());
  if (it == self->m_classes.end()) return NULL;
  it->second.initializeStatics();
  return &it->second.getStatics();
}

int64 RequestEvalState::unique() {
  RequestEvalState *self = s_res.get();
  return self->m_ids++;
}

Array RequestEvalState::getUserFunctionsInfo() {
  RequestEvalState *self = s_res.get();
  Array ret;
  for (hphp_const_char_imap<const FunctionStatement*>::const_iterator it =
         self->m_functions.begin(); it != self->m_functions.end(); ++it) {
    ret.append(it->first);
  }
  return ret;
}
Array RequestEvalState::getClassesInfo() {
  RequestEvalState *self = s_res.get();
  Array ret;
  for (hphp_const_char_imap<ClassEvalState>::const_iterator it =
         self->m_classes.begin(); it != self->m_classes.end(); ++it) {
    if (it->second.getClass()->getModifiers() & ClassStatement::Interface)
      continue;
    ret.append(it->first);
  }
  return ret;
}
Array RequestEvalState::getInterfacesInfo() {
  RequestEvalState *self = s_res.get();
  Array ret;
  for (hphp_const_char_imap<ClassEvalState>::const_iterator it =
         self->m_classes.begin(); it != self->m_classes.end(); ++it) {
    if (it->second.getClass()->getModifiers() & ClassStatement::Interface) {
      ret.append(it->first);
    }
  }
  return ret;
}

Array RequestEvalState::getConstants() {
  RequestEvalState *self = s_res.get();
  return self->m_constants;
}

const ClassInfo::MethodInfo *RequestEvalState::
findFunctionInfo(const char *name) {
  RequestEvalState *self = s_res.get();
  map<string, ClassInfo::MethodInfo>::iterator it =
    self->m_methodInfos.find(name);
  if (it == self->m_methodInfos.end()) {
    const FunctionStatement *fs = findUserFunction(name);
    if (fs) {
      ClassInfo::MethodInfo &m = self->m_methodInfos[name];
      fs->getInfo(m);
      return &m;
    }
    return NULL;
  } else {
    return &it->second;
  }
}

const ClassInfo *RequestEvalState::findClassInfo(const char *name) {
  RequestEvalState *self = s_res.get();
  map<string, ClassInfoEvaled>::const_iterator it =
    self->m_classInfos.find(name);
  if (it == self->m_classInfos.end()) {
    const ClassStatement *cls = findClass(name);
    if (cls && !(cls->getModifiers() & ClassStatement::Interface)) {
      ClassInfoEvaled &cl = self->m_classInfos[name];
      cls->getInfo(cl);
      return &cl;
    }
    return NULL;
  } else {
    return &it->second;
  }
}

const ClassInfo *RequestEvalState::findInterfaceInfo(const char *name) {
  RequestEvalState *self = s_res.get();
  map<string, ClassInfoEvaled>::const_iterator it =
    self->m_interfaceInfos.find(name);
  if (it == self->m_interfaceInfos.end()) {
    const ClassStatement *cls = findClass(name);
    if (cls && (cls->getModifiers() & ClassStatement::Interface)) {
      ClassInfoEvaled &cl = self->m_interfaceInfos[name];
      cls->getInfo(cl);
      return &cl;
    }
    return NULL;
  } else {
    return &it->second;
  }
}

const ClassInfo::ConstantInfo *RequestEvalState::
findConstantInfo(const char *name) {
  RequestEvalState *self = s_res.get();
  map<string, ClassInfo::ConstantInfo>::const_iterator it =
    self->m_constantInfos.find(name);
  if (it != self->m_constantInfos.end()) {
    return &it->second;
  }
  return NULL;
}

void RequestEvalState::registerObject(EvalObjectData *obj) {
  RequestEvalState *self = s_res.get();
  self->m_livingObjects.insert(obj);
}
void RequestEvalState::deregisterObject(EvalObjectData *obj) {
  RequestEvalState *self = s_res.get();
  self->m_livingObjects.erase(obj);
}

class EvalClassInfoHook : public ClassInfoHook {
public:
  EvalClassInfoHook() {
    ClassInfo::SetHook(this);
  }

  virtual Array getUserFunctions() const {
    return RequestEvalState::getUserFunctionsInfo();
  }
  virtual Array getClasses() const {
    return RequestEvalState::getClassesInfo();
  }
  virtual Array getInterfaces() const {
    return RequestEvalState::getInterfacesInfo();
  }
  virtual Array getConstants() const {
    return RequestEvalState::getConstants();
  }
  virtual const ClassInfo::MethodInfo *findFunction(const char *name) const {
    return RequestEvalState::findFunctionInfo(name);
  }
  virtual const ClassInfo *findClass(const char *name) const {
    return RequestEvalState::findClassInfo(name);
  }
  virtual const ClassInfo *findInterface(const char *name) const {
    return RequestEvalState::findInterfaceInfo(name);
  }
  virtual const ClassInfo::ConstantInfo *findConstant(const char *name) const {
    return RequestEvalState::findConstantInfo(name);
  }
};
static EvalClassInfoHook eval_class_info_hook;

class EvalSourceInfoHook : public SourceInfoHook {
public:
  EvalSourceInfoHook() {
    SourceInfo::SetHook(this);
  }
  virtual const char *getClassDeclaringFile(const char *name) {
    const ClassStatement *f = RequestEvalState::findClass(name);
    if (f) {
      return f->loc()->file;
    } else {
      return NULL;
    }
  }
  virtual const char *getFunctionDeclaringFile(const char *name) {
    const FunctionStatement *f = RequestEvalState::findUserFunction(name);
    if (f) {
      return f->loc()->file;
    } else {
      return NULL;
    }
  }
};

static EvalSourceInfoHook eval_source_info_hook;

void RequestEvalState::info() {
  RequestEvalState *self = s_res.get();
  cerr << "Eval State Status" << endl;
  cerr << "-----------------" << endl;
  cerr << "Classes:" << endl;
  for (hphp_const_char_imap<ClassEvalState>::const_iterator it =
         self->m_classes.begin(); it != self->m_classes.end(); ++it) {
    cerr << " " << it->first << " " << it->second.getClass()->name() << endl;
  }
  cerr << "Functions:" << endl;
    for (hphp_const_char_imap<const FunctionStatement*>::const_iterator it =
         self->m_functions.begin(); it != self->m_functions.end(); ++it) {
    cerr << " " << it->second->name() << endl;
  }
}

VariantStack &RequestEvalState::argStack() {
  RequestEvalState *self = s_res.get();
  return self->m_argStack;
}


VariantStack &RequestEvalState::bytecodeStack() {
  RequestEvalState *self = s_res.get();
  return self->m_bytecodeStack;
}


///////////////////////////////////////////////////////////////////////////////
}
}


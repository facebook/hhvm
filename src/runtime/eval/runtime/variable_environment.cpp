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
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/hphp_system.h>
#include <runtime/eval/ast/function_statement.h>
#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/parser/parser.h>

#include <runtime/ext/ext_closure.h>
#include <runtime/ext/ext_continuation.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

static StaticString s_Continuation("Continuation");
static StaticString s_this("this");
#define TEMP_STACK_SIZE 8192

class TempStack {
public:
  TempStack() : m_prevSize(0), m_size(0), m_alloc(TEMP_STACK_SIZE) {
    m_stack = new Variant[TEMP_STACK_SIZE];
  }
  Variant *alloc(int size, int &oldPrevSize) {
    ASSERT(size > 0);
    oldPrevSize = m_prevSize;
    if (UNLIKELY(m_size + size >= m_alloc)) {
      throw FatalErrorException("temp stack overflow");
    }
    m_prevSize = m_size;
    m_size += size;
    return m_stack + m_prevSize;
  }
  void release (int size, int oldPrevSize) {
    ASSERT(size > 0 && m_size - m_prevSize == size);
    for (Variant *v = m_stack + m_prevSize; v < m_stack + m_size; v++) {
      if (IS_REFCOUNTED_TYPE(v->getRawType())) v->~Variant();
    }
    m_size = m_prevSize;
    m_prevSize = oldPrevSize;
  }
  Variant getTemp(int index) {
    ASSERT(m_size > m_prevSize);
    ASSERT(index >= 0 && index < m_size - m_prevSize);
    return m_stack[m_prevSize + index];
  }
private:
  Variant *m_stack;
  int m_prevSize;
  int m_size;
  int m_alloc;
};
IMPLEMENT_THREAD_LOCAL_NO_CHECK(TempStack, s_tempStack);

void VariableEnvironment::InitTempStack() {
  s_tempStack.getCheck();
}

VariableEnvironment::VariableEnvironment()
    : m_currentClass(NULL), m_breakLevel(0), m_returning(false),
      m_closure(NULL)
{
}

void VariableEnvironment::flagGlobal(CStrRef name, int64 hash) {
  getVar(name, SgNormal).assignRef(get_globals()->get(name));
}

void VariableEnvironment::unset(CStrRef name, int64 hash) {
  SuperGlobal sg = VariableIndex::isSuperGlobal(name);
  HPHP::unset(getVar(name, sg));
}

void VariableEnvironment::setCurrentObject(CObjRef co) {
  ASSERT(!m_currentClass);
  m_currentObject = co;
  m_currentClass = co->o_getClassName();
  getVar(s_this, SgNormal) = co;
}
void VariableEnvironment::setCurrentClass(const char* cls) {
  ASSERT(m_currentObject.isNull());
  m_currentClass = cls;
}

const char* VariableEnvironment::currentClass() const {
  return m_currentClass;
}
const ClassStatement *VariableEnvironment::currentClassStatement() const {
  return NULL;
}
String VariableEnvironment::currentContext() const {
  return m_currentClass ? m_currentClass : "";
}

void VariableEnvironment::setIdx(int idx, Variant *v) {
  ASSERT(false);
  throw FatalErrorException("setIdx not supported in this env");
}

Array VariableEnvironment::getDefinedVariables() const {
  return Array::Create();
}

Variant *VariableEnvironment::createTempVariables(int size, int &oldPrevSize) {
  return s_tempStack->alloc(size, oldPrevSize);
}

Variant VariableEnvironment::getTempVariable(int index) {
  return s_tempStack->getTemp(index);
}

void VariableEnvironment::releaseTempVariables(int size, int oldPrevSize) {
  s_tempStack->release(size, oldPrevSize);
}

///////////////////////////////////////////////////////////////////////////////

DummyVariableEnvironment::DummyVariableEnvironment()
{}

void DummyVariableEnvironment::flagStatic(CStrRef name, int64 hash) {
  ASSERT(false);
}
void DummyVariableEnvironment::flagGlobal(CStrRef name, int64 hash) {
  ASSERT(false);
}
void DummyVariableEnvironment::unset(CStrRef name, int64 hash) {
  ASSERT(false);
}
bool DummyVariableEnvironment::exists(CStrRef s) const {
  ASSERT(false);
  return false;
}
Variant &DummyVariableEnvironment::getImpl(CStrRef s) {
  ASSERT(false);
  throw FatalErrorException("Tried to get from a dummy environment");
}
Variant &DummyVariableEnvironment::getVar(CStrRef s, SuperGlobal sg) {
  ASSERT(false);
  throw FatalErrorException("Tried to get var from a dummy environment");
}
Array DummyVariableEnvironment::getParams() const {
  ASSERT(false);
  return Array();
}
FuncScopeVariableEnvironment::
FuncScopeVariableEnvironment(const FunctionStatement *func)
  : m_func(func), m_staticEnv(NULL), m_argc(0),
    m_argStart(RequestEvalState::argStack().pos()) {

  const Block::VariableIndices &vi = func->varIndices();
  const vector<StringData*> &vars = func->variables();
  m_byIdx.resize(vi.size());
  Globals *g = NULL;
  for (int i = vars.size() - 1; i >= 0; i--) {
    String name(vars[i]);
    Block::VariableIndices::const_iterator it = vi.find(name);
    ASSERT(it != vi.end());
    if (it == vi.end()) continue;

    const VariableIndex &v = it->second;
    if (v.superGlobal() != SgNormal &&
        v.superGlobal() != SgGlobals) {
      if (!g) g = get_globals();
      // This is safe because superglobals are real members of the globals
      // and do not live in an array
      m_byIdx[v.idx()] = &g->get(name);
    } else {
      Variant &val = m_alist.prepend(name);
      m_byIdx[v.idx()] = &val;
      if (v.superGlobal() == SgGlobals) {
        val = get_global_array_wrapper();
      }
    }
  }
}

FuncScopeVariableEnvironment::~FuncScopeVariableEnvironment() {
  RequestEvalState::argStack().pop(m_argc);
  ASSERT(RequestEvalState::argStack().pos() == m_argStart);
}

void FuncScopeVariableEnvironment::flagStatic(CStrRef name, int64 hash) {
  if (!m_staticEnv) {
    void *closure = getClosure();
    if (UNLIKELY(closure != NULL)) {
      // statics for closures live on a closure
      c_GeneratorClosure *typedClosure = (c_GeneratorClosure*) closure;
      m_staticEnv = &typedClosure->m_statics;
    } else {
      bool isContClosure =
        ParserBase::IsContinuationFromClosureName(m_func->name().c_str());
      if (UNLIKELY(isContClosure)) {
        ObjectData *cont = getContinuation();
        ASSERT(cont != NULL);
        c_GenericContinuation *typedCont = (c_GenericContinuation*) cont;
        m_staticEnv = &typedCont->m_statics;
      } else {
        m_staticEnv = m_func->getStaticVars(*this);
      }
    }
  }
  ASSERT(m_staticEnv != NULL);
  if (!m_staticEnv->exists(name.data())) {
    m_staticEnv->getVar(name, SgNormal) = m_func->getStaticValue(*this, name);
  }
  getVar(name, SgNormal).assignRef(m_staticEnv->getVar(name, SgNormal));
}

void FuncScopeVariableEnvironment::setIdx(int idx, Variant *v) {
  ASSERT(false);
  throw FatalErrorException("setIdx not supported in this env");
}

bool FuncScopeVariableEnvironment::refReturn() const {
  return m_func->refReturn();
}

Array FuncScopeVariableEnvironment::getParams() const {
  return RequestEvalState::argStack().pull(m_argStart, m_argc);
}

bool FuncScopeVariableEnvironment::exists(CStrRef name) const {
  return m_alist.exists(name, true);
  //return LVariableTable::exists(name, hash);
}
Variant &FuncScopeVariableEnvironment::getImpl(CStrRef s) {
  SuperGlobal sg = VariableIndex::isSuperGlobal(s);
  return getVar(s, sg);
}

Variant &FuncScopeVariableEnvironment::getVar(CStrRef s, SuperGlobal sg) {
  if (sg == SgNormal) {
    Variant *v = m_alist.getPtr(s);
    if (!v) v = &m_alist.prepend(s);
    return *v; 
  }
  if (sg == SgGlobals) {
    Variant &v = m_alist.prepend(s);
    v = get_global_array_wrapper();
    return v;
  }
  return get_globals()->get(s);
}

Array FuncScopeVariableEnvironment::getDefinedVariables() const {
  return m_alist.toArray();
}

ObjectData *FuncScopeVariableEnvironment::getContinuation() const {
  const vector<ParameterPtr> &params = m_func->getParams();
  if (ParserBase::IsContinuationName(m_func->name().c_str()) &&
      params.size() == 1 &&
      params[0]->getName().find(CONTINUATION_OBJECT_NAME) == 0) {
    Array args = getParams();
    ASSERT(args.size() == 1);
    ObjectData *obj = args[0].getObjectData();
    if (LIKELY(obj->o_instanceof(s_Continuation))) return obj;
  }
  return NULL;
}

MethScopeVariableEnvironment::
MethScopeVariableEnvironment(const MethodStatement *meth)
  : FuncScopeVariableEnvironment(meth), m_cls(meth->getClass()) {}

String MethScopeVariableEnvironment::currentContext() const {
  return m_cls->name();
}

const ClassStatement* MethScopeVariableEnvironment::currentClassStatement()
  const {
  return m_cls;
}

NestedVariableEnvironment::NestedVariableEnvironment
(LVariableTable *ext, const Block &blk, CArrRef params /* = Array() */,
 CObjRef current_object /* = Object() */)
  : m_ext(ext), m_block(blk), m_params(params) {
  m_byIdx.resize(m_block.varIndices().size());
  if (!current_object.isNull()) setCurrentObject(current_object);
}

void NestedVariableEnvironment::flagStatic(CStrRef name, int64 hash) {
  // Behavior is to set the variable to init
  // .. and do some other stupid stuff that I'm not going to try
  getVar(name, SgNormal) = m_block.getStaticValue(*this, name);
}

void NestedVariableEnvironment::setIdx(int idx, Variant *v) {
  ASSERT(m_byIdx[idx] == NULL);
  m_byIdx[idx] = v;
}

bool NestedVariableEnvironment::exists(CStrRef s) const {
  return m_ext->exists(s);
}

Variant &NestedVariableEnvironment::getImpl(CStrRef s) {
  SuperGlobal sg = VariableIndex::isSuperGlobal(s);
  return getVar(s, sg);
}

Variant &NestedVariableEnvironment::getVar(CStrRef s, SuperGlobal sg) {
  if (sg == SgGlobals) {
    return m_global = get_global_array_wrapper();
  }
  if (sg == SgNormal) return m_ext->getVar(s, sg);
  return get_globals()->get(s);
}

Array NestedVariableEnvironment::getParams() const {
  return m_params;
}

Array NestedVariableEnvironment::getDefinedVariables() const {
  return m_ext->getDefinedVars();
}

///////////////////////////////////////////////////////////////////////////////
}
}


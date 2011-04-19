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

#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/hphp_system.h>
#include <runtime/eval/ast/function_statement.h>
#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/parser/parser.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

VariableEnvironment::VariableEnvironment()
    : m_currentClass(NULL), m_breakLevel(0), m_returning(false),
      m_closure(NULL)
{
}

void VariableEnvironment::flagGlobal(CStrRef name, int64 hash) {
  get(name).assignRef(get_globals()->get(name));
}

void VariableEnvironment::unset(CStrRef name, int64 hash) {
  HPHP::unset(get(name));
}

void VariableEnvironment::setCurrentObject(CObjRef co) {
  ASSERT(!m_currentClass);
  m_currentObject = co;
  m_currentClass = co->o_getClassName();
  get("this") = co;
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

Variant *VariableEnvironment::getIdx(int idx) {
  ASSERT(false);
  throw FatalErrorException("getIdx not supported in this env");
}

void VariableEnvironment::setIdx(int idx, Variant *v) {
  ASSERT(false);
  throw FatalErrorException("setIdx not supported in this env");
}

Array VariableEnvironment::getDefinedVariables() const {
  return Array::Create();
}

std::vector<Variant> &VariableEnvironment::createTempVariables() {
  m_tempStack.resize(m_tempStack.size() + 1);
  return m_tempStack.back();
}

Variant VariableEnvironment::getTempVariable(int index) {
  ASSERT(!m_tempStack.empty());
  ASSERT(index >= 0 && index < (int)m_tempStack.back().size());
  return m_tempStack.back()[index];
}

void VariableEnvironment::releaseTempVariables() {
  ASSERT(!m_tempStack.empty());
  m_tempStack.pop_back();
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
Array DummyVariableEnvironment::getParams() const {
  ASSERT(false);
  return Array();
}

class SuperGlobalInitializer {
public:
  SuperGlobalInitializer() { }
  void init(VariableEnvironment &env) const {
    for (int i = 0; i < s_num-1; i++) {
      env.flagGlobal(s_names[i]);
    }
    initGlobals(env);
  }
  void initGlobals(VariableEnvironment &env) const {
    env.get(s_names[s_num-1]) = get_global_array_wrapper();
  }
private:
  static const int s_num;
  static const StaticString s_names[];
};
const StaticString SuperGlobalInitializer::s_names[] =
  {"_SERVER",
   "_GET",
   "_POST",
   "_FILES",
   "_COOKIE",
   "_SESSION",
   "_REQUEST",
   "_ENV",
   "http_response_header",
   "GLOBALS"};
const int SuperGlobalInitializer::s_num =
  sizeof(SuperGlobalInitializer::s_names)/
  sizeof(SuperGlobalInitializer::s_names[0]);
static SuperGlobalInitializer g_sinit;

FuncScopeVariableEnvironment::
FuncScopeVariableEnvironment(const FunctionStatement *func)
  : m_func(func), m_staticEnv(NULL), m_argc(0),
    m_argStart(RequestEvalState::argStack().pos()) {

  const Block::VariableIndices &vi = func->varIndices();
  const vector<string> &vars = func->variables();
  m_byIdx.resize(vi.size());
  Globals *g = NULL;
  for (int i = vars.size() - 1; i >= 0; i--) {
    const string &name = vars[i];
    Block::VariableIndices::const_iterator it = vi.find(name);
    ASSERT(it != vi.end());
    if (it == vi.end()) continue;

    const VariableIndex &v = it->second;
    String sname(name.c_str(), name.size(), AttachLiteral);
    if (v.superGlobal() != VariableIndex::Normal &&
        v.superGlobal() != VariableIndex::Globals) {
      if (!g) g = get_globals();
      // This is safe because superglobals are real members of the globals
      // and do not live in an array
      m_byIdx[v.idx()] = &g->get(sname);
    } else {
      Variant &val = m_alist.prepend(sname);
      m_byIdx[v.idx()] = &val;
      if (v.superGlobal() == VariableIndex::Globals) {
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
    m_staticEnv = m_func->getStaticVars(*this);
  }
  if (!m_staticEnv->exists(name.data())) {
    m_staticEnv->get(name) = m_func->getStaticValue(*this, name);
  }
  get(name).assignRef(m_staticEnv->get(name));
}

Variant *FuncScopeVariableEnvironment::getIdx(int idx) {
  return m_byIdx[idx];
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
  {
    Variant *v = m_alist.getPtr(s);
    if (v) return *v;
  }
  VariableIndex::SuperGlobal sg = VariableIndex::isSuperGlobal(s);
  if (sg != VariableIndex::Normal && sg != VariableIndex::Globals) {
    return get_globals()->get(s);
  } else {
    Variant &v = m_alist.prepend(s);
    if (sg == VariableIndex::Globals) {
      v = get_global_array_wrapper();
    }
    return v;
  }
}

Array FuncScopeVariableEnvironment::getDefinedVariables() const {
  return m_alist.toArray();
}

ObjectData *FuncScopeVariableEnvironment::getContinuation() const {
  const vector<ParameterPtr> &params = m_func->getParams();
  if (m_func->name().find('0') == 0 && params.size() == 1
   && params[0]->getName().find(CONTINUATION_OBJECT_NAME) == 0) {
    Array args = getParams();
    ASSERT(args.size() == 1);
    ObjectData *obj = args[0].getObjectData();
    if (obj->o_instanceof("Continuation")) return obj;
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
  g_sinit.initGlobals(*this);
}

void NestedVariableEnvironment::flagStatic(CStrRef name, int64 hash) {
  // Behavior is to set the variable to init
  // .. and do some other stupid stuff that I'm not going to try
  get(name) = m_block.getStaticValue(*this, name);
}

Variant *NestedVariableEnvironment::getIdx(int idx) {
  return m_byIdx[idx];
}

void NestedVariableEnvironment::setIdx(int idx, Variant *v) {
  ASSERT(m_byIdx[idx] == NULL);
  m_byIdx[idx] = v;
}

bool NestedVariableEnvironment::exists(CStrRef s) const {
  return m_ext->exists(s);
}

Variant &NestedVariableEnvironment::getImpl(CStrRef s) {
  VariableIndex::SuperGlobal sg = VariableIndex::isSuperGlobal(s);
  if (sg != VariableIndex::Normal && sg != VariableIndex::Globals) {
    return get_globals()->get(s);
  } else {
    return m_ext->get(s);
  }
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


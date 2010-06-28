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

#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/hphp_system.h>
#include <runtime/eval/ast/function_statement.h>
#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/runtime/eval_state.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

VariableEnvironment::VariableEnvironment()
  : m_currentClass(NULL), m_breakLevel(0), m_returning(false)
{
}

void VariableEnvironment::flagGlobal(CStrRef name, int64 hash) {
  get(name, hash) = ref(get_globals()->get(name, hash));
}

void VariableEnvironment::unset(CStrRef name, int64 hash) {
  HPHP::unset(get(name, hash));
}

void VariableEnvironment::setCurrentObject(CObjRef co) {
  ASSERT(!m_currentClass);
  m_currentObject = co;
  m_currentClass = co->o_getClassName();
  get("this", -1) = co;
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
const char* VariableEnvironment::currentContext() const {
  return m_currentClass ? m_currentClass : "";
}

Variant &VariableEnvironment::getIdx(int idx) {
  ASSERT(false);
  throw FatalErrorException("getIdx not supported in this env");
}

Array VariableEnvironment::getDefinedVariables() const {
  return Array::Create();
}

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
bool DummyVariableEnvironment::exists(const char *s, int64 hash) const {
  ASSERT(false);
  return false;
}
Variant &DummyVariableEnvironment::getImpl(CStrRef s, int64 hash) {
  ASSERT(false);
  throw FatalErrorException("Tried to get from a dummy environment");
}
Array DummyVariableEnvironment::getParams() const {
  ASSERT(false);
  return Array();
}

class SuperGlobalInitializer {
public:
  SuperGlobalInitializer() {
    for (int i = 0; i < s_num; i++) {
      s_hashes[i] = hash_string(s_names[i].data(), s_names[i].size());
    }
  }
  void init(VariableEnvironment &env) const {
    for (int i = 0; i < s_num-1; i++) {
      env.flagGlobal(s_names[i], s_hashes[i]);
    }
    initGlobals(env);
  }
  void initGlobals(VariableEnvironment &env) const {
    env.get(s_names[s_num-1], s_hashes[s_num-1]) = get_global_array_wrapper();
  }
  void initAL(AssocList &al) const {
    Globals *g = get_globals();
    for (int i = 0; i < s_num-1; i++) {
      al.prepend(s_names[i]) = ref(g->get(s_names[i], s_hashes[i]));
    }
    al.prepend("GLOBALS") = get_global_array_wrapper();
  }
private:
  static const int s_num;
  static const StaticString s_names[];
  static int64 s_hashes[];
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
int64 SuperGlobalInitializer::s_hashes[SuperGlobalInitializer::s_num];
static SuperGlobalInitializer g_sinit;

FuncScopeVariableEnvironment::
FuncScopeVariableEnvironment(const FunctionStatement *func, int argc)
  : m_func(func), m_staticEnv(NULL), m_argc(argc),
    m_argStart(RequestEvalState::argStack().pos()) {

  const Block::VariableIndices &vi = func->varIndices();
  m_byIdx.resize(vi.size());
  Globals *g = NULL;
  for (Block::VariableIndices::const_iterator it = vi.begin();
       it != vi.end(); ++it) {
    const VariableIndex &v = it->second;
    if (v.superGlobal() != VariableIndex::Normal &&
        v.superGlobal() != VariableIndex::Globals) {
      if (!g) g = get_globals();
      // This is safe because superglobals are real members of the globals
      // and do not live in an array
      m_byIdx[v.idx()] = &g->get(String(it->first.c_str(), it->first.size(),
            AttachLiteral), v.hash());
    } else {
      Variant &val = m_alist.prepend(String(it->first.c_str(),
            it->first.size(),
            AttachLiteral));
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
    m_staticEnv->get(name) = m_func->getStaticValue(*this, name.c_str());
  }
  get(name, hash) = ref(m_staticEnv->get(name, hash));
}

Variant &FuncScopeVariableEnvironment::getIdx(int idx) {
  return *m_byIdx[idx];
}

bool FuncScopeVariableEnvironment::refReturn() const {
  return m_func->refReturn();
}

Array FuncScopeVariableEnvironment::getParams() const {
  return RequestEvalState::argStack().pull(m_argStart, m_argc);
}

bool FuncScopeVariableEnvironment::exists(const char *name, int64 hash) const {
  return m_alist.exists(name);
  //return LVariableTable::exists(name, hash);
}
Variant &FuncScopeVariableEnvironment::getImpl(CStrRef s, int64 hash) {
  {
    Variant *v = m_alist.getPtr(s);
    if (v) return *v;
  }
  VariableIndex::SuperGlobal sg = VariableIndex::isSuperGlobal(s);
  if (sg != VariableIndex::Normal && sg != VariableIndex::Globals) {
    return get_globals()->get(s, hash);
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

MethScopeVariableEnvironment::
MethScopeVariableEnvironment(const MethodStatement *meth, int argc)
  : FuncScopeVariableEnvironment(meth, argc), m_cls(meth->getClass()) {}

const char* MethScopeVariableEnvironment::currentContext() const {
  return m_cls->name().c_str();
}

const ClassStatement* MethScopeVariableEnvironment::currentClassStatement()
  const {
  return m_cls;
}

NestedVariableEnvironment::NestedVariableEnvironment
(LVariableTable *ext, const Block &blk, CArrRef params /* = Array() */,
 CObjRef current_object /* = Object() */)
  : m_ext(ext), m_block(blk), m_params(params) {
  if (!current_object.isNull()) setCurrentObject(current_object);
  g_sinit.initGlobals(*this);
}

void NestedVariableEnvironment::flagStatic(CStrRef name, int64 hash) {
  // Behavior is to set the variable to init
  // .. and do some other stupid stuff that I'm not going to try
  get(name, hash) = m_block.getStaticValue(*this, name);
}

bool NestedVariableEnvironment::exists(const char *s, int64 hash) const {
  return m_ext->exists(s, hash);
}

Variant &NestedVariableEnvironment::getImpl(CStrRef s, int64 hash) {
  return m_ext->get(s, hash);
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


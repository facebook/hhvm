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

#include <runtime/vm/instrumentation.h>
#include <runtime/vm/unit.h>
#include <runtime/vm/runtime.h>
#include <runtime/base/execution_context.h>

namespace HPHP {
namespace VM {

///////////////////////////////////////////////////////////////////////////////

void Injection::execute() const {
  if (m_builtin) {
    ASSERT(m_callback);
    // Execute function in runtime
    m_callback(m_arg);
    return;
  }
  // Execute php code piece
  TypedValue retval;
  VarEnv *varEnv = NULL;
  ActRec *cfpSave = NULL;
  ObjectData *this_ = NULL;
  Class *cls = NULL;
  ActRec *fp = g_vmContext->getFP();
  if (fp) {
    if (!fp->hasVarEnv()) {
      fp->m_varEnv = VarEnv::createLazyAttach(fp);
    }
    varEnv = fp->m_varEnv;
    cfpSave = varEnv->getCfp();
    if (fp->hasThis()) {
      this_ = fp->getThis();
    } else if (fp->hasClass()) {
      cls = fp->getClass();
    }
  }
  // Note: For now we don't merge analysis code's class and function.
  // Later we might decide to do so
  g_vmContext->invokeFunc(&retval, m_unit->getMain(Transl::curClass()),
                          Array::Create(), this_, cls, varEnv, NULL, NULL);
  if (varEnv) {
    varEnv->setCfp(cfpSave);
  }
}

///////////////////////////////////////////////////////////////////////////////

static InjectionCache* s_injectionCache = NULL;

class InjectionCacheHolder {
public:
  InjectionCacheHolder() {
    s_injectionCache = new InjectionCache();
  }
  ~InjectionCacheHolder() {
    delete s_injectionCache;
  }
};

static InjectionCacheHolder s_injectionCacheHolder;

const Injection* InjectionCache::GetInjection(const StringData* code,
                                              const StringData* desc) {
  return s_injectionCache->getInjectionImpl(code, desc);
}

const Injection* InjectionCache::GetInjection(const std::string& code,
                                              const std::string& desc) {
  return s_injectionCache->getInjectionImpl(code, desc);
}

const Injection* InjectionCache::GetInjection(Injection::Callback callback,
                                              void *arg,
                                              const StringData* desc) {
  return s_injectionCache->getInjectionImpl(callback, arg, desc);
}

const StringData* InjectionCache::GetStringData(const StringData* sd) {
  return s_injectionCache->getStringData(sd);
}

void InjectionCache::ClearCache() {
  s_injectionCache->clearCacheImpl();
}

const Injection* InjectionCache::getInjectionImpl(const StringData* code,
                                                  const StringData* desc) {
  Unit* unit = getUnit(getStringData(code));
  if (unit == NULL) {
    return NULL;
  }
  Injection injection(unit, getStringData(desc));
  const Injection* inj = getInjection(&injection);
  return inj;
}

const Injection* InjectionCache::getInjectionImpl(const std::string& code,
                                                  const std::string& desc) {
  StringData sdCode(code.c_str(), code.size(), AttachLiteral);
  StringData sdDesc(desc.c_str(), desc.size(), AttachLiteral);
  return getInjectionImpl(&sdCode, &sdDesc);
}

const Injection* InjectionCache::getInjectionImpl(Injection::Callback callback,
                                                  void *arg,
                                                  const StringData* desc) {
  Injection injection(callback, arg, desc);
  const Injection* inj = getInjection(&injection);
  return inj;
}

void InjectionCache::clearCacheImpl() {
  WriteLock lock(m_lock);
  for (InjectionMap::iterator iter = m_injectionCache.begin();
       iter != m_injectionCache.end(); ++iter) {
    delete iter->first;
  }
  m_injectionCache.clear();

  for (UnitMap::iterator iter = m_unitCache.begin();
       iter != m_unitCache.end(); ++iter) {
    delete iter->second;
  }
  m_unitCache.clear();

  for (StringDataMap::iterator iter = m_sdCache.begin();
       iter != m_sdCache.end(); ++iter) {
    delete iter->first;
  }
  m_sdCache.clear();
}

const StringData* InjectionCache::getStringData(const StringData* sd) {
  ReadLock lock(m_lock);
  StringDataMap::const_accessor accFind;
  if (m_sdCache.find(accFind, sd)) {
    return accFind->first;
  }
  accFind.release(); // Release read lock
  // Gap of lock
  StringData* sdata = new StringData(sd->data(), sd->size(), CopyMalloc);
  StringDataMap::accessor accInsert;
  if (!m_sdCache.insert(accInsert, sdata)) {
    // Same string inserted in gap of the lock
    delete sdata;
  }
  return accInsert->first;
}

Unit* InjectionCache::getUnit(const StringData* code) {
  ReadLock lock(m_lock);
  // Note: caller needs to make sure the parameter code is not temporary
  UnitMap::accessor acc;
  if (m_unitCache.insert(acc, code)) {
    Unit* unit = compile_string(code->data(), code->size());
    // Here we save it even if unit == NULL, that at least saves us from
    // compiling same illegal string
    acc->second = unit;
  }
  return acc->second;
}

const Injection* InjectionCache::getInjection(const Injection* inj) {
  ReadLock lock(m_lock);
  InjectionMap::const_accessor accFind;
  if (m_injectionCache.find(accFind, inj)) {
    return accFind->first;
  }
  accFind.release();
  Injection *injection = new Injection(*inj);
  InjectionMap::accessor accInsert;
  if (!m_injectionCache.insert(accInsert, injection)) {
    delete injection;
  }
  return accInsert->first;
}

///////////////////////////////////////////////////////////////////////////////

InjectionTables::InjectionTables()
  : m_int64Tables(InstHookTypeInt64Count), m_sdTables(InstHookTypeSDCount) {
  for (int i = 0; i < InstHookTypeInt64Count; i++) {
    m_int64Tables[i] = NULL;
  }
  for (int i = 0; i < InstHookTypeSDCount; i++) {
    m_sdTables[i] = NULL;
  }
}

InjectionTables::~InjectionTables() {
  clear();
}

void InjectionTables::clear() {
  for (int i = 0; i < InstHookTypeInt64Count; i++) {
    setInt64Table(i, NULL);
  }
  for (int i = 0; i < InstHookTypeSDCount; i++) {
    setSDTable(i, NULL);
  }
}


InjectionTables* InjectionTables::clone() {
  InjectionTables* newTables = new InjectionTables();
  for (int i = 0; i < InstHookTypeInt64Count; i++) {
    VM::InjectionTableInt64* table = m_int64Tables[i];
    if (!table) {
      newTables->m_int64Tables[i] = NULL;
      continue;
    }
    VM::InjectionTableInt64* newTable = new InjectionTableInt64();
    newTable->insert(table->begin(), table->end());
    newTables->m_int64Tables[i] = newTable;
  }
  for (int i = 0; i < InstHookTypeSDCount; i++) {
    VM::InjectionTableSD* table = m_sdTables[i];
    if (!table) {
      newTables->m_sdTables[i] = NULL;
      continue;
    }
    VM::InjectionTableSD* newTable = new InjectionTableSD();
    newTable->insert(table->begin(), table->end());
    newTables->m_sdTables[i] = newTable;
  }
  return newTables;
}

void InjectionTables::setInt64Table(int hookType, InjectionTableInt64* table) {
  if (m_int64Tables[hookType]) {
    delete m_int64Tables[hookType];
  }
  m_int64Tables[hookType] = table;
}

void InjectionTables::setSDTable(int hookType, InjectionTableSD* table) {
  if (m_sdTables[hookType]) {
    delete m_sdTables[hookType];
  }
  m_sdTables[hookType] = table;
}

int InjectionTables::countInjections() {
  int total = 0;
  for (int i = 0; i < InstHookTypeInt64Count; i++) {
    total += m_int64Tables[i]->size();
  }
  for (int i = 0; i < InstHookTypeSDCount; i++) {
    total += m_sdTables[i]->size();
  }
  return total;
}


///////////////////////////////////////////////////////////////////////////////

static InjectionTables* s_globalInjTables = NULL;
static ReadWriteMutex s_globalInjTableLock;

void InstHelpers::InstCustomStringCallback(const StringData* hook,
                                           Injection::Callback callback,
                                           void *arg, const StringData* desc) {
  const Injection* inj = InjectionCache::GetInjection(callback, arg, desc);
  ASSERT(inj);
  const StringData* hookCached = InjectionCache::GetStringData(hook);
  if (!g_vmContext->m_injTables) {
    g_vmContext->m_injTables = new InjectionTables();
  }
  if (!g_vmContext->m_injTables->getSDTable(InstHookTypeCustomEvt)) {
    g_vmContext->m_injTables->setSDTable(InstHookTypeCustomEvt,
                                       new InjectionTableSD());
  }
  InjectionTableSD* table =
    g_vmContext->m_injTables->getSDTable(InstHookTypeCustomEvt);
  (*table)[hookCached] = inj;
}

void InstHelpers::PushInstToGlobal() {
  WriteLock lock(s_globalInjTableLock);
  if (s_globalInjTables) {
    delete s_globalInjTables;
    s_globalInjTables = NULL;
  }
  if (g_vmContext->m_injTables) {
    s_globalInjTables = g_vmContext->m_injTables->clone();
  }
}

void InstHelpers::PullInstFromGlobal() {
  if (g_vmContext->m_injTables) {
    delete g_vmContext->m_injTables;
    g_vmContext->m_injTables = NULL;
  }
  ReadLock lock(s_globalInjTableLock);
  if (s_globalInjTables) {
    g_vmContext->m_injTables = s_globalInjTables->clone();
  }
}

int InstHelpers::CountGlobalInst() {
  ReadLock lock(s_globalInjTableLock);
  if (s_globalInjTables) {
    return s_globalInjTables->countInjections();
  }
  return 0;
}

void InstHelpers::ClearGlobalInst() {
  WriteLock lock(s_globalInjTableLock);
  if (s_globalInjTables) {
    delete s_globalInjTables;
    s_globalInjTables = NULL;
  }
}

///////////////////////////////////////////////////////////////////////////////

} }    // HPHP::VM

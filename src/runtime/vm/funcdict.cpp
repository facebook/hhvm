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

#include <runtime/base/runtime_option.h>
#include <util/base.h>

#include <runtime/base/execution_context.h>
#include <runtime/ext_hhvm/ext_hhvm.h>
#include <runtime/vm/funcdict.h>
#include <runtime/vm/translator/translator.h>
#include <runtime/vm/translator/targetcache.h>

namespace HPHP {
namespace VM {

FuncDict::FuncMap FuncDict::s_builtinFuncs;
FuncDict::ExtFuncMap FuncDict::s_extFuncHash;

FuncDict::FuncDict() : m_restrictRenameableFunctions(false) { }

void FuncDict::ProcessInit() {
  ASSERT(s_extFuncHash.empty());
  ASSERT(s_builtinFuncs.empty());
  // Populate Func::s_extFuncHash
  for (long long i = 0LL; i < hhbc_ext_funcs_count; ++i) {
    const HhbcExtFuncInfo* info = &hhbc_ext_funcs[i];
    StringData* s = StringData::GetStaticString(info->m_name);
    Func::BuiltinFunction bif = (Func::BuiltinFunction)info->m_pGenericFunc;
    const ClassInfo::MethodInfo* mi = ClassInfo::FindFunction(s->data());
    mapInsertUnique(s_extFuncHash, s, bif);
    mapInsertUnique(s_builtinFuncs, s, new Func(s, mi, bif));
  }
}

bool FuncDict::rename(const StringData* old, const StringData* n3w) {
  ASSERT(isFunctionRenameable(old) ||
         isFunctionRenameable(n3w));

  // When EvalJitEnableRenameFunction is false, the translator may wire
  // Func*'s into the TC. Don't rename functions.
  if (RuntimeOption::EvalJit && !RuntimeOption::EvalJitEnableRenameFunction) {
    raise_error("You must explicitly enable fb_rename_function in the JIT "
                "(-v Eval.JitEnableRenameFunction=true)");
  }

  Func* func;
  bool hitInBuiltinFuncs = false;
  if (!mapGet(m_funcs, old, &func)) {
    if (!mapGet(s_builtinFuncs, old, &func)) {
      // It's the caller's responsibility to ensure that the old function
      // exists.
      not_reached();
    } else {
      hitInBuiltinFuncs = true;
    }
  }

  // Once we've renamed a builtin, we should never look it up in the static
  // area again.
  if (hitInBuiltinFuncs) {
    // Make sure the old name, which may be a transient StringData, stays
    // around.
    old->setStatic();
    m_builtinBlackList.insert(old);
  }

  n3w->incRefCount();
  old->decRefCount();
  // This can't be the last reference to it; it was passed into fb_rename()
  ASSERT(old->getCount() > 0);

  m_funcs.erase(old);
  mapInsertUnique(m_funcs, n3w, func);

  if (RuntimeOption::EvalJit) {
    VM::Transl::TargetCache::invalidateFuncName(old);
  }

  ASSERT(get(old) == NULL);
  ASSERT(get(n3w) != NULL);
  return true;
}

Func* FuncDict::getBuiltin(const StringData* sd) const {
  ASSERT(!mapContains(m_funcs, sd));
  Func* retval;
  if (LIKELY(mapGet(s_builtinFuncs, sd, &retval)) &&
      !mapContains(m_builtinBlackList, sd)) {
    return retval;
  }
  return NULL;
}

void FuncDict::insert(const StringData* name, Func* f) {
  mapInsert(m_funcs, name, f);
}

bool FuncDict::isFunctionRenameable(const StringData* name) {
  return !m_restrictRenameableFunctions ||
    mapContains(m_renameableFunctions, name);
}

void FuncDict::addRenameableFunctions(ArrayData* arr) {
  m_restrictRenameableFunctions = true;
  for (ArrayIter iter(arr); iter; ++iter) {
    String name = iter.second().toString();
    if (!name.empty()) {
      m_renameableFunctions.insert(name.get());
    }
  }
}

bool FuncDict::interceptFunction(CStrRef name, CVarRef handler,
                                 CVarRef data) {
  if (!handler.toBoolean() && name.empty()) {
    // Resetting individual intercepts is handled below.
    m_interceptHandlers.clear();
    return true;
  }

  if (name.empty()) {
    // This is supposed to intercept "every function". This is 100% bonkers --
    // why would you ever, ever do that? -- and it doesn't even work in hphpi.
    not_implemented();
  }

  int pos = name.find("::");
  if (pos != String::npos) {
    // Intercepting a method.
    String classname = name.substr(0, pos);
    String methodname = name.substr(pos + 2);

    Class* cls = g_context->lookupClass(classname.get());
    if (cls == NULL || cls->m_isCppExtClass) {
      // Can't intercept in a nonexistent or C++ builtin class
      return false;
    }

    const Func* original = cls->lookupMethod(methodname.get());
    if (original == NULL) {
      // Can't intercept nonexistent method
      return false;
    }

    if (!handler.toBoolean()) {
      m_interceptHandlers.erase(original);
    } else {
      m_interceptHandlers[original] =
        InterceptDataPtr(new InterceptData(handler, data, name));
    }
  } else {
    // Intercepting a regular function
    const Func* original = get(name.get());
    if (original == NULL || original->m_info != NULL) {
      // Can't intercept nonexistent or builtin functions
      return false;
    }

    if (!handler.toBoolean()) {
      m_interceptHandlers.erase(original);
    } else {
      m_interceptHandlers[original] =
        InterceptDataPtr(new InterceptData(handler, data, name));
    }
  }

  return true;
}

bool FuncDict::hasAnyIntercepts() {
  return m_interceptHandlers.size() > 0;
}

FuncDict::InterceptDataPtr FuncDict::getInterceptData(const Func* func) {
  return mapGet(m_interceptHandlers, func);
}

Array FuncDict::getUserFunctions() {
  Array a = Array::Create();
  for (FuncMap::const_iterator it = m_funcs.begin(); it != m_funcs.end();
       ++it) {
    a.append(*(String*)(&it->second->m_name));
  }
  return a;
}

} } // HPHP::VM

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
#include <runtime/vm/unit.h>

#include <system/lib/systemlib.h>

namespace HPHP {
namespace VM {

RenamedFuncDict::RenamedFuncDict() : m_restrictRenameableFunctions(false) { }

bool RenamedFuncDict::rename(const StringData* old, const StringData* n3w) {
  ASSERT(isFunctionRenameable(old) ||
         isFunctionRenameable(n3w));

  NamedEntity *oldNe = const_cast<NamedEntity *>(Unit::GetNamedEntity(old));
  NamedEntity *newNe = const_cast<NamedEntity *>(Unit::GetNamedEntity(n3w));

  Func* func = Unit::lookupFunc(oldNe, old);
  if (!func) {
      // It's the caller's responsibility to ensure that the old function
      // exists.
      not_reached();
  }

  if (!(func->attrs() & AttrDynamicInvoke)) {
    // When EvalJitEnableRenameFunction is false, the translator may wire
    // non-DynamicInvoke Func*'s into the TC. Don't rename functions.
    if (RuntimeOption::EvalJit && !RuntimeOption::EvalJitEnableRenameFunction) {
      raise_error("You must explicitly enable fb_rename_function in the JIT "
                  "(-v Eval.JitEnableRenameFunction=true)");
    }
  }

  Func *fnew = Unit::lookupFunc(newNe, n3w);
  if (fnew && fnew != func) {
    // To match hphpc, we silently ignore functions defined in user code that
    // have the same name as a function defined in a separable extension
    if (!fnew->isIgnoreRedefinition()) {
      raise_error("Function already defined: %s", n3w->data());
    } else {
      return false;
    }
  }

  oldNe->setCachedFunc(NULL);
  if (UNLIKELY(newNe->m_cachedFuncOffset == 0)) {
    Transl::TargetCache::allocFixedFunction(newNe, false);
  }
  newNe->setCachedFunc(func);

  if (RuntimeOption::EvalJit) {
    VM::Transl::TargetCache::invalidateForRename(old);
  }

  return true;
}

bool RenamedFuncDict::isFunctionRenameable(const StringData* name) {
  return !m_restrictRenameableFunctions ||
    mapContains(m_renameableFunctions, name);
}

void RenamedFuncDict::addRenameableFunctions(ArrayData* arr) {
  m_restrictRenameableFunctions = true;
  for (ArrayIter iter(arr); iter; ++iter) {
    String name = iter.second().toString();
    if (!name.empty()) {
      m_renameableFunctions.insert(name.get());
    }
  }
}

} } // HPHP::VM

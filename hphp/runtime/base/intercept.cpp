/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/intercept.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/configs/jit.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/req-optional.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/tc-intercept.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/util/lock.h"
#include "hphp/util/rds-local.h"
#include "hphp/util/trace.h"


///////////////////////////////////////////////////////////////////////////////

namespace HPHP {

TRACE_SET_MOD(intercept);

struct InterceptRequestData final : RequestEventHandler {
  InterceptRequestData() {}

  void clear() {
    m_intercept_handlers.reset();
  }

  void requestInit() override { clear(); }
  void requestShutdown() override { clear(); }

  req::hash_map<const Func*, Variant>& intercept_handlers() {
    if (!m_intercept_handlers) m_intercept_handlers.emplace();
    return *m_intercept_handlers;
  }
  bool empty() const {
    return !m_intercept_handlers.has_value() ||
            m_intercept_handlers->empty();
  }

private:
  // get_intercept_handler() returns Variant* pointing into this map,
  // so we need reference stability.
  req::Optional<req::hash_map<const Func*, Variant>> m_intercept_handlers;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(InterceptRequestData, s_intercept_data);

///////////////////////////////////////////////////////////////////////////////

bool register_intercept_surprise_flags(const Variant& callback,
                                       Func *const interceptedFunc) {
  assertx(!RO::EvalFastMethodIntercept);

  if (!callback.toBoolean()) {
    if (!s_intercept_data->empty()) {
      auto& handlers = s_intercept_data->intercept_handlers();
      auto it = handlers.find(interceptedFunc);
      if (it != handlers.end()) {
        // erase the map entry before destroying the value
        auto tmp = it->second;
        handlers.erase(it);
      }
    }
    // We've cleared out all the intercepts, so we don't need to pay the
    // surprise flag cost anymore
    if (s_intercept_data->empty()) {
      EventHook::DisableIntercept();
    }
    return true;
  }

  EventHook::EnableIntercept();

  auto& handlers = s_intercept_data->intercept_handlers();
  handlers[interceptedFunc] = callback;
  interceptedFunc->setMaybeIntercepted();
  return true;
}

bool register_intercept_fast_method_intercept(const Variant& callback,
                                              Func *const interceptedFunc) {
  assertx(RO::EvalFastMethodIntercept);
  auto interceptedFuncId = interceptedFunc->getFuncId();

  if (!callback.toBoolean()) {
    if (!s_intercept_data->empty()) {
      auto& handlers = s_intercept_data->intercept_handlers();
      auto it = handlers.find(interceptedFunc);
      if (it != handlers.end()) {
        // erase the map entry before destroying the value
        auto tmp = it->second;
        handlers.erase(it);
        // Record that the current request does not intercept anymore interceptedFunc.
        // If no other request intercepts interceptedFunc, then replace the forced `jmp`
        // to the surprise stub with a conditional `jcc` in the TC translations of
        // interceptedFunc.
        jit::tc::stopInterceptFunc(interceptedFuncId);
      }
    }
    return true;
  }

  auto& handlers = s_intercept_data->intercept_handlers();
  auto [it, did_insert] = handlers.emplace(interceptedFunc, callback);
  if (!did_insert) {
    it->second = callback;
  } else {
    // The request didn't previously intercept this function: record this and
    // replace the conditional `jcc` implementing the surprise flag check with a
    // forced jmp to the surprise stub in all the TC translations of the function.
    jit::tc::startInterceptFunc(interceptedFuncId);
  };
  interceptedFunc->setMaybeIntercepted();
  return true;
}

bool register_intercept(const String& name, const Variant& callback) {
  SCOPE_EXIT {
    DEBUGGER_ATTACHED_ONLY(phpDebuggerInterceptRegisterHook(name));
  };

  auto const interceptedFunc = [&]() -> Func* {
    auto const pos = name.find("::");
    if (pos != 0 && pos != String::npos && pos + 2 < name.size()) {
      auto const cls = Class::load(name.substr(0, pos).get());
      if (!cls) return nullptr;
      auto const meth = cls->lookupMethod(name.substr(pos + 2).get());
      if (!meth || meth->cls() != cls) return nullptr;
      return meth;
    } else {
      return Func::load(name.get());
    }
  }();

  if (interceptedFunc == nullptr) {
    // This intercept request can't possibly do anything, we are done.
    // TODO: should this throw?
    return true;
  }

  /*
   * In production mode, only functions that we have assumed can be
   * intercepted during static analysis should actually be
   * intercepted. Otherwise, we refer to the NonInterceptableFunctions
   * blocklist in the config.
   */
  if (!(interceptedFunc->isInterceptable())) {
    if (RO::RepoAuthoritative) {
      raise_error("fb_intercept2 was used on a non-interceptable function (%s) "
                  "in RepoAuthoritative mode", interceptedFunc->fullName()->data());
    } else {
      raise_error("fb_intercept2 was used on a non-interceptable function (%s). "
                  "It appears in the NonInterceptableFunctions blocklist.",
                  interceptedFunc->fullName()->data());
    }
  }

  if (RO::EvalDumpJitEnableRenameFunctionStats &&
      StructuredLog::coinflip(Cfg::Jit::InterceptFunctionLogRate)) {
    StructuredLogEntry entry;
    entry.setStr("intercepted_func", interceptedFunc->fullName()->data());
    addBacktraceToStructLog(
      createBacktrace(BacktraceArgs()), entry
    );
    StructuredLog::log("hhvm_intercept_function", entry);
  }

  if (RO::EvalFastMethodIntercept) {
    return register_intercept_fast_method_intercept(callback, interceptedFunc);
  } else {
    return register_intercept_surprise_flags(callback, interceptedFunc);
  }
}

bool is_intercepted(const Func* func) {
  auto const h = get_intercept_handler(func);
  return (h != nullptr);
}

Variant* get_intercept_handler(const Func* func) {
  FTRACE(1, "get_intercept_handler {}\n", func->fullName());

  if (!s_intercept_data->empty()) {
    auto& handlers = s_intercept_data->intercept_handlers();
    auto iter = handlers.find(func);
    if (iter != handlers.end()) {
      assertx(func->maybeIntercepted());
      return &iter->second;
    }
  }
  return nullptr;
}

void reset_all_intercepted_functions() {
  if (RO::EvalFastMethodIntercept && !s_intercept_data->empty()) {
    auto& handlers = s_intercept_data->intercept_handlers();
    for (auto& h: handlers) {
      jit::tc::stopInterceptFunc(h.first->getFuncId());
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// fb_rename_function()

void rename_function(const String& old_name, const String& new_name) {
  auto const old = old_name.get();
  auto const n3w = new_name.get();
  auto const oldNe = const_cast<NamedFunc*>(NamedFunc::getNoCreate(old));

  Func* func = oldNe ? oldNe->getCachedFunc() : nullptr;
  if (!func) {
    // It's the caller's responsibility to ensure that the old function exists.
    not_reached();
  }

  if (!RuntimeOption::funcIsRenamable(old)) {
    if (Cfg::Jit::EnableRenameFunction == 2) {
      raise_error("fb_rename_function must be explicitly enabled for %s "
                  "(when Eval.JitEnableRenameFunction=2 by adding it to "
                  "option Eval.RenamableFunctions)", old->data());
    } else {
      raise_error("fb_rename_function must be explicitly enabled"
                  "(-v Eval.JitEnableRenameFunction=1)");
    }
  }

  auto const newNe = const_cast<NamedFunc*>(NamedFunc::getOrCreate(n3w));
  auto const fnew = newNe->getCachedFunc();
  if (fnew && fnew != func) {
    raise_error("Function already defined: %s", n3w->data());
  }

  if (StructuredLog::enabled() &&
    RuntimeOption::EvalDumpJitEnableRenameFunctionStats) {
    StructuredLogEntry entry;
    entry.setStr("old_function_name", old->data());
    entry.setStr("new_function_name", n3w->data());
    StructuredLog::log("hhvm_rename_function", entry);
  }

  always_assert(
    !rds::isPersistentHandle(oldNe->getFuncHandle(func->fullName()))
  );
  oldNe->setCachedFunc(nullptr);
  auto const newName = fnew ? fnew->fullName() : makeStaticString(n3w);
  newNe->m_cachedFunc.bind(rds::Mode::Normal, rds::LinkName{"NEFunc", newName});
  newNe->setCachedFunc(func);

  if (Cfg::Jit::Enabled) {
    jit::invalidateForRenameFunction(old);
  }
}

///////////////////////////////////////////////////////////////////////////////
}

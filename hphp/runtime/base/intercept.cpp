/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/event-hook.h"

#include "hphp/parser/parser.h"
#include "hphp/util/lock.h"

#include "hphp/runtime/base/file-repository.h"
#include "hphp/util/trace.h"

using namespace HPHP::Trace;

///////////////////////////////////////////////////////////////////////////////

namespace HPHP {

TRACE_SET_MOD(intercept);

class InterceptRequestData : public RequestEventHandler {
public:
  InterceptRequestData()
      : m_use_allowed_functions(false) {
  }

  void clear() {
    m_use_allowed_functions = false;
    m_allowed_functions.clear();
    m_renamed_functions.clear();
    m_global_handler.reset();
    m_intercept_handlers.clear();
  }

  virtual void requestInit() {
    clear();
  }

  virtual void requestShutdown() {
    clear();
  }

public:
  bool m_use_allowed_functions;
  StringISet m_allowed_functions;
  StringIMap<String> m_renamed_functions;

  Variant m_global_handler;
  StringIMap<Variant> m_intercept_handlers;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(InterceptRequestData, s_intercept_data);

static Mutex s_mutex;

/*
 * The bool indicates whether fb_intercept has ever been called
 * on a function with this name.
 * The vector contains a list of maybeIntercepted flags for functions
 * with this name.
 */
typedef StringIMap<std::pair<bool,vector<char*>>> RegisteredFlagsMap;

static RegisteredFlagsMap s_registered_flags;

///////////////////////////////////////////////////////////////////////////////

static void flag_maybe_intercepted(vector<char*> &flags) {
  for (auto flag : flags) {
    *flag = 1;
  }
}

bool register_intercept(const String& name, CVarRef callback, CVarRef data) {
  StringIMap<Variant> &handlers = s_intercept_data->m_intercept_handlers;
  if (!callback.toBoolean()) {
    if (name.empty()) {
      s_intercept_data->m_global_handler.reset();
      handlers.clear();
    } else {
      handlers.erase(name);
    }
    return true;
  }

  EventHook::EnableIntercept();

  Array handler = make_packed_array(callback, data);

  if (name.empty()) {
    s_intercept_data->m_global_handler = handler;
    handlers.clear();
  } else {
    handlers[name] = handler;
  }

  Lock lock(s_mutex);
  if (name.empty()) {
    for (auto& entry : s_registered_flags) {
      flag_maybe_intercepted(entry.second.second);
    }
  } else {
    StringData* sd = name.get();
    if (!sd->isStatic()) {
      sd = makeStaticString(sd);
    }
    auto &entry = s_registered_flags[StrNR(sd)];
    entry.first = true;
    flag_maybe_intercepted(entry.second);
  }

  return true;
}

static Variant *get_enabled_intercept_handler(const String& name) {
  Variant *handler = nullptr;
  StringIMap<Variant> &handlers = s_intercept_data->m_intercept_handlers;
  StringIMap<Variant>::iterator iter = handlers.find(name);
  if (iter != handlers.end()) {
    handler = &iter->second;
  } else {
    handler = &s_intercept_data->m_global_handler;
    if (handler->isNull()) {
      return nullptr;
    }
  }
  return handler;
}

Variant *get_intercept_handler(const String& name, char* flag) {
  TRACE(1, "get_intercept_handler %s flag is %d\n",
        name.get()->data(), (int)*flag);
  if (*flag == -1) {
    Lock lock(s_mutex);
    if (*flag == -1) {
      StringData *sd = name.get();
      if (!sd->isStatic()) {
        sd = makeStaticString(sd);
      }
      auto &entry = s_registered_flags[StrNR(sd)];
      entry.second.push_back(flag);
      *flag = entry.first;
    }
    if (!*flag) return nullptr;
  }

  Variant *handler = get_enabled_intercept_handler(name);
  if (handler == nullptr) {
    return nullptr;
  }
  assert(*flag);
  return handler;
}

void unregister_intercept_flag(const String& name, char *flag) {
  Lock lock(s_mutex);
  RegisteredFlagsMap::iterator iter =
    s_registered_flags.find(name);
  if (iter != s_registered_flags.end()) {
    vector<char*> &flags = iter->second.second;
    for (int i = flags.size(); i--; ) {
      if (flag == flags[i]) {
        flags.erase(flags.begin() + i);
        break;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// fb_rename_function()

void rename_function(const String& old_name, const String& new_name) {
  auto const old = old_name.get();
  auto const n3w = new_name.get();
  auto const oldNe = const_cast<NamedEntity*>(Unit::GetNamedEntity(old));
  auto const newNe = const_cast<NamedEntity*>(Unit::GetNamedEntity(n3w));

  Func* func = Unit::lookupFunc(oldNe);
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

  Func *fnew = Unit::lookupFunc(newNe);
  if (fnew && fnew != func) {
    // To match hphpc, we silently ignore functions defined in user code that
    // have the same name as a function defined in a separable extension
    if (!fnew->isAllowOverride()) {
      raise_error("Function already defined: %s", n3w->data());
    }
    return;
  }

  oldNe->setCachedFunc(nullptr);
  newNe->m_cachedFunc.bind();
  newNe->setCachedFunc(func);

  if (RuntimeOption::EvalJit) {
    JIT::invalidateForRenameFunction(old);
  }
}

///////////////////////////////////////////////////////////////////////////////
}

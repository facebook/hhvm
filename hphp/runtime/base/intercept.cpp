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

#include <vector>
#include <utility>

#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/event-hook.h"

#include "hphp/parser/parser.h"
#include "hphp/util/lock.h"

#include "hphp/runtime/base/unit-cache.h"
#include "hphp/util/trace.h"

using namespace HPHP::Trace;

///////////////////////////////////////////////////////////////////////////////

namespace HPHP {

TRACE_SET_MOD(intercept);

struct InterceptRequestData final : RequestEventHandler {
  InterceptRequestData() {}

  void clear() {
    m_global_handler.releaseForSweep();
    m_intercept_handlers.clear();
  }

  void requestInit() override { clear(); }
  void requestShutdown() override { clear(); }

  Variant& global_handler() { return m_global_handler; }
  req::StringIMap<Variant>& intercept_handlers() {
    if (!m_intercept_handlers) m_intercept_handlers.emplace();
    return *m_intercept_handlers;
  }
  bool empty() const {
    return !m_intercept_handlers.hasValue() ||
            m_intercept_handlers->empty();
  }
  void clearHandlers() {
    m_intercept_handlers.clear();
  }

private:
  Variant m_global_handler;
  req::Optional<req::StringIMap<Variant>> m_intercept_handlers;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(InterceptRequestData, s_intercept_data);

static Mutex s_mutex;

/*
 * The bool indicates whether fb_intercept has ever been called
 * on a function with this name.
 * The vector contains a list of maybeIntercepted flags for functions
 * with this name.
 */
typedef StringIMap<std::pair<bool,std::vector<int8_t*>>> RegisteredFlagsMap;

static RegisteredFlagsMap s_registered_flags;

///////////////////////////////////////////////////////////////////////////////

static void flag_maybe_intercepted(std::vector<int8_t*> &flags) {
  for (auto flag : flags) {
    *flag = 1;
  }
}

bool register_intercept(const String& name, const Variant& callback,
                        const Variant& data) {
  if (!callback.toBoolean()) {
    if (name.empty()) {
      s_intercept_data->global_handler().unset();
      s_intercept_data->clear();
    } else {
      if (!s_intercept_data->empty()) {
        auto& handlers = s_intercept_data->intercept_handlers();
        auto it = handlers.find(name);
        if (it != handlers.end()) {
          auto tmp = it->second;
          handlers.erase(it);
        }
      }
    }
    return true;
  }

  EventHook::EnableIntercept();

  Array handler = make_packed_array(callback, data);

  if (name.empty()) {
    s_intercept_data->global_handler() = handler;
    s_intercept_data->clearHandlers();
  } else {
    auto& handlers = s_intercept_data->intercept_handlers();
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
  if (!s_intercept_data->empty()) {
    auto& handlers = s_intercept_data->intercept_handlers();
    auto iter = handlers.find(name);
    if (iter != handlers.end()) {
      return &iter->second;
    }
  }
  auto handler = &s_intercept_data->global_handler();
  if (handler->isNull()) {
    return nullptr;
  }
  return handler;
}

Variant *get_intercept_handler(const String& name, int8_t* flag) {
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

void unregister_intercept_flag(const String& name, int8_t *flag) {
  Lock lock(s_mutex);
  RegisteredFlagsMap::iterator iter =
    s_registered_flags.find(name);
  if (iter != s_registered_flags.end()) {
    std::vector<int8_t*> &flags = iter->second.second;
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
  auto const oldNe = const_cast<NamedEntity*>(NamedEntity::get(old));
  auto const newNe = const_cast<NamedEntity*>(NamedEntity::get(n3w));

  Func* func = Unit::lookupFunc(oldNe);
  if (!func) {
    // It's the caller's responsibility to ensure that the old function
    // exists.
    not_reached();
  }

  // Interceptable functions can be renamed even when
  // JitEnableRenameFunction is false.
  if (!(func->attrs() & AttrInterceptable)) {
    if (!RuntimeOption::EvalJitEnableRenameFunction) {
      // When EvalJitEnableRenameFunction is false, the translator may
      // wire non-AttrInterceptable Func*'s into the TC. Don't rename
      // functions.
      raise_error("fb_rename_function must be explicitly enabled"
                  "(-v Eval.JitEnableRenameFunction=true)");
    }
  }

  auto const fnew = Unit::lookupFunc(newNe);
  if (fnew && fnew != func) {
    raise_error("Function already defined: %s", n3w->data());
  }

  always_assert(!rds::isPersistentHandle(oldNe->getFuncHandle()));
  oldNe->setCachedFunc(nullptr);
  newNe->m_cachedFunc.bind();
  newNe->setCachedFunc(func);

  if (RuntimeOption::EvalJit) {
    jit::invalidateForRenameFunction(old);
  }
}

///////////////////////////////////////////////////////////////////////////////
}

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
#include "hphp/runtime/base/intercept.h"
#include "hphp/runtime/base/util/request_local.h"
#include "hphp/runtime/base/array/array_init.h"
#include "hphp/runtime/base/array/array_iterator.h"
#include "hphp/runtime/base/type_conversions.h"
#include "hphp/runtime/base/builtin_functions.h"
#include "hphp/runtime/vm/translator/targetcache.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/event_hook.h"

#include "hphp/util/parser/parser.h"
#include "hphp/util/lock.h"

#include "hphp/runtime/eval/runtime/file_repository.h"
#include "hphp/runtime/vm/translator/translator-x64.h"
#include "hphp/util/trace.h"

using namespace HPHP::Trace;

///////////////////////////////////////////////////////////////////////////////

namespace HPHP {

static const Trace::Module TRACEMOD = Trace::intercept;

class InterceptRequestData : public RequestEventHandler {
public:
  InterceptRequestData()
      : m_use_allowed_functions(false) {
  }

  void clear() {
    *s_hasRenamedFunction = false;
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
IMPLEMENT_THREAD_LOCAL_NO_CHECK(bool, s_hasRenamedFunction);

static Mutex s_mutex;
typedef StringIMap<vector<char*> > RegisteredFlagsMap;

static RegisteredFlagsMap s_registered_flags;

///////////////////////////////////////////////////////////////////////////////

static void flag_maybe_interrupted(vector<char*> &flags) {
  for (int i = flags.size() - 1; i >= 0; i--) {
    *flags[i] = 1;
  }
}

bool register_intercept(CStrRef name, CVarRef callback, CVarRef data) {
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

  Array handler = CREATE_VECTOR2(callback, data);

  if (name.empty()) {
    s_intercept_data->m_global_handler = handler;
    handlers.clear();
  } else {
    handlers[name] = handler;
  }

  Lock lock(s_mutex);
  if (name.empty()) {
    for (RegisteredFlagsMap::iterator iter =
           s_registered_flags.begin();
         iter != s_registered_flags.end(); ++iter) {
      flag_maybe_interrupted(iter->second);
    }
  } else {
    RegisteredFlagsMap::iterator iter =
      s_registered_flags.find(name);
    if (iter != s_registered_flags.end()) {
      flag_maybe_interrupted(iter->second);
    }
  }

  return true;
}

Variant *get_enabled_intercept_handler(CStrRef name) {
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

Variant *get_intercept_handler(CStrRef name, char* flag) {
  TRACE(1, "get_intercept_handler %s flag is %d\n",
        name.get()->data(), (int)*flag);
  if (*flag == -1) {
    Lock lock(s_mutex);
    if (*flag == -1) {
      StringData *sd = name.get();
      if (!sd->isStatic()) {
        sd = StringData::GetStaticString(sd);
      }
      s_registered_flags[StrNR(sd)].push_back(flag);
      *flag = 0;
    }
  }

  Variant *handler = get_enabled_intercept_handler(name);
  if (handler == nullptr) {
    return nullptr;
  }
  *flag = 1;
  return handler;
}

void unregister_intercept_flag(CStrRef name, char *flag) {
  Lock lock(s_mutex);
  RegisteredFlagsMap::iterator iter =
    s_registered_flags.find(name);
  if (iter != s_registered_flags.end()) {
    vector<char*> &flags = iter->second;
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

void check_renamed_functions(CArrRef names) {
  g_vmContext->addRenameableFunctions(names.get());
}

bool check_renamed_function(CStrRef name) {
  return g_vmContext->isFunctionRenameable(name.get());
}

void rename_function(CStrRef old_name, CStrRef new_name) {
  g_vmContext->renameFunction(old_name.get(), new_name.get());
}

String get_renamed_function(CStrRef name) {
  HPHP::Func* f = HPHP::Unit::lookupFunc(name.get());
  if (f) {
    return f->nameRef();
  }
  return name;
}

///////////////////////////////////////////////////////////////////////////////
}

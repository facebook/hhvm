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
#include <runtime/base/intercept.h>
#include <runtime/base/util/request_local.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <util/lock.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

namespace HPHP {
class InterceptRequestData : public RequestEventHandler {
public:
  InterceptRequestData()
      : m_use_allowed_functions(false), m_has_renamed_functions(false) {
  }

  void clear() {
    m_use_allowed_functions = false;
    m_has_renamed_functions = false;
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
  bool m_has_renamed_functions;
  StringISet m_allowed_functions;
  StringIMap<String> m_renamed_functions;

  Variant m_global_handler;
  StringIMap<Variant> m_intercept_handlers;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(InterceptRequestData, s_intercept_data);

static Mutex s_mutex;
static hphp_string_imap<vector<char*> > s_registered_flags;
static set<char*> s_unregistered_flags;

///////////////////////////////////////////////////////////////////////////////

static void flag_maybe_interrupted(vector<char*> &flags) {
  for (int i = flags.size() - 1; i >= 0; i--) {
    char *p = flags[i];
    set<char*>::iterator iter = s_unregistered_flags.find(p);
    if (iter != s_unregistered_flags.end()) {
      flags.erase(flags.begin() + i);
      s_unregistered_flags.erase(iter);
    } else {
      *p = 1;
    }
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

  Array handler;
  handler.set("callback", callback);
  handler.set("data", data);

  if (name.empty()) {
    s_intercept_data->m_global_handler = handler;
    handlers.clear();
  } else {
    handlers[name] = handler;
  }

  Lock lock(s_mutex);
  if (name.empty()) {
    for (hphp_string_imap<vector<char*> >::iterator iter =
           s_registered_flags.begin();
         iter != s_registered_flags.end(); ++iter) {
      flag_maybe_interrupted(iter->second);
    }
  } else {
    hphp_string_imap<vector<char*> >::iterator iter =
      s_registered_flags.find(name.data());
    if (iter != s_registered_flags.end()) {
      flag_maybe_interrupted(iter->second);
    }
  }

  return true;
}

Variant get_intercept_handler(CStrRef name, char *flag) {
  if (*flag == -1) {
    Lock lock(s_mutex);
    if (*flag == -1) {
      s_unregistered_flags.erase(flag); // in case memory address re-use
      s_registered_flags[name.data()].push_back(flag);
      *flag = 0;
    }
  }

  Variant handler;
  StringIMap<Variant> &handlers = s_intercept_data->m_intercept_handlers;
  StringIMap<Variant>::iterator iter = handlers.find(name);
  if (iter != handlers.end()) {
    handler = iter->second;
  } else {
    handler = s_intercept_data->m_global_handler;
  }
  if (!handler.isNull()) {
    *flag = 1;
  }
  return handler;
}

bool handle_intercept(CVarRef handler, CStrRef name, CArrRef params,
                      Variant &ret) {
  ObjectData *obj = FrameInjection::GetThis();

  Variant done = true;
  ret = ref(f_call_user_func_array
            (handler["callback"],
             CREATE_VECTOR5(name, obj, params, handler["data"], ref(done))));
  return !done.same(false);
}

void unregister_intercept_flag(char *flag) {
  Lock lock(s_mutex);
  s_unregistered_flags.insert(flag);
}

///////////////////////////////////////////////////////////////////////////////
// fb_rename_function()

void check_renamed_functions(CArrRef names) {
  s_intercept_data->m_use_allowed_functions = true;
  StringISet &allowed = s_intercept_data->m_allowed_functions;
  for (ArrayIter iter(names); iter; ++iter) {
    String name = iter.second().toString();
    if (!name.empty()) {
      allowed.insert(name);
    }
  }
}

bool check_renamed_function(CStrRef name) {
  if (s_intercept_data->m_use_allowed_functions) {
    StringISet &allowed = s_intercept_data->m_allowed_functions;
    return allowed.find(name) != allowed.end();
  }
  return true;
}

void rename_function(CStrRef old_name, CStrRef new_name) {
  StringIMap<String> &funcs = s_intercept_data->m_renamed_functions;

  String orig_name = old_name;
  /*
    Name beginning with '1' is from create_function.
    We allow such functions to be renamed to multiple
    different names. They also report that they exist,
    even after renaming
  */
  if (old_name.data()[0] != '1') {
    StringIMap<String>::iterator iter = funcs.find(old_name);
    if (iter != funcs.end()) {
      if (!iter->second.empty()) {
        orig_name = iter->second;
        iter->second = empty_string;
      }
    } else {
      funcs[old_name] = empty_string;
    }
  }

  if (new_name.data()[0] != '1') {
    funcs[new_name] = orig_name;
  }
  s_intercept_data->m_has_renamed_functions = true;
}

String get_renamed_function(CStrRef name, bool *renamed /* = NULL */) {
  if (s_intercept_data->m_has_renamed_functions) {
    StringIMap<String> &funcs = s_intercept_data->m_renamed_functions;
    StringIMap<String>::const_iterator iter = funcs.find(name);
    if (iter != funcs.end()) {
      if (renamed) *renamed = true;
      return iter->second;
    }
  }
  if (renamed) *renamed = false;
  return name;
}

///////////////////////////////////////////////////////////////////////////////
}

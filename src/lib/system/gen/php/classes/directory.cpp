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

#include <php/classes/directory.h>
#include <cpp/ext/ext.h>
#include <cpp/eval/eval.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* preface starts */
/* preface finishes */
/* SRC: classes/directory.php line 3 */
Variant c_directory::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_directory::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_directory::o_get(ArrayElementVec &props) const {
  props.push_back(NEW(ArrayElement)("path", m_path.isReferenced() ? ref(m_path) : m_path));
  props.push_back(NEW(ArrayElement)("handle", m_handle.isReferenced() ? ref(m_handle) : m_handle));
  c_ObjectData::o_get(props);
}
bool c_directory::o_exists(CStrRef s, int64 hash) const {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_EXISTS_STRING(0x42DD5992F362B3C4LL, path, 4);
      HASH_EXISTS_STRING(0x48E8F48146EEEF5CLL, handle, 6);
      break;
    default:
      break;
  }
  return c_ObjectData::o_exists(s, hash);
}
Variant c_directory::o_get(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_RETURN_STRING(0x42DD5992F362B3C4LL, m_path,
                         path, 4);
      HASH_RETURN_STRING(0x48E8F48146EEEF5CLL, m_handle,
                         handle, 6);
      break;
    default:
      break;
  }
  return c_ObjectData::o_get(s, hash);
}
Variant c_directory::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_SET_STRING(0x42DD5992F362B3C4LL, m_path,
                      path, 4);
      HASH_SET_STRING(0x48E8F48146EEEF5CLL, m_handle,
                      handle, 6);
      break;
    default:
      break;
  }
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_directory::o_lval(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_RETURN_STRING(0x42DD5992F362B3C4LL, m_path,
                         path, 4);
      HASH_RETURN_STRING(0x48E8F48146EEEF5CLL, m_handle,
                         handle, 6);
      break;
    default:
      break;
  }
  return c_ObjectData::o_lval(s, hash);
}
Variant c_directory::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(directory)
ObjectData *c_directory::create(Variant v_path) {
  init();
  t___construct(v_path);
  return this;
}
ObjectData *c_directory::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0)));
  } else return this;
}
void c_directory::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0)));
}
ObjectData *c_directory::cloneImpl() {
  c_directory *obj = NEW(c_directory)();
  cloneSet(obj);
  return obj;
}
void c_directory::cloneSet(c_directory *clone) {
  clone->m_path = m_path.isReferenced() ? ref(m_path) : m_path;
  clone->m_handle = m_handle.isReferenced() ? ref(m_handle) : m_handle;
  ObjectData::cloneSet(clone);
}
Variant c_directory::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 7) {
    case 1:
      HASH_GUARD(0x78AE97BFBEBF5341LL, close) {
        return (t_close());
      }
      HASH_GUARD(0x1F479267E49EF301LL, read) {
        return (t_read());
      }
      break;
    case 2:
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        return (t_rewind());
      }
      break;
    case 7:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0)), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_directory::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 7) {
    case 1:
      HASH_GUARD(0x78AE97BFBEBF5341LL, close) {
        return (t_close());
      }
      HASH_GUARD(0x1F479267E49EF301LL, read) {
        return (t_read());
      }
      break;
    case 2:
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        return (t_rewind());
      }
      break;
    case 7:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_directory::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_directory::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 7) {
    case 1:
      HASH_GUARD(0x78AE97BFBEBF5341LL, close) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_close());
      }
      HASH_GUARD(0x1F479267E49EF301LL, read) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_read());
      }
      break;
    case 2:
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_rewind());
      }
      break;
    case 7:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___construct(a0), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_directory::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_directory$os_get(const char *s) {
  return c_directory::os_get(s, -1);
}
Variant &cw_directory$os_lval(const char *s) {
  return c_directory::os_lval(s, -1);
}
Variant cw_directory$os_constant(const char *s) {
  return c_directory::os_constant(s);
}
Variant cw_directory$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_directory::os_invoke(c, s, params, -1, fatal);
}
void c_directory::init() {
  m_path = null;
  m_handle = null;
}
/* SRC: classes/directory.php line 7 */
void c_directory::t___construct(Variant v_path) {
  INSTANCE_METHOD_INJECTION(Directory, Directory::__construct);
  bool oldInCtor = gasInCtor(true);
  m_path = v_path;
  m_handle = x_opendir(toString(v_path));
  gasInCtor(oldInCtor);
} /* function */
/* SRC: classes/directory.php line 12 */
Variant c_directory::t_read() {
  INSTANCE_METHOD_INJECTION(Directory, Directory::read);
  return x_readdir(toObject(m_handle));
} /* function */
/* SRC: classes/directory.php line 16 */
Variant c_directory::t_rewind() {
  INSTANCE_METHOD_INJECTION(Directory, Directory::rewind);
  return (x_rewinddir(toObject(m_handle)), null);
} /* function */
/* SRC: classes/directory.php line 20 */
Variant c_directory::t_close() {
  INSTANCE_METHOD_INJECTION(Directory, Directory::close);
  return (x_closedir(toObject(m_handle)), null);
} /* function */
Object co_directory(CArrRef params, bool init /* = true */) {
  return Object(p_directory(NEW(c_directory)())->dynCreate(params, init));
}
Variant pm_php$classes$directory_php(bool incOnce /* = false */, LVariableTable* variables /* = NULL */) {
  FUNCTION_INJECTION(run_init::classes/directory.php);
  {
    DECLARE_SYSTEM_GLOBALS(g);
    bool &alreadyRun = g->run_pm_php$classes$directory_php;
    if (alreadyRun) { if (incOnce) return true;}
    else alreadyRun = true;
    if (!variables) variables = g;
  }
  DECLARE_SYSTEM_GLOBALS(g);
  LVariableTable *gVariables __attribute__((__unused__)) = get_variable_table();
  return true;
} /* function */

///////////////////////////////////////////////////////////////////////////////
}

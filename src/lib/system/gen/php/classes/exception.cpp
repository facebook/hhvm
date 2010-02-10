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

#include <php/classes/exception.h>
#include <cpp/ext/ext.h>
#include <cpp/eval/eval.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* preface starts */
/* preface finishes */
/* SRC: classes/exception.php line 79 */
Variant c_unexpectedvalueexception::os_get(const char *s, int64 hash) {
  return c_runtimeexception::os_get(s, hash);
}
Variant &c_unexpectedvalueexception::os_lval(const char *s, int64 hash) {
  return c_runtimeexception::os_lval(s, hash);
}
void c_unexpectedvalueexception::o_get(ArrayElementVec &props) const {
  c_runtimeexception::o_get(props);
}
bool c_unexpectedvalueexception::o_exists(CStrRef s, int64 hash) const {
  return c_runtimeexception::o_exists(s, hash);
}
Variant c_unexpectedvalueexception::o_get(CStrRef s, int64 hash) {
  return c_runtimeexception::o_get(s, hash);
}
Variant c_unexpectedvalueexception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_runtimeexception::o_set(s, hash, v, forInit);
}
Variant &c_unexpectedvalueexception::o_lval(CStrRef s, int64 hash) {
  return c_runtimeexception::o_lval(s, hash);
}
Variant c_unexpectedvalueexception::os_constant(const char *s) {
  return c_runtimeexception::os_constant(s);
}
IMPLEMENT_CLASS(unexpectedvalueexception)
ObjectData *c_unexpectedvalueexception::cloneImpl() {
  c_unexpectedvalueexception *obj = NEW(c_unexpectedvalueexception)();
  cloneSet(obj);
  return obj;
}
void c_unexpectedvalueexception::cloneSet(c_unexpectedvalueexception *clone) {
  c_runtimeexception::cloneSet(clone);
}
Variant c_unexpectedvalueexception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_runtimeexception::o_invoke(s, params, hash, fatal);
}
Variant c_unexpectedvalueexception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_runtimeexception::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_unexpectedvalueexception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_runtimeexception::os_invoke(c, s, params, hash, fatal);
}
Variant c_unexpectedvalueexception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_runtimeexception::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_unexpectedvalueexception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_runtimeexception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_unexpectedvalueexception$os_get(const char *s) {
  return c_unexpectedvalueexception::os_get(s, -1);
}
Variant &cw_unexpectedvalueexception$os_lval(const char *s) {
  return c_unexpectedvalueexception::os_lval(s, -1);
}
Variant cw_unexpectedvalueexception$os_constant(const char *s) {
  return c_unexpectedvalueexception::os_constant(s);
}
Variant cw_unexpectedvalueexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_unexpectedvalueexception::os_invoke(c, s, params, -1, fatal);
}
void c_unexpectedvalueexception::init() {
  c_runtimeexception::init();
}
/* SRC: classes/exception.php line 76 */
Variant c_overflowexception::os_get(const char *s, int64 hash) {
  return c_runtimeexception::os_get(s, hash);
}
Variant &c_overflowexception::os_lval(const char *s, int64 hash) {
  return c_runtimeexception::os_lval(s, hash);
}
void c_overflowexception::o_get(ArrayElementVec &props) const {
  c_runtimeexception::o_get(props);
}
bool c_overflowexception::o_exists(CStrRef s, int64 hash) const {
  return c_runtimeexception::o_exists(s, hash);
}
Variant c_overflowexception::o_get(CStrRef s, int64 hash) {
  return c_runtimeexception::o_get(s, hash);
}
Variant c_overflowexception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_runtimeexception::o_set(s, hash, v, forInit);
}
Variant &c_overflowexception::o_lval(CStrRef s, int64 hash) {
  return c_runtimeexception::o_lval(s, hash);
}
Variant c_overflowexception::os_constant(const char *s) {
  return c_runtimeexception::os_constant(s);
}
IMPLEMENT_CLASS(overflowexception)
ObjectData *c_overflowexception::cloneImpl() {
  c_overflowexception *obj = NEW(c_overflowexception)();
  cloneSet(obj);
  return obj;
}
void c_overflowexception::cloneSet(c_overflowexception *clone) {
  c_runtimeexception::cloneSet(clone);
}
Variant c_overflowexception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_runtimeexception::o_invoke(s, params, hash, fatal);
}
Variant c_overflowexception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_runtimeexception::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_overflowexception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_runtimeexception::os_invoke(c, s, params, hash, fatal);
}
Variant c_overflowexception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_runtimeexception::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_overflowexception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_runtimeexception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_overflowexception$os_get(const char *s) {
  return c_overflowexception::os_get(s, -1);
}
Variant &cw_overflowexception$os_lval(const char *s) {
  return c_overflowexception::os_lval(s, -1);
}
Variant cw_overflowexception$os_constant(const char *s) {
  return c_overflowexception::os_constant(s);
}
Variant cw_overflowexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_overflowexception::os_invoke(c, s, params, -1, fatal);
}
void c_overflowexception::init() {
  c_runtimeexception::init();
}
/* SRC: classes/exception.php line 75 */
Variant c_outofboundsexception::os_get(const char *s, int64 hash) {
  return c_runtimeexception::os_get(s, hash);
}
Variant &c_outofboundsexception::os_lval(const char *s, int64 hash) {
  return c_runtimeexception::os_lval(s, hash);
}
void c_outofboundsexception::o_get(ArrayElementVec &props) const {
  c_runtimeexception::o_get(props);
}
bool c_outofboundsexception::o_exists(CStrRef s, int64 hash) const {
  return c_runtimeexception::o_exists(s, hash);
}
Variant c_outofboundsexception::o_get(CStrRef s, int64 hash) {
  return c_runtimeexception::o_get(s, hash);
}
Variant c_outofboundsexception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_runtimeexception::o_set(s, hash, v, forInit);
}
Variant &c_outofboundsexception::o_lval(CStrRef s, int64 hash) {
  return c_runtimeexception::o_lval(s, hash);
}
Variant c_outofboundsexception::os_constant(const char *s) {
  return c_runtimeexception::os_constant(s);
}
IMPLEMENT_CLASS(outofboundsexception)
ObjectData *c_outofboundsexception::cloneImpl() {
  c_outofboundsexception *obj = NEW(c_outofboundsexception)();
  cloneSet(obj);
  return obj;
}
void c_outofboundsexception::cloneSet(c_outofboundsexception *clone) {
  c_runtimeexception::cloneSet(clone);
}
Variant c_outofboundsexception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_runtimeexception::o_invoke(s, params, hash, fatal);
}
Variant c_outofboundsexception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_runtimeexception::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_outofboundsexception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_runtimeexception::os_invoke(c, s, params, hash, fatal);
}
Variant c_outofboundsexception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_runtimeexception::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_outofboundsexception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_runtimeexception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_outofboundsexception$os_get(const char *s) {
  return c_outofboundsexception::os_get(s, -1);
}
Variant &cw_outofboundsexception$os_lval(const char *s) {
  return c_outofboundsexception::os_lval(s, -1);
}
Variant cw_outofboundsexception$os_constant(const char *s) {
  return c_outofboundsexception::os_constant(s);
}
Variant cw_outofboundsexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_outofboundsexception::os_invoke(c, s, params, -1, fatal);
}
void c_outofboundsexception::init() {
  c_runtimeexception::init();
}
/* SRC: classes/exception.php line 67 */
Variant c_logicexception::os_get(const char *s, int64 hash) {
  return c_exception::os_get(s, hash);
}
Variant &c_logicexception::os_lval(const char *s, int64 hash) {
  return c_exception::os_lval(s, hash);
}
void c_logicexception::o_get(ArrayElementVec &props) const {
  c_exception::o_get(props);
}
bool c_logicexception::o_exists(CStrRef s, int64 hash) const {
  return c_exception::o_exists(s, hash);
}
Variant c_logicexception::o_get(CStrRef s, int64 hash) {
  return c_exception::o_get(s, hash);
}
Variant c_logicexception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_exception::o_set(s, hash, v, forInit);
}
Variant &c_logicexception::o_lval(CStrRef s, int64 hash) {
  return c_exception::o_lval(s, hash);
}
Variant c_logicexception::os_constant(const char *s) {
  return c_exception::os_constant(s);
}
IMPLEMENT_CLASS(logicexception)
ObjectData *c_logicexception::cloneImpl() {
  c_logicexception *obj = NEW(c_logicexception)();
  cloneSet(obj);
  return obj;
}
void c_logicexception::cloneSet(c_logicexception *clone) {
  c_exception::cloneSet(clone);
}
Variant c_logicexception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_exception::o_invoke(s, params, hash, fatal);
}
Variant c_logicexception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_exception::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_logicexception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_exception::os_invoke(c, s, params, hash, fatal);
}
Variant c_logicexception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_exception::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_logicexception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_exception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_logicexception$os_get(const char *s) {
  return c_logicexception::os_get(s, -1);
}
Variant &cw_logicexception$os_lval(const char *s) {
  return c_logicexception::os_lval(s, -1);
}
Variant cw_logicexception$os_constant(const char *s) {
  return c_logicexception::os_constant(s);
}
Variant cw_logicexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_logicexception::os_invoke(c, s, params, -1, fatal);
}
void c_logicexception::init() {
  c_exception::init();
}
/* SRC: classes/exception.php line 77 */
Variant c_rangeexception::os_get(const char *s, int64 hash) {
  return c_runtimeexception::os_get(s, hash);
}
Variant &c_rangeexception::os_lval(const char *s, int64 hash) {
  return c_runtimeexception::os_lval(s, hash);
}
void c_rangeexception::o_get(ArrayElementVec &props) const {
  c_runtimeexception::o_get(props);
}
bool c_rangeexception::o_exists(CStrRef s, int64 hash) const {
  return c_runtimeexception::o_exists(s, hash);
}
Variant c_rangeexception::o_get(CStrRef s, int64 hash) {
  return c_runtimeexception::o_get(s, hash);
}
Variant c_rangeexception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_runtimeexception::o_set(s, hash, v, forInit);
}
Variant &c_rangeexception::o_lval(CStrRef s, int64 hash) {
  return c_runtimeexception::o_lval(s, hash);
}
Variant c_rangeexception::os_constant(const char *s) {
  return c_runtimeexception::os_constant(s);
}
IMPLEMENT_CLASS(rangeexception)
ObjectData *c_rangeexception::cloneImpl() {
  c_rangeexception *obj = NEW(c_rangeexception)();
  cloneSet(obj);
  return obj;
}
void c_rangeexception::cloneSet(c_rangeexception *clone) {
  c_runtimeexception::cloneSet(clone);
}
Variant c_rangeexception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_runtimeexception::o_invoke(s, params, hash, fatal);
}
Variant c_rangeexception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_runtimeexception::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_rangeexception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_runtimeexception::os_invoke(c, s, params, hash, fatal);
}
Variant c_rangeexception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_runtimeexception::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_rangeexception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_runtimeexception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_rangeexception$os_get(const char *s) {
  return c_rangeexception::os_get(s, -1);
}
Variant &cw_rangeexception$os_lval(const char *s) {
  return c_rangeexception::os_lval(s, -1);
}
Variant cw_rangeexception$os_constant(const char *s) {
  return c_rangeexception::os_constant(s);
}
Variant cw_rangeexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_rangeexception::os_invoke(c, s, params, -1, fatal);
}
void c_rangeexception::init() {
  c_runtimeexception::init();
}
/* SRC: classes/exception.php line 71 */
Variant c_invalidargumentexception::os_get(const char *s, int64 hash) {
  return c_logicexception::os_get(s, hash);
}
Variant &c_invalidargumentexception::os_lval(const char *s, int64 hash) {
  return c_logicexception::os_lval(s, hash);
}
void c_invalidargumentexception::o_get(ArrayElementVec &props) const {
  c_logicexception::o_get(props);
}
bool c_invalidargumentexception::o_exists(CStrRef s, int64 hash) const {
  return c_logicexception::o_exists(s, hash);
}
Variant c_invalidargumentexception::o_get(CStrRef s, int64 hash) {
  return c_logicexception::o_get(s, hash);
}
Variant c_invalidargumentexception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_logicexception::o_set(s, hash, v, forInit);
}
Variant &c_invalidargumentexception::o_lval(CStrRef s, int64 hash) {
  return c_logicexception::o_lval(s, hash);
}
Variant c_invalidargumentexception::os_constant(const char *s) {
  return c_logicexception::os_constant(s);
}
IMPLEMENT_CLASS(invalidargumentexception)
ObjectData *c_invalidargumentexception::cloneImpl() {
  c_invalidargumentexception *obj = NEW(c_invalidargumentexception)();
  cloneSet(obj);
  return obj;
}
void c_invalidargumentexception::cloneSet(c_invalidargumentexception *clone) {
  c_logicexception::cloneSet(clone);
}
Variant c_invalidargumentexception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_logicexception::o_invoke(s, params, hash, fatal);
}
Variant c_invalidargumentexception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_logicexception::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_invalidargumentexception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_logicexception::os_invoke(c, s, params, hash, fatal);
}
Variant c_invalidargumentexception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_logicexception::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_invalidargumentexception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_logicexception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_invalidargumentexception$os_get(const char *s) {
  return c_invalidargumentexception::os_get(s, -1);
}
Variant &cw_invalidargumentexception$os_lval(const char *s) {
  return c_invalidargumentexception::os_lval(s, -1);
}
Variant cw_invalidargumentexception$os_constant(const char *s) {
  return c_invalidargumentexception::os_constant(s);
}
Variant cw_invalidargumentexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_invalidargumentexception::os_invoke(c, s, params, -1, fatal);
}
void c_invalidargumentexception::init() {
  c_logicexception::init();
}
/* SRC: classes/exception.php line 78 */
Variant c_underflowexception::os_get(const char *s, int64 hash) {
  return c_runtimeexception::os_get(s, hash);
}
Variant &c_underflowexception::os_lval(const char *s, int64 hash) {
  return c_runtimeexception::os_lval(s, hash);
}
void c_underflowexception::o_get(ArrayElementVec &props) const {
  c_runtimeexception::o_get(props);
}
bool c_underflowexception::o_exists(CStrRef s, int64 hash) const {
  return c_runtimeexception::o_exists(s, hash);
}
Variant c_underflowexception::o_get(CStrRef s, int64 hash) {
  return c_runtimeexception::o_get(s, hash);
}
Variant c_underflowexception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_runtimeexception::o_set(s, hash, v, forInit);
}
Variant &c_underflowexception::o_lval(CStrRef s, int64 hash) {
  return c_runtimeexception::o_lval(s, hash);
}
Variant c_underflowexception::os_constant(const char *s) {
  return c_runtimeexception::os_constant(s);
}
IMPLEMENT_CLASS(underflowexception)
ObjectData *c_underflowexception::cloneImpl() {
  c_underflowexception *obj = NEW(c_underflowexception)();
  cloneSet(obj);
  return obj;
}
void c_underflowexception::cloneSet(c_underflowexception *clone) {
  c_runtimeexception::cloneSet(clone);
}
Variant c_underflowexception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_runtimeexception::o_invoke(s, params, hash, fatal);
}
Variant c_underflowexception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_runtimeexception::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_underflowexception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_runtimeexception::os_invoke(c, s, params, hash, fatal);
}
Variant c_underflowexception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_runtimeexception::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_underflowexception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_runtimeexception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_underflowexception$os_get(const char *s) {
  return c_underflowexception::os_get(s, -1);
}
Variant &cw_underflowexception$os_lval(const char *s) {
  return c_underflowexception::os_lval(s, -1);
}
Variant cw_underflowexception$os_constant(const char *s) {
  return c_underflowexception::os_constant(s);
}
Variant cw_underflowexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_underflowexception::os_invoke(c, s, params, -1, fatal);
}
void c_underflowexception::init() {
  c_runtimeexception::init();
}
/* SRC: classes/exception.php line 73 */
Variant c_outofrangeexception::os_get(const char *s, int64 hash) {
  return c_logicexception::os_get(s, hash);
}
Variant &c_outofrangeexception::os_lval(const char *s, int64 hash) {
  return c_logicexception::os_lval(s, hash);
}
void c_outofrangeexception::o_get(ArrayElementVec &props) const {
  c_logicexception::o_get(props);
}
bool c_outofrangeexception::o_exists(CStrRef s, int64 hash) const {
  return c_logicexception::o_exists(s, hash);
}
Variant c_outofrangeexception::o_get(CStrRef s, int64 hash) {
  return c_logicexception::o_get(s, hash);
}
Variant c_outofrangeexception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_logicexception::o_set(s, hash, v, forInit);
}
Variant &c_outofrangeexception::o_lval(CStrRef s, int64 hash) {
  return c_logicexception::o_lval(s, hash);
}
Variant c_outofrangeexception::os_constant(const char *s) {
  return c_logicexception::os_constant(s);
}
IMPLEMENT_CLASS(outofrangeexception)
ObjectData *c_outofrangeexception::cloneImpl() {
  c_outofrangeexception *obj = NEW(c_outofrangeexception)();
  cloneSet(obj);
  return obj;
}
void c_outofrangeexception::cloneSet(c_outofrangeexception *clone) {
  c_logicexception::cloneSet(clone);
}
Variant c_outofrangeexception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_logicexception::o_invoke(s, params, hash, fatal);
}
Variant c_outofrangeexception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_logicexception::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_outofrangeexception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_logicexception::os_invoke(c, s, params, hash, fatal);
}
Variant c_outofrangeexception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_logicexception::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_outofrangeexception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_logicexception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_outofrangeexception$os_get(const char *s) {
  return c_outofrangeexception::os_get(s, -1);
}
Variant &cw_outofrangeexception$os_lval(const char *s) {
  return c_outofrangeexception::os_lval(s, -1);
}
Variant cw_outofrangeexception$os_constant(const char *s) {
  return c_outofrangeexception::os_constant(s);
}
Variant cw_outofrangeexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_outofrangeexception::os_invoke(c, s, params, -1, fatal);
}
void c_outofrangeexception::init() {
  c_logicexception::init();
}
/* SRC: classes/exception.php line 69 */
Variant c_badmethodcallexception::os_get(const char *s, int64 hash) {
  return c_badfunctioncallexception::os_get(s, hash);
}
Variant &c_badmethodcallexception::os_lval(const char *s, int64 hash) {
  return c_badfunctioncallexception::os_lval(s, hash);
}
void c_badmethodcallexception::o_get(ArrayElementVec &props) const {
  c_badfunctioncallexception::o_get(props);
}
bool c_badmethodcallexception::o_exists(CStrRef s, int64 hash) const {
  return c_badfunctioncallexception::o_exists(s, hash);
}
Variant c_badmethodcallexception::o_get(CStrRef s, int64 hash) {
  return c_badfunctioncallexception::o_get(s, hash);
}
Variant c_badmethodcallexception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_badfunctioncallexception::o_set(s, hash, v, forInit);
}
Variant &c_badmethodcallexception::o_lval(CStrRef s, int64 hash) {
  return c_badfunctioncallexception::o_lval(s, hash);
}
Variant c_badmethodcallexception::os_constant(const char *s) {
  return c_badfunctioncallexception::os_constant(s);
}
IMPLEMENT_CLASS(badmethodcallexception)
ObjectData *c_badmethodcallexception::cloneImpl() {
  c_badmethodcallexception *obj = NEW(c_badmethodcallexception)();
  cloneSet(obj);
  return obj;
}
void c_badmethodcallexception::cloneSet(c_badmethodcallexception *clone) {
  c_badfunctioncallexception::cloneSet(clone);
}
Variant c_badmethodcallexception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_badfunctioncallexception::o_invoke(s, params, hash, fatal);
}
Variant c_badmethodcallexception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_badfunctioncallexception::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_badmethodcallexception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_badfunctioncallexception::os_invoke(c, s, params, hash, fatal);
}
Variant c_badmethodcallexception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_badfunctioncallexception::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_badmethodcallexception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_badfunctioncallexception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_badmethodcallexception$os_get(const char *s) {
  return c_badmethodcallexception::os_get(s, -1);
}
Variant &cw_badmethodcallexception$os_lval(const char *s) {
  return c_badmethodcallexception::os_lval(s, -1);
}
Variant cw_badmethodcallexception$os_constant(const char *s) {
  return c_badmethodcallexception::os_constant(s);
}
Variant cw_badmethodcallexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_badmethodcallexception::os_invoke(c, s, params, -1, fatal);
}
void c_badmethodcallexception::init() {
  c_badfunctioncallexception::init();
}
/* SRC: classes/exception.php line 74 */
Variant c_runtimeexception::os_get(const char *s, int64 hash) {
  return c_exception::os_get(s, hash);
}
Variant &c_runtimeexception::os_lval(const char *s, int64 hash) {
  return c_exception::os_lval(s, hash);
}
void c_runtimeexception::o_get(ArrayElementVec &props) const {
  c_exception::o_get(props);
}
bool c_runtimeexception::o_exists(CStrRef s, int64 hash) const {
  return c_exception::o_exists(s, hash);
}
Variant c_runtimeexception::o_get(CStrRef s, int64 hash) {
  return c_exception::o_get(s, hash);
}
Variant c_runtimeexception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_exception::o_set(s, hash, v, forInit);
}
Variant &c_runtimeexception::o_lval(CStrRef s, int64 hash) {
  return c_exception::o_lval(s, hash);
}
Variant c_runtimeexception::os_constant(const char *s) {
  return c_exception::os_constant(s);
}
IMPLEMENT_CLASS(runtimeexception)
ObjectData *c_runtimeexception::cloneImpl() {
  c_runtimeexception *obj = NEW(c_runtimeexception)();
  cloneSet(obj);
  return obj;
}
void c_runtimeexception::cloneSet(c_runtimeexception *clone) {
  c_exception::cloneSet(clone);
}
Variant c_runtimeexception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_exception::o_invoke(s, params, hash, fatal);
}
Variant c_runtimeexception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_exception::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_runtimeexception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_exception::os_invoke(c, s, params, hash, fatal);
}
Variant c_runtimeexception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_exception::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_runtimeexception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_exception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_runtimeexception$os_get(const char *s) {
  return c_runtimeexception::os_get(s, -1);
}
Variant &cw_runtimeexception$os_lval(const char *s) {
  return c_runtimeexception::os_lval(s, -1);
}
Variant cw_runtimeexception$os_constant(const char *s) {
  return c_runtimeexception::os_constant(s);
}
Variant cw_runtimeexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_runtimeexception::os_invoke(c, s, params, -1, fatal);
}
void c_runtimeexception::init() {
  c_exception::init();
}
/* SRC: classes/exception.php line 3 */
Variant c_exception::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_exception::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_exception::o_get(ArrayElementVec &props) const {
  props.push_back(NEW(ArrayElement)("message", m_message.isReferenced() ? ref(m_message) : m_message));
  props.push_back(NEW(ArrayElement)("code", m_code.isReferenced() ? ref(m_code) : m_code));
  props.push_back(NEW(ArrayElement)("file", m_file.isReferenced() ? ref(m_file) : m_file));
  props.push_back(NEW(ArrayElement)("line", m_line.isReferenced() ? ref(m_line) : m_line));
  c_ObjectData::o_get(props);
}
bool c_exception::o_exists(CStrRef s, int64 hash) const {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 7) {
    case 3:
      HASH_EXISTS_STRING(0x612E37678CE7DB5BLL, file, 4);
      break;
    case 4:
      HASH_EXISTS_STRING(0x5B2CD7DDAB7A1DECLL, code, 4);
      HASH_EXISTS_STRING(0x21093C71DDF8728CLL, line, 4);
      break;
    case 7:
      HASH_EXISTS_STRING(0x3EAA4B97155366DFLL, message, 7);
      break;
    default:
      break;
  }
  return c_ObjectData::o_exists(s, hash);
}
Variant c_exception::o_get(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 7) {
    case 3:
      HASH_RETURN_STRING(0x612E37678CE7DB5BLL, m_file,
                         file, 4);
      break;
    case 4:
      HASH_RETURN_STRING(0x5B2CD7DDAB7A1DECLL, m_code,
                         code, 4);
      HASH_RETURN_STRING(0x21093C71DDF8728CLL, m_line,
                         line, 4);
      break;
    case 7:
      HASH_RETURN_STRING(0x3EAA4B97155366DFLL, m_message,
                         message, 7);
      break;
    default:
      break;
  }
  return c_ObjectData::o_get(s, hash);
}
Variant c_exception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 7) {
    case 3:
      HASH_SET_STRING(0x612E37678CE7DB5BLL, m_file,
                      file, 4);
      break;
    case 4:
      HASH_SET_STRING(0x5B2CD7DDAB7A1DECLL, m_code,
                      code, 4);
      HASH_SET_STRING(0x21093C71DDF8728CLL, m_line,
                      line, 4);
      break;
    case 7:
      HASH_SET_STRING(0x3EAA4B97155366DFLL, m_message,
                      message, 7);
      break;
    default:
      break;
  }
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_exception::o_lval(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 7) {
    case 3:
      HASH_RETURN_STRING(0x612E37678CE7DB5BLL, m_file,
                         file, 4);
      break;
    case 4:
      HASH_RETURN_STRING(0x5B2CD7DDAB7A1DECLL, m_code,
                         code, 4);
      HASH_RETURN_STRING(0x21093C71DDF8728CLL, m_line,
                         line, 4);
      break;
    case 7:
      HASH_RETURN_STRING(0x3EAA4B97155366DFLL, m_message,
                         message, 7);
      break;
    default:
      break;
  }
  return c_ObjectData::o_lval(s, hash);
}
Variant c_exception::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(exception)
ObjectData *c_exception::create(Variant v_message /* = "" */, Variant v_code /* = 0LL */) {
  init();
  t___construct(v_message, v_code);
  return this;
}
ObjectData *c_exception::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    int count = params.size();
    if (count <= 0) return (create());
    if (count == 1) return (create(params.rvalAt(0)));
    return (create(params.rvalAt(0), params.rvalAt(1)));
  } else return this;
}
void c_exception::dynConstruct(CArrRef params) {
  int count = params.size();
  if (count <= 0) (t___construct());
  if (count == 1) (t___construct(params.rvalAt(0)));
  (t___construct(params.rvalAt(0), params.rvalAt(1)));
}
ObjectData *c_exception::cloneImpl() {
  c_exception *obj = NEW(c_exception)();
  cloneSet(obj);
  return obj;
}
void c_exception::cloneSet(c_exception *clone) {
  clone->m_message = m_message.isReferenced() ? ref(m_message) : m_message;
  clone->m_code = m_code.isReferenced() ? ref(m_code) : m_code;
  clone->m_file = m_file.isReferenced() ? ref(m_file) : m_file;
  clone->m_line = m_line.isReferenced() ? ref(m_line) : m_line;
  ObjectData::cloneSet(clone);
}
Variant c_exception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_exception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_exception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_exception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_exception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_exception$os_get(const char *s) {
  return c_exception::os_get(s, -1);
}
Variant &cw_exception$os_lval(const char *s) {
  return c_exception::os_lval(s, -1);
}
Variant cw_exception$os_constant(const char *s) {
  return c_exception::os_constant(s);
}
Variant cw_exception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_exception::os_invoke(c, s, params, -1, fatal);
}
void c_exception::init() {
  m_message = "Unknown exception";
  m_code = 0LL;
  m_file = null;
  m_line = null;
}
/* SRC: classes/exception.php line 9 */
void c_exception::t___construct(Variant v_message /* = "" */, Variant v_code /* = 0LL */) {
  INSTANCE_METHOD_INJECTION(Exception, Exception::__construct);
  bool oldInCtor = gasInCtor(true);
  Variant v_frame;

  m_message = v_message;
  m_code = v_code;
  o_lval("trace", 0x0253015494C9CE77LL) = x_debug_backtrace();
  v_frame = x_array_shift(ref(lval(o_lval("trace", 0x0253015494C9CE77LL))));
  m_file = v_frame.rvalAt("file", 0x612E37678CE7DB5BLL);
  m_line = v_frame.rvalAt("line", 0x21093C71DDF8728CLL);
  gasInCtor(oldInCtor);
} /* function */
/* SRC: classes/exception.php line 19 */
Variant c_exception::t_getmessage() {
  INSTANCE_METHOD_INJECTION(Exception, Exception::getMessage);
  return m_message;
} /* function */
/* SRC: classes/exception.php line 24 */
Variant c_exception::t_getcode() {
  INSTANCE_METHOD_INJECTION(Exception, Exception::getCode);
  return m_code;
} /* function */
/* SRC: classes/exception.php line 29 */
Variant c_exception::t_getfile() {
  INSTANCE_METHOD_INJECTION(Exception, Exception::getFile);
  return m_file;
} /* function */
/* SRC: classes/exception.php line 34 */
Variant c_exception::t_getline() {
  INSTANCE_METHOD_INJECTION(Exception, Exception::getLine);
  return m_line;
} /* function */
/* SRC: classes/exception.php line 39 */
Variant c_exception::t_gettrace() {
  INSTANCE_METHOD_INJECTION(Exception, Exception::getTrace);
  return o_get("trace", 0x0253015494C9CE77LL);
} /* function */
/* SRC: classes/exception.php line 44 */
String c_exception::t_gettraceasstring() {
  INSTANCE_METHOD_INJECTION(Exception, Exception::getTraceAsString);
  int64 v_i = 0;
  String v_s;
  Variant v_frame;

  v_i = 0LL;
  v_s = "";
  {
    LOOP_COUNTER(1);
    Variant map2 = t_gettrace();
    for (ArrayIterPtr iter3 = map2.begin("exception"); !iter3->end(); iter3->next()) {
      LOOP_COUNTER_CHECK(1);
      v_frame = iter3->second();
      {
        if (!(x_is_array(v_frame))) continue;
        concat_assign(v_s, concat_rev(concat6("(", toString(v_frame.rvalAt("line", 0x21093C71DDF8728CLL)), "): ", (toString(isset(v_frame, "class", 0x45397FE5C82DBD12LL) ? ((Variant)(concat(toString(v_frame.rvalAt("class", 0x45397FE5C82DBD12LL)), toString(v_frame.rvalAt("type", 0x508FC7C8724A760ALL))))) : ((Variant)("")))), toString(v_frame.rvalAt("function", 0x736D912A52403931LL)), "()\n"), concat4("#", toString(v_i), " ", toString(v_frame.rvalAt("file", 0x612E37678CE7DB5BLL)))));
        v_i++;
      }
    }
  }
  concat_assign(v_s, concat3("#", toString(v_i), " {main}"));
  return v_s;
} /* function */
/* SRC: classes/exception.php line 62 */
String c_exception::t___tostring() {
  INSTANCE_METHOD_INJECTION(Exception, Exception::__toString);
  return toString(t_getmessage());
} /* function */
/* SRC: classes/exception.php line 68 */
Variant c_badfunctioncallexception::os_get(const char *s, int64 hash) {
  return c_logicexception::os_get(s, hash);
}
Variant &c_badfunctioncallexception::os_lval(const char *s, int64 hash) {
  return c_logicexception::os_lval(s, hash);
}
void c_badfunctioncallexception::o_get(ArrayElementVec &props) const {
  c_logicexception::o_get(props);
}
bool c_badfunctioncallexception::o_exists(CStrRef s, int64 hash) const {
  return c_logicexception::o_exists(s, hash);
}
Variant c_badfunctioncallexception::o_get(CStrRef s, int64 hash) {
  return c_logicexception::o_get(s, hash);
}
Variant c_badfunctioncallexception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_logicexception::o_set(s, hash, v, forInit);
}
Variant &c_badfunctioncallexception::o_lval(CStrRef s, int64 hash) {
  return c_logicexception::o_lval(s, hash);
}
Variant c_badfunctioncallexception::os_constant(const char *s) {
  return c_logicexception::os_constant(s);
}
IMPLEMENT_CLASS(badfunctioncallexception)
ObjectData *c_badfunctioncallexception::cloneImpl() {
  c_badfunctioncallexception *obj = NEW(c_badfunctioncallexception)();
  cloneSet(obj);
  return obj;
}
void c_badfunctioncallexception::cloneSet(c_badfunctioncallexception *clone) {
  c_logicexception::cloneSet(clone);
}
Variant c_badfunctioncallexception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_logicexception::o_invoke(s, params, hash, fatal);
}
Variant c_badfunctioncallexception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_logicexception::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_badfunctioncallexception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_logicexception::os_invoke(c, s, params, hash, fatal);
}
Variant c_badfunctioncallexception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_logicexception::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_badfunctioncallexception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_logicexception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_badfunctioncallexception$os_get(const char *s) {
  return c_badfunctioncallexception::os_get(s, -1);
}
Variant &cw_badfunctioncallexception$os_lval(const char *s) {
  return c_badfunctioncallexception::os_lval(s, -1);
}
Variant cw_badfunctioncallexception$os_constant(const char *s) {
  return c_badfunctioncallexception::os_constant(s);
}
Variant cw_badfunctioncallexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_badfunctioncallexception::os_invoke(c, s, params, -1, fatal);
}
void c_badfunctioncallexception::init() {
  c_logicexception::init();
}
/* SRC: classes/exception.php line 72 */
Variant c_lengthexception::os_get(const char *s, int64 hash) {
  return c_logicexception::os_get(s, hash);
}
Variant &c_lengthexception::os_lval(const char *s, int64 hash) {
  return c_logicexception::os_lval(s, hash);
}
void c_lengthexception::o_get(ArrayElementVec &props) const {
  c_logicexception::o_get(props);
}
bool c_lengthexception::o_exists(CStrRef s, int64 hash) const {
  return c_logicexception::o_exists(s, hash);
}
Variant c_lengthexception::o_get(CStrRef s, int64 hash) {
  return c_logicexception::o_get(s, hash);
}
Variant c_lengthexception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_logicexception::o_set(s, hash, v, forInit);
}
Variant &c_lengthexception::o_lval(CStrRef s, int64 hash) {
  return c_logicexception::o_lval(s, hash);
}
Variant c_lengthexception::os_constant(const char *s) {
  return c_logicexception::os_constant(s);
}
IMPLEMENT_CLASS(lengthexception)
ObjectData *c_lengthexception::cloneImpl() {
  c_lengthexception *obj = NEW(c_lengthexception)();
  cloneSet(obj);
  return obj;
}
void c_lengthexception::cloneSet(c_lengthexception *clone) {
  c_logicexception::cloneSet(clone);
}
Variant c_lengthexception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_logicexception::o_invoke(s, params, hash, fatal);
}
Variant c_lengthexception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_logicexception::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_lengthexception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_logicexception::os_invoke(c, s, params, hash, fatal);
}
Variant c_lengthexception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_logicexception::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_lengthexception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_logicexception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_lengthexception$os_get(const char *s) {
  return c_lengthexception::os_get(s, -1);
}
Variant &cw_lengthexception$os_lval(const char *s) {
  return c_lengthexception::os_lval(s, -1);
}
Variant cw_lengthexception$os_constant(const char *s) {
  return c_lengthexception::os_constant(s);
}
Variant cw_lengthexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_lengthexception::os_invoke(c, s, params, -1, fatal);
}
void c_lengthexception::init() {
  c_logicexception::init();
}
/* SRC: classes/exception.php line 70 */
Variant c_domainexception::os_get(const char *s, int64 hash) {
  return c_logicexception::os_get(s, hash);
}
Variant &c_domainexception::os_lval(const char *s, int64 hash) {
  return c_logicexception::os_lval(s, hash);
}
void c_domainexception::o_get(ArrayElementVec &props) const {
  c_logicexception::o_get(props);
}
bool c_domainexception::o_exists(CStrRef s, int64 hash) const {
  return c_logicexception::o_exists(s, hash);
}
Variant c_domainexception::o_get(CStrRef s, int64 hash) {
  return c_logicexception::o_get(s, hash);
}
Variant c_domainexception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_logicexception::o_set(s, hash, v, forInit);
}
Variant &c_domainexception::o_lval(CStrRef s, int64 hash) {
  return c_logicexception::o_lval(s, hash);
}
Variant c_domainexception::os_constant(const char *s) {
  return c_logicexception::os_constant(s);
}
IMPLEMENT_CLASS(domainexception)
ObjectData *c_domainexception::cloneImpl() {
  c_domainexception *obj = NEW(c_domainexception)();
  cloneSet(obj);
  return obj;
}
void c_domainexception::cloneSet(c_domainexception *clone) {
  c_logicexception::cloneSet(clone);
}
Variant c_domainexception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_logicexception::o_invoke(s, params, hash, fatal);
}
Variant c_domainexception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_logicexception::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domainexception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_logicexception::os_invoke(c, s, params, hash, fatal);
}
Variant c_domainexception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmessage());
      }
      break;
    case 3:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___tostring());
      }
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettraceasstring());
      }
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettrace());
      }
      break;
    case 14:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfile());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getcode());
      }
      break;
    default:
      break;
  }
  return c_logicexception::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domainexception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_logicexception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domainexception$os_get(const char *s) {
  return c_domainexception::os_get(s, -1);
}
Variant &cw_domainexception$os_lval(const char *s) {
  return c_domainexception::os_lval(s, -1);
}
Variant cw_domainexception$os_constant(const char *s) {
  return c_domainexception::os_constant(s);
}
Variant cw_domainexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domainexception::os_invoke(c, s, params, -1, fatal);
}
void c_domainexception::init() {
  c_logicexception::init();
}
Object co_unexpectedvalueexception(CArrRef params, bool init /* = true */) {
  return Object(p_unexpectedvalueexception(NEW(c_unexpectedvalueexception)())->dynCreate(params, init));
}
Object co_overflowexception(CArrRef params, bool init /* = true */) {
  return Object(p_overflowexception(NEW(c_overflowexception)())->dynCreate(params, init));
}
Object co_outofboundsexception(CArrRef params, bool init /* = true */) {
  return Object(p_outofboundsexception(NEW(c_outofboundsexception)())->dynCreate(params, init));
}
Object co_logicexception(CArrRef params, bool init /* = true */) {
  return Object(p_logicexception(NEW(c_logicexception)())->dynCreate(params, init));
}
Object co_rangeexception(CArrRef params, bool init /* = true */) {
  return Object(p_rangeexception(NEW(c_rangeexception)())->dynCreate(params, init));
}
Object co_invalidargumentexception(CArrRef params, bool init /* = true */) {
  return Object(p_invalidargumentexception(NEW(c_invalidargumentexception)())->dynCreate(params, init));
}
Object co_underflowexception(CArrRef params, bool init /* = true */) {
  return Object(p_underflowexception(NEW(c_underflowexception)())->dynCreate(params, init));
}
Object co_outofrangeexception(CArrRef params, bool init /* = true */) {
  return Object(p_outofrangeexception(NEW(c_outofrangeexception)())->dynCreate(params, init));
}
Object co_badmethodcallexception(CArrRef params, bool init /* = true */) {
  return Object(p_badmethodcallexception(NEW(c_badmethodcallexception)())->dynCreate(params, init));
}
Object co_runtimeexception(CArrRef params, bool init /* = true */) {
  return Object(p_runtimeexception(NEW(c_runtimeexception)())->dynCreate(params, init));
}
Object co_exception(CArrRef params, bool init /* = true */) {
  return Object(p_exception(NEW(c_exception)())->dynCreate(params, init));
}
Object co_badfunctioncallexception(CArrRef params, bool init /* = true */) {
  return Object(p_badfunctioncallexception(NEW(c_badfunctioncallexception)())->dynCreate(params, init));
}
Object co_lengthexception(CArrRef params, bool init /* = true */) {
  return Object(p_lengthexception(NEW(c_lengthexception)())->dynCreate(params, init));
}
Object co_domainexception(CArrRef params, bool init /* = true */) {
  return Object(p_domainexception(NEW(c_domainexception)())->dynCreate(params, init));
}
Variant pm_php$classes$exception_php(bool incOnce /* = false */, LVariableTable* variables /* = NULL */) {
  FUNCTION_INJECTION(run_init::classes/exception.php);
  {
    DECLARE_SYSTEM_GLOBALS(g);
    bool &alreadyRun = g->run_pm_php$classes$exception_php;
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

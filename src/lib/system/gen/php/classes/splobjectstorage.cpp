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

#include <php/classes/iterator.h>
#include <php/classes/splobjectstorage.h>
#include <cpp/ext/ext.h>
#include <cpp/eval/eval.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* preface starts */
/* preface finishes */
/* SRC: classes/splobjectstorage.php line 3 */
Variant c_splobjectstorage::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_splobjectstorage::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_splobjectstorage::o_get(ArrayElementVec &props) const {
  props.push_back(NEW(ArrayElement)("storage", m_storage.isReferenced() ? ref(m_storage) : m_storage));
  props.push_back(NEW(ArrayElement)("index", m_index));
  c_ObjectData::o_get(props);
}
bool c_splobjectstorage::o_exists(CStrRef s, int64 hash) const {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 1:
      HASH_EXISTS_STRING(0x1EA489BB64FC2CB1LL, storage, 7);
      HASH_EXISTS_STRING(0x440D5888C0FF3081LL, index, 5);
      break;
    default:
      break;
  }
  return c_ObjectData::o_exists(s, hash);
}
Variant c_splobjectstorage::o_get(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 1:
      HASH_RETURN_STRING(0x1EA489BB64FC2CB1LL, m_storage,
                         storage, 7);
      HASH_RETURN_STRING(0x440D5888C0FF3081LL, m_index,
                         index, 5);
      break;
    default:
      break;
  }
  return c_ObjectData::o_get(s, hash);
}
Variant c_splobjectstorage::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 1:
      HASH_SET_STRING(0x1EA489BB64FC2CB1LL, m_storage,
                      storage, 7);
      HASH_SET_STRING(0x440D5888C0FF3081LL, m_index,
                      index, 5);
      break;
    default:
      break;
  }
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_splobjectstorage::o_lval(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 1) {
    case 1:
      HASH_RETURN_STRING(0x1EA489BB64FC2CB1LL, m_storage,
                         storage, 7);
      break;
    default:
      break;
  }
  return c_ObjectData::o_lval(s, hash);
}
Variant c_splobjectstorage::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(splobjectstorage)
ObjectData *c_splobjectstorage::cloneImpl() {
  c_splobjectstorage *obj = NEW(c_splobjectstorage)();
  cloneSet(obj);
  return obj;
}
void c_splobjectstorage::cloneSet(c_splobjectstorage *clone) {
  clone->m_storage = m_storage.isReferenced() ? ref(m_storage) : m_storage;
  clone->m_index = m_index;
  ObjectData::cloneSet(clone);
}
Variant c_splobjectstorage::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 4:
      HASH_GUARD(0x6413CB5154808C44LL, valid) {
        return (t_valid());
      }
      break;
    case 10:
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        return (t_rewind(), null);
      }
      break;
    case 12:
      HASH_GUARD(0x62DD82BFEB88A4ACLL, attach) {
        return (t_attach(params.rvalAt(0)), null);
      }
      break;
    case 16:
      HASH_GUARD(0x5CEFA5A265104D10LL, count) {
        return (t_count());
      }
      HASH_GUARD(0x61B94551FA22D290LL, contains) {
        return (t_contains(params.rvalAt(0)));
      }
      break;
    case 17:
      HASH_GUARD(0x56EDB60C824E8C51LL, key) {
        return (t_key());
      }
      break;
    case 21:
      HASH_GUARD(0x3C7D0AC0EBA9A695LL, detach) {
        return (t_detach(params.rvalAt(0)), null);
      }
      break;
    case 24:
      HASH_GUARD(0x3C6D50F3BB8102B8LL, next) {
        return (t_next(), null);
      }
      break;
    case 28:
      HASH_GUARD(0x5B3A4A72846B21DCLL, current) {
        return (t_current());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_splobjectstorage::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 4:
      HASH_GUARD(0x6413CB5154808C44LL, valid) {
        return (t_valid());
      }
      break;
    case 10:
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        return (t_rewind(), null);
      }
      break;
    case 12:
      HASH_GUARD(0x62DD82BFEB88A4ACLL, attach) {
        return (t_attach(a0), null);
      }
      break;
    case 16:
      HASH_GUARD(0x5CEFA5A265104D10LL, count) {
        return (t_count());
      }
      HASH_GUARD(0x61B94551FA22D290LL, contains) {
        return (t_contains(a0));
      }
      break;
    case 17:
      HASH_GUARD(0x56EDB60C824E8C51LL, key) {
        return (t_key());
      }
      break;
    case 21:
      HASH_GUARD(0x3C7D0AC0EBA9A695LL, detach) {
        return (t_detach(a0), null);
      }
      break;
    case 24:
      HASH_GUARD(0x3C6D50F3BB8102B8LL, next) {
        return (t_next(), null);
      }
      break;
    case 28:
      HASH_GUARD(0x5B3A4A72846B21DCLL, current) {
        return (t_current());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_splobjectstorage::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_splobjectstorage::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 4:
      HASH_GUARD(0x6413CB5154808C44LL, valid) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_valid());
      }
      break;
    case 10:
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_rewind(), null);
      }
      break;
    case 12:
      HASH_GUARD(0x62DD82BFEB88A4ACLL, attach) {
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
        return (t_attach(a0), null);
      }
      break;
    case 16:
      HASH_GUARD(0x5CEFA5A265104D10LL, count) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_count());
      }
      HASH_GUARD(0x61B94551FA22D290LL, contains) {
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
        return (t_contains(a0));
      }
      break;
    case 17:
      HASH_GUARD(0x56EDB60C824E8C51LL, key) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_key());
      }
      break;
    case 21:
      HASH_GUARD(0x3C7D0AC0EBA9A695LL, detach) {
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
        return (t_detach(a0), null);
      }
      break;
    case 24:
      HASH_GUARD(0x3C6D50F3BB8102B8LL, next) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_next(), null);
      }
      break;
    case 28:
      HASH_GUARD(0x5B3A4A72846B21DCLL, current) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_current());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_splobjectstorage::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_splobjectstorage$os_get(const char *s) {
  return c_splobjectstorage::os_get(s, -1);
}
Variant &cw_splobjectstorage$os_lval(const char *s) {
  return c_splobjectstorage::os_lval(s, -1);
}
Variant cw_splobjectstorage$os_constant(const char *s) {
  return c_splobjectstorage::os_constant(s);
}
Variant cw_splobjectstorage$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_splobjectstorage::os_invoke(c, s, params, -1, fatal);
}
void c_splobjectstorage::init() {
  m_storage = SystemScalarArrays::ssa_[0];
  m_index = 0LL;
}
/* SRC: classes/splobjectstorage.php line 7 */
void c_splobjectstorage::t_rewind() {
  INSTANCE_METHOD_INJECTION(SplObjectStorage, SplObjectStorage::rewind);
  x_rewind(toObject(m_storage));
} /* function */
/* SRC: classes/splobjectstorage.php line 11 */
bool c_splobjectstorage::t_valid() {
  INSTANCE_METHOD_INJECTION(SplObjectStorage, SplObjectStorage::valid);
  return !same(x_key(ref(lval(m_storage))), false);
} /* function */
/* SRC: classes/splobjectstorage.php line 15 */
int64 c_splobjectstorage::t_key() {
  INSTANCE_METHOD_INJECTION(SplObjectStorage, SplObjectStorage::key);
  return m_index;
} /* function */
/* SRC: classes/splobjectstorage.php line 19 */
Variant c_splobjectstorage::t_current() {
  INSTANCE_METHOD_INJECTION(SplObjectStorage, SplObjectStorage::current);
  return x_current(ref(lval(m_storage)));
} /* function */
/* SRC: classes/splobjectstorage.php line 23 */
void c_splobjectstorage::t_next() {
  INSTANCE_METHOD_INJECTION(SplObjectStorage, SplObjectStorage::next);
  x_next(ref(lval(m_storage)));
  m_index++;
} /* function */
/* SRC: classes/splobjectstorage.php line 28 */
int c_splobjectstorage::t_count() {
  INSTANCE_METHOD_INJECTION(SplObjectStorage, SplObjectStorage::count);
  return x_count(m_storage);
} /* function */
/* SRC: classes/splobjectstorage.php line 32 */
bool c_splobjectstorage::t_contains(CVarRef v_obj) {
  INSTANCE_METHOD_INJECTION(SplObjectStorage, SplObjectStorage::contains);
  Variant v_object;

  if (x_is_object(v_obj)) {
    {
      LOOP_COUNTER(1);
      Variant map2 = m_storage;
      for (ArrayIterPtr iter3 = map2.begin("splobjectstorage"); !iter3->end(); iter3->next()) {
        LOOP_COUNTER_CHECK(1);
        v_object = iter3->second();
        {
          if (same(v_object, v_obj)) {
            return true;
          }
        }
      }
    }
  }
  return false;
} /* function */
/* SRC: classes/splobjectstorage.php line 43 */
void c_splobjectstorage::t_attach(CVarRef v_obj) {
  INSTANCE_METHOD_INJECTION(SplObjectStorage, SplObjectStorage::attach);
  if (x_is_object(v_obj) && !(t_contains(v_obj))) {
    m_storage.append((v_obj));
  }
} /* function */
/* SRC: classes/splobjectstorage.php line 49 */
void c_splobjectstorage::t_detach(CVarRef v_obj) {
  INSTANCE_METHOD_INJECTION(SplObjectStorage, SplObjectStorage::detach);
  DECLARE_SYSTEM_GLOBALS(g);
  Primitive v_idx = 0;
  Variant v_object;

  if (x_is_object(v_obj)) {
    {
      LOOP_COUNTER(4);
      Variant map5 = m_storage;
      for (ArrayIterPtr iter6 = map5.begin("splobjectstorage"); !iter6->end(); iter6->next()) {
        LOOP_COUNTER_CHECK(4);
        v_object = iter6->second();
        v_idx = iter6->first();
        {
          if (same(v_object, v_obj)) {
            m_storage.weakRemove(v_idx);
            o_root_invoke_few_args("rewind", 0x1670096FDE27AF6ALL, 0);
            return;
          }
        }
      }
    }
  }
} /* function */
Object co_splobjectstorage(CArrRef params, bool init /* = true */) {
  return Object(p_splobjectstorage(NEW(c_splobjectstorage)())->dynCreate(params, init));
}
Variant pm_php$classes$splobjectstorage_php(bool incOnce /* = false */, LVariableTable* variables /* = NULL */) {
  FUNCTION_INJECTION(run_init::classes/splobjectstorage.php);
  {
    DECLARE_SYSTEM_GLOBALS(g);
    bool &alreadyRun = g->run_pm_php$classes$splobjectstorage_php;
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

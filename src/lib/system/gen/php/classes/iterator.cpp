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

#include <php/classes/arrayaccess.h>
#include <php/classes/iterator.h>
#include <php/globals/constants.h>
#include <cpp/ext/ext.h>
#include <cpp/eval/eval.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* preface starts */
/* preface finishes */
/* SRC: classes/iterator.php line 31 */
Variant c_arrayiterator::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_arrayiterator::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_arrayiterator::o_get(ArrayElementVec &props) const {
  props.push_back(NEW(ArrayElement)("arr", m_arr.isReferenced() ? ref(m_arr) : m_arr));
  props.push_back(NEW(ArrayElement)("flags", m_flags.isReferenced() ? ref(m_flags) : m_flags));
  c_ObjectData::o_get(props);
}
bool c_arrayiterator::o_exists(CStrRef s, int64 hash) const {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_EXISTS_STRING(0x1776D8467CB08D68LL, arr, 3);
      break;
    case 1:
      HASH_EXISTS_STRING(0x6AFDA85728FAE70DLL, flags, 5);
      break;
    default:
      break;
  }
  return c_ObjectData::o_exists(s, hash);
}
Variant c_arrayiterator::o_get(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_RETURN_STRING(0x1776D8467CB08D68LL, m_arr,
                         arr, 3);
      break;
    case 1:
      HASH_RETURN_STRING(0x6AFDA85728FAE70DLL, m_flags,
                         flags, 5);
      break;
    default:
      break;
  }
  return c_ObjectData::o_get(s, hash);
}
Variant c_arrayiterator::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_SET_STRING(0x1776D8467CB08D68LL, m_arr,
                      arr, 3);
      break;
    case 1:
      HASH_SET_STRING(0x6AFDA85728FAE70DLL, m_flags,
                      flags, 5);
      break;
    default:
      break;
  }
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_arrayiterator::o_lval(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_RETURN_STRING(0x1776D8467CB08D68LL, m_arr,
                         arr, 3);
      break;
    case 1:
      HASH_RETURN_STRING(0x6AFDA85728FAE70DLL, m_flags,
                         flags, 5);
      break;
    default:
      break;
  }
  return c_ObjectData::o_lval(s, hash);
}
Variant c_arrayiterator::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(arrayiterator)
ObjectData *c_arrayiterator::create(Variant v_array, Variant v_flags /* = 0LL (SORT_REGULAR) */) {
  init();
  t___construct(v_array, v_flags);
  return this;
}
ObjectData *c_arrayiterator::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    int count = params.size();
    if (count <= 1) return (create(params.rvalAt(0)));
    return (create(params.rvalAt(0), params.rvalAt(1)));
  } else return this;
}
void c_arrayiterator::dynConstruct(CArrRef params) {
  int count = params.size();
  if (count <= 1) (t___construct(params.rvalAt(0)));
  (t___construct(params.rvalAt(0), params.rvalAt(1)));
}
ObjectData *c_arrayiterator::cloneImpl() {
  c_arrayiterator *obj = NEW(c_arrayiterator)();
  cloneSet(obj);
  return obj;
}
void c_arrayiterator::cloneSet(c_arrayiterator *clone) {
  clone->m_arr = m_arr.isReferenced() ? ref(m_arr) : m_arr;
  clone->m_flags = m_flags.isReferenced() ? ref(m_flags) : m_flags;
  ObjectData::cloneSet(clone);
}
Variant c_arrayiterator::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 0:
      HASH_GUARD(0x3E6BCFB9742FC700LL, offsetexists) {
        return (t_offsetexists(params.rvalAt(0)));
      }
      break;
    case 2:
      HASH_GUARD(0x4DEE4A472DC69EC2LL, append) {
        return (t_append(params.rvalAt(0)), null);
      }
      break;
    case 4:
      HASH_GUARD(0x6413CB5154808C44LL, valid) {
        return (t_valid());
      }
      HASH_GUARD(0x4842AF70A71BE6C4LL, uksort) {
        return (t_uksort(params.rvalAt(0)));
      }
      break;
    case 6:
      HASH_GUARD(0x234F6A0A486E8646LL, natcasesort) {
        return (t_natcasesort());
      }
      break;
    case 10:
      HASH_GUARD(0x2FC3A6941D522E0ALL, setflags) {
        return (t_setflags(params.rvalAt(0)), null);
      }
      break;
    case 16:
      HASH_GUARD(0x5CEFA5A265104D10LL, count) {
        return (t_count());
      }
      break;
    case 17:
      HASH_GUARD(0x56EDB60C824E8C51LL, key) {
        return (t_key());
      }
      break;
    case 24:
      HASH_GUARD(0x61D11ECEF4404498LL, offsetget) {
        return (t_offsetget(params.rvalAt(0)));
      }
      HASH_GUARD(0x70448A629A74FB18LL, ksort) {
        return (t_ksort());
      }
      break;
    case 28:
      HASH_GUARD(0x5B3A4A72846B21DCLL, current) {
        return (t_current());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      break;
    case 32:
      HASH_GUARD(0x6FACBD7F02B6FD60LL, uasort) {
        return (t_uasort(params.rvalAt(0)));
      }
      break;
    case 33:
      HASH_GUARD(0x0E210679B2DFD461LL, getarraycopy) {
        return (t_getarraycopy());
      }
      HASH_GUARD(0x27E7DBA875AD17E1LL, getflags) {
        return (t_getflags());
      }
      break;
    case 39:
      HASH_GUARD(0x7EF5445C77054C67LL, seek) {
        return (t_seek(params.rvalAt(0)), null);
      }
      break;
    case 42:
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        return (t_rewind());
      }
      break;
    case 46:
      HASH_GUARD(0x790B7C44A3442BEELL, asort) {
        return (t_asort());
      }
      break;
    case 51:
      HASH_GUARD(0x7DB9D839ACE0DEB3LL, natsort) {
        return (t_natsort());
      }
      break;
    case 56:
      HASH_GUARD(0x3C6D50F3BB8102B8LL, next) {
        return (t_next());
      }
      HASH_GUARD(0x0957F693A48AF738LL, offsetset) {
        return (t_offsetset(params.rvalAt(0), params.rvalAt(1)), null);
      }
      break;
    case 58:
      HASH_GUARD(0x08329980E6369ABALL, offsetunset) {
        return (t_offsetunset(params.rvalAt(0)), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_arrayiterator::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 0:
      HASH_GUARD(0x3E6BCFB9742FC700LL, offsetexists) {
        return (t_offsetexists(a0));
      }
      break;
    case 2:
      HASH_GUARD(0x4DEE4A472DC69EC2LL, append) {
        return (t_append(a0), null);
      }
      break;
    case 4:
      HASH_GUARD(0x6413CB5154808C44LL, valid) {
        return (t_valid());
      }
      HASH_GUARD(0x4842AF70A71BE6C4LL, uksort) {
        return (t_uksort(a0));
      }
      break;
    case 6:
      HASH_GUARD(0x234F6A0A486E8646LL, natcasesort) {
        return (t_natcasesort());
      }
      break;
    case 10:
      HASH_GUARD(0x2FC3A6941D522E0ALL, setflags) {
        return (t_setflags(a0), null);
      }
      break;
    case 16:
      HASH_GUARD(0x5CEFA5A265104D10LL, count) {
        return (t_count());
      }
      break;
    case 17:
      HASH_GUARD(0x56EDB60C824E8C51LL, key) {
        return (t_key());
      }
      break;
    case 24:
      HASH_GUARD(0x61D11ECEF4404498LL, offsetget) {
        return (t_offsetget(a0));
      }
      HASH_GUARD(0x70448A629A74FB18LL, ksort) {
        return (t_ksort());
      }
      break;
    case 28:
      HASH_GUARD(0x5B3A4A72846B21DCLL, current) {
        return (t_current());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      break;
    case 32:
      HASH_GUARD(0x6FACBD7F02B6FD60LL, uasort) {
        return (t_uasort(a0));
      }
      break;
    case 33:
      HASH_GUARD(0x0E210679B2DFD461LL, getarraycopy) {
        return (t_getarraycopy());
      }
      HASH_GUARD(0x27E7DBA875AD17E1LL, getflags) {
        return (t_getflags());
      }
      break;
    case 39:
      HASH_GUARD(0x7EF5445C77054C67LL, seek) {
        return (t_seek(a0), null);
      }
      break;
    case 42:
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        return (t_rewind());
      }
      break;
    case 46:
      HASH_GUARD(0x790B7C44A3442BEELL, asort) {
        return (t_asort());
      }
      break;
    case 51:
      HASH_GUARD(0x7DB9D839ACE0DEB3LL, natsort) {
        return (t_natsort());
      }
      break;
    case 56:
      HASH_GUARD(0x3C6D50F3BB8102B8LL, next) {
        return (t_next());
      }
      HASH_GUARD(0x0957F693A48AF738LL, offsetset) {
        return (t_offsetset(a0, a1), null);
      }
      break;
    case 58:
      HASH_GUARD(0x08329980E6369ABALL, offsetunset) {
        return (t_offsetunset(a0), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_arrayiterator::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_arrayiterator::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 0:
      HASH_GUARD(0x3E6BCFB9742FC700LL, offsetexists) {
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
        return (t_offsetexists(a0));
      }
      break;
    case 2:
      HASH_GUARD(0x4DEE4A472DC69EC2LL, append) {
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
        return (t_append(a0), null);
      }
      break;
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
      HASH_GUARD(0x4842AF70A71BE6C4LL, uksort) {
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
        return (t_uksort(a0));
      }
      break;
    case 6:
      HASH_GUARD(0x234F6A0A486E8646LL, natcasesort) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_natcasesort());
      }
      break;
    case 10:
      HASH_GUARD(0x2FC3A6941D522E0ALL, setflags) {
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
        return (t_setflags(a0), null);
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
    case 24:
      HASH_GUARD(0x61D11ECEF4404498LL, offsetget) {
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
        return (t_offsetget(a0));
      }
      HASH_GUARD(0x70448A629A74FB18LL, ksort) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_ksort());
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
    case 31:
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
        if (count <= 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      break;
    case 32:
      HASH_GUARD(0x6FACBD7F02B6FD60LL, uasort) {
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
        return (t_uasort(a0));
      }
      break;
    case 33:
      HASH_GUARD(0x0E210679B2DFD461LL, getarraycopy) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getarraycopy());
      }
      HASH_GUARD(0x27E7DBA875AD17E1LL, getflags) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getflags());
      }
      break;
    case 39:
      HASH_GUARD(0x7EF5445C77054C67LL, seek) {
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
        return (t_seek(a0), null);
      }
      break;
    case 42:
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
    case 46:
      HASH_GUARD(0x790B7C44A3442BEELL, asort) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_asort());
      }
      break;
    case 51:
      HASH_GUARD(0x7DB9D839ACE0DEB3LL, natsort) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_natsort());
      }
      break;
    case 56:
      HASH_GUARD(0x3C6D50F3BB8102B8LL, next) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_next());
      }
      HASH_GUARD(0x0957F693A48AF738LL, offsetset) {
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
        return (t_offsetset(a0, a1), null);
      }
      break;
    case 58:
      HASH_GUARD(0x08329980E6369ABALL, offsetunset) {
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
        return (t_offsetunset(a0), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_arrayiterator::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_arrayiterator$os_get(const char *s) {
  return c_arrayiterator::os_get(s, -1);
}
Variant &cw_arrayiterator$os_lval(const char *s) {
  return c_arrayiterator::os_lval(s, -1);
}
Variant cw_arrayiterator$os_constant(const char *s) {
  return c_arrayiterator::os_constant(s);
}
Variant cw_arrayiterator$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_arrayiterator::os_invoke(c, s, params, -1, fatal);
}
void c_arrayiterator::init() {
}
/* SRC: classes/iterator.php line 36 */
void c_arrayiterator::t___construct(Variant v_array, Variant v_flags /* = 0LL (SORT_REGULAR) */) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::__construct);
  bool oldInCtor = gasInCtor(true);
  m_arr = v_array;
  m_flags = v_flags;
  gasInCtor(oldInCtor);
} /* function */
/* SRC: classes/iterator.php line 41 */
void c_arrayiterator::t_append(CVarRef v_value) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::append);
  m_arr.append((v_value));
} /* function */
/* SRC: classes/iterator.php line 45 */
bool c_arrayiterator::t_asort() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::asort);
  return x_asort(ref(lval(m_arr)), toInt32(m_flags));
} /* function */
/* SRC: classes/iterator.php line 49 */
int c_arrayiterator::t_count() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::count);
  return x_count(m_arr);
} /* function */
/* SRC: classes/iterator.php line 53 */
Variant c_arrayiterator::t_current() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::current);
  return x_current(ref(lval(m_arr)));
} /* function */
/* SRC: classes/iterator.php line 57 */
Variant c_arrayiterator::t_getarraycopy() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::getArrayCopy);
  return m_arr;
} /* function */
/* SRC: classes/iterator.php line 61 */
Variant c_arrayiterator::t_getflags() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::getFlags);
  return m_flags;
} /* function */
/* SRC: classes/iterator.php line 65 */
Variant c_arrayiterator::t_key() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::key);
  return x_key(ref(lval(m_arr)));
} /* function */
/* SRC: classes/iterator.php line 69 */
bool c_arrayiterator::t_ksort() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::ksort);
  return x_ksort(ref(lval(m_arr)), toInt32(m_flags));
} /* function */
/* SRC: classes/iterator.php line 73 */
Variant c_arrayiterator::t_natcasesort() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::natcasesort);
  return x_natcasesort(ref(lval(m_arr)));
} /* function */
/* SRC: classes/iterator.php line 77 */
Variant c_arrayiterator::t_natsort() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::natsort);
  return x_natsort(ref(lval(m_arr)));
} /* function */
/* SRC: classes/iterator.php line 81 */
Variant c_arrayiterator::t_next() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::next);
  return x_next(ref(lval(m_arr)));
} /* function */
/* SRC: classes/iterator.php line 85 */
bool c_arrayiterator::t_offsetexists(CVarRef v_index) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::offsetExists);
  return isset(m_arr, v_index);
} /* function */
/* SRC: classes/iterator.php line 89 */
Variant c_arrayiterator::t_offsetget(Variant v_index) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::offsetGet);
  return m_arr.rvalAt(v_index);
} /* function */
/* SRC: classes/iterator.php line 89 */
Variant &c_arrayiterator::___offsetget_lval(Variant v_index) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::offsetGet);
  Variant &v = get_system_globals()->__lvalProxy;
  v = t_offsetget(v_index);
  return v;
} /* function */
/* SRC: classes/iterator.php line 93 */
void c_arrayiterator::t_offsetset(CVarRef v_index, CVarRef v_newval) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::offsetSet);
  m_arr.set(v_index, (v_newval));
} /* function */
/* SRC: classes/iterator.php line 97 */
void c_arrayiterator::t_offsetunset(CVarRef v_index) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::offsetUnset);
  DECLARE_SYSTEM_GLOBALS(g);
  m_arr.weakRemove(v_index);
} /* function */
/* SRC: classes/iterator.php line 101 */
Variant c_arrayiterator::t_rewind() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::rewind);
  return x_reset(ref(lval(m_arr)));
} /* function */
/* SRC: classes/iterator.php line 105 */
void c_arrayiterator::t_seek(CVarRef v_position) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::seek);
  int64 v_i = 0;

  x_reset(ref(lval(m_arr)));
  {
    LOOP_COUNTER(1);
    for (v_i = 0LL; less(v_i, v_position); v_i++) {
      LOOP_COUNTER_CHECK(1);
      {
        if (!(toBoolean(x_next(ref(lval(m_arr)))))) {
          break;
        }
      }
    }
  }
} /* function */
/* SRC: classes/iterator.php line 114 */
void c_arrayiterator::t_setflags(CVarRef v_flags) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::setFlags);
  m_flags = v_flags;
} /* function */
/* SRC: classes/iterator.php line 118 */
bool c_arrayiterator::t_uasort(CVarRef v_cmp_function) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::uasort);
  return x_uasort(ref(lval(m_arr)), v_cmp_function);
} /* function */
/* SRC: classes/iterator.php line 122 */
bool c_arrayiterator::t_uksort(Variant v_cmp_function) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::uksort);
  return invoke_failed("uksort", Array(ArrayInit(1).set(0, ArrayElement(ref(v_cmp_function))).create()), 0x4842AF70A71BE6C4LL);
} /* function */
/* SRC: classes/iterator.php line 126 */
bool c_arrayiterator::t_valid() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::valid);
  return !same(x_current(ref(lval(m_arr))), false);
} /* function */
Object co_arrayiterator(CArrRef params, bool init /* = true */) {
  return Object(p_arrayiterator(NEW(c_arrayiterator)())->dynCreate(params, init));
}
Variant pm_php$classes$iterator_php(bool incOnce /* = false */, LVariableTable* variables /* = NULL */) {
  FUNCTION_INJECTION(run_init::classes/iterator.php);
  {
    DECLARE_SYSTEM_GLOBALS(g);
    bool &alreadyRun = g->run_pm_php$classes$iterator_php;
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

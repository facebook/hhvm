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
#include <php/classes/splfile.h>
#include <php/globals/constants.h>
#include <cpp/ext/ext.h>
#include <cpp/eval/eval.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* preface starts */
/* preface finishes */
/* SRC: classes/iterator.php line 70 */
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
ObjectData *c_arrayiterator::create(Variant v_array, Variant v_flags //  = 0LL /* SORT_REGULAR */
) {
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
  m_arr = null;
  m_flags = null;
}
/* SRC: classes/iterator.php line 75 */
void c_arrayiterator::t___construct(Variant v_array, Variant v_flags //  = 0LL /* SORT_REGULAR */
) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::__construct);
  bool oldInCtor = gasInCtor(true);
  m_arr = v_array;
  m_flags = v_flags;
  gasInCtor(oldInCtor);
} /* function */
/* SRC: classes/iterator.php line 80 */
void c_arrayiterator::t_append(CVarRef v_value) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::append);
  m_arr.append((v_value));
} /* function */
/* SRC: classes/iterator.php line 84 */
bool c_arrayiterator::t_asort() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::asort);
  return x_asort(ref(lval(m_arr)), toInt32(m_flags));
} /* function */
/* SRC: classes/iterator.php line 88 */
int c_arrayiterator::t_count() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::count);
  return x_count(m_arr);
} /* function */
/* SRC: classes/iterator.php line 92 */
Variant c_arrayiterator::t_current() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::current);
  return x_current(ref(lval(m_arr)));
} /* function */
/* SRC: classes/iterator.php line 96 */
Variant c_arrayiterator::t_getarraycopy() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::getArrayCopy);
  return m_arr;
} /* function */
/* SRC: classes/iterator.php line 100 */
Variant c_arrayiterator::t_getflags() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::getFlags);
  return m_flags;
} /* function */
/* SRC: classes/iterator.php line 104 */
Variant c_arrayiterator::t_key() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::key);
  return x_key(ref(lval(m_arr)));
} /* function */
/* SRC: classes/iterator.php line 108 */
bool c_arrayiterator::t_ksort() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::ksort);
  return x_ksort(ref(lval(m_arr)), toInt32(m_flags));
} /* function */
/* SRC: classes/iterator.php line 112 */
Variant c_arrayiterator::t_natcasesort() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::natcasesort);
  return x_natcasesort(ref(lval(m_arr)));
} /* function */
/* SRC: classes/iterator.php line 116 */
Variant c_arrayiterator::t_natsort() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::natsort);
  return x_natsort(ref(lval(m_arr)));
} /* function */
/* SRC: classes/iterator.php line 120 */
Variant c_arrayiterator::t_next() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::next);
  return x_next(ref(lval(m_arr)));
} /* function */
/* SRC: classes/iterator.php line 124 */
bool c_arrayiterator::t_offsetexists(CVarRef v_index) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::offsetExists);
  return isset(m_arr, v_index);
} /* function */
/* SRC: classes/iterator.php line 128 */
Variant c_arrayiterator::t_offsetget(Variant v_index) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::offsetGet);
  return m_arr.rvalAt(v_index);
} /* function */
/* SRC: classes/iterator.php line 128 */
Variant &c_arrayiterator::___offsetget_lval(Variant v_index) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::offsetGet);
  Variant &v = get_system_globals()->__lvalProxy;
  v = t_offsetget(v_index);
  return v;
} /* function */
/* SRC: classes/iterator.php line 132 */
void c_arrayiterator::t_offsetset(CVarRef v_index, CVarRef v_newval) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::offsetSet);
  m_arr.set(v_index, (v_newval));
} /* function */
/* SRC: classes/iterator.php line 136 */
void c_arrayiterator::t_offsetunset(CVarRef v_index) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::offsetUnset);
  DECLARE_SYSTEM_GLOBALS(g);
  m_arr.weakRemove(v_index);
} /* function */
/* SRC: classes/iterator.php line 140 */
Variant c_arrayiterator::t_rewind() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::rewind);
  return x_reset(ref(lval(m_arr)));
} /* function */
/* SRC: classes/iterator.php line 144 */
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
/* SRC: classes/iterator.php line 153 */
void c_arrayiterator::t_setflags(CVarRef v_flags) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::setFlags);
  m_flags = v_flags;
} /* function */
/* SRC: classes/iterator.php line 157 */
bool c_arrayiterator::t_uasort(CVarRef v_cmp_function) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::uasort);
  return x_uasort(ref(lval(m_arr)), v_cmp_function);
} /* function */
/* SRC: classes/iterator.php line 161 */
bool c_arrayiterator::t_uksort(Variant v_cmp_function) {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::uksort);
  return invoke_failed("uksort", Array(ArrayInit(1).set(0, ref(v_cmp_function)).create()), 0x4842AF70A71BE6C4LL);
} /* function */
/* SRC: classes/iterator.php line 165 */
bool c_arrayiterator::t_valid() {
  INSTANCE_METHOD_INJECTION(ArrayIterator, ArrayIterator::valid);
  return !same(x_current(ref(lval(m_arr))), false);
} /* function */
/* SRC: classes/iterator.php line 280 */
Variant c_appenditerator::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_appenditerator::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_appenditerator::o_get(ArrayElementVec &props) const {
  props.push_back(NEW(ArrayElement)("iterators", m_iterators.isReferenced() ? ref(m_iterators) : m_iterators));
  c_ObjectData::o_get(props);
}
bool c_appenditerator::o_exists(CStrRef s, int64 hash) const {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 1) {
    case 0:
      HASH_EXISTS_STRING(0x1F6E21DFD4AF8244LL, iterators, 9);
      break;
    default:
      break;
  }
  return c_ObjectData::o_exists(s, hash);
}
Variant c_appenditerator::o_get(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 1) {
    case 0:
      HASH_RETURN_STRING(0x1F6E21DFD4AF8244LL, m_iterators,
                         iterators, 9);
      break;
    default:
      break;
  }
  return c_ObjectData::o_get(s, hash);
}
Variant c_appenditerator::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 1) {
    case 0:
      HASH_SET_STRING(0x1F6E21DFD4AF8244LL, m_iterators,
                      iterators, 9);
      break;
    default:
      break;
  }
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_appenditerator::o_lval(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 1) {
    case 0:
      HASH_RETURN_STRING(0x1F6E21DFD4AF8244LL, m_iterators,
                         iterators, 9);
      break;
    default:
      break;
  }
  return c_ObjectData::o_lval(s, hash);
}
Variant c_appenditerator::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(appenditerator)
ObjectData *c_appenditerator::create() {
  init();
  t___construct();
  return this;
}
ObjectData *c_appenditerator::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create());
  } else return this;
}
void c_appenditerator::dynConstruct(CArrRef params) {
  (t___construct());
}
ObjectData *c_appenditerator::cloneImpl() {
  c_appenditerator *obj = NEW(c_appenditerator)();
  cloneSet(obj);
  return obj;
}
void c_appenditerator::cloneSet(c_appenditerator *clone) {
  clone->m_iterators = m_iterators.isReferenced() ? ref(m_iterators) : m_iterators;
  ObjectData::cloneSet(clone);
}
Variant c_appenditerator::doCall(Variant v_name, Variant v_arguments, bool fatal) {
  return t___call(v_name, v_arguments);
}
Variant c_appenditerator::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 2:
      HASH_GUARD(0x4DEE4A472DC69EC2LL, append) {
        return (t_append(params.rvalAt(0)), null);
      }
      break;
    case 4:
      HASH_GUARD(0x6413CB5154808C44LL, valid) {
        return (t_valid());
      }
      HASH_GUARD(0x3106F858B09C7424LL, getinneriterator) {
        return (t_getinneriterator());
      }
      break;
    case 10:
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        return (t_rewind(), null);
      }
      break;
    case 12:
      HASH_GUARD(0x5D73364F53CEEB6CLL, __call) {
        return (t___call(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 17:
      HASH_GUARD(0x56EDB60C824E8C51LL, key) {
        return (t_key());
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
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_appenditerator::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 2:
      HASH_GUARD(0x4DEE4A472DC69EC2LL, append) {
        return (t_append(a0), null);
      }
      break;
    case 4:
      HASH_GUARD(0x6413CB5154808C44LL, valid) {
        return (t_valid());
      }
      HASH_GUARD(0x3106F858B09C7424LL, getinneriterator) {
        return (t_getinneriterator());
      }
      break;
    case 10:
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        return (t_rewind(), null);
      }
      break;
    case 12:
      HASH_GUARD(0x5D73364F53CEEB6CLL, __call) {
        return (t___call(a0, a1));
      }
      break;
    case 17:
      HASH_GUARD(0x56EDB60C824E8C51LL, key) {
        return (t_key());
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
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_appenditerator::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_appenditerator::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
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
      HASH_GUARD(0x3106F858B09C7424LL, getinneriterator) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getinneriterator());
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
      HASH_GUARD(0x5D73364F53CEEB6CLL, __call) {
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
        return (t___call(a0, a1));
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
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___construct(), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_appenditerator::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_appenditerator$os_get(const char *s) {
  return c_appenditerator::os_get(s, -1);
}
Variant &cw_appenditerator$os_lval(const char *s) {
  return c_appenditerator::os_lval(s, -1);
}
Variant cw_appenditerator$os_constant(const char *s) {
  return c_appenditerator::os_constant(s);
}
Variant cw_appenditerator$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_appenditerator::os_invoke(c, s, params, -1, fatal);
}
void c_appenditerator::init() {
  m_iterators = null;
}
/* SRC: classes/iterator.php line 283 */
void c_appenditerator::t___construct() {
  INSTANCE_METHOD_INJECTION(AppendIterator, AppendIterator::__construct);
  bool oldInCtor = gasInCtor(true);
  m_iterators = ((Object)(create_object("arrayiterator", Array())));
  gasInCtor(oldInCtor);
} /* function */
/* SRC: classes/iterator.php line 287 */
void c_appenditerator::t_append(Variant v_it) {
  INSTANCE_METHOD_INJECTION(AppendIterator, AppendIterator::append);
  m_iterators.o_invoke_few_args("append", 0x4DEE4A472DC69EC2LL, 1, v_it);
} /* function */
/* SRC: classes/iterator.php line 291 */
Variant c_appenditerator::t_getinneriterator() {
  INSTANCE_METHOD_INJECTION(AppendIterator, AppendIterator::getInnerIterator);
  return m_iterators.o_invoke_few_args("current", 0x5B3A4A72846B21DCLL, 0);
} /* function */
/* SRC: classes/iterator.php line 295 */
void c_appenditerator::t_rewind() {
  INSTANCE_METHOD_INJECTION(AppendIterator, AppendIterator::rewind);
  Variant eo_0;
  m_iterators.o_invoke_few_args("rewind", 0x1670096FDE27AF6ALL, 0);
  if (toBoolean(m_iterators.o_invoke_few_args("valid", 0x6413CB5154808C44LL, 0))) {
    (assignCallTemp(eo_0, toObject(o_root_invoke_few_args("getInnerIterator", 0x3106F858B09C7424LL, 0))),eo_0.o_invoke_few_args("rewind", 0x1670096FDE27AF6ALL, 0));
  }
} /* function */
/* SRC: classes/iterator.php line 302 */
bool c_appenditerator::t_valid() {
  INSTANCE_METHOD_INJECTION(AppendIterator, AppendIterator::valid);
  Variant eo_0;
  return toBoolean(m_iterators.o_invoke_few_args("valid", 0x6413CB5154808C44LL, 0)) && toBoolean((assignCallTemp(eo_0, toObject(o_root_invoke_few_args("getInnerIterator", 0x3106F858B09C7424LL, 0))),eo_0.o_invoke_few_args("valid", 0x6413CB5154808C44LL, 0)));
} /* function */
/* SRC: classes/iterator.php line 306 */
Variant c_appenditerator::t_current() {
  INSTANCE_METHOD_INJECTION(AppendIterator, AppendIterator::current);
  Variant eo_0;
  return toBoolean(m_iterators.o_invoke_few_args("valid", 0x6413CB5154808C44LL, 0)) ? ((Variant)((assignCallTemp(eo_0, toObject(o_root_invoke_few_args("getInnerIterator", 0x3106F858B09C7424LL, 0))),eo_0.o_invoke_few_args("current", 0x5B3A4A72846B21DCLL, 0)))) : ((Variant)(null));
} /* function */
/* SRC: classes/iterator.php line 315 */
Variant c_appenditerator::t_key() {
  INSTANCE_METHOD_INJECTION(AppendIterator, AppendIterator::key);
  Variant eo_0;
  return toBoolean(m_iterators.o_invoke_few_args("valid", 0x6413CB5154808C44LL, 0)) ? ((Variant)((assignCallTemp(eo_0, toObject(o_root_invoke_few_args("getInnerIterator", 0x3106F858B09C7424LL, 0))),eo_0.o_invoke_few_args("key", 0x56EDB60C824E8C51LL, 0)))) : ((Variant)(null));
} /* function */
/* SRC: classes/iterator.php line 319 */
void c_appenditerator::t_next() {
  INSTANCE_METHOD_INJECTION(AppendIterator, AppendIterator::next);
  Variant eo_0;
  Variant eo_1;
  Variant eo_2;
  Variant eo_3;
  if (!(toBoolean(m_iterators.o_invoke_few_args("valid", 0x6413CB5154808C44LL, 0)))) {
    return;
  }
  (assignCallTemp(eo_0, toObject(o_root_invoke_few_args("getInnerIterator", 0x3106F858B09C7424LL, 0))),eo_0.o_invoke_few_args("next", 0x3C6D50F3BB8102B8LL, 0));
  if (toBoolean((assignCallTemp(eo_1, toObject(o_root_invoke_few_args("getInnerIterator", 0x3106F858B09C7424LL, 0))),eo_1.o_invoke_few_args("valid", 0x6413CB5154808C44LL, 0)))) {
    return;
  }
  m_iterators.o_invoke_few_args("next", 0x3C6D50F3BB8102B8LL, 0);
  LOOP_COUNTER(2);
  {
    while (toBoolean(m_iterators.o_invoke_few_args("valid", 0x6413CB5154808C44LL, 0))) {
      LOOP_COUNTER_CHECK(2);
      {
        (assignCallTemp(eo_2, toObject(o_root_invoke_few_args("getInnerIterator", 0x3106F858B09C7424LL, 0))),eo_2.o_invoke_few_args("rewind", 0x1670096FDE27AF6ALL, 0));
        if (toBoolean((assignCallTemp(eo_3, toObject(o_root_invoke_few_args("getInnerIterator", 0x3106F858B09C7424LL, 0))),eo_3.o_invoke_few_args("valid", 0x6413CB5154808C44LL, 0)))) {
          return;
        }
        m_iterators.o_invoke_few_args("next", 0x3C6D50F3BB8102B8LL, 0);
      }
    }
  }
} /* function */
/* SRC: classes/iterator.php line 337 */
Variant c_appenditerator::t___call(Variant v_func, Variant v_params) {
  INSTANCE_METHOD_INJECTION(AppendIterator, AppendIterator::__call);
  INCALL_HELPER(v_func);
  Variant eo_0;
  Variant eo_1;
  return x_call_user_func_array((assignCallTemp(eo_0, o_root_invoke_few_args("getInnerIterator", 0x3106F858B09C7424LL, 0)),assignCallTemp(eo_1, v_func),Array(ArrayInit(2).set(0, eo_0).set(1, eo_1).create())), toArray(v_params));
} /* function */
/* SRC: classes/iterator.php line 209 */
const int64 q_recursivedirectoryiterator_CURRENT_AS_SELF = 0LL;
const int64 q_recursivedirectoryiterator_CURRENT_AS_FILEINFO = 16LL;
const int64 q_recursivedirectoryiterator_CURRENT_AS_PATHNAME = 32LL;
const int64 q_recursivedirectoryiterator_KEY_AS_PATHNAME = 0LL;
const int64 q_recursivedirectoryiterator_KEY_AS_FILENAME = 256LL;
const int64 q_recursivedirectoryiterator_NEW_CURRENT_AND_KEY = 272LL;
Variant c_recursivedirectoryiterator::os_get(const char *s, int64 hash) {
  return c_directoryiterator::os_get(s, hash);
}
Variant &c_recursivedirectoryiterator::os_lval(const char *s, int64 hash) {
  return c_directoryiterator::os_lval(s, hash);
}
void c_recursivedirectoryiterator::o_get(ArrayElementVec &props) const {
  c_directoryiterator::o_get(props);
}
bool c_recursivedirectoryiterator::o_exists(CStrRef s, int64 hash) const {
  return c_directoryiterator::o_exists(s, hash);
}
Variant c_recursivedirectoryiterator::o_get(CStrRef s, int64 hash) {
  return c_directoryiterator::o_get(s, hash);
}
Variant c_recursivedirectoryiterator::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_directoryiterator::o_set(s, hash, v, forInit);
}
Variant &c_recursivedirectoryiterator::o_lval(CStrRef s, int64 hash) {
  return c_directoryiterator::o_lval(s, hash);
}
Variant c_recursivedirectoryiterator::os_constant(const char *s) {
  int64 hash = hash_string(s);
  switch (hash & 15) {
    case 0:
      HASH_RETURN(0x4A34A9DA11ED8F50LL, q_recursivedirectoryiterator_KEY_AS_FILENAME, KEY_AS_FILENAME);
      break;
    case 1:
      HASH_RETURN(0x6AA4D24FB118FCF1LL, q_recursivedirectoryiterator_KEY_AS_PATHNAME, KEY_AS_PATHNAME);
      break;
    case 4:
      HASH_RETURN(0x0F0DCA1A52157D84LL, q_recursivedirectoryiterator_NEW_CURRENT_AND_KEY, NEW_CURRENT_AND_KEY);
      break;
    case 5:
      HASH_RETURN(0x29191B08277C8E85LL, q_recursivedirectoryiterator_CURRENT_AS_SELF, CURRENT_AS_SELF);
      break;
    case 6:
      HASH_RETURN(0x5C823ED8BD51E7F6LL, q_recursivedirectoryiterator_CURRENT_AS_FILEINFO, CURRENT_AS_FILEINFO);
      break;
    case 15:
      HASH_RETURN(0x2D581F4C45121E5FLL, q_recursivedirectoryiterator_CURRENT_AS_PATHNAME, CURRENT_AS_PATHNAME);
      break;
    default:
      break;
  }
  return c_directoryiterator::os_constant(s);
}
IMPLEMENT_CLASS(recursivedirectoryiterator)
ObjectData *c_recursivedirectoryiterator::create(Variant v_path, Variant v_flags //  = 16LL /* recursivedirectoryiterator::CURRENT_AS_FILEINFO */
) {
  init();
  t___construct(v_path, v_flags);
  return this;
}
ObjectData *c_recursivedirectoryiterator::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    int count = params.size();
    if (count <= 1) return (create(params.rvalAt(0)));
    return (create(params.rvalAt(0), params.rvalAt(1)));
  } else return this;
}
void c_recursivedirectoryiterator::dynConstruct(CArrRef params) {
  int count = params.size();
  if (count <= 1) (t___construct(params.rvalAt(0)));
  (t___construct(params.rvalAt(0), params.rvalAt(1)));
}
ObjectData *c_recursivedirectoryiterator::cloneImpl() {
  c_recursivedirectoryiterator *obj = NEW(c_recursivedirectoryiterator)();
  cloneSet(obj);
  return obj;
}
void c_recursivedirectoryiterator::cloneSet(c_recursivedirectoryiterator *clone) {
  c_directoryiterator::cloneSet(clone);
}
Variant c_recursivedirectoryiterator::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 6:
      HASH_GUARD(0x6B2EAD4A44934786LL, getrealpath) {
        return (t_getrealpath());
      }
      HASH_GUARD(0x1D3B08AA0AF50F06LL, gettype) {
        return (t_gettype());
      }
      break;
    case 8:
      HASH_GUARD(0x1ADA46FCC8EFEC08LL, isdir) {
        return (t_isdir());
      }
      break;
    case 9:
      HASH_GUARD(0x430BA7B88ED3A809LL, getsubpathname) {
        return (t_getsubpathname());
      }
      break;
    case 14:
      HASH_GUARD(0x32ABF385AD4BE48ELL, getowner) {
        return (t_getowner());
      }
      break;
    case 15:
      HASH_GUARD(0x732EC1BDA8EC520FLL, getchildren) {
        return (t_getchildren());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 21:
      HASH_GUARD(0x40044334DA397C15LL, haschildren) {
        return (t_haschildren());
      }
      break;
    case 28:
      HASH_GUARD(0x572E108C6731E29CLL, getbasename) {
        int count = params.size();
        if (count <= 0) return (t_getbasename());
        return (t_getbasename(params.rvalAt(0)));
      }
      break;
    case 29:
      HASH_GUARD(0x4C43532D60465F1DLL, isfile) {
        return (t_isfile());
      }
      break;
    case 31:
      HASH_GUARD(0x4BC19906B553C59FLL, getatime) {
        return (t_getatime());
      }
      break;
    case 37:
      HASH_GUARD(0x5948407CA9CC4DA5LL, setfileclass) {
        int count = params.size();
        if (count <= 0) return (t_setfileclass());
        return (t_setfileclass(params.rvalAt(0)));
      }
      break;
    case 43:
      HASH_GUARD(0x0D6276BAB75513ABLL, getlinktarget) {
        return (t_getlinktarget());
      }
      break;
    case 47:
      HASH_GUARD(0x5640A4755D0078AFLL, getctime) {
        return (t_getctime());
      }
      break;
    case 53:
      HASH_GUARD(0x337DEC2D48BDFE35LL, openfile) {
        int count = params.size();
        if (count <= 0) return (t_openfile());
        if (count == 1) return (t_openfile(params.rvalAt(0)));
        if (count == 2) return (t_openfile(params.rvalAt(0), params.rvalAt(1)));
        return (t_openfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 56:
      HASH_GUARD(0x3C6D50F3BB8102B8LL, next) {
        return (t_next());
      }
      break;
    case 63:
      HASH_GUARD(0x04C642C6C162243FLL, getpath) {
        return (t_getpath());
      }
      HASH_GUARD(0x7D50FA42F9D4923FLL, getfileinfo) {
        int count = params.size();
        if (count <= 0) return (t_getfileinfo());
        return (t_getfileinfo(params.rvalAt(0)));
      }
      break;
    case 68:
      HASH_GUARD(0x6413CB5154808C44LL, valid) {
        return (t_valid());
      }
      break;
    case 69:
      HASH_GUARD(0x5676046725D241C5LL, setinfoclass) {
        int count = params.size();
        if (count <= 0) return (t_setinfoclass());
        return (t_setinfoclass(params.rvalAt(0)));
      }
      break;
    case 74:
      HASH_GUARD(0x01A800A73CD2604ALL, getinode) {
        return (t_getinode());
      }
      break;
    case 77:
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        return (t_getfilename());
      }
      break;
    case 79:
      HASH_GUARD(0x569FC7D8E9401C4FLL, isreadable) {
        return (t_isreadable());
      }
      break;
    case 81:
      HASH_GUARD(0x56EDB60C824E8C51LL, key) {
        return (t_key());
      }
      break;
    case 88:
      HASH_GUARD(0x1D5801BB72C51C58LL, islink) {
        return (t_islink());
      }
      break;
    case 92:
      HASH_GUARD(0x5B3A4A72846B21DCLL, current) {
        return (t_current());
      }
      break;
    case 95:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      break;
    case 99:
      HASH_GUARD(0x638F2A56B8463A63LL, iswritable) {
        return (t_iswritable());
      }
      break;
    case 103:
      HASH_GUARD(0x00DCC39EDB16AFE7LL, getpathinfo) {
        int count = params.size();
        if (count <= 0) return (t_getpathinfo());
        return (t_getpathinfo(params.rvalAt(0)));
      }
      HASH_GUARD(0x7EF5445C77054C67LL, seek) {
        return (t_seek(params.rvalAt(0)));
      }
      break;
    case 106:
      HASH_GUARD(0x0F9EDEC32565D86ALL, getgroup) {
        return (t_getgroup());
      }
      HASH_GUARD(0x6615B5496D03A6EALL, getsize) {
        return (t_getsize());
      }
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        return (t_rewind());
      }
      break;
    case 116:
      HASH_GUARD(0x265BDC54C992EE74LL, getmtime) {
        return (t_getmtime());
      }
      HASH_GUARD(0x08D1EA51B78DA5F4LL, isdot) {
        return (t_isdot());
      }
      break;
    case 120:
      HASH_GUARD(0x25070641C3D924F8LL, getpathname) {
        return (t_getpathname());
      }
      break;
    case 122:
      HASH_GUARD(0x3786834B2A0CCB7ALL, isexecutable) {
        return (t_isexecutable());
      }
      break;
    case 123:
      HASH_GUARD(0x7CF26A0E76B5E27BLL, getsubpath) {
        return (t_getsubpath());
      }
      break;
    case 125:
      HASH_GUARD(0x4351578037A06E7DLL, getperms) {
        return (t_getperms());
      }
      break;
    default:
      break;
  }
  return c_directoryiterator::o_invoke(s, params, hash, fatal);
}
Variant c_recursivedirectoryiterator::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 6:
      HASH_GUARD(0x6B2EAD4A44934786LL, getrealpath) {
        return (t_getrealpath());
      }
      HASH_GUARD(0x1D3B08AA0AF50F06LL, gettype) {
        return (t_gettype());
      }
      break;
    case 8:
      HASH_GUARD(0x1ADA46FCC8EFEC08LL, isdir) {
        return (t_isdir());
      }
      break;
    case 9:
      HASH_GUARD(0x430BA7B88ED3A809LL, getsubpathname) {
        return (t_getsubpathname());
      }
      break;
    case 14:
      HASH_GUARD(0x32ABF385AD4BE48ELL, getowner) {
        return (t_getowner());
      }
      break;
    case 15:
      HASH_GUARD(0x732EC1BDA8EC520FLL, getchildren) {
        return (t_getchildren());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 21:
      HASH_GUARD(0x40044334DA397C15LL, haschildren) {
        return (t_haschildren());
      }
      break;
    case 28:
      HASH_GUARD(0x572E108C6731E29CLL, getbasename) {
        if (count <= 0) return (t_getbasename());
        return (t_getbasename(a0));
      }
      break;
    case 29:
      HASH_GUARD(0x4C43532D60465F1DLL, isfile) {
        return (t_isfile());
      }
      break;
    case 31:
      HASH_GUARD(0x4BC19906B553C59FLL, getatime) {
        return (t_getatime());
      }
      break;
    case 37:
      HASH_GUARD(0x5948407CA9CC4DA5LL, setfileclass) {
        if (count <= 0) return (t_setfileclass());
        return (t_setfileclass(a0));
      }
      break;
    case 43:
      HASH_GUARD(0x0D6276BAB75513ABLL, getlinktarget) {
        return (t_getlinktarget());
      }
      break;
    case 47:
      HASH_GUARD(0x5640A4755D0078AFLL, getctime) {
        return (t_getctime());
      }
      break;
    case 53:
      HASH_GUARD(0x337DEC2D48BDFE35LL, openfile) {
        if (count <= 0) return (t_openfile());
        if (count == 1) return (t_openfile(a0));
        if (count == 2) return (t_openfile(a0, a1));
        return (t_openfile(a0, a1, a2));
      }
      break;
    case 56:
      HASH_GUARD(0x3C6D50F3BB8102B8LL, next) {
        return (t_next());
      }
      break;
    case 63:
      HASH_GUARD(0x04C642C6C162243FLL, getpath) {
        return (t_getpath());
      }
      HASH_GUARD(0x7D50FA42F9D4923FLL, getfileinfo) {
        if (count <= 0) return (t_getfileinfo());
        return (t_getfileinfo(a0));
      }
      break;
    case 68:
      HASH_GUARD(0x6413CB5154808C44LL, valid) {
        return (t_valid());
      }
      break;
    case 69:
      HASH_GUARD(0x5676046725D241C5LL, setinfoclass) {
        if (count <= 0) return (t_setinfoclass());
        return (t_setinfoclass(a0));
      }
      break;
    case 74:
      HASH_GUARD(0x01A800A73CD2604ALL, getinode) {
        return (t_getinode());
      }
      break;
    case 77:
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        return (t_getfilename());
      }
      break;
    case 79:
      HASH_GUARD(0x569FC7D8E9401C4FLL, isreadable) {
        return (t_isreadable());
      }
      break;
    case 81:
      HASH_GUARD(0x56EDB60C824E8C51LL, key) {
        return (t_key());
      }
      break;
    case 88:
      HASH_GUARD(0x1D5801BB72C51C58LL, islink) {
        return (t_islink());
      }
      break;
    case 92:
      HASH_GUARD(0x5B3A4A72846B21DCLL, current) {
        return (t_current());
      }
      break;
    case 95:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      break;
    case 99:
      HASH_GUARD(0x638F2A56B8463A63LL, iswritable) {
        return (t_iswritable());
      }
      break;
    case 103:
      HASH_GUARD(0x00DCC39EDB16AFE7LL, getpathinfo) {
        if (count <= 0) return (t_getpathinfo());
        return (t_getpathinfo(a0));
      }
      HASH_GUARD(0x7EF5445C77054C67LL, seek) {
        return (t_seek(a0));
      }
      break;
    case 106:
      HASH_GUARD(0x0F9EDEC32565D86ALL, getgroup) {
        return (t_getgroup());
      }
      HASH_GUARD(0x6615B5496D03A6EALL, getsize) {
        return (t_getsize());
      }
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        return (t_rewind());
      }
      break;
    case 116:
      HASH_GUARD(0x265BDC54C992EE74LL, getmtime) {
        return (t_getmtime());
      }
      HASH_GUARD(0x08D1EA51B78DA5F4LL, isdot) {
        return (t_isdot());
      }
      break;
    case 120:
      HASH_GUARD(0x25070641C3D924F8LL, getpathname) {
        return (t_getpathname());
      }
      break;
    case 122:
      HASH_GUARD(0x3786834B2A0CCB7ALL, isexecutable) {
        return (t_isexecutable());
      }
      break;
    case 123:
      HASH_GUARD(0x7CF26A0E76B5E27BLL, getsubpath) {
        return (t_getsubpath());
      }
      break;
    case 125:
      HASH_GUARD(0x4351578037A06E7DLL, getperms) {
        return (t_getperms());
      }
      break;
    default:
      break;
  }
  return c_directoryiterator::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_recursivedirectoryiterator::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_directoryiterator::os_invoke(c, s, params, hash, fatal);
}
Variant c_recursivedirectoryiterator::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 6:
      HASH_GUARD(0x6B2EAD4A44934786LL, getrealpath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getrealpath());
      }
      HASH_GUARD(0x1D3B08AA0AF50F06LL, gettype) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettype());
      }
      break;
    case 8:
      HASH_GUARD(0x1ADA46FCC8EFEC08LL, isdir) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isdir());
      }
      break;
    case 9:
      HASH_GUARD(0x430BA7B88ED3A809LL, getsubpathname) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getsubpathname());
      }
      break;
    case 14:
      HASH_GUARD(0x32ABF385AD4BE48ELL, getowner) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getowner());
      }
      break;
    case 15:
      HASH_GUARD(0x732EC1BDA8EC520FLL, getchildren) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getchildren());
      }
      break;
    case 19:
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
      break;
    case 21:
      HASH_GUARD(0x40044334DA397C15LL, haschildren) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_haschildren());
      }
      break;
    case 28:
      HASH_GUARD(0x572E108C6731E29CLL, getbasename) {
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
        int count = params.size();
        if (count <= 0) return (t_getbasename());
        return (t_getbasename(a0));
      }
      break;
    case 29:
      HASH_GUARD(0x4C43532D60465F1DLL, isfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isfile());
      }
      break;
    case 31:
      HASH_GUARD(0x4BC19906B553C59FLL, getatime) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getatime());
      }
      break;
    case 37:
      HASH_GUARD(0x5948407CA9CC4DA5LL, setfileclass) {
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
        int count = params.size();
        if (count <= 0) return (t_setfileclass());
        return (t_setfileclass(a0));
      }
      break;
    case 43:
      HASH_GUARD(0x0D6276BAB75513ABLL, getlinktarget) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlinktarget());
      }
      break;
    case 47:
      HASH_GUARD(0x5640A4755D0078AFLL, getctime) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getctime());
      }
      break;
    case 53:
      HASH_GUARD(0x337DEC2D48BDFE35LL, openfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a2 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_openfile());
        if (count == 1) return (t_openfile(a0));
        if (count == 2) return (t_openfile(a0, a1));
        return (t_openfile(a0, a1, a2));
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
      break;
    case 63:
      HASH_GUARD(0x04C642C6C162243FLL, getpath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getpath());
      }
      HASH_GUARD(0x7D50FA42F9D4923FLL, getfileinfo) {
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
        int count = params.size();
        if (count <= 0) return (t_getfileinfo());
        return (t_getfileinfo(a0));
      }
      break;
    case 68:
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
    case 69:
      HASH_GUARD(0x5676046725D241C5LL, setinfoclass) {
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
        int count = params.size();
        if (count <= 0) return (t_setinfoclass());
        return (t_setinfoclass(a0));
      }
      break;
    case 74:
      HASH_GUARD(0x01A800A73CD2604ALL, getinode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getinode());
      }
      break;
    case 77:
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfilename());
      }
      break;
    case 79:
      HASH_GUARD(0x569FC7D8E9401C4FLL, isreadable) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isreadable());
      }
      break;
    case 81:
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
    case 88:
      HASH_GUARD(0x1D5801BB72C51C58LL, islink) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_islink());
      }
      break;
    case 92:
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
    case 95:
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
    case 99:
      HASH_GUARD(0x638F2A56B8463A63LL, iswritable) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_iswritable());
      }
      break;
    case 103:
      HASH_GUARD(0x00DCC39EDB16AFE7LL, getpathinfo) {
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
        int count = params.size();
        if (count <= 0) return (t_getpathinfo());
        return (t_getpathinfo(a0));
      }
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
        return (t_seek(a0));
      }
      break;
    case 106:
      HASH_GUARD(0x0F9EDEC32565D86ALL, getgroup) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getgroup());
      }
      HASH_GUARD(0x6615B5496D03A6EALL, getsize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getsize());
      }
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
    case 116:
      HASH_GUARD(0x265BDC54C992EE74LL, getmtime) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmtime());
      }
      HASH_GUARD(0x08D1EA51B78DA5F4LL, isdot) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isdot());
      }
      break;
    case 120:
      HASH_GUARD(0x25070641C3D924F8LL, getpathname) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getpathname());
      }
      break;
    case 122:
      HASH_GUARD(0x3786834B2A0CCB7ALL, isexecutable) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isexecutable());
      }
      break;
    case 123:
      HASH_GUARD(0x7CF26A0E76B5E27BLL, getsubpath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getsubpath());
      }
      break;
    case 125:
      HASH_GUARD(0x4351578037A06E7DLL, getperms) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getperms());
      }
      break;
    default:
      break;
  }
  return c_directoryiterator::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_recursivedirectoryiterator::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_directoryiterator::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_recursivedirectoryiterator$os_get(const char *s) {
  return c_recursivedirectoryiterator::os_get(s, -1);
}
Variant &cw_recursivedirectoryiterator$os_lval(const char *s) {
  return c_recursivedirectoryiterator::os_lval(s, -1);
}
Variant cw_recursivedirectoryiterator$os_constant(const char *s) {
  return c_recursivedirectoryiterator::os_constant(s);
}
Variant cw_recursivedirectoryiterator$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_recursivedirectoryiterator::os_invoke(c, s, params, -1, fatal);
}
void c_recursivedirectoryiterator::init() {
  c_directoryiterator::init();
}
/* SRC: classes/iterator.php line 218 */
void c_recursivedirectoryiterator::t___construct(Variant v_path, Variant v_flags //  = 16LL /* recursivedirectoryiterator::CURRENT_AS_FILEINFO */
) {
  INSTANCE_METHOD_INJECTION(RecursiveDirectoryIterator, RecursiveDirectoryIterator::__construct);
  bool oldInCtor = gasInCtor(true);
  x_hphp_recursivedirectoryiterator___construct(((Object)(this)), toString(v_path), toInt64(v_flags));
  gasInCtor(oldInCtor);
} /* function */
/* SRC: classes/iterator.php line 223 */
Variant c_recursivedirectoryiterator::t_current() {
  INSTANCE_METHOD_INJECTION(RecursiveDirectoryIterator, RecursiveDirectoryIterator::current);
  return x_hphp_recursivedirectoryiterator_current(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 227 */
Variant c_recursivedirectoryiterator::t_key() {
  INSTANCE_METHOD_INJECTION(RecursiveDirectoryIterator, RecursiveDirectoryIterator::key);
  return x_hphp_recursivedirectoryiterator_key(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 231 */
Variant c_recursivedirectoryiterator::t_next() {
  INSTANCE_METHOD_INJECTION(RecursiveDirectoryIterator, RecursiveDirectoryIterator::next);
  return (x_hphp_recursivedirectoryiterator_next(((Object)(this))), null);
} /* function */
/* SRC: classes/iterator.php line 235 */
Variant c_recursivedirectoryiterator::t_rewind() {
  INSTANCE_METHOD_INJECTION(RecursiveDirectoryIterator, RecursiveDirectoryIterator::rewind);
  return (x_hphp_recursivedirectoryiterator_rewind(((Object)(this))), null);
} /* function */
/* SRC: classes/iterator.php line 239 */
Variant c_recursivedirectoryiterator::t_seek(CVarRef v_position) {
  INSTANCE_METHOD_INJECTION(RecursiveDirectoryIterator, RecursiveDirectoryIterator::seek);
  return (invoke_failed("hphp_recursivedirectoryiterator_seek", Array(ArrayInit(1).set(0, ref(this)).create()), 0x09A98E99D51E8C2BLL), null);
} /* function */
/* SRC: classes/iterator.php line 243 */
String c_recursivedirectoryiterator::t___tostring() {
  INSTANCE_METHOD_INJECTION(RecursiveDirectoryIterator, RecursiveDirectoryIterator::__toString);
  return x_hphp_recursivedirectoryiterator___tostring(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 247 */
bool c_recursivedirectoryiterator::t_valid() {
  INSTANCE_METHOD_INJECTION(RecursiveDirectoryIterator, RecursiveDirectoryIterator::valid);
  return x_hphp_recursivedirectoryiterator_valid(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 251 */
bool c_recursivedirectoryiterator::t_haschildren() {
  INSTANCE_METHOD_INJECTION(RecursiveDirectoryIterator, RecursiveDirectoryIterator::hasChildren);
  return x_hphp_recursivedirectoryiterator_haschildren(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 255 */
Object c_recursivedirectoryiterator::t_getchildren() {
  INSTANCE_METHOD_INJECTION(RecursiveDirectoryIterator, RecursiveDirectoryIterator::getChildren);
  return x_hphp_recursivedirectoryiterator_getchildren(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 259 */
String c_recursivedirectoryiterator::t_getsubpath() {
  INSTANCE_METHOD_INJECTION(RecursiveDirectoryIterator, RecursiveDirectoryIterator::getSubPath);
  return x_hphp_recursivedirectoryiterator_getsubpath(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 263 */
String c_recursivedirectoryiterator::t_getsubpathname() {
  INSTANCE_METHOD_INJECTION(RecursiveDirectoryIterator, RecursiveDirectoryIterator::getSubPathname);
  return x_hphp_recursivedirectoryiterator_getsubpathname(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 170 */
Variant c_directoryiterator::os_get(const char *s, int64 hash) {
  return c_splfileinfo::os_get(s, hash);
}
Variant &c_directoryiterator::os_lval(const char *s, int64 hash) {
  return c_splfileinfo::os_lval(s, hash);
}
void c_directoryiterator::o_get(ArrayElementVec &props) const {
  c_splfileinfo::o_get(props);
}
bool c_directoryiterator::o_exists(CStrRef s, int64 hash) const {
  return c_splfileinfo::o_exists(s, hash);
}
Variant c_directoryiterator::o_get(CStrRef s, int64 hash) {
  return c_splfileinfo::o_get(s, hash);
}
Variant c_directoryiterator::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_splfileinfo::o_set(s, hash, v, forInit);
}
Variant &c_directoryiterator::o_lval(CStrRef s, int64 hash) {
  return c_splfileinfo::o_lval(s, hash);
}
Variant c_directoryiterator::os_constant(const char *s) {
  return c_splfileinfo::os_constant(s);
}
IMPLEMENT_CLASS(directoryiterator)
ObjectData *c_directoryiterator::create(Variant v_path) {
  init();
  t___construct(v_path);
  return this;
}
ObjectData *c_directoryiterator::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0)));
  } else return this;
}
void c_directoryiterator::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0)));
}
ObjectData *c_directoryiterator::cloneImpl() {
  c_directoryiterator *obj = NEW(c_directoryiterator)();
  cloneSet(obj);
  return obj;
}
void c_directoryiterator::cloneSet(c_directoryiterator *clone) {
  c_splfileinfo::cloneSet(clone);
}
Variant c_directoryiterator::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 6:
      HASH_GUARD(0x6B2EAD4A44934786LL, getrealpath) {
        return (t_getrealpath());
      }
      HASH_GUARD(0x1D3B08AA0AF50F06LL, gettype) {
        return (t_gettype());
      }
      break;
    case 8:
      HASH_GUARD(0x1ADA46FCC8EFEC08LL, isdir) {
        return (t_isdir());
      }
      break;
    case 14:
      HASH_GUARD(0x32ABF385AD4BE48ELL, getowner) {
        return (t_getowner());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 28:
      HASH_GUARD(0x572E108C6731E29CLL, getbasename) {
        int count = params.size();
        if (count <= 0) return (t_getbasename());
        return (t_getbasename(params.rvalAt(0)));
      }
      break;
    case 29:
      HASH_GUARD(0x4C43532D60465F1DLL, isfile) {
        return (t_isfile());
      }
      break;
    case 31:
      HASH_GUARD(0x4BC19906B553C59FLL, getatime) {
        return (t_getatime());
      }
      break;
    case 37:
      HASH_GUARD(0x5948407CA9CC4DA5LL, setfileclass) {
        int count = params.size();
        if (count <= 0) return (t_setfileclass());
        return (t_setfileclass(params.rvalAt(0)));
      }
      break;
    case 43:
      HASH_GUARD(0x0D6276BAB75513ABLL, getlinktarget) {
        return (t_getlinktarget());
      }
      break;
    case 47:
      HASH_GUARD(0x5640A4755D0078AFLL, getctime) {
        return (t_getctime());
      }
      break;
    case 53:
      HASH_GUARD(0x337DEC2D48BDFE35LL, openfile) {
        int count = params.size();
        if (count <= 0) return (t_openfile());
        if (count == 1) return (t_openfile(params.rvalAt(0)));
        if (count == 2) return (t_openfile(params.rvalAt(0), params.rvalAt(1)));
        return (t_openfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 56:
      HASH_GUARD(0x3C6D50F3BB8102B8LL, next) {
        return (t_next());
      }
      break;
    case 63:
      HASH_GUARD(0x04C642C6C162243FLL, getpath) {
        return (t_getpath());
      }
      HASH_GUARD(0x7D50FA42F9D4923FLL, getfileinfo) {
        int count = params.size();
        if (count <= 0) return (t_getfileinfo());
        return (t_getfileinfo(params.rvalAt(0)));
      }
      break;
    case 68:
      HASH_GUARD(0x6413CB5154808C44LL, valid) {
        return (t_valid());
      }
      break;
    case 69:
      HASH_GUARD(0x5676046725D241C5LL, setinfoclass) {
        int count = params.size();
        if (count <= 0) return (t_setinfoclass());
        return (t_setinfoclass(params.rvalAt(0)));
      }
      break;
    case 74:
      HASH_GUARD(0x01A800A73CD2604ALL, getinode) {
        return (t_getinode());
      }
      break;
    case 77:
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        return (t_getfilename());
      }
      break;
    case 79:
      HASH_GUARD(0x569FC7D8E9401C4FLL, isreadable) {
        return (t_isreadable());
      }
      break;
    case 81:
      HASH_GUARD(0x56EDB60C824E8C51LL, key) {
        return (t_key());
      }
      break;
    case 88:
      HASH_GUARD(0x1D5801BB72C51C58LL, islink) {
        return (t_islink());
      }
      break;
    case 92:
      HASH_GUARD(0x5B3A4A72846B21DCLL, current) {
        return (t_current());
      }
      break;
    case 95:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0)), null);
      }
      break;
    case 99:
      HASH_GUARD(0x638F2A56B8463A63LL, iswritable) {
        return (t_iswritable());
      }
      break;
    case 103:
      HASH_GUARD(0x00DCC39EDB16AFE7LL, getpathinfo) {
        int count = params.size();
        if (count <= 0) return (t_getpathinfo());
        return (t_getpathinfo(params.rvalAt(0)));
      }
      HASH_GUARD(0x7EF5445C77054C67LL, seek) {
        return (t_seek(params.rvalAt(0)));
      }
      break;
    case 106:
      HASH_GUARD(0x0F9EDEC32565D86ALL, getgroup) {
        return (t_getgroup());
      }
      HASH_GUARD(0x6615B5496D03A6EALL, getsize) {
        return (t_getsize());
      }
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        return (t_rewind());
      }
      break;
    case 116:
      HASH_GUARD(0x265BDC54C992EE74LL, getmtime) {
        return (t_getmtime());
      }
      HASH_GUARD(0x08D1EA51B78DA5F4LL, isdot) {
        return (t_isdot());
      }
      break;
    case 120:
      HASH_GUARD(0x25070641C3D924F8LL, getpathname) {
        return (t_getpathname());
      }
      break;
    case 122:
      HASH_GUARD(0x3786834B2A0CCB7ALL, isexecutable) {
        return (t_isexecutable());
      }
      break;
    case 125:
      HASH_GUARD(0x4351578037A06E7DLL, getperms) {
        return (t_getperms());
      }
      break;
    default:
      break;
  }
  return c_splfileinfo::o_invoke(s, params, hash, fatal);
}
Variant c_directoryiterator::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 6:
      HASH_GUARD(0x6B2EAD4A44934786LL, getrealpath) {
        return (t_getrealpath());
      }
      HASH_GUARD(0x1D3B08AA0AF50F06LL, gettype) {
        return (t_gettype());
      }
      break;
    case 8:
      HASH_GUARD(0x1ADA46FCC8EFEC08LL, isdir) {
        return (t_isdir());
      }
      break;
    case 14:
      HASH_GUARD(0x32ABF385AD4BE48ELL, getowner) {
        return (t_getowner());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 28:
      HASH_GUARD(0x572E108C6731E29CLL, getbasename) {
        if (count <= 0) return (t_getbasename());
        return (t_getbasename(a0));
      }
      break;
    case 29:
      HASH_GUARD(0x4C43532D60465F1DLL, isfile) {
        return (t_isfile());
      }
      break;
    case 31:
      HASH_GUARD(0x4BC19906B553C59FLL, getatime) {
        return (t_getatime());
      }
      break;
    case 37:
      HASH_GUARD(0x5948407CA9CC4DA5LL, setfileclass) {
        if (count <= 0) return (t_setfileclass());
        return (t_setfileclass(a0));
      }
      break;
    case 43:
      HASH_GUARD(0x0D6276BAB75513ABLL, getlinktarget) {
        return (t_getlinktarget());
      }
      break;
    case 47:
      HASH_GUARD(0x5640A4755D0078AFLL, getctime) {
        return (t_getctime());
      }
      break;
    case 53:
      HASH_GUARD(0x337DEC2D48BDFE35LL, openfile) {
        if (count <= 0) return (t_openfile());
        if (count == 1) return (t_openfile(a0));
        if (count == 2) return (t_openfile(a0, a1));
        return (t_openfile(a0, a1, a2));
      }
      break;
    case 56:
      HASH_GUARD(0x3C6D50F3BB8102B8LL, next) {
        return (t_next());
      }
      break;
    case 63:
      HASH_GUARD(0x04C642C6C162243FLL, getpath) {
        return (t_getpath());
      }
      HASH_GUARD(0x7D50FA42F9D4923FLL, getfileinfo) {
        if (count <= 0) return (t_getfileinfo());
        return (t_getfileinfo(a0));
      }
      break;
    case 68:
      HASH_GUARD(0x6413CB5154808C44LL, valid) {
        return (t_valid());
      }
      break;
    case 69:
      HASH_GUARD(0x5676046725D241C5LL, setinfoclass) {
        if (count <= 0) return (t_setinfoclass());
        return (t_setinfoclass(a0));
      }
      break;
    case 74:
      HASH_GUARD(0x01A800A73CD2604ALL, getinode) {
        return (t_getinode());
      }
      break;
    case 77:
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        return (t_getfilename());
      }
      break;
    case 79:
      HASH_GUARD(0x569FC7D8E9401C4FLL, isreadable) {
        return (t_isreadable());
      }
      break;
    case 81:
      HASH_GUARD(0x56EDB60C824E8C51LL, key) {
        return (t_key());
      }
      break;
    case 88:
      HASH_GUARD(0x1D5801BB72C51C58LL, islink) {
        return (t_islink());
      }
      break;
    case 92:
      HASH_GUARD(0x5B3A4A72846B21DCLL, current) {
        return (t_current());
      }
      break;
    case 95:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0), null);
      }
      break;
    case 99:
      HASH_GUARD(0x638F2A56B8463A63LL, iswritable) {
        return (t_iswritable());
      }
      break;
    case 103:
      HASH_GUARD(0x00DCC39EDB16AFE7LL, getpathinfo) {
        if (count <= 0) return (t_getpathinfo());
        return (t_getpathinfo(a0));
      }
      HASH_GUARD(0x7EF5445C77054C67LL, seek) {
        return (t_seek(a0));
      }
      break;
    case 106:
      HASH_GUARD(0x0F9EDEC32565D86ALL, getgroup) {
        return (t_getgroup());
      }
      HASH_GUARD(0x6615B5496D03A6EALL, getsize) {
        return (t_getsize());
      }
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        return (t_rewind());
      }
      break;
    case 116:
      HASH_GUARD(0x265BDC54C992EE74LL, getmtime) {
        return (t_getmtime());
      }
      HASH_GUARD(0x08D1EA51B78DA5F4LL, isdot) {
        return (t_isdot());
      }
      break;
    case 120:
      HASH_GUARD(0x25070641C3D924F8LL, getpathname) {
        return (t_getpathname());
      }
      break;
    case 122:
      HASH_GUARD(0x3786834B2A0CCB7ALL, isexecutable) {
        return (t_isexecutable());
      }
      break;
    case 125:
      HASH_GUARD(0x4351578037A06E7DLL, getperms) {
        return (t_getperms());
      }
      break;
    default:
      break;
  }
  return c_splfileinfo::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_directoryiterator::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_splfileinfo::os_invoke(c, s, params, hash, fatal);
}
Variant c_directoryiterator::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 6:
      HASH_GUARD(0x6B2EAD4A44934786LL, getrealpath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getrealpath());
      }
      HASH_GUARD(0x1D3B08AA0AF50F06LL, gettype) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettype());
      }
      break;
    case 8:
      HASH_GUARD(0x1ADA46FCC8EFEC08LL, isdir) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isdir());
      }
      break;
    case 14:
      HASH_GUARD(0x32ABF385AD4BE48ELL, getowner) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getowner());
      }
      break;
    case 19:
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
      break;
    case 28:
      HASH_GUARD(0x572E108C6731E29CLL, getbasename) {
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
        int count = params.size();
        if (count <= 0) return (t_getbasename());
        return (t_getbasename(a0));
      }
      break;
    case 29:
      HASH_GUARD(0x4C43532D60465F1DLL, isfile) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isfile());
      }
      break;
    case 31:
      HASH_GUARD(0x4BC19906B553C59FLL, getatime) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getatime());
      }
      break;
    case 37:
      HASH_GUARD(0x5948407CA9CC4DA5LL, setfileclass) {
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
        int count = params.size();
        if (count <= 0) return (t_setfileclass());
        return (t_setfileclass(a0));
      }
      break;
    case 43:
      HASH_GUARD(0x0D6276BAB75513ABLL, getlinktarget) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlinktarget());
      }
      break;
    case 47:
      HASH_GUARD(0x5640A4755D0078AFLL, getctime) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getctime());
      }
      break;
    case 53:
      HASH_GUARD(0x337DEC2D48BDFE35LL, openfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a2 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_openfile());
        if (count == 1) return (t_openfile(a0));
        if (count == 2) return (t_openfile(a0, a1));
        return (t_openfile(a0, a1, a2));
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
      break;
    case 63:
      HASH_GUARD(0x04C642C6C162243FLL, getpath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getpath());
      }
      HASH_GUARD(0x7D50FA42F9D4923FLL, getfileinfo) {
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
        int count = params.size();
        if (count <= 0) return (t_getfileinfo());
        return (t_getfileinfo(a0));
      }
      break;
    case 68:
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
    case 69:
      HASH_GUARD(0x5676046725D241C5LL, setinfoclass) {
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
        int count = params.size();
        if (count <= 0) return (t_setinfoclass());
        return (t_setinfoclass(a0));
      }
      break;
    case 74:
      HASH_GUARD(0x01A800A73CD2604ALL, getinode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getinode());
      }
      break;
    case 77:
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfilename());
      }
      break;
    case 79:
      HASH_GUARD(0x569FC7D8E9401C4FLL, isreadable) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isreadable());
      }
      break;
    case 81:
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
    case 88:
      HASH_GUARD(0x1D5801BB72C51C58LL, islink) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_islink());
      }
      break;
    case 92:
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
    case 95:
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
    case 99:
      HASH_GUARD(0x638F2A56B8463A63LL, iswritable) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_iswritable());
      }
      break;
    case 103:
      HASH_GUARD(0x00DCC39EDB16AFE7LL, getpathinfo) {
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
        int count = params.size();
        if (count <= 0) return (t_getpathinfo());
        return (t_getpathinfo(a0));
      }
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
        return (t_seek(a0));
      }
      break;
    case 106:
      HASH_GUARD(0x0F9EDEC32565D86ALL, getgroup) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getgroup());
      }
      HASH_GUARD(0x6615B5496D03A6EALL, getsize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getsize());
      }
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
    case 116:
      HASH_GUARD(0x265BDC54C992EE74LL, getmtime) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmtime());
      }
      HASH_GUARD(0x08D1EA51B78DA5F4LL, isdot) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isdot());
      }
      break;
    case 120:
      HASH_GUARD(0x25070641C3D924F8LL, getpathname) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getpathname());
      }
      break;
    case 122:
      HASH_GUARD(0x3786834B2A0CCB7ALL, isexecutable) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isexecutable());
      }
      break;
    case 125:
      HASH_GUARD(0x4351578037A06E7DLL, getperms) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getperms());
      }
      break;
    default:
      break;
  }
  return c_splfileinfo::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_directoryiterator::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_splfileinfo::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_directoryiterator$os_get(const char *s) {
  return c_directoryiterator::os_get(s, -1);
}
Variant &cw_directoryiterator$os_lval(const char *s) {
  return c_directoryiterator::os_lval(s, -1);
}
Variant cw_directoryiterator$os_constant(const char *s) {
  return c_directoryiterator::os_constant(s);
}
Variant cw_directoryiterator$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_directoryiterator::os_invoke(c, s, params, -1, fatal);
}
void c_directoryiterator::init() {
  c_splfileinfo::init();
}
/* SRC: classes/iterator.php line 172 */
void c_directoryiterator::t___construct(Variant v_path) {
  INSTANCE_METHOD_INJECTION(DirectoryIterator, DirectoryIterator::__construct);
  bool oldInCtor = gasInCtor(true);
  x_hphp_directoryiterator___construct(((Object)(this)), toString(v_path));
  gasInCtor(oldInCtor);
} /* function */
/* SRC: classes/iterator.php line 176 */
Variant c_directoryiterator::t_current() {
  INSTANCE_METHOD_INJECTION(DirectoryIterator, DirectoryIterator::current);
  return x_hphp_directoryiterator_current(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 180 */
Variant c_directoryiterator::t_key() {
  INSTANCE_METHOD_INJECTION(DirectoryIterator, DirectoryIterator::key);
  return x_hphp_directoryiterator_key(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 184 */
Variant c_directoryiterator::t_next() {
  INSTANCE_METHOD_INJECTION(DirectoryIterator, DirectoryIterator::next);
  return (x_hphp_directoryiterator_next(((Object)(this))), null);
} /* function */
/* SRC: classes/iterator.php line 188 */
Variant c_directoryiterator::t_rewind() {
  INSTANCE_METHOD_INJECTION(DirectoryIterator, DirectoryIterator::rewind);
  return (x_hphp_directoryiterator_rewind(((Object)(this))), null);
} /* function */
/* SRC: classes/iterator.php line 192 */
Variant c_directoryiterator::t_seek(CVarRef v_position) {
  INSTANCE_METHOD_INJECTION(DirectoryIterator, DirectoryIterator::seek);
  return (x_hphp_directoryiterator_seek(((Object)(this)), toInt64(v_position)), null);
} /* function */
/* SRC: classes/iterator.php line 196 */
String c_directoryiterator::t___tostring() {
  INSTANCE_METHOD_INJECTION(DirectoryIterator, DirectoryIterator::__toString);
  return x_hphp_directoryiterator___tostring(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 200 */
bool c_directoryiterator::t_valid() {
  INSTANCE_METHOD_INJECTION(DirectoryIterator, DirectoryIterator::valid);
  return x_hphp_directoryiterator_valid(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 204 */
bool c_directoryiterator::t_isdot() {
  INSTANCE_METHOD_INJECTION(DirectoryIterator, DirectoryIterator::isDot);
  return x_hphp_directoryiterator_isdot(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 28 */
const int64 q_recursiveiteratoriterator_LEAVES_ONLY = 0LL;
const int64 q_recursiveiteratoriterator_SELF_FIRST = 1LL;
const int64 q_recursiveiteratoriterator_CHILD_FIRST = 2LL;
const int64 q_recursiveiteratoriterator_CATCH_GET_CHILD = 16LL;
Variant c_recursiveiteratoriterator::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_recursiveiteratoriterator::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_recursiveiteratoriterator::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_recursiveiteratoriterator::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_recursiveiteratoriterator::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_recursiveiteratoriterator::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_recursiveiteratoriterator::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_recursiveiteratoriterator::os_constant(const char *s) {
  int64 hash = hash_string(s);
  switch (hash & 7) {
    case 1:
      HASH_RETURN(0x618BE0B31B5C1FD1LL, q_recursiveiteratoriterator_CHILD_FIRST, CHILD_FIRST);
      break;
    case 4:
      HASH_RETURN(0x181DAA5BC4B24F6CLL, q_recursiveiteratoriterator_LEAVES_ONLY, LEAVES_ONLY);
      HASH_RETURN(0x7F32D13555655AA4LL, q_recursiveiteratoriterator_CATCH_GET_CHILD, CATCH_GET_CHILD);
      break;
    case 5:
      HASH_RETURN(0x0822A034E83D2285LL, q_recursiveiteratoriterator_SELF_FIRST, SELF_FIRST);
      break;
    default:
      break;
  }
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(recursiveiteratoriterator)
ObjectData *c_recursiveiteratoriterator::create(Variant v_iterator, Variant v_mode //  = 0LL /* recursiveiteratoriterator::LEAVES_ONLY */
, Variant v_flags //  = 0LL
) {
  init();
  t___construct(v_iterator, v_mode, v_flags);
  return this;
}
ObjectData *c_recursiveiteratoriterator::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    int count = params.size();
    if (count <= 1) return (create(params.rvalAt(0)));
    if (count == 2) return (create(params.rvalAt(0), params.rvalAt(1)));
    return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
  } else return this;
}
void c_recursiveiteratoriterator::dynConstruct(CArrRef params) {
  int count = params.size();
  if (count <= 1) (t___construct(params.rvalAt(0)));
  if (count == 2) (t___construct(params.rvalAt(0), params.rvalAt(1)));
  (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
}
ObjectData *c_recursiveiteratoriterator::cloneImpl() {
  c_recursiveiteratoriterator *obj = NEW(c_recursiveiteratoriterator)();
  cloneSet(obj);
  return obj;
}
void c_recursiveiteratoriterator::cloneSet(c_recursiveiteratoriterator *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_recursiveiteratoriterator::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 1:
      HASH_GUARD(0x56EDB60C824E8C51LL, key) {
        return (t_key());
      }
      break;
    case 4:
      HASH_GUARD(0x6413CB5154808C44LL, valid) {
        return (t_valid());
      }
      HASH_GUARD(0x3106F858B09C7424LL, getinneriterator) {
        return (t_getinneriterator());
      }
      break;
    case 8:
      HASH_GUARD(0x3C6D50F3BB8102B8LL, next) {
        return (t_next());
      }
      break;
    case 10:
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        return (t_rewind());
      }
      break;
    case 12:
      HASH_GUARD(0x5B3A4A72846B21DCLL, current) {
        return (t_current());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 1) return (t___construct(params.rvalAt(0)), null);
        if (count == 2) return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_recursiveiteratoriterator::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 1:
      HASH_GUARD(0x56EDB60C824E8C51LL, key) {
        return (t_key());
      }
      break;
    case 4:
      HASH_GUARD(0x6413CB5154808C44LL, valid) {
        return (t_valid());
      }
      HASH_GUARD(0x3106F858B09C7424LL, getinneriterator) {
        return (t_getinneriterator());
      }
      break;
    case 8:
      HASH_GUARD(0x3C6D50F3BB8102B8LL, next) {
        return (t_next());
      }
      break;
    case 10:
      HASH_GUARD(0x1670096FDE27AF6ALL, rewind) {
        return (t_rewind());
      }
      break;
    case 12:
      HASH_GUARD(0x5B3A4A72846B21DCLL, current) {
        return (t_current());
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 1) return (t___construct(a0), null);
        if (count == 2) return (t___construct(a0, a1), null);
        return (t___construct(a0, a1, a2), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_recursiveiteratoriterator::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_recursiveiteratoriterator::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 1:
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
      HASH_GUARD(0x3106F858B09C7424LL, getinneriterator) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getinneriterator());
      }
      break;
    case 8:
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
        return (t_rewind());
      }
      break;
    case 12:
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
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        Variant a2;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a1 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a2 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t___construct(a0), null);
        if (count == 2) return (t___construct(a0, a1), null);
        return (t___construct(a0, a1, a2), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_recursiveiteratoriterator::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_recursiveiteratoriterator$os_get(const char *s) {
  return c_recursiveiteratoriterator::os_get(s, -1);
}
Variant &cw_recursiveiteratoriterator$os_lval(const char *s) {
  return c_recursiveiteratoriterator::os_lval(s, -1);
}
Variant cw_recursiveiteratoriterator$os_constant(const char *s) {
  return c_recursiveiteratoriterator::os_constant(s);
}
Variant cw_recursiveiteratoriterator$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_recursiveiteratoriterator::os_invoke(c, s, params, -1, fatal);
}
void c_recursiveiteratoriterator::init() {
}
/* SRC: classes/iterator.php line 35 */
void c_recursiveiteratoriterator::t___construct(Variant v_iterator, Variant v_mode //  = 0LL /* recursiveiteratoriterator::LEAVES_ONLY */
, Variant v_flags //  = 0LL
) {
  INSTANCE_METHOD_INJECTION(RecursiveIteratorIterator, RecursiveIteratorIterator::__construct);
  bool oldInCtor = gasInCtor(true);
  x_hphp_recursiveiteratoriterator___construct(((Object)(this)), toObject(v_iterator), toInt64(v_mode), toInt64(v_flags));
  gasInCtor(oldInCtor);
} /* function */
/* SRC: classes/iterator.php line 40 */
Object c_recursiveiteratoriterator::t_getinneriterator() {
  INSTANCE_METHOD_INJECTION(RecursiveIteratorIterator, RecursiveIteratorIterator::getInnerIterator);
  return x_hphp_recursiveiteratoriterator_getinneriterator(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 43 */
Variant c_recursiveiteratoriterator::t_current() {
  INSTANCE_METHOD_INJECTION(RecursiveIteratorIterator, RecursiveIteratorIterator::current);
  return x_hphp_recursiveiteratoriterator_current(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 46 */
Variant c_recursiveiteratoriterator::t_key() {
  INSTANCE_METHOD_INJECTION(RecursiveIteratorIterator, RecursiveIteratorIterator::key);
  return x_hphp_recursiveiteratoriterator_key(((Object)(this)));
} /* function */
/* SRC: classes/iterator.php line 49 */
Variant c_recursiveiteratoriterator::t_next() {
  INSTANCE_METHOD_INJECTION(RecursiveIteratorIterator, RecursiveIteratorIterator::next);
  return (x_hphp_recursiveiteratoriterator_next(((Object)(this))), null);
} /* function */
/* SRC: classes/iterator.php line 52 */
Variant c_recursiveiteratoriterator::t_rewind() {
  INSTANCE_METHOD_INJECTION(RecursiveIteratorIterator, RecursiveIteratorIterator::rewind);
  return (x_hphp_recursiveiteratoriterator_rewind(((Object)(this))), null);
} /* function */
/* SRC: classes/iterator.php line 55 */
bool c_recursiveiteratoriterator::t_valid() {
  INSTANCE_METHOD_INJECTION(RecursiveIteratorIterator, RecursiveIteratorIterator::valid);
  return x_hphp_recursiveiteratoriterator_valid(((Object)(this)));
} /* function */
Object co_arrayiterator(CArrRef params, bool init /* = true */) {
  return Object(p_arrayiterator(NEW(c_arrayiterator)())->dynCreate(params, init));
}
Object co_appenditerator(CArrRef params, bool init /* = true */) {
  return Object(p_appenditerator(NEW(c_appenditerator)())->dynCreate(params, init));
}
Object co_recursivedirectoryiterator(CArrRef params, bool init /* = true */) {
  return Object(p_recursivedirectoryiterator(NEW(c_recursivedirectoryiterator)())->dynCreate(params, init));
}
Object co_directoryiterator(CArrRef params, bool init /* = true */) {
  return Object(p_directoryiterator(NEW(c_directoryiterator)())->dynCreate(params, init));
}
Object co_recursiveiteratoriterator(CArrRef params, bool init /* = true */) {
  return Object(p_recursiveiteratoriterator(NEW(c_recursiveiteratoriterator)())->dynCreate(params, init));
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

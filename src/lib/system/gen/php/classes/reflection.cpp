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
#include <php/classes/reflection.h>
#include <cpp/ext/ext.h>
#include <cpp/eval/eval.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* preface starts */
/* preface finishes */
/* SRC: classes/reflection.php line 92 */
Variant c_reflectionfunctionabstract::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_reflectionfunctionabstract::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_reflectionfunctionabstract::o_get(ArrayElementVec &props) const {
  props.push_back(NEW(ArrayElement)("info", m_info.isReferenced() ? ref(m_info) : m_info));
  c_ObjectData::o_get(props);
}
bool c_reflectionfunctionabstract::o_exists(CStrRef s, int64 hash) const {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 1) {
    case 0:
      HASH_EXISTS_STRING(0x59E9384E33988B3ELL, info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_exists(s, hash);
}
Variant c_reflectionfunctionabstract::o_get(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 1) {
    case 0:
      HASH_RETURN_STRING(0x59E9384E33988B3ELL, m_info,
                         info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_get(s, hash);
}
Variant c_reflectionfunctionabstract::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 1) {
    case 0:
      HASH_SET_STRING(0x59E9384E33988B3ELL, m_info,
                      info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_reflectionfunctionabstract::o_lval(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 1) {
    case 0:
      HASH_RETURN_STRING(0x59E9384E33988B3ELL, m_info,
                         info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_lval(s, hash);
}
Variant c_reflectionfunctionabstract::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(reflectionfunctionabstract)
ObjectData *c_reflectionfunctionabstract::cloneImpl() {
  c_reflectionfunctionabstract *obj = NEW(c_reflectionfunctionabstract)();
  cloneSet(obj);
  return obj;
}
void c_reflectionfunctionabstract::cloneSet(c_reflectionfunctionabstract *clone) {
  clone->m_info = m_info.isReferenced() ? ref(m_info) : m_info;
  ObjectData::cloneSet(clone);
}
Variant c_reflectionfunctionabstract::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 0:
      HASH_GUARD(0x57D8DC34C9A03560LL, getnumberofparameters) {
        return (t_getnumberofparameters());
      }
      break;
    case 1:
      HASH_GUARD(0x4D637DECDBFA6221LL, getnumberofrequiredparameters) {
        return (t_getnumberofrequiredparameters());
      }
      break;
    case 2:
      HASH_GUARD(0x33A6C2CFBDB05EE2LL, getclosure) {
        return (t_getclosure());
      }
      break;
    case 4:
      HASH_GUARD(0x6A6B8BECAE7D4164LL, isuserdefined) {
        return (t_isuserdefined());
      }
      HASH_GUARD(0x39C1BB731CB1CB04LL, getstartline) {
        return (t_getstartline());
      }
      break;
    case 5:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 8:
      HASH_GUARD(0x37FFB8F44A3329A8LL, getstaticvariables) {
        return (t_getstaticvariables());
      }
      HASH_GUARD(0x1A3AB3B0276D2668LL, returnsreference) {
        return (t_returnsreference());
      }
      break;
    case 13:
      HASH_GUARD(0x3E62225132C2A32DLL, getparameters) {
        return (t_getparameters());
      }
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        return (t_getfilename());
      }
      break;
    case 15:
      HASH_GUARD(0x48FDF6C5835C64CFLL, isinternal) {
        return (t_isinternal());
      }
      break;
    case 16:
      HASH_GUARD(0x6C19E85007BC4570LL, getendline) {
        return (t_getendline());
      }
      break;
    case 24:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        return (t_getdoccomment());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_reflectionfunctionabstract::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 0:
      HASH_GUARD(0x57D8DC34C9A03560LL, getnumberofparameters) {
        return (t_getnumberofparameters());
      }
      break;
    case 1:
      HASH_GUARD(0x4D637DECDBFA6221LL, getnumberofrequiredparameters) {
        return (t_getnumberofrequiredparameters());
      }
      break;
    case 2:
      HASH_GUARD(0x33A6C2CFBDB05EE2LL, getclosure) {
        return (t_getclosure());
      }
      break;
    case 4:
      HASH_GUARD(0x6A6B8BECAE7D4164LL, isuserdefined) {
        return (t_isuserdefined());
      }
      HASH_GUARD(0x39C1BB731CB1CB04LL, getstartline) {
        return (t_getstartline());
      }
      break;
    case 5:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 8:
      HASH_GUARD(0x37FFB8F44A3329A8LL, getstaticvariables) {
        return (t_getstaticvariables());
      }
      HASH_GUARD(0x1A3AB3B0276D2668LL, returnsreference) {
        return (t_returnsreference());
      }
      break;
    case 13:
      HASH_GUARD(0x3E62225132C2A32DLL, getparameters) {
        return (t_getparameters());
      }
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        return (t_getfilename());
      }
      break;
    case 15:
      HASH_GUARD(0x48FDF6C5835C64CFLL, isinternal) {
        return (t_isinternal());
      }
      break;
    case 16:
      HASH_GUARD(0x6C19E85007BC4570LL, getendline) {
        return (t_getendline());
      }
      break;
    case 24:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        return (t_getdoccomment());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_reflectionfunctionabstract::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_reflectionfunctionabstract::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 0:
      HASH_GUARD(0x57D8DC34C9A03560LL, getnumberofparameters) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnumberofparameters());
      }
      break;
    case 1:
      HASH_GUARD(0x4D637DECDBFA6221LL, getnumberofrequiredparameters) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnumberofrequiredparameters());
      }
      break;
    case 2:
      HASH_GUARD(0x33A6C2CFBDB05EE2LL, getclosure) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getclosure());
      }
      break;
    case 4:
      HASH_GUARD(0x6A6B8BECAE7D4164LL, isuserdefined) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isuserdefined());
      }
      HASH_GUARD(0x39C1BB731CB1CB04LL, getstartline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getstartline());
      }
      break;
    case 5:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getname());
      }
      break;
    case 8:
      HASH_GUARD(0x37FFB8F44A3329A8LL, getstaticvariables) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getstaticvariables());
      }
      HASH_GUARD(0x1A3AB3B0276D2668LL, returnsreference) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_returnsreference());
      }
      break;
    case 13:
      HASH_GUARD(0x3E62225132C2A32DLL, getparameters) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getparameters());
      }
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
    case 15:
      HASH_GUARD(0x48FDF6C5835C64CFLL, isinternal) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isinternal());
      }
      break;
    case 16:
      HASH_GUARD(0x6C19E85007BC4570LL, getendline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getendline());
      }
      break;
    case 24:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getdoccomment());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_reflectionfunctionabstract::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_reflectionfunctionabstract$os_get(const char *s) {
  return c_reflectionfunctionabstract::os_get(s, -1);
}
Variant &cw_reflectionfunctionabstract$os_lval(const char *s) {
  return c_reflectionfunctionabstract::os_lval(s, -1);
}
Variant cw_reflectionfunctionabstract$os_constant(const char *s) {
  return c_reflectionfunctionabstract::os_constant(s);
}
Variant cw_reflectionfunctionabstract$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_reflectionfunctionabstract::os_invoke(c, s, params, -1, fatal);
}
void c_reflectionfunctionabstract::init() {
  m_info = null;
}
/* SRC: classes/reflection.php line 95 */
Variant c_reflectionfunctionabstract::t_getname() {
  INSTANCE_METHOD_INJECTION(ReflectionFunctionAbstract, ReflectionFunctionAbstract::getName);
  return m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL);
} /* function */
/* SRC: classes/reflection.php line 99 */
Variant c_reflectionfunctionabstract::t_isinternal() {
  INSTANCE_METHOD_INJECTION(ReflectionFunctionAbstract, ReflectionFunctionAbstract::isInternal);
  return m_info.rvalAt("internal", 0x575D95D69332A8ACLL);
} /* function */
/* SRC: classes/reflection.php line 103 */
Variant c_reflectionfunctionabstract::t_getclosure() {
  INSTANCE_METHOD_INJECTION(ReflectionFunctionAbstract, ReflectionFunctionAbstract::getClosure);
  return m_info.rvalAt("closure", 0x10958EC44CD61020LL);
} /* function */
/* SRC: classes/reflection.php line 107 */
bool c_reflectionfunctionabstract::t_isuserdefined() {
  INSTANCE_METHOD_INJECTION(ReflectionFunctionAbstract, ReflectionFunctionAbstract::isUserDefined);
  return !(toBoolean(m_info.rvalAt("internal", 0x575D95D69332A8ACLL)));
} /* function */
/* SRC: classes/reflection.php line 111 */
Variant c_reflectionfunctionabstract::t_getfilename() {
  INSTANCE_METHOD_INJECTION(ReflectionFunctionAbstract, ReflectionFunctionAbstract::getFileName);
  return m_info.rvalAt("file", 0x612E37678CE7DB5BLL);
} /* function */
/* SRC: classes/reflection.php line 115 */
Variant c_reflectionfunctionabstract::t_getstartline() {
  INSTANCE_METHOD_INJECTION(ReflectionFunctionAbstract, ReflectionFunctionAbstract::getStartLine);
  return m_info.rvalAt("line1", 0x7E7BD613D4C67725LL);
} /* function */
/* SRC: classes/reflection.php line 119 */
Variant c_reflectionfunctionabstract::t_getendline() {
  INSTANCE_METHOD_INJECTION(ReflectionFunctionAbstract, ReflectionFunctionAbstract::getEndLine);
  return m_info.rvalAt("line2", 0x641B12C5BEFE32A8LL);
} /* function */
/* SRC: classes/reflection.php line 123 */
Variant c_reflectionfunctionabstract::t_getdoccomment() {
  INSTANCE_METHOD_INJECTION(ReflectionFunctionAbstract, ReflectionFunctionAbstract::getDocComment);
  return m_info.rvalAt("doc", 0x16758C759CFA17A6LL);
} /* function */
/* SRC: classes/reflection.php line 127 */
Variant c_reflectionfunctionabstract::t_getstaticvariables() {
  INSTANCE_METHOD_INJECTION(ReflectionFunctionAbstract, ReflectionFunctionAbstract::getStaticVariables);
  return m_info.rvalAt("static_variables", 0x4BC9448B5BE7A94ALL);
} /* function */
/* SRC: classes/reflection.php line 131 */
Variant c_reflectionfunctionabstract::t_returnsreference() {
  INSTANCE_METHOD_INJECTION(ReflectionFunctionAbstract, ReflectionFunctionAbstract::returnsReference);
  return m_info.rvalAt("ref", 0x0B1A6D25134FD5FALL);
} /* function */
/* SRC: classes/reflection.php line 135 */
Array c_reflectionfunctionabstract::t_getparameters() {
  INSTANCE_METHOD_INJECTION(ReflectionFunctionAbstract, ReflectionFunctionAbstract::getParameters);
  Array v_ret;
  Primitive v_name = 0;
  Variant v_info;
  p_reflectionparameter v_param;

  v_ret = SystemScalarArrays::ssa_[0];
  {
    LOOP_COUNTER(1);
    Variant map2 = m_info.rvalAt("params", 0x6E4C9E151F20AC62LL);
    for (ArrayIterPtr iter3 = map2.begin("reflectionfunctionabstract"); !iter3->end(); iter3->next()) {
      LOOP_COUNTER_CHECK(1);
      v_info = iter3->second();
      v_name = iter3->first();
      {
        ((Object)(v_param = ((Object)(p_reflectionparameter(p_reflectionparameter(NEWOBJ(c_reflectionparameter)())->create(null, null))))));
        v_param->m_info = v_info;
        v_ret.append((((Object)(v_param))));
      }
    }
  }
  return v_ret;
} /* function */
/* SRC: classes/reflection.php line 145 */
int c_reflectionfunctionabstract::t_getnumberofparameters() {
  INSTANCE_METHOD_INJECTION(ReflectionFunctionAbstract, ReflectionFunctionAbstract::getNumberOfParameters);
  return x_count(m_info.rvalAt("params", 0x6E4C9E151F20AC62LL));
} /* function */
/* SRC: classes/reflection.php line 149 */
int64 c_reflectionfunctionabstract::t_getnumberofrequiredparameters() {
  INSTANCE_METHOD_INJECTION(ReflectionFunctionAbstract, ReflectionFunctionAbstract::getNumberOfRequiredParameters);
  int64 v_count = 0;
  Array v_params;
  Primitive v_name = 0;
  Variant v_param;

  v_count = 0LL;
  v_params = t_getparameters();
  {
    LOOP_COUNTER(4);
    for (ArrayIter iter6 = v_params.begin("reflectionfunctionabstract"); !iter6.end(); ++iter6) {
      LOOP_COUNTER_CHECK(4);
      v_param = iter6.second();
      v_name = iter6.first();
      {
        if (toBoolean(v_param.o_invoke_few_args("isOptional", 0x2D6EF48BBAB22735LL, 0))) {
          break;
        }
        v_count++;
      }
    }
  }
  return v_count;
} /* function */
/* SRC: classes/reflection.php line 472 */
Variant c_reflectionobject::os_get(const char *s, int64 hash) {
  return c_reflectionclass::os_get(s, hash);
}
Variant &c_reflectionobject::os_lval(const char *s, int64 hash) {
  return c_reflectionclass::os_lval(s, hash);
}
void c_reflectionobject::o_get(ArrayElementVec &props) const {
  c_reflectionclass::o_get(props);
}
bool c_reflectionobject::o_exists(CStrRef s, int64 hash) const {
  return c_reflectionclass::o_exists(s, hash);
}
Variant c_reflectionobject::o_get(CStrRef s, int64 hash) {
  return c_reflectionclass::o_get(s, hash);
}
Variant c_reflectionobject::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_reflectionclass::o_set(s, hash, v, forInit);
}
Variant &c_reflectionobject::o_lval(CStrRef s, int64 hash) {
  return c_reflectionclass::o_lval(s, hash);
}
Variant c_reflectionobject::os_constant(const char *s) {
  return c_reflectionclass::os_constant(s);
}
IMPLEMENT_CLASS(reflectionobject)
ObjectData *c_reflectionobject::create(Variant v_obj) {
  init();
  t___construct(v_obj);
  return this;
}
ObjectData *c_reflectionobject::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0)));
  } else return this;
}
void c_reflectionobject::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0)));
}
ObjectData *c_reflectionobject::cloneImpl() {
  c_reflectionobject *obj = NEW(c_reflectionobject)();
  cloneSet(obj);
  return obj;
}
void c_reflectionobject::cloneSet(c_reflectionobject *clone) {
  c_reflectionclass::cloneSet(clone);
}
Variant c_reflectionobject::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 4:
      HASH_GUARD(0x39C1BB731CB1CB04LL, getstartline) {
        return (t_getstartline());
      }
      break;
    case 15:
      HASH_GUARD(0x40C7B30DCB439C8FLL, hasproperty) {
        return (t_hasproperty(params.rvalAt(0)));
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 20:
      HASH_GUARD(0x24253EBA491D6014LL, getmodifiers) {
        return (t_getmodifiers());
      }
      break;
    case 24:
      HASH_GUARD(0x21820E7AA4733998LL, hasmethod) {
        return (t_hasmethod(params.rvalAt(0)));
      }
      break;
    case 27:
      HASH_GUARD(0x0F1AD0A8EC4C229BLL, getdefaultproperties) {
        return (t_getdefaultproperties());
      }
      break;
    case 30:
      HASH_GUARD(0x1BC5F3D87676509ELL, isinterface) {
        return (t_isinterface());
      }
      break;
    case 34:
      HASH_GUARD(0x323D9BCB05797B22LL, getstaticproperties) {
        return (t_getstaticproperties());
      }
      break;
    case 41:
      HASH_GUARD(0x030CE1D6142F8C29LL, isinstance) {
        return (t_isinstance(params.rvalAt(0)));
      }
      HASH_GUARD(0x1D6B8CA358B49929LL, getextensionname) {
        return (t_getextensionname());
      }
      break;
    case 42:
      HASH_GUARD(0x16BA16CE6488AAAALL, getmethods) {
        return (t_getmethods());
      }
      HASH_GUARD(0x226F6E80CECD3CAALL, getconstructor) {
        return (t_getconstructor());
      }
      break;
    case 46:
      HASH_GUARD(0x3C882D4A895F612ELL, getstaticpropertyvalue) {
        int count = params.size();
        if (count <= 1) return (t_getstaticpropertyvalue(params.rvalAt(0)));
        return (t_getstaticpropertyvalue(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x06FB6A7DC3D795AELL, isfinal) {
        return (t_isfinal());
      }
      break;
    case 48:
      HASH_GUARD(0x30A86FCA01FE7030LL, newinstance) {
        int count = params.size();
        if (count <= 0) return (t_newinstance(count));
        return (t_newinstance(count,params.slice(0, count - 0, false)));
      }
      break;
    case 52:
      HASH_GUARD(0x3DB53E1FBD3C0734LL, getconstant) {
        return (t_getconstant(params.rvalAt(0)));
      }
      break;
    case 54:
      HASH_GUARD(0x0D81ECE253A3B5B6LL, getmethod) {
        return (t_getmethod(params.rvalAt(0)));
      }
      break;
    case 55:
      HASH_GUARD(0x7460D945DA32FDB7LL, isabstract) {
        return (t_isabstract());
      }
      break;
    case 59:
      HASH_GUARD(0x25D24435915E6E3BLL, getextension) {
        return (t_getextension());
      }
      break;
    case 60:
      HASH_GUARD(0x0D8AAD6BA2BBCD3CLL, isinstantiable) {
        return (t_isinstantiable());
      }
      break;
    case 63:
      HASH_GUARD(0x54C2DC04C4A62B3FLL, hasconstant) {
        return (t_hasconstant(params.rvalAt(0)));
      }
      break;
    case 67:
      HASH_GUARD(0x67C15E3D98C00B43LL, getinterfaces) {
        return (t_getinterfaces());
      }
      break;
    case 68:
      HASH_GUARD(0x1EB679C3602F4B44LL, getproperties) {
        return (t_getproperties());
      }
      break;
    case 71:
      HASH_GUARD(0x0FD73627FB023047LL, getproperty) {
        return (t_getproperty(params.rvalAt(0)));
      }
      break;
    case 77:
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        return (t_getfilename());
      }
      break;
    case 78:
      HASH_GUARD(0x7D5A57B5370B68CELL, isiterateable) {
        return (t_isiterateable());
      }
      break;
    case 79:
      HASH_GUARD(0x48FDF6C5835C64CFLL, isinternal) {
        return (t_isinternal());
      }
      break;
    case 85:
      HASH_GUARD(0x1CC71CB013143955LL, getconstants) {
        return (t_getconstants());
      }
      break;
    case 95:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0)), null);
      }
      break;
    case 100:
      HASH_GUARD(0x6A6B8BECAE7D4164LL, isuserdefined) {
        return (t_isuserdefined());
      }
      break;
    case 101:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 102:
      HASH_GUARD(0x2735DCC254EE5C66LL, newinstanceargs) {
        return (t_newinstanceargs(params.rvalAt(0)));
      }
      break;
    case 104:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(o_getClassName(), params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 112:
      HASH_GUARD(0x6C19E85007BC4570LL, getendline) {
        return (t_getendline());
      }
      break;
    case 113:
      HASH_GUARD(0x07ECA928E37717F1LL, setstaticpropertyvalue) {
        return (t_setstaticpropertyvalue(params.rvalAt(0), params.rvalAt(1)), null);
      }
      break;
    case 114:
      HASH_GUARD(0x74F7FEDE16957472LL, getparentclass) {
        return (t_getparentclass());
      }
      break;
    case 120:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        return (t_getdoccomment());
      }
      break;
    case 123:
      HASH_GUARD(0x28DC702215C7D6FBLL, implementsinterface) {
        return (t_implementsinterface(params.rvalAt(0)));
      }
      break;
    case 126:
      HASH_GUARD(0x373333991926C97ELL, issubclassof) {
        return (t_issubclassof(params.rvalAt(0)));
      }
      break;
    default:
      break;
  }
  return c_reflectionclass::o_invoke(s, params, hash, fatal);
}
Variant c_reflectionobject::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 4:
      HASH_GUARD(0x39C1BB731CB1CB04LL, getstartline) {
        return (t_getstartline());
      }
      break;
    case 15:
      HASH_GUARD(0x40C7B30DCB439C8FLL, hasproperty) {
        return (t_hasproperty(a0));
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 20:
      HASH_GUARD(0x24253EBA491D6014LL, getmodifiers) {
        return (t_getmodifiers());
      }
      break;
    case 24:
      HASH_GUARD(0x21820E7AA4733998LL, hasmethod) {
        return (t_hasmethod(a0));
      }
      break;
    case 27:
      HASH_GUARD(0x0F1AD0A8EC4C229BLL, getdefaultproperties) {
        return (t_getdefaultproperties());
      }
      break;
    case 30:
      HASH_GUARD(0x1BC5F3D87676509ELL, isinterface) {
        return (t_isinterface());
      }
      break;
    case 34:
      HASH_GUARD(0x323D9BCB05797B22LL, getstaticproperties) {
        return (t_getstaticproperties());
      }
      break;
    case 41:
      HASH_GUARD(0x030CE1D6142F8C29LL, isinstance) {
        return (t_isinstance(a0));
      }
      HASH_GUARD(0x1D6B8CA358B49929LL, getextensionname) {
        return (t_getextensionname());
      }
      break;
    case 42:
      HASH_GUARD(0x16BA16CE6488AAAALL, getmethods) {
        return (t_getmethods());
      }
      HASH_GUARD(0x226F6E80CECD3CAALL, getconstructor) {
        return (t_getconstructor());
      }
      break;
    case 46:
      HASH_GUARD(0x3C882D4A895F612ELL, getstaticpropertyvalue) {
        if (count <= 1) return (t_getstaticpropertyvalue(a0));
        return (t_getstaticpropertyvalue(a0, a1));
      }
      HASH_GUARD(0x06FB6A7DC3D795AELL, isfinal) {
        return (t_isfinal());
      }
      break;
    case 48:
      HASH_GUARD(0x30A86FCA01FE7030LL, newinstance) {
        if (count <= 0) return (t_newinstance(count));
        Array params;
        if (count >= 1) params.append(a0);
        if (count >= 2) params.append(a1);
        if (count >= 3) params.append(a2);
        if (count >= 4) params.append(a3);
        if (count >= 5) params.append(a4);
        if (count >= 6) params.append(a5);
        return (t_newinstance(count,params));
      }
      break;
    case 52:
      HASH_GUARD(0x3DB53E1FBD3C0734LL, getconstant) {
        return (t_getconstant(a0));
      }
      break;
    case 54:
      HASH_GUARD(0x0D81ECE253A3B5B6LL, getmethod) {
        return (t_getmethod(a0));
      }
      break;
    case 55:
      HASH_GUARD(0x7460D945DA32FDB7LL, isabstract) {
        return (t_isabstract());
      }
      break;
    case 59:
      HASH_GUARD(0x25D24435915E6E3BLL, getextension) {
        return (t_getextension());
      }
      break;
    case 60:
      HASH_GUARD(0x0D8AAD6BA2BBCD3CLL, isinstantiable) {
        return (t_isinstantiable());
      }
      break;
    case 63:
      HASH_GUARD(0x54C2DC04C4A62B3FLL, hasconstant) {
        return (t_hasconstant(a0));
      }
      break;
    case 67:
      HASH_GUARD(0x67C15E3D98C00B43LL, getinterfaces) {
        return (t_getinterfaces());
      }
      break;
    case 68:
      HASH_GUARD(0x1EB679C3602F4B44LL, getproperties) {
        return (t_getproperties());
      }
      break;
    case 71:
      HASH_GUARD(0x0FD73627FB023047LL, getproperty) {
        return (t_getproperty(a0));
      }
      break;
    case 77:
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        return (t_getfilename());
      }
      break;
    case 78:
      HASH_GUARD(0x7D5A57B5370B68CELL, isiterateable) {
        return (t_isiterateable());
      }
      break;
    case 79:
      HASH_GUARD(0x48FDF6C5835C64CFLL, isinternal) {
        return (t_isinternal());
      }
      break;
    case 85:
      HASH_GUARD(0x1CC71CB013143955LL, getconstants) {
        return (t_getconstants());
      }
      break;
    case 95:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0), null);
      }
      break;
    case 100:
      HASH_GUARD(0x6A6B8BECAE7D4164LL, isuserdefined) {
        return (t_isuserdefined());
      }
      break;
    case 101:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 102:
      HASH_GUARD(0x2735DCC254EE5C66LL, newinstanceargs) {
        return (t_newinstanceargs(a0));
      }
      break;
    case 104:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(o_getClassName(), a0, a1));
      }
      break;
    case 112:
      HASH_GUARD(0x6C19E85007BC4570LL, getendline) {
        return (t_getendline());
      }
      break;
    case 113:
      HASH_GUARD(0x07ECA928E37717F1LL, setstaticpropertyvalue) {
        return (t_setstaticpropertyvalue(a0, a1), null);
      }
      break;
    case 114:
      HASH_GUARD(0x74F7FEDE16957472LL, getparentclass) {
        return (t_getparentclass());
      }
      break;
    case 120:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        return (t_getdoccomment());
      }
      break;
    case 123:
      HASH_GUARD(0x28DC702215C7D6FBLL, implementsinterface) {
        return (t_implementsinterface(a0));
      }
      break;
    case 126:
      HASH_GUARD(0x373333991926C97ELL, issubclassof) {
        return (t_issubclassof(a0));
      }
      break;
    default:
      break;
  }
  return c_reflectionclass::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_reflectionobject::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 1) {
    case 0:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(c, params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    default:
      break;
  }
  return c_reflectionclass::os_invoke(c, s, params, hash, fatal);
}
Variant c_reflectionobject::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 4:
      HASH_GUARD(0x39C1BB731CB1CB04LL, getstartline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getstartline());
      }
      break;
    case 15:
      HASH_GUARD(0x40C7B30DCB439C8FLL, hasproperty) {
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
        return (t_hasproperty(a0));
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
    case 20:
      HASH_GUARD(0x24253EBA491D6014LL, getmodifiers) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmodifiers());
      }
      break;
    case 24:
      HASH_GUARD(0x21820E7AA4733998LL, hasmethod) {
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
        return (t_hasmethod(a0));
      }
      break;
    case 27:
      HASH_GUARD(0x0F1AD0A8EC4C229BLL, getdefaultproperties) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getdefaultproperties());
      }
      break;
    case 30:
      HASH_GUARD(0x1BC5F3D87676509ELL, isinterface) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isinterface());
      }
      break;
    case 34:
      HASH_GUARD(0x323D9BCB05797B22LL, getstaticproperties) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getstaticproperties());
      }
      break;
    case 41:
      HASH_GUARD(0x030CE1D6142F8C29LL, isinstance) {
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
        return (t_isinstance(a0));
      }
      HASH_GUARD(0x1D6B8CA358B49929LL, getextensionname) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getextensionname());
      }
      break;
    case 42:
      HASH_GUARD(0x16BA16CE6488AAAALL, getmethods) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmethods());
      }
      HASH_GUARD(0x226F6E80CECD3CAALL, getconstructor) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getconstructor());
      }
      break;
    case 46:
      HASH_GUARD(0x3C882D4A895F612ELL, getstaticpropertyvalue) {
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
        if (count <= 1) return (t_getstaticpropertyvalue(a0));
        return (t_getstaticpropertyvalue(a0, a1));
      }
      HASH_GUARD(0x06FB6A7DC3D795AELL, isfinal) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isfinal());
      }
      break;
    case 48:
      HASH_GUARD(0x30A86FCA01FE7030LL, newinstance) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        Array vargs;
        for (; it != params.end(); ++it) {
          vargs.append((*it)->eval(env));
        }
        int count = params.size();
        if (count <= 0) return (t_newinstance(count));
        return (t_newinstance(count,vargs));
      }
      break;
    case 52:
      HASH_GUARD(0x3DB53E1FBD3C0734LL, getconstant) {
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
        return (t_getconstant(a0));
      }
      break;
    case 54:
      HASH_GUARD(0x0D81ECE253A3B5B6LL, getmethod) {
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
        return (t_getmethod(a0));
      }
      break;
    case 55:
      HASH_GUARD(0x7460D945DA32FDB7LL, isabstract) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isabstract());
      }
      break;
    case 59:
      HASH_GUARD(0x25D24435915E6E3BLL, getextension) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getextension());
      }
      break;
    case 60:
      HASH_GUARD(0x0D8AAD6BA2BBCD3CLL, isinstantiable) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isinstantiable());
      }
      break;
    case 63:
      HASH_GUARD(0x54C2DC04C4A62B3FLL, hasconstant) {
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
        return (t_hasconstant(a0));
      }
      break;
    case 67:
      HASH_GUARD(0x67C15E3D98C00B43LL, getinterfaces) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getinterfaces());
      }
      break;
    case 68:
      HASH_GUARD(0x1EB679C3602F4B44LL, getproperties) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getproperties());
      }
      break;
    case 71:
      HASH_GUARD(0x0FD73627FB023047LL, getproperty) {
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
        return (t_getproperty(a0));
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
    case 78:
      HASH_GUARD(0x7D5A57B5370B68CELL, isiterateable) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isiterateable());
      }
      break;
    case 79:
      HASH_GUARD(0x48FDF6C5835C64CFLL, isinternal) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isinternal());
      }
      break;
    case 85:
      HASH_GUARD(0x1CC71CB013143955LL, getconstants) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getconstants());
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
    case 100:
      HASH_GUARD(0x6A6B8BECAE7D4164LL, isuserdefined) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isuserdefined());
      }
      break;
    case 101:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getname());
      }
      break;
    case 102:
      HASH_GUARD(0x2735DCC254EE5C66LL, newinstanceargs) {
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
        return (t_newinstanceargs(a0));
      }
      break;
    case 104:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
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
        return (ti_export(o_getClassName(), a0, a1));
      }
      break;
    case 112:
      HASH_GUARD(0x6C19E85007BC4570LL, getendline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getendline());
      }
      break;
    case 113:
      HASH_GUARD(0x07ECA928E37717F1LL, setstaticpropertyvalue) {
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
        return (t_setstaticpropertyvalue(a0, a1), null);
      }
      break;
    case 114:
      HASH_GUARD(0x74F7FEDE16957472LL, getparentclass) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getparentclass());
      }
      break;
    case 120:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getdoccomment());
      }
      break;
    case 123:
      HASH_GUARD(0x28DC702215C7D6FBLL, implementsinterface) {
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
        return (t_implementsinterface(a0));
      }
      break;
    case 126:
      HASH_GUARD(0x373333991926C97ELL, issubclassof) {
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
        return (t_issubclassof(a0));
      }
      break;
    default:
      break;
  }
  return c_reflectionclass::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_reflectionobject::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 1) {
    case 0:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
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
        return (ti_export(c, a0, a1));
      }
      break;
    default:
      break;
  }
  return c_reflectionclass::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_reflectionobject$os_get(const char *s) {
  return c_reflectionobject::os_get(s, -1);
}
Variant &cw_reflectionobject$os_lval(const char *s) {
  return c_reflectionobject::os_lval(s, -1);
}
Variant cw_reflectionobject$os_constant(const char *s) {
  return c_reflectionobject::os_constant(s);
}
Variant cw_reflectionobject$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_reflectionobject::os_invoke(c, s, params, -1, fatal);
}
void c_reflectionobject::init() {
  c_reflectionclass::init();
}
/* SRC: classes/reflection.php line 473 */
void c_reflectionobject::t___construct(Variant v_obj) {
  INSTANCE_METHOD_INJECTION(ReflectionObject, ReflectionObject::__construct);
  bool oldInCtor = gasInCtor(true);
  m_info = x_hphp_get_class_info(v_obj);
  gasInCtor(oldInCtor);
} /* function */
/* SRC: classes/reflection.php line 477 */
Variant c_reflectionobject::ti_export(const char* cls, Variant v_obj, CVarRef v_ret) {
  STATIC_METHOD_INJECTION(ReflectionObject, ReflectionObject::export);
  String v_str;

  v_obj = ((Object)(p_reflectionobject(p_reflectionobject(NEWOBJ(c_reflectionobject)())->create(v_obj))));
  v_str = toString(v_obj);
  if (toBoolean(v_ret)) {
    return v_str;
  }
  print(v_str);
  return null;
} /* function */
/* SRC: classes/reflection.php line 11 */
Variant c_reflectionexception::os_get(const char *s, int64 hash) {
  return c_exception::os_get(s, hash);
}
Variant &c_reflectionexception::os_lval(const char *s, int64 hash) {
  return c_exception::os_lval(s, hash);
}
void c_reflectionexception::o_get(ArrayElementVec &props) const {
  c_exception::o_get(props);
}
bool c_reflectionexception::o_exists(CStrRef s, int64 hash) const {
  return c_exception::o_exists(s, hash);
}
Variant c_reflectionexception::o_get(CStrRef s, int64 hash) {
  return c_exception::o_get(s, hash);
}
Variant c_reflectionexception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_exception::o_set(s, hash, v, forInit);
}
Variant &c_reflectionexception::o_lval(CStrRef s, int64 hash) {
  return c_exception::o_lval(s, hash);
}
Variant c_reflectionexception::os_constant(const char *s) {
  return c_exception::os_constant(s);
}
IMPLEMENT_CLASS(reflectionexception)
ObjectData *c_reflectionexception::cloneImpl() {
  c_reflectionexception *obj = NEW(c_reflectionexception)();
  cloneSet(obj);
  return obj;
}
void c_reflectionexception::cloneSet(c_reflectionexception *clone) {
  c_exception::cloneSet(clone);
}
Variant c_reflectionexception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
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
Variant c_reflectionexception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
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
Variant c_reflectionexception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_exception::os_invoke(c, s, params, hash, fatal);
}
Variant c_reflectionexception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
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
Variant c_reflectionexception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_exception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_reflectionexception$os_get(const char *s) {
  return c_reflectionexception::os_get(s, -1);
}
Variant &cw_reflectionexception$os_lval(const char *s) {
  return c_reflectionexception::os_lval(s, -1);
}
Variant cw_reflectionexception$os_constant(const char *s) {
  return c_reflectionexception::os_constant(s);
}
Variant cw_reflectionexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_reflectionexception::os_invoke(c, s, params, -1, fatal);
}
void c_reflectionexception::init() {
  c_exception::init();
}
/* SRC: classes/reflection.php line 200 */
Variant c_reflectionclass::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_reflectionclass::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_reflectionclass::o_get(ArrayElementVec &props) const {
  props.push_back(NEW(ArrayElement)("name", m_name.isReferenced() ? ref(m_name) : m_name));
  props.push_back(NEW(ArrayElement)("info", m_info.isReferenced() ? ref(m_info) : m_info));
  c_ObjectData::o_get(props);
}
bool c_reflectionclass::o_exists(CStrRef s, int64 hash) const {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_EXISTS_STRING(0x0BCDB293DC3CBDDCLL, name, 4);
      break;
    case 2:
      HASH_EXISTS_STRING(0x59E9384E33988B3ELL, info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_exists(s, hash);
}
Variant c_reflectionclass::o_get(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_RETURN_STRING(0x0BCDB293DC3CBDDCLL, m_name,
                         name, 4);
      break;
    case 2:
      HASH_RETURN_STRING(0x59E9384E33988B3ELL, m_info,
                         info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_get(s, hash);
}
Variant c_reflectionclass::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_SET_STRING(0x0BCDB293DC3CBDDCLL, m_name,
                      name, 4);
      break;
    case 2:
      HASH_SET_STRING(0x59E9384E33988B3ELL, m_info,
                      info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_reflectionclass::o_lval(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_RETURN_STRING(0x0BCDB293DC3CBDDCLL, m_name,
                         name, 4);
      break;
    case 2:
      HASH_RETURN_STRING(0x59E9384E33988B3ELL, m_info,
                         info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_lval(s, hash);
}
Variant c_reflectionclass::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(reflectionclass)
ObjectData *c_reflectionclass::create(Variant v_name) {
  init();
  t___construct(v_name);
  return this;
}
ObjectData *c_reflectionclass::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0)));
  } else return this;
}
void c_reflectionclass::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0)));
}
ObjectData *c_reflectionclass::cloneImpl() {
  c_reflectionclass *obj = NEW(c_reflectionclass)();
  cloneSet(obj);
  return obj;
}
void c_reflectionclass::cloneSet(c_reflectionclass *clone) {
  clone->m_name = m_name.isReferenced() ? ref(m_name) : m_name;
  clone->m_info = m_info.isReferenced() ? ref(m_info) : m_info;
  ObjectData::cloneSet(clone);
}
Variant c_reflectionclass::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 4:
      HASH_GUARD(0x39C1BB731CB1CB04LL, getstartline) {
        return (t_getstartline());
      }
      break;
    case 15:
      HASH_GUARD(0x40C7B30DCB439C8FLL, hasproperty) {
        return (t_hasproperty(params.rvalAt(0)));
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 20:
      HASH_GUARD(0x24253EBA491D6014LL, getmodifiers) {
        return (t_getmodifiers());
      }
      break;
    case 24:
      HASH_GUARD(0x21820E7AA4733998LL, hasmethod) {
        return (t_hasmethod(params.rvalAt(0)));
      }
      break;
    case 27:
      HASH_GUARD(0x0F1AD0A8EC4C229BLL, getdefaultproperties) {
        return (t_getdefaultproperties());
      }
      break;
    case 30:
      HASH_GUARD(0x1BC5F3D87676509ELL, isinterface) {
        return (t_isinterface());
      }
      break;
    case 34:
      HASH_GUARD(0x323D9BCB05797B22LL, getstaticproperties) {
        return (t_getstaticproperties());
      }
      break;
    case 41:
      HASH_GUARD(0x030CE1D6142F8C29LL, isinstance) {
        return (t_isinstance(params.rvalAt(0)));
      }
      HASH_GUARD(0x1D6B8CA358B49929LL, getextensionname) {
        return (t_getextensionname());
      }
      break;
    case 42:
      HASH_GUARD(0x16BA16CE6488AAAALL, getmethods) {
        return (t_getmethods());
      }
      HASH_GUARD(0x226F6E80CECD3CAALL, getconstructor) {
        return (t_getconstructor());
      }
      break;
    case 46:
      HASH_GUARD(0x3C882D4A895F612ELL, getstaticpropertyvalue) {
        int count = params.size();
        if (count <= 1) return (t_getstaticpropertyvalue(params.rvalAt(0)));
        return (t_getstaticpropertyvalue(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x06FB6A7DC3D795AELL, isfinal) {
        return (t_isfinal());
      }
      break;
    case 48:
      HASH_GUARD(0x30A86FCA01FE7030LL, newinstance) {
        int count = params.size();
        if (count <= 0) return (t_newinstance(count));
        return (t_newinstance(count,params.slice(0, count - 0, false)));
      }
      break;
    case 52:
      HASH_GUARD(0x3DB53E1FBD3C0734LL, getconstant) {
        return (t_getconstant(params.rvalAt(0)));
      }
      break;
    case 54:
      HASH_GUARD(0x0D81ECE253A3B5B6LL, getmethod) {
        return (t_getmethod(params.rvalAt(0)));
      }
      break;
    case 55:
      HASH_GUARD(0x7460D945DA32FDB7LL, isabstract) {
        return (t_isabstract());
      }
      break;
    case 59:
      HASH_GUARD(0x25D24435915E6E3BLL, getextension) {
        return (t_getextension());
      }
      break;
    case 60:
      HASH_GUARD(0x0D8AAD6BA2BBCD3CLL, isinstantiable) {
        return (t_isinstantiable());
      }
      break;
    case 63:
      HASH_GUARD(0x54C2DC04C4A62B3FLL, hasconstant) {
        return (t_hasconstant(params.rvalAt(0)));
      }
      break;
    case 67:
      HASH_GUARD(0x67C15E3D98C00B43LL, getinterfaces) {
        return (t_getinterfaces());
      }
      break;
    case 68:
      HASH_GUARD(0x1EB679C3602F4B44LL, getproperties) {
        return (t_getproperties());
      }
      break;
    case 71:
      HASH_GUARD(0x0FD73627FB023047LL, getproperty) {
        return (t_getproperty(params.rvalAt(0)));
      }
      break;
    case 77:
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        return (t_getfilename());
      }
      break;
    case 78:
      HASH_GUARD(0x7D5A57B5370B68CELL, isiterateable) {
        return (t_isiterateable());
      }
      break;
    case 79:
      HASH_GUARD(0x48FDF6C5835C64CFLL, isinternal) {
        return (t_isinternal());
      }
      break;
    case 85:
      HASH_GUARD(0x1CC71CB013143955LL, getconstants) {
        return (t_getconstants());
      }
      break;
    case 95:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0)), null);
      }
      break;
    case 100:
      HASH_GUARD(0x6A6B8BECAE7D4164LL, isuserdefined) {
        return (t_isuserdefined());
      }
      break;
    case 101:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 102:
      HASH_GUARD(0x2735DCC254EE5C66LL, newinstanceargs) {
        return (t_newinstanceargs(params.rvalAt(0)));
      }
      break;
    case 104:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(o_getClassName(), params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 112:
      HASH_GUARD(0x6C19E85007BC4570LL, getendline) {
        return (t_getendline());
      }
      break;
    case 113:
      HASH_GUARD(0x07ECA928E37717F1LL, setstaticpropertyvalue) {
        return (t_setstaticpropertyvalue(params.rvalAt(0), params.rvalAt(1)), null);
      }
      break;
    case 114:
      HASH_GUARD(0x74F7FEDE16957472LL, getparentclass) {
        return (t_getparentclass());
      }
      break;
    case 120:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        return (t_getdoccomment());
      }
      break;
    case 123:
      HASH_GUARD(0x28DC702215C7D6FBLL, implementsinterface) {
        return (t_implementsinterface(params.rvalAt(0)));
      }
      break;
    case 126:
      HASH_GUARD(0x373333991926C97ELL, issubclassof) {
        return (t_issubclassof(params.rvalAt(0)));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_reflectionclass::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 4:
      HASH_GUARD(0x39C1BB731CB1CB04LL, getstartline) {
        return (t_getstartline());
      }
      break;
    case 15:
      HASH_GUARD(0x40C7B30DCB439C8FLL, hasproperty) {
        return (t_hasproperty(a0));
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 20:
      HASH_GUARD(0x24253EBA491D6014LL, getmodifiers) {
        return (t_getmodifiers());
      }
      break;
    case 24:
      HASH_GUARD(0x21820E7AA4733998LL, hasmethod) {
        return (t_hasmethod(a0));
      }
      break;
    case 27:
      HASH_GUARD(0x0F1AD0A8EC4C229BLL, getdefaultproperties) {
        return (t_getdefaultproperties());
      }
      break;
    case 30:
      HASH_GUARD(0x1BC5F3D87676509ELL, isinterface) {
        return (t_isinterface());
      }
      break;
    case 34:
      HASH_GUARD(0x323D9BCB05797B22LL, getstaticproperties) {
        return (t_getstaticproperties());
      }
      break;
    case 41:
      HASH_GUARD(0x030CE1D6142F8C29LL, isinstance) {
        return (t_isinstance(a0));
      }
      HASH_GUARD(0x1D6B8CA358B49929LL, getextensionname) {
        return (t_getextensionname());
      }
      break;
    case 42:
      HASH_GUARD(0x16BA16CE6488AAAALL, getmethods) {
        return (t_getmethods());
      }
      HASH_GUARD(0x226F6E80CECD3CAALL, getconstructor) {
        return (t_getconstructor());
      }
      break;
    case 46:
      HASH_GUARD(0x3C882D4A895F612ELL, getstaticpropertyvalue) {
        if (count <= 1) return (t_getstaticpropertyvalue(a0));
        return (t_getstaticpropertyvalue(a0, a1));
      }
      HASH_GUARD(0x06FB6A7DC3D795AELL, isfinal) {
        return (t_isfinal());
      }
      break;
    case 48:
      HASH_GUARD(0x30A86FCA01FE7030LL, newinstance) {
        if (count <= 0) return (t_newinstance(count));
        Array params;
        if (count >= 1) params.append(a0);
        if (count >= 2) params.append(a1);
        if (count >= 3) params.append(a2);
        if (count >= 4) params.append(a3);
        if (count >= 5) params.append(a4);
        if (count >= 6) params.append(a5);
        return (t_newinstance(count,params));
      }
      break;
    case 52:
      HASH_GUARD(0x3DB53E1FBD3C0734LL, getconstant) {
        return (t_getconstant(a0));
      }
      break;
    case 54:
      HASH_GUARD(0x0D81ECE253A3B5B6LL, getmethod) {
        return (t_getmethod(a0));
      }
      break;
    case 55:
      HASH_GUARD(0x7460D945DA32FDB7LL, isabstract) {
        return (t_isabstract());
      }
      break;
    case 59:
      HASH_GUARD(0x25D24435915E6E3BLL, getextension) {
        return (t_getextension());
      }
      break;
    case 60:
      HASH_GUARD(0x0D8AAD6BA2BBCD3CLL, isinstantiable) {
        return (t_isinstantiable());
      }
      break;
    case 63:
      HASH_GUARD(0x54C2DC04C4A62B3FLL, hasconstant) {
        return (t_hasconstant(a0));
      }
      break;
    case 67:
      HASH_GUARD(0x67C15E3D98C00B43LL, getinterfaces) {
        return (t_getinterfaces());
      }
      break;
    case 68:
      HASH_GUARD(0x1EB679C3602F4B44LL, getproperties) {
        return (t_getproperties());
      }
      break;
    case 71:
      HASH_GUARD(0x0FD73627FB023047LL, getproperty) {
        return (t_getproperty(a0));
      }
      break;
    case 77:
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        return (t_getfilename());
      }
      break;
    case 78:
      HASH_GUARD(0x7D5A57B5370B68CELL, isiterateable) {
        return (t_isiterateable());
      }
      break;
    case 79:
      HASH_GUARD(0x48FDF6C5835C64CFLL, isinternal) {
        return (t_isinternal());
      }
      break;
    case 85:
      HASH_GUARD(0x1CC71CB013143955LL, getconstants) {
        return (t_getconstants());
      }
      break;
    case 95:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0), null);
      }
      break;
    case 100:
      HASH_GUARD(0x6A6B8BECAE7D4164LL, isuserdefined) {
        return (t_isuserdefined());
      }
      break;
    case 101:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 102:
      HASH_GUARD(0x2735DCC254EE5C66LL, newinstanceargs) {
        return (t_newinstanceargs(a0));
      }
      break;
    case 104:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(o_getClassName(), a0, a1));
      }
      break;
    case 112:
      HASH_GUARD(0x6C19E85007BC4570LL, getendline) {
        return (t_getendline());
      }
      break;
    case 113:
      HASH_GUARD(0x07ECA928E37717F1LL, setstaticpropertyvalue) {
        return (t_setstaticpropertyvalue(a0, a1), null);
      }
      break;
    case 114:
      HASH_GUARD(0x74F7FEDE16957472LL, getparentclass) {
        return (t_getparentclass());
      }
      break;
    case 120:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        return (t_getdoccomment());
      }
      break;
    case 123:
      HASH_GUARD(0x28DC702215C7D6FBLL, implementsinterface) {
        return (t_implementsinterface(a0));
      }
      break;
    case 126:
      HASH_GUARD(0x373333991926C97ELL, issubclassof) {
        return (t_issubclassof(a0));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_reflectionclass::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 1) {
    case 0:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(c, params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_reflectionclass::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 4:
      HASH_GUARD(0x39C1BB731CB1CB04LL, getstartline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getstartline());
      }
      break;
    case 15:
      HASH_GUARD(0x40C7B30DCB439C8FLL, hasproperty) {
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
        return (t_hasproperty(a0));
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
    case 20:
      HASH_GUARD(0x24253EBA491D6014LL, getmodifiers) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmodifiers());
      }
      break;
    case 24:
      HASH_GUARD(0x21820E7AA4733998LL, hasmethod) {
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
        return (t_hasmethod(a0));
      }
      break;
    case 27:
      HASH_GUARD(0x0F1AD0A8EC4C229BLL, getdefaultproperties) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getdefaultproperties());
      }
      break;
    case 30:
      HASH_GUARD(0x1BC5F3D87676509ELL, isinterface) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isinterface());
      }
      break;
    case 34:
      HASH_GUARD(0x323D9BCB05797B22LL, getstaticproperties) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getstaticproperties());
      }
      break;
    case 41:
      HASH_GUARD(0x030CE1D6142F8C29LL, isinstance) {
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
        return (t_isinstance(a0));
      }
      HASH_GUARD(0x1D6B8CA358B49929LL, getextensionname) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getextensionname());
      }
      break;
    case 42:
      HASH_GUARD(0x16BA16CE6488AAAALL, getmethods) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmethods());
      }
      HASH_GUARD(0x226F6E80CECD3CAALL, getconstructor) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getconstructor());
      }
      break;
    case 46:
      HASH_GUARD(0x3C882D4A895F612ELL, getstaticpropertyvalue) {
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
        if (count <= 1) return (t_getstaticpropertyvalue(a0));
        return (t_getstaticpropertyvalue(a0, a1));
      }
      HASH_GUARD(0x06FB6A7DC3D795AELL, isfinal) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isfinal());
      }
      break;
    case 48:
      HASH_GUARD(0x30A86FCA01FE7030LL, newinstance) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        Array vargs;
        for (; it != params.end(); ++it) {
          vargs.append((*it)->eval(env));
        }
        int count = params.size();
        if (count <= 0) return (t_newinstance(count));
        return (t_newinstance(count,vargs));
      }
      break;
    case 52:
      HASH_GUARD(0x3DB53E1FBD3C0734LL, getconstant) {
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
        return (t_getconstant(a0));
      }
      break;
    case 54:
      HASH_GUARD(0x0D81ECE253A3B5B6LL, getmethod) {
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
        return (t_getmethod(a0));
      }
      break;
    case 55:
      HASH_GUARD(0x7460D945DA32FDB7LL, isabstract) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isabstract());
      }
      break;
    case 59:
      HASH_GUARD(0x25D24435915E6E3BLL, getextension) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getextension());
      }
      break;
    case 60:
      HASH_GUARD(0x0D8AAD6BA2BBCD3CLL, isinstantiable) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isinstantiable());
      }
      break;
    case 63:
      HASH_GUARD(0x54C2DC04C4A62B3FLL, hasconstant) {
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
        return (t_hasconstant(a0));
      }
      break;
    case 67:
      HASH_GUARD(0x67C15E3D98C00B43LL, getinterfaces) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getinterfaces());
      }
      break;
    case 68:
      HASH_GUARD(0x1EB679C3602F4B44LL, getproperties) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getproperties());
      }
      break;
    case 71:
      HASH_GUARD(0x0FD73627FB023047LL, getproperty) {
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
        return (t_getproperty(a0));
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
    case 78:
      HASH_GUARD(0x7D5A57B5370B68CELL, isiterateable) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isiterateable());
      }
      break;
    case 79:
      HASH_GUARD(0x48FDF6C5835C64CFLL, isinternal) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isinternal());
      }
      break;
    case 85:
      HASH_GUARD(0x1CC71CB013143955LL, getconstants) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getconstants());
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
    case 100:
      HASH_GUARD(0x6A6B8BECAE7D4164LL, isuserdefined) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isuserdefined());
      }
      break;
    case 101:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getname());
      }
      break;
    case 102:
      HASH_GUARD(0x2735DCC254EE5C66LL, newinstanceargs) {
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
        return (t_newinstanceargs(a0));
      }
      break;
    case 104:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
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
        return (ti_export(o_getClassName(), a0, a1));
      }
      break;
    case 112:
      HASH_GUARD(0x6C19E85007BC4570LL, getendline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getendline());
      }
      break;
    case 113:
      HASH_GUARD(0x07ECA928E37717F1LL, setstaticpropertyvalue) {
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
        return (t_setstaticpropertyvalue(a0, a1), null);
      }
      break;
    case 114:
      HASH_GUARD(0x74F7FEDE16957472LL, getparentclass) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getparentclass());
      }
      break;
    case 120:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getdoccomment());
      }
      break;
    case 123:
      HASH_GUARD(0x28DC702215C7D6FBLL, implementsinterface) {
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
        return (t_implementsinterface(a0));
      }
      break;
    case 126:
      HASH_GUARD(0x373333991926C97ELL, issubclassof) {
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
        return (t_issubclassof(a0));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_reflectionclass::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 1) {
    case 0:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
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
        return (ti_export(c, a0, a1));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_reflectionclass$os_get(const char *s) {
  return c_reflectionclass::os_get(s, -1);
}
Variant &cw_reflectionclass$os_lval(const char *s) {
  return c_reflectionclass::os_lval(s, -1);
}
Variant cw_reflectionclass$os_constant(const char *s) {
  return c_reflectionclass::os_constant(s);
}
Variant cw_reflectionclass$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_reflectionclass::os_invoke(c, s, params, -1, fatal);
}
void c_reflectionclass::init() {
  m_name = null;
  m_info = null;
}
/* SRC: classes/reflection.php line 204 */
void c_reflectionclass::t___construct(Variant v_name) {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::__construct);
  bool oldInCtor = gasInCtor(true);
  p_reflectionclass v_p;
  Primitive v_interface = 0;
  Variant v__;

  m_info = x_hphp_get_class_info(v_name);
  if (empty(m_info)) {
    if (x_is_object(v_name)) v_name = x_get_class(v_name);
    throw_exception(((Object)(p_reflectionexception(p_reflectionexception(NEWOBJ(c_reflectionexception)())->create(concat3("Class ", toString(v_name), " does not exist"))))));
  }
  m_name = m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL);
  if (!(empty(m_info, "parent", 0x16E2F26FFB10FD8CLL))) {
    ((Object)(v_p = ((Object)(p_reflectionclass(p_reflectionclass(NEWOBJ(c_reflectionclass)())->create(m_info.rvalAt("parent", 0x16E2F26FFB10FD8CLL)))))));
    lval(m_info.lvalAt("interfaces", 0x0C5BD661CFB8E254LL)) += v_p->m_info.rvalAt("interfaces", 0x0C5BD661CFB8E254LL);
    lval(m_info.lvalAt("properties", 0x5D7B5CC390269404LL)) += v_p->m_info.rvalAt("properties", 0x5D7B5CC390269404LL);
    lval(m_info.lvalAt("methods", 0x2A7E90235B229AD5LL)) += v_p->m_info.rvalAt("methods", 0x2A7E90235B229AD5LL);
    lval(m_info.lvalAt("constants", 0x3A127EB7623AE369LL)) += v_p->m_info.rvalAt("constants", 0x3A127EB7623AE369LL);
  }
  {
    LOOP_COUNTER(7);
    Variant map8 = m_info.rvalAt("interfaces", 0x0C5BD661CFB8E254LL);
    for (ArrayIterPtr iter9 = map8.begin("reflectionclass"); !iter9->end(); iter9->next()) {
      LOOP_COUNTER_CHECK(7);
      v__ = iter9->second();
      v_interface = iter9->first();
      {
        ((Object)(v_p = ((Object)(p_reflectionclass(p_reflectionclass(NEWOBJ(c_reflectionclass)())->create(v_interface))))));
        lval(m_info.lvalAt("methods", 0x2A7E90235B229AD5LL)) += v_p->m_info.rvalAt("methods", 0x2A7E90235B229AD5LL);
      }
    }
  }
  gasInCtor(oldInCtor);
} /* function */
/* SRC: classes/reflection.php line 226 */
String c_reflectionclass::t___tostring() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::__toString);
  return toString(null);
} /* function */
/* SRC: classes/reflection.php line 229 */
Variant c_reflectionclass::ti_export(const char* cls, CVarRef v_name, CVarRef v_ret) {
  STATIC_METHOD_INJECTION(ReflectionClass, ReflectionClass::export);
  p_reflectionclass v_obj;
  String v_str;

  ((Object)(v_obj = ((Object)(p_reflectionclass(p_reflectionclass(NEWOBJ(c_reflectionclass)())->create(v_name))))));
  v_str = toString(((Object)(v_obj)));
  if (toBoolean(v_ret)) {
    return v_str;
  }
  print(v_str);
  return null;
} /* function */
/* SRC: classes/reflection.php line 238 */
Variant c_reflectionclass::t_getname() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getName);
  return m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL);
} /* function */
/* SRC: classes/reflection.php line 242 */
Variant c_reflectionclass::t_isinternal() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::isInternal);
  return m_info.rvalAt("internal", 0x575D95D69332A8ACLL);
} /* function */
/* SRC: classes/reflection.php line 246 */
bool c_reflectionclass::t_isuserdefined() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::isUserDefined);
  return !(toBoolean(m_info.rvalAt("internal", 0x575D95D69332A8ACLL)));
} /* function */
/* SRC: classes/reflection.php line 250 */
bool c_reflectionclass::t_isinstantiable() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::isInstantiable);
  return !(toBoolean(m_info.rvalAt("abstract", 0x475C06CE12C8A8A6LL)));
} /* function */
/* SRC: classes/reflection.php line 254 */
bool c_reflectionclass::t_hasconstant(CVarRef v_name) {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::hasConstant);
  return isset(m_info.rvalAt("constants", 0x3A127EB7623AE369LL), v_name);
} /* function */
/* SRC: classes/reflection.php line 258 */
bool c_reflectionclass::t_hasmethod(CVarRef v_name) {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::hasMethod);
  return isset(m_info.rvalAt("methods", 0x2A7E90235B229AD5LL), v_name);
} /* function */
/* SRC: classes/reflection.php line 262 */
bool c_reflectionclass::t_hasproperty(CVarRef v_name) {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::hasProperty);
  return isset(m_info.rvalAt("properties", 0x5D7B5CC390269404LL), v_name);
} /* function */
/* SRC: classes/reflection.php line 266 */
Variant c_reflectionclass::t_getfilename() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getFileName);
  return m_info.rvalAt("file", 0x612E37678CE7DB5BLL);
} /* function */
/* SRC: classes/reflection.php line 270 */
Variant c_reflectionclass::t_getstartline() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getStartLine);
  return m_info.rvalAt("line1", 0x7E7BD613D4C67725LL);
} /* function */
/* SRC: classes/reflection.php line 274 */
Variant c_reflectionclass::t_getendline() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getEndLine);
  return m_info.rvalAt("line2", 0x641B12C5BEFE32A8LL);
} /* function */
/* SRC: classes/reflection.php line 278 */
Variant c_reflectionclass::t_getdoccomment() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getDocComment);
  return m_info.rvalAt("doc", 0x16758C759CFA17A6LL);
} /* function */
/* SRC: classes/reflection.php line 282 */
Variant c_reflectionclass::t_getconstructor() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getConstructor);
  if (t_hasmethod("__construct")) {
    return ((Object)(t_getmethod("__construct")));
  }
  if (t_hasmethod(m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL))) {
    return ((Object)(t_getmethod(m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL))));
  }
  return null;
} /* function */
/* SRC: classes/reflection.php line 292 */
p_reflectionmethod c_reflectionclass::t_getmethod(CVarRef v_name) {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getMethod);
  String v_lname;
  Variant v_class;
  p_reflectionmethod v_ret;

  v_lname = x_strtolower(toString(v_name));
  if (!(isset(m_info.rvalAt("methods", 0x2A7E90235B229AD5LL), v_lname))) {
    v_class = m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL);
    throw_exception(((Object)(p_reflectionexception(p_reflectionexception(NEWOBJ(c_reflectionexception)())->create(concat5("Method ", toString(v_class), "::", toString(v_name), " does not exist"))))));
  }
  ((Object)(v_ret = ((Object)(p_reflectionmethod(p_reflectionmethod(NEWOBJ(c_reflectionmethod)())->create(null, null))))));
  v_ret->m_info = m_info.rvalAt("methods", 0x2A7E90235B229AD5LL).rvalAt(v_lname);
  v_ret->m_name = v_lname;
  v_ret->m_class = m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL);
  return ((Object)(v_ret));
} /* function */
/* SRC: classes/reflection.php line 306 */
Array c_reflectionclass::t_getmethods() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getMethods);
  Array v_ret;
  Primitive v_name = 0;
  Variant v__;

  v_ret = SystemScalarArrays::ssa_[0];
  {
    LOOP_COUNTER(10);
    Variant map11 = m_info.rvalAt("methods", 0x2A7E90235B229AD5LL);
    for (ArrayIterPtr iter12 = map11.begin("reflectionclass"); !iter12->end(); iter12->next()) {
      LOOP_COUNTER_CHECK(10);
      v__ = iter12->second();
      v_name = iter12->first();
      {
        v_ret.append((((Object)(t_getmethod(v_name)))));
      }
    }
  }
  return v_ret;
} /* function */
/* SRC: classes/reflection.php line 314 */
p_reflectionproperty c_reflectionclass::t_getproperty(CVarRef v_name) {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getProperty);
  Variant v_class;
  p_reflectionproperty v_ret;

  if (!(isset(m_info.rvalAt("properties", 0x5D7B5CC390269404LL), v_name))) {
    v_class = m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL);
    throw_exception(((Object)(p_reflectionexception(p_reflectionexception(NEWOBJ(c_reflectionexception)())->create(concat5("Property ", toString(v_class), "::", toString(v_name), " does not exist"))))));
  }
  ((Object)(v_ret = ((Object)(p_reflectionproperty(p_reflectionproperty(NEWOBJ(c_reflectionproperty)())->create(null, null))))));
  v_ret->m_info = m_info.rvalAt("properties", 0x5D7B5CC390269404LL).rvalAt(v_name);
  v_ret->m_name = v_name;
  v_ret->m_class = m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL);
  return ((Object)(v_ret));
} /* function */
/* SRC: classes/reflection.php line 327 */
Array c_reflectionclass::t_getproperties() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getProperties);
  Array v_ret;
  Primitive v_name = 0;
  Variant v__;

  v_ret = SystemScalarArrays::ssa_[0];
  {
    LOOP_COUNTER(13);
    Variant map14 = m_info.rvalAt("properties", 0x5D7B5CC390269404LL);
    for (ArrayIterPtr iter15 = map14.begin("reflectionclass"); !iter15->end(); iter15->next()) {
      LOOP_COUNTER_CHECK(13);
      v__ = iter15->second();
      v_name = iter15->first();
      {
        v_ret.append((((Object)(t_getproperty(v_name)))));
      }
    }
  }
  return v_ret;
} /* function */
/* SRC: classes/reflection.php line 335 */
Variant c_reflectionclass::t_getconstants() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getConstants);
  return m_info.rvalAt("constants", 0x3A127EB7623AE369LL);
} /* function */
/* SRC: classes/reflection.php line 339 */
Variant c_reflectionclass::t_getconstant(CVarRef v_name) {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getConstant);
  Variant v_class;

  if (!(isset(m_info.rvalAt("constants", 0x3A127EB7623AE369LL), v_name))) {
    v_class = m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL);
    throw_exception(((Object)(p_reflectionexception(p_reflectionexception(NEWOBJ(c_reflectionexception)())->create(concat5("Class constant ", toString(v_class), "::", toString(v_name), " does not exist"))))));
  }
  return m_info.rvalAt("constants", 0x3A127EB7623AE369LL).rvalAt(v_name);
} /* function */
/* SRC: classes/reflection.php line 347 */
Variant c_reflectionclass::t_getinterfaces() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getInterfaces);
  Variant v_ret;
  Primitive v_name = 0;
  Variant v__;
  p_reflectionclass v_cls;

  v_ret = SystemScalarArrays::ssa_[0];
  {
    LOOP_COUNTER(16);
    Variant map17 = m_info.rvalAt("interfaces", 0x0C5BD661CFB8E254LL);
    for (ArrayIterPtr iter18 = map17.begin("reflectionclass"); !iter18->end(); iter18->next()) {
      LOOP_COUNTER_CHECK(16);
      v__ = iter18->second();
      v_name = iter18->first();
      {
        ((Object)(v_cls = ((Object)(p_reflectionclass(p_reflectionclass(NEWOBJ(c_reflectionclass)())->create(v_name))))));
        if (toBoolean(v_cls->t_isinterface())) {
          v_ret.set(v_name, (((Object)(v_cls))));
        }
      }
    }
  }
  return v_ret;
} /* function */
/* SRC: classes/reflection.php line 358 */
Variant c_reflectionclass::t_isinterface() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::isInterface);
  return m_info.rvalAt("interface", 0x448B10707228E959LL);
} /* function */
/* SRC: classes/reflection.php line 362 */
Variant c_reflectionclass::t_isabstract() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::isAbstract);
  return m_info.rvalAt("abstract", 0x475C06CE12C8A8A6LL);
} /* function */
/* SRC: classes/reflection.php line 366 */
Variant c_reflectionclass::t_isfinal() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::isFinal);
  return m_info.rvalAt("final", 0x5192930B2145036ELL);
} /* function */
/* SRC: classes/reflection.php line 370 */
Variant c_reflectionclass::t_getmodifiers() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getModifiers);
  return m_info.rvalAt("modifiers", 0x2CAF244C9F244C80LL);
} /* function */
/* SRC: classes/reflection.php line 374 */
bool c_reflectionclass::t_isinstance(CVarRef v_obj) {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::isInstance);
  return x_hphp_instanceof(toObject(v_obj), toString(m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL)));
} /* function */
/* SRC: classes/reflection.php line 378 */
Object c_reflectionclass::t_newinstance(int num_args, Array args /* = Array() */) {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::newInstance);
  Array v_args;

  v_args = func_get_args(num_args, Array(),args);
  return x_hphp_create_object(toString(m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL)), v_args);
} /* function */
/* SRC: classes/reflection.php line 383 */
Object c_reflectionclass::t_newinstanceargs(CVarRef v_args) {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::newInstanceArgs);
  return x_hphp_create_object(toString(m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL)), toArray(v_args));
} /* function */
/* SRC: classes/reflection.php line 387 */
Variant c_reflectionclass::t_getparentclass() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getParentClass);
  if (empty(m_info, "parent", 0x16E2F26FFB10FD8CLL)) {
    return false;
  }
  return ((Object)(p_reflectionclass(p_reflectionclass(NEWOBJ(c_reflectionclass)())->create(m_info.rvalAt("parent", 0x16E2F26FFB10FD8CLL)))));
} /* function */
/* SRC: classes/reflection.php line 394 */
Variant c_reflectionclass::t_issubclassof(Variant v_cls) {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::isSubclassOf);
  Variant eo_0;
  Variant eo_1;
  Primitive v_name = 0;
  Variant v__;

  if (instanceOf(v_cls, "ReflectionClass")) {
    v_cls = v_cls.o_get("info", 0x59E9384E33988B3ELL).rvalAt("name", 0x0BCDB293DC3CBDDCLL);
  }
  {
    LOOP_COUNTER(19);
    Variant map20 = m_info.rvalAt("interfaces", 0x0C5BD661CFB8E254LL);
    for (ArrayIterPtr iter21 = map20.begin("reflectionclass"); !iter21->end(); iter21->next()) {
      LOOP_COUNTER_CHECK(19);
      v__ = iter21->second();
      v_name = iter21->first();
      {
        if (equal(x_strcasecmp(toString(v_cls), toString(v_name)), 0LL)) {
          return true;
        }
      }
    }
  }
  if (empty(m_info, "parent", 0x16E2F26FFB10FD8CLL)) {
    return false;
  }
  if (equal(x_strcasecmp(toString(v_cls), toString(m_info.rvalAt("parent", 0x16E2F26FFB10FD8CLL))), 0LL)) {
    return true;
  }
  return (assignCallTemp(eo_0, toObject(t_getparentclass())),assignCallTemp(eo_1, ref(v_cls)),eo_0.o_invoke("isSubclassOf", Array(ArrayInit(1).set(0, ArrayElement(eo_1)).create()), 0x373333991926C97ELL));
} /* function */
/* SRC: classes/reflection.php line 412 */
Variant c_reflectionclass::t_getstaticproperties() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getStaticProperties);
  Variant v_ret;
  Variant v_prop;

  v_ret = SystemScalarArrays::ssa_[0];
  {
    LOOP_COUNTER(22);
    Variant map23 = t_getproperties();
    for (ArrayIterPtr iter24 = map23.begin("reflectionclass"); !iter24->end(); iter24->next()) {
      LOOP_COUNTER_CHECK(22);
      v_prop = iter24->second();
      {
        if (toBoolean(v_prop.o_invoke_few_args("isStatic", 0x7A15DC56E8CC0B19LL, 0))) {
          v_ret.set(v_prop.o_get("name", 0x0BCDB293DC3CBDDCLL), (v_prop));
        }
      }
    }
  }
  return v_ret;
} /* function */
/* SRC: classes/reflection.php line 422 */
Variant c_reflectionclass::t_getstaticpropertyvalue(CVarRef v_name, CVarRef v_default /* = null_variant */) {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getStaticPropertyValue);
  if (t_hasproperty(v_name) && toBoolean(t_getproperty(v_name)->t_isstatic())) {
    return x_hphp_get_static_property(toString(m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL)), toString(v_name));
  }
  return v_default;
} /* function */
/* SRC: classes/reflection.php line 430 */
void c_reflectionclass::t_setstaticpropertyvalue(CVarRef v_name, CVarRef v_value) {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::setStaticPropertyValue);
  x_hphp_set_static_property(toString(m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL)), toString(v_name), v_value);
} /* function */
/* SRC: classes/reflection.php line 434 */
Variant c_reflectionclass::t_getdefaultproperties() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getDefaultProperties);
  Variant v_ret;
  Variant v_prop;

  v_ret = SystemScalarArrays::ssa_[0];
  {
    LOOP_COUNTER(25);
    Variant map26 = t_getproperties();
    for (ArrayIterPtr iter27 = map26.begin("reflectionclass"); !iter27->end(); iter27->next()) {
      LOOP_COUNTER_CHECK(25);
      v_prop = iter27->second();
      {
        if (toBoolean(v_prop.o_invoke_few_args("isDefault", 0x384A52597AB11F15LL, 0))) {
          v_ret.set(v_prop.o_get("name", 0x0BCDB293DC3CBDDCLL), (v_prop));
        }
      }
    }
  }
  return v_ret;
} /* function */
/* SRC: classes/reflection.php line 444 */
Variant c_reflectionclass::t_isiterateable() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::isIterateable);
  return t_issubclassof("ArrayAccess");
} /* function */
/* SRC: classes/reflection.php line 448 */
bool c_reflectionclass::t_implementsinterface(Variant v_cls) {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::implementsInterface);
  Primitive v_name = 0;
  Variant v__;

  if (instanceOf(v_cls, "ReflectionClass")) {
    v_cls = v_cls.o_get("info", 0x59E9384E33988B3ELL).rvalAt("name", 0x0BCDB293DC3CBDDCLL);
  }
  {
    LOOP_COUNTER(28);
    Variant map29 = m_info.rvalAt("interfaces", 0x0C5BD661CFB8E254LL);
    for (ArrayIterPtr iter30 = map29.begin("reflectionclass"); !iter30->end(); iter30->next()) {
      LOOP_COUNTER_CHECK(28);
      v__ = iter30->second();
      v_name = iter30->first();
      {
        if (equal(x_strcasecmp(toString(v_cls), toString(v_name)), 0LL)) {
          return true;
        }
      }
    }
  }
  return false;
} /* function */
/* SRC: classes/reflection.php line 460 */
Variant c_reflectionclass::t_getextension() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getExtension);
  return m_info.rvalAt("extension", 0x3E8C2484E9BF4500LL);
} /* function */
/* SRC: classes/reflection.php line 464 */
Variant c_reflectionclass::t_getextensionname() {
  INSTANCE_METHOD_INJECTION(ReflectionClass, ReflectionClass::getExtensionName);
  return m_info.rvalAt("extension", 0x3E8C2484E9BF4500LL).o_invoke_few_args("getName", 0x23F51CDECC198965LL, 0);
} /* function */
/* SRC: classes/reflection.php line 676 */
Variant c_reflectionextension::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_reflectionextension::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_reflectionextension::o_get(ArrayElementVec &props) const {
  props.push_back(NEW(ArrayElement)("name", m_name));
  props.push_back(NEW(ArrayElement)("info", m_info.isReferenced() ? ref(m_info) : m_info));
  c_ObjectData::o_get(props);
}
bool c_reflectionextension::o_exists(CStrRef s, int64 hash) const {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_EXISTS_STRING(0x0BCDB293DC3CBDDCLL, name, 4);
      break;
    case 2:
      HASH_EXISTS_STRING(0x59E9384E33988B3ELL, info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_exists(s, hash);
}
Variant c_reflectionextension::o_get(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_RETURN_STRING(0x0BCDB293DC3CBDDCLL, m_name,
                         name, 4);
      break;
    case 2:
      HASH_RETURN_STRING(0x59E9384E33988B3ELL, m_info,
                         info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_get(s, hash);
}
Variant c_reflectionextension::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_SET_STRING(0x0BCDB293DC3CBDDCLL, m_name,
                      name, 4);
      break;
    case 2:
      HASH_SET_STRING(0x59E9384E33988B3ELL, m_info,
                      info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_reflectionextension::o_lval(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 1) {
    case 0:
      HASH_RETURN_STRING(0x59E9384E33988B3ELL, m_info,
                         info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_lval(s, hash);
}
Variant c_reflectionextension::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(reflectionextension)
ObjectData *c_reflectionextension::create(Variant v_name) {
  init();
  t___construct(v_name);
  return this;
}
ObjectData *c_reflectionextension::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0)));
  } else return this;
}
void c_reflectionextension::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0)));
}
ObjectData *c_reflectionextension::cloneImpl() {
  c_reflectionextension *obj = NEW(c_reflectionextension)();
  cloneSet(obj);
  return obj;
}
void c_reflectionextension::cloneSet(c_reflectionextension *clone) {
  clone->m_name = m_name;
  clone->m_info = m_info.isReferenced() ? ref(m_info) : m_info;
  ObjectData::cloneSet(clone);
}
Variant c_reflectionextension::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 2:
      HASH_GUARD(0x0113D73FC859EDC2LL, getclasses) {
        return (t_getclasses());
      }
      break;
    case 5:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 8:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(o_getClassName(), params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 15:
      HASH_GUARD(0x652BDFA6E22F17AFLL, getfunctions) {
        return (t_getfunctions());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 20:
      HASH_GUARD(0x5CE2786E11341594LL, getclassnames) {
        return (t_getclassnames());
      }
      break;
    case 21:
      HASH_GUARD(0x306B5F4D1D03D335LL, getinientries) {
        return (t_getinientries());
      }
      HASH_GUARD(0x1CC71CB013143955LL, getconstants) {
        return (t_getconstants());
      }
      break;
    case 22:
      HASH_GUARD(0x7521E8833BE3D316LL, getversion) {
        return (t_getversion());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0)), null);
      }
      HASH_GUARD(0x0F2EF58F157D479FLL, info) {
        return (t_info());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_reflectionextension::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 2:
      HASH_GUARD(0x0113D73FC859EDC2LL, getclasses) {
        return (t_getclasses());
      }
      break;
    case 5:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 8:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(o_getClassName(), a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x652BDFA6E22F17AFLL, getfunctions) {
        return (t_getfunctions());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 20:
      HASH_GUARD(0x5CE2786E11341594LL, getclassnames) {
        return (t_getclassnames());
      }
      break;
    case 21:
      HASH_GUARD(0x306B5F4D1D03D335LL, getinientries) {
        return (t_getinientries());
      }
      HASH_GUARD(0x1CC71CB013143955LL, getconstants) {
        return (t_getconstants());
      }
      break;
    case 22:
      HASH_GUARD(0x7521E8833BE3D316LL, getversion) {
        return (t_getversion());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0), null);
      }
      HASH_GUARD(0x0F2EF58F157D479FLL, info) {
        return (t_info());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_reflectionextension::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 1) {
    case 0:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(c, params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_reflectionextension::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 2:
      HASH_GUARD(0x0113D73FC859EDC2LL, getclasses) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getclasses());
      }
      break;
    case 5:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getname());
      }
      break;
    case 8:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
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
        return (ti_export(o_getClassName(), a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x652BDFA6E22F17AFLL, getfunctions) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getfunctions());
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
    case 20:
      HASH_GUARD(0x5CE2786E11341594LL, getclassnames) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getclassnames());
      }
      break;
    case 21:
      HASH_GUARD(0x306B5F4D1D03D335LL, getinientries) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getinientries());
      }
      HASH_GUARD(0x1CC71CB013143955LL, getconstants) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getconstants());
      }
      break;
    case 22:
      HASH_GUARD(0x7521E8833BE3D316LL, getversion) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getversion());
      }
      break;
    case 31:
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
      HASH_GUARD(0x0F2EF58F157D479FLL, info) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_info());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_reflectionextension::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 1) {
    case 0:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
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
        return (ti_export(c, a0, a1));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_reflectionextension$os_get(const char *s) {
  return c_reflectionextension::os_get(s, -1);
}
Variant &cw_reflectionextension$os_lval(const char *s) {
  return c_reflectionextension::os_lval(s, -1);
}
Variant cw_reflectionextension$os_constant(const char *s) {
  return c_reflectionextension::os_constant(s);
}
Variant cw_reflectionextension$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_reflectionextension::os_invoke(c, s, params, -1, fatal);
}
void c_reflectionextension::init() {
  m_name = null;
  m_info = null;
}
/* SRC: classes/reflection.php line 680 */
void c_reflectionextension::t___construct(Variant v_name) {
  INSTANCE_METHOD_INJECTION(ReflectionExtension, ReflectionExtension::__construct);
  bool oldInCtor = gasInCtor(true);
  m_info = x_hphp_get_extension_info(toString(v_name));
  gasInCtor(oldInCtor);
} /* function */
/* SRC: classes/reflection.php line 684 */
String c_reflectionextension::t___tostring() {
  INSTANCE_METHOD_INJECTION(ReflectionExtension, ReflectionExtension::__toString);
  return toString(null);
} /* function */
/* SRC: classes/reflection.php line 687 */
Variant c_reflectionextension::ti_export(const char* cls, CVarRef v_name, CVarRef v_ret) {
  STATIC_METHOD_INJECTION(ReflectionExtension, ReflectionExtension::export);
  p_reflectionextension v_obj;
  String v_str;

  ((Object)(v_obj = ((Object)(p_reflectionextension(p_reflectionextension(NEWOBJ(c_reflectionextension)())->create(v_name))))));
  v_str = toString(((Object)(v_obj)));
  if (toBoolean(v_ret)) {
    return v_str;
  }
  print(v_str);
  return null;
} /* function */
/* SRC: classes/reflection.php line 696 */
Variant c_reflectionextension::t_getname() {
  INSTANCE_METHOD_INJECTION(ReflectionExtension, ReflectionExtension::getName);
  return m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL);
} /* function */
/* SRC: classes/reflection.php line 700 */
Variant c_reflectionextension::t_getversion() {
  INSTANCE_METHOD_INJECTION(ReflectionExtension, ReflectionExtension::getVersion);
  return m_info.rvalAt("version", 0x2AF5F0847CD91DB4LL);
} /* function */
/* SRC: classes/reflection.php line 704 */
Variant c_reflectionextension::t_getfunctions() {
  INSTANCE_METHOD_INJECTION(ReflectionExtension, ReflectionExtension::getFunctions);
  return m_info.rvalAt("functions", 0x345241CAC8396B02LL);
} /* function */
/* SRC: classes/reflection.php line 708 */
Variant c_reflectionextension::t_getconstants() {
  INSTANCE_METHOD_INJECTION(ReflectionExtension, ReflectionExtension::getConstants);
  return m_info.rvalAt("constants", 0x3A127EB7623AE369LL);
} /* function */
/* SRC: classes/reflection.php line 712 */
Variant c_reflectionextension::t_getinientries() {
  INSTANCE_METHOD_INJECTION(ReflectionExtension, ReflectionExtension::getINIEntries);
  return m_info.rvalAt("ini", 0x62EA1C97CEDEF5DCLL);
} /* function */
/* SRC: classes/reflection.php line 716 */
Variant c_reflectionextension::t_getclasses() {
  INSTANCE_METHOD_INJECTION(ReflectionExtension, ReflectionExtension::getClasses);
  return m_info.rvalAt("classes", 0x475D2D970415E4A0LL);
} /* function */
/* SRC: classes/reflection.php line 720 */
Array c_reflectionextension::t_getclassnames() {
  INSTANCE_METHOD_INJECTION(ReflectionExtension, ReflectionExtension::getClassNames);
  Array v_ret;
  Variant v_cls;

  v_ret = SystemScalarArrays::ssa_[0];
  {
    LOOP_COUNTER(31);
    Variant map32 = m_info.rvalAt("classes", 0x475D2D970415E4A0LL);
    for (ArrayIterPtr iter33 = map32.begin("reflectionextension"); !iter33->end(); iter33->next()) {
      LOOP_COUNTER_CHECK(31);
      v_cls = iter33->second();
      {
        v_ret.append((v_cls.o_invoke_few_args("getName", 0x23F51CDECC198965LL, 0)));
      }
    }
  }
  return v_ret;
} /* function */
/* SRC: classes/reflection.php line 728 */
Variant c_reflectionextension::t_info() {
  INSTANCE_METHOD_INJECTION(ReflectionExtension, ReflectionExtension::info);
  return m_info.rvalAt("info", 0x59E9384E33988B3ELL);
} /* function */
/* SRC: classes/reflection.php line 582 */
Variant c_reflectionmethod::os_get(const char *s, int64 hash) {
  return c_reflectionfunctionabstract::os_get(s, hash);
}
Variant &c_reflectionmethod::os_lval(const char *s, int64 hash) {
  return c_reflectionfunctionabstract::os_lval(s, hash);
}
void c_reflectionmethod::o_get(ArrayElementVec &props) const {
  props.push_back(NEW(ArrayElement)("name", m_name.isReferenced() ? ref(m_name) : m_name));
  props.push_back(NEW(ArrayElement)("class", m_class.isReferenced() ? ref(m_class) : m_class));
  c_reflectionfunctionabstract::o_get(props);
}
bool c_reflectionmethod::o_exists(CStrRef s, int64 hash) const {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_EXISTS_STRING(0x0BCDB293DC3CBDDCLL, name, 4);
      break;
    case 2:
      HASH_EXISTS_STRING(0x45397FE5C82DBD12LL, class, 5);
      break;
    default:
      break;
  }
  return c_reflectionfunctionabstract::o_exists(s, hash);
}
Variant c_reflectionmethod::o_get(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_RETURN_STRING(0x0BCDB293DC3CBDDCLL, m_name,
                         name, 4);
      break;
    case 2:
      HASH_RETURN_STRING(0x45397FE5C82DBD12LL, m_class,
                         class, 5);
      break;
    default:
      break;
  }
  return c_reflectionfunctionabstract::o_get(s, hash);
}
Variant c_reflectionmethod::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_SET_STRING(0x0BCDB293DC3CBDDCLL, m_name,
                      name, 4);
      break;
    case 2:
      HASH_SET_STRING(0x45397FE5C82DBD12LL, m_class,
                      class, 5);
      break;
    default:
      break;
  }
  return c_reflectionfunctionabstract::o_set(s, hash, v, forInit);
}
Variant &c_reflectionmethod::o_lval(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 3) {
    case 0:
      HASH_RETURN_STRING(0x0BCDB293DC3CBDDCLL, m_name,
                         name, 4);
      break;
    case 2:
      HASH_RETURN_STRING(0x45397FE5C82DBD12LL, m_class,
                         class, 5);
      break;
    default:
      break;
  }
  return c_reflectionfunctionabstract::o_lval(s, hash);
}
Variant c_reflectionmethod::os_constant(const char *s) {
  return c_reflectionfunctionabstract::os_constant(s);
}
IMPLEMENT_CLASS(reflectionmethod)
ObjectData *c_reflectionmethod::create(Variant v_cls, Variant v_name) {
  init();
  t___construct(v_cls, v_name);
  return this;
}
ObjectData *c_reflectionmethod::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0), params.rvalAt(1)));
  } else return this;
}
void c_reflectionmethod::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0), params.rvalAt(1)));
}
ObjectData *c_reflectionmethod::cloneImpl() {
  c_reflectionmethod *obj = NEW(c_reflectionmethod)();
  cloneSet(obj);
  return obj;
}
void c_reflectionmethod::cloneSet(c_reflectionmethod *clone) {
  clone->m_name = m_name.isReferenced() ? ref(m_name) : m_name;
  clone->m_class = m_class.isReferenced() ? ref(m_class) : m_class;
  c_reflectionfunctionabstract::cloneSet(clone);
}
Variant c_reflectionmethod::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 2:
      HASH_GUARD(0x3FCE192CF6199942LL, invoke) {
        int count = params.size();
        if (count <= 1) return (t_invoke(count, params.rvalAt(0)));
        return (t_invoke(count,params.rvalAt(0), params.slice(1, count - 1, false)));
      }
      break;
    case 4:
      HASH_GUARD(0x3235AF57F23103C4LL, invokeargs) {
        return (t_invokeargs(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x39C1BB731CB1CB04LL, getstartline) {
        return (t_getstartline());
      }
      break;
    case 7:
      HASH_GUARD(0x51A20EA0E327F607LL, isdestructor) {
        return (t_isdestructor());
      }
      break;
    case 13:
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        return (t_getfilename());
      }
      break;
    case 15:
      HASH_GUARD(0x48FDF6C5835C64CFLL, isinternal) {
        return (t_isinternal());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 20:
      HASH_GUARD(0x24253EBA491D6014LL, getmodifiers) {
        return (t_getmodifiers());
      }
      break;
    case 25:
      HASH_GUARD(0x7A15DC56E8CC0B19LL, isstatic) {
        return (t_isstatic());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      break;
    case 32:
      HASH_GUARD(0x57D8DC34C9A03560LL, getnumberofparameters) {
        return (t_getnumberofparameters());
      }
      break;
    case 33:
      HASH_GUARD(0x4D637DECDBFA6221LL, getnumberofrequiredparameters) {
        return (t_getnumberofrequiredparameters());
      }
      break;
    case 34:
      HASH_GUARD(0x33A6C2CFBDB05EE2LL, getclosure) {
        return (t_getclosure());
      }
      break;
    case 35:
      HASH_GUARD(0x6ED51288559D6063LL, getdeclaringclass) {
        return (t_getdeclaringclass());
      }
      break;
    case 36:
      HASH_GUARD(0x6A6B8BECAE7D4164LL, isuserdefined) {
        return (t_isuserdefined());
      }
      break;
    case 37:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 40:
      HASH_GUARD(0x37FFB8F44A3329A8LL, getstaticvariables) {
        return (t_getstaticvariables());
      }
      HASH_GUARD(0x1A3AB3B0276D2668LL, returnsreference) {
        return (t_returnsreference());
      }
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(o_getClassName(), params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 42:
      HASH_GUARD(0x2D7209A590477CEALL, isprotected) {
        return (t_isprotected());
      }
      break;
    case 45:
      HASH_GUARD(0x3E62225132C2A32DLL, getparameters) {
        return (t_getparameters());
      }
      break;
    case 46:
      HASH_GUARD(0x06FB6A7DC3D795AELL, isfinal) {
        return (t_isfinal());
      }
      break;
    case 47:
      HASH_GUARD(0x37AAE0845E2F636FLL, isprivate) {
        return (t_isprivate());
      }
      break;
    case 48:
      HASH_GUARD(0x6C19E85007BC4570LL, getendline) {
        return (t_getendline());
      }
      break;
    case 55:
      HASH_GUARD(0x7460D945DA32FDB7LL, isabstract) {
        return (t_isabstract());
      }
      break;
    case 56:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        return (t_getdoccomment());
      }
      break;
    case 58:
      HASH_GUARD(0x654B5F965C5CAC7ALL, isconstructor) {
        return (t_isconstructor());
      }
      break;
    case 60:
      HASH_GUARD(0x2820F10358723B7CLL, ispublic) {
        return (t_ispublic());
      }
      break;
    default:
      break;
  }
  return c_reflectionfunctionabstract::o_invoke(s, params, hash, fatal);
}
Variant c_reflectionmethod::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 2:
      HASH_GUARD(0x3FCE192CF6199942LL, invoke) {
        if (count <= 1) return (t_invoke(count, a0));
        Array params;
        if (count >= 2) params.append(a1);
        if (count >= 3) params.append(a2);
        if (count >= 4) params.append(a3);
        if (count >= 5) params.append(a4);
        if (count >= 6) params.append(a5);
        return (t_invoke(count,a0, params));
      }
      break;
    case 4:
      HASH_GUARD(0x3235AF57F23103C4LL, invokeargs) {
        return (t_invokeargs(a0, a1));
      }
      HASH_GUARD(0x39C1BB731CB1CB04LL, getstartline) {
        return (t_getstartline());
      }
      break;
    case 7:
      HASH_GUARD(0x51A20EA0E327F607LL, isdestructor) {
        return (t_isdestructor());
      }
      break;
    case 13:
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        return (t_getfilename());
      }
      break;
    case 15:
      HASH_GUARD(0x48FDF6C5835C64CFLL, isinternal) {
        return (t_isinternal());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 20:
      HASH_GUARD(0x24253EBA491D6014LL, getmodifiers) {
        return (t_getmodifiers());
      }
      break;
    case 25:
      HASH_GUARD(0x7A15DC56E8CC0B19LL, isstatic) {
        return (t_isstatic());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0, a1), null);
      }
      break;
    case 32:
      HASH_GUARD(0x57D8DC34C9A03560LL, getnumberofparameters) {
        return (t_getnumberofparameters());
      }
      break;
    case 33:
      HASH_GUARD(0x4D637DECDBFA6221LL, getnumberofrequiredparameters) {
        return (t_getnumberofrequiredparameters());
      }
      break;
    case 34:
      HASH_GUARD(0x33A6C2CFBDB05EE2LL, getclosure) {
        return (t_getclosure());
      }
      break;
    case 35:
      HASH_GUARD(0x6ED51288559D6063LL, getdeclaringclass) {
        return (t_getdeclaringclass());
      }
      break;
    case 36:
      HASH_GUARD(0x6A6B8BECAE7D4164LL, isuserdefined) {
        return (t_isuserdefined());
      }
      break;
    case 37:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 40:
      HASH_GUARD(0x37FFB8F44A3329A8LL, getstaticvariables) {
        return (t_getstaticvariables());
      }
      HASH_GUARD(0x1A3AB3B0276D2668LL, returnsreference) {
        return (t_returnsreference());
      }
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(o_getClassName(), a0, a1, a2));
      }
      break;
    case 42:
      HASH_GUARD(0x2D7209A590477CEALL, isprotected) {
        return (t_isprotected());
      }
      break;
    case 45:
      HASH_GUARD(0x3E62225132C2A32DLL, getparameters) {
        return (t_getparameters());
      }
      break;
    case 46:
      HASH_GUARD(0x06FB6A7DC3D795AELL, isfinal) {
        return (t_isfinal());
      }
      break;
    case 47:
      HASH_GUARD(0x37AAE0845E2F636FLL, isprivate) {
        return (t_isprivate());
      }
      break;
    case 48:
      HASH_GUARD(0x6C19E85007BC4570LL, getendline) {
        return (t_getendline());
      }
      break;
    case 55:
      HASH_GUARD(0x7460D945DA32FDB7LL, isabstract) {
        return (t_isabstract());
      }
      break;
    case 56:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        return (t_getdoccomment());
      }
      break;
    case 58:
      HASH_GUARD(0x654B5F965C5CAC7ALL, isconstructor) {
        return (t_isconstructor());
      }
      break;
    case 60:
      HASH_GUARD(0x2820F10358723B7CLL, ispublic) {
        return (t_ispublic());
      }
      break;
    default:
      break;
  }
  return c_reflectionfunctionabstract::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_reflectionmethod::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 1) {
    case 0:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(c, params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    default:
      break;
  }
  return c_reflectionfunctionabstract::os_invoke(c, s, params, hash, fatal);
}
Variant c_reflectionmethod::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 2:
      HASH_GUARD(0x3FCE192CF6199942LL, invoke) {
        Variant a0;
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
          if (it == params.end()) break;
          a0 = (*it)->eval(env);
          it++;
        } while(false);
        Array vargs;
        for (; it != params.end(); ++it) {
          vargs.append((*it)->eval(env));
        }
        int count = params.size();
        if (count <= 1) return (t_invoke(count, a0));
        return (t_invoke(count, a0,vargs));
      }
      break;
    case 4:
      HASH_GUARD(0x3235AF57F23103C4LL, invokeargs) {
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
        return (t_invokeargs(a0, a1));
      }
      HASH_GUARD(0x39C1BB731CB1CB04LL, getstartline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getstartline());
      }
      break;
    case 7:
      HASH_GUARD(0x51A20EA0E327F607LL, isdestructor) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isdestructor());
      }
      break;
    case 13:
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
    case 15:
      HASH_GUARD(0x48FDF6C5835C64CFLL, isinternal) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isinternal());
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
    case 20:
      HASH_GUARD(0x24253EBA491D6014LL, getmodifiers) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmodifiers());
      }
      break;
    case 25:
      HASH_GUARD(0x7A15DC56E8CC0B19LL, isstatic) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isstatic());
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
        return (t___construct(a0, a1), null);
      }
      break;
    case 32:
      HASH_GUARD(0x57D8DC34C9A03560LL, getnumberofparameters) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnumberofparameters());
      }
      break;
    case 33:
      HASH_GUARD(0x4D637DECDBFA6221LL, getnumberofrequiredparameters) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnumberofrequiredparameters());
      }
      break;
    case 34:
      HASH_GUARD(0x33A6C2CFBDB05EE2LL, getclosure) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getclosure());
      }
      break;
    case 35:
      HASH_GUARD(0x6ED51288559D6063LL, getdeclaringclass) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getdeclaringclass());
      }
      break;
    case 36:
      HASH_GUARD(0x6A6B8BECAE7D4164LL, isuserdefined) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isuserdefined());
      }
      break;
    case 37:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getname());
      }
      break;
    case 40:
      HASH_GUARD(0x37FFB8F44A3329A8LL, getstaticvariables) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getstaticvariables());
      }
      HASH_GUARD(0x1A3AB3B0276D2668LL, returnsreference) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_returnsreference());
      }
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
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
        return (ti_export(o_getClassName(), a0, a1, a2));
      }
      break;
    case 42:
      HASH_GUARD(0x2D7209A590477CEALL, isprotected) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isprotected());
      }
      break;
    case 45:
      HASH_GUARD(0x3E62225132C2A32DLL, getparameters) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getparameters());
      }
      break;
    case 46:
      HASH_GUARD(0x06FB6A7DC3D795AELL, isfinal) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isfinal());
      }
      break;
    case 47:
      HASH_GUARD(0x37AAE0845E2F636FLL, isprivate) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isprivate());
      }
      break;
    case 48:
      HASH_GUARD(0x6C19E85007BC4570LL, getendline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getendline());
      }
      break;
    case 55:
      HASH_GUARD(0x7460D945DA32FDB7LL, isabstract) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isabstract());
      }
      break;
    case 56:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getdoccomment());
      }
      break;
    case 58:
      HASH_GUARD(0x654B5F965C5CAC7ALL, isconstructor) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isconstructor());
      }
      break;
    case 60:
      HASH_GUARD(0x2820F10358723B7CLL, ispublic) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_ispublic());
      }
      break;
    default:
      break;
  }
  return c_reflectionfunctionabstract::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_reflectionmethod::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 1) {
    case 0:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
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
        return (ti_export(c, a0, a1, a2));
      }
      break;
    default:
      break;
  }
  return c_reflectionfunctionabstract::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_reflectionmethod$os_get(const char *s) {
  return c_reflectionmethod::os_get(s, -1);
}
Variant &cw_reflectionmethod$os_lval(const char *s) {
  return c_reflectionmethod::os_lval(s, -1);
}
Variant cw_reflectionmethod$os_constant(const char *s) {
  return c_reflectionmethod::os_constant(s);
}
Variant cw_reflectionmethod$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_reflectionmethod::os_invoke(c, s, params, -1, fatal);
}
void c_reflectionmethod::init() {
  c_reflectionfunctionabstract::init();
  m_name = null;
  m_class = null;
}
/* SRC: classes/reflection.php line 587 */
void c_reflectionmethod::t___construct(Variant v_cls, Variant v_name) {
  INSTANCE_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::__construct);
  bool oldInCtor = gasInCtor(true);
  Variant v_method;

  if (toBoolean(v_cls) && toBoolean(v_name)) {
    if (!(x_is_object(v_cls))) v_cls = ((Object)(p_reflectionclass(p_reflectionclass(NEWOBJ(c_reflectionclass)())->create(v_cls))));
    v_method = v_cls.o_invoke_few_args("getMethod", 0x0D81ECE253A3B5B6LL, 1, v_name);
    if (toBoolean(v_method)) {
      m_info = v_method.o_get("info", 0x59E9384E33988B3ELL);
      m_name = v_method.o_get("name", 0x0BCDB293DC3CBDDCLL);
      m_class = v_method.o_get("class", 0x45397FE5C82DBD12LL);
    }
  }
  gasInCtor(oldInCtor);
} /* function */
/* SRC: classes/reflection.php line 599 */
String c_reflectionmethod::t___tostring() {
  INSTANCE_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::__toString);
  return toString(null);
} /* function */
/* SRC: classes/reflection.php line 603 */
Variant c_reflectionmethod::ti_export(const char* cls, Variant v_cls, Variant v_name, CVarRef v_ret) {
  STATIC_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::export);
  Variant v_obj;
  String v_str;

  if (!(x_is_object(v_cls))) v_cls = ((Object)(p_reflectionclass(p_reflectionclass(NEWOBJ(c_reflectionclass)())->create(v_cls))));
  v_obj = v_cls.o_invoke_few_args("getMethod", 0x0D81ECE253A3B5B6LL, 1, v_name);
  v_str = toString(v_obj);
  if (toBoolean(v_ret)) {
    return v_str;
  }
  print(v_str);
  return null;
} /* function */
/* SRC: classes/reflection.php line 613 */
Variant c_reflectionmethod::t_invoke(int num_args, CVarRef v_obj, Array args /* = Array() */) {
  INSTANCE_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::invoke);
  Variant v_args;

  v_args = func_get_args(num_args, Array(ArrayInit(1).set(0, ArrayElement(v_obj)).create()),args);
  x_array_shift(ref(v_args));
  return x_hphp_invoke_method(v_obj, toString(m_info.rvalAt("class", 0x45397FE5C82DBD12LL)), toString(m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL)), toArray(v_args));
} /* function */
/* SRC: classes/reflection.php line 620 */
Variant c_reflectionmethod::t_invokeargs(CVarRef v_obj, CVarRef v_args) {
  INSTANCE_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::invokeArgs);
  return x_hphp_invoke_method(v_obj, toString(m_info.rvalAt("class", 0x45397FE5C82DBD12LL)), toString(m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL)), toArray(v_args));
} /* function */
/* SRC: classes/reflection.php line 625 */
Variant c_reflectionmethod::t_isfinal() {
  INSTANCE_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::isFinal);
  return m_info.rvalAt("final", 0x5192930B2145036ELL);
} /* function */
/* SRC: classes/reflection.php line 629 */
Variant c_reflectionmethod::t_isabstract() {
  INSTANCE_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::isAbstract);
  return m_info.rvalAt("abstract", 0x475C06CE12C8A8A6LL);
} /* function */
/* SRC: classes/reflection.php line 633 */
bool c_reflectionmethod::t_ispublic() {
  INSTANCE_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::isPublic);
  return equal(m_info.rvalAt("access", 0x432ABF90750CDA3BLL), "public");
} /* function */
/* SRC: classes/reflection.php line 637 */
bool c_reflectionmethod::t_isprivate() {
  INSTANCE_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::isPrivate);
  return equal(m_info.rvalAt("access", 0x432ABF90750CDA3BLL), "private");
} /* function */
/* SRC: classes/reflection.php line 641 */
bool c_reflectionmethod::t_isprotected() {
  INSTANCE_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::isProtected);
  return equal(m_info.rvalAt("access", 0x432ABF90750CDA3BLL), "protected");
} /* function */
/* SRC: classes/reflection.php line 645 */
Variant c_reflectionmethod::t_isstatic() {
  INSTANCE_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::isStatic);
  return m_info.rvalAt("static", 0x1F5751E5F08D205DLL);
} /* function */
/* SRC: classes/reflection.php line 649 */
bool c_reflectionmethod::t_isconstructor() {
  INSTANCE_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::isConstructor);
  return equal(t_getname(), "__construct");
} /* function */
/* SRC: classes/reflection.php line 653 */
bool c_reflectionmethod::t_isdestructor() {
  INSTANCE_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::isDestructor);
  return equal(t_getname(), "__destruct");
} /* function */
/* SRC: classes/reflection.php line 657 */
Variant c_reflectionmethod::t_getmodifiers() {
  INSTANCE_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::getModifiers);
  return m_info.rvalAt("modifiers", 0x2CAF244C9F244C80LL);
} /* function */
/* SRC: classes/reflection.php line 661 */
Variant c_reflectionmethod::t_getclosure() {
  INSTANCE_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::getClosure);
  return m_info.rvalAt("closure", 0x10958EC44CD61020LL);
} /* function */
/* SRC: classes/reflection.php line 665 */
Variant c_reflectionmethod::t_getdeclaringclass() {
  INSTANCE_METHOD_INJECTION(ReflectionMethod, ReflectionMethod::getDeclaringClass);
  if (empty(m_info, "class", 0x45397FE5C82DBD12LL)) {
    return null;
  }
  return ((Object)(p_reflectionclass(p_reflectionclass(NEWOBJ(c_reflectionclass)())->create(m_info.rvalAt("class", 0x45397FE5C82DBD12LL)))));
} /* function */
/* SRC: classes/reflection.php line 490 */
Variant c_reflectionproperty::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_reflectionproperty::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_reflectionproperty::o_get(ArrayElementVec &props) const {
  props.push_back(NEW(ArrayElement)("info", m_info.isReferenced() ? ref(m_info) : m_info));
  props.push_back(NEW(ArrayElement)("name", m_name.isReferenced() ? ref(m_name) : m_name));
  props.push_back(NEW(ArrayElement)("class", m_class.isReferenced() ? ref(m_class) : m_class));
  c_ObjectData::o_get(props);
}
bool c_reflectionproperty::o_exists(CStrRef s, int64 hash) const {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 7) {
    case 2:
      HASH_EXISTS_STRING(0x45397FE5C82DBD12LL, class, 5);
      break;
    case 4:
      HASH_EXISTS_STRING(0x0BCDB293DC3CBDDCLL, name, 4);
      break;
    case 6:
      HASH_EXISTS_STRING(0x59E9384E33988B3ELL, info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_exists(s, hash);
}
Variant c_reflectionproperty::o_get(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 7) {
    case 2:
      HASH_RETURN_STRING(0x45397FE5C82DBD12LL, m_class,
                         class, 5);
      break;
    case 4:
      HASH_RETURN_STRING(0x0BCDB293DC3CBDDCLL, m_name,
                         name, 4);
      break;
    case 6:
      HASH_RETURN_STRING(0x59E9384E33988B3ELL, m_info,
                         info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_get(s, hash);
}
Variant c_reflectionproperty::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 7) {
    case 2:
      HASH_SET_STRING(0x45397FE5C82DBD12LL, m_class,
                      class, 5);
      break;
    case 4:
      HASH_SET_STRING(0x0BCDB293DC3CBDDCLL, m_name,
                      name, 4);
      break;
    case 6:
      HASH_SET_STRING(0x59E9384E33988B3ELL, m_info,
                      info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_reflectionproperty::o_lval(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 7) {
    case 2:
      HASH_RETURN_STRING(0x45397FE5C82DBD12LL, m_class,
                         class, 5);
      break;
    case 4:
      HASH_RETURN_STRING(0x0BCDB293DC3CBDDCLL, m_name,
                         name, 4);
      break;
    case 6:
      HASH_RETURN_STRING(0x59E9384E33988B3ELL, m_info,
                         info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_lval(s, hash);
}
Variant c_reflectionproperty::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(reflectionproperty)
ObjectData *c_reflectionproperty::create(Variant v_cls, Variant v_name) {
  init();
  t___construct(v_cls, v_name);
  return this;
}
ObjectData *c_reflectionproperty::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0), params.rvalAt(1)));
  } else return this;
}
void c_reflectionproperty::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0), params.rvalAt(1)));
}
ObjectData *c_reflectionproperty::cloneImpl() {
  c_reflectionproperty *obj = NEW(c_reflectionproperty)();
  cloneSet(obj);
  return obj;
}
void c_reflectionproperty::cloneSet(c_reflectionproperty *clone) {
  clone->m_info = m_info.isReferenced() ? ref(m_info) : m_info;
  clone->m_name = m_name.isReferenced() ? ref(m_name) : m_name;
  clone->m_class = m_class.isReferenced() ? ref(m_class) : m_class;
  ObjectData::cloneSet(clone);
}
Variant c_reflectionproperty::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 2:
      HASH_GUARD(0x791E9751F5B8C5A2LL, setaccessible) {
        return (t_setaccessible(), null);
      }
      break;
    case 3:
      HASH_GUARD(0x56879BCEB40997E3LL, getvalue) {
        return (t_getvalue(params.rvalAt(0)));
      }
      HASH_GUARD(0x6ED51288559D6063LL, getdeclaringclass) {
        return (t_getdeclaringclass());
      }
      break;
    case 5:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 8:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(o_getClassName(), params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 10:
      HASH_GUARD(0x2D7209A590477CEALL, isprotected) {
        return (t_isprotected());
      }
      break;
    case 15:
      HASH_GUARD(0x37AAE0845E2F636FLL, isprivate) {
        return (t_isprivate());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 20:
      HASH_GUARD(0x24253EBA491D6014LL, getmodifiers) {
        return (t_getmodifiers());
      }
      break;
    case 21:
      HASH_GUARD(0x384A52597AB11F15LL, isdefault) {
        return (t_isdefault());
      }
      HASH_GUARD(0x36FBED35008C8DB5LL, setvalue) {
        return (t_setvalue(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 24:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        return (t_getdoccomment());
      }
      break;
    case 25:
      HASH_GUARD(0x7A15DC56E8CC0B19LL, isstatic) {
        return (t_isstatic());
      }
      break;
    case 28:
      HASH_GUARD(0x2820F10358723B7CLL, ispublic) {
        return (t_ispublic());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_reflectionproperty::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 2:
      HASH_GUARD(0x791E9751F5B8C5A2LL, setaccessible) {
        return (t_setaccessible(), null);
      }
      break;
    case 3:
      HASH_GUARD(0x56879BCEB40997E3LL, getvalue) {
        return (t_getvalue(a0));
      }
      HASH_GUARD(0x6ED51288559D6063LL, getdeclaringclass) {
        return (t_getdeclaringclass());
      }
      break;
    case 5:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 8:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(o_getClassName(), a0, a1, a2));
      }
      break;
    case 10:
      HASH_GUARD(0x2D7209A590477CEALL, isprotected) {
        return (t_isprotected());
      }
      break;
    case 15:
      HASH_GUARD(0x37AAE0845E2F636FLL, isprivate) {
        return (t_isprivate());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 20:
      HASH_GUARD(0x24253EBA491D6014LL, getmodifiers) {
        return (t_getmodifiers());
      }
      break;
    case 21:
      HASH_GUARD(0x384A52597AB11F15LL, isdefault) {
        return (t_isdefault());
      }
      HASH_GUARD(0x36FBED35008C8DB5LL, setvalue) {
        return (t_setvalue(a0, a1));
      }
      break;
    case 24:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        return (t_getdoccomment());
      }
      break;
    case 25:
      HASH_GUARD(0x7A15DC56E8CC0B19LL, isstatic) {
        return (t_isstatic());
      }
      break;
    case 28:
      HASH_GUARD(0x2820F10358723B7CLL, ispublic) {
        return (t_ispublic());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0, a1), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_reflectionproperty::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 1) {
    case 0:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(c, params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_reflectionproperty::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 2:
      HASH_GUARD(0x791E9751F5B8C5A2LL, setaccessible) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_setaccessible(), null);
      }
      break;
    case 3:
      HASH_GUARD(0x56879BCEB40997E3LL, getvalue) {
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
        return (t_getvalue(a0));
      }
      HASH_GUARD(0x6ED51288559D6063LL, getdeclaringclass) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getdeclaringclass());
      }
      break;
    case 5:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getname());
      }
      break;
    case 8:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
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
        return (ti_export(o_getClassName(), a0, a1, a2));
      }
      break;
    case 10:
      HASH_GUARD(0x2D7209A590477CEALL, isprotected) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isprotected());
      }
      break;
    case 15:
      HASH_GUARD(0x37AAE0845E2F636FLL, isprivate) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isprivate());
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
    case 20:
      HASH_GUARD(0x24253EBA491D6014LL, getmodifiers) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getmodifiers());
      }
      break;
    case 21:
      HASH_GUARD(0x384A52597AB11F15LL, isdefault) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isdefault());
      }
      HASH_GUARD(0x36FBED35008C8DB5LL, setvalue) {
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
        return (t_setvalue(a0, a1));
      }
      break;
    case 24:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getdoccomment());
      }
      break;
    case 25:
      HASH_GUARD(0x7A15DC56E8CC0B19LL, isstatic) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isstatic());
      }
      break;
    case 28:
      HASH_GUARD(0x2820F10358723B7CLL, ispublic) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_ispublic());
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
        return (t___construct(a0, a1), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_reflectionproperty::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 1) {
    case 0:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
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
        return (ti_export(c, a0, a1, a2));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_reflectionproperty$os_get(const char *s) {
  return c_reflectionproperty::os_get(s, -1);
}
Variant &cw_reflectionproperty$os_lval(const char *s) {
  return c_reflectionproperty::os_lval(s, -1);
}
Variant cw_reflectionproperty$os_constant(const char *s) {
  return c_reflectionproperty::os_constant(s);
}
Variant cw_reflectionproperty$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_reflectionproperty::os_invoke(c, s, params, -1, fatal);
}
void c_reflectionproperty::init() {
  m_info = null;
  m_name = null;
  m_class = null;
}
/* SRC: classes/reflection.php line 495 */
void c_reflectionproperty::t___construct(Variant v_cls, Variant v_name) {
  INSTANCE_METHOD_INJECTION(ReflectionProperty, ReflectionProperty::__construct);
  bool oldInCtor = gasInCtor(true);
  Variant v_prop;

  if (toBoolean(v_cls) && toBoolean(v_name)) {
    if (!(x_is_object(v_cls))) v_cls = ((Object)(p_reflectionclass(p_reflectionclass(NEWOBJ(c_reflectionclass)())->create(v_cls))));
    v_prop = v_cls.o_invoke_few_args("getProperty", 0x0FD73627FB023047LL, 1, v_name);
    if (toBoolean(v_prop)) {
      m_info = v_prop.o_get("info", 0x59E9384E33988B3ELL);
      m_name = v_prop.o_get("name", 0x0BCDB293DC3CBDDCLL);
      m_class = v_prop.o_get("class", 0x45397FE5C82DBD12LL);
    }
  }
  gasInCtor(oldInCtor);
} /* function */
/* SRC: classes/reflection.php line 507 */
String c_reflectionproperty::t___tostring() {
  INSTANCE_METHOD_INJECTION(ReflectionProperty, ReflectionProperty::__toString);
  return toString(null);
} /* function */
/* SRC: classes/reflection.php line 510 */
Variant c_reflectionproperty::ti_export(const char* cls, Variant v_cls, Variant v_name, CVarRef v_ret) {
  STATIC_METHOD_INJECTION(ReflectionProperty, ReflectionProperty::export);
  Variant v_obj;
  String v_str;

  if (!(x_is_object(v_cls))) v_cls = ((Object)(p_reflectionclass(p_reflectionclass(NEWOBJ(c_reflectionclass)())->create(v_cls))));
  v_obj = v_cls.o_invoke_few_args("getProperty", 0x0FD73627FB023047LL, 1, v_name);
  v_str = toString(v_obj);
  if (toBoolean(v_ret)) {
    return v_str;
  }
  print(v_str);
  return null;
} /* function */
/* SRC: classes/reflection.php line 520 */
Variant c_reflectionproperty::t_getname() {
  INSTANCE_METHOD_INJECTION(ReflectionProperty, ReflectionProperty::getName);
  return m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL);
} /* function */
/* SRC: classes/reflection.php line 524 */
bool c_reflectionproperty::t_ispublic() {
  INSTANCE_METHOD_INJECTION(ReflectionProperty, ReflectionProperty::isPublic);
  return equal(m_info.rvalAt("access", 0x432ABF90750CDA3BLL), "public");
} /* function */
/* SRC: classes/reflection.php line 528 */
bool c_reflectionproperty::t_isprivate() {
  INSTANCE_METHOD_INJECTION(ReflectionProperty, ReflectionProperty::isPrivate);
  return equal(m_info.rvalAt("access", 0x432ABF90750CDA3BLL), "private");
} /* function */
/* SRC: classes/reflection.php line 532 */
bool c_reflectionproperty::t_isprotected() {
  INSTANCE_METHOD_INJECTION(ReflectionProperty, ReflectionProperty::isProtected);
  return equal(m_info.rvalAt("access", 0x432ABF90750CDA3BLL), "protected");
} /* function */
/* SRC: classes/reflection.php line 536 */
Variant c_reflectionproperty::t_isstatic() {
  INSTANCE_METHOD_INJECTION(ReflectionProperty, ReflectionProperty::isStatic);
  return m_info.rvalAt("static", 0x1F5751E5F08D205DLL);
} /* function */
/* SRC: classes/reflection.php line 540 */
Variant c_reflectionproperty::t_isdefault() {
  INSTANCE_METHOD_INJECTION(ReflectionProperty, ReflectionProperty::isDefault);
  return m_info.rvalAt("default", 0x6DE26F84570270CCLL);
} /* function */
/* SRC: classes/reflection.php line 544 */
void c_reflectionproperty::t_setaccessible() {
  INSTANCE_METHOD_INJECTION(ReflectionProperty, ReflectionProperty::setAccessible);
} /* function */
/* SRC: classes/reflection.php line 547 */
Variant c_reflectionproperty::t_getmodifiers() {
  INSTANCE_METHOD_INJECTION(ReflectionProperty, ReflectionProperty::getModifiers);
  return m_info.rvalAt("modifiers", 0x2CAF244C9F244C80LL);
} /* function */
/* SRC: classes/reflection.php line 551 */
Variant c_reflectionproperty::t_getvalue(CVarRef v_obj) {
  INSTANCE_METHOD_INJECTION(ReflectionProperty, ReflectionProperty::getValue);
  if (toBoolean(t_isstatic())) {
    return x_hphp_get_static_property(toString(m_info.rvalAt("class", 0x45397FE5C82DBD12LL)), toString(m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL)));
  }
  return x_hphp_get_property(toObject(v_obj), toString(m_info.rvalAt("class", 0x45397FE5C82DBD12LL)), toString(m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL)));
} /* function */
/* SRC: classes/reflection.php line 559 */
Variant c_reflectionproperty::t_setvalue(CVarRef v_obj, CVarRef v_value) {
  INSTANCE_METHOD_INJECTION(ReflectionProperty, ReflectionProperty::setValue);
  if (toBoolean(t_isstatic())) {
    return (x_hphp_set_static_property(toString(m_info.rvalAt("class", 0x45397FE5C82DBD12LL)), toString(m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL)), v_value), null);
  }
  x_hphp_set_property(toObject(v_obj), toString(m_info.rvalAt("class", 0x45397FE5C82DBD12LL)), toString(m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL)), v_value);
  return null;
} /* function */
/* SRC: classes/reflection.php line 567 */
Variant c_reflectionproperty::t_getdeclaringclass() {
  INSTANCE_METHOD_INJECTION(ReflectionProperty, ReflectionProperty::getDeclaringClass);
  if (empty(m_info, "class", 0x45397FE5C82DBD12LL)) {
    return null;
  }
  return ((Object)(p_reflectionclass(p_reflectionclass(NEWOBJ(c_reflectionclass)())->create(m_info.rvalAt("class", 0x45397FE5C82DBD12LL)))));
} /* function */
/* SRC: classes/reflection.php line 574 */
Variant c_reflectionproperty::t_getdoccomment() {
  INSTANCE_METHOD_INJECTION(ReflectionProperty, ReflectionProperty::getDocComment);
  return m_info.rvalAt("doc", 0x16758C759CFA17A6LL);
} /* function */
/* SRC: classes/reflection.php line 165 */
Variant c_reflectionfunction::os_get(const char *s, int64 hash) {
  return c_reflectionfunctionabstract::os_get(s, hash);
}
Variant &c_reflectionfunction::os_lval(const char *s, int64 hash) {
  return c_reflectionfunctionabstract::os_lval(s, hash);
}
void c_reflectionfunction::o_get(ArrayElementVec &props) const {
  c_reflectionfunctionabstract::o_get(props);
}
bool c_reflectionfunction::o_exists(CStrRef s, int64 hash) const {
  return c_reflectionfunctionabstract::o_exists(s, hash);
}
Variant c_reflectionfunction::o_get(CStrRef s, int64 hash) {
  return c_reflectionfunctionabstract::o_get(s, hash);
}
Variant c_reflectionfunction::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_reflectionfunctionabstract::o_set(s, hash, v, forInit);
}
Variant &c_reflectionfunction::o_lval(CStrRef s, int64 hash) {
  return c_reflectionfunctionabstract::o_lval(s, hash);
}
Variant c_reflectionfunction::os_constant(const char *s) {
  return c_reflectionfunctionabstract::os_constant(s);
}
IMPLEMENT_CLASS(reflectionfunction)
ObjectData *c_reflectionfunction::create(Variant v_name) {
  init();
  t___construct(v_name);
  return this;
}
ObjectData *c_reflectionfunction::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0)));
  } else return this;
}
void c_reflectionfunction::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0)));
}
ObjectData *c_reflectionfunction::cloneImpl() {
  c_reflectionfunction *obj = NEW(c_reflectionfunction)();
  cloneSet(obj);
  return obj;
}
void c_reflectionfunction::cloneSet(c_reflectionfunction *clone) {
  c_reflectionfunctionabstract::cloneSet(clone);
}
Variant c_reflectionfunction::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 2:
      HASH_GUARD(0x3FCE192CF6199942LL, invoke) {
        int count = params.size();
        if (count <= 0) return (t_invoke(count));
        return (t_invoke(count,params.slice(0, count - 0, false)));
      }
      break;
    case 4:
      HASH_GUARD(0x3235AF57F23103C4LL, invokeargs) {
        return (t_invokeargs(params.rvalAt(0)));
      }
      HASH_GUARD(0x39C1BB731CB1CB04LL, getstartline) {
        return (t_getstartline());
      }
      break;
    case 13:
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        return (t_getfilename());
      }
      break;
    case 15:
      HASH_GUARD(0x48FDF6C5835C64CFLL, isinternal) {
        return (t_isinternal());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0)), null);
      }
      break;
    case 32:
      HASH_GUARD(0x57D8DC34C9A03560LL, getnumberofparameters) {
        return (t_getnumberofparameters());
      }
      break;
    case 33:
      HASH_GUARD(0x4D637DECDBFA6221LL, getnumberofrequiredparameters) {
        return (t_getnumberofrequiredparameters());
      }
      break;
    case 34:
      HASH_GUARD(0x33A6C2CFBDB05EE2LL, getclosure) {
        return (t_getclosure());
      }
      break;
    case 36:
      HASH_GUARD(0x6A6B8BECAE7D4164LL, isuserdefined) {
        return (t_isuserdefined());
      }
      break;
    case 37:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 40:
      HASH_GUARD(0x37FFB8F44A3329A8LL, getstaticvariables) {
        return (t_getstaticvariables());
      }
      HASH_GUARD(0x1A3AB3B0276D2668LL, returnsreference) {
        return (t_returnsreference());
      }
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(o_getClassName(), params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 45:
      HASH_GUARD(0x3E62225132C2A32DLL, getparameters) {
        return (t_getparameters());
      }
      break;
    case 48:
      HASH_GUARD(0x6C19E85007BC4570LL, getendline) {
        return (t_getendline());
      }
      break;
    case 56:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        return (t_getdoccomment());
      }
      break;
    default:
      break;
  }
  return c_reflectionfunctionabstract::o_invoke(s, params, hash, fatal);
}
Variant c_reflectionfunction::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 2:
      HASH_GUARD(0x3FCE192CF6199942LL, invoke) {
        if (count <= 0) return (t_invoke(count));
        Array params;
        if (count >= 1) params.append(a0);
        if (count >= 2) params.append(a1);
        if (count >= 3) params.append(a2);
        if (count >= 4) params.append(a3);
        if (count >= 5) params.append(a4);
        if (count >= 6) params.append(a5);
        return (t_invoke(count,params));
      }
      break;
    case 4:
      HASH_GUARD(0x3235AF57F23103C4LL, invokeargs) {
        return (t_invokeargs(a0));
      }
      HASH_GUARD(0x39C1BB731CB1CB04LL, getstartline) {
        return (t_getstartline());
      }
      break;
    case 13:
      HASH_GUARD(0x1930CE336D39474DLL, getfilename) {
        return (t_getfilename());
      }
      break;
    case 15:
      HASH_GUARD(0x48FDF6C5835C64CFLL, isinternal) {
        return (t_isinternal());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0), null);
      }
      break;
    case 32:
      HASH_GUARD(0x57D8DC34C9A03560LL, getnumberofparameters) {
        return (t_getnumberofparameters());
      }
      break;
    case 33:
      HASH_GUARD(0x4D637DECDBFA6221LL, getnumberofrequiredparameters) {
        return (t_getnumberofrequiredparameters());
      }
      break;
    case 34:
      HASH_GUARD(0x33A6C2CFBDB05EE2LL, getclosure) {
        return (t_getclosure());
      }
      break;
    case 36:
      HASH_GUARD(0x6A6B8BECAE7D4164LL, isuserdefined) {
        return (t_isuserdefined());
      }
      break;
    case 37:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 40:
      HASH_GUARD(0x37FFB8F44A3329A8LL, getstaticvariables) {
        return (t_getstaticvariables());
      }
      HASH_GUARD(0x1A3AB3B0276D2668LL, returnsreference) {
        return (t_returnsreference());
      }
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(o_getClassName(), a0, a1));
      }
      break;
    case 45:
      HASH_GUARD(0x3E62225132C2A32DLL, getparameters) {
        return (t_getparameters());
      }
      break;
    case 48:
      HASH_GUARD(0x6C19E85007BC4570LL, getendline) {
        return (t_getendline());
      }
      break;
    case 56:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        return (t_getdoccomment());
      }
      break;
    default:
      break;
  }
  return c_reflectionfunctionabstract::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_reflectionfunction::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 1) {
    case 0:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(c, params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    default:
      break;
  }
  return c_reflectionfunctionabstract::os_invoke(c, s, params, hash, fatal);
}
Variant c_reflectionfunction::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 2:
      HASH_GUARD(0x3FCE192CF6199942LL, invoke) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        Array vargs;
        for (; it != params.end(); ++it) {
          vargs.append((*it)->eval(env));
        }
        int count = params.size();
        if (count <= 0) return (t_invoke(count));
        return (t_invoke(count,vargs));
      }
      break;
    case 4:
      HASH_GUARD(0x3235AF57F23103C4LL, invokeargs) {
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
        return (t_invokeargs(a0));
      }
      HASH_GUARD(0x39C1BB731CB1CB04LL, getstartline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getstartline());
      }
      break;
    case 13:
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
    case 15:
      HASH_GUARD(0x48FDF6C5835C64CFLL, isinternal) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isinternal());
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
    case 31:
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
    case 32:
      HASH_GUARD(0x57D8DC34C9A03560LL, getnumberofparameters) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnumberofparameters());
      }
      break;
    case 33:
      HASH_GUARD(0x4D637DECDBFA6221LL, getnumberofrequiredparameters) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnumberofrequiredparameters());
      }
      break;
    case 34:
      HASH_GUARD(0x33A6C2CFBDB05EE2LL, getclosure) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getclosure());
      }
      break;
    case 36:
      HASH_GUARD(0x6A6B8BECAE7D4164LL, isuserdefined) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isuserdefined());
      }
      break;
    case 37:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getname());
      }
      break;
    case 40:
      HASH_GUARD(0x37FFB8F44A3329A8LL, getstaticvariables) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getstaticvariables());
      }
      HASH_GUARD(0x1A3AB3B0276D2668LL, returnsreference) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_returnsreference());
      }
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
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
        return (ti_export(o_getClassName(), a0, a1));
      }
      break;
    case 45:
      HASH_GUARD(0x3E62225132C2A32DLL, getparameters) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getparameters());
      }
      break;
    case 48:
      HASH_GUARD(0x6C19E85007BC4570LL, getendline) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getendline());
      }
      break;
    case 56:
      HASH_GUARD(0x7C4F424FDA56ADF8LL, getdoccomment) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getdoccomment());
      }
      break;
    default:
      break;
  }
  return c_reflectionfunctionabstract::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_reflectionfunction::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 1) {
    case 0:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
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
        return (ti_export(c, a0, a1));
      }
      break;
    default:
      break;
  }
  return c_reflectionfunctionabstract::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_reflectionfunction$os_get(const char *s) {
  return c_reflectionfunction::os_get(s, -1);
}
Variant &cw_reflectionfunction$os_lval(const char *s) {
  return c_reflectionfunction::os_lval(s, -1);
}
Variant cw_reflectionfunction$os_constant(const char *s) {
  return c_reflectionfunction::os_constant(s);
}
Variant cw_reflectionfunction$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_reflectionfunction::os_invoke(c, s, params, -1, fatal);
}
void c_reflectionfunction::init() {
  c_reflectionfunctionabstract::init();
}
/* SRC: classes/reflection.php line 167 */
void c_reflectionfunction::t___construct(Variant v_name) {
  INSTANCE_METHOD_INJECTION(ReflectionFunction, ReflectionFunction::__construct);
  bool oldInCtor = gasInCtor(true);
  m_info = x_hphp_get_function_info(toString(v_name));
  if (empty(m_info)) {
    throw_exception(((Object)(p_reflectionexception(p_reflectionexception(NEWOBJ(c_reflectionexception)())->create(concat3("Function ", toString(v_name), " does not exist"))))));
  }
  gasInCtor(oldInCtor);
} /* function */
/* SRC: classes/reflection.php line 174 */
String c_reflectionfunction::t___tostring() {
  INSTANCE_METHOD_INJECTION(ReflectionFunction, ReflectionFunction::__toString);
  return toString(null);
} /* function */
/* SRC: classes/reflection.php line 178 */
Variant c_reflectionfunction::ti_export(const char* cls, CVarRef v_name, CVarRef v_ret) {
  STATIC_METHOD_INJECTION(ReflectionFunction, ReflectionFunction::export);
  p_reflectionfunction v_obj;
  String v_str;

  ((Object)(v_obj = ((Object)(p_reflectionfunction(p_reflectionfunction(NEWOBJ(c_reflectionfunction)())->create(v_name))))));
  v_str = toString(((Object)(v_obj)));
  if (toBoolean(v_ret)) {
    return v_str;
  }
  print(v_str);
  return null;
} /* function */
/* SRC: classes/reflection.php line 187 */
Variant c_reflectionfunction::t_invoke(int num_args, Array args /* = Array() */) {
  INSTANCE_METHOD_INJECTION(ReflectionFunction, ReflectionFunction::invoke);
  Array v_args;

  v_args = func_get_args(num_args, Array(),args);
  return x_hphp_invoke(toString(m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL)), v_args);
} /* function */
/* SRC: classes/reflection.php line 192 */
Variant c_reflectionfunction::t_invokeargs(CVarRef v_args) {
  INSTANCE_METHOD_INJECTION(ReflectionFunction, ReflectionFunction::invokeArgs);
  return x_hphp_invoke(toString(m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL)), toArray(v_args));
} /* function */
/* SRC: classes/reflection.php line 17 */
Variant c_reflectionparameter::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_reflectionparameter::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_reflectionparameter::o_get(ArrayElementVec &props) const {
  props.push_back(NEW(ArrayElement)("info", m_info.isReferenced() ? ref(m_info) : m_info));
  c_ObjectData::o_get(props);
}
bool c_reflectionparameter::o_exists(CStrRef s, int64 hash) const {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 1) {
    case 0:
      HASH_EXISTS_STRING(0x59E9384E33988B3ELL, info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_exists(s, hash);
}
Variant c_reflectionparameter::o_get(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 1) {
    case 0:
      HASH_RETURN_STRING(0x59E9384E33988B3ELL, m_info,
                         info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_get(s, hash);
}
Variant c_reflectionparameter::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 1) {
    case 0:
      HASH_SET_STRING(0x59E9384E33988B3ELL, m_info,
                      info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_reflectionparameter::o_lval(CStrRef s, int64 hash) {
  if (hash < 0) hash = hash_string(s.data(), s.length());
  switch (hash & 1) {
    case 0:
      HASH_RETURN_STRING(0x59E9384E33988B3ELL, m_info,
                         info, 4);
      break;
    default:
      break;
  }
  return c_ObjectData::o_lval(s, hash);
}
Variant c_reflectionparameter::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(reflectionparameter)
ObjectData *c_reflectionparameter::create(Variant v_func, Variant v_param) {
  init();
  t___construct(v_func, v_param);
  return this;
}
ObjectData *c_reflectionparameter::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0), params.rvalAt(1)));
  } else return this;
}
void c_reflectionparameter::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0), params.rvalAt(1)));
}
ObjectData *c_reflectionparameter::cloneImpl() {
  c_reflectionparameter *obj = NEW(c_reflectionparameter)();
  cloneSet(obj);
  return obj;
}
void c_reflectionparameter::cloneSet(c_reflectionparameter *clone) {
  clone->m_info = m_info.isReferenced() ? ref(m_info) : m_info;
  ObjectData::cloneSet(clone);
}
Variant c_reflectionparameter::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 3:
      HASH_GUARD(0x6ED51288559D6063LL, getdeclaringclass) {
        return (t_getdeclaringclass());
      }
      break;
    case 5:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 8:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(o_getClassName(), params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      HASH_GUARD(0x4044F1EEBF3BB8C8LL, getposition) {
        return (t_getposition());
      }
      break;
    case 9:
      HASH_GUARD(0x4F51DA0B633E9909LL, getclass) {
        return (t_getclass());
      }
      HASH_GUARD(0x24ED05F4504C4C09LL, allowsnull) {
        return (t_allowsnull());
      }
      break;
    case 12:
      HASH_GUARD(0x27C482A6C7951E0CLL, getdefaultvalue) {
        return (t_getdefaultvalue());
      }
      break;
    case 17:
      HASH_GUARD(0x13E3F304BDD89FB1LL, ispassedbyreference) {
        return (t_ispassedbyreference());
      }
      break;
    case 18:
      HASH_GUARD(0x6E34805C91257C92LL, isdefaultvalueavailable) {
        return (t_isdefaultvalueavailable());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 21:
      HASH_GUARD(0x2D6EF48BBAB22735LL, isoptional) {
        return (t_isoptional());
      }
      break;
    case 29:
      HASH_GUARD(0x5A9CE40C0F25871DLL, isarray) {
        return (t_isarray());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_reflectionparameter::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 3:
      HASH_GUARD(0x6ED51288559D6063LL, getdeclaringclass) {
        return (t_getdeclaringclass());
      }
      break;
    case 5:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 8:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(o_getClassName(), a0, a1, a2));
      }
      HASH_GUARD(0x4044F1EEBF3BB8C8LL, getposition) {
        return (t_getposition());
      }
      break;
    case 9:
      HASH_GUARD(0x4F51DA0B633E9909LL, getclass) {
        return (t_getclass());
      }
      HASH_GUARD(0x24ED05F4504C4C09LL, allowsnull) {
        return (t_allowsnull());
      }
      break;
    case 12:
      HASH_GUARD(0x27C482A6C7951E0CLL, getdefaultvalue) {
        return (t_getdefaultvalue());
      }
      break;
    case 17:
      HASH_GUARD(0x13E3F304BDD89FB1LL, ispassedbyreference) {
        return (t_ispassedbyreference());
      }
      break;
    case 18:
      HASH_GUARD(0x6E34805C91257C92LL, isdefaultvalueavailable) {
        return (t_isdefaultvalueavailable());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 21:
      HASH_GUARD(0x2D6EF48BBAB22735LL, isoptional) {
        return (t_isoptional());
      }
      break;
    case 29:
      HASH_GUARD(0x5A9CE40C0F25871DLL, isarray) {
        return (t_isarray());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0, a1), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_reflectionparameter::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 1) {
    case 0:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
        return (ti_export(c, params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_reflectionparameter::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 3:
      HASH_GUARD(0x6ED51288559D6063LL, getdeclaringclass) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getdeclaringclass());
      }
      break;
    case 5:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getname());
      }
      break;
    case 8:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
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
        return (ti_export(o_getClassName(), a0, a1, a2));
      }
      HASH_GUARD(0x4044F1EEBF3BB8C8LL, getposition) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getposition());
      }
      break;
    case 9:
      HASH_GUARD(0x4F51DA0B633E9909LL, getclass) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getclass());
      }
      HASH_GUARD(0x24ED05F4504C4C09LL, allowsnull) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_allowsnull());
      }
      break;
    case 12:
      HASH_GUARD(0x27C482A6C7951E0CLL, getdefaultvalue) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getdefaultvalue());
      }
      break;
    case 17:
      HASH_GUARD(0x13E3F304BDD89FB1LL, ispassedbyreference) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_ispassedbyreference());
      }
      break;
    case 18:
      HASH_GUARD(0x6E34805C91257C92LL, isdefaultvalueavailable) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isdefaultvalueavailable());
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
      HASH_GUARD(0x2D6EF48BBAB22735LL, isoptional) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isoptional());
      }
      break;
    case 29:
      HASH_GUARD(0x5A9CE40C0F25871DLL, isarray) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isarray());
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
        return (t___construct(a0, a1), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_reflectionparameter::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 1) {
    case 0:
      HASH_GUARD(0x0B5ABC58C98E70E8LL, export) {
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
        return (ti_export(c, a0, a1, a2));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_reflectionparameter$os_get(const char *s) {
  return c_reflectionparameter::os_get(s, -1);
}
Variant &cw_reflectionparameter$os_lval(const char *s) {
  return c_reflectionparameter::os_lval(s, -1);
}
Variant cw_reflectionparameter$os_constant(const char *s) {
  return c_reflectionparameter::os_constant(s);
}
Variant cw_reflectionparameter$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_reflectionparameter::os_invoke(c, s, params, -1, fatal);
}
void c_reflectionparameter::init() {
  m_info = null;
}
/* SRC: classes/reflection.php line 20 */
void c_reflectionparameter::t___construct(Variant v_func, Variant v_param) {
  INSTANCE_METHOD_INJECTION(ReflectionParameter, ReflectionParameter::__construct);
  bool oldInCtor = gasInCtor(true);
  Variant v_params;

  if (toBoolean(v_func) && toBoolean(v_param)) {
    v_params = v_func.o_invoke_few_args("getParameters", 0x3E62225132C2A32DLL, 0);
    m_info = v_params.rvalAt(v_param).o_get("info", 0x59E9384E33988B3ELL);
  }
  gasInCtor(oldInCtor);
} /* function */
/* SRC: classes/reflection.php line 27 */
String c_reflectionparameter::t___tostring() {
  INSTANCE_METHOD_INJECTION(ReflectionParameter, ReflectionParameter::__toString);
  return toString(null);
} /* function */
/* SRC: classes/reflection.php line 31 */
Variant c_reflectionparameter::ti_export(const char* cls, CVarRef v_func, CVarRef v_param, CVarRef v_ret) {
  STATIC_METHOD_INJECTION(ReflectionParameter, ReflectionParameter::export);
  p_reflectionparameter v_obj;
  String v_str;

  ((Object)(v_obj = ((Object)(p_reflectionparameter(p_reflectionparameter(NEWOBJ(c_reflectionparameter)())->create(v_func, v_param))))));
  v_str = toString(((Object)(v_obj)));
  if (toBoolean(v_ret)) {
    return v_str;
  }
  print(v_str);
  return null;
} /* function */
/* SRC: classes/reflection.php line 40 */
Variant c_reflectionparameter::t_getname() {
  INSTANCE_METHOD_INJECTION(ReflectionParameter, ReflectionParameter::getName);
  return m_info.rvalAt("name", 0x0BCDB293DC3CBDDCLL);
} /* function */
/* SRC: classes/reflection.php line 44 */
Variant c_reflectionparameter::t_ispassedbyreference() {
  INSTANCE_METHOD_INJECTION(ReflectionParameter, ReflectionParameter::isPassedByReference);
  return m_info.rvalAt("ref", 0x0B1A6D25134FD5FALL);
} /* function */
/* SRC: classes/reflection.php line 48 */
Variant c_reflectionparameter::t_getdeclaringclass() {
  INSTANCE_METHOD_INJECTION(ReflectionParameter, ReflectionParameter::getDeclaringClass);
  if (empty(m_info, "class", 0x45397FE5C82DBD12LL)) {
    return null;
  }
  return ((Object)(p_reflectionclass(p_reflectionclass(NEWOBJ(c_reflectionclass)())->create(m_info.rvalAt("class", 0x45397FE5C82DBD12LL)))));
} /* function */
/* SRC: classes/reflection.php line 55 */
Variant c_reflectionparameter::t_getclass() {
  INSTANCE_METHOD_INJECTION(ReflectionParameter, ReflectionParameter::getClass);
  if (empty(m_info, "type", 0x508FC7C8724A760ALL)) {
    return null;
  }
  return ((Object)(p_reflectionclass(p_reflectionclass(NEWOBJ(c_reflectionclass)())->create(m_info.rvalAt("type", 0x508FC7C8724A760ALL)))));
} /* function */
/* SRC: classes/reflection.php line 62 */
bool c_reflectionparameter::t_isarray() {
  INSTANCE_METHOD_INJECTION(ReflectionParameter, ReflectionParameter::isArray);
  return equal(m_info.rvalAt("type", 0x508FC7C8724A760ALL), "array");
} /* function */
/* SRC: classes/reflection.php line 66 */
Variant c_reflectionparameter::t_allowsnull() {
  INSTANCE_METHOD_INJECTION(ReflectionParameter, ReflectionParameter::allowsNull);
  return m_info.rvalAt("nullable", 0x5E22F816EDD47A43LL);
} /* function */
/* SRC: classes/reflection.php line 70 */
bool c_reflectionparameter::t_isoptional() {
  INSTANCE_METHOD_INJECTION(ReflectionParameter, ReflectionParameter::isOptional);
  return isset(m_info, "default", 0x6DE26F84570270CCLL);
} /* function */
/* SRC: classes/reflection.php line 74 */
bool c_reflectionparameter::t_isdefaultvalueavailable() {
  INSTANCE_METHOD_INJECTION(ReflectionParameter, ReflectionParameter::isDefaultValueAvailable);
  return isset(m_info, "default", 0x6DE26F84570270CCLL);
} /* function */
/* SRC: classes/reflection.php line 78 */
Variant c_reflectionparameter::t_getdefaultvalue() {
  INSTANCE_METHOD_INJECTION(ReflectionParameter, ReflectionParameter::getDefaultValue);
  if (!(t_isoptional())) {
    throw_exception(((Object)(p_reflectionexception(p_reflectionexception(NEWOBJ(c_reflectionexception)())->create("Parameter is not optional")))));
  }
  return m_info.rvalAt("default", 0x6DE26F84570270CCLL);
} /* function */
/* SRC: classes/reflection.php line 85 */
Variant c_reflectionparameter::t_getposition() {
  INSTANCE_METHOD_INJECTION(ReflectionParameter, ReflectionParameter::getPosition);
  return m_info.rvalAt("index", 0x440D5888C0FF3081LL);
} /* function */
Object co_reflectionfunctionabstract(CArrRef params, bool init /* = true */) {
  return Object(p_reflectionfunctionabstract(NEW(c_reflectionfunctionabstract)())->dynCreate(params, init));
}
Object co_reflectionobject(CArrRef params, bool init /* = true */) {
  return Object(p_reflectionobject(NEW(c_reflectionobject)())->dynCreate(params, init));
}
Object co_reflectionexception(CArrRef params, bool init /* = true */) {
  return Object(p_reflectionexception(NEW(c_reflectionexception)())->dynCreate(params, init));
}
Object co_reflectionclass(CArrRef params, bool init /* = true */) {
  return Object(p_reflectionclass(NEW(c_reflectionclass)())->dynCreate(params, init));
}
Object co_reflectionextension(CArrRef params, bool init /* = true */) {
  return Object(p_reflectionextension(NEW(c_reflectionextension)())->dynCreate(params, init));
}
Object co_reflectionmethod(CArrRef params, bool init /* = true */) {
  return Object(p_reflectionmethod(NEW(c_reflectionmethod)())->dynCreate(params, init));
}
Object co_reflectionproperty(CArrRef params, bool init /* = true */) {
  return Object(p_reflectionproperty(NEW(c_reflectionproperty)())->dynCreate(params, init));
}
Object co_reflectionfunction(CArrRef params, bool init /* = true */) {
  return Object(p_reflectionfunction(NEW(c_reflectionfunction)())->dynCreate(params, init));
}
Object co_reflectionparameter(CArrRef params, bool init /* = true */) {
  return Object(p_reflectionparameter(NEW(c_reflectionparameter)())->dynCreate(params, init));
}
Variant pm_php$classes$reflection_php(bool incOnce /* = false */, LVariableTable* variables /* = NULL */) {
  FUNCTION_INJECTION(run_init::classes/reflection.php);
  {
    DECLARE_SYSTEM_GLOBALS(g);
    bool &alreadyRun = g->run_pm_php$classes$reflection_php;
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

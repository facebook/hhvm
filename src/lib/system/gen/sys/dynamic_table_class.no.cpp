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

#include <cpp/base/hphp_system.h>
#include <cpp/ext/ext.h>
#include <cpp/eval/eval.h>


using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////


// Class Invoke Tables
Variant cw_reflectionfunctionabstract$os_get(const char *s);
Variant &cw_reflectionfunctionabstract$os_lval(const char *s);
Variant cw_reflectionfunctionabstract$os_constant(const char *s);
Variant cw_reflectionfunctionabstract$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_reflectionobject$os_get(const char *s);
Variant &cw_reflectionobject$os_lval(const char *s);
Variant cw_reflectionobject$os_constant(const char *s);
Variant cw_reflectionobject$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_splfileobject$os_get(const char *s);
Variant &cw_splfileobject$os_lval(const char *s);
Variant cw_splfileobject$os_constant(const char *s);
Variant cw_splfileobject$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_unexpectedvalueexception$os_get(const char *s);
Variant &cw_unexpectedvalueexception$os_lval(const char *s);
Variant cw_unexpectedvalueexception$os_constant(const char *s);
Variant cw_unexpectedvalueexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_stdclass$os_get(const char *s);
Variant &cw_stdclass$os_lval(const char *s);
Variant cw_stdclass$os_constant(const char *s);
Variant cw_stdclass$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_reflectionexception$os_get(const char *s);
Variant &cw_reflectionexception$os_lval(const char *s);
Variant cw_reflectionexception$os_constant(const char *s);
Variant cw_reflectionexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_overflowexception$os_get(const char *s);
Variant &cw_overflowexception$os_lval(const char *s);
Variant cw_overflowexception$os_constant(const char *s);
Variant cw_overflowexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_splfileinfo$os_get(const char *s);
Variant &cw_splfileinfo$os_lval(const char *s);
Variant cw_splfileinfo$os_constant(const char *s);
Variant cw_splfileinfo$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_arrayiterator$os_get(const char *s);
Variant &cw_arrayiterator$os_lval(const char *s);
Variant cw_arrayiterator$os_constant(const char *s);
Variant cw_arrayiterator$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_appenditerator$os_get(const char *s);
Variant &cw_appenditerator$os_lval(const char *s);
Variant cw_appenditerator$os_constant(const char *s);
Variant cw_appenditerator$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_outofboundsexception$os_get(const char *s);
Variant &cw_outofboundsexception$os_lval(const char *s);
Variant cw_outofboundsexception$os_constant(const char *s);
Variant cw_outofboundsexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_logicexception$os_get(const char *s);
Variant &cw_logicexception$os_lval(const char *s);
Variant cw_logicexception$os_constant(const char *s);
Variant cw_logicexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_rangeexception$os_get(const char *s);
Variant &cw_rangeexception$os_lval(const char *s);
Variant cw_rangeexception$os_constant(const char *s);
Variant cw_rangeexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_splobjectstorage$os_get(const char *s);
Variant &cw_splobjectstorage$os_lval(const char *s);
Variant cw_splobjectstorage$os_constant(const char *s);
Variant cw_splobjectstorage$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_invalidargumentexception$os_get(const char *s);
Variant &cw_invalidargumentexception$os_lval(const char *s);
Variant cw_invalidargumentexception$os_constant(const char *s);
Variant cw_invalidargumentexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_reflectionclass$os_get(const char *s);
Variant &cw_reflectionclass$os_lval(const char *s);
Variant cw_reflectionclass$os_constant(const char *s);
Variant cw_reflectionclass$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_underflowexception$os_get(const char *s);
Variant &cw_underflowexception$os_lval(const char *s);
Variant cw_underflowexception$os_constant(const char *s);
Variant cw_underflowexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_reflectionextension$os_get(const char *s);
Variant &cw_reflectionextension$os_lval(const char *s);
Variant cw_reflectionextension$os_constant(const char *s);
Variant cw_reflectionextension$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_outofrangeexception$os_get(const char *s);
Variant &cw_outofrangeexception$os_lval(const char *s);
Variant cw_outofrangeexception$os_constant(const char *s);
Variant cw_outofrangeexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_reflectionmethod$os_get(const char *s);
Variant &cw_reflectionmethod$os_lval(const char *s);
Variant cw_reflectionmethod$os_constant(const char *s);
Variant cw_reflectionmethod$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_badmethodcallexception$os_get(const char *s);
Variant &cw_badmethodcallexception$os_lval(const char *s);
Variant cw_badmethodcallexception$os_constant(const char *s);
Variant cw_badmethodcallexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_pear_error$os_get(const char *s);
Variant &cw_pear_error$os_lval(const char *s);
Variant cw_pear_error$os_constant(const char *s);
Variant cw_pear_error$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_reflectionproperty$os_get(const char *s);
Variant &cw_reflectionproperty$os_lval(const char *s);
Variant cw_reflectionproperty$os_constant(const char *s);
Variant cw_reflectionproperty$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw___php_incomplete_class$os_get(const char *s);
Variant &cw___php_incomplete_class$os_lval(const char *s);
Variant cw___php_incomplete_class$os_constant(const char *s);
Variant cw___php_incomplete_class$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_runtimeexception$os_get(const char *s);
Variant &cw_runtimeexception$os_lval(const char *s);
Variant cw_runtimeexception$os_constant(const char *s);
Variant cw_runtimeexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_exception$os_get(const char *s);
Variant &cw_exception$os_lval(const char *s);
Variant cw_exception$os_constant(const char *s);
Variant cw_exception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_reflectionfunction$os_get(const char *s);
Variant &cw_reflectionfunction$os_lval(const char *s);
Variant cw_reflectionfunction$os_constant(const char *s);
Variant cw_reflectionfunction$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_recursivedirectoryiterator$os_get(const char *s);
Variant &cw_recursivedirectoryiterator$os_lval(const char *s);
Variant cw_recursivedirectoryiterator$os_constant(const char *s);
Variant cw_recursivedirectoryiterator$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_directoryiterator$os_get(const char *s);
Variant &cw_directoryiterator$os_lval(const char *s);
Variant cw_directoryiterator$os_constant(const char *s);
Variant cw_directoryiterator$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_badfunctioncallexception$os_get(const char *s);
Variant &cw_badfunctioncallexception$os_lval(const char *s);
Variant cw_badfunctioncallexception$os_constant(const char *s);
Variant cw_badfunctioncallexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_lengthexception$os_get(const char *s);
Variant &cw_lengthexception$os_lval(const char *s);
Variant cw_lengthexception$os_constant(const char *s);
Variant cw_lengthexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_domainexception$os_get(const char *s);
Variant &cw_domainexception$os_lval(const char *s);
Variant cw_domainexception$os_constant(const char *s);
Variant cw_domainexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_recursiveiteratoriterator$os_get(const char *s);
Variant &cw_recursiveiteratoriterator$os_lval(const char *s);
Variant cw_recursiveiteratoriterator$os_constant(const char *s);
Variant cw_recursiveiteratoriterator$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_directory$os_get(const char *s);
Variant &cw_directory$os_lval(const char *s);
Variant cw_directory$os_constant(const char *s);
Variant cw_directory$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Variant cw_reflectionparameter$os_get(const char *s);
Variant &cw_reflectionparameter$os_lval(const char *s);
Variant cw_reflectionparameter$os_constant(const char *s);
Variant cw_reflectionparameter$os_invoke(const char *c, const char *s, CArrRef params, bool fatal = true);
Object co_domdocumentfragment(CArrRef params, bool init /* = true */) {
  return Object(p_domdocumentfragment(NEW(c_domdocumentfragment)())->dynCreate(params, init));
}
Variant c_domdocumentfragment::os_get(const char *s, int64 hash) {
  return c_domnode::os_get(s, hash);
}
Variant &c_domdocumentfragment::os_lval(const char *s, int64 hash) {
  return c_domnode::os_lval(s, hash);
}
void c_domdocumentfragment::o_get(ArrayElementVec &props) const {
  c_domnode::o_get(props);
}
bool c_domdocumentfragment::o_exists(CStrRef s, int64 hash) const {
  return c_domnode::o_exists(s, hash);
}
Variant c_domdocumentfragment::o_get(CStrRef s, int64 hash) {
  return c_domnode::o_get(s, hash);
}
Variant c_domdocumentfragment::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_domnode::o_set(s, hash, v, forInit);
}
Variant &c_domdocumentfragment::o_lval(CStrRef s, int64 hash) {
  return c_domnode::o_lval(s, hash);
}
Variant c_domdocumentfragment::os_constant(const char *s) {
  return c_domnode::os_constant(s);
}
IMPLEMENT_CLASS(domdocumentfragment)
ObjectData *c_domdocumentfragment::create() {
  init();
  t___construct();
  return this;
}
ObjectData *c_domdocumentfragment::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create());
  } else return this;
}
void c_domdocumentfragment::dynConstruct(CArrRef params) {
  (t___construct());
}
void c_domdocumentfragment::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domdocumentfragment::cloneImpl() {
  c_domdocumentfragment *obj = NEW(c_domdocumentfragment)();
  cloneSet(obj);
  return obj;
}
void c_domdocumentfragment::cloneSet(c_domdocumentfragment *clone) {
  c_domnode::cloneSet(clone);
}
Variant c_domdocumentfragment::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(params.rvalAt(0)));
        if (count == 2) return (t_c14n(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(params.rvalAt(0)));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        int count = params.size();
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(params.rvalAt(0)));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(params.rvalAt(0)));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(params.rvalAt(0)));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(params.rvalAt(0)));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(params.rvalAt(0)));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(params.rvalAt(0)));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      HASH_GUARD(0x1E44C8E8C1920931LL, appendxml) {
        return (t_appendxml(params.rvalAt(0)));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        int count = params.size();
        if (count <= 1) return (t_insertbefore(params.rvalAt(0)));
        return (t_insertbefore(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        int count = params.size();
        if (count <= 1) return (t_c14nfile(params.rvalAt(0)));
        if (count == 2) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke(s, params, hash, fatal);
}
Variant c_domdocumentfragment::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(a0));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      HASH_GUARD(0x1E44C8E8C1920931LL, appendxml) {
        return (t_appendxml(a0));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domdocumentfragment::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_domnode::os_invoke(c, s, params, hash, fatal);
}
Variant c_domdocumentfragment::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
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
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
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
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
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
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
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
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
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
        return (t_removechild(a0));
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
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
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
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
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
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
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
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_hasattributes());
      }
      HASH_GUARD(0x1E44C8E8C1920931LL, appendxml) {
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
        return (t_appendxml(a0));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
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
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
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
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domdocumentfragment::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_domnode::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domdocumentfragment$os_get(const char *s) {
  return c_domdocumentfragment::os_get(s, -1);
}
Variant &cw_domdocumentfragment$os_lval(const char *s) {
  return c_domdocumentfragment::os_lval(s, -1);
}
Variant cw_domdocumentfragment$os_constant(const char *s) {
  return c_domdocumentfragment::os_constant(s);
}
Variant cw_domdocumentfragment$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domdocumentfragment::os_invoke(c, s, params, -1, fatal);
}
Object co_domtext(CArrRef params, bool init /* = true */) {
  return Object(p_domtext(NEW(c_domtext)())->dynCreate(params, init));
}
Variant c_domtext::os_get(const char *s, int64 hash) {
  return c_domcharacterdata::os_get(s, hash);
}
Variant &c_domtext::os_lval(const char *s, int64 hash) {
  return c_domcharacterdata::os_lval(s, hash);
}
void c_domtext::o_get(ArrayElementVec &props) const {
  c_domcharacterdata::o_get(props);
}
bool c_domtext::o_exists(CStrRef s, int64 hash) const {
  return c_domcharacterdata::o_exists(s, hash);
}
Variant c_domtext::o_get(CStrRef s, int64 hash) {
  return c_domcharacterdata::o_get(s, hash);
}
Variant c_domtext::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_domcharacterdata::o_set(s, hash, v, forInit);
}
Variant &c_domtext::o_lval(CStrRef s, int64 hash) {
  return c_domcharacterdata::o_lval(s, hash);
}
Variant c_domtext::os_constant(const char *s) {
  return c_domcharacterdata::os_constant(s);
}
IMPLEMENT_CLASS(domtext)
ObjectData *c_domtext::create(Variant a0) {
  init();
  t___construct(a0);
  return this;
}
ObjectData *c_domtext::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0)));
  } else return this;
}
void c_domtext::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0)));
}
void c_domtext::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domtext::cloneImpl() {
  c_domtext *obj = NEW(c_domtext)();
  cloneSet(obj);
  return obj;
}
void c_domtext::cloneSet(c_domtext *clone) {
  c_domcharacterdata::cloneSet(clone);
}
Variant c_domtext::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(params.rvalAt(0)));
        if (count == 2) return (t_c14n(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(params.rvalAt(0)));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        int count = params.size();
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(params.rvalAt(0)));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      HASH_GUARD(0x7543EC746714D118LL, deletedata) {
        return (t_deletedata(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(params.rvalAt(0)));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(params.rvalAt(0)));
      }
      HASH_GUARD(0x28FBAB4F0F6C02DDLL, iswhitespaceinelementcontent) {
        return (t_iswhitespaceinelementcontent());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0)), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(params.rvalAt(0)));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(params.rvalAt(0)));
      }
      break;
    case 45:
      HASH_GUARD(0x3DAC04A51FD2D92DLL, appenddata) {
        return (t_appenddata(params.rvalAt(0)));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(params.rvalAt(0)));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 50:
      HASH_GUARD(0x2D8D2253C2149972LL, substringdata) {
        return (t_substringdata(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        int count = params.size();
        if (count <= 1) return (t_insertbefore(params.rvalAt(0)));
        return (t_insertbefore(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 52:
      HASH_GUARD(0x470F2F0EA1F3D4B4LL, replacedata) {
        return (t_replacedata(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        int count = params.size();
        if (count <= 1) return (t_c14nfile(params.rvalAt(0)));
        if (count == 2) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
      }
      break;
    case 55:
      HASH_GUARD(0x714D38308D8212B7LL, splittext) {
        return (t_splittext(params.rvalAt(0)));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    case 61:
      HASH_GUARD(0x0E19CE7886BCEF3DLL, insertdata) {
        return (t_insertdata(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    default:
      break;
  }
  return c_domcharacterdata::o_invoke(s, params, hash, fatal);
}
Variant c_domtext::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      HASH_GUARD(0x7543EC746714D118LL, deletedata) {
        return (t_deletedata(a0, a1));
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(a0));
      }
      HASH_GUARD(0x28FBAB4F0F6C02DDLL, iswhitespaceinelementcontent) {
        return (t_iswhitespaceinelementcontent());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 45:
      HASH_GUARD(0x3DAC04A51FD2D92DLL, appenddata) {
        return (t_appenddata(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 50:
      HASH_GUARD(0x2D8D2253C2149972LL, substringdata) {
        return (t_substringdata(a0, a1));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 52:
      HASH_GUARD(0x470F2F0EA1F3D4B4LL, replacedata) {
        return (t_replacedata(a0, a1, a2));
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 55:
      HASH_GUARD(0x714D38308D8212B7LL, splittext) {
        return (t_splittext(a0));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    case 61:
      HASH_GUARD(0x0E19CE7886BCEF3DLL, insertdata) {
        return (t_insertdata(a0, a1));
      }
      break;
    default:
      break;
  }
  return c_domcharacterdata::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domtext::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_domcharacterdata::os_invoke(c, s, params, hash, fatal);
}
Variant c_domtext::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
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
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
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
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
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
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlineno());
      }
      HASH_GUARD(0x7543EC746714D118LL, deletedata) {
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
        return (t_deletedata(a0, a1));
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
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
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
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
        return (t_removechild(a0));
      }
      HASH_GUARD(0x28FBAB4F0F6C02DDLL, iswhitespaceinelementcontent) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_iswhitespaceinelementcontent());
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
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
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
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
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
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 45:
      HASH_GUARD(0x3DAC04A51FD2D92DLL, appenddata) {
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
        return (t_appenddata(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
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
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_hasattributes());
      }
      break;
    case 50:
      HASH_GUARD(0x2D8D2253C2149972LL, substringdata) {
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
        return (t_substringdata(a0, a1));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
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
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 52:
      HASH_GUARD(0x470F2F0EA1F3D4B4LL, replacedata) {
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
        return (t_replacedata(a0, a1, a2));
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 55:
      HASH_GUARD(0x714D38308D8212B7LL, splittext) {
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
        return (t_splittext(a0));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
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
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_haschildnodes());
      }
      break;
    case 61:
      HASH_GUARD(0x0E19CE7886BCEF3DLL, insertdata) {
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
        return (t_insertdata(a0, a1));
      }
      break;
    default:
      break;
  }
  return c_domcharacterdata::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domtext::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_domcharacterdata::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domtext$os_get(const char *s) {
  return c_domtext::os_get(s, -1);
}
Variant &cw_domtext$os_lval(const char *s) {
  return c_domtext::os_lval(s, -1);
}
Variant cw_domtext$os_constant(const char *s) {
  return c_domtext::os_constant(s);
}
Variant cw_domtext$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domtext::os_invoke(c, s, params, -1, fatal);
}
Object co_domcdatasection(CArrRef params, bool init /* = true */) {
  return Object(p_domcdatasection(NEW(c_domcdatasection)())->dynCreate(params, init));
}
Variant c_domcdatasection::os_get(const char *s, int64 hash) {
  return c_domtext::os_get(s, hash);
}
Variant &c_domcdatasection::os_lval(const char *s, int64 hash) {
  return c_domtext::os_lval(s, hash);
}
void c_domcdatasection::o_get(ArrayElementVec &props) const {
  c_domtext::o_get(props);
}
bool c_domcdatasection::o_exists(CStrRef s, int64 hash) const {
  return c_domtext::o_exists(s, hash);
}
Variant c_domcdatasection::o_get(CStrRef s, int64 hash) {
  return c_domtext::o_get(s, hash);
}
Variant c_domcdatasection::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_domtext::o_set(s, hash, v, forInit);
}
Variant &c_domcdatasection::o_lval(CStrRef s, int64 hash) {
  return c_domtext::o_lval(s, hash);
}
Variant c_domcdatasection::os_constant(const char *s) {
  return c_domtext::os_constant(s);
}
IMPLEMENT_CLASS(domcdatasection)
ObjectData *c_domcdatasection::create(Variant a0) {
  init();
  t___construct(a0);
  return this;
}
ObjectData *c_domcdatasection::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0)));
  } else return this;
}
void c_domcdatasection::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0)));
}
void c_domcdatasection::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domcdatasection::cloneImpl() {
  c_domcdatasection *obj = NEW(c_domcdatasection)();
  cloneSet(obj);
  return obj;
}
void c_domcdatasection::cloneSet(c_domcdatasection *clone) {
  c_domtext::cloneSet(clone);
}
Variant c_domcdatasection::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(params.rvalAt(0)));
        if (count == 2) return (t_c14n(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(params.rvalAt(0)));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        int count = params.size();
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(params.rvalAt(0)));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      HASH_GUARD(0x7543EC746714D118LL, deletedata) {
        return (t_deletedata(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(params.rvalAt(0)));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(params.rvalAt(0)));
      }
      HASH_GUARD(0x28FBAB4F0F6C02DDLL, iswhitespaceinelementcontent) {
        return (t_iswhitespaceinelementcontent());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0)), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(params.rvalAt(0)));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(params.rvalAt(0)));
      }
      break;
    case 45:
      HASH_GUARD(0x3DAC04A51FD2D92DLL, appenddata) {
        return (t_appenddata(params.rvalAt(0)));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(params.rvalAt(0)));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 50:
      HASH_GUARD(0x2D8D2253C2149972LL, substringdata) {
        return (t_substringdata(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        int count = params.size();
        if (count <= 1) return (t_insertbefore(params.rvalAt(0)));
        return (t_insertbefore(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 52:
      HASH_GUARD(0x470F2F0EA1F3D4B4LL, replacedata) {
        return (t_replacedata(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        int count = params.size();
        if (count <= 1) return (t_c14nfile(params.rvalAt(0)));
        if (count == 2) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
      }
      break;
    case 55:
      HASH_GUARD(0x714D38308D8212B7LL, splittext) {
        return (t_splittext(params.rvalAt(0)));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    case 61:
      HASH_GUARD(0x0E19CE7886BCEF3DLL, insertdata) {
        return (t_insertdata(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    default:
      break;
  }
  return c_domtext::o_invoke(s, params, hash, fatal);
}
Variant c_domcdatasection::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      HASH_GUARD(0x7543EC746714D118LL, deletedata) {
        return (t_deletedata(a0, a1));
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(a0));
      }
      HASH_GUARD(0x28FBAB4F0F6C02DDLL, iswhitespaceinelementcontent) {
        return (t_iswhitespaceinelementcontent());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 45:
      HASH_GUARD(0x3DAC04A51FD2D92DLL, appenddata) {
        return (t_appenddata(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 50:
      HASH_GUARD(0x2D8D2253C2149972LL, substringdata) {
        return (t_substringdata(a0, a1));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 52:
      HASH_GUARD(0x470F2F0EA1F3D4B4LL, replacedata) {
        return (t_replacedata(a0, a1, a2));
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 55:
      HASH_GUARD(0x714D38308D8212B7LL, splittext) {
        return (t_splittext(a0));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    case 61:
      HASH_GUARD(0x0E19CE7886BCEF3DLL, insertdata) {
        return (t_insertdata(a0, a1));
      }
      break;
    default:
      break;
  }
  return c_domtext::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domcdatasection::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_domtext::os_invoke(c, s, params, hash, fatal);
}
Variant c_domcdatasection::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
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
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
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
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
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
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlineno());
      }
      HASH_GUARD(0x7543EC746714D118LL, deletedata) {
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
        return (t_deletedata(a0, a1));
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
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
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
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
        return (t_removechild(a0));
      }
      HASH_GUARD(0x28FBAB4F0F6C02DDLL, iswhitespaceinelementcontent) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_iswhitespaceinelementcontent());
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
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
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
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
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
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 45:
      HASH_GUARD(0x3DAC04A51FD2D92DLL, appenddata) {
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
        return (t_appenddata(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
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
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_hasattributes());
      }
      break;
    case 50:
      HASH_GUARD(0x2D8D2253C2149972LL, substringdata) {
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
        return (t_substringdata(a0, a1));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
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
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 52:
      HASH_GUARD(0x470F2F0EA1F3D4B4LL, replacedata) {
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
        return (t_replacedata(a0, a1, a2));
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 55:
      HASH_GUARD(0x714D38308D8212B7LL, splittext) {
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
        return (t_splittext(a0));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
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
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_haschildnodes());
      }
      break;
    case 61:
      HASH_GUARD(0x0E19CE7886BCEF3DLL, insertdata) {
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
        return (t_insertdata(a0, a1));
      }
      break;
    default:
      break;
  }
  return c_domtext::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domcdatasection::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_domtext::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domcdatasection$os_get(const char *s) {
  return c_domcdatasection::os_get(s, -1);
}
Variant &cw_domcdatasection$os_lval(const char *s) {
  return c_domcdatasection::os_lval(s, -1);
}
Variant cw_domcdatasection$os_constant(const char *s) {
  return c_domcdatasection::os_constant(s);
}
Variant cw_domcdatasection$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domcdatasection::os_invoke(c, s, params, -1, fatal);
}
Object co_domdocumenttype(CArrRef params, bool init /* = true */) {
  return Object(p_domdocumenttype(NEW(c_domdocumenttype)())->dynCreate(params, init));
}
Variant c_domdocumenttype::os_get(const char *s, int64 hash) {
  return c_domnode::os_get(s, hash);
}
Variant &c_domdocumenttype::os_lval(const char *s, int64 hash) {
  return c_domnode::os_lval(s, hash);
}
void c_domdocumenttype::o_get(ArrayElementVec &props) const {
  c_domnode::o_get(props);
}
bool c_domdocumenttype::o_exists(CStrRef s, int64 hash) const {
  return c_domnode::o_exists(s, hash);
}
Variant c_domdocumenttype::o_get(CStrRef s, int64 hash) {
  return c_domnode::o_get(s, hash);
}
Variant c_domdocumenttype::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_domnode::o_set(s, hash, v, forInit);
}
Variant &c_domdocumenttype::o_lval(CStrRef s, int64 hash) {
  return c_domnode::o_lval(s, hash);
}
Variant c_domdocumenttype::os_constant(const char *s) {
  return c_domnode::os_constant(s);
}
IMPLEMENT_CLASS(domdocumenttype)
ObjectData *c_domdocumenttype::create() {
  init();
  t___construct();
  return this;
}
ObjectData *c_domdocumenttype::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create());
  } else return this;
}
void c_domdocumenttype::dynConstruct(CArrRef params) {
  (t___construct());
}
void c_domdocumenttype::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domdocumenttype::cloneImpl() {
  c_domdocumenttype *obj = NEW(c_domdocumenttype)();
  cloneSet(obj);
  return obj;
}
void c_domdocumenttype::cloneSet(c_domdocumenttype *clone) {
  c_domnode::cloneSet(clone);
}
Variant c_domdocumenttype::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(params.rvalAt(0)));
        if (count == 2) return (t_c14n(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(params.rvalAt(0)));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        int count = params.size();
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(params.rvalAt(0)));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(params.rvalAt(0)));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(params.rvalAt(0)));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(params.rvalAt(0)));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(params.rvalAt(0)));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(params.rvalAt(0)));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        int count = params.size();
        if (count <= 1) return (t_insertbefore(params.rvalAt(0)));
        return (t_insertbefore(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        int count = params.size();
        if (count <= 1) return (t_c14nfile(params.rvalAt(0)));
        if (count == 2) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke(s, params, hash, fatal);
}
Variant c_domdocumenttype::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(a0));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domdocumenttype::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_domnode::os_invoke(c, s, params, hash, fatal);
}
Variant c_domdocumenttype::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
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
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
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
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
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
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
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
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
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
        return (t_removechild(a0));
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
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
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
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
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
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
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
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
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
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
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
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domdocumenttype::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_domnode::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domdocumenttype$os_get(const char *s) {
  return c_domdocumenttype::os_get(s, -1);
}
Variant &cw_domdocumenttype$os_lval(const char *s) {
  return c_domdocumenttype::os_lval(s, -1);
}
Variant cw_domdocumenttype$os_constant(const char *s) {
  return c_domdocumenttype::os_constant(s);
}
Variant cw_domdocumenttype$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domdocumenttype::os_invoke(c, s, params, -1, fatal);
}
Object co_sqlite3stmt(CArrRef params, bool init /* = true */) {
  return Object(p_sqlite3stmt(NEW(c_sqlite3stmt)())->dynCreate(params, init));
}
Variant c_sqlite3stmt::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_sqlite3stmt::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_sqlite3stmt::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_sqlite3stmt::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_sqlite3stmt::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_sqlite3stmt::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_sqlite3stmt::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_sqlite3stmt::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(sqlite3stmt)
ObjectData *c_sqlite3stmt::create(Object a0, String a1) {
  init();
  t___construct(a0, a1);
  return this;
}
ObjectData *c_sqlite3stmt::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0), params.rvalAt(1)));
  } else return this;
}
void c_sqlite3stmt::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0), params.rvalAt(1)));
}
void c_sqlite3stmt::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_sqlite3stmt::cloneImpl() {
  c_sqlite3stmt *obj = NEW(c_sqlite3stmt)();
  cloneSet(obj);
  return obj;
}
void c_sqlite3stmt::cloneSet(c_sqlite3stmt *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_sqlite3stmt::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 0:
      HASH_GUARD(0x340A51AE22A924E0LL, reset) {
        return (t_reset());
      }
      break;
    case 1:
      HASH_GUARD(0x78AE97BFBEBF5341LL, close) {
        return (t_close());
      }
      break;
    case 3:
      HASH_GUARD(0x436E6AFC3628E403LL, bindvalue) {
        int count = params.size();
        if (count <= 2) return (t_bindvalue(params.rvalAt(0), params.rvalAt(1)));
        return (t_bindvalue(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 7:
      HASH_GUARD(0x31DA235C5A226667LL, clear) {
        return (t_clear());
      }
      break;
    case 14:
      HASH_GUARD(0x187C7F43EB57714ELL, execute) {
        return (t_execute());
      }
      break;
    case 19:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 22:
      HASH_GUARD(0x77B13FCF1BA41696LL, bindparam) {
        int count = params.size();
        if (count <= 2) return (t_bindparam(params.rvalAt(0), ref(const_cast<Array&>(params).lvalAt(1))));
        return (t_bindparam(params.rvalAt(0), ref(const_cast<Array&>(params).lvalAt(1)), params.rvalAt(2)));
      }
      break;
    case 23:
      HASH_GUARD(0x72FA987001E93357LL, paramcount) {
        return (t_paramcount());
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
Variant c_sqlite3stmt::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 0:
      HASH_GUARD(0x340A51AE22A924E0LL, reset) {
        return (t_reset());
      }
      break;
    case 1:
      HASH_GUARD(0x78AE97BFBEBF5341LL, close) {
        return (t_close());
      }
      break;
    case 3:
      HASH_GUARD(0x436E6AFC3628E403LL, bindvalue) {
        if (count <= 2) return (t_bindvalue(a0, a1));
        return (t_bindvalue(a0, a1, a2));
      }
      break;
    case 7:
      HASH_GUARD(0x31DA235C5A226667LL, clear) {
        return (t_clear());
      }
      break;
    case 14:
      HASH_GUARD(0x187C7F43EB57714ELL, execute) {
        return (t_execute());
      }
      break;
    case 19:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 22:
      HASH_GUARD(0x77B13FCF1BA41696LL, bindparam) {
        if (count <= 2) return (t_bindparam(a0, ref(a1)));
        return (t_bindparam(a0, ref(a1), a2));
      }
      break;
    case 23:
      HASH_GUARD(0x72FA987001E93357LL, paramcount) {
        return (t_paramcount());
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
Variant c_sqlite3stmt::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_sqlite3stmt::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 0:
      HASH_GUARD(0x340A51AE22A924E0LL, reset) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_reset());
      }
      break;
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
      break;
    case 3:
      HASH_GUARD(0x436E6AFC3628E403LL, bindvalue) {
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
        if (count <= 2) return (t_bindvalue(a0, a1));
        return (t_bindvalue(a0, a1, a2));
      }
      break;
    case 7:
      HASH_GUARD(0x31DA235C5A226667LL, clear) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_clear());
      }
      break;
    case 14:
      HASH_GUARD(0x187C7F43EB57714ELL, execute) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_execute());
      }
      break;
    case 19:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 22:
      HASH_GUARD(0x77B13FCF1BA41696LL, bindparam) {
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
          a1 = ref((*it)->refval(env));
          it++;
          if (it == params.end()) break;
          a2 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 2) return (t_bindparam(a0, ref(a1)));
        return (t_bindparam(a0, ref(a1), a2));
      }
      break;
    case 23:
      HASH_GUARD(0x72FA987001E93357LL, paramcount) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_paramcount());
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
Variant c_sqlite3stmt::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_sqlite3stmt$os_get(const char *s) {
  return c_sqlite3stmt::os_get(s, -1);
}
Variant &cw_sqlite3stmt$os_lval(const char *s) {
  return c_sqlite3stmt::os_lval(s, -1);
}
Variant cw_sqlite3stmt$os_constant(const char *s) {
  return c_sqlite3stmt::os_constant(s);
}
Variant cw_sqlite3stmt$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_sqlite3stmt::os_invoke(c, s, params, -1, fatal);
}
Object co_domnodelist(CArrRef params, bool init /* = true */) {
  return Object(p_domnodelist(NEW(c_domnodelist)())->dynCreate(params, init));
}
Variant c_domnodelist::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_domnodelist::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_domnodelist::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_domnodelist::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_domnodelist::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_domnodelist::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_domnodelist::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_domnodelist::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(domnodelist)
ObjectData *c_domnodelist::create() {
  init();
  t___construct();
  return this;
}
ObjectData *c_domnodelist::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create());
  } else return this;
}
void c_domnodelist::dynConstruct(CArrRef params) {
  (t___construct());
}
void c_domnodelist::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domnodelist::cloneImpl() {
  c_domnodelist *obj = NEW(c_domnodelist)();
  cloneSet(obj);
  return obj;
}
void c_domnodelist::cloneSet(c_domnodelist *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_domnodelist::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 7) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 6:
      HASH_GUARD(0x0A41DBE0830902C6LL, item) {
        return (t_item(params.rvalAt(0)));
      }
      break;
    case 7:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_domnodelist::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 7) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 6:
      HASH_GUARD(0x0A41DBE0830902C6LL, item) {
        return (t_item(a0));
      }
      break;
    case 7:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domnodelist::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_domnodelist::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 7) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 6:
      HASH_GUARD(0x0A41DBE0830902C6LL, item) {
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
        return (t_item(a0));
      }
      break;
    case 7:
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
Variant c_domnodelist::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domnodelist$os_get(const char *s) {
  return c_domnodelist::os_get(s, -1);
}
Variant &cw_domnodelist$os_lval(const char *s) {
  return c_domnodelist::os_lval(s, -1);
}
Variant cw_domnodelist$os_constant(const char *s) {
  return c_domnodelist::os_constant(s);
}
Variant cw_domnodelist$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domnodelist::os_invoke(c, s, params, -1, fatal);
}
Object co_domcharacterdata(CArrRef params, bool init /* = true */) {
  return Object(p_domcharacterdata(NEW(c_domcharacterdata)())->dynCreate(params, init));
}
Variant c_domcharacterdata::os_get(const char *s, int64 hash) {
  return c_domnode::os_get(s, hash);
}
Variant &c_domcharacterdata::os_lval(const char *s, int64 hash) {
  return c_domnode::os_lval(s, hash);
}
void c_domcharacterdata::o_get(ArrayElementVec &props) const {
  c_domnode::o_get(props);
}
bool c_domcharacterdata::o_exists(CStrRef s, int64 hash) const {
  return c_domnode::o_exists(s, hash);
}
Variant c_domcharacterdata::o_get(CStrRef s, int64 hash) {
  return c_domnode::o_get(s, hash);
}
Variant c_domcharacterdata::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_domnode::o_set(s, hash, v, forInit);
}
Variant &c_domcharacterdata::o_lval(CStrRef s, int64 hash) {
  return c_domnode::o_lval(s, hash);
}
Variant c_domcharacterdata::os_constant(const char *s) {
  return c_domnode::os_constant(s);
}
IMPLEMENT_CLASS(domcharacterdata)
ObjectData *c_domcharacterdata::create(Variant a0) {
  init();
  t___construct(a0);
  return this;
}
ObjectData *c_domcharacterdata::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0)));
  } else return this;
}
void c_domcharacterdata::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0)));
}
void c_domcharacterdata::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domcharacterdata::cloneImpl() {
  c_domcharacterdata *obj = NEW(c_domcharacterdata)();
  cloneSet(obj);
  return obj;
}
void c_domcharacterdata::cloneSet(c_domcharacterdata *clone) {
  c_domnode::cloneSet(clone);
}
Variant c_domcharacterdata::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(params.rvalAt(0)));
        if (count == 2) return (t_c14n(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(params.rvalAt(0)));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        int count = params.size();
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(params.rvalAt(0)));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      HASH_GUARD(0x7543EC746714D118LL, deletedata) {
        return (t_deletedata(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(params.rvalAt(0)));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(params.rvalAt(0)));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0)), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(params.rvalAt(0)));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(params.rvalAt(0)));
      }
      break;
    case 45:
      HASH_GUARD(0x3DAC04A51FD2D92DLL, appenddata) {
        return (t_appenddata(params.rvalAt(0)));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(params.rvalAt(0)));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 50:
      HASH_GUARD(0x2D8D2253C2149972LL, substringdata) {
        return (t_substringdata(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        int count = params.size();
        if (count <= 1) return (t_insertbefore(params.rvalAt(0)));
        return (t_insertbefore(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 52:
      HASH_GUARD(0x470F2F0EA1F3D4B4LL, replacedata) {
        return (t_replacedata(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        int count = params.size();
        if (count <= 1) return (t_c14nfile(params.rvalAt(0)));
        if (count == 2) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    case 61:
      HASH_GUARD(0x0E19CE7886BCEF3DLL, insertdata) {
        return (t_insertdata(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke(s, params, hash, fatal);
}
Variant c_domcharacterdata::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      HASH_GUARD(0x7543EC746714D118LL, deletedata) {
        return (t_deletedata(a0, a1));
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(a0));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 45:
      HASH_GUARD(0x3DAC04A51FD2D92DLL, appenddata) {
        return (t_appenddata(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 50:
      HASH_GUARD(0x2D8D2253C2149972LL, substringdata) {
        return (t_substringdata(a0, a1));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 52:
      HASH_GUARD(0x470F2F0EA1F3D4B4LL, replacedata) {
        return (t_replacedata(a0, a1, a2));
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    case 61:
      HASH_GUARD(0x0E19CE7886BCEF3DLL, insertdata) {
        return (t_insertdata(a0, a1));
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domcharacterdata::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_domnode::os_invoke(c, s, params, hash, fatal);
}
Variant c_domcharacterdata::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
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
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
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
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
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
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlineno());
      }
      HASH_GUARD(0x7543EC746714D118LL, deletedata) {
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
        return (t_deletedata(a0, a1));
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
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
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
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
        return (t_removechild(a0));
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
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
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
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
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
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 45:
      HASH_GUARD(0x3DAC04A51FD2D92DLL, appenddata) {
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
        return (t_appenddata(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
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
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_hasattributes());
      }
      break;
    case 50:
      HASH_GUARD(0x2D8D2253C2149972LL, substringdata) {
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
        return (t_substringdata(a0, a1));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
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
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 52:
      HASH_GUARD(0x470F2F0EA1F3D4B4LL, replacedata) {
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
        return (t_replacedata(a0, a1, a2));
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
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
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_haschildnodes());
      }
      break;
    case 61:
      HASH_GUARD(0x0E19CE7886BCEF3DLL, insertdata) {
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
        return (t_insertdata(a0, a1));
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domcharacterdata::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_domnode::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domcharacterdata$os_get(const char *s) {
  return c_domcharacterdata::os_get(s, -1);
}
Variant &cw_domcharacterdata$os_lval(const char *s) {
  return c_domcharacterdata::os_lval(s, -1);
}
Variant cw_domcharacterdata$os_constant(const char *s) {
  return c_domcharacterdata::os_constant(s);
}
Variant cw_domcharacterdata$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domcharacterdata::os_invoke(c, s, params, -1, fatal);
}
Object co_domentityreference(CArrRef params, bool init /* = true */) {
  return Object(p_domentityreference(NEW(c_domentityreference)())->dynCreate(params, init));
}
Variant c_domentityreference::os_get(const char *s, int64 hash) {
  return c_domnode::os_get(s, hash);
}
Variant &c_domentityreference::os_lval(const char *s, int64 hash) {
  return c_domnode::os_lval(s, hash);
}
void c_domentityreference::o_get(ArrayElementVec &props) const {
  c_domnode::o_get(props);
}
bool c_domentityreference::o_exists(CStrRef s, int64 hash) const {
  return c_domnode::o_exists(s, hash);
}
Variant c_domentityreference::o_get(CStrRef s, int64 hash) {
  return c_domnode::o_get(s, hash);
}
Variant c_domentityreference::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_domnode::o_set(s, hash, v, forInit);
}
Variant &c_domentityreference::o_lval(CStrRef s, int64 hash) {
  return c_domnode::o_lval(s, hash);
}
Variant c_domentityreference::os_constant(const char *s) {
  return c_domnode::os_constant(s);
}
IMPLEMENT_CLASS(domentityreference)
ObjectData *c_domentityreference::create(Variant a0) {
  init();
  t___construct(a0);
  return this;
}
ObjectData *c_domentityreference::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0)));
  } else return this;
}
void c_domentityreference::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0)));
}
void c_domentityreference::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domentityreference::cloneImpl() {
  c_domentityreference *obj = NEW(c_domentityreference)();
  cloneSet(obj);
  return obj;
}
void c_domentityreference::cloneSet(c_domentityreference *clone) {
  c_domnode::cloneSet(clone);
}
Variant c_domentityreference::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(params.rvalAt(0)));
        if (count == 2) return (t_c14n(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(params.rvalAt(0)));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        int count = params.size();
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(params.rvalAt(0)));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(params.rvalAt(0)));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(params.rvalAt(0)));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0)), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(params.rvalAt(0)));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(params.rvalAt(0)));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(params.rvalAt(0)));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        int count = params.size();
        if (count <= 1) return (t_insertbefore(params.rvalAt(0)));
        return (t_insertbefore(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        int count = params.size();
        if (count <= 1) return (t_c14nfile(params.rvalAt(0)));
        if (count == 2) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke(s, params, hash, fatal);
}
Variant c_domentityreference::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(a0));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domentityreference::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_domnode::os_invoke(c, s, params, hash, fatal);
}
Variant c_domentityreference::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
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
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
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
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
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
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
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
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
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
        return (t_removechild(a0));
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
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
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
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
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
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
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
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
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
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
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
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domentityreference::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_domnode::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domentityreference$os_get(const char *s) {
  return c_domentityreference::os_get(s, -1);
}
Variant &cw_domentityreference$os_lval(const char *s) {
  return c_domentityreference::os_lval(s, -1);
}
Variant cw_domentityreference$os_constant(const char *s) {
  return c_domentityreference::os_constant(s);
}
Variant cw_domentityreference$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domentityreference::os_invoke(c, s, params, -1, fatal);
}
Object co_datetimezone(CArrRef params, bool init /* = true */) {
  return Object(p_datetimezone(NEW(c_datetimezone)())->dynCreate(params, init));
}
Variant c_datetimezone::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_datetimezone::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_datetimezone::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_datetimezone::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_datetimezone::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_datetimezone::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_datetimezone::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_datetimezone::os_constant(const char *s) {
  int64 hash = hash_string(s);
  switch (hash & 31) {
    case 2:
      HASH_RETURN(0x5213CDB166554802LL, q_datetimezone_AFRICA, AFRICA);
      break;
    case 10:
      HASH_RETURN(0x1F67AB7044E3CA2ALL, q_datetimezone_EUROPE, EUROPE);
      break;
    case 12:
      HASH_RETURN(0x76F99914EEA96ECCLL, q_datetimezone_ARCTIC, ARCTIC);
      break;
    case 13:
      HASH_RETURN(0x338F44565E0C038DLL, q_datetimezone_AMERICA, AMERICA);
      break;
    case 15:
      HASH_RETURN(0x22CAA6990573E5EFLL, q_datetimezone_PACIFIC, PACIFIC);
      break;
    case 19:
      HASH_RETURN(0x7879534F49C301F3LL, q_datetimezone_PER_COUNTRY, PER_COUNTRY);
      break;
    case 20:
      HASH_RETURN(0x6FB9D84AA364CA14LL, q_datetimezone_UTC, UTC);
      break;
    case 23:
      HASH_RETURN(0x5A01AACAA61E0FF7LL, q_datetimezone_ASIA, ASIA);
      HASH_RETURN(0x3E74CAEDAD25E137LL, q_datetimezone_INDIAN, INDIAN);
      break;
    case 24:
      HASH_RETURN(0x0C4F30470A9EEBF8LL, q_datetimezone_ALL, ALL);
      break;
    case 25:
      HASH_RETURN(0x2BEDCED53E63BC79LL, q_datetimezone_ATLANTIC, ATLANTIC);
      break;
    case 30:
      HASH_RETURN(0x3C168ED86522549ELL, q_datetimezone_AUSTRALIA, AUSTRALIA);
      HASH_RETURN(0x690180C3BA732B5ELL, q_datetimezone_ALL_WITH_BC, ALL_WITH_BC);
      break;
    case 31:
      HASH_RETURN(0x6B29DE65F46EAFDFLL, q_datetimezone_ANTARCTICA, ANTARCTICA);
      break;
    default:
      break;
  }
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(datetimezone)
ObjectData *c_datetimezone::create(String a0) {
  init();
  t___construct(a0);
  return this;
}
ObjectData *c_datetimezone::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0)));
  } else return this;
}
void c_datetimezone::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0)));
}
void c_datetimezone::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_datetimezone::cloneImpl() {
  c_datetimezone *obj = NEW(c_datetimezone)();
  cloneSet(obj);
  return obj;
}
void c_datetimezone::cloneSet(c_datetimezone *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_datetimezone::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 3:
      HASH_GUARD(0x6D7CD16BBA93D063LL, listidentifiers) {
        return (ti_listidentifiers(o_getClassName()));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 4:
      HASH_GUARD(0x63D3CF3884CE93F4LL, gettransitions) {
        return (t_gettransitions());
      }
      break;
    case 5:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 8:
      HASH_GUARD(0x2D0CCAB16C3ED068LL, listabbreviations) {
        return (ti_listabbreviations(o_getClassName()));
      }
      break;
    case 11:
      HASH_GUARD(0x68DB66F60B55BD0BLL, getoffset) {
        return (t_getoffset(params.rvalAt(0)));
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0)), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_datetimezone::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 3:
      HASH_GUARD(0x6D7CD16BBA93D063LL, listidentifiers) {
        return (ti_listidentifiers(o_getClassName()));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 4:
      HASH_GUARD(0x63D3CF3884CE93F4LL, gettransitions) {
        return (t_gettransitions());
      }
      break;
    case 5:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 8:
      HASH_GUARD(0x2D0CCAB16C3ED068LL, listabbreviations) {
        return (ti_listabbreviations(o_getClassName()));
      }
      break;
    case 11:
      HASH_GUARD(0x68DB66F60B55BD0BLL, getoffset) {
        return (t_getoffset(a0));
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_datetimezone::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 3) {
    case 0:
      HASH_GUARD(0x2D0CCAB16C3ED068LL, listabbreviations) {
        return (ti_listabbreviations(c));
      }
      break;
    case 3:
      HASH_GUARD(0x6D7CD16BBA93D063LL, listidentifiers) {
        return (ti_listidentifiers(c));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_datetimezone::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 3:
      HASH_GUARD(0x6D7CD16BBA93D063LL, listidentifiers) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (ti_listidentifiers(o_getClassName()));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 4:
      HASH_GUARD(0x63D3CF3884CE93F4LL, gettransitions) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettransitions());
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
      HASH_GUARD(0x2D0CCAB16C3ED068LL, listabbreviations) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (ti_listabbreviations(o_getClassName()));
      }
      break;
    case 11:
      HASH_GUARD(0x68DB66F60B55BD0BLL, getoffset) {
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
        return (t_getoffset(a0));
      }
      break;
    case 15:
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
Variant c_datetimezone::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 3) {
    case 0:
      HASH_GUARD(0x2D0CCAB16C3ED068LL, listabbreviations) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (ti_listabbreviations(c));
      }
      break;
    case 3:
      HASH_GUARD(0x6D7CD16BBA93D063LL, listidentifiers) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (ti_listidentifiers(c));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_datetimezone$os_get(const char *s) {
  return c_datetimezone::os_get(s, -1);
}
Variant &cw_datetimezone$os_lval(const char *s) {
  return c_datetimezone::os_lval(s, -1);
}
Variant cw_datetimezone$os_constant(const char *s) {
  return c_datetimezone::os_constant(s);
}
Variant cw_datetimezone$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_datetimezone::os_invoke(c, s, params, -1, fatal);
}
Object co_domimplementation(CArrRef params, bool init /* = true */) {
  return Object(p_domimplementation(NEW(c_domimplementation)())->dynCreate(params, init));
}
Variant c_domimplementation::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_domimplementation::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_domimplementation::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_domimplementation::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_domimplementation::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_domimplementation::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_domimplementation::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_domimplementation::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(domimplementation)
ObjectData *c_domimplementation::create() {
  init();
  t___construct();
  return this;
}
ObjectData *c_domimplementation::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create());
  } else return this;
}
void c_domimplementation::dynConstruct(CArrRef params) {
  (t___construct());
}
void c_domimplementation::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domimplementation::cloneImpl() {
  c_domimplementation *obj = NEW(c_domimplementation)();
  cloneSet(obj);
  return obj;
}
void c_domimplementation::cloneSet(c_domimplementation *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_domimplementation::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 3:
      HASH_GUARD(0x3CB56A796F3DBAC3LL, createdocumenttype) {
        int count = params.size();
        if (count <= 0) return (t_createdocumenttype());
        if (count == 1) return (t_createdocumenttype(params.rvalAt(0)));
        if (count == 2) return (t_createdocumenttype(params.rvalAt(0), params.rvalAt(1)));
        return (t_createdocumenttype(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      HASH_GUARD(0x280878EAC306A6A3LL, hasfeature) {
        return (t_hasfeature(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 14:
      HASH_GUARD(0x30855E95B37D1B4ELL, createdocument) {
        int count = params.size();
        if (count <= 0) return (t_createdocument());
        if (count == 1) return (t_createdocument(params.rvalAt(0)));
        if (count == 2) return (t_createdocument(params.rvalAt(0), params.rvalAt(1)));
        return (t_createdocument(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_domimplementation::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 3:
      HASH_GUARD(0x3CB56A796F3DBAC3LL, createdocumenttype) {
        if (count <= 0) return (t_createdocumenttype());
        if (count == 1) return (t_createdocumenttype(a0));
        if (count == 2) return (t_createdocumenttype(a0, a1));
        return (t_createdocumenttype(a0, a1, a2));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      HASH_GUARD(0x280878EAC306A6A3LL, hasfeature) {
        return (t_hasfeature(a0, a1));
      }
      break;
    case 14:
      HASH_GUARD(0x30855E95B37D1B4ELL, createdocument) {
        if (count <= 0) return (t_createdocument());
        if (count == 1) return (t_createdocument(a0));
        if (count == 2) return (t_createdocument(a0, a1));
        return (t_createdocument(a0, a1, a2));
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domimplementation::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_domimplementation::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 3:
      HASH_GUARD(0x3CB56A796F3DBAC3LL, createdocumenttype) {
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
        if (count <= 0) return (t_createdocumenttype());
        if (count == 1) return (t_createdocumenttype(a0));
        if (count == 2) return (t_createdocumenttype(a0, a1));
        return (t_createdocumenttype(a0, a1, a2));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      HASH_GUARD(0x280878EAC306A6A3LL, hasfeature) {
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
        return (t_hasfeature(a0, a1));
      }
      break;
    case 14:
      HASH_GUARD(0x30855E95B37D1B4ELL, createdocument) {
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
        if (count <= 0) return (t_createdocument());
        if (count == 1) return (t_createdocument(a0));
        if (count == 2) return (t_createdocument(a0, a1));
        return (t_createdocument(a0, a1, a2));
      }
      break;
    case 15:
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
Variant c_domimplementation::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domimplementation$os_get(const char *s) {
  return c_domimplementation::os_get(s, -1);
}
Variant &cw_domimplementation$os_lval(const char *s) {
  return c_domimplementation::os_lval(s, -1);
}
Variant cw_domimplementation$os_constant(const char *s) {
  return c_domimplementation::os_constant(s);
}
Variant cw_domimplementation$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domimplementation::os_invoke(c, s, params, -1, fatal);
}
Object co_soapheader(CArrRef params, bool init /* = true */) {
  return Object(p_soapheader(NEW(c_soapheader)())->dynCreate(params, init));
}
Variant c_soapheader::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_soapheader::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_soapheader::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_soapheader::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_soapheader::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_soapheader::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_soapheader::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_soapheader::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(soapheader)
ObjectData *c_soapheader::create(String a0, String a1, Variant a2, bool a3, Variant a4) {
  init();
  t___construct(a0, a1, a2, a3, a4);
  return this;
}
ObjectData *c_soapheader::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    int count = params.size();
    if (count <= 2) return (create(params.rvalAt(0), params.rvalAt(1)));
    if (count == 3) return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
    if (count == 4) return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
    return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
  } else return this;
}
void c_soapheader::dynConstruct(CArrRef params) {
  int count = params.size();
  if (count <= 2) (t___construct(params.rvalAt(0), params.rvalAt(1)));
  if (count == 3) (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
  if (count == 4) (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
  (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
}
void c_soapheader::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_soapheader::cloneImpl() {
  c_soapheader *obj = NEW(c_soapheader)();
  cloneSet(obj);
  return obj;
}
void c_soapheader::cloneSet(c_soapheader *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_soapheader::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 3) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 2) return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
        if (count == 3) return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)), null);
        if (count == 4) return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_soapheader::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 3) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 2) return (t___construct(a0, a1), null);
        if (count == 3) return (t___construct(a0, a1, a2), null);
        if (count == 4) return (t___construct(a0, a1, a2, a3), null);
        return (t___construct(a0, a1, a2, a3, a4), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_soapheader::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_soapheader::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 3) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 2) return (t___construct(a0, a1), null);
        if (count == 3) return (t___construct(a0, a1, a2), null);
        if (count == 4) return (t___construct(a0, a1, a2, a3), null);
        return (t___construct(a0, a1, a2, a3, a4), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_soapheader::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_soapheader$os_get(const char *s) {
  return c_soapheader::os_get(s, -1);
}
Variant &cw_soapheader$os_lval(const char *s) {
  return c_soapheader::os_lval(s, -1);
}
Variant cw_soapheader$os_constant(const char *s) {
  return c_soapheader::os_constant(s);
}
Variant cw_soapheader$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_soapheader::os_invoke(c, s, params, -1, fatal);
}
Object co_domnotation(CArrRef params, bool init /* = true */) {
  return Object(p_domnotation(NEW(c_domnotation)())->dynCreate(params, init));
}
Variant c_domnotation::os_get(const char *s, int64 hash) {
  return c_domnode::os_get(s, hash);
}
Variant &c_domnotation::os_lval(const char *s, int64 hash) {
  return c_domnode::os_lval(s, hash);
}
void c_domnotation::o_get(ArrayElementVec &props) const {
  c_domnode::o_get(props);
}
bool c_domnotation::o_exists(CStrRef s, int64 hash) const {
  return c_domnode::o_exists(s, hash);
}
Variant c_domnotation::o_get(CStrRef s, int64 hash) {
  return c_domnode::o_get(s, hash);
}
Variant c_domnotation::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_domnode::o_set(s, hash, v, forInit);
}
Variant &c_domnotation::o_lval(CStrRef s, int64 hash) {
  return c_domnode::o_lval(s, hash);
}
Variant c_domnotation::os_constant(const char *s) {
  return c_domnode::os_constant(s);
}
IMPLEMENT_CLASS(domnotation)
ObjectData *c_domnotation::create() {
  init();
  t___construct();
  return this;
}
ObjectData *c_domnotation::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create());
  } else return this;
}
void c_domnotation::dynConstruct(CArrRef params) {
  (t___construct());
}
void c_domnotation::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domnotation::cloneImpl() {
  c_domnotation *obj = NEW(c_domnotation)();
  cloneSet(obj);
  return obj;
}
void c_domnotation::cloneSet(c_domnotation *clone) {
  c_domnode::cloneSet(clone);
}
Variant c_domnotation::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(params.rvalAt(0)));
        if (count == 2) return (t_c14n(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(params.rvalAt(0)));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        int count = params.size();
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(params.rvalAt(0)));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(params.rvalAt(0)));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(params.rvalAt(0)));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(params.rvalAt(0)));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(params.rvalAt(0)));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(params.rvalAt(0)));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        int count = params.size();
        if (count <= 1) return (t_insertbefore(params.rvalAt(0)));
        return (t_insertbefore(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        int count = params.size();
        if (count <= 1) return (t_c14nfile(params.rvalAt(0)));
        if (count == 2) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke(s, params, hash, fatal);
}
Variant c_domnotation::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(a0));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domnotation::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_domnode::os_invoke(c, s, params, hash, fatal);
}
Variant c_domnotation::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
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
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
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
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
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
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
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
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
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
        return (t_removechild(a0));
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
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
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
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
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
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
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
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
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
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
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
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domnotation::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_domnode::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domnotation$os_get(const char *s) {
  return c_domnotation::os_get(s, -1);
}
Variant &cw_domnotation$os_lval(const char *s) {
  return c_domnotation::os_lval(s, -1);
}
Variant cw_domnotation$os_constant(const char *s) {
  return c_domnotation::os_constant(s);
}
Variant cw_domnotation$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domnotation::os_invoke(c, s, params, -1, fatal);
}
Object co_domcomment(CArrRef params, bool init /* = true */) {
  return Object(p_domcomment(NEW(c_domcomment)())->dynCreate(params, init));
}
Variant c_domcomment::os_get(const char *s, int64 hash) {
  return c_domcharacterdata::os_get(s, hash);
}
Variant &c_domcomment::os_lval(const char *s, int64 hash) {
  return c_domcharacterdata::os_lval(s, hash);
}
void c_domcomment::o_get(ArrayElementVec &props) const {
  c_domcharacterdata::o_get(props);
}
bool c_domcomment::o_exists(CStrRef s, int64 hash) const {
  return c_domcharacterdata::o_exists(s, hash);
}
Variant c_domcomment::o_get(CStrRef s, int64 hash) {
  return c_domcharacterdata::o_get(s, hash);
}
Variant c_domcomment::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_domcharacterdata::o_set(s, hash, v, forInit);
}
Variant &c_domcomment::o_lval(CStrRef s, int64 hash) {
  return c_domcharacterdata::o_lval(s, hash);
}
Variant c_domcomment::os_constant(const char *s) {
  return c_domcharacterdata::os_constant(s);
}
IMPLEMENT_CLASS(domcomment)
ObjectData *c_domcomment::create(Variant a0) {
  init();
  t___construct(a0);
  return this;
}
ObjectData *c_domcomment::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0)));
  } else return this;
}
void c_domcomment::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0)));
}
void c_domcomment::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domcomment::cloneImpl() {
  c_domcomment *obj = NEW(c_domcomment)();
  cloneSet(obj);
  return obj;
}
void c_domcomment::cloneSet(c_domcomment *clone) {
  c_domcharacterdata::cloneSet(clone);
}
Variant c_domcomment::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(params.rvalAt(0)));
        if (count == 2) return (t_c14n(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(params.rvalAt(0)));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        int count = params.size();
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(params.rvalAt(0)));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      HASH_GUARD(0x7543EC746714D118LL, deletedata) {
        return (t_deletedata(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(params.rvalAt(0)));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(params.rvalAt(0)));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0)), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(params.rvalAt(0)));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(params.rvalAt(0)));
      }
      break;
    case 45:
      HASH_GUARD(0x3DAC04A51FD2D92DLL, appenddata) {
        return (t_appenddata(params.rvalAt(0)));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(params.rvalAt(0)));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 50:
      HASH_GUARD(0x2D8D2253C2149972LL, substringdata) {
        return (t_substringdata(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        int count = params.size();
        if (count <= 1) return (t_insertbefore(params.rvalAt(0)));
        return (t_insertbefore(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 52:
      HASH_GUARD(0x470F2F0EA1F3D4B4LL, replacedata) {
        return (t_replacedata(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        int count = params.size();
        if (count <= 1) return (t_c14nfile(params.rvalAt(0)));
        if (count == 2) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    case 61:
      HASH_GUARD(0x0E19CE7886BCEF3DLL, insertdata) {
        return (t_insertdata(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    default:
      break;
  }
  return c_domcharacterdata::o_invoke(s, params, hash, fatal);
}
Variant c_domcomment::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      HASH_GUARD(0x7543EC746714D118LL, deletedata) {
        return (t_deletedata(a0, a1));
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(a0));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 45:
      HASH_GUARD(0x3DAC04A51FD2D92DLL, appenddata) {
        return (t_appenddata(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 50:
      HASH_GUARD(0x2D8D2253C2149972LL, substringdata) {
        return (t_substringdata(a0, a1));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 52:
      HASH_GUARD(0x470F2F0EA1F3D4B4LL, replacedata) {
        return (t_replacedata(a0, a1, a2));
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    case 61:
      HASH_GUARD(0x0E19CE7886BCEF3DLL, insertdata) {
        return (t_insertdata(a0, a1));
      }
      break;
    default:
      break;
  }
  return c_domcharacterdata::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domcomment::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_domcharacterdata::os_invoke(c, s, params, hash, fatal);
}
Variant c_domcomment::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
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
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
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
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
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
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlineno());
      }
      HASH_GUARD(0x7543EC746714D118LL, deletedata) {
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
        return (t_deletedata(a0, a1));
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
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
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
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
        return (t_removechild(a0));
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
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
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
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
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
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 45:
      HASH_GUARD(0x3DAC04A51FD2D92DLL, appenddata) {
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
        return (t_appenddata(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
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
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_hasattributes());
      }
      break;
    case 50:
      HASH_GUARD(0x2D8D2253C2149972LL, substringdata) {
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
        return (t_substringdata(a0, a1));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
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
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 52:
      HASH_GUARD(0x470F2F0EA1F3D4B4LL, replacedata) {
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
        return (t_replacedata(a0, a1, a2));
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
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
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_haschildnodes());
      }
      break;
    case 61:
      HASH_GUARD(0x0E19CE7886BCEF3DLL, insertdata) {
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
        return (t_insertdata(a0, a1));
      }
      break;
    default:
      break;
  }
  return c_domcharacterdata::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domcomment::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_domcharacterdata::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domcomment$os_get(const char *s) {
  return c_domcomment::os_get(s, -1);
}
Variant &cw_domcomment$os_lval(const char *s) {
  return c_domcomment::os_lval(s, -1);
}
Variant cw_domcomment$os_constant(const char *s) {
  return c_domcomment::os_constant(s);
}
Variant cw_domcomment$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domcomment::os_invoke(c, s, params, -1, fatal);
}
Object co_sqlite3(CArrRef params, bool init /* = true */) {
  return Object(p_sqlite3(NEW(c_sqlite3)())->dynCreate(params, init));
}
Variant c_sqlite3::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_sqlite3::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_sqlite3::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_sqlite3::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_sqlite3::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_sqlite3::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_sqlite3::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_sqlite3::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(sqlite3)
ObjectData *c_sqlite3::create() {
  init();
  t___construct();
  return this;
}
ObjectData *c_sqlite3::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create());
  } else return this;
}
void c_sqlite3::dynConstruct(CArrRef params) {
  (t___construct());
}
void c_sqlite3::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_sqlite3::cloneImpl() {
  c_sqlite3 *obj = NEW(c_sqlite3)();
  cloneSet(obj);
  return obj;
}
void c_sqlite3::cloneSet(c_sqlite3 *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_sqlite3::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 1:
      HASH_GUARD(0x78AE97BFBEBF5341LL, close) {
        return (t_close());
      }
      break;
    case 2:
      HASH_GUARD(0x6081F282094466C2LL, lasterrormsg) {
        return (t_lasterrormsg());
      }
      HASH_GUARD(0x65BD6700742BF3C2LL, openblob) {
        int count = params.size();
        if (count <= 3) return (t_openblob(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_openblob(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 3:
      HASH_GUARD(0x3AA4CCEC6BD68143LL, createaggregate) {
        int count = params.size();
        if (count <= 3) return (t_createaggregate(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_createaggregate(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 4:
      HASH_GUARD(0x21F68C010C124BC4LL, open) {
        int count = params.size();
        if (count <= 1) return (t_open(params.rvalAt(0)), null);
        if (count == 2) return (t_open(params.rvalAt(0), params.rvalAt(1)), null);
        return (t_open(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)), null);
      }
      break;
    case 10:
      HASH_GUARD(0x632B468FC2FCAB0ALL, lasterrorcode) {
        return (t_lasterrorcode());
      }
      break;
    case 12:
      HASH_GUARD(0x4F1D1ED7B087208CLL, exec) {
        return (t_exec(params.rvalAt(0)));
      }
      break;
    case 13:
      HASH_GUARD(0x7CECF508291057CDLL, createfunction) {
        int count = params.size();
        if (count <= 2) return (t_createfunction(params.rvalAt(0), params.rvalAt(1)));
        return (t_createfunction(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 25:
      HASH_GUARD(0x64D2BDB189B02819LL, changes) {
        return (t_changes());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    case 36:
      HASH_GUARD(0x1B3CDE890A487664LL, loadextension) {
        return (t_loadextension(params.rvalAt(0)));
      }
      break;
    case 38:
      HASH_GUARD(0x5024F08DB0FDCBE6LL, lastinsertrowid) {
        return (t_lastinsertrowid());
      }
      break;
    case 42:
      HASH_GUARD(0x5AE41239FF63D86ALL, version) {
        return (t_version());
      }
      break;
    case 46:
      HASH_GUARD(0x156B1FAE4132ABEELL, querysingle) {
        int count = params.size();
        if (count <= 1) return (t_querysingle(params.rvalAt(0)));
        return (t_querysingle(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 47:
      HASH_GUARD(0x540020AE4BA591AFLL, prepare) {
        return (t_prepare(params.rvalAt(0)));
      }
      break;
    case 51:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x607C4C73BB251635LL, escapestring) {
        return (t_escapestring(params.rvalAt(0)));
      }
      break;
    case 55:
      HASH_GUARD(0x356758D4414DA377LL, query) {
        return (t_query(params.rvalAt(0)));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_sqlite3::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 1:
      HASH_GUARD(0x78AE97BFBEBF5341LL, close) {
        return (t_close());
      }
      break;
    case 2:
      HASH_GUARD(0x6081F282094466C2LL, lasterrormsg) {
        return (t_lasterrormsg());
      }
      HASH_GUARD(0x65BD6700742BF3C2LL, openblob) {
        if (count <= 3) return (t_openblob(a0, a1, a2));
        return (t_openblob(a0, a1, a2, a3));
      }
      break;
    case 3:
      HASH_GUARD(0x3AA4CCEC6BD68143LL, createaggregate) {
        if (count <= 3) return (t_createaggregate(a0, a1, a2));
        return (t_createaggregate(a0, a1, a2, a3));
      }
      break;
    case 4:
      HASH_GUARD(0x21F68C010C124BC4LL, open) {
        if (count <= 1) return (t_open(a0), null);
        if (count == 2) return (t_open(a0, a1), null);
        return (t_open(a0, a1, a2), null);
      }
      break;
    case 10:
      HASH_GUARD(0x632B468FC2FCAB0ALL, lasterrorcode) {
        return (t_lasterrorcode());
      }
      break;
    case 12:
      HASH_GUARD(0x4F1D1ED7B087208CLL, exec) {
        return (t_exec(a0));
      }
      break;
    case 13:
      HASH_GUARD(0x7CECF508291057CDLL, createfunction) {
        if (count <= 2) return (t_createfunction(a0, a1));
        return (t_createfunction(a0, a1, a2));
      }
      break;
    case 25:
      HASH_GUARD(0x64D2BDB189B02819LL, changes) {
        return (t_changes());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    case 36:
      HASH_GUARD(0x1B3CDE890A487664LL, loadextension) {
        return (t_loadextension(a0));
      }
      break;
    case 38:
      HASH_GUARD(0x5024F08DB0FDCBE6LL, lastinsertrowid) {
        return (t_lastinsertrowid());
      }
      break;
    case 42:
      HASH_GUARD(0x5AE41239FF63D86ALL, version) {
        return (t_version());
      }
      break;
    case 46:
      HASH_GUARD(0x156B1FAE4132ABEELL, querysingle) {
        if (count <= 1) return (t_querysingle(a0));
        return (t_querysingle(a0, a1));
      }
      break;
    case 47:
      HASH_GUARD(0x540020AE4BA591AFLL, prepare) {
        return (t_prepare(a0));
      }
      break;
    case 51:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x607C4C73BB251635LL, escapestring) {
        return (t_escapestring(a0));
      }
      break;
    case 55:
      HASH_GUARD(0x356758D4414DA377LL, query) {
        return (t_query(a0));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_sqlite3::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_sqlite3::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
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
      break;
    case 2:
      HASH_GUARD(0x6081F282094466C2LL, lasterrormsg) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_lasterrormsg());
      }
      HASH_GUARD(0x65BD6700742BF3C2LL, openblob) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 3) return (t_openblob(a0, a1, a2));
        return (t_openblob(a0, a1, a2, a3));
      }
      break;
    case 3:
      HASH_GUARD(0x3AA4CCEC6BD68143LL, createaggregate) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 3) return (t_createaggregate(a0, a1, a2));
        return (t_createaggregate(a0, a1, a2, a3));
      }
      break;
    case 4:
      HASH_GUARD(0x21F68C010C124BC4LL, open) {
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
        if (count <= 1) return (t_open(a0), null);
        if (count == 2) return (t_open(a0, a1), null);
        return (t_open(a0, a1, a2), null);
      }
      break;
    case 10:
      HASH_GUARD(0x632B468FC2FCAB0ALL, lasterrorcode) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_lasterrorcode());
      }
      break;
    case 12:
      HASH_GUARD(0x4F1D1ED7B087208CLL, exec) {
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
        return (t_exec(a0));
      }
      break;
    case 13:
      HASH_GUARD(0x7CECF508291057CDLL, createfunction) {
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
        if (count <= 2) return (t_createfunction(a0, a1));
        return (t_createfunction(a0, a1, a2));
      }
      break;
    case 25:
      HASH_GUARD(0x64D2BDB189B02819LL, changes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_changes());
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
    case 36:
      HASH_GUARD(0x1B3CDE890A487664LL, loadextension) {
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
        return (t_loadextension(a0));
      }
      break;
    case 38:
      HASH_GUARD(0x5024F08DB0FDCBE6LL, lastinsertrowid) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_lastinsertrowid());
      }
      break;
    case 42:
      HASH_GUARD(0x5AE41239FF63D86ALL, version) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_version());
      }
      break;
    case 46:
      HASH_GUARD(0x156B1FAE4132ABEELL, querysingle) {
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
        if (count <= 1) return (t_querysingle(a0));
        return (t_querysingle(a0, a1));
      }
      break;
    case 47:
      HASH_GUARD(0x540020AE4BA591AFLL, prepare) {
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
        return (t_prepare(a0));
      }
      break;
    case 51:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x607C4C73BB251635LL, escapestring) {
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
        return (t_escapestring(a0));
      }
      break;
    case 55:
      HASH_GUARD(0x356758D4414DA377LL, query) {
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
        return (t_query(a0));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_sqlite3::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_sqlite3$os_get(const char *s) {
  return c_sqlite3::os_get(s, -1);
}
Variant &cw_sqlite3$os_lval(const char *s) {
  return c_sqlite3::os_lval(s, -1);
}
Variant cw_sqlite3$os_constant(const char *s) {
  return c_sqlite3::os_constant(s);
}
Variant cw_sqlite3$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_sqlite3::os_invoke(c, s, params, -1, fatal);
}
Object co_domattr(CArrRef params, bool init /* = true */) {
  return Object(p_domattr(NEW(c_domattr)())->dynCreate(params, init));
}
Variant c_domattr::os_get(const char *s, int64 hash) {
  return c_domnode::os_get(s, hash);
}
Variant &c_domattr::os_lval(const char *s, int64 hash) {
  return c_domnode::os_lval(s, hash);
}
void c_domattr::o_get(ArrayElementVec &props) const {
  c_domnode::o_get(props);
}
bool c_domattr::o_exists(CStrRef s, int64 hash) const {
  return c_domnode::o_exists(s, hash);
}
Variant c_domattr::o_get(CStrRef s, int64 hash) {
  return c_domnode::o_get(s, hash);
}
Variant c_domattr::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_domnode::o_set(s, hash, v, forInit);
}
Variant &c_domattr::o_lval(CStrRef s, int64 hash) {
  return c_domnode::o_lval(s, hash);
}
Variant c_domattr::os_constant(const char *s) {
  return c_domnode::os_constant(s);
}
IMPLEMENT_CLASS(domattr)
ObjectData *c_domattr::create(Variant a0, Variant a1) {
  init();
  t___construct(a0, a1);
  return this;
}
ObjectData *c_domattr::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0), params.rvalAt(1)));
  } else return this;
}
void c_domattr::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0), params.rvalAt(1)));
}
void c_domattr::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domattr::cloneImpl() {
  c_domattr *obj = NEW(c_domattr)();
  cloneSet(obj);
  return obj;
}
void c_domattr::cloneSet(c_domattr *clone) {
  c_domnode::cloneSet(clone);
}
Variant c_domattr::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(params.rvalAt(0)));
        if (count == 2) return (t_c14n(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(params.rvalAt(0)));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        int count = params.size();
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(params.rvalAt(0)));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(params.rvalAt(0)));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(params.rvalAt(0)));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(params.rvalAt(0)));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(params.rvalAt(0)));
      }
      HASH_GUARD(0x30A9329415D17F24LL, isid) {
        return (t_isid());
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(params.rvalAt(0)));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        int count = params.size();
        if (count <= 1) return (t_insertbefore(params.rvalAt(0)));
        return (t_insertbefore(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        int count = params.size();
        if (count <= 1) return (t_c14nfile(params.rvalAt(0)));
        if (count == 2) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke(s, params, hash, fatal);
}
Variant c_domattr::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(a0));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0, a1), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(a0));
      }
      HASH_GUARD(0x30A9329415D17F24LL, isid) {
        return (t_isid());
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domattr::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_domnode::os_invoke(c, s, params, hash, fatal);
}
Variant c_domattr::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
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
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
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
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
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
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
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
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
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
        return (t_removechild(a0));
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
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
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
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
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
        return (t_isdefaultnamespace(a0));
      }
      HASH_GUARD(0x30A9329415D17F24LL, isid) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_isid());
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
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
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
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
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
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
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domattr::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_domnode::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domattr$os_get(const char *s) {
  return c_domattr::os_get(s, -1);
}
Variant &cw_domattr$os_lval(const char *s) {
  return c_domattr::os_lval(s, -1);
}
Variant cw_domattr$os_constant(const char *s) {
  return c_domattr::os_constant(s);
}
Variant cw_domattr$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domattr::os_invoke(c, s, params, -1, fatal);
}
Object co_soapvar(CArrRef params, bool init /* = true */) {
  return Object(p_soapvar(NEW(c_soapvar)())->dynCreate(params, init));
}
Variant c_soapvar::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_soapvar::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_soapvar::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_soapvar::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_soapvar::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_soapvar::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_soapvar::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_soapvar::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(soapvar)
ObjectData *c_soapvar::create(Variant a0, Variant a1, String a2, String a3, String a4, String a5) {
  init();
  t___construct(a0, a1, a2, a3, a4, a5);
  return this;
}
ObjectData *c_soapvar::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    int count = params.size();
    if (count <= 2) return (create(params.rvalAt(0), params.rvalAt(1)));
    if (count == 3) return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
    if (count == 4) return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
    if (count == 5) return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
    return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4), params.rvalAt(5)));
  } else return this;
}
void c_soapvar::dynConstruct(CArrRef params) {
  int count = params.size();
  if (count <= 2) (t___construct(params.rvalAt(0), params.rvalAt(1)));
  if (count == 3) (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
  if (count == 4) (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
  if (count == 5) (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
  (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4), params.rvalAt(5)));
}
void c_soapvar::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_soapvar::cloneImpl() {
  c_soapvar *obj = NEW(c_soapvar)();
  cloneSet(obj);
  return obj;
}
void c_soapvar::cloneSet(c_soapvar *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_soapvar::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 3) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 2) return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
        if (count == 3) return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)), null);
        if (count == 4) return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)), null);
        if (count == 5) return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4), params.rvalAt(5)), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_soapvar::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 3) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 2) return (t___construct(a0, a1), null);
        if (count == 3) return (t___construct(a0, a1, a2), null);
        if (count == 4) return (t___construct(a0, a1, a2, a3), null);
        if (count == 5) return (t___construct(a0, a1, a2, a3, a4), null);
        return (t___construct(a0, a1, a2, a3, a4, a5), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_soapvar::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_soapvar::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 3) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
        Variant a5;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a5 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 2) return (t___construct(a0, a1), null);
        if (count == 3) return (t___construct(a0, a1, a2), null);
        if (count == 4) return (t___construct(a0, a1, a2, a3), null);
        if (count == 5) return (t___construct(a0, a1, a2, a3, a4), null);
        return (t___construct(a0, a1, a2, a3, a4, a5), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_soapvar::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_soapvar$os_get(const char *s) {
  return c_soapvar::os_get(s, -1);
}
Variant &cw_soapvar$os_lval(const char *s) {
  return c_soapvar::os_lval(s, -1);
}
Variant cw_soapvar$os_constant(const char *s) {
  return c_soapvar::os_constant(s);
}
Variant cw_soapvar$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_soapvar::os_invoke(c, s, params, -1, fatal);
}
Object co_domnamednodemap(CArrRef params, bool init /* = true */) {
  return Object(p_domnamednodemap(NEW(c_domnamednodemap)())->dynCreate(params, init));
}
Variant c_domnamednodemap::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_domnamednodemap::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_domnamednodemap::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_domnamednodemap::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_domnamednodemap::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_domnamednodemap::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_domnamednodemap::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_domnamednodemap::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(domnamednodemap)
ObjectData *c_domnamednodemap::create() {
  init();
  t___construct();
  return this;
}
ObjectData *c_domnamednodemap::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create());
  } else return this;
}
void c_domnamednodemap::dynConstruct(CArrRef params) {
  (t___construct());
}
void c_domnamednodemap::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domnamednodemap::cloneImpl() {
  c_domnamednodemap *obj = NEW(c_domnamednodemap)();
  cloneSet(obj);
  return obj;
}
void c_domnamednodemap::cloneSet(c_domnamednodemap *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_domnamednodemap::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 6:
      HASH_GUARD(0x0A41DBE0830902C6LL, item) {
        return (t_item(params.rvalAt(0)));
      }
      break;
    case 10:
      HASH_GUARD(0x5D3CEC627F9ADC3ALL, getnameditem) {
        return (t_getnameditem(params.rvalAt(0)));
      }
      break;
    case 11:
      HASH_GUARD(0x53B4D840FF4F0F1BLL, getnameditemns) {
        return (t_getnameditemns(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_domnamednodemap::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 6:
      HASH_GUARD(0x0A41DBE0830902C6LL, item) {
        return (t_item(a0));
      }
      break;
    case 10:
      HASH_GUARD(0x5D3CEC627F9ADC3ALL, getnameditem) {
        return (t_getnameditem(a0));
      }
      break;
    case 11:
      HASH_GUARD(0x53B4D840FF4F0F1BLL, getnameditemns) {
        return (t_getnameditemns(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domnamednodemap::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_domnamednodemap::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 6:
      HASH_GUARD(0x0A41DBE0830902C6LL, item) {
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
        return (t_item(a0));
      }
      break;
    case 10:
      HASH_GUARD(0x5D3CEC627F9ADC3ALL, getnameditem) {
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
        return (t_getnameditem(a0));
      }
      break;
    case 11:
      HASH_GUARD(0x53B4D840FF4F0F1BLL, getnameditemns) {
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
        return (t_getnameditemns(a0, a1));
      }
      break;
    case 15:
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
Variant c_domnamednodemap::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domnamednodemap$os_get(const char *s) {
  return c_domnamednodemap::os_get(s, -1);
}
Variant &cw_domnamednodemap$os_lval(const char *s) {
  return c_domnamednodemap::os_lval(s, -1);
}
Variant cw_domnamednodemap$os_constant(const char *s) {
  return c_domnamednodemap::os_constant(s);
}
Variant cw_domnamednodemap$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domnamednodemap::os_invoke(c, s, params, -1, fatal);
}
Object co_sqlite3result(CArrRef params, bool init /* = true */) {
  return Object(p_sqlite3result(NEW(c_sqlite3result)())->dynCreate(params, init));
}
Variant c_sqlite3result::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_sqlite3result::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_sqlite3result::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_sqlite3result::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_sqlite3result::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_sqlite3result::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_sqlite3result::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_sqlite3result::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(sqlite3result)
ObjectData *c_sqlite3result::create() {
  init();
  t___construct();
  return this;
}
ObjectData *c_sqlite3result::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create());
  } else return this;
}
void c_sqlite3result::dynConstruct(CArrRef params) {
  (t___construct());
}
void c_sqlite3result::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_sqlite3result::cloneImpl() {
  c_sqlite3result *obj = NEW(c_sqlite3result)();
  cloneSet(obj);
  return obj;
}
void c_sqlite3result::cloneSet(c_sqlite3result *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_sqlite3result::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 0:
      HASH_GUARD(0x340A51AE22A924E0LL, reset) {
        return (t_reset());
      }
      break;
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 6:
      HASH_GUARD(0x4C7702FC22784996LL, numcolumns) {
        return (t_numcolumns());
      }
      break;
    case 8:
      HASH_GUARD(0x696EFC04EE7E47F8LL, columnname) {
        return (t_columnname(params.rvalAt(0)));
      }
      break;
    case 11:
      HASH_GUARD(0x59F73C4DEF1C3A6BLL, fetcharray) {
        int count = params.size();
        if (count <= 0) return (t_fetcharray());
        return (t_fetcharray(params.rvalAt(0)));
      }
      break;
    case 12:
      HASH_GUARD(0x32A58B0CE493957CLL, finalize) {
        return (t_finalize());
      }
      break;
    case 13:
      HASH_GUARD(0x42EDB5CA009D8C6DLL, columntype) {
        return (t_columntype(params.rvalAt(0)));
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_sqlite3result::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 0:
      HASH_GUARD(0x340A51AE22A924E0LL, reset) {
        return (t_reset());
      }
      break;
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 6:
      HASH_GUARD(0x4C7702FC22784996LL, numcolumns) {
        return (t_numcolumns());
      }
      break;
    case 8:
      HASH_GUARD(0x696EFC04EE7E47F8LL, columnname) {
        return (t_columnname(a0));
      }
      break;
    case 11:
      HASH_GUARD(0x59F73C4DEF1C3A6BLL, fetcharray) {
        if (count <= 0) return (t_fetcharray());
        return (t_fetcharray(a0));
      }
      break;
    case 12:
      HASH_GUARD(0x32A58B0CE493957CLL, finalize) {
        return (t_finalize());
      }
      break;
    case 13:
      HASH_GUARD(0x42EDB5CA009D8C6DLL, columntype) {
        return (t_columntype(a0));
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_sqlite3result::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_sqlite3result::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 0:
      HASH_GUARD(0x340A51AE22A924E0LL, reset) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_reset());
      }
      break;
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 6:
      HASH_GUARD(0x4C7702FC22784996LL, numcolumns) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_numcolumns());
      }
      break;
    case 8:
      HASH_GUARD(0x696EFC04EE7E47F8LL, columnname) {
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
        return (t_columnname(a0));
      }
      break;
    case 11:
      HASH_GUARD(0x59F73C4DEF1C3A6BLL, fetcharray) {
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
        if (count <= 0) return (t_fetcharray());
        return (t_fetcharray(a0));
      }
      break;
    case 12:
      HASH_GUARD(0x32A58B0CE493957CLL, finalize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_finalize());
      }
      break;
    case 13:
      HASH_GUARD(0x42EDB5CA009D8C6DLL, columntype) {
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
        return (t_columntype(a0));
      }
      break;
    case 15:
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
Variant c_sqlite3result::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_sqlite3result$os_get(const char *s) {
  return c_sqlite3result::os_get(s, -1);
}
Variant &cw_sqlite3result$os_lval(const char *s) {
  return c_sqlite3result::os_lval(s, -1);
}
Variant cw_sqlite3result$os_constant(const char *s) {
  return c_sqlite3result::os_constant(s);
}
Variant cw_sqlite3result$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_sqlite3result::os_invoke(c, s, params, -1, fatal);
}
Object co_simplexmlelement(CArrRef params, bool init /* = true */) {
  return Object(p_simplexmlelement(NEW(c_simplexmlelement)())->dynCreate(params, init));
}
Variant c_simplexmlelement::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_simplexmlelement::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_simplexmlelement::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_simplexmlelement::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_simplexmlelement::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_simplexmlelement::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_simplexmlelement::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_simplexmlelement::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(simplexmlelement)
ObjectData *c_simplexmlelement::create(String a0, int64 a1, bool a2, String a3, bool a4) {
  init();
  t___construct(a0, a1, a2, a3, a4);
  return this;
}
ObjectData *c_simplexmlelement::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    int count = params.size();
    if (count <= 1) return (create(params.rvalAt(0)));
    if (count == 2) return (create(params.rvalAt(0), params.rvalAt(1)));
    if (count == 3) return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
    if (count == 4) return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
    return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
  } else return this;
}
void c_simplexmlelement::dynConstruct(CArrRef params) {
  int count = params.size();
  if (count <= 1) (t___construct(params.rvalAt(0)));
  if (count == 2) (t___construct(params.rvalAt(0), params.rvalAt(1)));
  if (count == 3) (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
  if (count == 4) (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
  (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
}
void c_simplexmlelement::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_simplexmlelement::cloneImpl() {
  c_simplexmlelement *obj = NEW(c_simplexmlelement)();
  cloneSet(obj);
  return obj;
}
void c_simplexmlelement::cloneSet(c_simplexmlelement *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_simplexmlelement::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 0:
      HASH_GUARD(0x3E6BCFB9742FC700LL, offsetexists) {
        return (t_offsetexists(params.rvalAt(0)));
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 20:
      HASH_GUARD(0x2DDE12A9866FC794LL, addchild) {
        int count = params.size();
        if (count <= 1) return (t_addchild(params.rvalAt(0)));
        if (count == 2) return (t_addchild(params.rvalAt(0), params.rvalAt(1)));
        return (t_addchild(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 22:
      HASH_GUARD(0x52F7AA6C19BEEAD6LL, addattribute) {
        int count = params.size();
        if (count <= 1) return (t_addattribute(params.rvalAt(0)), null);
        if (count == 2) return (t_addattribute(params.rvalAt(0), params.rvalAt(1)), null);
        return (t_addattribute(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)), null);
      }
      break;
    case 24:
      HASH_GUARD(0x61D11ECEF4404498LL, offsetget) {
        return (t_offsetget(params.rvalAt(0)));
      }
      break;
    case 27:
      HASH_GUARD(0x0817454F196A9A1BLL, xpath) {
        return (t_xpath(params.rvalAt(0)));
      }
      break;
    case 29:
      HASH_GUARD(0x4B65E1A09A33F21DLL, attributes) {
        int count = params.size();
        if (count <= 0) return (t_attributes());
        if (count == 1) return (t_attributes(params.rvalAt(0)));
        return (t_attributes(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 1) return (t___construct(params.rvalAt(0)), null);
        if (count == 2) return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
        if (count == 3) return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)), null);
        if (count == 4) return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)), null);
      }
      break;
    case 37:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 38:
      HASH_GUARD(0x6D9D75B7E3B40FA6LL, getnamespaces) {
        int count = params.size();
        if (count <= 0) return (t_getnamespaces());
        return (t_getnamespaces(params.rvalAt(0)));
      }
      HASH_GUARD(0x5C09D9345FD864A6LL, asxml) {
        return (t_asxml(params.rvalAt(0)));
      }
      break;
    case 45:
      HASH_GUARD(0x2B299E949E2E82ADLL, children) {
        int count = params.size();
        if (count <= 0) return (t_children());
        if (count == 1) return (t_children(params.rvalAt(0)));
        return (t_children(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 49:
      HASH_GUARD(0x0D08AA5F0B2F34F1LL, getdocnamespaces) {
        int count = params.size();
        if (count <= 0) return (t_getdocnamespaces());
        return (t_getdocnamespaces(params.rvalAt(0)));
      }
      break;
    case 51:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 56:
      HASH_GUARD(0x0957F693A48AF738LL, offsetset) {
        return (t_offsetset(params.rvalAt(0), params.rvalAt(1)), null);
      }
      break;
    case 58:
      HASH_GUARD(0x08329980E6369ABALL, offsetunset) {
        return (t_offsetunset(params.rvalAt(0)), null);
      }
      break;
    case 61:
      HASH_GUARD(0x3FE8F6937854F83DLL, registerxpathnamespace) {
        return (t_registerxpathnamespace(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_simplexmlelement::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 0:
      HASH_GUARD(0x3E6BCFB9742FC700LL, offsetexists) {
        return (t_offsetexists(a0));
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      break;
    case 20:
      HASH_GUARD(0x2DDE12A9866FC794LL, addchild) {
        if (count <= 1) return (t_addchild(a0));
        if (count == 2) return (t_addchild(a0, a1));
        return (t_addchild(a0, a1, a2));
      }
      break;
    case 22:
      HASH_GUARD(0x52F7AA6C19BEEAD6LL, addattribute) {
        if (count <= 1) return (t_addattribute(a0), null);
        if (count == 2) return (t_addattribute(a0, a1), null);
        return (t_addattribute(a0, a1, a2), null);
      }
      break;
    case 24:
      HASH_GUARD(0x61D11ECEF4404498LL, offsetget) {
        return (t_offsetget(a0));
      }
      break;
    case 27:
      HASH_GUARD(0x0817454F196A9A1BLL, xpath) {
        return (t_xpath(a0));
      }
      break;
    case 29:
      HASH_GUARD(0x4B65E1A09A33F21DLL, attributes) {
        if (count <= 0) return (t_attributes());
        if (count == 1) return (t_attributes(a0));
        return (t_attributes(a0, a1));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 1) return (t___construct(a0), null);
        if (count == 2) return (t___construct(a0, a1), null);
        if (count == 3) return (t___construct(a0, a1, a2), null);
        if (count == 4) return (t___construct(a0, a1, a2, a3), null);
        return (t___construct(a0, a1, a2, a3, a4), null);
      }
      break;
    case 37:
      HASH_GUARD(0x23F51CDECC198965LL, getname) {
        return (t_getname());
      }
      break;
    case 38:
      HASH_GUARD(0x6D9D75B7E3B40FA6LL, getnamespaces) {
        if (count <= 0) return (t_getnamespaces());
        return (t_getnamespaces(a0));
      }
      HASH_GUARD(0x5C09D9345FD864A6LL, asxml) {
        return (t_asxml(a0));
      }
      break;
    case 45:
      HASH_GUARD(0x2B299E949E2E82ADLL, children) {
        if (count <= 0) return (t_children());
        if (count == 1) return (t_children(a0));
        return (t_children(a0, a1));
      }
      break;
    case 49:
      HASH_GUARD(0x0D08AA5F0B2F34F1LL, getdocnamespaces) {
        if (count <= 0) return (t_getdocnamespaces());
        return (t_getdocnamespaces(a0));
      }
      break;
    case 51:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 56:
      HASH_GUARD(0x0957F693A48AF738LL, offsetset) {
        return (t_offsetset(a0, a1), null);
      }
      break;
    case 58:
      HASH_GUARD(0x08329980E6369ABALL, offsetunset) {
        return (t_offsetunset(a0), null);
      }
      break;
    case 61:
      HASH_GUARD(0x3FE8F6937854F83DLL, registerxpathnamespace) {
        return (t_registerxpathnamespace(a0, a1));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_simplexmlelement::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_simplexmlelement::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
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
      HASH_GUARD(0x2DDE12A9866FC794LL, addchild) {
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
        if (count <= 1) return (t_addchild(a0));
        if (count == 2) return (t_addchild(a0, a1));
        return (t_addchild(a0, a1, a2));
      }
      break;
    case 22:
      HASH_GUARD(0x52F7AA6C19BEEAD6LL, addattribute) {
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
        if (count <= 1) return (t_addattribute(a0), null);
        if (count == 2) return (t_addattribute(a0, a1), null);
        return (t_addattribute(a0, a1, a2), null);
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
      break;
    case 27:
      HASH_GUARD(0x0817454F196A9A1BLL, xpath) {
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
        return (t_xpath(a0));
      }
      break;
    case 29:
      HASH_GUARD(0x4B65E1A09A33F21DLL, attributes) {
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
        if (count <= 0) return (t_attributes());
        if (count == 1) return (t_attributes(a0));
        return (t_attributes(a0, a1));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t___construct(a0), null);
        if (count == 2) return (t___construct(a0, a1), null);
        if (count == 3) return (t___construct(a0, a1, a2), null);
        if (count == 4) return (t___construct(a0, a1, a2, a3), null);
        return (t___construct(a0, a1, a2, a3, a4), null);
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
    case 38:
      HASH_GUARD(0x6D9D75B7E3B40FA6LL, getnamespaces) {
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
        if (count <= 0) return (t_getnamespaces());
        return (t_getnamespaces(a0));
      }
      HASH_GUARD(0x5C09D9345FD864A6LL, asxml) {
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
        return (t_asxml(a0));
      }
      break;
    case 45:
      HASH_GUARD(0x2B299E949E2E82ADLL, children) {
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
        if (count <= 0) return (t_children());
        if (count == 1) return (t_children(a0));
        return (t_children(a0, a1));
      }
      break;
    case 49:
      HASH_GUARD(0x0D08AA5F0B2F34F1LL, getdocnamespaces) {
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
        if (count <= 0) return (t_getdocnamespaces());
        return (t_getdocnamespaces(a0));
      }
      break;
    case 51:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 56:
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
    case 61:
      HASH_GUARD(0x3FE8F6937854F83DLL, registerxpathnamespace) {
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
        return (t_registerxpathnamespace(a0, a1));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_simplexmlelement::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_simplexmlelement$os_get(const char *s) {
  return c_simplexmlelement::os_get(s, -1);
}
Variant &cw_simplexmlelement$os_lval(const char *s) {
  return c_simplexmlelement::os_lval(s, -1);
}
Variant cw_simplexmlelement$os_constant(const char *s) {
  return c_simplexmlelement::os_constant(s);
}
Variant cw_simplexmlelement$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_simplexmlelement::os_invoke(c, s, params, -1, fatal);
}
Object co_domprocessinginstruction(CArrRef params, bool init /* = true */) {
  return Object(p_domprocessinginstruction(NEW(c_domprocessinginstruction)())->dynCreate(params, init));
}
Variant c_domprocessinginstruction::os_get(const char *s, int64 hash) {
  return c_domnode::os_get(s, hash);
}
Variant &c_domprocessinginstruction::os_lval(const char *s, int64 hash) {
  return c_domnode::os_lval(s, hash);
}
void c_domprocessinginstruction::o_get(ArrayElementVec &props) const {
  c_domnode::o_get(props);
}
bool c_domprocessinginstruction::o_exists(CStrRef s, int64 hash) const {
  return c_domnode::o_exists(s, hash);
}
Variant c_domprocessinginstruction::o_get(CStrRef s, int64 hash) {
  return c_domnode::o_get(s, hash);
}
Variant c_domprocessinginstruction::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_domnode::o_set(s, hash, v, forInit);
}
Variant &c_domprocessinginstruction::o_lval(CStrRef s, int64 hash) {
  return c_domnode::o_lval(s, hash);
}
Variant c_domprocessinginstruction::os_constant(const char *s) {
  return c_domnode::os_constant(s);
}
IMPLEMENT_CLASS(domprocessinginstruction)
ObjectData *c_domprocessinginstruction::create(Variant a0, Variant a1) {
  init();
  t___construct(a0, a1);
  return this;
}
ObjectData *c_domprocessinginstruction::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    int count = params.size();
    if (count <= 1) return (create(params.rvalAt(0)));
    return (create(params.rvalAt(0), params.rvalAt(1)));
  } else return this;
}
void c_domprocessinginstruction::dynConstruct(CArrRef params) {
  int count = params.size();
  if (count <= 1) (t___construct(params.rvalAt(0)));
  (t___construct(params.rvalAt(0), params.rvalAt(1)));
}
void c_domprocessinginstruction::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domprocessinginstruction::cloneImpl() {
  c_domprocessinginstruction *obj = NEW(c_domprocessinginstruction)();
  cloneSet(obj);
  return obj;
}
void c_domprocessinginstruction::cloneSet(c_domprocessinginstruction *clone) {
  c_domnode::cloneSet(clone);
}
Variant c_domprocessinginstruction::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(params.rvalAt(0)));
        if (count == 2) return (t_c14n(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(params.rvalAt(0)));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        int count = params.size();
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(params.rvalAt(0)));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(params.rvalAt(0)));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(params.rvalAt(0)));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(params.rvalAt(0)));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(params.rvalAt(0)));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(params.rvalAt(0)));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        int count = params.size();
        if (count <= 1) return (t_insertbefore(params.rvalAt(0)));
        return (t_insertbefore(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        int count = params.size();
        if (count <= 1) return (t_c14nfile(params.rvalAt(0)));
        if (count == 2) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke(s, params, hash, fatal);
}
Variant c_domprocessinginstruction::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(a0));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domprocessinginstruction::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_domnode::os_invoke(c, s, params, hash, fatal);
}
Variant c_domprocessinginstruction::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
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
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
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
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
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
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
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
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
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
        return (t_removechild(a0));
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
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
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
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
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
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
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
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
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
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
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
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domprocessinginstruction::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_domnode::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domprocessinginstruction$os_get(const char *s) {
  return c_domprocessinginstruction::os_get(s, -1);
}
Variant &cw_domprocessinginstruction$os_lval(const char *s) {
  return c_domprocessinginstruction::os_lval(s, -1);
}
Variant cw_domprocessinginstruction$os_constant(const char *s) {
  return c_domprocessinginstruction::os_constant(s);
}
Variant cw_domprocessinginstruction$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domprocessinginstruction::os_invoke(c, s, params, -1, fatal);
}
Object co_soapclient(CArrRef params, bool init /* = true */) {
  return Object(p_soapclient(NEW(c_soapclient)())->dynCreate(params, init));
}
Variant c_soapclient::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_soapclient::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_soapclient::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_soapclient::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_soapclient::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_soapclient::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_soapclient::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_soapclient::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(soapclient)
ObjectData *c_soapclient::create(Variant a0, Array a1) {
  init();
  t___construct(a0, a1);
  return this;
}
ObjectData *c_soapclient::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    int count = params.size();
    if (count <= 1) return (create(params.rvalAt(0)));
    return (create(params.rvalAt(0), params.rvalAt(1)));
  } else return this;
}
void c_soapclient::dynConstruct(CArrRef params) {
  int count = params.size();
  if (count <= 1) (t___construct(params.rvalAt(0)));
  (t___construct(params.rvalAt(0), params.rvalAt(1)));
}
void c_soapclient::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_soapclient::cloneImpl() {
  c_soapclient *obj = NEW(c_soapclient)();
  cloneSet(obj);
  return obj;
}
void c_soapclient::cloneSet(c_soapclient *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_soapclient::doCall(Variant v_name, Variant v_arguments, bool fatal) {
  return t___call(v_name, v_arguments);
}
Variant c_soapclient::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 4:
      HASH_GUARD(0x5C5C1E24A140F6E4LL, __getlastrequest) {
        return (t___getlastrequest());
      }
      break;
    case 12:
      HASH_GUARD(0x3648EE7D3B37DE8CLL, __getlastrequestheaders) {
        return (t___getlastrequestheaders());
      }
      HASH_GUARD(0x5D73364F53CEEB6CLL, __call) {
        return (t___call(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 17:
      HASH_GUARD(0x1365B3105FC59871LL, __getfunctions) {
        return (t___getfunctions());
      }
      break;
    case 19:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 20:
      HASH_GUARD(0x3CA9145031035A54LL, __getlastresponse) {
        return (t___getlastresponse());
      }
      break;
    case 21:
      HASH_GUARD(0x19A26CEC7E8AAEF5LL, __dorequest) {
        int count = params.size();
        if (count <= 4) return (t___dorequest(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t___dorequest(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
      }
      HASH_GUARD(0x52C6ABACCBFD0C75LL, __setcookie) {
        int count = params.size();
        if (count <= 1) return (t___setcookie(params.rvalAt(0)));
        return (t___setcookie(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x628D398E7604B9D5LL, __gettypes) {
        return (t___gettypes());
      }
      break;
    case 25:
      HASH_GUARD(0x4A2F89F662050D99LL, __soapcall) {
        int count = params.size();
        if (count <= 2) return (t___soapcall(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t___soapcall(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t___soapcall(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t___soapcall(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), ref(const_cast<Array&>(params).lvalAt(4))));
      }
      break;
    case 26:
      HASH_GUARD(0x0C94657A58A82CFALL, __getlastresponseheaders) {
        return (t___getlastresponseheaders());
      }
      break;
    case 27:
      HASH_GUARD(0x14842228D5E335BBLL, __setsoapheaders) {
        int count = params.size();
        if (count <= 0) return (t___setsoapheaders());
        return (t___setsoapheaders(params.rvalAt(0)));
      }
      break;
    case 31:
      HASH_GUARD(0x232D13904DBC36FFLL, __setlocation) {
        int count = params.size();
        if (count <= 0) return (t___setlocation());
        return (t___setlocation(params.rvalAt(0)));
      }
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_soapclient::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 4:
      HASH_GUARD(0x5C5C1E24A140F6E4LL, __getlastrequest) {
        return (t___getlastrequest());
      }
      break;
    case 12:
      HASH_GUARD(0x3648EE7D3B37DE8CLL, __getlastrequestheaders) {
        return (t___getlastrequestheaders());
      }
      HASH_GUARD(0x5D73364F53CEEB6CLL, __call) {
        return (t___call(a0, a1));
      }
      break;
    case 17:
      HASH_GUARD(0x1365B3105FC59871LL, __getfunctions) {
        return (t___getfunctions());
      }
      break;
    case 19:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 20:
      HASH_GUARD(0x3CA9145031035A54LL, __getlastresponse) {
        return (t___getlastresponse());
      }
      break;
    case 21:
      HASH_GUARD(0x19A26CEC7E8AAEF5LL, __dorequest) {
        if (count <= 4) return (t___dorequest(a0, a1, a2, a3));
        return (t___dorequest(a0, a1, a2, a3, a4));
      }
      HASH_GUARD(0x52C6ABACCBFD0C75LL, __setcookie) {
        if (count <= 1) return (t___setcookie(a0));
        return (t___setcookie(a0, a1));
      }
      HASH_GUARD(0x628D398E7604B9D5LL, __gettypes) {
        return (t___gettypes());
      }
      break;
    case 25:
      HASH_GUARD(0x4A2F89F662050D99LL, __soapcall) {
        if (count <= 2) return (t___soapcall(a0, a1));
        if (count == 3) return (t___soapcall(a0, a1, a2));
        if (count == 4) return (t___soapcall(a0, a1, a2, a3));
        return (t___soapcall(a0, a1, a2, a3, ref(a4)));
      }
      break;
    case 26:
      HASH_GUARD(0x0C94657A58A82CFALL, __getlastresponseheaders) {
        return (t___getlastresponseheaders());
      }
      break;
    case 27:
      HASH_GUARD(0x14842228D5E335BBLL, __setsoapheaders) {
        if (count <= 0) return (t___setsoapheaders());
        return (t___setsoapheaders(a0));
      }
      break;
    case 31:
      HASH_GUARD(0x232D13904DBC36FFLL, __setlocation) {
        if (count <= 0) return (t___setlocation());
        return (t___setlocation(a0));
      }
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_soapclient::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_soapclient::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 4:
      HASH_GUARD(0x5C5C1E24A140F6E4LL, __getlastrequest) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___getlastrequest());
      }
      break;
    case 12:
      HASH_GUARD(0x3648EE7D3B37DE8CLL, __getlastrequestheaders) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___getlastrequestheaders());
      }
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
      HASH_GUARD(0x1365B3105FC59871LL, __getfunctions) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___getfunctions());
      }
      break;
    case 19:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 20:
      HASH_GUARD(0x3CA9145031035A54LL, __getlastresponse) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___getlastresponse());
      }
      break;
    case 21:
      HASH_GUARD(0x19A26CEC7E8AAEF5LL, __dorequest) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 4) return (t___dorequest(a0, a1, a2, a3));
        return (t___dorequest(a0, a1, a2, a3, a4));
      }
      HASH_GUARD(0x52C6ABACCBFD0C75LL, __setcookie) {
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
        if (count <= 1) return (t___setcookie(a0));
        return (t___setcookie(a0, a1));
      }
      HASH_GUARD(0x628D398E7604B9D5LL, __gettypes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___gettypes());
      }
      break;
    case 25:
      HASH_GUARD(0x4A2F89F662050D99LL, __soapcall) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = ref((*it)->refval(env));
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 2) return (t___soapcall(a0, a1));
        if (count == 3) return (t___soapcall(a0, a1, a2));
        if (count == 4) return (t___soapcall(a0, a1, a2, a3));
        return (t___soapcall(a0, a1, a2, a3, ref(a4)));
      }
      break;
    case 26:
      HASH_GUARD(0x0C94657A58A82CFALL, __getlastresponseheaders) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___getlastresponseheaders());
      }
      break;
    case 27:
      HASH_GUARD(0x14842228D5E335BBLL, __setsoapheaders) {
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
        if (count <= 0) return (t___setsoapheaders());
        return (t___setsoapheaders(a0));
      }
      break;
    case 31:
      HASH_GUARD(0x232D13904DBC36FFLL, __setlocation) {
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
        if (count <= 0) return (t___setlocation());
        return (t___setlocation(a0));
      }
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
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_soapclient::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_soapclient$os_get(const char *s) {
  return c_soapclient::os_get(s, -1);
}
Variant &cw_soapclient$os_lval(const char *s) {
  return c_soapclient::os_lval(s, -1);
}
Variant cw_soapclient$os_constant(const char *s) {
  return c_soapclient::os_constant(s);
}
Variant cw_soapclient$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_soapclient::os_invoke(c, s, params, -1, fatal);
}
Object co_soapparam(CArrRef params, bool init /* = true */) {
  return Object(p_soapparam(NEW(c_soapparam)())->dynCreate(params, init));
}
Variant c_soapparam::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_soapparam::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_soapparam::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_soapparam::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_soapparam::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_soapparam::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_soapparam::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_soapparam::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(soapparam)
ObjectData *c_soapparam::create(Variant a0, String a1) {
  init();
  t___construct(a0, a1);
  return this;
}
ObjectData *c_soapparam::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0), params.rvalAt(1)));
  } else return this;
}
void c_soapparam::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0), params.rvalAt(1)));
}
void c_soapparam::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_soapparam::cloneImpl() {
  c_soapparam *obj = NEW(c_soapparam)();
  cloneSet(obj);
  return obj;
}
void c_soapparam::cloneSet(c_soapparam *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_soapparam::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 3) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_soapparam::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 3) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0, a1), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_soapparam::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_soapparam::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 3) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
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
Variant c_soapparam::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_soapparam$os_get(const char *s) {
  return c_soapparam::os_get(s, -1);
}
Variant &cw_soapparam$os_lval(const char *s) {
  return c_soapparam::os_lval(s, -1);
}
Variant cw_soapparam$os_constant(const char *s) {
  return c_soapparam::os_constant(s);
}
Variant cw_soapparam$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_soapparam::os_invoke(c, s, params, -1, fatal);
}
Object co_datetime(CArrRef params, bool init /* = true */) {
  return Object(p_datetime(NEW(c_datetime)())->dynCreate(params, init));
}
Variant c_datetime::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_datetime::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_datetime::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_datetime::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_datetime::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_datetime::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_datetime::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_datetime::os_constant(const char *s) {
  int64 hash = hash_string(s);
  switch (hash & 31) {
    case 4:
      HASH_RETURN(0x65F665B8975D4C44LL, q_datetime_RSS, RSS);
      break;
    case 5:
      HASH_RETURN(0x4C70260A54584B05LL, q_datetime_ATOM, ATOM);
      break;
    case 6:
      HASH_RETURN(0x68B7C078B5B92826LL, q_datetime_RFC1123, RFC1123);
      break;
    case 8:
      HASH_RETURN(0x53F6E373F1EBB708LL, q_datetime_RFC822, RFC822);
      break;
    case 12:
      HASH_RETURN(0x2183317DB64F7D2CLL, q_datetime_COOKIE, COOKIE);
      break;
    case 16:
      HASH_RETURN(0x5B0E14CE27A57F90LL, q_datetime_RFC3339, RFC3339);
      HASH_RETURN(0x5D2FBC8B1577ACB0LL, q_datetime_W3C, W3C);
      break;
    case 17:
      HASH_RETURN(0x66C93EBB188AAF11LL, q_datetime_RFC1036, RFC1036);
      break;
    case 18:
      HASH_RETURN(0x1AE03F7E0EC93DD2LL, q_datetime_RFC850, RFC850);
      break;
    case 27:
      HASH_RETURN(0x6D8D5D3FBA38037BLL, q_datetime_ISO8601, ISO8601);
      break;
    case 31:
      HASH_RETURN(0x2C528E9FD5EDAE9FLL, q_datetime_RFC2822, RFC2822);
      break;
    default:
      break;
  }
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(datetime)
ObjectData *c_datetime::create(String a0, Object a1) {
  init();
  t___construct(a0, a1);
  return this;
}
ObjectData *c_datetime::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    int count = params.size();
    if (count <= 0) return (create());
    if (count == 1) return (create(params.rvalAt(0)));
    return (create(params.rvalAt(0), params.rvalAt(1)));
  } else return this;
}
void c_datetime::dynConstruct(CArrRef params) {
  int count = params.size();
  if (count <= 0) (t___construct());
  if (count == 1) (t___construct(params.rvalAt(0)));
  (t___construct(params.rvalAt(0), params.rvalAt(1)));
}
void c_datetime::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_datetime::cloneImpl() {
  c_datetime *obj = NEW(c_datetime)();
  cloneSet(obj);
  return obj;
}
void c_datetime::cloneSet(c_datetime *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_datetime::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 7:
      HASH_GUARD(0x65A9B2950F080587LL, modify) {
        return (t_modify(params.rvalAt(0)));
      }
      break;
    case 11:
      HASH_GUARD(0x67314943DC9D120BLL, setdate) {
        return (t_setdate(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      HASH_GUARD(0x68DB66F60B55BD0BLL, getoffset) {
        return (t_getoffset());
      }
      break;
    case 12:
      HASH_GUARD(0x688601F33BE7716CLL, gettimezone) {
        return (t_gettimezone());
      }
      break;
    case 19:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 20:
      HASH_GUARD(0x16D6DBFD6EE55A74LL, setisodate) {
        int count = params.size();
        if (count <= 2) return (t_setisodate(params.rvalAt(0), params.rvalAt(1)));
        return (t_setisodate(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 24:
      HASH_GUARD(0x07CB96AB34F4C258LL, settime) {
        int count = params.size();
        if (count <= 2) return (t_settime(params.rvalAt(0), params.rvalAt(1)));
        return (t_settime(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      HASH_GUARD(0x3B576E558B957CB8LL, settimezone) {
        return (t_settimezone(params.rvalAt(0)));
      }
      break;
    case 27:
      HASH_GUARD(0x1B6DDAF6AAF8CA9BLL, format) {
        return (t_format(params.rvalAt(0)));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_datetime::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 7:
      HASH_GUARD(0x65A9B2950F080587LL, modify) {
        return (t_modify(a0));
      }
      break;
    case 11:
      HASH_GUARD(0x67314943DC9D120BLL, setdate) {
        return (t_setdate(a0, a1, a2));
      }
      HASH_GUARD(0x68DB66F60B55BD0BLL, getoffset) {
        return (t_getoffset());
      }
      break;
    case 12:
      HASH_GUARD(0x688601F33BE7716CLL, gettimezone) {
        return (t_gettimezone());
      }
      break;
    case 19:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 20:
      HASH_GUARD(0x16D6DBFD6EE55A74LL, setisodate) {
        if (count <= 2) return (t_setisodate(a0, a1));
        return (t_setisodate(a0, a1, a2));
      }
      break;
    case 24:
      HASH_GUARD(0x07CB96AB34F4C258LL, settime) {
        if (count <= 2) return (t_settime(a0, a1));
        return (t_settime(a0, a1, a2));
      }
      HASH_GUARD(0x3B576E558B957CB8LL, settimezone) {
        return (t_settimezone(a0));
      }
      break;
    case 27:
      HASH_GUARD(0x1B6DDAF6AAF8CA9BLL, format) {
        return (t_format(a0));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_datetime::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_datetime::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 7:
      HASH_GUARD(0x65A9B2950F080587LL, modify) {
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
        return (t_modify(a0));
      }
      break;
    case 11:
      HASH_GUARD(0x67314943DC9D120BLL, setdate) {
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
        return (t_setdate(a0, a1, a2));
      }
      HASH_GUARD(0x68DB66F60B55BD0BLL, getoffset) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getoffset());
      }
      break;
    case 12:
      HASH_GUARD(0x688601F33BE7716CLL, gettimezone) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_gettimezone());
      }
      break;
    case 19:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 20:
      HASH_GUARD(0x16D6DBFD6EE55A74LL, setisodate) {
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
        if (count <= 2) return (t_setisodate(a0, a1));
        return (t_setisodate(a0, a1, a2));
      }
      break;
    case 24:
      HASH_GUARD(0x07CB96AB34F4C258LL, settime) {
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
        if (count <= 2) return (t_settime(a0, a1));
        return (t_settime(a0, a1, a2));
      }
      HASH_GUARD(0x3B576E558B957CB8LL, settimezone) {
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
        return (t_settimezone(a0));
      }
      break;
    case 27:
      HASH_GUARD(0x1B6DDAF6AAF8CA9BLL, format) {
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
        return (t_format(a0));
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
        if (count <= 0) return (t___construct(), null);
        if (count == 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_datetime::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_datetime$os_get(const char *s) {
  return c_datetime::os_get(s, -1);
}
Variant &cw_datetime$os_lval(const char *s) {
  return c_datetime::os_lval(s, -1);
}
Variant cw_datetime$os_constant(const char *s) {
  return c_datetime::os_constant(s);
}
Variant cw_datetime$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_datetime::os_invoke(c, s, params, -1, fatal);
}
Object co_domentity(CArrRef params, bool init /* = true */) {
  return Object(p_domentity(NEW(c_domentity)())->dynCreate(params, init));
}
Variant c_domentity::os_get(const char *s, int64 hash) {
  return c_domnode::os_get(s, hash);
}
Variant &c_domentity::os_lval(const char *s, int64 hash) {
  return c_domnode::os_lval(s, hash);
}
void c_domentity::o_get(ArrayElementVec &props) const {
  c_domnode::o_get(props);
}
bool c_domentity::o_exists(CStrRef s, int64 hash) const {
  return c_domnode::o_exists(s, hash);
}
Variant c_domentity::o_get(CStrRef s, int64 hash) {
  return c_domnode::o_get(s, hash);
}
Variant c_domentity::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_domnode::o_set(s, hash, v, forInit);
}
Variant &c_domentity::o_lval(CStrRef s, int64 hash) {
  return c_domnode::o_lval(s, hash);
}
Variant c_domentity::os_constant(const char *s) {
  return c_domnode::os_constant(s);
}
IMPLEMENT_CLASS(domentity)
ObjectData *c_domentity::create() {
  init();
  t___construct();
  return this;
}
ObjectData *c_domentity::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create());
  } else return this;
}
void c_domentity::dynConstruct(CArrRef params) {
  (t___construct());
}
void c_domentity::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domentity::cloneImpl() {
  c_domentity *obj = NEW(c_domentity)();
  cloneSet(obj);
  return obj;
}
void c_domentity::cloneSet(c_domentity *clone) {
  c_domnode::cloneSet(clone);
}
Variant c_domentity::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(params.rvalAt(0)));
        if (count == 2) return (t_c14n(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(params.rvalAt(0)));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        int count = params.size();
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(params.rvalAt(0)));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(params.rvalAt(0)));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(params.rvalAt(0)));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(params.rvalAt(0)));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(params.rvalAt(0)));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(params.rvalAt(0)));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        int count = params.size();
        if (count <= 1) return (t_insertbefore(params.rvalAt(0)));
        return (t_insertbefore(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        int count = params.size();
        if (count <= 1) return (t_c14nfile(params.rvalAt(0)));
        if (count == 2) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke(s, params, hash, fatal);
}
Variant c_domentity::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(a0));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domentity::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_domnode::os_invoke(c, s, params, hash, fatal);
}
Variant c_domentity::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
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
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
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
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
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
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
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
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
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
        return (t_removechild(a0));
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
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
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
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
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
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
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
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
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
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
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
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domentity::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_domnode::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domentity$os_get(const char *s) {
  return c_domentity::os_get(s, -1);
}
Variant &cw_domentity$os_lval(const char *s) {
  return c_domentity::os_lval(s, -1);
}
Variant cw_domentity$os_constant(const char *s) {
  return c_domentity::os_constant(s);
}
Variant cw_domentity$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domentity::os_invoke(c, s, params, -1, fatal);
}
Object co_xmlwriter(CArrRef params, bool init /* = true */) {
  return Object(p_xmlwriter(NEW(c_xmlwriter)())->dynCreate(params, init));
}
Variant c_xmlwriter::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_xmlwriter::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_xmlwriter::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_xmlwriter::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_xmlwriter::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_xmlwriter::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_xmlwriter::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_xmlwriter::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(xmlwriter)
ObjectData *c_xmlwriter::create() {
  init();
  t___construct();
  return this;
}
ObjectData *c_xmlwriter::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create());
  } else return this;
}
void c_xmlwriter::dynConstruct(CArrRef params) {
  (t___construct());
}
void c_xmlwriter::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_xmlwriter::cloneImpl() {
  c_xmlwriter *obj = NEW(c_xmlwriter)();
  cloneSet(obj);
  return obj;
}
void c_xmlwriter::cloneSet(c_xmlwriter *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_xmlwriter::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 2:
      HASH_GUARD(0x1EBCBEA66B6B0982LL, writecdata) {
        return (t_writecdata(params.rvalAt(0)));
      }
      break;
    case 3:
      HASH_GUARD(0x55E29E84E96ACF83LL, startdtd) {
        int count = params.size();
        if (count <= 1) return (t_startdtd(params.rvalAt(0)));
        if (count == 2) return (t_startdtd(params.rvalAt(0), params.rvalAt(1)));
        return (t_startdtd(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 8:
      HASH_GUARD(0x7C1FF8A79BAB3608LL, writedtd) {
        int count = params.size();
        if (count <= 1) return (t_writedtd(params.rvalAt(0)));
        if (count == 2) return (t_writedtd(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_writedtd(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_writedtd(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 14:
      HASH_GUARD(0x119CC9E7C3E7EF0ELL, openmemory) {
        return (t_openmemory());
      }
      break;
    case 16:
      HASH_GUARD(0x0A7FE14C41766610LL, endcdata) {
        return (t_endcdata());
      }
      break;
    case 19:
      HASH_GUARD(0x75F8C992981A4093LL, writecomment) {
        return (t_writecomment(params.rvalAt(0)));
      }
      HASH_GUARD(0x5A7AE5A26994FF13LL, startattribute) {
        return (t_startattribute(params.rvalAt(0)));
      }
      break;
    case 28:
      HASH_GUARD(0x337EFFF3783B919CLL, writeelement) {
        int count = params.size();
        if (count <= 1) return (t_writeelement(params.rvalAt(0)));
        return (t_writeelement(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 29:
      HASH_GUARD(0x4D26D167066BB11DLL, text) {
        return (t_text(params.rvalAt(0)));
      }
      break;
    case 32:
      HASH_GUARD(0x7DF0368A44C01B20LL, startcdata) {
        return (t_startcdata());
      }
      break;
    case 33:
      HASH_GUARD(0x5D49A460168D5C21LL, endpi) {
        return (t_endpi());
      }
      break;
    case 41:
      HASH_GUARD(0x1D81ABDC9890C5A9LL, writedtdattlist) {
        return (t_writedtdattlist(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 43:
      HASH_GUARD(0x0DEAF425F1AD73ABLL, setindentstring) {
        return (t_setindentstring(params.rvalAt(0)));
      }
      break;
    case 47:
      HASH_GUARD(0x78A7A5D96EF2F4AFLL, endelement) {
        return (t_endelement());
      }
      break;
    case 53:
      HASH_GUARD(0x75B357D89DDE6EB5LL, endcomment) {
        return (t_endcomment());
      }
      break;
    case 55:
      HASH_GUARD(0x4B0F1F6361A59C37LL, writeraw) {
        return (t_writeraw(params.rvalAt(0)));
      }
      break;
    case 56:
      HASH_GUARD(0x7FC89CF1AFFB1E38LL, startdocument) {
        int count = params.size();
        if (count <= 0) return (t_startdocument());
        if (count == 1) return (t_startdocument(params.rvalAt(0)));
        if (count == 2) return (t_startdocument(params.rvalAt(0), params.rvalAt(1)));
        return (t_startdocument(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 58:
      HASH_GUARD(0x0D155D1E5C6641BALL, outputmemory) {
        int count = params.size();
        if (count <= 0) return (t_outputmemory());
        return (t_outputmemory(params.rvalAt(0)));
      }
      break;
    case 62:
      HASH_GUARD(0x1FBE635680ADB7BELL, openuri) {
        return (t_openuri(params.rvalAt(0)));
      }
      break;
    case 68:
      HASH_GUARD(0x275896BFD7A77144LL, enddtd) {
        return (t_enddtd());
      }
      break;
    case 70:
      HASH_GUARD(0x5D4650E5C0DAEE46LL, writedtdentity) {
        int count = params.size();
        if (count <= 2) return (t_writedtdentity(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_writedtdentity(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t_writedtdentity(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        if (count == 5) return (t_writedtdentity(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
        return (t_writedtdentity(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4), params.rvalAt(5)));
      }
      break;
    case 71:
      HASH_GUARD(0x017687FD7029CCC7LL, enddtdelement) {
        return (t_enddtdelement());
      }
      break;
    case 75:
      HASH_GUARD(0x3954A2C0306CCCCBLL, writeattribute) {
        return (t_writeattribute(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x7C43ECE80FF886CBLL, endattribute) {
        return (t_endattribute());
      }
      break;
    case 77:
      HASH_GUARD(0x4F1D890D5B36D24DLL, startattributens) {
        return (t_startattributens(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 82:
      HASH_GUARD(0x1357CA1E92397452LL, writedtdelement) {
        return (t_writedtdelement(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 84:
      HASH_GUARD(0x154AE94AEFAC7C54LL, writeattributens) {
        return (t_writeattributens(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 86:
      HASH_GUARD(0x4520735705A382D6LL, enddtdentity) {
        return (t_enddtdentity());
      }
      HASH_GUARD(0x37D51A94C6EE8F56LL, writepi) {
        return (t_writepi(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 88:
      HASH_GUARD(0x24A376E9310BF058LL, writeelementns) {
        int count = params.size();
        if (count <= 3) return (t_writeelementns(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_writeelementns(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 89:
      HASH_GUARD(0x390B6C01AE7C4159LL, startpi) {
        return (t_startpi(params.rvalAt(0)));
      }
      break;
    case 92:
      HASH_GUARD(0x0DC5AC1E0A2F63DCLL, enddocument) {
        return (t_enddocument());
      }
      break;
    case 93:
      HASH_GUARD(0x75A033B824EA1ADDLL, startdtdentity) {
        return (t_startdtdentity(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 95:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    case 98:
      HASH_GUARD(0x55BFFBE5717EFD62LL, startelement) {
        return (t_startelement(params.rvalAt(0)));
      }
      break;
    case 101:
      HASH_GUARD(0x349B7446B5EE65E5LL, setindent) {
        return (t_setindent(params.rvalAt(0)));
      }
      break;
    case 105:
      HASH_GUARD(0x608A2D39DE12E169LL, startelementns) {
        return (t_startelementns(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 112:
      HASH_GUARD(0x0CBB10FA542B7D70LL, startdtdattlist) {
        return (t_startdtdattlist(params.rvalAt(0)));
      }
      break;
    case 113:
      HASH_GUARD(0x496175DBD52E1771LL, enddtdattlist) {
        return (t_enddtdattlist());
      }
      break;
    case 115:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 120:
      HASH_GUARD(0x451436438EBA2AF8LL, startdtdelement) {
        return (t_startdtdelement(params.rvalAt(0)));
      }
      break;
    case 122:
      HASH_GUARD(0x6A3D9F8EDB005E7ALL, flush) {
        int count = params.size();
        if (count <= 0) return (t_flush());
        return (t_flush(params.rvalAt(0)));
      }
      break;
    case 126:
      HASH_GUARD(0x3174CCE00BFB9FFELL, fullendelement) {
        return (t_fullendelement());
      }
      break;
    case 127:
      HASH_GUARD(0x6A8CC27A7F0466FFLL, startcomment) {
        return (t_startcomment());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_xmlwriter::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 2:
      HASH_GUARD(0x1EBCBEA66B6B0982LL, writecdata) {
        return (t_writecdata(a0));
      }
      break;
    case 3:
      HASH_GUARD(0x55E29E84E96ACF83LL, startdtd) {
        if (count <= 1) return (t_startdtd(a0));
        if (count == 2) return (t_startdtd(a0, a1));
        return (t_startdtd(a0, a1, a2));
      }
      break;
    case 8:
      HASH_GUARD(0x7C1FF8A79BAB3608LL, writedtd) {
        if (count <= 1) return (t_writedtd(a0));
        if (count == 2) return (t_writedtd(a0, a1));
        if (count == 3) return (t_writedtd(a0, a1, a2));
        return (t_writedtd(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x119CC9E7C3E7EF0ELL, openmemory) {
        return (t_openmemory());
      }
      break;
    case 16:
      HASH_GUARD(0x0A7FE14C41766610LL, endcdata) {
        return (t_endcdata());
      }
      break;
    case 19:
      HASH_GUARD(0x75F8C992981A4093LL, writecomment) {
        return (t_writecomment(a0));
      }
      HASH_GUARD(0x5A7AE5A26994FF13LL, startattribute) {
        return (t_startattribute(a0));
      }
      break;
    case 28:
      HASH_GUARD(0x337EFFF3783B919CLL, writeelement) {
        if (count <= 1) return (t_writeelement(a0));
        return (t_writeelement(a0, a1));
      }
      break;
    case 29:
      HASH_GUARD(0x4D26D167066BB11DLL, text) {
        return (t_text(a0));
      }
      break;
    case 32:
      HASH_GUARD(0x7DF0368A44C01B20LL, startcdata) {
        return (t_startcdata());
      }
      break;
    case 33:
      HASH_GUARD(0x5D49A460168D5C21LL, endpi) {
        return (t_endpi());
      }
      break;
    case 41:
      HASH_GUARD(0x1D81ABDC9890C5A9LL, writedtdattlist) {
        return (t_writedtdattlist(a0, a1));
      }
      break;
    case 43:
      HASH_GUARD(0x0DEAF425F1AD73ABLL, setindentstring) {
        return (t_setindentstring(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x78A7A5D96EF2F4AFLL, endelement) {
        return (t_endelement());
      }
      break;
    case 53:
      HASH_GUARD(0x75B357D89DDE6EB5LL, endcomment) {
        return (t_endcomment());
      }
      break;
    case 55:
      HASH_GUARD(0x4B0F1F6361A59C37LL, writeraw) {
        return (t_writeraw(a0));
      }
      break;
    case 56:
      HASH_GUARD(0x7FC89CF1AFFB1E38LL, startdocument) {
        if (count <= 0) return (t_startdocument());
        if (count == 1) return (t_startdocument(a0));
        if (count == 2) return (t_startdocument(a0, a1));
        return (t_startdocument(a0, a1, a2));
      }
      break;
    case 58:
      HASH_GUARD(0x0D155D1E5C6641BALL, outputmemory) {
        if (count <= 0) return (t_outputmemory());
        return (t_outputmemory(a0));
      }
      break;
    case 62:
      HASH_GUARD(0x1FBE635680ADB7BELL, openuri) {
        return (t_openuri(a0));
      }
      break;
    case 68:
      HASH_GUARD(0x275896BFD7A77144LL, enddtd) {
        return (t_enddtd());
      }
      break;
    case 70:
      HASH_GUARD(0x5D4650E5C0DAEE46LL, writedtdentity) {
        if (count <= 2) return (t_writedtdentity(a0, a1));
        if (count == 3) return (t_writedtdentity(a0, a1, a2));
        if (count == 4) return (t_writedtdentity(a0, a1, a2, a3));
        if (count == 5) return (t_writedtdentity(a0, a1, a2, a3, a4));
        return (t_writedtdentity(a0, a1, a2, a3, a4, a5));
      }
      break;
    case 71:
      HASH_GUARD(0x017687FD7029CCC7LL, enddtdelement) {
        return (t_enddtdelement());
      }
      break;
    case 75:
      HASH_GUARD(0x3954A2C0306CCCCBLL, writeattribute) {
        return (t_writeattribute(a0, a1));
      }
      HASH_GUARD(0x7C43ECE80FF886CBLL, endattribute) {
        return (t_endattribute());
      }
      break;
    case 77:
      HASH_GUARD(0x4F1D890D5B36D24DLL, startattributens) {
        return (t_startattributens(a0, a1, a2));
      }
      break;
    case 82:
      HASH_GUARD(0x1357CA1E92397452LL, writedtdelement) {
        return (t_writedtdelement(a0, a1));
      }
      break;
    case 84:
      HASH_GUARD(0x154AE94AEFAC7C54LL, writeattributens) {
        return (t_writeattributens(a0, a1, a2, a3));
      }
      break;
    case 86:
      HASH_GUARD(0x4520735705A382D6LL, enddtdentity) {
        return (t_enddtdentity());
      }
      HASH_GUARD(0x37D51A94C6EE8F56LL, writepi) {
        return (t_writepi(a0, a1));
      }
      break;
    case 88:
      HASH_GUARD(0x24A376E9310BF058LL, writeelementns) {
        if (count <= 3) return (t_writeelementns(a0, a1, a2));
        return (t_writeelementns(a0, a1, a2, a3));
      }
      break;
    case 89:
      HASH_GUARD(0x390B6C01AE7C4159LL, startpi) {
        return (t_startpi(a0));
      }
      break;
    case 92:
      HASH_GUARD(0x0DC5AC1E0A2F63DCLL, enddocument) {
        return (t_enddocument());
      }
      break;
    case 93:
      HASH_GUARD(0x75A033B824EA1ADDLL, startdtdentity) {
        return (t_startdtdentity(a0, a1));
      }
      break;
    case 95:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    case 98:
      HASH_GUARD(0x55BFFBE5717EFD62LL, startelement) {
        return (t_startelement(a0));
      }
      break;
    case 101:
      HASH_GUARD(0x349B7446B5EE65E5LL, setindent) {
        return (t_setindent(a0));
      }
      break;
    case 105:
      HASH_GUARD(0x608A2D39DE12E169LL, startelementns) {
        return (t_startelementns(a0, a1, a2));
      }
      break;
    case 112:
      HASH_GUARD(0x0CBB10FA542B7D70LL, startdtdattlist) {
        return (t_startdtdattlist(a0));
      }
      break;
    case 113:
      HASH_GUARD(0x496175DBD52E1771LL, enddtdattlist) {
        return (t_enddtdattlist());
      }
      break;
    case 115:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 120:
      HASH_GUARD(0x451436438EBA2AF8LL, startdtdelement) {
        return (t_startdtdelement(a0));
      }
      break;
    case 122:
      HASH_GUARD(0x6A3D9F8EDB005E7ALL, flush) {
        if (count <= 0) return (t_flush());
        return (t_flush(a0));
      }
      break;
    case 126:
      HASH_GUARD(0x3174CCE00BFB9FFELL, fullendelement) {
        return (t_fullendelement());
      }
      break;
    case 127:
      HASH_GUARD(0x6A8CC27A7F0466FFLL, startcomment) {
        return (t_startcomment());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_xmlwriter::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_xmlwriter::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 2:
      HASH_GUARD(0x1EBCBEA66B6B0982LL, writecdata) {
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
        return (t_writecdata(a0));
      }
      break;
    case 3:
      HASH_GUARD(0x55E29E84E96ACF83LL, startdtd) {
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
        if (count <= 1) return (t_startdtd(a0));
        if (count == 2) return (t_startdtd(a0, a1));
        return (t_startdtd(a0, a1, a2));
      }
      break;
    case 8:
      HASH_GUARD(0x7C1FF8A79BAB3608LL, writedtd) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t_writedtd(a0));
        if (count == 2) return (t_writedtd(a0, a1));
        if (count == 3) return (t_writedtd(a0, a1, a2));
        return (t_writedtd(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x119CC9E7C3E7EF0ELL, openmemory) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_openmemory());
      }
      break;
    case 16:
      HASH_GUARD(0x0A7FE14C41766610LL, endcdata) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_endcdata());
      }
      break;
    case 19:
      HASH_GUARD(0x75F8C992981A4093LL, writecomment) {
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
        return (t_writecomment(a0));
      }
      HASH_GUARD(0x5A7AE5A26994FF13LL, startattribute) {
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
        return (t_startattribute(a0));
      }
      break;
    case 28:
      HASH_GUARD(0x337EFFF3783B919CLL, writeelement) {
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
        if (count <= 1) return (t_writeelement(a0));
        return (t_writeelement(a0, a1));
      }
      break;
    case 29:
      HASH_GUARD(0x4D26D167066BB11DLL, text) {
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
        return (t_text(a0));
      }
      break;
    case 32:
      HASH_GUARD(0x7DF0368A44C01B20LL, startcdata) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_startcdata());
      }
      break;
    case 33:
      HASH_GUARD(0x5D49A460168D5C21LL, endpi) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_endpi());
      }
      break;
    case 41:
      HASH_GUARD(0x1D81ABDC9890C5A9LL, writedtdattlist) {
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
        return (t_writedtdattlist(a0, a1));
      }
      break;
    case 43:
      HASH_GUARD(0x0DEAF425F1AD73ABLL, setindentstring) {
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
        return (t_setindentstring(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x78A7A5D96EF2F4AFLL, endelement) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_endelement());
      }
      break;
    case 53:
      HASH_GUARD(0x75B357D89DDE6EB5LL, endcomment) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_endcomment());
      }
      break;
    case 55:
      HASH_GUARD(0x4B0F1F6361A59C37LL, writeraw) {
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
        return (t_writeraw(a0));
      }
      break;
    case 56:
      HASH_GUARD(0x7FC89CF1AFFB1E38LL, startdocument) {
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
        if (count <= 0) return (t_startdocument());
        if (count == 1) return (t_startdocument(a0));
        if (count == 2) return (t_startdocument(a0, a1));
        return (t_startdocument(a0, a1, a2));
      }
      break;
    case 58:
      HASH_GUARD(0x0D155D1E5C6641BALL, outputmemory) {
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
        if (count <= 0) return (t_outputmemory());
        return (t_outputmemory(a0));
      }
      break;
    case 62:
      HASH_GUARD(0x1FBE635680ADB7BELL, openuri) {
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
        return (t_openuri(a0));
      }
      break;
    case 68:
      HASH_GUARD(0x275896BFD7A77144LL, enddtd) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_enddtd());
      }
      break;
    case 70:
      HASH_GUARD(0x5D4650E5C0DAEE46LL, writedtdentity) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
        Variant a5;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a5 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 2) return (t_writedtdentity(a0, a1));
        if (count == 3) return (t_writedtdentity(a0, a1, a2));
        if (count == 4) return (t_writedtdentity(a0, a1, a2, a3));
        if (count == 5) return (t_writedtdentity(a0, a1, a2, a3, a4));
        return (t_writedtdentity(a0, a1, a2, a3, a4, a5));
      }
      break;
    case 71:
      HASH_GUARD(0x017687FD7029CCC7LL, enddtdelement) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_enddtdelement());
      }
      break;
    case 75:
      HASH_GUARD(0x3954A2C0306CCCCBLL, writeattribute) {
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
        return (t_writeattribute(a0, a1));
      }
      HASH_GUARD(0x7C43ECE80FF886CBLL, endattribute) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_endattribute());
      }
      break;
    case 77:
      HASH_GUARD(0x4F1D890D5B36D24DLL, startattributens) {
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
        return (t_startattributens(a0, a1, a2));
      }
      break;
    case 82:
      HASH_GUARD(0x1357CA1E92397452LL, writedtdelement) {
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
        return (t_writedtdelement(a0, a1));
      }
      break;
    case 84:
      HASH_GUARD(0x154AE94AEFAC7C54LL, writeattributens) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_writeattributens(a0, a1, a2, a3));
      }
      break;
    case 86:
      HASH_GUARD(0x4520735705A382D6LL, enddtdentity) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_enddtdentity());
      }
      HASH_GUARD(0x37D51A94C6EE8F56LL, writepi) {
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
        return (t_writepi(a0, a1));
      }
      break;
    case 88:
      HASH_GUARD(0x24A376E9310BF058LL, writeelementns) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 3) return (t_writeelementns(a0, a1, a2));
        return (t_writeelementns(a0, a1, a2, a3));
      }
      break;
    case 89:
      HASH_GUARD(0x390B6C01AE7C4159LL, startpi) {
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
        return (t_startpi(a0));
      }
      break;
    case 92:
      HASH_GUARD(0x0DC5AC1E0A2F63DCLL, enddocument) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_enddocument());
      }
      break;
    case 93:
      HASH_GUARD(0x75A033B824EA1ADDLL, startdtdentity) {
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
        return (t_startdtdentity(a0, a1));
      }
      break;
    case 95:
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
    case 98:
      HASH_GUARD(0x55BFFBE5717EFD62LL, startelement) {
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
        return (t_startelement(a0));
      }
      break;
    case 101:
      HASH_GUARD(0x349B7446B5EE65E5LL, setindent) {
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
        return (t_setindent(a0));
      }
      break;
    case 105:
      HASH_GUARD(0x608A2D39DE12E169LL, startelementns) {
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
        return (t_startelementns(a0, a1, a2));
      }
      break;
    case 112:
      HASH_GUARD(0x0CBB10FA542B7D70LL, startdtdattlist) {
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
        return (t_startdtdattlist(a0));
      }
      break;
    case 113:
      HASH_GUARD(0x496175DBD52E1771LL, enddtdattlist) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_enddtdattlist());
      }
      break;
    case 115:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 120:
      HASH_GUARD(0x451436438EBA2AF8LL, startdtdelement) {
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
        return (t_startdtdelement(a0));
      }
      break;
    case 122:
      HASH_GUARD(0x6A3D9F8EDB005E7ALL, flush) {
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
        if (count <= 0) return (t_flush());
        return (t_flush(a0));
      }
      break;
    case 126:
      HASH_GUARD(0x3174CCE00BFB9FFELL, fullendelement) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_fullendelement());
      }
      break;
    case 127:
      HASH_GUARD(0x6A8CC27A7F0466FFLL, startcomment) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_startcomment());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_xmlwriter::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_xmlwriter$os_get(const char *s) {
  return c_xmlwriter::os_get(s, -1);
}
Variant &cw_xmlwriter$os_lval(const char *s) {
  return c_xmlwriter::os_lval(s, -1);
}
Variant cw_xmlwriter$os_constant(const char *s) {
  return c_xmlwriter::os_constant(s);
}
Variant cw_xmlwriter$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_xmlwriter::os_invoke(c, s, params, -1, fatal);
}
Object co_domexception(CArrRef params, bool init /* = true */) {
  return Object(p_domexception(NEW(c_domexception)())->dynCreate(params, init));
}
Variant c_domexception::os_get(const char *s, int64 hash) {
  return c_exception::os_get(s, hash);
}
Variant &c_domexception::os_lval(const char *s, int64 hash) {
  return c_exception::os_lval(s, hash);
}
void c_domexception::o_get(ArrayElementVec &props) const {
  c_exception::o_get(props);
}
bool c_domexception::o_exists(CStrRef s, int64 hash) const {
  return c_exception::o_exists(s, hash);
}
Variant c_domexception::o_get(CStrRef s, int64 hash) {
  return c_exception::o_get(s, hash);
}
Variant c_domexception::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_exception::o_set(s, hash, v, forInit);
}
Variant &c_domexception::o_lval(CStrRef s, int64 hash) {
  return c_exception::o_lval(s, hash);
}
Variant c_domexception::os_constant(const char *s) {
  return c_exception::os_constant(s);
}
IMPLEMENT_CLASS(domexception)
ObjectData *c_domexception::create() {
  init();
  t___construct();
  return this;
}
ObjectData *c_domexception::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create());
  } else return this;
}
void c_domexception::dynConstruct(CArrRef params) {
  (t___construct());
}
void c_domexception::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domexception::cloneImpl() {
  c_domexception *obj = NEW(c_domexception)();
  cloneSet(obj);
  return obj;
}
void c_domexception::cloneSet(c_domexception *clone) {
  c_exception::cloneSet(clone);
}
Variant c_domexception::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 3:
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      break;
    case 15:
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    case 18:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 26:
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 30:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
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
  return c_exception::o_invoke(s, params, hash, fatal);
}
Variant c_domexception::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 3:
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      break;
    case 15:
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    case 18:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 26:
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 30:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
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
  return c_exception::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domexception::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_exception::os_invoke(c, s, params, hash, fatal);
}
Variant c_domexception::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 3:
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
      break;
    case 15:
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
    case 18:
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
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 26:
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
    case 30:
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
  return c_exception::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domexception::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_exception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domexception$os_get(const char *s) {
  return c_domexception::os_get(s, -1);
}
Variant &cw_domexception$os_lval(const char *s) {
  return c_domexception::os_lval(s, -1);
}
Variant cw_domexception$os_constant(const char *s) {
  return c_domexception::os_constant(s);
}
Variant cw_domexception$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domexception::os_invoke(c, s, params, -1, fatal);
}
Object co_domxpath(CArrRef params, bool init /* = true */) {
  return Object(p_domxpath(NEW(c_domxpath)())->dynCreate(params, init));
}
Variant c_domxpath::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_domxpath::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_domxpath::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_domxpath::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_domxpath::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_domxpath::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_domxpath::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_domxpath::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(domxpath)
ObjectData *c_domxpath::create(Variant a0) {
  init();
  t___construct(a0);
  return this;
}
ObjectData *c_domxpath::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0)));
  } else return this;
}
void c_domxpath::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0)));
}
void c_domxpath::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domxpath::cloneImpl() {
  c_domxpath *obj = NEW(c_domxpath)();
  cloneSet(obj);
  return obj;
}
void c_domxpath::cloneSet(c_domxpath *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_domxpath::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x144FADF6B374AB82LL, registerphpfunctions) {
        int count = params.size();
        if (count <= 0) return (t_registerphpfunctions());
        return (t_registerphpfunctions(params.rvalAt(0)));
      }
      break;
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 7:
      HASH_GUARD(0x356758D4414DA377LL, query) {
        int count = params.size();
        if (count <= 1) return (t_query(params.rvalAt(0)));
        return (t_query(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 8:
      HASH_GUARD(0x47ACFB6D8681B0E8LL, registernamespace) {
        return (t_registernamespace(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0)), null);
      }
      HASH_GUARD(0x2443593B6F3C912FLL, evaluate) {
        int count = params.size();
        if (count <= 1) return (t_evaluate(params.rvalAt(0)));
        return (t_evaluate(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_domxpath::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x144FADF6B374AB82LL, registerphpfunctions) {
        if (count <= 0) return (t_registerphpfunctions());
        return (t_registerphpfunctions(a0));
      }
      break;
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 7:
      HASH_GUARD(0x356758D4414DA377LL, query) {
        if (count <= 1) return (t_query(a0));
        return (t_query(a0, a1));
      }
      break;
    case 8:
      HASH_GUARD(0x47ACFB6D8681B0E8LL, registernamespace) {
        return (t_registernamespace(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0), null);
      }
      HASH_GUARD(0x2443593B6F3C912FLL, evaluate) {
        if (count <= 1) return (t_evaluate(a0));
        return (t_evaluate(a0, a1));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domxpath::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_domxpath::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 15) {
    case 2:
      HASH_GUARD(0x144FADF6B374AB82LL, registerphpfunctions) {
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
        if (count <= 0) return (t_registerphpfunctions());
        return (t_registerphpfunctions(a0));
      }
      break;
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 7:
      HASH_GUARD(0x356758D4414DA377LL, query) {
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
        if (count <= 1) return (t_query(a0));
        return (t_query(a0, a1));
      }
      break;
    case 8:
      HASH_GUARD(0x47ACFB6D8681B0E8LL, registernamespace) {
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
        return (t_registernamespace(a0, a1));
      }
      break;
    case 15:
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
      HASH_GUARD(0x2443593B6F3C912FLL, evaluate) {
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
        if (count <= 1) return (t_evaluate(a0));
        return (t_evaluate(a0, a1));
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domxpath::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domxpath$os_get(const char *s) {
  return c_domxpath::os_get(s, -1);
}
Variant &cw_domxpath$os_lval(const char *s) {
  return c_domxpath::os_lval(s, -1);
}
Variant cw_domxpath$os_constant(const char *s) {
  return c_domxpath::os_constant(s);
}
Variant cw_domxpath$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domxpath::os_invoke(c, s, params, -1, fatal);
}
Object co_soapserver(CArrRef params, bool init /* = true */) {
  return Object(p_soapserver(NEW(c_soapserver)())->dynCreate(params, init));
}
Variant c_soapserver::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_soapserver::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_soapserver::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_soapserver::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_soapserver::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_soapserver::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_soapserver::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_soapserver::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(soapserver)
ObjectData *c_soapserver::create(Variant a0, Array a1) {
  init();
  t___construct(a0, a1);
  return this;
}
ObjectData *c_soapserver::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    int count = params.size();
    if (count <= 1) return (create(params.rvalAt(0)));
    return (create(params.rvalAt(0), params.rvalAt(1)));
  } else return this;
}
void c_soapserver::dynConstruct(CArrRef params) {
  int count = params.size();
  if (count <= 1) (t___construct(params.rvalAt(0)));
  (t___construct(params.rvalAt(0), params.rvalAt(1)));
}
void c_soapserver::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_soapserver::cloneImpl() {
  c_soapserver *obj = NEW(c_soapserver)();
  cloneSet(obj);
  return obj;
}
void c_soapserver::cloneSet(c_soapserver *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_soapserver::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 3:
      HASH_GUARD(0x758A4E04590FE203LL, fault) {
        int count = params.size();
        if (count <= 2) return (t_fault(params.rvalAt(0), params.rvalAt(1)), null);
        if (count == 3) return (t_fault(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)), null);
        if (count == 4) return (t_fault(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)), null);
        return (t_fault(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)), null);
      }
      break;
    case 8:
      HASH_GUARD(0x5E77DD94E0A69328LL, setpersistence) {
        return (t_setpersistence(params.rvalAt(0)), null);
      }
      break;
    case 15:
      HASH_GUARD(0x652BDFA6E22F17AFLL, getfunctions) {
        return (t_getfunctions());
      }
      break;
    case 18:
      HASH_GUARD(0x5C4CA333F4541532LL, handle) {
        int count = params.size();
        if (count <= 0) return (t_handle(), null);
        return (t_handle(params.rvalAt(0)), null);
      }
      break;
    case 19:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 20:
      HASH_GUARD(0x597B151CC4F70834LL, setclass) {
        int count = params.size();
        if (count <= 1) return (t_setclass(count, params.rvalAt(0)), null);
        return (t_setclass(count,params.rvalAt(0), params.slice(1, count - 1, false)), null);
      }
      break;
    case 25:
      HASH_GUARD(0x559622F84FE626B9LL, setobject) {
        return (t_setobject(params.rvalAt(0)), null);
      }
      break;
    case 30:
      HASH_GUARD(0x278B0E6CCA74963ELL, addsoapheader) {
        return (t_addsoapheader(params.rvalAt(0)), null);
      }
      break;
    case 31:
      HASH_GUARD(0x48B5852A397D2D9FLL, addfunction) {
        return (t_addfunction(params.rvalAt(0)), null);
      }
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 1) return (t___construct(params.rvalAt(0)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_soapserver::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 3:
      HASH_GUARD(0x758A4E04590FE203LL, fault) {
        if (count <= 2) return (t_fault(a0, a1), null);
        if (count == 3) return (t_fault(a0, a1, a2), null);
        if (count == 4) return (t_fault(a0, a1, a2, a3), null);
        return (t_fault(a0, a1, a2, a3, a4), null);
      }
      break;
    case 8:
      HASH_GUARD(0x5E77DD94E0A69328LL, setpersistence) {
        return (t_setpersistence(a0), null);
      }
      break;
    case 15:
      HASH_GUARD(0x652BDFA6E22F17AFLL, getfunctions) {
        return (t_getfunctions());
      }
      break;
    case 18:
      HASH_GUARD(0x5C4CA333F4541532LL, handle) {
        if (count <= 0) return (t_handle(), null);
        return (t_handle(a0), null);
      }
      break;
    case 19:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 20:
      HASH_GUARD(0x597B151CC4F70834LL, setclass) {
        if (count <= 1) return (t_setclass(count, a0), null);
        Array params;
        if (count >= 2) params.append(a1);
        if (count >= 3) params.append(a2);
        if (count >= 4) params.append(a3);
        if (count >= 5) params.append(a4);
        if (count >= 6) params.append(a5);
        return (t_setclass(count,a0, params), null);
      }
      break;
    case 25:
      HASH_GUARD(0x559622F84FE626B9LL, setobject) {
        return (t_setobject(a0), null);
      }
      break;
    case 30:
      HASH_GUARD(0x278B0E6CCA74963ELL, addsoapheader) {
        return (t_addsoapheader(a0), null);
      }
      break;
    case 31:
      HASH_GUARD(0x48B5852A397D2D9FLL, addfunction) {
        return (t_addfunction(a0), null);
      }
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 1) return (t___construct(a0), null);
        return (t___construct(a0, a1), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_soapserver::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_soapserver::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 3:
      HASH_GUARD(0x758A4E04590FE203LL, fault) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 2) return (t_fault(a0, a1), null);
        if (count == 3) return (t_fault(a0, a1, a2), null);
        if (count == 4) return (t_fault(a0, a1, a2, a3), null);
        return (t_fault(a0, a1, a2, a3, a4), null);
      }
      break;
    case 8:
      HASH_GUARD(0x5E77DD94E0A69328LL, setpersistence) {
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
        return (t_setpersistence(a0), null);
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
    case 18:
      HASH_GUARD(0x5C4CA333F4541532LL, handle) {
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
        if (count <= 0) return (t_handle(), null);
        return (t_handle(a0), null);
      }
      break;
    case 19:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 20:
      HASH_GUARD(0x597B151CC4F70834LL, setclass) {
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
        if (count <= 1) return (t_setclass(count, a0), null);
        return (t_setclass(count, a0,vargs), null);
      }
      break;
    case 25:
      HASH_GUARD(0x559622F84FE626B9LL, setobject) {
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
        return (t_setobject(a0), null);
      }
      break;
    case 30:
      HASH_GUARD(0x278B0E6CCA74963ELL, addsoapheader) {
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
        return (t_addsoapheader(a0), null);
      }
      break;
    case 31:
      HASH_GUARD(0x48B5852A397D2D9FLL, addfunction) {
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
        return (t_addfunction(a0), null);
      }
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
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_soapserver::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_soapserver$os_get(const char *s) {
  return c_soapserver::os_get(s, -1);
}
Variant &cw_soapserver$os_lval(const char *s) {
  return c_soapserver::os_lval(s, -1);
}
Variant cw_soapserver$os_constant(const char *s) {
  return c_soapserver::os_constant(s);
}
Variant cw_soapserver$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_soapserver::os_invoke(c, s, params, -1, fatal);
}
Object co_domnode(CArrRef params, bool init /* = true */) {
  return Object(p_domnode(NEW(c_domnode)())->dynCreate(params, init));
}
Variant c_domnode::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_domnode::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_domnode::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_domnode::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_domnode::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_domnode::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_domnode::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_domnode::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(domnode)
ObjectData *c_domnode::create() {
  init();
  t___construct();
  return this;
}
ObjectData *c_domnode::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create());
  } else return this;
}
void c_domnode::dynConstruct(CArrRef params) {
  (t___construct());
}
void c_domnode::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domnode::cloneImpl() {
  c_domnode *obj = NEW(c_domnode)();
  cloneSet(obj);
  return obj;
}
void c_domnode::cloneSet(c_domnode *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_domnode::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(params.rvalAt(0)));
        if (count == 2) return (t_c14n(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(params.rvalAt(0)));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        int count = params.size();
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(params.rvalAt(0)));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(params.rvalAt(0)));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(params.rvalAt(0)));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(params.rvalAt(0)));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(params.rvalAt(0)));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(params.rvalAt(0)));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        int count = params.size();
        if (count <= 1) return (t_insertbefore(params.rvalAt(0)));
        return (t_insertbefore(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        int count = params.size();
        if (count <= 1) return (t_c14nfile(params.rvalAt(0)));
        if (count == 2) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_domnode::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(a0));
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domnode::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_domnode::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 63) {
    case 3:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnodepath());
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
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
        return (t_replacechild(a0, a1));
      }
      break;
    case 15:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
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
        return (t_appendchild(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
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
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlineno());
      }
      break;
    case 26:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_normalize(), null);
      }
      break;
    case 29:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
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
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
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
        return (t_removechild(a0));
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
    case 33:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
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
        return (t_lookupprefix(a0));
      }
      break;
    case 36:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
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
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
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
        return (t_issamenode(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
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
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 53:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 58:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
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
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_haschildnodes());
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domnode::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domnode$os_get(const char *s) {
  return c_domnode::os_get(s, -1);
}
Variant &cw_domnode$os_lval(const char *s) {
  return c_domnode::os_lval(s, -1);
}
Variant cw_domnode$os_constant(const char *s) {
  return c_domnode::os_constant(s);
}
Variant cw_domnode$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domnode::os_invoke(c, s, params, -1, fatal);
}
Object co_domdocument(CArrRef params, bool init /* = true */) {
  return Object(p_domdocument(NEW(c_domdocument)())->dynCreate(params, init));
}
Variant c_domdocument::os_get(const char *s, int64 hash) {
  return c_domnode::os_get(s, hash);
}
Variant &c_domdocument::os_lval(const char *s, int64 hash) {
  return c_domnode::os_lval(s, hash);
}
void c_domdocument::o_get(ArrayElementVec &props) const {
  c_domnode::o_get(props);
}
bool c_domdocument::o_exists(CStrRef s, int64 hash) const {
  return c_domnode::o_exists(s, hash);
}
Variant c_domdocument::o_get(CStrRef s, int64 hash) {
  return c_domnode::o_get(s, hash);
}
Variant c_domdocument::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_domnode::o_set(s, hash, v, forInit);
}
Variant &c_domdocument::o_lval(CStrRef s, int64 hash) {
  return c_domnode::o_lval(s, hash);
}
Variant c_domdocument::os_constant(const char *s) {
  return c_domnode::os_constant(s);
}
IMPLEMENT_CLASS(domdocument)
ObjectData *c_domdocument::create(String a0, String a1) {
  init();
  t___construct(a0, a1);
  return this;
}
ObjectData *c_domdocument::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0), params.rvalAt(1)));
  } else return this;
}
void c_domdocument::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0), params.rvalAt(1)));
}
void c_domdocument::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domdocument::cloneImpl() {
  c_domdocument *obj = NEW(c_domdocument)();
  cloneSet(obj);
  return obj;
}
void c_domdocument::cloneSet(c_domdocument *clone) {
  c_domnode::cloneSet(clone);
}
Variant c_domdocument::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 5:
      HASH_GUARD(0x515FE7746601E385LL, getelementbyid) {
        return (t_getelementbyid(params.rvalAt(0)));
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(params.rvalAt(0)));
        if (count == 2) return (t_c14n(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      HASH_GUARD(0x142A28F8CE4A4E8ALL, xinclude) {
        int count = params.size();
        if (count <= 0) return (t_xinclude());
        return (t_xinclude(params.rvalAt(0)));
      }
      break;
    case 13:
      HASH_GUARD(0x0D128C363EBF7F0DLL, createcdatasection) {
        return (t_createcdatasection(params.rvalAt(0)));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 17:
      HASH_GUARD(0x69C6F7D584EC5011LL, schemavalidatesource) {
        return (t_schemavalidatesource(params.rvalAt(0)));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        int count = params.size();
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(params.rvalAt(0)));
      }
      break;
    case 20:
      HASH_GUARD(0x04F8F7CEA46D0514LL, save) {
        int count = params.size();
        if (count <= 1) return (t_save(params.rvalAt(0)));
        return (t_save(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 21:
      HASH_GUARD(0x2FC58024A75AEB15LL, createattributens) {
        return (t_createattributens(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 28:
      HASH_GUARD(0x1A78385D7CD10A1CLL, relaxngvalidate) {
        return (t_relaxngvalidate(params.rvalAt(0)));
      }
      break;
    case 34:
      HASH_GUARD(0x1B0F4D437C8404A2LL, loadhtml) {
        return (t_loadhtml(params.rvalAt(0)));
      }
      HASH_GUARD(0x4C40B0F935B39FA2LL, createelement) {
        int count = params.size();
        if (count <= 1) return (t_createelement(params.rvalAt(0)));
        return (t_createelement(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 44:
      HASH_GUARD(0x1304C35F6E006FACLL, relaxngvalidatesource) {
        return (t_relaxngvalidatesource(params.rvalAt(0)));
      }
      HASH_GUARD(0x2BFDE9CF0FE9A82CLL, createattribute) {
        return (t_createattribute(params.rvalAt(0)));
      }
      break;
    case 47:
      HASH_GUARD(0x4A6C6D9AB88CD42FLL, importnode) {
        int count = params.size();
        if (count <= 1) return (t_importnode(params.rvalAt(0)));
        return (t_importnode(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x5FE94CA513F14AAFLL, loadhtmlfile) {
        return (t_loadhtmlfile(params.rvalAt(0)));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      HASH_GUARD(0x20E2B2FD2B7AE431LL, loadxml) {
        int count = params.size();
        if (count <= 1) return (t_loadxml(params.rvalAt(0)));
        return (t_loadxml(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 50:
      HASH_GUARD(0x7A9C4709CAD09832LL, createentityreference) {
        return (t_createentityreference(params.rvalAt(0)));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        int count = params.size();
        if (count <= 1) return (t_insertbefore(params.rvalAt(0)));
        return (t_insertbefore(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 55:
      HASH_GUARD(0x1CA408E02262F737LL, validate) {
        return (t_validate());
      }
      break;
    case 67:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 71:
      HASH_GUARD(0x29C79E90C6FCC0C7LL, createcomment) {
        return (t_createcomment(params.rvalAt(0)));
      }
      break;
    case 72:
      HASH_GUARD(0x7EB8C68BABDC5648LL, schemavalidate) {
        return (t_schemavalidate(params.rvalAt(0)));
      }
      break;
    case 74:
      HASH_GUARD(0x3FFA3F55ECAB93CALL, getelementsbytagname) {
        return (t_getelementsbytagname(params.rvalAt(0)));
      }
      HASH_GUARD(0x1A9CC561CA4817CALL, getelementsbytagnamens) {
        return (t_getelementsbytagnamens(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 77:
      HASH_GUARD(0x35C0FF513ED3054DLL, savehtmlfile) {
        return (t_savehtmlfile(params.rvalAt(0)));
      }
      break;
    case 79:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(params.rvalAt(0)));
      }
      break;
    case 83:
      HASH_GUARD(0x21F3F1C9058310D3LL, savehtml) {
        return (t_savehtml());
      }
      break;
    case 90:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 93:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(params.rvalAt(0)));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(params.rvalAt(0)));
      }
      break;
    case 95:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
      }
      HASH_GUARD(0x79B7A5774A0943DFLL, load) {
        int count = params.size();
        if (count <= 1) return (t_load(params.rvalAt(0)));
        return (t_load(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 97:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(params.rvalAt(0)));
      }
      break;
    case 100:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(params.rvalAt(0)));
      }
      break;
    case 101:
      HASH_GUARD(0x26D66F56DDDC32E5LL, savexml) {
        int count = params.size();
        if (count <= 0) return (t_savexml());
        if (count == 1) return (t_savexml(params.rvalAt(0)));
        return (t_savexml(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 106:
      HASH_GUARD(0x0AC931EAB2FE3D6ALL, normalizedocument) {
        return (t_normalizedocument(), null);
      }
      break;
    case 107:
      HASH_GUARD(0x05EA36F416B7EBEBLL, createelementns) {
        int count = params.size();
        if (count <= 2) return (t_createelementns(params.rvalAt(0), params.rvalAt(1)));
        return (t_createelementns(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 109:
      HASH_GUARD(0x2A99431FC6E7BA6DLL, createprocessinginstruction) {
        int count = params.size();
        if (count <= 1) return (t_createprocessinginstruction(params.rvalAt(0)));
        return (t_createprocessinginstruction(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 111:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(params.rvalAt(0)));
      }
      break;
    case 115:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 117:
      HASH_GUARD(0x615771958E1EB375LL, registernodeclass) {
        return (t_registernodeclass(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        int count = params.size();
        if (count <= 1) return (t_c14nfile(params.rvalAt(0)));
        if (count == 2) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
      }
      break;
    case 120:
      HASH_GUARD(0x44FD8C24F4EA46F8LL, createdocumentfragment) {
        return (t_createdocumentfragment());
      }
      break;
    case 122:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    case 123:
      HASH_GUARD(0x6056A5BB9855D7FBLL, createtextnode) {
        return (t_createtextnode(params.rvalAt(0)));
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke(s, params, hash, fatal);
}
Variant c_domdocument::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 5:
      HASH_GUARD(0x515FE7746601E385LL, getelementbyid) {
        return (t_getelementbyid(a0));
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      HASH_GUARD(0x142A28F8CE4A4E8ALL, xinclude) {
        if (count <= 0) return (t_xinclude());
        return (t_xinclude(a0));
      }
      break;
    case 13:
      HASH_GUARD(0x0D128C363EBF7F0DLL, createcdatasection) {
        return (t_createcdatasection(a0));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(a0, a1));
      }
      break;
    case 17:
      HASH_GUARD(0x69C6F7D584EC5011LL, schemavalidatesource) {
        return (t_schemavalidatesource(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 20:
      HASH_GUARD(0x04F8F7CEA46D0514LL, save) {
        if (count <= 1) return (t_save(a0));
        return (t_save(a0, a1));
      }
      break;
    case 21:
      HASH_GUARD(0x2FC58024A75AEB15LL, createattributens) {
        return (t_createattributens(a0, a1));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      break;
    case 28:
      HASH_GUARD(0x1A78385D7CD10A1CLL, relaxngvalidate) {
        return (t_relaxngvalidate(a0));
      }
      break;
    case 34:
      HASH_GUARD(0x1B0F4D437C8404A2LL, loadhtml) {
        return (t_loadhtml(a0));
      }
      HASH_GUARD(0x4C40B0F935B39FA2LL, createelement) {
        if (count <= 1) return (t_createelement(a0));
        return (t_createelement(a0, a1));
      }
      break;
    case 44:
      HASH_GUARD(0x1304C35F6E006FACLL, relaxngvalidatesource) {
        return (t_relaxngvalidatesource(a0));
      }
      HASH_GUARD(0x2BFDE9CF0FE9A82CLL, createattribute) {
        return (t_createattribute(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x4A6C6D9AB88CD42FLL, importnode) {
        if (count <= 1) return (t_importnode(a0));
        return (t_importnode(a0, a1));
      }
      HASH_GUARD(0x5FE94CA513F14AAFLL, loadhtmlfile) {
        return (t_loadhtmlfile(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      HASH_GUARD(0x20E2B2FD2B7AE431LL, loadxml) {
        if (count <= 1) return (t_loadxml(a0));
        return (t_loadxml(a0, a1));
      }
      break;
    case 50:
      HASH_GUARD(0x7A9C4709CAD09832LL, createentityreference) {
        return (t_createentityreference(a0));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      break;
    case 55:
      HASH_GUARD(0x1CA408E02262F737LL, validate) {
        return (t_validate());
      }
      break;
    case 67:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 71:
      HASH_GUARD(0x29C79E90C6FCC0C7LL, createcomment) {
        return (t_createcomment(a0));
      }
      break;
    case 72:
      HASH_GUARD(0x7EB8C68BABDC5648LL, schemavalidate) {
        return (t_schemavalidate(a0));
      }
      break;
    case 74:
      HASH_GUARD(0x3FFA3F55ECAB93CALL, getelementsbytagname) {
        return (t_getelementsbytagname(a0));
      }
      HASH_GUARD(0x1A9CC561CA4817CALL, getelementsbytagnamens) {
        return (t_getelementsbytagnamens(a0, a1));
      }
      break;
    case 77:
      HASH_GUARD(0x35C0FF513ED3054DLL, savehtmlfile) {
        return (t_savehtmlfile(a0));
      }
      break;
    case 79:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(a0));
      }
      break;
    case 83:
      HASH_GUARD(0x21F3F1C9058310D3LL, savehtml) {
        return (t_savehtml());
      }
      break;
    case 90:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 93:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(a0));
      }
      break;
    case 95:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x79B7A5774A0943DFLL, load) {
        if (count <= 1) return (t_load(a0));
        return (t_load(a0, a1));
      }
      break;
    case 97:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(a0));
      }
      break;
    case 100:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 101:
      HASH_GUARD(0x26D66F56DDDC32E5LL, savexml) {
        if (count <= 0) return (t_savexml());
        if (count == 1) return (t_savexml(a0));
        return (t_savexml(a0, a1));
      }
      break;
    case 106:
      HASH_GUARD(0x0AC931EAB2FE3D6ALL, normalizedocument) {
        return (t_normalizedocument(), null);
      }
      break;
    case 107:
      HASH_GUARD(0x05EA36F416B7EBEBLL, createelementns) {
        if (count <= 2) return (t_createelementns(a0, a1));
        return (t_createelementns(a0, a1, a2));
      }
      break;
    case 109:
      HASH_GUARD(0x2A99431FC6E7BA6DLL, createprocessinginstruction) {
        if (count <= 1) return (t_createprocessinginstruction(a0));
        return (t_createprocessinginstruction(a0, a1));
      }
      break;
    case 111:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(a0));
      }
      break;
    case 115:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 117:
      HASH_GUARD(0x615771958E1EB375LL, registernodeclass) {
        return (t_registernodeclass(a0, a1));
      }
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 120:
      HASH_GUARD(0x44FD8C24F4EA46F8LL, createdocumentfragment) {
        return (t_createdocumentfragment());
      }
      break;
    case 122:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    case 123:
      HASH_GUARD(0x6056A5BB9855D7FBLL, createtextnode) {
        return (t_createtextnode(a0));
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domdocument::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_domnode::os_invoke(c, s, params, hash, fatal);
}
Variant c_domdocument::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 5:
      HASH_GUARD(0x515FE7746601E385LL, getelementbyid) {
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
        return (t_getelementbyid(a0));
      }
      break;
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      HASH_GUARD(0x142A28F8CE4A4E8ALL, xinclude) {
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
        if (count <= 0) return (t_xinclude());
        return (t_xinclude(a0));
      }
      break;
    case 13:
      HASH_GUARD(0x0D128C363EBF7F0DLL, createcdatasection) {
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
        return (t_createcdatasection(a0));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
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
        return (t_replacechild(a0, a1));
      }
      break;
    case 17:
      HASH_GUARD(0x69C6F7D584EC5011LL, schemavalidatesource) {
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
        return (t_schemavalidatesource(a0));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
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
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 20:
      HASH_GUARD(0x04F8F7CEA46D0514LL, save) {
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
        if (count <= 1) return (t_save(a0));
        return (t_save(a0, a1));
      }
      break;
    case 21:
      HASH_GUARD(0x2FC58024A75AEB15LL, createattributens) {
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
        return (t_createattributens(a0, a1));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlineno());
      }
      break;
    case 28:
      HASH_GUARD(0x1A78385D7CD10A1CLL, relaxngvalidate) {
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
        return (t_relaxngvalidate(a0));
      }
      break;
    case 34:
      HASH_GUARD(0x1B0F4D437C8404A2LL, loadhtml) {
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
        return (t_loadhtml(a0));
      }
      HASH_GUARD(0x4C40B0F935B39FA2LL, createelement) {
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
        if (count <= 1) return (t_createelement(a0));
        return (t_createelement(a0, a1));
      }
      break;
    case 44:
      HASH_GUARD(0x1304C35F6E006FACLL, relaxngvalidatesource) {
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
        return (t_relaxngvalidatesource(a0));
      }
      HASH_GUARD(0x2BFDE9CF0FE9A82CLL, createattribute) {
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
        return (t_createattribute(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x4A6C6D9AB88CD42FLL, importnode) {
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
        if (count <= 1) return (t_importnode(a0));
        return (t_importnode(a0, a1));
      }
      HASH_GUARD(0x5FE94CA513F14AAFLL, loadhtmlfile) {
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
        return (t_loadhtmlfile(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_hasattributes());
      }
      HASH_GUARD(0x20E2B2FD2B7AE431LL, loadxml) {
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
        if (count <= 1) return (t_loadxml(a0));
        return (t_loadxml(a0, a1));
      }
      break;
    case 50:
      HASH_GUARD(0x7A9C4709CAD09832LL, createentityreference) {
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
        return (t_createentityreference(a0));
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
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
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      break;
    case 55:
      HASH_GUARD(0x1CA408E02262F737LL, validate) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_validate());
      }
      break;
    case 67:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnodepath());
      }
      break;
    case 71:
      HASH_GUARD(0x29C79E90C6FCC0C7LL, createcomment) {
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
        return (t_createcomment(a0));
      }
      break;
    case 72:
      HASH_GUARD(0x7EB8C68BABDC5648LL, schemavalidate) {
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
        return (t_schemavalidate(a0));
      }
      break;
    case 74:
      HASH_GUARD(0x3FFA3F55ECAB93CALL, getelementsbytagname) {
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
        return (t_getelementsbytagname(a0));
      }
      HASH_GUARD(0x1A9CC561CA4817CALL, getelementsbytagnamens) {
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
        return (t_getelementsbytagnamens(a0, a1));
      }
      break;
    case 77:
      HASH_GUARD(0x35C0FF513ED3054DLL, savehtmlfile) {
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
        return (t_savehtmlfile(a0));
      }
      break;
    case 79:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
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
        return (t_appendchild(a0));
      }
      break;
    case 83:
      HASH_GUARD(0x21F3F1C9058310D3LL, savehtml) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_savehtml());
      }
      break;
    case 90:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_normalize(), null);
      }
      break;
    case 93:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
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
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
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
        return (t_removechild(a0));
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
        return (t___construct(a0, a1), null);
      }
      HASH_GUARD(0x79B7A5774A0943DFLL, load) {
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
        if (count <= 1) return (t_load(a0));
        return (t_load(a0, a1));
      }
      break;
    case 97:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
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
        return (t_lookupprefix(a0));
      }
      break;
    case 100:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
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
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 101:
      HASH_GUARD(0x26D66F56DDDC32E5LL, savexml) {
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
        if (count <= 0) return (t_savexml());
        if (count == 1) return (t_savexml(a0));
        return (t_savexml(a0, a1));
      }
      break;
    case 106:
      HASH_GUARD(0x0AC931EAB2FE3D6ALL, normalizedocument) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_normalizedocument(), null);
      }
      break;
    case 107:
      HASH_GUARD(0x05EA36F416B7EBEBLL, createelementns) {
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
        if (count <= 2) return (t_createelementns(a0, a1));
        return (t_createelementns(a0, a1, a2));
      }
      break;
    case 109:
      HASH_GUARD(0x2A99431FC6E7BA6DLL, createprocessinginstruction) {
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
        if (count <= 1) return (t_createprocessinginstruction(a0));
        return (t_createprocessinginstruction(a0, a1));
      }
      break;
    case 111:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
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
        return (t_issamenode(a0));
      }
      break;
    case 115:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 117:
      HASH_GUARD(0x615771958E1EB375LL, registernodeclass) {
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
        return (t_registernodeclass(a0, a1));
      }
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 120:
      HASH_GUARD(0x44FD8C24F4EA46F8LL, createdocumentfragment) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_createdocumentfragment());
      }
      break;
    case 122:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
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
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_haschildnodes());
      }
      break;
    case 123:
      HASH_GUARD(0x6056A5BB9855D7FBLL, createtextnode) {
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
        return (t_createtextnode(a0));
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domdocument::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_domnode::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domdocument$os_get(const char *s) {
  return c_domdocument::os_get(s, -1);
}
Variant &cw_domdocument$os_lval(const char *s) {
  return c_domdocument::os_lval(s, -1);
}
Variant cw_domdocument$os_constant(const char *s) {
  return c_domdocument::os_constant(s);
}
Variant cw_domdocument$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domdocument::os_invoke(c, s, params, -1, fatal);
}
Object co_libxmlerror(CArrRef params, bool init /* = true */) {
  return Object(p_libxmlerror(NEW(c_libxmlerror)())->dynCreate(params, init));
}
Variant c_libxmlerror::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_libxmlerror::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_libxmlerror::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_libxmlerror::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_libxmlerror::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_libxmlerror::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_libxmlerror::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_libxmlerror::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(libxmlerror)
ObjectData *c_libxmlerror::create() {
  init();
  t___construct();
  return this;
}
ObjectData *c_libxmlerror::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create());
  } else return this;
}
void c_libxmlerror::dynConstruct(CArrRef params) {
  (t___construct());
}
void c_libxmlerror::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_libxmlerror::cloneImpl() {
  c_libxmlerror *obj = NEW(c_libxmlerror)();
  cloneSet(obj);
  return obj;
}
void c_libxmlerror::cloneSet(c_libxmlerror *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_libxmlerror::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 3) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_libxmlerror::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 3) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(), null);
      }
      break;
    default:
      break;
  }
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_libxmlerror::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_libxmlerror::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 3) {
    case 3:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
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
Variant c_libxmlerror::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_libxmlerror$os_get(const char *s) {
  return c_libxmlerror::os_get(s, -1);
}
Variant &cw_libxmlerror$os_lval(const char *s) {
  return c_libxmlerror::os_lval(s, -1);
}
Variant cw_libxmlerror$os_constant(const char *s) {
  return c_libxmlerror::os_constant(s);
}
Variant cw_libxmlerror$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_libxmlerror::os_invoke(c, s, params, -1, fatal);
}
Object co_soapfault(CArrRef params, bool init /* = true */) {
  return Object(p_soapfault(NEW(c_soapfault)())->dynCreate(params, init));
}
Variant c_soapfault::os_get(const char *s, int64 hash) {
  return c_exception::os_get(s, hash);
}
Variant &c_soapfault::os_lval(const char *s, int64 hash) {
  return c_exception::os_lval(s, hash);
}
void c_soapfault::o_get(ArrayElementVec &props) const {
  c_exception::o_get(props);
}
bool c_soapfault::o_exists(CStrRef s, int64 hash) const {
  return c_exception::o_exists(s, hash);
}
Variant c_soapfault::o_get(CStrRef s, int64 hash) {
  return c_exception::o_get(s, hash);
}
Variant c_soapfault::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_exception::o_set(s, hash, v, forInit);
}
Variant &c_soapfault::o_lval(CStrRef s, int64 hash) {
  return c_exception::o_lval(s, hash);
}
Variant c_soapfault::os_constant(const char *s) {
  return c_exception::os_constant(s);
}
IMPLEMENT_CLASS(soapfault)
ObjectData *c_soapfault::create(Variant a0, String a1, String a2, Variant a3, String a4, Variant a5) {
  init();
  t___construct(a0, a1, a2, a3, a4, a5);
  return this;
}
ObjectData *c_soapfault::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    int count = params.size();
    if (count <= 2) return (create(params.rvalAt(0), params.rvalAt(1)));
    if (count == 3) return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
    if (count == 4) return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
    if (count == 5) return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
    return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4), params.rvalAt(5)));
  } else return this;
}
void c_soapfault::dynConstruct(CArrRef params) {
  int count = params.size();
  if (count <= 2) (t___construct(params.rvalAt(0), params.rvalAt(1)));
  if (count == 3) (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
  if (count == 4) (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
  if (count == 5) (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
  (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4), params.rvalAt(5)));
}
void c_soapfault::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_soapfault::cloneImpl() {
  c_soapfault *obj = NEW(c_soapfault)();
  cloneSet(obj);
  return obj;
}
void c_soapfault::cloneSet(c_soapfault *clone) {
  c_exception::cloneSet(clone);
}
Variant c_soapfault::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 3:
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      break;
    case 15:
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    case 18:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 26:
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 30:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        int count = params.size();
        if (count <= 2) return (t___construct(params.rvalAt(0), params.rvalAt(1)), null);
        if (count == 3) return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)), null);
        if (count == 4) return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)), null);
        if (count == 5) return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)), null);
        return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4), params.rvalAt(5)), null);
      }
      break;
    default:
      break;
  }
  return c_exception::o_invoke(s, params, hash, fatal);
}
Variant c_soapfault::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 3:
      HASH_GUARD(0x31D981FD9D2728E3LL, getline) {
        return (t_getline());
      }
      break;
    case 10:
      HASH_GUARD(0x03CA4360169ECC8ALL, gettraceasstring) {
        return (t_gettraceasstring());
      }
      break;
    case 15:
      HASH_GUARD(0x5C108B351DC3D04FLL, getcode) {
        return (t_getcode());
      }
      break;
    case 18:
      HASH_GUARD(0x71859D7313E682D2LL, getmessage) {
        return (t_getmessage());
      }
      break;
    case 19:
      HASH_GUARD(0x642C2D2994B34A13LL, __tostring) {
        return (t___tostring());
      }
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 26:
      HASH_GUARD(0x6800B2B4C4EC4CBALL, gettrace) {
        return (t_gettrace());
      }
      break;
    case 30:
      HASH_GUARD(0x3CE90CB8F0C9579ELL, getfile) {
        return (t_getfile());
      }
      break;
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        if (count <= 2) return (t___construct(a0, a1), null);
        if (count == 3) return (t___construct(a0, a1, a2), null);
        if (count == 4) return (t___construct(a0, a1, a2, a3), null);
        if (count == 5) return (t___construct(a0, a1, a2, a3, a4), null);
        return (t___construct(a0, a1, a2, a3, a4, a5), null);
      }
      break;
    default:
      break;
  }
  return c_exception::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_soapfault::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_exception::os_invoke(c, s, params, hash, fatal);
}
Variant c_soapfault::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 31) {
    case 3:
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
      break;
    case 15:
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
    case 18:
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
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 26:
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
    case 30:
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
    case 31:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
        Variant a5;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a5 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 2) return (t___construct(a0, a1), null);
        if (count == 3) return (t___construct(a0, a1, a2), null);
        if (count == 4) return (t___construct(a0, a1, a2, a3), null);
        if (count == 5) return (t___construct(a0, a1, a2, a3, a4), null);
        return (t___construct(a0, a1, a2, a3, a4, a5), null);
      }
      break;
    default:
      break;
  }
  return c_exception::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_soapfault::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_exception::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_soapfault$os_get(const char *s) {
  return c_soapfault::os_get(s, -1);
}
Variant &cw_soapfault$os_lval(const char *s) {
  return c_soapfault::os_lval(s, -1);
}
Variant cw_soapfault$os_constant(const char *s) {
  return c_soapfault::os_constant(s);
}
Variant cw_soapfault$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_soapfault::os_invoke(c, s, params, -1, fatal);
}
Object co_domelement(CArrRef params, bool init /* = true */) {
  return Object(p_domelement(NEW(c_domelement)())->dynCreate(params, init));
}
Variant c_domelement::os_get(const char *s, int64 hash) {
  return c_domnode::os_get(s, hash);
}
Variant &c_domelement::os_lval(const char *s, int64 hash) {
  return c_domnode::os_lval(s, hash);
}
void c_domelement::o_get(ArrayElementVec &props) const {
  c_domnode::o_get(props);
}
bool c_domelement::o_exists(CStrRef s, int64 hash) const {
  return c_domnode::o_exists(s, hash);
}
Variant c_domelement::o_get(CStrRef s, int64 hash) {
  return c_domnode::o_get(s, hash);
}
Variant c_domelement::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_domnode::o_set(s, hash, v, forInit);
}
Variant &c_domelement::o_lval(CStrRef s, int64 hash) {
  return c_domnode::o_lval(s, hash);
}
Variant c_domelement::os_constant(const char *s) {
  return c_domnode::os_constant(s);
}
IMPLEMENT_CLASS(domelement)
ObjectData *c_domelement::create(Variant a0, Variant a1, Variant a2) {
  init();
  t___construct(a0, a1, a2);
  return this;
}
ObjectData *c_domelement::dynCreate(CArrRef params, bool init /* = true */) {
  if (init) {
    return (create(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
  } else return this;
}
void c_domelement::dynConstruct(CArrRef params) {
  (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
}
void c_domelement::destruct() {
  if (!inCtorDtor()) {
    incRefCount();
    try {
      t___destruct();
    } catch (...) { handle_destructor_exception();}
  }
}
ObjectData *c_domelement::cloneImpl() {
  c_domelement *obj = NEW(c_domelement)();
  cloneSet(obj);
  return obj;
}
void c_domelement::cloneSet(c_domelement *clone) {
  c_domnode::cloneSet(clone);
}
Variant c_domelement::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(params.rvalAt(0)));
        if (count == 2) return (t_c14n(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        return (t_c14n(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
      }
      HASH_GUARD(0x1D5B8B8144F4AB8ALL, setattribute) {
        return (t_setattribute(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 16:
      HASH_GUARD(0x710986B01E1A0890LL, removeattributens) {
        return (t_removeattributens(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 17:
      HASH_GUARD(0x31426AC6A851EE11LL, setidattributens) {
        return (t_setidattributens(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        int count = params.size();
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(params.rvalAt(0)));
      }
      break;
    case 22:
      HASH_GUARD(0x6F83EC29E110D616LL, setidattribute) {
        return (t_setidattribute(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      HASH_GUARD(0x640FA60DDC80EC18LL, getattributenodens) {
        return (t_getattributenodens(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 25:
      HASH_GUARD(0x34E103E06D3F0899LL, getattributens) {
        return (t_getattributens(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 40:
      HASH_GUARD(0x49F89C466612FC28LL, getattribute) {
        return (t_getattribute(params.rvalAt(0)));
      }
      break;
    case 47:
      HASH_GUARD(0x47B96C0FC62E1E2FLL, removeattribute) {
        return (t_removeattribute(params.rvalAt(0)));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        int count = params.size();
        if (count <= 1) return (t_insertbefore(params.rvalAt(0)));
        return (t_insertbefore(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 67:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 74:
      HASH_GUARD(0x3FFA3F55ECAB93CALL, getelementsbytagname) {
        return (t_getelementsbytagname(params.rvalAt(0)));
      }
      HASH_GUARD(0x1A9CC561CA4817CALL, getelementsbytagnamens) {
        return (t_getelementsbytagnamens(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x29D272CBBC0CDA4ALL, setidattributenode) {
        return (t_setidattributenode(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 79:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(params.rvalAt(0)));
      }
      break;
    case 81:
      HASH_GUARD(0x482E3873306253D1LL, setattributenodens) {
        return (t_setattributenodens(params.rvalAt(0)));
      }
      break;
    case 85:
      HASH_GUARD(0x5EF7C3A45311D955LL, hasattributens) {
        return (t_hasattributens(params.rvalAt(0), params.rvalAt(1)));
      }
      break;
    case 87:
      HASH_GUARD(0x62A40E71FAF19157LL, setattributens) {
        return (t_setattributens(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
      }
      break;
    case 90:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 92:
      HASH_GUARD(0x606827EF5CF34C5CLL, setattributenode) {
        return (t_setattributenode(params.rvalAt(0)));
      }
      break;
    case 93:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(params.rvalAt(0)));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(params.rvalAt(0)));
      }
      break;
    case 95:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)), null);
      }
      break;
    case 97:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(params.rvalAt(0)));
      }
      break;
    case 100:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(params.rvalAt(0)));
      }
      break;
    case 106:
      HASH_GUARD(0x1BFDB4272EA150EALL, removeattributenode) {
        return (t_removeattributenode(params.rvalAt(0)));
      }
      break;
    case 108:
      HASH_GUARD(0x6471DF8BB944FFECLL, hasattribute) {
        return (t_hasattribute(params.rvalAt(0)));
      }
      break;
    case 111:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(params.rvalAt(0)));
      }
      break;
    case 115:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 117:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        int count = params.size();
        if (count <= 1) return (t_c14nfile(params.rvalAt(0)));
        if (count == 2) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1)));
        if (count == 3) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2)));
        if (count == 4) return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3)));
        return (t_c14nfile(params.rvalAt(0), params.rvalAt(1), params.rvalAt(2), params.rvalAt(3), params.rvalAt(4)));
      }
      break;
    case 122:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(params.rvalAt(0), params.rvalAt(1)));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    case 126:
      HASH_GUARD(0x343DF562BB5AB1FELL, getattributenode) {
        return (t_getattributenode(params.rvalAt(0)));
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke(s, params, hash, fatal);
}
Variant c_domelement::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      HASH_GUARD(0x1D5B8B8144F4AB8ALL, setattribute) {
        return (t_setattribute(a0, a1));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
        return (t_replacechild(a0, a1));
      }
      break;
    case 16:
      HASH_GUARD(0x710986B01E1A0890LL, removeattributens) {
        return (t_removeattributens(a0, a1));
      }
      break;
    case 17:
      HASH_GUARD(0x31426AC6A851EE11LL, setidattributens) {
        return (t_setidattributens(a0, a1, a2));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 22:
      HASH_GUARD(0x6F83EC29E110D616LL, setidattribute) {
        return (t_setidattribute(a0, a1));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        return (t_getlineno());
      }
      HASH_GUARD(0x640FA60DDC80EC18LL, getattributenodens) {
        return (t_getattributenodens(a0, a1));
      }
      break;
    case 25:
      HASH_GUARD(0x34E103E06D3F0899LL, getattributens) {
        return (t_getattributens(a0, a1));
      }
      break;
    case 40:
      HASH_GUARD(0x49F89C466612FC28LL, getattribute) {
        return (t_getattribute(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x47B96C0FC62E1E2FLL, removeattribute) {
        return (t_removeattribute(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      break;
    case 67:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        return (t_getnodepath());
      }
      break;
    case 74:
      HASH_GUARD(0x3FFA3F55ECAB93CALL, getelementsbytagname) {
        return (t_getelementsbytagname(a0));
      }
      HASH_GUARD(0x1A9CC561CA4817CALL, getelementsbytagnamens) {
        return (t_getelementsbytagnamens(a0, a1));
      }
      HASH_GUARD(0x29D272CBBC0CDA4ALL, setidattributenode) {
        return (t_setidattributenode(a0, a1));
      }
      break;
    case 79:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
        return (t_appendchild(a0));
      }
      break;
    case 81:
      HASH_GUARD(0x482E3873306253D1LL, setattributenodens) {
        return (t_setattributenodens(a0));
      }
      break;
    case 85:
      HASH_GUARD(0x5EF7C3A45311D955LL, hasattributens) {
        return (t_hasattributens(a0, a1));
      }
      break;
    case 87:
      HASH_GUARD(0x62A40E71FAF19157LL, setattributens) {
        return (t_setattributens(a0, a1, a2));
      }
      break;
    case 90:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        return (t_normalize(), null);
      }
      break;
    case 92:
      HASH_GUARD(0x606827EF5CF34C5CLL, setattributenode) {
        return (t_setattributenode(a0));
      }
      break;
    case 93:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
        return (t_removechild(a0));
      }
      break;
    case 95:
      HASH_GUARD(0x0D31D0AC229C615FLL, __construct) {
        return (t___construct(a0, a1, a2), null);
      }
      break;
    case 97:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
        return (t_lookupprefix(a0));
      }
      break;
    case 100:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 106:
      HASH_GUARD(0x1BFDB4272EA150EALL, removeattributenode) {
        return (t_removeattributenode(a0));
      }
      break;
    case 108:
      HASH_GUARD(0x6471DF8BB944FFECLL, hasattribute) {
        return (t_hasattribute(a0));
      }
      break;
    case 111:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
        return (t_issamenode(a0));
      }
      break;
    case 115:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        return (t___destruct());
      }
      break;
    case 117:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 122:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        return (t_haschildnodes());
      }
      break;
    case 126:
      HASH_GUARD(0x343DF562BB5AB1FELL, getattributenode) {
        return (t_getattributenode(a0));
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_domelement::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_domnode::os_invoke(c, s, params, hash, fatal);
}
Variant c_domelement::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  if (hash < 0) hash = hash_string_i(s);
  switch (hash & 127) {
    case 10:
      HASH_GUARD(0x77570221C582ED8ALL, c14n) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 0) return (t_c14n());
        if (count == 1) return (t_c14n(a0));
        if (count == 2) return (t_c14n(a0, a1));
        if (count == 3) return (t_c14n(a0, a1, a2));
        return (t_c14n(a0, a1, a2, a3));
      }
      HASH_GUARD(0x1D5B8B8144F4AB8ALL, setattribute) {
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
        return (t_setattribute(a0, a1));
      }
      break;
    case 14:
      HASH_GUARD(0x1C51CE3C2356D08ELL, replacechild) {
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
        return (t_replacechild(a0, a1));
      }
      break;
    case 16:
      HASH_GUARD(0x710986B01E1A0890LL, removeattributens) {
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
        return (t_removeattributens(a0, a1));
      }
      break;
    case 17:
      HASH_GUARD(0x31426AC6A851EE11LL, setidattributens) {
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
        return (t_setidattributens(a0, a1, a2));
      }
      break;
    case 18:
      HASH_GUARD(0x4F615934A04E3092LL, clonenode) {
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
        if (count <= 0) return (t_clonenode());
        return (t_clonenode(a0));
      }
      break;
    case 22:
      HASH_GUARD(0x6F83EC29E110D616LL, setidattribute) {
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
        return (t_setidattribute(a0, a1));
      }
      break;
    case 24:
      HASH_GUARD(0x58F8D846AF9E5D18LL, getlineno) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getlineno());
      }
      HASH_GUARD(0x640FA60DDC80EC18LL, getattributenodens) {
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
        return (t_getattributenodens(a0, a1));
      }
      break;
    case 25:
      HASH_GUARD(0x34E103E06D3F0899LL, getattributens) {
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
        return (t_getattributens(a0, a1));
      }
      break;
    case 40:
      HASH_GUARD(0x49F89C466612FC28LL, getattribute) {
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
        return (t_getattribute(a0));
      }
      break;
    case 47:
      HASH_GUARD(0x47B96C0FC62E1E2FLL, removeattribute) {
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
        return (t_removeattribute(a0));
      }
      break;
    case 49:
      HASH_GUARD(0x3CFA6EECF6F5AEB1LL, hasattributes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_hasattributes());
      }
      break;
    case 51:
      HASH_GUARD(0x076FF0CF9185C433LL, insertbefore) {
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
        if (count <= 1) return (t_insertbefore(a0));
        return (t_insertbefore(a0, a1));
      }
      break;
    case 67:
      HASH_GUARD(0x69797649716AE343LL, getnodepath) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_getnodepath());
      }
      break;
    case 74:
      HASH_GUARD(0x3FFA3F55ECAB93CALL, getelementsbytagname) {
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
        return (t_getelementsbytagname(a0));
      }
      HASH_GUARD(0x1A9CC561CA4817CALL, getelementsbytagnamens) {
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
        return (t_getelementsbytagnamens(a0, a1));
      }
      HASH_GUARD(0x29D272CBBC0CDA4ALL, setidattributenode) {
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
        return (t_setidattributenode(a0, a1));
      }
      break;
    case 79:
      HASH_GUARD(0x0B44A59A023E75CFLL, appendchild) {
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
        return (t_appendchild(a0));
      }
      break;
    case 81:
      HASH_GUARD(0x482E3873306253D1LL, setattributenodens) {
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
        return (t_setattributenodens(a0));
      }
      break;
    case 85:
      HASH_GUARD(0x5EF7C3A45311D955LL, hasattributens) {
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
        return (t_hasattributens(a0, a1));
      }
      break;
    case 87:
      HASH_GUARD(0x62A40E71FAF19157LL, setattributens) {
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
        return (t_setattributens(a0, a1, a2));
      }
      break;
    case 90:
      HASH_GUARD(0x11A2D4B742CB6C5ALL, normalize) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_normalize(), null);
      }
      break;
    case 92:
      HASH_GUARD(0x606827EF5CF34C5CLL, setattributenode) {
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
        return (t_setattributenode(a0));
      }
      break;
    case 93:
      HASH_GUARD(0x28D1E151A83B8F5DLL, lookupnamespaceuri) {
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
        return (t_lookupnamespaceuri(a0));
      }
      HASH_GUARD(0x72044C28B65364DDLL, removechild) {
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
        return (t_removechild(a0));
      }
      break;
    case 95:
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
        return (t___construct(a0, a1, a2), null);
      }
      break;
    case 97:
      HASH_GUARD(0x599DE23996CB7D61LL, lookupprefix) {
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
        return (t_lookupprefix(a0));
      }
      break;
    case 100:
      HASH_GUARD(0x643EFC8E9378F664LL, isdefaultnamespace) {
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
        return (t_isdefaultnamespace(a0));
      }
      break;
    case 106:
      HASH_GUARD(0x1BFDB4272EA150EALL, removeattributenode) {
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
        return (t_removeattributenode(a0));
      }
      break;
    case 108:
      HASH_GUARD(0x6471DF8BB944FFECLL, hasattribute) {
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
        return (t_hasattribute(a0));
      }
      break;
    case 111:
      HASH_GUARD(0x102549E9CA2EC6EFLL, issamenode) {
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
        return (t_issamenode(a0));
      }
      break;
    case 115:
      HASH_GUARD(0x7F974836AACC1EF3LL, __destruct) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t___destruct());
      }
      break;
    case 117:
      HASH_GUARD(0x1D1768A6CF365A75LL, c14nfile) {
        Variant a0;
        Variant a1;
        Variant a2;
        Variant a3;
        Variant a4;
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
          if (it == params.end()) break;
          a3 = (*it)->eval(env);
          it++;
          if (it == params.end()) break;
          a4 = (*it)->eval(env);
          it++;
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        int count = params.size();
        if (count <= 1) return (t_c14nfile(a0));
        if (count == 2) return (t_c14nfile(a0, a1));
        if (count == 3) return (t_c14nfile(a0, a1, a2));
        if (count == 4) return (t_c14nfile(a0, a1, a2, a3));
        return (t_c14nfile(a0, a1, a2, a3, a4));
      }
      break;
    case 122:
      HASH_GUARD(0x279AA1B214B7677ALL, issupported) {
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
        return (t_issupported(a0, a1));
      }
      HASH_GUARD(0x5F63574EA50595FALL, haschildnodes) {
        const std::vector<Eval::ExpressionPtr> &params = caller->params();
        std::vector<Eval::ExpressionPtr>::const_iterator it = params.begin();
        do {
        } while(false);
        for (; it != params.end(); ++it) {
          (*it)->eval(env);
        }
        return (t_haschildnodes());
      }
      break;
    case 126:
      HASH_GUARD(0x343DF562BB5AB1FELL, getattributenode) {
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
        return (t_getattributenode(a0));
      }
      break;
    default:
      break;
  }
  return c_domnode::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_domelement::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_domnode::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_domelement$os_get(const char *s) {
  return c_domelement::os_get(s, -1);
}
Variant &cw_domelement$os_lval(const char *s) {
  return c_domelement::os_lval(s, -1);
}
Variant cw_domelement$os_constant(const char *s) {
  return c_domelement::os_constant(s);
}
Variant cw_domelement$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_domelement::os_invoke(c, s, params, -1, fatal);
}
Object create_builtin_object(const char *s, CArrRef params, bool init /* = true */,ObjectData* root /* = NULL*/) {
  DECLARE_SYSTEM_GLOBALS(g);
  int64 hash = hash_string_i(s);
  switch (hash & 255) {
    case 9:
      HASH_CREATE_OBJECT(0x21EF70351574EC09LL, reflectionfunction);
      break;
    case 11:
      HASH_CREATE_OBJECT(0x4365BE9E5A85E60BLL, domnotation);
      break;
    case 19:
      HASH_CREATE_OBJECT(0x6A76B9AABB7CC713LL, simplexmlelement);
      break;
    case 21:
      HASH_CREATE_OBJECT(0x32E5C767255D2515LL, soapfault);
      break;
    case 26:
      HASH_CREATE_OBJECT(0x5CBA5B52C68B501ALL, domentityreference);
      break;
    case 29:
      HASH_CREATE_OBJECT(0x35A44A5E6AE2E71DLL, reflectionclass);
      break;
    case 30:
      HASH_CREATE_OBJECT(0x3DB8FB455A602A1ELL, datetime);
      break;
    case 50:
      HASH_CREATE_OBJECT(0x365899865E2EAA32LL, recursiveiteratoriterator);
      break;
    case 53:
      HASH_CREATE_OBJECT(0x7909270014376235LL, soapvar);
      break;
    case 56:
      HASH_CREATE_OBJECT(0x672AD818DDE95538LL, reflectionexception);
      break;
    case 62:
      HASH_CREATE_OBJECT(0x49FC1A1F7B878C3ELL, soapserver);
      break;
    case 65:
      HASH_CREATE_OBJECT(0x227C70ED5B143841LL, outofboundsexception);
      break;
    case 70:
      HASH_CREATE_OBJECT(0x196F76C9C527B946LL, domnamednodemap);
      HASH_CREATE_OBJECT(0x66282FC8E4EF4E46LL, badfunctioncallexception);
      break;
    case 73:
      HASH_CREATE_OBJECT(0x5AA2EE582E0D3849LL, unexpectedvalueexception);
      break;
    case 75:
      HASH_CREATE_OBJECT(0x4860DC2F9B0E6D4BLL, logicexception);
      break;
    case 77:
      HASH_CREATE_OBJECT(0x164F490DAC49174DLL, domtext);
      break;
    case 78:
      HASH_CREATE_OBJECT(0x308D1D59986A3D4ELL, soapheader);
      break;
    case 79:
      HASH_CREATE_OBJECT(0x5BA243B9FBA7A64FLL, splobjectstorage);
      break;
    case 83:
      HASH_CREATE_OBJECT(0x02239AA5D64B1453LL, domcharacterdata);
      HASH_CREATE_OBJECT(0x6AF83706F76A9D53LL, rangeexception);
      break;
    case 94:
      HASH_CREATE_OBJECT(0x7754323897E8A15ELL, directoryiterator);
      break;
    case 97:
      HASH_CREATE_OBJECT(0x70EBB2CC1FBF9761LL, domdocumenttype);
      break;
    case 98:
      HASH_CREATE_OBJECT(0x70747A09B3523662LL, domnode);
      break;
    case 101:
      HASH_CREATE_OBJECT(0x2FF1EF6EBB3DA065LL, xmlwriter);
      break;
    case 104:
      HASH_CREATE_OBJECT(0x7E07B9C27FE59D68LL, domcomment);
      HASH_CREATE_OBJECT(0x17A40D895C55D968LL, soapparam);
      break;
    case 106:
      HASH_CREATE_OBJECT(0x6A5B99D5B1A4566ALL, domexception);
      break;
    case 107:
      HASH_CREATE_OBJECT(0x3CAE91CFA2AAD16BLL, lengthexception);
      break;
    case 115:
      HASH_CREATE_OBJECT(0x3D5870E53BF89873LL, arrayiterator);
      HASH_CREATE_OBJECT(0x0B61E0BFCFA06573LL, reflectionextension);
      break;
    case 117:
      HASH_CREATE_OBJECT(0x085A4F6FD393D475LL, invalidargumentexception);
      break;
    case 122:
      HASH_CREATE_OBJECT(0x41A9F7D81254DD7ALL, reflectionobject);
      break;
    case 125:
      HASH_CREATE_OBJECT(0x7AD161197633B87DLL, domdocumentfragment);
      HASH_CREATE_OBJECT(0x2E899D2A2572EF7DLL, sqlite3result);
      break;
    case 128:
      HASH_CREATE_OBJECT(0x59B3EFD1CD987F80LL, domprocessinginstruction);
      break;
    case 130:
      HASH_CREATE_OBJECT(0x33BD46E935281082LL, reflectionfunctionabstract);
      break;
    case 131:
      HASH_CREATE_OBJECT(0x24DE53FD7D3E8383LL, datetimezone);
      break;
    case 132:
      HASH_CREATE_OBJECT(0x70D4A29DE6A7BA84LL, domainexception);
      break;
    case 143:
      HASH_CREATE_OBJECT(0x34C95AF311506C8FLL, directory);
      break;
    case 144:
      HASH_CREATE_OBJECT(0x06335FF83CDEA590LL, badmethodcallexception);
      break;
    case 147:
      HASH_CREATE_OBJECT(0x2E7081C468A05993LL, reflectionparameter);
      break;
    case 148:
      HASH_CREATE_OBJECT(0x6FFDF8DF15CABE94LL, domcdatasection);
      HASH_CREATE_OBJECT(0x6576EACCCE24D694LL, __php_incomplete_class);
      HASH_CREATE_OBJECT(0x47D93E6F80B66A94LL, exception);
      break;
    case 152:
      HASH_CREATE_OBJECT(0x3CEBA108A1BAB998LL, pear_error);
      break;
    case 167:
      HASH_CREATE_OBJECT(0x71089C29FE923FA7LL, splfileinfo);
      break;
    case 168:
      HASH_CREATE_OBJECT(0x7D46DB2E09C6DBA8LL, domxpath);
      break;
    case 169:
      HASH_CREATE_OBJECT(0x1D35C3EFD00E11A9LL, sqlite3);
      break;
    case 175:
      HASH_CREATE_OBJECT(0x2E650138BA043AAFLL, domdocument);
      break;
    case 176:
      HASH_CREATE_OBJECT(0x43BBC8F6F28E44B0LL, reflectionmethod);
      break;
    case 177:
      HASH_CREATE_OBJECT(0x6DD772FA3B20B8B1LL, domnodelist);
      break;
    case 195:
      HASH_CREATE_OBJECT(0x40B3C44077F5DDC3LL, domelement);
      break;
    case 200:
      HASH_CREATE_OBJECT(0x2E363D51549781C8LL, appenditerator);
      break;
    case 203:
      HASH_CREATE_OBJECT(0x253AC17234FC8CCBLL, libxmlerror);
      break;
    case 210:
      HASH_CREATE_OBJECT(0x52E5CC58B841AED2LL, sqlite3stmt);
      HASH_CREATE_OBJECT(0x297ECCC7A259EDD2LL, splfileobject);
      break;
    case 212:
      HASH_CREATE_OBJECT(0x5E623FEAF7B068D4LL, underflowexception);
      break;
    case 216:
      HASH_CREATE_OBJECT(0x464D3427431A6ED8LL, recursivedirectoryiterator);
      break;
    case 217:
      HASH_CREATE_OBJECT(0x0E4CCE95F6A727D9LL, overflowexception);
      break;
    case 218:
      HASH_CREATE_OBJECT(0x7A52EBE538182BDALL, soapclient);
      break;
    case 220:
      HASH_CREATE_OBJECT(0x1078CB3118A8B3DCLL, runtimeexception);
      break;
    case 228:
      HASH_CREATE_OBJECT(0x75AA2571BDB659E4LL, reflectionproperty);
      break;
    case 244:
      HASH_CREATE_OBJECT(0x297174231D4912F4LL, domattr);
      break;
    case 247:
      HASH_CREATE_OBJECT(0x3D69936178BA13F7LL, stdclass);
      break;
    case 252:
      HASH_CREATE_OBJECT(0x622D4DF07E2A01FCLL, outofrangeexception);
      break;
    case 253:
      HASH_CREATE_OBJECT(0x74419B70A46387FDLL, domimplementation);
      break;
    case 255:
      HASH_CREATE_OBJECT(0x624835B2D74B86FFLL, domentity);
      break;
    default:
      break;
  }
  return throw_missing_class(s);
}
Variant invoke_builtin_static_method(const char *s, const char *method, CArrRef params, bool fatal) {
  DECLARE_SYSTEM_GLOBALS(g);
  int64 hash = hash_string_i(s);
  switch (hash & 255) {
    case 9:
      HASH_INVOKE_STATIC_METHOD(0x21EF70351574EC09LL, reflectionfunction);
      break;
    case 11:
      HASH_INVOKE_STATIC_METHOD(0x4365BE9E5A85E60BLL, domnotation);
      break;
    case 19:
      HASH_INVOKE_STATIC_METHOD(0x6A76B9AABB7CC713LL, simplexmlelement);
      break;
    case 21:
      HASH_INVOKE_STATIC_METHOD(0x32E5C767255D2515LL, soapfault);
      break;
    case 26:
      HASH_INVOKE_STATIC_METHOD(0x5CBA5B52C68B501ALL, domentityreference);
      break;
    case 29:
      HASH_INVOKE_STATIC_METHOD(0x35A44A5E6AE2E71DLL, reflectionclass);
      break;
    case 30:
      HASH_INVOKE_STATIC_METHOD(0x3DB8FB455A602A1ELL, datetime);
      break;
    case 50:
      HASH_INVOKE_STATIC_METHOD(0x365899865E2EAA32LL, recursiveiteratoriterator);
      break;
    case 53:
      HASH_INVOKE_STATIC_METHOD(0x7909270014376235LL, soapvar);
      break;
    case 56:
      HASH_INVOKE_STATIC_METHOD(0x672AD818DDE95538LL, reflectionexception);
      break;
    case 62:
      HASH_INVOKE_STATIC_METHOD(0x49FC1A1F7B878C3ELL, soapserver);
      break;
    case 65:
      HASH_INVOKE_STATIC_METHOD(0x227C70ED5B143841LL, outofboundsexception);
      break;
    case 70:
      HASH_INVOKE_STATIC_METHOD(0x196F76C9C527B946LL, domnamednodemap);
      HASH_INVOKE_STATIC_METHOD(0x66282FC8E4EF4E46LL, badfunctioncallexception);
      break;
    case 73:
      HASH_INVOKE_STATIC_METHOD(0x5AA2EE582E0D3849LL, unexpectedvalueexception);
      break;
    case 75:
      HASH_INVOKE_STATIC_METHOD(0x4860DC2F9B0E6D4BLL, logicexception);
      break;
    case 77:
      HASH_INVOKE_STATIC_METHOD(0x164F490DAC49174DLL, domtext);
      break;
    case 78:
      HASH_INVOKE_STATIC_METHOD(0x308D1D59986A3D4ELL, soapheader);
      break;
    case 79:
      HASH_INVOKE_STATIC_METHOD(0x5BA243B9FBA7A64FLL, splobjectstorage);
      break;
    case 83:
      HASH_INVOKE_STATIC_METHOD(0x02239AA5D64B1453LL, domcharacterdata);
      HASH_INVOKE_STATIC_METHOD(0x6AF83706F76A9D53LL, rangeexception);
      break;
    case 94:
      HASH_INVOKE_STATIC_METHOD(0x7754323897E8A15ELL, directoryiterator);
      break;
    case 97:
      HASH_INVOKE_STATIC_METHOD(0x70EBB2CC1FBF9761LL, domdocumenttype);
      break;
    case 98:
      HASH_INVOKE_STATIC_METHOD(0x70747A09B3523662LL, domnode);
      break;
    case 101:
      HASH_INVOKE_STATIC_METHOD(0x2FF1EF6EBB3DA065LL, xmlwriter);
      break;
    case 104:
      HASH_INVOKE_STATIC_METHOD(0x7E07B9C27FE59D68LL, domcomment);
      HASH_INVOKE_STATIC_METHOD(0x17A40D895C55D968LL, soapparam);
      break;
    case 106:
      HASH_INVOKE_STATIC_METHOD(0x6A5B99D5B1A4566ALL, domexception);
      break;
    case 107:
      HASH_INVOKE_STATIC_METHOD(0x3CAE91CFA2AAD16BLL, lengthexception);
      break;
    case 115:
      HASH_INVOKE_STATIC_METHOD(0x3D5870E53BF89873LL, arrayiterator);
      HASH_INVOKE_STATIC_METHOD(0x0B61E0BFCFA06573LL, reflectionextension);
      break;
    case 117:
      HASH_INVOKE_STATIC_METHOD(0x085A4F6FD393D475LL, invalidargumentexception);
      break;
    case 122:
      HASH_INVOKE_STATIC_METHOD(0x41A9F7D81254DD7ALL, reflectionobject);
      break;
    case 125:
      HASH_INVOKE_STATIC_METHOD(0x7AD161197633B87DLL, domdocumentfragment);
      HASH_INVOKE_STATIC_METHOD(0x2E899D2A2572EF7DLL, sqlite3result);
      break;
    case 128:
      HASH_INVOKE_STATIC_METHOD(0x59B3EFD1CD987F80LL, domprocessinginstruction);
      break;
    case 130:
      HASH_INVOKE_STATIC_METHOD(0x33BD46E935281082LL, reflectionfunctionabstract);
      break;
    case 131:
      HASH_INVOKE_STATIC_METHOD(0x24DE53FD7D3E8383LL, datetimezone);
      break;
    case 132:
      HASH_INVOKE_STATIC_METHOD(0x70D4A29DE6A7BA84LL, domainexception);
      break;
    case 143:
      HASH_INVOKE_STATIC_METHOD(0x34C95AF311506C8FLL, directory);
      break;
    case 144:
      HASH_INVOKE_STATIC_METHOD(0x06335FF83CDEA590LL, badmethodcallexception);
      break;
    case 147:
      HASH_INVOKE_STATIC_METHOD(0x2E7081C468A05993LL, reflectionparameter);
      break;
    case 148:
      HASH_INVOKE_STATIC_METHOD(0x6FFDF8DF15CABE94LL, domcdatasection);
      HASH_INVOKE_STATIC_METHOD(0x6576EACCCE24D694LL, __php_incomplete_class);
      HASH_INVOKE_STATIC_METHOD(0x47D93E6F80B66A94LL, exception);
      break;
    case 152:
      HASH_INVOKE_STATIC_METHOD(0x3CEBA108A1BAB998LL, pear_error);
      break;
    case 167:
      HASH_INVOKE_STATIC_METHOD(0x71089C29FE923FA7LL, splfileinfo);
      break;
    case 168:
      HASH_INVOKE_STATIC_METHOD(0x7D46DB2E09C6DBA8LL, domxpath);
      break;
    case 169:
      HASH_INVOKE_STATIC_METHOD(0x1D35C3EFD00E11A9LL, sqlite3);
      break;
    case 175:
      HASH_INVOKE_STATIC_METHOD(0x2E650138BA043AAFLL, domdocument);
      break;
    case 176:
      HASH_INVOKE_STATIC_METHOD(0x43BBC8F6F28E44B0LL, reflectionmethod);
      break;
    case 177:
      HASH_INVOKE_STATIC_METHOD(0x6DD772FA3B20B8B1LL, domnodelist);
      break;
    case 195:
      HASH_INVOKE_STATIC_METHOD(0x40B3C44077F5DDC3LL, domelement);
      break;
    case 200:
      HASH_INVOKE_STATIC_METHOD(0x2E363D51549781C8LL, appenditerator);
      break;
    case 203:
      HASH_INVOKE_STATIC_METHOD(0x253AC17234FC8CCBLL, libxmlerror);
      break;
    case 210:
      HASH_INVOKE_STATIC_METHOD(0x52E5CC58B841AED2LL, sqlite3stmt);
      HASH_INVOKE_STATIC_METHOD(0x297ECCC7A259EDD2LL, splfileobject);
      break;
    case 212:
      HASH_INVOKE_STATIC_METHOD(0x5E623FEAF7B068D4LL, underflowexception);
      break;
    case 216:
      HASH_INVOKE_STATIC_METHOD(0x464D3427431A6ED8LL, recursivedirectoryiterator);
      break;
    case 217:
      HASH_INVOKE_STATIC_METHOD(0x0E4CCE95F6A727D9LL, overflowexception);
      break;
    case 218:
      HASH_INVOKE_STATIC_METHOD(0x7A52EBE538182BDALL, soapclient);
      break;
    case 220:
      HASH_INVOKE_STATIC_METHOD(0x1078CB3118A8B3DCLL, runtimeexception);
      break;
    case 228:
      HASH_INVOKE_STATIC_METHOD(0x75AA2571BDB659E4LL, reflectionproperty);
      break;
    case 244:
      HASH_INVOKE_STATIC_METHOD(0x297174231D4912F4LL, domattr);
      break;
    case 247:
      HASH_INVOKE_STATIC_METHOD(0x3D69936178BA13F7LL, stdclass);
      break;
    case 252:
      HASH_INVOKE_STATIC_METHOD(0x622D4DF07E2A01FCLL, outofrangeexception);
      break;
    case 253:
      HASH_INVOKE_STATIC_METHOD(0x74419B70A46387FDLL, domimplementation);
      break;
    case 255:
      HASH_INVOKE_STATIC_METHOD(0x624835B2D74B86FFLL, domentity);
      break;
    default:
      break;
  }
  if (fatal) {
    return throw_missing_class(s);
  } else {
    Logger::Warning("call_user_func to non-existent class's method %s::%s", s, method);
    return false;
  }
}
Variant get_builtin_static_property(const char *s, const char *prop) {
  DECLARE_SYSTEM_GLOBALS(g);
  int64 hash = hash_string_i(s);
  switch (hash & 255) {
    case 9:
      HASH_GET_STATIC_PROPERTY(0x21EF70351574EC09LL, reflectionfunction);
      break;
    case 11:
      HASH_GET_STATIC_PROPERTY(0x4365BE9E5A85E60BLL, domnotation);
      break;
    case 19:
      HASH_GET_STATIC_PROPERTY(0x6A76B9AABB7CC713LL, simplexmlelement);
      break;
    case 21:
      HASH_GET_STATIC_PROPERTY(0x32E5C767255D2515LL, soapfault);
      break;
    case 26:
      HASH_GET_STATIC_PROPERTY(0x5CBA5B52C68B501ALL, domentityreference);
      break;
    case 29:
      HASH_GET_STATIC_PROPERTY(0x35A44A5E6AE2E71DLL, reflectionclass);
      break;
    case 30:
      HASH_GET_STATIC_PROPERTY(0x3DB8FB455A602A1ELL, datetime);
      break;
    case 50:
      HASH_GET_STATIC_PROPERTY(0x365899865E2EAA32LL, recursiveiteratoriterator);
      break;
    case 53:
      HASH_GET_STATIC_PROPERTY(0x7909270014376235LL, soapvar);
      break;
    case 56:
      HASH_GET_STATIC_PROPERTY(0x672AD818DDE95538LL, reflectionexception);
      break;
    case 62:
      HASH_GET_STATIC_PROPERTY(0x49FC1A1F7B878C3ELL, soapserver);
      break;
    case 65:
      HASH_GET_STATIC_PROPERTY(0x227C70ED5B143841LL, outofboundsexception);
      break;
    case 70:
      HASH_GET_STATIC_PROPERTY(0x196F76C9C527B946LL, domnamednodemap);
      HASH_GET_STATIC_PROPERTY(0x66282FC8E4EF4E46LL, badfunctioncallexception);
      break;
    case 73:
      HASH_GET_STATIC_PROPERTY(0x5AA2EE582E0D3849LL, unexpectedvalueexception);
      break;
    case 75:
      HASH_GET_STATIC_PROPERTY(0x4860DC2F9B0E6D4BLL, logicexception);
      break;
    case 77:
      HASH_GET_STATIC_PROPERTY(0x164F490DAC49174DLL, domtext);
      break;
    case 78:
      HASH_GET_STATIC_PROPERTY(0x308D1D59986A3D4ELL, soapheader);
      break;
    case 79:
      HASH_GET_STATIC_PROPERTY(0x5BA243B9FBA7A64FLL, splobjectstorage);
      break;
    case 83:
      HASH_GET_STATIC_PROPERTY(0x02239AA5D64B1453LL, domcharacterdata);
      HASH_GET_STATIC_PROPERTY(0x6AF83706F76A9D53LL, rangeexception);
      break;
    case 94:
      HASH_GET_STATIC_PROPERTY(0x7754323897E8A15ELL, directoryiterator);
      break;
    case 97:
      HASH_GET_STATIC_PROPERTY(0x70EBB2CC1FBF9761LL, domdocumenttype);
      break;
    case 98:
      HASH_GET_STATIC_PROPERTY(0x70747A09B3523662LL, domnode);
      break;
    case 101:
      HASH_GET_STATIC_PROPERTY(0x2FF1EF6EBB3DA065LL, xmlwriter);
      break;
    case 104:
      HASH_GET_STATIC_PROPERTY(0x7E07B9C27FE59D68LL, domcomment);
      HASH_GET_STATIC_PROPERTY(0x17A40D895C55D968LL, soapparam);
      break;
    case 106:
      HASH_GET_STATIC_PROPERTY(0x6A5B99D5B1A4566ALL, domexception);
      break;
    case 107:
      HASH_GET_STATIC_PROPERTY(0x3CAE91CFA2AAD16BLL, lengthexception);
      break;
    case 115:
      HASH_GET_STATIC_PROPERTY(0x3D5870E53BF89873LL, arrayiterator);
      HASH_GET_STATIC_PROPERTY(0x0B61E0BFCFA06573LL, reflectionextension);
      break;
    case 117:
      HASH_GET_STATIC_PROPERTY(0x085A4F6FD393D475LL, invalidargumentexception);
      break;
    case 122:
      HASH_GET_STATIC_PROPERTY(0x41A9F7D81254DD7ALL, reflectionobject);
      break;
    case 125:
      HASH_GET_STATIC_PROPERTY(0x7AD161197633B87DLL, domdocumentfragment);
      HASH_GET_STATIC_PROPERTY(0x2E899D2A2572EF7DLL, sqlite3result);
      break;
    case 128:
      HASH_GET_STATIC_PROPERTY(0x59B3EFD1CD987F80LL, domprocessinginstruction);
      break;
    case 130:
      HASH_GET_STATIC_PROPERTY(0x33BD46E935281082LL, reflectionfunctionabstract);
      break;
    case 131:
      HASH_GET_STATIC_PROPERTY(0x24DE53FD7D3E8383LL, datetimezone);
      break;
    case 132:
      HASH_GET_STATIC_PROPERTY(0x70D4A29DE6A7BA84LL, domainexception);
      break;
    case 143:
      HASH_GET_STATIC_PROPERTY(0x34C95AF311506C8FLL, directory);
      break;
    case 144:
      HASH_GET_STATIC_PROPERTY(0x06335FF83CDEA590LL, badmethodcallexception);
      break;
    case 147:
      HASH_GET_STATIC_PROPERTY(0x2E7081C468A05993LL, reflectionparameter);
      break;
    case 148:
      HASH_GET_STATIC_PROPERTY(0x6FFDF8DF15CABE94LL, domcdatasection);
      HASH_GET_STATIC_PROPERTY(0x6576EACCCE24D694LL, __php_incomplete_class);
      HASH_GET_STATIC_PROPERTY(0x47D93E6F80B66A94LL, exception);
      break;
    case 152:
      HASH_GET_STATIC_PROPERTY(0x3CEBA108A1BAB998LL, pear_error);
      break;
    case 167:
      HASH_GET_STATIC_PROPERTY(0x71089C29FE923FA7LL, splfileinfo);
      break;
    case 168:
      HASH_GET_STATIC_PROPERTY(0x7D46DB2E09C6DBA8LL, domxpath);
      break;
    case 169:
      HASH_GET_STATIC_PROPERTY(0x1D35C3EFD00E11A9LL, sqlite3);
      break;
    case 175:
      HASH_GET_STATIC_PROPERTY(0x2E650138BA043AAFLL, domdocument);
      break;
    case 176:
      HASH_GET_STATIC_PROPERTY(0x43BBC8F6F28E44B0LL, reflectionmethod);
      break;
    case 177:
      HASH_GET_STATIC_PROPERTY(0x6DD772FA3B20B8B1LL, domnodelist);
      break;
    case 195:
      HASH_GET_STATIC_PROPERTY(0x40B3C44077F5DDC3LL, domelement);
      break;
    case 200:
      HASH_GET_STATIC_PROPERTY(0x2E363D51549781C8LL, appenditerator);
      break;
    case 203:
      HASH_GET_STATIC_PROPERTY(0x253AC17234FC8CCBLL, libxmlerror);
      break;
    case 210:
      HASH_GET_STATIC_PROPERTY(0x52E5CC58B841AED2LL, sqlite3stmt);
      HASH_GET_STATIC_PROPERTY(0x297ECCC7A259EDD2LL, splfileobject);
      break;
    case 212:
      HASH_GET_STATIC_PROPERTY(0x5E623FEAF7B068D4LL, underflowexception);
      break;
    case 216:
      HASH_GET_STATIC_PROPERTY(0x464D3427431A6ED8LL, recursivedirectoryiterator);
      break;
    case 217:
      HASH_GET_STATIC_PROPERTY(0x0E4CCE95F6A727D9LL, overflowexception);
      break;
    case 218:
      HASH_GET_STATIC_PROPERTY(0x7A52EBE538182BDALL, soapclient);
      break;
    case 220:
      HASH_GET_STATIC_PROPERTY(0x1078CB3118A8B3DCLL, runtimeexception);
      break;
    case 228:
      HASH_GET_STATIC_PROPERTY(0x75AA2571BDB659E4LL, reflectionproperty);
      break;
    case 244:
      HASH_GET_STATIC_PROPERTY(0x297174231D4912F4LL, domattr);
      break;
    case 247:
      HASH_GET_STATIC_PROPERTY(0x3D69936178BA13F7LL, stdclass);
      break;
    case 252:
      HASH_GET_STATIC_PROPERTY(0x622D4DF07E2A01FCLL, outofrangeexception);
      break;
    case 253:
      HASH_GET_STATIC_PROPERTY(0x74419B70A46387FDLL, domimplementation);
      break;
    case 255:
      HASH_GET_STATIC_PROPERTY(0x624835B2D74B86FFLL, domentity);
      break;
    default:
      break;
  }
  return null;
}
Variant *get_builtin_static_property_lv(const char *s, const char *prop) {
  DECLARE_SYSTEM_GLOBALS(g);
  int64 hash = hash_string_i(s);
  switch (hash & 255) {
    case 9:
      HASH_GET_STATIC_PROPERTY_LV(0x21EF70351574EC09LL, reflectionfunction);
      break;
    case 11:
      HASH_GET_STATIC_PROPERTY_LV(0x4365BE9E5A85E60BLL, domnotation);
      break;
    case 19:
      HASH_GET_STATIC_PROPERTY_LV(0x6A76B9AABB7CC713LL, simplexmlelement);
      break;
    case 21:
      HASH_GET_STATIC_PROPERTY_LV(0x32E5C767255D2515LL, soapfault);
      break;
    case 26:
      HASH_GET_STATIC_PROPERTY_LV(0x5CBA5B52C68B501ALL, domentityreference);
      break;
    case 29:
      HASH_GET_STATIC_PROPERTY_LV(0x35A44A5E6AE2E71DLL, reflectionclass);
      break;
    case 30:
      HASH_GET_STATIC_PROPERTY_LV(0x3DB8FB455A602A1ELL, datetime);
      break;
    case 50:
      HASH_GET_STATIC_PROPERTY_LV(0x365899865E2EAA32LL, recursiveiteratoriterator);
      break;
    case 53:
      HASH_GET_STATIC_PROPERTY_LV(0x7909270014376235LL, soapvar);
      break;
    case 56:
      HASH_GET_STATIC_PROPERTY_LV(0x672AD818DDE95538LL, reflectionexception);
      break;
    case 62:
      HASH_GET_STATIC_PROPERTY_LV(0x49FC1A1F7B878C3ELL, soapserver);
      break;
    case 65:
      HASH_GET_STATIC_PROPERTY_LV(0x227C70ED5B143841LL, outofboundsexception);
      break;
    case 70:
      HASH_GET_STATIC_PROPERTY_LV(0x196F76C9C527B946LL, domnamednodemap);
      HASH_GET_STATIC_PROPERTY_LV(0x66282FC8E4EF4E46LL, badfunctioncallexception);
      break;
    case 73:
      HASH_GET_STATIC_PROPERTY_LV(0x5AA2EE582E0D3849LL, unexpectedvalueexception);
      break;
    case 75:
      HASH_GET_STATIC_PROPERTY_LV(0x4860DC2F9B0E6D4BLL, logicexception);
      break;
    case 77:
      HASH_GET_STATIC_PROPERTY_LV(0x164F490DAC49174DLL, domtext);
      break;
    case 78:
      HASH_GET_STATIC_PROPERTY_LV(0x308D1D59986A3D4ELL, soapheader);
      break;
    case 79:
      HASH_GET_STATIC_PROPERTY_LV(0x5BA243B9FBA7A64FLL, splobjectstorage);
      break;
    case 83:
      HASH_GET_STATIC_PROPERTY_LV(0x02239AA5D64B1453LL, domcharacterdata);
      HASH_GET_STATIC_PROPERTY_LV(0x6AF83706F76A9D53LL, rangeexception);
      break;
    case 94:
      HASH_GET_STATIC_PROPERTY_LV(0x7754323897E8A15ELL, directoryiterator);
      break;
    case 97:
      HASH_GET_STATIC_PROPERTY_LV(0x70EBB2CC1FBF9761LL, domdocumenttype);
      break;
    case 98:
      HASH_GET_STATIC_PROPERTY_LV(0x70747A09B3523662LL, domnode);
      break;
    case 101:
      HASH_GET_STATIC_PROPERTY_LV(0x2FF1EF6EBB3DA065LL, xmlwriter);
      break;
    case 104:
      HASH_GET_STATIC_PROPERTY_LV(0x7E07B9C27FE59D68LL, domcomment);
      HASH_GET_STATIC_PROPERTY_LV(0x17A40D895C55D968LL, soapparam);
      break;
    case 106:
      HASH_GET_STATIC_PROPERTY_LV(0x6A5B99D5B1A4566ALL, domexception);
      break;
    case 107:
      HASH_GET_STATIC_PROPERTY_LV(0x3CAE91CFA2AAD16BLL, lengthexception);
      break;
    case 115:
      HASH_GET_STATIC_PROPERTY_LV(0x3D5870E53BF89873LL, arrayiterator);
      HASH_GET_STATIC_PROPERTY_LV(0x0B61E0BFCFA06573LL, reflectionextension);
      break;
    case 117:
      HASH_GET_STATIC_PROPERTY_LV(0x085A4F6FD393D475LL, invalidargumentexception);
      break;
    case 122:
      HASH_GET_STATIC_PROPERTY_LV(0x41A9F7D81254DD7ALL, reflectionobject);
      break;
    case 125:
      HASH_GET_STATIC_PROPERTY_LV(0x7AD161197633B87DLL, domdocumentfragment);
      HASH_GET_STATIC_PROPERTY_LV(0x2E899D2A2572EF7DLL, sqlite3result);
      break;
    case 128:
      HASH_GET_STATIC_PROPERTY_LV(0x59B3EFD1CD987F80LL, domprocessinginstruction);
      break;
    case 130:
      HASH_GET_STATIC_PROPERTY_LV(0x33BD46E935281082LL, reflectionfunctionabstract);
      break;
    case 131:
      HASH_GET_STATIC_PROPERTY_LV(0x24DE53FD7D3E8383LL, datetimezone);
      break;
    case 132:
      HASH_GET_STATIC_PROPERTY_LV(0x70D4A29DE6A7BA84LL, domainexception);
      break;
    case 143:
      HASH_GET_STATIC_PROPERTY_LV(0x34C95AF311506C8FLL, directory);
      break;
    case 144:
      HASH_GET_STATIC_PROPERTY_LV(0x06335FF83CDEA590LL, badmethodcallexception);
      break;
    case 147:
      HASH_GET_STATIC_PROPERTY_LV(0x2E7081C468A05993LL, reflectionparameter);
      break;
    case 148:
      HASH_GET_STATIC_PROPERTY_LV(0x6FFDF8DF15CABE94LL, domcdatasection);
      HASH_GET_STATIC_PROPERTY_LV(0x6576EACCCE24D694LL, __php_incomplete_class);
      HASH_GET_STATIC_PROPERTY_LV(0x47D93E6F80B66A94LL, exception);
      break;
    case 152:
      HASH_GET_STATIC_PROPERTY_LV(0x3CEBA108A1BAB998LL, pear_error);
      break;
    case 167:
      HASH_GET_STATIC_PROPERTY_LV(0x71089C29FE923FA7LL, splfileinfo);
      break;
    case 168:
      HASH_GET_STATIC_PROPERTY_LV(0x7D46DB2E09C6DBA8LL, domxpath);
      break;
    case 169:
      HASH_GET_STATIC_PROPERTY_LV(0x1D35C3EFD00E11A9LL, sqlite3);
      break;
    case 175:
      HASH_GET_STATIC_PROPERTY_LV(0x2E650138BA043AAFLL, domdocument);
      break;
    case 176:
      HASH_GET_STATIC_PROPERTY_LV(0x43BBC8F6F28E44B0LL, reflectionmethod);
      break;
    case 177:
      HASH_GET_STATIC_PROPERTY_LV(0x6DD772FA3B20B8B1LL, domnodelist);
      break;
    case 195:
      HASH_GET_STATIC_PROPERTY_LV(0x40B3C44077F5DDC3LL, domelement);
      break;
    case 200:
      HASH_GET_STATIC_PROPERTY_LV(0x2E363D51549781C8LL, appenditerator);
      break;
    case 203:
      HASH_GET_STATIC_PROPERTY_LV(0x253AC17234FC8CCBLL, libxmlerror);
      break;
    case 210:
      HASH_GET_STATIC_PROPERTY_LV(0x52E5CC58B841AED2LL, sqlite3stmt);
      HASH_GET_STATIC_PROPERTY_LV(0x297ECCC7A259EDD2LL, splfileobject);
      break;
    case 212:
      HASH_GET_STATIC_PROPERTY_LV(0x5E623FEAF7B068D4LL, underflowexception);
      break;
    case 216:
      HASH_GET_STATIC_PROPERTY_LV(0x464D3427431A6ED8LL, recursivedirectoryiterator);
      break;
    case 217:
      HASH_GET_STATIC_PROPERTY_LV(0x0E4CCE95F6A727D9LL, overflowexception);
      break;
    case 218:
      HASH_GET_STATIC_PROPERTY_LV(0x7A52EBE538182BDALL, soapclient);
      break;
    case 220:
      HASH_GET_STATIC_PROPERTY_LV(0x1078CB3118A8B3DCLL, runtimeexception);
      break;
    case 228:
      HASH_GET_STATIC_PROPERTY_LV(0x75AA2571BDB659E4LL, reflectionproperty);
      break;
    case 244:
      HASH_GET_STATIC_PROPERTY_LV(0x297174231D4912F4LL, domattr);
      break;
    case 247:
      HASH_GET_STATIC_PROPERTY_LV(0x3D69936178BA13F7LL, stdclass);
      break;
    case 252:
      HASH_GET_STATIC_PROPERTY_LV(0x622D4DF07E2A01FCLL, outofrangeexception);
      break;
    case 253:
      HASH_GET_STATIC_PROPERTY_LV(0x74419B70A46387FDLL, domimplementation);
      break;
    case 255:
      HASH_GET_STATIC_PROPERTY_LV(0x624835B2D74B86FFLL, domentity);
      break;
    default:
      break;
  }
  return NULL;
}
Variant get_builtin_class_constant(const char *s, const char *constant) {
  DECLARE_SYSTEM_GLOBALS(g);
  int64 hash = hash_string_i(s);
  switch (hash & 255) {
    case 9:
      HASH_GET_CLASS_CONSTANT(0x21EF70351574EC09LL, reflectionfunction);
      break;
    case 11:
      HASH_GET_CLASS_CONSTANT(0x4365BE9E5A85E60BLL, domnotation);
      break;
    case 19:
      HASH_GET_CLASS_CONSTANT(0x6A76B9AABB7CC713LL, simplexmlelement);
      break;
    case 21:
      HASH_GET_CLASS_CONSTANT(0x32E5C767255D2515LL, soapfault);
      break;
    case 26:
      HASH_GET_CLASS_CONSTANT(0x5CBA5B52C68B501ALL, domentityreference);
      break;
    case 29:
      HASH_GET_CLASS_CONSTANT(0x35A44A5E6AE2E71DLL, reflectionclass);
      break;
    case 30:
      HASH_GET_CLASS_CONSTANT(0x3DB8FB455A602A1ELL, datetime);
      break;
    case 50:
      HASH_GET_CLASS_CONSTANT(0x365899865E2EAA32LL, recursiveiteratoriterator);
      break;
    case 53:
      HASH_GET_CLASS_CONSTANT(0x7909270014376235LL, soapvar);
      break;
    case 56:
      HASH_GET_CLASS_CONSTANT(0x672AD818DDE95538LL, reflectionexception);
      break;
    case 62:
      HASH_GET_CLASS_CONSTANT(0x49FC1A1F7B878C3ELL, soapserver);
      break;
    case 65:
      HASH_GET_CLASS_CONSTANT(0x227C70ED5B143841LL, outofboundsexception);
      break;
    case 70:
      HASH_GET_CLASS_CONSTANT(0x196F76C9C527B946LL, domnamednodemap);
      HASH_GET_CLASS_CONSTANT(0x66282FC8E4EF4E46LL, badfunctioncallexception);
      break;
    case 73:
      HASH_GET_CLASS_CONSTANT(0x5AA2EE582E0D3849LL, unexpectedvalueexception);
      break;
    case 75:
      HASH_GET_CLASS_CONSTANT(0x4860DC2F9B0E6D4BLL, logicexception);
      break;
    case 77:
      HASH_GET_CLASS_CONSTANT(0x164F490DAC49174DLL, domtext);
      break;
    case 78:
      HASH_GET_CLASS_CONSTANT(0x308D1D59986A3D4ELL, soapheader);
      break;
    case 79:
      HASH_GET_CLASS_CONSTANT(0x5BA243B9FBA7A64FLL, splobjectstorage);
      break;
    case 83:
      HASH_GET_CLASS_CONSTANT(0x02239AA5D64B1453LL, domcharacterdata);
      HASH_GET_CLASS_CONSTANT(0x6AF83706F76A9D53LL, rangeexception);
      break;
    case 94:
      HASH_GET_CLASS_CONSTANT(0x7754323897E8A15ELL, directoryiterator);
      break;
    case 97:
      HASH_GET_CLASS_CONSTANT(0x70EBB2CC1FBF9761LL, domdocumenttype);
      break;
    case 98:
      HASH_GET_CLASS_CONSTANT(0x70747A09B3523662LL, domnode);
      break;
    case 101:
      HASH_GET_CLASS_CONSTANT(0x2FF1EF6EBB3DA065LL, xmlwriter);
      break;
    case 104:
      HASH_GET_CLASS_CONSTANT(0x7E07B9C27FE59D68LL, domcomment);
      HASH_GET_CLASS_CONSTANT(0x17A40D895C55D968LL, soapparam);
      break;
    case 106:
      HASH_GET_CLASS_CONSTANT(0x6A5B99D5B1A4566ALL, domexception);
      break;
    case 107:
      HASH_GET_CLASS_CONSTANT(0x3CAE91CFA2AAD16BLL, lengthexception);
      break;
    case 115:
      HASH_GET_CLASS_CONSTANT(0x3D5870E53BF89873LL, arrayiterator);
      HASH_GET_CLASS_CONSTANT(0x0B61E0BFCFA06573LL, reflectionextension);
      break;
    case 117:
      HASH_GET_CLASS_CONSTANT(0x085A4F6FD393D475LL, invalidargumentexception);
      break;
    case 122:
      HASH_GET_CLASS_CONSTANT(0x41A9F7D81254DD7ALL, reflectionobject);
      break;
    case 125:
      HASH_GET_CLASS_CONSTANT(0x7AD161197633B87DLL, domdocumentfragment);
      HASH_GET_CLASS_CONSTANT(0x2E899D2A2572EF7DLL, sqlite3result);
      break;
    case 128:
      HASH_GET_CLASS_CONSTANT(0x59B3EFD1CD987F80LL, domprocessinginstruction);
      break;
    case 130:
      HASH_GET_CLASS_CONSTANT(0x33BD46E935281082LL, reflectionfunctionabstract);
      break;
    case 131:
      HASH_GET_CLASS_CONSTANT(0x24DE53FD7D3E8383LL, datetimezone);
      break;
    case 132:
      HASH_GET_CLASS_CONSTANT(0x70D4A29DE6A7BA84LL, domainexception);
      break;
    case 143:
      HASH_GET_CLASS_CONSTANT(0x34C95AF311506C8FLL, directory);
      break;
    case 144:
      HASH_GET_CLASS_CONSTANT(0x06335FF83CDEA590LL, badmethodcallexception);
      break;
    case 147:
      HASH_GET_CLASS_CONSTANT(0x2E7081C468A05993LL, reflectionparameter);
      break;
    case 148:
      HASH_GET_CLASS_CONSTANT(0x6FFDF8DF15CABE94LL, domcdatasection);
      HASH_GET_CLASS_CONSTANT(0x6576EACCCE24D694LL, __php_incomplete_class);
      HASH_GET_CLASS_CONSTANT(0x47D93E6F80B66A94LL, exception);
      break;
    case 152:
      HASH_GET_CLASS_CONSTANT(0x3CEBA108A1BAB998LL, pear_error);
      break;
    case 167:
      HASH_GET_CLASS_CONSTANT(0x71089C29FE923FA7LL, splfileinfo);
      break;
    case 168:
      HASH_GET_CLASS_CONSTANT(0x7D46DB2E09C6DBA8LL, domxpath);
      break;
    case 169:
      HASH_GET_CLASS_CONSTANT(0x1D35C3EFD00E11A9LL, sqlite3);
      break;
    case 175:
      HASH_GET_CLASS_CONSTANT(0x2E650138BA043AAFLL, domdocument);
      break;
    case 176:
      HASH_GET_CLASS_CONSTANT(0x43BBC8F6F28E44B0LL, reflectionmethod);
      break;
    case 177:
      HASH_GET_CLASS_CONSTANT(0x6DD772FA3B20B8B1LL, domnodelist);
      break;
    case 195:
      HASH_GET_CLASS_CONSTANT(0x40B3C44077F5DDC3LL, domelement);
      break;
    case 200:
      HASH_GET_CLASS_CONSTANT(0x2E363D51549781C8LL, appenditerator);
      break;
    case 203:
      HASH_GET_CLASS_CONSTANT(0x253AC17234FC8CCBLL, libxmlerror);
      break;
    case 210:
      HASH_GET_CLASS_CONSTANT(0x52E5CC58B841AED2LL, sqlite3stmt);
      HASH_GET_CLASS_CONSTANT(0x297ECCC7A259EDD2LL, splfileobject);
      break;
    case 212:
      HASH_GET_CLASS_CONSTANT(0x5E623FEAF7B068D4LL, underflowexception);
      break;
    case 216:
      HASH_GET_CLASS_CONSTANT(0x464D3427431A6ED8LL, recursivedirectoryiterator);
      break;
    case 217:
      HASH_GET_CLASS_CONSTANT(0x0E4CCE95F6A727D9LL, overflowexception);
      break;
    case 218:
      HASH_GET_CLASS_CONSTANT(0x7A52EBE538182BDALL, soapclient);
      break;
    case 220:
      HASH_GET_CLASS_CONSTANT(0x1078CB3118A8B3DCLL, runtimeexception);
      break;
    case 228:
      HASH_GET_CLASS_CONSTANT(0x75AA2571BDB659E4LL, reflectionproperty);
      break;
    case 244:
      HASH_GET_CLASS_CONSTANT(0x297174231D4912F4LL, domattr);
      break;
    case 247:
      HASH_GET_CLASS_CONSTANT(0x3D69936178BA13F7LL, stdclass);
      break;
    case 252:
      HASH_GET_CLASS_CONSTANT(0x622D4DF07E2A01FCLL, outofrangeexception);
      break;
    case 253:
      HASH_GET_CLASS_CONSTANT(0x74419B70A46387FDLL, domimplementation);
      break;
    case 255:
      HASH_GET_CLASS_CONSTANT(0x624835B2D74B86FFLL, domentity);
      break;
    default:
      break;
  }
  return null;
}

///////////////////////////////////////////////////////////////////////////////
}

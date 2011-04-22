/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <runtime/ext/ext_array.h>
#include <runtime/ext/ext_function.h>
#include <runtime/base/util/request_local.h>
#include <runtime/base/zend/zend_collator.h>
#include <util/logger.h>

#define SORT_REGULAR            0
#define SORT_NUMERIC            1
#define SORT_STRING             2
#define SORT_LOCALE_STRING      5

#define SORT_DESC               3
#define SORT_ASC                4

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const int64 k_UCOL_DEFAULT = UCOL_DEFAULT;

const int64 k_UCOL_PRIMARY = UCOL_PRIMARY;
const int64 k_UCOL_SECONDARY = UCOL_SECONDARY;
const int64 k_UCOL_TERTIARY = UCOL_TERTIARY;
const int64 k_UCOL_DEFAULT_STRENGTH = UCOL_DEFAULT_STRENGTH;
const int64 k_UCOL_QUATERNARY = UCOL_QUATERNARY;
const int64 k_UCOL_IDENTICAL = UCOL_IDENTICAL;

const int64 k_UCOL_OFF = UCOL_OFF;
const int64 k_UCOL_ON = UCOL_ON;

const int64 k_UCOL_SHIFTED = UCOL_SHIFTED;
const int64 k_UCOL_NON_IGNORABLE = UCOL_NON_IGNORABLE;

const int64 k_UCOL_LOWER_FIRST = UCOL_LOWER_FIRST;
const int64 k_UCOL_UPPER_FIRST = UCOL_UPPER_FIRST;

const int64 k_UCOL_FRENCH_COLLATION = UCOL_FRENCH_COLLATION;
const int64 k_UCOL_ALTERNATE_HANDLING = UCOL_ALTERNATE_HANDLING;
const int64 k_UCOL_CASE_FIRST = UCOL_CASE_FIRST;
const int64 k_UCOL_CASE_LEVEL = UCOL_CASE_LEVEL;
const int64 k_UCOL_NORMALIZATION_MODE = UCOL_NORMALIZATION_MODE;
const int64 k_UCOL_STRENGTH = UCOL_STRENGTH;
const int64 k_UCOL_HIRAGANA_QUATERNARY_MODE = UCOL_HIRAGANA_QUATERNARY_MODE;
const int64 k_UCOL_NUMERIC_COLLATION = UCOL_NUMERIC_COLLATION;

static bool filter_func(CVarRef value, const void *data) {
  MethodCallPackage *mcp = (MethodCallPackage *)data;
  if (mcp->m_isFunc) {
    if (CallInfo::FuncInvoker1Args invoker = mcp->ci->getFunc1Args()) {
      return invoker(mcp->extra, 1, value);
    }
    return (mcp->ci->getFunc())(mcp->extra, CREATE_VECTOR1(value));
  } else {
    if (CallInfo::MethInvoker1Args invoker = mcp->ci->getMeth1Args()) {
      return invoker(*mcp, 1, value);
    }
    return (mcp->ci->getMeth())(*mcp, CREATE_VECTOR1(value));
  }
}
Variant f_array_filter(CVarRef input, CVarRef callback /* = null_variant */) {
  if (!input.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr_input = input.toArrNR();
  if (callback.isNull()) {
    return ArrayUtil::Filter(arr_input);
  }
  MethodCallPackage mcp;
  String classname, methodname;
  if (!get_user_func_handler(callback, mcp, classname, methodname)) {
    return null;
  }
  return ArrayUtil::Filter(arr_input, filter_func, &mcp);
}

bool f_array_key_exists(CVarRef key, CVarRef search) {
  if (!search.isArray() && !search.isObject()) {
    throw_bad_type_exception("array_key_exists expects an array or an object; "
                             "false returned.");
    return false;
  }

  if (key.isString()) {
    int64 n = 0;
    CStrRef k = key.toStrNR();
    if (k->isStrictlyInteger(n)) {
      return toArray(search).exists(n);
    }
    return toArray(search)->exists(k);
  }
  if (key.isInteger()) {
    return toArray(search).exists(key.toInt64());
  }
  if (key.isNull()) {
    return toArray(search).exists(empty_string);
  }
  raise_warning("Array key should be either a string or an integer");
  return false;
}

static Variant map_func(CArrRef params, const void *data) {
  if (!data) {
    if (params.size() == 1) {
      return params[0];
    }
    return params;
  }
  MethodCallPackage *mcp = (MethodCallPackage *)data;
  if (mcp->m_isFunc) {
    return (mcp->ci->getFunc())(mcp->extra, params);
  } else {
    return (mcp->ci->getMeth())(*mcp, params);
  }
}
Variant f_array_map(int _argc, CVarRef callback, CVarRef arr1, CArrRef _argv /* = null_array */) {
  Array inputs;
  if (!arr1.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  inputs.append(arr1);
  if (!_argv.empty()) {
    inputs = inputs.merge(_argv);
  }
  MethodCallPackage mcp;
  String classname, methodname;
  if (!get_user_func_handler(callback, mcp, classname, methodname)) {
    return ArrayUtil::Map(inputs, map_func, NULL);
  }
  return ArrayUtil::Map(inputs, map_func, &mcp);
}

static void php_array_merge(Array &arr1, CArrRef arr2) {
  arr1.merge(arr2);
}

static void php_array_merge_recursive(PointerSet &seen, bool check,
                                      Array &arr1, CArrRef arr2) {
  if (check) {
    if (seen.find((void*)arr1.get()) != seen.end()) {
      raise_warning("array_merge_recursive(): recursion detected");
      return;
    }
    seen.insert((void*)arr1.get());
  }

  for (ArrayIter iter(arr2); iter; ++iter) {
    Variant key(iter.first());
    CVarRef value(iter.secondRef());
    if (key.isNumeric()) {
      arr1.appendWithRef(value);
    } else if (arr1.exists(key, true)) {
      // There is no need to do toKey() conversion, for a key that is already
      // in the array.
      Variant &v = arr1.lvalAt(key, AccessFlags::Key);
      Array subarr1(v.toArray()->copy());
      php_array_merge_recursive(seen, v.isReferenced(), subarr1,
                                value.toArray());
      v.unset(); // avoid contamination of the value that was strongly bound
      v = subarr1;
    } else {
      arr1.addLval(key, true).setWithRef(value);
    }
  }

  if (check) {
    seen.erase((void*)arr1.get());
  }
}

Variant f_array_merge(int _argc, CVarRef array1,
                      CArrRef _argv /* = null_array */) {
  if (!array1.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr_array1 = array1.toArrNR();
  Array ret = Array::Create();
  php_array_merge(ret, arr_array1);
  for (ArrayIter iter(_argv); iter; ++iter) {
    Variant v = iter.second();
    if (!v.isArray()) {
      throw_bad_array_exception();
      return null;
    }
    CArrRef arr_v = v.toArrNR();
    php_array_merge(ret, arr_v);
  }
  return ret;
}

Variant f_array_merge_recursive(int _argc, CVarRef array1,
                                CArrRef _argv /* = null_array */) {
  if (!array1.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr_array1 = array1.toArrNR();
  Array ret = Array::Create();
  PointerSet seen;
  php_array_merge_recursive(seen, false, ret, arr_array1);
  ASSERT(seen.empty());
  for (ArrayIter iter(_argv); iter; ++iter) {
    Variant v = iter.second();
    if (!v.isArray()) {
      throw_bad_array_exception();
      return null;
    }
    CArrRef arr_v = v.toArrNR();
    php_array_merge_recursive(seen, false, ret, arr_v);
    ASSERT(seen.empty());
  }
  return ret;
}

static void php_array_replace(Array &arr1, CArrRef arr2) {
  for (ArrayIter iter(arr2); iter; ++iter) {
    Variant key = iter.first();
    CVarRef value = iter.secondRef();
    arr1.lvalAt(key, AccessFlags::Key).setWithRef(value);
  }
}

static void php_array_replace_recursive(PointerSet &seen, bool check,
                                        Array &arr1, CArrRef arr2) {
  if (check) {
    if (seen.find((void*)arr1.get()) != seen.end()) {
      raise_warning("array_replace_recursive(): recursion detected");
      return;
    }
    seen.insert((void*)arr1.get());
  }

  for (ArrayIter iter(arr2); iter; ++iter) {
    Variant key = iter.first();
    CVarRef value = iter.secondRef();
    if (arr1.exists(key, true) && value.isArray()) {
      Variant &v = arr1.lvalAt(key, AccessFlags::Key);
      if (v.isArray()) {
        Array subarr1 = v.toArray();
        CArrRef arr_value = value.toArrNR();
        php_array_replace_recursive(seen, v.isReferenced(), subarr1,
                                    arr_value);
        v = subarr1;
      } else {
        arr1.set(key, value, true);
      }
    } else {
      arr1.lvalAt(key, AccessFlags::Key).setWithRef(value);
    }
  }

  if (check) {
    seen.erase((void*)arr1.get());
  }
}

Variant f_array_replace(int _argc, CVarRef array1,
                        CArrRef _argv /* = null_array */) {
  if (!array1.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr_array1 = array1.toArrNR();
  Array ret = Array::Create();
  php_array_replace(ret, arr_array1);
  for (ArrayIter iter(_argv); iter; ++iter) {
    CVarRef v = iter.secondRef();
    if (!v.isArray()) {
      throw_bad_array_exception();
      return null;
    }
    CArrRef arr_v = v.toArrNR();
    php_array_replace(ret, arr_v);
  }
  return ret;
}

Variant f_array_replace_recursive(int _argc, CVarRef array1,
                                  CArrRef _argv /* = null_array */) {
  if (!array1.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr_array1 = array1.toArrNR();
  Array ret = Array::Create();
  PointerSet seen;
  php_array_replace_recursive(seen, false, ret, arr_array1);
  ASSERT(seen.empty());
  for (ArrayIter iter(_argv); iter; ++iter) {
    CVarRef v = iter.secondRef();
    if (!v.isArray()) {
      throw_bad_array_exception();
      return null;
    }
    CArrRef arr_v = v.toArrNR();
    php_array_replace_recursive(seen, false, ret, arr_v);
    ASSERT(seen.empty());
  }
  return ret;
}

Variant f_array_push(int _argc, VRefParam array, CVarRef var, CArrRef _argv /* = null_array */) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  array.append(var);
  for (ArrayIter iter(_argv); iter; ++iter) {
    array.append(iter.second());
  }
  return array.toArrNR().size();
}

static Variant reduce_func(CVarRef result, CVarRef operand, const void *data) {
  MethodCallPackage *mcp = (MethodCallPackage *)data;
  if (mcp->m_isFunc) {
    if (CallInfo::FuncInvoker2Args invoker = mcp->ci->getFunc2Args()) {
      return invoker(mcp->extra, 2, result, operand);
    }
    return (mcp->ci->getFunc())(mcp->extra, CREATE_VECTOR2(result, operand));
  } else {
    if (CallInfo::MethInvoker2Args invoker = mcp->ci->getMeth2Args()) {
      return invoker(*mcp, 2, result, operand);
    }
    return (mcp->ci->getMeth())(*mcp, CREATE_VECTOR2(result, operand));
  }
}
Variant f_array_reduce(CVarRef input, CVarRef callback,
                       CVarRef initial /* = null_variant */) {
  if (!input.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  CArrRef arr_input = input.toArrNR();
  MethodCallPackage mcp;
  String classname, methodname;
  if (!get_user_func_handler(callback, mcp, classname, methodname)) {
    return null;
  }
  return ArrayUtil::Reduce(arr_input, reduce_func, &mcp, initial);
}

int f_array_unshift(int _argc, VRefParam array, CVarRef var, CArrRef _argv /* = null_array */) {
  if (array.toArray()->isVectorData()) {
    if (!_argv.empty()) {
      for (ssize_t pos = _argv->iter_end(); pos != ArrayData::invalid_index;
        pos = _argv->iter_rewind(pos)) {
        array.prepend(_argv->getValueRef(pos));
      }
    }
    array.prepend(var);
  } else {
    {
      Array newArray;
      newArray.append(var);
      if (!_argv.empty()) {
        for (ssize_t pos = _argv->iter_begin();
             pos != ArrayData::invalid_index;
             pos = _argv->iter_advance(pos)) {
          newArray.append(_argv->getValueRef(pos));
        }
      }
      for (ArrayIter iter(array); iter; ++iter) {
        Variant key(iter.first());
        CVarRef value(iter.secondRef());
        if (key.isInteger()) {
          newArray.appendWithRef(value);
        } else {
          newArray.lvalAt(key, AccessFlags::Key).setWithRef(value);
        }
      }
      array = newArray;
    }
    // Reset the array's internal pointer
    if (array.is(KindOfArray)) {
      array.array_iter_reset();
    }
  }
  return array.toArray().size();
}

static void walk_func(VRefParam value, CVarRef key, CVarRef userdata,
                      const void *data) {
  MethodCallPackage *mcp = (MethodCallPackage *)data;
  // Here to avoid crash in interpreter, we need to use different variation
  // in 'FewArgs' cases
  if (mcp->m_isFunc) {
    if (mcp->ci->getFuncFewArgs()) { // To test whether we have FewArgs
      if (userdata.isNull()) {
        (mcp->ci->getFunc2Args())(mcp->extra, 2, value, key);
      } else{
        (mcp->ci->getFunc3Args())(mcp->extra, 3, value, key, userdata);
      }
      return;
    }
    (mcp->ci->getFunc())(mcp->extra, CREATE_VECTOR3(ref(value), key,
                         userdata));
  } else {
    if (mcp->ci->getMethFewArgs()) { // To test whether we have FewArgs
      if (userdata.isNull()) {
        (mcp->ci->getMeth2Args())(*mcp, 2, value, key);
      } else {
        (mcp->ci->getMeth3Args())(*mcp, 3, value, key, userdata);
      }
      return;
    }
    (mcp->ci->getMeth())(*mcp, CREATE_VECTOR3(ref(value), key,
                         userdata));
  }
}
bool f_array_walk_recursive(VRefParam input, CVarRef funcname,
                            CVarRef userdata /* = null_variant */) {
  if (!input.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  MethodCallPackage mcp;
  String classname, methodname;
  if (!get_user_func_handler(funcname, mcp, classname, methodname)) {
    return null;
  }
  PointerSet seen;
  ArrayUtil::Walk(input, walk_func, &mcp, true, &seen, userdata);
  return true;
}
bool f_array_walk(VRefParam input, CVarRef funcname,
                  CVarRef userdata /* = null_variant */) {
  if (!input.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  MethodCallPackage mcp;
  String classname, methodname;
  if (!get_user_func_handler(funcname, mcp, classname, methodname)) {
    return null;
  }
  ArrayUtil::Walk(ref(input), walk_func, &mcp, false, NULL, userdata);
  return true;
}

Array f_compact(int _argc, CVarRef varname, CArrRef _argv /* = null_array */) {
  throw FatalErrorException("bad HPHP code generation");
}

template<typename T>
static void compact(T *variables, Array &ret, CVarRef var) {
  if (var.isArray()) {
    CArrRef vars = var.toArrNR();
    for (ArrayIter iter(vars); iter; ++iter) {
      compact(variables, ret, iter.second());
    }
  } else {
    String varname = var.toString();
    if (!varname.empty() && variables->exists(varname)) {
      ret.set(varname, variables->get(varname));
    }
  }
}

Array compact(RVariableTable *variables, int _argc, CVarRef varname,
              CArrRef _argv /* = null_array */) {
  FUNCTION_INJECTION_BUILTIN(compact);
  Array ret = Array::Create();
  compact(variables, ret, varname);
  compact(variables, ret, _argv);
  return ret;
}

Array compact(LVariableTable *variables, int _argc, CVarRef varname,
              CArrRef _argv /* = null_array */) {
  FUNCTION_INJECTION_BUILTIN(compact);
  Array ret = Array::Create();
  compact(variables, ret, varname);
  compact(variables, ret, _argv);
  return ret;
}

static int php_count_recursive(CArrRef array) {
  long cnt = array.size();
  for (ArrayIter iter(array); iter; ++iter) {
    Variant value = iter.second();
    if (value.isArray()) {
      CArrRef arr_value = value.toArrNR();
      cnt += php_count_recursive(arr_value);
    }
  }
  return cnt;
}

int f_count(CVarRef var, bool recursive /* = false */) {
  switch (var.getType()) {
  case KindOfUninit:
  case KindOfNull:
    return 0;
  case KindOfObject:
    {
      Object obj = var.toObject();
      if (obj.instanceof("Countable")) {
        return obj->o_invoke("count", null_array, -1);
      }
    }
    break;
  case KindOfArray:
    if (recursive) {
      CArrRef arr_var = var.toArrNR();
      return php_count_recursive(arr_var);
    }
    return var.getArrayData()->size();
  default:
    break;
  }
  return 1;
}

Variant f_hphp_get_iterator(VRefParam iterable, bool isMutable) {
  if (iterable.isArray()) {
    if (isMutable) {
      return create_object("MutableArrayIterator",
                           CREATE_VECTOR1(ref(iterable)));
    }
    return create_object("ArrayIterator", CREATE_VECTOR1(iterable));
  }
  if (iterable.isObject()) {
    CStrRef context = FrameInjection::GetClassName(true);

    ObjectData *obj = iterable.getObjectData();
    if (isMutable) {
      while (obj->o_instanceof("IteratorAggregate")) {
        Variant iterator = obj->o_invoke("getiterator", Array());
        if (!iterator.isObject()) break;
        obj = iterator.getObjectData();
      }
      if (obj->o_instanceof("Iterator")) {
        throw FatalErrorException("An iterator cannot be used for "
                                  "iteration by reference");
      }
      Array properties = obj->o_toIterArray(context, true);
      return create_object("MutableArrayIterator",
                           CREATE_VECTOR1(ref(properties)));
    }

    while (obj->o_instanceof("IteratorAggregate")) {
      Variant iterator = obj->o_invoke("getiterator", Array());
      if (!iterator.isObject()) break;
      obj = iterator.getObjectData();
    }
    if (obj->o_instanceof("Iterator")) {
      // Queue up any continuations to the first element
      if (obj->o_instanceof("Continuation")) {
        obj->o_invoke("next", Array());
      }
      return obj;
    }

    return create_object("ArrayIterator",
                         CREATE_VECTOR1(obj->o_toIterArray(context)));
  }
  raise_warning("Invalid argument supplied for iteration");
  return null;
}

Variant f_range(CVarRef low, CVarRef high, CVarRef step /* = 1 */) {
  bool is_step_double = false;
  double dstep = 1.0;
  if (step.isDouble()) {
    dstep = step.toDouble();
    is_step_double = true;
  } else if (step.isString()) {
    int64 sn;
    double sd;
    DataType stype = step.toString()->isNumericWithVal(sn, sd, 0);
    if (stype == KindOfDouble) {
      is_step_double = true;
      dstep = sd;
    } else if (stype == KindOfInt64) {
      dstep = (double)sn;
    } else {
      dstep = step.toDouble();
    }
  } else {
    dstep = step.toDouble();
  }
  /* We only want positive step values. */
  if (dstep < 0.0) dstep *= -1;
  if (low.isString() && high.isString()) {
    String slow = low.toString();
    String shigh = high.toString();
    if (slow.size() >= 1 && shigh.size() >=1) {
      int64 n1, n2;
      double d1, d2;
      DataType type1 = slow->isNumericWithVal(n1, d1, 0);
      DataType type2 = shigh->isNumericWithVal(n2, d2, 0);
      if (type1 == KindOfDouble || type2 == KindOfDouble || is_step_double) {
        if (type1 != KindOfDouble) d1 = slow.toDouble();
        if (type2 != KindOfDouble) d2 = shigh.toDouble();
        return ArrayUtil::Range(d1, d2, dstep);
      }

      int64 lstep = (int64) dstep;
      if (type1 == KindOfInt64 || type2 == KindOfInt64) {
        if (type1 != KindOfInt64) n1 = slow.toInt64();
        if (type2 != KindOfInt64) n2 = shigh.toInt64();
        return ArrayUtil::Range((double)n1, (double)n2, lstep);
      }

      return ArrayUtil::Range((unsigned char)slow.charAt(0),
                              (unsigned char)shigh.charAt(0), lstep);
    }
  }

  if (low.is(KindOfDouble) || high.is(KindOfDouble) || is_step_double) {
    return ArrayUtil::Range(low.toDouble(), high.toDouble(), dstep);
  }

  int64 lstep = (int64) dstep;
  return ArrayUtil::Range(low.toDouble(), high.toDouble(), lstep);
}
///////////////////////////////////////////////////////////////////////////////
// diff/intersect helpers

static int cmp_func(CVarRef v1, CVarRef v2, const void *data) {
  Variant *callback = (Variant *)data;
  return f_call_user_func_array(*callback, CREATE_VECTOR2(v1, v2));
}

#define COMMA ,
#define diff_intersect_body(type,intersect_params,user_setup)   \
  if (!array1.isArray()) {                                      \
    throw_bad_array_exception();                                \
    return null;                                                \
  }                                                             \
  Array ret = array1.getArrayData();                            \
  if (ret.size()) {                                             \
    user_setup                                                  \
    ret = ret.type(array2, intersect_params);                   \
    if (ret.size()) {                                           \
      for (ArrayIter iter(_argv); iter; ++iter) {               \
        ret = ret.type(iter.second(), intersect_params);        \
        if (!ret.size()) break;                                 \
      }                                                         \
    }                                                           \
  }                                                             \
  return ret;

///////////////////////////////////////////////////////////////////////////////
// diff functions

Variant f_array_diff(int _argc, CVarRef array1, CVarRef array2,
                   CArrRef _argv /* = null_array */) {
  diff_intersect_body(diff, false COMMA true,);
}

Variant f_array_udiff(int _argc, CVarRef array1, CVarRef array2,
                      CVarRef data_compare_func,
                      CArrRef _argv /* = null_array */) {
  diff_intersect_body(diff, false COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_diff_assoc(int _argc, CVarRef array1, CVarRef array2,
                           CArrRef _argv /* = null_array */) {
  diff_intersect_body(diff, true COMMA true,);
}

Variant f_array_diff_uassoc(int _argc, CVarRef array1, CVarRef array2,
                            CVarRef key_compare_func,
                            CArrRef _argv /* = null_array */) {
  diff_intersect_body(diff, true COMMA true COMMA cmp_func COMMA &func,
                      Variant func = key_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_udiff_assoc(int _argc, CVarRef array1, CVarRef array2,
                            CVarRef data_compare_func,
                            CArrRef _argv /* = null_array */) {
  diff_intersect_body(diff, true COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_udiff_uassoc(int _argc, CVarRef array1, CVarRef array2,
                             CVarRef data_compare_func,
                             CVarRef key_compare_func,
                             CArrRef _argv /* = null_array */) {
  diff_intersect_body(diff, true COMMA true COMMA cmp_func COMMA &key_func
                      COMMA cmp_func COMMA &data_func,
                      Variant data_func = data_compare_func;
                      Variant key_func = key_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(key_func);
                        extra.prepend(data_func);
                        key_func = extra.pop();
                        data_func = extra.pop();
                      });
}

Variant f_array_diff_key(int _argc, CVarRef array1, CVarRef array2,
                         CArrRef _argv /* = null_array */) {
  diff_intersect_body(diff, true COMMA false,);
}

Variant f_array_diff_ukey(int _argc, CVarRef array1, CVarRef array2,
                          CVarRef key_compare_func,
                          CArrRef _argv /* = null_array */) {
  diff_intersect_body(diff, true COMMA false COMMA cmp_func COMMA &func,
                      Variant func = key_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

///////////////////////////////////////////////////////////////////////////////
// intersect functions

Variant f_array_intersect(int _argc, CVarRef array1, CVarRef array2,
                          CArrRef _argv /* = null_array */) {
  diff_intersect_body(intersect, false COMMA true,);
}

Variant f_array_uintersect(int _argc, CVarRef array1, CVarRef array2,
                           CVarRef data_compare_func,
                           CArrRef _argv /* = null_array */) {
  diff_intersect_body(intersect, false COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });

  return ret;
}

Variant f_array_intersect_assoc(int _argc, CVarRef array1, CVarRef array2,
                                CArrRef _argv /* = null_array */) {
  diff_intersect_body(intersect, true COMMA true,);
}

Variant f_array_intersect_uassoc(int _argc, CVarRef array1, CVarRef array2,
                                 CVarRef key_compare_func,
                                 CArrRef _argv /* = null_array */) {
  diff_intersect_body(intersect, true COMMA true COMMA cmp_func COMMA &func,
                      Variant func = key_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_uintersect_assoc(int _argc, CVarRef array1, CVarRef array2,
                                 CVarRef data_compare_func,
                                 CArrRef _argv /* = null_array */) {
  diff_intersect_body(intersect, true COMMA true COMMA NULL COMMA NULL
                      COMMA cmp_func COMMA &func,
                      Variant func = data_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

Variant f_array_uintersect_uassoc(int _argc, CVarRef array1, CVarRef array2,
                                  CVarRef data_compare_func,
                                  CVarRef key_compare_func,
                                  CArrRef _argv /* = null_array */) {
  diff_intersect_body(intersect, true COMMA true COMMA cmp_func COMMA &key_func
                      COMMA cmp_func COMMA &data_func,
                      Variant data_func = data_compare_func;
                      Variant key_func = key_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(key_func);
                        extra.prepend(data_func);
                        key_func = extra.pop();
                        data_func = extra.pop();
                      });
}

Variant f_array_intersect_key(int _argc, CVarRef array1, CVarRef array2, CArrRef _argv /* = null_array */) {
  diff_intersect_body(intersect, true COMMA false,);
}

Variant f_array_intersect_ukey(int _argc, CVarRef array1, CVarRef array2,
                             CVarRef key_compare_func, CArrRef _argv /* = null_array */) {
  diff_intersect_body(intersect, true COMMA false COMMA cmp_func COMMA &func,
                      Variant func = key_compare_func;
                      Array extra = _argv;
                      if (!extra.empty()) {
                        extra.prepend(func);
                        func = extra.pop();
                      });
}

///////////////////////////////////////////////////////////////////////////////
// sorting functions

class Collator : public RequestEventHandler {
public:
  String getLocale() {
    return m_locale;
  }
  intl_error &getErrorCodeRef() {
    return m_errcode;
  }
  bool setLocale(CStrRef locale) {
    if (m_locale.same(locale)) {
      return true;
    }
    if (m_ucoll) {
      ucol_close(m_ucoll);
      m_ucoll = NULL;
    }
    m_errcode.clear();
    m_ucoll = ucol_open(locale.data(), &(m_errcode.code));
    if (m_ucoll == NULL) {
      raise_warning("failed to load %s locale from icu data", locale.data());
      return false;
    }
    if (U_FAILURE(m_errcode.code)) {
      ucol_close(m_ucoll);
      m_ucoll = NULL;
      return false;
    }
    m_locale = locale;
    return true;
  }

  UCollator *getCollator() {
    return m_ucoll;
  }

  bool setAttribute(int64 attr, int64 val) {
    if (!m_ucoll) {
      Logger::Verbose("m_ucoll is NULL");
      return false;
    }
    m_errcode.clear();
    ucol_setAttribute(m_ucoll, (UColAttribute)attr,
                      (UColAttributeValue)val, &(m_errcode.code));
    if (U_FAILURE(m_errcode.code)) {
      Logger::Verbose("Error setting attribute value");
      return false;
    }
    return true;
  }

  bool setStrength(int64 strength) {
    if (!m_ucoll) {
      Logger::Verbose("m_ucoll is NULL");
      return false;
    }
    ucol_setStrength(m_ucoll, (UCollationStrength)strength);
    return true;
  }

  Variant getErrorCode() {
    if (!m_ucoll) {
      Logger::Verbose("m_ucoll is NULL");
      return false;
    }
    return m_errcode.code;
  }

  virtual void requestInit() {
    m_locale = String(uloc_getDefault(), CopyString);
    m_errcode.clear();
    m_ucoll = ucol_open(m_locale.data(), &(m_errcode.code));
    ASSERT(m_ucoll);
  }
  virtual void requestShutdown() {
    m_locale.reset();
    m_errcode.clear();
    if (m_ucoll) {
      ucol_close(m_ucoll);
      m_ucoll = NULL;
    }
  }

private:
  String     m_locale;
  UCollator *m_ucoll;
  intl_error m_errcode;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(Collator, s_collator);

static Array::PFUNC_CMP get_cmp_func(int sort_flags, bool ascending) {
  switch (sort_flags) {
  case SORT_NUMERIC:
    return ascending ?
      Array::SortNumericAscending : Array::SortNumericDescending;
  case SORT_STRING:
    return ascending ?
      Array::SortStringAscending : Array::SortStringDescending;
  case SORT_LOCALE_STRING:
    return ascending ?
      Array::SortLocaleStringAscending : Array::SortLocaleStringDescending;
  case SORT_REGULAR:
  default:
    return ascending ?
      Array::SortRegularAscending : Array::SortRegularDescending;
  }
}

bool f_sort(VRefParam array, int sort_flags /* = 0 */,
            bool use_collator /* = false */) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  if (use_collator && sort_flags != SORT_LOCALE_STRING) {
    UCollator *coll = s_collator->getCollator();
    if (coll) {
      intl_error &errcode = s_collator->getErrorCodeRef();
      return collator_sort(array, sort_flags, true, coll, &errcode);
    }
  }
  Array temp = array.toArray();
  temp.sort(get_cmp_func(sort_flags, true), false, true);
  array = temp;
  return true;
}

bool f_rsort(VRefParam array, int sort_flags /* = 0 */,
             bool use_collator /* = false */) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  if (use_collator && sort_flags != SORT_LOCALE_STRING) {
    UCollator *coll = s_collator->getCollator();
    if (coll) {
      intl_error &errcode = s_collator->getErrorCodeRef();
      return collator_sort(array, sort_flags, false, coll, &errcode);
    }
  }
  Array temp = array.toArray();
  temp.sort(get_cmp_func(sort_flags, false), false, true);
  array = temp;
  return true;
}

bool f_asort(VRefParam array, int sort_flags /* = 0 */,
             bool use_collator /* = false */) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  if (use_collator && sort_flags != SORT_LOCALE_STRING) {
    UCollator *coll = s_collator->getCollator();
    if (coll) {
      intl_error &errcode = s_collator->getErrorCodeRef();
      return collator_asort(array, sort_flags, true, coll, &errcode);
    }
  }
  Array temp = array.toArray();
  temp.sort(get_cmp_func(sort_flags, true), false, false);
  array = temp;
  return true;
}

bool f_arsort(VRefParam array, int sort_flags /* = 0 */,
              bool use_collator /* = false */) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  if (use_collator && sort_flags != SORT_LOCALE_STRING) {
    UCollator *coll = s_collator->getCollator();
    if (coll) {
      intl_error &errcode = s_collator->getErrorCodeRef();
      return collator_asort(array, sort_flags, false, coll, &errcode);
    }
  }
  Array temp = array.toArray();
  temp.sort(get_cmp_func(sort_flags, false), false, false);
  array = temp;
  return true;
}

bool f_ksort(VRefParam array, int sort_flags /* = 0 */) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  Array temp = array.toArray();
  temp.sort(get_cmp_func(sort_flags, true), true, false);
  array = temp;
  return true;
}

bool f_krsort(VRefParam array, int sort_flags /* = 0 */) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  Array temp = array.toArray();
  temp.sort(get_cmp_func(sort_flags, false), true, false);
  array = temp;
  return true;
}

bool f_usort(VRefParam array, CVarRef cmp_function) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  Array temp = array.toArray();
  temp.sort(cmp_func, false, true, &cmp_function);
  array = temp;
  return true;
}

bool f_uasort(VRefParam array, CVarRef cmp_function) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  Array temp = array.toArray();
  temp.sort(cmp_func, false, false, &cmp_function);
  array = temp;
  return true;
}

bool f_uksort(VRefParam array, CVarRef cmp_function) {
  if (!array.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  Array temp = array.toArray();
  temp.sort(cmp_func, true, false, &cmp_function);
  array = temp;
  return true;
}

Variant f_natsort(VRefParam array) {
  // NOTE, PHP natsort accepts ArrayAccess objects as well,
  // which does not make much sense, and which is not supported here.
  if (!array.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  Array temp = array.toArray();
  temp.sort(Array::SortNatural, false, false);
  array = temp;
  return true;
}

Variant f_natcasesort(VRefParam array) {
  // NOTE, PHP natcasesort accepts ArrayAccess objects as well,
  // which does not make much sense, and which is not supported here.
  if (!array.isArray()) {
    throw_bad_array_exception();
    return null;
  }
  Array temp = array.toArray();
  temp.sort(Array::SortNaturalCase, false, false);
  array = temp;
  return true;
}

bool f_array_multisort(int _argc, VRefParam ar1,
                       CArrRef _argv /* = null_array */) {
  if (!ar1.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  std::vector<Array::SortData> data;
  std::vector<Array> arrays;
  arrays.reserve(1 + _argv.size()); // so no resize would happen

  Array::SortData sd;
  sd.original = &ar1;
  arrays.push_back(ar1.toArray());
  sd.array = &arrays.back();
  sd.by_key = false;

  int sort_flags = SORT_REGULAR;
  bool ascending = true;
  for (int i = 0; i < _argv.size(); i++) {
    Variant *v = &((Array&)_argv).lvalAt(i);
    if (v->isArray()) {
      sd.cmp_func = get_cmp_func(sort_flags, ascending);
      data.push_back(sd);

      sort_flags = SORT_REGULAR;
      ascending = true;

      sd.original = v;
      arrays.push_back(sd.original->toArray());
      sd.array = &arrays.back();
    } else {
      int n = v->toInt32();
      if (n == SORT_ASC) {
        ascending = true;
      } else if (n == SORT_DESC) {
        ascending = false;
      } else {
        sort_flags = n;
      }
    }
  }

  sd.cmp_func = get_cmp_func(sort_flags, ascending);
  data.push_back(sd);

  return Array::MultiSort(data, true);
}

Variant f_array_unique(CVarRef array, int sort_flags /* = 2 */) {
  // NOTE, PHP array_unique accepts ArrayAccess objects as well,
  // which is not supported here.
  if (!array.isArray()) {
    throw_bad_array_exception();
    return false;
  }
  CArrRef input = array.toArrNR();
  switch (sort_flags) {
  case SORT_STRING:
  case SORT_LOCALE_STRING:
    return ArrayUtil::StringUnique(input);
  case SORT_NUMERIC:
    return ArrayUtil::NumericUnique(input);
  case SORT_REGULAR:
  default:
    return ArrayUtil::RegularSortUnique(input);
  }
}

String f_i18n_loc_get_default() {
  return s_collator->getLocale();
}

bool f_i18n_loc_set_default(CStrRef locale) {
  return s_collator->setLocale(locale);
}

bool f_i18n_loc_set_attribute(int64 attr, int64 val) {
  return s_collator->setAttribute(attr, val);
}

bool f_i18n_loc_set_strength(int64 strength) {
  return s_collator->setStrength(strength);
}

Variant f_i18n_loc_get_error_code() {
  return s_collator->getErrorCode();
}

///////////////////////////////////////////////////////////////////////////////
}

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/actrec-args.h"

#include "hphp/util/exception.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

static void countArgs(const char *format, unsigned &min, unsigned &max) {
  bool required = true;
  min = max = 0;
  while (auto c = *(format++)) {
    if (c == '|') {
      required = false;
      continue;
    }
    if (c == '!') {
      continue;
    }
    if (required) min++;
    max++;
  }
}

static const char *argTypeName(DataType dt) {
  switch (dt) {
    case KindOfNull:          return "null";
    case KindOfBoolean:       return "boolean";
    case KindOfInt64:         return "integer";
    case KindOfDouble:        return "double";
    case KindOfPersistentString:
    case KindOfString:        return "string";
    case KindOfPersistentArray:
    case KindOfArray:         return "array";
    case KindOfObject:        return "object";
    case KindOfResource:      return "resource";

    case KindOfUninit:
    case KindOfRef:
    case KindOfClass:         return "unknown";
  }
  not_reached();
}

template <DataType DType, class T>
void parseArgValue(TypedValue *tv,
                   va_list va, bool check_null) {
  T* pval = va_arg(va, T*);
  if (check_null) {
     *va_arg(va, bool*) = (tv->m_type == KindOfNull);
  }
  if (!tvCoerceParamInPlace(tv, DType)) {
    throw_invalid_argument("Expected %s, got %s",
                           argTypeName(DType),
                           argTypeName(tv->m_type));
    tvCastInPlace(tv, DType);
  }
  *pval = unpack_tv<DType>(tv);
}

template <DataType DType, class T>
bool parseArgPointer(TypedValue *tv,
                     va_list va, bool check_null) {
  T* pval = va_arg(va, T*);
  if (check_null && (tv->m_type == KindOfNull)) {
    *pval = nullptr;
    return true;
  }
  if (tv->m_type != DType) {
    throw_invalid_argument("Expected %s, got %s",
                           argTypeName(DType),
                           argTypeName(tv->m_type));
    return false;
  }
  *pval = unpack_tv<DType>(tv);
  return true;
}

#define PARSE_ARG_VAL(fmt, dt) \
  case fmt: \
    parseArgValue<dt, typename DataTypeCPPType<dt>::type> \
      (tv, va, check_null); break;

#define PARSE_ARG_PTR(fmt, dt) \
  case fmt: \
    if (!parseArgPointer<dt, typename DataTypeCPPType<dt>::type> \
      (tv, va, check_null)) { return false; } break;

bool parseArgs(ActRec *ar, const char *format, ...) {
  unsigned min, max, count = ar->numArgs();
  countArgs(format, min, max);
  if (count < min || max < count) {
    throw_wrong_arguments_nr(Native::getInvokeName(ar)->data(),
                             count, min, max);
    return false;
  }

  unsigned arg = 0;
  va_list va;
  va_start(va, format);
  SCOPE_EXIT { va_end(va); };

  while (auto c = *(format++)) {
    if (c == '|' || c == '!') {
      continue;
    }

    if (arg >= count) {
      // Still have format specs, but no more args passed
      // throw_wrong_arguments_nr check should guarantee
      // that we're already past min args
      assert(arg >= min);
      break;
    }

    bool check_null = (format[0] == '!');
    TypedValue *tv = getArg(ar, arg++);

    switch (c) {
      PARSE_ARG_VAL('b', KindOfBoolean);
      PARSE_ARG_VAL('l', KindOfInt64);
      PARSE_ARG_VAL('d', KindOfDouble);
      PARSE_ARG_PTR('r', KindOfResource);
      PARSE_ARG_PTR('a', KindOfArray);
      PARSE_ARG_PTR('o', KindOfObject);

      case 's': { // KindOfString
        StringData **psval = va_arg(va, StringData**);
        if (check_null && (tv->m_type == KindOfNull)) {
          *psval = nullptr;
          break;
        }
        if (!tvCoerceParamInPlace(tv, KindOfString)) {
          throw_invalid_argument("Expected string, got %s",
                                 argTypeName(tv->m_type));
          return false;
        }
        *psval = unpack_tv<KindOfString>(tv);
        break;
      }

      case 'O': { // KindOfObject (specific type)
        ObjectData **objval = va_arg(va, ObjectData**);
        Class *expClass = va_arg(va, Class*);
        if (check_null && (tv->m_type == KindOfNull)) {
          *objval = nullptr;
          break;
        }
        if (tv->m_type != KindOfObject) {
          throw_invalid_argument("Expected %s, got %s",
                                 expClass->name()->data(),
                                 argTypeName(tv->m_type));
          return false;
        }
        auto odata = unpack_tv<KindOfObject>(tv);
        Class *cls = odata->getVMClass();
        if ((cls != expClass) && !cls->classof(expClass)) {
          throw_invalid_argument("Expected %s, got %s",
                                 expClass->name()->data(),
                                 cls->name()->data());
          return false;
        }
        *objval = odata;
        break;
      }

      case 'C': { // KindOfClass
        Class **clsval = va_arg(va, Class**);
        if (check_null && (tv->m_type == KindOfNull)) {
          *clsval = nullptr;
          break;
        }
        if (!tvCoerceParamInPlace(tv, KindOfString)) {
          throw_invalid_argument("Expected string class name, got %s",
                                 argTypeName(tv->m_type));
          return false;
        }
        auto cls = Unit::loadClass(tv->m_data.pstr);
        if (!cls) {
          throw_invalid_argument("Unknown class %s",
                                 tv->m_data.pstr->data());
          return false;
        }
        *clsval = cls;
        break;
      }

      case 'A': // KindOfArray || KindOfObject
        if (!isArrayType(tv->m_type) && tv->m_type != KindOfObject) {
          throw_invalid_argument("Expected array or object, got %s",
                                 argTypeName(tv->m_type));
          return false;
        }
        /* fallthrough */
      case 'v': // Variant
        *va_arg(va, Variant*) = tv ? tvAsVariant(tv) : uninit_null();
        break;

      case 'V': // TypedValue*
        *va_arg(va, TypedValue**) = tv;

      default:
        not_reached();
    }
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

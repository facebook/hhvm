/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef HPHP_IDL_H
#define HPHP_IDL_H

#include "folly/Conv.h"
#include "folly/DynamicConverter.h"
#include "folly/FBString.h"
#include "folly/FBVector.h"

#include "hphp/runtime/base/datatype.h"

using folly::fbstring;
using folly::fbvector;

namespace HPHP { namespace IDL {
/////////////////////////////////////////////////////////////////////////////

enum FuncFlags {
  IsAbstract                    = (1 <<  4),
  IsFinal                       = (1 <<  5),
  IsPublic                      = (1 <<  6),
  IsProtected                   = (1 <<  7),
  IsPrivate                     = (1 <<  8),
  IgnoreRedefinition            = (1 <<  8),
  IsStatic                      = (1 <<  9),
  IsCppAbstract                 = (1 << 10),
  IsReference                   = (1 << 11),
  IsConstructor                 = (1 << 12),
  IsNothing                     = (1 << 13),
  HasDocComment                 = (1 << 14),
  HipHopSpecific                = (1 << 16),
  VariableArguments             = (1 << 17),
  RefVariableArguments          = (1 << 18),
  MixedVariableArguments        = (1 << 19),
  FunctionIsFoldable            = (1 << 20),
  NoEffect                      = (1 << 21),
  NoInjection                   = (1 << 22),
  HasOptFunction                = (1 << 23),
  AllowIntercept                = (1 << 24),
  NoProfile                     = (1 << 25),
  ContextSensitive              = (1 << 26),
  NoDefaultSweep                = (1 << 27),
  IsSystem                      = (1 << 28),
  IsTrait                       = (1 << 29),
  NeedsActRec                   = (1 << 31),
};

#define VarArgsMask (VariableArguments | \
                     RefVariableArguments | \
                     MixedVariableArguments)

bool isKindOfIndirect(DataType kindof);

static inline fbstring kindOfString(DataType t) {
  switch (t) {
    case KindOfAny:          return "Any";
    case KindOfUnknown:      return "Unknown";
    case KindOfNull:         return "Null";
    case KindOfBoolean:      return "Boolean";
    case KindOfInt64:        return "Int64";
    case KindOfDouble:       return "Double";
    case KindOfStaticString: return "StaticString";
    case KindOfString:       return "String";
    case KindOfArray:        return "Array";
    case KindOfObject:       return "Object";
    case KindOfRef:          return "Ref";
    default:
      // No other enums should occur in IDL parsing
      assert(false);
      return "";
  }
}

class PhpParam {
 public:
  explicit PhpParam(const folly::dynamic& param, bool isMagicMethod = false);

  fbstring name() const { return m_name; }
  fbstring getCppType() const { return m_cppType; }
  fbstring getStrippedCppType() const {
    fbstring ret = m_cppType;
    if (!ret.compare(0, 6, "HPHP::")) {
      ret = ret.substr(6);
    }
    if (!ret.compare(ret.length() - 7, 7, " const&")) {
      ret = ret.substr(0, ret.length() - 7);
    }
    return ret;
  }
  DataType kindOf() const { return m_kindOf; }

  bool isRef() const { return m_param.getDefault("ref", false).asBool(); }

  bool hasDefault() const {
    return m_param.find("value") != m_param.items().end();
  }
  fbstring getDefault() const {
    return hasDefault() ? m_param["value"].asString() : "";
  }

  bool isCheckedType() const {
    return !isRef() && (kindOf() != KindOfAny);
  }
  bool defValueNeedsVariable() const;

  bool isIndirectPass() const { return isKindOfIndirect(kindOf()); }

 private:
  fbstring m_name;
  folly::dynamic m_param;
  DataType m_kindOf;
  fbstring m_cppType;
};

class PhpFunc {
 public:
  PhpFunc(const folly::dynamic& d, const fbstring& className);

  fbstring name() const { return m_name; }
  fbstring className() const { return m_className; }

  bool isMethod() const {
    return !m_className.empty();
  }

  bool isMagicMethod() const {
    return (isMethod() && (
        (m_name == "__get") ||
        (m_name == "__set") ||
        (m_name == "__isset") ||
        (m_name == "__unset") ||
        (m_name == "__call")));
  }

  fbstring getCppSig() const;

  fbstring getPrettyName() const {
    if (isMethod()) {
      return m_className + "::" + m_name;
    } else {
      return m_name;
    }
  }

  fbstring getUniqueName() const {
    if (isMethod()) {
      return folly::to<fbstring>(m_className.length(), m_className,
                                 '_', m_name);
    } else {
      return m_name;
    }
  }

  bool isReturnRef() const { return m_returnRef; }
  DataType returnKindOf() const { return m_returnKindOf; }
  fbstring returnCppType() const { return m_returnCppType; }

  bool isIndirectReturn() const { return isKindOfIndirect(returnKindOf()); }

  bool isCtor() const { return isMethod() && (m_name == "__construct"); }
  bool isStatic() const { return m_flags & IsStatic; }
  bool isVarArgs() const { return m_flags & VarArgsMask; }
  bool usesThis() const { return isMethod() && !isStatic(); }

  int numParams() const { return m_params.size(); }
  int minNumParams() const { return m_minNumParams; }
  int numTypeChecks() const { return m_numTypeChecks; }

  const PhpParam& param(int p) const { return m_params[p]; }
  const fbvector<PhpParam>& params() const { return m_params; }

private:
  fbstring m_name;
  fbstring m_className;
  folly::dynamic m_func;
  unsigned long m_flags;

  // Return value
  bool m_returnRef;
  DataType m_returnKindOf;
  fbstring m_returnCppType;

  fbvector<PhpParam> m_params;

  // Computed properties.
  int m_minNumParams;
  int m_numTypeChecks;
};

class PhpClass {
 public:
  explicit PhpClass(const folly::dynamic &c);

  fbstring name() const { return m_name; }

  int numMethods() const { return m_methods.size(); }
  const fbvector<PhpFunc>& methods() const { return m_methods; }

  unsigned long flags() const { return m_flags; }

 private:
  folly::dynamic m_class;
  fbstring m_name;
  fbvector<PhpFunc> m_methods;
  unsigned long m_flags;

  void initFlagsProperty();
};

void parseIDL(const char* idlFilePath,
              fbvector<PhpFunc>& funcVec,
              fbvector<PhpClass>& classVec);

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::IDL
#endif

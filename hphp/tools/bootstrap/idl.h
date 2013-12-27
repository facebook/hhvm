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
  ZendParamModeNull             = (1 <<  0),
  CppCustomDelete               = (1 <<  1),
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
  ZendCompat                    = (1 << 14),
  IsCppSerializable             = (1 << 15),
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
  ZendParamModeFalse            = (1 << 30),
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
    case KindOfResource:     return "Resource";
    case KindOfRef:          return "Ref";
    default:
      // No other enums should occur in IDL parsing
      assert(false);
      return "";
  }
}

static inline fbstring escapeCpp(const fbstring& str) {
  std::ostringstream ssb;
  for (auto c : str) {
    switch (c) {
      case '\n': ssb << "\\n";   break;
      case '\r': ssb << "\\r";   break;
      case '\t': ssb << "\\t";   break;
      case '\a': ssb << "\\a";   break;
      case '\b': ssb << "\\b";   break;
      case '\f': ssb << "\\f";   break;
      case '\v': ssb << "\\v";   break;
      case '\0': ssb << "\\000"; break;
      case '\"': ssb << "\\\"";  break;
      case '\\': ssb << "\\\\";  break;
      case '\?': ssb << "\\?";   break; // trigraphs
      default:
        if ((c >= 0x20) && (c <= 0x7f)) {
          ssb << c;
        } else {
          char buf[6];
          snprintf(buf, sizeof(buf), "\\%03o", (unsigned int)c);
          ssb << buf;
        }
    }
  }
  return fbstring(ssb.str());
}

static inline fbstring strtolower(const fbstring& str) {
  fbstring lcase = str;
  std::transform(str.begin(), str.end(), lcase.begin(),
                 std::ptr_fun<int, int>(std::tolower));
  return lcase;
}

fbstring phpSerialize(const folly::dynamic& d);

enum class ParamMode {
  CoerceAndCall,
  ZendNull,
  ZendFalse
};

class PhpConst {
 public:
  explicit PhpConst(const folly::dynamic& cns, fbstring cls = "");

  fbstring name() const { return m_name; }
  fbstring varname() const {
    return m_className.empty() ? ("k_" + m_name)
                               : ("q_" + m_className + "$$" + m_name);
  }
  bool isSystem() const { return m_className.empty(); }
  fbstring getCppType() const { return m_cppType; }
  DataType kindOf() const { return m_kindOf; }
  bool hasValue() const {
    return (m_constant.find("value") != m_constant.items().end());
  }
  fbstring value() const {
    auto it = m_constant.find("value");
    assert(it != m_constant.items().end());
    auto v = it->second;
    return v.isNull() ? "uninit_null()" : v.asString();
  }

  fbstring serialize() const { return phpSerialize(m_constant["value"]); }

 private:
  folly::dynamic m_constant;
  fbstring m_name;
  fbstring m_className;
  fbstring m_cppType;
  DataType m_kindOf;

  bool parseType(const folly::dynamic& cns);
  bool inferType(const folly::dynamic& cns);
};

class PhpParam {
 public:
  explicit PhpParam(const folly::dynamic& param, bool isMagicMethod = false,
                    ParamMode paramMode = ParamMode::CoerceAndCall);

  fbstring name() const { return m_name; }
  fbstring getDesc() const { return m_desc; }
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
  fbstring getPhpType() const { return m_phpType; }

  bool isRef() const { return m_param.getDefault("ref", false).asBool(); }

  bool hasDefault() const {
    return m_param.find("value") != m_param.items().end();
  }
  fbstring getDefault() const {
    return hasDefault() ? m_param["value"].asString() : "";
  }
  fbstring getDefaultSerialized() const;
  fbstring getDefaultPhp() const;

  bool isCheckedType() const {
    return !isRef() && (kindOf() != KindOfAny);
  }
  bool defValueNeedsVariable() const;

  bool isIndirectPass() const { return isKindOfIndirect(kindOf()); }

  ParamMode getParamMode() const { return m_paramMode; }

 private:
  fbstring m_name;
  folly::dynamic m_param;
  fbstring m_desc;
  DataType m_kindOf;
  fbstring m_cppType;
  fbstring m_phpType;
  ParamMode m_paramMode;
};

class PhpFunc {
 public:
  PhpFunc(const folly::dynamic& d, const fbstring& className);

  fbstring name() const { return m_name; }
  fbstring lowerName() const {
    fbstring name = m_name;
    for (char& c : name) {
      c = tolower(c);
    }
    return name;
  }
  fbstring className() const { return m_className; }
  fbstring getDesc() const { return m_desc; }

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
  fbstring returnPhpType() const { return m_returnPhpType; }
  fbstring returnDesc() const { return m_returnDesc; }

  bool isIndirectReturn() const { return isKindOfIndirect(returnKindOf()); }

  bool isCtor() const { return isMethod() && (m_name == "__construct"); }
  bool isStatic() const { return m_flags & IsStatic; }
  bool isVarArgs() const { return m_flags & VarArgsMask; }
  bool usesThis() const { return isMethod() && !isStatic(); }
  unsigned int flags() const { return m_flags; }

  int numParams() const { return m_params.size(); }
  int minNumParams() const { return m_minNumParams; }
  int numTypeChecks() const { return m_numTypeChecks; }

  const PhpParam& param(int p) const { return m_params[p]; }
  const fbvector<PhpParam>& params() const { return m_params; }

  bool hasDocComment() const {
    auto it = m_func.find("doc");
    return (it != m_func.items().end()) && it->second.size();
  }
  fbstring docComment() const {
    return hasDocComment() ? m_func["doc"].asString() : "";
  }

private:
  fbstring m_name;
  fbstring m_className;
  folly::dynamic m_func;
  unsigned long m_flags;
  fbstring m_desc;

  // Return value
  bool m_returnRef;
  DataType m_returnKindOf;
  fbstring m_returnCppType;
  fbstring m_returnPhpType;
  fbstring m_returnDesc;

  fbvector<PhpParam> m_params;

  // Computed properties.
  int m_minNumParams;
  int m_numTypeChecks;
};

class PhpProp {
 public:
  PhpProp(const folly::dynamic& d, fbstring cls);

  fbstring name() const { return m_name; }
  fbstring className() const { return m_className; }
  unsigned long flags() const { return m_flags; }
  DataType kindOf() const { return m_kindOf; }

 private:
  fbstring m_name;
  fbstring m_className;
  folly::dynamic m_prop;
  unsigned long m_flags;
  DataType m_kindOf;
};

class PhpClass {
 public:
  explicit PhpClass(const folly::dynamic &c);

  fbstring lowerName() const {
    fbstring name = m_cppName;
    for (char& c : name) {
      c = tolower(c);
    }
    return name;
  }

  fbstring getPhpName() const { return m_phpName; };
  fbstring getCppName() const { return m_cppName; };

  fbstring parent() const {
    auto p = m_class.find("parent");
    if (p == m_class.items().end()) {
      return "";
    }
    assert(p->second.isString());
    return p->second.asString();
  }

  int numIfaces() const { return m_ifaces.size(); }
  fbvector<fbstring> ifaces() const { return m_ifaces; }

  bool hasDocComment() const {
    auto it = m_class.find("doc");
    return (it != m_class.items().end()) && it->second.size();
  }
  fbstring docComment() const {
    return hasDocComment() ? m_class["doc"].asString() : "";
  }

  fbstring getDesc() const { return m_desc; }

  int numMethods() const { return m_methods.size(); }
  const fbvector<PhpFunc>& methods() const { return m_methods; }

  unsigned long flags() const { return m_flags; }

  int numProperties() const { return m_properties.size(); }
  fbvector<PhpProp> properties() const { return m_properties; }

  int numConstants() const { return m_constants.size(); }
  fbvector<PhpConst> constants() const { return m_constants; }

 private:
  folly::dynamic m_class;
  // The name in the IDL file. Use '_' for namespaces.
  fbstring m_idlName;
  // The name in PHP land.
  fbstring m_phpName;
  // The name in the IDL with namespaces stripped.
  fbstring m_cppName;
  fbvector<fbstring> m_ifaces;
  fbvector<PhpFunc> m_methods;
  fbvector<PhpConst> m_constants;
  fbvector<PhpProp> m_properties;
  unsigned long m_flags;
  fbstring m_desc;
};

class PhpExtension {
 public:
  explicit PhpExtension(const folly::dynamic& e)
    : m_extension(e) { }

  /* The C++ symbol name of the Extension struct.
   * Only needed if s_(name)_extension is not correct.
   * Set to blank if this ext doens't declare an Extension.
   */
  fbstring symbol() const {
    auto it = m_extension.find("symbol");
    if (it != m_extension.items().end()) {
      return it->second.asString();
    }
    return "";
  }

 private:
  folly::dynamic m_extension;
};

void parseIDL(const char* idlFilePath,
              fbvector<PhpFunc>& funcVec,
              fbvector<PhpClass>& classVec);

void parseIDL(const char* idlFilePath,
              fbvector<PhpFunc>& funcVec,
              fbvector<PhpClass>& classVec,
              fbvector<PhpConst>& constVec,
              fbvector<PhpExtension>& extVec);

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::IDL
#endif

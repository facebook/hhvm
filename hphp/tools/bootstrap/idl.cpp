/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/tools/bootstrap/idl.h"

#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <string>

#include "folly/Format.h"
#include "folly/json.h"

#ifdef __APPLE__
#define INT64_TYPE "long long"
#else
#define INT64_TYPE "long"
#endif

namespace HPHP { namespace IDL {
/////////////////////////////////////////////////////////////////////////////

static const std::unordered_map<fbstring, DataType> g_kindOfMap =
{
  {"Boolean",     KindOfBoolean},
  {"Int32",       KindOfInt64},
  {"Int64",       KindOfInt64},
  {"Double",      KindOfDouble},
  {"String",      KindOfString},
  {"Int64Vec",    KindOfArray},
  {"StringVec",   KindOfArray},
  {"VariantVec",  KindOfArray},
  {"Int64Map",    KindOfArray},
  {"StringMap",   KindOfArray},
  {"VariantMap",  KindOfArray},
  {"Object",      KindOfObject},
  {"Resource",    KindOfResource},
  {"Variant",     KindOfAny},
  {"Numeric",     KindOfAny},
  {"Primitive",   KindOfAny},
  {"PlusOperand", KindOfAny},
  {"Sequence",    KindOfAny},
  {"Any",         KindOfAny},
};

static const std::unordered_map<int, fbstring> g_typeMap =
{
  {(int)KindOfInvalid,     "void"},
  {(int)KindOfNull,        "HPHP::Variant"},
  {(int)KindOfBoolean,     "bool"},
  {(int)KindOfInt64,       INT64_TYPE},
  {(int)KindOfDouble,      "double"},
  {(int)KindOfString,      "HPHP::String"},
  {(int)KindOfArray,       "HPHP::Array"},
  {(int)KindOfObject,      "HPHP::Object"},
  {(int)KindOfResource,    "HPHP::Resource"},
  {(int)KindOfAny,         "HPHP::Variant"},
};

static const std::unordered_map<int, fbstring> g_phpTypeMap =
{
  {(int)KindOfInvalid,     "void"},
  {(int)KindOfNull,        "void"},
  {(int)KindOfBoolean,     "bool"},
  {(int)KindOfInt64,       INT64_TYPE},
  {(int)KindOfDouble,      "double"},
  {(int)KindOfString,      "String"},
  {(int)KindOfArray,       "Array"},
  {(int)KindOfObject,      "Object"},
  {(int)KindOfResource,    "Resource"},
  {(int)KindOfAny,         "mixed"},
};

static const std::unordered_map<fbstring, FuncFlags> g_flagsMap =
{
  {"ZendParamModeNull",              ZendParamModeNull},
  {"ZendParamModeFalse",             ZendParamModeFalse},
  {"CppCustomDelete",                CppCustomDelete},
  {"ZendCompat",                     ZendCompat},
  {"IsAbstract",                     IsAbstract},
  {"IsFinal",                        IsFinal},
  {"IsPublic",                       IsPublic},
  {"IsProtected",                    IsProtected},
  {"IsPrivate",                      IsPrivate},
  {"IgnoreRedefinition",             IgnoreRedefinition},
  {"IsStatic",                       IsStatic},
  {"IsCppAbstract",                  IsCppAbstract},
  {"IsReference",                    IsReference},
  {"IsNothing",                      IsNothing},
  {"IsCppSerializable",              IsCppSerializable},
  {"HipHopSpecific",                 HipHopSpecific},
  {"VariableArguments",              VariableArguments},
  {"RefVariableArguments",           RefVariableArguments},
  {"MixedVariableArguments",         MixedVariableArguments},
  {"FunctionIsFoldable",             FunctionIsFoldable},
  {"NoEffect",                       NoEffect},
  {"NoInjection",                    NoInjection},
  {"HasOptFunction",                 HasOptFunction},
  {"AllowIntercept",                 AllowIntercept},
  {"NoProfile",                      NoProfile},
  {"ContextSensitive",               ContextSensitive},
  {"NoDefaultSweep",                 NoDefaultSweep},
  {"IsSystem",                       IsSystem},
  {"IsTrait",                        IsTrait},
  {"NoFCallBuiltin",                 NoFCallBuiltin},
};

static const std::unordered_set<fbstring> g_knownStringConstants =
{ "k_HPHP_TRIM_CHARLIST" };

bool isKindOfIndirect(DataType kindof) {
  return (kindof != KindOfBoolean) &&
         (kindof != KindOfInt64) &&
         (kindof != KindOfDouble) &&
         (kindof != KindOfInvalid) &&
         (kindof != KindOfNull);
}

// Parse type from a descriptive string, e.g. "int", "bool", etc...
static DataType kindOfFromDynamic(const folly::dynamic& t) {
  if (!t.isString()) {
    return KindOfInvalid;
  }

  // If you hit this assert, the IDL contains "type": "Array"; you need to
  // use one of {Int64,String,Variant}{Vec,Map} instead.
  //
  // These are still KindOfArray, not the HH types.
  assert(t.asString() != "Array");

  auto it = g_kindOfMap.find(t.asString());
  if (it == g_kindOfMap.end()) {
    return KindOfObject;
  }

  return it->second;
}

// Infer type from an actual value, e.g. 123, "foo", null, true, etc...
static DataType kindOfFromValue(const folly::dynamic& v) {
  if (v.isNull()) {
    return KindOfNull;
  }
  if (v.isBool()) {
    return KindOfBoolean;
  }
  if (v.isInt()) {
    return KindOfInt64;
  }
  if (v.isDouble()) {
    return KindOfDouble;
  }
  if (v.isString()) {
    return KindOfString;
  }
  if (v.isArray()) {
    return KindOfArray;
  }
  if (v.isObject()) {
    return KindOfObject;
  }
  return KindOfInvalid;
}

static fbstring phpTypeFromDataType(DataType dt) {
  auto it = g_phpTypeMap.find((int)dt);
  if (it == g_phpTypeMap.end()) {
    return "mixed";
  }
  return it->second;
}

static fbstring typeString(const folly::dynamic& typeNode, bool isReturnType) {
  if (typeNode == "Int32") {
    return "int";
  }

  DataType kindof = kindOfFromDynamic(typeNode);
  auto it = g_typeMap.find((int)kindof);
  assert(it != g_typeMap.end());

  auto& type = it->second;
  if (!isReturnType && isKindOfIndirect(kindof)) {
    return type + " const&";
  } else {
    return type;
  }
}

static unsigned long parseFlags(const folly::dynamic &flags) {
  if (flags.isNull()) {
    return 0;
  }
  if (!flags.isArray()) {
    throw std::logic_error("'flags' field must be an array");
  }

  unsigned long ret = 0;
  for (auto &flag : flags) {
    auto f = g_flagsMap.find(flag.asString());
    if (f == g_flagsMap.end()) {
      throw std::logic_error(
        folly::format("Unknown flag '{0}' specified", flag.asString()).str()
      );
    }
    ret |= f->second;
  }
  return ret;
}

static const std::unordered_map<fbstring,fbstring> g_serializedDefaults = {
  {"true",              "b:1;"},
  {"false",             "b:0;"},
  {"null",              "N;"},
  {"empty_array",       "a:0:{}"},
  {"null_string",       "N;"},
  {"null_array",        "N;"},
  {"null_object",       "N;"},
  {"null_resource",     "N;"},
  {"null_variant",      "N;"},
  {"INT_MAX",           "i:2147483647;"}, // (1 << 31) - 1
};

static const std::unordered_map<fbstring,fbstring> g_phpDefaults = {
  {"true",              "true"},
  {"false",             "false"},
  {"null",              "null"},
  {"empty_array",       "array()"},
  {"null_string",       "null"},
  {"null_array",        "null"},
  {"null_object",       "null"},
  {"null_resource",     "null"},
  {"null_variant",      "null"},
  {"INT_MAX",           "null"},
};

static fbstring unescapeString(fbstring val) {
  fbstring s = "";
  for (int i = 0; i < val.size(); ) {
    int ch = val[i++];
    if (ch == '\\') {
      if (i == val.size()) {
        throw std::logic_error(
          folly::format("Malformed string: '{0}'", val).str());
      }
      ch = val[i++];
      switch (ch) {
        case 'n': ch = '\n'; break;
        case 'r': ch = '\r'; break;
        case 't': ch = '\t'; break;
        case '/':
        case '"':
        case '\'':
        case '\\':break;
        case '0':
          ch = 0;
          if (i == val.size() ||
              (!isdigit(val[i]) && val[i] != 'x' && val[i] != 'X')) {
            break;
          }
          // fall through
        default:
          throw std::logic_error(
            folly::format("Malformed string: '{0}'", val).str());
          break;
      }
    }
    s += (char)ch;
  }
  return s;
}

/**
 * From idl/base.php:get_serialized_default()
 */
fbstring PhpParam::getDefaultSerialized() const {
  auto valIt = m_param.find("value");
  if (valIt == m_param.items().end()) {
    return ""; // No default
  }
  auto dval = valIt->second;
  if (!dval.isString()) {
    throw std::logic_error(
      folly::format("Parameter '{0}' default value is non-string",
                    m_name).str()
    );
  }
  auto val = dval.asString();
  if (!val.size()) {
    throw std::logic_error(
      folly::format("Parameter '{0}' default value malformed (empty string), "
                    "specify \"\" as default value for actual empty string",
                    m_name).str()
    );
  }

  // Function calls "foo()" or "foo::bar()" to C/C++ functions/static methods,
  // a constant, or a bitmask of constants
  //
  // Used by ext_reflection to resolve the value at runtime or
  // represent the function/method call.
  if ((val.size() > 2) &&
      (!strncmp(val.c_str(), "k_", 2) ||
       !strncmp(val.c_str(), "q_", 2) ||
       !strcmp(val.c_str() + val.size() - 2, "()"))) {
    return "\x01";
  }

  // Fixed substitutions
  auto it = g_serializedDefaults.find(val);
  if (it != g_serializedDefaults.end()) {
    return it->second;
  }

  if (val == "RAND_MAX") {
    return folly::to<fbstring>("i:", RAND_MAX, ";");
  }

  // Quoted string:  "foo"
  if ((val.size() >= 2) && (val[0] == '"') && (val[val.size()-1] == '"')) {
    auto s = unescapeString(val.substr(1, val.size() - 2));
    return phpSerialize(s);
  }

  // Integers and Floats
  if (strchr(val.c_str(), '.')) {
    // Decimal float?
    char *e = nullptr;
    double dval = strtod(val.c_str(), &e);
    if (e && !*e) {
      return folly::to<fbstring>("d:", dval, ";");
    }
  }

  if (val[0] == '0') {
    if ((val.size() > 1) && (val[1] == 'x')) {
      // Hex?
      char *e = nullptr;
      long lval = strtol(val.c_str() + 2, &e, 16);
      if (e && !*e) {
        return folly::to<fbstring>("i:", lval, ";");
      }
    } else {
      // Octal?
      char *e = nullptr;
      long lval = strtol(val.c_str() + 1, &e, 8);
      if (e && !*e) {
        return folly::to<fbstring>("i:", lval, ";");
      }
    }
  }

  // Decimal?
  char *e = nullptr;
  long lval = strtol(val.c_str(), &e, 10);
  if (e && !*e) {
    return folly::to<fbstring>("i:", lval, ";");
  }

  throw std::logic_error(
    folly::format("'{0}' is not a valid default arg value", val).str()
  );
}

static fbstring transformConstants(const fbstring val) {
  fbstring ret = val;
  int i = 0;
  int len = ret.size();

  while (i < len) {
    while ((i < len) && (ret[i] == ' ')) i++;
    if ((len - i) < 2) break;

    if ((ret[i+1] == '_') &&
        ((ret[i] == 'k') || (ret[i] == 'q'))) {
      ret[i] = ret[i+1] = ' ';
    }
    while ((i < len) && (ret[i] != '|')) {
      if (ret[i] == '$') {
        ret[i] = ':';
      }
      i++;
    }
    i++;
  }
  return ret;
}

fbstring PhpParam::getDefaultPhp() const {
  fbstring val = getDefault();
  if (!val.size()) {
    return "";
  }

  auto it = g_phpDefaults.find(val);
  if (it != g_phpDefaults.end()) {
    return it->second;
  }

  if (val == "RAND_MAX") {
    return folly::to<fbstring>(RAND_MAX);
  }

  if ((val.size() > 2) && (val[1] == '_') &&
    ((val[0] == 'k') || (val[0] == 'q'))) {
    return transformConstants(val);
  }

  return val;
}

fbstring phpSerialize(const folly::dynamic& d) {
  if (d.isNull()) {
    return "N;";
  }
  if (d.isBool()) {
    return d.asBool() ? "b:1;" : "b:0;";
  }
  if (d.isInt()) {
    return "i:" + d.asString() + ";";
  }
  if (d.isDouble()) {
    return "d:" + d.asString() + ";";
  }
  if (d.isString()) {
    auto str = d.asString();
    return folly::to<fbstring>("s:", str.size(), ":\"", str, "\";");
  }
  if (d.isArray()) {
    fbstring ret = folly::to<fbstring>("a:", d.size(), ":{");
    int i = 0;
    for (auto &v : d) {
      ret += folly::to<fbstring>("i:", i, ";", phpSerialize(v));
    }
    return ret + "};";
  }
  if (d.isObject()) {
    fbstring ret = folly::to<fbstring>("a:", d.size(), ":{");
    int nextindex = 0;
    for (auto &k : d.keys()) {
      if (k.isNull()) {
        ret += "i:0;";
        if (nextindex <= 0) {
          nextindex = 1;
        }
      } else if (k.isInt() || k.isDouble()) {
        int i = k.asInt();
        ret += folly::to<fbstring>("i:", i, ";");
        if (nextindex <= i) {
          nextindex = i + 1;
        }
      } else if (k.isString()) {
        ret += folly::to<fbstring>("s:", k.size(), ":\"",
                                   escapeCpp(k.asString()), "\";");
      } else {
        /* Should never be reached, but cover it to be safe */
        ret += folly::to<fbstring>("i:", nextindex++, ";");
      }
      ret += phpSerialize(d[k]);
    }
    return ret + "};";
  }
  throw std::logic_error("Unhandled dynamic type in php serialization");
  return "N;";
}

static auto NAMESPACE_STRING = "\\";

static fbstring getFollyDynamicDefaultString(const folly::dynamic& d,
                                             const fbstring& key,
                                             const fbstring& def) {
  auto it = d.find(key);
  if (it == d.items().end()) {
    return def;
  }
  auto val = it->second;
  if (val.isNull()) {
    return def;
  }
  return val.asString();
}

static fbstring toPhpName(fbstring idlName) {
  fbstring phpName = idlName;
  return phpName;
}

/*
 * Strip an idl name of any namespaces.
 */
static fbstring toCppName(fbstring idlName) {
  fbstring cppName;

  size_t endNs = idlName.find_last_of(NAMESPACE_STRING);
  cppName = (std::string::npos == endNs) ? idlName : idlName.substr(endNs + 1);
  assert(!cppName.empty());

  return cppName;
}

/////////////////////////////////////////////////////////////////////////////
// PhpConst

bool PhpConst::parseType(const folly::dynamic& cns) {
  auto it = cns.find("type");
  if (it != cns.items().end()) {
    m_kindOf = kindOfFromDynamic(it->second);
    m_cppType = typeString(it->second, false);
    return true;
  }
  return false;
}

bool PhpConst::inferType(const folly::dynamic& cns) {
  auto it = cns.find("value");
  if (it != cns.items().end()) {
    m_kindOf = kindOfFromValue(it->second);
    auto typeIt = g_typeMap.find((int)m_kindOf);
    if (typeIt != g_typeMap.end()) {
      m_cppType = typeIt->second;
      return true;
    }
  }
  return false;
}

PhpConst::PhpConst(const folly::dynamic& cns,
                   fbstring cls /* = "" */) :
    m_constant(cns),
    m_name(cns["name"].asString()),
    m_className(cls) {
  if (!parseType(cns) && !inferType(cns)) {
    // Constant has neither explicit type nor implicit type from 'value'
    assert(false);
    m_kindOf = KindOfInvalid;
    m_cppType = "void";
  }

  // Override typeString()'s selection for string values
  if (m_kindOf == KindOfString) {
    m_cppType = "HPHP::StaticString";
  }
}

/////////////////////////////////////////////////////////////////////////////
// PhpParam

PhpParam::PhpParam(const folly::dynamic& param,
                   bool isMagicMethod /*= false */,
                   ParamMode paramMode /*= CoerceAndCall */) :
    m_name(param["name"].asString()),
    m_param(param),
    m_desc(getFollyDynamicDefaultString(param, "desc", "")),
    m_paramMode(paramMode) {
  if (isMagicMethod) {
    m_kindOf = KindOfAny;
    m_cppType = "HPHP::Variant";
    return;
  }

  if (isRef()) {
    m_kindOf = KindOfRef;
    m_cppType = "HPHP::VRefParamValue const&";
  } else {
    m_kindOf = kindOfFromDynamic(param["type"]);
    m_cppType = typeString(param["type"], false);
  }

  m_phpType = phpTypeFromDataType(m_kindOf);
}

bool PhpParam::defValueNeedsVariable() const {
  DataType cppKindOf = kindOf();

  if (!hasDefault() || !isIndirectPass()) {
    return false;
  }

  fbstring defVal = getDefault();

  if (cppKindOf == KindOfString &&
      ((defVal == "empty_string") ||
       (defVal == "null_string") ||
       (g_knownStringConstants.count(defVal) > 0))) {
    return false;
  }
  if ((cppKindOf == KindOfArray) && (defVal == "null_array")) {
    return false;
  }
  if ((cppKindOf == KindOfObject) && (defVal == "null_object")) {
    return false;
  }
  if ((cppKindOf == KindOfResource) && (defVal == "null_resource")) {
    return false;
  }
  if ((cppKindOf == KindOfAny) && (defVal == "null_variant")) {
    return false;
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////
// PhpFunc

PhpFunc::PhpFunc(const folly::dynamic& d,
                 const fbstring& className) :
    m_idlName(d["name"].asString()),
    m_phpName(toPhpName(m_idlName)),
    m_cppName(toCppName(m_idlName)),
    m_className(className),
    m_func(d),
    m_desc(getFollyDynamicDefaultString(d, "desc", "")),
    m_returnRef(d.getDefault("ref", "false") == "true"),
    m_returnKindOf(KindOfNull),
    m_returnCppType("void"),
    m_returnPhpType("void"),
    m_minNumParams(0),
    m_numTypeChecks(0) {

  if (isMethod() &&
      m_idlName.find_last_of(NAMESPACE_STRING) != std::string::npos) {
    throw std::logic_error(
      folly::format("'{0}' is a method and cannot have a namespace in its name",
                    m_idlName).str()
    );
  }
  auto returnIt = d.find("return");
  if (returnIt != d.items().end()) {
    auto retNode = returnIt->second;
    auto typeIt = retNode.find("type");
    if (typeIt != retNode.items().end()) {
      auto type = typeIt->second;
      if ((type.isString()) && (type != "void") && (type != "null")) {
        m_returnKindOf = m_returnRef ? KindOfRef : kindOfFromDynamic(type);
        m_returnCppType = typeString(type, true);
        m_returnPhpType = phpTypeFromDataType(m_returnKindOf);
      }
    }
    m_returnDesc = getFollyDynamicDefaultString(retNode, "desc", "");
  }

  auto args = d.find("args");
  if (args == d.items().end() || !args->second.isArray()) {
    throw std::logic_error(
      folly::format("'{0}' must have an array field 'args'", m_idlName).str()
    );
  }
  auto ret = d.find("return");
  if (ret == d.items().end() || !ret->second.isObject() ||
      ret->second.find("type") == ret->second.items().end()) {
    throw std::logic_error(
      folly::format("'{0}' must have an array field 'return', which must have "
                    "a string field 'type'", m_idlName).str()
    );
  }

  bool magic = isMagicMethod();

  m_flags = parseFlags(m_func["flags"]);

  ParamMode paramMode = ParamMode::CoerceAndCall;
  if (m_flags & ZendParamModeNull) {
    paramMode = ParamMode::ZendNull;
  } else if (m_flags & ZendParamModeFalse) {
    paramMode = ParamMode::ZendFalse;
  }

  for (auto &p : args->second) {
    PhpParam param(p, magic, paramMode);
    m_params.push_back(param);
    if (!param.hasDefault()) {
      ++m_minNumParams;
    }
    if (param.isCheckedType()) {
      ++m_numTypeChecks;
    }
  }
}

fbstring PhpFunc::getCppSig() const {
  std::ostringstream out;

  fbstring nm = getCppName();
  fbstring lowername = nm;
  std::transform(nm.begin(), nm.end(), lowername.begin(),
                 std::ptr_fun<int, int>(std::tolower));

  if (!isMethod()) {
    out << "HPHP::f_" << lowername << "(";
  } else {
    if (isStatic()) {
      out << "HPHP::c_" << className() << "::ti_" << lowername << "(";
    } else {
      out << "HPHP::c_" << className() << "::t_" << lowername << "(";
    }
  }

  bool firstParam = true;
  if (isVarArgs()) {
    if (!firstParam) {
      out << ", ";
    }
    out << "int";
    firstParam = false;
  }

  for (auto const& param : m_params) {
    if (!firstParam) {
      out << ", ";
    }
    out << param.getCppType();
    firstParam = false;
  }

  if (isVarArgs()) {
    assert(!firstParam);
    out << ", HPHP::Array const&";
  }

  out << ")";
  return out.str();
}

/////////////////////////////////////////////////////////////////////////////
// PhpProp

PhpProp::PhpProp(const folly::dynamic& d, fbstring cls) :
    m_name(d["name"].asString()),
    m_className(cls),
    m_prop(d),
    m_flags(parseFlags(m_prop["flags"])),
    m_kindOf(kindOfFromDynamic(m_prop["type"])) {
}

/////////////////////////////////////////////////////////////////////////////
// PhpClass

PhpClass::PhpClass(const folly::dynamic &c) :
  m_class(c),
  m_idlName(c["name"].asString()),
  m_phpName(toPhpName(m_idlName)),
  m_cppName(toCppName(m_idlName)),
  m_flags(parseFlags(m_class["flags"])),
  m_desc(getFollyDynamicDefaultString(c, "desc", "")) {

  auto ifacesIt = m_class.find("ifaces");
  if (ifacesIt != m_class.items().end()) {
    auto ifaces = ifacesIt->second;
    if (!ifaces.isArray()) {
      throw std::logic_error(
        folly::format("Class {0}.ifaces field must be an array",
          m_idlName).str()
      );
    }
    for (auto &interface : ifaces) {
      m_ifaces.push_back(interface.asString());
    }
  }

  for (auto const& f : c["funcs"]) {
    PhpFunc func(f, getCppName());
    m_methods.push_back(func);
  }

  if (c.find("consts") != c.items().end()) {
    for (auto const& cns : c["consts"]) {
      PhpConst cons(cns, getCppName());
      m_constants.push_back(cons);
    }
  }

  if (c.find("properties") != c.items().end()) {
    for (auto const& prp : c["properties"]) {
      PhpProp prop(prp, getCppName());
      m_properties.push_back(prop);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

void parseIDL(const char* idlFilePath,
              fbvector<PhpFunc>& funcVec,
              fbvector<PhpClass>& classVec,
              fbvector<PhpConst>& constVec,
              fbvector<PhpExtension>& extVec) {
  std::ostringstream jsonString;
  std::ifstream infile(idlFilePath);
  infile >> jsonString.rdbuf();

  auto parsed = folly::parseJson(jsonString.str());

  for (auto const& f : parsed["funcs"]) {
    PhpFunc func(f, "");
    funcVec.push_back(func);
  }
  for (auto const& c : parsed["classes"]) {
    PhpClass klass(c);
    classVec.push_back(klass);
  }
  for (auto const& c : parsed["consts"]) {
    PhpConst cns(c);
    constVec.push_back(cns);
  }
  auto it = parsed.find("extension");
  if (it != parsed.items().end()) {
    PhpExtension ext(it->second);
    extVec.push_back(ext);
  }
}

void parseIDL(const char* idlFilePath,
              fbvector<PhpFunc>& funcVec,
              fbvector<PhpClass>& classVec) {
  fbvector<PhpConst> consts; // dummy
  fbvector<PhpExtension> exts; // dummy
  parseIDL(idlFilePath, funcVec, classVec, consts, exts);
}

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::IDL

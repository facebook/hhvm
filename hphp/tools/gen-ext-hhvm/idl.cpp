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

#include "hphp/tools/gen-ext-hhvm/idl.h"

#include <fstream>
#include <unordered_map>
#include <unordered_set>

#include "folly/Format.h"
#include "folly/json.h"

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
  {"Resource",    KindOfObject},
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
  {(int)KindOfBoolean,     "bool"},
  {(int)KindOfInt64,       "long"},
  {(int)KindOfDouble,      "double"},
  {(int)KindOfString,      "HPHP::String"},
  {(int)KindOfArray,       "HPHP::Array"},
  {(int)KindOfObject,      "HPHP::Object"},
  {(int)KindOfAny,         "HPHP::Variant"},
};

static const std::unordered_map<fbstring, FuncFlags> g_flagsMap =
{
  {"IsAbstract",                     IsAbstract},
  {"IsFinal",                        IsFinal},
  {"IsPublic",                       IsPublic},
  {"IsProtected",                    IsProtected},
  {"IsPrivate",                      IsPrivate},
  {"IgnoreRedefinition",             IgnoreRedefinition},
  {"IsStatic",                       IsStatic},
  {"IsCppAbstract",                  IsCppAbstract},
  {"IsReference",                    IsReference},
  {"IsConstructor",                  IsConstructor},
  {"IsNothing",                      IsNothing},
  {"HasDocComment",                  HasDocComment},
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
  {"NeedsActRec",                    NeedsActRec},
};

static const std::unordered_set<fbstring> g_knownStringConstants =
{ "k_HPHP_TRIM_CHARLIST" };

bool isKindOfIndirect(DataType kindof) {
  return (kindof != KindOfBoolean) &&
         (kindof != KindOfInt64) &&
         (kindof != KindOfDouble) &&
         (kindof != KindOfInvalid);
}

static DataType kindOfFromDynamic(const folly::dynamic& t) {
  if (!t.isString()) {
    return KindOfInvalid;
  }
  auto it = g_kindOfMap.find(t.asString());
  if (it == g_kindOfMap.end()) {
    return KindOfObject;
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
      continue;
    }
    ret |= f->second;
  }
  return ret;
}

/////////////////////////////////////////////////////////////////////////////
// PhpParam

PhpParam::PhpParam(const folly::dynamic& param,
                   bool isMagicMethod /*= false */) :
    m_name(param["name"].asString()),
    m_param(param) {
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
  if ((cppKindOf == KindOfAny) && (defVal == "null_variant")) {
    return false;
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////
// PhpFunc

PhpFunc::PhpFunc(const folly::dynamic& d,
                 const fbstring& className) :
    m_name(d["name"].asString()),
    m_className(className),
    m_func(d),
    m_returnRef(d.getDefault("ref", "false") == "true"),
    m_returnKindOf(m_returnRef ? KindOfRef :
                   kindOfFromDynamic(d["return"]["type"])),
    m_returnCppType(typeString(d["return"]["type"], true)),
    m_minNumParams(0),
    m_numTypeChecks(0) {
  // Better at least have a name
  auto args = d.find("args");
  if (args == d.items().end() || !args->second.isArray()) {
    throw std::logic_error(
      folly::format("'{0}' must have an array field 'args'", name()).str()
    );
  }
  auto ret = d.find("return");
  if (ret == d.items().end() || !ret->second.isObject() ||
      ret->second.find("type") == ret->second.items().end()) {
    throw std::logic_error(
      folly::format("'{0}' must have an array field 'return', which must have "
                    "a string field 'type'", name()).str()
    );
  }

  bool magic = isMagicMethod();
  for (auto &p : args->second) {
    PhpParam param(p, magic);
    m_params.push_back(param);
    if (!param.hasDefault()) {
      ++m_minNumParams;
    }
    if (param.isCheckedType()) {
      ++m_numTypeChecks;
    }
  }

  m_flags = parseFlags(m_func["flags"]);
}

fbstring PhpFunc::getCppSig() const {
  std::ostringstream out;

  fbstring nm = name();
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
// PhpClass

PhpClass::PhpClass(const folly::dynamic &c) :
  m_class(c),
  m_name(c["name"].asString()) {
  for (auto const& f : c["funcs"]) {
    PhpFunc func(f, m_name);
    m_methods.push_back(func);
  }
  m_flags = parseFlags(m_class["flags"]);
}

/////////////////////////////////////////////////////////////////////////////

void parseIDL(const char* idlFilePath,
              fbvector<PhpFunc>& funcVec,
              fbvector<PhpClass>& classVec) {
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
}

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::IDL

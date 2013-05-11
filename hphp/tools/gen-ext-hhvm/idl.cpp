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

#include "idl.h"

#include <fstream>
#include <unordered_map>
#include <unordered_set>

#include "folly/Format.h"
#include "folly/json.h"


const std::unordered_map<fbstring, fbstring> g_typeMap =
{
  {"Boolean",     "bool"},
  {"Int32",       "int"},
  {"Int64",       "long"},
  {"Double",      "double"},
  {"String",      "HPHP::String"},
  {"Int64Vec",    "HPHP::Array"},
  {"StringVec",   "HPHP::Array"},
  {"VariantVec",  "HPHP::Array"},
  {"Int64Map",    "HPHP::Array"},
  {"StringMap",   "HPHP::Array"},
  {"VariantMap",  "HPHP::Array"},
  {"Object",      "HPHP::Object"},
  {"Resource",    "HPHP::Object"},
  {"Variant",     "HPHP::Variant"},
  {"Numeric",     "HPHP::Variant"},
  {"Primitive",   "HPHP::Variant"},
  {"PlusOperand", "HPHP::Variant"},
  {"Sequence",    "HPHP::Variant"},
  {"Any",         "HPHP::Variant"},
};

const std::unordered_set<fbstring> g_knownStringConstants =
{ "k_HPHP_TRIM_CHARLIST" };


bool isTypeCppIndirectPass(const fbstring& type) {
  return (type != "bool" &&
          type != "int" &&
          type != "long" &&
          type != "double" &&
          type != "void");
}

fbstring typeString(const folly::dynamic& typeNode, bool isReturnType) {
  if (!typeNode.isString()) {
    return "void";
  }

  auto typeIt = g_typeMap.find(typeNode.asString());
  if (typeIt == g_typeMap.end()) {
    return (isReturnType ? "HPHP::Object" : "HPHP::Object const&");
  }

  auto& type = typeIt->second;
  if (!isReturnType && isTypeCppIndirectPass(type)) {
    return type + " const&";
  } else {
    return type;
  }
}

bool PhpParam::defValNeedsVariable() const {
  if (defVal.empty() || !isTypeCppIndirectPass(cppType)) {
    return false;
  }

  if (cppType == "HPHP::String const&" && (defVal == "empty_string" ||
                                           defVal == "null_string")) {
    return false;
  } else if (cppType == "HPHP::String const&" &&
             g_knownStringConstants.count(defVal) > 0) {
    return false;
  } else if (cppType == "HPHP::Array const&" && defVal == "null_array") {
    return false;
  } else if (cppType == "HPHP::Object const&" && defVal == "null_object") {
    return false;
  } else if (cppType == "HPHP::Variant const&" && defVal == "null_variant") {
    return false;
  }
  return true;
}

PhpFunc PhpFunc::fromDynamic(const folly::dynamic& d,
                             const fbstring& className) {
  // Better at least have a name
  auto name = d["name"].asString();
  auto args = d.find("args");
  auto flags = d.find("flags");
  auto ret = d.find("return");
  if (args == d.items().end() || !args->second.isArray()) {
    throw std::logic_error(
      folly::format("'{0}' must have an array field 'args'", name).str()
    );
  }
  if (flags == d.items().end() || !flags->second.isArray()) {
    throw std::logic_error(
      folly::format("'{0}' must have an array field 'flags'", name).str()
    );
  }
  if (ret == d.items().end() || !ret->second.isObject() ||
      ret->second.find("type") == ret->second.items().end()) {
    throw std::logic_error(
      folly::format("'{0}' must have an array field 'return', which must have "
                    "a string field 'type'", name).str()
    );
  }

  auto areVarargs = [](const fbstring& str) {
    return (str == "VariableArguments" ||
            str == "RefVariableArguments" ||
            str == "MixedVariableArguments");
  };

  fbvector<PhpParam> params;
  try {
    params = std::move(folly::convertTo<fbvector<PhpParam>>(args->second));
  } catch (const std::exception& exc) {
    throw std::logic_error(
      folly::format("'{0}' has an arg with either 'name' or 'type' field "
                    "missing", name).str()
    );
  }
  if (name == "__get" ||
      name == "__set" ||
      name == "__isset" ||
      name == "__unset" ||
      name == "__call") {
    for (auto& param : params) {
      param.cppType = "HPHP::Variant";
    }
  }

  auto refIt = d.find("ref");
  bool ref = (refIt != d.items().end() && refIt->second.asString() == "true");

  PhpFunc retval;
  retval.name = name;
  retval.className = className;
  retval.returnCppType = typeString(ret->second["type"], true);
  retval.returnByRef = ref;
  retval.params = params;
  retval.isVarargs = anyFlags(areVarargs, flags->second);
  retval.isStatic = false;

  if (!className.empty()) {
    auto areStatic = [](const fbstring& str) {
      return str == "IsStatic";
    };

    retval.isStatic = anyFlags(areStatic, flags->second);
  }
  retval.initComputedProps();
  return retval;
}

void PhpFunc::initComputedProps() {
  maxNumParams = params.size();
  minNumParams = params.size();
  numTypeChecks = 0;
  for (auto const& param : params) {
    if (!param.defVal.empty()) {
      --minNumParams;
    }
    if (param.isCheckedType()) {
      ++numTypeChecks;
    }
  }
}

fbstring PhpFunc::getCppSig() const {
  std::ostringstream out;

  auto lowername = name;
  std::transform(name.begin(), name.end(), lowername.begin(),
                 std::ptr_fun<int, int>(std::tolower));

  if (className.empty()) {
    out << "HPHP::f_" << lowername << "(";
  } else {
    if (isStatic) {
      out << "HPHP::c_" << className << "::ti_" << lowername << "(";
    } else {
      out << "HPHP::c_" << className << "::t_" << lowername << "(";
    }
  }

  bool firstParam = true;
  if (isVarargs) {
    if (!firstParam) {
      out << ", ";
    }
    out << "int";
    firstParam = false;
  }

  for (auto const& param : params) {
    if (!firstParam) {
      out << ", ";
    }
    out << param.cppType;
    firstParam = false;
  }

  if (isVarargs) {
    assert(!firstParam);
    out << ", HPHP::Array const&";
  }

  out << ")";
  return out.str();
}

bool anyFlags(std::function<bool(const fbstring&)> pred,
              const folly::dynamic& flagsArray) {
  if (!flagsArray.isArray()) {
    return false;
  }
  for (auto const& strDyn : flagsArray) {
    auto str = strDyn.asString();
    if (pred(str)) {
      return true;
    }
  }
  return false;
}

void parseIDL(const char* idlFilePath,
              fbvector<PhpFunc>& funcVec,
              fbvector<PhpClass>& classVec) {
  std::ostringstream jsonString;
  std::ifstream infile(idlFilePath);
  infile >> jsonString.rdbuf();

  auto parsed = folly::parseJson(jsonString.str());

  for (auto const& f : parsed["funcs"]) {
    auto func = PhpFunc::fromDynamic(f, "");
    funcVec.push_back(func);
  }
  for (auto const& c : parsed["classes"]) {
    PhpClass klass;
    klass.name = c["name"].asString();
    klass.flags = folly::convertTo<fbvector<fbstring>>(c["flags"]);

    for (auto const& f : c["funcs"]) {
      auto func = PhpFunc::fromDynamic(f, c["name"].asString());
      klass.methods.push_back(func);
    }

    classVec.push_back(klass);
  }
}

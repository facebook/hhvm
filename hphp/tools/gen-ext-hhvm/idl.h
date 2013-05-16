/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

using folly::fbstring;
using folly::fbvector;


bool isTypeCppIndirectPass(const fbstring& type);

fbstring typeString(const folly::dynamic& typeNode, bool isReturnType);

struct PhpParam {
  fbstring name;
  fbstring cppType;
  fbstring defVal;
  bool byRef;

  bool isCheckedType() const {
    return (cppType != "HPHP::Variant" &&
            cppType != "HPHP::VRefParamValue const&" &&
            cppType != "HPHP::Variant const&");
  }

  bool defValNeedsVariable() const;
};

namespace folly {
template<>
struct DynamicConverter<PhpParam> {
  static PhpParam convert(const dynamic& d) {
    auto refIt = d.find("ref");
    bool ref = (refIt != d.items().end() && refIt->second.asBool());
    auto valIt = d.find("value");
    auto value = (valIt != d.items().end() ? valIt->second.asString() : "");
    auto type =
      (ref ? "HPHP::VRefParamValue const&" : typeString(d["type"], false));

    return {d["name"].asString(), type, value, ref};
  }
};
} // namespace folly


struct PhpFunc {
  fbstring name;
  fbstring className;
  fbstring returnCppType;
  bool returnByRef;
  fbvector<PhpParam> params;
  bool isVarargs;
  bool isStatic;

  // Computed properties.
  int minNumParams;
  int maxNumParams;
  int numTypeChecks;

  static PhpFunc fromDynamic(const folly::dynamic& d,
                             const fbstring& className);

  bool isMethod() const {
    return !className.empty();
  }

  fbstring getCppSig() const;

  fbstring getPrettyName() const {
    if (className.empty()) {
      return name;
    } else {
      return className + "::" + name;
    }
  }

  fbstring getUniqueName() const {
    if (className.empty()) {
      return name;
    } else {
      return folly::to<fbstring>(className.length(), className, '_', name);
    }
  }

private:
  PhpFunc() = default;
  void initComputedProps();
};

struct PhpClass {
  fbstring name;
  fbvector<fbstring> flags;
  fbvector<PhpFunc> methods;
};

bool anyFlags(std::function<bool(const fbstring&)> pred,
              const folly::dynamic& flagsArray);

void parseIDL(const char* idlFilePath,
              fbvector<PhpFunc>& funcVec,
              fbvector<PhpClass>& classVec);

#endif

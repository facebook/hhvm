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


#include "hphp/runtime/vm/runtime_type_profiler.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdio.h>

#include "folly/AtomicHashMap.h"
#include "folly/dynamic.h"
#include "folly/json.h"

#include "external/google_base/atomicops.h"


namespace HPHP {

static FuncTypeCounter emptyFuncCounter(0);
static RuntimeProfileInfo allProfileInfo(100000, &emptyFuncCounter);

void profileOneArgument(const TypedValue value,
    const int param, const Func* function) {
  const char* typeString = giveTypeString(&value);
  if (function->fullName()->size() != 0) {
    logType(function, typeString, param+1);
  }
}

std::string dumpRawParamInfo(const Func* function){
  folly::dynamic info = {};
  auto funcParamMap = allProfileInfo.get(function->getFuncId());
  for (int i = 0; i < funcParamMap->size(); i++) {
    info.push_back(folly::dynamic::object);
    auto typeCount = funcParamMap->at(i);
    for (auto j = typeCount->begin(); j != typeCount->end(); j++) {
      folly::dynamic key = std::string(j->first);
      folly::dynamic value = j->second;
      info[i][key] = value;
    }
  }
  std::string json = folly::toJson(info).toStdString();
  return json;
}

void writeProfileInformationToDisk() {
  folly::dynamic all_info = folly::dynamic::object;
  for (auto i = 0; i  <= Func::nextFuncId(); i++) {
    folly::dynamic info = {};
    auto funcParamMap = allProfileInfo.get(i);
    if (*funcParamMap == emptyFuncCounter) {
      continue;
    }
    for (auto j = 0; j < funcParamMap->size(); j++) {
      auto typeCount = funcParamMap->at(j);
      if (typeCount == nullptr) {
        continue;
      }
      info.push_back(folly::dynamic::object);
      for (auto k = typeCount->begin(); k != typeCount->end(); k++) {
        folly::dynamic key = std::string(k->first);
        folly::dynamic value = k->second;
        info[j][key] = value;
      }
    }
    const Func* func = Func::fromFuncId(i);
    all_info[std::string(func->fullName()->data())] = info;
  }
  std::ofstream logfile;
  logfile.open ("/tmp/profile");
  logfile << folly::toJson(all_info).toStdString();
  logfile.close();
}


void logType(const Func* func, const char* typeString, int64_t param) {
  if (param <= func->numParams()) {
    if (allProfileInfo.get(func->getFuncId()) == &emptyFuncCounter)
      initFuncTypeProfileData(func);
    auto it = allProfileInfo.get(func->getFuncId());
    TypeCounter* hashmap = it->at(param);
    auto success = hashmap->insert(std::make_pair(typeString, 1));
    if (!success.second) {
      base::subtle::NoBarrier_AtomicIncrement(&success.first->second, 1);
    }
  }
}

void initFuncTypeProfileData(const Func* func) {
  bool success = allProfileInfo.compare_exchange(
      func->getFuncId(), &emptyFuncCounter,
      new FuncTypeCounter(func->numParams()));
  if (success) {
    for (long i = 0; i < func->numParams() + 1; i++) {
      auto myVector = allProfileInfo.get(func->getFuncId());
      myVector->insert(myVector->begin()+ i, std::move(new TypeCounter(20)));
    }
  }
}

const char* giveTypeString(const TypedValue* value) {
  const char* typeString;
  if (value->m_type == KindOfObject){
    typeString = value->m_data.pobj->o_getClassName()->data();
  } else {
    typeString = getDataTypeString(value->m_type).c_str();
  }
  return typeString;
}
}

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


#include "hphp/runtime/vm/runtime-type-profiler.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdio.h>

#include "folly/AtomicHashMap.h"
#include "folly/dynamic.h"
#include "folly/json.h"

#include <external/google_base/atomicops.h>


namespace HPHP {

static FuncTypeCounter emptyFuncCounter(1,0);
static RuntimeProfileInfo allProfileInfo
      (RuntimeOption::EvalRuntimeTypeProfile
       ? 1: 750000, &emptyFuncCounter);

void profileOneArgument(const TypedValue value,
    const int param, const Func* function) {
  const char* typeString = giveTypeString(&value);
  if (function->fullName()->size() != 0) {
    logType(function, typeString, param + 1);
  }
}

std::string dumpRawParamInfo(const Func* function){
  folly::dynamic info = {};
  auto funcParamMap = allProfileInfo.get(function->getFuncId());
  for (int i = 0; i <= function->numParams(); i++) {
    info.push_back(folly::dynamic::object);
    auto typeCount = funcParamMap->get(i);
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
    if (funcParamMap == &emptyFuncCounter) {
      continue;
    }
    for (auto j = 0; j < Func::fromFuncId(i)->numParams(); j++) {
      auto typeCount = funcParamMap->get(j);
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
    TypeCounter* hashmap = it->get(param);
    try {
      auto success = hashmap->insert(std::make_pair(typeString, 1));
      if (!success.second) {
        base::subtle::NoBarrier_AtomicIncrement(&success.first->second, 1);
      }
    } catch (folly::AtomicHashMapFullError& e) {
      //fail silently if hashmap is full
    }
  }
}

void initFuncTypeProfileData(const Func* func) {
  auto myVector = new FuncTypeCounter(func->numParams() + 1, 0);
  for (long i = 0; i < func->numParams() + 1; i++) {
    myVector->exchange(i, new TypeCounter(200));
  }
  allProfileInfo.exchange(func->getFuncId(), myVector);
}

const char* giveTypeString(const TypedValue* value) {
  if (value->m_type == KindOfObject || value->m_type == KindOfResource){
    return value->m_data.pobj->o_getClassName()->data();
  }
  return getDataTypeString(value->m_type).c_str();
}
}

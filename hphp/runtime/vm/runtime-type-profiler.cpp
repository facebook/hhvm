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

#include "hphp/util/atomic-vector.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Holds an atomic count of profiled types.
 */
namespace {
struct ProfileCounter {
  std::atomic<int64_t> m_count;

  ProfileCounter() : m_count(0) {}
  ~ProfileCounter() = default;
  ProfileCounter& operator=(const ProfileCounter& pc) = delete;

  ProfileCounter(const ProfileCounter& pc) : m_count(pc.m_count.load()) {}
  void inc() { m_count.fetch_add(1); }
  int64_t load() { return m_count.load(); }
};
}

typedef folly::AtomicHashMap<const char*, ProfileCounter> TypeCounter;
typedef AtomicVector<TypeCounter*> FuncTypeCounter;
typedef AtomicVector<FuncTypeCounter*> RuntimeProfileInfo;

//////////////////////////////////////////////////////////////////////

static FuncTypeCounter emptyFuncCounter(1,0);
static RuntimeProfileInfo* allProfileInfo;
static std::atomic<int> counter(0);

//////////////////////////////////////////////////////////////////////

namespace {

void initFuncTypeProfileData(const Func* func) {
  auto myVector = new FuncTypeCounter(func->numParams() + 1, 0);
  for (long i = 0; i < func->numParams() + 1; i++) {
    myVector->exchange(i, new TypeCounter(200));
  }
  allProfileInfo->exchange(func->getFuncId(), myVector);
}

const char* getTypeString(const TypedValue* value) {
  if (value->m_type == KindOfObject || value->m_type == KindOfResource) {
    return value->m_data.pobj->o_getClassName()->data();
  }
  return getDataTypeString(value->m_type).c_str();
}

void logType(const Func* func, int64_t paramIndex, const char* typeString) {
  assert(paramIndex <= func->numParams());
  if (allProfileInfo->get(func->getFuncId()) == &emptyFuncCounter) {
    initFuncTypeProfileData(func);
  }
  auto it = allProfileInfo->get(func->getFuncId());
  TypeCounter* hashmap = it->get(paramIndex);
  try {
    auto success = hashmap->insert(typeString, ProfileCounter());
    if (!success.second) {
      success.first->second.inc();
    }
  } catch (folly::AtomicHashMapFullError& e) {
    // Fail silently if hashmap is full
  }
}

}

//////////////////////////////////////////////////////////////////////

void initTypeProfileStructure() {
  allProfileInfo = new RuntimeProfileInfo(750000, &emptyFuncCounter);
}

void profileOneArgument(const TypedValue value, const int paramIndex,
                        const Func* func) {
  assert(allProfileInfo != nullptr);
  const char* typeString = getTypeString(&value);

  if (func->fullName()->size() != 0) {
    logType(func, paramIndex + 1, typeString);
  }

  if (paramIndex == -1) {
    counter++;
  }

  if (counter.load() % RuntimeOption::EvalRuntimeTypeProfileLoggingFreq == 0) {
    writeProfileInformationToDisk();
  }
}

void writeProfileInformationToDisk() {
  assert(allProfileInfo != nullptr);
  folly::dynamic all_info = folly::dynamic::object;
  for (auto i = 0; i  <= Func::nextFuncId(); i++) {
    folly::dynamic info = {};
    auto funcParamMap = allProfileInfo->get(i);
    if (funcParamMap == &emptyFuncCounter) {
      continue;
    }
    for (auto j = 0; j <= Func::fromFuncId(i)->numParams(); j++) {
      auto typeCount = funcParamMap->get(j);
      if (typeCount == nullptr) {
        continue;
      }
      info.push_back(folly::dynamic::object);
      for (auto k = typeCount->begin(); k != typeCount->end(); k++) {
        folly::dynamic key = std::string(k->first);
        folly::dynamic value = k->second.load();
        info[j][key] = value;
      }
    }
    const Func* func = Func::fromFuncId(i);
    all_info[std::string(func->fullName()->data())] = info;
  }
  std::ofstream logfile;
  logfile.open("/tmp/type-profile.txt", std::fstream::out | std::fstream::app);
  logfile << folly::toJson(all_info).toStdString();
  logfile.close();
}

}

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

#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
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

static const int kShowTopN = 6;

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
  if (value->m_type == KindOfObject) {
    return value->m_data.pobj->o_getClassName()->data();
  }
  if (value->m_type == KindOfResource) {
    return value->m_data.pres->o_getClassName()->data();
  }
  return getDataTypeString(value->m_type).c_str();
}

void logType(const Func* func, int32_t paramIndex, const char* typeString) {
  if (paramIndex > func->numParams()) {
    // Don't bother logging types for extra args.
    return;
  }
  if (allProfileInfo->get(func->getFuncId()) == &emptyFuncCounter) {
    initFuncTypeProfileData(func);
  }
  auto it = allProfileInfo->get(func->getFuncId());
  TypeCounter* hashmap = it->get(paramIndex);
  try {
    auto result = hashmap->insert(typeString, ProfileCounter());
    result.first->second.inc();
  } catch (folly::AtomicHashMapFullError& e) {
    // Fail silently if hashmap is full
  }
}

Array getTopN(Array &allTypes, int n) {
  Array ret;
  for (int i = 0; i < n; i ++ ) {
    double max = 0;
    String max_key;
    for (ArrayIter iter(allTypes); iter; ++iter) {
      if (iter.second().toDouble() > max) {
        max_key = iter.first().toString();
        max = iter.second().toDouble();
      }
    }
    if (max != 0) {
      ret.set(max_key, VarNR(max));
      allTypes.remove(max_key);
    }
  }
  return ret;
}

}

//////////////////////////////////////////////////////////////////////

void initTypeProfileStructure() {
  allProfileInfo = new RuntimeProfileInfo(750000, &emptyFuncCounter);
}

void profileOneArgument(const TypedValue value, const int32_t paramIndex,
                        const Func* func) {
  assert(allProfileInfo != nullptr);
  if (!func || !func->fullName()) return;

  const char* typeString = getTypeString(&value);

  if (func->fullName()->size() != 0) {
    logType(func, paramIndex + 1, typeString);
  }

  if (paramIndex == -1) {
    counter++;
  }

  if (RuntimeOption::EvalRuntimeTypeProfileLoggingFreq &&
      counter.load() % RuntimeOption::EvalRuntimeTypeProfileLoggingFreq == 0) {
    writeProfileInformationToDisk();
  }
}

void profileAllArguments(ActRec* ar) {
  for (int i = 0; i < ar->m_func->numParams(); i++) {
    logType(ar->m_func, i + 1, getTypeString(frame_local(ar, i)));
  }
}

Array getPercentParamInfoArray(const Func* func) {
  Array ret;
  auto funcParamMap = allProfileInfo->get(func->getFuncId());
  for (int i = 0; i <= func->numParams(); i++) {
    Array types;
    auto typeCount = funcParamMap->get(i);
    if (typeCount == nullptr) {
      break;
    }
    int64_t total = 0;
    for (auto j = typeCount->begin(); j != typeCount->end(); j++) {
      total += j->second.load();
    }
    for (auto j = typeCount->begin(); j != typeCount->end(); j++) {
      String key = String(j->first);
      int64_t count = j->second.load();
      types.set(key, VarNR(double(count) / total));
    }
    Array topTypes = getTopN(types, kShowTopN);
    ret.append(VarNR(topTypes));
  }
  return ret;
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

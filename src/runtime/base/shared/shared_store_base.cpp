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

#include <runtime/base/shared/shared_store_base.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/memory/leak_detectable.h>
#include <runtime/base/server/server_stats.h>
#include <runtime/base/shared/shared_store.h>
#include <runtime/base/shared/concurrent_shared_store.h>

using namespace std;
using namespace boost;

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// SharedStore

SharedStore::SharedStore(int id) : m_id(id) {
}

SharedStore::~SharedStore() {
}

std::string SharedStore::GetSkeleton(CStrRef key) {
  std::string ret;
  const char *p = key.data();
  ret.reserve(key.size());
  bool added = false; // whether consecutive numbers are replaced by # yet
  for (int i = 0; i < key.size(); i++) {
    char ch = *p++;
    if (ch >= '0' && ch <= '9') {
      if (!added) {
        ret += '#';
        added = true;
      }
    } else {
      added = false;
      ret += ch;
    }
  }
  return ret;
}

bool SharedStore::erase(CStrRef key, bool expired /* = false */) {
  bool success = eraseImpl(key, expired);

  if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats) {
    ServerStats::Log(success ? "apc.erased" : "apc.erase", 1);
  }
  return success;
}

static std::string appendElement(int indent, const char *name, int value) {
  string ret;
  for (int i = 0; i < indent; i++) {
    ret += "  ";
  }
  ret += "<"; ret += name; ret += ">";
  ret += lexical_cast<string>(value);
  ret += "</"; ret += name; ret += ">\n";
  return ret;
}

std::string SharedStore::reportStats(int &reachable, int indent) {
  int expired, persistent;
  count(reachable, expired, persistent);

  string ret;
  ret += appendElement(indent, "Key", size());
  ret += appendElement(indent, "Persistent", persistent);
  ret += appendElement(indent, "Expiration", size() - persistent);
  ret += appendElement(indent, "Expired", expired);
  ret += appendElement(indent, "Reachable", reachable);
  return ret;
}

void StoreValue::set(SharedVariant *v, int64 ttl) {
  var = v;
  expiry = ttl ? time(NULL) + ttl : 0;
}
bool StoreValue::expired() const {
  return expiry && time(NULL) >= expiry;
}

///////////////////////////////////////////////////////////////////////////////
// SharedStores

SharedStores s_apc_store;

SharedStores::SharedStores() {
}

void SharedStores::create() {
  for (int i = 0; i < MAX_SHARED_STORE; i++) {
    switch (RuntimeOption::ApcTableType) {
      case RuntimeOption::ApcHashTable:
        switch (RuntimeOption::ApcTableLockType) {
          case RuntimeOption::ApcMutex:
            m_stores[i] = new MutexHashTableSharedStore(i);
            break;
          default:
            m_stores[i] = new RwLockHashTableSharedStore(i);
        }
        break;
      case RuntimeOption::ApcLfuTable:
        {
          time_t maturity = RuntimeOption::ApcKeyMaturityThreshold;
          size_t maxCap = RuntimeOption::ApcMaximumCapacity;
          int updatePeriod = RuntimeOption::ApcKeyFrequencyUpdatePeriod;

          if (i == SHARED_STORE_DNS_CACHE) {
            maturity = RuntimeOption::DnsCacheKeyMaturityThreshold;
            maxCap = RuntimeOption::DnsCacheMaximumCapacity;
            updatePeriod = RuntimeOption::DnsCacheKeyFrequencyUpdatePeriod;
          }
          m_stores[i] = new LfuTableSharedStore(i, maturity, maxCap,
              updatePeriod);
        }
        break;
      case RuntimeOption::ApcConcurrentTable:
        m_stores[i] = new ConcurrentTableSharedStore(i);
        break;
      default:
        ASSERT(false);
    }
  }
}

SharedStores::~SharedStores() {
  clear();
}

void SharedStores::clear() {
  for (int i = 0; i < MAX_SHARED_STORE; i++) {
    delete m_stores[i];
  }
}

void SharedStores::reset() {
  clear();
  create();
}

std::string SharedStores::reportStats(int indent) {
  string ret;
  int totalReachable = 0;
#ifdef DEBUG_APC_LEAK
  LeakDetectable::BeginLeakChecking();
#endif
  for (int i = 0; i < MAX_SHARED_STORE; i++) {
    for (int j = 0; j < indent; j++) ret += "  ";
    ret += "<SharedStore>\n";

    ret += appendElement(indent + 1, "Index", i);
    int reachable = 0;
    ret += m_stores[i]->reportStats(reachable, indent + 1);
    totalReachable += reachable;

    for (int j = 0; j < indent; j++) ret += "  ";
    ret += "</SharedStore>\n";
  }
#ifdef DEBUG_APC_LEAK
  ret += appendElement(indent, "TotalVariantsAllocated",
                       SharedVariant::TotalAllocated);
  ret += appendElement(indent, "AliveVariants",
                       SharedVariant::TotalCount);
  ret += appendElement(indent, "LeakedVariantsByReach",
                       SharedVariant::TotalCount - totalReachable);

  string dumps;
  int leaked = LeakDetectable::EndLeakChecking(dumps, 10000);
  ret += appendElement(indent, "LeakedVariantsByLeakDetectable", leaked);
  ret += dumps;
#endif
  return ret;
}

void SharedStores::Create() {
  s_apc_store.create();
}

std::string SharedStores::ReportStats(int indent) {
  return s_apc_store.reportStats(indent);
}

///////////////////////////////////////////////////////////////////////////////
}

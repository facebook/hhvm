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

#include <runtime/base/shared/shared_store.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool LockedSharedStore::clear() {
  lockMap();
  clearImpl();
  unlockMap();
  return true;
}

bool LockedSharedStore::get(CStrRef key, Variant &value) {
  bool stats = RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats;

  readLockMap();
  StoreValue *val;
  bool expired = false;
  if (!find(key, val, expired)) {
    readUnlockMap();
    if (expired) {
      erase(key, true);
    }
    value = false;
    if (stats) {
      ServerStats::Log("apc.miss", 1);
    }
    return false;
  }
  value = getVar(val->var)->toLocal();
  readUnlockMap();
  if (stats) ServerStats::Log("apc.hit", 1);
  return true;
}

bool LockedSharedStore::store(CStrRef key, CVarRef val, int64 ttl,
                              bool overwrite /* = true */) {
  bool stats = RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats;

  lockMap();
  StoreValue *sval;
  SharedVariant* var = construct(val);
  bool expired = false;
  bool added = false;
  if (find(key, sval, expired) || expired) {
    if (overwrite || expired) {
      getVar(sval->var)->decRef();
      sval->set(putVar(var), ttl);
      if (stats) ServerStats::Log("apc.update", 1);
      added = true;
    }
  } else {
    set(key, var, ttl);
    added = true;
    if (stats) {
      ServerStats::Log("apc.new", 1);
      if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCKeyStats) {
        string prefix = "apc.new.";
        prefix += GetSkeleton(key);
        ServerStats::Log(prefix, 1);
      }
    }
  }

  unlockMap();

  if (!added) var->decRef();

  return added;
}

void LockedSharedStore::prime(const std::vector<KeyValuePair> &vars) {
  lockMap();
  // we are priming, so we are not checking existence or expiration
  for (unsigned int i = 0; i < vars.size(); i++) {
    const KeyValuePair &item = vars[i];
    set(String(item.key, item.len, CopyString), item.value, 0);
  }
  unlockMap();
}

bool LockedSharedStore::eraseImpl(CStrRef key, bool expired /* = false */) {
  lockMap();
  bool success = eraseLockedImpl(key, expired);
  unlockMap();
  return success;
}

int64 LockedSharedStore::inc(CStrRef key, int64 step, bool &found) {
  found = false;
  int64 ret = 0;
  lockMap();
  StoreValue *val;
  bool expired = false;
  if (find(key, val, expired)) {
    Variant v = getVar(val->var)->toLocal();
    ret = v.toInt64() + step;
    v = ret;
    SharedVariant *var = construct(v);
    getVar(val->var)->decRef();
    val->var = putVar(var);
    found = true;
  }
  unlockMap();
  if (expired) {
    erase(key, true);
  }

  if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats) {
    ServerStats::Log("apc.inc", 1);
  }
  return ret;
}

bool LockedSharedStore::cas(CStrRef key, int64 old, int64 val) {
  bool success = false;
  lockMap();
  StoreValue *sval;
  bool expired = false;
  if (find(key, sval, expired)) {
    Variant v = getVar(sval->var)->toLocal();
    if (v.toInt64() == old) {
      v = val;
      SharedVariant *var = construct(v);
      getVar(sval->var)->decRef();
      sval->var = putVar(var);
      success = true;
    }
  }
  unlockMap();
  if (expired) {
    erase(key, true);
  }

  if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats) {
    ServerStats::Log("apc.cas", 1);
  }
  return success;
}

///////////////////////////////////////////////////////////////////////////////
}

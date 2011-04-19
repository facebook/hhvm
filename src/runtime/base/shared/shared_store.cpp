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

using namespace std;
using namespace boost;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void LockedSharedStore::clear() {
  lockMap();
  clearImpl();
  unlockMap();
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

bool LfuTableSharedStore::get(CStrRef key, Variant &value) {
  class GetReader : public Map::AtomicReader {
  public:
    GetReader(Variant &v) : expired(false), value(v) {}
    void read(StringData* const &k, const StoreValue &val) {
      value = val.var->toLocal();
      expired = val.expired();
    }
    bool expired;
    Variant &value;
  };
  bool stats = RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats;
  GetReader reader(value);
  if (!m_vars.atomicRead(key.get(), reader) || reader.expired) {
    if (reader.expired) {
      erase(key, true);
    }
    value = false;
    if (stats) ServerStats::Log("apc.miss", 1);
    return false;
  }
  if (stats) ServerStats::Log("apc.hit", 1);
  return true;
}

bool LockedSharedStore::store(CStrRef key, CVarRef val, int64 ttl,
                              bool overwrite /* = true */) {
  bool stats = RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats;

  lockMap();
  StoreValue *sval;
  SharedVariant* var = construct(key, val);
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

bool LfuTableSharedStore::store(CStrRef key, CVarRef val, int64 ttl,
                                bool overwrite /* = true */) {
  class StoreUpdater : public Map::AtomicUpdater {
  public:
    StoreUpdater(int64 t, SharedVariant *v, CStrRef k, bool ovr)
      : added(false), overwrite(ovr), ttl(t), var(v), key(k),
        newkey(key.get()->copy(true)) {}
    bool update(StringData* const &k, StoreValue &val, bool newlyCreated) {
      bool stats = RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats;
      if (!newlyCreated) {
        if (overwrite || val.expired()) {
          val.var->decRef();
          val.set(var, ttl);
          added = true;
          if (stats) ServerStats::Log("apc.update", 1);
        }
        newkey->destruct();
      } else {
        val.set(var, ttl);
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
      return false;
    }
    StringData *newKey() { return newkey; }
    bool added;
  private:
    bool overwrite;
    int64 ttl;
    SharedVariant *var;
    CStrRef key;
    StringData *newkey;
  };
  SharedVariant* var = construct(key, val);
  StoreUpdater updater(ttl, var, key, overwrite);
  m_vars.atomicUpdate(updater.newKey(), updater, true);
  if (!updater.added) {
    var->decRef();
  }
  return updater.added;
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

void LfuTableSharedStore::prime
(const std::vector<SharedStore::KeyValuePair> &vars) {
  // we are priming, so we are not checking existence or expiration
  for (unsigned int i = 0; i < vars.size(); i++) {
    const SharedStore::KeyValuePair &item = vars[i];
    // Primed values are immortal
    set(String(item.key, item.len, CopyString), item.value, 0, true);
  }
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
    SharedVariant *var = construct(key, v);
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

int64 LfuTableSharedStore::inc(CStrRef key, int64 step, bool &found) {
  class IncUpdater : public Map::AtomicUpdater {
  public:
    IncUpdater(int64 s, bool &f, CStrRef k, LfuTableSharedStore *str)
      : ret(0), step(s), found(f), key(k), store(str) {}
    bool update(StringData* const &k, StoreValue &val, bool newlyCreated) {
      if (val.expired()) {
        return true;
      }

      Variant v = val.var->toLocal();
      ret = v.toInt64() + step;
      v = ret;
      SharedVariant *var = store->construct(key, v);
      val.var->decRef();
      val.var = var;
      found = true;
      return false;
    }
    int64 ret;
  private:
    int64 step;
    bool &found;
    CStrRef key;
    LfuTableSharedStore *store;
  };

  found = false;
  IncUpdater updater(step, found, key, this);
  m_vars.atomicUpdate(key.get(), updater, false);

  if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats) {
    ServerStats::Log("apc.inc", 1);
  }
  return updater.ret;
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
      SharedVariant *var = construct(key, v);
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

bool LfuTableSharedStore::cas(CStrRef key, int64 old, int64 val) {
  class CasUpdater : public Map::AtomicUpdater {
  public:
    CasUpdater(LfuTableSharedStore *s, CStrRef k, int64 o, int64 v)
      : success(false), store(s), key(k), old(o), val(v) {}
    bool update(StringData* const &k, StoreValue &sval, bool newlyCreated) {
      if (sval.expired()) {
        return true;
      }
      Variant v = sval.var->toLocal();
      if (v.toInt64() == old) {
        v = val;
        SharedVariant *var = store->construct(key, v);
        sval.var->decRef();
        sval.var = var;
        success = true;
      }
      return false;
    }
    bool success;
  private:
    LfuTableSharedStore *store;
    CStrRef key;
    int64 old;
    int64 val;
  };
  CasUpdater updater(this, key, old, val);
  m_vars.atomicUpdate(key.get(), updater, false);

  if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats) {
    ServerStats::Log("apc.cas", 1);
  }
  return updater.success;
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

std::string LfuTableSharedStore::reportStats(int &reachable, int indent) {
  string ret = SharedStore::reportStats(reachable, indent);
  ret += appendElement(indent, "Immortal", m_vars.immortalCount());
  ret += appendElement(indent, "Maximum Capacity", m_vars.maximumCapacity());
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}

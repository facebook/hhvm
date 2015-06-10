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

#ifndef incl_HPHP_APC_STATS_H_
#define incl_HPHP_APC_STATS_H_

#include "hphp/runtime/base/apc-string.h"
#include "hphp/util/service-data.h"

namespace HPHP {

struct APCHandle;
struct APCArray;
struct APCObject;
struct APCString;
struct StringData;
struct ArrayData;

//////////////////////////////////////////////////////////////////////

/*
 * The following functions are used to get a count of the number of bytes
 * used by the different kind of objects that can be stored via APC in the
 * ConcurrentTableSharedStore.
 * The number returned should include the header for each of the object and
 * the data. Any space used by the memory allocator does not have to be
 * included.
 */
size_t getMemSize(const APCHandle*);
size_t getMemSize(const APCArray*);
size_t getMemSize(const APCObject*);
size_t getMemSize(const ArrayData*);

inline
size_t getMemSize(const StringData* string) {
  return string->isStatic() ? 0 : sizeof(StringData) + string->size();
}

inline
size_t getMemSize(const APCString* string) {
  return sizeof(APCString) + getMemSize(string->getStringData());
}

//////////////////////////////////////////////////////////////////////

/*
 * Stats and ODS API
 */

/*
 * Detailed stats about APC, controlled by a runtime config flag.
 */
struct APCDetailedStats {
  APCDetailedStats();

  // Return a formatted string with info about APC usage.
  std::string getStatsInfo() const;
  // adds stats to the given collection
  void collectStats(std::map<const StringData*, int64_t>& stats) const;

  // A new value was added to APC. This is a fresh value not replacing
  // an existing value. However the key may exists already a be a primed
  // mapped to file entry
  void addAPCValue(APCHandle* handle);
  // A new value replaces an existing one
  void updateAPCValue(APCHandle* handle, APCHandle* oldHandle, bool expired);
  // A value is removed. The entry may still exists if it is a primed entry
  void removeAPCValue(APCHandle* handle, bool expired);

private:
  void addType(APCHandle* handle);
  void removeType(APCHandle* handle);

private:
  /*
   * Data type counters
   */

  // Number of uncounted types, where uncounted means primitives
  // and static string
  ServiceData::ExportedCounter* m_uncounted;
  // Number of APC strings
  ServiceData::ExportedCounter* m_apcString;
  // Number of uncounted strings. Uncounted strings are kind of
  // static strings whose lifetime is controlled by the cache
  ServiceData::ExportedCounter* m_uncString;
  // Number of serialized arrays
  ServiceData::ExportedCounter* m_serArray;
  // Number of APC arrays
  ServiceData::ExportedCounter* m_apcArray;
  // Number of uncounted arrays. Uncounted arrays are kind of
  // static arrays whose lifetime is controlled by the cache
  ServiceData::ExportedCounter* m_uncArray;
  // Number of serialized objects
  ServiceData::ExportedCounter* m_serObject;
  // Number of APC objects
  ServiceData::ExportedCounter* m_apcObject;

  /*
   * Operation counters.
   */

  // Number of overall set values
  ServiceData::ExportedCounter* m_setValues;
  // Number of overall deleted operations
  ServiceData::ExportedCounter* m_delValues;
  // number of overall replaced values
  ServiceData::ExportedCounter* m_replValues;
  // Number of overall expired values
  ServiceData::ExportedCounter* m_expValues;
};

/*
 * Class that wraps the ODS counters and offers a simple API to
 * update the counters.
 * The counters are also used by the admin port by the PHP APC stats API
 * to get basic info about APC.
 */
struct APCStats {

  static APCStats& getAPCStats() {
    return *s_apcStats.get();
  }

  static void Create();

  APCStats();
  ~APCStats();

  // Return a formatted string with info about APC usage.
  std::string getStatsInfo() const;
  // adds stats to the given collection
  void collectStats(std::map<const StringData*, int64_t>& stats) const;

  // A new key is added. Value is added through addAPCValue()
  void addKey(size_t len) {
    assert(len > 0);
    m_entries->increment();
    m_keySize->addValue(len);
  }

  // A key is removed. Value is removed through removeAPCValue()
  void removeKey(size_t len) {
    assert(len > 0);
    m_entries->decrement();
    m_keySize->addValue(-len);
  }

  // A primed key is added. Implies a key is added as well.
  void addPrimedKey(size_t len) {
    assert(len > 0);
    m_primedEntries->increment();
    addKey(len);
  }

  // A value of a certain size was added to the primed set that is mapped
  // to file
  void addInFileValue(size_t size) {
    assert(size > 0);
    m_inFileSize->addValue(size);
  }

  // A new value was added to APC. This is a fresh value not replacing
  // an existing value. However the key may exists already a be a primed
  // mapped to file entry
  void addAPCValue(APCHandle* handle, size_t size, bool livePrimed) {
    assert(handle && size > 0);
    m_valueSize->addValue(size);
    if (livePrimed) {
      m_livePrimedSize->addValue(size);
      m_livePrimedEntries->increment();
    }
    if (m_detailedStats) {
      m_detailedStats->addAPCValue(handle);
    }
  }

  // A new value replaces an existing one
  void updateAPCValue(APCHandle* handle,
                      size_t size,
                      APCHandle* oldHandle,
                      size_t oldSize,
                      bool livePrimed,
                      bool expired) {
    assert(handle && size > 0 && oldHandle && oldSize > 0);
    auto diff = size - oldSize;
    if (diff != 0) {
      m_valueSize->addValue(diff);
      if (livePrimed) {
        m_livePrimedSize->addValue(diff);
      }
    }
    if (m_detailedStats) {
      m_detailedStats->updateAPCValue(handle, oldHandle, expired);
    }
  }

  // A value is removed. The entry may still exists if it is a primed entry
  void removeAPCValue(size_t size,
                      APCHandle* handle,
                      bool livePrimed,
                      bool expired) {
    assert(size > 0);
    m_valueSize->addValue(-size);
    if (livePrimed) {
      m_livePrimedSize->addValue(-size);
      m_livePrimedEntries->decrement();
    }
    if (m_detailedStats) {
      m_detailedStats->removeAPCValue(handle, expired);
    }
  }

  void addPendingDelete(size_t size) {
    m_pendingDeleteSize->addValue(size);
  }

  void removePendingDelete(size_t size) {
    m_pendingDeleteSize->addValue(-size);
  }

private:
  static std::unique_ptr<APCStats> s_apcStats;

private:
  /*
   * Memory size counters
   */

  // Keep track of the overall memory usage from all live values
  ServiceData::ExportedTimeSeries* m_valueSize;
  // Keep track of the overall memory usage of all keys
  ServiceData::ExportedTimeSeries* m_keySize;
  // Size of data stored in the paged out in memory file. This
  // is basically the size of the primed data that goes into the file
  ServiceData::ExportedTimeSeries* m_inFileSize;
  // Size of primed data that is brought to life, that is, that goes
  // into memory
  ServiceData::ExportedTimeSeries* m_livePrimedSize;
  // Size of the APC data pending deletes in the treadmill
  ServiceData::ExportedTimeSeries* m_pendingDeleteSize;

  /*
   * Number of entry counters
   */

  // Number of entries (keys)
  ServiceData::ExportedCounter* m_entries;
  // Number of primed entries
  ServiceData::ExportedCounter* m_primedEntries;
  // Number of live primed entries
  ServiceData::ExportedCounter* m_livePrimedEntries;

  // detailed info
  APCDetailedStats* m_detailedStats;

};

}

#endif

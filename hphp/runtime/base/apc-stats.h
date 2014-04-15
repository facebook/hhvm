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

#include "hphp/runtime/base/array-data.h"
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
 * Class that wraps the ODS counter and offers a simple API to
 * update the counters. ODS reporting is controlled by a runtime
 * option and thus the API may result into a no-op.
 */
struct APCStats {
  explicit APCStats()
    : m_valueSize(nullptr)
    , m_keySize(nullptr)
    , m_inFileSize(nullptr)
    , m_livePrimedSize(nullptr)
    , m_entries(nullptr)
    , m_primedEntries(nullptr)
    , m_inFileEntries(nullptr)
    , m_uncounted(nullptr)
    , m_apcString(nullptr)
    , m_uncString(nullptr)
    , m_serArray(nullptr)
    , m_apcArray(nullptr)
    , m_uncArray(nullptr)
    , m_serObject(nullptr)
    , m_apcObject(nullptr)
    , m_setValues(nullptr)
    , m_delValues(nullptr)
    , m_replValues(nullptr)
    , m_expValues(nullptr)
  {
    if (RuntimeOption::EnableAPCStats) {
      m_valueSize = ServiceData::createTimeseries(
          "apc.value_size", {ServiceData::StatsType::SUM});
      m_keySize = ServiceData::createTimeseries(
          "apc.key_size", {ServiceData::StatsType::SUM});
      m_inFileSize = ServiceData::createTimeseries(
          "apc.in_file_size", {ServiceData::StatsType::SUM});
      m_livePrimedSize = ServiceData::createTimeseries(
          "apc.primed_live_size", {ServiceData::StatsType::SUM});

      m_entries = ServiceData::createCounter("apc.entries");
      m_primedEntries = ServiceData::createCounter("apc.primed_entries");
      m_inFileEntries = ServiceData::createCounter("apc.in_file_entries");

      m_uncounted = ServiceData::createCounter("apc.type_uncounted");
      m_apcString = ServiceData::createCounter("apc.type_apc_string");
      m_uncString = ServiceData::createCounter("apc.type_unc_string");
      m_serArray = ServiceData::createCounter("apc.type_ser_array");
      m_apcArray = ServiceData::createCounter("apc.type_apc_array");
      m_uncArray = ServiceData::createCounter("apc.type_unc_array");
      m_serObject = ServiceData::createCounter("apc.type_ser_object");
      m_apcObject = ServiceData::createCounter("apc.type_apc_object");

      m_setValues = ServiceData::createCounter("apc.set_values");
      m_delValues = ServiceData::createCounter("apc.deleted_values");
      m_replValues = ServiceData::createCounter("apc.replaced_values");
      m_expValues = ServiceData::createCounter("apc.expired_values");
    }
  }

  void addKey(const char* key) {
    if (!m_entries) return;
    assert(key);
    m_entries->increment();
    m_keySize->addValue(strlen(key));
  }

  void removeKey(const char* key) {
    if (!m_entries) return;
    assert(key);
    m_entries->decrement();
    m_keySize->addValue(-strlen(key));
  }

  void addPrimedKey(const char* key) {
    if (!m_primedEntries) return;
    assert(key);
    m_primedEntries->increment();
    addKey(key);
  }

  size_t addAPCValue(APCHandle* handle, bool livePrimed) {
    if (!m_setValues) return 0;
    m_setValues->increment();
    auto size = getMemSize(handle);
    m_valueSize->addValue(size);
    if (livePrimed) {
      m_livePrimedSize->addValue(size);
    }
    addType(handle);
    return size;
  }

  size_t updateAPCValue(APCHandle* handle,
                        APCHandle* oldHandle,
                        size_t oldSize,
                        bool livePrimed,
                        bool expired) {
    if (!m_setValues) return 0;
    m_setValues->increment();
    auto size = getMemSize(handle);
    auto diff = size - oldSize;
    if (diff != 0) {
      m_valueSize->addValue(diff);
      if (livePrimed) {
        m_livePrimedSize->addValue(diff);
      }
    }
    removeType(oldHandle);
    addType(handle);
    if (expired) {
      m_expValues->increment();
    } else {
      m_replValues->increment();
    }
    return size;
  }

  void removeAPCValue(size_t size,
                      APCHandle* handle,
                      bool livePrimed,
                      bool expired) {
    if (!m_valueSize) return;
    assert(size > 0);
    m_valueSize->addValue(-size);
    if (livePrimed) {
      m_livePrimedSize->addValue(-size);
    }
    removeType(handle);
    if (expired) {
      m_expValues->increment();
    } else {
      m_delValues->increment();
    }
  }

  void addInFileValue(int64_t value) {
    if (!m_inFileEntries) return;
    assert(value > 0);
    m_inFileEntries->increment();
    m_inFileSize->addValue(value);
  }

  void removeInFileValue(int64_t value) {
    if (!m_inFileEntries) return;
    assert(value > 0);
    m_inFileEntries->decrement();
    m_inFileSize->addValue(-value);
  }

private:
  void addType(APCHandle* handle) {
    if (!m_valueSize) return;
    DataType type = handle->getType();
    assert(!IS_REFCOUNTED_TYPE(type) ||
           type == KindOfString ||
           type == KindOfArray ||
           type == KindOfObject);
    if (!IS_REFCOUNTED_TYPE(type)) {
      m_uncounted->increment();
      return;
    }
    switch (type) {
    case KindOfString:
      if (handle->getUncounted()) {
        m_uncString->increment();
      } else {
        m_apcString->increment();
      }
      return;
    case KindOfArray:
      if (handle->getUncounted()) {
        m_uncArray->increment();
      } else if (handle->getSerializedArray()) {
        m_serArray->increment();
      } else {
        m_apcArray->increment();
      }
      return;
    case KindOfObject:
      if (handle->getIsObj()) {
        m_apcObject->increment();
      } else {
        m_serObject->increment();
      }
      return;
    default:
      return;
    }
  }

  void removeType(APCHandle* handle) {
    if (!m_valueSize) return;
    DataType type = handle->getType();
    assert(!IS_REFCOUNTED_TYPE(type) ||
           type == KindOfString ||
           type == KindOfArray ||
           type == KindOfObject);
    if (!IS_REFCOUNTED_TYPE(type)) {
      m_uncounted->decrement();
      return;
    }
    switch (type) {
    case KindOfString:
      if (handle->getUncounted()) {
        m_uncString->decrement();
      } else {
        m_apcString->decrement();
      }
      return;
    case KindOfArray:
      if (handle->getUncounted()) {
        m_uncArray->decrement();
      } else if (handle->getSerializedArray()) {
        m_serArray->decrement();
      } else {
        m_apcArray->decrement();
      }
      return;
    case KindOfObject:
      if (handle->getIsObj()) {
        m_apcObject->decrement();
      } else {
        m_serObject->decrement();
      }
      return;
    default:
      return;
    }
  }

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

  /*
   * Number of entry counters
   */

  // Number of entries (keys)
  ServiceData::ExportedCounter* m_entries;
  // Number of entries that are for primed keys
  ServiceData::ExportedCounter* m_primedEntries;
  // Number of entries for the primed in file data
  ServiceData::ExportedCounter* m_inFileEntries;

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

}

#endif

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_JIT_REGION_PROF_COUNTERS_H_
#define incl_HPHP_JIT_REGION_PROF_COUNTERS_H_

#include "hphp/runtime/vm/jit/prof-counters.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/region-selection.h"

#include "folly/SharedMutex.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * RegionProfCounters keeps a list of counters associated with each given
 * region.

 * The intended use of these counters is the following.  When the JIT profile
 * data is going to be serialized, these counters are created and incremented
 * from JITed code, and then serialized along with the rest of the JIT profile
 * data.  When JIT profile data is deserialized, these counters can be retrieved
 * and used to set weights driving JIT optimizations based on data observed
 * during profiling.
 *
 * RegionProfCounters also keeps a piece of metadata associated with each
 * counter, which clients may use as they wish (e.g. to validate the counters).
 */
template<typename T, typename M>
struct RegionProfCounters {
  /*
   * Add a new counter associated with the given region and metadata,
   * and return the counters address.
   */
  T* addCounter(const RegionEntryKey& regionKey, const M& meta) {
    folly::SharedMutex::WriteHolder lock(m_lock);
    auto& indices = m_regionIndices[regionKey];
    auto const index = m_meta.size();
    m_meta.push_back(meta);
    indices.push_back(index);
    return m_counters.getAddr(index);
  }

  /*
   * Return a vector with the values of all the counters associated with the
   * given region, as well as a vector with the corresponding associated
   * metadata.
   */
  jit::vector<T> getCounters(const RegionEntryKey& regionKey,
                             jit::vector<M>& meta) const {
    folly::SharedMutex::ReadHolder lock(m_lock);
    meta.clear();
    jit::vector<T> counters;
    auto const it = m_regionIndices.find(regionKey);
    if (it == m_regionIndices.end()) return counters;
    auto const& indices = it->second;
    for (auto index : indices) {
      counters.push_back(m_counters.get(index));
      meta.push_back(m_meta[index]);
    }
    return counters;
  }

  /*
   * Free the memory used to keep all the counters.
   */
  void freeCounters() {
    folly::SharedMutex::WriteHolder lock(m_lock);
    m_meta.clear();
    m_regionIndices.clear();
    m_counters.clear();
  }

  void serialize(ProfDataSerializer& ser) {
    folly::SharedMutex::WriteHolder lock(m_lock);
    write_raw(ser, m_regionIndices.size());
    for (auto& it : m_regionIndices) {
      auto const& regionKey = it.first;
      auto const& indices   = it.second;
      write_regionkey(ser, regionKey);
      write_raw(ser, indices.size());
      for (auto index : indices) {
        // NB: counters currently start at 0 and go down, so flip the sign here
        auto const count = -m_counters.get(index);
        auto const& meta = m_meta[index];
        write_raw(ser, count);
        write_raw(ser, meta);
      }
    }
  }

  void deserialize(ProfDataDeserializer& des) {
    folly::SharedMutex::WriteHolder lock(m_lock);
    size_t index = 0;
    size_t nregions;
    read_raw(des, nregions);
    while (nregions--) {
      RegionEntryKey regionKey = read_regionkey(des);
      size_t nentries;
      read_raw(des, nentries);
      auto& indices = m_regionIndices[regionKey];
      while (nentries--) {
        T count;
        read_raw(des, count);
        M meta;
        read_raw(des, meta);
        *m_counters.getAddr(index) = count;
        assertx(m_meta.size() == index);
        m_meta.push_back(meta);
        indices.push_back(index);
        index++;
      }
    }
  }

 private:
  ProfCounters<T> m_counters{0};
  jit::vector<M>  m_meta;

  // Maps each region to the vector containing its corresponding indices in
  // m_counters and m_meta.
  hphp_hash_map<RegionEntryKey,jit::vector<size_t>,
                RegionEntryKey::Hash,RegionEntryKey::Eq> m_regionIndices;

  // This lock protects all concurrent accesses to the vectors and map above.
  mutable folly::SharedMutex m_lock;
};

///////////////////////////////////////////////////////////////////////////////

} }

#endif

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

#pragma once

#include "hphp/runtime/base/prof-counters.h"

#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/region-selection.h"

#include "folly/SharedMutex.h"

namespace HPHP { namespace jit {

using Indices = jit::vector<size_t>;

///////////////////////////////////////////////////////////////////////////////

/*
 * TransProfCounters keeps list of counters associated with each given
 * region or prologue.

 * The intended use of these counters is the following.  When the JIT profile
 * data is going to be serialized, these counters are created and incremented
 * from JITed code, and then serialized along with the rest of the JIT profile
 * data.  When JIT profile data is deserialized, these counters can be retrieved
 * and used to set weights driving JIT optimizations based on data observed
 * during profiling.
 *
 * TransProfCounters also keeps a piece of metadata associated with each
 * counter, which clients may use as they wish (e.g. to validate the counters).
 */
template<typename T, typename M>
struct TransProfCounters {

  // Helper methods for accessing indices
  Optional<Indices>
  findIndices(const PrologueID& pid) const {
    auto const it = m_prologueIndices.find(pid);
    if (it == m_prologueIndices.end()) return std::nullopt;
    return it->second;
  }
  Optional<Indices>
  findIndices(const RegionEntryKey& regionKey) const {
    auto const it = m_regionIndices.find(regionKey);
    if (it == m_regionIndices.end()) return std::nullopt;
    return it->second;
  }
  Optional<Indices>
  findIndices(const std::string& name) const {
    auto const it = m_uniqueStubIndices.find(name);
    if (it == m_uniqueStubIndices.end()) return std::nullopt;
    return it->second;
  }

  auto& getIndices(const PrologueID& pid) {
    return m_prologueIndices[pid];
  }
  auto& getIndices(const RegionEntryKey& regionKey) {
    return m_regionIndices[regionKey];
  }
  auto& getIndices(const std::string& name) {
    return m_uniqueStubIndices[name];
  }

  /*
   * Add a new counter associated with the given region or prologue,
   * and metadata, and return the counters address.
   */
  template <typename K>
  T* addCounter(const K& key, const M& meta) {
    std::unique_lock lock(m_lock);
    auto& indices = getIndices(key);
    auto const index = m_meta.size();
    m_meta.push_back(meta);
    indices.push_back(index);
    return m_counters.getAddr(index);
  }

  /*
   * Retrieve the first counter associated with the given region. We typically
   * keep counters sorted by e.g. RPO order, so the first one is for the entry.
   */
  template <typename K>
  Optional<T> getFirstCounter(const K& key) {
    folly::SharedMutex::ReadHolder lock(m_lock);
    auto const& opt = findIndices(key);
    if (opt == std::nullopt) return std::nullopt;
    auto const& indices = opt.value();
    if (indices.empty()) return std::nullopt;
    return m_counters.get(indices[0]);
  }

  /*
   * Return a vector with the values of all the counters associated with the
   * given region, as well as a vector with the corresponding associated
   * metadata.
   */
  template <typename K>
  jit::vector<T> getCounters(const K& key, jit::vector<M>& meta) const {
    folly::SharedMutex::ReadHolder lock(m_lock);
    meta.clear();
    jit::vector<T> counters;
    auto const& opt = findIndices(key);
    if (opt == std::nullopt) return counters;
    auto const& indices = opt.value();
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
    std::unique_lock lock(m_lock);
    m_meta.clear();
    m_regionIndices.clear();
    m_prologueIndices.clear();
    m_uniqueStubIndices.clear();
    m_counters.clear();
  }

  void serialize(ProfDataSerializer& ser) {
    std::unique_lock lock(m_lock);
    auto write_counters = [&](const Indices& indices) {
      write_raw(ser, indices.size());
      for (auto index : indices) {
        // NB: counters currently start at 0 and go down, so flip the sign here
        auto const count = -m_counters.get(index);
        auto const& meta = m_meta[index];
        write_raw(ser, count);
        write_raw(ser, meta);
      }
    };
    write_raw(ser, m_regionIndices.size());
    for (auto& it : m_regionIndices) {
      auto const& regionKey = it.first;
      auto const& indices   = it.second;
      write_regionkey(ser, regionKey);
      write_counters(indices);
    }
    write_raw(ser, m_prologueIndices.size());
    for (auto& it : m_prologueIndices) {
      auto const& pid       = it.first;
      auto const& indices   = it.second;
      write_prologueid(ser, pid);
      write_counters(indices);
    }
    write_raw(ser, m_uniqueStubIndices.size());
    for (auto& it : m_uniqueStubIndices) {
      auto const& name      = it.first;
      auto const& indices   = it.second;
      write_string(ser, name);
      write_counters(indices);
    }
  }

  void deserialize(ProfDataDeserializer& des) {
    std::unique_lock lock(m_lock);
    auto deserialize_counters = [&]
      (Indices& indices, size_t& index)
    {
      size_t nentries;
      read_raw(des, nentries);
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
    };
    size_t index = 0;
    size_t nregions;
    read_raw(des, nregions);
    while (nregions--) {
      RegionEntryKey regionKey = read_regionkey(des);
      auto& indices = m_regionIndices[regionKey];
      deserialize_counters(indices, index);
    }
    size_t nprologues;
    read_raw(des, nprologues);
    while (nprologues--) {
      PrologueID pid = read_prologueid(des);
      auto& indices = m_prologueIndices[pid];
      deserialize_counters(indices, index);
    }
    size_t nstubs;
    read_raw(des, nstubs);
    while (nstubs--) {
      auto const name = read_cpp_string(des);
      auto& indices = m_uniqueStubIndices[name];
      deserialize_counters(indices, index);
    }
  }

 private:
  ProfCounters<T> m_counters{0};
  jit::vector<M>  m_meta;

  // Maps each region to the vector containing its corresponding indices in
  // m_counters and m_meta.
  hphp_hash_map<RegionEntryKey,Indices,
                RegionEntryKey::Hash,RegionEntryKey::Eq> m_regionIndices;

  // Map for prologues.
  hphp_hash_map<PrologueID,Indices,
                PrologueID::Hasher,PrologueID::Eq> m_prologueIndices;

  // Map for unique stubs.
  hphp_string_map<Indices> m_uniqueStubIndices;

  // This lock protects all concurrent accesses to the vectors and map above.
  mutable folly::SharedMutex m_lock;
};

///////////////////////////////////////////////////////////////////////////////

} }

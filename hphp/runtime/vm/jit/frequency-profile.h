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

#ifndef incl_HPHP_JIT_FREQUENCY_PROFILE_H_
#define incl_HPHP_JIT_FREQUENCY_PROFILE_H_

#include "hphp/util/assertions.h"
#include "hphp/util/safe-cast.h"

#include <folly/Format.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <sstream>


namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

// This profile tracks N high-frequency values in a stream of values of type T.
// Clients must choose a special `Missing` value of type T that is not tracked.
template<typename T, uint32_t N, T Missing>
struct FrequencyProfile {
  // A single element of the frequency profile. Works best if sizeof(T) == 4.
  struct Entry {
    uint32_t count{0};
    T value{Missing};
  };

public:
  // Returns the count and value of the most frequent value seen so far,
  // or nullptr if we haven't seen any values.
  const Entry* choose() const;

  // Merge another profile into this profile, keeping the top N values.
  void reduce(const FrequencyProfile& other);

  // Update the profile. Note that missing values only count towards the total.
  // Returns true if the given value is one of our N tracked values.
  bool update(T value, uint32_t count);

  uint32_t total() const { return m_total; }

  std::string toString() const;

private:
  // Basic consistency checks run in debug mode.
  bool checkInvariants() const;

  // RDS locals are zero-initialized, so we need to do lazy initialization.
  void initialize();

  // Check if the profiles are initialized. Accounts for zero-initialization.
  bool initialized() const { return m_total > 0; };

public:
  // Logically private, but exposed for custom serialization.
  Entry m_entries[N];
  uint32_t m_total{0};
};

///////////////////////////////////////////////////////////////////////////////

template<typename T, uint32_t N, T Missing>
const typename FrequencyProfile<T, N, Missing>::Entry*
FrequencyProfile<T, N, Missing>::choose() const {
  assertx(checkInvariants());
  if (!initialized()) return nullptr;

  auto hottest = &m_entries[0];
  for (auto const& entry : m_entries) {
    if (entry.value == Missing) break;
    if (entry.count > hottest->count) hottest = &entry;
  }

  return hottest->value == Missing ? nullptr : hottest;
}

template<typename T, uint32_t N, T Missing>
void FrequencyProfile<T, N, Missing>::reduce(const FrequencyProfile& other) {
  assertx(checkInvariants());
  assertx(other.checkInvariants());
  if (!other.initialized()) return;
  if (!initialized()) initialize();
  auto const total = m_total + other.m_total;

  auto n = uint32_t{0};
  Entry scratch[2 * N];
  for (auto const& entry : other.m_entries) {
    if (entry.value == Missing) break;

    // If we track the value, update its count; else, save it to scratch.
    if (!update(entry.value, entry.count)) {
      scratch[n++] = entry;
    }
  }

  // Undo the changes to m_total from update. We want to use the simpler
  // computation `m_total + other.m_total` which includes untracked values.
  m_total = total;

  if (n == 0) return;
  assertx(n <= N);

  // Sort the combined samples, then copy the top hits back into `m_entries'.
  std::memcpy(&scratch[n], &m_entries[0], N * sizeof(Entry));
  std::sort(&scratch[0], &scratch[n + N],
            [] (auto a, auto b) { return a.count > b.count; });
  std::memcpy(m_entries, scratch, N * sizeof(Entry));
}

template<typename T, uint32_t N, T Missing>
bool FrequencyProfile<T, N, Missing>::update(T value, uint32_t count) {
  assertx(checkInvariants());
  if (!initialized()) initialize();
  m_total += count;
  if (count == 0 || value == Missing) return false;

  for (auto& entry : m_entries) {
    if (entry.value == value || entry.value == Missing) {
      entry.count += count;
      entry.value = value;
      return true;
    }
  }

  return false;
}

template<typename T, uint32_t N, T Missing>
std::string FrequencyProfile<T, N, Missing>::toString() const {
  assertx(checkInvariants());
  if (!initialized()) return "uninitialized";
  std::ostringstream out;
  for (auto const& entry : m_entries) {
    out << folly::format("{}:{},", entry.value, entry.count);
  }
  out << folly::format("total:{}", m_total);
  return out.str();
}

template<typename T, uint32_t N, T Missing>
bool FrequencyProfile<T, N, Missing>::checkInvariants() const {
  if (!initialized()) return true;
  auto total_tracked = 0;
  auto found_empty_entry = false;
  for (auto i = 0; i < N; i++) {
    auto const& entry = m_entries[i];
    total_tracked += entry.count;
    if (entry.value == Missing) {
      // Missing values should always have a count of 0.
      assertx(entry.count == 0);
      found_empty_entry = true;
    } else {
      // Tracked values should have a non-zero count and should be distinct.
      // All tracked values should come before any empty slots.
      assertx(entry.count > 0);
      assertx(!found_empty_entry);
      for (auto j = i + 1; j < N; j++) {
        assertx(entry.value != m_entries[j].value);
      }
    }
  }
  assertx(total_tracked <= m_total);
  return true;
}

template<typename T, uint32_t N, T Missing>
void FrequencyProfile<T, N, Missing>::initialize() {
  assertx(!initialized());
  for (auto& entry : m_entries) entry.value = Missing;
}

}}

#endif

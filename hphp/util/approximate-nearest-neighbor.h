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

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include <folly/Hash.h>

#include "hphp/util/assertions.h"
#include "hphp/util/hash.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/hash-set.h"
#include "hphp/util/optional.h"
#include "hphp/util/tiny-vector.h"

///////////////////////////////////////////////////////////////////////////////

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * ApproximateNearestNeighbor: MinHash-based Locality Sensitive Hashing (LSH)
 * for finding similar sets efficiently.
 *
 * This data structure implements the MinHash algorithm to find sets with high
 * Jaccard similarity. Given a collection of sets, it can quickly identify which
 * sets are most similar to a query set without comparing all pairs.
 *
 * Algorithm Overview:
 * -------------------
 * MinHash uses the property that for two sets A and B, the probability that
 * their minimum hash values are equal is exactly their Jaccard similarity:
 *   P(min_hash(A) == min_hash(B)) = |A ∩ B| / |A ∪ B|
 *
 * We use multiple hash functions (rounds) to create a "sketch" of each set,
 * then combine sketches into a signature. Sets with similar signatures are
 * placed in the same bucket (experiment). By running multiple independent
 * experiments, we increase the probability of finding similar sets while
 * reducing false positives.
 *
 * Key Properties:
 * ---------------
 * - Probabilistic: May miss some similar sets or report dissimilar ones
 * - Fast: O(k*r) per query vs O(n) brute force (k=experiments, r=rounds, n=sets)
 * - Configurable: Trade accuracy for speed via similarity/probability bounds
 * - Space-efficient: Stores compact sketches rather than full sets
 *
 * Template Parameters:
 * --------------------
 * E - ElemId type: Unsigned integral type for element identifiers
 * I - SetId type: Unsigned integral type for set identifiers
 *
 * Usage Example:
 * --------------
 *   // Find sets with >70% similarity (90% probability) but not <40% (10% prob)
 *   std::mt19937 prng(42);
 *   ApproximateNearestNeighbor<uint32_t, uint32_t> ann(
 *     prng,
 *     {0.4, 0.1},  // Lower bound: 40% similarity, 10% probability
 *     {0.7, 0.9},  // Upper bound: 70% similarity, 90% probability
 *     10000,       // Max element ID
 *     1000         // Number of sets
 *   );
 *
 *   // Add sets (element IDs in each set)
 *   std::vector<uint32_t> set1 = {1, 5, 7, 10, 15};
 *   std::vector<uint32_t> set2 = {1, 5, 8, 11, 16};
 *   ann.add(0, set1.begin(), set1.end());
 *   ann.add(1, set2.begin(), set2.end());
 *
 *   // Find nearest neighbor
 *   auto nearest = ann.nearest(0);  // Find set most similar to set 0
 *
 * Performance Characteristics:
 * ----------------------------
 * - Construction: O(E + S) where E=numElems, S=numSets
 * - Add set: O(k * r * |set|) where k=experiments, r=rounds, |set|=set size
 * - Query: O(k * bucket_size) typically much smaller than O(S)
 * - Space: O(S * k + total_elements_across_sets)
 *
 * Thread Safety:
 * --------------
 * This data structure is NOT thread-safe without external synchronization:
 *
 * - Concurrent queries (nearest(), all()) from multiple threads: SAFE
 *   All query methods are const and only read shared state.
 *
 * - Concurrent adds with add(), preadd(): UNSAFE
 *   These methods modify multiple experiments and require external locking.
 *
 * - addByExperiment(): SAFE across different (SetId, experimentNum) pairs
 *   Multiple threads can call addByExperiment() concurrently as long as each
 *   thread works on a different (SetId, experimentNum) combination. This allows
 *   parallelization across both sets and experiments.
 *
 * - addWithToken(): SAFE across different SetIds. Multiple threads
 *   can call addWithToken() concurrently as long as each thread works
 *   on a different SetId.
 *
 * - Querying while adding: UNSAFE
 *   Queries may observe partially-constructed state or trigger data races.
 *
 * Recommended usage patterns:
 * 1. Single-threaded construction, then multi-threaded queries (safest)
 *
 * 2. External mutex protecting all add operations, queries can proceed
 *    without lock
 *
 * 3. For parallel construction: call preadd() for all sets, then parallelize
 *    addByExperiment() or addWithToken() ensuring each thread works on
 *    different (SetId, experimentNum) pairs (e.g., partition by experiment
 *    number)
 */

template <typename E, typename I>
struct ApproximateNearestNeighbor {
private:
  using HashType = uint64_t;
  using Sketch = std::vector<HashType>;

public:
  using ElemId = E;
  using SetId = I;

  static_assert(std::is_integral_v<ElemId> && !std::is_signed_v<ElemId>);
  static_assert(std::is_integral_v<SetId> && !std::is_signed_v<SetId>);

  struct Bound;
  struct Counter;

  // Non-copyable, non-movable due to internal pointers and expensive state
  ApproximateNearestNeighbor(const ApproximateNearestNeighbor&) = delete;
  ApproximateNearestNeighbor(ApproximateNearestNeighbor&&) = delete;
  ApproximateNearestNeighbor& operator=(const ApproximateNearestNeighbor&) = delete;
  ApproximateNearestNeighbor& operator=(ApproximateNearestNeighbor&&) = delete;

  /*
   * Construct a new ApproximateNearestNeighbor data structure.
   *
   * The lower and higher bounds define the similarity detection range:
   * - Sets with similarity >= higher.similarity will be found with probability
   *   >= higher.probability
   * - Sets with similarity <= lower.similarity will be found with probability
   *   <= lower.probability
   *
   * The algorithm automatically determines the number of experiments (k) and
   * rounds per experiment (r) to satisfy these bounds.
   *
   * Parameters:
   *   prng - Pseudo-random number generator for hash function initialization.
   *          Must satisfy UniformRandomBitGenerator concept.
   *   lower - Lower bound on similarity/probability. Sets this dissimilar
   *           should rarely be reported as neighbors.
   *   higher - Upper bound on similarity/probability. Sets this similar
   *            should usually be reported as neighbors.
   *   numElems - Maximum element ID + 1. All element IDs must be < numElems.
   *   numSets - Total number of sets that will be stored.
   *
   * Time: O(numElems + numSets + k*r) where k and r are computed from bounds
   * Space: O(numSets * k)
   */
  template <typename R>
  ApproximateNearestNeighbor(R&& prng,
                             Bound lower,
                             Bound higher,
                             size_t numElems,
                             size_t numSets);

  /*
   * Returns the number of independent experiments (k).
   * More experiments = higher probability of finding similar sets.
   */
  size_t numExperiments() const { return perExperiment.size(); }

  /*
   * Returns the number of hash rounds per experiment (r).
   * More rounds = better discrimination between different similarity levels.
   */
  size_t roundsPerExperiment() const { return numRounds; }

  /*
   * Add a set to the data structure.
   *
   * The set is represented as a sequence of element IDs in [begin, end).
   * All element IDs must be < numElems. The set ID must be < numSets and
   * must not have been used before.
   *
   * This is the simplest way to add a set - it computes the sketch and
   * inserts it into all experiments in one call.
   *
   * Parameters:
   *   id - Unique identifier for this set
   *   begin, end - Iterator range of ElemId values in the set
   *
   * Time: O(k * r * |set|) where k=experiments, r=rounds, |set|=set size
   */
  template <typename It>
  void add(SetId id, It begin, It end);

  /*
   * Pre-allocate storage for a set before adding it incrementally.
   *
   * Call this before using addByExperiment() or addWithToken() for a given
   * set ID. This reserves space in the sketch vector.
   *
   * Parameters:
   *   id - Unique identifier for this set
   *
   * Time: O(k) where k=experiments
   */
  void preadd(SetId id);

  /*
   * Add a set to a specific experiment only.
   *
   * This is useful for incremental construction or when you want to
   * parallelize across experiments. Must call preadd() first.
   *
   * Parameters:
   *   id - Set identifier (must have called preadd(id) first)
   *   experimentNum - Which experiment to add to (0 <= experimentNum < k)
   *   begin, end - Iterator range of ElemId values in the set
   *
   * Time: O(r * |set|) where r=rounds, |set|=set size
   */
  template <typename It>
  void addByExperiment(SetId id, size_t experimentNum, It begin, It end);

  /*
   * Token: Precomputed sketch for a set.
   *
   * A token stores the MinHash sketches for all experiments without storing
   * the token in the data structure. This allows you to compute a sketch once
   * and reuse it to add the same set to multiple data structures, or to defer
   * insertion until later.
   */
  struct Token {
  private:
    Sketch sketch;
    friend struct ApproximateNearestNeighbor;
  };

  /*
   * Create a reusable token for a set without adding it.
   *
   * The token can be passed to addWithToken() later. This is useful when
   * you want to compute the sketch once but add it to multiple data structures
   * or experiments at different times.
   *
   * Parameters:
   *   begin, end - Iterator range of ElemId values in the set
   *
   * Returns: Token containing the precomputed sketch
   *
   * Time: O(k * r * |set|) where k=experiments, r=rounds, |set|=set size
   */
  template <typename It> Token makeToken(It begin, It end) const;

  /*
   * Add a set using a precomputed token.
   *
   * The token must have been created by makeToken() using the same data
   * structure (same hash functions). Must call preadd() first.
   *
   * Parameters:
   *   id - Set identifier (must have called preadd(id) first)
   *   experimentNum - Which experiment to add to (0 <= experimentNum < k)
   *   token - Precomputed sketch from makeToken()
   *
   * Time: O(1) amortized
   */
  void addWithToken(SetId id, size_t experimentNum, const Token&);

  /*
   * Find the nearest neighbor to a given set.
   *
   * This version allows providing a reusable Counter and a filter predicate.
   * The filter is called for each candidate set; return false to exclude it.
   *
   * Parameters:
   *   id - Query set identifier
   *   counts - Reusable counter (passed by reference for efficiency)
   *   filter - Predicate called as filter(SetId) to exclude candidates
   *
   * Returns: SetId of the nearest neighbor, or nullopt if none found
   *
   * Time: O(k * bucket_size) where bucket_size is typically << numSets
   */
  template <typename F>
  Optional<SetId> nearest(SetId id, Counter& counts, F&& filter) const;

  /*
   * Find the nearest neighbor to a given set (simplified version).
   *
   * This is equivalent to nearest(id, counts, [](SetId) { return true; })
   * with a local Counter.
   *
   * Parameters:
   *   id - Query set identifier
   *
   * Returns: SetId of the nearest neighbor, or nullopt if none found
   *
   * Time: O(k * bucket_size) where bucket_size is typically << numSets
   */
  Optional<SetId> nearest(SetId id) const {
    Counter counts;
    return nearest(id, counts, [] (SetId) { return true; });
  }

  /*
   * Report all potential neighbors of a given set.
   *
   * Calls report(SetId) for each set that shares a bucket with the query
   * set in at least one experiment. This may include duplicates (same set
   * reported multiple times if it shares multiple buckets).
   *
   * Parameters:
   *   id - Query set identifier
   *   report - Callable invoked as report(SetId) for each candidate
   *
   * Time: O(k * bucket_size) where bucket_size is typically << numSets
   */
  template <typename F> void all(SetId id, F&& report) const;

private:
  // Maximum element ID + 1 (all element IDs must be < numElems)
  const size_t numElems;

  // Number of hash rounds per experiment (r parameter)
  const size_t numRounds;

  /*
   * PerExperiment: Data for one independent experiment.
   *
   * Each experiment uses different hash functions and maintains its own
   * hash table. Sets with matching sketches are placed in the same bucket.
   */
  struct PerExperiment {
    using Bucket = TinyVector<SetId, 1>;
    // Maps sketch hash -> list of SetIds with that sketch
    hphp_fast_map<uint64_t, Bucket> buckets;
  };
  std::vector<PerExperiment> perExperiment;  // k experiments

  /*
   * PerSet: Data associated with one set.
   *
   * Stores the sketch (one hash per experiment) and whether the set
   * has been added yet.
   */
  struct PerSet {
    Sketch sketch;       // One hash value per experiment
    bool present{false}; // Has this set been added?
  };
  std::vector<PerSet> perSet;  // One entry per set ID

  // Number of bits to shift element IDs when combining with hash index
  size_t elemShift;

  // Tabulation hashing for fast, independent hash functions
  TabulationHash<HashType, HashType, uint8_t, 2> hasher;

  /*
   * Compute the number of rounds (r) and experiments (k) needed to satisfy
   * the given similarity/probability bounds.
   *
   * Returns: pair<rounds, experiments>
   */
  static std::pair<uint64_t, uint64_t> findSizes(Bound, Bound);
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Bound: Defines a similarity/probability constraint.
 *
 * Used to specify the desired behavior of the MinHash algorithm:
 * - similarity: Jaccard similarity threshold (0.0 to 1.0)
 * - probability: Detection probability (0.0 to 1.0, exclusive)
 *
 * Interpretation:
 *   "Sets with Jaccard similarity >= s should be detected with probability >= p"
 *
 * Example:
 *   Bound{0.7, 0.9} means: Sets with 70% similarity should be found 90% of the time
 *   Bound{0.4, 0.1} means: Sets with 40% similarity should be found at most 10% of the time
 *
 * The constructor uses two Bounds (lower and upper) to define a range:
 * - Upper bound: minimum similarity we want to reliably detect
 * - Lower bound: maximum similarity we want to mostly ignore
 */
template <typename E, typename I>
struct ApproximateNearestNeighbor<E, I>::Bound {
  double similarity;   // Jaccard similarity: |A ∩ B| / |A ∪ B|
  double probability;  // Detection probability

  Bound(double s, double p)
    : similarity{s}
    , probability{p}
  {
    always_assert(similarity >= 0.0 && similarity <= 1.0);
    always_assert(probability > 0.0 && probability < 1.0);
  }
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Counter: Efficient counter with amortized O(1) reset.
 *
 * This counter is used by nearest() to count how many buckets each candidate
 * set shares with the query set. It provides:
 * - O(1) increment (bump)
 * - O(1) query (operator[], contains)
 * - O(1) amortized reset across multiple queries
 * - Automatic tracking of the maximum count
 *
 * The amortized reset is achieved using a "base" offset: instead of zeroing
 * the entire table on reset, we just increase the base and treat values below
 * the base as zero. Only when base gets too large do we actually clear.
 *
 * Usage:
 *   Counter counts;
 *   counts.reset(numSets);       // Prepare for a new query
 *   counts.bump(42);             // Increment counter for set 42
 *   counts.bump(42);             // Increment again
 *   size_t count = counts[42];   // Get count (returns 2)
 *   size_t best = counts.biggest(); // Get ID with highest count
 */
template <typename E, typename I>
struct ApproximateNearestNeighbor<E, I>::Counter {
  /*
   * Get the current count for a given set ID.
   *
   * Returns 0 if the set hasn't been bumped since the last reset.
   */
  uint64_t operator[](size_t idx) const {
    assertx(idx < table.size());
    auto const& c = table[idx];
    return (c < base) ? 0 : (c - base);
  }

  /*
   * Check if a set has been bumped at least once since the last reset.
   */
  bool contains(size_t idx) const { return (*this)[idx] > 0; }

  /*
   * Increment the counter for a given set ID.
   *
   * Also updates the tracking of which set has the maximum count.
   */
  void bump(size_t idx) {
    assertx(idx < table.size());
    auto& c = table[idx];
    if (c < base) c = base;
    auto const real = ++c - base;
    assertx(real > 0);
    if (real < max) return;
    if (real > max || (real == max && idx < maxIdx)) {
      max = real;
      maxIdx = idx;
    }
  }

  /*
   * Get the set ID with the highest count.
   *
   * Returns the SetId that has been bumped the most times since reset.
   * If multiple sets are tied, returns the one with the smallest ID.
   */
  size_t biggest() const { return maxIdx; }

  /*
   * Reset the counter for a new query.
   *
   * This is amortized O(1): usually just increments the base, but
   * occasionally clears the table when base gets too large.
   *
   * Parameters:
   *   size - Number of sets (size of the counter table)
   */
  void reset(size_t size) {
    base += max;
    max = 0;
    maxIdx = std::numeric_limits<size_t>::max();
    if (size != table.size() ||
        base >= std::numeric_limits<uint64_t>::max()/2) {
      base = 0;
      table.clear();
      table.resize(size, 0);
    }
  }

private:
  std::vector<uint64_t> table;  // Actual counters
  uint64_t base{0};             // Offset for amortized reset
  uint64_t max{0};              // Maximum count seen
  size_t maxIdx{std::numeric_limits<size_t>::max()}; // ID with max count
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Constructor implementation.
 *
 * Initializes the MinHash data structure with the specified parameters.
 * The findSizes() function determines the optimal number of rounds and
 * experiments to satisfy the probability bounds.
 */
template <typename E, typename I>
template <typename R>
ApproximateNearestNeighbor<E, I>::ApproximateNearestNeighbor(R&& prng,
                                                             Bound lower,
                                                             Bound higher,
                                                             size_t numElems,
                                                             size_t numSets)
  : numElems{numElems}
  , numRounds{findSizes(lower, higher).first}
  , perExperiment{findSizes(lower, higher).second}
  , perSet{numSets}
  , elemShift{static_cast<size_t>(std::bit_width(numElems-1))}
  , hasher{prng}
{
  always_assert(numElems > 0);
  always_assert(numSets > 0);
  always_assert_flog(
    std::bit_width(numSets-1) <= std::numeric_limits<SetId>::digits,
    "Use a larger SetId type for this many sets"
  );
  always_assert_flog(
    elemShift <= std::numeric_limits<ElemId>::digits,
    "Use a larger ElemId type for this many elements"
  );

  auto const numHashes = perExperiment.size() * numRounds;
  always_assert(
    std::bit_width(numHashes-1) + elemShift <=
    std::numeric_limits<HashType>::digits
  );
}

/*
 * Add a set to all experiments.
 *
 * This is the all-in-one method that handles both preadd and per-experiment
 * insertion in a single call.
 */
template <typename E, typename I>
template <typename It>
void ApproximateNearestNeighbor<E, I>::add(SetId id, It begin, It end) {
  assertx(id < perSet.size());
  assertx(!perSet[id].present);
  preadd(id);
  for (size_t i = 0; i < perExperiment.size(); ++i) {
    addByExperiment(id, i, begin, end);
  }
  assertx(perSet[id].present);
}

/*
 * Pre-allocate space for a set's sketch.
 */
template <typename E, typename I>
void ApproximateNearestNeighbor<E, I>::preadd(SetId id) {
  assertx(id < perSet.size());
  auto& set = perSet[id];
  assertx(!set.present);
  assertx(set.sketch.empty());
  set.sketch.resize(perExperiment.size());
  set.present = true;
}

/*
 * Add a set to a specific experiment.
 *
 * Algorithm:
 * 1. For each of r rounds, compute the minimum hash value across all elements
 * 2. Combine these r minimum hashes into a single signature hash
 * 3. Insert the set into the bucket for this signature
 *
 * The key insight: if two sets are similar (high Jaccard similarity), they're
 * likely to have the same minimum hash value in each round, so they'll end up
 * in the same bucket.
 */
template <typename E, typename I>
template <typename It>
void ApproximateNearestNeighbor<E, I>::addByExperiment(SetId id,
                                                       size_t experimentNum,
                                                       It begin,
                                                       It end) {
  assertx(experimentNum < perExperiment.size());
  assertx(id < perSet.size());

  auto& set = perSet[id];
  assertx(set.present);
  assertx(set.sketch.size() == perExperiment.size());

  auto& experiment = perExperiment[experimentNum];
  HashType hash = 0;
  // For each round, compute the minimum hash of all elements
  for (size_t r = 0; r < numRounds; ++r) {
    uint64_t hashIdx = experimentNum * numRounds + r;
    auto min = std::numeric_limits<HashType>::max();
    for (auto it = begin; it != end; ++it) {
      const ElemId e = *it;
      assertx(e < numElems);
      // Hash combines the round index and element ID for independence
      min = std::min<HashType>(min, hasher((hashIdx << elemShift) | e));
    }
    // Combine all rounds' minimum hashes into a signature
    hash = folly::hash::hash_combine(hash, min);
  }

  // Store the signature and add this set to the corresponding bucket
  set.sketch[experimentNum] = hash;
  auto& list = experiment.buckets[hash];
  list.emplace_back(id);
}

/*
 * Create a reusable token containing the sketch for a set.
 *
 * This is essentially the same algorithm as addByExperiment, but we compute
 * all experiments and store them in a token instead of inserting into buckets.
 */
template <typename E, typename I>
template <typename It>
ApproximateNearestNeighbor<E, I>::Token
ApproximateNearestNeighbor<E, I>::makeToken(It begin, It end) const {
  Token out;
  out.sketch.reserve(perExperiment.size());

  for (size_t i = 0, size = perExperiment.size(); i < size; ++i) {
    HashType hash = 0;
    for (size_t r = 0; r < numRounds; ++r) {
      HashType hashIdx = i * numRounds + r;
      auto min = std::numeric_limits<HashType>::max();
      for (auto it = begin; it != end; ++it) {
        const ElemId e = *it;
        assertx(e < numElems);
        min = std::min<HashType>(min, hasher((hashIdx << elemShift) | e));
      }
      hash = folly::hash::hash_combine(hash, min);
    }
    out.sketch.emplace_back(hash);
  }

  return out;
}

/*
 * Add a set using a precomputed token.
 *
 * This is much faster than addByExperiment since the sketch is already
 * computed - we just need to insert into the bucket.
 */
template <typename E, typename I>
void ApproximateNearestNeighbor<E, I>::addWithToken(SetId id,
                                                    size_t experimentNum,
                                                    const Token& token) {
  assertx(experimentNum < perExperiment.size());
  assertx(id < perSet.size());

  auto& set = perSet[id];
  assertx(set.present);
  assertx(set.sketch.size() == perExperiment.size());
  assertx(token.sketch.size() == perExperiment.size());

  auto& experiment = perExperiment[experimentNum];
  auto const h = token.sketch[experimentNum];
  set.sketch[experimentNum] = h;
  auto& list = experiment.buckets[h];
  list.emplace_back(id);
}

/*
 * Find the nearest neighbor to a given set.
 *
 * Algorithm:
 * 1. For each experiment, look up the query set's signature
 * 2. Find all sets in that bucket (candidates with matching signature)
 * 3. Count how many buckets each candidate appears in (using Counter)
 * 4. Return the candidate that appears in the most buckets
 *
 * The intuition: truly similar sets will match in multiple experiments,
 * while false positives will typically match in only one or two.
 *
 * The filter predicate is called at most once per candidate set to avoid
 * redundant evaluations if the filter is expensive.
 */
template <typename E, typename I>
template <typename F>
Optional<typename ApproximateNearestNeighbor<E, I>::SetId>
ApproximateNearestNeighbor<E, I>::nearest(SetId id,
                                          Counter& counts,
                                          F&& filter) const {
  assertx(id < perSet.size());
  auto& set = perSet[id];
  assertx(set.present);
  assertx(set.sketch.size() == perExperiment.size());

  // Track sets that have been rejected by the filter to avoid re-evaluation
  hphp_fast_set<SetId> ignored;

  // Count how many experiments each candidate set matches in
  counts.reset(perSet.size());
  for (size_t i = 0, size = perExperiment.size(); i < size; ++i) {
    auto const s = set.sketch[i];
    assertx(perExperiment[i].buckets.contains(s));

    // For each set in this bucket
    for (auto const oid : perExperiment[i].buckets.at(s)) {
      assertx(oid < perSet.size());
      if (oid == id) continue;  // Skip self

      // Skip sets that have already been filtered out
      if (ignored.contains(oid)) continue;

      // Apply filter on first encounter
      if (!counts.contains(oid)) {
        if (!filter(oid)) {
          ignored.emplace(oid);  // Remember this set was filtered out
          continue;
        }
      }
      counts.bump(oid);  // Increment match count
    }
  }

  // Return the set with the most matches
  auto const b = counts.biggest();
  if (b < perSet.size()) return SetId(b);
  return std::nullopt;
}

/*
 * Report all potential neighbors.
 *
 * This simply enumerates all sets that share at least one bucket with the
 * query set, without counting or filtering. Useful for getting all candidates
 * for further processing.
 *
 * Note: the same set may be reported multiple times. The caller
 * should do any necessary de-duplication.
 */
template <typename E, typename I>
template <typename F>
void ApproximateNearestNeighbor<E, I>::all(SetId id, F&& report) const {
  assertx(id < perSet.size());
  auto& set = perSet[id];
  assertx(set.present);
  assertx(set.sketch.size() == perExperiment.size());

  for (size_t i = 0, size = perExperiment.size(); i < size; ++i) {
    auto const s = set.sketch[i];
    assertx(perExperiment[i].buckets.contains(s));
    for (auto const oid : perExperiment[i].buckets.at(s)) {
      assertx(oid < perSet.size());
      if (oid == id) continue;
      report(oid);  // May report same set multiple times
    }
  }
}

/*
 * Compute the number of rounds (r) and experiments (k) to satisfy bounds.
 *
 * This solves for r and k such that:
 *   P(match | similarity s) = 1 - (1 - s^r)^k
 *
 * We want:
 *   P(match | s=higher.similarity) >= higher.probability
 *   P(match | s=lower.similarity) <= lower.probability
 *
 * The algorithm tries increasing values of r until it finds a value where
 * there exists a valid k satisfying both constraints.
 *
 * Mathematical background:
 * - For r rounds, P(all r min-hashes match) = s^r
 * - For k experiments, P(at least one matches) = 1 - (1 - s^r)^k
 * - Solving for k: k = log(1-p) / log(1-s^r)
 */
template <typename E, typename I>
std::pair<uint64_t, uint64_t>
ApproximateNearestNeighbor<E, I>::findSizes(Bound lower, Bound higher) {
  always_assert(lower.similarity <= higher.similarity);
  always_assert(lower.probability <= higher.probability);
  always_assert(IMPLIES(lower.similarity == higher.similarity,
                        lower.probability == higher.probability));
  always_assert(IMPLIES(lower.probability == higher.probability,
                        lower.similarity == higher.similarity));

  // Try increasing numbers of rounds
  for (uint64_t x = 1; x < 40; ++x) {
    // Minimum experiments needed for higher bound
    auto const atleast =
      std::ceil(log(1.0-higher.probability)/log(1.0-powf(higher.similarity, x)));
    // Maximum experiments allowed for lower bound
    auto const atmost = lower.probability == higher.probability
      ? atleast
      : std::floor(log(1.0-lower.probability)/log(1.0-powf(lower.similarity, x)));
    // If there's a valid range, use it
    if (atleast <= atmost) return std::make_pair(x, atleast);
  }
  always_assert_flog(
    false,
    "Unable to find suitable sizes for "
    "<{:4f} with probability {:4f} and >{:4f} with probability {:4f}",
    lower.similarity, lower.probability,
    higher.similarity, higher.probability
  );
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

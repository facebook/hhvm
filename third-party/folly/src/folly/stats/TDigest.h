/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cassert>
#include <limits>
#include <vector>

#include <folly/Range.h>
#include <folly/Utility.h>

namespace folly {

/*
 * TDigests are a biased quantile estimator designed to estimate the values of
 * the quantiles of streaming data with high accuracy and low memory,
 * particularly for quantiles at the tails (p0.1, p1, p99, p99.9). See
 * https://github.com/tdunning/t-digest/blob/master/docs/t-digest-paper/histo.pdf
 * for an explanation of what the purpose of TDigests is, and how they work.
 *
 * There is a notable difference between the implementation here and the
 * implementation in the paper. In the paper, the recommended scaling function
 * for bucketing centroids is an arcsin function. The arcsin function provides
 * high accuracy for low memory, but comes at a relatively high compute cost.
 * A good choice algorithm has the following properties:
 *   - The value of the function k(0, delta) = 0, and k(1, delta) = delta.
 *     This is a requirement for any t-digest function.
 *   - The limit of the derivative of the function dk/dq at 0 is inf, and at
 *     1 is inf. This provides bias to improve accuracy at the tails.
 *   - For any q <= 0.5, dk/dq(q) = dk/dq(1-q). This ensures that the accuracy
 *     of upper and lower quantiles are equivalent.
 * As such, TDigest uses a sqrt function with these properties, which is faster
 * than arcsin. There is a small, but relatively negligible impact to accuracy
 * at the tail. In empirical tests, accuracy of the sqrt approach has been
 * adequate.
 */
class TDigest {
 public:
  static constexpr size_t kDefaultMaxSize = 100;
  // Recommended number of values to buffer before each merge.
  static constexpr size_t kDefaultBufferSize = 1000;

  class Centroid {
   public:
    explicit Centroid(double mean = 0.0, double weight = 1.0)
        : mean_(mean), weight_(weight) {
      assert(weight > 0);
    }

    inline double mean() const { return mean_; }

    inline double weight() const { return weight_; }

    /*
     * Adds the sum/weight to this centroid, and returns the new sum.
     */
    inline double add(double sum, double weight);

    inline bool operator<(const Centroid& other) const {
      return mean() < other.mean();
    }

   private:
    double mean_;
    double weight_;
  };

  class MergeWorkingBuffer {
    friend TDigest;
    std::vector<Centroid> buf;
  };

  explicit TDigest(size_t maxSize = kDefaultMaxSize) : maxSize_(maxSize) {}

  explicit TDigest(
      std::vector<Centroid> centroids,
      double sum,
      double count,
      double max_val,
      double min_val,
      size_t maxSize = kDefaultMaxSize);

  /*
   * Returns a new TDigest constructed with values merged from the current
   * digest and the given sortedValues.
   */
  TDigest merge(sorted_equivalent_t, Range<const double*> sortedValues) const;
  /*
   * Returns a new TDigest constructed with values merged from the current
   * digest and the given unsortedValues.
   */
  TDigest merge(Range<const double*> unsortedValues) const;

  /*
   * Returns a new TDigest constructed with values merged from the given
   * digests.
   */
  static TDigest merge(Range<const TDigest*> digests);
  static TDigest merge(Range<const TDigest**> digests);
  static TDigest merge(const TDigest& d1, const TDigest& d2);

  /*
   * Merge in place, using the provided storage.
   */
  void merge(
      sorted_equivalent_t,
      Range<const double*> sortedValues,
      MergeWorkingBuffer& workingBuffer);

  /*
   * Estimates the value of the given quantile.
   */
  double estimateQuantile(double q) const;

  /*
   * Returns the estimate of the CDF at the given input.
   * Raise an invalid_argument exception if the input is NaN or infinite.
   * Returns NaN if the centroids are emtpy.
   */
  double estimateCdf(double x) const;

  double mean() const { return count_ > 0 ? sum_ / count_ : 0; }

  double sum() const { return sum_; }

  double count() const { return count_; }

  double min() const { return min_; }

  double max() const { return max_; }

  bool empty() const { return centroids_.empty(); }

  const std::vector<Centroid>& getCentroids() const { return centroids_; }

  size_t maxSize() const { return maxSize_; }

 private:
  class CentroidMerger;

  void mergeValues(
      TDigest& dst,
      Range<const double*> sortedValues,
      std::vector<Centroid>& workingBuffer) const;

  template <class T>
  static TDigest mergeImpl(Range<T> ds);

  static TDigest merge2Impl(const TDigest& d1, const TDigest& d2);

  std::vector<Centroid> centroids_;
  size_t maxSize_;
  double sum_ = 0.0;
  double count_ = 0.0;
  double max_ = std::numeric_limits<double>::quiet_NaN();
  double min_ = std::numeric_limits<double>::quiet_NaN();
};

} // namespace folly

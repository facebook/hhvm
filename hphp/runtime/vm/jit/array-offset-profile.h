/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_ARRAY_OFFSET_PROFILE_H_
#define incl_HPHP_JIT_ARRAY_OFFSET_PROFILE_H_

#include <folly/Optional.h>

#include <cstdint>

namespace HPHP {

struct ArrayData;
struct StringData;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Target profile for known MixedArray or SetArray access offsets.
 */
struct ArrayOffsetProfile {
  /*
   * Choose the "hot position" for the profiled array access using
   * questionable heuristics.
   */
  folly::Optional<uint32_t> choose() const;

  /*
   * Update the profile to register an access at `key' in `ad'.
   */
  void update(const ArrayData* ad, int64_t key);
  void update(const ArrayData* ad, const StringData* key,
              bool checkForInt = true);

  /*
   * Combine `l' and `r', retaining the kNumTrackedSamples with the highest
   * counts.
   */
  static void reduce(ArrayOffsetProfile& l,
                     const ArrayOffsetProfile& r);

  std::string toString() const;
private:
  /*
   * Initialize the samples.
   *
   * The default `pos' values need to be -1, but since this is stored in RDS,
   * we need to manually initialize.
   */
  void init();

  /*
   * Update the profile for `pos' by `count'.
   *
   * Returns whether or not we were able to account for the update precisely.
   */
  bool update(int32_t pos, uint32_t count);

private:
  struct Line {
    int32_t pos{-1};
    uint32_t count{0};
  };

  static const size_t kNumTrackedSamples = 4;

private:
  Line m_hits[kNumTrackedSamples];
  uint32_t m_untracked{0};
  bool m_init{false};
};

///////////////////////////////////////////////////////////////////////////////

}}

#endif

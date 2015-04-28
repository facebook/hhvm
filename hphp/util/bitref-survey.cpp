/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/bitref-survey.h"
#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/array-data.h"
#include "folly/stats/Histogram.h"
#include "folly/stats/Histogram-defs.h"
#include <mutex>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(bitref);
BitrefSurvey *g_survey;
uint64_t arr_empty_count, str_empty_count;
std::mutex m;
folly::Histogram<uint32_t> str_rc_copy_histogram(8, 1, 2048);
folly::Histogram<uint32_t> arr_rc_copy_histogram(8, 1, 2048);

inline BitrefSurvey *survey() {
  if (!g_survey) {
    g_survey = new BitrefSurvey();
  }
  return g_survey;
}

/*
 * Log when a copy-on-write check takes place. 
 */
void cow_check_occurred(RefCount refcount, bool bitref) {
  // default: type is array
  survey()->cow_check_occurred(refcount, bitref, true);
}

void cow_check_occurred(ArrayData* ad) {
  survey()->cow_check_occurred(ad);
}

void cow_check_occurred(StringData* sd) {
  survey()->cow_check_occurred(sd);
}

void survey_request_end() {
  survey()->survey_request_end();
}

void BitrefSurvey::cow_check_occurred(RefCount refcount, bool bitref, bool isArray) {
  m.lock();

  check_count++;
  isArray ? arr_check_count++ : str_check_count++;
  if ((uint32_t)refcount > 1) {
    assert(bitref);
    //'necessary' copy
    rc_copy_count++;
    isArray ? arr_rc_copy_count++ : str_rc_copy_count++;

    // these will all be copied under bitref, although bitref may not be set 
    // for static objects right now (need to fix that)
    bitref_copy_count++;
    isArray ? arr_bitref_copy_count++ : str_bitref_copy_count++;

    // log static objects
    if (!check_refcount_ns(refcount)) {
      static_count++;
      isArray ? arr_static_count++ : str_static_count++;
    }
  }
  // non-static objects that are *only* copied by bitref, not in rc
  if (refcount == 1 && bitref) {
    bitref_copy_count++;
    isArray ? arr_bitref_copy_count++ : str_bitref_copy_count++;
  }
  m.unlock();
}

void BitrefSurvey::cow_check_occurred(ArrayData* ad) {
  cow_check_occurred(ad->getCount(), check_one_bit_ref(ad->m_pad), true);
  m.lock();
  if (ad->kind() == ArrayData::kEmptyKind) {
    // Arrays with kEmptyKind should be static, so always copied
    // log them here to get an idea of what percentage of copies are
    // of the empty, static array
    arr_empty_count++;
    //TODO check zero length, compare to global static array, see dif in 3 measurements
  }
  if ((uint32_t)ad->getCount() > 1) { //includes static
    arr_rc_copy_histogram.addValue(ad->m_size);
  }
  m.unlock();
}

void BitrefSurvey::cow_check_occurred(StringData* sd) {
  cow_check_occurred(sd->getCount(), check_one_bit_ref(sd->m_pad), false);
  m.lock();
  if (sd->m_len == 0) {
    str_empty_count++;
  }
  if ((uint32_t)sd->getCount() > 1) { //includes static
    str_rc_copy_histogram.addValue(sd->m_len);
  }
  m.unlock();
}

void BitrefSurvey::survey_request_end() {
  m.lock();
  uint64_t avoided = check_count - bitref_copy_count;
  double avoided_pc = ((double)avoided / (double)check_count) * 100;
  uint64_t arr_avoided = arr_check_count - arr_bitref_copy_count;
  double arr_avoided_pc = ((double)arr_avoided / (double)arr_check_count) * 100;
  uint64_t str_avoided = str_check_count - str_bitref_copy_count;
  double str_avoided_pc = ((double)str_avoided / (double)str_check_count) * 100;

  double rc_copy_pc = ((double) rc_copy_count / (double) check_count) * 100;
  double bitref_copy_pc = ((double)bitref_copy_count / (double)check_count) * 100;
  double arr_rc_copy_pc = ((double) arr_rc_copy_count / (double) arr_check_count) * 100;
  double arr_bitref_copy_pc = ((double)arr_bitref_copy_count / (double)arr_check_count) * 100;
  double str_rc_copy_pc = ((double) str_rc_copy_count / (double) str_check_count) * 100;
  double str_bitref_copy_pc = ((double)str_bitref_copy_count / (double)str_check_count) * 100;
  

  TRACE(1, "     || # checks |        # rc copies       | # 1bit copies   | # avoided copy  \n");
  TRACE(1, "     ||          |       all       | static |(incl. rc copies)|                 \n");
  TRACE(1, "--------------------------------------------------------------------------------\n");

  TRACE(1, " all ||%10ld|%8ld (%5.2f%%)|%8ld|%8ld (%5.2f%%)|%8ld (%5.2f%%)\n", check_count,
      rc_copy_count, rc_copy_pc, static_count, bitref_copy_count, bitref_copy_pc,
      avoided, avoided_pc);

  TRACE(1, " arr ||%10ld|%8ld (%5.2f%%)|%8ld|%8ld (%5.2f%%)|%8ld (%5.2f%%)\n", arr_check_count,
      arr_rc_copy_count, arr_rc_copy_pc, arr_static_count, arr_bitref_copy_count, 
      arr_bitref_copy_pc, arr_avoided, arr_avoided_pc);

  TRACE(1, " str ||%10ld|%8ld (%5.2f%%)|%8ld|%8ld (%5.2f%%)|%8ld (%5.2f%%)\n", str_check_count,
      str_rc_copy_count, str_rc_copy_pc, str_static_count, str_bitref_copy_count, 
      str_bitref_copy_pc, str_avoided, str_avoided_pc);

  TRACE(1, "--------------------------------------------------------------------------------\n");
  TRACE(1, "# []: %8ld (%5.2f%% of rc array copies, %5.2f%% of 1bit array copies)\n",
      arr_empty_count,
      ((double)arr_empty_count / (double)arr_rc_copy_count) * 100,
      ((double)arr_empty_count / (double)arr_bitref_copy_count) * 100);
  TRACE(1, "# \"\": %8ld (%5.2f%% of rc string copies, %5.2f%% of 1bit string copies)\n",
      str_empty_count,
      ((double)str_empty_count / (double)str_rc_copy_count) * 100,
      ((double)str_empty_count / (double)str_bitref_copy_count) * 100);
  
  TRACE(1, "--------------------------------------------------------------------------------\n");
  
  unsigned int numBuckets = str_rc_copy_histogram.getNumBuckets();
  TRACE(1, "String length:    0-   0 %8lu\n", str_rc_copy_histogram.getBucketByIndex(0).count);
  for (unsigned int n = 1; n < numBuckets - 1; n++) {
    TRACE(1, "               %4u-%4u %8lu\n", str_rc_copy_histogram.getBucketMin(n),
        str_rc_copy_histogram.getBucketMax(n), str_rc_copy_histogram.getBucketByIndex(n).count);
  }
  TRACE(1, "               2048+     %8lu\n", str_rc_copy_histogram.getBucketByIndex(numBuckets - 1).count);
  
  TRACE(1, "--------------------------------------------------------------------------------\n");

  numBuckets = arr_rc_copy_histogram.getNumBuckets();
  TRACE(1, "Array length :    0-   0 %8lu\n", arr_rc_copy_histogram.getBucketByIndex(0).count);
  for (unsigned int n = 1; n < numBuckets - 1; n++) {
    TRACE(1, "               %4u-%4u %8lu\n", arr_rc_copy_histogram.getBucketMin(n),
        arr_rc_copy_histogram.getBucketMax(n), arr_rc_copy_histogram.getBucketByIndex(n).count);
  }
  TRACE(1, "               2048+     %8lu\n", arr_rc_copy_histogram.getBucketByIndex(numBuckets - 1).count);

  TRACE(1, "\n");
  m.unlock();
  /*
  check_count = 0;
  arr_check_count = 0;
  str_check_count = 0;

  rc_copy_count = 0;
  arr_rc_copy_count = 0;
  str_rc_copy_count = 0;

  static_count = 0;
  arr_static_count = 0;
  str_static_count = 0;

  bitref_copy_count = 0;
  arr_bitref_copy_count = 0;
  str_bitref_copy_count = 0;

  arr_empty_count = 0;
  */
}

///////////////////////////////////////////////////////////////////////////////


}
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
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/struct-array.h"
#include "hphp/runtime/base/struct-array-defs.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/empty-array.h"
#include "hphp/runtime/base/apc-array.h"
#include "folly/stats/Histogram.h"
#include "folly/stats/Histogram-defs.h"
#include <mutex>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

uint64_t arr_empty_count, arr_rc_empty_count, arr_bitref_empty_count, arr_rc_size,
  arr_bitref_size, arr_bcow_size, str_empty_count;
std::mutex m;
folly::Histogram<uint32_t> str_rc_copy_histogram(8, 1, 2048);
folly::Histogram<uint32_t> arr_rc_copy_histogram(8, 1, 2048);

uint64_t arr_rc_type_count[ArrayData::kNumKinds];
uint64_t arr_bitref_type_count[ArrayData::kNumKinds];

inline BitrefSurvey &survey() {
  static BitrefSurvey survey;
  return survey;
}

/*
 * Log when a copy-on-write check takes place. 
 */
void cow_check_occurred(RefCount refcount, bool bitref) {
  // default: type is array
  survey().cow_check_occurred(refcount, bitref, true);
}

void cow_check_occurred(const ArrayData* ad) {
  survey().cow_check_occurred(ad);
}

void cow_check_occurred(const StringData* sd) {
  survey().cow_check_occurred(sd);
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

void BitrefSurvey::cow_check_occurred(const ArrayData* ad) {
  cow_check_occurred(ad->getCount(), check_one_bit_ref(ad->m_pad), true);
  m.lock();

  if (ad->empty()) {
    arr_empty_count++;
    if ((uint32_t)ad->getCount() > 1) {
      arr_rc_empty_count++;
    }
    if (check_one_bit_ref(ad->m_pad)) {
      arr_bitref_empty_count++;
    }
  }
  // heap size of copies for refcounting
  if ((uint32_t)ad->getCount() > 1) { //includes static
    arr_rc_copy_histogram.addValue(ad->m_size);

    arr_rc_type_count[ad->m_kind]++;

    // no need to cover globals as it doesn't support CoW
    // need to cover proxy
    if (ad->isPacked()) {
      arr_rc_size += ((PackedArray*)ad)->heapSize(ad);
    } else if (ad->isStruct()) {
      arr_rc_size += ((StructArray*)ad)->heapSize(ad);
    } else if (ad->isMixed()) {
      arr_rc_size += ((MixedArray*)ad)->heapSize();
    } else if (ad->isEmptyArray()) {
      arr_rc_size += sizeof(EmptyArray);
    } else if (ad->isApcArray()) {
      arr_rc_size += getMemSize(ad);
    }
  }
  // heap size of copies for 1 bit ref
  if (check_one_bit_ref(ad->m_pad)) {

    arr_bitref_type_count[ad->m_kind]++;

    if (ad->isPacked()) {
      arr_bitref_size += ((PackedArray*)ad)->heapSize(ad);
    } else if (ad->isStruct()) {
      arr_bitref_size += ((StructArray*)ad)->heapSize(ad);
    } else if (ad->isMixed()) {
      arr_bitref_size += ((MixedArray*)ad)->heapSize();
    } else if (ad->isEmptyArray()) {
      arr_bitref_size += sizeof(EmptyArray);
    } else if (ad->isApcArray()) {
      arr_bitref_size += getMemSize(ad);
    }
  }

  // heap size of copies for blind cow
  if (ad->isPacked()) {
    arr_bcow_size += ((PackedArray*)ad)->heapSize(ad);
  } else if (ad->isStruct()) {
    arr_bcow_size += ((StructArray*)ad)->heapSize(ad);
  } else if (ad->isMixed()) {
    arr_bcow_size += ((MixedArray*)ad)->heapSize();
  } else if (ad->isEmptyArray()) {
    arr_bcow_size += sizeof(EmptyArray);
  } else if (ad->isApcArray()) {
    arr_bcow_size += getMemSize(ad);
  }


  m.unlock();
}

void BitrefSurvey::cow_check_occurred(const StringData* sd) {
  cow_check_occurred(sd->getCount(), check_one_bit_ref(sd->m_pad), false);
  m.lock();

  if (sd->empty()) {
    str_empty_count++;
  }
  if ((uint32_t)sd->getCount() > 1) { //includes static
    str_rc_copy_histogram.addValue(sd->m_len);
  }
  m.unlock();
}

BitrefSurvey::~BitrefSurvey() {
  double bcow_copy_pc = ((double) check_count / (double) rc_copy_count) * 100;
  double arr_bcow_copy_pc = ((double) arr_check_count / (double) arr_rc_copy_count) * 100;
  double str_bcow_copy_pc = ((double) str_check_count / (double) str_rc_copy_count) * 100;

  double bitref_copy_pc = ((double)bitref_copy_count / (double)rc_copy_count) * 100;
  double arr_bitref_copy_pc = ((double)arr_bitref_copy_count / (double)arr_rc_copy_count) * 100;
  double str_bitref_copy_pc = ((double)str_bitref_copy_count / (double)str_rc_copy_count) * 100;
  
  FILE *fp = fopen("/tmp/hphp_mrb.log", "a");

  fprintf(fp, "     ||    # rc copies   |   # 1bit copies    |    blind CoW        \n");
  fprintf(fp, "     ||   all   | static |  (incl. rc copies) |                     \n");
  fprintf(fp, "--------------------------------------------------------------------\n");

  fprintf(fp, " all ||%9ld|%8ld|%9ld (%7.2f%%)|%10ld (%7.2f%%)\n", 
    rc_copy_count, static_count, bitref_copy_count, bitref_copy_pc,
    check_count, bcow_copy_pc);

  fprintf(fp, " arr ||%9ld|%8ld|%9ld (%7.2f%%)|%10ld (%7.2f%%)\n", 
    arr_rc_copy_count, arr_static_count, arr_bitref_copy_count, 
    arr_bitref_copy_pc, arr_check_count, arr_bcow_copy_pc);

  fprintf(fp, " str ||%9ld|%8ld|%9ld (%7.2f%%)|%10ld (%7.2f%%)\n",
    str_rc_copy_count, str_static_count, str_bitref_copy_count, 
    str_bitref_copy_pc, str_check_count, str_bcow_copy_pc);

  fprintf(fp, "--------------------------------------------------------------------\n");
  fprintf(fp, "# []:  %9ld|        |%9ld           |%10ld        \n",
      arr_rc_empty_count, arr_bitref_empty_count, arr_empty_count);
  fprintf(fp, "Array heap size: rc:%8.2fMB 1bit:%9.2fMB bcow:%10.2fMB\n",
    (double)arr_rc_size/1000000, (double)arr_bitref_size/1000000, (double)arr_bcow_size/1000000);

  fprintf(fp, "Array kind:\t\tRC \t\t\t1BIT\n");
  for (int i = 0; i < ArrayData::kNumKinds; i++) {
    fprintf(fp, "\t%s\t:%9ld\t%9ld\n", ArrayData::kindToString((ArrayData::ArrayKind)i),
      arr_rc_type_count[i], arr_bitref_type_count[i]);
  }
  fprintf(fp, "# \"\": %8ld (%5.2f%% of rc string copies, %5.2f%% of 1bit string copies)\n",
      str_empty_count,
      ((double)str_empty_count / (double)str_rc_copy_count) * 100,
      ((double)str_empty_count / (double)str_bitref_copy_count) * 100);
  
  fprintf(fp, "--------------------------------------------------------------------\n");
  
  /*unsigned int numBuckets = str_rc_copy_histogram.getNumBuckets();
  fprintf(fp, "String length:    0-   0 %8lu\n", str_rc_copy_histogram.getBucketByIndex(0).count);
  for (unsigned int n = 1; n < numBuckets - 1; n++) {
    fprintf(fp, "               %4u-%4u %8lu\n", str_rc_copy_histogram.getBucketMin(n),
        str_rc_copy_histogram.getBucketMax(n), str_rc_copy_histogram.getBucketByIndex(n).count);
  }
  fprintf(fp, "               2048+     %8lu\n", str_rc_copy_histogram.getBucketByIndex(numBuckets - 1).count);
  
  fprintf(fp, "--------------------------------------------------------------------------------\n");

  numBuckets = arr_rc_copy_histogram.getNumBuckets();
  fprintf(fp, "Array length :    0-   0 %8lu\n", arr_rc_copy_histogram.getBucketByIndex(0).count);
  for (unsigned int n = 1; n < numBuckets - 1; n++) {
    fprintf(fp, "               %4u-%4u %8lu\n", arr_rc_copy_histogram.getBucketMin(n),
        arr_rc_copy_histogram.getBucketMax(n), arr_rc_copy_histogram.getBucketByIndex(n).count);
  }
  fprintf(fp, "               2048+     %8lu\n", arr_rc_copy_histogram.getBucketByIndex(numBuckets - 1).count);

  fprintf(fp, "\n");*/
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

  arr_kEmptyKind_count = 0;
  */

  fclose(fp);
}

///////////////////////////////////////////////////////////////////////////////


}

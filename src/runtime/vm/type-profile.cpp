/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include "util/trace.h"
#include "runtime/base/types.h"
#include "runtime/base/runtime_option.h"
#include "runtime/vm/translator/translator.h"
#include "runtime/vm/type-profile.h"
 
namespace HPHP {
namespace VM {

TRACE_SET_MOD(typeProfile);

/*
 * It is useful at translation time to have a hunch as to the types a given
 * instruction is likely to produce. This is a probabilistic data structure
 * that tries to balance: cost of reading and writing; memory overhead;
 * prediction recall (percentage of time we make a prediction); prediction
 * accuracy (fidelity relative to reality); and prediction precision
 * (fidelity relative to ourselves).
 */

struct ValueProfile {
  uint32_t m_tag;
  // All of these saturate at 255.
  uint8_t  m_totalSamples;
  uint8_t  m_samples[MaxNumDataTypes];
};

/* 
 * Magic tunables.
 */

/* 
 * kNumEntries
 *
 * Tradeoff: size vs. precision.
 *
 * ~4 million entries.
 *
 * Size: (MaxNumDataTypes = 16) B -> 64 MB.
 *
 * Precision: If we collide, it will disrupt the precision of our
 * predictions; other sites will mess with our prediction, changing it
 * without reality having changed. www-sized codebases are weighing in
 * around 50MB these days.  Swagging three bytes per instruction, this
 * provides enough space to profile about every eighth instruction before
 * collisions start becoming really problematic.
 */

static const int kNumEntries = 1 << 22;
static const int kLineSize = 4;
static const int kNumLines = kNumEntries / kLineSize;
static const int kNumLinesMask = kNumLines - 1;

/*
 * kMinInstances
 *
 * Tradeoff: recall vs. accuracy. This is the minimum number of examples
 * before we'll report any information at all.
 *
 * Recall: If we set this too high, we'll never be able to predict
 * anything.
 *
 * Accuracy: If we set this too low, we'll be "jumpy", eagerly predicting
 * types on the basis of weak evidence.
 */
static const double kMinInstances = 199.0;

typedef ValueProfile ValueProfileLine[kLineSize];
static ValueProfileLine* profiles;

static ValueProfileLine*
profileInitMmap() {
  const std::string& path = RuntimeOption::EvalJitProfilePath;
  if (path.empty()) {
    return NULL;
  }

  TRACE(1, "profileInit: path %s\n", path.c_str());
  int fd = open(path.c_str(), O_RDWR | O_CREAT, 0600);
  if (fd < 0) {
    TRACE(0, "profileInit: open %s failed: %s\n", path.c_str(),
          strerror(errno));
    perror("open");
    return NULL;
  }

  size_t len = sizeof(ValueProfileLine) * kNumLines;
  int retval = ftruncate(fd, len);
  if (retval < 0) {
    perror("truncate");
    TRACE(0, "profileInit: truncate %s failed: %s\n", path.c_str(),
          strerror(errno));
    return NULL;
  }

  int flags = PROT_READ |
    (RuntimeOption::EvalJitProfileRecord ? PROT_WRITE : 0);
  void* mmapRet = mmap(0, len, flags, MAP_SHARED, // Yes, shared.
                       fd, 0);
  if (mmapRet == MAP_FAILED) {
    perror("mmap");
    TRACE(0, "profileInit: mmap %s failed: %s\n", path.c_str(),
          strerror(errno));
    return NULL;
  }
  return (ValueProfileLine*)mmapRet;
}

void
profileInit() {
  if (!profiles) {
    profiles = profileInitMmap();
    if (!profiles) {
      TRACE(1, "profileInit: anonymous memory.\n");
      profiles = (ValueProfileLine*)calloc(sizeof(ValueProfileLine), kNumLines);
      ASSERT(profiles);
    }
  }
}

enum KeyToVPMode {
  Read, Write
};

uint64_t
TypeProfileKey::hash() const {
  /*
   * The reason we don't use SrcKey::hash() here is that it isn't stable
   * across runs of the same code, because function IDs depend on the order
   * of the request stream. This makes debugging this module really
   * difficult, since the predictions can be wildly different from run to
   * run. Function names and bytecode offsets are not racy.
   */
  return hash_int64_pair(m_func->fullName()->hash(), 
                         m_offset);
}

static inline ValueProfile*
keyToVP(const TypeProfileKey& key, KeyToVPMode mode) {
  ASSERT(profiles);
  uint64_t h = key.hash();
  ValueProfileLine& l = profiles[h & kNumLinesMask];
  int replaceCandidate = 0;
  int minCount = 255;
  for (int i = 0; i < kLineSize; i++) {
    if (l[i].m_tag == uint32_t(h)) {
      return &l[i];
    }
    if (mode == Write && l[i].m_totalSamples < minCount) {
      replaceCandidate = i;
      minCount = l[i].m_totalSamples;
    }
  }
  if (mode == Write) {
    ASSERT(replaceCandidate >= 0 && replaceCandidate < kLineSize);
    l[replaceCandidate].m_tag = uint32_t(h);
    l[replaceCandidate].m_totalSamples = 0;
    return &l[replaceCandidate];
  }
  return NULL;
}

static void bump8(uint8_t& cnt) {
  if (cnt < 255) cnt++;
}

void recordType(const TypeProfileKey& key, DataType dt) {
  if (!profiles) return;
  ValueProfile *prof = keyToVP(key, Write);
  bump8(prof->m_samples[dt]);
  bump8(prof->m_totalSamples);
}

std::pair<DataType, double> predictType(const TypeProfileKey& key) {
  static const std::pair<DataType, double> kNullPred =
    std::make_pair(KindOfUninit, 0.0);
  if (!profiles) return kNullPred;
  const ValueProfile *prof = keyToVP(key, Read);
  if (!prof) return kNullPred;
  double total = 0.0;
  for (int i = 0; i < MaxNumDataTypes; ++i) total += prof->m_samples[i];
  double maxProb = 0.0;
  DataType pred = KindOfUninit;
  // If we have fewer than kMinInstances predictions, consider it too
  // little data to be actionable.
  if (total >= kMinInstances) for (int i = 0; i < MaxNumDataTypes; ++i) {
    double prob = (1.0 * prof->m_samples[i]) / total;
    if (prob > maxProb) {
      maxProb = prob;
      pred = (DataType)i;
    }
    if (prob == 1.0) break;
  }
  return std::make_pair(pred, maxProb);
}

bool isProfileOpcode(const PC& pc) {
  return *pc == OpRetC || *pc == OpCGetM;
}

}
}

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
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/util/trace.h"

#include "folly/String.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

namespace HPHP {

TRACE_SET_MOD(typeProfile);

/*
 * It is useful at translation time to have a hunch as to the types a given
 * instruction is likely to produce. This is a probabilistic data structure
 * that tries to balance: cost of reading and writing; memory overhead;
 * prediction recall (percentage of time we make a prediction); prediction
 * accuracy (fidelity relative to reality); and prediction precision
 * (fidelity relative to ourselves).
 */

typedef uint16_t Counter;
static const Counter kMaxCounter = USHRT_MAX;

struct ValueProfile {
  uint32_t m_tag;
  // All of these saturate at 255.
  Counter m_totalSamples;
  Counter m_samples[MaxNumDataTypesIndex];
  void dump() {
    for (int i = 0; i < MaxNumDataTypesIndex; i++) {
      TRACE(0, "t%3d: %4d\n", i, m_samples[getDataTypeValue(i)]);
    }
  }
};

/*
 * Magic tunables.
 */

/*
 * kNumEntries
 *
 * Tradeoff: size vs. accuracy.
 *
 * ~256K entries.
 *
 * Size: (sizeof(ValueProfile) == 16) B -> 4MB
 *
 * Accuracy: If we collide further than kLineSize, we toss out perfectly
 * good evidence. www seems to use about 12000 method names at this
 * writing, so we have a decent pad for collisions.
 */

static const int kNumEntries = 1 << 18;
static const int kLineSizeLog2 = 2;
static const int kLineSize = 1 << kLineSizeLog2;
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
static const double kMinInstances = 99.0;

typedef ValueProfile ValueProfileLine[kLineSize];
static ValueProfileLine* profiles;

static ValueProfileLine*
profileInitMmap() {
  const std::string& path = RuntimeOption::EvalJitProfilePath;
  if (path.empty()) {
    return nullptr;
  }

  TRACE(1, "profileInit: path %s\n", path.c_str());
  int fd = open(path.c_str(), O_RDWR | O_CREAT, 0600);
  if (fd < 0) {
    TRACE(0, "profileInit: open %s failed: %s\n", path.c_str(),
          folly::errnoStr(errno).c_str());
    perror("open");
    return nullptr;
  }

  size_t len = sizeof(ValueProfileLine) * kNumLines;
  int retval = ftruncate(fd, len);
  if (retval < 0) {
    perror("truncate");
    TRACE(0, "profileInit: truncate %s failed: %s\n", path.c_str(),
          folly::errnoStr(errno).c_str());
    return nullptr;
  }

  int flags = PROT_READ |
    (RuntimeOption::EvalJitProfileRecord ? PROT_WRITE : 0);
  void* mmapRet = mmap(0, len, flags, MAP_SHARED, // Yes, shared.
                       fd, 0);
  if (mmapRet == MAP_FAILED) {
    perror("mmap");
    TRACE(0, "profileInit: mmap %s failed: %s\n", path.c_str(),
          folly::errnoStr(errno).c_str());
    return nullptr;
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
      assert(profiles);
    }
  }
}

/*
 * Warmup.
 *
 * In cli mode, we only record samples if we're in recording to replay
 * later.
 *
 * For server mode, we record samples for all requests started after
 * the EvalJitWarmupRequests'th req.
 */
bool __thread profileOn = false;
static int64_t numRequests;

int64_t requestCount() {
  return numRequests;
}

static inline bool warmedUp() {
  return (numRequests >= RuntimeOption::EvalJitWarmupRequests) ||
    (RuntimeOption::ClientExecutionMode() &&
     !RuntimeOption::EvalJitProfileRecord);
}

static inline bool profileThisRequest() {
  if (warmedUp()) return false;
  if (RuntimeOption::ServerExecutionMode()) return true;
  return RuntimeOption::EvalJitProfileRecord;
}

void
profileRequestStart() {
  bool p = profileThisRequest();
  if (p != profileOn) {
    profileOn = p;
    if (!ThreadInfo::s_threadInfo.isNull()) {
      ThreadInfo::s_threadInfo->m_reqInjectionData.updateJit();
    }
  }
}

void profileRequestEnd() {
  numRequests++; // racy RMW; ok to miss a rare few.
}

enum class KeyToVPMode {
  Read, Write
};

uint64_t
TypeProfileKey::hash() const {
  return hash_int64_pair(m_kind, m_name->hash());
}

static inline ValueProfile*
keyToVP(TypeProfileKey key, KeyToVPMode mode) {
  assert(profiles);
  uint64_t h = key.hash();
  // Use the low-order kLineSizeLog2 bits to as tag bits to distinguish
  // within the line, rather than to choose a line. Without the shift, all
  // the tags in the line would have the same low-order bits, making
  // collisions kLineSize times more likely.
  int hidx = (h >> kLineSizeLog2) & kNumLinesMask;
  ValueProfileLine& l = profiles[hidx];
  int replaceCandidate = 0;
  int minCount = 255;
  for (int i = 0; i < kLineSize; i++) {
    if (l[i].m_tag == uint32_t(h)) {
      TRACE(2, "Found %d for %s -> %d\n",
            l[i].m_tag, key.m_name->data(), uint32_t(h));
      return &l[i];
    }
    if (mode == KeyToVPMode::Write && l[i].m_totalSamples < minCount) {
      replaceCandidate = i;
      minCount = l[i].m_totalSamples;
    }
  }
  if (mode == KeyToVPMode::Write) {
    assert(replaceCandidate >= 0 && replaceCandidate < kLineSize);
    ValueProfile& vp = l[replaceCandidate];
    Stats::inc(Stats::TypePred_Evict, vp.m_totalSamples != 0);
    Stats::inc(Stats::TypePred_Insert);
    TRACE(1, "Killing %d in favor of %s -> %d\n",
          vp.m_tag, key.m_name ? key.m_name->data() : "NULL", uint32_t(h));
    vp.m_totalSamples = 0;
    memset(&vp.m_samples, 0, sizeof(vp.m_samples));
    // Zero first, then claim. It seems safer to temporarily zero out some
    // other function's values than to have this new function using the
    // possibly-non-trivial prediction from an unrelated function.
    compiler_membar();
    vp.m_tag = uint32_t(h);
    return &l[replaceCandidate];
  }
  return nullptr;
}

void recordType(TypeProfileKey key, DataType dt) {
  if (!profiles) return;
  if (!shouldProfile()) return;
  assert(dt != KindOfUninit);
  // Normalize strings to KindOfString.
  if (dt == KindOfStaticString) dt = KindOfString;
  TRACE(1, "recordType lookup: %s -> %d\n", key.m_name->data(), dt);
  ValueProfile *prof = keyToVP(key, KeyToVPMode::Write);
  if (prof->m_totalSamples != kMaxCounter) {
    prof->m_totalSamples++;
    // NB: we can't quite assert that we have fewer than kMaxCounter samples,
    // because other threads are updating this structure without locks.
    int dtIndex = getDataTypeIndex(dt);
    if (prof->m_samples[dtIndex] < kMaxCounter) {
      prof->m_samples[dtIndex]++;
    }
  }
  ONTRACE(2, prof->dump());
}

typedef std::pair<DataType, double> PredVal;

PredVal predictType(TypeProfileKey key) {
  PredVal kNullPred = std::make_pair(KindOfUninit, 0.0);
  if (!profiles) return kNullPred;
  const ValueProfile *prof = keyToVP(key, KeyToVPMode::Read);
  if (!prof) {
    TRACE(2, "predictType lookup: %s -> MISS\n", key.m_name->data());
    Stats::inc(Stats::TypePred_Miss);
    return kNullPred;
  }
  double total = prof->m_totalSamples;
  if (total < kMinInstances) {
    Stats::inc(Stats::TypePred_MissTooFew);
    TRACE(2, "TypePred: hit %s but too few samples numSamples %d\n",
          key.m_name->data(), prof->m_totalSamples);
    return kNullPred;
  }
  double maxProb = 0.0;
  DataType pred = KindOfUninit;
  // If we have fewer than kMinInstances predictions, consider it too
  // little data to be actionable.
  for (int i = 0; i < MaxNumDataTypesIndex; ++i) {
    double prob = (1.0 * prof->m_samples[i]) / total;
    if (prob > maxProb) {
      maxProb = prob;
      pred = getDataTypeValue(i);
    }
    if (prob >= 1.0) break;
  }
  Stats::inc(Stats::TypePred_Hit, maxProb >= 1.0);
  Stats::inc(Stats::TypePred_MissTooWeak, maxProb < 1.0);
  TRACE(2, "TypePred: hit %s numSamples %d pred %d prob %g\n",
        key.m_name->data(), prof->m_totalSamples, pred, maxProb);
  // Probabilities over 1.0 are possible due to racy updates.
  if (maxProb > 1.0) maxProb = 1.0;
  return std::make_pair(pred, maxProb);
}

bool isProfileOpcode(const PC& pc) {
  auto const op = toOp(*pc);
  return op == OpRetC || op == OpCGetM;
}

}

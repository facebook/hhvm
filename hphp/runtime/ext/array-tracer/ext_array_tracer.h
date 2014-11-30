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

#ifndef incl_HPHP_EXT_ARRAY_TRACER_H_
#define incl_HPHP_EXT_ARRAY_TRACER_H_

#include <atomic>
#include <folly/AtomicHashMap.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/type-variant.h"

struct TypedValue;

namespace HPHP {

/*
 * ArrayUsage is used to track how an array has ever been used,
 * i.e. if IntKey is set, it means that at some point there as an
 * int key in this array, but may no longer be the case.
 *
 * This is useful to keep track of in our local maps, because when
 * a new array is created from an old one via CoW or from Growing,
 * we can pass along this Usage to the next array.
 */

struct ArrayUsage {
  // UsageBits bitfield, encodes what's happened to the Array
  uint8_t usageBitVec;
};

/*
 * ArrayUsage bitfield. Not an enum class because we're using it as bits and
 * not a real enum
 */
enum UsageBits : uint8_t {
  Reffy = 1 << 0, // Either elements taken by reference or references added
  StrKey = 1 << 1,  // Array has string keys
  IntKey = 1 << 2,  // Array has int keys
};

/*
 * Array Classification bucketizes the "type" of array, whether it is
 * of Mixed, Packed, or Empty kind and what kind of keys it has.
 */
enum ArrayClass : uint8_t {
  MixedIntMap = 0,      // Just Int Keys
  MixedStrMap = 1,      // Just String Keys
  MixedMap = 2,         // String and Int Keys
  PackedVector = 3,
  Empty = 4,
  NoInfo = 5,           // We have no info about the array, likely an empty
                        // array from ArrayInit via ArrayData::Create(key, val)
  NumArrayClasses = 6,
};

enum class ArrayFunc : uint8_t {
  Release = 0,
  NvGetInt,
  NvGetIntConverted,
  NvGetStr,
  NvGetKey,
  SetInt,
  SetIntConverted,
  SetStr,
  Vsize,
  GetValueRef,
  IsVectorData,
  ExistsInt,
  ExistsStr,
  LvalInt,
  LvalStr,
  LvalNew,
  LvalNewRef,
  SetRefInt,
  SetRefStr,
  AddInt,
  AddStr,
  RemoveInt,
  RemoveStr,
  IterBegin,
  IterLast,
  IterEnd,
  IterAdvance,
  IterRewind,
  ValidMArrayIter,
  AdvanceMArrayIter,
  EscalateForSort,
  Ksort,
  Sort,
  Asort,
  Uksort,
  Usort,
  Uasort,
  Copy,
  CopyWithStrongIterators,
  NonSmartCopy,
  Append,
  AppendRef,
  AppendWithRef,
  PlusEq,
  Merge,
  Pop,
  Dequeue,
  Prepend,
  Renumber,
  OnSetEvalScalar,
  Escalate,
  ZSetInt,
  ZSetStr,
  ZAppend,
};

////////////////////////////////////////////////////////////////////////////////
// BitVector Manipulation Helpers
// Just reads/sets the bit that the function names
////////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
bool isIntKey(const ArrayUsage a) {
  return a.usageBitVec & UsageBits::IntKey;
}

ALWAYS_INLINE
void setIntKey(ArrayUsage& usage) {
  usage.usageBitVec |= UsageBits::IntKey;
}

ALWAYS_INLINE
bool isStrKey(const ArrayUsage a) {
  return a.usageBitVec & UsageBits::StrKey;
}

ALWAYS_INLINE
void setStrKey(ArrayUsage& usage) {
  usage.usageBitVec |= UsageBits::StrKey;
}

ALWAYS_INLINE
bool isReffy(const ArrayUsage a) {
  return a.usageBitVec & UsageBits::Reffy;
}

ALWAYS_INLINE
void setReffy(ArrayUsage& usage) {
  usage.usageBitVec |= UsageBits::Reffy;
}

ALWAYS_INLINE
void mergeArrayUsage(const ArrayUsage left, ArrayUsage& write) {
  write.usageBitVec |= left.usageBitVec;
}

struct LogarithmicHistogram {
 public:
  void addValue(uint32_t val) {
    auto idx = folly::findLastSet(val);
    m_buckets[idx]++;
  }

  std::string toString() {
    std::ostringstream os;
    os << "0: " << m_buckets[0].load() << "\n";
    for (uint8_t i = 1; i < m_buckets.size(); i++) {
      os << (uint32_t{1} << (i-1)) << ": "
         << m_buckets[i].load() << "\n";
    }
    return os.str();
  }

 private:
  // m_buckets[0] == [0, 1)
  // m_buckets[1] == [1, 2)
  // m_buckets[2] == [2, 4)
  // ...
  // m_buckets[32] == [2^31, 2^32)
  std::array<std::atomic<uint64_t>, 8 * sizeof(uint32_t) + 1> m_buckets;
};

struct ArrayTracer {

  // Detail: Internally AtomicHashMap is going to use a full 8 bytes to store
  // ArrayUsage, so we can grow another 7 bytes before we'll be in any trouble
  typedef folly::AtomicHashMap<const ArrayData*, ArrayUsage> ArrayUsageMap;

public:
  ArrayTracer() : m_numCows{0}, m_numPackedToMixed{0}, m_numUnknownReleases{0},
    m_kindCntrs(ArrayData::kNumKinds),
    m_classCntrs(ArrayClass::NumArrayClasses),
    m_staticKindCntrs(ArrayData::kNumKinds),
    m_staticClassCntrs(ArrayClass::NumArrayClasses)
  {}

  /*
   * Mutates the ArrayFunctions g_array_funcs vtable to contain tracked
   * methods.
   */
  static void turnOnTracing();

  /*
   * Initializes the collection of alive arrays to the empty collection.
   */
  void requestStart();

  /*
   * Goes through the collection of still alive arrays and syncs their
   * information back to their respective LocationKeys in the usage map.
   */
  void requestEnd();

  /*
   * Classify the ArrayUsage as being a kind of checked array
   */
  ArrayClass classifyUsage(const ArrayData::ArrayKind kind,
                           const ArrayUsage usage);
  const char* usageToCStr(const ArrayUsage u);

  /*
   * Bump the frequency counter for the given ArrayKind/ArrayFunc pair
   */
  void bumpArrayFuncStats(ArrayData::ArrayKind kind,
                          ArrayFunc func);

public:
  template <class UpdateUsage>
  ArrayUsage registerArray(const ArrayData* ad,
                           ArrayUsage usg,
                           UpdateUsage updateUsage,
                           bool isStatic = false) {
    // isStatic flag is an override hook for nonSmartCopy to hint that
    // the returned ArrayData, while not yet set as static yet, will be
    assert(ad->kind() == ArrayData::kMixedKind ||
           ad->kind() == ArrayData::kPackedKind ||
           ad->kind() == ArrayData::kEmptyKind);
    if (ad->isStatic() || isStatic) {
      auto pair = staticArrays()->insert(ad, usg);
      if (pair.second) {
        readElementsUsage(ad, pair.first->second);
      }
      updateUsage(pair.first->second);
      return pair.first->second;
    } else {
      auto pair = liveArrays()->emplace(ad, usg);
      updateUsage(pair.first->second);
      return pair.first->second;
    }
  }

  template <class UpdateUsage, class ArrayOp>
  ArrayData* handleArrayFuncMutable(const ArrayData* ad,
                                    bool copy,
                                    UpdateUsage updateUsage,
                                    ArrayOp arrayOp,
                                    ArrayFunc func) {
    // (1) Register ad, which we may or may not have seen before and
    // record what its previous usage was.
    auto oldUsage = registerArray(ad, ArrayUsage{}, [&](ArrayUsage& usg){});

    // (2) Record information about ad before it (possibly) gets mutated
    auto oldKind = ad->kind();
    // (2a) Bump the ArrayFunc frequency stats
    bumpArrayFuncStats(oldKind, func);

    // (3) Perform the array operation
    auto ret = arrayOp();

    // (4) Inspect to see if the function either performed
    // (a) Copy-On-Write,
    // (b) Changed kinds, e.g. from kPackedKind to kMixedKind
    // In all cases, we will propagate the oldUsage
    if (ret != ad) {
      if (copy) {
        // (a) Copy-On-Write: Perform the updateUsage on the copy
        if (oldKind != ArrayData::kEmptyKind) {
          // Filter out CoWs due to the EmptyArray
          m_numCows++;
        }
      }
      if (ret->kind() != oldKind) {
        // (b) Changed kinds: Perform the updateUsage on the new
        // kinded array
        if (oldKind == ArrayData::kPackedKind &&
            ret->kind() == ArrayData::kMixedKind) {
          m_numPackedToMixed++;
        }
      }
      registerArray(ret, oldUsage, updateUsage,
                    func == ArrayFunc::NonSmartCopy);
    }

    return ret;
  }

  template <class UpdateUsage>
  void handleArrayFunc(const ArrayData* ad,
                       UpdateUsage updateUsage,
                       ArrayFunc func) {
    registerArray(ad, ArrayUsage{}, updateUsage);
    bumpArrayFuncStats(ad->kind(), func);
  }

  void updateStaticArrayStats();
  void handleRelease(const ArrayData* ad);
  void handleAccounting(const ArrayData* ad);

  // Non g_array_funcs helpers for member-operations shenanigans
  void handleTakenByRef(ArrayData* ad);

  // Way to dump the information gathered
  void dumpToFile(const std::string& filename);
  Array dumpToArray();

////////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////////
 private:
  /*
   * Mutates the ArrayFunctions g_array_funcs vtable to contain tracked
   * methods
   */
  static void mutateArrayFunctions();
  /*
   * Read through the elements of an array (usually static) and update the
   * information we have about it.
   */
  void readElementsUsage(const ArrayData* ad, ArrayUsage& usage);

  static std::unordered_map<const ArrayData*, ArrayUsage>* liveArrays();
  static ArrayUsageMap* staticArrays();

////////////////////////////////////////////////////////////////////////////////
// Private members
////////////////////////////////////////////////////////////////////////////////
  std::atomic<uint64_t> m_numCows;
  std::atomic<uint64_t> m_numPackedToMixed;
  std::atomic<uint64_t> m_numUnknownReleases;

  std::mutex m_statsMutex;
  std::vector<uint64_t> m_kindCntrs;
  std::vector<uint64_t> m_classCntrs;
  std::vector<uint64_t> m_staticKindCntrs;
  std::vector<uint64_t> m_staticClassCntrs;

  LogarithmicHistogram m_noInfoSizes;
  // kNumKind * kNumArrayFuncs matrix of counters
  static_assert(sizeof(g_array_funcs_unmodified) %
                sizeof(g_array_funcs_unmodified.release) == 0,
                "The calculation below will be incorrect if it's not"
                " a strict multiple");
  static constexpr size_t kNumArrayFuncs = sizeof(g_array_funcs_unmodified)
    / sizeof(g_array_funcs_unmodified.release);
  std::array<std::atomic<uint64_t>,
    (static_cast<size_t>(ArrayData::kNumKinds) *
     kNumArrayFuncs)> m_atomicArrayFuncCntrs;
};

ArrayTracer* getArrayTracer();

void array_tracer_dump(const std::string& filename);

/*
 * ArrayFunctionsWrapper holds a templated version of every function in
 * ArrayFunctions. Every function actually dispatches the work to
 * g_array_funcs_unmodified, and calls into the ArrayTracer to do some
 * bookkeeping.
 *
 * This bookkeeping mostly consists of remembering which ArrayData pointers
 * we've seen, and associating with them an ArrayUsage, which these functions
 * might update. For example, the ArrayFunctionsWrapper::setStr function would
 * set the string key flag on the associated ArrayUsage.
 */
struct ArrayFunctionsWrapper {

  template <ArrayData::ArrayKind kind>
  static void release(ArrayData* ad) {
    // Release MUST be re-entrant due to cyclic nature of arrays
    getArrayTracer()->handleRelease(ad);
    return g_array_funcs_unmodified.release[kind](ad);
  }

  template <ArrayData::ArrayKind kind>
  static const TypedValue* nvGetInt(const ArrayData* ad, int64_t ikey) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::NvGetInt);
    return g_array_funcs_unmodified.nvGetInt[kind](ad, ikey);
  }

  template <ArrayData::ArrayKind kind>
  static const TypedValue* nvGetIntConverted(const ArrayData* ad,
                                             int64_t ikey) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::NvGetIntConverted);
    return g_array_funcs_unmodified.nvGetIntConverted[kind](ad, ikey);
  }

  template <ArrayData::ArrayKind kind>
  static const TypedValue* nvGetStr(const ArrayData* ad,
                                    const StringData* skey) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::NvGetStr);
    return g_array_funcs_unmodified.nvGetStr[kind](ad, skey);
  }

  template <ArrayData::ArrayKind kind>
  static void nvGetKey(const ArrayData* ad, TypedValue* out, ssize_t pos) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::NvGetKey);
    g_array_funcs_unmodified.nvGetKey[kind](ad, out, pos);
    getArrayTracer()->registerArray(ad, ArrayUsage{},
        // We expect to find this array, we're just now going to update
        // the key type based on information we've learned
        [&](ArrayUsage& usage) {
          if (out->m_type == KindOfInt64) {
            setIntKey(usage);
          } else {
            assert(IS_STRING_TYPE(out->m_type));
            setStrKey(usage);
          }
        });
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* setInt(ArrayData* ad, int64_t ikey, Cell v, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {
          setIntKey(usage);
        },
        [&]() {
          return g_array_funcs_unmodified.setInt[kind](ad, ikey, v, copy);
        },
        ArrayFunc::SetInt);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* setIntConverted(ArrayData* ad, int64_t ikey,
                                    Cell v, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {
          setIntKey(usage);
        },
        [&]() {
          return g_array_funcs_unmodified.setIntConverted[kind](ad, ikey,
                                                                v, copy);
        },
        ArrayFunc::SetIntConverted);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* setStr(ArrayData* ad, StringData* skey, Cell v, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {
          setStrKey(usage);
        },
        [&](){
          return g_array_funcs_unmodified.setStr[kind](ad, skey, v, copy);
        },
        ArrayFunc::SetStr);
  }

  template <ArrayData::ArrayKind kind>
  static size_t vsize(const ArrayData* ad) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::Vsize);
    return g_array_funcs_unmodified.vsize[kind](ad);
  }

  template <ArrayData::ArrayKind kind>
  static const Variant& getValueRef(const ArrayData* ad, ssize_t pos) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::GetValueRef);
    auto& ret = g_array_funcs_unmodified.getValueRef[kind](ad, pos);
    getArrayTracer()->registerArray(ad, ArrayUsage{},
        // We expect to find this array, we're just now going to update
        // the value type based on information we've learned
        [&](ArrayUsage& usage) {
          if (ret.getType() == KindOfRef) {
            setReffy(usage);
          }
        });
    return ret;
  }

  template <ArrayData::ArrayKind kind>
  static bool isVectorData(const ArrayData* ad) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::IsVectorData);
    return g_array_funcs_unmodified.isVectorData[kind](ad);
  }

  template <ArrayData::ArrayKind kind>
  static bool existsInt(const ArrayData* ad, int64_t ikey) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::ExistsInt);
    return g_array_funcs_unmodified.existsInt[kind](ad, ikey);
  }

  template <ArrayData::ArrayKind kind>
  static bool existsStr(const ArrayData* ad, const StringData* skey) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::ExistsStr);
    return g_array_funcs_unmodified.existsStr[kind](ad, skey);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* lvalInt(ArrayData* ad, int64_t ikey,
                            Variant*& ret, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {
          setIntKey(usage);
        },
        [&](){
          return g_array_funcs_unmodified.lvalInt[kind](ad, ikey, ret, copy);
        },
        ArrayFunc::LvalInt);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* lvalStr(ArrayData* ad, StringData* skey,
                            Variant*& ret, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {
          setStrKey(usage);
        },
        [&](){
          return g_array_funcs_unmodified.lvalStr[kind](ad, skey, ret, copy);
        },
        ArrayFunc::LvalStr);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* lvalNew(ArrayData* ad, Variant*& ret, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {
          setIntKey(usage);
        },
        [&](){
          return g_array_funcs_unmodified.lvalNew[kind](ad, ret, copy);
        },
        ArrayFunc::LvalNew);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* lvalNewRef(ArrayData* ad, Variant*& ret, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {
          setIntKey(usage);
          setReffy(usage);
        },
        [&](){
          return g_array_funcs_unmodified.lvalNewRef[kind](ad, ret, copy);
        },
        ArrayFunc::LvalNewRef);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* setRefInt(ArrayData* ad, int64_t ikey,
                              Variant& v, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {
          setIntKey(usage);
          setReffy(usage);
        },
        [&](){
          return g_array_funcs_unmodified.setRefInt[kind](ad, ikey, v, copy);
        },
        ArrayFunc::SetRefInt);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* setRefStr(ArrayData* ad, StringData* skey,
                              Variant& v, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {
          setStrKey(usage);
          setReffy(usage);
        },
        [&](){
          return g_array_funcs_unmodified.setRefStr[kind](ad, skey, v, copy);
        },
        ArrayFunc::SetRefStr);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* addInt(ArrayData* ad, int64_t ikey, Cell v, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {
          setIntKey(usage);
        },
        [&](){
          return g_array_funcs_unmodified.addInt[kind](ad, ikey, v, copy);
        },
        ArrayFunc::AddInt);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* addStr(ArrayData* ad, StringData* skey, Cell v, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {
          setStrKey(usage);
        },
        [&](){
          return g_array_funcs_unmodified.addStr[kind](ad, skey, v, copy);
        },
        ArrayFunc::AddStr);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* removeInt(ArrayData* ad, int64_t ikey, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {},
        [&]() {
          return g_array_funcs_unmodified.removeInt[kind](ad, ikey, copy);
        },
        ArrayFunc::RemoveInt);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* removeStr(ArrayData* ad, const StringData* skey,
                              bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {},
        [&](){
          return g_array_funcs_unmodified.removeStr[kind](ad, skey, copy);
        },
        ArrayFunc::RemoveStr);
  }

  template <ArrayData::ArrayKind kind>
  static ssize_t iterBegin(const ArrayData* ad) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::IterBegin);
    return g_array_funcs_unmodified.iterBegin[kind](ad);
  }

  template <ArrayData::ArrayKind kind>
  static ssize_t iterLast(const ArrayData* ad) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::IterLast);
    return g_array_funcs_unmodified.iterLast[kind](ad);
  }

  template <ArrayData::ArrayKind kind>
  static ssize_t iterEnd(const ArrayData* ad) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::IterEnd);
    return g_array_funcs_unmodified.iterEnd[kind](ad);
  }

  template <ArrayData::ArrayKind kind>
  static ssize_t iterAdvance(const ArrayData* ad, ssize_t pos) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::IterAdvance);
    return g_array_funcs_unmodified.iterAdvance[kind](ad, pos);
  }

  template <ArrayData::ArrayKind kind>
  static ssize_t iterRewind(const ArrayData* ad, ssize_t pos) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::IterRewind);
    return g_array_funcs_unmodified.iterRewind[kind](ad, pos);
  }

  template <ArrayData::ArrayKind kind>
  static bool validMArrayIter(const ArrayData* ad, const MArrayIter& fp) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {
                                        setReffy(usage);
                                      },
                                      ArrayFunc::ValidMArrayIter);
    return g_array_funcs_unmodified.validMArrayIter[kind](ad, fp);
  }

  template <ArrayData::ArrayKind kind>
  static bool advanceMArrayIter(ArrayData* ad, MArrayIter& fp) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {
                                        setReffy(usage);
                                      },
                                      ArrayFunc::AdvanceMArrayIter);
    return g_array_funcs_unmodified.advanceMArrayIter[kind](ad, fp);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* escalateForSort(ArrayData* ad) {
    return getArrayTracer()->handleArrayFuncMutable(ad, false,
        [&](ArrayUsage& usage) {},
        [&]() {
          return g_array_funcs_unmodified.escalateForSort[kind](ad);
        },
        ArrayFunc::EscalateForSort);
  }

  template <ArrayData::ArrayKind kind>
  static void ksort(ArrayData* ad, int sort_flags, bool ascending) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::Ksort);
    return g_array_funcs_unmodified.ksort[kind](ad, sort_flags, ascending);
  }

  template <ArrayData::ArrayKind kind>
  static void sort(ArrayData* ad, int sort_flags, bool ascending) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::Sort);
    return g_array_funcs_unmodified.sort[kind](ad, sort_flags, ascending);
  }

  template <ArrayData::ArrayKind kind>
  static void asort(ArrayData* ad, int sort_flags, bool ascending) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::Asort);
    return g_array_funcs_unmodified.asort[kind](ad, sort_flags, ascending);
  }

  template <ArrayData::ArrayKind kind>
  static bool uksort(ArrayData* ad, const Variant& cmp_function) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::Uksort);
    return g_array_funcs_unmodified.uksort[kind](ad, cmp_function);
  }

  template <ArrayData::ArrayKind kind>
  static bool usort(ArrayData* ad, const Variant& cmp_function) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::Usort);
    return g_array_funcs_unmodified.usort[kind](ad, cmp_function);
  }

  template <ArrayData::ArrayKind kind>
  static bool uasort(ArrayData* ad, const Variant& cmp_function) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::Uasort);
    return g_array_funcs_unmodified.uasort[kind](ad, cmp_function);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* copy(const ArrayData* ad) {
    return getArrayTracer()->handleArrayFuncMutable(ad, false,
        [&](ArrayUsage& usage) {},
        [&](){
          return g_array_funcs_unmodified.copy[kind](ad);
        },
        ArrayFunc::Copy);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* copyWithStrongIterators(const ArrayData* ad) {
    return getArrayTracer()->handleArrayFuncMutable(ad, false,
        [&](ArrayUsage& usage) {},
        [&](){
          return g_array_funcs_unmodified.copyWithStrongIterators[kind](ad);
        },
        ArrayFunc::CopyWithStrongIterators);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* nonSmartCopy(const ArrayData* ad) {
    // Whatever we return here is actually going to be a static array...
    return getArrayTracer()->handleArrayFuncMutable(ad, false,
        [&](ArrayUsage& usage) {},
        [&](){
          return g_array_funcs_unmodified.nonSmartCopy[kind](ad);
        },
        ArrayFunc::NonSmartCopy);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* append(ArrayData* ad, const Variant& v, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {
          setIntKey(usage);
        },
        [&](){
          return g_array_funcs_unmodified.append[kind](ad, v, copy);
        },
        ArrayFunc::Append);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* appendRef(ArrayData* ad, Variant& v, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {
          setIntKey(usage);
          setReffy(usage);
        },
        [&](){
          return g_array_funcs_unmodified.appendRef[kind](ad, v, copy);
        },
        ArrayFunc::AppendRef);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* appendWithRef(ArrayData* ad, const Variant& v, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {
          setIntKey(usage);
          setReffy(usage);
        },
        [&]() {
          return g_array_funcs_unmodified.appendWithRef[kind](ad, v, copy);
        },
        ArrayFunc::AppendWithRef);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* plusEq(ArrayData* ad, const ArrayData* elems) {
    ArrayUsage elemsUsage{};
    if (elems->kind() == ArrayData::kMixedKind ||
        elems->kind() == ArrayData::kPackedKind) {
      // registerArray asserts that the kind of elems is one of the kinds we've
      //  actively overridden. This is not necessarily true for elems, so if
      // it's possible we do have info about it look it up, else default to
      // no usage information.
      elemsUsage =
        getArrayTracer()->registerArray(elems, ArrayUsage{},
                                        [&](ArrayUsage& usg){});
    }
    return getArrayTracer()->handleArrayFuncMutable(ad, ad->hasMultipleRefs(),
        [&](ArrayUsage& usage) {
          mergeArrayUsage(elemsUsage, usage);
        },
        [&](){
          return g_array_funcs_unmodified.plusEq[kind](ad, elems);
        },
        ArrayFunc::PlusEq);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* merge(ArrayData* ad, const ArrayData* elems) {
    ArrayUsage elemsUsage{};
    if (elems->kind() == ArrayData::kMixedKind ||
        elems->kind() == ArrayData::kPackedKind) {
      // registerArray asserts that the kind of elems is one of the kinds we've
      //  actively overridden. This is not necessarily true for elems, so if
      // it's possible we do have info about it look it up, else default to
      // no usage information.
      elemsUsage =
        getArrayTracer()->registerArray(elems, ArrayUsage{},
                                        [&](ArrayUsage& usg){});
    }
    /* copy is set to false because Merge always returns a new ArrayData */
    return getArrayTracer()->handleArrayFuncMutable(ad, false,
        [&](ArrayUsage& usage) {
          mergeArrayUsage(elemsUsage, usage);
        },
        [&](){
          return g_array_funcs_unmodified.merge[kind](ad, elems);
        },
        ArrayFunc::Merge);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* pop(ArrayData* ad, Variant& value) {
    return getArrayTracer()->handleArrayFuncMutable(ad, ad->hasMultipleRefs(),
        [&](ArrayUsage& usage) {},
        [&](){
          return g_array_funcs_unmodified.pop[kind](ad, value);
        },
        ArrayFunc::Pop);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* dequeue(ArrayData* ad, Variant& value) {
    return getArrayTracer()->handleArrayFuncMutable(ad, ad->hasMultipleRefs(),
        [&](ArrayUsage& usage) {},
        [&](){
          return g_array_funcs_unmodified.dequeue[kind](ad, value);
        },
        ArrayFunc::Dequeue);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* prepend(ArrayData* ad, const Variant& v, bool copy) {
    return getArrayTracer()->handleArrayFuncMutable(ad, copy,
        [&](ArrayUsage& usage) {
          setIntKey(usage);
        },
        [&](){
          return g_array_funcs_unmodified.prepend[kind](ad, v, copy);
        },
        ArrayFunc::Prepend);
  }

  template <ArrayData::ArrayKind kind>
  static void renumber(ArrayData* ad) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::Renumber);
    return g_array_funcs_unmodified.renumber[kind](ad);
  }

  template <ArrayData::ArrayKind kind>
  static void onSetEvalScalar(ArrayData* ad) {
    getArrayTracer()->handleArrayFunc(ad,
                                      [&](ArrayUsage& usage) {},
                                      ArrayFunc::OnSetEvalScalar);
    return g_array_funcs_unmodified.onSetEvalScalar[kind](ad);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* escalate(const ArrayData* ad) {
    return getArrayTracer()->handleArrayFuncMutable(ad, false,
        [&](ArrayUsage& usage) {},
        [&]() {
          return g_array_funcs_unmodified.escalate[kind](ad);
        },
        ArrayFunc::Escalate);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* zSetInt(ArrayData* ad, int64_t ikey, RefData* v) {
    return getArrayTracer()->handleArrayFuncMutable(ad, false,
        [&](ArrayUsage& usage) {
          setIntKey(usage);
        },
        [&]() {
          return g_array_funcs_unmodified.zSetInt[kind](ad, ikey, v);
        },
        ArrayFunc::ZSetInt);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* zSetStr(ArrayData* ad, StringData* skey, RefData* v) {
    return getArrayTracer()->handleArrayFuncMutable(ad, false,
        [&](ArrayUsage& usage) {
          setStrKey(usage);
        },
        [&]() {
          return g_array_funcs_unmodified.zSetStr[kind](ad, skey, v);
        },
        ArrayFunc::ZSetStr);
  }

  template <ArrayData::ArrayKind kind>
  static ArrayData* zAppend(ArrayData* ad, RefData* v, int64_t* key_ptr) {
    return getArrayTracer()->handleArrayFuncMutable(ad, false,
        [&](ArrayUsage& usage) {
          setIntKey(usage);
        },
        [&]() {
          return g_array_funcs_unmodified.zAppend[kind](ad, v, key_ptr);
        },
        ArrayFunc::ZAppend);
  }

};

} // HPHP::

#endif

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

#include "hphp/runtime/ext/array-tracer/ext_array_tracer.h"

#include <fstream>

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/system/systemlib.h"

namespace HPHP {

typedef folly::AtomicHashMap<const ArrayData*, ArrayUsage> ArrayUsageMap;

ArrayTracer* getArrayTracer() {
  always_assert(RuntimeOption::EvalTraceArrays);
  static ArrayTracer arrayTracer{};
  return &arrayTracer;
}

std::unordered_map<const ArrayData*, ArrayUsage>* ArrayTracer::liveArrays() {
  static thread_local std::unordered_map<const ArrayData*, ArrayUsage>
    tl_liveArrays{};
  return &tl_liveArrays;
}

static ArrayUsageMap* s_staticArrays{nullptr};

ArrayUsageMap* ArrayTracer::staticArrays() {
  return s_staticArrays;
}

void array_tracer_dump(const std::string& filename) {
  getArrayTracer()->dumpToFile(filename);
}

// Important that we use g_array_funcs here not _unmodified, otherwise we'd
// blow away our changes on subsequent uses.
// ArrayFunctionsWrapper needs a corresponding template for every function
// in ArrayFunctions to intercept it and record metadata about the operation
#define DISPATCH(entry, kind)                           \
{                                                       \
  (kind == 0 ? ArrayFunctionsWrapper::entry<kind>       \
             : g_array_funcs.entry[0]),                 \
  (kind == 1 ? ArrayFunctionsWrapper::entry<kind>       \
             : g_array_funcs.entry[1]),                 \
  (kind == 2 ? ArrayFunctionsWrapper::entry<kind>       \
             : g_array_funcs.entry[2]),                 \
  (kind == 3 ? ArrayFunctionsWrapper::entry<kind>       \
             : g_array_funcs.entry[3]),                 \
  (kind == 4 ? ArrayFunctionsWrapper::entry<kind>       \
             : g_array_funcs.entry[4]),                 \
  (kind == 5 ? ArrayFunctionsWrapper::entry<kind>       \
             : g_array_funcs.entry[5]),                 \
  (kind == 6 ? ArrayFunctionsWrapper::entry<kind>       \
             : g_array_funcs.entry[6]),                 \
  (kind == 7 ? ArrayFunctionsWrapper::entry<kind>       \
             : g_array_funcs.entry[7]),                 \
  (kind == 8 ? ArrayFunctionsWrapper::entry<kind>       \
             : g_array_funcs.entry[8]),                 \
  (kind == 9 ? ArrayFunctionsWrapper::entry<kind>       \
             : g_array_funcs.entry[9]),                 \
},

template<ArrayData::ArrayKind kind>
static void mutateArrayFunctionsForKind() {
  static_assert(ArrayData::ArrayKind::kNumKinds == 10,
                "Implicit assumption in DISPATCH macro above");
  ArrayFunctions wrappedFuncs = {
    DISPATCH(release, kind)
    DISPATCH(nvGetInt, kind)
    DISPATCH(nvGetIntConverted, kind)
    DISPATCH(nvGetStr, kind)
    DISPATCH(nvGetKey, kind)
    DISPATCH(setInt, kind)
    DISPATCH(setIntConverted, kind)
    DISPATCH(setStr, kind)
    DISPATCH(vsize, kind)
    DISPATCH(getValueRef, kind)
    DISPATCH(isVectorData, kind)
    DISPATCH(existsInt, kind)
    DISPATCH(existsStr, kind)
    DISPATCH(lvalInt, kind)
    DISPATCH(lvalStr, kind)
    DISPATCH(lvalNew, kind)
    DISPATCH(lvalNewRef, kind)
    DISPATCH(setRefInt, kind)
    DISPATCH(setRefStr, kind)
    DISPATCH(addInt, kind)
    DISPATCH(addStr, kind)
    DISPATCH(removeInt, kind)
    DISPATCH(removeStr, kind)
    DISPATCH(iterBegin, kind)
    DISPATCH(iterLast, kind)
    DISPATCH(iterEnd, kind)
    DISPATCH(iterAdvance, kind)
    DISPATCH(iterRewind, kind)
    DISPATCH(validMArrayIter, kind)
    DISPATCH(advanceMArrayIter, kind)
    DISPATCH(escalateForSort, kind)
    DISPATCH(ksort, kind)
    DISPATCH(sort, kind)
    DISPATCH(asort, kind)
    DISPATCH(uksort, kind)
    DISPATCH(usort, kind)
    DISPATCH(uasort, kind)
    DISPATCH(copy, kind)
    DISPATCH(copyWithStrongIterators, kind)
    DISPATCH(nonSmartCopy, kind)
    DISPATCH(append, kind)
    DISPATCH(appendRef, kind)
    DISPATCH(appendWithRef, kind)
    DISPATCH(plusEq, kind)
    DISPATCH(merge, kind)
    DISPATCH(pop, kind)
    DISPATCH(dequeue, kind)
    DISPATCH(prepend, kind)
    DISPATCH(renumber, kind)
    DISPATCH(onSetEvalScalar, kind)
    DISPATCH(escalate, kind)
    DISPATCH(zSetInt, kind)
    DISPATCH(zSetStr, kind)
    DISPATCH(zAppend, kind)
  };

  g_array_funcs = wrappedFuncs;
}

#undef DISPATCH

void ArrayTracer::mutateArrayFunctions() {
  mutateArrayFunctionsForKind<ArrayData::ArrayKind::kEmptyKind>();
  mutateArrayFunctionsForKind<ArrayData::ArrayKind::kPackedKind>();
  mutateArrayFunctionsForKind<ArrayData::ArrayKind::kMixedKind>();
}

void ArrayTracer::turnOnTracing() {
  static std::once_flag s_onceFlag;
  std::call_once(s_onceFlag,
                 [&]() {
                   ArrayTracer::mutateArrayFunctions();
                   // 2^16 = 65536
                   constexpr size_t estimatedNumStaticArrays = 1 << 16;
                   s_staticArrays = new ArrayUsageMap{estimatedNumStaticArrays};
                 });
}

void ArrayTracer::requestStart() {
  *liveArrays() = std::unordered_map<const ArrayData*, ArrayUsage>{};
}

void ArrayTracer::requestEnd() {
  // NOTE: ArrayIterator, ArrayInit, and maybe others, call
  // MixedArray::Release or PackedArray::Release without going through
  // handleRelease, this means we can easily be trying to read freed
  // memory. In this case we'll unfortunately have to ignore the arrays
  // that are in fact still alive at the end of a request and not
  // try to count them.
  *liveArrays() = std::unordered_map<const ArrayData*, ArrayUsage>{};
}

void ArrayTracer::bumpArrayFuncStats(ArrayData::ArrayKind kind,
                                     ArrayFunc func) {
  const auto sKind = static_cast<size_t>(kind);
  const auto sFunc = static_cast<size_t>(func);
  const auto numFuncs = kNumArrayFuncs;
  m_atomicArrayFuncCntrs[(sKind * numFuncs) + sFunc]++;
}

void ArrayTracer::updateStaticArrayStats() {
  std::vector<uint64_t> staticKindCntrs(ArrayData::kNumKinds);
  std::vector<uint64_t> staticClassCntrs(ArrayClass::NumArrayClasses);
  for (const auto& pair : *staticArrays()) {
    const auto ad = pair.first;
    const auto usage = pair.second;
    auto kind = ad->kind();
    always_assert(kind < ArrayData::kNumKinds);
    staticKindCntrs[kind]++;
    staticClassCntrs[classifyUsage(kind, usage)]++;
  }
  {
    std::lock_guard<std::mutex> statsLock(m_statsMutex);
    m_staticKindCntrs = staticKindCntrs;
    m_staticClassCntrs = staticClassCntrs;
  }
}

bool usageIsSane(const ArrayUsage u) {
  static const uint8_t maxUsageBitVec = [](){
    uint8_t bitVec{0};
    bitVec |= UsageBits::Reffy;
    bitVec |= UsageBits::StrKey;
    bitVec |= UsageBits::IntKey;
    return bitVec;
  }();
  return u.usageBitVec <= maxUsageBitVec;
}

ArrayClass ArrayTracer::classifyUsage(const ArrayData::ArrayKind kind,
                                      const ArrayUsage u) {
  always_assert(usageIsSane(u));
  if (isIntKey(u) && isStrKey(u)) {
    always_assert(kind == ArrayData::kMixedKind);
    return ArrayClass::MixedMap;
  } else if (isIntKey(u)) {
    if (kind == ArrayData::kMixedKind) {
      return ArrayClass::MixedIntMap;
    } else {
      always_assert(kind == ArrayData::kPackedKind);
      return ArrayClass::PackedVector;
    }
  } else if (isStrKey(u)) {
    always_assert(kind == ArrayData::kMixedKind);
    return ArrayClass::MixedStrMap;
  } else if (kind == ArrayData::kEmptyKind) {
    return ArrayClass::Empty;
  } else {
    return ArrayClass::NoInfo;
  }
}

const char* classToCStr(const ArrayClass aClass) {
  static std::array<const char*,6> names = {{
    "MixedIntMap",
    "MixedStrMap",
    "MixedMap",
    "PackedVector",
    "Empty",
    "NoInfo",
  }};
  static_assert(names.size() == ArrayClass::NumArrayClasses,
                "Update names with new ArrayClass");
  return names[aClass];
}

const char* arrayFuncToCStr(const ArrayFunc func) {
  switch (func) {
    case ArrayFunc::Release:
      return "Release";
    case ArrayFunc::NvGetInt:
      return "NvGetInt";
    case ArrayFunc::NvGetIntConverted:
      return "NvGetIntConverted";
    case ArrayFunc::NvGetStr:
      return "NvGetStr";
    case ArrayFunc::NvGetKey:
      return "NvGetKey";
    case ArrayFunc::SetInt:
      return "SetInt";
    case ArrayFunc::SetIntConverted:
      return "SetIntConverted";
    case ArrayFunc::SetStr:
      return "SetStr";
    case ArrayFunc::Vsize:
      return "Vsize";
    case ArrayFunc::GetValueRef:
      return "GetValueRef";
    case ArrayFunc::IsVectorData:
      return "IsVectorData";
    case ArrayFunc::ExistsInt:
      return "ExistsInt";
    case ArrayFunc::ExistsStr:
      return "ExistsStr";
    case ArrayFunc::LvalInt:
      return "LvalInt";
    case ArrayFunc::LvalStr:
      return "LvalStr";
    case ArrayFunc::LvalNew:
      return "LvalNew";
    case ArrayFunc::LvalNewRef:
      return "LvalNewRef";
    case ArrayFunc::SetRefInt:
      return "SetRefInt";
    case ArrayFunc::SetRefStr:
      return "SetRefStr";
    case ArrayFunc::AddInt:
      return "AddInt";
    case ArrayFunc::AddStr:
      return "AddStr";
    case ArrayFunc::RemoveInt:
      return "RemoveInt";
    case ArrayFunc::RemoveStr:
      return "RemoveStr";
    case ArrayFunc::IterBegin:
      return "IterBegin";
    case ArrayFunc::IterLast:
      return "IterLast";
    case ArrayFunc::IterEnd:
      return "IterEnd";
    case ArrayFunc::IterAdvance:
      return "IterAdvance";
    case ArrayFunc::IterRewind:
      return "IterRewind";
    case ArrayFunc::ValidMArrayIter:
      return "ValidMArrayIter";
    case ArrayFunc::AdvanceMArrayIter:
      return "AdvanceMArrayIter";
    case ArrayFunc::EscalateForSort:
      return "EscalateForSort";
    case ArrayFunc::Ksort:
      return "Ksort";
    case ArrayFunc::Sort:
      return "Sort";
    case ArrayFunc::Asort:
      return "Asort";
    case ArrayFunc::Uksort:
      return "Uksort";
    case ArrayFunc::Usort:
      return "Usort";
    case ArrayFunc::Uasort:
      return "Uasort";
    case ArrayFunc::Copy:
      return "Copy";
    case ArrayFunc::CopyWithStrongIterators:
      return "CopyWithStrongIterators";
    case ArrayFunc::NonSmartCopy:
      return "NonSmartCopy";
    case ArrayFunc::Append:
      return "Append";
    case ArrayFunc::AppendRef:
      return "AppendRef";
    case ArrayFunc::AppendWithRef:
      return "AppendWithRef";
    case ArrayFunc::PlusEq:
      return "PlusEq";
    case ArrayFunc::Merge:
      return "Merge";
    case ArrayFunc::Pop:
      return "Pop";
    case ArrayFunc::Dequeue:
      return "Dequeue";
    case ArrayFunc::Prepend:
      return "Prepend";
    case ArrayFunc::Renumber:
      return "Renumber";
    case ArrayFunc::OnSetEvalScalar:
      return "OnSetEvalScalar";
    case ArrayFunc::Escalate:
      return "Escalate";
    case ArrayFunc::ZSetInt:
      return "ZSetInt";
    case ArrayFunc::ZSetStr:
      return "ZSetStr";
    case ArrayFunc::ZAppend:
      return "ZAppend";
  }
  always_assert(false); // unreachable
}

static const StaticString s_numCows("numCopyOnWrites");
static const StaticString s_numPackedToMixed("numPackedtoMixed");
static const StaticString s_numUnknownReleases("numUnknownReleases");

void ArrayTracer::dumpToFile(const std::string& filename) {
  std::ofstream out(filename.c_str());
  std::vector<uint64_t> classCntrsCopy;
  std::vector<uint64_t> kindCntrsCopy;
  std::vector<uint64_t> staticClassCntrsCopy;
  std::vector<uint64_t> staticKindCntrsCopy;
  std::vector<uint64_t> arrayFuncCntrsCopy(m_atomicArrayFuncCntrs.size());

  updateStaticArrayStats();
  {
    std::lock_guard<std::mutex> statsLock(m_statsMutex);
    classCntrsCopy = m_classCntrs;
    kindCntrsCopy = m_kindCntrs;
    staticClassCntrsCopy = m_staticClassCntrs;
    staticKindCntrsCopy = m_staticKindCntrs;
  }
  for (size_t i = 0; i < m_atomicArrayFuncCntrs.size(); i++) {
    arrayFuncCntrsCopy[i] = m_atomicArrayFuncCntrs[i].load();
  }
  out << "===Array Func Calls By Kind===\n";
  for (size_t kind = 0;
       static_cast<ArrayData::ArrayKind>(kind) < ArrayData::kNumKinds;
       kind++) {
    out << "==="
        << ArrayData::kindToString(static_cast<ArrayData::ArrayKind>(kind))
        << "===\n";
    for (size_t arrFunc = 0;
         arrFunc < kNumArrayFuncs;
         arrFunc++) {
      const auto count =
        arrayFuncCntrsCopy[kind * kNumArrayFuncs
                           + arrFunc];
      out << arrayFuncToCStr(static_cast<ArrayFunc>(arrFunc))
          << ": " << count << "\n";
    }
  }
  out << "===Request Local Arrays===\n";
  for (auto i = 0; i < classCntrsCopy.size(); i++) {
    const auto arrayClass = static_cast<ArrayClass>(i);
    const auto count = classCntrsCopy[i];
    out << classToCStr(arrayClass) << ": " << count << "\n";
  }
  for (auto i = 0; i < kindCntrsCopy.size(); i++) {
    const auto arrayKind =
      static_cast<ArrayData::ArrayKind>(i);
    const auto count = kindCntrsCopy[i];
    out << ArrayData::kindToString(arrayKind) << ": " << count << "\n";
  }
  out << "===Static Arrays===\n";
  for (auto i = 0; i < staticClassCntrsCopy.size(); i++) {
    const auto arrayClass = static_cast<ArrayClass>(i);
    const auto count = staticClassCntrsCopy[i];
    out << classToCStr(arrayClass) << ": " << count << "\n";
  }
  for (auto i = 0; i < staticKindCntrsCopy.size(); i++) {
    const auto arrayKind =
      static_cast<ArrayData::ArrayKind>(i);
    const auto count = staticKindCntrsCopy[i];
    out << ArrayData::kindToString(arrayKind) << ": " << count << "\n";
  }

  out << "===General Stats===\n";
  out << s_numCows.data() << ": " << m_numCows.load() << "\n";
  out << s_numPackedToMixed.data() << ": "
      << m_numPackedToMixed.load() << "\n";
  out << s_numUnknownReleases.data() << ": "
      << m_numUnknownReleases.load() << "\n";
  out << "===No Info Size Histogram===\n"
      << m_noInfoSizes.toString();

  out.close();
}

Array ArrayTracer::dumpToArray() {
  updateStaticArrayStats();
  std::vector<uint64_t> classCntrsCopy;
  std::vector<uint64_t> kindCntrsCopy;
  std::vector<uint64_t> staticClassCntrsCopy;
  std::vector<uint64_t> staticKindCntrsCopy;
  std::vector<uint64_t> arrayFuncCntrsCopy(m_atomicArrayFuncCntrs.size());
  {
    std::lock_guard<std::mutex> statsLock(m_statsMutex);
    classCntrsCopy = m_classCntrs;
    kindCntrsCopy = m_kindCntrs;
    staticClassCntrsCopy = m_staticClassCntrs;
    staticKindCntrsCopy = m_staticKindCntrs;
  }
  for (size_t i = 0; i < m_atomicArrayFuncCntrs.size(); i++) {
    arrayFuncCntrsCopy[i] = m_atomicArrayFuncCntrs[i].load();
  }

  Array ret;
  for (size_t kind = 0;
       static_cast<ArrayData::ArrayKind>(kind) < ArrayData::kNumKinds;
       kind++) {
    Array kindArr;
    for (size_t arrFunc = 0;
         arrFunc < kNumArrayFuncs;
         arrFunc++) {
      const auto count =
        arrayFuncCntrsCopy[kind * kNumArrayFuncs
                           + arrFunc];
      kindArr.set(String(arrayFuncToCStr(static_cast<ArrayFunc>(arrFunc))),
                  count);
    }
    ret.set(
      String(std::string("Array Func Calls For ") +
             ArrayData::kindToString(static_cast<ArrayData::ArrayKind>(kind))),
      kindArr);
  }
  for (auto i = 0; i < classCntrsCopy.size(); i++) {
    const auto arrayClass = static_cast<ArrayClass>(i);
    const auto count = classCntrsCopy[i];
    ret.set(String(classToCStr(arrayClass)), count);
  }
  for (auto i = 0; i < kindCntrsCopy.size(); i++) {
    const auto arrayKind =
      static_cast<ArrayData::ArrayKind>(i);
    const auto count = kindCntrsCopy[i];
    ret.set(String(ArrayData::kindToString(arrayKind)),
            count);
  }
  for (auto i = 0; i < staticClassCntrsCopy.size(); i++) {
    const auto arrayClass = static_cast<ArrayClass>(i);
    const auto count = staticClassCntrsCopy[i];
    ret.set(String(std::string("static ") + classToCStr(arrayClass)), count);
  }
  for (auto i = 0; i < staticKindCntrsCopy.size(); i++) {
    const auto arrayKind =
      static_cast<ArrayData::ArrayKind>(i);
    const auto count = staticKindCntrsCopy[i];
    ret.set(String(std::string("static ") + ArrayData::kindToString(arrayKind)),
            count);
  }
  ret.set(s_numCows, m_numCows.load());
  ret.set(s_numPackedToMixed, m_numPackedToMixed.load());
  ret.set(s_numUnknownReleases, m_numUnknownReleases.load());
  return ret;
}

void ArrayTracer::readElementsUsage(const ArrayData* ad, ArrayUsage& usage) {
  /*
   * We have to be careful here because we are going to intercept our own calls
   * to iter_end and iter_begin, so we call into g_array_funcs_unmodified
   */
  auto kind = ad->kind();
  auto itr = g_array_funcs_unmodified.iterBegin[kind](ad);
  auto itr_end = g_array_funcs_unmodified.iterEnd[kind](ad);
  while (itr != itr_end) {
    TypedValue tv{};
    g_array_funcs_unmodified.nvGetKey[kind](ad, &tv, itr);
    if (tv.m_type == KindOfInt64) {
      setIntKey(usage);
    } else {
      setStrKey(usage);
    }
    Variant v = g_array_funcs_unmodified.getValueRef[kind](ad, itr);
    if (v.getType() == KindOfRef) {
      setReffy(usage);
    }
    itr = g_array_funcs_unmodified.iterAdvance[kind](ad, itr);
  }
}

// Assumes you have lock on m_statsMutex;
// Assumes you are passing in a non-static array ad that we'll look up
// in liveArrays()
void ArrayTracer::handleAccounting(const ArrayData* ad) {
  auto kind = ad->kind();
  always_assert(kind < ArrayData::kNumKinds);
  m_kindCntrs[kind]++;

  auto itr = liveArrays()->find(ad);
  if (itr != liveArrays()->end()) {
    auto arrayClass = classifyUsage(kind, itr->second);
    m_classCntrs[arrayClass]++;
    if (arrayClass == ArrayClass::NoInfo) {
      m_noInfoSizes.addValue(ad->getSize());
    }
  } else {
    // Keep internal bookkeeping about arrays we don't know about
    m_numUnknownReleases++;
  }
}

void ArrayTracer::handleRelease(const ArrayData* ad) {
  always_assert(!ad->isStatic());
  {
    std::lock_guard<std::mutex> statsLock(m_statsMutex);
    handleAccounting(ad);
  }
  liveArrays()->erase(ad);
}

void ArrayTracer::handleTakenByRef(ArrayData* ad) {
  // Note: Would need to mark taken by ref in ElemDArray when reffy is true
  // if we want to keep track of reffiness correctly
}

Array HHVM_FUNCTION(hphp_array_tracer_dump) {
  auto ret = Array::Create();
  if (RuntimeOption::EvalTraceArrays) {
    ret = getArrayTracer()->dumpToArray();
  }
  return ret;
}

class ArrayTracerExtension final : public Extension {
  public:
  ArrayTracerExtension() : Extension("array_tracer") {}
  void moduleInit() override {
    HHVM_FE(hphp_array_tracer_dump);

    loadSystemlib();
  }
} s_array_tracer_extension;


////////////////////////////////////////////////////////////////////////////////

} // HPHP::

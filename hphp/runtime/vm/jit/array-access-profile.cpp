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

#include "hphp/runtime/vm/jit/array-access-profile.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/member-operations.h"

#include "hphp/util/safe-cast.h"

#include <algorithm>
#include <cstring>
#include <sstream>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

template<typename Arr>
int32_t findStringKey(const Arr* ad, const StringData* sd) {
  // We can only optimize cases where the strings compare equal as pointers.
  auto const pos = ad->find(sd, sd->hash());
  return validPos(pos) && ad->data()[pos].strKey() == sd ? pos : -1;
}

bool isSmallStaticArray(const ArrayData* ad) {
  if (!ad->isVanillaDict()) return false;
  auto const arr = MixedArray::asMixed(ad);
  return arr->iterLimit() <= MixedArray::SmallSize &&
         arr->keyTypes().mustBeStaticStrs();
}

}

///////////////////////////////////////////////////////////////////////////////

static std::string actionString(ArrayAccessProfile::Action action) {
  switch(action) {
    case ArrayAccessProfile::Action::None : return "None";
    case ArrayAccessProfile::Action::Cold : return "Cold";
    case ArrayAccessProfile::Action::Exit : return "Exit";
  }
  not_reached();
}

std::string ArrayAccessProfile::Result::toString() const {
  return folly::sformat("offset={}({}) empty={} missing={}",
                        actionString(offset.first), offset.second,
                        actionString(empty),
                        actionString(missing));
}

std::string ArrayAccessProfile::toString() const {
  if (!m_init) return std::string("uninitialized");
  uint64_t total = m_untracked;
  for (auto const& line : m_hits) {
    if (line.pos != -1) total += line.count;
  }
  if (total == 0) total = 1; // avoid div by 0
  auto pct = [&] (uint32_t value) { return 100.0 * value / total; };
  std::ostringstream out;
  for (auto const& line : m_hits) {
    out << folly::format("{}:{}({:.1f}%),",
                         line.pos, line.count, pct(line.count));
  }
  out << folly::format(
    "untracked:{}({:.1f}%),small:{}({:.1f}%),"
    "empty:{}({:.1f}%),missing:{}({:.1f}%)",
    m_untracked, pct(m_untracked), m_small, pct(m_small),
    m_empty, pct(m_empty), m_missing, pct(m_missing)
  );
  return out.str();
}

folly::dynamic ArrayAccessProfile::toDynamic() const {
  using folly::dynamic;
  if (!m_init) return dynamic();

  uint32_t total = 0;
  dynamic hits = dynamic::array;
  for (auto const& line : m_hits) {
    hits.push_back(dynamic::object("position", line.pos)
                                  ("count", line.count));
    total += line.count;
  }
  return dynamic::object("hits", hits)
                        ("untracked", m_untracked)
                        ("small", m_small)
                        ("empty", m_empty)
                        ("missing", m_missing)
                        ("total", total)
                        ("profileType", "ArrayAccessProfile");
}

ArrayAccessProfile::Result ArrayAccessProfile::choose() const {
  if (!m_init) return Result{};

  Line hottest;
  uint32_t total = 0;
  for (auto i = 0; i < kNumTrackedSamples; ++i) {
    auto const& line = m_hits[i];
    if (!validPos(line.pos)) break;

    if (line.count > hottest.count) hottest = line;
    total += line.count;
  }
  total += m_untracked;

  auto const pickAction = [&](
    uint32_t val,
    double cold,
    double exit = RO::EvalHHIRExitArrayProfileThreshold
  ) {
    if (val >= total * exit) return Action::Exit;
    if (val >= total * cold) return Action::Cold;
    return Action::None;
  };

  auto const offset_threshold  = RO::EvalHHIROffsetArrayProfileThreshold;
  auto const size_threshold = RO::EvalHHIRSmallArrayProfileThreshold;
  auto const missing_threshold = RO::EvalHHIRMissingArrayProfileThreshold;
  auto const index_action =
    pickAction(hottest.count, offset_threshold,
               RO::EvalHHIROffsetExitArrayProfileThreshold);
  auto const offset =
    std::make_pair(index_action, index_action == Action::None
                                 ? 0 : safe_cast<uint32_t>(hottest.pos));
  auto const size_hint = m_small >= total * size_threshold
                         ? SizeHintData::SmallStatic : SizeHintData::Default;
  auto const empty = pickAction(m_empty, missing_threshold);
  auto const missing = pickAction(m_missing, missing_threshold);
  return Result{offset, SizeHintData(size_hint), empty, missing};
}

bool ArrayAccessProfile::update(int32_t pos, uint32_t count) {
  if (!m_init) init();

  if (!validPos(pos)) return false;

  for (auto i = 0; i < kNumTrackedSamples; ++i) {
    auto& line = m_hits[i];

    if (line.pos == pos) {
      line.count += count;
      return true;
    }
    if (line.pos == -1) {
      line.pos = pos;
      line.count = count;
      return true;
    }
  }

  return false;
}

void ArrayAccessProfile::update(const ArrayData* ad, int64_t i, bool cowCheck) {
  // Don't count accesses that would require a COW, as we can't optimize them.
  auto const h = hash_int64(i);
  auto const pos =
    cowCheck && ad->cowCheck() ? -1 :
    ad->isVanillaDict() ? MixedArray::asMixed(ad)->find(i, h) :
    ad->isVanillaKeyset() ? SetArray::asSet(ad)->find(i, h) :
    -1;
  if (!update(pos, 1)) m_untracked++;
  if (isSmallStaticArray(ad)) m_small++;
  if (ad->size() == 0) m_empty++;
}

void ArrayAccessProfile::update(const ArrayData* ad, const StringData* sd,
                                bool cowCheck) {
  // Don't count accesses that would require a COW, as we can't optimize them.
  // Additionally, only count cases where the string keys compare equal as
  // pointers (checked within findStringKey).
  auto const pos =
    cowCheck && ad->cowCheck() ? -1 :
    ad->isVanillaDict() ? findStringKey(MixedArray::asMixed(ad), sd) :
    ad->isVanillaKeyset() ? findStringKey(SetArray::asSet(ad), sd) :
    -1;
  if (!update(pos, 1)) m_untracked++;
  if (isSmallStaticArray(ad)) m_small++;
  if (ad->size() == 0) m_empty++;
  if (ad->hasStrKeyTable() && !ad->missingKeySideTable().mayContain(sd)) {
    m_missing++;
  }
}

void ArrayAccessProfile::reduce(ArrayAccessProfile& l,
                                const ArrayAccessProfile& r) {
  if (!r.m_init) return;
  if (!l.m_init) l.init();

  Line scratch[2 * kNumTrackedSamples];
  auto n = uint32_t{0};

  for (auto i = 0; i < kNumTrackedSamples; ++i) {
    auto const& rline = r.m_hits[i];
    if (!validPos(rline.pos)) break;

    // Update `l'.  If `l' can't record the update, save it to scratch.  If
    // `update' fails to record, we don't increment m_untracked here. The
    // entries that cannot be kept in the final tracked set due to capacity will
    // be included in l.m_untracked below.
    if (!l.update(rline.pos, rline.count)) {
      scratch[n++] = rline;
    }
  }
  l.m_untracked += r.m_untracked;
  l.m_small += r.m_small;
  l.m_empty += r.m_empty;
  l.m_missing += r.m_missing;

  if (n == 0) return;
  assertx(n <= kNumTrackedSamples);

  // Sort the combined samples, then copy the top hits back into `l'.
  std::memcpy(&scratch[n], &l.m_hits[0],
              kNumTrackedSamples * sizeof(Line));
  std::sort(&scratch[0], &scratch[n + kNumTrackedSamples],
            [] (Line a, Line b) { return a.count > b.count; });
  std::memcpy(l.m_hits, scratch, kNumTrackedSamples * sizeof(Line));

  // Add the hits in the discarded tail to m_untracked.
  for (auto i = kNumTrackedSamples; i < n + kNumTrackedSamples; ++i) {
    auto const& line = scratch[i];
    if (!validPos(line.pos)) break;

    l.m_untracked += line.count;
  }
}

void ArrayAccessProfile::init() {
  assertx(!m_init);
  for (auto i = 0; i < kNumTrackedSamples; ++i) {
    m_hits[i] = Line{};
  }
  m_init = true;
}

///////////////////////////////////////////////////////////////////////////////

}}

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

#include "hphp/util/interval-set.h"

#include "hphp/util/assertions.h"
#include "hphp/util/compilation-flags.h"

#include <sstream>

namespace HPHP {

IntervalSet IntervalSet::operator ~() const {
  IntervalSet ret;
  auto start = NEG_INF;
  for (auto const& i : intervals) {
    if (i.start > start) {
      ret.intervals.push_back({start, i.start - 1});
      if (i.end == POS_INF) return ret.checkInvariants();
    }
    start = i.end + 1;
  }
  ret.intervals.push_back({start, POS_INF});
  return ret.checkInvariants();
}
IntervalSet& IntervalSet::operator |=(const IntervalSet& o) {
  *this = *this | o;
  return *this;
}
IntervalSet& IntervalSet::operator &=(const IntervalSet& o) {
  *this = *this & o;
  return *this;
}
IntervalSet& IntervalSet::operator -=(const IntervalSet& o) {
  *this = *this - o;
  return *this;
}
IntervalSet IntervalSet::operator |(const IntervalSet& o) const {
  IntervalSet ret;
  size_t j0 = 0, j1 = 0;
  while (j0 < intervals.size() || j1 < o.intervals.size()) {
    auto const& i = [&] {
      if (j0 == intervals.size()) return o.intervals[j1++];
      if (j1 == o.intervals.size()) return intervals[j0++];
      if (intervals[j0].start < o.intervals[j1].start) return intervals[j0++];
      return o.intervals[j1++];
    }();
    if (!ret.intervals.size()) {
      ret.intervals.emplace_back(i);
      continue;
    }
    auto& last = ret.intervals.back();
    if (last.end == POS_INF) return ret.checkInvariants();
    if (last.end + 1 >= i.start) {
      if (i.end > last.end) last.end = i.end;
    } else {
      ret.intervals.emplace_back(i);
    }
  }
  return ret.checkInvariants();
}
IntervalSet IntervalSet::operator &(const IntervalSet& o) const {
  IntervalSet ret;
  size_t j0 = 0, j1 = 0;
  while (j0 < intervals.size() && j1 < o.intervals.size()) {
    auto const& i0 = intervals[j0];
    auto const& i1 = o.intervals[j1];
    if (i0.end < i1.start) {
      j0++;
    } else if (i1.end < i0.start) {
      j1++;
    } else {
      ret.intervals.push_back({
        std::max(i0.start, i1.start),
        std::min(i0.end, i1.end)
      });
      if (i0.end < i1.end) j0++;
      else j1++;
    }
  }
  return ret.checkInvariants();
}
IntervalSet IntervalSet::operator -(const IntervalSet& o) const {
  return *this & ~o;
}

bool IntervalSet::operator ==(const IntervalSet& o) const {
  return intervals == o.intervals;
}
bool IntervalSet::operator !=(const IntervalSet& o) const {
  return !(*this == o);
}

IntervalSet IntervalSet::None() { return IntervalSet{}; }
IntervalSet IntervalSet::All() {
  return Inclusive(NEG_INF, POS_INF);
}
IntervalSet IntervalSet::Point(value_type v) {
  IntervalSet ret;
  ret.intervals.push_back({v, v});
  return ret.checkInvariants();
}
IntervalSet IntervalSet::Inclusive(value_type start, value_type end) {
  IntervalSet ret;
  ret.intervals.push_back({start, end});
  return ret.checkInvariants();
}
IntervalSet IntervalSet::Range(value_type start, value_type end) {
  assertx(start != NEG_INF);
  assert_flog(start <= end, "start: {}, end: {}", start, end);
  if (start == end) return None();
  IntervalSet ret;
  ret.intervals.push_back({start, end - 1});
  return ret.checkInvariants();
}
IntervalSet IntervalSet::Below(value_type end) {
  assertx(end != NEG_INF);
  IntervalSet ret;
  ret.intervals.push_back({NEG_INF, end - 1});
  return ret.checkInvariants();
}
IntervalSet IntervalSet::Above(value_type start) {
  IntervalSet ret;
  ret.intervals.push_back({start, POS_INF});
  return ret.checkInvariants();
}

std::string IntervalSet::toString() const {
  std::string sep;
  std::ostringstream os;
  os << "{";
  for (auto const& i : intervals) {
    os << sep;
    if (i.start == NEG_INF) os << "(-INF";
    else os << "[" << i.start;
    os << ", ";
    if (i.end == POS_INF) os << "+INF";
    else os << i.end + 1;
    os << ")";
    sep = ", ";
  }
  os << "}";
  return os.str();
}

IntervalSet& IntervalSet::checkInvariants() {
  if constexpr (!debug) return *this;
  SCOPE_ASSERT_DETAIL("IntervalSet") {
    return toString();
  };
  for (size_t j = 0; j < intervals.size(); j++) {
    always_assert(intervals[j].start <= intervals[j].end);
    always_assert(IMPLIES(j, intervals[j - 1].end < POS_INF));
    always_assert(IMPLIES(j, intervals[j - 1].end + 1 < intervals[j].start));
  }
  return *this;
}
const IntervalSet& IntervalSet::checkInvariants() const {
  return const_cast<IntervalSet*>(this)->checkInvariants();
}

} // namespace HPHP

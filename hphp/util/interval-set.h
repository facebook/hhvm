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

#pragma once

#include <limits>
#include <cstdint>
#include <string>
#include <vector>

namespace HPHP {

struct IntervalSet {
  using value_type = int32_t;
  static constexpr value_type NEG_INF = std::numeric_limits<value_type>::min();
  static constexpr value_type POS_INF = std::numeric_limits<value_type>::max();

  IntervalSet operator ~() const;
  IntervalSet& operator |=(const IntervalSet& o);
  IntervalSet& operator &=(const IntervalSet& o);
  IntervalSet& operator -=(const IntervalSet& o);
  IntervalSet operator |(const IntervalSet& o) const;
  IntervalSet operator &(const IntervalSet& o) const;
  IntervalSet operator -(const IntervalSet& o) const;

  bool operator ==(const IntervalSet& o) const;
  bool operator !=(const IntervalSet& o) const;

  static IntervalSet None();
  static IntervalSet All();
  static IntervalSet Point(value_type v);
  static IntervalSet Inclusive(value_type start, value_type end);
  static IntervalSet Range(value_type start, value_type end);
  static IntervalSet Below(value_type end);
  static IntervalSet Above(value_type start);

  std::string toString() const;

private:
  IntervalSet& checkInvariants();
  const IntervalSet& checkInvariants() const;

  struct Interval {
    // Inclusive on both sides.
    value_type start;
    value_type end;

    bool operator ==(const Interval& o) const {
      return start == o.start && end == o.end;
    }
    bool operator !=(const Interval& o) const {
      return !(*this == o);
    }
  };
  std::vector<Interval> intervals;
};

}  // namespace HPHP

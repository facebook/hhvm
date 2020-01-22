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

#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-print.h"

#include <folly/Lazy.h>
#include <folly/portability/GTest.h>

namespace HPHP { namespace jit {

namespace {

std::string safeShow(const VregSet& s) {
  std::ostringstream str;
  auto comma = false;
  str << '{';
  for (auto const r : s) {
    if (comma) str << ", ";
    comma = true;
    str << '%' << (size_t)r;
  }
  str << '}';
  return str.str();
}

std::string safeShow(const VregList& l) {
  using namespace folly::gen;
  return folly::sformat(
    "[{}]",
    from(l)
    | map([] (Vreg r) { return folly::sformat("%{}", (size_t)r); })
    | unsplit<std::string>(", ")
  );
}

}

void PrintTo(const VregSet& s, std::ostream* os) { *os << safeShow(s); }
void PrintTo(const VregList& l, std::ostream* os) { *os << safeShow(l); }

namespace {

VregSet rvalueize(VregSet s) { return s; }

bool inVregList(const VregList& regs, Vreg r) {
  return std::find(regs.begin(), regs.end(), r) != regs.end();
}

VregList addToVregList(VregList regs, Vreg r) {
  regs.push_back(r);
  std::sort(regs.begin(), regs.end());
  regs.erase(std::unique(regs.begin(), regs.end()), regs.end());
  return regs;
}

VregList removeFromVregList(VregList regs, Vreg r) {
  regs.erase(std::remove(regs.begin(), regs.end(), r), regs.end());
  return regs;
}

VregList unionVregList(VregList l1, const VregList& l2) {
  l1.insert(l1.end(), l2.begin(), l2.end());
  std::sort(l1.begin(), l1.end());
  l1.erase(std::unique(l1.begin(), l1.end()), l1.end());
  return l1;
}

VregList intersectVregList(VregList l1, const VregList& l2) {
  l1.erase(
    std::remove_if(
      l1.begin(), l1.end(),
      [&] (Vreg r) { return !inVregList(l2, r); }
    ),
    l1.end()
  );
  return l1;
}

VregList diffVregList(VregList l1, const VregList& l2) {
  l1.erase(
    std::remove_if(
      l1.begin(), l1.end(),
      [&] (Vreg r) { return inVregList(l2, r); }
    ),
    l1.end()
  );
  return l1;
}

VregList toVregList(const VregSet& s) {
  VregList ret;
  for (auto const r : s) ret.emplace_back(r);
  return ret;
}

VregList removePhysFromVregList(const VregList& l) {
  VregList ret;
  for (auto const r : l) {
    if (r.isPhys()) continue;
    ret.emplace_back(r);
  }
  return ret;
}

bool vregListContainsPhys(const VregList& l) {
  for (auto const r : l) {
    if (r.isPhys()) return true;
  }
  return false;
}

bool vregListAllPhys(const VregList& l) {
  for (auto const r : l) {
    if (!r.isPhys()) return false;
  }
  return true;
}

RegSet vregListToRegSet(const VregList& l) {
  RegSet s;
  for (auto const r : l) {
    if (r.isPhys()) s |= r.physReg();
  }
  return s;
}

auto const operatorVregs = folly::lazy(
  [] {
    return VregList {
      Vreg{0},
      Vreg{1},
      Vreg{79},
      Vreg{255},
      Vreg{311},
      Vreg{400},
      Vreg{750},
      Vreg{999},
      Vreg{5001}
    };
  }
);

auto const subsetVregs = folly::lazy(
  [] {
    return VregList {
      Vreg{0},
      Vreg{1},
      Vreg{100},
      Vreg{150},
      Vreg{VregSet::kBitsPerBlock-1},
      Vreg{VregSet::kBitsPerBlock},
      Vreg{400},
      Vreg{VregSet::kBitsPerBlock*2 - 1},
      Vreg{VregSet::kBitsPerBlock*2},
      Vreg{680},
      Vreg{VregSet::kBitsPerBlock*3 - 1},
      Vreg{VregSet::kBitsPerBlock*3},
      Vreg{VregSet::kBitsPerBlock*4 - 1},
      Vreg{VregSet::kBitsPerBlock*4},
      Vreg{5001}
    };
  }
);

auto const physVregs = folly::lazy(
  [] {
    return VregList{
      Reg64{0},
      Reg64{1},
      Reg64{11},
      RegXMM{0},
      RegXMM{1},
      RegXMM{8},
      RegSF{0},
      Vreg{128},
      Vreg{129},
      Vreg{309},
      Vreg{5781}
    };
  }
);

auto const equalityVregs = folly::lazy(
  [] {
    return VregList {
      Vreg{0},
      Vreg{1},
      Vreg{128},
      Vreg{VregSet::kBitsPerBlock-1},
      Vreg{VregSet::kBitsPerBlock},
      Vreg{400},
      Vreg{VregSet::kBitsPerBlock*2 - 1},
      Vreg{VregSet::kBitsPerBlock*2},
      Vreg{VregSet::kBitsPerBlock*3 - 1},
      Vreg{VregSet::kBitsPerBlock*3},
      Vreg{VregSet::kBitsPerBlock*4 - 1},
      Vreg{VregSet::kBitsPerBlock*4},
      Vreg{7112}
    };
  }
);

auto const membershipVregs = folly::lazy(
  [] {
    auto out = subsetVregs();
    out.emplace_back(Vreg{2});
    out.emplace_back(Vreg{101});
    out.emplace_back(Vreg{157});
    out.emplace_back(Vreg{300});
    out.emplace_back(Vreg{599});
    out.emplace_back(Vreg{1000});
    out.emplace_back(Vreg{9999});
    return out;
  }
);

std::vector<VregList> makeSubsets(const VregList& v) {
  assertx(v.size() < 64);
  std::vector<VregList> out;

  uint64_t selected = 0;
  while (selected < (1ULL << v.size())) {
    VregList current;
    for (size_t i = 0; i < v.size(); ++i) {
      if (selected & (1ULL << i)) current.emplace_back(v[i]);
    }
    out.emplace_back(std::move(current));
    ++selected;
  }

  for (auto& o : out) std::sort(o.begin(), o.end());
  return out;
}

std::vector<std::pair<VregList, VregSet>>
makeCases(const VregList& vregs) {
  auto subsets = makeSubsets(vregs);
  std::vector<std::pair<VregList, VregSet>> out;
  out.reserve(subsets.size());
  for (auto& subset : subsets) {
    VregSet s;
    for (auto const r : subset) s.add(r);
    out.emplace_back(std::move(subset), std::move(s));
  }
  return out;
}

auto const operatorCases = folly::lazy(
  [] { return makeCases(operatorVregs()); }
);

auto const subsetCases = folly::lazy(
  [] { return makeCases(subsetVregs()); }
);

auto const physCases = folly::lazy(
  [] { return makeCases(physVregs()); }
);

auto const equalityCases = folly::lazy(
  [] { return makeCases(equalityVregs()); }
);

void commonTests(const VregSet& s,
                 const VregList& l,
                 const VregList& membership) {
  EXPECT_EQ(s.none(), l.empty());
  EXPECT_NE(s.any(), l.empty());
  EXPECT_EQ(s.size(), l.size());
  EXPECT_EQ(s.empty(), s.none());
  EXPECT_EQ(s.empty(), s.begin() == s.end());
  EXPECT_EQ(s.any(), s.begin() != s.end());

  for (auto const r : membership) EXPECT_EQ(s[r], inVregList(l, r));
  for (auto const r : l) EXPECT_TRUE(s[r]);
  EXPECT_EQ(toVregList(s), l);
}

}

TEST(VregSet, Ctors) {
  {
    VregSet s;
    EXPECT_TRUE(s.none());
    EXPECT_TRUE(s.empty());
    EXPECT_FALSE(s.any());
    EXPECT_EQ(s.size(), 0);
    for (auto const r : membershipVregs()) EXPECT_FALSE(s[r]);
    EXPECT_EQ(toVregList(s), VregList{});
  }

  for (auto const r : subsetVregs()) {
    VregSet s{r};
    commonTests(s, VregList{r}, membershipVregs());
  }

  for (auto const& c : subsetCases()) {
    VregSet s{c.first};
    commonTests(s, c.first, membershipVregs());
  }

  for (auto const& c : subsetCases()) {
    VregSet s{c.first.begin(), c.first.end()};
    commonTests(s, c.first, membershipVregs());
  }

  for (auto const& c : physCases()) {
    RegSet rs;
    VregList l;
    for (auto const r : c.first) {
      if (!r.isPhys()) continue;
      rs |= r.physReg();
      l.emplace_back(r);
    }
    VregSet s{rs};
    EXPECT_TRUE(s.allPhys());
    EXPECT_EQ(!rs.empty(), s.containsPhys());
    EXPECT_EQ(s.physRegs(), rs);
    commonTests(s, l, physVregs());
  }

  {
    VregSet s{Vreg{100}, Vreg{3}, Vreg{801}};
    EXPECT_EQ(s.size(), 3);
    EXPECT_FALSE(s.none());
    EXPECT_FALSE(s.empty());
    EXPECT_TRUE(s.any());

    VregList l{Vreg{3}, Vreg{100}, Vreg{801}};
    EXPECT_EQ(toVregList(s), l);
  }
}

TEST(VregSet, CopyAssign) {
  for (auto const& c : operatorCases()) {
    auto copy = c.second;
    EXPECT_EQ(copy, c.second);
    commonTests(copy, c.first, operatorVregs());

    auto moved = std::move(copy);
    EXPECT_EQ(moved, c.second);
    commonTests(moved, c.first, operatorVregs());

    for (auto const& c2 : operatorCases()) {
      auto s = copy;

      s = c2.second;
      EXPECT_EQ(s, c2.second);
      commonTests(s, c2.first, operatorVregs());

      auto s2 = c2.second;
      s = std::move(s2);
      EXPECT_EQ(s, c2.second);
      commonTests(s, c2.first, operatorVregs());
    }
  }
}

TEST(VregSet, Add) {
  auto const& cases = subsetCases();
  for (auto const& c : cases) {
    commonTests(c.second, c.first, membershipVregs());
  }
  for (auto const r : subsetVregs()) {
    for (auto const& c : cases) {
      auto s = c.second;
      s.add(r);
      commonTests(s, addToVregList(c.first, r), membershipVregs());
    }
  }
}

TEST(VregSet, Remove) {
  auto const& cases = subsetCases();
  for (auto const r : membershipVregs()) {
    for (auto const& c : cases) {
      auto s = c.second;
      s.remove(r);
      commonTests(s, removeFromVregList(c.first, r), membershipVregs());
    }
  }
}

TEST(VregSet, Reset) {
  for (auto const& c : subsetCases()) {
    auto s = c.second;
    s.reset();
    EXPECT_TRUE(s.none());
    EXPECT_TRUE(s.empty());
    EXPECT_FALSE(s.any());
    EXPECT_EQ(s.size(), 0);
    commonTests(s, VregList{}, membershipVregs());
  }
}

TEST(VregSet, Union) {
  auto const& cases = operatorCases();
  for (auto const& c1 : cases) {
    for (auto const& c2 : cases) {
      auto const l = unionVregList(c1.first, c2.first);
      commonTests(c1.second | c2.second, l, operatorVregs());
      commonTests(rvalueize(c1.second) | c2.second, l, operatorVregs());
      commonTests(c1.second | rvalueize(c2.second), l, operatorVregs());
      commonTests(
        rvalueize(c1.second) | rvalueize(c2.second),
        l, operatorVregs()
      );
      auto s = c1.second;
      s |= c2.second;
      commonTests(s, l, operatorVregs());
    }
  }
}

TEST(VregSet, Intersect) {
  auto const& cases = operatorCases();
  for (auto const& c1 : cases) {
    for (auto const& c2 : cases) {
      auto const l = intersectVregList(c1.first, c2.first);
      commonTests(c1.second & c2.second, l, operatorVregs());
      commonTests(rvalueize(c1.second) & c2.second, l, operatorVregs());
      commonTests(c1.second & rvalueize(c2.second), l, operatorVregs());
      commonTests(
        rvalueize(c1.second) & rvalueize(c2.second),
        l, operatorVregs()
      );
      auto s = c1.second;
      s &= c2.second;
      commonTests(s, l, operatorVregs());

      EXPECT_EQ(
        c1.second.intersects(c2.second), (c1.second & c2.second).any()
      );
    }
  }
}

TEST(VregSet, Diff) {
  auto const& cases = operatorCases();
  for (auto const& c1 : cases) {
    for (auto const& c2 : cases) {
      auto const l = diffVregList(c1.first, c2.first);
      commonTests(c1.second - c2.second, l, operatorVregs());
      commonTests(rvalueize(c1.second) - c2.second, l, operatorVregs());
      commonTests(c1.second - rvalueize(c2.second), l, operatorVregs());
      commonTests(
        rvalueize(c1.second) - rvalueize(c2.second),
        l, operatorVregs()
      );
      auto s = c1.second;
      s -= c2.second;
      commonTests(s, l, operatorVregs());
    }
  }
}

TEST(VregSet, Phys) {
  for (auto const& c : physCases()) {
    EXPECT_EQ(toVregList(c.second), c.first);
    for (auto const r : physVregs()) {
      EXPECT_EQ(c.second[r], inVregList(c.first, r));
    }
    EXPECT_EQ(c.second.containsPhys(), vregListContainsPhys(c.first));
    EXPECT_EQ(c.second.allPhys(), vregListAllPhys(c.first));
    EXPECT_EQ(c.second.physRegs(), vregListToRegSet(c.first));

    auto copy = c.second;
    copy.removePhys();
    EXPECT_FALSE(copy.containsPhys());
    EXPECT_EQ(copy.any(), !copy.allPhys());
    EXPECT_EQ(c.second.allPhys(), copy.none());
    EXPECT_TRUE(copy.physRegs().empty());
    EXPECT_EQ(toVregList(copy), removePhysFromVregList(c.first));
  }

  for (auto const& c1 : operatorCases()) {
    for (auto const& c2 : physCases()) {
      auto s = c1.second;

      RegSet rs;
      auto l = c1.first;
      for (auto const r : c2.first) {
        if (!r.isPhys()) continue;
        rs |= r.physReg();
        l.emplace_back(r);
      }
      std::sort(l.begin(), l.end());
      l.erase(std::unique(l.begin(), l.end()), l.end());

      s.add(rs);

      EXPECT_EQ(s.containsPhys(), vregListContainsPhys(l));
      EXPECT_EQ(s.allPhys(), vregListAllPhys(l));
      EXPECT_EQ(s.physRegs(), vregListToRegSet(l));
      commonTests(s, l, unionVregList(physVregs(), operatorVregs()));
    }
  }
}

TEST(VregSet, Swap) {
  for (auto const& c1 : operatorCases()) {
    for (auto const& c2 : operatorCases()) {
      auto s1 = c1.second;
      auto s2 = c2.second;

      swap(s1, s2);

      EXPECT_EQ(s1, c2.second);
      EXPECT_EQ(s2, c1.second);
      if (c1.first == c2.first) {
        EXPECT_EQ(s1, s2);
      } else {
        EXPECT_NE(s1, s2);
      }
      commonTests(s1, c2.first, operatorVregs());
      commonTests(s2, c1.first, operatorVregs());
    }
  }
}

TEST(VregSet, Equality) {
  for (auto const& c1 : equalityCases()) {
    for (auto const& c2 : equalityCases()) {
      if (c1.first == c2.first) {
        EXPECT_EQ(c1.second, c2.second);
      } else {
        EXPECT_NE(c1.second, c2.second);
      }
    }
  }
}

}}

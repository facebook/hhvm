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

void PrintTo(const VregSet& s, std::ostream* os) { *os << show(s); }

namespace {

VregList toVregList(const VregSet& s) {
  VregList ret;
  s.forEach([&] (Vreg r) { ret.push_back(r); });
  return ret;
}

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

VregList unionVregList(VregList l1, VregList l2) {
  l1.insert(l1.end(), l2.begin(), l2.end());
  std::sort(l1.begin(), l1.end());
  l1.erase(std::unique(l1.begin(), l1.end()), l1.end());
  return l1;
}

VregList intersectVregList(VregList l1, VregList l2) {
  l1.erase(
    std::remove_if(
      l1.begin(), l1.end(),
      [&] (Vreg r) { return !inVregList(l2, r); }
    ),
    l1.end()
  );
  return l1;
}

VregList diffVregList(VregList l1, VregList l2) {
  l1.erase(
    std::remove_if(
      l1.begin(), l1.end(),
      [&] (Vreg r) { return inVregList(l2, r); }
    ),
    l1.end()
  );
  return l1;
}

VregSet rvalueize(VregSet s) { return s; }

auto const vregs = folly::lazy([] {
  return VregList {
    Vreg{1000},
    Vreg{1001},
    Vreg{1050},
    Vreg{1051},
    Vreg{2000},
    Vreg{2001},
    Vreg{2050}
  };
});

auto const all = folly::lazy([] {
  std::vector<std::pair<VregList, VregSet>> out;

  auto const permute = [&] (VregList v) {
    std::sort(v.begin(), v.end());
    do {
      VregSet s{v.begin(), v.end()};
      auto v2 = v;
      std::sort(v2.begin(), v2.end());
      out.emplace_back(std::move(v2), std::move(s));
    } while (std::next_permutation(v.begin(), v.end()));
  };

  auto const& regs = vregs();
  uint64_t selected = 0;
  while (selected < (1ULL << regs.size())) {
    VregList v;
    for (size_t i = 0; i < regs.size(); ++i) {
      if (selected & (1ULL << i)) v.emplace_back(regs[i]);
    }
    permute(std::move(v));
    ++selected;
  }
  return out;
});

auto const unique = folly::lazy([] {
  auto v = all();
  std::sort(
    v.begin(),
    v.end(),
    [&](const std::pair<VregList, VregSet>& p1,
        const std::pair<VregList, VregSet>& p2) {
      return p1.first < p2.first;
    }
  );
  v.erase(std::unique(v.begin(), v.end()), v.end());
  return v;
});

auto const uniqueDouble = folly::lazy([] {
  auto u1 = unique();
  auto const& u2 = unique();
  u1.insert(u1.end(), u2.begin(), u2.end());
  return u1;
});

}

TEST(VregSet, All) {
  auto const test = [] (const VregSet& s, const VregList& l) {
    EXPECT_EQ(s.none(), l.empty());
    EXPECT_NE(s.any(), l.empty());
    EXPECT_EQ(s.size(), l.size());

    for (auto const r : vregs()) EXPECT_EQ(s[r], inVregList(l, r));
    EXPECT_EQ(toVregList(s), l);

    for (auto const& p : unique()) {
      if (l == p.first) {
        EXPECT_EQ(s, p.second);
      } else {
        EXPECT_NE(s, p.second);
      }
    }
  };

  for (auto const& p : all()) test(p.second, p.first);

  for (auto const& p : unique()) {
    for (auto const r : vregs()) {
      auto s = p.second;
      s.add(r);
      test(s, addToVregList(p.first, r));
    }
  }

  for (auto const& p : unique()) {
    for (auto const r : vregs()) {
      auto s = p.second;
      s.remove(r);
      test(s, removeFromVregList(p.first, r));
    }
  }

  for (auto const& p : unique()) {
    for (auto const r1 : vregs()) {
      auto s1 = p.second;
      s1.add(r1);
      for (auto const r2 : vregs()) {
        auto s2 = s1;
        s2.add(r2);
        test(s2, addToVregList(addToVregList(p.first, r1), r2));
      }
      for (auto const r2 : vregs()) {
        auto s2 = s1;
        s2.remove(r2);
        test(s2, removeFromVregList(addToVregList(p.first, r1), r2));
      }

      s1 = p.second;
      s1.remove(r1);
      for (auto const r2 : vregs()) {
        auto s2 = s1;
        s2.add(r2);
        test(s2, addToVregList(removeFromVregList(p.first, r1), r2));
      }
      for (auto const r2 : vregs()) {
        auto s2 = s1;
        s2.remove(r2);
        test(s2, removeFromVregList(removeFromVregList(p.first, r1), r2));
      }
    }
  }

  for (auto const& p : unique()) {
    auto s = p.second;
    s.reset();
    test(s, VregList{});
  }

  for (auto const& p1 : uniqueDouble()) {
    for (auto const& p2 : uniqueDouble()) {
      auto const l = unionVregList(p1.first, p2.first);
      test(p1.second | p2.second, l);
      test(rvalueize(p1.second) | p2.second, l);
      test(p1.second | rvalueize(p2.second), l);
      test(rvalueize(p1.second) | rvalueize(p2.second), l);
      auto s = p1.second;
      s |= p2.second;
      test(s, l);
    }
  }

  for (auto const& p1 : uniqueDouble()) {
    for (auto const& p2 : uniqueDouble()) {
      auto const l = intersectVregList(p1.first, p2.first);
      test(p1.second & p2.second, l);
      test(rvalueize(p1.second) & p2.second, l);
      test(p1.second & rvalueize(p2.second), l);
      test(rvalueize(p1.second) & rvalueize(p2.second), l);
      auto s = p1.second;
      s &= p2.second;
      test(s, l);
    }
  }

  for (auto const& p1 : uniqueDouble()) {
    for (auto const& p2 : uniqueDouble()) {
      auto const l = diffVregList(p1.first, p2.first);
      test(p1.second - p2.second, l);
      test(rvalueize(p1.second) - p2.second, l);
      test(p1.second - rvalueize(p2.second), l);
      test(rvalueize(p1.second) - rvalueize(p2.second), l);
      auto s = p1.second;
      s -= p2.second;
      test(s, l);
    }
  }
}

}}

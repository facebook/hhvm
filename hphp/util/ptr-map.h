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

#ifndef incl_HPHP_UTIL_PTRMAP_H_
#define incl_HPHP_UTIL_PTRMAP_H_

#include <algorithm>
#include <vector>
#include "hphp/util/assertions.h"
#include "hphp/util/portability.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// information about heap objects, indexed by valid object starts.
template<class T> struct PtrMap {
  using Region = std::pair<T, std::size_t>;

  void insert(T h, size_t size) {
    sorted_ &= regions_.empty() || h > regions_.back().first;
    regions_.emplace_back(h, size);
  }

  const Region* region(const void* p) const {
    assert(sorted_);
    if (uintptr_t(p) - uintptr_t(span_.first) >= span_.second) {
      return nullptr;
    }
    // Find the first region which begins beyond p.
    auto it = std::upper_bound(regions_.begin(), regions_.end(), p,
      [](const void* p, const Region& region) {
        return p < region.first;
      });
    // If it == first region, p is before any region, which we already
    // checked above.
    assert(it != regions_.begin());
    --it; // backup to the previous region.
    // p can only potentially point within this previous region, so check that.
    return uintptr_t(p) - uintptr_t(it->first) < it->second ? &*it :
           nullptr;
  }

  T start(const void* p) const {
    auto r = region(p);
    return r ? r->first : nullptr;
  }

  bool isStart(const void* p) const {
    auto h = start(p);
    return h && h == p;
  }

  size_t index(const Region* r) const {
    return r - &regions_[0];
  }

  // where does this region start sit in the regions_ vector?
  size_t index(T h) const {
    assert(start(h));
    return region(h) - &regions_[0];
  }

  void prepare() {
    if (!sorted_) {
      std::sort(regions_.begin(), regions_.end());
      sorted_ = true;
    }
    if (!regions_.empty()) {
      auto& front = regions_.front();
      auto& back = regions_.back();
      span_ = Region{
        front.first,
        (const char*)back.first + back.second - (const char*)front.first
      };
    }
    assert(sanityCheck());
  }

  size_t size() const {
    return regions_.size();
  }

  template<class Fn> void iterate(Fn fn) const {
    for (auto& r : regions_) {
      fn(r.first, r.second);
    }
  }

  Region span() const {
    return span_;
  }

private:
  bool sanityCheck() const {
    // Verify that all the regions are in increasing and non-overlapping order.
    DEBUG_ONLY void* last = nullptr;
    for (const auto& region : regions_) {
      assert(!last || last <= region.first);
      last = (void*)(uintptr_t(region.first) + region.second);
    }
    return true;
  }

  Region span_{nullptr, 0};
  std::vector<Region> regions_;
  bool sorted_{true};
};

///////////////////////////////////////////////////////////////////////////////
}
#endif

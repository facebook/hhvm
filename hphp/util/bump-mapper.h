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

#include "hphp/util/address-range.h"
#include "hphp/util/alloc-defs.h"
#include "hphp/util/alloc.h"
#include <functional>
#include <utility>

#if USE_JEMALLOC_EXTENT_HOOKS

namespace HPHP {

extern bool g_useTHPUponHugeTLBFailure;

namespace alloc {

// Interface for all mappers. It also includes data about page limit (for huge
// page mappers) and NUMA support.
struct RangeMapper {
 public:
  // State is shared between multiple mappers and multiple threads.
  explicit RangeMapper(RangeState& state, uint32_t maxHugePages = 0,
                       uint32_t numaMask = 0, short nextNode = 0)
    : m_state(state)
    , m_maxHugePages(maxHugePages)
    , m_interleaveMask(numaMask)
    , m_nextNode(nextNode) {}

  virtual ~RangeMapper() = default;

  // Form a linked list of mappers for the same range.
  static void append(RangeMapper**& ptail, RangeMapper* next) {
    assert((*ptail) == nullptr);        // always keep track of the tail.
    *ptail = next;
    ptail = &(next->m_fallback);
    assert((*ptail) == nullptr);        // next already has fallback?
  }

  RangeMapper* next() {
    return m_fallback;
  }

  RangeState& getRangeState() {
    return m_state;
  }

  void setMaxPages(uint32_t maxHugePages) {
    m_maxHugePages = maxHugePages;
  }

  bool addMapping() {
    if (!m_failed) {
      if (addMappingImpl()) return true;
      // Some hugepage mappers start with zero page budget. Don't fail
      // permanently if they didn't work in the beginning.
      if (m_maxHugePages) m_failed = true;
    }
    return false;
  }

  void* alloc(size_t size, size_t align) {
    if (!m_failed) {
      auto const d = direction();
      do {
        if (m_state.trivial(size, align, d)) {
          if (auto r = m_state.tryAlloc(size, align, d)) return r;
        }
        if (m_state.infeasible(size, align, d)) return nullptr;
      } while (addMapping());
    }
    if (m_fallback) return m_fallback->alloc(size, align);
    return nullptr;
  }

 protected:
  virtual bool addMappingImpl() = 0;
  virtual Direction direction() const = 0;

 protected:
  RangeState& m_state;
  uint32_t m_maxHugePages{0};
  uint32_t m_currHugePages{0};
  uint32_t m_interleaveMask;            // 0 indicates no NUMA
  short m_nextNode{0};
  bool m_failed{false};
  RangeMapper* m_fallback{nullptr};
};


struct Bump1GMapper : public RangeMapper {
 public:
  template<typename... Args>
  explicit Bump1GMapper(Args&&... args)
    : RangeMapper(std::forward<Args>(args)...) {}

 protected:
  Direction direction() const override { return Direction::LowToHigh; }
  bool addMappingImpl() override;
};


struct Bump2MMapper : public RangeMapper {
 public:
  template<typename... Args>
  explicit Bump2MMapper(Args&&... args)
    : RangeMapper(std::forward<Args>(args)...) {}

 protected:
  Direction direction() const override { return Direction::LowToHigh; }
  bool addMappingImpl() override;
};


template<Direction D = Direction::LowToHigh>
struct BumpNormalMapper : public RangeMapper {
 public:
  template<typename... Args>
  explicit BumpNormalMapper(Args&&... args)
    : RangeMapper(std::forward<Args>(args)...) {}

 protected:
  Direction direction() const override { return D; }
  bool addMappingImpl() override;
};

// Create mappings backed by a file
struct BumpFileMapper : public RangeMapper {
 public:
  template<typename... Args>
  explicit BumpFileMapper(Args&&... args)
    : RangeMapper(std::forward<Args>(args)...) {
    m_failed = true;                    // disabled initially
  }
  void enable() {
    m_failed = false;
  }
  bool setDirectory(const char* dir);

 protected:
  Direction direction() const override { return Direction::LowToHigh; }
  bool addMappingImpl() override;
 private:
  int m_fd{0};
  // PATH_MAX feels like a waste of memory, so use something smaller and make it
  // configurable at build time.
#ifndef TMPDIRMAXLEN
#define TMPDIRMAXLEN 80
#endif
  char m_dirName[TMPDIRMAXLEN]{};
};


// A mapper that is used when other mappers all fail. To avoid crashing, this
// could allocate from some emergency buffer, and initiate a graceful shutdown.
// The range passed in should be pre-mapped.
struct BumpEmergencyMapper : public RangeMapper {
 public:
  using ExitFun = std::function<void()>;
  template<typename... Args>
  explicit BumpEmergencyMapper(ExitFun&& exitFn, Args&&... args)
    : RangeMapper(std::forward<Args>(args)...)
    , m_exit(exitFn) {}
 protected:
  Direction direction() const override { return Direction::LowToHigh; }
  bool addMappingImpl() override;
 protected:
  // The exit function to call when we start using the emergency mapping.
  ExitFun m_exit;
};

}}

#endif

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_EXT_ASIO_BLOCKABLE_H_
#define incl_HPHP_EXT_ASIO_BLOCKABLE_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/ext/asio/asio-context.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class c_WaitableWaitHandle;

struct AsioBlockable final {
  enum class Kind : uint8_t {
    AsyncFunctionWaitHandleNode,
    AsyncGeneratorWaitHandle,
    AwaitAllWaitHandleNode,
    ConditionWaitHandle,

    // DEPRECATED
    GenArrayWaitHandle,
    GenMapWaitHandle,
    GenVectorWaitHandle,
  };

  static constexpr ptrdiff_t bitsOff() {
    return offsetof(AsioBlockable, m_bits);
  }

  AsioBlockable* getNextParent() const {
    return reinterpret_cast<AsioBlockable*>(m_bits & kParentMask);
  }

  uint32_t getExtraData() const {
    // Currently only AAWH use this for its child idx.
    assert(getKind() == Kind::AwaitAllWaitHandleNode);
    return static_cast<uint32_t>(m_bits >> kBitsPtr);
  }

  Kind getKind() const {
    return static_cast<Kind>(m_bits & 7);
  }

  c_WaitableWaitHandle* getWaitHandle() const;

  void setNextParent(AsioBlockable* parent, Kind kind, uint32_t extraInfo) {
    assert(!(reinterpret_cast<intptr_t>(parent) & ~kParentMask));
    assert(!(static_cast<intptr_t>(kind) & ~7UL));
    assert(extraInfo <= kExtraInfoMax);
    m_bits = reinterpret_cast<intptr_t>(parent) |
             static_cast<intptr_t>(kind) |
             (static_cast<uint64_t>(extraInfo) << kBitsPtr);
  }

  // Only update the next parent w/o changing kind & extraInfo
  void updateNextParent(AsioBlockable* parent) {
    assert(!(reinterpret_cast<intptr_t>(parent) & ~kParentMask));
    m_bits = (m_bits & ~kParentMask) | reinterpret_cast<intptr_t>(parent);
  }

  static constexpr int kBitsPtr          = 48;
  static constexpr uint64_t kExtraInfoMax = (1UL << (64 - kBitsPtr)) - 1;
  static constexpr uint64_t kPointerMask  = (1UL << kBitsPtr) - 1;
  static constexpr uint64_t kParentMask   = kPointerMask & (~7UL);

private:
  // m_bits stores kind (lowest 3 bits), next parent (in the middle) and extra
  // info in highest 16 bits (currently only AAWH uses it for child index).
  // +--------- Layout for m_bits (64 bits) ------------+
  // | 63........48 | 47.....................3 | 2....0 |
  // | [Extra info] | [Pointer to next parent] | [Kind] |
  // +--------------------------------------------------+
  uintptr_t m_bits;
};

struct AsioBlockableChain final {
  static constexpr ptrdiff_t firstParentOff() {
    return offsetof(AsioBlockableChain, m_firstParent);
  }

  void init() noexcept {
    m_firstParent = nullptr;
  }

  void addParent(AsioBlockable& parent, AsioBlockable::Kind kind,
                 const uint32_t extraInfo = 0) {
    parent.setNextParent(m_firstParent, kind, extraInfo);
    m_firstParent = &parent;
  }

  static void Unblock(AsioBlockableChain chain) { chain.unblock(); }
  void unblock();
  void exitContext(context_idx_t ctx_idx);
  void removeFromChain(AsioBlockable* parent);
  Array toArray();
  c_WaitableWaitHandle* firstInContext(context_idx_t ctx_idx);

  template<class F> void scan(F& mark) const {
    for (auto p = m_firstParent; p; p = p->getNextParent()) {
      mark(p->getWaitHandle());
    }
  }

private:
  AsioBlockable* m_firstParent;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_BLOCKABLE_H_

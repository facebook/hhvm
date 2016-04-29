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

struct c_WaitableWaitHandle;

struct AsioBlockable final {
  enum class Kind : uint8_t {
    AsyncFunctionWaitHandleNode,
    AsyncGeneratorWaitHandle,
    AwaitAllWaitHandleNode,
    ConditionWaitHandle,
  };

  static constexpr ptrdiff_t bitsOff() {
    return offsetof(AsioBlockable, m_bits);
  }

  AsioBlockable* getNextParent() const {
    return reinterpret_cast<AsioBlockable*>(m_bits & kParentMask);
  }

  Kind getKind() const {
    return static_cast<Kind>(m_bits & kKindMask);
  }

  c_WaitableWaitHandle* getWaitHandle() const;

  void setNextParent(AsioBlockable* parent, Kind kind) {
    assert(!(reinterpret_cast<intptr_t>(parent) & ~kParentMask));
    assert(!(static_cast<intptr_t>(kind) & kParentMask));
    m_bits = reinterpret_cast<intptr_t>(parent) | static_cast<intptr_t>(kind);
  }

  // Only update the next parent w/o changing kind.
  void updateNextParent(AsioBlockable* parent) {
    assert(!(reinterpret_cast<intptr_t>(parent) & ~kParentMask));
    m_bits = (m_bits & ~kParentMask) | reinterpret_cast<intptr_t>(parent);
  }

  static constexpr uint64_t kKindMask   = 7UL;
  static constexpr uint64_t kParentMask = ~kKindMask;

private:
  // m_bits stores kind in the lowest 3 bits and next parent in the
  // upper 61 bits.
  // +--- Layout for m_bits (64 bits) ---+
  // | 63.....................3 | 2....0 |
  // | [Pointer to next parent] | [Kind] |
  // +-----------------------------------+
  uintptr_t m_bits;
};

struct AsioBlockableChain final {
  static constexpr ptrdiff_t firstParentOff() {
    return offsetof(AsioBlockableChain, m_firstParent);
  }

  void init() noexcept {
    m_firstParent = nullptr;
  }

  void addParent(AsioBlockable& parent, AsioBlockable::Kind kind) {
    parent.setNextParent(m_firstParent, kind);
    m_firstParent = &parent;
  }

  static void Unblock(AsioBlockableChain chain) { chain.unblock(); }
  static void UnblockJitHelper(ActRec* ar,
                               TypedValue* sp,
                               AsioBlockableChain chain);
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

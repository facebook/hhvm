/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
    AwaitAllWaitHandle,
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
    return reinterpret_cast<AsioBlockable*>(m_bits & ~7UL);
  }

  Kind getKind() const {
    return static_cast<Kind>(m_bits & 7);
  }

  c_WaitableWaitHandle* getWaitHandle() const;

  void setNextParent(AsioBlockable* parent, Kind kind) {
    assert(!(reinterpret_cast<intptr_t>(parent) & 7));
    assert(!(static_cast<intptr_t>(kind) & ~7UL));
    m_bits = reinterpret_cast<intptr_t>(parent) |
             static_cast<intptr_t>(kind);
  }

private:
  // Stores pointer to the next parent + kind in the lowest 3 bits.
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
  void unblock();
  void exitContext(context_idx_t ctx_idx);
  Array toArray();
  c_WaitableWaitHandle* firstInContext(context_idx_t ctx_idx);

private:
  AsioBlockable* m_firstParent;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_BLOCKABLE_H_

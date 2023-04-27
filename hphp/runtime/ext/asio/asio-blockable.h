/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

/*
 * AsioBlockable is an intrusive node in an AsioBlockableChain.
 * Each node contains a pointer to the prev node (AsioBlockable*)
 * as well as a Kind, describing the waithandle that owns *this* node.
 */
struct AsioBlockable final {
  enum class Kind : uint8_t {
    AsyncFunctionWaitHandleNode,
    AsyncGeneratorWaitHandle,
    AwaitAllWaitHandleNode,
    ConcurrentWaitHandleNode,
    ConditionWaitHandle,
  };

  static constexpr ptrdiff_t bitsOff() {
    return offsetof(AsioBlockable, m_bits);
  }

  AsioBlockable* getPrevParent() const {
    return reinterpret_cast<AsioBlockable*>(m_bits & kParentMask);
  }

  Kind getKind() const {
    return static_cast<Kind>(m_bits & kKindMask);
  }

  // return ptr to the WH containing this blockable
  c_WaitableWaitHandle* getWaitHandle() const;

  void setPrevParent(AsioBlockable* parent, Kind kind) {
    assertx(!(reinterpret_cast<intptr_t>(parent) & ~kParentMask));
    assertx(!(static_cast<intptr_t>(kind) & kParentMask));
    m_bits = reinterpret_cast<intptr_t>(parent) | static_cast<intptr_t>(kind);
  }

  // Only update the prev parent w/o changing kind.
  void updatePrevParent(AsioBlockable* parent) {
    assertx(!(reinterpret_cast<intptr_t>(parent) & ~kParentMask));
    m_bits = (m_bits & ~kParentMask) | reinterpret_cast<intptr_t>(parent);
  }

  static constexpr uint64_t kKindMask   = 7UL;
  static constexpr uint64_t kParentMask = ~kKindMask;

private:
  // m_bits stores kind in the lowest 3 bits and prev parent in the
  // upper 61 bits.
  // +--- Layout for m_bits (64 bits) ---+
  // | 63.....................3 | 2....0 |
  // | [Pointer to prev parent] | [Kind] |
  // +-----------------------------------+
  union {
    uintptr_t m_bits;
    AsioBlockable* m_prev;
  };
  TYPE_SCAN_CUSTOM() {
    scanner.scan(m_prev);
  }
};

/*
 * AsioBlockableChain is an intrusive linked list of parent
 * waithandles, formed by AsioBlockable fields in each waithandle.
 */
struct AsioBlockableChain final {
  static constexpr ptrdiff_t lastParentOff() {
    return offsetof(AsioBlockableChain, m_lastParent);
  }

  void init() noexcept {
    m_lastParent = nullptr;
  }

  void addParent(AsioBlockable& parent, AsioBlockable::Kind kind) {
    parent.setPrevParent(m_lastParent, kind);
    m_lastParent = &parent;
  }

  static void Unblock(AsioBlockableChain chain) { chain.unblock(); }
  static void UnblockJitHelper(ActRec* ar,
                               TypedValue* sp,
                               AsioBlockableChain chain);
  void unblock();
  void exitContext(context_idx_t ctx_idx);
  void removeFromChain(AsioBlockable* parent);
  c_WaitableWaitHandle* firstInContext(context_idx_t ctx_idx);

private:
  AsioBlockable* m_lastParent;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_BLOCKABLE_H_

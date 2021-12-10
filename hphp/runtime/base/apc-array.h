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

#include "hphp/runtime/base/apc-handle-defs.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/base/vanilla-keyset.h"
#include "hphp/runtime/base/vanilla-vec.h"
#include "hphp/util/atomic.h"
#include "hphp/util/hash.h"
#include "hphp/util/lock.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct APCArray {
  static APCHandle::Pair MakeSharedVec(ArrayData* data,
                                       APCHandleLevel level,
                                       bool unserializeObj);
  static APCHandle::Pair MakeSharedDict(ArrayData* data,
                                        APCHandleLevel level,
                                        bool unserializeObj);
  static APCHandle::Pair MakeSharedKeyset(ArrayData* data,
                                          APCHandleLevel level,
                                          bool unserializeObj);

  static APCHandle* MakeUncountedArray(
      ArrayData* ad, DataWalker::PointerMap* seen);

  static APCHandle::Pair MakeSharedEmptyVec();
  static void Delete(APCHandle* handle);

  static APCArray* fromHandle(APCHandle* handle) {
    assertx(handle->checkInvariants());
    assertx(handle->kind() == APCKind::SharedVec ||
            handle->kind() == APCKind::SharedLegacyVec ||
            handle->kind() == APCKind::SharedDict ||
            handle->kind() == APCKind::SharedLegacyDict ||
            handle->kind() == APCKind::SharedKeyset);
    static_assert(offsetof(APCArray, m_handle) == 0, "");
    return reinterpret_cast<APCArray*>(handle);
  }

  static const APCArray* fromHandle(const APCHandle* handle) {
    assertx(handle->checkInvariants());
    assertx(handle->kind() == APCKind::SharedVec ||
            handle->kind() == APCKind::SharedLegacyVec ||
            handle->kind() == APCKind::SharedDict ||
            handle->kind() == APCKind::SharedLegacyDict ||
            handle->kind() == APCKind::SharedKeyset);
    static_assert(offsetof(APCArray, m_handle) == 0, "");
    return reinterpret_cast<const APCArray*>(handle);
  }

  ArrayData* toLocalVec() const {
    return VanillaVec::MakeVecFromAPC(this);
  }
  ArrayData* toLocalLegacyVec() const {
    return VanillaVec::MakeVecFromAPC(this, /*isLegacy=*/true);
  }
  ArrayData* toLocalDict() const {
    return VanillaDict::MakeDictFromAPC(this);
  }
  ArrayData* toLocalLegacyDict() const {
    return VanillaDict::MakeDictFromAPC(this, /*isLegacy=*/true);
  }
  ArrayData* toLocalKeyset() const {
    return VanillaKeyset::MakeSetFromAPC(this);
  }

  //
  // Array API
  //

  size_t size() const {
    return m_size;
  }

  APCHandle* getHashedKey(ssize_t pos) const {
    assertx(isHashed());
    assertx(static_cast<size_t>(pos) < m_size);
    return vals()[2 * pos];
  }

  APCHandle* getHashedVal(ssize_t pos) const {
    assertx(isHashed());
    assertx(static_cast<size_t>(pos) < m_size);
    return vals()[2 * pos + 1];
  }

  APCHandle* getPackedVal(ssize_t pos) const {
    assertx(isPacked());
    assertx(static_cast<size_t>(pos) < m_size);
    return vals()[pos];
  }

  bool isPacked() const {
    auto const k = m_handle.kind();
    return k == APCKind::SharedVec ||
           k == APCKind::SharedLegacyVec ||
           k == APCKind::SharedKeyset;
  }

  bool isHashed() const {
    auto const k = m_handle.kind();
    return k == APCKind::SharedDict || k == APCKind::SharedLegacyDict;
  }

  bool isVec() const {
    return m_handle.kind() == APCKind::SharedVec ||
           m_handle.kind() == APCKind::SharedLegacyVec;
  }

  bool isDict() const {
    return m_handle.kind() == APCKind::SharedDict ||
           m_handle.kind() == APCKind::SharedLegacyDict;
  }

  bool isKeyset() const {
    return m_handle.kind() == APCKind::SharedKeyset;
  }

private:
  enum class PackedCtor {};
  APCArray(PackedCtor, APCKind kind, size_t size)
    : m_handle(kind, kInvalidDataType), m_size(size) {
    assertx(isPacked());
  }

  enum class HashedCtor {};
  APCArray(HashedCtor, APCKind kind, size_t size)
    : m_handle(kind, kInvalidDataType), m_size(size) {
    assertx(isHashed());
  }
  ~APCArray();

  APCArray(const APCArray&) = delete;
  APCArray& operator=(const APCArray&) = delete;

private:
  template <typename A, typename B>
  static APCHandle::Pair MakeSharedImpl(ArrayData*, APCHandleLevel, A, B);

  static APCHandle::Pair MakeHash(ArrayData* data, APCKind kind,
                                  bool unserializeObj);
  static APCHandle::Pair MakePacked(ArrayData* data, APCKind kind,
                                    bool unserializeObj);

private:
  friend size_t getMemSize(const APCArray*);

  // For isHashed() - i.e. dicts - an array of alternating (key, value) pairs;
  // for isPacked() - i.e. vecs and keysets - an array of elements;
  APCHandle** vals() const { return (APCHandle**)(this + 1); }

  APCHandle* getHandle() {
    return &m_handle;
  }
  const APCHandle* getHandle() const {
    return &m_handle;
  }

private:
  APCHandle m_handle;
  uint32_t m_size;
};

//////////////////////////////////////////////////////////////////////

}


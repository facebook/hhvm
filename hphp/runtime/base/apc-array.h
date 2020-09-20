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
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/util/atomic.h"
#include "hphp/util/lock.h"
#include "hphp/util/hash.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct APCArray {
  static APCHandle::Pair MakeSharedArray(ArrayData* data,
                                         APCHandleLevel level,
                                         bool unserializeObj);
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
      ArrayData* array, DataWalker::PointerMap* m);
  static APCHandle* MakeUncountedVec(
      ArrayData* vec, DataWalker::PointerMap* m);
  static APCHandle* MakeUncountedDict(
      ArrayData* dict, DataWalker::PointerMap* m);
  static APCHandle* MakeUncountedKeyset(
      ArrayData* dict, DataWalker::PointerMap* m);

  static APCHandle::Pair MakeSharedEmptyArray();
  static void Delete(APCHandle* handle);

  static APCArray* fromHandle(APCHandle* handle) {
    assertx(handle->checkInvariants());
    assertx(handle->kind() == APCKind::SharedArray ||
           handle->kind() == APCKind::SharedPackedArray ||
           handle->kind() == APCKind::SharedVArray ||
           handle->kind() == APCKind::SharedMarkedVArray ||
           handle->kind() == APCKind::SharedDArray ||
           handle->kind() == APCKind::SharedMarkedDArray ||
           handle->kind() == APCKind::SharedVec ||
           handle->kind() == APCKind::SharedLegacyVec ||
           handle->kind() == APCKind::SharedDict ||
           handle->kind() == APCKind::SharedLegacyDict ||
           handle->kind() == APCKind::SharedKeyset);
    static_assert(offsetof(APCArray, m_handle) == 0, "");
    return reinterpret_cast<APCArray*>(handle);
  }

  static const APCArray* fromHandle(const APCHandle* handle) {
    assertx(handle->checkInvariants());
    assertx(handle->kind() == APCKind::SharedArray ||
           handle->kind() == APCKind::SharedPackedArray ||
           handle->kind() == APCKind::SharedVArray ||
           handle->kind() == APCKind::SharedMarkedVArray ||
           handle->kind() == APCKind::SharedDArray ||
           handle->kind() == APCKind::SharedMarkedDArray ||
           handle->kind() == APCKind::SharedVec ||
           handle->kind() == APCKind::SharedLegacyVec ||
           handle->kind() == APCKind::SharedDict ||
           handle->kind() == APCKind::SharedLegacyDict ||
           handle->kind() == APCKind::SharedKeyset);
    static_assert(offsetof(APCArray, m_handle) == 0, "");
    return reinterpret_cast<const APCArray*>(handle);
  }

  ArrayData* toLocalArray() const {
    // We don't have direct constructors for "plain" PHP arrays, so convert a
    // Hack array of the appropriate type instead. Since it has refcount 1,
    // we'll do the conversion in place rather than making a copy.
    auto const ad = isPacked() ? PackedArray::MakeVecFromAPC(this)
                               : MixedArray::MakeDictFromAPC(this);
    return ad->toPHPArray(/*copy=*/false);
  }
  ArrayData* toLocalVArray() const {
    assertx(!RuntimeOption::EvalHackArrDVArrs);
    return PackedArray::MakeVArrayFromAPC(this);
  }
  ArrayData* toLocalMarkedVArray() const {
    assertx(!RuntimeOption::EvalHackArrDVArrs);
    return PackedArray::MakeVArrayFromAPC(this, /*isMarked=*/true);
  }
  ArrayData* toLocalDArray() const {
    assertx(!RuntimeOption::EvalHackArrDVArrs);
    return MixedArray::MakeDArrayFromAPC(this);
  }
  ArrayData* toLocalMarkedDArray() const {
    assertx(!RuntimeOption::EvalHackArrDVArrs);
    return MixedArray::MakeDArrayFromAPC(this, /*isMarked=*/true);
  }
  ArrayData* toLocalVec() const { return PackedArray::MakeVecFromAPC(this); }
  ArrayData* toLocalLegacyVec() const {
    return PackedArray::MakeVecFromAPC(this, /*isLegacy=*/true);
  }
  ArrayData* toLocalDict() const { return MixedArray::MakeDictFromAPC(this); }
  ArrayData* toLocalLegacyDict() const {
    return MixedArray::MakeDictFromAPC(this, /*isLegacy=*/true);
  }
  ArrayData* toLocalKeyset() const { return SetArray::MakeSetFromAPC(this); }

  //
  // Array API
  //

  size_t size() const {
    return isPacked() ? m_size : m.m_num;
  }

  Variant getKey(ssize_t pos) const {
    if (isPacked()) {
      assertx(static_cast<size_t>(pos) < m_size);
      return pos;
    }
    assertx(static_cast<size_t>(pos) < m.m_num);
    return buckets()[pos].key->toLocal();
  }

  APCHandle* getValue(ssize_t pos) const {
    if (isPacked()) {
      assertx(static_cast<size_t>(pos) < m_size);
      return vals()[pos];
    }
    assertx(static_cast<size_t>(pos) < m.m_num);
    return buckets()[pos].val;
  }

  ssize_t getIndex(const StringData* key) const {
    return isPacked() ? -1 : indexOf(key);
  }

  ssize_t getIndex(int64_t key) const {
    if (isPacked()) {
      return (static_cast<uint64_t>(key) >= m_size) ? -1 : key;
    }
    return indexOf(key);
  }

  bool isPacked() const {
    auto const k = m_handle.kind();
    return
      k == APCKind::SharedPackedArray ||
      k == APCKind::SharedVArray ||
      k == APCKind::SharedMarkedVArray ||
      k == APCKind::SharedVec ||
      k == APCKind::SharedLegacyVec ||
      k == APCKind::SharedKeyset;
  }

  bool isHashed() const {
    auto const k = m_handle.kind();
    return
      k == APCKind::SharedArray ||
      k == APCKind::SharedDArray ||
      k == APCKind::SharedMarkedDArray ||
      k == APCKind::SharedDict ||
      k == APCKind::SharedLegacyDict;
  }

  bool isPHPArray() const {
    auto const k = m_handle.kind();
    return
      k == APCKind::SharedArray ||
      k == APCKind::SharedPackedArray ||
      k == APCKind::SharedVArray ||
      k == APCKind::SharedMarkedVArray ||
      k == APCKind::SharedDArray ||
      k == APCKind::SharedMarkedDArray;
  }

  bool isVArray() const {
    return m_handle.kind() == APCKind::SharedVArray ||
           m_handle.kind() == APCKind::SharedMarkedVArray;
  }

  bool isDArray() const {
    return m_handle.kind() == APCKind::SharedDArray ||
           m_handle.kind() == APCKind::SharedMarkedDArray;
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
  struct Bucket {
    /** index of the next bucket, or -1 if the end of a chain */
    int next;
    /** the value of this bucket */
    APCHandle *key;
    APCHandle *val;
  };

private:
  enum class PackedCtor {};
  APCArray(PackedCtor, APCKind kind, size_t size)
    : m_handle(kind, kInvalidDataType), m_size(size) {
    assertx(isPacked());
  }

  enum class HashedCtor {};
  APCArray(HashedCtor, APCKind kind, unsigned int cap) : m_handle(kind) {
    assertx(isHashed());
    m.m_capacity_mask = cap - 1;
    m.m_num = 0;
  }
  ~APCArray();

  APCArray(const APCArray&) = delete;
  APCArray& operator=(const APCArray&) = delete;

private:
  template <typename A, typename B, typename C>
  static APCHandle::Pair MakeSharedImpl(ArrayData*, APCHandleLevel, A, B, C);

  static APCHandle::Pair MakeHash(ArrayData* data, APCKind kind,
                                  bool unserializeObj);
  static APCHandle::Pair MakePacked(ArrayData* data, APCKind kind,
                                    bool unserializeObj);

private:
  friend size_t getMemSize(const APCArray*);

  void add(APCHandle* key, APCHandle* val);
  ssize_t indexOf(const StringData* key) const;
  ssize_t indexOf(int64_t key) const;

  /* index of the beginning of each hash chain */
  int* hash() const { return (int*)(this + 1); }
  /* buckets, stored in index order */
  Bucket* buckets() const { return (Bucket*)(hash() + m.m_capacity_mask + 1); }
  /* start of the data for packed array */
  APCHandle** vals() const { return (APCHandle**)(this + 1); }

  APCHandle* getHandle() {
    return &m_handle;
  }
  const APCHandle* getHandle() const {
    return &m_handle;
  }

private:
  APCHandle m_handle;
  union {
    // for map style arrays
    struct {
      unsigned int m_capacity_mask;
      unsigned int m_num;
    } m;
    // for packed arrays
    size_t m_size;
  };
};

//////////////////////////////////////////////////////////////////////

}


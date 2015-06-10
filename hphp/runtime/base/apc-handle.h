/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_APC_HANDLE_H_
#define incl_HPHP_APC_HANDLE_H_

#include <atomic>

#include "hphp/util/atomic.h"
#include "hphp/util/hash.h"
#include "hphp/util/lock.h"

#include "hphp/runtime/base/type-variant.h"

#if (defined(__APPLE__) || defined(__APPLE_CC__)) && (defined(__BIG_ENDIAN__) || defined(__LITTLE_ENDIAN__))
# if defined(__LITTLE_ENDIAN__)
#  undef WORDS_BIGENDIAN
# else
#  if defined(__BIG_ENDIAN__)
#   define WORDS_BIGENDIAN
#  endif
# endif
#endif

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * An APCHandle is the externally visible handle for in-memory APC values.  The
 * main role of the APCHandle is to hold the type information of the value,
 * manage the lifetime.  When new values are added to APC, they are created
 * using APCHandle::Create.
 *
 * Internally the family of APC entities (all APCXXXX classes, e.g. APCString,
 * APCArray, ...) embed the handle and provide an API to return the instance
 * from the handle pointer. Examples:
 *
 *    APCString                         APCTypedValue
 *   ----------------                  --------------
 *   | APCHandle    |                  | SharedData |
 *   | StringData   |                  | APCHandle  |
 *   ----------------                  --------------
 *
 * So, for an APCString the caller gets back an APCHandle* pointing to the
 * location of the field in the class.  APCString::fromHandle(APCHandle*
 * handle) will simply return the pointer itself as the handle sits at offset 0
 * (reinterpret_cast<APCString*>(handle)).  For an APCTypedValue,
 * APCTypedValue::fromHandle(APCHandle* handle) would return the handle pointer
 * minus the offset (in words) of the handle.  Normally the handle sits at
 * position 0 so the call ends up being a no-op.
 *
 * APCTypedValue is, at the moment, the only entity with the handle at a
 * non-zero offset and that is because the layout of APCTypedValue and
 * TypedValue have to be the same, and we let the DataType field in APCHandle
 * be used through a TypedValue*.  This layout allows for an important
 * optimization when an APCTypedValue is contained inside an APCArray---in such
 * cases the APCTypedValue is returned directly instead of creating another
 * TypedValue.
 *
 * It's important to point out that APCHandle::type() is not enough to
 * determine the APCXXX class behind the APCHandle.  That is because both
 * APCObject and APCArray can have a serialized form that it is just a string
 * (see isSerializedArray and isSerializedObj).
 *
 * Thread safety:
 *
 *    const-qualified member functions on this class are safe for concurrent
 *    use by multiple threads, as long as no other thread may be calling any
 *    non-const member functions that are not documented as exceptions to this
 *    rule.
 *
 */
struct APCHandle {
  struct Pair {
    APCHandle* handle;
    size_t size;
  };

  /*
   * Create an instance of an APC object according to the type of source and
   * the various flags. This is the only entry point to create APC entities.
   */
  static Pair Create(const Variant& source, bool serialized,
                     bool inner = false, bool unserializeObj = false);

  /*
   * Memory management API.
   *
   * APC handles can be managed both by eager reference counting and via the
   * Treadmill.  Memory management operations on APC handles go through this
   * API to hide which scheme is being used from users of APCHandle.  The
   * active scheme can be different for different handles in the same process.
   *
   *          function | Uncounted |  Counted
   *  -----------------+-----------+-----------
   *         reference |   no-op   |  incref
   *       unreference |   no-op   |  decref
   *   unreferenceRoot | treadmill |  decref
   *
   * The `size' argument to unreferenceRoot is only use for stats collection,
   * so zero may be passed in some situations.
   *
   * unreferenceRoot may be called while this APCHandle is still being read by
   * other threads---it is an exception to the thread-safety rule documented
   * above the class.
   */
  void reference() const {
    if (!isUncounted()) {
      realIncRef();
    }
  }
  void unreference() const {
    if (!isUncounted()) {
      realDecRef();
    }
  }
  void unreferenceRoot(size_t size);

  /*
   * Get an instance of the PHP object represented by this APCHandle. The
   * instance returned will be local to the request/thread that performed
   * the call.
   */
  Variant toLocal() const;

  /*
   * Return the PHP DataType represented by this APCHandle.
   *
   * Note that this does not entirely determine the type of APC storage being
   * used---for example, objects and arrays can be represented as serialized
   * APCStrings, in which case type() will still KindOfObject or KindOfArray.
   * See isSerializedArray and isSerializedObj below.
   */
  DataType type() const { return m_type; }

  /*
   * When we load KindOfObjects that are !isObj() (i.e. they are back by an
   * APCString containing a serialized object), we may attempt to convert it to
   * an APCObject representation.  If the conversion is not possible (for
   * example, if there are internal references), we set this flag so that we
   * don't try over and over.
   *
   * The non-const setObjAttempted function is safe for concurrent use with
   * multiple readers and writers on a live APCHandle---it is an exception to
   * the thread-safety rule documented above the class.
   */
  bool objAttempted() const {
    return m_obj_attempted.load(std::memory_order_relaxed);
  }
  void setObjAttempted() {
    m_obj_attempted.store(true, std::memory_order_relaxed);
  }

  /*
   * For KindOfObject and KindOfArray, reprectively, these flags distinguish
   * between the APCObject and APCArray representations and serialized
   * APCString representations.  And an APCCollection wraps an array that
   * represents a php KindOfObject for a particular collection type.
   */
  bool isSerializedObj() const { return m_flags & FSerializedObj; }
  bool isSerializedArray() const { return m_flags & FSerializedArray; }
  bool isAPCCollection() const { return m_flags & FAPCCollection; }
  void setSerializedObj() { m_flags |= FSerializedObj; }
  void setSerializedArray() { m_flags |= FSerializedArray; }
  void setAPCCollection() { m_flags |= FAPCCollection; }

  /*
   * If true, this APCHandle is not using reference counting.
   */
  bool isUncounted() const { return m_flags & FUncounted; }
  void setUncounted() { m_flags |= FUncounted; }

  /*
   * If this APCHandle is using an APCArray representation, this flag
   * discriminates between two different storage schemes inside APCArray.
   */
  bool isPacked() const { return m_flags & FPacked; }
  void setPacked() { m_flags |= FPacked; }

private:
  constexpr static uint8_t FSerializedArray = 1 << 0;
  constexpr static uint8_t FSerializedObj   = 1 << 1;
  constexpr static uint8_t FPacked          = 1 << 2;
  constexpr static uint8_t FUncounted       = 1 << 3;
  constexpr static uint8_t FAPCCollection   = 1 << 4;

private:
  friend struct APCTypedValue;
  friend struct APCString;
  friend struct APCArray;
  friend struct APCObject;
  friend struct APCCollection;
  explicit APCHandle(DataType type) : m_type(type) {}
  APCHandle(const APCHandle&) = delete;
  APCHandle& operator=(APCHandle const&) = delete;

private:
  void realIncRef() const {
    assert(IS_REFCOUNTED_TYPE(m_type));
    ++m_count;
  }

  void realDecRef() const {
    assert(m_count.load() > 0);
    if (m_count > 1) {
      assert(IS_REFCOUNTED_TYPE(m_type));
      if (--m_count) return;
    }
    const_cast<APCHandle*>(this)->deleteShared();
  }

  void deleteShared();

private:
  DataType m_type;
  std::atomic<uint8_t> m_obj_attempted{false};
  uint8_t m_flags{0};
  mutable std::atomic<uint32_t> m_count{1};
};

///////////////////////////////////////////////////////////////////////////////
}

#endif

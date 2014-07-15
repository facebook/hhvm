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

#ifndef incl_HPHP_APC_COLLECTION_H_
#define incl_HPHP_APC_COLLECTION_H_

#include <cinttypes>

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/apc-handle.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Representation of a collection stored in APC.
 * An APCCollection comes in one of two forms:
 * 1- the collection is backed up by an uncounted array, in which case
 * the uncounted array is stored and used to create the collection with no
 * extra copy
 * 2- the collection is not built on the hphp array implementation or
 * it contains data that is not allowed in an uncounted array. In that case
 * an APCArray is built and we loop through it to create the original
 * collection.
 */
struct APCCollection {
  // Create an APCCollection from a php collection; returns its APCHandle.
  static APCHandle* MakeShared(ObjectData* data, size_t& size, bool inner);

  // Return an instance of a PHP collection from the given apc handle
  static Variant MakeObject(APCHandle* handle);

  // Delete the APC collection holding the object data
  static void Delete(APCHandle* handle);

  static APCCollection* fromHandle(APCHandle* handle) {
    assert(offsetof(APCCollection, m_handle) == 0);
    return reinterpret_cast<APCCollection*>(handle);
  }

  static const APCCollection* fromHandle(const APCHandle* handle) {
    assert(offsetof(APCCollection, m_handle) == 0);
    return reinterpret_cast<const APCCollection*>(handle);
  }

  APCHandle* getHandle() { return &m_handle; }

private:
  friend size_t getMemSize(const APCCollection*);

private:
  explicit APCCollection(APCHandle*, Collection::Type, uint32_t);
  ~APCCollection();
  APCCollection(const APCCollection&) = delete;
  APCCollection& operator=(const APCCollection&) = delete;

private:
  APCHandle m_handle;
  APCHandle* m_data; // the uncounted array containing the data
  Collection::Type m_type;
  uint32_t m_size; // size of bytes of m_data
};

//////////////////////////////////////////////////////////////////////

}

#endif

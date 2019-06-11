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
#ifndef incl_HPHP_WEAKREF_DATA_H_
#define incl_HPHP_WEAKREF_DATA_H_

#include "hphp/runtime/base/req-memory.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/util/type-scan.h"

namespace HPHP {

struct Object;

// A single WeakRefData is shared between all WeakRefs to a given refcounted
// object.
struct WeakRefData {
  TypedValue pointee; // Object being weakly referenced.
  TYPE_SCAN_CUSTOM() {
    scanner.weak(this);
  }

  // Invalidate WeakRefData associated with a refcounted object.
  static void invalidateWeakRef(uintptr_t ptr);

  // Create or find WeakRefData for a given refcounted object.
  static req::shared_ptr<WeakRefData> forObject(Object obj);

  ~WeakRefData();
  explicit WeakRefData(const TypedValue& tv): pointee(tv) {}

  bool isValid() const;
};

void weakref_cleanup();

} // namespace HPHP
#endif // incl_HPHP_WEAKREF_DATA_H_

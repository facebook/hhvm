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

#ifndef incl_HPHP_WEAKREF_DATA_HANDLE_H_
#define incl_HPHP_WEAKREF_DATA_HANDLE_H_

#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/weakref-data.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/type-scan.h"

namespace HPHP {

struct WeakRefDataHandle final {
  // We share the general validity and pointer between WeakRefHandles.
  req::shared_ptr<WeakRefData> wr_data;
  int32_t acquire_count;
  TYPE_SCAN_CUSTOM_FIELD(wr_data) {
    // If we've acquired the data, mark it, otherwise, we might need to be
    // nulled out.
    if (wr_data) {
      if (acquire_count) {
        scanner.scan(wr_data->pointee);
      } else {
        scanner.weak(this);
      }
    }
  }

  WeakRefDataHandle(const WeakRefDataHandle&) = delete;

  WeakRefDataHandle(): wr_data(nullptr), acquire_count(0) {}
  WeakRefDataHandle& operator=(const WeakRefDataHandle& other);

  ~WeakRefDataHandle();

  void sweep() {}
};

}
#endif // incl_HPHP_WEAKREF_DATA_HANDLE_H_

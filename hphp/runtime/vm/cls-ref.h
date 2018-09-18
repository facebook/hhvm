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

#ifndef incl_HPHP_VM_CLSREF_H_
#define incl_HPHP_VM_CLSREF_H_

#include "hphp/util/low-ptr.h"

namespace HPHP {

struct ArrayData;
struct Class;

// Structure that contains the class and its reified types
struct cls_ref {
  ArrayData* reified_types;
  LowPtr<Class> cls;

  static constexpr ptrdiff_t reifiedOff() {
    return offsetof(cls_ref, reified_types);
  }

  static constexpr ptrdiff_t clsOff() {
    return offsetof(cls_ref, cls);
  }
};

}

#endif

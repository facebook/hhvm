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
#include "hphp/hack/src/hackc/hhvm_cxx/hhvm_types/as-base-ffi.h"

namespace HPHP {

rust::String attrs_to_string_ffi(AttrContext ctx, Attr attrs) {
  return attrs_to_string(ctx, attrs);
}

rust::String type_flags_to_string_ffi(TypeConstraintFlags flags) {
  return type_flags_to_string(flags);
}

}

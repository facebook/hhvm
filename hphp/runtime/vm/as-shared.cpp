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
#include "hphp/runtime/vm/as-shared.h"

#include "hphp/runtime/vm/as-base-hhas.h"

#include <folly/gen/Base.h>
#include <folly/gen/String.h>

namespace HPHP {

Optional<TypeConstraintFlags> string_to_type_flag(
  const std::string& name) {
#define X(flag, str) \
  if (name == str) return TypeConstraintFlags::flag;
  HHAS_TYPE_FLAGS
#undef X

return std::nullopt;
}

Optional<Attr> string_to_attr(AttrContext ctx,
                              const std::string& name) {
#define X(attr, mask, str) \
  if (supported(mask, ctx) && name == str) return attr;
  HHAS_ATTRS
#undef X

return std::nullopt;
}

}

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
#include "hphp/runtime/vm/as-base.h"

#include "hphp/runtime/vm/as-base-hhas.h"

#include <folly/gen/String.h>

namespace HPHP {

std::vector<std::string> attrs_to_vec(AttrContext ctx, Attr attrs) {
  std::vector<std::string> vec;

#define X(attr, mask, str) \
  if (supported(mask, ctx) && (attrs & attr)) vec.push_back(str);
  HHAS_ATTRS
#undef X

  return vec;
}

std::string attrs_to_string(AttrContext ctx, Attr attrs) {
  using namespace folly::gen;
  return from(attrs_to_vec(ctx, attrs)) | unsplit<std::string>(" ");
}

std::string type_flags_to_string(TypeConstraintFlags flags) {
  std::vector<std::string> vec;

#define X(flag, str) \
  if (flags & TypeConstraintFlags::flag) vec.push_back(str);
  HHAS_TYPE_FLAGS
#undef X

  using namespace folly::gen;
  return from(vec) | unsplit<std::string>(" ");
}

}

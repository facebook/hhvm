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

#if defined(FOLLY_PACKAGE_VERSION)
// This file is included in the hackc binary build where we can't easily include
// folly.
#error Folly not allowed in this source file.
#endif

namespace HPHP {

namespace {
std::string join_with_spaces(const std::vector<std::string> &input) {
  size_t len = 0;
  for (const auto& piece: input) {
    len += piece.length() + 1;
  }

  std::string result;
  result.reserve(len);

  for (const auto& piece: input) {
    if (!result.empty()) {
      result.push_back(' ');
    }
    result.append(piece);
  }

  return result;
}
} // anonymous namespace

std::vector<std::string> attrs_to_vec(AttrContext ctx, Attr attrs) {
  std::vector<std::string> vec;

#define X(attr, mask, str) \
  if (supported(mask, ctx) && (attrs & attr)) vec.push_back(str);
  HHAS_ATTRS
#undef X

  return vec;
}

std::string attrs_to_string(AttrContext ctx, Attr attrs) {
  return join_with_spaces(attrs_to_vec(ctx, attrs));
}

std::string type_flags_to_string(TypeConstraintFlags flags) {
  std::vector<std::string> vec;

#define X(flag, str) \
  if (contains(flags, TypeConstraintFlags::flag)) vec.push_back(str);
  HHAS_TYPE_FLAGS
#undef X

  return join_with_spaces(vec);
}

std::string fcall_flags_to_string(FCallArgsFlags flags) {
  std::vector<std::string> vec;

#define X(flag, str) \
  if (flags & FCallArgsFlags::flag) vec.push_back(str);
  HHAS_FCALL_FLAGS
#undef X

  return join_with_spaces(vec);
}

}

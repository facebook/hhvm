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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/base/vanilla-vec.h"
#include "hphp/runtime/base/vanilla-vec-defs.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/builtin-functions.h"

#include <algorithm>
#include <limits>

namespace HPHP {

Array HHVM_FUNCTION(sandbox_dict_with_capacity, int64_t capacity) {
  if (capacity <= 0) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "HH\\Sandbox\\dict_with_capacity expects a positive capacity"
    );
  }
  if (Cfg::Repo::Authoritative) {
    return Array::attach(staticEmptyDictArray());
  }
  // Cap at reasonable max to avoid OOM
  auto cap = static_cast<uint32_t>(
    std::min<int64_t>(capacity, VanillaDict::MaxSize)
  );
  return Array::attach(VanillaDict::MakeReserveDict(cap));
}

Array HHVM_FUNCTION(sandbox_vec_with_capacity, int64_t capacity) {
  if (capacity <= 0) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "HH\\Sandbox\\vec_with_capacity expects a positive capacity"
    );
  }
  if (Cfg::Repo::Authoritative) {
    return Array::attach(staticEmptyVec());
  }
  // Cap at reasonable max to avoid OOM
  auto cap = static_cast<uint32_t>(
    std::min<int64_t>(capacity, VanillaDict::MaxSize)
  );
  return Array::attach(VanillaVec::MakeReserveVec(cap));
}

namespace {

struct SandboxExtension final : Extension {
  SandboxExtension() : Extension("sandbox", "1.0", "sandbox_infra") {}

  void moduleRegisterNative() override {
    HHVM_NAMED_FE(HH\\Sandbox\\dict_with_capacity,
                  HHVM_FN(sandbox_dict_with_capacity));
    HHVM_NAMED_FE(HH\\Sandbox\\vec_with_capacity,
                  HHVM_FN(sandbox_vec_with_capacity));
  }
} s_sandbox_extension;

} // anonymous namespace
} // namespace HPHP

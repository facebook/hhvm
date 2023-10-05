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

#include "hphp/runtime/base/record-replay.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

namespace {
  static std::unordered_map<NativeFunction, std::string> g_nativeFuncNames;

  static const std::unordered_set<std::string> g_noRecordingPrefixes{
    "DateTime->",
    "HH\\ImmMap->",
    "HH\\ImmSet->",
    "HH\\ImmVector->",
    "HH\\Map->",
    "HH\\Set->",
    "HH\\Vector->",
    "NativeTrimmableMap->",
    "NativeTrimmableMap2->",
    "NativeTrimmableMap3->",
  };

  static const std::unordered_set<std::string> g_noRecordingSuffixes{
    "->__sleep",
    "->__wakeup",
  };
} // namespace

void addNativeFuncName(NativeFunction ptr, std::string_view name) {
  g_nativeFuncNames[ptr] = name;
}

std::string_view getNativeFuncName(NativeFunction ptr) {
  return g_nativeFuncNames[ptr];
}

NativeFunction getNativeFuncPtr(std::string_view name) {
  for (const auto& [ptr, n] : g_nativeFuncNames) {
    if (n == name) {
      return ptr;
    }
  }
  return nullptr;
}

bool shouldRecordReplay(NativeFunction ptr) {
  const auto name{g_nativeFuncNames[ptr]};
  for (const auto& prefix : g_noRecordingPrefixes) {
    if (name.starts_with(prefix)) {
      return false;
    }
  }
  for (const auto& suffix : g_noRecordingSuffixes) {
    if (name.ends_with(suffix)) {
      return false;
    }
  }
  auto func{vmfp()->func()};
  if (func->nativeFuncPtr() != ptr) {
    func = Func::load(StringData::Make(name));
  }
  assert(func != nullptr);
  const auto attrs{func->attrs()};
  return !((attrs & AttrNoRecording) | (attrs & AttrIsFoldable));
}

} // namespace HPHP

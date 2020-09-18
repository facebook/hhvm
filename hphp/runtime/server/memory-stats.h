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

#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <string>

#include "hphp/runtime/server/writer.h"

namespace HPHP {

enum class AllocKind {
  StaticString,
  StaticArray,
  Unit,
  Class,
  Func,
  NumKinds
};

struct MemoryStats {
  static void ReportMemory(std::string &out, Writer::Format format);

  static void ResetStaticStringSize() {
    s_allocSizes[static_cast<unsigned>(AllocKind::StaticString)]
      .store(0, std::memory_order_relaxed);
  }

  static void LogAlloc(AllocKind kind, size_t bytes) {
    s_allocSizes[static_cast<unsigned>(kind)]
      .fetch_add(bytes, std::memory_order_relaxed);
  }

 private:
  static size_t totalSize(AllocKind kind) {
    return s_allocSizes[static_cast<unsigned>(kind)]
      .load(std::memory_order_relaxed);
  }

  static auto constexpr N = static_cast<unsigned>(AllocKind::NumKinds);
  static std::array<std::atomic_size_t, N> s_allocSizes;
};

}


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

#include <cstdint>
#include <utility>
#include <vector>

#include "hphp/runtime/vm/jit/cont-prof-key.h"

namespace HPHP::jit {

struct ContProfRecordHeader {
  ContProfFuncKey funcKey{};
  uint64_t capturedAtMs{0};

  bool operator==(const ContProfRecordHeader&) const = default;
};

enum class ContProfStartKind : uint8_t {
  FuncEntry = 1,
  NamedParamsFuncEntry = 2,
};

struct ContProfProfileTranslation {
  ContProfStartKind startKind{ContProfStartKind::FuncEntry};
  uint32_t numEntryArgs{0};
  uint32_t regionLength{0};
  uint64_t executionCount{0};

  auto startKey() const {
    return std::pair{startKind, numEntryArgs};
  }

  bool operator==(const ContProfProfileTranslation&) const = default;
};

/* The initial format is intentionally entry-only. */
struct ContProfProfileRecord {
  ContProfRecordHeader header{};
  std::vector<ContProfProfileTranslation> translations;

  uint64_t functionExecutions() const;
  bool hasCanonicalTranslationOrder() const;

  bool operator==(const ContProfProfileRecord&) const = default;
};

/* Validate the record's structural and canonical-ordering invariants. */
bool isValidContProfProfileRecord(const ContProfProfileRecord&);

}

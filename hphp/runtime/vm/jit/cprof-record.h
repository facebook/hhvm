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

#include <compare>
#include <cstdint>
#include <utility>
#include <vector>

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/vm/jit/cprof-key.h"

namespace HPHP::jit::cprof {

struct ContProfRecordHeader {
  ContProfFuncKey funcKey{};
  uint64_t capturedAtMs{0};

  bool operator==(const ContProfRecordHeader&) const = default;
};

enum class ContProfStartKind : uint8_t {
  FuncEntry = 1,
  NamedParamsFuncEntry = 2,
  // A single-block ResumeMode::None region with an empty initial stack.
  Bytecode = 3,
};

struct ContProfLocalTypeGuard {
  uint32_t localId{0};
  DataType type{kInvalidDataType};

  std::strong_ordering operator<=>(const ContProfLocalTypeGuard&) const
    = default;
};

struct ContProfLocalPostCondition {
  uint32_t localId{0};
  // Whether the local may have been overwritten, rather than just refined.
  bool changed{false};
  // kInvalidDataType means a changed local's type is unknown (TCell on replay).
  DataType type{kInvalidDataType};

  std::strong_ordering operator<=>(const ContProfLocalPostCondition&) const
    = default;
};

struct ContProfProfileTranslation {
  ContProfStartKind startKind{ContProfStartKind::FuncEntry};
  // Shared payload, like SrcKey: bytecode offset or FuncEntry argument count.
  // NamedParamsFuncEntry uses zero; use accessors for kind-specific reads.
  uint32_t offsetOrNumEntryArgs{0};
  // Instruction count, including the synthetic function entry when present.
  uint32_t regionLength{0};
  uint64_t executionCount{0};
  // Sorted by local ID, with at most one guard per local.
  std::vector<ContProfLocalTypeGuard> localTypeGuards;
  // Sorted, unique predecessor indices into this record's translations.
  std::vector<uint32_t> incoming;
  std::vector<ContProfLocalPostCondition> localPostConditions;

  // Valid only for Bytecode starts.
  uint32_t offset() const;
  // Valid only for FuncEntry starts, not NamedParamsFuncEntry.
  uint32_t numEntryArgs() const;

  auto startKey() const {
    return std::pair{startKind, offsetOrNumEntryArgs};
  }

  bool operator==(const ContProfProfileTranslation&) const = default;
};

struct ContProfProfileRecord {
  ContProfRecordHeader header{};
  std::vector<ContProfProfileTranslation> translations;

  // Sum entry counts only; mid-function counts can include loop iterations.
  uint64_t functionExecutions() const;
  bool hasCanonicalTranslationOrder() const;

  bool operator==(const ContProfProfileRecord&) const = default;
};

/* Validate the record's structural and canonical-ordering invariants. */
bool isValidContProfProfileRecord(const ContProfProfileRecord&);

}

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

#include "hphp/runtime/vm/jit/cont-prof-record.h"

#include <cstddef>
#include <limits>

namespace HPHP::jit {

uint64_t ContProfProfileRecord::functionExecutions() const {
  uint64_t result{};
  for (auto const& translation : translations) {
    result += translation.executionCount;
  }
  return result;
}

bool ContProfProfileRecord::hasCanonicalTranslationOrder() const {
  for (size_t i = 1; i < translations.size(); ++i) {
    if (translations[i - 1].startKey() >= translations[i].startKey()) {
      return false;
    }
  }
  return true;
}

bool isValidContProfProfileRecord(const ContProfProfileRecord& record) {
  if (!isValidContProfFuncKey(record.header.funcKey) ||
      record.header.capturedAtMs == 0 || record.translations.empty() ||
      !record.hasCanonicalTranslationOrder()) {
    return false;
  }

  uint64_t totalExecutions = 0;

  for (size_t i = 0; i < record.translations.size(); ++i) {
    auto const& translation = record.translations[i];

    switch (translation.startKind) {
      case ContProfStartKind::FuncEntry:
        break;
      case ContProfStartKind::NamedParamsFuncEntry:
        if (translation.numEntryArgs != 0) return false;
        break;
      default:
        return false;
    }

    if (translation.regionLength == 0 || translation.executionCount == 0) {
      return false;
    }

    if (translation.executionCount >
        std::numeric_limits<uint64_t>::max() - totalExecutions) {
      return false;
    }

    totalExecutions += translation.executionCount;
  }

  return true;
}

}

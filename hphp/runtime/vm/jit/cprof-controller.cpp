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

#include "hphp/runtime/vm/jit/cprof-controller.h"

#include <map>
#include <utility>
#include <vector>

#include <folly/Synchronized.h>

#include "hphp/runtime/vm/jit/cprof-capture.h"

namespace HPHP::jit::cprof {

namespace {

using RecordMap = std::map<ContProfFuncKey, ContProfProfileRecord>;
folly::Synchronized<RecordMap> s_records;

}

bool captureContProfProfile(const ProfData& profData, const Func& func) {
  auto record = snapshotContProfProfileRecord(profData, func);
  if (!record) return false;

  auto records = s_records.wlock();
  return records->try_emplace(
    record->header.funcKey,
    std::move(*record)
  ).second;
}

size_t numContProfProfileRecords() {
  auto records = s_records.rlock();
  return records->size();
}

std::vector<ContProfProfileRecord> snapshotContProfProfileRecords() {
  auto records = s_records.rlock();

  std::vector<ContProfProfileRecord> result;
  result.reserve(records->size());

  for (auto const& [_, record] : *records) {
    result.push_back(record);
  }

  return result;
}

}

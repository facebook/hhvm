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

#include "hphp/runtime/vm/jit/cprof-record.h"

#include <cstddef>
#include <limits>

#include "hphp/runtime/base/types.h"
#include "hphp/util/assertions.h"

namespace HPHP::jit::cprof {

uint32_t ContProfProfileTranslation::offset() const {
  assertx(startKind == ContProfStartKind::Bytecode);
  return offsetOrNumEntryArgs;
}

uint32_t ContProfProfileTranslation::numEntryArgs() const {
  assertx(startKind == ContProfStartKind::FuncEntry);
  return offsetOrNumEntryArgs;
}

uint64_t ContProfProfileRecord::functionExecutions() const {
  uint64_t result{};
  for (auto const& translation : translations) {
    if (translation.startKind == ContProfStartKind::Bytecode) continue;
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

  uint64_t entryExecutions{};

  for (auto const& translation : record.translations) {
    switch (translation.startKind) {
      case ContProfStartKind::FuncEntry:
        break;

      case ContProfStartKind::NamedParamsFuncEntry:
        if (translation.offsetOrNumEntryArgs != 0) return false;
        break;

      case ContProfStartKind::Bytecode:
        if (translation.offset() == 0 || translation.offset() >= kInvalidOffset) {
          return false;
        }
        break;

      default:
        return false;
    }

    if (translation.regionLength == 0 || translation.executionCount == 0) {
      return false;
    }

    auto const& guards = translation.localTypeGuards;
    for (size_t i = 0; i < guards.size(); ++i) {
      auto const& guard = guards[i];
      if (!isRealType(guard.type)) return false;
      if (i != 0 && guards[i - 1].localId >= guard.localId) {
        return false;
      }
    }
    auto const& incoming = translation.incoming;
    if (translation.startKind != ContProfStartKind::Bytecode &&
        !incoming.empty()) {
      return false;
    }

    for (size_t i = 0; i < incoming.size(); ++i) {
      auto const predecessor = incoming[i];

      if (predecessor >= record.translations.size()) return false;

      if (i != 0 && incoming[i - 1] >= predecessor) {
        return false;
      }
    }

    auto const& posts = translation.localPostConditions;
    for (size_t i = 0; i < posts.size(); ++i) {
      auto const& post = posts[i];

      if (post.type != kInvalidDataType && !isRealType(post.type)) return false;

      if (!post.changed && post.type == kInvalidDataType) return false;

      if (i != 0 && posts[i - 1].localId >= post.localId) {
        return false;
      }
    }

    auto const& profiles = translation.targetProfiles;
    for (size_t i = 0; i < profiles.size(); ++i) {
      auto const& profile = profiles[i];

      if (!isValidContProfTargetProfile(profile)) return false;

      if (i != 0 && !contProfTargetProfileKeyLess(profiles[i - 1], profile)) {
        return false;
      }
    }

    if (translation.startKind != ContProfStartKind::Bytecode) {
      if (translation.executionCount >
          std::numeric_limits<uint64_t>::max() - entryExecutions) {
        return false;
      }
      entryExecutions += translation.executionCount;
    }
  }

  return entryExecutions != 0;
}

}
